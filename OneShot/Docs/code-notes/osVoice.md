# osVoice.c

## 1. Voices
Eight. A note takes a free voice, else the oldest, which is cut (it cannot fade under the new note in the same slot).
Mono mode fades every sounding voice before the new note starts. Mono is the default since 2026-11-11 (owner); a
project saved before keeps the Voices it saved.

## 2. Fades
3 ms for a stolen voice, a Mono retrigger and a change of sample. A note off in Gate mode starts the envelope's release.

## 3. Interpolation
Band-limited, by osResample (its notes). The step is the file's rate over the host's, times the pitch ratio, so a
44.1 kHz file plays at its own pitch in a 48 kHz project. It replaced 4-point Hermite on 2026-11-09, which aliased.

## 4. Pan and Volume
Read once a block, so a move would step at every block boundary (a zipper). The output gains - equal-power pan x
volume, one per side - instead follow the parameters through a one-pole smoother, 5 ms time constant, per sample,
shared by every voice; the first block of an instance starts on the setting. A hard-left-to-hard-right jump measured
a largest step of 0.4% of full level a sample, settled within 0.1% after 30 ms (2026-11-09).

## 5. Start
A note starts Start seconds into the sample (0-2 s; the dial's law is Attack's, exponential above 1 ms, 1000 steps).
Snap (default On) moves it to the nearest zero crossing within 5 ms either side (osSample §6), so the note starts
without a step; with Snap Off it starts exactly there and fades in over 1 ms, which is what keeps that from clicking.
A Start past the end plays nothing. Checked 2026-11-09 on a 100 Hz sine from 12.3 ms: Snap On started at the crossing
at 10.000 ms, sample-exact after; Off started at 12.29 ms with the 1 ms rise, then exact.

## 6. Reverse
The sample backwards. Positions are counted as played, so Start runs from the sample's end and Snap looks for its
crossing there; a voice reads the forward data at (frames - 1 - position), which the interpolator's symmetric kernel
makes the reversed sample. At the file's own rate it is the file backwards, sample for sample. Taken at note on:
switching it does not turn a sounding note round.

## 7. Through a copy
A note whose step is not 1 plays from osCache's copy for that sample, direction and step if one is ready: step 1
through it, from the copy frame nearest Start. Otherwise it plays live and asks for one. The voice holds the copy
until it ends, which every way a voice ends goes through (`end_voice`).

## 8. Stretched
Stretch (the plug-in's multiple, times the base tempo over the host's with Sync on) and Protect come in the play
settings; any stretch other than 1 takes a stretched copy (osCache §7, §8), so Tune and Keys then change the pitch
but not the length. `os_voices_key` is the one place the copy a note wants is worked out - the note and the
plug-in's look-ahead both use it, so they ask for the same thing.

## 9. End
A note stops at End: a fraction of the sample's length counted in the order it plays (so from the end when reversed),
in the original's frames, snapped like Start (5 ms either side, osSample §6). Snap On cuts at the crossing; Off fades
out over 1 ms ending there. At 100% (the default) nothing changes and the filter's ring after the last frame plays as
before. End at or before Start plays nothing. Through a copy, End and Start are carried into it by its plan
(osCache §7) - and, snapped, looked for again in the copy's own data: stretching moves the waveform's phase, so a
crossing of the original is not one of the copy (measured 2026-11-11: 0.5 at the cut on a 0.5 tone before, at a
crossing after). Checked live and stretched, Snap On and Off (`tools/osCacheTest.c`).

## 10. Delay
A note waits Delay before it sounds - the envelope not started, nothing read - counted in whole output frames (21 us at
48 kHz). It is real time: neither Stretch nor the tempo moves it. In Gate mode a note off during the wait releases a
note that has not sounded, so it stays silent. Checked: a delayed note is the undelayed one moved, sample for sample.
For sample stacking, where layers are lined up against each other.

## 11. Protect snapped
With its own Snap On (STRETCH's, not TRIM's - owner, 2026-11-11; default On) the end of the protected part moves to the nearest zero crossing within 5 ms, like Start and End, so the
untouched attack hands over to the stretched rest at a crossing. It is worked out in `os_voices_key`, so the copy is
keyed and built on the snapped boundary, and the panel's purple line uses the same function. Checked: Protect 51 ms on
a 440 Hz tone moves from frame 2249 (0.187) to the crossing at 2255 (0.004); Off leaves it at 2249.

## 12. Offline
In a bounce (the host's offline mode) a note whose copy is not ready builds it before it plays, rather than playing
live, or a stretched note playing the nearest copy at the wrong length. So an offline bounce is what real-time
playback with every copy ready sounds like, and the same each time, whatever the host's speed or automation.

## 13. Stereo
A stereo sample stays stereo everywhere: both channels through the interpolator (one set of weights), the copies,
Reverse, the stretcher (its channels together, so the image keeps its phase) and each layer's filter and saturation
(a state per channel). Pan on a stereo sample is a balance: each side keeps its own channel and only its level moves
(equal power; centre leaves both as they are). Checked (`tools/osCacheTest.c`, 440 Hz left and 660 Hz right): every
path keeps each tone on its own side, the other side's at least 115 dB down - stretched, 68 dB (the stretcher's
joint processing) - and half-left pan gives +2.4 / -5.3 dB.
