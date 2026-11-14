# Nl3Alike to test

Finished work not yet checked against the instrument or in a host.

- The first build in a host (VST3 and AU): loads, the panel opens, a note plays (the placeholder engine)
- Load/Store Preset browsers sort by category (2026-11-09): the presets fall under their own categories
- the Sort menu with Prev/Next, and a note held across a preset change (it stops, no clip lamp) (2026-11-12): in a host
- Triangle and Sine on Osc 1 and Osc 2 in a host, both Arithmetic settings; a Saw a shade cleaner with Distortion at 0 (2026-11-13)
- Panel buttons (2026-11-14, SynthLib buttonPress.h §1): Prev/Next/Load, the tabs (and Link HW where there is one) light while held and act on the release, and not at all when released off the button
- Pulse on Osc 1 and Osc 2 in a host (osc notes §11): Shape 0 a square, turning it narrows the pulse to almost nothing at 127, both Arithmetic settings and Anti-alias On/Off; checked against the instrument offline at three widths, not yet by ear in a host
- LFOs (voices notes §21): Saw falling, Triangle from its top, Square polarity, rate 0.03-523 Hz, pitch depth cubic in Amount (2 semitones at 32, 16 at 64) - by ear against the instrument
- LFO destinations (voices notes §22): to Filter it only opens (a semitone an Amount step), to Shape it only widens the step count, to Amp it dips the level (silent at 64, inverting past it), Osc Mix swings both ways - by ear against the instrument
- Envelopes (envelope notes): Amp and Filter attack, decay, sustain and release against the instrument by ear - attack 0.5 ms to 45 s, Decay/Release 127 hold, Amp Sustain squared, Exp Attack's slow start
- Mod Env modes (envelope notes §6): AD falls whether held or not, Repeat restarts it while held, AR holds at the top until release - by ear
- Morph groups (voices notes §25): Wheel, Velocity and Keyboard morphs move their knobs by range x source (Keyboard C2 to C6), each voice its own - by ear on a program with morphs (most factory programs use them)
- Band-pass 12 and 24 dB (filter notes §9-10): Filter Type BP now plays the measured band-pass, exact in Fixed - by ear, and its peak level against the instrument
- 6 dB lowpass and highpass (filter notes §11-12): Slope 6 dB now plays the measured 6 dB forms - by ear
- Band-reject (filter notes §14): Filter Type BR now plays the measured notch, at either slope - by ear
- Unison (voices notes §26): String Pad / Happy ending / Ethereality - the doubled, slightly sharp copies left and right; Detune 127 about +50 and +35 cents - by ear against the instrument
- Off/On choices as toggle buttons (draw notes §16): every OFF/ON control flips on click, lit when on - check none still opens a menu and the named two-value choices still do
- Multi mode type 1 (filter notes §15): String Pad's lowpass + highpass in parallel on the two cutoffs - by ear against the instrument
- Multi modes 4 and 5 (filter notes §16): Ethereality's peaks and Happy ending's notches, Freq 2 widening them, Resonance deepening them - by ear
- The Sine's Shape (osc notes §12): Ethereality's sine, and Shape on a Sine swept by ear - growing brighter, as feedback FM does
- Unison as a mirrored pair a side (voices notes §27): String Pad and Happy ending beside the instrument - the beating no deeper than its, and the phasing at about 4 s in String Pad gone
- Voice stealing (voices notes §28): pad chords played quickly, past 24 voices - the attacks still heard
