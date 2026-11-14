# Nl3Alike todo

## First
- Priority (CT 2026-11-14): programs 2-4 (String Pad, Happy ending, Ethereality) - Unison done; multi types 1, 4, 5 and the Sine's Shape done (filter notes §15-16, osc notes §12) - programs 2-4 lack only vibrato; what reaches the second cutoff (Filter 2 Env, key track, LFO to Filter 2) measured, then the Sine's Shape (program 4); vibrato later
- Modulation (CT 2026-11-14: first): LFO 2 to Resonance and Pan, Invert on the positive-only destinations, LFO to Osc Mod once Osc Mod exists (pitch, filter, Shape, Osc Mix and Amp done, voices notes §21-22); Smooth Random and Special waves; keyed start phase; Env (single cycle) and Mono modes
- Modulation: the Mod Env to destinations other than the filter (pitch cubic as the LFO's, the rest presumably as voices notes §22); what Amount 127 does to pitch (the offset reads zero)
- Morph: Aftertouch (and Control Pedal) as the third group's source - the plug-in receives neither yet (voices notes §25); whether a morphed knob is clamped to its range before or after the groups add
- Check the switch names against the instrument (stored value = panel position is measured; the names follow the
  manual's order)
- Replace the placeholder engine section by section: oscillators, envelopes, LFOs, then FM, sync, morph
- Filter: Classic's coefficient laws (its sample path is built, filter notes §13); Distorted LP 12/24; the multi modes (LP/HP 6, BP 12/24, BR done, notes §9-14)
- Filter, FLOAT arithmetic: the 6 dB modes and band-reject part from FIXED at cutoff 127 with a near-full-scale input, where the stage saturates chaotically - check by ear whether it matters, else find the step
  (HP-LP series/parallel, LP-BP, BP-BP, Multi Peak, Multi Notch) with the second cutoff
- Filter: how modulation reaches the cutoff word (envelope amount, key track, velocity), measured
- Filter: check against the instrument on the rig (QU-24 9/10) - responses and self-oscillation level
- Fixed point beyond the filter, per the Arithmetic parameter
- Oscillators: Noise, Dual Sine, then sync and Shape on the other waves (the Saw, Pulse, Triangle and Sine are built); the Sine's feedback (osc notes §10)
- Pulse width modulation (osc notes §11): the LFO and modulation-envelope depth on the width word is a placeholder
- Pitch (osc notes §8): fractions of a semitone - their resolution; whether the tuning word follows Master Tune
- Master Tune per instance (GLOBAL): ±6 semitones and ±100 cents, as the instrument's, scaling the tuning word
- Unison (voices notes §26) built; still to match: the 6-tap kernel's correction term, the reads' start on a new note, and whether Unison off really bypasses the ring (no fixed fractional delay)
- Remove Q Normalise (Sn2Alike's; does nothing now); replace the Dist dial's placeholder saturator with the
  Distortion the instrument has (Flt Dist, and the Distorted LP modes)
- Osc Bypass (a switch, per program): sends an oscillator around the filter, added after it; the filter takes that
  second input already (both arithmetics). Build it with the oscillators
- The MIDI link: send single parameters as the instrument's CCs (the whole program goes now)
- Panel: the pages for the Nord Lead 3's sections
- `Docs/code-notes/` came across from Sn2Alike with the code: rewrite each as its file changes
