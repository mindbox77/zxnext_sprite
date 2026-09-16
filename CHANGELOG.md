# Changelog

All notable changes to the C Hardware Sprite API (zxnext_sprite)
for Spectrum Next will be documented in this file.

## 2026-09-16

* Updated the API to the sprite features of the current Spectrum Next core:
  * 128 sprites (set_sprite_slot() now accepts sprite slots 0 - 127).
  * 4-bit sprite patterns: set_sprite_pattern_slot_4bit(), set_sprite_pattern_4bit()
    and load_sprite_patterns_4bit().
  * Extended (5-byte) sprite attributes with scaling, 4-bit patterns and a
    9-bit y coordinate: set_extended_sprite_attributes().
  * Relative sprites (composite and unified): set_relative_sprite_attributes().
  * Sprite clip window: set_sprite_clip_window() and set_sprite_border_clipping().
  * Sprite priority order: set_sprite_priority().
  * Sprite transparency index: set_sprite_transparency_index() and
    get_sprite_transparency_index().
  * Random access to sprite attributes via Next registers: set_sprite_attribute().
  * Layer priority modes LAYER_PRIORITIES_S_U_PLUS_L and
    LAYER_PRIORITIES_S_U_PLUS_L_MINUS_5.
* set_sprite_layers_system() now preserves the sprite priority and border
  clipping bits of Next register 0x15 instead of clearing them.
* Corrected the documented byte order of the RGB333 colours given to
  set_sprite_palette() to match the implementation and the palette file format
  (RGB332 byte first, lowest blue bit second). Added the RGB333_COLOR() and
  RGB333_FROM_RGB332() macros for creating such colours.
* Updated the demo to use scaled, 4-bit, relative and clipped sprites.
* Added a GitHub Actions workflow that builds the library and demo with the
  latest z88dk nightly and attaches the distribution archives to releases.
* Rebuilt the distribution archives with the latest z88dk nightly.
* Updated the documentation for the current Spectrum Next core and emulators.

## 2022-06-06

* Moved the zxnext_sprite_demo repository into this repository's demo folder.

## 2018-05-03

* Added support for the z88dk-lib tool.

## 2018-01-20

* Added macros for getting/setting the global transparency colour.
* Added support for sprite palette with 9-bit colour values.

## 2017-09-18

* Used intrinsic_outi() in set_sprite_pattern.c.
* Added load_sprite_patterns() function.

## 2017-09-04

* Updated set_sprite_layers_system() function.

## 2017-08-17

* Updated set_sprite_pattern() to work in CSpect.

## 2017-07-25

* Changed sprite pattern port from 0x55 to 0x5B, use otir in set_sprite_pattern().

## 2017-07-23

* Updated API with new sprites / layer 2 / ULA layer priorities feature.

## 2017-07-19

* Added missing include guard in header file.

## 2017-07-15

* Used otir when setting palette and declared port 0x57 as an 8-bit port.

## 2017-07-02

* First public release.
