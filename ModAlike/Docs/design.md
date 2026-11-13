# ModAlike - design

Started 2026-11-13. A standalone effect with a Phaser, Chorus and Flanger, one chosen at a time.

## 1. The engine is a library (owner, 2026-11-13)

Not a copy of the three modules: the plug-in links the engine library (G2AlikeMaths, beside AllAlike) and plays a
fixed FX-area patch through it, In -> effect -> Out, at 96 kHz. So the sound is exactly the engine's, and nothing
drifts; the cost is carrying the whole engine for three modules, and its resampling latency (31 samples).

`do-plugin` builds the library universal into its own `build-universal/` and links it before SynthLib. The library
exports only its own `g2am_*` calls; before that, its copy of SynthLib's log globals collided with SynthLib's own.

## 2. Stereo (owner, 2026-11-13)

The Phaser and Flanger modules are mono in, mono out; the Chorus mono in, stereo out.

- **Phaser and Flanger**: two modules, one a side. Slot A holds the left, slot B the right, each module at the same
  index: the engine draws an LFO's start phase from a seed each slot has its own copy of, so the two sides start in
  step and stay there. Two modules in ONE slot drew different phases and the sides drifted apart (the first test
  caught it).
- **Chorus**: L+R summed through a Mix2-1A (both levels 100 give the average, 0.98) into one module in slot A.
- **Stereo modes** (owner, 2026-11-13): Linked as above; **Wide** sets the right side's LFO behind the left by
  the LFO Offset, through the library's `g2am_lfo_link()` (an extension: the modules have no phase control) - both
  then run at one rate, so the gap holds through a change of Rate; **Mid/Side** feeds the engine the mid on both
  sides and adds the side back after it, delayed by the engine's latency. The **Chorus** is a module a side by
  default (left from slot A, right from slot B), with the mono sum as Chorus Input "Mono".
- **Mix**: a dry copy, delayed by the engine's latency, blended with the effect. The modules have no such control.

## 3. Level

Latency: 31 samples below 88.2 kHz, where the engine resamples to 96 kHz and back; none at 88.2 kHz and above.
The plug-in asks the library (`g2am_latency()`) and tells the host when a new rate changes it.

`2-In` -> `2-Out` loses 4.6 dB at Pad 0 (measured by the library's `make test`); ModAlike makes it up, so a bypassed
patch is at unity. Whether the instrument itself loses that between its inputs and outputs has not been measured.

## 3a. Engine options (owner, 2026-11-13)

- **Rate**: the library's economy - the graph at the host's rate below 88.2 kHz, so no resampling and no latency.
  Measured against 96 kHz on noise the effects sit within about 1 dB, all of it the Anti-alias low-pass taking the
  band above 18 kHz; with Anti-alias off the path is flat to 20 kHz. At 96 kHz the engine's own resampling rolls off
  at the top of a 48 kHz host: -3.4 dB at 20 kHz.
- **Maths**: Fixed point, built for this in the engine (its sound-engine-notes §227) - the Phaser's, Chorus's and
  Flanger's words truncated to the 24-bit step. Against floating point the difference is noise, ~104 dB down.
- **Stage**: an output converter's roll-off (a two-pole low-pass at 33.5 kHz, -0.46 dB at 15 kHz). Off by default.
  At a 48 kHz graph it cannot be placed - 33.5 kHz is past Nyquist - and cut 3.5 dB at 15 kHz, so ModAlike holds it
  off at the host rate whatever the switch says.
- **Anti-alias**: the owner's rule for every Alike (On by default). At 96 kHz nothing here aliases - there are no
  oscillators - so it acts only at the host rate, as a two-pole Butterworth low-pass at 18 kHz on what goes into the
  effect; the dry paths are left alone. Drawn dimmed at 96 kHz.

## 4. Parameters

They reach the engine from an apply thread every 5 ms, never from the audio thread: a change is a library control
call and a commit, which takes locks and rebuilds a snapshot. Choosing another effect rebuilds the patch.

## 5. Still to do

See `todo.md`: dial values in their units rather than 0-127, a click-free switch between effects.
