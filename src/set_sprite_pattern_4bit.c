/*******************************************************************************
 * Stefan Bylund 2017
 *
 * Implementation of set_sprite_pattern_4bit() in zxnext_sprite.h.
 ******************************************************************************/

#include <intrinsic.h>
#include <arch/zxn.h>
#include "zxnext_sprite.h"

void set_sprite_pattern_4bit(const void *sprite_pattern)
{
    intrinsic_outi((void *) sprite_pattern, __IO_SPRITE_PATTERN, 128);
}
