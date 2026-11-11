# osCache.c

## 1. Why
Every note between the file's rate and the host's - which is every note of a 44.1 kHz file in a 48 kHz project, and
every transposed one - went through the 65-tap interpolator (osResample) per output sample, per channel. Eight busy
voices measured 4-7% of a core (2026-11-11), about 90% of it the interpolator. A sample's resampling depends only on
the step, so it is done once per step, into a copy, and a note reads the copy straight: 8 voices then measured 0.9%,
the rest being the envelope and the mix (`tools/osCacheTest.c`).

## 2. Entries and who may touch them
24 slots. `state` is the whole protocol: -1 is the worker's (free, being built or being freed), 0 or more is ready
with that many voices reading. The audio thread takes a slot by raising a state that is not negative (compare and
swap) and only then reads its key, so it never sees one half-written; the worker takes a slot back only from 0 to -1,
which fails while any voice holds it. Nothing waits on anything, and a copy is never freed under a voice.

## 3. The worker
One thread per instance, asleep until the audio thread posts a request (a single-producer ring, 32 deep) and signals it
without taking the lock; a signal lost in the gap before the worker sleeps costs at most a second. It reads the sample
only through `borrow`, which hands it the current sample if it is still the one asked for and keeps the plug-in from
freeing it meanwhile (osPlugin §4).

## 4. What a copy holds
Frame n is the sample at played position n x step, through the same interpolator, running on through the filter's
ring after the last frame - so a note through a copy plays what it would have played live, to float rounding
(largest difference 2.4e-7, forwards and reversed, 0 to +24 st). Reversed, it is read from the end: the kernel is
symmetric, so reading the forward data at (frames - 1 - position) is reading the reversed sample. A note whose Start
is not a whole number of copy frames starts on the nearest one, less than one output sample away. Step 1 (the file's
rate, untransposed) needs no copy: the sample is read straight.

## 5. Room
256 MB across the slots. Copies of any other sample go first, then the least recently played that no voice holds. If
everything left is playing, the pitch plays live.

## 6. Key Track
Off, every note plays at one step: the plug-in asks for that one with `only`, and every other copy goes (an older
Tune, say). On, a copy is made for a note when it is first played - never the whole keyboard ahead - and that first
note plays live while it builds. The root key's pitch is asked for ahead of any note either way.
