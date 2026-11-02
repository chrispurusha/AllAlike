# Nl3Alike design

Started 2026-11-02 as a reproduction inspired by the Clavia Nord Lead 3, as a SynthLib plug-in (VST3 + AU). Public, in
AllAlike.

## State

The skeleton is Sn2Alike's, renamed: plug-in wrappers, panel, presets, patch bar, MIDI link. The engine behind it is
still Sn2Alike's and plays as a placeholder; every section is to be replaced by the Nord Lead 3's own.

## The instrument

A virtual-analogue synth: six slots of 24 voices in all (to check), each voice two oscillators with FM and sync, a
multimode filter, three envelopes (mod, filter, amp), two LFOs, a morph group per knob, and the arpeggiator. Four
stereo outputs.

## Validation

The reference is the instrument: on the Elektron TM-1, MIDI channel 3, its audio into QU-24 inputs 9/10. Laws in the
code are there because they reproduce what the instrument plays; this repo states only what was measured.

## To decide

- Scope: one program per instance, as the other Alikes (slots and performances later, if at all).
- The panel, once the parameters are mapped.
