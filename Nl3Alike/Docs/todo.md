# Nl3Alike todo

## First
- Check the switch names against the instrument (stored value = panel position is measured; the names follow the
  manual's order)
- Replace the placeholder engine section by section: oscillators, envelopes, LFOs, then FM, sync, morph
- Filter: the other modes - LP/HP 6 dB, BP and BR 12/24, Classic LP 24, Distorted LP 12/24, and the multi modes
  (HP-LP series/parallel, LP-BP, BP-BP, Multi Peak, Multi Notch) with the second cutoff
- Filter: how modulation reaches the cutoff word (envelope amount, key track, velocity), measured
- Filter: check against the instrument on the rig (QU-24 9/10) - responses and self-oscillation level
- Fixed point beyond the filter, per the Arithmetic parameter
- Oscillators: Pulse, Noise, Dual Sine, then sync and Shape (the Saw, Triangle and Sine are built); the Sine's feedback (osc notes §10)
- Pitch (osc notes §8): fractions of a semitone - their resolution; whether the tuning word follows Master Tune
- Master Tune per instance (GLOBAL): ±6 semitones and ±100 cents, as the instrument's, scaling the tuning word
- The voice delay stage after the filter: Unison detune by a delay ramp, and the 6-tap fractional-delay kernel
- Remove Q Normalise (Sn2Alike's; does nothing now); replace the Dist dial's placeholder saturator with the
  Distortion the instrument has (Flt Dist, and the Distorted LP modes)
- Osc Bypass (a switch, per program): sends an oscillator around the filter, added after it; the filter takes that
  second input already (both arithmetics). Build it with the oscillators
- The MIDI link: send single parameters as the instrument's CCs (the whole program goes now)
- Panel: the pages for the Nord Lead 3's sections
- `Docs/code-notes/` came across from Sn2Alike with the code: rewrite each as its file changes
