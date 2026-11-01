# sn2Patch.c - programs

## 1. Format
A program is two SysEx messages, the 297-byte main dump and the 99-byte extension, held here one after the other. There are no checksums. Reading a file, a main dump starts a program and an extension with the same bank and number completes it. `Docs/program-format.md` lists every field.

## 2. Files
Every program in a .syx or a .mid, in file order. An F7 event continues the message before it.

## 3. What the panel carries
Only the fields with a parameter so far: the filter's, the amp envelope's and pulse width. Every other byte is kept, so a saved program loses nothing the engine does not use yet.

## 4. Writing back
A byte is written only if its parameter changed, as it may hold more than the panel shows.

## 5. Presets
A001-L128 in one file, read when it changes. Other instances' stores are kept.

## 6. Importing
In file order from A001; what is left over is Init.

## 7. Filter type
The program byte runs 3-14, in panel order: LPF, BPF, HPF, Res LPF, Res BPF, Res HPF, Notch, LPF+LPF, BPF+BPF, HPF+HPF, LPF+BPF, BPF+HPF. The parameter is the byte less 3. Every one of the 1424 backup programs lies in that range.

## 8. Oscillator fields
Stored with an offset by oscillator. Type: 0, 10, 20 (0 Square, 1 Saw, 2 and 3 audio inputs, 4 Double Saw). Octave: 30, 35, 40 at -2. Semitone: 45, 70, 95 at -12. Audio inputs are not built and play as a Saw.
