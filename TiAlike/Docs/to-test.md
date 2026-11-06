# To test

Built but not yet checked by ear or in a host. Confirmed: delete the line. Failed: move it to `todo.md`.

- Presets > Load Preset / Store Preset to...: the browser reads Bank A-W, Loc 001-128 (as A001-W128), names without the slot prefix; A-Z sorts by name (2026-11-06)
- ***FIRST BUILD (2026-10-26)*** - load TiAlike in a host: the panel (OSC, FILTER, AMP, GLOBAL), dials and lists, a chord at
  Voice Limit 16 and 2 (the oldest voice is taken), Cutoff and Reso by ear, both formats.
- Link on, in Live: turning Filter 1's Cutoff turns Filter 2's Cutoff dial with it, its text "cutoff (offset)"; dragging Filter 2's dial changes the offset only; Link off: a plain dial again (tiPlugin notes §4)
- Patches (tiDraw notes §9-12), in a host: Load... a one-Single `.syx` (`ChrisPsyBass.syx`) and a sound set `.mid` (bank browser, choose one); the file label and `12/128`; Save Patch As, then Save Patch (overwrite confirm), reload it; Init
- Presets: Import from Folder on the four `VirChrisBank?.syx` (A-D), Prev/Next wrap D128 <-> A001, Load Preset browser, edit a dial -> `A001*`, Revert, Store to... another slot, Export then Import it back
- The name field: click, type mixed case, Return keeps / Escape drops; Store shows the new name in the browser
- Reopen a project: the sound, its name, file / preset and `*` come back as left (tiPlugin notes §5)
- The amplifier envelope by ear in a host (ENV page): Attack, Decay to Sustain, Sustain Time both ways, Release; a loaded Single sets all five (tiEnvelope notes)
- Notes at their own sample: a fast attack and release sound the same at host block sizes 64 and 1024
- The five Overdrives by ear at low, middle and high Intensity; Saturation Rectifier

- Smooth Mode in a host: Off/On/Auto/Note a Cutoff move lands at once; Quantise 1/4 holds it to the beat (host tempo)
- Virus TI menu (2026-11-02), with the instrument on USB and Link off: Read Banks A-D (about 8 min) - the presets' A-D should match an earlier backup of the RAM banks; then Write Presets D to Bank D with D unchanged - "all 128 read back the same"; then one edited preset in D written and read back
