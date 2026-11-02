# tiOsc.c - the oscillators

## 1. The classic oscillator
Shape 0..64 crossfades the wave to a saw, 64..127 the saw to a pulse, each a straight line. The saw falls, in phase with the sine and the pulse, so the crossfades add. Pulse width: duty 1/2 + width / 254, silent at the far end.

## 2. PolyBLEP
Band-limits the saw's and the pulse's steps.

## 3. The waves
Sine, triangle and 62 spectral waves, the last held as 64 measured partials each (`tiWaves.c`, generated), their phases measured against the saw so that a wave blends with it correctly. Tables are built once, each with half the harmonics of the last, and the highest whose top partial stays under Nyquist is read.

## 4. The HyperSaw
Up to nine saws. Density reaches 2, 3, 4, 5, 6, 7, 8 and 9 saws at 32, 64, 80, 96, 104, 112, 120 and 127, and in between the next saw fades in, a little faster than a straight line in the dial, while the stack slides to stay centred. Power stays constant, each saw at 1 / sqrt(n). Spread sets the spacing, 3.47 cents at 64 and 1.79 times as much every 16 steps; fewer saws sit further apart (`kHyperWiden`). Measured at note 60; a lone saw is 1.066 of a classic one.

## 5. The Wavetable oscillator
A hundred tables, read with Wave Select choosing the table and Index moving through it. Each table has its own number
of frames, from 2 to 44 (1016 in all), spread evenly over Index: frame k sits at Index k x 127 / (frames - 1). Each
frame is 64 partials, amplitude and phase, measured at the middle of the Index range where Smooth 127 holds it alone;
the phases share one origin, so the partials of neighbouring frames add as they should. A table's waves are built
when first used, in five band-limited versions.

Between two frames the oscillator crossfades, by f, the place between them. **Smooth** (Interpolation) steepens that
crossfade about its middle: the next frame's weight is clamp(1/2 + (f - 1/2) / (1 - Smooth/128)^2, 0, 1). Smooth 0 is
a straight crossfade, 127 steps from frame to frame, and 32, 64 and 96 measure as the law gives (slopes 1.8, 4 and 16).
Against the reference, table 1 at every Index and Smooth is within 0.3% of its spectrum (median). The frame counts
come from where Smooth 127 steps: at (k + 1/2) x 127 / (frames - 1); some tables have two identical neighbouring
frames, so the count is the one whose steps fit, never fewer steps than are seen.

Until 2026-11-02 every table was read as 16 positions at 127/15; between them it drifted from the crossfade (table 1
at Index 13 had the wrong partials).

## 6. Wave PWM
The Wavetable oscillator read twice and subtracted, over the square root of two: (wave(a) - wave(b + offset)) / 1.414,
with the same frames and Smooth as the Wavetable model. Pulse Width sets the offset, (127 - PW) / 254 of a cycle: half
a cycle at 0, which cancels the even harmonics, and none at 127, which is silence. Detune runs the two copies apart, b
up by c cents and a down by c + 0.0018 c^2, with c = 0.0495 x Detune below 32 and 5.04 x 2^((Detune - 64) / 19.3)
above (about 1.8 times every 16 steps; 48 cents at 127). The same in cents at every note. Against the reference: 27
settings of table, Index and Pulse Width within 0.6% of the spectrum (median), the worst 5.6%; the Detune lines
within 0.03 Hz.

## 7. Grain and Formant (Simple)
One grain each period, centred on the period's start: the wave is read k times as fast as the note across the
grain, and a window W fades it from 1 at the centre to 0 half a period either side, W(x) + W(1 - x) = 1 (calculated
as an odd polynomial about x = 1/2). Grain: k = 2^((Formant - 64) / 12), so 64 plays the wave as it is. Formant: k =
2^((Formant - 64 + 60 - note) / 12), the formant held at one pitch, middle C at Formant 64, whatever the key. The
level is the Wavetable model's. Against the reference, median spectrum error 0.8% (Grain) and 0.5% (Formant) over
tables, Index, Formant and two notes. Open: with a rich table and a high Formant the reference keeps fewer of the
partials pushed towards the top, up to 2 dB quieter.

The frames' phases matter here, where the window lines up with the wave's start: they are stored against the
oscillator's own phase origin (fitted on the Grain model at k = 1).

## 8. Grain and Formant (Complex)
Two streams of grains, each grain two periods long and the streams one period apart, crossfading into each other with
the same window. Sync spreads the streams' k, one up and one down by 3/8 of a semitone a step (48 semitones at 127);
Detune drifts their reading apart, evenly up and down by the cents of section 6. Against the reference, median
spectrum error 2.3% (Grain Complex) and 0.3% (Formant Complex); Detune's lines within 0.05 Hz. Open: with Detune on,
a component at half the note's frequency about 24 dB down that the reference does not have; and Sync at 120 is up to
33% out on rich tables.
