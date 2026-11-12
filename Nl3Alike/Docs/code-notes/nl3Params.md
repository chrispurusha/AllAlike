# nl3Params.c - the parameters and the panel's pages

## 1. What the parameters are
The plug-in's own settings (voice limit, master, arithmetic, anti-alias, program and bank, bend, wheel, pedal, the hardware link),
then **one parameter per field of the Nord Lead 3's Program**, in the order the instrument dumps them, then the four
morph groups (wheel, aftertouch, velocity, keyboard), 26 offsets each. A dial's plain range is the field's own:
0..127 for a knob, -128..127 for a morph offset, 0..n-1 for a switch. The defaults are the instrument's Init sound.
The ids are what host projects save.

## 2. Pages
OSC, FILTER, AMP, LFO, MOD, PLAY (glide, vibrato, voice, arpeggiator), a page per morph group (WHEEL, TOUCH, VEL, KBD),
GLOBAL - the order every Alike shares. Every field of the Program has a control, built or not: a sound read from the
instrument shows all of it. A section stays on one row and a row holds twelve controls, so OSC 2 and each morph group
are split into sections that fit. The panel is 780 x 560, room for four rows.

## 3. Fields
`gNl3Fields` gives each parameter's place in the unpacked Program body, in bits. Measured on the instrument: every
documented controller was sent to it and the program dumped (`nl3Patch.md` §1).

## 4. Switch names
In the order the manual lists them. Measured: each switch stores its panel position as its value (the controller's
ranges in order, every position dumped), and the Filter Type controller runs through the six single modes and then
the six multi modes. That the panel order is the manual's is not yet checked for the waveforms, the destinations and
the modulation types.

## 5. The sound
`nl3_sound_from_values` maps the parameters onto the engine as it stands, which is still a placeholder for most of
the voice: Saw and Pulse play (the other waveforms as Saw), Osc Mix balances the two, Noise 3 replaces Osc 2, the
filter's LP and HP at 12 and 24 dB are the instrument's (6 dB plays as 12, the other types as LP), the filter and amp
envelopes, the LFOs to pitch, cutoff and pulse width, the Mod Env to cutoff, glide, mono and legato. Osc Mod, sync,
FM, the multi filter modes, vibrato, unison, the arpeggiator and morph are not built yet. The scales here are interim.
