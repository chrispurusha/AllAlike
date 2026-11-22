# tiParams.c - the parameters and the panel's pages

## 1. Ranges
Each dial's plain range is the panel's 0..127, as the instrument's controls are, so a value read from a patch or a
controller maps straight on. The ids are appended to and never reordered: host projects save them.

## 2. Pages
The page names and order every Alike shares (CT 2026-11-06), with Pulse2Alike's section colours: OSC, FILTER, AMP, ENV, LFO, MOD, then the instrument's own pages (ARP, TABLES beside OSC, the effects), GLOBAL always last. A page appears only once something on it is built, so the panel shows what plays;
for now OSC, FILTER, AMP, ENV (AMP ENV) and GLOBAL. Volume sits under AMP's MASTER and the voice limit under GLOBAL's PLUG-IN,
where Pulse2Alike puts its own.

## 3. The live controls
Hidden, not saved, and bound to MIDI: see tiVoices.md §8. New parameters are appended, as the ids are what host projects save.

## 4. Values in their units
One function (`ti_param_units`) for the panel and the host, from the engine's own laws: volumes in dB below the dial's
top (Osc Volume below its centre, and past it the drive), an attack as its whole time, a decay or release as its time to
half, Sustain Time as "-" a fall's time to half or "+" a rise's whole time, rates in Hz or the note value they are
clocked to, Cutoff in Hz (Cutoff 2 with Link on: in Hz to the host, the panel keeps its own reading, tiPlugin notes §4), Env Amount in semitones,
EQ in dB and Hz, Delay Time and Predelay in ms, Porta as the time to the next note, Semi, Transpose and Bend in
semitones, Detune in cents, Unison Detune from the flattest voice to the sharpest. The filter's, third and fourth
envelopes' Attack is the fixed arithmetic's law; FLOAT's 0 to 2 are a block long. The middle EQ band's frequency between
its points is the straight line in octaves. What has no honest unit keeps its number.
