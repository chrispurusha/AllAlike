# Sn2Alike to test

Finished work not yet checked against the instrument.
- Presets > Load Preset / Store Preset to...: the browser reads Bank A-L, Loc 000-127 (as A000-L127), names without the slot prefix; A-Z sorts by name (2026-11-06)
- Filter tab no longer crashes: not yet opened in a host
- A new instance starts on A001: seen in the log, with stored presets that are all Init
- Program changes and the fade on a change of sound: run through the Audio Unit only, on presets that are all Init; not heard, not run in a host
- Bend range and wheel routes: pitch in semitones, cutoff on the envelope scale and level are assumed, not yet measured
- Mono: the Mode list, Limit reading Mono, last-note return, Legato and Retrigger, in a host
- Effects Bypass on the GLOBAL page, and the A000-L127 numbering in the patch bar and browsers
- Sustain pedal in a host; order against notes in one block
- The LFOS page: Shape, Speed, Range, Delay, Fade, Offset, Trigger, Cutoff; routes to pitch and width come from programs, with no dials yet
- LFO Key and Free in a chord: Free programs cannot be compared with the reference by time window, its phase depends on its run time
- Link in a host: MIDI menu, Link HW button (green / amber No port / Link busy), Channel, SysEx ID; notes, bend, wheel, pedal; editing a dial while holding notes
- GLOBAL page: Limit, Mode, Trigger, Effects, Delay, Reverb and Hardware all fit
- Presets > Read from Hardware in a host: the Reading count, the confirm, and the instrument's data carrying on after it
