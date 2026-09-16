/*******************************************************************************
 * Stefan Bylund 2017
 *
 * Implementation of set_extended_sprite_attributes() in zxnext_sprite.h.
 ******************************************************************************/

#include <stdint.h>
#include <stdbool.h>
#include <arch/zxn.h>
#include "zxnext_sprite.h"
#include "sprite_defs.h"

#define X_LSB(x) (uint8_t) ((x) & X_LSB_MASK)

#define X_MSB(x) (uint8_t) (((x) & X_MSB_MASK) >> X_MSB_SHIFT)

#define Y_LSB(y) (uint8_t) ((y) & Y_LSB_MASK)

#define Y_MSB(y) (uint8_t) (((y) & Y_MSB_MASK) >> Y_MSB_SHIFT)

void set_extended_sprite_attributes(uint8_t sprite_pattern_slot,
                                    uint16_t x,
                                    uint16_t y,
                                    uint8_t palette_offset,
                                    uint8_t sprite_flags,
                                    uint8_t sprite_flags_ext,
                                    bool visible)
{
    uint8_t pattern_slot;
    uint8_t attr4 = (sprite_flags_ext & ATTR4_ANCHOR_FLAGS_MASK) | Y_MSB(y);

    if (sprite_flags_ext & SPRITE_4BIT_MASK)
    {
        /* 4-bit pattern number N5..N0,N6: N5..N0 in attribute 3, N6 in attribute 4. */
        if (sprite_pattern_slot & PATTERN_4BIT_N6_MASK)
        {
            attr4 = attr4 | ATTR4_N6_ANCHOR_MASK;
        }
        sprite_pattern_slot = sprite_pattern_slot >> PATTERN_4BIT_N_SHIFT;
    }

    pattern_slot = (sprite_pattern_slot & SPRITE_PATTERN_SLOT_MASK) | SPRITE_EXTENDED_MASK;

    if (visible)
    {
        pattern_slot = pattern_slot | SPRITE_VISIBLE_MASK;
    }

    IO_SPRITE_ATTRIBUTE = X_LSB(x);
    IO_SPRITE_ATTRIBUTE = Y_LSB(y);
    IO_SPRITE_ATTRIBUTE = (palette_offset << PALETTE_OFFSET_SHIFT) | (sprite_flags & SPRITE_FLAGS_MASK) | X_MSB(x);
    IO_SPRITE_ATTRIBUTE = pattern_slot;
    IO_SPRITE_ATTRIBUTE = attr4;
}
