/*
  SDL_mixer:  An audio mixer library based on the SDL library
  Copyright (C) 1997-2013 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/

/* $Id$ */

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include <retro_miscellaneous.h>

#include "SDL_mixer.h"

int volatile music_active = 1;
static int volatile music_stopped = 0;
static int music_loops = 0;
static Mix_Music * volatile music_playing = NULL;
static int music_volume = MIX_MAX_VOLUME;

struct _Mix_Music
{
    Mix_MusicType type;
    union
    {
    } data;
    Mix_Fading fading;
    int fade_step;
    int fade_steps;
    int error;
};

/* Used to calculate fading steps */
static int ms_per_step;

/* rcg06042009 report available decoders at runtime. */
static const char **music_decoders = NULL;
static int num_decoders = 0;

/* Semicolon-separated SoundFont paths */

int Mix_GetNumMusicDecoders(void)
{
    return(num_decoders);
}

const char *Mix_GetMusicDecoder(int index)
{
   if ((index < 0) || (index >= num_decoders))
      return NULL;
   return(music_decoders[index]);
}

static void add_music_decoder(const char *decoder)
{
   void *ptr = realloc((void *)music_decoders, (num_decoders + 1) * sizeof (const char *));
   if (ptr == NULL)
      return;  /* oh well, go on without it. */
   music_decoders = (const char **) ptr;
   music_decoders[num_decoders++] = decoder;
}

/* Local low-level functions prototypes */
static void music_internal_initialize_volume(void);
static void music_internal_volume(int volume);
static int  music_internal_play(Mix_Music *music, double position);
static int  music_internal_position(double position);
static int  music_internal_playing();
static void music_internal_halt(void);


/* Support for hooking when the music has finished */
static void (*music_finished_hook)(void) = NULL;

void Mix_HookMusicFinished(void (*music_finished)(void))
{
   music_finished_hook = music_finished;
}

/* If music isn't playing, halt it if no looping is required, restart it */
/* othesrchise. NOP if the music is playing */
static int music_halt_or_loop (void)
{
   /* Restart music if it has to loop */

   if (!music_internal_playing())
   {
      /* Restart music if it has to loop at a high level */
      if (music_loops)
      {
         Mix_Fading current_fade;
         if (music_loops > 0)
            --music_loops;
         current_fade = music_playing->fading;
         music_internal_play(music_playing, 0.0);
         music_playing->fading = current_fade;
      }
      else
      {
         music_internal_halt();
         if (music_finished_hook)
            music_finished_hook();

         return 0;
      }
   }

   return 1;
}



/* Mixing function */
void music_mixer(void *udata, Uint8 *stream, int len)
{
   int left = 0;

   if ( music_playing && music_active )
   {
      /* Handle fading */
      if ( music_playing->fading != MIX_NO_FADING )
      {
         if ( music_playing->fade_step++ < music_playing->fade_steps )
         {
            int volume;
            int fade_step = music_playing->fade_step;
            int fade_steps = music_playing->fade_steps;

            if ( music_playing->fading == MIX_FADING_OUT )
               volume = (music_volume * (fade_steps-fade_step)) / fade_steps;
            else /* Fading in */
               volume = (music_volume * fade_step) / fade_steps;
            music_internal_volume(volume);
         }
         else
         {
            if ( music_playing->fading == MIX_FADING_OUT )
            {
               music_internal_halt();
               if ( music_finished_hook )
                  music_finished_hook();
               return;
            }
            music_playing->fading = MIX_NO_FADING;
         }
      }

      music_halt_or_loop();
      if (!music_internal_playing())
         return;
   }

skip:
   /* Handle seamless music looping */
   if (left > 0 && left < len)
   {
      music_halt_or_loop();
      if (music_internal_playing())
         music_mixer(udata, stream+(len-left), left);
   }
}

/* Initialize the music players with a certain desired audio format */
int open_music(SDL_AudioSpec *mixer)
{
   music_playing = NULL;
   music_stopped = 0;
   Mix_VolumeMusic(SDL_MIX_MAXVOLUME);

   /* Calculate the number of ms for each callback */
   ms_per_step = (int) (((float)mixer->samples * 1000.0) / mixer->freq);

   return(0);
}

/* Free a music chunk previously loaded */
void Mix_FreeMusic(Mix_Music *music)
{
   if (!music)
      return;

   /* Stop the music if it's currently playing */
   if (music == music_playing)
   {
      /* Wait for any fade out to finish */
      while (music->fading == MIX_FADING_OUT)
         rarch_sleep(100);
      if (music == music_playing)
         music_internal_halt();
   }

   free(music);
}

/* Find out the music format of a mixer music, or the currently playing
   music, if 'music' is NULL.
*/
Mix_MusicType Mix_GetMusicType(const Mix_Music *music)
{
   Mix_MusicType type = MUS_NONE;

   if (music)
      type = music->type;
   else
   {
      if (music_playing)
         type = music_playing->type;
   }
   return(type);
}

/* Play a music chunk.  Returns 0, or -1 if there was an error.
 */
static int music_internal_play(Mix_Music *music, double position)
{
   int retval = 0;

   /* Note the music we're playing */
   if ( music_playing )
      music_internal_halt();
   music_playing = music;

   /* Set the initial volume */
   music_internal_initialize_volume();

   /* Set up for playback */
   switch (music->type)
   {
      default:
         Mix_SetError("Can't play unknown music type");
         retval = -1;
         break;
   }

skip:
   /* Set the playback position, note any errors if an offset is used */
   if ( retval == 0 )
   {
      if ( position > 0.0 )
      {
         if ( music_internal_position(position) < 0 )
         {
            Mix_SetError("Position not implemented for music type");
            retval = -1;
         }
      }
      else
         music_internal_position(0.0);
   }

   /* If the setup failed, we're not playing any music anymore */
   if ( retval < 0 )
      music_playing = NULL;
   return(retval);
}

/* Set the playing music position */
int music_internal_position(double position)
{
   return -1;
}

int Mix_SetMusicPosition(double position)
{
   int retval;

   if ( music_playing )
   {
      retval = music_internal_position(position);
      if ( retval < 0 )
         Mix_SetError("Position not implemented for music type");
   }
   else
   {
      Mix_SetError("Music isn't playing");
      retval = -1;
   }

   return(retval);
}

/* Set the music's initial volume */
static void music_internal_initialize_volume(void)
{
   if ( music_playing->fading == MIX_FADING_IN )
      music_internal_volume(0);
   else
      music_internal_volume(music_volume);
}

/* Set the music volume */
static void music_internal_volume(int volume)
{
}

int Mix_VolumeMusic(int volume)
{
   int prev_volume = music_volume;

   if ( volume < 0 )
      return prev_volume;
   if ( volume > SDL_MIX_MAXVOLUME )
      volume = SDL_MIX_MAXVOLUME;
   music_volume = volume;
   if ( music_playing )
      music_internal_volume(music_volume);
   return(prev_volume);
}

/* Halt playing of music */
static void music_internal_halt(void)
{
   switch (music_playing->type)
   {
      default:
         /* Unknown music type?? */
         return;
   }

skip:
   music_playing->fading = MIX_NO_FADING;
   music_playing = NULL;
}

int Mix_HaltMusic(void)
{
   if ( music_playing )
   {
      music_internal_halt();
      if ( music_finished_hook )
         music_finished_hook();
   }

   return(0);
}

Mix_Fading Mix_FadingMusic(void)
{
   Mix_Fading fading = MIX_NO_FADING;

   if ( music_playing )
      fading = music_playing->fading;

   return(fading);
}

/* Pause/Resume the music stream */
void Mix_PauseMusic(void)
{
   music_active = 0;
}

void Mix_ResumeMusic(void)
{
   music_active = 1;
}

void Mix_RewindMusic(void)
{
   Mix_SetMusicPosition(0.0);
}

int Mix_PausedMusic(void)
{
   return (music_active == 0);
}

/* Check the status of the music */
static int music_internal_playing(void)
{
   return 0;
}
int Mix_PlayingMusic(void)
{
   int playing = 0;

   if ( music_playing )
      playing = music_loops || music_internal_playing();

   return(playing);
}

int Mix_SetSynchroValue(int i)
{
   /* Not supported by any players at this time */
   return(-1);
}

int Mix_GetSynchroValue(void)
{
   /* Not supported by any players at this time */
   return(-1);
}

/* Uninitialize the music players */
void close_music(void)
{
   Mix_HaltMusic();

   /* rcg06042009 report available decoders at runtime. */
   free((void *)music_decoders);
   music_decoders = NULL;
   num_decoders = 0;

   ms_per_step = 0;
}
