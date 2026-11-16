# UltraAlike todo

## First
- The E5000 Ultra is on the rig (2026-11-16): MIDI TM-1 ch 9, audio QU-24 17/18; parameter SysEx, preset dumps and SDS test samples all work
- Output polarity switch (owner 11-16): the E5000's outputs are inverted, a known design flaw some owners fix in hardware - offer Normal / Inverted (as the hardware); measured 11-16: Main L/R inverted, Sub 1 normal, otherwise identical within 0.2 dB
- Playback path measured at the sample's own pitch (11-16): THD 0.004%, a -1.3 dB dip at 8-10 kHz recovering by 16 kHz, -5 dB at 20 kHz; the filter wide open is out of the path - reproduce that response
- Emulator X3 as the first reference (owner 2026-11-14): its `.exb` banks, then its filters, envelopes and modulation, measured
- Whether it stays public here (owner); the skeleton is OneShot's, renamed
- Add to do-plugins and do-release when it is more than a renamed OneShot
- Bank playback BUILT 11-14 (Emulator X `.exb` via SynthLib emuBank, zones by key and velocity, root, loops): the amplifier envelope BUILT 11-14 (uaBankVoice notes §3, straight segments - check their shape against X3); next each voice's tuning, level, pan, the filter and auxiliary envelopes
- `.E4B` banks (EOS) beside `.exb`
- G2Alike's Sampler onto SynthLib's emuBank, its own copy removed
- Interpolation BUILT 11-16 (uaEmuInterp, Anti-alias Off): matches the E5000 to 0.1-0.4 dB passband; kernels 5-6 images 5-9 dB too loud - refit them
- Anti-alias Off exact at any host rate: render bank voices at 44.1 kHz and resample to the host, so products fold where the instrument folds them
- The output stage's own response (-0.5 dB 3-9 kHz, -1.4 dB 20-21 kHz at the root pitch) - add with Anti-alias Off
- Panel is the E5000's (11-16, uaParams §14). Still to come: Solo modes' differences, Glide, Chorus, the waveform's envelope overlay (still drawn from the hidden OneShot dials), filter and LFO sections
- Remove OneShot's leftovers once nothing needs them: the sample player (uaVoice, uaCache, uaStretch), trim, effects, hidden params
- Envelopes: shape and level law BUILT 11-16 (uaBankVoice §5); still to do - the attack's exact curve (depends on the depth), the per-stage speeds for EOS banks, the amp-envelope depth per voice, filter and auxiliary envelopes
- Emulator X's envelope stages carry one unknown byte each (SynthLib emuBank §5) - check whether it is the EOS speed
- LFOs, filters next (owner 11-16), measured on the E5000; E-mu's patch cords as the other Alikes' modulation matrix, in their terms
- Then the Z-plane filters, LFOs and patch cords, matched to Emulator X3 first; the filters checked again on the E5000 (X3's are said not to match the hardware)
