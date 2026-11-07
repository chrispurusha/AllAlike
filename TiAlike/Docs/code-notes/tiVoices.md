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
  0.4-0.6 dB rms, as at Resonance 0. The analog modes run as before (the ladder, then a resonance-free section).
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
## 7. The oscillator section's levels and tuning
Against a sine of 1, a triangle is 1, a saw 2/3 and a pulse 1/2. Measured with Analog Boost off, because Init has it on.

- Osc Balance: two straight lines meeting at the centre, each oscillator at 0.8 there.
- Osc Volume, up to 64: the amplifier's sustain curve read at twice the dial. Above 64 it no longer changes the level, only the drive. It also scales the sub and Osc 3, which sit in the same group; noise and ring do not.
- Osc 3's Volume is a straight line, 1.69 of a sine at full, inside Osc Volume and outside Osc Balance. Its Slave mode adds nothing of its own. It takes Osc 2's keyfollow and Detune's curve, but not Osc 2's Semitone.
- Sub, noise and ring share one mix curve, the sustain curve to the power 1.25.
- Patch Volume: 0.2513 dB a step down to 48, then steeper (the table).
- Panorama: two straight lines; a hard pan is 1.585 of the centre.
- Osc 2 Detune is piecewise and shrinks with pitch, by 1.393 per two octaves.
- Oscillator keyfollow: 96 tracks, 64 is fixed, centred on note 60.
- Tuning: with Osc Init Phase at 0 the oscillators sit about a cent flat, by a little less as pitch rises; above 0 they sit 0.29 cents sharp.
- A gentle high pass at about 15 Hz on the oscillators, before noise and ring, so two identical pulses multiplied keep their DC.
- The output is inverted against the oscillators.
- Noise is white at Color 64, a low pass below it and a shelf away from the lows above it.


Init Phase sets Osc 2's phase against Osc 1, value/128 of a cycle, and above 0 every note starts its oscillators from there, so a run of notes is identical from one to the next. The oscillators' high pass starts clean on such a note: a voice left to keep its own would bring back whatever DC its last note ended on, and a run of notes would drift. At 0 the oscillators run free, so two at the same pitch sit at a different phase on every note, and their sum and ring product change from note to note.

What cannot be heard is not computed: an oscillator with no level unless ring or FM needs it, the FM modulator with no FM, Osc 3 with no level, the second high pass outside Split, Filter 1 in Parallel with no share. Phases run on regardless, so the sound is byte for byte the same.
## 8. The live controls
Bend, pressure, the wheel and the other controllers the matrix can read. Hidden host parameters bound to their MIDI controls, so they arrive through the host's mapping.

## 9. The matrix
Six slots with up to three destinations each, and each LFO's assign. Sources are 0..1, with an LFO's bipolar source -0.5..0.5, Key Follow (note - 60) / 63, Arp Input -0.5 with the arpeggiator off, and Velocity Off full until a key is released; amounts (byte - 64) / 64. Full amount is 128 dial units, or 128 semitones of pitch: the 10% constant source moves Cutoff by 12.8. Destinations the engine lacks (the effects, for now) are summed and ignored.

## 10. Keys
Mono modes keep the keys held and fall back to the last still down. Hold keeps notes after release, as the pedal does. Portamento is an exponential approach: its time constant is 5.65 ms times 3 to the power dial / 32.


Filter key follow takes a downward Transpose into account, and not an upward one.
## 11. What moves the cutoff
- Env Amount: three slopes, 0.5, 1 and 1.25 dial units per step (`env_amount`).
- A filter envelope's attack is a straight line in time, but on its own curve: exactly 8 times the amplifier's time at 64, drawing level with it near 120 (`attack_rate`). Decay and release are the amplifier's.
- Only the amplifier's sustain is curved. A filter envelope's sustain is a straight line in its dial.
- Keyfollow: a straight line, about one dial unit per semitone at full, centred on Filter Keytrack Base.
- Velocity scales the envelope's offset, linearly in velocity, its amount through the same 9-point curve as Filter Balance: a positive amount from full at the loudest note down, a negative amount from full at the softest down.
- Velocity to Shape, Pulse Width and FM add straight lines: (byte - 64) x 2 x velocity for the first two.
- Velocity to Volume runs through the mix curve. Velocity to Resonance adds to it, through the 9-point curve of the linked Cutoff 2 offset, up to 128 dial units.

## 12. LFOs and pitch
An LFO's pitch amount follows a steep curve, the same either side of zero, 2 semitones at half amount and 31 at full (`lfo_pitch_amount`). The filter envelope's route to Osc 2 is the same curve doubled. LFO 3 goes as the fourth power of its amount, 8 semitones at full. Trigger Phase 1..127 runs one cycle from the wave's minimum. Pulse Width, Shape and FM's fixed LFO routes are straight lines, 128, 64 and full travel at full amount. Cutoff, Resonance and Panorama's run through the 9-point curve of the linked Cutoff 2 offset, 64 dial units at full. An LFO in Env Mode always restarts with a note. A clocked LFO follows the host's tempo, or the patch's own (byte + 63 bpm) with none.

## 13. Unison
A stack is at most six voices. Each stands at a place from -1 to 1, not quite evenly (four are at -1, -0.5, 0.5, 1), which sets its detune and pan, and its level: 1 at the centre, 0.75 at the edges, with no gain on the stack as a whole. Detune is 24 cents at full, 10 at 96, 4.6 at 64. Unison LFO Phase moves every second voice by (byte - 64) x 2.8125 degrees, so 64 is no offset. With an LFO both clocked and in Env Mode, every second voice of a stack holds that LFO at its top instead of running it. A Single older than the TI format (byte 0 below 12) keeps Unison in page A 97-100.

## 14. FM
Osc 2's frequency swings by up to 15.75 times itself at full amount, through zero. The swing is 1, 4, 8 and 16 times the frequency at amounts 32, 64, 96 and 128, a straight line between each. The modulator is Osc 1 as a positive triangle, a triangle, its wave, or noise.

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
An LFO with Keytrigger off runs from the moment the instance starts, whether clocked or not, so a note finds it wherever it has got to and two notes struck apart agree. Its phase at a note is worked out from the time since start, rate times seconds.

## 21. HyperSaw sync
With Osc 2 in the HyperSaw model the FM Amount, and whatever modulates it, sets a cross-oscillator sync frequency. The saws run at 2^(amount / 32) times their pitch, reaching 16 times at full, and all restart when Osc 1 does. At 0 nothing restarts. Restarting together the saws would add in step, so the stack is scaled back to the level of one. FM itself does not apply.

## 22. Where the matrix reaches
Every destination that is a setting in the sound has a route. LFO amounts add to their own amount before its curve. A slot's amount can be moved by another slot, so those routes are summed in a first pass and added to the amount in the second. Effects are shared by the voices, so they take the newest voice's modulation, held when nothing sounds, added to their bytes for each block.

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
