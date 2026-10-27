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
