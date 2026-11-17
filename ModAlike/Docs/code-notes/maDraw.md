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

## 1. Double-click a dial
Two presses on the same dial within 0.4 s put it back to its default (the parameter table's, given to the panel at
start by `ma_draw_set_default()`), through the host like any edit, so it can be undone there. The second press does
not grab the dial. As OneShot's (osDraw notes §10), added across the Alikes 2026-11-14.

## 2. Shift for fine adjustment
With Shift held, dragging a dial moves it a tenth as far for the same distance - the same on the Mac and Windows
(SynthLib's panel views read the modifiers on every drag event). Rotary mode is unchanged. Added 2026-11-14.

## 3. Colours and Off/On buttons (2026-11-17)
The panel follows `../../Docs/panel-layout.md` (Colours), on SynthLib's defaults (synthlibDefs.h §7). A section or
control not in use (the two effects not chosen, Anti-alias at 96 kHz) is SynthLib's grey, `RGB_PANEL_UNAVAILABLE`,
where it was its own colour at 35%: dimmed now means Off. A choice of Off and On (Anti-alias, Stage) is a button that
flips on the release that lands on it, lit in its section's colour when on and dimmed with light text when off.
OUTPUT takes `RGB_SECTION_OUTPUT`; ENGINE is now PLUG-IN, in `RGB_SECTION_PLUGIN` and the shared order (Anti-alias,
Maths, then Rate and Stage); EFFECT is gold.
