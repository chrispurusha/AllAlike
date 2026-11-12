# tiOsc.c - the oscillators

## 1. The classic oscillator
Shape crossfades the wave to a saw and the saw to a pulse, each a straight line: the wave's share below the middle is (127 - 2 Shape) / 127, the pulse's above it (2 Shape - 127) / 127, the rest saw - so Shape 64 is 126/127 saw and 1/127 pulse, and no setting is a pure saw (the instrument's law, 2026-11-12; it was Shape / 64 and (Shape - 64) / 63). The saw falls, in phase with the sine and the pulse, so the crossfades add. Pulse width: duty 1/2 + width / 254, silent at the far end.

## 2. PolyBLEP
Band-limits the saw's and the pulse's steps.


## 2a. A sync restart, corrected
With Sync on, Osc 1 wrapping restarts Osc 2 part-way through a sample: Osc 2 steps from where it had got to back to the
start of its cycle. The step is corrected as PolyBLEP corrects a wrap - with the step's size and its place within the
sample, half the correction on the sample before it and the rest on the sample after (tiVoices). On the sample just after
a restart Osc 2's phase is small without a wrap having happened, so its own wrap correction is left out there, or it
would correct a step that is not there. Against the instrument with Osc 2 a fifth up, the energy between the harmonics
is 9 dB below the instrument's at C6 and C7 (it was 9 dB above).

## 3. The waves
Sine, triangle and 62 spectral waves, the last held as 64 measured partials each (`tiWaves.c`, generated), their phases measured against the saw so that a wave blends with it correctly. Tables are built once, each with half the harmonics of the last, and the highest whose top partial stays under Nyquist is read.

## 4. The HyperSaw
Up to nine saws. Density reaches 2, 3, 4, 5, 6, 7, 8 and 9 saws at 32, 64, 80, 96, 104, 112, 120 and 127, and in between the next saw fades in, a little faster than a straight line in the dial, while the stack slides to stay centred. Power stays constant, each saw at 1 / sqrt(n). Spread sets the spacing: a straight line from 0 (all the saws at one pitch) to 1.09 cents at 32, then 3.50 cents at 64 and 1.79 times as much every 16 steps; fewer saws sit further apart (`kHyperWiden`). The stack sits 0.27 cents below the classic oscillator. The saws run free and never restart on a note, whatever Init Phase says (reference renders with the note at different times give different phase patterns), so at small spreads the level of any one note is down to chance. Measured at note 60; a lone saw is 1.066 of a classic one.

## 5. The Wavetable oscillator
A hundred tables, read with Wave Select choosing the table and Index moving through it. Each table has its own number
of frames, from 2 to 44 (1016 in all), spread evenly over Index: frame k sits at Index k x 127 / (frames - 1).

A frame is one cycle of 512, 256, 128 or 64 samples - its table's own size, the shortest that holds the table's
partials (55 tables at 512, 20 at 256, 18 at 128, 7 at 64) - so up to 256 partials, each an amplitude and a phase
(`tiWavetables.c`, only the partials a frame has). The instrument holds each frame in nine band-limited versions,
256, 128, 64, 32, 16, 8, 4, 2 and 1 partials, in cycles of 512, 256, 128, 64, 64, 64, 32, 32 and 32 samples (none
longer than the table's own). A note plays one version an octave: counting the increment in half-cycles a sample at
the rate the voices run, version v takes over at 2^(v-1)/256 (so 1/256 for version 1).
Each version is read with a straight line between samples whose step is steepened: the line covers the interval in
1/m of its length and then holds, m = the next version's threshold over the increment, in 1/128ths (2.63 at C2 on
a 256-sample table, 1.31 an octave up). The versions below the top up to the 16-partial one are read so, and the top
one on 55 of the 100 tables (`gTiWavetableSharpTop`); the rest are plain straight lines. The phases share one origin, so neighbouring frames add as they should. All
tables are built when the first instance starts (about 0.2 s, off the audio thread). Table 0's two frames are 64
partials as measured. Against the instrument at C1-C6 (11 tables, 2026-11-08): the median partial within about 0.3 dB
and 90% of the partials within about 1.7 dB in the median case, the total level within 1.5 dB in nearly all - see
the todo for the cases still out.

Between two frames the oscillator crossfades, by f, the place between them. **Smooth** (Interpolation) steepens that
crossfade about its middle: the next frame's weight is clamp(1/2 + (f - 1/2) / (1 - Smooth/128)^2, 0, 1). Smooth 0 is
a straight crossfade, 127 steps from frame to frame, and 32, 64 and 96 measure as the law gives (slopes 1.8, 4 and 16).
Against the reference, table 1 at every Index and Smooth is within 0.3% of its spectrum (median). The frame counts
come from where Smooth 127 steps: at (k + 1/2) x 127 / (frames - 1); some tables have two identical neighbouring
frames, so the count is the one whose steps fit, never fewer steps than are seen.

Until 2026-11-02 every table was read as 16 positions at 127/15; between them it drifted from the crossfade (table 1
at Index 13 had the wrong partials).

## 5a. User wavetables
Wave 100 on Osc 1 or Osc 2 is that oscillator's own table, loaded from a file (tiUserTable.md), and every model
that reads a wavetable reads it: Wavetable, Wave PWM, Grain and Formant. It is built as the instrument's tables are:
each frame's cycle turned into its partials (a discrete Fourier transform, below the 512-sample cycle's Nyquist), the
table's cycle the shortest of 64 to 512 that holds them, the same nine band-limited versions, the same reading and the
same crossfade between frames, Index spreading the frames evenly. The loudest frame's peak is set to 1, a sine's level.
Up to 256 frames. With no table loaded Wave 100 plays the sine table rather than silence.

The tables a voice reads are set per thread at the start of each block (`ti_osc_set_user_tables`), so every instance
plays its own.

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

## 9. Anti-alias off: the instrument's own classic oscillator
The Global Anti-alias switch (On by default) chooses between the band-limited oscillators of §1-§3 and the instrument's
own way of making the classic models, aliases included. Off reproduces it step for step:

1. **Points.** The cycle is made as a whole number N of points, chosen by note: N = base x 2^(10 - octave), counting
   notes and octaves from A (note 9), with base 5, 5, 5, 5, 4, 4, 4, 4, 3.5, 3.5, 3, 3 for A to G sharp. So the points
   come at 70-84 kHz whatever the note (40 a cycle at C7, 32 at C sharp 7, 28 at F7, 24 at G7).
2. **A point's value** is the classic mix of §1 with nothing band-limited: the saw at the point's own place in the
   cycle, the wave and the pulse at the point before (the wave is read in full, every partial).
3. **A short filter** on the points: (p0 + 4 p1 + 5 p2 + 4 p3 + p4) / 16.
4. **Two passes a sample.** Each pass moves the place in the cycle on by half a sample; a pass that crosses a point
   takes it in. The value is the filter's last two outputs interpolated in a straight line by how far the place is past
   the newest point, and the second pass of each sample is the one heard.

The aliases are those of the N-point saw (folding about N/2 harmonics), of the straight-line interpolation (about the
points' rate) and of keeping only every second pass. The same steps also roll the top harmonics off: at C7 harmonics 8
to 10 come out 4-8 dB below the band-limited saw. Against the instrument's oscillator output at C7, every alias
line within about 1 dB. N follows the note the pitch rounds to and is held for a block; FM and sync move the place as
they move the phase. Osc 1, Osc 2 and Osc 3's classic, saw and square modes use it, and each saw of a HyperSaw on points of its own (a lone HyperSaw saw aliases within 0.3 dB of the instrument's at C6 and C7, against 9-11 dB cleaner band-limited; a synced HyperSaw stays band-limited); the other models are unchanged.

With Sync on, Osc 2 is made on Osc 1's points: the same count per cycle, chosen by Osc 1's note, and clocked by Osc 1's
place in its cycle, with Osc 2's place the ratio of their frequencies times Osc 1's - so Osc 2 restarts exactly on one
of Osc 1's points and the restart goes through the same filter and interpolation as the rest. Against the instrument's
synced oscillator (Osc 2 a fifth up, C6 and C7) the energy between the harmonics is within 0.2 dB, and the lines
themselves within 1-2 dB at C3. Still apart: the instrument's restart runs about one point late, which at a whole-number
ratio leaves a faint line at every harmonic of Osc 1 (about -38 dB two octaves up); ours has none.

The sub's square, with Anti-alias off, is made on Osc 1's points too: it flips on the point where Osc 1 wraps and goes
through the same filter and interpolation, so its edges always land on Osc 1's points. Against the instrument at C6 and
C7 its aliasing is within 1-2 dB (with the band-limited square it was 4-8 dB cleaner than the instrument). The triangle
sub is unchanged.

## 10. Anti-alias off: Osc 3
Osc 3 runs at the host's rate, a sample at a time, and each sample is the plain wave's average over the phase that
sample spans - worked exactly from the wave's integral, so a step is spread across the sample it falls in. Saw, square
and the copy of Osc 2's classic mix (its wave read at the middle of the span) are made so. Against the instrument at C6
and C7 the aliasing is within 2-3.5 dB (with the band-limited shapes it was 7-10 dB cleaner).

