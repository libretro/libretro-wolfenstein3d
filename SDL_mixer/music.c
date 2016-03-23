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

/* Semicolon-separated SoundFont paths */

/* Mixing function */
void music_mixer(void *udata, uint8_t *stream, int len)
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

   return prev_volume;
}

/* Initialize the music players with a certain desired audio format */
int open_music(SDL_AudioSpec *mixer)
{
   music_stopped = 0;
   Mix_VolumeMusic(SDL_MIX_MAXVOLUME);

   /* Calculate the number of ms for each callback */
   ms_per_step = (int) (((float)mixer->samples * 1000.0) / mixer->freq);

   return 0;
}

/* Free a music chunk previously loaded */
void Mix_FreeMusic(Mix_Music *music)
{
   if (!music)
      return;

   free(music);
}

/* Set the music volume */
static void music_internal_volume(int volume)
{
}

/* Set the music's initial volume */
static void music_internal_initialize_volume(void)
{
   music_internal_volume(music_volume);
}

/* Play a music chunk.  Returns 0, or -1 if there was an error.
*/
static int music_internal_play(Mix_Music *music, double position)
{
   /* Set the initial volume */
   music_internal_initialize_volume();

   return -1;
}

/* Set the playing music position */
int music_internal_position(double position)
{
   return -1;
}

int Mix_SetMusicPosition(double position)
{
   return -1;
}

int Mix_HaltMusic(void)
{
   return 0;
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

/* Uninitialize the music players */
void close_music(void)
{
   Mix_HaltMusic();

   ms_per_step = 0;
}
