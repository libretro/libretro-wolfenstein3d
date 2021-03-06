// ID_LIBRETRO.C

#include <string.h>
#include "wl_def.h"
#include "surface.h"

LR_Color curpal[256];

void VL_WaitVBL(int vbls)
{
   rarch_sleep(vbls * 8);
}

void VW_UpdateScreen(void)
{
   VL_ScreenToScreen(screenBuffer, screen);
#ifdef __LIBRETRO__
   LR_Flip(NULL);
#else
   LR_Flip(screen);
#endif
}

/*
=======================
=
= VL_Startup
=
=======================
*/

void    VL_Startup (void)
{
#ifndef __LIBRETRO__
   screen     = (LR_Surface*)calloc(1, sizeof(*screen));

   screen->surf     = LR_SetVideoMode(screenWidth, screenHeight, 16, 0);

   if(!screen->surf)
      exit(1);

   LR_SetColors(screen->surf, gamepal, 0, 256);
#endif
   memcpy(curpal, gamepal, sizeof(LR_Color) * 256);

   screenBuffer = (LR_Surface*)calloc(1, sizeof(*screenBuffer));

   screenBuffer->surf = LR_CreateRGBSurface(SDL_SWSURFACE, screenWidth,
         screenHeight, 8, 0, 0, 0, 0);
   if(!screenBuffer->surf)
      exit(1);
   LR_SetColors(screenBuffer->surf, gamepal, 0, 256);

   bufferPitch = screenBuffer->surf->pitch;
   scaleFactor = screenWidth/320;

   if(screenHeight/200 < scaleFactor)
      scaleFactor = screenHeight/200;


   pixelangle = (short *) malloc(screenWidth * sizeof(short));
   wallheight = (int *) malloc(screenWidth * sizeof(int));
}

/*
=======================
=
= VL_Shutdown
=
=======================
*/

void VL_Shutdown (void)
{
   if (screenBuffer)
      free(screenBuffer);
#ifndef __LIBRETRO__
   if (screen)
      free(screen);

   screen       = NULL;
#endif
   screenBuffer = NULL;
}

/*
=================
=
= VL_SetPalette
=
=================
*/

void VL_SetPalette (LR_Color *palette, bool forceupdate)
{
   memcpy(curpal, palette, sizeof(LR_Color) * 256);

   LR_SetPalette(screenBuffer->surf, SDL_LOGPAL, palette, 0, 256);
   if (forceupdate)
      VW_UpdateScreen();
}

/*
=================
=
= VL_GetPalette
=
=================
*/

void VL_GetPalette (LR_Color *palette)
{
   memcpy(palette, curpal, sizeof(LR_Color) * 256);
}

byte *VL_LockSurface(LR_Surface *surface)
{
   return (byte *) surface->surf->pixels;
}

void VL_UnlockSurface(LR_Surface *surface)
{
}

int IN_MouseButtons (void)
{
   return 0;
}
