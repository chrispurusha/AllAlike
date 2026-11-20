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

While the Prophecy control layout is being established, the panel exposes only the shared Global page. The inherited oscillator, filter, envelope, modulation and effects pages are hidden until they can be rebuilt around the Prophecy's actual signal structure.

The fitted engine should offer both a fixed-point path that reproduces the hardware DSP's arithmetic and a floating-point path for comparison. The fixed-point path must preserve each stage's rounding and overflow behavior rather than only matching the final coefficient values. Following the other AllAlike plug-ins, arithmetic mode and anti-aliasing are per-instance Global settings, held by the host and excluded from patch data. Expose the arithmetic choice once both paths are implemented.

## First engine milestone

Choose a simple voice configuration and establish its clean signal path, note and velocity response, oscillator
behaviour, filter response and envelope timing. Fit each stage from captures, then check combinations and repeat
the measurements on the hardware. Keep measurement findings in this document or numbered code notes; keep
`Docs/todo.md` as a one-line-per-item task list.
