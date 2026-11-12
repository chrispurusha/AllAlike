# TODO

One line per item. Measurements and reasoning go in `findings.md`, design in `design.md`.

## First
- Spitter (C110), worse since the matrix reaches the pitch depths at full strength (correct in isolation): cut down, it is Portamento 26 + Cutoff 26 + Velocity into LFO 1's Osc 1 depth; the vibrato tracks the reference, but ours is about 8.7 dB louder - look at the filter's key tracking during a glide. Also LFO 1 at rate 48 drifts a few percent from the reference over 2 s. Phobos (C094) +5.6 from another cause (LFO clocks, Env Mode)
- Sync: the instrument's restart runs about one point late, leaving faint lines at every Osc 1 harmonic at whole-number ratios (-38 dB two octaves up); ours has none. Both Anti-alias modes otherwise done (tiOsc notes §2a, §9)
- Filter 2 band pass at Cutoff 127 (linked): about 5 dB louder than the reference (within 0.5 dB at 64-115); Dgtalrido (C033) still 14.9 after the Split fix - recheck at its own settings. Wired (C127) reduces to four LFO 2 settings (LFO 2 to Osc Shape)
- Anti-alias Off: the HyperSaw still about 10 dB cleaner than the instrument (between a box-filtered and a band-limited saw); Osc 3 and the sub within 1-3.5 dB since 2026-11-12; FM mixed
- Arpeggiator built 2026-11-11 (tiArp notes): old-format Singles' user pattern (C062), C117 chord mode, Arp>Matrix's matrix source, the host song position as the pattern's phase while playing
- Blame tool: longer windows (several seconds) for evolving patches before trusting a short-window score
- Bank C still apart from the reference after the Serial 6 fix: C001 (analog 4-pole, top octaves 15-30 dB dark), C009 AprilPad, C003 Aeternal, C008 AnlogFeel
- Synced Osc 2 below Osc 1 with a pulse edge near the restart (C007 at Detune 0): settle where the slave restarts against the edge
- FM into a Wavetable Osc 2, and Classic at full amount: ours goes inharmonic where the reference stays on the note
- Grain/Formant models: sweep their own controls against the reference (Shape and Width do not drive them)
- Matrix push past a dial's end: the instrument caps it at the end too (voices notes §22) - DO NOT lift. Env Mode before Mono built 2026-11-08 (voices notes §12a; CuttaD6 gap 11.4/27.0 -> 3.3/10.0 dB). Next against the instrument, bank C: AnlogBD's tail (23.5 dB late), Acidica's LFOs (22.7 early), 3-Dumm's tail (15.0), Bombers' effects
- Osc 2 Detune drift built 2026-11-08 (voices notes §7a: a per-voice triangle, ±27%, up to Detune 92): check by ear against the hardware with two oscillators at equal level
- Portamento by key mode built 2026-11-08 (Poly/Mono 1/Mono 3 from the last key, Mono 2/Mono 4 legato only; voices notes §10) - every mode matches the reference separated and legato; its time law is the reference's since 2026-11-12 (a share of the gap plus 1/32 of the interval per step): check a legato glide that starts mid-glide
- Smooth Mode On glides Shape (Wavetable Index) changes: a dial swept through a table is smoother on the instrument (5 dB less between the harmonics with On than Off, where ours matches Off). Find which controls it glides, and the rate per step (one-step changes did not resolve against noise; try a high note, On against Off)
- Reference caveat: the reference is buggy when stepping through presets - program changes and anything carried across a change of sound are checked on the hardware Virus TI, not the reference
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
- Oscillator models (11-02): Wave PWM, Grain and Formant built (tiOsc notes §6-§8). Open: their F-Shift / F-Spread matrix routes; high Formant or Sync on rich tables keeps too many top partials (up to 2 dB) (Complex + Detune's half-frequency energy matches the instrument within about 2 dB on models 5 and 7, Detune 0-127 - checked 2026-11-08)
- Wavetables (osc notes §5, the instrument's frames, versions and read built 2026-11-08): still out - between frames the instrument mixes a little of the next frame where Smooth 127 should hold one alone (table 50 frame 2 carries about 6% of frame 3), which looks like the Index-to-frame weighting; table 34 off at C2 and C5 (-7.7 dB); 88 at C2 and 1 at C6 on a few partials; the 8-, 4- and 2-partial versions are not exact truncations on 11% of frames; table 0 still measured
- Velocity to FM (B50): follows the instrument (spectral centroid with B50 96 within 4%), about 15% short at B50 127 - the FM depth near full. Velocity to Pulse Width (B49) matches within 0.1 dB (checked 2026-11-08). With Osc Balance 64 and FM 0, ours is about 4% darker than the instrument on a saw pair at C3
- Worst Singles: Theory was mostly the LFO rate law below 48 (ours up to twice too fast; fixed 2026-11-08, lfo notes §1) and a free-running LFO's phase at note on (not knowable); error 18.5 -> 3.8 without it); RuffBelsBC the HyperSaw cross-oscillator sync (A27); DRY DRUM is the error measure, not the sound - a 46 Hz sine leaves the upper bands to the instrument's noise floor (about 78 dB down; ours about 102), which the band score counts
- Oscillator start phases in a null: free-running ones are not known, and a fixed one is not yet aligned
- Serial 6 with an analog Filter 1: still the ladder then a resonance-free section - check against renders
- Saturation curves 6 and 7 are still blended measurements; Bit Reducer (8) checked by level only
- Analog Boost's top-end dip, and the other character types (13% of the banks)
- Soft knobs, the wheel-side ones, and bender scale
- Oscillator phase and drift: why free-running ones sit a cent flat
- Oscillator upper harmonics at note 72: about 2.9 dB harmonic error against the reference with the saturation curve doing nothing (Osc Volume 72); none at note 48
- Osc Volume above 64 drives the analog filter and the Saturation curves (built; the matrix moves the drive too since 2026-11-12): re-fit the ladder's input level (filter notes §6)
- Analog modes: self-oscillation 0.3-0.5% sharp at Cutoff 90-105 (within 0.05% from 30 to 70)
- Singles drive only what is built (tiPatch notes §3; the amp envelope's 59-63 now carried): add each section's bytes to `kCarried` as it is built - Osc 1 shape/PW (page A 17/18) with the oscillator
- Changing Sustain while a note holds: glides at 2^-12 a tick (tiEnvelope notes §6) - check against the reference
- The reference answers a note 1-1.5 ms after its sample: measure the delay and whether it steps with a block of its own
- Loading of custom wavetables, in various common file formats.
- Smooth Mode On/Auto/Note: the hardware's Cutoff step takes 2-15 ms (10-90%) - measure Off against On on a clean sound to see whether On has a glide of a few ms
- Smooth Mode Quantise: measure one held control besides Cutoff (Osc Balance or an LFO amount) and one that lands at once (Filter Mode) on the hardware


## Bank A against the hardware (CT, 2026-11-09)

- Atx Bass (a factory bank, third, 8): far below the instrument above 2.5 kHz (20-50 dB) and at 40-80 Hz (42 dB)

- Start with those that use no distortion: A012 A016 A019 A021 A048 A053 A063 A067 A071 A086, and C007
- A076 and A064 (Rectifier) are not the distortion: with it off on both, the TI and the engine differ as much (A076 octave shape 3.8 -> 4.1 dB rms, the same pattern)
- C007 very different (FM?)
- A006 delays very different: Pattern 5+3 - the eleven patterns 2+5 to 5+5 built 2026-11-09; the routing still differs: on the instrument the right's tap is a single echo fed into the left's recirculating loop (findings 2026-11-09) - confirm on a second pattern, then rebuild the pattern delay
- A012 brighter on the hardware
- A014 distortion tonally different
- A016 more tones on the hardware
- A019 crackles here; the filter modulation may differ
- A021 very different: the LFO 1 -> osc 2 route and its depth are right; look at its LFO assigns (dest 124, 43) and Assign 1's dest 126 acting on that route (findings 2026-11-09)
- LFO sine Symmetry: fitted from captures 2026-11-09 (tiLfo notes §3) - check the law against the reference next session; the other shapes' Symmetry (saw, square, S&H) against the instrument
- A024 tonally very different
- A027 much brighter on the hardware
- A028 very different
- A031 very different
- A032 flat on one tone
- A048 an extra tone here
- A049 much weaker here
- A053 zipper: the chorus - fixed 2026-11-09 (tiEffects notes §13), confirmed by CT; the phaser, delay and reverb glide their modulated settings too (§25)
- Delay modulation (A116 rate, A117 depth, A118 shape) is not built: its LFO runs the voices' rate law about 7.35 times faster (4.2 and 6.6 Hz at Rate 50 and 60); the depth is not yet known. When built, read the line interpolated and move the tap per sample, or it will zipper
- A060 wildly different
- A062 distortion tonally different
- A063 sounds like another patch altogether
- A064 a saturation or distortion mode missing?
- A066 an extra tone here, not at the same tuning
- A067 tonally different
- A069 ring modulation on the hardware
- A070 very different
- A071 an octave out?
- A076 hugely different (distortion mode?)
- A083 hugely different
- A084 the hardware much brighter
- A086 very different
- A092 very different
- A094 something here not following pitch
- A100 the hardware much brighter

## Later
- Effects (next to last), then the arpeggiator (last, if at all) - design §4
- A Virus TI over USB (design §6): the instrument follows the instance's sound, a Single read back from it, and the instrument as a second reference through the audio interface

## Infrastructure
- A sidechain (audio) input into the voice, as the instrument's external input: send test signals straight into the filters and saturation, and play external audio through the Single
- The panel (`tiDraw.c`) is Pulse2Alike's `p2Draw.c` without the link, backup and arp steps - a SynthLib candidate; the patch bar and presets are now copies too
- Calculate rather than tabulate: tiWaves.c and tiWavetables.c ship only if no formula can be found
- Clocked LFOs: their phase against the host timeline is unchecked
- Saturation Rate Reducer at full drive, and Bit Reducer above drive 96, are still out
- Hardware: the USB link could go direct through libusb, bypassing the maker's driver
- Delay: modulation, Tape types, Pattern timing at other tempos, order against the EQ. Reverb. Chorus types Vintage, Hyper, Air, Vibrato, Rotary
- Reverb: top octave 4-6 dB hot, tank modulation, Feedback 1 and 2 modes, Reverb Clock
- Analog filter modes: a high-frequency floor in the reference, 20 dB higher at 44.1 kHz than at 48, not in ours
- Panel: controls with no effect yet: Tape, Delay LFO, Reverb Clock and Feedback, Quality, Tone (wavetable Sync, Formant, Detune and Smooth act since 11-02, on the models that use them)
- Panel: not offered: vocoder and input follower, soft knobs
- Delay Clock: tempo law built but unverified on our side; the times wander at 60, 120 and 200 BPM, possibly with sample rate; sequencer patterns, whose right channel runs at the exact tempo
- Reverb Clock: acts as predelay, not built
- Wave and wavetable images above their top partial (a stored cycle's mirror images, near -65 dB)
- HyperSaw and oscillators sit 0.3 cents sharp of the reference
- Oscillators at the very top: about 6 dB down at note 108 on every model - NOT REPRODUCED 2026-11-08: saw, pulse, wave 10 and sine at notes 60/96/102/108 (48 kHz, open filter, Osc 1 alone) match the reference in level within 0.2 dB; delete unless it shows again under other settings
- Stored waves alias near -50 dB, by note range; ours are cleaner. Saw and pulse are 6 dB cleaner than PolyBLEP
- Analog filter modes at high resonance; Osc 3 sine (blamed in real patches)
- Anti-alias On should keep the instrument's top-octave roll-off without its aliases (tiOsc notes §9: harmonics 8-10 at C7 4-8 dB down on the instrument, not in the band-limited saw); also Anti-alias Off for the wavetable models and the HyperSaw, and N at sample rates other than 44.1 kHz (taken as the same by note)
- Matrix: the new destinations (Contours, Interpolation, F-Shift/Spread, Unison, arp) not yet checked against the reference; effect modulation is by block. Osc 3 Detune's curve 3-7% deep above dial 96 (tiVoices notes §7b)
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
- Fixed-point residues the reference leaves and ours does not: a trace of the harmonics with the filter collapsed (Cutoff 10 and below, about -50 dB of the fundamental at 16) and a DC of about -83 dB with an envelope at its floor (Sustain Time 0)
- LFO 3 to pitch at 112 and 127: 3.8% and 5.8% shallower than the reference (voices notes §12b); LFO 1 at the same depth is not
- FM into a Wavetable Osc 2, modes 4-6 (audio in), and Wave PWM at Width 127: still far apart in the mode sweep (2026-11-12)

