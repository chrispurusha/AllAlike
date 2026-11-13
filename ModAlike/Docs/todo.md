# TODO

One line per item.

## Next
- Size (measured 11-13, 3.25 MB universal): reserve fewer engines (engine count as a build setting, G2Alike engine), strip local symbols and export only the entry points (do-plugin), trim FreeType to TrueType (SynthLib, ~620 KB a side) - owner approved as to-do 11-13
- Size, later: split the engine's part of the module tables from the editor's layout (G2Alike) - ~170 KB and the last instrument names in the binary
- Dial values in their units (Rate in Hz, Range, Detune), not 0-127
- Switching effects clicks (the patch is rebuilt) - crossfade the old effect out and the new one in
- Instances: at most 32 per host process (the engine library's limit) - a 33rd fails to load; raise it, or engines per effect

## Later
- Double-click a dial to its default (OneShot has it; AllAlike todo for every Alike)
- Plug-in exports: only the entry points (GetPluginFactory, SynthLibAUFactory), so -dead_strip can drop unused library calls - every Alike's do-plugin
- Wide for the Chorus: g2am_lfo_link handles only the Phaser and Flanger
- More effects from the same engine: FreqShift, Delay, Reverb, Compressor
- Windows: needs the engine library's Windows build first
- Measure on the instrument whether In -> Out loses 4.6 dB there too (design §3)
