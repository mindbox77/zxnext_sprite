# Demonstration of zxnext_sprite

This folder contains a simple example program demonstrating how to use the
[zxnext_sprite](https://github.com/stefanbylund/zxnext_sprite) C hardware sprite
API for the ZX Spectrum Next.

![zxnext_sprite demo](zxnext_sprite_demo.png)

The demo shows:

* A plain 8-bit sprite using the original 4-byte attributes (bouncing).
* The same pattern scaled 2x and mirrored using extended attributes (bouncing).
* A 4-bit version of the pattern shown twice with different palette offsets
  (top-left and top-right corners).
* A unified big sprite made of an anchor sprite and a relative sprite, which
  is rotated every 64 frames by rewriting a single attribute byte via the Next
  register interface (bouncing).
* A sprite clip window that hides the sprites 8 pixels from the screen edges.

Press any key to exit the demo.

## How to Build

If you want to build this demo program yourself, follow the steps below:

1. On Windows, you need [MinGW](http://www.mingw.org/),
[UnxUtils](https://sourceforge.net/projects/unxutils/) or
[Cygwin](https://www.cygwin.com/) for the basic Unix commands. Add the chosen
set of Unix commands to your path.

2. Install the latest version of [z88dk](https://github.com/z88dk/z88dk) and the
[ZEsarUX](https://github.com/chernandezba/zesarux/releases) or
[CSpect](https://mdf200.itch.io/cspect) emulator.

3. Download the zxnext_sprite repository either as a ZIP archive using the
"Code" button at the top of this page or with Git using the following command:

> git clone https://github.com/stefanbylund/zxnext_sprite.git

4. Go to the zxnext_sprite directory and enter the following command to
build the C hardware sprite API for Spectrum Next:

> make all

5. Go to the zxnext_sprite/demo directory and enter the following command to
build the demo program:

> make all

6. Run the zxnext_sprite/demo/bin/\<compiler-flavour\>/zxnext_sprite_demo.nex
file in the ZEsarUX or CSpect emulator or on a real Spectrum Next (core 3.0
or later).

**Tip:** If you don't care for makefiles or don't want to install
MinGW/UnxUtils/Cygwin on Windows, you can build zxnext_sprite demo manually
by using the z88dk command-line invocation located in the header of the
zxnext_sprite_demo.c source file.

**Tip:** To start the ZEsarUX emulator directly in Spectrum Next mode, start it
with the following options:

> zesarux --noconfigfile --machine tbblue --enabletimexvideo --tbblue-fast-boot-mode
  --quickexit --enable-esxdos-handler --esxdos-root-dir <zxnext_sprite_demo_folder>
  zxnext_sprite_demo.nex

**Tip:** To start the CSpect emulator directly in Spectrum Next mode, start it
with the following options:

> CSpect -w2 -tv -zxnext -mmc=<zxnext_sprite_demo_folder>/ zxnext_sprite_demo.nex

## License

This software is licensed under the terms of the MIT license.
