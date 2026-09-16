/*******************************************************************************
 * Stefan Bylund 2017
 *
 * A simple sprite demo program for ZX Spectrum Next.
 *
 * Sprite 0 is a plain 8-bit sprite using the original 4-byte attributes.
 * Sprite 1 is the same pattern scaled 2x and mirrored using extended attributes.
 * Sprites 2 and 3 use a 4-bit pattern with two different palette offsets.
 * Sprites 4 and 5 form a unified big sprite (anchor + relative sprite) that is
 * periodically rotated by writing a single attribute byte via Next registers.
 * A clip window hides the sprites 8 pixels from the edges of the screen.
 *
 * zcc +zxn -subtype=nex -vn -SO3 -startup=31 -clib=sdcc_iy
 *   --max-allocs-per-node200000 -L<zxnext_sprite>/lib/sdcc_iy -lzxnext_sprite
 *   -I<zxnext_sprite>/include zxnext_sprite_demo.c -o zxnext_sprite_demo -create-app
 ******************************************************************************/

#include <arch/zxn.h>
#include <input.h>
#include <z80.h>
#include <intrinsic.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "zxnext_sprite.h"

#pragma output CRT_ORG_CODE = 0x8184
#pragma output REGISTER_SP = 0xFF58
#pragma output CLIB_MALLOC_HEAP_SIZE = 0
#pragma output CLIB_STDIO_HEAP_SIZE = 0
#pragma output CLIB_FOPEN_MAX = -1

/*
 * Define IDE_FRIENDLY in your C IDE to disable Z88DK C extensions and avoid
 * parser errors/warnings in the IDE. Do NOT define IDE_FRIENDLY when compiling
 * the code with Z88DK.
 */
#ifdef IDE_FRIENDLY
#define __z88dk_fastcall
#define __preserves_regs(...)
#endif

/*******************************************************************************
 * Type Definitions
 ******************************************************************************/

typedef struct sprite_info {
    uint8_t x; // X coordinate in pixels
    uint8_t y; // Y coordinate in pixels
    int8_t dx; // Horizontal displacement in pixels
    int8_t dy; // Vertical displacement in pixels
} sprite_info_t;

/*******************************************************************************
 * Function Prototypes
 ******************************************************************************/

static void init_hardware(void);

static void init_isr(void);

static void create_background(void);

static void create_sprites(void);

static void update_sprites(void);

static void move_sprite(sprite_info_t *sprite, uint8_t max_x, uint8_t max_y);

/*******************************************************************************
 * Variables
 ******************************************************************************/

/* 8-bit sprite pattern, 256 bytes, palette index 0xE3 is transparent. */
static const uint8_t sprite_pattern[] =
{
    0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0xE3, 0xE3, 0xE3, 0xE3, 0xE3, 0xE3, 0xE3, 0xE3, 0xE3,
    0x04, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x04, 0xE3, 0xE3, 0xE3, 0xE3, 0xE3, 0xE3, 0xE3, 0xE3, 0xE3,
    0x04, 0xFF, 0xFB, 0xFB, 0xFB, 0xFF, 0x04, 0xE3, 0xE3, 0xE3, 0xE3, 0xE3, 0xE3, 0xE3, 0xE3, 0xE3,
    0x04, 0xFF, 0xFB, 0xF5, 0xF5, 0xFB, 0xFF, 0x04, 0xE3, 0xE3, 0xE3, 0xE3, 0xE3, 0xE3, 0xE3, 0xE3,
    0x04, 0xFF, 0xFB, 0xF5, 0xA8, 0xA8, 0xFB, 0xFF, 0x04, 0xE3, 0xE3, 0xE3, 0xE3, 0xE3, 0xE3, 0xE3,
    0x04, 0xFF, 0xFF, 0xFB, 0xA8, 0x44, 0xA8, 0xFB, 0xFF, 0x04, 0xE3, 0xE3, 0xE3, 0xE3, 0xE3, 0xE3,
    0x04, 0x04, 0x04, 0xFF, 0xFB, 0xA8, 0x44, 0xA8, 0xFB, 0xFF, 0x04, 0xE3, 0xE3, 0xE3, 0xE3, 0xE3,
    0xE3, 0xE3, 0xE3, 0x04, 0xFF, 0xFB, 0xA8, 0x44, 0x44, 0xFB, 0xFF, 0x04, 0xE3, 0x04, 0xE3, 0xE3,
    0xE3, 0xE3, 0xE3, 0xE3, 0x04, 0xFF, 0xFB, 0x44, 0x44, 0x44, 0xFB, 0xFF, 0x04, 0x4D, 0x04, 0xE3,
    0xE3, 0xE3, 0xE3, 0xE3, 0xE3, 0x04, 0xFF, 0xFB, 0x44, 0x44, 0x44, 0x44, 0xFA, 0x4D, 0x04, 0xE3,
    0xE3, 0xE3, 0xE3, 0xE3, 0xE3, 0xE3, 0x04, 0xFF, 0xFB, 0x44, 0xFF, 0xF5, 0x44, 0x04, 0xE3, 0xE3,
    0xE3, 0xE3, 0xE3, 0xE3, 0xE3, 0xE3, 0xE3, 0x04, 0xFF, 0x44, 0xF5, 0xA8, 0x04, 0xE3, 0xE3, 0xE3,
    0xE3, 0xE3, 0xE3, 0xE3, 0xE3, 0xE3, 0xE3, 0xE3, 0x04, 0xFA, 0x44, 0x04, 0xA8, 0x04, 0xE3, 0xE3,
    0xE3, 0xE3, 0xE3, 0xE3, 0xE3, 0xE3, 0xE3, 0x04, 0x4D, 0x4D, 0x04, 0xE3, 0x04, 0xF5, 0x04, 0xE3,
    0xE3, 0xE3, 0xE3, 0xE3, 0xE3, 0xE3, 0xE3, 0xE3, 0x04, 0x04, 0xE3, 0xE3, 0xE3, 0x04, 0xFA, 0x04,
    0xE3, 0xE3, 0xE3, 0xE3, 0xE3, 0xE3, 0xE3, 0xE3, 0xE3, 0xE3, 0xE3, 0xE3, 0xE3, 0xE3, 0x04, 0x04
};

/*
 * 4-bit version of the same sprite pattern, 128 bytes with two pixels per byte.
 * Each pixel is an index 0 - 8 into a 16-colour sub-palette, index 3 (the low
 * nibble of the default transparency index 0xE3) is transparent.
 */
static const uint8_t sprite_pattern_4bit[] =
{
    0x00, 0x00, 0x00, 0x03, 0x33, 0x33, 0x33, 0x33,
    0x01, 0x11, 0x11, 0x03, 0x33, 0x33, 0x33, 0x33,
    0x01, 0x22, 0x21, 0x03, 0x33, 0x33, 0x33, 0x33,
    0x01, 0x24, 0x42, 0x10, 0x33, 0x33, 0x33, 0x33,
    0x01, 0x24, 0x55, 0x21, 0x03, 0x33, 0x33, 0x33,
    0x01, 0x12, 0x56, 0x52, 0x10, 0x33, 0x33, 0x33,
    0x00, 0x01, 0x25, 0x65, 0x21, 0x03, 0x33, 0x33,
    0x33, 0x30, 0x12, 0x56, 0x62, 0x10, 0x30, 0x33,
    0x33, 0x33, 0x01, 0x26, 0x66, 0x21, 0x07, 0x03,
    0x33, 0x33, 0x30, 0x12, 0x66, 0x66, 0x87, 0x03,
    0x33, 0x33, 0x33, 0x01, 0x26, 0x14, 0x60, 0x33,
    0x33, 0x33, 0x33, 0x30, 0x16, 0x45, 0x03, 0x33,
    0x33, 0x33, 0x33, 0x33, 0x08, 0x60, 0x50, 0x33,
    0x33, 0x33, 0x33, 0x30, 0x77, 0x03, 0x04, 0x03,
    0x33, 0x33, 0x33, 0x33, 0x00, 0x33, 0x30, 0x80,
    0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x00
};

/* Sub-palette 1 (palette indexes 16 - 24) for the 4-bit sprite, RGB333 colours. */
static const uint16_t sprite_palette_1[] =
{
    RGB333_FROM_RGB332(0x04), RGB333_FROM_RGB332(0xFF), RGB333_FROM_RGB332(0xFB), RGB333_FROM_RGB332(0xE3), RGB333_FROM_RGB332(0xF5),
    RGB333_FROM_RGB332(0xA8), RGB333_FROM_RGB332(0x44), RGB333_FROM_RGB332(0x4D), RGB333_FROM_RGB332(0xFA)
};

/* Sub-palette 2 (palette indexes 32 - 40): the same colours with red and green swapped. */
static const uint16_t sprite_palette_2[] =
{
    RGB333_FROM_RGB332(0x20), RGB333_FROM_RGB332(0xFF), RGB333_FROM_RGB332(0xDF), RGB333_FROM_RGB332(0x1F), RGB333_FROM_RGB332(0xBD),
    RGB333_FROM_RGB332(0x54), RGB333_FROM_RGB332(0x28), RGB333_FROM_RGB332(0x69), RGB333_FROM_RGB332(0xDE)
};

/* Plain 8-bit sprite (sprite 0), 16 * 16 pixels. */
static sprite_info_t sprite = {120, 88, 1, 1};

/* Scaled 2x sprite (sprite 1), 32 * 32 pixels. */
static sprite_info_t scaled_sprite = {40, 120, -1, 1};

/* Unified big sprite (sprites 4 and 5), 32 * 16 pixels. */
static sprite_info_t big_sprite = {180, 40, 1, -1};

/* Rotation state of the unified big sprite. */
static uint8_t big_sprite_flags = 0;

static uint8_t frame_counter = 0;

/*******************************************************************************
 * Functions
 ******************************************************************************/

static void init_hardware(void)
{
    // Put Z80 in 28 MHz turbo mode.
    ZXN_NEXTREG(REG_TURBO_MODE, 0x03);

    // Disable RAM memory contention.
    ZXN_NEXTREGA(REG_PERIPHERAL_3, ZXN_READ_REG(REG_PERIPHERAL_3) | RP3_DISABLE_CONTENTION);
}

static void init_isr(void)
{
    // Set up IM2 interrupt service routine:
    // Put Z80 in IM2 mode with a 257-byte interrupt vector table located
    // at 0x8000 (before CRT_ORG_CODE) filled with 0x81 bytes. Install an
    // empty interrupt service routine at the interrupt service routine
    // entry at address 0x8181.

    intrinsic_di();
    im2_init((void *) 0x8000);
    memset((void *) 0x8000, 0x81, 257);
    z80_bpoke(0x8181, 0xFB);
    z80_bpoke(0x8182, 0xED);
    z80_bpoke(0x8183, 0x4D);
    intrinsic_ei();
}

static void create_background(void)
{
    zx_border(INK_YELLOW);
    zx_cls(BRIGHT | INK_BLACK | PAPER_WHITE);
}

static void create_sprites(void)
{
    // Sprite transparency index (this is the default value).
    set_sprite_transparency_index(SPRITE_TRANSPARENCY_INDEX_DEFAULT);

    // Sub-palettes 1 and 2 for the 4-bit sprites.
    set_sprite_rw_palette(true);
    set_sprite_palette(sprite_palette_1, 9, 16);
    set_sprite_palette(sprite_palette_2, 9, 32);

    // Upload the 8-bit pattern to 8-bit pattern slot 0 and the 4-bit pattern
    // to 4-bit pattern slot 2 (i.e. the first half of 8-bit pattern slot 1).
    set_sprite_slot(0);
    set_sprite_pattern(sprite_pattern);
    set_sprite_pattern_slot_4bit(2);
    set_sprite_pattern_4bit(sprite_pattern_4bit);

    // Sprite 0: plain 8-bit sprite using 4-byte attributes.
    set_sprite_slot(0);
    set_sprite_attributes_ext(0, sprite.x, sprite.y, 0, 0, true);

    // Sprite 1: same pattern, scaled 2x and mirrored x-wise.
    set_extended_sprite_attributes(0,
                                   SPRITE_X_FROM_SCREEN(scaled_sprite.x),
                                   SPRITE_Y_FROM_SCREEN(scaled_sprite.y),
                                   0, MIRROR_X_MASK, SCALE_X_2 | SCALE_Y_2, true);

    // Sprites 2 and 3: 4-bit pattern 2 with palette offsets 1 and 2.
    set_extended_sprite_attributes(2, SPRITE_X_FROM_SCREEN(8), SPRITE_Y_FROM_SCREEN(8),
                                   1, 0, SPRITE_4BIT_MASK, true);
    set_extended_sprite_attributes(2, SPRITE_X_FROM_SCREEN(232), SPRITE_Y_FROM_SCREEN(8),
                                   2, MIRROR_X_MASK, SPRITE_4BIT_MASK, true);

    // Sprite 4: anchor of a unified big sprite.
    set_extended_sprite_attributes(0,
                                   SPRITE_X_FROM_SCREEN(big_sprite.x),
                                   SPRITE_Y_FROM_SCREEN(big_sprite.y),
                                   0, big_sprite_flags, RELATIVE_SPRITES_UNIFIED_MASK, true);

    // Sprite 5: relative sprite 16 pixels to the right of the anchor, mirrored.
    set_relative_sprite_attributes(0, 16, 0, 0, MIRROR_X_MASK, 0, true);

    // Hide the sprites 8 pixels from the edges of the screen.
    set_sprite_clip_window(8, 247, 8, 183);
}

static void move_sprite(sprite_info_t *sprite, uint8_t max_x, uint8_t max_y)
{
    // Calculate next position of sprite.
    sprite->x += sprite->dx;
    sprite->y += sprite->dy;

    // If sprite is at the edge of the screen then change its direction.
    if ((sprite->x == 0) || (sprite->x >= max_x))
    {
        sprite->dx = -sprite->dx;
    }
    if ((sprite->y == 0) || (sprite->y >= max_y))
    {
        sprite->dy = -sprite->dy;
    }
}

static void update_sprites(void)
{
    move_sprite(&sprite, 240, 176);
    move_sprite(&scaled_sprite, 224, 160);
    move_sprite(&big_sprite, 224, 176);

    // Update sprite positions.
    set_sprite_slot(0);
    set_sprite_attributes_ext(0, sprite.x, sprite.y, 0, 0, true);
    set_extended_sprite_attributes(0,
                                   SPRITE_X_FROM_SCREEN(scaled_sprite.x),
                                   SPRITE_Y_FROM_SCREEN(scaled_sprite.y),
                                   0, MIRROR_X_MASK, SCALE_X_2 | SCALE_Y_2, true);
    set_sprite_slot(4);
    set_extended_sprite_attributes(0,
                                   SPRITE_X_FROM_SCREEN(big_sprite.x),
                                   SPRITE_Y_FROM_SCREEN(big_sprite.y),
                                   0, big_sprite_flags, RELATIVE_SPRITES_UNIFIED_MASK, true);

    // Every 64 frames, toggle the rotation of the unified big sprite by
    // rewriting only the flags attribute byte of the anchor sprite via the
    // Next register interface. Bit 0 of that byte is the MSB of the x coordinate.
    if (++frame_counter == 64)
    {
        frame_counter = 0;
        big_sprite_flags ^= ROTATE_MASK;
        set_sprite_attribute(4, SPRITE_ATTRIBUTE_FLAGS,
                             big_sprite_flags | (SPRITE_X_FROM_SCREEN(big_sprite.x) >> 8));
    }
}

int main(void)
{
    init_hardware();
    init_isr();
    create_background();
    create_sprites();
    set_sprite_layers_system(true, false, LAYER_PRIORITIES_S_L_U, false);

    while (true)
    {
        if (in_inkey() != 0)
        {
            break;
        }

        // Wait for vertical blanking interval.
        intrinsic_halt();

        update_sprites();
    }

    // Trig a soft reset. The Next hardware registers and I/O ports will be reset by NextZXOS after a soft reset.
    ZXN_NEXTREG(REG_RESET, RR_SOFT_RESET);
    return 0;
}
