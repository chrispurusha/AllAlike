# nl3Hardware.c

## 1. Who owns the instrument
One instance on the machine, by a file lock beside the presets, so two instances cannot fight over the edit buffer.

## 2. What is sent
Only Slot A: the whole program as one Program Dump from a Slot (command 20, Slot 0), which places it in the Slot and
stores nothing. A dump to a memory location (21) would write flash and is never sent. Every edit sends the whole
sound. Device ID and channel are the instrument's own settings, on the GLOBAL page.

## 3. Reading the programs
The instrument has no request for all its programs, so the read asks for each memory location in turn (command 31,
bank, program), each answer bringing the next request. An empty location does not answer, so after 0.6 s of
silence the read moves on to the next. Done when every location has been asked for and the data has stopped.
Tried 2026-11-07: 1010 programs in 100 seconds, the 14 empty locations skipped. Again 2026-11-13, the same: the instrument holds 8 banks of 128 (a ninth bank does not answer), and the empty ones (4:122-4:128, 6:128, 8:121-8:126) give no reply even after 3 s. The progress bar counted to 1424, a figure from the skeleton; it counts to 1024 now. The answers need MIDI from the instrument on the port of the same name. Nothing is stored until
the import's confirm.
