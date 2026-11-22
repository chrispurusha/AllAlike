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

## 4. Values in their units
One function (`sn2_param_units`) for the panel and the host, from the engine's own laws: levels in dB below the dial's
top, times from the envelopes' and LFOs' steps (a decay or release as its time to half, a fade out likewise), LFO
speed in Hz or the note it is locked to, cutoff where the filter rings (sn2Filter notes §8), Sync, Skew and Fine as pitch, Unison Detune from the
flattest voice to the sharpest. What has no honest unit keeps its number - Resonance, Overdrive, Soften, the route
amounts - and so does an FM depth. The unit stays while a dial is dragged (owner, 2026-11-22).
