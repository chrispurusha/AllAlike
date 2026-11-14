# Third-Party Software

UltraAlike is licensed under the **GNU General Public License v3.0** (see `LICENSE`).

Copied from the sibling projects 2026-11-09 (same SynthLib link; not yet re-checked against UltraAlike's own binary) (`otool -L`, `nm`): three third-party components are linked in (Rubber Band added 2026-11-11), and
everything else is an Apple system framework (CoreAudio, CoreMIDI, CoreFoundation, CoreGraphics, Cocoa/AppKit,
Foundation, Metal, QuartzCore, libc++, libobjc, libSystem). Check again when the source list in `do-plugin` changes.

## FreeType 2 (statically linked)

- **Website**: https://freetype.org
- **Licence**: The FreeType Project License (FTL) - `SynthLib/ThirdParty/freetype/docs/FTL.TXT`

FreeType is dual-licensed FTL or GPLv2, and this project takes the **FTL** option: FreeType's own `LICENSE.TXT` says
the FTL "is compatible to the GNU General Public License version 3, but not version 2".

**Binary redistribution obligation (FTL §2):** the distribution's documentation must say the software is based in
part on the work of the FreeType Team. `do-release` writes that into the `.dmg`'s `Read Me First.txt` and copies
`FTL.TXT` into its `Licences/` folder. The panel draws its text with it.

## VST3 SDK - interface headers and IID definitions (statically linked)

- **Website**: https://github.com/steinbergmedia/vst3sdk
- **Licence**: MIT, Copyright (c) 2025 Steinberg Media Technologies GmbH

`funknown.cpp`, `coreiids.cpp`, `vstinitiids.cpp`, `commoniids.cpp`, `ustring.cpp`, `conststringtable.cpp` and the
`pluginterfaces/` headers; no CMake, no VSTGUI, no `public.sdk` helper classes.

**Binary redistribution obligation (MIT):** the copyright and permission notice travels with the binary - `do-release`
copies the SDK's `LICENSE.txt` into the `.dmg`'s `Licences/` folder.

## Not in the binary

| Library | Why |
|---|---|
| **GLFW** (zlib/libpng) | Its headers only, for SynthLib's key constants; a plug-in is handed a window by its host. `_wake_glfw` in the binary is SynthLib's own stub (`pluginStubs.c`). |
| **libusb** (LGPL v2.1) | Not used. |

## Rubber Band Library (statically linked)

- **Website**: https://breakfastquay.com/rubberband/
- **Source**: https://github.com/breakfastquay/rubberband, release v4.0.0, a submodule of SynthLib at `SynthLib/ThirdParty/rubberband`
- **Licence**: GNU General Public License, version 2 or (at your option) any later version - `SynthLib/ThirdParty/rubberband/COPYING`.
  Copyright 2007-2024 Particular Programs Ltd.

The time-stretching (Stretch, Tempo Sync). Built by SynthLib's `do-build` from its single-file compilation unit into
`librubberband.a`, which `do-plugin` links into both formats, with Apple's Accelerate framework for its FFT.

**Licence compatibility:** UltraAlike is GPLv3, which the library's "version 2 or later" permits; the combined work is
distributed under GPLv3. **Binary redistribution obligation (GPL):** the source of the whole - UltraAlike's repository and
the library's own - is public, and `do-release` copies the library's `COPYING` into the `.dmg`'s `Licences/` folder and
names the library in its `Read Me First.txt`.

