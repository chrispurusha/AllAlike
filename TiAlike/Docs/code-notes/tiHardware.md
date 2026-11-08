# tiHardware.c - talking to the instrument

## 1. Ports
The instrument offers two MIDI ports over USB. External I/O is its MIDI sockets; Plugin I/O reaches the synth itself, on any device number, and that is the one used here. The MIDI menu can send elsewhere instead - any of the
machine's MIDI outputs, an IAC Driver bus into another instance or a sequencer, say - kept for the machine
(`hardwarePort` in the preferences); its first entry returns to the Virus TI's own port.

## 2. One owner
Only one instance on the machine talks to the instrument, by an exclusive flock() on `hardware.lock` beside the presets, which the system releases if the owner goes.

## 3. Only the edit buffer
Link sends a new sound as a whole Single to the edit buffer, about a second over this port at some 600 bytes a second, and a small edit as single-byte messages, page, part 40, index, value. There is no store command here, so no bank on the instrument can change. ROM banks cannot be written back in any case: a future store must refuse them.

## 4. Reading banks
A bank comes for its request (`F0 00 20 33 01 10 32 <bank> F7`) as 128 Single dumps, each carrying its bank (1-23) and
program, at some 600 bytes a second - two minutes a bank. Each is kept by those two bytes, so the order they come in
does not matter. A new request aborts a dump in progress, so the next bank is asked for only once this one has all
128, and a bank that stops short is asked for again only after six seconds of quiet, three times at most (bank O once
needed it). Nothing is stored until the import's confirm. The answer comes back on the Plugin port's input.

## 5. Writing a RAM bank
Only banks 1-4 (A-D) are RAM; the others are ROM and are refused here. Each Single is readdressed to its place in the
bank (`ti_patch_address`) and sent as a dump, one a second (a Single takes 0.9 s on this port), then after three
seconds the bank is read back and compared. A read-back may differ in the device byte, page A bytes 1-3 and the
checksums, as the edit buffer's does; anything else counts as a mismatch. A transfer holds the same machine-wide lock
as Link (§2), so it is refused while Link is on.
