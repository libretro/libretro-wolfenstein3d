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

  These are some helper functions for the internal mixer special effects.
*/

/* $Id$ */


     /* ------ These are used internally only. Don't touch. ------ */

#include <stdio.h>
#include <stdlib.h>
#include "SDL_mixer.h"

#define __MIX_INTERNAL_EFFECT__
#include "effects_internal.h"

/* Should we favor speed over memory usage and/or quality of output? */
int _Mix_effects_max_speed = 0;


void _Mix_InitEffects(void)
{
    _Mix_effects_max_speed = (SDL_getenv(MIX_EFFECTSMAXSPEED) != NULL);
}

void _Mix_DeinitEffects(void)
{
    _Eff_PositionDeinit();
}

/* end of effects.c ... */

