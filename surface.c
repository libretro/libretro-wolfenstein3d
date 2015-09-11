#include <stdint.h>

#if defined(__CELLOS_LV2__) && !defined(__PSL1GHT__)
#include <sys/timer.h>
#elif defined(XENON)
#include <time/time.h>
#elif defined(GEKKO) || defined(__PSL1GHT__) || defined(__QNX__)
#include <unistd.h>
#elif defined(PSP)
#include <pspthreadman.h> 
#elif defined(VITA)
#include <psp2/kernel/threadmgr.h>
#elif defined(_3DS)
#include <3ds.h>
#else
#include <time.h>
#endif

#include "surface.h"

typedef uint64_t retro_perf_tick_t;

/**
 * rarch_sleep:
 * @msec         : amount in milliseconds to sleep
 *
 * Sleeps for a specified amount of milliseconds (@msec).
 **/
static inline void rarch_sleep(unsigned msec)
{
#if defined(__CELLOS_LV2__) && !defined(__PSL1GHT__)
   sys_timer_usleep(1000 * msec);
#elif defined(PSP) || defined(VITA)
   sceKernelDelayThread(1000 * msec);
#elif defined(_3DS)
   svcSleepThread(1000000 * (s64)msec);
#elif defined(_WIN32)
   Sleep(msec);
#elif defined(XENON)
   udelay(1000 * msec);
#elif defined(GEKKO) || defined(__PSL1GHT__) || defined(__QNX__)
   usleep(1000 * msec);
#else
   struct timespec tv = {0};
   tv.tv_sec = msec / 1000;
   tv.tv_nsec = (msec % 1000) * 1000000;
   nanosleep(&tv, NULL);
#endif
}

static retro_perf_tick_t rarch_get_perf_counter(void)
{
   retro_perf_tick_t time_ticks = 0;
#if defined(__linux__) || defined(__QNX__) || defined(__MACH__)
   struct timespec tv;
   if (clock_gettime(CLOCK_MONOTONIC, &tv) == 0)
      time_ticks = (retro_perf_tick_t)tv.tv_sec * 1000000000 +
         (retro_perf_tick_t)tv.tv_nsec;

#elif defined(__GNUC__) && !defined(RARCH_CONSOLE)

#if defined(__i386__) || defined(__i486__) || defined(__i686__)
   __asm__ volatile ("rdtsc" : "=A" (time_ticks));
#elif defined(__x86_64__)
   unsigned a, d;
   __asm__ volatile ("rdtsc" : "=a" (a), "=d" (d));
   time_ticks = (retro_perf_tick_t)a | ((retro_perf_tick_t)d << 32);
#endif

#elif defined(__ARM_ARCH_6__)
   __asm__ volatile( "mrc p15, 0, %0, c9, c13, 0" : "=r"(time_ticks) );
#elif defined(__CELLOS_LV2__) || defined(GEKKO) || defined(_XBOX360) || defined(__powerpc__) || defined(__ppc__) || defined(__POWERPC__)
   time_ticks = __mftb();
#elif defined(PSP) || defined(VITA)
   sceRtcGetCurrentTick(&time_ticks);
#elif defined(_3DS)
   time_ticks = svcGetSystemTick();
#elif defined(__mips__)
   struct timeval tv;
   gettimeofday(&tv,NULL);
   time_ticks = (1000000 * tv.tv_sec + tv.tv_usec);
#elif defined(_WIN32)
   long tv_sec, tv_usec;
   static const unsigned __int64 epoch = 11644473600000000Ui64;
   FILETIME file_time;
   SYSTEMTIME system_time;
   ULARGE_INTEGER ularge;

   GetSystemTime(&system_time);
   SystemTimeToFileTime(&system_time, &file_time);
   ularge.LowPart = file_time.dwLowDateTime;
   ularge.HighPart = file_time.dwHighDateTime;

   tv_sec = (long)((ularge.QuadPart - epoch) / 10000000L);
   tv_usec = (long)(system_time.wMilliseconds * 1000);

   time_ticks = (1000000 * tv_sec + tv_usec);
#endif

   return time_ticks / 1000000;
}

uint32_t LR_GetTicks(void)
{
   return (uint32_t)rarch_get_perf_counter();
}

void LR_FillRect(LR_Surface *surface, const void *rect_data, uint32_t color)
{
   unsigned i, j;
   (void)rect_data;

   for (i = 0; i < surface->surf->w; i++)
      for (j = 0; j < surface->surf->h; j++)
      {
         uint8_t *pix = (uint8_t*)surface->surf->pixels + j * surface->surf->pitch + i * 4;
         *(uint32_t*)pix = color;
      }
}

void LR_Delay(uint32_t ms)
{
   rarch_sleep(ms);
}

void LR_SetPalette(SDL_Surface *surface, int flags, LR_Color *colors, int firstcolor, int ncolors)
{
   SDL_SetPalette(surface, flags, (SDL_Color*)colors, firstcolor, ncolors);
}

int LR_SetColors(SDL_Surface *surface, LR_Color *colors, int firstcolor, int ncolors)
{
   return SDL_SetColors(surface, (SDL_Color*)colors, firstcolor, ncolors);
}

int LR_Init(uint32_t flags)
{
   return SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
}

void LR_Quit(void)
{
   SDL_Quit();
}

SDL_Surface* LR_CreateRGBSurface(uint32_t flags,
      int    width,
      int    height,
      int    depth,
      uint32_t Rmask,
      uint32_t Gmask,
      uint32_t Bmask,
      uint32_t Amask)
{
   return SDL_CreateRGBSurface(flags, width, height, depth, Rmask, Gmask, Bmask, Amask);
}

void LR_FreeSurface(SDL_Surface* surface)
{
   SDL_FreeSurface(surface);
}

int LR_BlitSurface(LR_Surface *src, SDL_Rect *srcrect, LR_Surface *dst, SDL_Rect *dstrect)
{
   return SDL_UpperBlit(src->surf, srcrect, dst->surf, dstrect);
}

int LR_Flip(SDL_Surface *screen)
{
   return SDL_Flip(screen);
}

SDL_Surface *LR_SetVideoMode(int width, int height, int bpp, uint32_t flags)
{
   return SDL_SetVideoMode(width, height, bpp, flags);
}

SDL_Surface *LR_ConvertSurface(LR_Surface *src, SDL_PixelFormat *fmt, uint32_t flags)
{
   return SDL_ConvertSurface(src->surf, fmt, flags);
}

uint32_t LR_MapRGB(SDL_PixelFormat *fmt, uint8_t r, uint8_t g, uint8_t b)
{
   return SDL_MapRGB(fmt, r, g, b);
}
