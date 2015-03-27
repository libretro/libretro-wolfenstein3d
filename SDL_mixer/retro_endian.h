
#ifndef _RETRO_ENDIAN_H
#define _RETRO_ENDIAN_H

#include <retro_inline.h>

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
