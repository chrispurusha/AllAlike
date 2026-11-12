# tiVoices.c - the voices

## 1. A voice
Oscillator, the two filters and the amplifier envelope (tiEnvelope.md), per voice. The oscillator is still a
placeholder; the filter envelope, modulation and the rest come section by section. A voice is free once its amplifier
envelope has finished its release.

## 2. The voice limit
The instance's voice limit (design §1), 1 to 32. A note takes the voice already playing its key, else a free
voice, else the oldest. Lowering the limit releases the voices above it rather than cutting them.

## 3. A Single is the sound
The voices read a Single's pages directly (`tTiSound`), so every byte of a loaded patch reaches the engine whether or not the panel has a control for it. The panel's parameters are written over those bytes when they change.

## 4. Filter coefficients every 32 samples
The filter's coefficients are worked out every 32 samples and shared by every voice: nothing per voice moves the
cutoff yet. Every 32 rather than once a host block so that a glide (§5) is not stepped by a host's block size, which
can be 1024 samples. Keyfollow and the filter envelope will make them per voice.

## 5. Smoothing a control change: none, or held to the tempo
Parameter Smooth Mode never glides. Off, On, Auto and Note: a changed control takes effect at once. The seventeen
Quantise values (1/64 .. 1/1, the triplet and dotted ones too): the change is held until the next division of the
tempo, then lands as a step. Measured on the hardware TI with Cutoff (findings 2026-11-06: every mode a step within
2-15 ms; Quantise 1/16, 1/4, 1/1 held 28, 245-305, 590-1340 ms, each under one division at 120 bpm; the reference
renders likewise jump). It used to glide in a straight line over 43.7 ms with Smooth On, read from an early comparison
of the Init Single - wrong.

What is held: the continuous controls of page A (`kQuantiseHeld`) - the oscillators' shape, pulse width, semitone,
detune, keyfollow and FM, the mixer levels, Noise Color, Cutoff, Resonance, Balance, envelope amounts and keyfollow,
the LFO rates, symmetries, keyfollows and amounts, the amp and filter envelopes' Sustain and Sustain Time, Portamento,
Patch Volume, Panorama and the chorus and delay dials. Switches, modes, shapes, the wavetable selects, Saturation Curve,
the filter modes and routing, and the amp envelope's Attack, Decay and Release land at once. Only Cutoff has been
measured held; the rest follow it as controls of the same kind.

The voices play `v->sound`, not the Single they are handed: `hear()` copies the Single in, keeping a held byte's old
value while a Quantise mode is on, and the block the division passes takes the whole Single. Everything that reaches
the voices goes through it - the plug-in's host edits (`ti_voices_hear()`) as well as the render.

The divisions are the clock's. While the host plays they are the host's own beats (`projectTimeMusic`, advanced
through the block at the tempo), so a held change lands on the host's 1/4 or 1/16 grid, as on an instrument following
MIDI clock; starting the transport restarts the clock and lands anything held at once. Stopped, they run on the
engine's own clock (`seconds` since the instance started) at the host's tempo, or the sound's Clock Tempo (63 + B16)
with none - the instrument's clock runs free of the note too, so where its boundaries fall against a note differs
take to take there as here. A block early at most. Loading a Single is not a control change: all of it lands at once
and a change held from the old sound is dropped (`ti_voices_new_sound()`). A short treble spike at some jumps comes
with Off as well - the filter's own response to an instant change, not a smoothing.


## 5a. A new sound stops the notes
Loading a sound stops every voice at once, inside the swap's silence (tiPlugin notes §12): no note of the old sound
carries on with the new sound's settings. The keys still down are kept, so their releases land as usual. Analog Boost's
smoothed copy of the output is emptied too: it held the old sound and, scaled by the new sound's boost, put a
clipping-loud transient into the first block (A061 to A112, CT).

## 6. The two filters: routing, Balance, Link
Filter 2 has the digital modes only (low, high, band pass, band stop). Balance runs through a centre-detented curve,
bal = -1, -0.6875, -0.375, -0.125, 0, then the mirror up to +1, at dial 0, 16, 32 ... 112, 127 and straight between
(the top two dial steps stretched to reach +1). With F1, F2 the filters, x the oscillator, and g2 a gain on Filter 2:

- **Serial 4.** bal < 0: out = g2 F2(F1(x)) + 0.707 |bal| F1(x), g2 = 0.707 (1 - |bal|) - at Balance 0 Filter 1 alone at
  0.707, and Filter 2 fades in towards the centre. bal = 0: out = 0.707 F2(F1(x)). bal > 0: out = 0.707 F2(u), with
  Filter 2 fed u = (1 - bal) F1(x) + bal x - at Balance 127 Filter 2 alone on the oscillator.
- **Parallel 4.** Both filters on the oscillator: out = g2 F2(x) + m F1(x); bal < 0: m = 0.3535 + 0.1465 |bal|,
  g2 = 0.3535 (1 - |bal|); bal >= 0: m = 0.3535 (1 - bal), g2 = 0.3535 + 0.1465 bal. Each 0.3535 at the centre, 0.5
  alone at either end.
- **Serial 6.** Filter 1 is two identical sections in series (four poles in a digital mode). Their poles take 5/8 of
  the resonance; the output gain is the mode's row gain at the WHOLE resonance, on the first section only, the second
  at unity. Band pass has its own gains for this (5, 6, 8, 12, 16, 24, 32, 48, 64 of 64 by row, x 1/4 overall in unit-peak
  terms). Against reference renders, noise through every digital mode at Resonance 0-127 and two cutoffs: all within
  0.4-0.6 dB rms, as at Resonance 0. With an analog Filter 1, Serial 6 is exactly Serial 4 (reference renders of all four analog modes, Resonance 0 and
  80: identical to 0.1 dB); it used to add a resonance-free section after the ladder, which took 9-30 dB off the top
  octaves (bank C's analog patches, e.g. C007).
- **Split** runs as Parallel 4 with its own levels (0.707 at the centre) summed to mono.
- **Close cutoffs.** In the serial routings, when Filter 2's cutoff is within 2.13 dial steps of Filter 1's (4.27 in
  Serial 6), Filter 2's resonance is taken down: res2 x (1 - 0.375 (1 - apart/limit) res1/128) - to 0.625 x res2 at
  full Filter 1 resonance and the same cutoff. Its output gain stays that of the resonance before the cut (band pass
  excepted, which takes the cut one).
- **Link.** On (the default), Filter 2's cutoff is Filter 1's plus Cutoff 2 through a second curve: -1, -0.5, -0.25,
  -0.0625, 0 ... +1 of the whole dial range at the same nine points; off, Cutoff 2 is Filter 2's cutoff itself.

Cutoff 2, Resonance 2 and Balance glide as §5. Checked against reference renders (saw at C1, harmonics against the
open filter at Balance 0): Serial 4 at Balance 0, 1, 32, 64, 96, 127, with Filter 2 offset in high and band pass, and
at close cutoffs with resonance; Parallel 4 at Balance 0, 32, 64, 127; Link off - all within 0.3 dB rms below 3 kHz,
most within 0.05.



After an analog Filter 1, Filter 2 has no output gain falling with resonance, except in band pass.

A linked Filter 2 follows Filter 1's cutoff with its modulation, not only the dial.

In Split, Filter 1 takes Osc 1 and the sub, Filter 2 takes Osc 2, Osc 3 and the noise, and the ring modulator is left
out (2026-11-12, against reference renders with each source alone: Osc 3 and the noise had gone to Filter 1, and the
ring was heard where the reference has none - Filter 2 modes 1-3 under Split were up to 48 dB out on noise).

## 7. The oscillator section's levels and tuning
Against a sine of 1, a triangle is 1, a saw 2/3 and a pulse 1/2. Measured with Analog Boost off, because Init has it on.

- Osc Balance: two straight lines meeting at the centre, each oscillator at 0.8 there.
- Osc Volume, up to 64: the amplifier's sustain curve read at twice the dial. Above 64 it no longer changes the level, only the drive. It also scales the sub and Osc 3, which sit in the same group; noise and ring do not.
- Osc 3's Volume is a straight line, 1.69 of a sine at full, inside Osc Volume and outside Osc Balance. Its Slave mode adds nothing of its own. It takes Osc 2's keyfollow and Detune's curve, but not Osc 2's Semitone.
- Sub, noise and ring share one mix curve, the sustain curve to the power 1.25.
- Patch Volume: 0.2513 dB a step down to 48, then steeper (the table), and below 8 a straight line in level down to nothing at 0 - 6 dB for each halving (against the reference at 8, 2 and 1 within 0.3 dB, 2026-11-12; the table ran on towards -90 dB there, 18 dB too low at 2, and N031 Donky, its volume raised from 0 by an LFO, was silent).
- Panorama: two straight lines; a hard pan is 1.585 of the centre.
- Osc 2 Detune is piecewise and shrinks with pitch, by 1.393 per two octaves - the sounding pitch, Transpose included.
  Its table above the drift (93-127) is read off reference renders at C4 (17.7 cents at 92, 49.1 at 127); the drifting
  range below is the earlier table x 1.158, the factor the old one read low by. The table is the centre of the drift
  below (§7a); measured on single notes, it read low by whatever the drift happened to be.
- Oscillator keyfollow: 96 tracks, 64 is fixed, centred on note 60.
- Tuning: with Osc Init Phase at 0 the oscillators sit 1.12 cents flat at C4, 1.55 times less every two octaves up;
  above 0 they sit 0.31 cents sharp. Checked against long reference renders, C2 to C6, within 0.1 cents.
- A fixed Init Phase restarts the oscillators on every new note, but not on a legato one in a mono mode: there the
  reference keeps the phase running (a Mono 1 glide shows no restart).
- A gentle high pass at about 15 Hz on the oscillators, before noise and ring, so two identical pulses multiplied keep their DC.
- The output is inverted against the oscillators.
- Noise is white at Color 64, a low pass below it and a shelf away from the lows above it.


Init Phase sets Osc 2's phase against Osc 1, value/128 of a cycle, and above 0 every note starts its oscillators from there, so a run of notes is identical from one to the next. The oscillators' high pass starts clean on such a note: a voice left to keep its own would bring back whatever DC its last note ended on, and a run of notes would drift. At 0 the oscillators run free, so two at the same pitch sit at a different phase on every note, and their sum and ring product change from note to note.

What cannot be heard is not computed: an oscillator with no level unless ring or FM needs it, the FM modulator with no FM, Osc 3 with no level, the second high pass outside Split, Filter 1 in Parallel with no share. Phases run on regardless, so the sound is byte for byte the same.

## 7a. Osc 2 Detune drifts
Up to Detune 92 Osc 2's detune is not fixed: it is the dial's cents times (1 + 0.273 d), where d is a triangle of
the voice's own between -1 and 1. At each limit the triangle turns back at a new random speed, 1/2 + r/4 with r
random in ±1, a whole ramp taking 0.743 s divided by that speed (0.99 to 2.97 s). Each voice has its own triangle,
moving while the voice sounds, so two notes drift apart and a new note starts wherever its voice left off. From 93 the detune holds still. Measured at C4: Detune 64 wanders between 6.1 and 10.5 cents, 32 between 2.0
and 3.5; the hardware gave 2.1-3.7 over four notes at 32 (2026-11-08).

## 8. The live controls
Bend, pressure, the wheel and the other controllers the matrix can read. Hidden host parameters bound to their MIDI controls, so they arrive through the host's mapping.

## 9. The matrix
Random gives each note a value across the same span as the bipolar LFOs (-0.5 to 0.5): at full amount to an
oscillator's pitch, 24 notes on the reference and on ours spread alike (34 and 35 semitones standard deviation).
Six slots with up to three destinations each, and each LFO's assign. Sources are 0..1, with an LFO's bipolar source -0.5..0.5, Key Follow (note - 60) / 63, Arp Input -0.5 with the arpeggiator off, and Velocity Off full until a key is released; amounts (byte - 64) / 64. Full amount is 128 dial units, or 128 semitones of pitch: the 10% constant source moves Cutoff by 12.8. Destinations the engine lacks (the effects, for now) are summed and ignored.

## 7c. Pan and level slide across a block
A voice's pan and level are worked out once a block; the gains slide in a straight line from the last block's to this
one's across the block, as the amplifier's level does. Held still, a fast LFO into Pan stepped the gains every block and
ticked (T048, LFO 2 to Pan at full with a 9 Hz triangle, 2026-11-12). A new note starts on its own gains. The reference slides its pan smoothly too, sample by sample, whatever the Anti-alias setting (2026-11-12).

## 7b. The oscillators' Semitone; Osc 3's Detune
No oscillator's Semitone is equal-tempered: within each octave it is a just ratio - 1, 17/16, 9/8, 6/5, 5/4, 4/3, 7/5,
3/2, 8/5, 5/3, 7/4, 15/8 - with octaves exact; the table is in 1/65536ths of a semitone. Osc 1's and Osc 2's stop at 48
semitones either way (bytes 16-112). Measured on Osc 3 on every value 16-112, the same in every key; on Osc 1 and Osc 2
(2026-11-12) at -24 to +44 semitones, within 0.3 cents (+32 and +44 are 13.7 cents sharp of equal temperament, +40 13.7
flat, as the ratios say). Osc 3's Detune lowers the pitch, 46 cents at full at C4 (a table, every fourth step), shrinking
with pitch as Osc 2's does, Transpose included but not Osc 3's own Semitone. Within a cent of reference renders.

Osc 3's Detune follows the same nine-point curve as Osc 2's (0, 1/64, 1/16, 1/8, 3/16, 1/4, 13/32, 5/8, 1 across the dial
in sixteenths), scaled to 46.5 and then with pitch as before (2026-11-12; it was a measured 33-point table that ran up to
15% deep at the top). Against reference renders at notes 48 and 60: within 0.3 cent up to dial 77; above, ours is 3-7%
deeper (3.6 cents at the end at C3).

## 10. Keys
Mono modes keep the keys held and fall back to the last still down. Hold keeps notes after release, as the pedal does.
Portamento steps every 64 samples at 44.1 kHz (1.45 ms, whatever the host's rate): each step moves the pitch by a share
of what is left plus 1/32 of the whole interval the glide started with, and stops on the note. The share comes from a
nine-point table read at the dial / 16 - 1, 1/4, 1/16, 5/128, 3/128, 1/64, 1/128, 1/256, 1/512 at 0, 16 ... 128 -
straight between, so 0 is instant. The straight-line part is why a glide arrives in a finite time (0.64 s from note 84
to 60 at 96) instead of creeping in. Matched to reference pitch tracks at 32, 64, 96 and 127 within the tracker's
resolution (0.1 semitone at 50 ms), step for step against the reference's own values (2026-11-12). A voice that has never played glides from note 84, as the reference's do after loading. Which notes glide
depends on the key mode, as on the instrument: Poly, Mono 1 and Mono 3 glide every note from the last key pressed;
Mono 2 and Mono 4 only a note played while another is still held (legato), either way; a mono voice falling back to a
key still held glides back. A note that does not glide starts on its own pitch. (Checked on the hardware by ear,
2026-11-08, with the instrument's MIDI not looped back.)


Filter key follow takes Transpose into account, up and down (an upward one was left out until 2026-11-11: reference renders at Transpose -24 to +24 and keyfollow 96 and 127 now agree to within 1 Hz of spectral centroid).
## 11. What moves the cutoff
A cutoff pushed past the top of the dial by envelope, key or modulation stays at the top: on the reference every
setting from 127 up gives the same response.
- Env Amount: three slopes, 0.5, 1 and 1.25 dial units per step (`env_amount`).
- A filter envelope's attack is a straight line in time, but on its own curve: exactly 8 times the amplifier's time at 64, drawing level with it near 120 (`attack_rate`). Decay and release are the amplifier's.
- Only the amplifier's sustain is curved. A filter envelope's sustain is a straight line in its dial.
- Keyfollow: a straight line, about one dial unit per semitone at full, centred on Filter Keytrack Base.
- Velocity scales the envelope's offset, linearly in velocity, its amount through the same 9-point curve as Filter Balance: a positive amount from full at the loudest note down, a negative amount from full at the softest down.
- Velocity to Shape, Pulse Width and FM add straight lines: (byte - 64) x 2 x velocity for the first two.
- Velocity to Volume runs through the mix curve. Velocity to Resonance adds to it, through the 9-point curve of the linked Cutoff 2 offset, up to 128 dial units.

## 12. LFOs and pitch
An LFO's pitch amount follows a curve read at the amount / 8, nine points either side of zero, straight between: 0,
0.09375, 0.3125, 0.8125, 2, 4, 8, 16 and 32 semitones at amounts 64, 72 ... 128, the same downwards; 127 counts as 128,
so the top of the dial is 32. The depth is the LFO's swing times this. Within 0.7% of reference pitch tracks at every
amount from 24 to 104 (2026-11-12; the curve fitted before skipped three of the points and ran 23% deep between them).
The filter envelope's route to Osc 2 is the same curve doubled. LFO 3 reads its upper half at half the amount, a quarter
as deep (notes §12b). Trigger Phase 1..127 starts the cycle (Trigger Phase - 1)/128 of the way round from the wave's minimum (in 128ths: a square at 65 starts exactly at its half-way edge, as the reference shows, 2026-11-12). Pulse Width, Shape and FM's fixed LFO routes are straight lines, 128, 64 and full travel at full amount. Cutoff, Resonance and Panorama's run through the 9-point curve of the linked Cutoff 2 offset, 64 dial units at full. LFO 2's Cutoff 1 route moves Filter 1 alone: with Cutoff Link on, Filter 2 follows Cutoff 1's dial (and what the matrix does to it) but not this route, which is why LFO 2 to Cutoff 1 and to Cutoff 2 sound the same on two equal filters in series (2026-11-12; at full amount ours had closed both). An LFO in Env Mode always restarts with a note. A clocked LFO follows the host's tempo, always: 120 BPM until the host gives one, and the last it gave once it stops. The patch's own tempo byte is kept with the sound but not used.

Modulation reaches the three pitch depths (Filter Env to Pitch, LFO 1 to Osc 1 and to Osc 2) as it reaches any dial:
added to the dial in full-amount units (128 dial steps), rounded and held within 0-127, then read through the depth
curve - so an LFO's own Assign at amount A moves the depth dial by LFO x (A - 64). Until 2026-11-12 the modulation
was added after the dial was scaled to -1..1 and so counted half: an LFO 1 Assign at 81 onto its own Osc 2 depth with
a square LFO gave Osc 2 +11 cents where the reference gives +38 (ImpOsc02HS); now within 1 cent from +10 to +1600.

## 12a. Env Mode before Mono
An LFO set to Mono and to Env Mode behaves as a per-voice LFO in Env Mode: it starts its single cycle with each note.
On the instrument the two settings give the same sweep, within a hertz of brightness, on a held note (LFO 2 on Cutoff
2 in bank C's CuttaD6, 2026-11-08). Before this a Mono LFO ignored Env Mode and ran free, which left that sound's
filter sweep out. What a chord does with a Mono LFO in Env Mode was not checked.

## 13. Unison
A stack is at most six voices. Each stands at a place from -1 to 1, not quite evenly (four are at -1, -0.5, 0.5, 1), which sets its detune and pan, and its level: 1 at the centre, 0.75 at the edges, with no gain on the stack as a whole. Detune is 24 cents at full, 10 at 96, 4.6 at 64. Unison LFO Phase moves every second voice by (byte - 64) x 2.8125 degrees, so 64 is no offset. With an LFO both clocked and in Env Mode, every second voice of a stack holds that LFO at its top instead of running it. A Single older than the TI format (byte 0 below 12) keeps Unison in page A 97-100.

Pan Spread gives a stack voice an offset o = 2 x place x Spread / 127, at most 1 either way, so it is full from 64 up.
With p the voice's own pan (Panorama, Pan modulation and Pan Velocity together), the voice sits at p + o + |o| x p,
p limited to -1..1 in the last term and the whole to -1..1. Towards the far side the offset shrinks by 1 - |p|; towards
p's own side it grows by 1 + |p|. At Panorama 96 a two-voice stack at Spread 64 sits at the centre and hard right, at 127
both are hard right, and a deep Pan modulation carries the whole stack to either side. Against reference renders voice
by voice, from the line pairs a detuned stack splits each harmonic into: within 0.01 of a pan (2026-11-12).

## 12b. LFO 3's pitch depth
To the oscillators' pitch LFO 3's amount a gives a quarter of the pitch curve of §12 read at a / 2 above its middle:
1.2 cents at the top of the swing at 8, 50 at 64, 200 at 96, 400 at 112, 775 at 127. Against reference pitch tracks
within 0.5% up to 96; at 112 and 127 ours is 3.8% and 5.8% shallower, which LFO 1 at the same depth is not - open.
The width destinations keep the straight line.

## 12c. LFO 1 to Filter Gain
A gain on what the oscillators send the filters, from LFO 1. Its depth is a nine-point table, -0.625, -0.5, -0.375,
-0.25, 0, 0.25, 0.375, 0.5, 0.625 at amounts 0, 16 ... 128, straight between. With u the LFO moved to 0..1 (turned
over for a negative depth) and d the depth: gain = max(0, 2 (S(|d| u) + 1 - |d|) - 1), S the amplifier's sustain curve
read at 128 |d| u. At 0 depth the gain is 1; at either end it swings from silence to 0.63. Worked out once a block and
ramped across it. Against reference renders: within a dB at the loud end at every amount (C039 14 -> 3 dB). With LFO 1 in
Env Mode, whose cycle swings 0..2 from rest (tiLfo notes §2), u is half of it: 0 at rest, so once the cycle is spent
a full depth leaves silence, as the reference does (B093: it fell to -23 dB here and held there, 2026-11-12).

## 14. FM
Osc 2's frequency swings by up to 15.75 times itself at full amount, through zero. The swing is 1, 4, 8 and 16 times the frequency at amounts 32, 64, 96 and 128, a straight line between each. The modulator is Osc 1 as a positive triangle, a triangle, its wave, or noise. The triangle starts from its peak
when Osc 1 restarts: with sync on, where the modulator sits against the restart decides the fundamental, and only from
the peak does a synced Osc 2 match reference renders (20 dB too much fundamental before, C007).

## 15. Punch
A decaying offset added to the oscillators, ahead of their high pass and the filters: full size from the start, its level Punch to the power 1.6 and Osc Volume, not Balance, its sign that of the wave's first sample, taken without the correction at a wave's edge. Its time constant is 0.4 ms at note 36, 2.4 times shorter an octave up.

## 16. Analog Boost
A low shelf at the output. Intensity sets the lift, up to about 6 dB; Tune moves its corner and takes the lift down as it rises. The small dip at the very top is not reproduced. The other character types are not built.

## 17. Saturation
On Filter 1's output, before the mix, so Filter 2 on its own hears none of it. The analog modes have no saturation stage at all.

Above Osc Volume 64 the drive sets two things across nine points: the gain into the curve, 0.125 up to 1 in equal ratios, and a make-up gain out of it, its own for each curve. Curves 1-4 are arctangents, a different one each side of zero. Digital is a hard limit well below full scale. Rectifier blends from the signal to a folded form of it. Shaper is a sine of the driven signal, its level falling by a quarter across the drive. Rate Reducer (9, and 10 with key follow) follows the Distortion curve of that name (tiEffects §2), with drive as Intensity.

Bit Reducer (8) rounds to a step set over nine points of drive: the step is 1 / (16384 n) in the instrument's units, n running 1/16, 1/32, 1/64, 1/128, 1/192, 1/256, 1/384, 1/512, 1/1024 and interpolated between. Its harmonics cannot be compared closely with reference renders (rounding a saw makes them depend on the exact level and phase), but its level is within 0.6 dB.

Low Pass and High Pass (11 and 13, and 12 and 14 with key follow) are bilinear one-poles: low pass y = q (x + x[-1]) + (1 - 2q) y[-1], high pass y = (1 - q)(x - x[-1]) + (1 - 2q) y[-1]. The corner moves over the digital filters' 32 third-octave points (tiFilter notes §1), from 19.7 Hz to 22969 Hz, q = t / (1 + t) with t = tan(pi f / rate) at each point and interpolated between them. The point is 31 x control, the control 1 - drive for low pass and drive for high pass, so low pass closes and high pass opens as Osc Volume rises above 64; key follow adds (note - 60) x 0.535 / 128, about half an octave of corner per octave of note. Against reference renders, a saw at Osc Volume 72-120 and notes 48 and 72: levels within 0.7 dB and harmonics within 1 dB where the oscillator itself agrees (median error 0.95 dB, was 2.9).

## 18. The end of the chain
It clips hard at full scale. There is no limiter: a quiet note under a loud chord is not turned down, and recovers the moment the chord stops.

## 19. Arithmetic
See tiFixed.md.

## 20. Free LFOs
An LFO with Keytrigger off runs from the moment the instance starts, whether clocked or not, so a note finds it wherever it has got to and two notes struck apart agree. Its phase at a note is worked out from the time since start, rate times seconds, starting from half a cycle (tiLfo §4).

## 21. HyperSaw sync
With Osc 2 in the HyperSaw model the FM Amount, and whatever modulates it, sets a cross-oscillator sync frequency. The saws run at 2^(amount / 32) times their pitch, reaching 16 times at full, and all restart when Osc 1 does. At 0 nothing restarts. Restarting together the saws would add in step, so the stack is scaled back to the level of one. FM itself does not apply.

## 22. Where the matrix reaches
Every destination that is a setting in the sound has a route. LFO amounts add to their own amount before its curve, and a push past the end of the dial stops there, as on the instrument: 127 plus any further push sweeps exactly as 127 does (checked on LFO 2 to Cutoff 1, 2026-11-08, with the curve within 7% of the instrument's from 72 to 127). A slot's amount can be moved by another slot, so those routes are summed in a first pass and added to the amount in the second. Effects are shared by the voices, so they take the newest voice's modulation, held when nothing sounds, added to their bytes for each block.

## 22a. Destinations that re-read their dial
A matrix destination moves its parameter's dial and the parameter's own law is read again: the dial plus the sum in
full-amount units (128 steps), rounded and held within 0-127. Built so, 2026-11-12: Osc 3 Detune, LFO 1 to Filter
Gain, LFO 1 and LFO 2 Contour, Osc 1 and Osc 2 Interpolation (Smooth), Osc 1 and Osc 2 F-Shift (Formant) and F-Spread
(Sync of the grain models), Unison Detune and Pan Spread (each voice's place in the stack times the moved setting, every
block, so the stack follows the matrix while it plays), and the arpeggiator's Note Length, Swing and Pattern (from the
newest voice's matrix, as the effects take theirs). Unison LFO Phase acts when a note starts and is not moved.
The moved dial is not rounded where the law reads between table points (the pitch depths, Osc 3 Detune): the reference
hands the parameter's law the dial plus the sum unrounded (Osc 3 Detune at 64 with the 10% constant source at amounts 96
and 127 reads 70.4 and 76.8). An amount of 127 counts as full (128 steps), as on the dials. Against reference renders
with that source at full amount: Filter Gain within 0.2 dB; Osc 3 Detune exact.

## 23. Pulse width under modulation
The dial covers the upper half of the width, but modulation carries it on through the square to the narrow side, so the value is limited to -127 to 127 and not 0 to 127.

## 24. Pitch limit
An oscillator's Semi and its matrix pitch modulation are added, and the sum stops at 48 semitones either way.

## 25. Rate Reducer
In the saturation stage the hold is longer than in the Distortion stage by 3%, as measured. With key follow it halves for every octave above note 60, so the reduced rate rises with the pitch. Key follow stops at a hold of 131 samples at 48 kHz. The rate is fixed in hertz, so the hold follows the sample rate.

## 26. The filters and the envelope
The filters are set once a block, and take the envelope as it will stand at the end of that block. Taking it at the start would leave a new note's first block with the filter shut, and hold the filter too far open through a fast decay. Attack 0 needs no time at all: the envelope is at the top on the first sample.

## 27. A key struck again
Every note takes a new voice, even on a key already sounding, held or not: the note before rings out behind it, and six strikes of a held key are six voices.

## 28. Wave PWM's copies
Each oscillator keeps two more phases for Wave PWM's detuned copies (tiOsc notes §6); they start at the
oscillator's phase and are reset with it by a hard sync.

## 29. Grain and Formant
The models' k per stream comes from Formant, Sync and the oscillator's own pitch (tiOsc notes §7-§8). The Complex
ones keep a phase at half the note's rate and the two streams' Detune drift. Formant's and Sync's matrix destinations
(F-Shift, F-Spread) are not routed yet.

## 30. The rate
The instrument computes everything - voices, filters, effects - at its audio interface's rate, one of 32, 44.1, 48,
64, 88.2 and 96 kHz, and so do the voices here. `ti_voices_engine_rate()` picks it for a host rate: the host's own
if it is one of those, else the nearest of the same family (44.1 kHz or 48 kHz multiples), else the nearest at all.

## 31. The wavetable Index, ramped
The Index (Shape, with its modulation) is worked out once a block, but the wavetable models read it ramped sample by
sample from where the last block left it, as the instrument's crossfade of two frames is: a dial moving through the
frames slides instead of stepping each block. A new note starts at its own Index.

## 32. The amplifier, a block at a time
The amplifier's level reaches the output once a block (32 samples, as the filters, §4): the gain goes in a straight
line from where it stood to where the envelope will stand at the end of the block. The envelope itself still moves
every sample - it is what the matrix reads and what ends a voice. A slow envelope sounds the same either way; what
it changes is a jump. Attack 0 lands in one sample, and a key struck again in a mono mode, its envelope still high in
the release, jumped the last few dB at once: on a dark, resonant sound that is a click every note (W010, a Single in
Mono 1 with Attack 0 and Filter 1 near shut). Spread over the block, the same jump is no longer heard. Measured
2026-11-11: W010's re-strikes left a burst above 6 kHz 10-14 dB over the sound around it, now 0-2 dB; on a plain
tone through a dark filter the re-strike's burst fell from -30 to -52 dB against the note, which is what the
instrument does; the fastest release keeps its shape (within 0.03 of the instrument, sample for sample).

## 33. The arpeggiator
In modes Up to Chord the keys go to the arpeggiator (tiArp notes) and not to the voices; it starts and stops notes,
each at its own frame of the block, and the block is rendered in pieces between them. Arp>Matrix plays the keys as they
are. Turning the arpeggiator off stops what it has sounding.
