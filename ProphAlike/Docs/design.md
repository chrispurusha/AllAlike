# ProphAlike design

ProphAlike is a SynthLib instrument plug-in inspired by the Korg Prophecy. This initial placeholder was made from
the AllAlike instrument skeleton. Its current oscillator, filter and envelope are scaffolding only; none of their
laws or sound should be treated as a Prophecy model.

## Product shape

- One instrument per plug-in instance, hosted as VST3 or AUv2.
- The GUI, parameter handling, state and plug-in wrappers use the existing AllAlike and SynthLib conventions.
- The engine will be replaced incrementally as Prophecy behaviours are measured and fitted.

## Research and validation

The Prophecy hardware is the authority for audio captures and checks. Measurements should record the patch state,
MIDI events, sample rate, capture path and analysis method alongside each result. The engine will be developed from
those observations and validated against repeatable hardware captures.

The current code is a temporary host and UI scaffold. Its inherited oscillator, filter, envelope, effects, patch handling and hardware controls have not been validated against the Prophecy. Replace or remove each part before relying on it.

The first playable version (2026-11-21) has pages for Osc 1, Filter 1 and the Amp EG beside Global, named and ranged as the Prophecy's own controls. Filter 1 is the instrument's state-variable filter (code-notes/prophFilter.md), exact in fixed point against the instrument's arithmetic, with Cutoff, Resonance and the five types. The oscillator is still the skeleton's band-limited saw and square, and the Amp EG has the Prophecy's eight controls with approximate times. The skeleton's other pages stay hidden until rebuilt around the Prophecy's signal structure.

The fitted engine should offer both a fixed-point path that reproduces the hardware DSP's arithmetic and a floating-point path for comparison. The fixed-point path must preserve each stage's rounding and overflow behavior rather than only matching the final coefficient values. Following the other AllAlike plug-ins, arithmetic mode and anti-aliasing are per-instance Global settings, held by the host and excluded from patch data. Expose the arithmetic choice once both paths are implemented.

## First engine milestone

Choose a simple voice configuration and establish its clean signal path, note and velocity response, oscillator
behaviour, filter response and envelope timing. Fit each stage from captures, then check combinations and repeat
the measurements on the hardware. Keep measurement findings in this document or numbered code notes; keep
`Docs/todo.md` as a one-line-per-item task list.

## Status (2026-11-21)

### Done
- **Filter 1** (`code-notes/prophFilter.md`): the instrument's state-variable filter - one structure giving THRU, LPF,
  HPF, BPF and BRF - in its fixed-point arithmetic, checked word for word against the instrument's own arithmetic
  over a cutoff change and at Resonance 99, and in floating point (within -95 dB of the fixed point below full scale).
  Cutoff target at all 100 steps, the 128-sample cutoff glide, Resonance as damping plus input-gain compensation plus a
  cutoff lift (known at 0, 10 .. 90, 99), the gain and damping glides.
- **Maths** (FLOAT/FIXED, Global, per instance) and the existing Anti-alias switch.
- Engine at 48 kHz, the instrument's voice rate.
- Panel pages OSC, FILTER, AMP EG, GLOBAL with the Prophecy's own control names and ranges.

### Placeholder
- The oscillator: the skeleton's band-limited saw and square (no Prophecy oscillator model yet).
- The amp EG: the Prophecy's eight controls with straight segments and approximate times.
- The level the oscillators reach the filter at (a quarter of full scale), until the oscillators are known.
- Cutoff between Cutoff steps (modulation): linear interpolation of the 100-step table.

### Next, in order
1. The Standard oscillator set (saw, pulse), then the level it feeds the filter.
2. The amp EG's times and segment curves.
3. Filter 2 and the filter routing; Input Trim; Cutoff key tracking, EG and LFO modulation at fine resolution.
4. The mixer, the amp stage before it, the output stage.
5. The other oscillator models, the LFOs and EGs, the effects.

