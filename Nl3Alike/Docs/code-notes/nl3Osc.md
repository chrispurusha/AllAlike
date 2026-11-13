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
words. Checked word for word against the instrument's arithmetic at four notes from C1 to C8, 4284 samples: 4273
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
Not built: the Sine's feedback - each sample moves the phase by 0.69 x a per-oscillator amount x (an earlier output -
this one). That amount is 0 in the Init program; which control sets it, and which earlier sample, are still to read.
