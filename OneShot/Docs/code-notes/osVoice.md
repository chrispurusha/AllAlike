# osVoice.c

## 1. Voices
Eight. A note takes a free voice, else the oldest, which is cut (it cannot fade under the new note in the same slot).
Mono mode fades every sounding voice before the new note starts.

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
