# TODO

One line per item. Measurements and reasoning go in `findings.md`, design in `design.md`.

## First
- See if Virus has a backend limiter which might be of use.
- Filter 1 (all eight modes) and Filter 2 with Serial 4 / Parallel 4, Balance and Link are in (code-notes/tiFilter.md, tiVoices.md §6); next: Serial 6 (Filter 1 four-pole) and Split (stereo), then the plug-in's stereo output
- Osc Volume on the panel: its upper half drives the analog filter (filter notes §5, `drive`) and the Saturation curves; then re-fit the ladder's input level (filter notes §6)
- Analog modes: the self-oscillation pitch at Resonance 127 is 0.9% below the reference's (634 vs 640 Hz) - check at other cutoffs
- Filter: keyfollow, the filter envelope, saturation
- The oscillator against the reference renders: the classic shape's waves, pulse width, level against pitch
- Singles drive only what is built (tiPatch notes §3; the amp envelope's 59-63 now carried): add each section's bytes to `kCarried` as it is built - Osc 1 shape/PW (page A 17/18) with the oscillator
- Program change: a preset by MIDI program change, as Pulse2Alike's (bank select picks A-D)
- Start a new instance on A001, as Pulse2Alike starts on P001 (now Init)
- The filter envelope (controllers 54-58): the amplifier's code with its own controls and the filter's Env Amount (44/45)
- Changing Sustain while a note holds: glides at 2^-12 a tick (tiEnvelope notes §6) - check against the reference
- Velocity to volume and the other velocity amounts (the Init has 0; tiVoices notes §7)
- The reference answers a note 1-1.5 ms after its sample: measure the delay and whether it steps with a block of its own
- Parameter Smooth Mode: only On is built (cutoff, resonance - tiVoices notes §5); Off, Auto, Note and the quantised modes, and the other smoothed controls (oscillator shape, PW, ...)
- Loading of custom wavetables, in various common file formats.


## Later
- Effects (next to last), then the arpeggiator (last, if at all) - design §4
- A Virus TI over USB (design §6): the instrument follows the instance's sound, a Single read back from it, and the instrument as a second reference through the audio interface

## Infrastructure
- `do-plugin`'s SynthLib list is by hand (MANUAL in the script): add files as the panel grows
- The panel (`tiDraw.c`) is Pulse2Alike's `p2Draw.c` without the link, backup and arp steps - a SynthLib candidate; the patch bar and presets are now copies too
