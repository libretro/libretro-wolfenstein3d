// ID_VL.H

// wolf compatability

// Win32
//#ifndef ID_VL.H 
//#define ID_VL.H
#ifndef ID_VL_H 
#define ID_VL_H

#include "wl_def.h"

#include "boolean.h"

void Quit (const char *error,...);

//===========================================================================

#define CHARWIDTH       2
#define TILEWIDTH       4

//===========================================================================

extern LR_Surface *screen, *screenBuffer;

extern  boolean  fullscreen;
extern  unsigned screenWidth, screenHeight, screenBits, bufferPitch;
extern  unsigned scaleFactor;

extern  boolean  screenfaded;
extern  unsigned bordercolor;

extern LR_Color gamepal[256];

//===========================================================================

//
// VGA hardware routines
//

void VL_WaitVBL(int vbls);

void VL_SetTextMode (void);
void VL_Startup (void);
void VL_Shutdown (void);

void VL_FillPalette (int red, int green, int blue);
void VL_SetPalette  (LR_Color *palette, bool forceupdate);
void VL_GetPalette  (LR_Color *palette);
void VL_FadeOut     (int start, int end, int red, int green, int blue, int steps);
void VL_FadeIn      (int start, int end, LR_Color *palette, int steps);

byte *VL_LockSurface(LR_Surface *surface);
void VL_UnlockSurface(LR_Surface *surface);

byte VL_GetPixel        (int x, int y);
void VL_Plot            (int x, int y, int color);
void VL_Hlin            (unsigned x, unsigned y, unsigned width, int color);
void VL_Vlin            (int x, int y, int height, int color);
void VL_BarScaledCoord  (int scx, int scy, int scwidth, int scheight, int color);

static inline void VL_Bar      (int x, int y, int width, int height, int color)
{
   VL_BarScaledCoord(scaleFactor*x, scaleFactor*y,
         scaleFactor*width, scaleFactor*height, color);
}

static inline void VL_ClearScreen(int color)
{
   LR_FillRect(screenBuffer, NULL, color);
}

void VL_MungePic                (byte *source, unsigned width, unsigned height);
void VL_DrawPicBare             (int x, int y, byte *pic, int width, int height);
void VL_MemToLatch              (byte *source, int width, int height,
                                    LR_Surface *destSurface, int x, int y);
void VL_ScreenToScreen          (LR_Surface *source, LR_Surface *dest);
void VL_MemToScreenScaledCoord  (byte *source, int width, int height, int scx, int scy);
void VL_MemToScreenScaledCoord2  (byte *source, int origwidth, int origheight, int srcx, int srcy,
                                    int destx, int desty, int width, int height);

static inline void VL_MemToScreen (byte *source, int width, int height, int x, int y)
{
    VL_MemToScreenScaledCoord(source, width, height, scaleFactor*x, scaleFactor*y);
}

void VL_MaskedToScreen (byte *source, int width, int height, int x, int y);

void VL_LatchToScreenScaledCoord (LR_Surface *source, int xsrc, int ysrc,
    int width, int height, int scxdest, int scydest);

static inline void VL_LatchToScreen (LR_Surface *source, int xsrc, int ysrc,
    int width, int height, int xdest, int ydest)
{
    VL_LatchToScreenScaledCoord(source, xsrc, ysrc, width, height,
        scaleFactor*xdest,scaleFactor*ydest);
}

static inline void VL_LatchToScreenScaledCoord2(LR_Surface *source, int scx, int scy)
{
    VL_LatchToScreenScaledCoord(source, 0, 0, source->surf->w, source->surf->h, scx,  scy);
}

#endif
