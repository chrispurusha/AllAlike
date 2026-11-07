# Nl3Alike todo

## First
- Map the program format (the Nord Lead 3's program dump) and its parameters onto the panel
- Replace the placeholder engine section by section: oscillators, envelopes, LFOs, then FM, sync, morph
- Filter: the other modes - LP/HP 6 dB, BP and BR 12/24, Classic LP 24, Distorted LP 12/24, and the multi modes
  (HP-LP series/parallel, LP-BP, BP-BP, Multi Peak, Multi Notch) with the second cutoff
- Filter: how modulation reaches the cutoff word (envelope amount, key track, velocity), measured
- Filter: check against the instrument on the rig (QU-24 9/10) - responses and self-oscillation level
- Fixed point beyond the filter, per the Arithmetic parameter
- Remove Q Normalise (Sn2Alike's; does nothing now); replace the Dist dial's placeholder saturator with the
  Distortion the instrument has (Flt Dist, and the Distorted LP modes)
- Osc Bypass (a switch, per program): sends an oscillator around the filter, added after it; the filter takes that
  second input already (both arithmetics). Build it with the oscillators
- The MIDI link: the Nord Lead 3's own CCs and SysEx in place of the placeholder's
- Presets: read the instrument's programs into the presets
- Panel: the pages for the Nord Lead 3's sections
- `Docs/code-notes/` came across from Sn2Alike with the code: rewrite each as its file changes
