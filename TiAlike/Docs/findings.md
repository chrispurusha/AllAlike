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

## 2026-11-09 - A006's delay: Pattern 5+3 on the instrument

A006 uses Delay Mode 20, Pattern 5+3. Patterns 2+5 to 5+5 (modes 12-22) had not been built and played as 1+1;
now built (tiEffects notes §8). Against the instrument, a 0.08 s note at 60, echo onsets in sixteenths at the patch's
own tempo, 135 BPM (the instrument alone runs at the patch tempo; the plug-in always at the host's):
- Left: 0, 2.9, 5.0, 7.9, 10.0, 12.9, 15.3, 17.9 - Right: 0, 3.0, 5.5, 8.0, 13.0 (the right's after 3 22 dB down).
- Levels: Right 3 at -33.5 dB; Left 5 at -51.6, 8 at -43.1, 10 at -56.3, 13 at -47.8 (the dry note is on the left only).
- So the 5 tap recirculates on the left (5, 10, 15) and the right's 3 echo is fed into that loop (8, 13, 18), but not
  back into the right. The engine reads one line at both taps, the left recirculating, so the right hears 3, 8, 13 and
  the left only 5, 10 - the shape to rebuild: the right a single 3-sixteenth tap of the input, its output into the left's
  5-sixteenth loop. Check the rule on a second pattern (e.g. 2+5, 4+3) before building it.

## 2026-11-11 - W010's ticks: the amplifier, a block at a time

W010 (At@riKart, Mono 1, Attack 0, Filter 1 near shut with resonance 125) ticked on every key struck again while the
last note was still releasing. The instrument does not. Narrowed by switching parts off in turn: not the effects,
noise, LFOs, matrix or saturation; it went with Poly, with a slower amp Attack, or with the amplifier's restart
skipped. The envelope does the same thing on both - back to full on the new note - but ours put that jump into the
output within one sample; the instrument carries the amplifier's level to the output once a block and goes there in a
straight line. Built (voices notes §32), the 32-sample block the filters use. Against the instrument:
- W010's re-strikes: the burst above 6 kHz, 10-14 dB over the sound around it, is now 0-2 dB (none on the instrument).
- A plain tone through a dark filter, struck again from a low sustain: the re-strike's burst -30 / -33 dB against the
  note before, -53 / -50 now; the instrument -60 / -51. Shorter glides fell short (8 samples -40 / -36, 16 -44 / -40).
- Release 0's fall keeps its shape: within 0.026 of the instrument sample for sample (0.033 before).
- Onsets of a factory bank's first 16 moved towards the instrument where they moved at all (high band in the first
  3 ms: Atx Bass -16.3 -> -29.8, instrument -26.0; Bad Pop -14.7 -> -19.9, -19.0; Bomber -33.3 -> -39.5, -38.2). Held
  notes unchanged within 0.1 dB except content 50 dB and more below the note.

## 2026-11-11 - The amplifier envelope in the instrument's arithmetic

A sweep of every page A and B parameter against the reference (one at a time on a plain sound, compared cycle by
cycle) left the amp envelope's Sustain Time worst: at 0 ours 12 dB loud, at 32 its fall 150 ms late. The cause was the
sustain level - 2% low at the top of the dial, full scale on the instrument at 127 - so a Decay 127 sound took 150 ms
to reach it before Sustain Time could begin. Rather than fit the curve again, the envelope now runs on the
instrument's own arithmetic in Fixed mode (tiEnvelope notes §7): 48-bit level, 16-sample updates, the curves' points,
and the same rounding; every rate and every sustain level is the same word as the reference's, and a whole note's
level matches at every update. Sweep of A59-A63 after: worst level difference 12.2 -> 2.2 dB (Sustain Time 0, where
both sit at the floor), Sustain Time 32 falling with the reference from the first frame.
The filter's envelope and envelopes 3 and 4 share the update and the Decay, Release and Sustain Time rates word for word;
their Attack reads the fall curve (about eight times slower than the amplifier's at 64), their sustain is a straight
line, and they start at 0 and rise to full scale. All four now run on it in Fixed mode; filter-envelope Attack 0 went
from 6.3 dB of band error against the reference to 0.5.

## 2026-11-11 - Bank C's analog patches; pitch laws; FM under sync

Bank C's Anaklav patches (C005-C007) sounded dull against the instrument. Switching stages off one at a time on both
sides found it: with an analog Filter 1, Serial 6 routing added a resonance-free section after the ladder, 9-30 dB off
the top octaves, where on the reference Serial 6 is exactly Serial 4 (tiVoices notes §6). C007 went from 6.2 to 2.2 dB
mean error across octave bands, C005 7.4 -> 3.3. Not a regression: the last committed engine renders them the same.
Also from the run-through, each checked against reference renders: Osc 3's Semitone is a just-intonation table and
its Detune lowers the pitch (§7b); both detunes shrink with Transpose; Osc 2's detune table read 13-16% low; the
free-running tuning offset (§7); portamento's time constant above 64 and the first glide from note 84 (§10); LFO 3's
pitch depth curve (§12b); the HyperSaw's spacing at small Spread and its tuning (tiOsc §4); FM's triangle modulator
starts from its peak, which fixes a synced Osc 2 with FM (§14); a legato mono note keeps its phase (§7).
Measurement trap: the reference renderer's output holds short runs of exact zeros; any window longer than ~80 ms
must skip them, or a plain saw reads 10 dB wrong in places.
Still open: a synced Osc 2 below Osc 1 whose pulse edge falls near where the restart cuts it (C007 at Detune 0) -
where the reference restarts the slave against the pulse's edge is not settled.

## 2026-11-11 (night) - bank C ranked; Filter Gain built; Random's span; cutoffs stop at the top

All 128 of bank C against the reference, effects and arpeggiator off: median 3.4 dB per octave band. The worst, by
reverting one setting at a time: the arpeggiator (not built), LFO 1 to Filter Gain (not built - now built, notes
§12c; C039 14 -> 3 dB), Random twice the bipolar sources' span (notes §9), a cutoff carried past 127 (§11). Still apart:
C052 Flangepad (Filter 2 high pass with a strong envelope), C029 CrstlCave (Split, low cutoff), C074 (FM mode 3).

## 2026-11-11 (late night) - The arpeggiator

Built from the instrument's own: the note choice of each mode, the 63 preset patterns (each a list of steps with a
length, duration and velocity), the 96-a-quarter clock and the Arp Clock divisions, Note Length, Swing and the step
velocity rule (tiArp notes). Against the reference plug-in: Up, Down, Up&Down, As Played over chords played in and out
of order and one to four octaves, note for note; 63 of 64 patterns onset for onset within 6 ms; swing at 0, 64 and 127
and six clock divisions within 2 ms; the velocity contour of a crescendo pattern step for step. A pattern starts from
its first step on the clock's next tick after the first key (renders that play a note straight after loading seemed
to start from the second step - an artefact of that). Bank C's C057 now plays its arpeggio as the reference does.
Checked again with every key at its own time: the first step plays at the first key (keys 1 ms apart miss it),
and all 64 patterns, pattern 59 included, match onset for onset. Open: old-format Singles' user pattern (C062) and
C117's chord mode.
Also: the amplifier's level now moves sample by sample between envelope updates, as the reference's (tiEnvelope §7).

## 2026-11-11 (late night) - LFO rate law and the free-running start

The LFO rate now follows the instrument's own law (the envelope's fall curve, tiLfo §1), within 1% of reference renders
from 48 to 104 where the old table was out by up to 4.4%. A free-running LFO starts half a cycle in at load (tiLfo §4).
With the reference renders' timing matched (they play the note just after loading), the LFO section of the sweep
halved its mean error (4.8 -> 2.4); LFO 2 to Cutoff at 0 went from 90 to 16, LFO 1 to Osc 1 at 127 from 40 to 8. The
bank-ranking entry A035 (LFO 2 to Cutoff 1) was this timing, not the instrument.

## 2026-11-11 (night) - Filter keyfollow takes an upward Transpose too

The bank-A blame on A062 led to it: keyfollow ignored an upward Transpose (notes §10 said so). Reference renders at
Transpose -24 to +24 with keyfollow 96 and 127 now agree within 1 Hz of spectral centroid; A062 28.8 -> 8.1.

## 2026-11-12 - Pitch: the LFO curve, just Semitones, Portamento's law

A fresh parameter sweep on the current build, then the voice rows worst first:
- **LFO pitch curve** (Osc 1/2 LFO 1 Amount, Filter Env to Osc 2): nine points either side of zero, 32 semitones at
  full (notes §12). Pitch depth against reference renders within 0.7% at amounts 24-104, where the old curve was 23%
  deep at 24 and 104. LFO 3 to pitch reads the same curve (§12b): within 0.5% to 96, a few percent shallow above (open).
- **Osc 1 and Osc 2 Semitone are just intervals**, like Osc 3's (§7b): +32 and +44 semitones 13.7 cents sharp, +40 flat,
  now within 0.3 cents; the dial stops at +-48.
- **Portamento** (§10): a share of the gap plus 1/32 of the interval every 1.45 ms, so a glide lands on its note; the
  old exponential crept in (0.3 semitones out at 0.7 s at 96, where the reference has arrived).
- Rows that are the sweep's own measure, not a difference: Osc 1 Semitone at 112 and Portamento at 127 (the harmonic
  analysis assumes the base note's pitch), Osc 1 LFO 1 Amount at 0 (a 64-semitone swing).
- Still open from the sweep: Amp Sustain Time 0 (spectrum, not level), Ring at 95 (+3.9 dB), Transpose 0, Osc Balance 64,
  Cutoff 1 LFO 2 Amount at full (the curve is right; how the modulation lands on the cutoff is not), Cutoff 0 (ours
  silent, the reference sounds), then the effects.

## 2026-11-12 (later) - Cutoff 1's LFO route and the link; what the rest of the sweep was

- **LFO 2 to Cutoff 1 no longer reaches Filter 2 through the link** (voices notes §12). At full amount ours closed both
  filters of a Serial pair, 15 dB too deep at the bottom of the swing; now the same as LFO 2 to Cutoff 2, as the
  reference has it (envelope score 5.4 -> 1.8, bands 5.5 -> 0.2).
- **Ring and Osc Balance rows were Osc 2's detune drift.** The sweep's base Single has Osc 2 Detune 32, whose random
  wander (notes §7a) moves the two oscillators' phase apart differently on each side. With Detune 0 Ring matches at
  every amount (level 0.0-0.1 dB, bands at most 0.6) and Osc Balance 64 too (harmonics 0.2). The static Detune law is
  the reference's to within 1%. Two-oscillator checks need Detune 0 (validation.md).
- **Cutoff 10 and below, Sustain Time 0**: both instruments end in fixed-point residue there, 74 dB or more below the
  normal level - the filter collapses at the same dial on both sides, but the reference keeps a trace of the harmonics
  (ours a little DC), and an envelope held at its floor leaves the reference a DC of about -83 dB. Left for the fixed
  point work.
- **Transpose 0** puts the note near 6.5 Hz; the frames are the onset finder landing on different cycles.

## 2026-11-12 Anti-alias switch; the classic oscillator's own way of working
The classic oscillator, as the instrument makes it: a whole number of points per cycle chosen by note, the
classic mix unfiltered at each point, a five-point filter and a straight-line interpolation at two passes a sample,
the second heard (tiOsc notes §9). That is the Global Anti-alias switch's Off position, matching the reference's
oscillator output line for line at C7. On, the default, keeps the band-limited oscillators. The same work gave
the Shape crossfade's exact law (tiOsc notes §1: no setting is a pure saw). Trap: the reference's final audio is a
poor place to measure alias lines - its pitch estimate moves them out of the search window at the 40th harmonic and
its capture has its own sidebands; measure the oscillator's own output instead.

## 2026-11-12 Pitch depths under modulation
ImpOsc02HS (A016) sounded like a detuned unison on the instrument and not here: its LFO 1 Assign drives LFO 1's own
depth to Osc 2 Pitch, and with a square LFO that leaves Osc 2 a constant +38 cents above Osc 1. Ours gave +11: the
three pitch depths took their modulation at half strength (tiVoices notes §12). Now the same to a cent at every
amount, and ImpOsc02HS's two oscillators sit where the reference's do.

## 2026-11-12 Split routing
P@rtials 1 (A012) sounded wildly different on the instrument, around its noise. Cut down to its fault it was noise,
Filter 2 in mode 2 and Split routing. In Split the instrument sends Osc 3 and the noise through Filter 2 with Osc 2, and
carries no ring; ours sent both through Filter 1 and kept the ring. Fixed (tiVoices notes §6).

## 2026-11-12 Unison Pan Spread; delay modulation

- T048 ticked harder on the reference. Its pan is not stepped there - it moves smoothly, sample by sample - but it swings
  further: a two-voice stack with Pan Spread 97 kept the whole ±33 dB swing of LFO 2 to Pan, where ours fell to ±11 dB.
  Measured voice by voice (a detuned stack splits each harmonic into a pair of lines, one per voice), the offset is
  2 x place x Spread / 127, full from 64, and a voice sits at p + o + |o| p: towards the far side the offset shrinks as
  the pan moves off centre, towards its own side it grows. Every voice within 0.007 of a pan (tiVoices notes §13).
- Delay modulation, from the pitch track of a sine's echo (wet only): the voices' LFO at half their rate, the time
  swinging by 0.104 of itself at full Depth, the two sides in antiphase. The earlier note of 4.2 Hz at Rate 50 was wrong:
  it is 0.29 Hz (tiEffects notes §8a).
- A preset change while a note sounds (A061 -> A112): no clip since the voices stop at the change and the lamp counts
  only once the new sound is in (CT, confirmed).

- User wavetables (tiOsc notes §5a, tiUserTable notes): the PPG WAVs carry no cycle mark and are 22.05 kHz files of
  64 or 128 cycles of 256 - an autocorrelation finds 256 or a high harmonic of it, never 2048, though 16384 divides by
  2048 too. The TI2 and Massive sets are Serum-style, `<!>2048` in a `clm ` chunk. A table rebuilt from what a project
  stores matched the loaded one to -55 dB until the top partial was kept below the 512-sample cycle's Nyquist; since,
  to -95 dB (16-bit). A lone HyperSaw saw with Anti-alias off, made on points of its own as the classic saw is: -46.6
  and -42.9 dB between the harmonics at C6 and C7 against the reference's -46.5 and -43.2.
- Bank re-ranking (2026-11-12): two noise floors no longer score as a difference (a window where both sides sit below
  -70 dBFS is left out); A026's 44 dB was one. Three real faults it found: LFO 1 in Env Mode reads Filter Gain from
  its rest, so a spent cycle leaves silence (B093 held on at -23 dB); Key Mode Hold turns the arpeggiator off (C074
  and C117 re-attacked every step); Mint Overdrive comes out at twice its old level (5-7 dB low at every Intensity
  and input). The distortion survey, one oscillator at two input levels and two Intensities, left Bit Reducer,
  Rectifier, Saffron and Onion apart (todo).
- All 23 banks ranked (2026-11-12): medians 2.5-4.7 dB a window. The worst were ours falling silent where the
  instrument plays on. The arpeggiator's user pattern counts a step's length in 64ths of the step, 64 the whole step
  and 127 a tie to the next step that plays (read as 127ths every note stopped halfway: W115, P006); an LFO in Env
  Mode runs one whole cycle from its Keytrigger phase and then rests at the value it started from, not at zero
  (K030; through Filter Gain, saw and square at every phase within 0.1 dB); Keytrigger places the start in 128ths of
  a cycle (126ths put a square started at 65 on the wrong side of its edge). The Rectifier's fold left an offset
  that read as 7-20 dB of level on a quiet input; it now keeps the high-pass.
- Second pass over all banks (2026-11-12): a clocked LFO in Env Mode is not doubled in speed (U076, 42 -> 2); Patch
  Volume below 8 is a straight line in level (it fell towards -90 dB, 18 dB low at 2). The ranking judges a silent
  side by its level: a window the reference has a dropout across had been scored as silence (a dozen arpeggiated
  Singles at the maximum). Filter Gain does act on noise, as here.
- Reverb Predelay and Clock, Tape delay (2026-11-12): Predelay was a third of the instrument's (1.875 ms a step, not
  5.461); Reverb Clock is Delay Clock's note values at the tempo as it is, folded below 533 ms. Tape takes its own
  step (5.461 ms), Feedback / 64 a trip with a limit above 64, Bandwidth a band closing on 1 kHz, and no delay
  modulation. Trap: a short note cannot measure long times - the reference stops processing about 0.4 s after its
  voices end, which first looked like a 350 ms ceiling on both.

