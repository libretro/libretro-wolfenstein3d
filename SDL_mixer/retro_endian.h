
#ifndef _RETRO_ENDIAN_H
#define _RETRO_ENDIAN_H

#include <retro_inline.h>

static INLINE int32_t Retro_SwapLES32(int32_t x)
{
#ifdef MSB_FIRST
   uint8_t b1 = x         & 255;
   uint8_t b2 = (x >>  8) & 255;
   uint8_t b3 = (x >> 16) & 255;
   uint8_t b4 = (x >> 24) & 255;

   return ((int32_t)b1 << 24) + ((int32_t)b2 << 16) + ((int32_t)b3 << 8) + b4;
#else
   return x;
#endif
}

int16_t INLINE int16_t Retro_SwapLES16(int16_t x)
{
#ifdef MSB_FIRST
   uint8_t b1 = x        & 255;
   uint8_t b2 = (x >> 8) & 255;

   return (b1 << 8) + b2;
#else
   return x;
#endif
}

static INLINE uint16_t Retro_SwapLE16(uint16_t x)
{
#ifdef MSB_FIRST
   return (x << 8) | (x >> 8);
#else
   return x;
#endif
}

static INLINE uint16_t Retro_SwapBE16(uint16_t x)
{
#ifdef MSB_FIRST
   return x;
#else
   return (x << 8) | (x >> 8);
#endif
}

#endif
