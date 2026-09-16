# C Hardware Sprite API for Spectrum Next

The **zxnext_sprite** project provides a C API for using the hardware sprites of
ZX Spectrum Next as specified at https://wiki.specnext.dev/Sprites. This API is
a thin C wrapper on top of the I/O port and Next register interface of the
hardware sprite system.

The API covers the sprite features of the current Spectrum Next core (3.x):

* 128 sprites with 8-bit (256 colour) or 4-bit (16 colour) patterns
* Sprite scaling 2x, 4x and 8x in each direction, mirroring and rotation
* Relative sprites grouped under an anchor sprite (composite and unified)
* Sprite clip window, sprite priority order and sprite transparency index
* Random access to individual sprite attributes via Next registers
* Two 9-bit RGB333 sprite palettes
* Loading of sprite patterns and palettes from files using ESXDOS

The original 4-byte sprite attribute functions are still available and work on
all core versions.

The [demo](demo) folder contains a simple example program demonstrating how to
use this API.

## Download

The latest version of this API can be downloaded here:

* [zxnext_sprite.zip](build/zxnext_sprite.zip)
* [zxnext_sprite_z88dk.zip](build/zxnext_sprite_z88dk.zip)

The zxnext_sprite.zip archive contains the following header file and libraries:

* zxnext_sprite/include/zxnext_sprite.h
* zxnext_sprite/lib/sccz80/zxnext_sprite.lib
* zxnext_sprite/lib/sdcc_ix/zxnext_sprite.lib
* zxnext_sprite/lib/sdcc_iy/zxnext_sprite.lib

The zxnext_sprite_z88dk.zip archive contains a packaging of zxnext_sprite that
can be installed directly into your z88dk installation for convenience, see the
tip below.

**Note:** A z88dk library can only be linked with the z88dk version it was built
with (or a compatible one). The archives above are built with the latest z88dk
nightly at the time of the release. If the linker complains about the library
version, rebuild the library with your own z88dk installation as described in
the "How to Build" section below. The libraries are also built by the GitHub
Actions workflow of this repository and are available as workflow artifacts
and release assets.

## API Documentation

The zxnext_sprite API is documented in the following header file:

* [zxnext_sprite.h](include/zxnext_sprite.h)

## How to Use

1. Download [zxnext_sprite.zip](build/zxnext_sprite.zip) and unpack it in a
suitable place. It contains the files listed in the "Download" section above.
For convenience, you can instead download
[zxnext_sprite_z88dk.zip](build/zxnext_sprite_z88dk.zip) and install it into
your z88dk installation, see the tip below.

2. Install the latest version of [z88dk](https://github.com/z88dk/z88dk) and
the [ZEsarUX](https://github.com/chernandezba/zesarux/releases) or
[CSpect](https://mdf200.itch.io/cspect) emulator.

3. Read about how hardware sprites work in the "Hardware Sprites" section below
or in the official specification at https://wiki.specnext.dev/Sprites.

4. Familiarize yourself with the zxnext_sprite.h API.

5. Include zxnext_sprite.h in your program and start creating sprites.

6. Compile your program with z88dk and link it with the appropriate version of
zxnext_sprite.lib.

7. Run your program in the ZEsarUX or CSpect emulator or on a real Spectrum Next.

**Tip:** See the [demo](demo) folder for a simple example of how to use
zxnext_sprite.h and link with zxnext_sprite.lib.

**Tip:** You can install zxnext_sprite into your z88dk installation by using
its third-party library installer z88dk-lib. Unpack the zxnext_sprite_z88dk.zip
archive in a temporary directory, go to this directory (where the unpacked
zxnext_sprite subdirectory is located) and enter the following command:

> z88dk-lib +zxn -f zxnext_sprite

The -f option will make z88dk-lib overwrite any existing files without
confirmation (e.g. if you update zxnext_sprite it will replace the older version).
Run z88dk-lib without any arguments to see a list of all its options.

The z88dk compiler will now automatically find the zxnext_sprite header file and
library without the need for setting up any include and library paths. The
zxnext_sprite.h header file is now included with **#include <lib/zxn/zxnext_sprite.h>**
and the zxnext_sprite.lib library is linked against using **-llib/zxn/zxnext_sprite**.

**Tip:** To start the ZEsarUX emulator directly in Spectrum Next mode, start it
with the following options:

> zesarux --noconfigfile --machine tbblue --enabletimexvideo --tbblue-fast-boot-mode
  --quickexit --enable-esxdos-handler --esxdos-root-dir <virtual_mmc_root_folder>
  <my_program>.nex

**Tip:** To start the CSpect emulator directly in Spectrum Next mode, start it
with the following options:

> CSpect -w2 -tv -zxnext -mmc=<virtual_mmc_root_folder>/ <my_program>.nex

## How to Build

If you want to build the zxnext_sprite libraries yourself, follow the steps below:

1. On Windows, you need [MinGW](http://www.mingw.org/),
[UnxUtils](https://sourceforge.net/projects/unxutils/) or
[Cygwin](https://www.cygwin.com/) for the basic Unix commands. Add the chosen
set of Unix commands to your path.

2. Install the latest version of [z88dk](https://github.com/z88dk/z88dk) and add
it to your path. Prebuilt nightly builds for Windows, macOS and Linux are
available at http://nightly.z88dk.org/.

3. Download the zxnext_sprite repository either as a ZIP archive using the
"Code" button at the top of this page or with Git using the following command:

> git clone https://github.com/stefanbylund/zxnext_sprite.git

4. Go to the zxnext_sprite repository and enter the following command:

> make all

The libraries are created in the lib/sccz80, lib/sdcc_ix and lib/sdcc_iy
directories. Enter **make distro** to create the distribution archives in the
build directory.

The GitHub Actions workflow in
[.github/workflows/build.yml](.github/workflows/build.yml) builds the library
and demo with the latest z88dk nightly on every push and pull request.

## Hardware Sprites

The Spectrum Next provides 128 hardware sprites numbered from 0 to 127.
Each sprite is 16 * 16 pixels and uses either an 8-bit sprite pattern (256
bytes, one byte per pixel) or a 4-bit sprite pattern (128 bytes, two pixels
per byte). The pixel value is an index into a 256-colour sprite palette. The
sprite pixels are laid out linearly from left to right and top to bottom. The
16 KB sprite pattern memory holds 64 8-bit patterns or 128 4-bit patterns or
any mix of the two; 4-bit pattern 2n is the first half of 8-bit pattern n and
4-bit pattern 2n+1 is the second half.

The sprite palette consists of 256 9-bit RGB333 colour values, i.e. the total
number of colours is 512. There are actually two sprite palettes, which one
is currently used for displaying the sprites can be selected at runtime. The
colour encoding of the sprite palette is the same as for the palette of the
ULA and layer 2 screens.

At reset, the sprite palette is initialized with the RGB332 colours 0 to 255
using a one-to-one mapping between palette indexes and palette colours, i.e.
palette index 0 contains colour 0, palette index 1 contains colour 1, ...,
palette index 255 contains colour 255. The effective palette colours will be
9-bit RGB333 colours where the lower blue bit is an OR between bit 1 and bit
0 in the 8-bit RGB332 colours.

One palette index is defined as the sprite transparency index. A sprite pixel
whose pattern value equals the transparency index is not drawn. By default,
the sprite transparency index is 0xE3 (227), which with the default palette is
the pink colour 0xE3. For 4-bit patterns only the low 4 bits of the
transparency index are used, i.e. 0x3 by default. The sprite transparency
index is separate from the global transparency colour used by the ULA and
layer 2 screens.

Tip: If you're drawing your sprites in a general-purpose paint program, it's
good to know that the default transparency colour 0xE3 corresponds to the
24-bit RGB colour 0xE000C0 (224, 0, 192).

Sprites can optionally be rendered on the border of the screen. The coordinate
system of the sprites therefore includes the border, which is 32 pixels, and
the total sprite resolution is thus 320 * 256 pixels. The standard screen
resolution is 256 * 192 pixels. This means that if sprites are not rendered on
the border, the sprite coordinates range from (32, 32) to (287, 223). The x
coordinate is 9 bits (0 - 511) and the y coordinate is 8 bits (0 - 255) or,
for sprites with extended attributes, 9 bits (0 - 511). Both axes wrap at 512.
A clip window can be set to limit the area where the sprites are visible.

For convenience, there is an extended version of the set_sprite_attributes()
function, for setting the sprite position, called set_sprite_attributes_ext(),
which accepts screen-based coordinates (256 * 192 pixels) and internally
converts them to border-based coordinates (320 * 256 pixels). This function
is convenient if you prefer to work in screen coordinates and don't want to
render the sprites on the border area.

When using the sprites there is a differentiation between the actual sprites
and the sprite pattern (i.e. the sprite bitmap) used by the sprites. The
sprite patterns are defined independently of the sprites and are referenced
by the sprites. This means that multiple sprites can share the same sprite
pattern.

The sprite pattern is set for the currently selected sprite pattern slot.
The attributes of a sprite is set for the currently selected sprite slot
(0 - 127). The sprite attributes determine which sprite pattern the sprite
should use, the x and y position of the sprite, an optional sprite palette
offset, a bit-mask of flags for sprite mirroring and rotation, and whether or
not the sprite should be visible.

A sprite is described by either 4 or 5 attribute bytes. The original 4-byte
attributes, set with set_sprite_attributes(), describe an unscaled sprite with
an 8-bit pattern and work on all core versions. The 5-byte extended attributes,
set with set_extended_sprite_attributes() and set_relative_sprite_attributes(),
add scaling by 2x, 4x or 8x in each direction, 4-bit patterns, a 9-bit y
coordinate and relative sprites.

A relative sprite is grouped with the closest preceding anchor sprite (a sprite
with extended attributes that is not itself relative) and its coordinates are
signed offsets from the anchor sprite's position, so moving the anchor moves
the whole group. The anchor decides whether the group is composite, where each
relative sprite keeps its own scaling, mirroring and rotation, or unified, where
the group is scaled, mirrored and rotated as if it was a single big sprite. A
relative sprite can optionally add the anchor's palette offset and pattern
number to its own, which makes it easy to recolour or animate the whole group
by changing only the anchor.

If the optional sprite palette offset (0 - 15) is used when setting the
attributes of a sprite, it is added to the 4 most significant bits of each
8-bit palette index in the sprite pattern of the sprite. In this way, the
256-colour sprite palette is effectively divided into 16 sub-palettes
numbered from 0 to 15 where each sub-palette contains 16 colours indexed from
0 to 15. The palette offset then controls which of the 16 sub-palettes should
be used. For example, if a pixel in a sprite pattern contains the palette
index 0x14, which denotes the colour at index 4 in sub-palette 1, and the
palette offset is 2, the actual palette index used for that pixel will be
0x34, which denotes the colour at index 4 in sub-palette 3 (sub-palette 1 +
palette offset 2). When the palette offset is added to a sub-palette number,
the addition is actually done in modulo 16. For example, adding palette
offset 5 to sub-palette number 13 gives sub-palette number 2. If used, the
palette offset is an efficient way of displaying the same sprite pattern in
different colours. For 4-bit patterns, the 4-bit pixel value is the index
within the sub-palette and the palette offset selects the sub-palette.

The priority between the sprites is determined by the sprite slot number.
By default, sprite 0 has the lowest priority and sprite 127 has the highest
priority, i.e. a sprite with a higher slot number is drawn over a sprite with
a lower slot number. This order can be flipped with set_sprite_priority().
The layer priority between the sprites and the layer 2 and ULA screens is
configurable, the default priority is sprites over layer 2 screen over ULA
screen.

The sprite system provides collision detection of the sprites. A collision of
two or more sprites happen if a non-transparent pixel of the sprites are drawn
in the same position on the screen. The sprite system only informs whether a
sprite collision has occurred or not, which sprites has actually collided must
be determined in software.

The sprite hardware can draw at least 100 unscaled sprites per scanline. If
that limit is exceeded, the remaining sprites on that scanline are not drawn
and a flag is set that can be read with get_sprite_system_state().

## Known Problems

* A z88dk library can only be linked with the z88dk version it was built with.
If the linker reports a wrong library version, rebuild the library with your
z88dk installation using **make all**.

## License

This software is licensed under the terms of the MIT license.
