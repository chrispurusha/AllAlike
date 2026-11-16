# uaBankVoice.c

## 1. Notes and zones
A note starts a voice for every zone of the preset whose key and velocity windows hold it. Its step is the sample's
own rate over the host's, times 2^((note - root + Tune)/12): the sample's tuning is in its rate (SynthLib emuBank
§3). Every note off releases, whatever the layer's Mode - a sampler's notes are held. Up to 64 voices a layer; past
that the oldest is faded out in 3 ms. Not yet read from the bank, so not played: a voice's tuning, level, pan,
envelopes, filter, LFOs and cords - each voice plays at unity through the layer's own envelope.

## 2. Loops
A looped sample plays to its loop end and then round the loop for as long as the voice sounds, release included.
The interpolator's taps either side of the seam are gathered in the order they sound - past the loop end, the
loop's start again; once looping, before the loop start, the loop's end - so the seam reads as one continuous
signal. Away from the seam it reads the sample directly.

## 3. The amplifier envelope
Every voice plays the E5000's envelope (uaEosEnv): the zone's own, converted from the bank's times when the preset
was made, with the AMP ENVELOPE dials' edits (§6). A loose sample plays as a one-voice preset (uaPlugin §16) with the
dials' envelope.

## 4. Anti-alias Off
A note started with Anti-alias Off reads through the E5000's own kernels (uaEmuInterp) instead of uaResample: the
kernel chosen once, from the note's ratio, at the note on. The loop seam is gathered the same either way; the reach
around the seam and the end of a one-shot are the kernel's own length.

## 6. The AMP ENVELOPE dials
The dials show a preset's first voice's rates and levels when it is chosen in the editor (uaPlugin §15). At a note on
each voice's envelope gets every dial's movement since then added, step for step - a rate up 5 is 5 steps faster in
every voice - clamped to 0-127 and 0-100%. Untouched dials leave every voice as converted. A loose sample's envelope
is the dials' own.

## 7. Solo
Any Solo mode but Off plays one note at a time: a new note fades out what is sounding. The modes' differences
(retrigger, legato, priority, fingered glide) are not built yet.

## 8. Pitch
A note plays at the sample's rate times 2^((note - root + Transpose + Coarse + Fine / 64) / 12), the root the zone's
or, for a loose sample, the Root dial; Non-Transpose plays every key at the root's pitch.
