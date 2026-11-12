# To test

Built but not yet checked by ear or in a host. Confirmed: delete the line. Failed: move it to `todo.md`.

- Presets > Load Preset / Store Preset to...: the browser reads Bank A-W, Loc 001-128 (as A001-W128), names without the slot prefix; A-Z sorts by name (2026-11-06)
- ***FIRST BUILD (2026-10-26)*** - load TiAlike in a host: the panel (OSC, FILTER, AMP, GLOBAL), dials and lists, a chord at
  Voice Limit 16 and 2 (the oldest voice is taken), Cutoff and Reso by ear, both formats.
- Link on, in Live: turning Filter 1's Cutoff turns Filter 2's Cutoff dial with it, its text "cutoff (offset)"; dragging Filter 2's dial changes the offset only; Link off: a plain dial again (tiPlugin notes §4)
- Patches (tiDraw notes §9-12), in a host: Load... a one-Single `.syx` (`ChrisPsyBass.syx`) and a sound set `.mid` (bank browser, choose one); the file label and `12/128`; Save Patch As, then Save Patch (overwrite confirm), reload it; Init
- Presets: Import from Folder on the four `VirChrisBank?.syx` (A-D), Prev/Next wrap D128 <-> A001, Load Preset browser, edit a dial -> `A001*`, Revert, Store to... another slot, Export then Import it back
- The name field: click, type mixed case, Return keeps / Escape drops; Store shows the new name in the browser
- Reopen a project: the sound, its name, file / preset and `*` come back as left (tiPlugin notes §5)
- The amplifier envelope by ear in a host (ENV page): Attack, Decay to Sustain, Sustain Time both ways, Release; a loaded Single sets all five (tiEnvelope notes)
- Notes at their own sample: a fast attack and release sound the same at host block sizes 64 and 1024
- The five Overdrives by ear at low, middle and high Intensity; Saturation Rectifier

- Smooth Mode in a host: Off/On/Auto/Note a Cutoff move lands at once; Quantise 1/4 holds it to the host's beat grid while playing (on the beat, not a beat from when the instance loaded); a preset change with a Quantise Single sounds its own cutoff at once
- Virus TI menu (2026-11-02), with the instrument on USB and Link off: Read Banks A-D (about 8 min) - the presets' A-D should match an earlier backup of the RAM banks; then Write Presets D to Bank D with D unchanged - "all 128 read back the same"; then one edited preset in D written and read back
- Load/Store Preset browsers sort by category (2026-11-09): the presets fall under their own categories
- LFO sine Symmetry (2026-11-09): sounds with a sine LFO away from Symmetry 64, by ear against the instrument
- A053's zipper gone with the chorus's delay and depth glided (2026-11-09)
- Delay Patterns 2+5 to 5+5 (2026-11-09): A006 (5+3) against the instrument
- Modulated phaser, delay and reverb glide (2026-11-09): sounds that move them, no zipper and nothing else changed
- W010 by ear in Live: repeated notes and fast phrases without the ticks; other Attack 0 Singles' first notes (the amp now reaches its level over 32 samples)
- Fixed-point amp envelope: Attack/Decay/Sustain/Release by ear, Fixed against Float; Sustain Time patches (falls start with the note now); Sustain 127 holds full; filter-envelope sweeps (Attack 0 and slow attacks) and envelope 3/4 modulation, Fixed against Float
- Bank C Anaklav1-3 (C005-C007) and other analog-filter patches in Serial 6: brighter now, should match the instrument
- Osc 3 Semitone intervals (just ratios) and Osc 3 Detune going flat; portamento lengths above 64; LFO 3 vibrato depth
- Arpeggiator (ARP page): modes with chords, patterns, Clock, Swing, Note Length, Hold, in a host at different tempi; arp patches in banks A-D
- Attack 0 sounds (plucks, arps) start at full level on the first sample now: listen for the click against the instrument
- Global Anti-alias switch (On by default): Off sounds grittier on high saw/pulse notes, On unchanged; saved with the project, not with a Single (2026-11-12)
- Arp Mode, Unison and Ring on an older factory Single (banks A-D): the panel now changes them; and the sustain pedal with the arpeggiator holds the keys as Hold does, without sustaining the arp's notes (2026-11-12)
- Delay modulation built 2026-11-12 (tiEffects notes §8a; rate, depth, antiphase within 0.4 dB of the reference on a sine's echo): listen to a Single with Depth up (A006, A116-A118) against the hardware
- User wavetables (2026-11-12, Wavetable menu): load a PPG, a TI2/Serum and a Waldorf .syx table on each oscillator, step with Prev/Next beside Wave (greyed for the built-in tables), sweep Index, save the project and reopen it with the files moved away
- LFO 1 in Env Mode to Filter Gain silent once its cycle is spent (B093 RePunch, 2026-11-12), Key Mode Hold with no arpeggio (C074, C117), Mint Overdrive at twice its old level: listen against the hardware
- Rectifier with a quiet input, arpeggiator user patterns (step lengths in 64ths, 127 a tie), an Env Mode LFO with Keytrigger resting where its cycle began (K030 FunkMamaBC) (2026-11-12): listen against the hardware
- A clocked LFO in Env Mode (U076 OfcHell), Patch Volume below 8 (2026-11-12): listen against the hardware
- Tape delay (Clocked, Free, Doppler; Ratio, Bandwidth, Feedback) and Reverb Predelay / Reverb Clock (2026-11-12): listen against the hardware, and turn Delay Time while Tape Free and Doppler run
