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

## 9. The sustain pedal
With the pedal down a note off leaves its voices sounding, marked as held by the pedal; when it lifts, every voice so
held releases. A held voice whose key is struck again keeps sounding beside the new note, as the voices do anyway.

## 10. Pitch bend
Every sounding voice reads its sample at its note's rate times the wheel's ratio, 2^(wheel x Bend Range / 12) with the
wheel -1..1, read once a block - so a bend moves held and releasing notes too. The interpolation kernel (§4) stays the
one chosen at the note on.

## 11. Voice limit, stealing and the same note
Measured on the E5000 (2026-11-16): a key struck again does not cut its last voice - with the sustain pedal down each
strike adds a voice, without it each sums with the last one's release - so a repeated note under the pedal fills
every voice. Three GLOBAL settings bound it:
- **Voice Limit** (1-64, default the E5000's 64): voices sounding at once, a fading one not counted.
- **Steal**, at the limit: *Released* (default) fades the quietest voice whose key is up - pedal-held or releasing -
  and only if every voice is held, the oldest; *Oldest* fades the oldest. The stolen voice fades over the steal
  fade while the new note takes a free slot. What the E5000 itself takes is not yet measured; the default follows it
  once it is.
- **Same Note**: *Stack* (default) as the E5000; *Piano* fades a key's last voice when it is struck again, as a
  piano's damper - not the E5000's behaviour, an option.
A voice past its attack whose level is under -100 dB is let go, pedal or not: a pedalled tail that has died away
stops costing anything. Before this a note took a free slot, else the oldest outright, and nothing below the limit
of 64 ever ended while the pedal was down.
