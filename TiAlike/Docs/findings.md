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

## 2026-11-06 - the capture rig, and the Virus's S/PDIF low bit (`VirusTI/48k/spdif_check/`, `tools/spdif_bits.py`)
- Rig (the Intel MacBook, as Pulse2Alike's): the Virus's S/PDIF into the Fireface UC, inputs 9/10 (indices 8/9; the
  Fireface's input channel limit raised to 16), the Fireface clocked from the Virus at 48 kHz. MIDI on the port
  **"Virus TI USB Plugin I/O"**, channel 1 - "External I/O" is the Virus's MIDI socket, not the synth. The Virus's own
  USB audio (6 channels) carries no synth audio without Virus Control running: two channels sit at small fixed values
  (about -75 dBFS), the rest at 0.
- **S/PDIF silence is exact digital zero** (every word 0, both channels).
- **The least significant bit is not audio.** During a held note (-9 dBFS, 153 000 samples a channel) bit 0 is 1 on
  every positive sample and 0 on every negative one, on both channels - the sign, inverted. Bits 1-7 are ordinary
  (0.50 ones each, uncorrelated with each other); bit 0 is 0.54 ones and correlates +0.95 with itself a sample on.
  So the stream carries 23 bits: an error under one 24-bit step (about -138 dBFS) at each zero crossing, inaudible,
  but a floor for bit-exact comparisons and deep nulls - drop bit 0 (or compare to 23 bits) when nulling against the
  engine.

## 2026-11-06 - Parameter Smooth Mode on the instrument (`VirusTI/48k/smooth/`, `smooth_init/`, `tools/ti_smooth_session.py`, `ti_smooth_analyse.py`)
A held note (C4), Cutoff 30 -> 100 one second in, by CC 40 or by SysEx (page A 40), Smooth Mode (page B 25) Off and
On; first on whatever sound was loaded, then on TiAlike's Init Single sent to the edit buffer. Read as the energy
above 1.5 kHz (2 ms rms, dB), which rises 12.6 dB:
- **Off: a step.** The change lands within 2-3 ms, by CC and by SysEx, and stays.
- **On: also a step.** No glide anywhere like TiAlike's 45 ms (tiVoices notes §5, now removed). These two takes
  showed a treble overshoot (about +9 dB, ~25 ms) after the On jumps; the cleaner takes below show one after Off
  jumps too, so it is not Smooth Mode's - the filter's own transient at an instant change.
- So the reference renders (a jump in every mode) are right about the glide; they do not show the overshoot.
- The Init sound's own ripple (about every 8 ms) defeats a 10%-90% timing; the curves (smooth.png) are the result.
  To time the overshoot exactly: a sound without that ripple (one oscillator, no detune) and the MIDI's send time
  marked in the capture.

## 2026-11-06 - the Cutoff jumps again, osc 1 alone (`VirusTI/48k/smooth_osc1/`, `tools/ti_smooth2_session.py`, `ti_smooth2_analyse.py`)
Init with Osc Balance 0, Sub, Noise and both Filter Env Amounts at 0; CC Cutoff 30->100, 30->60, 100->30, Smooth Off
and On. Every jump is a step in both modes; a +10 dB treble spike right at the jump came with Smooth OFF (30->100,
30->60), so the overshoot is not the smoothing. The takes are not clean enough to measure the spike: the one-period
rms is not quite the Virus's period (its C4 is not exactly 261.63 Hz in Init), and the Init sound still moves for
hundreds of ms after the jump. Enough to settle the glide: there is none.

## 2026-11-06 - every Smooth Mode on the hardware (`VirusTI/48k/smooth_modes/`, `tools/ti_smooth_modes_session.py`, `ti_smooth_modes_analyse.py`)
Init, osc 1 alone; a CC Cutoff jump 30 -> 100 sent 1.0 or 1.3 s into a held C4, for Off, On, Auto, Note, Quantise
1/16, 1/4 and 1/1. The Init sound sweeps its own treble about every 350 ms, locked to the note, so each take is read
against two references played the same way (Cutoff 30 throughout, 100 throughout). From the 1.3 s sends (at 1.0 s the
references sit too close to read):
- Off, On, Auto, Note: landed 9-22 ms after sending, a step (10-90% in 2-15 ms).
- Quantise 1/16: 28 ms; 1/4: 245 and 305 ms; 1/1: 590 and 1340 ms - each a step (7-8 ms), each under one division at
  120 bpm (125 ms, 500 ms, 2 s): the change waits for the next division of a clock that runs free of the note.
- No mode glides. Built so (tiVoices notes §5); the offline render lands each mode at its division.


## 2026-11-06 - EQ middle band, saturation level
- The middle band's cut is narrower than its boost: half-gain widths at Q 0 / 64 / 127 of 2.25 / 0.87 / 0.29 octaves at -12 dB against 5.56 / 3.02 / 1.02 at +12, centre 633 Hz at Frequency 64 (reference renders, noise through the voice). A band pass added to the signal fits every case within 0.03 octaves; built so (tiEffects notes §1), now within 0.24 dB of the renders across Frequency, Q and Gain.
- Saturation no longer costs level: 29 Singles using it against 29 not, median level error 1.81 dB against 2.11, signed -0.55 against -0.80. Curve 5 (Digital) reads +4.5 dB over five Singles, too few to act on.
- Serial 6, noise through Filter 1 alone in each digital mode, Resonance 0/64/100/127, Cutoff 50 and 90 (32 cases): median 2.0 dB rms before, 0.46 now (tiVoices notes §6). The output gain follows the whole resonance while the poles take 5/8 of it.
- Analog 4 Pole self-oscillation against reference renders, Cutoff 20-105 at Resonance 80-127: 0.8% flat at every cutoff up to 70, whatever the resonance, levels within 0.1 dB - a coefficient factor, not the saturation. With the coefficient raised 0.82% (tiFilter notes §5): within 0.05% from 30 to 70.
- Saturation Low Pass / High Pass curves 11-14 against reference renders (saw, Osc Volume 72-120, notes 48 and 72): median harmonic error 2.9 dB before, 0.95 now; the key-follow forms were up to 3.5 dB out in level, now within 0.7 (tiVoices notes §17).

## 2026-11-09 - Bank A by ear, and A021 taken apart

CT played Bank A against the instrument; the list is in todo.md. Ten of the 30 flagged use no distortion (A012 A016
A019 A021 A048 A053 A063 A067 A071 A086), and C007 none either. Note 48, velocity 100, 2 s held, octave bands over
0.3-1.8 s, engine minus instrument after the median level offset (QU-24 inputs 23/24).
- **A076 and A064 (Rectifier) are not the distortion:** A076 3.8 dB rms as it is, 4.1 with Distortion Curve Off on
  both; A064 2.5 against 3.7. The Rectifier is still unchecked (to-test).
- **A021 is LFO 1 on osc 2's pitch** (A75 at 127, Symmetry A71 at 127). As it is 5.0 dB rms (-10.3 and -6.0 at 250
  and 500 Hz); delay send off 5.0, LFO 2's FM off 5.8, both assign slots off 8.4, LFO 1 -> osc 2 off 2.2. Symmetry 64
  7.8, amount 96 3.7, both 1.4. So the engine's pitch swing differs from the instrument's near full depth and at the
  extreme Symmetry: `lfo_pitch_amount()` at the top of its range and the Symmetry shaping, to measure by tracking
  osc 2's pitch on the instrument (osc 1 silent) at amounts 96-127 and Symmetry 0/64/127.
- **The route itself is right.** Init with osc 2 alone (Osc Balance 127), LFO 1 rate 48, osc 2's pitch tracked at
  note 60: the swing at Symmetry 64 matches at amounts 80 / 96 / 112 / 127 (+-0.3 / +-2.0 / +-8 / to +35 st), and
  LFO shape 1 (A021's) matches at Symmetry 64, 96 and 127, cycle for cycle. So A021's difference is something acting
  on that route - its LFO assigns (B79 dest 124, B81 dest 43, both at 127) and Assign 1's second destination (126):
  find what 124 and 126 are, and whether one moves the LFO 1 -> osc 2 amount, which at 127 sits on the steepest
  part of `lfo_pitch_amount()`.
- **The sine's Symmetry is wrong** (a separate fault, not A021's): on the instrument, Symmetry above 64 flattens the
  sine towards a square - held at the top and bottom for about half the cycle at 96, most of it at 112, a square at
  127, the edges still sine-shaped - and below 64 changes it little (0: a sine, slightly skewed). The engine bends
  the phase both ways (tiLfo notes §3), so at 127 it plays a slow ramp and at 0 the ramp reversed.
- Built as a sine driven into a limit above the middle, gain (1 - contour)^-0.8, plain below (tiLfo notes §3):
  osc 2's pitch track against the instrument 0.15 / 0.18 / 0.28 / 0.29 semitones rms at Symmetry 96 / 112 / 127 / 0
  (amount 96, a +-2 st swing); the phase-bent sine had gone to -11 st at 127 where the instrument stays at the top.

