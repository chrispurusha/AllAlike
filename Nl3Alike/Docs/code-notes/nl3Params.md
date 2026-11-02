# nl3Params.c - the parameters and the panel's pages

## 1. Ranges
Each dial's plain range is the panel's 0..127, as the instrument's controls are, so a value read from a patch or a
controller maps straight on. The ids are appended to and never reordered: host projects save them.

## 2. Pages
Pulse2Alike's page names and section colours, so the two plug-ins read alike: OSCILLATORS, FILTER, AMP, ENVELOPES,
LFOS, MOD, EFFECTS, ARP, GLOBAL. A page appears only once something on it is built, so the panel shows what plays;
for now OSCILLATORS, FILTER, AMP, ENVELOPES (AMP ENV) and GLOBAL. Volume sits under AMP's MASTER and the voice limit under GLOBAL's PLUG-IN,
where Pulse2Alike puts its own.
