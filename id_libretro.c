// ID_LIBRETRO.C

#include <string.h>
#include "wl_def.h"
#include "surface.h"

void VL_WaitVBL(int vbls)
{
   rarch_sleep(vbls * 8);
}

void VW_UpdateScreen(void)
{
   VL_ScreenToScreen(screenBuffer, screen);
   LR_Flip(screen);
}


int IN_MouseButtons (void)
{
   return 0;
}
