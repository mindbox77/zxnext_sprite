/*******************************************************************************
 * Stefan Bylund 2017
 *
 * Implementation of set_sprite_clip_window() in zxnext_sprite.h.
 ******************************************************************************/

#include <stdint.h>
#include <arch/zxn.h>
#include "zxnext_sprite.h"

void set_sprite_clip_window(uint8_t x1, uint8_t x2, uint8_t y1, uint8_t y2)
{
    /* Reset the clip window coordinate index so that the writes start at X1. */
    IO_NEXTREG_REG = REG_CLIP_WINDOW_CONTROL;
    IO_NEXTREG_DAT = RCWC_RESET_SPRITE_CLIP_INDEX;

    IO_NEXTREG_REG = REG_CLIP_WINDOW_SPRITES;
    IO_NEXTREG_DAT = x1;
    IO_NEXTREG_DAT = x2;
    IO_NEXTREG_DAT = y1;
    IO_NEXTREG_DAT = y2;
}
