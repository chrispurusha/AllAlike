# UltraAlike to test

Built but not yet checked in a host or against the instrument. Confirmed: delete the line. Failed: move it to todo.md.

- The skeleton (2026-11-14): loads in a host as UltraAlike, plays a sample as OneShot did, and sits beside OneShot without either replacing the other
- Bank presets (plugin notes §12): load an Emulator X .exb, Prev/Next through its presets, play across the keyboard - zones by key and velocity, loops held, the project reopening on the same preset
- Loop ends one frame earlier (SynthLib emuBank notes §3): Soul Oohs held - no tick on each loop
- Shift-drag a dial: a tenth the movement, in each dial mode but Rotary
- Bank amplifier envelopes (uaBankVoice notes §3): plucked and held presets against Emulator X3 - stage times and the shape of each segment
- Panel tidied (uaParams notes §12): PITCH, PLAY, OUTPUT, ENVELOPE only; nothing a bank preset ignores
- One preset or sample an instance (uaPlugin notes §13): no tabs; a project reopens on its bank and preset
- Bank envelopes in the E5000's shape (11-16, uaBankVoice notes §5): decays and releases straight in dB, levels on the measured law (50% is ~44 dB down, not 6) - play a few Emulator X presets, especially pads with low sustains, and say whether they now sit right or fall too quiet
- Anti-alias Off and Polarity on the GLOBAL section (11-16): play a bank preset high and low with Anti-alias Off and listen for the E5000's air; Polarity Inverted flips the output
- ENVELOPE dials on a bank preset (11-16, uaBankVoice §6, uaPlugin §15): loading or stepping a preset moves Attack/Decay/Sustain/Release to its envelope (Hold Inf, AHDSR); turning one changes that stage of every voice in proportion; reopening a project keeps an edited dial's sound; the preset line reads P000...
- The E5000 panel (11-16): PITCH/PLAY/OUTPUT/AMP ENV RATE/LEVEL/GLOBAL - a preset's rates and levels land on the dials, and turning one moves every voice; a loose WAV plays through the same voice with the dials' envelope; Solo plays one note at a time
- Emulator X envelopes read as rates (11-16, uaEosEnv §5, SynthLib emuBank §7): 9ft Grand Piano decays over ~20 s and releases in ~1 s; check pads, plucks and drums sound the right length - the x254 scale is inferred from the library, not confirmed against Emulator X
- Sustain pedal (11-16, uaBankVoice §9, uaPlugin §17): CC 64 holds released notes on the piano until the pedal lifts, in the VST3 and the AU
- Pitch bend and mod wheel (11-16, uaBankVoice §10, uaParams §16): the wheel bends held and releasing notes by Bend Range (default 2); the mod wheel is received but drives nothing yet
