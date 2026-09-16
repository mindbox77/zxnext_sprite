/*******************************************************************************
 * Stefan Bylund 2017
 *
 * Implementation of set_sprite_pattern_slot_4bit() in zxnext_sprite.h.
 ******************************************************************************/

#include <stdint.h>
#include <arch/zxn.h>
#include "zxnext_sprite.h"
#include "sprite_defs.h"

void set_sprite_pattern_slot_4bit(uint8_t pattern_slot)
{
    /*
     * Bits 6-0 select the 256-byte pattern slot and bit 7 selects its second
     * 128-byte half, i.e. 4-bit pattern number N5..N0,N6 is written as N6,N5..N0.
     */
    IO_SPRITE_SLOT = (pattern_slot >> PATTERN_4BIT_N_SHIFT) |
                     ((pattern_slot & PATTERN_4BIT_N6_MASK) ? PATTERN_HALF_MASK : 0);
}
