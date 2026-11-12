# tiUserTable.c

Reading a user wavetable from a file, as one cycle per frame. The table itself is built by tiOsc (tiOsc notes §5a).

## 1. What it reads
`.wav`, `.syx` and `.mid`. A file of none of these is not offered, and Prev/Next pass over it.

## 2. WAV
PCM of 8, 16, 24 or 32 bits, or float of 32 or 64, the first channel only. The cycle length is the file's own mark
when it has one - a `clm ` chunk beginning `<!>` and the length (`<!>2048`), as Serum writes - and otherwise is decided
by the file:

- 22050 Hz: 256 samples (the PPG tables, 64 or 128 frames of 256);
- a whole number of 2048-sample cycles: 2048;
- a whole number of 256-sample cycles: 256;
- up to 4096 samples: the whole file, one cycle.

With more than 256 frames, 256 evenly spaced ones are kept.

## 3. Waldorf user wavetables
One SysEx message a wave: `F0 3E 13 dev 12 slot wave 00`, then 128 samples of 21 bits in three 7-bit bytes, most
significant first, then a 14-character name and a checksum. Up to 64 waves; a wave missing from the dump repeats the
one before it. The same messages are found inside a `.mid` file, where they are its SysEx events, so the two are read
alike. The table takes the name in the dump; other files take the file's name.
