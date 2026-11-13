# maDraw.c - notes

1. **Where it comes from.** Nl3Alike's panel (itself Pulse2Alike's look), cut down: no tabs (one page), no patch
   bar, name field, presets, bank browser or hardware link. What is left is generic - sections flowing left to right,
   dials (drag, or Rotary following the pointer), list buttons opening a menu of values, the menu bar and SynthLib's
   popups - and is the cleanest cut so far for moving the panel into SynthLib (ClaudeFindings' candidates list).
2. **Dimmed sections.** A section's `onlyWhen` is an entry of the Effect list: unless the list stands there, its
   title, labels and values draw grey and its dials at a third of their colour. It still takes edits, so an effect's
   settings can be prepared before it is chosen.
3. **Value text** is `format` applied to `plainMin + step x plainStep`: Output is 72 steps of 0.5 dB from -24, Mix
   0-100%, LFO Offset 0-180 deg, the modules' dials 0-127. ASCII only - the glyph atlas has no degree sign.
4. **Dimmed controls.** A control's `litParam`/`litSteps`/`litStep` dim it unless a list stands at an entry -
   Anti-alias is lit only at the host rate. Like a dimmed section, it still takes edits.
