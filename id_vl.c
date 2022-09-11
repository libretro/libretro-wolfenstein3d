// ID_VL.C
#include <string.h>
#include "wl_def.h"
#ifdef CRT
#include "id_crt.h"
#endif
#include "surface.h"
// Uncomment the following line, if you get destination out of bounds
// assertion errors and want to ignore them during debugging
//#define IGNORE_BAD_DEST

#ifdef IGNORE_BAD_DEST
#undef assert
#define assert(x) if(!(x)) return
#define assert_ret(x) if(!(x)) return 0
#else
#define assert_ret(x) assert(x)
#endif

#if defined(_arch_dreamcast)
boolean usedoublebuffering = false;
unsigned screenWidth = 320;
unsigned screenHeight = 200;
int      screenBits = 8;
#elif defined(GP2X)
boolean usedoublebuffering = true;
unsigned screenWidth = 320;
unsigned screenHeight = 240;
#if defined(GP2X_940)
int      screenBits = 8;
#else
int      screenBits = 16;
#endif
//WIP:
#elif defined(PS2)
boolean usedoublebuffering = true;
u32 screenWidth = 640;
u32 screenHeight = 448;
int screenBits = 8;
#elif defined(N3DS)
boolean fullscreen = true;
boolean usedoublebuffering = true;
int screenWidth = 400;
int screenHeight = 240;
int screenBits = 32;      // use "best" color depth according to libSDL  // ADDEDFIX 0
#else
boolean usedoublebuffering = true;
u32 screenWidth = 640;
u32 screenHeight = 405;
u32 screenBits = 8;      // use "best" color depth according to libSDL
#endif

SDL_Surface *screen = NULL;
SDL_Surface* screenBuffer = NULL;
#if N3DS
int screenPitch;
int bufferPitch;
#else
u32 screenPitch;
u32 bufferPitch;
#endif
#if SDL_MAJOR_VERSION == 2
SDL_Window *window;
SDL_Renderer *renderer;
SDL_Texture *texture;
#endif

int      scaleFactor;

boolean	 screenfaded;
unsigned bordercolor;

uint32_t *ylookup;

SDL_Color palette1[256], palette2[256];
#ifndef __LIBRETRO__
SDL_Color curpal[256];
#else
LR_Color curpal[256];
#endif
#define CASSERT(x) extern int ASSERT_COMPILE[((x) != 0) * 2 - 1];
#define RGB(r, g, b) {(r)*255/63, (g)*255/63, (b)*255/63, 0}

SDL_Color gamepal[] =
{
#ifdef SPEAR
    #include "sodpal.inc"
#else
    #include "wolfpal.inc"
#endif
};

CASSERT(lengthof(gamepal) == 256)

//===========================================================================


/*
=======================
=
= VL_Shutdown
=
=======================
*/

void VL_Shutdown (void)
{
    SDL_FreeSurface (screenBuffer);
	
#if SDL_MAJOR_VERSION == 2
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_DestroyTexture(texture);
#endif

    free (ylookup);
    free (pixelangle);
    free (wallheight);
#if defined(USE_FLOORCEILINGTEX) || defined(USE_CLOUDSKY)
    free (spanstart);

    spanstart = NULL;
#endif
    screenBuffer = NULL;
    ylookup = NULL;
    pixelangle = NULL;
    wallheight = NULL;
}

#ifdef __LIBRETRO__
void VL_WaitVBL(int vbls)
{
   rarch_sleep(vbls * 8);
}
#endif
/*
=======================
=
= VL_SetVGAPlaneMode
=
=======================
*/

void VL_SetVGAPlaneMode (void)
{
    int i;
    uint32_t a,r,g,b;

#ifdef SPEAR
    const char* title = "Spear of Destiny";
#else
    const char* title = "Wolfenstein 3D";
#endif

#if SDL_MAJOR_VERSION == 1
#ifdef CRT
    //Fab's CRT Hack
    //Adjust height so the screen is 4:3 aspect ratio
    screenHeight = screenWidth * 3.0 / 4.0;
#endif  
    SDL_WM_SetCaption(title, NULL);

    if (screenBits == -1)
    {
        const SDL_VideoInfo *vidInfo = SDL_GetVideoInfo();
        screenBits = vidInfo->vfmt->BitsPerPixel;
    }
#ifndef __LIBRETRO__
#if N3DS
    screen = SDL_SetVideoMode(screenWidth, screenHeight, 32, SDL_TOPSCR | SDL_CONSOLEBOTTOM | SDL_DOUBLEBUF);
#else
    screen = SDL_SetVideoMode(screenWidth, screenHeight, screenBits,
        (usedoublebuffering ? SDL_HWSURFACE | SDL_DOUBLEBUF : 0) | (screenBits == 8 ? SDL_HWPALETTE : 0)

#ifdef CRT
        | (fullscreen ? SDL_FULLSCREEN : 0) | SDL_OPENGL | SDL_OPENGLBLIT);
#else       
        | (fullscreen ? SDL_FULLSCREEN : 0));
#endif
#endif

    if(!screen)
    {
        printf("Unable to set %ux%ux%i video mode: %s\n", screenWidth,
            screenHeight, screenBits, SDL_GetError());
        exit(1);
    }
    if((screen->flags & SDL_DOUBLEBUF) != SDL_DOUBLEBUF)
        usedoublebuffering = false;
    SDL_ShowCursor(SDL_DISABLE);

    SDL_SetColors(screen, gamepal, 0, 256);
    memcpy(curpal, gamepal, sizeof(SDL_Color) * 256);

#else
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

#endif
#ifdef CRT  
    //Fab's and Andr�s CRT Hack
    CRT_Init(screen);
#endif 
    screenBuffer = SDL_CreateRGBSurface(SDL_SWSURFACE, screenWidth,
        screenHeight, 8, 0, 0, 0, 0);
    if(!screenBuffer)
    {
        printf("Unable to create screen buffer surface: %s\n", SDL_GetError());
        exit(1);
    }
    SDL_SetColors(screenBuffer, gamepal, 0, 256);
#elif SDL_MAJOR_VERSION == 2
#ifdef CRT
    //Fab's CRT Hack:
    //Adjust height so the screen is 4:3 aspect ratio
    screenHeight = screenWidth * 3.0 / 4.0;
#endif    
    window = SDL_CreateWindow(title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, screenWidth, screenHeight,
    (fullscreen ? SDL_WINDOW_FULLSCREEN : 0 | SDL_WINDOW_OPENGL));

    SDL_PixelFormatEnumToMasks (SDL_PIXELFORMAT_ARGB8888,&screenBits,&r,&g,&b,&a);

    screen = SDL_CreateRGBSurface(0,screenWidth,screenHeight,screenBits,r,g,b,a);

    if(!screen)
    {
        printf("Unable to set %ux%ux%i video mode: %s\n", screenWidth, screenHeight, screenBits, SDL_GetError());
        exit(1);
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");

    SDL_ShowCursor(SDL_DISABLE);

    SDL_SetPaletteColors(screen->format->palette, gamepal, 0, 256);
    memcpy(curpal, gamepal, sizeof(SDL_Color) * 256);
    
#ifdef CRT  
    //Fab's and Andr�s CRT Hack
    CRT_Init(screen);
#endif

    screenBuffer = SDL_CreateRGBSurface(0, screenWidth, screenHeight, 
    8, 0, 0, 0, 0);
    
    if(!screenBuffer)
    {
        printf("Unable to create screen buffer surface: %s\n", SDL_GetError());
        exit(1);
    }
    SDL_SetPaletteColors(screenBuffer->format->palette, gamepal, 0, 256);

    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, 
    SDL_TEXTUREACCESS_STREAMING, screenWidth, screenHeight);
#endif

    screenPitch = screen->pitch;
#ifndef __LIBRETRO__	
    bufferPitch = screenBuffer->pitch;

    scaleFactor = screenWidth/320;
    if(screenHeight/200 < scaleFactor) scaleFactor = screenHeight/200;

    ylookup = SafeMalloc(screenHeight * sizeof(*ylookup));
    pixelangle = SafeMalloc(screenWidth * sizeof(*pixelangle));
    wallheight = SafeMalloc(screenWidth * sizeof(*wallheight));
#if defined(USE_FLOORCEILINGTEX) || defined(USE_CLOUDSKY)
    spanstart = SafeMalloc((screenHeight / 2) * sizeof(*spanstart));
#endif
#else
   bufferPitch = screenBuffer->surf->pitch;
   scaleFactor = screenWidth/320;

   if(screenHeight/200 < scaleFactor)
      scaleFactor = screenHeight/200;

   pixelangle = (short *) malloc(screenWidth * sizeof(short));
   wallheight = (int *) malloc(screenWidth * sizeof(int));
    for (i = 0; i < screenHeight; i++)
        ylookup[i] = i * bufferPitch;
#endif
}

/*
=============================================================================

						PALETTE OPS

		To avoid snow, do a WaitVBL BEFORE calling these

=============================================================================
*/

/*
=================
=
= VL_ConvertPalette
=
=================
*/

void VL_ConvertPalette(byte *srcpal, SDL_Color *destpal, int numColors)
{
    int i;

    for(i=0; i<numColors; i++)
    {
#if N3DS
        destpal[i].r = *srcpal++;
        destpal[i].g = *srcpal++;
        destpal[i].b = *srcpal++;
#else
        destpal[i].r = *srcpal++ * 255 / 63;
        destpal[i].g = *srcpal++ * 255 / 63;
        destpal[i].b = *srcpal++ * 255 / 63;
#endif

    }
}

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
    SDL_Color pal[256];

    for(i=0; i<256; i++)
    {
        pal[i].r = red;
        pal[i].g = green;
        pal[i].b = blue;
    }

    VL_SetPalette(pal, true);
}

//===========================================================================

/*
=================
=
= VL_SetColor
=
=================
*/

void VL_SetColor	(int color, int red, int green, int blue)
{
    SDL_Color col = 
    { 
        (Uint8) red, 
        (Uint8) green, 
        (Uint8) blue 
    };
    
    curpal[color] = col;

    if(screenBits == 8)
#if SDL_MAJOR_VERSION == 1
        SDL_SetPalette(screen, SDL_PHYSPAL, &col, color, 1);
    else
    {
        SDL_SetPalette(screenBuffer, SDL_LOGPAL, &col, color, 1);

#ifdef CRT        
        CRT_Init(screen);
        CRT_DAC();
#else
        VH_UpdateScreen(screen);
#endif
#elif SDL_MAJOR_VERSION == 2
        SDL_SetPaletteColors(screen->format->palette, &col, color, 1);
    else
    {
        SDL_SetPaletteColors(screenBuffer->format->palette, &col, color, 1);

        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, screenBuffer);
               
#ifdef CRT        
        CRT_Init(screen);
        CRT_DAC();
#else
        VH_UpdateScreen(screen);
#endif
#endif
    }
}




//===========================================================================

/*
=================
=
= VL_GetColor
=
=================
*/

void VL_GetColor	(int color, int *red, int *green, int *blue)
{
    SDL_Color *col = &curpal[color];
    *red = col->r;
    *green = col->g;
    *blue = col->b;
}

//===========================================================================

/*
=================
=
= VL_SetPalette
=
=================
*/

void VL_SetPalette(SDL_Color* palette, boolean forceupdate)
{
#ifndef __LIBRETRO__	
    memcpy(curpal, palette, sizeof(SDL_Color) * 256);
#else	
	memcpy(curpal, palette, sizeof(LR_Color) * 256);

	LR_SetPalette(screenBuffer->surf, SDL_LOGPAL, palette, 0, 256);
	if (forceupdate)
      VW_UpdateScreen();
#endif

    if(screenBits == 8)
#if SDL_MAJOR_VERSION == 1
        SDL_SetPalette(screen, SDL_PHYSPAL, palette, 0, 256);
    else
    {
        SDL_SetPalette(screenBuffer, SDL_LOGPAL, palette, 0, 256);
        if(forceupdate)
        {
            SDL_BlitSurface(screenBuffer, NULL, screen, NULL);

	        SDL_Flip (screen);
#else
        SDL_SetPaletteColors(screen->format->palette, palette, 0, 256);
    else
    {
        SDL_SetPaletteColors(screenBuffer->format->palette, palette, 0, 256);
        if (forceupdate)
        {
            SDL_BlitSurface(screenBuffer, NULL, screen, NULL);

            VH_RenderTextures(screen);
#endif
        }
    }
}


//===========================================================================

/*
=================
=
= VL_GetPalette
=
=================
*/

#ifndef __LIBRETRO__
void VL_GetPalette (SDL_Color *palette)
{
    memcpy(palette, curpal, sizeof(SDL_Color) * 256);
}
#else
void VL_GetPalette (LR_Color *palette)
{
   memcpy(palette, curpal, sizeof(LR_Color) * 256);
}
#endif
//===========================================================================

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
	int		    i,j,orig,delta;
	SDL_Color   *origptr, *newptr;

    red = red * 255 / 63;
    green = green * 255 / 63;
    blue = blue * 255 / 63;

	VL_WaitVBL(1);
	VL_GetPalette(palette1);
	memcpy(palette2, palette1, sizeof(SDL_Color) * 256);

//
// fade through intermediate frames
//
	for (i=0;i<steps;i++)
	{
		origptr = &palette1[start];
		newptr = &palette2[start];
		for (j=start;j<=end;j++)
		{
			orig = origptr->r;
			delta = red-orig;
			newptr->r = orig + delta * i / steps;
			orig = origptr->g;
			delta = green-orig;
			newptr->g = orig + delta * i / steps;
			orig = origptr->b;
			delta = blue-orig;
			newptr->b = orig + delta * i / steps;
			origptr++;
			newptr++;
		}

		if(!usedoublebuffering || screenBits == 8) VL_WaitVBL(1);
		VL_SetPalette (palette2, true);
	}

//
// final color
//
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

void VL_FadeIn (int start, int end, SDL_Color *palette, int steps)
{
	int j,delta;

	VL_WaitVBL(1);
	VL_GetPalette(palette1);
	memcpy(palette2, palette1, sizeof(SDL_Color) * 256);

//
// fade through intermediate frames
//
	for (int i=0; i<steps; i++)
	{
		for (j=start;j<=end;j++)
		{
			delta = palette[j].r-palette1[j].r;
			palette2[j].r = palette1[j].r + delta * i / steps;
			delta = palette[j].g-palette1[j].g;
			palette2[j].g = palette1[j].g + delta * i / steps;
			delta = palette[j].b-palette1[j].b;
			palette2[j].b = palette1[j].b + delta * i / steps;
		}

		if(!usedoublebuffering || screenBits == 8) VL_WaitVBL(1);
		VL_SetPalette(palette2, true);
	}

//
// final color
//
	VL_SetPalette (palette, true);
	screenfaded = false;
}

/*
=============================================================================

							PIXEL OPS

=============================================================================
*/
#ifndef __LIBRETRO__
byte *VL_LockSurface(SDL_Surface *surface)
{
    if(SDL_MUSTLOCK(surface))
    {
        if(SDL_LockSurface(surface) < 0)
            return NULL;
    }
    return (byte *) surface->pixels;
}

#else
byte *VL_LockSurface(LR_Surface *surface)
{
   return (byte *) surface->surf->pixels;
}
#endif
#ifndef __LIBRETRO__
void VL_UnlockSurface(SDL_Surface *surface)
{
    if(SDL_MUSTLOCK(surface))
    {
        SDL_UnlockSurface(surface);
    }
}
#else
void VL_UnlockSurface(LR_Surface *surface)
{
}
#endif
/*
=================
=
= VL_Plot
=
=================
*/

void VL_Plot (int x, int y, int color)
{
    byte *dest;

    assert(x >= 0 && (unsigned) x < screenWidth
            && y >= 0 && (unsigned) y < screenHeight
            && "VL_Plot: Pixel out of bounds!");

    dest = VL_LockSurface(screenBuffer);
    if(dest == NULL) return;

    dest[ylookup[y] + x] = color;

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

    assert_ret(x >= 0 && (unsigned) x < screenWidth
            && y >= 0 && (unsigned) y < screenHeight
            && "VL_GetPixel: Pixel out of bounds!");

    if (!VL_LockSurface(screenBuffer))
        return 0;

    col = ((byte *) screenBuffer->pixels)[ylookup[y] + x];

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
    byte *dest;

    assert(x >= 0 && x + width <= screenWidth
            && y >= 0 && y < screenHeight
            && "VL_Hlin: Destination rectangle out of bounds!");

    dest = VL_LockSurface(screenBuffer);
    if(dest == NULL) return;

    memset(dest + ylookup[y] + x, color, width);

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
	byte *dest;

	assert(x >= 0 && (unsigned) x < screenWidth
			&& y >= 0 && (unsigned) y + height <= screenHeight
			&& "VL_Vlin: Destination rectangle out of bounds!");

	dest = VL_LockSurface(screenBuffer);
	if(dest == NULL) return;

	dest += ylookup[y] + x;

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

void VL_Bar (int x, int y, int width, int height, int color)
{
    VL_BarScaledCoord(scaleFactor*x, scaleFactor*y,scaleFactor*width, scaleFactor*height, color);
}

void VL_BarScaledCoord (int scx, int scy, int scwidth, int scheight, int color)
{
	byte *dest;

	assert(scx >= 0 && (unsigned) scx + scwidth <= screenWidth
			&& scy >= 0 && (unsigned) scy + scheight <= screenHeight
			&& "VL_BarScaledCoord: Destination rectangle out of bounds!");

	dest = VL_LockSurface(screenBuffer);
	if(dest == NULL) return;

	dest += ylookup[scy] + scx;

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
===================
=
= VL_DePlaneVGA
=
= Unweave a VGA graphic to simplify drawing
=
===================
*/

void VL_DePlaneVGA (byte *source, int width, int height)
{
    int  x,y,plane;
    word size,pwidth;
    byte *temp,*dest,*srcline;

    size = width * height;

    if (width & 3)
        Quit ("DePlaneVGA: width not divisible by 4!");

    temp = SafeMalloc(size);

//
// munge pic into the temp buffer
//
    srcline = source;
    pwidth = width >> 2;

    for (plane = 0; plane < 4; plane++)
    {
        dest = temp;

        for (y = 0; y < height; y++)
        {
            for (x = 0; x < pwidth; x++)
                *(dest + (x << 2) + plane) = *srcline++;

            dest += width;
        }
    }

//
// copy the temp buffer back into the original source
//
    memcpy (source,temp,size);

    free (temp);
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

void VL_MemToScreen (byte *source, int width, int height, int x, int y)
{
    VL_MemToScreenScaledCoord(source, width, height, scaleFactor*x, scaleFactor*y);
}

void VL_MemToScreenScaledCoord (byte *source, int width, int height, int destx, int desty)
{
    byte *dest;
    int i, j, sci, scj;
    unsigned m, n;

    assert(destx >= 0 && destx + width * scaleFactor <= screenWidth
            && desty >= 0 && desty + height * scaleFactor <= screenHeight
            && "VL_MemToScreenScaledCoord: Destination rectangle out of bounds!");

    dest = VL_LockSurface(screenBuffer);
    if(dest == NULL) return;

    for(j = 0, scj = 0; j < height; j++, scj += scaleFactor)
    {
        for(i = 0, sci = 0; i < width; i++, sci += scaleFactor)
        {
            byte col = source[(j * width) + i];
            for(m = 0; m < scaleFactor; m++)
            {
                for(n = 0; n < scaleFactor; n++)
                {
                    dest[ylookup[scj + m + desty] + sci + n + destx] = col;
                }
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
    byte *dest;
    int i, j, sci, scj;
    unsigned m, n;

    assert(destx >= 0 && destx + width * scaleFactor <= screenWidth
            && desty >= 0 && desty + height * scaleFactor <= screenHeight
            && "VL_MemToScreenScaledCoord: Destination rectangle out of bounds!");

    dest = VL_LockSurface(screenBuffer);
    if(dest == NULL) return;

    for(j = 0, scj = 0; j < height; j++, scj += scaleFactor)
    {
        for(i = 0, sci = 0; i < width; i++, sci += scaleFactor)
        {
            byte col = source[((j + srcy) * origwidth) + (i + srcx)];

            for(m = 0; m < scaleFactor; m++)
            {
                for(n = 0; n < scaleFactor; n++)
                {
                    dest[ylookup[scj + m + desty] + sci + n + destx] = col;
                }
            }
        }
    }
    VL_UnlockSurface(screenBuffer);
}

//==========================================================================

/*
=================
=
= VL_ScreenToScreen
=
=================
*/

void VL_ScreenToScreen (SDL_Surface *source, SDL_Surface *dest)
{
    SDL_BlitSurface(source, NULL, dest, NULL);
}
