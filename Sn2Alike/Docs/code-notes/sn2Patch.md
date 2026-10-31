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
