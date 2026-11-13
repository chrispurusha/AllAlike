# nl3Patch.c - programs

## 1. Format
A program is one SysEx message of 261 bytes: F0 33, device, 09, the source (20 a Slot, 21 a memory location), bank
or Slot, program, the name (16 bytes, zero-padded), 16 spare bytes, the version (two 7-bit bytes, 120 for OS 1.20),
then the body packed seven bits to a byte, F7. Unpacked, the body is 191 bytes read as one run of bits, in the order
the manual's dump format gives, with these corrections measured on the instrument (every documented controller sent
and the program dumped; 89 of 90 land in their field): an undocumented bit after each LFO's Env/KB Sync and after its
Mono, and one before Mod Env Invert; spare 5 is 1 bit, not 8; 18 bits stand before Legato, not 9. The last body byte
is the checksum, the byte sum of the 190 before it.

## 2. Files
Every program in a .syx or a .mid, in file order. An F7 event continues the message before it.

## 3. Fields to parameters
Each field read as its parameter's plain value; a morph offset is signed. Checked: six programs from the instrument
read and written back over themselves are identical, byte for byte, and written from their parameters alone they read
back the same; the instrument takes a program written that way and returns every field unchanged.

## 4. Writing back
Every field is written. The bits no parameter covers (spares, the undocumented bits) and the version are kept from
the program the sound came from; a new one is zero there and version 120. Programs saved under OS 1.18 (most of a
long-used instrument's) have the same layout. Checked on a whole instrument's 1010 programs, read and written back
over themselves.

## 5. Presets
1:1 to 8:128, as the instrument's eight banks, in one file read when it changes. Other instances' stores are kept.
Number 0 addresses Slot A.

## 6. Importing
Programs from memory locations go back to their own bank and number when every one has its own, as a read from the
instrument does, so an empty location stays empty. A bank file's bank byte is only the bank it was dumped from (the
instrument stores a received bank in the bank selected at the time), so when bank and number repeat, a new bank
starts wherever the program number goes back down. Programs from a Slot fill the gaps in file order. What is left over
is Init. Checked: a whole instrument read, and Clavia's eight factory bank files one after the other - banks 4, 6
and 8 hold 121, 127 and 120 programs, so their last locations are empty on the instrument too.
