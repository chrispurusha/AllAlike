# nl3Osc.c - the oscillator

The Saw (§5-7), Triangle (§9) and Sine (§10) are the Nord Lead 3's own. Square and Double Saw (§1-4) are the
placeholder engine's, inherited from Sn2Alike, and stand in for Pulse until it is built; Noise and Dual Sine play as
Saw for now.

## 1. Shape
Phase runs -1..1. A saw is that ramp multiplied by a curve (§4) of the distance from the jump, which gives the edge its band limit and its ripple. A square is two such saws subtracted, the second offset by Width, so Width 64 is square and 0 silence. Double Saw adds them, the offset counted from Width 64. The curve stops at point 1007, which is the level of the body of the wave.

## 2. Stretch
How many curve points a unit of distance covers, and so how wide the edge is: the lower of a limit set by pitch and one set by Soften. The pitch limit is 30 / increment less 25, within about 2% of its stepped measurements. The Soften limit doubles every 16 steps and runs nearly straight between them; it is low above Soften 112, which only matters below about note 30.

## 3. Not built
Sync, Hardness, Skew and Formant Width.

## 4. The edge curve
Calculated when an instance starts, in three parts fitted to measurements. The rise to point 128 is a sum of sines. To point 768 it is a level of 0.82 with a ripple of period 64 points, its size falling as 0.036 + 5/i. The last part fades that ripple out. Together within about 0.0003 rms of the measured curve.

## 5. The Saw
The phase is a 24-bit word running -1..1 that wraps at the top: a jump of 2 once a cycle, the increment 2f/44100 a
sample. Near the jump a correction is added - what remains of a band-limited jump at that distance - so the edge is
band-limited without oversampling. The output is (phase + correction)/2, less increment/256 (a small tilt the
instrument's output carries).

The correction table is computed when an instance starts: the running integral of a windowed sinc,

    k(x) = sinc(x/128) * blackman(x/512)      x in table points, 128 a sample, 4 samples each side
    blackman(u) = 0.42 + 0.5 cos(pi u) + 0.08 cos(2 pi u)
    T[i] = 2 (1 - integral of k below i / integral of k)

T[0] = 1 at the jump, falling through the sinc's ringing to 0 four samples away; the side before the jump is the
same, negated. Integrated at 64 points a table point. Against the instrument's table, 502 of 512 words are equal and
the other 10 one apart.

Only the nearest jump is corrected, so a note with fewer than eight samples a cycle has its edges corrected one at a
time. On the instrument the Saw's aliases sit around -55 dB at A4 and -41 dB at C7; the engine through an open
filter gives -50 and -37.

## 6. Fixed point
As the instrument computes it: the distance to the jump is the phase plus a half cycle, as a word, wrapping. The table
place is that distance times the increment's reciprocal: the increment normalised to 0.5..1, 0.25 divided by it to
nine bits, and the normalising shift taken back. The place is truncated, limited to ±511 and read from the table of
words. Checked word for word against the measured output at four notes from C1 to C8, 4284 samples: 4273
equal, the other 11 one apart, all of them through the 10 table words above.

Float runs the same law in doubles, with the exact distance and a straight line between table points; it differs from
the fixed path by less than 0.008 on the samples next to a jump.

## 7. Anti-alias On
The Anti-alias switch's default position, not the instrument's: a step 16 samples each side, its sinc cut off at 0.92 of
the Nyquist frequency, and every jump within reach corrected, not only the nearest. Aliases fall to about -90 dB at
A4 and -76 dB at C7 through the engine; the harmonics stay within 0.1 dB to 15 kHz. Anti-alias On always runs in floating
point, whatever the Arithmetic setting.

## 8. Pitch
An oscillator's pitch, in notes with fractions (the note, octave and semitone, fine tune, bend, modulation), becomes
its increment as the instrument computes it. Counted from the E below: a 16-bit mantissa floor(32768 · 2^(k/12)) for
k semitones above that E (E itself is 0.25), an exponent of one per octave, the octave starting at E. The mantissa
times the instrument's tuning word 0x3D376D, shifted by the octave and truncated, is the increment word: 0x028DB6 at
A4, 439.89 Hz - every note sits 0.43-0.47 cents below equal temperament, as on the instrument. Checked word for word
at the twelve semitones and the next C. Float is the same law without the floor and the truncation:
increment = (0x3D376D / 2²³) · 2^((note − 64)/12 − 5), within 5·10⁻⁷ of the fixed path.

Not yet known: the resolution for fractions of a semitone (assumed the same formula), and whether the tuning word
follows the instrument's Master Tune.

## 8a. Held below Nyquist (2026-11-16)
The increment saturates just below Nyquist in both arithmetics, as the instrument's word does (`nl3_word_limit`).
The float one used not to: a pitch far above the top - 1:4 "Ethereality" at full mod wheel, whose wheel morph swings
Osc 2 by 128 semitones - made it larger than a period, the phase outran its one-step wrap, the Sine's polynomial left
its range and, with Shape feeding it back, the output grew without bound until the plug-in went silent. Held, every
waveform wraps correctly and float and fixed agree; all 1,024 programs at full wheel stay bounded.

## 9. The Triangle
The same phase as the Saw, -1..1 a cycle and advanced before use. The wave is |p| - |p - 1|, with p - 1 wrapped back
into -1..1: +1 at phase -1, -1 at phase 0, +1 again at 1. The corner nearer the phase is band-limited: its distance,
counted in table points as the Saw's jump is (128 a sample, through the same reciprocal of the increment, §6), picks
a correction that is multiplied by the increment and taken away, negated at the valley. Then the whole is halved and
increment squared x 0x0D5F85 is added.
- The correction table is four times the integral, from each point to the end, of what remains of the Saw's jump
  (§5): a band-limited corner from a band-limited step. 511 of its 512 words come out within 1 LSB of the
  instrument's; the first, the corner itself, is the instrument's word 0x3750DA.
- At the valley the index is formed as a negative number and shifted, which rounds it up; at the peaks it rounds
  down. Without that the fixed-point wave is off by up to 1000 LSB beside the valley.
- Checked sample by sample at A4: the fixed-point wave is exact on 990 of 1052 samples and within 10 LSB on all;
  floating point within 0.0001. A rendered note matches the instrument's harmonics to 0.1 dB up to the 25th, and its
  level beside the Saw's to 0.01 dB.
- Anti-alias On uses this same wave: a triangle's aliases are already low.

## 10. The Sine
-cos(pi phase) / 2, as an odd polynomial in u = 2|phase| - 1: u c1 + (u^3 c3 + u^5 c5) / 2, with c1 = 0x648035
(pi/4 to 0.03%), c3 = -0x522B2B, c5 = 0x092AA9. Its third harmonic sits at -107 dB, the fifth at -82, the seventh at
-101 - the instrument's own impurity, kept. Each power is truncated to a word and the last sum rounded; the
fixed-point wave is exact on every sample checked (1111 at A4), floating point within 1 LSB.
Its Shape is a feedback: see §12.

## 11. The Pulse
The phase less a second phase, the first moved on by the width plus half a cycle (phases run -1..1, so a cycle spans
2.0): out = (phase - second + step) / 2, a pulse of ±0.5 about its own mean. Only one jump is band-limited each
sample, the one nearer: of the two phases, the one of larger magnitude (the second on a tie). Its signed distance to
the wrap - negated when it is the second phase's, since that jump goes the other way - goes through the Saw's step
table and index exactly (§5, §6), so the Pulse shares the Saw's table and the increment's nine-bit reciprocal.
The Shape dial sets the width word: Shape x 2^16, with 127 the largest word. Shape 0 is a square; the duty is
(1 - word)/2, so 64 gives 25% and 112 gives 6.25%, and 127 leaves almost nothing.
Checked on the instrument (Init with the Pulse, A4, Shape 0, 64 and 112, QU-24 at 48 kHz, 2026-11-14): every
harmonic to the 12th within 0.6 dB of |sin(pi k d)|/k for those duties, falling a little more above (the same
treble droop as the Saw's), and the harmonics a duty should lack 59-73 dB down. Fixed point is exact on all but
three of the samples measured (worst 1 LSB, from the step table's formula).
Anti-alias On band-limits both jumps, every one within reach, as the Saw's §7 does.
Not settled: how an LFO or the modulation envelope moves the width - the depth is a placeholder.

## 12. The Sine's Shape
A feedback. After each sample the phase is moved on by 0.69 × Shape × (the previous sample − this one), and kept: the
moves add up, so the phase runs ahead of the plain ramp by 0.69 × Shape × (the first sample − the last), which is
feedback FM on the oscillator's own output. Shape is the same word as the Pulse's width (Shape/128, 127 as full), so
the LFO and Mod envelope routes to Shape move it too. At full Shape the phase swings by about a sixth of a cycle either
way. Measured: the phase and output exact on every sample checked at Shape 0, 64 and 127. In floating point the
feedback carries rounding into the phase, so at full Shape the two paths drift apart (0.04 after a second); at Shape 0
they stay within 1 LSB.

## 13. Sync
A synced oscillator has a hidden partner at the note's own pitch, and itself runs Sync Detune / 2 semitones above it
(the program's sync tune, which the Shape knob sets in sync mode: 16 semitones at 32, 32 at 64, 48 at 96, 63.5 at 127,
the increments exact). When the hidden phase passes zero going up - lands in [0, its increment) - the oscillator's
phase restarts at +0.5 (not at the start of its ramp), moved on by its own increment times the share of a sample since
the crossing. The waveform is then the ordinary one on that phase: the saw's output is the plain saw's on the synced
phase to within 1 LSB, and its band-limiting knows nothing of the restart. What the instrument mixes is that output a
sample late, scaled by min(1, |hidden phase| / 4 hidden increments) - a fade to nothing at the restart and back over
four samples either side - which is what keeps the jump from spiking.
Measured on the instrument at A4 (Init, filter open), sync tune 64 and 127: harmonics to 20 kHz within 1.9 and 1.7 dB
(median) of the plug-in, against 3.6 and 7.4 dB without the fade. The reset phase is within 18 LSB of the
instrument's, whose share of a sample comes through a reciprocal table. Only the saw was checked; the other
waveforms take the same restart and fade.

## 15. The Noise waveform (Osc 1), 2026-11-16
Measured on the instrument (test programs sent to its slot: Osc 1 Noise, filter open, Shape 0-127, captured) and
built to match:
- A 24-bit generator per voice: s' = (0x0B2D9D s + 0x361963) mod 2^24 - full period - read as a signed word.
- Averaged with its last average (a one-pole at 1/2), halved.
- The two-pole lowpass the filter uses, at q 1, its word from Shape by the filter's own cutoff law (Shape in place of
  Cutoff), the normalisation read from the 1/root table without a straight line between entries; twice the
  coefficient a step, as the filter. Out the mean of its last two lowpass values, at full level (a halving tried
  first was making up for the Volume law, voices notes §14, which put Volume 127 7 dB too low against 86).
- Shape modulation (LFO, morphs) moves the colour: the word comes from the modulated width.
Against the instrument: spectral shape within about 1 dB (3 dB at worst) at Shape 32-127, level within 0-1 dB;
1:6 "White Noise" peaks at 105 Hz (instrument 108) and at half wheel at 1,332 Hz (instrument 1,336). It used to play
as a Saw. Osc 2's Noise (Noise Type, Noise Frequency) is another routine and still plays as a Saw; the Noise 3 mix
path's colour is still unset.

## 16. Osc 2's Coarse, in semitones or partials
Measured on the instrument's own pitch arithmetic (a note held, Coarse stepped through all 128 values, Osc 2's
pitch read against Osc 1's at each): in semitones Coarse is value - 64, -64 to +63 - not clamped at +-60 as first
built. With Partial on it is a frequency ratio: 25 is unison, 26-43 run 1.5 to 10 in halves, 44 up whole partials,
11 to 94 (the readings at C4 stop at 84.3, the oscillator's top); 24 is 3/4, and below it the ratio falls in binary
fractions - 20-23 are 5/16 to 8/16, 12-19 are 9/64 to 16/64, 6-11 are 11/128 to 16/128, 2-5 are 17/256 to
20/256, 0 and 1 both 1/16. Partial was read from the program and ignored before: SawLead (1:21, Coarse 25 with
Partial) played Osc 2 39 semitones down instead of in unison. Fine Tune adds its cents in both modes.

## 17. Osc Mod FM and DFM
Measured on the instrument's own arithmetic (2026-11-16, both oscillators sine, the multiply traced at Amount 16, 32,
64 and 127): Osc Mod's depth is the square of its Amount over 128 - 1/64, 1/16, 1/4 - and 127 counts as 1. With FM,
Osc 1's increment gains 4 x depth x Osc 2's last output, a fixed deviation whatever the note, so the spectrum changes
across the keyboard; with DFM it gains 16 x depth x the change in Osc 2's output between its last two samples - phase
modulation, the index the same at every pitch, as the instrument's manual describes. Increments are in the units the
phase uses (2.0 a cycle). The Mod Env's OSCMOD destination adds its Amount steps to Osc Mod's before the square. Not
yet: DM, DIST, FILT and RM; LFO to OSCMOD; Osc 2's output taken before or after its level (taken raw here); a check
of the result against the instrument's sound.
