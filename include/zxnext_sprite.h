/*******************************************************************************
 * Stefan Bylund 2017
 *
 * C API for using the hardware sprites of the ZX Spectrum Next as specified at
 * https://wiki.specnext.dev/Sprites. This C API is a thin wrapper on top of the
 * I/O port and Next register interface of the hardware sprites.
 *
 * The Spectrum Next provides 128 hardware sprites numbered from 0 to 127.
 * Each sprite is 16 * 16 pixels and uses either an 8-bit sprite pattern (256
 * bytes, one byte per pixel) or a 4-bit sprite pattern (128 bytes, two pixels
 * per byte). The pixel value is an index into a 256-colour sprite palette. The
 * sprite pixels are laid out linearly from left to right and top to bottom.
 * The 16 KB sprite pattern memory holds 64 8-bit patterns or 128 4-bit patterns
 * or any mix of the two.
 *
 * The sprite palette consists of 256 9-bit RGB333 colour values, i.e. the total
 * number of colours is 512. There are actually two sprite palettes, which one
 * is currently used for displaying the sprites can be selected at runtime. The
 * colour encoding of the sprite palette is the same as for the palette of the
 * ULA and layer 2 screens.
 *
 * At reset, the sprite palette is initialized with the RGB332 colours 0 to 255
 * using a one-to-one mapping between palette indexes and palette colours, i.e.
 * palette index 0 contains colour 0, palette index 1 contains colour 1, ...,
 * palette index 255 contains colour 255. The effective palette colours will be
 * 9-bit RGB333 colours where the lower blue bit is an OR between bit 1 and bit
 * 0 in the 8-bit RGB332 colours.
 *
 * One palette index is defined as the sprite transparency index. A sprite pixel
 * whose (8-bit or 4-bit) pattern value equals the transparency index is not
 * drawn. By default, the sprite transparency index is 0xE3 (227), which with
 * the default palette is the pink colour 0xE3. For 4-bit patterns only the low
 * 4 bits of the transparency index are used, i.e. 0x3 by default. The sprite
 * transparency index is separate from the global transparency colour used by
 * the ULA and layer 2 screens.
 *
 * Tip: If you're drawing your sprites in a general-purpose paint program, it's
 * good to know that the default transparency colour 0xE3 corresponds to the
 * 24-bit RGB colour 0xE000C0 (224, 0, 192).
 *
 * Sprites can optionally be rendered on the border of the screen. The coordinate
 * system of the sprites therefore includes the border, which is 32 pixels, and
 * the total sprite resolution is thus 320 * 256 pixels. The standard screen
 * resolution is 256 * 192 pixels. This means that if sprites are not rendered
 * on the border, the sprite coordinates range from (32, 32) to (287, 223). The
 * x coordinate is 9 bits (0 - 511) and the y coordinate is 8 bits (0 - 255) or,
 * for sprites with extended attributes, 9 bits (0 - 511). Both axes wrap at 512.
 *
 * For convenience, there is an extended version of the set_sprite_attributes()
 * function, for setting the sprite position, called set_sprite_attributes_ext(),
 * which accepts screen-based coordinates (256 * 192 pixels) and internally
 * converts them to border-based coordinates (320 * 256 pixels). This function
 * is convenient if you prefer to work in screen coordinates and don't want to
 * render the sprites on the border area.
 *
 * When using the sprites there is a differentiation between the actual sprites
 * and the sprite pattern (i.e. the sprite bitmap) used by the sprites. The
 * sprite patterns are defined independently of the sprites and are referenced
 * by the sprites. This means that multiple sprites can share the same sprite
 * pattern.
 *
 * The sprite pattern is set for the currently selected sprite pattern slot.
 * The attributes of a sprite is set for the currently selected sprite slot
 * (0 - 127). The sprite attributes determine which sprite pattern the sprite
 * should use, the x and y position of the sprite, an optional sprite palette
 * offset, a bit-mask of flags for sprite mirroring and rotation, and whether or
 * not the sprite should be visible. Sprites with extended attributes can in
 * addition be scaled 2x, 4x or 8x in each direction, use 4-bit patterns, and be
 * grouped as relative sprites under an anchor sprite.
 *
 * If the optional sprite palette offset (0 - 15) is used when setting the
 * attributes of a sprite, it is added to the 4 most significant bits of each
 * 8-bit palette index in the sprite pattern of the sprite. In this way, the
 * 256-colour sprite palette is effectively divided into 16 sub-palettes
 * numbered from 0 to 15 where each sub-palette contains 16 colours indexed from
 * 0 to 15. The palette offset then controls which of the 16 sub-palettes should
 * be used. For example, if a pixel in a sprite pattern contains the palette
 * index 0x14, which denotes the colour at index 4 in sub-palette 1, and the
 * palette offset is 2, the actual palette index used for that pixel will be
 * 0x34, which denotes the colour at index 4 in sub-palette 3 (sub-palette 1 +
 * palette offset 2). When the palette offset is added to a sub-palette number,
 * the addition is actually done in modulo 16. For example, adding palette
 * offset 5 to sub-palette number 13 gives sub-palette number 2. If used, the
 * palette offset is an efficient way of displaying the same sprite pattern in
 * different colours. For 4-bit patterns, the 4-bit pixel value is the index
 * within the sub-palette and the palette offset selects the sub-palette.
 *
 * The priority between the sprites is determined by the sprite slot number.
 * By default, sprite 0 has the lowest priority and sprite 127 has the highest
 * priority, i.e. a sprite with a higher slot number is drawn over a sprite
 * with a lower slot number. This order can be flipped with set_sprite_priority().
 * The layer priority between the sprites and the layer 2 and ULA screens is
 * configurable, the default priority is sprites over layer 2 screen over ULA
 * screen.
 *
 * The sprite system provides collision detection of the sprites. A collision of
 * two or more sprites happen if a non-transparent pixel of the sprites are drawn
 * in the same position on the screen. The sprite system only informs whether a
 * sprite collision has occurred or not, which sprites has actually collided must
 * be determined in software.
 *
 * All functions in this API that write to the sprite system hardware write to
 * the ZX Spectrum Next I/O ports 0x303B, 0x57 and 0x5B or the Next registers
 * 0x15, 0x19, 0x1C, 0x34-0x39, 0x40-0x44 and 0x4B.
 ******************************************************************************/

#ifndef _ZXNEXT_SPRITE_H
#define _ZXNEXT_SPRITE_H

#include <arch/zxn.h>
#include <stdint.h>
#include <stdbool.h>

/* Number of hardware sprites (numbered 0 - 127). */
#define MAX_SPRITES 128

/* Number of 8-bit sprite patterns (numbered 0 - 63), 256 bytes each. */
#define MAX_SPRITE_PATTERNS 64

/* Number of 4-bit sprite patterns (numbered 0 - 127), 128 bytes each. */
#define MAX_SPRITE_PATTERNS_4BIT 128

/* Size in bytes of an 8-bit sprite pattern. */
#define SPRITE_PATTERN_SIZE 256

/* Size in bytes of a 4-bit sprite pattern. */
#define SPRITE_PATTERN_SIZE_4BIT 128

/* Default sprite transparency index (and RGB332 colour with the default palette). */
#define SPRITE_TRANSPARENCY_INDEX_DEFAULT 0xE3

/*
 * Create an RGB333 sprite palette colour from its 3-bit red, green and blue
 * components (0 - 7), in the format expected by set_sprite_palette().
 */
#define RGB333_COLOR(r, g, b) ((uint16_t) (((((b) & 0x01)) << 8) | (((r) & 0x07) << 5) | (((g) & 0x07) << 2) | (((b) >> 1) & 0x03)))

/*
 * Create an RGB333 sprite palette colour from an RGB332 colour (RRRGGGBB), in
 * the format expected by set_sprite_palette(). The lowest blue bit is set to
 * the OR of the two RGB332 blue bits, as in the default sprite palette.
 */
#define RGB333_FROM_RGB332(c) ((uint16_t) (((((c) >> 1) | (c)) & 0x01) << 8) | ((c) & 0xFF))

/* Convert a screen-based x coordinate (256 * 192) to a border-based (320 * 256). */
#define SPRITE_X_FROM_SCREEN(x) (((uint16_t) (x)) + 32)

/* Convert a screen-based y coordinate (256 * 192) to a border-based (320 * 256). */
#define SPRITE_Y_FROM_SCREEN(y) (((uint16_t) (y)) + 32)

/* Sprites over layer 2 screen over ULA screen (default). */
#define LAYER_PRIORITIES_S_L_U 0x0

/* Layer 2 screen over sprites over ULA screen. */
#define LAYER_PRIORITIES_L_S_U 0x1

/* Sprites over ULA screen over layer 2 screen. */
#define LAYER_PRIORITIES_S_U_L 0x2

/* Layer 2 screen over ULA screen over sprites. */
#define LAYER_PRIORITIES_L_U_S 0x3

/* ULA screen over sprites over layer 2 screen. */
#define LAYER_PRIORITIES_U_S_L 0x4

/* ULA screen over layer 2 screen over sprites. */
#define LAYER_PRIORITIES_U_L_S 0x5

/*
 * Sprites over a blend of the ULA and layer 2 screens, where the RGB colour
 * channels of the two screens are added and clamped to 7 (core 3.0+).
 */
#define LAYER_PRIORITIES_S_U_PLUS_L 0x6

/*
 * Sprites over a blend of the ULA and layer 2 screens, where the RGB colour
 * channels of the two screens are added, 5 is subtracted, and the result is
 * clamped to 0 - 7 (core 3.0+).
 */
#define LAYER_PRIORITIES_S_U_PLUS_L_MINUS_5 0x7

/* Max sprites per scanline limit reached. */
#define MAX_SPRITES_PER_SCANLINE_MASK 0x02

/* A collision between two or more sprites. */
#define SPRITE_COLLISION_MASK 0x01

/*
 * Sprite flags used in the sprite_flags parameter of the set_sprite_attributes()
 * family of functions. Rotation is applied before mirroring.
 */

/* Mirror the sprite x-wise. */
#define MIRROR_X_MASK 0x08

/* Mirror the sprite y-wise. */
#define MIRROR_Y_MASK 0x04

/* Rotate the sprite 90 degrees clockwise. */
#define ROTATE_MASK 0x02

/*
 * Relative sprites only: add the anchor sprite's palette offset to this
 * sprite's palette offset.
 */
#define RELATIVE_PALETTE_OFFSET_MASK 0x01

/*
 * Extended sprite flags used in the sprite_flags_ext parameter of the
 * set_extended_sprite_attributes() and set_relative_sprite_attributes()
 * functions.
 */

/*
 * The sprite uses a 4-bit sprite pattern. When this flag is set, the pattern
 * slot given to the function is a 4-bit pattern number (0 - 127), otherwise
 * it is an 8-bit pattern number (0 - 63). For relative sprites, whether 4-bit
 * patterns are used is determined by the anchor sprite; the flag only tells
 * the function how to interpret the pattern slot.
 */
#define SPRITE_4BIT_MASK 0x80

/*
 * Anchor sprites only: the relative sprites following this anchor sprite are
 * unified (big sprite) instead of composite. Unified relative sprites are
 * rotated, mirrored and scaled together with their anchor as if they were a
 * single big sprite.
 */
#define RELATIVE_SPRITES_UNIFIED_MASK 0x20

/* Horizontal scaling. */
#define SCALE_X_1 0x00
#define SCALE_X_2 0x08
#define SCALE_X_4 0x10
#define SCALE_X_8 0x18
#define SCALE_X_MASK 0x18

/* Vertical scaling. */
#define SCALE_Y_1 0x00
#define SCALE_Y_2 0x02
#define SCALE_Y_4 0x04
#define SCALE_Y_8 0x06
#define SCALE_Y_MASK 0x06

/*
 * Relative sprites only: add the anchor sprite's pattern number to this
 * sprite's pattern number. This makes it possible to animate all sprites in a
 * group by only changing the pattern number of the anchor sprite.
 */
#define RELATIVE_PATTERN_OFFSET_MASK 0x01

/* Sprite attribute indexes used by set_sprite_attribute(). */
#define SPRITE_ATTRIBUTE_X       0 /* Bits 7-0 of the x coordinate. */
#define SPRITE_ATTRIBUTE_Y       1 /* Bits 7-0 of the y coordinate. */
#define SPRITE_ATTRIBUTE_FLAGS   2 /* Palette offset, sprite flags and x bit 8. */
#define SPRITE_ATTRIBUTE_PATTERN 3 /* Visible, extended and pattern number. */
#define SPRITE_ATTRIBUTE_EXT     4 /* Extended sprite flags and y bit 8. */

/*
 * Set the sprite and layers system properties. Specify if the sprites should be
 * visible, if they should be rendered on the border of the screen, the layer
 * priority between the sprites and the layer 2 and ULA screens
 * (LAYER_PRIORITIES_S_L_U etc), and if the low-resolution ULA screen mode
 * should be enabled.
 *
 * The sprite priority order and the sprite clipping in over border mode, set
 * with set_sprite_priority() and set_sprite_border_clipping(), are preserved.
 */
void set_sprite_layers_system(bool sprites_visible,
                              bool sprites_on_border,
                              uint8_t layer_priorities,
                              bool lores_mode);

/*
 * Set the priority order between the sprites. By default, sprite 127 is drawn
 * on top of all other sprites and sprite 0 is drawn below all other sprites.
 * If sprite_0_on_top is true, the order is flipped so that sprite 0 is drawn on
 * top of all other sprites.
 */
void set_sprite_priority(bool sprite_0_on_top);

/*
 * Enable or disable the sprite clip window when the sprites are rendered on the
 * border of the screen. When the sprites are not rendered on the border, the
 * clip window is always active. When the sprites are rendered on the border,
 * the clip window is by default not active and the sprites are visible on the
 * whole 320 * 256 area; this function enables the clip window also in that mode,
 * where the x coordinates of the clip window are doubled and its origin moved
 * to the top-left corner of the border (see set_sprite_clip_window()).
 */
void set_sprite_border_clipping(bool enable);

/*
 * Set the sprite clip window. The sprites are only visible inside the clip
 * window. The coordinates are inclusive, i.e. the default clip window of
 * (0, 255, 0, 191) covers the full screen.
 *
 * When the sprites are not rendered on the border, the clip window is given in
 * screen coordinates (256 * 192) with (0, 0) being the top-left corner of the
 * screen. When the sprites are rendered on the border and clipping is enabled
 * with set_sprite_border_clipping(), the x coordinates are doubled internally
 * and the origin is moved to the top-left corner of the border, i.e. the clip
 * window (0, 159, 0, 255) covers the full 320 * 256 sprite area.
 */
void set_sprite_clip_window(uint8_t x1, uint8_t x2, uint8_t y1, uint8_t y2);

/*
 * Returns the state of the sprite system as a bit-mask informing if the maximum
 * number of sprites per scanline limit has been reached (MAX_SPRITES_PER_SCANLINE_MASK)
 * and if two or more sprites has collided (SPRITE_COLLISION_MASK). Reading the
 * state resets both flags.
 */
uint8_t get_sprite_system_state(void);

/*
 * Set the sprite transparency index. A sprite pattern pixel with this palette
 * index is transparent. For 4-bit sprite patterns, only the low 4 bits of the
 * index are used. The default transparency index is 0xE3.
 */
#define set_sprite_transparency_index(index) ZXN_WRITE_REG(REG_SPRITE_TRANSPARENCY_INDEX, index)

/*
 * Returns the sprite transparency index. The default transparency index is 0xE3.
 */
#define get_sprite_transparency_index() ZXN_READ_REG(REG_SPRITE_TRANSPARENCY_INDEX)

/*
 * Set the global RGB332 colour used for transparency in the ULA and layer 2
 * screens. The default transparency colour is 0xE3. Note that the sprites use
 * the sprite transparency index instead, see set_sprite_transparency_index().
 */
#define set_global_transparency_color(color) ZXN_WRITE_REG(REG_GLOBAL_TRANSPARENCY_COLOR, color)

/*
 * Returns the global RGB332 colour used for transparency in the ULA and layer 2
 * screens. The default transparency colour is 0xE3.
 */
#define get_global_transparency_color() ZXN_READ_REG(REG_GLOBAL_TRANSPARENCY_COLOR)

/*
 * Set the sprite display palette (first or second palette), i.e. the palette
 * used when displaying the sprites. By default, the first palette is used.
 */
void set_sprite_display_palette(bool first_palette);

/*
 * Set the sprite read/write palette (first or second palette), i.e. the palette
 * used when reading/writing the sprite palette colours.
 */
void set_sprite_rw_palette(bool first_palette);

/*
 * Set a range of RGB333 colours starting from the given palette index in the
 * currently selected sprite read/write palette.
 *
 * The RGB333 colours are 16-bit values stored in the same byte order as in a
 * sprite palette file: the first (low) byte holds the RGB332 bits (RRRGGGBB)
 * and the second (high) byte the zero-extended lowest blue bit (0000000B).
 * Use the RGB333_COLOR() or RGB333_FROM_RGB332() macros to create the values.
 */
void set_sprite_palette(const uint16_t *colors, uint16_t length, uint8_t palette_index);

/*
 * Reset the currently selected sprite read/write palette to contain the default
 * sprite palette colours.
 */
void reset_sprite_palette(void);

/*
 * Set the sprite slot used for the set_sprite_pattern() and set_sprite_attributes()
 * family of functions. The sprite slot is a number between 0 and 127. For
 * set_sprite_pattern(), the low 6 bits select one of the 64 8-bit sprite
 * pattern slots (0 - 63).
 *
 * To upload a 4-bit sprite pattern, use set_sprite_pattern_slot_4bit() instead.
 */
void set_sprite_slot(uint8_t sprite_slot);

/*
 * Set the 4-bit sprite pattern slot (0 - 127) used for the set_sprite_pattern_4bit()
 * function. Each 8-bit sprite pattern slot holds two 4-bit sprite pattern slots:
 * 4-bit pattern 2n is the first half of 8-bit pattern n and 4-bit pattern 2n+1
 * is the second half.
 *
 * Note that this function also selects sprite slot (pattern_slot / 2) for the
 * set_sprite_attributes() family of functions.
 */
void set_sprite_pattern_slot_4bit(uint8_t pattern_slot);

/*
 * Set the sprite pattern data for the selected sprite pattern slot. The sprite
 * pattern data should be 16 * 16 pixels where each pixel is an 8-bit index
 * between 0 and 255 into the 256-colour sprite palette. The pixels are laid out
 * left to right and top to bottom.
 *
 * After each call to set_sprite_pattern(), the current sprite pattern slot is
 * automatically incremented.
 */
void set_sprite_pattern(const void *sprite_pattern);

/*
 * Set the 4-bit sprite pattern data for the selected 4-bit sprite pattern slot,
 * see set_sprite_pattern_slot_4bit(). The sprite pattern data should be 16 * 16
 * pixels where each pixel is a 4-bit index between 0 and 15 into a 16-colour
 * sub-palette of the sprite palette. Two pixels are stored in each byte, the
 * left pixel in the high nibble and the right pixel in the low nibble. The
 * pixels are laid out left to right and top to bottom, 128 bytes in total.
 *
 * After each call to set_sprite_pattern_4bit(), the current 4-bit sprite
 * pattern slot is automatically incremented.
 */
void set_sprite_pattern_4bit(const void *sprite_pattern);

/*
 * Set the sprite attributes for the selected sprite slot. The given coordinates
 * are border-based (320 * 256 pixels).
 *
 * The sprite attributes determine which sprite pattern the sprite should use,
 * the x and y position of the sprite, an optional sprite palette offset
 * (0 - 15, set to 0 if not used), a bit-mask of sprite flags (MIRROR_X_MASK,
 * MIRROR_Y_MASK, ROTATE_MASK) or 0 if not set, and whether or not the sprite
 * should be visible.
 *
 * The sprite is an anchor sprite with 8-bit sprite pattern (0 - 63) without
 * extended attributes, i.e. no scaling. It is compatible with all Spectrum
 * Next core versions.
 *
 * After each call to set_sprite_attributes(), the current sprite slot is
 * automatically incremented.
 */
void set_sprite_attributes(uint8_t sprite_pattern_slot,
                           uint16_t x,
                           uint8_t y,
                           uint8_t palette_offset,
                           uint8_t sprite_flags,
                           bool visible);

/*
 * Set the sprite attributes for the selected sprite slot. The given coordinates
 * are screen-based (256 * 192 pixels) and internally converted to border-based
 * coordinates (320 * 256 pixels). This function is convenient if you prefer to
 * work in screen coordinates and don't want to render the sprites on the border
 * area.
 *
 * The sprite attributes determine which sprite pattern the sprite should use,
 * the x and y position of the sprite, an optional sprite palette offset
 * (0 - 15, set to 0 if not used), a bit-mask of sprite flags (MIRROR_X_MASK,
 * MIRROR_Y_MASK, ROTATE_MASK) or 0 if not set, and whether or not the sprite
 * should be visible.
 *
 * After each call to set_sprite_attributes_ext(), the current sprite slot is
 * automatically incremented.
 */
void set_sprite_attributes_ext(uint8_t sprite_pattern_slot,
                               uint8_t x,
                               uint8_t y,
                               uint8_t palette_offset,
                               uint8_t sprite_flags,
                               bool visible);

/*
 * Set the extended sprite attributes for the selected sprite slot. The given
 * coordinates are border-based (320 * 256 pixels) and both are 9 bits (0 - 511).
 *
 * The sprite attributes determine which sprite pattern the sprite should use,
 * the x and y position of the sprite, an optional sprite palette offset
 * (0 - 15, set to 0 if not used), a bit-mask of sprite flags (MIRROR_X_MASK,
 * MIRROR_Y_MASK, ROTATE_MASK) or 0 if not set, a bit-mask of extended sprite
 * flags (SPRITE_4BIT_MASK, RELATIVE_SPRITES_UNIFIED_MASK, SCALE_X_2 etc,
 * SCALE_Y_2 etc) or 0 if not set, and whether or not the sprite should be
 * visible.
 *
 * If SPRITE_4BIT_MASK is set, the sprite uses a 4-bit sprite pattern and the
 * sprite pattern slot is a 4-bit pattern number (0 - 127), otherwise the sprite
 * uses an 8-bit sprite pattern and the sprite pattern slot is an 8-bit pattern
 * number (0 - 63).
 *
 * The sprite is an anchor sprite. Any relative sprites in the sprite slots
 * following it, up to the next anchor sprite, are grouped with this sprite.
 * If RELATIVE_SPRITES_UNIFIED_MASK is set, they form a unified big sprite,
 * otherwise a composite sprite, see set_relative_sprite_attributes().
 *
 * Extended sprite attributes require Spectrum Next core 2.00.26 or later.
 *
 * After each call to set_extended_sprite_attributes(), the current sprite slot
 * is automatically incremented.
 */
void set_extended_sprite_attributes(uint8_t sprite_pattern_slot,
                                    uint16_t x,
                                    uint16_t y,
                                    uint8_t palette_offset,
                                    uint8_t sprite_flags,
                                    uint8_t sprite_flags_ext,
                                    bool visible);

/*
 * Set the attributes of a relative sprite for the selected sprite slot. A
 * relative sprite is grouped with the closest preceding anchor sprite (set with
 * set_extended_sprite_attributes()) and the given coordinates are signed
 * offsets (-128 to 127) from the position of the anchor sprite. Moving the
 * anchor sprite moves all its relative sprites with it. A relative sprite is
 * only visible if both it and its anchor sprite are visible.
 *
 * The sprite attributes determine which sprite pattern the sprite should use,
 * the x and y offset of the sprite from the anchor sprite, an optional sprite
 * palette offset (0 - 15, set to 0 if not used), a bit-mask of sprite flags
 * (MIRROR_X_MASK, MIRROR_Y_MASK, ROTATE_MASK, RELATIVE_PALETTE_OFFSET_MASK) or
 * 0 if not set, a bit-mask of extended sprite flags (SPRITE_4BIT_MASK,
 * RELATIVE_PATTERN_OFFSET_MASK, SCALE_X_2 etc, SCALE_Y_2 etc) or 0 if not set,
 * and whether or not the sprite should be visible.
 *
 * Whether the sprite uses a 4-bit or 8-bit sprite pattern is determined by the
 * anchor sprite. SPRITE_4BIT_MASK must be set if the anchor sprite uses 4-bit
 * patterns, in which case the sprite pattern slot is a 4-bit pattern number
 * (0 - 127), otherwise it is an 8-bit pattern number (0 - 63).
 *
 * If the anchor sprite is a composite sprite, the relative sprite is scaled
 * according to its own SCALE_X and SCALE_Y flags, while its rotation and
 * mirroring are independent of the anchor sprite. If the anchor sprite is a
 * unified sprite, the scaling, rotation and mirroring flags of the anchor
 * sprite are applied to the whole group as if it was a single big sprite, and
 * the scaling flags of the relative sprite should be 0.
 *
 * Relative sprites require Spectrum Next core 2.00.26 or later.
 *
 * After each call to set_relative_sprite_attributes(), the current sprite slot
 * is automatically incremented.
 */
void set_relative_sprite_attributes(uint8_t sprite_pattern_slot,
                                    int8_t x,
                                    int8_t y,
                                    uint8_t palette_offset,
                                    uint8_t sprite_flags,
                                    uint8_t sprite_flags_ext,
                                    bool visible);

/*
 * Set a single attribute byte (SPRITE_ATTRIBUTE_X, SPRITE_ATTRIBUTE_Y,
 * SPRITE_ATTRIBUTE_FLAGS, SPRITE_ATTRIBUTE_PATTERN or SPRITE_ATTRIBUTE_EXT) of
 * the given sprite (0 - 127) using the Next register interface of the sprite
 * system. This gives random access to the sprite attributes without going
 * through the sprite slot selection and sequential attribute upload of the
 * set_sprite_attributes() family of functions, e.g. for changing the pattern
 * number of a sprite when animating it or for updating the low byte of its
 * position.
 *
 * The attribute byte layout is the same as described for the I/O port
 * interface at https://wiki.specnext.dev/Sprites.
 *
 * By default, the Next register interface is decoupled from the sprite slot
 * selected with set_sprite_slot(). If the sprite id lockstep mode is enabled
 * (Next register 0x09 bit 4), this function also changes the selected sprite
 * slot.
 *
 * The Next register interface to the sprite attributes requires Spectrum Next
 * core 3.0 or later.
 */
void set_sprite_attribute(uint8_t sprite_slot, uint8_t attribute_index, uint8_t value);

/*
 * Load the specified number of 8-bit sprite patterns (max 64) from the
 * specified sprite pattern file using ESXDOS into the sprite pattern slots
 * starting from the specified slot. Each loaded sprite pattern is temporarily
 * stored in the given 256 bytes long buffer.
 *
 * The sprite pattern file should contain a contiguous set of 16 * 16 pixel
 * 8-bit sprite patterns.
 *
 * If there is any error when loading the file, errno is set with the
 * corresponding ESXDOS error code.
 */
void load_sprite_patterns(const char *filename,
                          const void *sprite_pattern_buf,
                          uint8_t num_sprite_patterns,
                          uint8_t start_sprite_pattern_slot);

/*
 * Load the specified number of 4-bit sprite patterns (max 128) from the
 * specified sprite pattern file using ESXDOS into the 4-bit sprite pattern
 * slots starting from the specified slot. Each loaded sprite pattern is
 * temporarily stored in the given 128 bytes long buffer.
 *
 * The sprite pattern file should contain a contiguous set of 16 * 16 pixel
 * 4-bit sprite patterns.
 *
 * If there is any error when loading the file, errno is set with the
 * corresponding ESXDOS error code.
 */
void load_sprite_patterns_4bit(const char *filename,
                               const void *sprite_pattern_buf,
                               uint8_t num_sprite_patterns,
                               uint8_t start_sprite_pattern_slot);

/*
 * Load the specified sprite palette file (containing 256 RGB333 colours, 512
 * bytes in size) using ESXDOS into the currently selected sprite read/write
 * palette. The loaded sprite palette is temporarily stored in the given 256
 * bytes long buffer.
 *
 * The RGB333 colours should be 16-bit values where the first byte holds the
 * RGB332 bits (RRRGGGBB) and the second byte the zero-extended lowest blue bit
 * (0000000B).
 *
 * If there is any error when loading the file, errno is set with the
 * corresponding ESXDOS error code.
 */
void load_sprite_palette(const char *filename, const void *sprite_palette_buf);

#endif
