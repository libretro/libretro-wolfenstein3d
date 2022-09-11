// File: id_crt.h
// Project: Wolf4SDL
// Original author: Fabien sanglard 
// Modificaton author: André Guilherme 
// Creation date: 2022-07-11 
// Original Creation date: 2014-08-26.
// Description: 
// Header file for the whole contents
// Note: This file is heavly modified on: 
// https://github.com/fabiensanglard/Chocolate-Wolfenstein-3D/blob/master/crt.h

#ifndef id_crt_h
#define id_crt_h
#include "version.h"
#include <SDL.h>
#ifdef CRT
// Win32
#if SDL_MAJOR_VERSION == 1
#if defined (N3DS)

#elif defined(_XBOX)
#include "xbox\fakeglx.h"
#else
#include <WTypes.h>
#include <gl\GL.h>
#if __linux__
#include <GL/gl.h>
#endif
#endif
#elif SDL_MAJOR_VERSION == 2

#endif

extern SDL_Color curpal[256];
/*
 * CRT aspect ratio is 4:3, height will be infered.
 */
void CRT_Init(int _width);
/*
 *   Trigger the Digital To Analogic convertion
 */
void CRT_DAC(void);
void CRT_Screenshot(void);

void CRT_FreeScreenshot(SDL_Surface *surface1, SDL_Surface *surface2);

void BlitImage(SDL_Surface* img1src, SDL_Surface* img1dst, SDL_Surface* img2src, SDL_Surface* img2dst);

#endif
#endif
