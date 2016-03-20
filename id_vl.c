// ID_VL.C

#include <string.h>
#include "wl_def.h"
#include "surface.h"

boolean fullscreen = false;

unsigned screenWidth = 320;
unsigned screenHeight = 200;

LR_Surface *screen = NULL;

LR_Surface *screenBuffer = NULL;
unsigned bufferPitch;

unsigned scaleFactor;

boolean  screenfaded;
unsigned bordercolor;

LR_Color palette1[256], palette2[256];

#define RGB(r, g, b) {(r)*255/63, (g)*255/63, (b)*255/63, 0}

LR_Color gamepal[]={
#ifdef SPEAR
    #include "sodpal.inc"
#else
    #include "wolfpal.inc"
#endif
};

/*
=============================================================================

                        PALETTE OPS

        To avoid snow, do a WaitVBL BEFORE calling these

=============================================================================
*/

/*
=================
=
= VL_FillPalette
=
=================
*/

void VL_FillPalette (int red, int green, int blue)
{
    int i;
    LR_Color pal[256];

    for(i = 0; i < 256; i++)
    {
        pal[i].r = red;
        pal[i].g = green;
        pal[i].b = blue;
    }

    VL_SetPalette(pal, true);
}

/*
=================
=
= VL_FadeOut
=
= Fades the current palette to the given color in the given number of steps
=
=================
*/

void VL_FadeOut (int start, int end, int red, int green, int blue, int steps)
{
   int         i,j,orig,delta;
   LR_Color   *origptr, *newptr;

   red   = red * 255 / 63;
   green = green * 255 / 63;
   blue  = blue * 255 / 63;

   VL_WaitVBL(1);
   VL_GetPalette(palette1);
   memcpy(palette2, palette1, sizeof(LR_Color) * 256);

   /* fade through intermediate frames */
   for (i = 0; i < steps; i++)
   {
      origptr = &palette1[start];
      newptr  = &palette2[start];

      for (j = start;j <= end; j++)
      {
         orig      = origptr->r;
         delta     = red-orig;
         newptr->r = orig + delta * i / steps;
         orig      = origptr->g;
         delta     = green-orig;
         newptr->g = orig + delta * i / steps;
         orig      = origptr->b;
         delta     = blue-orig;
         newptr->b = orig + delta * i / steps;
         origptr++;
         newptr++;
      }

      VL_WaitVBL(1);
      VL_SetPalette (palette2, true);
   }

   /* final color */
   VL_FillPalette (red,green,blue);

   screenfaded = true;
}


/*
=================
=
= VL_FadeIn
=
=================
*/

void VL_FadeIn (int start, int end, LR_Color *palette, int steps)
{
   int i,j,delta;

   VL_WaitVBL(1);
   VL_GetPalette(palette1);
   memcpy(palette2, palette1, sizeof(LR_Color) * 256);

   /* fade through intermediate frames */
   for (i = 0;i < steps; i++)
   {
      for (j = start; j <= end; j++)
      {
         delta         = palette[j].r-palette1[j].r;
         palette2[j].r = palette1[j].r + delta * i / steps;
         delta         = palette[j].g-palette1[j].g;
         palette2[j].g = palette1[j].g + delta * i / steps;
         delta         = palette[j].b-palette1[j].b;
         palette2[j].b = palette1[j].b + delta * i / steps;
      }

      VL_WaitVBL(1);
      VL_SetPalette(palette2, true);
   }

   /* final color */
   VL_SetPalette (palette, true);
   screenfaded = false;
}

/*
=============================================================================

                            PIXEL OPS

=============================================================================
*/

/*
=================
=
= VL_Plot
=
=================
*/

void VL_Plot (int x, int y, int color)
{
   assert(x >= 0 && (unsigned) x < screenWidth
         && y >= 0 && (unsigned) y < screenHeight
         && "VL_Plot: Pixel out of bounds!");

   VL_LockSurface(screenBuffer);
   ((byte *) screenBuffer->surf->pixels)[y * bufferPitch + x] = color;
   VL_UnlockSurface(screenBuffer);
}

/*
=================
=
= VL_GetPixel
=
=================
*/

byte VL_GetPixel (int x, int y)
{
   byte col;
   assert(x >= 0 && (unsigned) x < screenWidth
         && y >= 0 && (unsigned) y < screenHeight
         && "VL_GetPixel: Pixel out of bounds!");

   VL_LockSurface(screenBuffer);
   col = ((byte *) screenBuffer->surf->pixels)[y * bufferPitch + x];
   VL_UnlockSurface(screenBuffer);
   return col;
}


/*
=================
=
= VL_Hlin
=
=================
*/

void VL_Hlin (unsigned x, unsigned y, unsigned width, int color)
{
   Uint8 *dest;

   assert(x >= 0 && x + width <= screenWidth
         && y >= 0 && y < screenHeight
         && "VL_Hlin: Destination rectangle out of bounds!");

   VL_LockSurface(screenBuffer);
   dest = ((byte *) screenBuffer->surf->pixels) + y * bufferPitch + x;
   memset(dest, color, width);
   VL_UnlockSurface(screenBuffer);
}


/*
=================
=
= VL_Vlin
=
=================
*/

void VL_Vlin (int x, int y, int height, int color)
{
   Uint8 *dest;

   assert(x >= 0 && (unsigned) x < screenWidth
         && y >= 0 && (unsigned) y + height <= screenHeight
         && "VL_Vlin: Destination rectangle out of bounds!");

   VL_LockSurface(screenBuffer);
   dest = ((byte *) screenBuffer->surf->pixels) + y * bufferPitch + x;

   while (height--)
   {
      *dest = color;
      dest += bufferPitch;
   }
   VL_UnlockSurface(screenBuffer);
}


/*
=================
=
= VL_Bar
=
=================
*/

void VL_BarScaledCoord (int scx, int scy, int scwidth, int scheight, int color)
{
   Uint8 *dest;

   assert(scx >= 0 && (unsigned) scx + scwidth <= screenWidth
         && scy >= 0 && (unsigned) scy + scheight <= screenHeight
         && "VL_BarScaledCoord: Destination rectangle out of bounds!");

   VL_LockSurface(screenBuffer);
   dest = ((byte *) screenBuffer->surf->pixels) + scy * bufferPitch + scx;

   while (scheight--)
   {
      memset(dest, color, scwidth);
      dest += bufferPitch;
   }
   VL_UnlockSurface(screenBuffer);
}

/*
============================================================================

                            MEMORY OPS

============================================================================
*/

/*
=================
=
= VL_MemToLatch
=
=================
*/

void VL_MemToLatch(byte *source, int width, int height,
    LR_Surface *destSurface, int x, int y)
{
   unsigned ysrc, xsrc;
   int pitch;
   byte *dest;

   assert(x >= 0 && (unsigned) x + width <= screenWidth
         && y >= 0 && (unsigned) y + height <= screenHeight
         && "VL_MemToLatch: Destination rectangle out of bounds!");

   VL_LockSurface(destSurface);

   pitch = destSurface->surf->pitch;
   dest  = (byte *) destSurface->surf->pixels + y * pitch + x;

   for(ysrc = 0; ysrc < height; ysrc++)
   {
      for(xsrc = 0; xsrc < width; xsrc++)
         dest[ysrc * pitch + xsrc] = source[(ysrc * (width >> 2) + (xsrc >> 2))
            + (xsrc & 3) * (width >> 2) * height];
   }
   VL_UnlockSurface(destSurface);
}

/*
=================
=
= VL_MemToScreenScaledCoord
=
= Draws a block of data to the screen with scaling according to scaleFactor.
=
=================
*/

void VL_MemToScreenScaledCoord (byte *source, int width, int height, int destx, int desty)
{
   unsigned i, j, m, n, scj, sci;
   byte *vbuf;

   assert(destx >= 0 && destx + width * scaleFactor <= screenWidth
         && desty >= 0 && desty + height * scaleFactor <= screenHeight
         && "VL_MemToScreenScaledCoord: Destination rectangle out of bounds!");

   VL_LockSurface(screenBuffer);
   vbuf = (byte *) screenBuffer->surf->pixels;

   for(j = 0, scj = 0; j < height; j++, scj += scaleFactor)
   {
      for(i = 0,sci = 0; i < width; i++, sci += scaleFactor)
      {
         byte col = source[(j*(width>>2)+(i>>2))+(i&3)*(width>>2)*height];
         for(m = 0; m < scaleFactor; m++)
         {
            for(n = 0; n < scaleFactor; n++)
               vbuf[(scj+m+desty) * bufferPitch + sci+n+destx] = col;
         }
      }
   }
   VL_UnlockSurface(screenBuffer);
}

/*
=================
=
= VL_MemToScreenScaledCoord
=
= Draws a part of a block of data to the screen.
= The block has the size origwidth*origheight.
= The part at (srcx, srcy) has the size width*height
= and will be painted to (destx, desty) with scaling according to scaleFactor.
=
=================
*/

void VL_MemToScreenScaledCoord2 (byte *source, int origwidth, int origheight, int srcx, int srcy,
                                int destx, int desty, int width, int height)
{
   unsigned i, j, sci, scj, m, n;
   byte *vbuf;

   assert(destx >= 0 && destx + width * scaleFactor <= screenWidth
         && desty >= 0 && desty + height * scaleFactor <= screenHeight
         && "VL_MemToScreenScaledCoord: Destination rectangle out of bounds!");

   VL_LockSurface(screenBuffer);
   vbuf = (byte *) screenBuffer->surf->pixels;

   for(j = 0,scj = 0; j < height; j++, scj += scaleFactor)
   {
      for(i = 0, sci = 0; i < width; i++, sci += scaleFactor)
      {
         byte col = source[((j+srcy)*(origwidth>>2)+((i+srcx)>>2))+((i+srcx)&3)*(origwidth>>2)*origheight];
         for(m = 0; m < scaleFactor; m++)
         {
            for(n = 0; n < scaleFactor; n++)
               vbuf[(scj+m+desty)* bufferPitch + sci+n+destx] = col;
         }
      }
   }
   VL_UnlockSurface(screenBuffer);
}

/*
=================
=
= VL_LatchToScreen
=
=================
*/

void VL_LatchToScreenScaledCoord(LR_Surface *source, int xsrc, int ysrc,
    int width, int height, int scxdest, int scydest)
{
   unsigned i, j, sci, m, n, scj;
   byte *src, *vbuf;
   unsigned srcPitch;

   assert(scxdest >= 0 && scxdest + width * scaleFactor <= screenWidth
         && scydest >= 0 && scydest + height * scaleFactor <= screenHeight
         && "VL_LatchToScreenScaledCoord: Destination rectangle out of bounds!");

   VL_LockSurface(source);
   src = (byte *)source->surf->pixels;
   srcPitch = source->surf->pitch;

   VL_LockSurface(screenBuffer);
   vbuf = (byte *) screenBuffer->surf->pixels;

   for(j = 0, scj = 0; j < height; j++, scj += scaleFactor)
   {
      for(i = 0, sci = 0; i < width; i++, sci += scaleFactor)
      {
         byte col = src[(ysrc + j)*srcPitch + xsrc + i];
         for(m = 0; m < scaleFactor; m++)
         {
            for(n = 0; n < scaleFactor; n++)
               vbuf[(scydest+scj+m)* bufferPitch + scxdest+sci+n] = col;
         }
      }
   }
   VL_UnlockSurface(screenBuffer);
   VL_UnlockSurface(source);
}

/*
=================
=
= VL_ScreenToScreen
=
=================
*/

void VL_ScreenToScreen (LR_Surface *source, LR_Surface *dest)
{
   LR_BlitSurface(source, NULL, dest, NULL);
}
