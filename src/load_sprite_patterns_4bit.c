/*******************************************************************************
 * Stefan Bylund 2017
 *
 * Implementation of load_sprite_patterns_4bit() in zxnext_sprite.h.
 ******************************************************************************/

#include <stddef.h>
#include <stdint.h>
#include <errno.h>
#include <arch/zxn/esxdos.h>
#include "zxnext_sprite.h"

void load_sprite_patterns_4bit(const char *filename,
                               const void *sprite_pattern_buf,
                               uint8_t num_sprite_patterns,
                               uint8_t start_sprite_pattern_slot)
{
    uint8_t filehandle;

    if ((filename == NULL) || (sprite_pattern_buf == NULL) ||
        (num_sprite_patterns == 0) || (start_sprite_pattern_slot > MAX_SPRITE_PATTERNS_4BIT - 1))
    {
        return;
    }

    if (start_sprite_pattern_slot + num_sprite_patterns > MAX_SPRITE_PATTERNS_4BIT)
    {
        num_sprite_patterns = MAX_SPRITE_PATTERNS_4BIT - start_sprite_pattern_slot;
    }

    errno = 0;
    filehandle = esxdos_f_open(filename, ESXDOS_MODE_R | ESXDOS_MODE_OE);
    if (errno)
    {
        return;
    }

    set_sprite_pattern_slot_4bit(start_sprite_pattern_slot);

    while (num_sprite_patterns--)
    {
        esxdos_f_read(filehandle, (void *) sprite_pattern_buf, SPRITE_PATTERN_SIZE_4BIT);
        if (errno)
        {
            break;
        }
        set_sprite_pattern_4bit(sprite_pattern_buf);
    }

    esxdos_f_close(filehandle);
}
