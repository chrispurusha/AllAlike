# UltraAlike to test

Built but not yet checked in a host or against the instrument. Confirmed: delete the line. Failed: move it to todo.md.

- The skeleton (2026-11-14): loads in a host as UltraAlike, plays a sample as OneShot did, and sits beside OneShot without either replacing the other
- Bank presets (plugin notes §12): load an Emulator X .exb, Prev/Next through its presets, play across the keyboard - zones by key and velocity, loops held, the project reopening on the same preset
- Loop ends one frame earlier (SynthLib emuBank notes §3): Soul Oohs held - no tick on each loop
- Shift-drag a dial: a tenth the movement, in each dial mode but Rotary
- Bank amplifier envelopes (uaBankVoice notes §3): plucked and held presets against Emulator X3 - stage times and the shape of each segment
- Panel tidied (uaParams notes §12): PITCH, PLAY, OUTPUT, ENVELOPE only; nothing a bank preset ignores
- One preset or sample an instance (uaPlugin notes §13): no tabs; a project reopens on its bank and preset
