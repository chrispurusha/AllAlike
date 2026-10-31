# tiHardware.c - talking to the instrument

## 1. Ports
The instrument offers two MIDI ports over USB. External I/O is its MIDI sockets; Plugin I/O reaches the synth itself, on any device number, and that is the one used here.

## 2. One owner
Only one instance on the machine talks to the instrument, by an exclusive flock() on `hardware.lock` beside the presets, which the system releases if the owner goes.

## 3. Only the edit buffer
Link sends a new sound as a whole Single to the edit buffer, about a second over this port at some 600 bytes a second, and a small edit as single-byte messages, page, part 40, index, value. There is no store command here, so no bank on the instrument can change. ROM banks cannot be written back in any case: a future store must refuse them.
