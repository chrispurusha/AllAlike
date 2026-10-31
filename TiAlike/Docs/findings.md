# Findings

Measurements, results and the traps that cost time, newest last.

## 2026-10-26 - the skeleton

- The project's infrastructure follows Pulse2Alike's: SynthLib as a submodule (its glfw and freetype submodules
  initialised, FreeType built at macOS 12.0), the build and release scripts, the licence and third-party notices,
  uncrustify, and the panel.
- First build: VST3 and Audio Unit, universal, `auval -v aumu TIal CPur` passes. Rendered offline: a five-note chord
  plays all five at Voice Limit 16 and the last two at 2; the filter's spectral centroid follows Cutoff (125 Hz at 38,
  380 Hz at 76, 3 kHz at 127 with LP 24) and Resonance lifts the band near it; the output stays finite.

## 2026-10-26 - Filter 1's digital modes

- Low pass, high pass and band pass are biquads with the cutoff on third-octave points (1 kHz at point 17), nine
  resonance rows of fixed Q (-3 to 48 dB) and a per-row output gain (code-notes/tiFilter.md).
- Against reference renders (the Init Single, oscillator 2 out, Filter Balance at Filter 1, a saw at C1, harmonics
  against the open filter): Cutoff 60 at Resonance 0, 64 and 100 within 0.2 dB from 200 Hz to 5 kHz. In the Init's
  Serial 4 routing, Filter Balance at either end gives one 12 dB/oct filter and the middle the two in series.
- The cutoff in hertz holds at a 48 kHz host: the points are fixed in hertz, not in fractions of the sample rate.

## 2026-10-26 - Filter 1's analog modes

- Analog 1-4 Pole are one four-stage ladder at twice the rate, tapped after stage 1-4, the feedback always from stage
  4, a 2x - x|x| saturator after stage 1 (code-notes/tiFilter.md §5).
- Against reference renders (Init, Filter Balance at Filter 1, a saw at C1, Cutoff 60): Resonance 0 and 64, 1-4 Pole,
  within about 0.5 dB from 100 Hz to 3 kHz; the 1-pole notch at 1 kHz reproduced. Resonance 100 and 127 self-oscillate
  at 637.8 / 634.4 Hz against 636.7 / 640.0. The resonant peak sits where the digital filter's cutoff is.
- The oscillator's level into the ladder (0.115) is fitted, not measured (§6).
- Band stop: the cookbook notch with no gain row; Cutoff 60, Resonance 0/64/120, within 0.6 dB of the reference renders
  including the notch (2026-10-26).

## 2026-10-26 - Smoothing
- With the Init Single (Parameter Smooth Mode On), a Cutoff change glides linearly in dial units over a fixed ~44 ms
  whatever its size (20->80, 40->100, 20->127 all arrive 40-47 ms after the change); with the mode Off it is immediate.
  Taken as 2048 samples at 46875 Hz. TiAlike's glide matches it in length and shape; the reference starts 5-8 ms later
  (its reaction to a CC, not part of the glide).
- So the reference does not step audibly when a knob is turned slowly: each 0.94-semitone step is glided over.

## 2026-10-26 - Filter 2, routing and Balance
- Serial 4 and Parallel 4 with Balance and Link: within 0.3 dB rms of the reference renders below 3 kHz in 14 cases
  (most within 0.05), including Balance 1 (no jump from 0) and close cutoffs at high resonance (code-notes/tiVoices.md
  §6). Serial 6 and Split are stand-ins.
- The reference's filter at a 44.1 kHz host matches its 48 kHz response within 0.5 dB (Cutoff 60, Resonance 64): its
  cutoffs are fixed in hertz, as TiAlike's are (2026-10-26).

## 2026-10-27 - Singles and presets
- Every Single in 51 `.syx` / `.mid` files (4679 of them) reads and writes back byte for byte, bar its address
  (code-notes/tiPatch.md §4). Two traps found on the way: some older patches carry 64 in a filter mode byte, and one
  name a 0x7F - both were altered by a save until bytes were rewritten only when their parameter changed.

## 2026-10-27 - the amplifier envelope
- Attack is a straight line in amplitude, Decay and Release exponential falls towards zero, all moving in ticks of
  4 samples at 46875 Hz (code-notes/tiEnvelope.md). Decay's curve does not depend on the sustain level: it stops there.
- Times against reference renders within 1%: Attack 64 87.4 ms (87.8 measured), Release 64 349.5 ms (349.4), Release
  127 8.98 s (8.96). Sustain 64 at 0.33 of full. Sustain Time below the centre an exponential fall, above it a
  straight rise to 0.99.
- Trap: the Init Single's two oscillators are detuned, so their beat swamps an envelope measurement - oscillator 2 out
  (Osc Balance 0) first.
- Notes now start at their own sample within a block (they started at the block's start, up to a block early, which
  made fast attacks and releases look wrong by several dB). The reference itself still answers a note about 1-1.5 ms
  after its sample.

## 2026-10-30 - the voice
- The engine now reads the whole Single: two oscillators and Osc 3, sub, noise, ring, sync and FM, the filter envelope, keyfollow, velocity, three LFOs, the matrix, unison, the mono modes, portamento, stereo, Analog Boost, Punch, saturation, Serial 6 and Split (code-notes/tiVoices.md §7-18).
- The oscillator section against reference renders: waves, shapes, balance, sub, volumes within about 0.1 dB.
- Trap: Init has Analog Boost on, so a saw's harmonics do not fall as 1/k until it is off, and every level measured with it on was coloured.
- Trap: arp and effects must be off before comparing, in both of a patch's locations. Older Singles keep Arp Mode at page B 1, and an arp left running looks like a broken amplifier.
- Free-running LFOs depend on how long the reference has been running, so a patch using one cannot match in time.
- There is no limiter: the end of the chain clips at full scale (tiVoices §18).
- Across 24 varied Singles, arp and effects off: median 4.1 dB in level and 7.1 dB in spectrum over a note (the Init is 0.5 and 3.5). Every Single in 50 files still reads and writes back byte for byte, 240 Singles rendered with nothing unstable, and 32 voices cost 4.2% of one core on an M4.

## 2026-10-30 - later
- A pure sine: pitch within 0.0002 Hz, level within 0.013 dB. The reference's own sine carries a floor at -72 dB, which is as deep as a null can go on it.
- The filter envelope's attack is on its own timing, several times the amplifier's. An LFO in Env Mode restarts with every note. Clocked LFOs follow the patch's tempo with no host.
- 24 varied Singles: median 3.0 dB in level, 6.8 dB in spectrum; a fresh 16: 1.95 and 6.7. Two of them, each time, within 1.5 and 3.

## 2026-10-30 - cutoff, voice by voice
- A voice's cutoff compared with the reference's across 40 varied Singles: 29 within 2 dial units, median 1.0. Across 24 Singles, 2.65 dB in level and 6.47 dB in spectrum.
- Env Amount has slopes 0.5, 1 and 1.25. An LFO's bipolar matrix source and assign are half what was first built. Velocity's amount and LFO 2's Cutoff routes run through 9-point curves. Unison LFO Phase moves every second voice; 64 is none.

## 2026-10-30 - saturation
- Curves 1-5 against reference renders at Osc Volume 64, 100 and 127, saw and sine: curves 1-4 within 0.1 to 0.8 dB across the harmonics, Digital within 2.2 dB. Total error across the cases 27.8 to 7.4.
- Across 24 Singles: spectrum 6.18 dB, level 3.02 dB.
- The panel grew to 117 parameters and a taller window: Osc 3, the models, LFO 3, Envelopes 3 and 4, Unison and the six matrix slots.

## 2026-10-30 - HyperSaw
- Against reference renders at notes 36, 60 and 84: every saw within about half a cent of the reference's, and levels within a few percent. Across 20 Singles using it: median 2.68 dB in level, 6.86 dB in spectrum, none yet within 1.5 and 3.

## 2026-10-31 - levels
- One scale explains the saturation curves, the ladder and its self-oscillation: a voice's sine of 1 at 1 / (4 x root 2) of full scale. Analog modes against reference renders, 28 cases of cutoff, resonance and drive: total error 48.4 down to 7.3.
- Osc 3's Volume is a straight line. A Unison stack has no gain of its own: each voice's level follows its place. Older Singles keep the Ring Modulator at page A 38.
- Wavetable: 11 table and Index cases within 0.0 to 0.85 dB, including points between the 16 measured positions.
- Across 70 Singles: signed level error about -0.15 dB, and median 1.76 dB in level and 5.55 in spectrum; on fresh samples 2.2 and 4.9 to 5.8, with up to 5 of 29 within 1.5 and 3.

## 2026-10-31 - EQ and FM
- FM depth has four slopes, reaching 15 times the frequency, through zero. The high pass sits on the oscillators. Across 70 Singles: level 1.55 dB, spectrum 5.01, signed level 0.0.
- EQ against reference renders: shelves within about 0.6 dB, mid boosts within about 0.8. Mid cuts are too wide.

