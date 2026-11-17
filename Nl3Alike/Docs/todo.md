# Nl3Alike todo

## First
- Priority (CT 2026-11-14): programs 2-4 (String Pad, Happy ending, Ethereality) - Unison done; multi types 1, 4, 5 and the Sine's Shape done (filter notes §15-16, osc notes §12) - vibrato BUILT 11-14 (voices notes §31; source 1 = aftertouch, 2 = wheel); what reaches the second cutoff (Filter 2 Env, key track, LFO to Filter 2) measured, then the Sine's Shape (program 4); vibrato later
- Modulation (CT 2026-11-14: first): LFO 2 to Resonance and Pan, Invert on the positive-only destinations, LFO to Osc Mod once Osc Mod exists (pitch, filter, Shape, Osc Mix and Amp done, voices notes §21-22); Smooth Random and Special waves; keyed start phase; Env (single cycle) and Mono modes
- Modulation: what the Mod Env's Amount 127 does to pitch (the offset reads zero); its other destinations are built bar OSCMOD and FILT 2 (below)
- Morph: aftertouch BUILT 11-14 (channel pressure, the wheel's law assumed - measure it); the Control Pedal as a source; whether a morphed knob is clamped to its range before or after the groups add
- Check the switch names against the instrument (stored value = panel position is measured; the names follow the
  manual's order)
- Replace the placeholder engine section by section: oscillators, envelopes, LFOs, then FM, sync, morph
- Filter: Classic BUILT 11-14 (filter notes §13; its resonant peak ~6 dB high at Resonance 100); Early lead (Classic, Freq 13) still 18 dB dull from h3 up - its filter envelope / key track lift, not the filter; Distorted LP 12/24; the multi modes (LP/HP 6, BP 12/24, BR done, notes §9-14)
- Filter, FLOAT arithmetic: the 6 dB modes and band-reject part from FIXED at cutoff 127 with a near-full-scale input, where the stage saturates chaotically - check by ear whether it matters, else find the step
  (HP-LP series/parallel, LP-BP, BP-BP, Multi Peak, Multi Notch) with the second cutoff
- Filter: how modulation reaches the cutoff word (envelope amount, key track, velocity), measured
- Filter: check against the instrument on the rig (QU-24 9/10) - responses and self-oscillation level
- Fixed point beyond the filter, per the Arithmetic parameter
- Oscillators: sync BUILT 11-14 (osc notes §13, saw measured; check pulse, triangle and sine against the instrument); Osc 1 Noise BUILT 11-16 (osc notes §15, matches the instrument); Osc 2 Noise (Noise Type/Frequency), Noise 3's colour (noiseSoften never set), Dual Sine, Shape on the other waves
- Pulse width modulation (osc notes §11): the LFO and modulation-envelope depth on the width word is a placeholder
- Pitch (osc notes §8): fractions of a semitone - their resolution; whether the tuning word follows Master Tune
- Master Tune per instance (GLOBAL): ±6 semitones and ±100 cents, as the instrument's, scaling the tuning word
- Unison (voices notes §26) built; still to match: the 6-tap kernel's correction term, the reads' start on a new note, and whether Unison off really bypasses the ring (no fixed fractional delay)
- Remove Q Normalise (Sn2Alike's; does nothing now); replace the Dist dial's placeholder saturator with the
  Distortion the instrument has (Flt Dist, and the Distorted LP modes)
- Osc Bypass (a switch, per program): sends an oscillator around the filter, added after it; the filter takes that
  second input already (both arithmetics). Build it with the oscillators
- The MIDI link: knobs go as CCs BUILT 11-16 (plugin notes §14); switches and choices still go whole and reach only the next note - send those as CCs too once their CC values are measured (scaled, not the knob value)
- Panel: the pages for the Nord Lead 3's sections
- `Docs/code-notes/` came across from Sn2Alike with the code: rewrite each as its file changes
- Octave Shift with Filter Keyboard Track on: check the track follows the shifted note (voices notes §33, assumed)
- Owner A/B 11-16, bank 1 against the instrument:
  - 1:2 String Pad: louder on the instrument (unison?); 1:3 Happy ending explained by the Osc Mix law, FIXED 11-16 - recheck both
  - 1:7 Self Mod Filter: louder in Nl3Alike
  - 1:15 Open Air: very different - a missing feature?
  - 1:17 Icisematta: tonally different
  - 1:18 Albatross: zipper/tick in Nl3Alike
  - 1:19 ChimeOrch: louder on the instrument
  - 1:21 SawLead: detuning FIXED 11-16 (Osc 2 Partial, osc notes §16) - recheck the ticking; 5-7 dB quiet against the other programs (Classic filter, Distortion 100?)
- Osc Mod: FM and DFM BUILT 11-16 (osc notes §17, the firmware's arithmetic; Mod Env OSCMOD reaches them - 129 of its 164 programs); still DM, DIST (21 Mod Env programs), FILT, RM, LFO to OSCMOD, and a check against the instrument's sound; Mod Env FILT 2 (13) does nothing yet
- Poly Legato (manual p.108): a chord played over a held one takes over the old voices without restarting the envelopes, and with Glide each glides in parallel to its new note, plus the rules for a chord with more or fewer notes - replaces the stopgap in voices notes §12a (poly + Legato glides each voice from its own last pitch while a key is held; envelopes restart)
