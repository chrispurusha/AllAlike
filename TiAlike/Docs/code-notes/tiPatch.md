# tiPatch.c - Singles and the presets

## 1. A Single
A TI Single dump is one SysEx message of 524 bytes: `F0 00 20 33 01`, the device id, `10` (single dump), the bank
and program it is addressed to, 256 bytes and their checksum, 256 more and their checksum, `F7`. The first block is
page A (0-127, where parameter n is controller n) then page B (128-255); the name is page B 112-121. The second
block (pages C and D) is kept whole and not yet read.

Checksums: the first is the sum of every byte from the device id up to it, the second every byte from the device id
up to it (the first checksum included), each taken & 0x7F. Both were checked against every Single in 51 files.

An older Virus Single (267 bytes: one block and its checksum) is read as a TI Single whose second block is zeros.

## 2. Reading files
A `.syx` is scanned for F0..F7 messages. A `.mid` (a sound set) is walked track by track; a SysEx event (`F0`) may
be continued by `F7` events, which are joined to it. Any message that is not a Single is skipped, so a file of mixed
dumps yields the Singles in it, in file order.

## 3. What a Single drives
Only what is built plays from a Single (todo.md): Filter 1 and 2 Cutoff and Resonance (page A 40-43), Filter
Balance (48), the two filter modes (51, 52), the routing (53, low two bits), Filter Link (page B 32) and the
amplifier envelope's Attack, Decay, Sustain, Sustain Time and Release (59-63). Every other
parameter keeps its value when a Single loads. A value past a list's end (some older patches hold 64 in a mode byte)
shows as the list's last entry.

## 4. Writing one back
Over the Single it came from, so every byte the panel does not show survives a save. A carried parameter's byte is
rewritten only if the parameter changed, and the name only if it was edited - otherwise an out-of-range mode, or a
name with a character the panel cannot show, would be altered just by saving. Checked by reading and rewriting all
4679 Singles in 51 files: every one came back byte for byte bar its address.

A saved Single is addressed to the edit buffer (device 10 omni, bank 0, program 40), so sending the file to an
instrument plays it without storing it anywhere. With nothing to keep bytes from (after Init) the rest is zeros.

## 5. The presets
A001-W128: four banks of 128, one `.syx` of 512 Singles in `~/Library/Application Support/TiAlike/Presets.syx`, each
addressed to its own bank (1-4) and program, as the instrument's RAM banks are. Missing or short, the rest are Init.
Read again whenever the file's modification time changes, so a store from another instance shows; a store re-reads
first so it never drops another instance's.

## 6. Importing
From a file, its Singles from A001 in file order; from a folder, every `.syx` and `.mid` in name order, one after the
other - so a folder of four bank files (A, B, C, D) lands as banks A-D. Past 512 the rest are ignored; short of it the
rest are Init.

## 7. The pages, and a new sound
`ti_patch_sound` hands the voices the four pages. A new sound starts from the instrument's own starting bytes rather than zeros.

Older Singles (byte 0 below 12) keep Unison, Arp Mode and the Ring Modulator elsewhere: page A 97-100, page B 1 and page A 38.
