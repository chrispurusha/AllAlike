# AllAlike - to do

Work that spans the plug-ins. One line per item; each product's own list is in its `Docs/todo.md`.

## Presets and sound files

- Browse folder (SynthLib, then every Alike): one root folder, remembered in prefs, listed as every single in every `.syx`/`.mid` beneath it, subfolders included; loading from it never writes the presets
- Browse folder: rows show name, category and source (`Subfolder/File #12`), sortable A-Z, by category and by source - the bank browser needs the source column
- Browse folder: Prev/Next step through the browse list while browsing; program changes and the hardware link still address the presets only
- Browse folder: an index (path, modification time, names, categories) beside the presets, rescanned only when the folder changes
- Store to next empty slot, beside Store Preset, so a kept sound never lands on an occupied one
- Import no longer replaces the presets: becomes "Replace All Presets..." (confirmed, the presets file backed up first) once the browse folder exists
- If import stays as an add: append from the first empty bank, a file per bank, skip singles identical apart from their address, report what was added - and the fixed preset counts (TiAlike 23 banks) would have to grow
- TiAlike tiPatch notes §6 still says "past 512": the set is 23 banks of 128
