/*******************************************************************************
 * Stefan Bylund 2017
 *
 * Implementation of set_sprite_attribute() in zxnext_sprite.h.
 ******************************************************************************/

#include <stdint.h>
#include <arch/zxn.h>
#include "zxnext_sprite.h"
#include "sprite_defs.h"

void set_sprite_attribute(uint8_t sprite_slot, uint8_t attribute_index, uint8_t value)
{
    if (attribute_index > SPRITE_ATTRIBUTE_EXT)
    {
        return;
    }

    IO_NEXTREG_REG = REG_SPRITE_NUMBER;
    IO_NEXTREG_DAT = sprite_slot & SPRITE_SLOT_MASK;

    IO_NEXTREG_REG = REG_SPRITE_ATTRIBUTE_0 + attribute_index;
    IO_NEXTREG_DAT = value;
}
