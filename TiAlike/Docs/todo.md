# TODO

One line per item. Measurements and reasoning go in `findings.md`, design in `design.md`.

## First
- Filter 1 low/high/band pass, band stop and Analog 1-4 Pole are in (code-notes/tiFilter.md); next: Filter 2, the routings (Serial 4/6, Parallel, Split) and Filter Balance, the 4-pole digital forms
- Osc Volume on the panel: its upper half drives the analog filter (filter notes §5, `drive`) and the Saturation curves; then re-fit the ladder's input level (filter notes §6)
- Analog modes: the self-oscillation pitch at Resonance 127 is 0.9% below the reference's (634 vs 640 Hz) - check at other cutoffs
- Filter: keyfollow, the filter envelope, saturation; check the cutoff holds in hertz at a 44.1 kHz host too
- The oscillator against the reference renders: the classic shape's waves, pulse width, level against pitch
- Patch loading: a Single from a `.syx` / `.mid` bank into the instance (design §2), with a patch bar on the panel (Pulse2Alike's Prev / Next / Load)
- The amplifier envelope, then the filter envelope - the placeholder is a 2 ms / 120 ms gate
- Notes at their sample offset within a block (now at the block's start)
- Parameter Smooth Mode: only On is built (cutoff, resonance - tiVoices notes §5); Off, Auto, Note and the quantised modes, and the other smoothed controls (oscillator shape, PW, ...)

## Later
- A Virus TI over USB (design §6): the instrument follows the instance's sound, a Single read back from it, and the instrument as a second reference through the audio interface

## Infrastructure
- `do-plugin`'s SynthLib list is by hand (MANUAL in the script): add files as the panel grows
- The panel (`tiDraw.c`) is Pulse2Alike's `p2Draw.c` cut down - a SynthLib candidate
