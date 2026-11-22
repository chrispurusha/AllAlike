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
Osc 1's increment gains 4 x depth x Osc 2's last Osc Mod signal - half its output, +out / 2, the word Osc 2 hands Osc
Mod (2026-11-20: first built on the whole output, twice too deep; plain sines with DFM 64 at A4 now match the
instrument's arithmetic within about 1 dB on ten lines, where they were 5-50 dB apart), a fixed deviation whatever the note, so the spectrum changes
across the keyboard; with DFM it gains 16 x depth x the change in that signal between its last two samples - phase
modulation, the index the same at every pitch, as the instrument's manual describes. Increments are in the units the
phase uses (2.0 a cycle). The Mod Env's OSCMOD destination adds its Amount steps to Osc Mod's before the square. The modulated increment is
stored as the accumulator's middle word, not limited: past half a cycle a sample it wraps round, as a phase does
(limiting it, as first built, pinned Osc 1 at the top of the band - FM 127 lost its fundamental by some 56 dB).
Checked 2026-11-22 against the instrument's arithmetic with plain sines, Osc 2 detuned so that their phase sweeps:
FM and DFM at 64, 96 and 127 agree within 0.7 dB in every octave band. Where the two oscillators share a pitch the
result rests on the phase between them, which is free (plain waves) or, for Dual Sine FM, set at the note's start
within a sample or so by when the increments arrive - it differs from note to note on the instrument too, and at
high Amounts that sample changes the timbre a great deal. Not
yet (DM, DIST and RM are §18, FILT filter notes §18): LFO to OSCMOD; Osc 2's output taken before or after its level (taken raw here); a check
of the result against the instrument's sound.

## 18. Osc Mod DM, DIST and RM
Each takes the two oscillators after Osc Mix, at the instrument's level (its mix gains carry 2/3, which Nl3Alike's
levels leave out, so the parts go in scaled by 2/3 and come out scaled back), and gives the filter one signal. With A
the Amount (Mod Env steps added) and d = A / 128, 127 counting as 1:

- **DIST**: y = the mix times 64 x drive, clipped to +-1, then 0.75 (y - c y^3). drive = d^3, held at 1/64 or more;
  c = (A / 32)^3 / 3 below 32, then 1/3. So at low Amounts the curve is nearly straight at three quarters of the
  level, and from 32 up the drive rises with the cube of the knob.
- **RM**: Osc 1's part times 2/3 (1 - d^2), plus twice Osc 1's part times Osc 2's raw output times 2/3 (2d - d^2),
  plus Osc 2's part unchanged - Osc Mix's two gains again, fading Osc 1 into its product with Osc 2.
- **DM**: a hidden sine at Osc 1's increment times 2^((A - 64) / 12), a semitone a step with unison at 64, following
  the keyboard as Osc 1 does. It is raised to 0..1, (1 + sine) / 2, multiplied with the mix, the product times 8 is
  clipped to +-1 and halved. The hidden sine's phase is free-running.

Against the instrument's arithmetic (2026-11-20, both oscillators sine, A4, Amount 64, levels against the same sound
with Osc Mod at 0): DIST's harmonics within 1 dB to h9; RM's two lines within 0.3 dB; DM within 1-3 dB on its strong
lines (the weak ones hang on the hidden sine's phase against Osc 1). Built in floating point in both arithmetics.

## 19. Dual Sine and Dual Sine FM
Two sines per oscillator, A and B, each made at half size from its phase (2.0 a cycle, read through a triangle). The
Sync switch selects Dual Sine FM instead, as the instrument's manual has it. s is the Shape word in Dual Sine and the
FM Amount word in Dual Sine FM (Shape / 128 or Amount / 128), applied squared.

- **Dual Sine**: the output is (A + s^2 B) / 2. Osc 1's B is an octave above A; Osc 2's A and B have their own pitches.
- **Dual Sine FM**: B runs 1:1 with A on Osc 1, at its own pitch on Osc 2, and modulates A's phase; the output is A / 2.
  Osc 1 modulates by B's last change, 32 s^2 (B[n-2] - B[n-1]); Osc 2 by B's last value, s^2 B[n-1].
- **Osc 2's pitches** come from Carrier and Modulator, not Coarse and Fine: A is floor(11 Carrier / 128) - 1 semitones
  from the note (-1 to +9), or with Partial on the partial floor(10 Carrier / 128) of the note (index 0 is 1/2, then
  1 to 9 - 1:11 IonoSPHERE's Carrier 122 is 9 times the note, which first played +9 semitones); B is floor((119 Modulator - 2432) / 128) semitones from the note, or with Partial on the Partial
  ratio of §16 read from Modulator. B's increment stops at the top of the word (about 67.8 semitones up).
- **Phases on a new note**: Osc 1's Dual Sine FM starts A at 0.5 and B at -59/128; Osc 2 starts both at 69/128, or in
  FM A at 69/128 and B at -54/128. Osc 1's plain Dual Sine runs on.

Measured on the instrument's arithmetic (2026-11-20): every Carrier and Modulator value read off the pitch words,
Partial included (all within the laws above); against renders of it at A4, plain Dual Sine's lines within about
1.5 dB; Dual Sine FM's spectra have the right shape but the sidebands differ by 1-3 dB (Osc 1, ours stronger) and
about 4 dB (Osc 2, ours weaker) - to settle sample for sample. The Osc Mod FM's modulator from a Dual Sine Osc 2 is
the instrument's (2026-11-20): -(A + 3 s^2 B) / 4 in plain Dual Sine and -A / 4 in Dual Sine FM, where a plain
waveform hands its output over as +out / 2 - in our units -(A/2 + 3 s^2 B/2) and -A/2 against +out. Osc Mod's FM and
DFM reach Osc 1's sine A alone; sine B keeps the plain increment.

## 20. Osc 2 Noise
The same generator as Osc 1's Noise (§15) through a resonant two-pole state-variable filter of its own, as the
instrument's manual describes - but in three fixed modes, not a sweep. Fine (the program's Noise Type) picks the mode
in thirds: 0-42 low pass, 43-85 band pass, 86-127 high pass. Coarse gives way to the Noise Frequency: the cutoff is
Osc 2's pitch with Noise Frequency - 64 semitones in place of Coarse, so 64 is the note's own pitch, it follows the
keyboard (and Osc 2's Keyboard Track switch) and Fine Tune does nothing. Shape is the resonance.

- **The stage**: the main filter's (filter notes §2) without the cubic term: low = low' + F band',
  high = u - low - k band', band = band' + 4F high.
- **The coefficients**: cutoff word w = pi/16 x Osc 2's increment (so g = 8w = pi f / 44.1 kHz at Osc 2's pitch);
  damping q = (1.0234375 - s)^2 with s = Shape / 128; N from the 1/sqrt(1 + i/8) table with i = (4w^2 + q w) x 128, no
  straight line between entries; F = 8wN, k = qN.
- **Input**: each sample of the generator times 1 - 0.85 s, over 512. Low pass takes it plus the last one; band pass
  the same times N; high pass N^2 times the sample alone.
- **Output**: low pass 64 x (low + the last low), band pass 64 x band, high pass 256 x high, each held to a word.

Against renders of the instrument's arithmetic (2026-11-20, A4, Noise Frequency 100): third-octave spectra within
about 0.5 dB in all three modes at Shape 64, level against a plain sine within 0.1 dB; at Shape 127 the resonant peak
and skirts within 1-3 dB and the level 1.6 dB high. Built in floating point in both arithmetics.

## 21. Osc 2 Keyboard Track off
Osc 2 plays from MIDI note 64 instead of the key played, with no Octave Shift and no glide: in semitones Coarse is
then the MIDI note itself (64 is E4, 70 is 70), and Partial plays no part. In Noise the cutoff is the Noise Frequency
as a MIDI note; in Dual Sine A takes Coarse's note and B Modulator's, Carrier playing no part. Read off the
instrument's pitch words (2026-11-20) at notes 48, 69 and 84, with and without Octave Shift, Coarse 64 and 70,
Carrier 0-127 and Modulator 0-127 in Dual Sine, Partial on and off. Fine Tune, bend and modulation are taken to apply
as with tracking on - not yet checked. It was read from the program but not acted on before: 1:1 TD Style Lead's
Osc 2 (its DFM modulator, fixed at C3) followed the keys, so its timbre did not sweep through the glide as on the
instrument.

## 13a. Shape modulation in sync mode
With Sync on, Shape is the sync pitch (§13), so whatever modulates Shape - the LFOs, the Mod Env - moves the sync
pitch, in Sync Detune steps (half a semitone each), held to 0-127. It moved only the pulse width word before, which a
synced oscillator does not use: 1:1 TD Style Lead's LFO 1 to Shape 1 (raised by its velocity morph) did nothing.
Found by blame against the instrument's arithmetic (2026-11-20): score 7.5 -> 2.3.

## 22. Synched noise
Noise with Sync on (either oscillator): the same stretch of noise restarted every cycle, so it sounds at the note's
pitch with a fixed "body". Four sub-steps a sample, each moving a phase on by a quarter of the oscillator's increment;
the noise generator (§15) steps every eighth sub-step, and whenever the phase wraps - once a cycle - it restarts at a
seed: the Sync Noise Timbre value's top four bits (value & 0x78, as a word), so 16 different waveforms, stepped as the
manual says. The four sub-samples are averaged, and the output is the last output and the last average, halved.
Found by blame on 1:18 Albatross (2026-11-20; it played as filtered noise before): score 36.3 -> 2.2.
