/*******************************************************************************
 * Stefan Bylund 2017
 *
 * Implementation of set_sprite_priority() in zxnext_sprite.h.
 ******************************************************************************/

#include <stdbool.h>
#include <arch/zxn.h>
#include "zxnext_sprite.h"
#include "sprite_defs.h"

void set_sprite_priority(bool sprite_0_on_top)
{
    IO_NEXTREG_REG = REG_SPRITE_LAYER_SYSTEM;
    IO_NEXTREG_DAT = (IO_NEXTREG_DAT & ~RSLS_SPRITE_PRIORITY_MASK) |
                     (sprite_0_on_top ? RSLS_SPRITE_PRIORITY_MASK : 0);
}
