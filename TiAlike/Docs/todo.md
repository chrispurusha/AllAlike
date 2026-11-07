# TODO

One line per item. Measurements and reasoning go in `findings.md`, design in `design.md`.

## First
- Phaser: Spread between 0 and 96, the LFO's depth and shape, and matrix routes to it
- Chorus: the other types, and Feedback checked. 
- Filter Bank: VariSlope in-between Shapes, Vowel at high resonance, Vowel and shifter Stereo Phase
- Effects, in order: Chorus (11% with mix on), Character types (13%), Filterbank (12%), and last of all Delay and Reverb
- Distortion: Mint at Intensity 32 is 3.7 dB low and 7 dB out at 64; Treble Booster, High Cut and Tone
- Ring at the same pitch: level right, spectrum still out on saws and pulses
- Saturation curve 5 (Digital): about 4.5 dB hot across five Singles - check with more
- Level indications at various stages of the voice (oscillators, filters, saturation, output), to help monitor gain staging
- Fixed point, per the Arithmetic parameter (with only the biquad converted, fixed and float differ 60-110 dB below the signal on 15 of 20 Singles, 31-54 dB on Bumztik LD, RoachLead and Mushi BS - look at those first): the biquad is done. Next oscillators, envelopes, mix, saturation, the ladder. A null needs all of them, with the phases and timing
- HyperSaw: cross-oscillator sync frequency (byte 27) and its filter-envelope routes
- Oscillator models (11-02): Wave PWM, Grain and Formant built (tiOsc notes §6-§8). Open: their F-Shift / F-Spread matrix routes; Complex + Detune makes a half-frequency component 24 dB down; high Formant or Sync on rich tables keeps too many top partials (up to 2 dB)
- Wavetables 7, 35, 77: about 0.5-1 dB of content above the 64th partial is missing
- Velocity to Pulse Width and FM are built by analogy with Shape: check them
- Worst Singles still to explain: Theory, RuffBelsBC, DRY DRUM
- Oscillator start phases in a null: free-running ones are not known, and a fixed one is not yet aligned
- Serial 6 with an analog Filter 1: still the ladder then a resonance-free section - check against renders
- Saturation curves 6 and 7 are still blended measurements; Bit Reducer (8) checked by level only
- Analog Boost's top-end dip, and the other character types (13% of the banks)
- Soft knobs, the wheel-side ones, and bender scale
- Oscillator phase and drift: why free-running ones sit a cent flat
- Oscillator upper harmonics at note 72: about 2.9 dB harmonic error against the reference with the saturation curve doing nothing (Osc Volume 72); none at note 48
- Osc Volume on the panel: its upper half drives the analog filter (filter notes §5, `drive`) and the Saturation curves; then re-fit the ladder's input level (filter notes §6)
- Analog modes: self-oscillation 0.3-0.5% sharp at Cutoff 90-105 (within 0.05% from 30 to 70)
- Singles drive only what is built (tiPatch notes §3; the amp envelope's 59-63 now carried): add each section's bytes to `kCarried` as it is built - Osc 1 shape/PW (page A 17/18) with the oscillator
- Changing Sustain while a note holds: glides at 2^-12 a tick (tiEnvelope notes §6) - check against the reference
- The reference answers a note 1-1.5 ms after its sample: measure the delay and whether it steps with a block of its own
- Loading of custom wavetables, in various common file formats.
- Smooth Mode On/Auto/Note: the hardware's Cutoff step takes 2-15 ms (10-90%) - measure Off against On on a clean sound to see whether On has a glide of a few ms
- Smooth Mode Quantise: measure one held control besides Cutoff (Osc Balance or an LFO amount) and one that lands at once (Filter Mode) on the hardware

## Later
- Effects (next to last), then the arpeggiator (last, if at all) - design §4
- A Virus TI over USB (design §6): the instrument follows the instance's sound, a Single read back from it, and the instrument as a second reference through the audio interface

## Infrastructure
- A sidechain (audio) input into the voice, as the instrument's external input: send test signals straight into the filters and saturation, and play external audio through the Single
- The panel (`tiDraw.c`) is Pulse2Alike's `p2Draw.c` without the link, backup and arp steps - a SynthLib candidate; the patch bar and presets are now copies too
- Calculate rather than tabulate: tiWaves.c and tiWavetables.c ship only if no formula can be found
- Clocked LFOs: their phase against the host timeline is unchecked
- Saturation Rate Reducer at full drive, and Bit Reducer above drive 96, are still out
- Panel: named drop-downs for matrix sources and destinations, waves and other numbered choices
- Link to hardware: ROM banks cannot be written back; they can be edited locally, but any write to them must be greyed out or refused
- Hardware: the USB link could go direct through libusb, bypassing the maker's driver
- Delay: Color, modulation, Tape types, Pattern timing at other tempos, order against the EQ. Reverb. Chorus types Vintage, Hyper, Air, Vibrato, Rotary
- Reverb: top octave 4-6 dB hot, tank modulation, Feedback 1 and 2 modes, Reverb Clock
- Analog filter modes: a high-frequency floor in the reference, 20 dB higher at 44.1 kHz than at 48, not in ours
- Panel: controls with no effect yet: Delay Color, Tape, Delay LFO, Reverb Clock and Feedback, Quality, Tone (wavetable Sync, Formant, Detune and Smooth act since 11-02, on the models that use them)
- Panel: not offered: arpeggiator, vocoder and input follower, soft knobs
- Delay Clock: tempo law built but unverified on our side; the times wander at 60, 120 and 200 BPM, possibly with sample rate; sequencer patterns, whose right channel runs at the exact tempo
- Reverb Clock: acts as predelay, not built
- Wave and wavetable images above their top partial (a stored cycle's mirror images, near -65 dB)
- HyperSaw and oscillators sit 0.3 cents sharp of the reference
- Oscillators at the very top: about 6 dB down at note 108 on every model
- Stored waves alias near -50 dB, by note range; ours are cleaner. Saw and pulse are 6 dB cleaner than PolyBLEP
- Analog filter modes at high resonance; Osc 3 sine (blamed in real patches)
- Aliasing setting, per instance and held by the host, not in the sound: Faithful (default) or Clean
- Matrix: LFO 1 to Filter Gain, Unison destinations, Osc 3 Detune; effect modulation is by block
- Filterbank Frequency Shifter driven by the matrix swings too far
- Chorus: types other than Classic, sample and hold LFO shapes, the first cycle about 0.1 s ahead
- Phaser feedback at the extremes, 4-6 dB in a static test
- Filter attack within a block: a note partway through a block still sees it late
- Punch: decay by note is measured, up to about note 48; the law behind it and a second parameter are not known
- Punch: sign is opposite on Osc Init Phase 64 and on Osc 1 saw with Osc 2 near its edge; recheck since the move ahead of the high pass
- Punch into Saturation: a sine through a saturation curve differs in the first milliseconds (0.08 against 0.16)
- Comparisons by waveform as well as spectrum, for attacks
- Free-running voices on one held key can cancel: start phases need the true law
- Note start: about 10% residual in the first millisecond on ChrisTestBass, the largest error left on that sound
- Late release: up to 10% residual beyond 80 ms on ChrisTestBass
- Filter Attack 1 to 8: a smooth curve on the instrument, a single value here
- Distortion: Wave Shaper; Bounces and Folds at high Amount; Rate Reducer at Intensity 96 depends on phase
- Matrix to effects: routes built but not yet measured or heard
