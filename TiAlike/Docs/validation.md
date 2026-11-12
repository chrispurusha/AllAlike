# Validation against the reference

How TiAlike is checked against reference renders of the instrument. The renders and tools live outside this repo; this
is what they do and what to watch for.

## Comparing
- **One setting at a time** on a plain Single (one oscillator, filter half open), every parameter at five points, then
  each family of modes on a base that lets it act. Scores per window: level, envelope, harmonics (amplitude and phase
  against the fundamental), octave bands, cents.
- **Whole presets**: every Single of a bank ranked by octave-band difference over the attack and the held part; the
  worst then "blamed" by putting each setting back to the Init Single's, one at a time, to see which closes the gap.
  A blamed setting is where to look, not yet the cause: confirm with a controlled test on both sides.
- **Exact arithmetic**: where a law is known exactly (the envelopes, the arpeggiator's patterns), the engine's own
  values are compared number for number, not by ear or by spectrum.

## Diagnosis switches
- **LFO phase**: a free-running LFO's phase at a note depends on when the Single was loaded. Renders are lined up so
  the LFO stands where the reference's does (half a cycle in at the first note), or every LFO is set to Keytrigger on
  both sides, so LFO phase drops out of the comparison.
- **Long notes** for presets that evolve: windows out to 2.5 s rather than the first second.
- **Effects and arpeggiator** off or on, to separate the voice from what follows it.

## Traps
- The reference's renders can hold short runs of exact silence; analysis windows longer than ~80 ms skip them, and a
  median over short windows is used. Never filter TiAlike's own silences the same way - they are real.
- A pitch finder can lock on the wrong fundamental (FM, sync): measure at the known harmonics instead.
- A renderer that delivers MIDI a block at a time gathers keys a few milliseconds apart into one moment; arpeggiator
  timing is checked with every key at its own time.
- Something that starts when a Single loads (free LFOs, the arpeggiator's first step) looks different on a note played
  straight after loading than on a later one.
- Osc 2's Detune drifts at random below 93, so with two oscillators sounding the phase between them wanders
  differently on each side and the harmonics follow it. Set Detune to 0 for any check with two oscillators (ring,
  balance, FM, sync) unless the drift itself is what is being checked.
- Near-silent windows (a tail 90 dB down) can score badly without being audible: check the level.
