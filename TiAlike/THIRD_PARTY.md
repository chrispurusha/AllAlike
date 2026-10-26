# Third-Party Software

TiAlike is licensed under the **GNU General Public License v3.0** (see `LICENSE`).

Checked against the built binary on 2026-10-26 (`otool -L`, `nm`): two third-party components are linked in, and
everything else is an Apple system framework (CoreAudio, CoreMIDI, CoreFoundation, CoreGraphics, Cocoa/AppKit,
Foundation, Metal, QuartzCore, libc++, libobjc, libSystem). Check again when the source list in `do-plugin` changes.

## FreeType 2 (statically linked)

- **Website**: https://freetype.org
- **Licence**: The FreeType Project License (FTL) - `SynthLib/ThirdParty/freetype/docs/FTL.TXT`

FreeType is dual-licensed FTL or GPLv2, and this project takes the **FTL** option: FreeType's own `LICENSE.TXT` says
the FTL "is compatible to the GNU General Public License version 3, but not version 2".

**Binary redistribution obligation (FTL §2):** the distribution's documentation must say the software is based in
part on the work of the FreeType Team. `do-release` writes that into the `.dmg`'s `Read Me First.txt`, copies
`FTL.TXT` into its `Licences/` folder, and stops if the file is missing. The panel draws its text with it.

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
| **libusb** (LGPL v2.1) | Not used: this project speaks CoreMIDI and CoreAudio only. |
