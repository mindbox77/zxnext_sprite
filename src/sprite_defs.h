/*******************************************************************************
 * Stefan Bylund 2017
 *
 * Common definitions used by the implementation of zxnext_sprite.h.
 ******************************************************************************/

#ifndef _SPRITE_DEFS_H
#define _SPRITE_DEFS_H

/* Port 0x303B: sprite slot (bits 6-0) and 4-bit pattern half select (bit 7). */
#define SPRITE_SLOT_MASK          0x7F
#define PATTERN_HALF_MASK         0x80

/* Attribute byte 3: visible (bit 7), extended (bit 6), pattern (bits 5-0). */
#define SPRITE_PATTERN_SLOT_MASK  0x3F
#define SPRITE_VISIBLE_MASK       0x80
#define SPRITE_EXTENDED_MASK      0x40

/* Attribute byte 2: palette offset (bits 7-4), flags (bits 3-1), X MSB (bit 0). */
#define PALETTE_OFFSET_SHIFT      4
#define SPRITE_FLAGS_MASK         0x0E
#define X_LSB_MASK                0x00FF
#define X_MSB_MASK                0x0100
#define X_MSB_SHIFT               8

/* Attribute byte 4 (anchor): H (7), N6 (6), T (5), XX (4-3), YY (2-1), Y MSB (0). */
#define ATTR4_N6_ANCHOR_MASK      0x40
#define ATTR4_ANCHOR_FLAGS_MASK   0xBE
#define Y_LSB_MASK                0x00FF
#define Y_MSB_MASK                0x0100
#define Y_MSB_SHIFT               8

/* Attribute byte 4 (relative): 01 (7-6), N6 (5), XX (4-3), YY (2-1), PO (0). */
#define ATTR4_RELATIVE_MASK       0x40
#define ATTR4_N6_RELATIVE_MASK    0x20
#define ATTR4_RELATIVE_FLAGS_MASK 0x1F

/* A 7-bit 4-bit pattern number is N5..N0,N6 with N6 the least significant bit. */
#define PATTERN_4BIT_N6_MASK      0x01
#define PATTERN_4BIT_N_SHIFT      1

/* Next registers for the sprite attribute nextreg interface. */
#ifndef REG_SPRITE_NUMBER
#define REG_SPRITE_NUMBER         0x34
#endif
#ifndef REG_SPRITE_ATTRIBUTE_0
#define REG_SPRITE_ATTRIBUTE_0    0x35
#endif

/* Next register 0x15 bits that set_sprite_layers_system() preserves. */
#define RSLS_SPRITE_PRIORITY_MASK 0x40
#define RSLS_CLIP_OVER_BORDER_MASK 0x20
#define RSLS_PRESERVED_MASK       (RSLS_SPRITE_PRIORITY_MASK | RSLS_CLIP_OVER_BORDER_MASK)

#endif
