# sn2Params.c - the parameters and the panel's pages

## 1. Ranges
Each dial's plain range is the panel's 0..127, as the instrument's controls are, so a value read from a patch or a
controller maps straight on. The ids are appended to and never reordered: host projects save them.

## 2. Pages
The page names and order every Alike shares (CT 2026-11-06), with Pulse2Alike's section colours: OSC, FILTER, AMP, ENV, LFO, MOD, then the instrument's own pages (ARP, TABLES beside OSC, the effects), GLOBAL always last. A page appears only once something on it is built, so the panel shows what plays;
for now OSC, FILTER, AMP, ENV, LFO, MOD, EFFECTS and GLOBAL. Volume sits under AMP's MASTER and the voice limit under GLOBAL's
PLUG-IN, where Pulse2Alike puts its own; it starts at 16 voices, as TiAlike's does (CT 2026-10-30). MOD holds the bend and
wheel: both live controls (Bend shown -8192..8191, as the pitch-bend message), each oscillator's bend range, the wheel to
each oscillator's pitch and level, and the wheel to cutoff and resonance. The wheel's other routes are not built.
