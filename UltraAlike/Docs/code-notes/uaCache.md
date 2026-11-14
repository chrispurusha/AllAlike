# uaCache.c

## 1. Why
Every note between the file's rate and the host's - which is every note of a 44.1 kHz file in a 48 kHz project, and
every transposed one - went through the 65-tap interpolator (uaResample) per output sample, per channel. Eight busy
voices measured 4-7% of a core (2026-11-11), about 90% of it the interpolator. A sample's resampling depends only on
the step, so it is done once per step, into a copy, and a note reads the copy straight: 8 voices then measured 0.9%,
the rest being the envelope and the mix (`tools/uaCacheTest.c`).

## 2. Entries and who may touch them
24 slots. `state` is the whole protocol: -1 is the worker's (free, being built or being freed), 0 or more is ready
with that many voices reading. The audio thread takes a slot by raising a state that is not negative (compare and
swap) and only then reads its key, so it never sees one half-written; the worker takes a slot back only from 0 to -1,
which fails while any voice holds it. Nothing waits on anything, and a copy is never freed under a voice.

## 3. The worker
One thread per instance, asleep until the audio thread posts a request (a single-producer ring, 32 deep) and signals it
without taking the lock; a signal lost in the gap before the worker sleeps costs at most a second. It reads the sample
only through `borrow`, which hands it the current sample if it is still the one asked for and keeps the plug-in from
freeing it meanwhile (uaPlugin §4).

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
Tune, say) - once the new one is built, so notes meanwhile still find the old one (§8). An `only` request with a later
one queued behind it is skipped: it is already out of date. On, a copy is made for a note when it is first played - never the whole keyboard ahead - and that first
note plays live while it builds. The root key's pitch is asked for ahead of any note either way.

## 7. Stretched copies
With Stretch not x1 (Tempo Sync included) a copy is also stretched, and keyed by the stretch and Protect as well. Its
plan: the first Protect seconds of the original are resampled as before - tape-style, so at a pitch p they last
Protect / p - and the rest goes through Rubber Band (uaStretch) at the same pitch, to fill what remains of stretch x
the original's length. So the whole note is exactly stretch x as long whatever the pitch, and the attack is the
sample's own. In the owner's example: 4 s, 1 s protected, x2 - the note lasts 8 s, the 3 s after the protected second
becomes 7 s.

If the protected part alone is longer than the length asked for, it plays whole and the rest is squeezed to the
stretcher's limit (1/16; 16 the other way): the note runs long rather than cutting the attack (owner, 2026-11-11).
The two parts meet in a 3 ms crossfade, both starting from the same moment of the sample; on a 440 Hz tone no step
across it is larger than the tone's own and the level moves under 0.9 dB. A note's Start, in the original's frames,
lands in the protected part or proportionally in the stretched one (`ua_cache_position`).

## 8. While a stretched copy is built
A note whose stretched copy is not ready plays the most recently played copy of the same sample at the same pitch
stretched by any amount - the length a little off, the sound right - and asks for its own. Only with no stretched copy
at all does it play unstretched. A host tempo change asks for nothing by itself: the next note does, so twenty
instances do not all rebuild at the same moment (owner, 2026-11-11), and that note plays the old tempo's copy.

## 9. Building now, offline
`ua_cache_build_now` builds a copy on the calling thread and hands it back held. Only a bounce uses it (uaVoice §12):
there the audio thread may wait, and a note must not play a stand-in that real time would not have played. A mutex
(`building`) keeps it and the worker from building at once - they would choose the same free slot and share the
one `borrowed` mark. Checked under ThreadSanitizer with the worker racing for the same copy.
