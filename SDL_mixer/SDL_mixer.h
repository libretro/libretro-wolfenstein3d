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

#ifndef _SDL_MIXER_H
#define _SDL_MIXER_H

#include "SDL_audio.h"
#include "SDL_version.h"

/* Loads dynamic libraries and prepares them for use.  Flags should be
   one or more flags from MIX_InitFlags OR'd together.
   It returns the flags successfully initialized, or 0 on failure.
 */
int Mix_Init(int flags);

/* Unloads libraries loaded with Mix_Init */
void Mix_Quit(void);


/* The default mixer has 8 simultaneous mixing channels */
#ifndef MIX_CHANNELS
#define MIX_CHANNELS    8
#endif

/* Good default values for a PC soundcard */
#define MIX_DEFAULT_FREQUENCY   44010
#ifdef MSB_FIRST
#define MIX_DEFAULT_FORMAT  AUDIO_S16MSB
#else
#define MIX_DEFAULT_FORMAT  AUDIO_S16LSB
#endif
#define MIX_DEFAULT_CHANNELS    2
#define MIX_MAX_VOLUME          128 /* Volume of a chunk */

/* The internal format for an audio chunk */
typedef struct Mix_Chunk {
    int allocated;
    Uint8 *abuf;
    Uint32 alen;
    Uint8 volume;       /* Per-sample volume, 0-128 */
} Mix_Chunk;

/* The different fading types supported */
typedef enum {
    MIX_NO_FADING,
    MIX_FADING_OUT,
    MIX_FADING_IN
} Mix_Fading;

typedef enum {
    MUS_NONE,
    MUS_WAV,
} Mix_MusicType;

/* The internal format for a music chunk interpreted via mikmod */
typedef struct _Mix_Music Mix_Music;

/* Open the mixer with a certain audio format */
int Mix_OpenAudio(int frequency, Uint16 format, int channels, int chunksize);

/* Dynamically change the number of channels managed by the mixer.
   If decreasing the number of channels, the upper channels are
   stopped.
   This function returns the new number of allocated channels.
 */
int Mix_AllocateChannels(int numchans);

/* Find out what the actual audio device parameters are.
   This function returns 1 if the audio has been opened, 0 otherwise.
 */
int Mix_QuerySpec(int *frequency,Uint16 *format,int *channels);

/* Load a wave file or a music (.mod .s3m .it .xm) file */
Mix_Chunk * Mix_LoadWAV_RW(SDL_RWops *src, int freesrc);

/* Load a wave file of the mixer format from a memory buffer */
extern Mix_Chunk * Mix_QuickLoad_WAV(Uint8 *mem);

/* Free an audio chunk previously loaded */
extern void Mix_FreeChunk(Mix_Chunk *chunk);
extern void Mix_FreeMusic(Mix_Music *music);

/* Get a list of chunk/music decoders that this build of SDL_mixer provides.
   This list can change between builds AND runs of the program, if external
   libraries that add functionality become available.
   You must successfully call Mix_OpenAudio() before calling these functions.
   This API is only available in SDL_mixer 1.2.9 and later.

   // usage...
   int i;
   const int total = Mix_GetNumChunkDecoders();
   for (i = 0; i < total; i++)
       printf("Supported chunk decoder: [%s]\n", Mix_GetChunkDecoder(i));

   Appearing in this list doesn't promise your specific audio file will
   decode...but it's handy to know if you have, say, a functioning Timidity
   install.

   These return values are static, read-only data; do not modify or free it.
   The pointers remain valid until you call Mix_CloseAudio().
*/
extern int Mix_GetNumChunkDecoders(void);
extern const char * Mix_GetChunkDecoder(int index);
extern int Mix_GetNumMusicDecoders(void);
extern const char * Mix_GetMusicDecoder(int index);

/* Add your own music player or additional mixer function.
   If 'mix_func' is NULL, the default music player is re-enabled.
 */
extern void Mix_HookMusic(void (*mix_func)(void *udata, Uint8 *stream, int len), void *arg);

/* Get a pointer to the user data for the current music hook */
extern void * Mix_GetMusicHookData(void);

extern void Mix_ChannelFinished(void (*channel_finished)(int channel));


/* Special Effects API by ryan c. gordon. (icculus@icculus.org) */

#define MIX_CHANNEL_POST  -2

/* This is the format of a special effect callback:
 *
 *   myeffect(int chan, void *stream, int len, void *udata);
 *
 * (chan) is the channel number that your effect is affecting. (stream) is
 *  the buffer of data to work upon. (len) is the size of (stream), and
 *  (udata) is a user-defined bit of data, which you pass as the last arg of
 *  Mix_RegisterEffect(), and is passed back unmolested to your callback.
 *  Your effect changes the contents of (stream) based on whatever parameters
 *  are significant, or just leaves it be, if you prefer. You can do whatever
 *  you like to the buffer, though, and it will continue in its changed state
 *  down the mixing pipeline, through any other effect functions, then finally
 *  to be mixed with the rest of the channels and music for the final output
 *  stream.
 *
 */
typedef void (*Mix_EffectFunc_t)(int chan, void *stream, int len, void *udata);

/*
 * This is a callback that signifies that a channel has finished all its
 *  loops and has completed playback. This gets called if the buffer
 *  plays out normally, or if you call Mix_HaltChannel(), implicitly stop
 *  a channel via Mix_AllocateChannels(), or unregister a callback while
 *  it's still playing.
 *
 */
typedef void (*Mix_EffectDone_t)(int chan, void *udata);


/* You may not need to call this explicitly, unless you need to stop all
 *  effects from processing in the middle of a chunk's playback. Note that
 *  this will also shut off some internal effect processing, since
 *  Mix_SetPanning() and others may use this API under the hood. This is
 *  called internally when a channel completes playback.
 * Posteffects are never implicitly unregistered as they are for channels,
 *  but they may be explicitly unregistered through this function by
 *  specifying MIX_CHANNEL_POST for a channel.
 * returns zero if error (no such channel), nonzero if all effects removed.
 *  Error messages can be retrieved from Mix_GetError().
 */
extern int Mix_UnregisterAllEffects(int channel);


#define MIX_EFFECTSMAXSPEED  "MIX_EFFECTSMAXSPEED"

/*
 * These are the internally-defined mixing effects. They use the same API that
 *  effects defined in the application use, but are provided here as a
 *  convenience. Some effects can reduce their quality or use more memory in
 *  the name of speed; to enable this, make sure the environment variable
 *  MIX_EFFECTSMAXSPEED (see above) is defined before you call
 *  Mix_OpenAudio().
 */


/* Set the panning of a channel. The left and right channels are specified
 *  as integers between 0 and 255, quietest to loudest, respectively.
 *
 * Technically, this is just individual volume control for a sample with
 *  two (stereo) channels, so it can be used for more than just panning.
 *  If you want real panning, call it like this:
 *
 *   Mix_SetPanning(channel, left, 255 - left);
 *
 * ...which isn't so hard.
 *
 * Setting (channel) to MIX_CHANNEL_POST registers this as a posteffect, and
 *  the panning will be done to the final mixed stream before passing it on
 *  to the audio device.
 *
 * This uses the Mix_RegisterEffect() API internally, and returns without
 *  registering the effect function if the audio device is not configured
 *  for stereo output. Setting both (left) and (right) to 255 causes this
 *  effect to be unregistered, since that is the data's normal state.
 *
 * returns zero if error (no such channel or Mix_RegisterEffect() fails),
 *  nonzero if panning effect enabled. Note that an audio device in mono
 *  mode is a no-op, but this call will return successful in that case.
 *  Error messages can be retrieved from Mix_GetError().
 */
extern int Mix_SetPanning(int channel, Uint8 left, Uint8 right);


/* Set the position of a channel. (angle) is an integer from 0 to 360, that
 *  specifies the location of the sound in relation to the listener. (angle)
 *  will be reduced as neccesary (540 becomes 180 degrees, -100 becomes 260).
 *  Angle 0 is due north, and rotates clockwise as the value increases.
 *  For efficiency, the precision of this effect may be limited (angles 1
 *  through 7 might all produce the same effect, 8 through 15 are equal, etc).
 *  (distance) is an integer between 0 and 255 that specifies the space
 *  between the sound and the listener. The larger the number, the further
 *  away the sound is. Using 255 does not guarantee that the channel will be
 *  culled from the mixing process or be completely silent. For efficiency,
 *  the precision of this effect may be limited (distance 0 through 5 might
 *  all produce the same effect, 6 through 10 are equal, etc). Setting (angle)
 *  and (distance) to 0 unregisters this effect, since the data would be
 *  unchanged.
 *
 * If you need more precise positional audio, consider using OpenAL for
 *  spatialized effects instead of SDL_mixer. This is only meant to be a
 *  basic effect for simple "3D" games.
 *
 * If the audio device is configured for mono output, then you won't get
 *  any effectiveness from the angle; however, distance attenuation on the
 *  channel will still occur. While this effect will function with stereo
 *  voices, it makes more sense to use voices with only one channel of sound,
 *  so when they are mixed through this effect, the positioning will sound
 *  correct. You can convert them to mono through SDL before giving them to
 *  the mixer in the first place if you like.
 *
 * Setting (channel) to MIX_CHANNEL_POST registers this as a posteffect, and
 *  the positioning will be done to the final mixed stream before passing it
 *  on to the audio device.
 *
 * This is a convenience wrapper over Mix_SetDistance() and Mix_SetPanning().
 *
 * returns zero if error (no such channel or Mix_RegisterEffect() fails),
 *  nonzero if position effect is enabled.
 *  Error messages can be retrieved from Mix_GetError().
 */
extern int Mix_SetPosition(int channel, Sint16 angle, Uint8 distance);

/* end of effects API. --ryan. */


/* Reserve the first channels (0 -> n-1) for the application, i.e. don't allocate
   them dynamically to the next sample if requested with a -1 value below.
   Returns the number of reserved channels.
 */
extern int Mix_ReserveChannels(int num);

/* Channel grouping functions */

/* Attach a tag to a channel. A tag can be assigned to several mixer
   channels, to form groups of channels.
   If 'tag' is -1, the tag is removed (actually -1 is the tag used to
   represent the group of all the channels).
   Returns true if everything was OK.
 */
extern int Mix_GroupChannel(int which, int tag);
/* Assign several consecutive channels to a group */
extern int Mix_GroupChannels(int from, int to, int tag);
/* Finds the first available channel in a group of channels,
   returning -1 if none are available.
 */
extern int Mix_GroupAvailable(int tag);
/* Finds the "oldest" sample playing in a group of channels */
extern int Mix_GroupOldest(int tag);

/* Play an audio chunk on a specific channel.
   If the specified channel is -1, play on the first free channel.
   If 'loops' is greater than zero, loop the sound that many times.
   If 'loops' is -1, loop inifinitely (~65000 times).
   Returns which channel was used to play the sound.
*/
#define Mix_PlayChannel(channel,chunk,loops) Mix_PlayChannelTimed(channel,chunk,loops,-1)
/* The same as above, but the sound is played at most 'ticks' milliseconds */
extern int Mix_PlayChannelTimed(int channel, Mix_Chunk *chunk, int loops, int ticks);

/* Set the volume in the range of 0-128 of a specific channel or chunk.
   If the specified channel is -1, set volume for all channels.
   Returns the original volume.
   If the specified volume is -1, just return the current volume.
*/
extern int Mix_Volume(int channel, int volume);
extern int Mix_VolumeMusic(int volume);

/* Halt playing of a particular channel */
extern int Mix_HaltChannel(int channel);
extern int Mix_HaltMusic(void);

/* Change the expiration delay for a particular channel.
   The sample will stop playing after the 'ticks' milliseconds have elapsed,
   or remove the expiration if 'ticks' is -1
*/
extern int Mix_ExpireChannel(int channel, int ticks);

/* Pause/Resume a particular channel */
extern void Mix_Pause(int channel);
extern void Mix_Resume(int channel);
extern int Mix_Paused(int channel);

/* Pause/Resume the music stream */
extern void Mix_PauseMusic(void);
extern void Mix_ResumeMusic(void);
extern void Mix_RewindMusic(void);
extern int Mix_PausedMusic(void);

/* Set the current position in the music stream.
   This returns 0 if successful, or -1 if it failed or isn't implemented.
   This function is only implemented for MOD music formats (set pattern
   order number) and for OGG, FLAC, MP3_MAD, and MODPLUG music (set
   position in seconds), at the moment.
*/
extern int Mix_SetMusicPosition(double position);

/* Check the status of a specific channel.
   If the specified channel is -1, check all channels.
*/
extern int Mix_Playing(int channel);

/* Get the Mix_Chunk currently associated with a mixer channel
    Returns NULL if it's an invalid channel, or there's no chunk associated.
*/
extern Mix_Chunk * Mix_GetChunk(int channel);

/* Close the mixer, halting all playing audio */
extern void Mix_CloseAudio(void);

#include "close_code.h"

#endif /* _SDL_MIXER_H */
