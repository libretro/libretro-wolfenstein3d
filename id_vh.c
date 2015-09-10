#include "wl_def.h"

pictabletype    *pictable;
SDL_Surface     *latchpics[NUMLATCHPICS];

int     px,py;
byte    fontcolor,backcolor;
int     fontnumber;

void VWB_DrawPropString(const char* string)
{
   int         width, step;
   byte        *source;
   byte        ch;
   unsigned     i, sy, sx;
   byte       *vbuf = LOCK();
   fontstruct *font = (fontstruct *) grsegs[STARTFONT+fontnumber];
   int       height = font->height;
   byte       *dest = vbuf + scaleFactor * (py * curPitch + px);

   while ((ch = (byte)*string++) != 0)
   {
      width  = step = font->width[ch];
      source = ((byte *)font)+font->location[ch];

      while (width--)
      {
         for(i=0;i<height;i++)
         {
            if(source[i*step])
            {
               for(sy=0; sy<scaleFactor; sy++)
                  for(sx=0; sx<scaleFactor; sx++)
                     dest[(scaleFactor*i+sy)*curPitch+sx]=fontcolor;
            }
         }

         source++;
         px++;
         dest+=scaleFactor;
      }
   }

   UNLOCK();
}

/*
=================
=
= VL_MungePic
=
=================
*/

void VL_MungePic (byte *source, unsigned width, unsigned height)
{
   unsigned x,y,plane,pwidth;
   byte *temp, *dest, *srcline;
   unsigned size = width*height;

   if (width&3)
      Quit ("VL_MungePic: Not divisable by 4!");

   /* copy the pic to a temp buffer */
   temp=(byte *) malloc(size);
   CHECKMALLOCRESULT(temp);
   memcpy (temp,source,size);

   /* munge it back into the original buffer */
   dest = source;
   pwidth = width/4;

   for (plane=0;plane<4;plane++)
   {
      srcline = temp;
      for (y=0;y<height;y++)
      {
         for (x=0;x<pwidth;x++)
            *dest++ = *(srcline+x*4+plane);
         srcline+=width;
      }
   }

   free(temp);
}

void VWL_MeasureString (const char *string, word *width, word *height, fontstruct *font)
{
   *height = font->height;
   for (*width = 0;*string;string++)
      *width += font->width[*((byte *)string)];   // proportional width
}

void VW_MeasurePropString (const char *string, word *width, word *height)
{
   VWL_MeasureString(string,width,height,(fontstruct *)grsegs[STARTFONT+fontnumber]);
}

/*
=============================================================================

                Double buffer management routines

=============================================================================
*/

void VH_UpdateScreen(void)
{
   VL_ScreenToScreen(screenBuffer, screen);
   SDL_Flip(screen);
}


void VWB_DrawTile8 (int x, int y, int tile)
{
   LatchDrawChar(x,y,tile);
}

void VWB_DrawTile8M (int x, int y, int tile)
{
   VL_MemToScreen (((byte *)grsegs[STARTTILE8M])+tile*64,8,8,x,y);
}

void VWB_DrawPic (int x, int y, int chunknum)
{
   int picnum      = chunknum - STARTPICS;
   unsigned width  = pictable[picnum].width;
   unsigned height = pictable[picnum].height;

   x &= ~7;

   VL_MemToScreen (grsegs[chunknum], width, height, x, y);
}

void VWB_DrawPicScaledCoord (int scx, int scy, int chunknum)
{
   int picnum      = chunknum - STARTPICS;
   unsigned width  = pictable[picnum].width;
   unsigned height = pictable[picnum].height;

   VL_MemToScreenScaledCoord (grsegs[chunknum], width, height, scx, scy);
}


void VWB_Bar (int x, int y, int width, int height, int color)
{
   VW_Bar (x, y, width, height, color);
}

void VWB_Plot (int x, int y, int color)
{
   if(scaleFactor == 1)
      VW_Plot(x,y,color);
   else
      VW_Bar(x, y, 1, 1, color);
}

void VWB_Hlin (int x1, int x2, int y, int color)
{
   if(scaleFactor == 1)
      VW_Hlin(x1,x2,y,color);
   else
      VW_Bar(x1, y, x2-x1+1, 1, color);
}

void VWB_Vlin (int y1, int y2, int x, int color)
{
   if(scaleFactor == 1)
      VW_Vlin(y1,y2,x,color);
   else
      VW_Bar(x, y1, 1, y2-y1+1, color);
}


/*
=============================================================================

                        WOLFENSTEIN STUFF

=============================================================================
*/

/*
=====================
=
= LatchDrawPic
=
=====================
*/

void LatchDrawPic (unsigned x, unsigned y, unsigned picnum)
{
   VL_LatchToScreenScaledCoord2(latchpics[2+picnum-LATCHPICS_LUMP_START], scaleFactor * (x * 8), scaleFactor * y);
}

void LatchDrawPicScaledCoord (unsigned scx, unsigned scy, unsigned picnum)
{
   VL_LatchToScreenScaledCoord2(latchpics[2+picnum-LATCHPICS_LUMP_START], scx*8, scy);
}


//==========================================================================

/*
===================
=
= LoadLatchMem
=
===================
*/

void LoadLatchMem (void)
{
   int i,width,height,start,end;
   byte *src;

   /* tile 8s */
   SDL_Surface *surf = SDL_CreateRGBSurface(SDL_HWSURFACE, 8*8,
         ((NUMTILE8 + 7) / 8) * 8, 8, 0, 0, 0, 0);

   if(!surf)
      Quit("Unable to create surface for tiles!");

   LR_SetColors(surf, gamepal, 0, 256);

   latchpics[0] = surf;
   CA_CacheGrChunk (STARTTILE8);
   src = grsegs[STARTTILE8];

   for (i=0;i<NUMTILE8;i++)
   {
      VL_MemToLatch (src, 8, 8, surf, (i & 7) * 8, (i >> 3) * 8);
      src += 64;
   }
   UNCACHEGRCHUNK (STARTTILE8);

   /* pics */
   start = LATCHPICS_LUMP_START;
   end   = LATCHPICS_LUMP_END;

   for (i = start; i <= end; i++)
   {
      width  = pictable[i-STARTPICS].width;
      height = pictable[i-STARTPICS].height;
      surf   = SDL_CreateRGBSurface(SDL_HWSURFACE, width, height, 8, 0, 0, 0, 0);

      if(!surf)
         Quit("Unable to create surface for picture!");
      LR_SetColors(surf, gamepal, 0, 256);

      latchpics[2+i-start] = surf;
      CA_CacheGrChunk (i);
      VL_MemToLatch (grsegs[i], width, height, surf, 0, 0);
      UNCACHEGRCHUNK(i);
   }
}

//==========================================================================

/*
===================
=
= FizzleFade
=
= returns true if aborted
=
= It uses maximum-length Linear Feedback Shift Registers (LFSR) counters.
= You can find a list of them with lengths from 3 to 168 at:
= http://www.xilinx.com/support/documentation/application_notes/xapp052.pdf
= Many thanks to Xilinx for this list!!!
=
===================
*/

/* XOR masks for the pseudo-random number sequence starting with n=17 bits */
static const uint32_t rndmasks[] = {
                    // n    XNOR from (starting at 1, not 0 as usual)
    0x00012000,     // 17   17,14
    0x00020400,     // 18   18,11
    0x00040023,     // 19   19,6,2,1
    0x00090000,     // 20   20,17
    0x00140000,     // 21   21,19
    0x00300000,     // 22   22,21
    0x00420000,     // 23   23,18
    0x00e10000,     // 24   24,23,22,17
    0x01200000,     // 25   25,22      (this is enough for 8191x4095)
};

static unsigned int rndbits_y;
static unsigned int rndmask;

extern LR_Color curpal[256];

/* Returns the number of bits needed to represent the given value */
static int log2_ceil(uint32_t x)
{
   int n      = 0;
   uint32_t v = 1;

   while(v < x)
   {
      n++;
      v <<= 1;
   }
   return n;
}

void VH_Startup(void)
{
   int rndbits;
   int rndbits_x = log2_ceil(screenWidth);

   rndbits_y     = log2_ceil(screenHeight);
   rndbits       = rndbits_x + rndbits_y;

   if(rndbits < 17)
      rndbits = 17;       // no problem, just a bit slower
   else if(rndbits > 25)
      rndbits = 25;       // fizzle fade will not fill whole screen

   rndmask = rndmasks[rndbits - 17];
}

static boolean FizzleFadeFinish(SDL_Surface *source_copy, SDL_Surface *screen_copy)
{
   VL_UnlockSurface(source_copy);
   VL_UnlockSurface(screen_copy);
   VL_ScreenToScreen(screen_copy, screenBuffer);
   VH_UpdateScreen();
   SDL_FreeSurface(source_copy);
   SDL_FreeSurface(screen_copy);
   return false;
}

boolean FizzleFade (SDL_Surface *source, int x1, int y1,
    unsigned width, unsigned height, unsigned frames, boolean abortable)
{
   unsigned x, y, frame;
   int32_t  rndval;
   unsigned i, p;
   SDL_Surface *source_copy, *screen_copy;
   byte *srcptr;
   int32_t lastrndval = 0;
   unsigned pixperframe = width * height / frames;

   IN_StartAck ();

   frame       = GetTimeCount();

   /* can't rely on screen as dest b/c crt.cpp writes over it with screenBuffer
    * can't rely on screenBuffer as source for same reason: every flip it has to be updated
    */
   source_copy = SDL_ConvertSurface(source, source->format, source->flags);
   screen_copy = SDL_ConvertSurface(screen, screen->format, screen->flags);
   srcptr      = VL_LockSurface(source_copy);

   do
   {
      byte *destptr;

      if(abortable && IN_CheckAck ())
      {
         VL_UnlockSurface(source_copy);
         VL_ScreenToScreen(screen_copy, screenBuffer);
         VH_UpdateScreen();

         SDL_FreeSurface(source_copy);
         SDL_FreeSurface(screen_copy);
         return true;
      }

      destptr = VL_LockSurface(screen_copy);
      rndval  = lastrndval;

      for(p = 0; p < pixperframe; p++)
      {
         byte col;
         uint32_t fullcol;

         /* seperate random value into x/y pair */
         x = rndval >> rndbits_y;
         y = rndval & ((1 << rndbits_y) - 1);

         /* advance to next random element */
         rndval = (rndval >> 1) ^ (rndval & 1 ? 0 : rndmask);

         if(x >= width || y >= height)
         {
            if(rndval == 0)     /* entire sequence has been completed */
               return FizzleFadeFinish(source_copy, screen_copy);
            p--;
            continue;
         }

         /* copy one pixel */
         col     = *(srcptr + (y1 + y) * source->pitch + x1 + x);
         fullcol = SDL_MapRGB(screen->format, curpal[col].r, curpal[col].g, curpal[col].b);
         memcpy(destptr + (y1 + y) * screen->pitch + (x1 + x) * screen->format->BytesPerPixel,
               &fullcol, screen->format->BytesPerPixel);

         if(rndval == 0)     /* entire sequence has been completed */
            return FizzleFadeFinish(source_copy, screen_copy);
      }

      lastrndval = rndval;

      VL_UnlockSurface(screen_copy);
      VL_ScreenToScreen(screen_copy, screenBuffer);
      VH_UpdateScreen();

      frame++;
      Delay(frame - GetTimeCount()); /* don't go too fast */
   } while (1);

   return FizzleFadeFinish(source_copy, screen_copy);
}
