# To test

Built but not yet checked by ear or in a host. Confirmed: delete the line. Failed: move it to `todo.md`.

- ***FIRST BUILD (2026-10-26)*** - load TiAlike in a host: the panel (OSCILLATORS, FILTER, AMP, GLOBAL), dials and lists, a chord at
  Voice Limit 16 and 2 (the oldest voice is taken), Cutoff and Reso by ear, both formats.
- Link on, in Live: turning Filter 1's Cutoff turns Filter 2's Cutoff dial with it, its text "cutoff (offset)"; dragging Filter 2's dial changes the offset only; Link off: a plain dial again (tiPlugin notes §4)
- Patches (tiDraw notes §9-12), in a host: Load... a one-Single `.syx` (`ChrisPsyBass.syx`) and a sound set `.mid` (bank browser, choose one); the file label and `12/128`; Save Patch As, then Save Patch (overwrite confirm), reload it; Init
- Presets: Import from Folder on the four `VirChrisBank?.syx` (A-D), Prev/Next wrap D128 <-> A001, Load Preset browser, edit a dial -> `A001*`, Revert, Store to... another slot, Export then Import it back
- The name field: click, type mixed case, Return keeps / Escape drops; Store shows the new name in the browser
- Reopen a project: the sound, its name, file / preset and `*` come back as left (tiPlugin notes §5)
