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

  This file by Ryan C. Gordon (icculus@icculus.org)

  These are some internally supported special effects that use SDL_mixer's
  effect callback API. They are meant for speed over quality.  :)
*/

/* $Id$ */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "SDL.h"
#include "SDL_mixer.h"
#include <retro_endian.h>

#define __MIX_INTERNAL_EFFECT__
#include "effects_internal.h"

/* Positional effects...panning, distance attenuation, etc. */

typedef struct _Eff_positionargs
{
    volatile float left_f;
    volatile float right_f;
    volatile Uint8 left_u8;
    volatile Uint8 right_u8;
    volatile float left_rear_f;
    volatile float right_rear_f;
    volatile float center_f;
    volatile float lfe_f;
    volatile Uint8 left_rear_u8;
    volatile Uint8 right_rear_u8;
    volatile Uint8 center_u8;
    volatile Uint8 lfe_u8;
    volatile float distance_f;
    volatile Uint8 distance_u8;
    volatile Sint16 room_angle;
    volatile int in_use;
    volatile int channels;
} position_args;

static position_args **pos_args_array = NULL;
static position_args *pos_args_global = NULL;
static int position_channels = 0;

void _Eff_PositionDeinit(void)
{
    int i;
    for (i = 0; i < position_channels; i++) {
        free(pos_args_array[i]);
    }

    position_channels = 0;

    free(pos_args_global);
    pos_args_global = NULL;
    free(pos_args_array);
    pos_args_array = NULL;
}

/* This just frees up the callback-specific data. */
static void _Eff_PositionDone(int channel, void *udata)
{
    if (channel < 0) {
        if (pos_args_global != NULL) {
            free(pos_args_global);
            pos_args_global = NULL;
        }
    }

    else if (pos_args_array[channel] != NULL) {
        free(pos_args_array[channel]);
        pos_args_array[channel] = NULL;
    }
}

#ifdef MSB_FIRST
static void _Eff_position_s16msb(int chan, void *stream, int len, void *udata)
{
    /* 16 signed bits (lsb) * 2 channels. */
    volatile position_args *args = (volatile position_args *) udata;
    Sint16 *ptr = (Sint16 *) stream;
    int i;

    for (i = 0; i < len; i += sizeof (Sint16) * 2) {
        Sint16 swapl = (Sint16) ((((float) (Sint16) Retro_SwapBE16(*(ptr+0))) *
                                    args->left_f) * args->distance_f);
        Sint16 swapr = (Sint16) ((((float) (Sint16) Retro_SwapBE16(*(ptr+1))) *
                                    args->right_f) * args->distance_f);
        *(ptr++) = (Sint16) Retro_SwapBE16(swapl);
        *(ptr++) = (Sint16) Retro_SwapBE16(swapr);
    }
}

static void _Eff_position_s16msb_c4(int chan, void *stream, int len, void *udata)
{
    /* 16 signed bits (lsb) * 4 channels. */
    volatile position_args *args = (volatile position_args *) udata;
    Sint16 *ptr = (Sint16 *) stream;
    int i;

    for (i = 0; i < len; i += sizeof (Sint16) * 4) {
        Sint16 swapl = (Sint16) ((((float) (Sint16) Retro_SwapBE16(*(ptr+0))) *
                                    args->left_f) * args->distance_f);
        Sint16 swapr = (Sint16) ((((float) (Sint16) Retro_SwapBE16(*(ptr+1))) *
                                    args->right_f) * args->distance_f);
        Sint16 swaplr = (Sint16) ((((float) (Sint16) Retro_SwapBE16(*(ptr+2))) *
                                    args->left_rear_f) * args->distance_f);
        Sint16 swaprr = (Sint16) ((((float) (Sint16) Retro_SwapBE16(*(ptr+3))) *
                                    args->right_rear_f) * args->distance_f);
    switch (args->room_angle) {
        case 0:
                *(ptr++) = (Sint16) Retro_SwapBE16(swapl);
                *(ptr++) = (Sint16) Retro_SwapBE16(swapr);
                *(ptr++) = (Sint16) Retro_SwapBE16(swaplr);
                *(ptr++) = (Sint16) Retro_SwapBE16(swaprr);
            break;
        case 90:
                *(ptr++) = (Sint16) Retro_SwapBE16(swapr);
                *(ptr++) = (Sint16) Retro_SwapBE16(swaprr);
                *(ptr++) = (Sint16) Retro_SwapBE16(swapl);
                *(ptr++) = (Sint16) Retro_SwapBE16(swaplr);
            break;
        case 180:
                *(ptr++) = (Sint16) Retro_SwapBE16(swaprr);
                *(ptr++) = (Sint16) Retro_SwapBE16(swaplr);
                *(ptr++) = (Sint16) Retro_SwapBE16(swapr);
                *(ptr++) = (Sint16) Retro_SwapBE16(swapl);
            break;
        case 270:
                *(ptr++) = (Sint16) Retro_SwapBE16(swaplr);
                *(ptr++) = (Sint16) Retro_SwapBE16(swapl);
                *(ptr++) = (Sint16) Retro_SwapBE16(swaprr);
                *(ptr++) = (Sint16) Retro_SwapBE16(swapr);
            break;
    }
    }
}
static void _Eff_position_s16msb_c6(int chan, void *stream, int len, void *udata)
{
    /* 16 signed bits (lsb) * 6 channels. */
    volatile position_args *args = (volatile position_args *) udata;
    Sint16 *ptr = (Sint16 *) stream;
    int i;

    for (i = 0; i < len; i += sizeof (Sint16) * 6) {
        Sint16 swapl = (Sint16) ((((float) (Sint16) Retro_SwapBE16(*(ptr+0))) *
                                    args->left_f) * args->distance_f);
        Sint16 swapr = (Sint16) ((((float) (Sint16) Retro_SwapBE16(*(ptr+1))) *
                                    args->right_f) * args->distance_f);
        Sint16 swaplr = (Sint16) ((((float) (Sint16) Retro_SwapBE16(*(ptr+2))) *
                                    args->left_rear_f) * args->distance_f);
        Sint16 swaprr = (Sint16) ((((float) (Sint16) Retro_SwapBE16(*(ptr+3))) *
                                    args->right_rear_f) * args->distance_f);
        Sint16 swapce = (Sint16) ((((float) (Sint16) Retro_SwapBE16(*(ptr+4))) *
                                    args->center_f) * args->distance_f);
        Sint16 swapwf = (Sint16) ((((float) (Sint16) Retro_SwapBE16(*(ptr+5))) *
                                    args->lfe_f) * args->distance_f);

    switch (args->room_angle) {
        case 0:
                *(ptr++) = (Sint16) Retro_SwapBE16(swapl);
                *(ptr++) = (Sint16) Retro_SwapBE16(swapr);
                *(ptr++) = (Sint16) Retro_SwapBE16(swaplr);
                *(ptr++) = (Sint16) Retro_SwapBE16(swaprr);
                *(ptr++) = (Sint16) Retro_SwapBE16(swapce);
                *(ptr++) = (Sint16) Retro_SwapBE16(swapwf);
            break;
        case 90:
                *(ptr++) = (Sint16) Retro_SwapBE16(swapr);
                *(ptr++) = (Sint16) Retro_SwapBE16(swaprr);
                *(ptr++) = (Sint16) Retro_SwapBE16(swapl);
                *(ptr++) = (Sint16) Retro_SwapBE16(swaplr);
                *(ptr++) = (Sint16) Retro_SwapBE16(swapr)/2 + (Sint16) Retro_SwapBE16(swaprr)/2;
                *(ptr++) = (Sint16) Retro_SwapBE16(swapwf);
            break;
        case 180:
                *(ptr++) = (Sint16) Retro_SwapBE16(swaprr);
                *(ptr++) = (Sint16) Retro_SwapBE16(swaplr);
                *(ptr++) = (Sint16) Retro_SwapBE16(swapr);
                *(ptr++) = (Sint16) Retro_SwapBE16(swapl);
                *(ptr++) = (Sint16) Retro_SwapBE16(swaprr)/2 + (Sint16) Retro_SwapBE16(swaplr)/2;
                *(ptr++) = (Sint16) Retro_SwapBE16(swapwf);
            break;
        case 270:
                *(ptr++) = (Sint16) Retro_SwapBE16(swaplr);
                *(ptr++) = (Sint16) Retro_SwapBE16(swapl);
                *(ptr++) = (Sint16) Retro_SwapBE16(swaprr);
                *(ptr++) = (Sint16) Retro_SwapBE16(swapr);
                *(ptr++) = (Sint16) Retro_SwapBE16(swapl)/2 + (Sint16) Retro_SwapBE16(swaplr)/2;
                *(ptr++) = (Sint16) Retro_SwapBE16(swapwf);
            break;
    }
    }
}
#else
static void _Eff_position_s16lsb(int chan, void *stream, int len, void *udata)
{
   /* 16 signed bits (lsb) * 2 channels. */
   volatile position_args *args = (volatile position_args *) udata;
   Sint16 *ptr = (Sint16 *) stream;
   int i;

   for (i = 0; i < len; i += sizeof (Sint16) * 2) {
      Sint16 swapl = (Sint16) ((((float) (Sint16) Retro_SwapLE16(*(ptr+0))) *
               args->left_f) * args->distance_f);
      Sint16 swapr = (Sint16) ((((float) (Sint16) Retro_SwapLE16(*(ptr+1))) *
               args->right_f) * args->distance_f);
      if (args->room_angle == 180) {
         *(ptr++) = (Sint16) Retro_SwapLE16(swapr);
         *(ptr++) = (Sint16) Retro_SwapLE16(swapl);
      }
      else {
         *(ptr++) = (Sint16) Retro_SwapLE16(swapl);
         *(ptr++) = (Sint16) Retro_SwapLE16(swapr);
      }
   }
}

static void _Eff_position_s16lsb_c4(int chan, void *stream, int len, void *udata)
{
    /* 16 signed bits (lsb) * 4 channels. */
    volatile position_args *args = (volatile position_args *) udata;
    Sint16 *ptr = (Sint16 *) stream;
    int i;

    for (i = 0; i < len; i += sizeof (Sint16) * 4) {
        Sint16 swapl = (Sint16) ((((float) (Sint16) Retro_SwapLE16(*(ptr+0))) *
                                    args->left_f) * args->distance_f);
        Sint16 swapr = (Sint16) ((((float) (Sint16) Retro_SwapLE16(*(ptr+1))) *
                                    args->right_f) * args->distance_f);
        Sint16 swaplr = (Sint16) ((((float) (Sint16) Retro_SwapLE16(*(ptr+1))) *
                                    args->left_rear_f) * args->distance_f);
        Sint16 swaprr = (Sint16) ((((float) (Sint16) Retro_SwapLE16(*(ptr+2))) *
                                    args->right_rear_f) * args->distance_f);
    switch (args->room_angle) {
        case 0:
                *(ptr++) = (Sint16) Retro_SwapLE16(swapl);
                *(ptr++) = (Sint16) Retro_SwapLE16(swapr);
                *(ptr++) = (Sint16) Retro_SwapLE16(swaplr);
                *(ptr++) = (Sint16) Retro_SwapLE16(swaprr);
            break;
        case 90:
                *(ptr++) = (Sint16) Retro_SwapLE16(swapr);
                *(ptr++) = (Sint16) Retro_SwapLE16(swaprr);
                *(ptr++) = (Sint16) Retro_SwapLE16(swapl);
                *(ptr++) = (Sint16) Retro_SwapLE16(swaplr);
            break;
        case 180:
                *(ptr++) = (Sint16) Retro_SwapLE16(swaprr);
                *(ptr++) = (Sint16) Retro_SwapLE16(swaplr);
                *(ptr++) = (Sint16) Retro_SwapLE16(swapr);
                *(ptr++) = (Sint16) Retro_SwapLE16(swapl);
            break;
        case 270:
                *(ptr++) = (Sint16) Retro_SwapLE16(swaplr);
                *(ptr++) = (Sint16) Retro_SwapLE16(swapl);
                *(ptr++) = (Sint16) Retro_SwapLE16(swaprr);
                *(ptr++) = (Sint16) Retro_SwapLE16(swapr);
            break;
    }
    }
}

static void _Eff_position_s16lsb_c6(int chan, void *stream, int len, void *udata)
{
    /* 16 signed bits (lsb) * 6 channels. */
    volatile position_args *args = (volatile position_args *) udata;
    Sint16 *ptr = (Sint16 *) stream;
    int i;

    for (i = 0; i < len; i += sizeof (Sint16) * 6) {
        Sint16 swapl = (Sint16) ((((float) (Sint16) Retro_SwapLE16(*(ptr+0))) *
                                    args->left_f) * args->distance_f);
        Sint16 swapr = (Sint16) ((((float) (Sint16) Retro_SwapLE16(*(ptr+1))) *
                                    args->right_f) * args->distance_f);
        Sint16 swaplr = (Sint16) ((((float) (Sint16) Retro_SwapLE16(*(ptr+2))) *
                                    args->left_rear_f) * args->distance_f);
        Sint16 swaprr = (Sint16) ((((float) (Sint16) Retro_SwapLE16(*(ptr+3))) *
                                    args->right_rear_f) * args->distance_f);
        Sint16 swapce = (Sint16) ((((float) (Sint16) Retro_SwapLE16(*(ptr+4))) *
                                    args->center_f) * args->distance_f);
        Sint16 swapwf = (Sint16) ((((float) (Sint16) Retro_SwapLE16(*(ptr+5))) *
                                    args->lfe_f) * args->distance_f);
    switch (args->room_angle) {
        case 0:
                *(ptr++) = (Sint16) Retro_SwapLE16(swapl);
                *(ptr++) = (Sint16) Retro_SwapLE16(swapr);
                *(ptr++) = (Sint16) Retro_SwapLE16(swaplr);
                *(ptr++) = (Sint16) Retro_SwapLE16(swaprr);
                *(ptr++) = (Sint16) Retro_SwapLE16(swapce);
                *(ptr++) = (Sint16) Retro_SwapLE16(swapwf);
            break;
        case 90:
                *(ptr++) = (Sint16) Retro_SwapLE16(swapr);
                *(ptr++) = (Sint16) Retro_SwapLE16(swaprr);
                *(ptr++) = (Sint16) Retro_SwapLE16(swapl);
                *(ptr++) = (Sint16) Retro_SwapLE16(swaplr);
                *(ptr++) = (Sint16) Retro_SwapLE16(swapr)/2 + (Sint16) Retro_SwapLE16(swaprr)/2;
                *(ptr++) = (Sint16) Retro_SwapLE16(swapwf);
            break;
        case 180:
                *(ptr++) = (Sint16) Retro_SwapLE16(swaprr);
                *(ptr++) = (Sint16) Retro_SwapLE16(swaplr);
                *(ptr++) = (Sint16) Retro_SwapLE16(swapr);
                *(ptr++) = (Sint16) Retro_SwapLE16(swapl);
                *(ptr++) = (Sint16) Retro_SwapLE16(swaprr)/2 + (Sint16) Retro_SwapLE16(swaplr)/2;
                *(ptr++) = (Sint16) Retro_SwapLE16(swapwf);
            break;
        case 270:
                *(ptr++) = (Sint16) Retro_SwapLE16(swaplr);
                *(ptr++) = (Sint16) Retro_SwapLE16(swapl);
                *(ptr++) = (Sint16) Retro_SwapLE16(swaprr);
                *(ptr++) = (Sint16) Retro_SwapLE16(swapr);
                *(ptr++) = (Sint16) Retro_SwapLE16(swapl)/2 + (Sint16) Retro_SwapLE16(swaplr)/2;
                *(ptr++) = (Sint16) Retro_SwapLE16(swapwf);
            break;
    }
    }
}
#endif

static void init_position_args(position_args *args)
{
    memset(args, '\0', sizeof (position_args));
    args->in_use = 0;
    args->room_angle = 0;
    args->left_u8 = args->right_u8 = args->distance_u8 = 255;
    args->left_f  = args->right_f  = args->distance_f  = 1.0f;
    args->left_rear_u8 = args->right_rear_u8 = args->center_u8 = args->lfe_u8 = 255;
    args->left_rear_f = args->right_rear_f = args->center_f = args->lfe_f = 1.0f;
    Mix_QuerySpec(NULL, NULL, (int *) &args->channels);
}


static position_args *get_position_arg(int channel)
{
    void *rc;
    int i;

    if (channel < 0)
    {
       if (pos_args_global == NULL)
       {
          pos_args_global = malloc(sizeof (position_args));
          if (!pos_args_global)
             return(NULL);
          init_position_args(pos_args_global);
       }

       return(pos_args_global);
    }

    if (channel >= position_channels)
    {
        rc = realloc(pos_args_array, (channel + 1) * sizeof (position_args *));
        if (rc == NULL)
            return(NULL);
        pos_args_array = (position_args **) rc;
        for (i = position_channels; i <= channel; i++) {
            pos_args_array[i] = NULL;
        }
        position_channels = channel + 1;
    }

    if (pos_args_array[channel] == NULL)
    {
        pos_args_array[channel] = (position_args *)malloc(sizeof(position_args));
        if (pos_args_array[channel] == NULL)
            return(NULL);
        init_position_args(pos_args_array[channel]);
    }

    return(pos_args_array[channel]);
}


static Mix_EffectFunc_t get_position_effect_func(Uint16 format, int channels)
{
   switch (channels)
   {
      case 1:
      case 2:
#ifdef MSB_FIRST
         return _Eff_position_s16msb;
#else
         return _Eff_position_s16lsb;
#endif
      case 4:
#ifdef MSB_FIRST
         return _Eff_position_s16msb_c4;
#else
         return _Eff_position_s16lsb_c4;
#endif
      case 6:
#ifdef MSB_FIRST
         return _Eff_position_s16msb_c6;
#else
         return _Eff_position_s16lsb_c6;
#endif
   }

   return NULL;
}

static Uint8 speaker_amplitude[6];

static void set_amplitudes(int channels, int angle, int room_angle)
{
    int left = 255, right = 255;
    int left_rear = 255, right_rear = 255, center = 255;

    angle = abs(angle) % 360;  /* make angle between 0 and 359. */

    if (channels == 2)
    {
        /*
         * We only attenuate by position if the angle falls on the far side
         *  of center; That is, an angle that's due north would not attenuate
         *  either channel. Due west attenuates the right channel to 0.0, and
         *  due east attenuates the left channel to 0.0. Slightly east of
         *  center attenuates the left channel a little, and the right channel
         *  not at all. I think of this as occlusion by one's own head.  :)
         *
         *   ...so, we split our angle circle into four quadrants...
         */
        if (angle < 90) {
            left = 255 - ((int) (255.0f * (((float) angle) / 89.0f)));
        } else if (angle < 180) {
            left = (int) (255.0f * (((float) (angle - 90)) / 89.0f));
        } else if (angle < 270) {
            right = 255 - ((int) (255.0f * (((float) (angle - 180)) / 89.0f)));
        } else {
            right = (int) (255.0f * (((float) (angle - 270)) / 89.0f));
        }
    }

    if (channels == 4 || channels == 6)
    {
        /*
         *  An angle that's due north does not attenuate the center channel.
         *  An angle in the first quadrant, 0-90, does not attenuate the RF.
         *
         *   ...so, we split our angle circle into 8 ...
         *
         *             CE
         *             0
         *     LF      |         RF
         *             |
         *  270<-------|----------->90
         *             |
         *     LR      |         RR
         *            180
         *
         */
        if (angle < 45) {
            left = ((int) (255.0f * (((float) (180 - angle)) / 179.0f)));
            left_rear = 255 - ((int) (255.0f * (((float) (angle + 45)) / 89.0f)));
            right_rear = 255 - ((int) (255.0f * (((float) (90 - angle)) / 179.0f)));
        } else if (angle < 90) {
            center = ((int) (255.0f * (((float) (225 - angle)) / 179.0f)));
            left = ((int) (255.0f * (((float) (180 - angle)) / 179.0f)));
            left_rear = 255 - ((int) (255.0f * (((float) (135 - angle)) / 89.0f)));
            right_rear = ((int) (255.0f * (((float) (90 + angle)) / 179.0f)));
        } else if (angle < 135) {
            center = ((int) (255.0f * (((float) (225 - angle)) / 179.0f)));
            left = 255 - ((int) (255.0f * (((float) (angle - 45)) / 89.0f)));
            right = ((int) (255.0f * (((float) (270 - angle)) / 179.0f)));
            left_rear = ((int) (255.0f * (((float) (angle)) / 179.0f)));
        } else if (angle < 180) {
            center = 255 - ((int) (255.0f * (((float) (angle - 90)) / 89.0f)));
            left = 255 - ((int) (255.0f * (((float) (225 - angle)) / 89.0f)));
            right = ((int) (255.0f * (((float) (270 - angle)) / 179.0f)));
            left_rear = ((int) (255.0f * (((float) (angle)) / 179.0f)));
        } else if (angle < 225) {
            center = 255 - ((int) (255.0f * (((float) (270 - angle)) / 89.0f)));
            left = ((int) (255.0f * (((float) (angle - 90)) / 179.0f)));
            right = 255 - ((int) (255.0f * (((float) (angle - 135)) / 89.0f)));
            right_rear = ((int) (255.0f * (((float) (360 - angle)) / 179.0f)));
        } else if (angle < 270) {
            center = ((int) (255.0f * (((float) (angle - 135)) / 179.0f)));
            left = ((int) (255.0f * (((float) (angle - 90)) / 179.0f)));
            right = 255 - ((int) (255.0f * (((float) (315 - angle)) / 89.0f)));
            right_rear = ((int) (255.0f * (((float) (360 - angle)) / 179.0f)));
        } else if (angle < 315) {
            center = ((int) (255.0f * (((float) (angle - 135)) / 179.0f)));
            right = ((int) (255.0f * (((float) (angle - 180)) / 179.0f)));
            left_rear = ((int) (255.0f * (((float) (450 - angle)) / 179.0f)));
            right_rear = 255 - ((int) (255.0f * (((float) (angle - 225)) / 89.0f)));
        } else {
            right = ((int) (255.0f * (((float) (angle - 180)) / 179.0f)));
            left_rear = ((int) (255.0f * (((float) (450 - angle)) / 179.0f)));
            right_rear = 255 - ((int) (255.0f * (((float) (405 - angle)) / 89.0f)));
        }
    }

    if (left < 0) left = 0; if (left > 255) left = 255;
    if (right < 0) right = 0; if (right > 255) right = 255;
    if (left_rear < 0) left_rear = 0; if (left_rear > 255) left_rear = 255;
    if (right_rear < 0) right_rear = 0; if (right_rear > 255) right_rear = 255;
    if (center < 0) center = 0; if (center > 255) center = 255;

    if (room_angle == 90) {
        speaker_amplitude[0] = (Uint8)left_rear;
        speaker_amplitude[1] = (Uint8)left;
        speaker_amplitude[2] = (Uint8)right_rear;
        speaker_amplitude[3] = (Uint8)right;
    }
    else if (room_angle == 180) {
    if (channels == 2) {
            speaker_amplitude[0] = (Uint8)right;
            speaker_amplitude[1] = (Uint8)left;
    }
    else {
            speaker_amplitude[0] = (Uint8)right_rear;
            speaker_amplitude[1] = (Uint8)left_rear;
            speaker_amplitude[2] = (Uint8)right;
            speaker_amplitude[3] = (Uint8)left;
    }
    }
    else if (room_angle == 270) {
        speaker_amplitude[0] = (Uint8)right;
        speaker_amplitude[1] = (Uint8)right_rear;
        speaker_amplitude[2] = (Uint8)left;
        speaker_amplitude[3] = (Uint8)left_rear;
    }
    else {
        speaker_amplitude[0] = (Uint8)left;
        speaker_amplitude[1] = (Uint8)right;
        speaker_amplitude[2] = (Uint8)left_rear;
        speaker_amplitude[3] = (Uint8)right_rear;
    }
    speaker_amplitude[4] = (Uint8)center;
    speaker_amplitude[5] = 255;
}

int Mix_SetPosition(int channel, Sint16 angle, Uint8 distance)
{
   Mix_EffectFunc_t f = NULL;
   Uint16 format;
   int channels;
   position_args *args = NULL;
   Sint16 room_angle = 0;
   int retval = 1;

   Mix_QuerySpec(NULL, &format, &channels);
   f = get_position_effect_func(format, channels);
   if (f == NULL)
      return(0);

   angle = abs(angle) % 360;  /* make angle between 0 and 359. */

   args = get_position_arg(channel);
   if (!args)
      return(0);

   /* it's a no-op; unregister the effect, if it's registered. */
   if (!angle)
   {
      return(1);
   }

   if (channels == 2)
   {
      if (angle > 180)
         room_angle = 180; /* exchange left and right channels */
      else room_angle = 0;
   }

   if (channels == 4 || channels == 6)
   {
      if (angle > 315) room_angle = 0;
      else if (angle > 225) room_angle = 270;
      else if (angle > 135) room_angle = 180;
      else if (angle > 45) room_angle = 90;
      else room_angle = 0;
   }

   set_amplitudes(channels, angle, room_angle);

   args->left_u8 = speaker_amplitude[0];
   args->left_f = ((float) speaker_amplitude[0]) / 255.0f;
   args->right_u8 = speaker_amplitude[1];
   args->right_f = ((float) speaker_amplitude[1]) / 255.0f;
   args->left_rear_u8 = speaker_amplitude[2];
   args->left_rear_f = ((float) speaker_amplitude[2]) / 255.0f;
   args->right_rear_u8 = speaker_amplitude[3];
   args->right_rear_f = ((float) speaker_amplitude[3]) / 255.0f;
   args->center_u8 = speaker_amplitude[4];
   args->center_f = ((float) speaker_amplitude[4]) / 255.0f;
   args->lfe_u8 = speaker_amplitude[5];
   args->lfe_f = ((float) speaker_amplitude[5]) / 255.0f;
   args->distance_u8 = 255;
   args->distance_f = ((float) 255) / 255.0f;
   args->room_angle = room_angle;

   return(retval);
}

int Mix_SetPanning(int channel, Uint8 left, Uint8 right)
{
   Mix_EffectFunc_t f = NULL;
   int channels;
   Uint16 format;
   position_args *args = NULL;
   int retval = 1;

   Mix_QuerySpec(NULL, &format, &channels);

   if (channels != 2 && channels != 4 && channels != 6)    /* it's a no-op; we call that successful. */
      return(1);

   if (channels > 2) {
      /* left = right = 255 => angle = 0, to unregister effect as when channels = 2 */
      /* left = 255 =>  angle = -90;  left = 0 => angle = +89 */
      int angle = 0;
      if ((left != 255) || (right != 255)) {
         angle = (int)left;
         angle = 127 - angle;
         angle = -angle;
         angle = angle * 90 / 128; /* Make it larger for more effect? */
      }
      return( Mix_SetPosition(channel, angle, 0) );
   }

   f = get_position_effect_func(format, channels);
   if (f == NULL)
      return(0);

   args = get_position_arg(channel);
   if (!args) {
      return(0);
   }

   /* it's a no-op; unregister the effect, if it's registered. */
   if ((args->distance_u8 == 255) && (left == 255) && (right == 255))
         return(1);

   args->left_u8 = left;
   args->left_f = ((float) left) / 255.0f;
   args->right_u8 = right;
   args->right_f = ((float) right) / 255.0f;
   args->room_angle = 0;

   return(retval);
}



/* end of effects_position.c ... */

