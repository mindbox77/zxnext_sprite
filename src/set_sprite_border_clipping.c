/*******************************************************************************
 * Stefan Bylund 2017
 *
 * Implementation of set_sprite_border_clipping() in zxnext_sprite.h.
 ******************************************************************************/

#include <stdbool.h>
#include <arch/zxn.h>
#include "zxnext_sprite.h"
#include "sprite_defs.h"

void set_sprite_border_clipping(bool enable)
{
    IO_NEXTREG_REG = REG_SPRITE_LAYER_SYSTEM;
    IO_NEXTREG_DAT = (IO_NEXTREG_DAT & ~RSLS_CLIP_OVER_BORDER_MASK) |
                     (enable ? RSLS_CLIP_OVER_BORDER_MASK : 0);
}
