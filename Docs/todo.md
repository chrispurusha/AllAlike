# AllAlike - to do

Work that spans the plug-ins. One line per item; each product's own list is in its `Docs/todo.md`.

## Distribution

- Update check (CT 2026-11-14): ask GitHub's releases API for the newest tag at start (opt-out in prefs, at most daily), compare with the build's version, show "Update available" with a link; shared in SynthLib so every app and plug-in gets it
- Auto-install updates (CT 2026-11-14): investigate whether it is possible - download the .dmg/.zip, verify it, replace the app or the installed plug-ins; unsigned builds mean quarantine to clear, /Library plug-in folders may need admin, a plug-in cannot be replaced while a host has it loaded, and a public repo must never fetch and run anything unverified
- MPC/Force plug-ins (CT 2026-11-14): inMusic's official SDK (MIT) is cloned at GitHub/mpc-plugin-builder-sdk - a VST2-ABI .so (arm32 for Live/X/One/Key/Force, arm64 for Live III/XL), MPC OS 3.10.1+, 44.1 kHz fixed, no GUI (UI from ui.json skins + 16 Q-Links), state as a chunk, glibc 2.31, a few % of one core per instance (10-40x slower than a laptop). Route: a SynthLib wrapper from tSynthLibPluginDesc to its mpc::Plugin, and ui.json generated from our page/section tables; candidates OneShot and Pulse2Alike (measure first), ModAlike only once SOUND_ENGINE_MAX_ENGINES is settable (the MPC checks free memory before loading)

## Presets and sound files

- Browse folder (SynthLib, then every Alike): one root folder, remembered in prefs, listed as every single in every `.syx`/`.mid` beneath it, subfolders included; loading from it never writes the presets
- Browse folder: rows show name, category and source (`Subfolder/File #12`), sortable A-Z, by category and by source - the bank browser needs the source column
- Browse folder: Prev/Next step through the browse list while browsing; program changes and the hardware link still address the presets only
- Browse folder: an index (path, modification time, names, categories) beside the presets, rescanned only when the folder changes
- Store to next empty slot, beside Store Preset, so a kept sound never lands on an occupied one
- Import no longer replaces the presets: becomes "Replace All Presets..." (confirmed, the presets file backed up first) once the browse folder exists
- If import stays as an add: append from the first empty bank, a file per bank, skip singles identical apart from their address, report what was added - and the fixed preset counts (TiAlike 23 banks) would have to grow
- TiAlike tiPatch notes §6 still says "past 512": the set is 23 banks of 128

## Panels, every Alike

- Double-click on a dial returns it to its default; a keyboard-tracking dial's default is its one-to-one setting

## New plug-ins

- ModAlike: started 2026-11-13 - its own list is ModAlike/Docs/todo.md

