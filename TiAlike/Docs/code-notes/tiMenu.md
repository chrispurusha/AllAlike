# tiMenu.c - the menu bar

File, Presets, Controls (how a drag turns a dial) and Help (the panel, About), as Pulse2Alike's.

## 1. Presets
Pulse2Alike's Presets menu over A001-W128: Load, Store to the preset the sound came from, Store to a chosen one,
Revert, Import from a file or a folder, Export. Pulse2Alike's Pulse 2 menu (link, backup) has no counterpart yet - it
comes with the hardware link (design.md §6).

## 2. Virus TI
Read Banks A-D, or all of A-W, into the presets, and Write Presets A-D to the same bank on the instrument. ROM banks
cannot be written (tiHardware notes §5).

## 3. Sort
A top-level menu for the order the preset browser lists in - Bank / Location, Category or A-Z - which Prev/Next follow
too (tiDraw notes §11). It is the browser's own setting, remembered across sessions and shared with its buttons, so
choosing it in either place changes both.

## 4. Wavetable
Load Wavetable... for Osc 1 and for Osc 2; Prev and Next are on the panel beside each Wave (tiDraw notes §16).
