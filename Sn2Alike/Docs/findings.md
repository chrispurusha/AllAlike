# Sn2Alike findings

Measurements and what they settled, newest last.

## 2026-10-31 - filter types
- A program stores the filter type as 3-14, in panel order, so the byte is the parameter plus 3. The table had been indexed by the byte, which mislabelled the panel, left out HPF+HPF, LPF+BPF and BPF+HPF, and played a wrong type from any program using them.
- All 12 types at all 3 slopes, 108 settings against reference renders: median 0.9 dB. High-pass types go deeper below cutoff than the reference.

## 2026-10-31 - sidebands and nulls
- The sidebands on every partial were the chorus, which editing a program does not turn off in a render. Bypassed through the live parameter they fall from about -1 dB to -38 dB at the fundamental, and L and R match exactly.
- Oscillators free-run, so whole-sample alignment stops near -25 dB. Aligned to a fraction of a sample, renders repeat to about -44 dB across block sizes 64 to 1024.

## 2026-10-31 - sample rate
- The reference renders at a fixed 39062 Hz and is converted to the host rate. Level and spectrum are unchanged from 44.1 to 192 kHz, nothing but a floor lies above about 20 kHz, and alias positions at notes 79-91 fit this rate to 0.03 Hz.
- With effects bypassed, all 12 filter types at all 3 slopes: median 0.7 dB. A few settings land far off, as stray renders have before; they are not yet explained.

## 2026-10-31 - the filter law and the engine rate
- Sn2Alike now runs at 39062 Hz and converts to the host. Output level is the same at 44.1 to 192 kHz and every block size tried, with images near -125 dB.
- Cutoff is a pure exponential and damping a closed form (code-notes/sn2Filter.md §3); the plain types floor section 1, which is their whole difference from the Res types at 24 dB. All 12 types at all 3 slopes: median 0.1 dB, down from 0.7. Below cutoff the high-pass types meet a reference floor near -60 dB, which ours lacks.
- Trap: a far-out score in a parallel batch is a stray render; each repeated alone within 0.3 dB. Repeat an outlier alone before believing it. And an impulse of 1 trips the level-dependent damping, so the test tool uses a small one.

## 2026-10-31 - the oscillator
- A saw is its ramp shaped by a measured edge curve; a square is two such saws, offset by Width (code-notes/sn2Osc.md). Level is right to about 0.1% through a wide-open filter, across notes, Soften, Width, Mix Level and Volume.
- Nulls against reference renders, one saw or square through the filter: -25 to -40 dB, with -25 at note 96 and about -31 for most settings, against about -44 for the reference repeating itself. A literal run of the edge at the exact stretch reached -41, so what is left is mostly the stretch law, within about 2% of its stepped values.
- Resonance 127 nulls at -24.5 dB and level is within 1%, so a quarter for the level into the filter looks right; it is not proven.
- Trap: stray renders come out louder (rms 0.059-0.064 against 0.0445) and are cached by the harness. Repeat alone with a different cache key.

## 2026-10-31 - three oscillators
- The edge curve is now calculated from 27 constants, within 0.0003 rms of the measured curve, and every null moved by about 0.1 dB or less, bar Soften 0 at 1.2 dB.
- Osc 2 and 3 use the same law as Osc 1, each with its own offsets in the stored Type, Octave and Semitone. Level matches on Saw, Square and Double Saw, with nulls of about -26 to -30 dB.
- Each oscillator of a voice sits a fixed number of hertz off pitch, Osc 2 about +1 Hz and Osc 3 about -0.4 Hz on the voice measured, and it grows a little with the octave (nulls drop to about -12 dB two octaves off). Other voices differ.
- The window grew to 780 x 440 so the three oscillators sit on one page.

## 2026-11-01 - what real programs use
- Across the 1424 backup programs: Overdrive non-zero in 99.6%, at 16 in 72%; Env Velocity set in all, 64 (none) in 54%; Tracking 127 in 75%; Osc 2 on in 79%, Osc 3 in 74%; noise 15%, ring 10%, FM 6%.
- Tracking moves cutoff (note - 60) x Tracking / 127 steps. Nulls -30 to -44 dB.
- Overdrive is a saturator ahead of the filter; at 16 and 47 level is within 0.3% and nulls -28 to -39 dB. At 127 ours is 12% loud. Velocity is a blend towards a cubic, within about 3% of level at the settings tried.
- With the mix reaching the saturator at a half and the filter at a quarter, Resonance 127 nulls at -26 dB with level right.

## 2026-11-01 - the amplifier envelope
- Attack is a straight line, Release an exponential with a fixed halving time, Sustain the dial squared, Decay an approach to Sustain (code-notes/sn2Envelope.md). Over a note against reference renders, median within about 0.2 dB. Sustain Time and Rate are not built.

## 2026-11-01 - real programs
- 36 of 40 varied backup programs, held note, effects bypassed: median spectrum 1.7 dB, level 3.1 dB, signed level 0.0, 14 within 1.5 and 3. The worst are noise, percussion and FM sounds, and Filter Envelope patches, none of which is built yet.

## 2026-11-01 - Env 2 and 3, noise, ring
- Env 2 and 3 share the amplifier's law with a linear Sustain, and move the cutoff 2.2146 dial steps per step of amount. Ring is exact to 0.1 dB in level and spectrum. Noise is shaped from measurements only, and the change near Soften 120 is a guess.

## 2026-11-01 - Q Normalise
- Set in 60% of programs. The input is scaled by 1 - (Q / 128) (Resonance / 127), and above Q 120 the damping is trimmed (code-notes/sn2Filter.md §6). Real programs, median level error: 0.9 dB on a fresh sample, down from 1.5.

## 2026-11-01 - the chorus
- Across the backup: chorus on in 36% of programs, reverb 35%, delay 23%, EQ about 37%, distortion 14%.
- With effects on, the dry path at full is 3 dB above the bypassed signal. The chorus is two taps a side, each on its own triangle (code-notes/sn2Effects.md), found by reading the clicks from a low saw's edges.

## 2026-11-01 - delay and glide
- Delay: every echo time and gain within 0.1 ms and 0.01 against reference renders, across Time, Feedback, Send, Width, Ratio and Sync. One line with two taps, fed back from the full time; Ratio programs showed that.
- Glide takes the same time for any interval. Pitch curves within about 0.3 semitones against reference renders, Linear and Exponential, up and down; Exponential over two octaves at Time 80 is up to a semitone quick.

## 2026-11-01 - the reverb
- A model. Each type is measured for decay at four Decay settings, time to the fullest point and level; HF Damp shortens only the top; Send is linear; Early Reflections showed no effect. Against reference renders at Decay 86, decay time within about 10% and level calibrated, stereo correlation about right. Gated types have no gate yet.

## 2026-11-01 - effects order
- All 19 built from their names. A render of the reference ignores this setting, live or in a program, so series gains are unmeasured. Order 0 is unchanged.

## 2026-11-01 - a correction
- The pitch offsets per oscillator recorded earlier were an error in how test programs were placed in the reference, and are gone. All oscillators of a note share one pitch and, in Percussive mode, one starting phase. The real instrument holds three identical saws steady, take after take. Results above that used those offsets are superseded.

## 2026-11-01 - effects order on the instrument
- Played as a program, the real instrument runs its effects side by side in every order tried (0, 1, 3, 5, 13, 16), as the reference does. Programs no longer load the order. Measured from its outputs, the delay echo stayed within about 2% of the dry level throughout.

## 2026-11-01 - EQ and distortion, on the instrument
- EQ measured from the real instrument: two first-order shelves, within about 1 dB of it in almost every band, 2.8 dB at most above 10 kHz at Treble 0.
- Distortion is on the mix after the filter, shown by a filter sweep and by two notes intermodulating. Gain Compensation does nothing at Level 0. Curve showed no effect.

## 2026-11-01: chords
The Distortion curve ran at Level 0 and soft-clipped the summed voices on every program. On the instrument, Distortion 0 is clean: intermodulation of two notes -84 dB, against -46 at Level 30 and -34 at 90, so Distortion works on the mix and is out of the path at 0. Fixed: at Level 0 the curve is skipped.
Level rises as a plain sum of voices (3 dB per doubling), with no gain reduction for polyphony. Glide in Poly is the instrument's behaviour: Auto glides a note over a held note.

Program Volume and the output ceiling, voices notes §14. An earlier note here and in the todo that Volume has no effect was wrong.

Distortion against the instrument, two notes, f1+f2 against f1: Volume 63, Level 67, compensation 112: -18.5 dB, here -16.6. Level 60: Volume 84 -18.5, here -17.0; Volume 30 -26.4, here -28.8; Volume 10 -36.5, here -42.6. Volume comes before Distortion on the instrument, as here. A002 is Level 67, so the crunch that follows Prog Level is the patch.

A002 with Distortion 0, four notes at velocity 127: a chord is the sum of its notes to -142 dB up to Volume 40; at 63 it reaches the ceiling 1.4% of the time. The instrument's capture of the same chord peaks at 0.250 on one side, which is 0.977 less the rig's 11.8 dB, so the ceiling looks to be the instrument's too; not proven, the other side reads 0.272.

Correction: Volume 40 is not clean on A002. Held for 3 s, Distortion 0, four notes: the instrument's peaks stop growing between 20 and 40 (0.199, 0.295, 0.330) and kurtosis falls 5.08, 4.44, 3.32 at 20, 40, 63, with the top bands against Volume 20 up 4-20 dB at 40 and about 30 dB at 63; here 0-9 dB at 40. So the instrument clips this chord, and harder than this engine: its peaks at low Volume are about 1.7 times ours at the same rms.

C3 E3 G3 B3 at velocity 110, Distortion 0, held 3 s: the instrument's peaks are 0.092, 0.125, 0.174, 0.266, 0.296, 0.320 at Volume 10, 14, 20, 30, 40, 63, linear to 30 and bending from there. Here 0.218, 0.305, 0.436, 0.654, 0.873, 0.977, reaching the ceiling only above 40.

A001 Skewed Arp on the instrument, C3 E3 G3 B3, effects on against bypass (levels with the rig's 11.8 dB taken off): rms -11.7 and -15.2, peak 0.2 and -3.8 dBFS, kurtosis 3.36 and 2.88. Here: rms -9.7 and -14.0, peak -0.3 and -0.2, kurtosis 2.34 and 2.92. So bypass runs 1.2 dB hot with 3.6 dB more peak; with effects on 2 dB hot, and the distortion stage flatter than the instrument's. Plain saw through a two-note test: 4-5 dB brighter than the instrument above 1 kHz with Distortion 0, and the same offset with it on, so the Distortion curve itself adds no extra top.

Distortion rebuilt (effects notes §7a). A001 chord with Distortion 0, rms against the instrument by Volume 10-63: a constant 13.2-13.6 dB, where it had drifted 10-11 dB. So the rig offset is about 13 dB, not 11.8. Still out: Distortion at 67 is 1.9 dB low and flat (kurtosis 2.26 against 3.36).
Soften 127, 100 and 64 match to 0.1 dB, so the 4-5 dB above 1 kHz against the instrument is the rig or its output, not the oscillator.
A001 voices, with Sync off on both: above 3 kHz ours is 22-26 dB dark against the model, chord peak 0.89 against 0.71. Not Sync alone: find what else opens its top.

A001's 2.4 dB between panel and backup is one byte: the stored program's Delay Send is 0, the backup's 104. With it at 0 a load matches the panel, -23.85 against -23.8 dB.

Effects on against bypass on the instrument, plain saw, Volume 30: 3.07 dB, Dry linear (127, 90, 64: -44.22, -47.21, -50.17 dB). The program's Effects Bypass byte works from the edit buffer. The rig reads 13.9 dB.

## 2026-11-02 - chords and the voice pool (instrument, edit buffer only)
A001 with effects and distortion off, held 1/2/4/8 notes (48 55 60 64 67 72 76 79), level in the sustain against one note:
- Unison 1 voice: instrument +3.1/+5.1/+9.3 dB, reference +3.3/+5.8/+7.8, Sn2Alike +3.3/+6.2/+8.7 - a plain sum on all three.
- Unison 16 voices (A001 as stored): instrument -0.3/+3.7/+6.2 dB, reference +3.1/+5.6/+8.3, Sn2Alike +3.3/+6.2/+8.7.
  The instrument runs out of voices; neither the reference nor Sn2Alike does.
- On a program change the instrument's delay echoes stop, as Sn2Alike's now do (by ear).

