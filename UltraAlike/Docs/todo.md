# UltraAlike todo

## First
- Panel positioning (CT 2026-11-17): review against `../Docs/panel-layout.md` when the GUI gets its overhaul (it needs lots of work anyway) - plug-in settings are a section called GLOBAL, not PLUG-IN; Voice Limit, Same Note, Steal in the shared order and names; rows left to right in signal order
- The E5000 Ultra is on the rig (2026-11-16): MIDI TM-1 ch 9, audio QU-24 17/18; parameter SysEx, preset dumps and SDS test samples all work
- Output polarity switch (owner 11-16): the E5000's outputs are inverted, a known design flaw some owners fix in hardware - offer Normal / Inverted (as the hardware); measured 11-16: Main L/R inverted, Sub 1 normal, otherwise identical within 0.2 dB
- Playback path measured at the sample's own pitch (11-16): THD 0.004%, a -1.3 dB dip at 8-10 kHz recovering by 16 kHz, -5 dB at 20 kHz; the filter wide open is out of the path - reproduce that response
- Emulator X3 as the first reference (owner 2026-11-14): its `.exb` banks, then its filters, envelopes and modulation, measured
- Bank playback BUILT 11-14 (Emulator X `.exb` via SynthLib emuBank, zones by key and velocity, root, loops): the amplifier envelope BUILT 11-14 (uaBankVoice notes §3, straight segments - check their shape against X3); next each voice's tuning, level, pan, the filter and auxiliary envelopes
- `.E4B` banks BUILT 11-21 (SynthLib emuBank §8), zone level and pan played: next each voice's filter from the bank, then the rest of the voice
- SFZ instruments BUILT 11-21 (emuBank §9), `ampeg_*` envelopes as E5000 rates (uaEosEnv §13), Prev/Next past a bank's end into the folder tree (uaPlugin §20): `trigger=release` (uaBankVoice §13); next `ampeg_hold`, `offset`, `pitch_keytrack`, `seq_length` round robins
- Kontakt instruments (.nki, owner 11-21 "if possible"): the owner's 7,215 are pre-Kontakt-5 - 92% the 2-4 generation (compressed binary), 8% the 1 generation (zlib XML), WAVs beside them, no monoliths. Waiting on whether UltraAlike may name the format publicly
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
- E5000 voice stealing (11-16, uaBankVoice notes §11): measure which voice the E5000 takes at 64 (held, and half released) - notes 50+ ms apart, check each sounds alone, P000's voice/zone count first - then set Steal's default to it
- Filter, measured on the E5000 (owner 11-16: worth it - nearly every preset's voice has its filter envelope on the filter by a default cord): (1) the filter types the libraries use most, likely the classic 2- and 4-pole lowpasses first, Z-plane morphs after - type/cutoff/resonance set over SysEx, noise captured and fitted; (2) the filter envelope - six stages, uaEosEnv probably shared, confirm its rate law; (3) the FiltEnv -> filter cord, its amount from each bank preset (reportedly linear in the stored bytes - confirm); (4) the general cords as the modulation matrix, LFOs with it
- Velocity and key crossfades (owner 11-16): the bank reader takes only each window's low and high edges (SynthLib emuBank.c ETW_LOW/ETW_HIGH), not E-mu's fade-in/out widths, so crossfaded layers switch hard - read the fade widths and give each zone its gain at note-on; check against the E5000 with a velocity sweep through a crossfaded preset
- Aftertouch (11-16): channel and poly pressure as an E-mu cord source - with the modulation matrix
- Display, owner 2026-11-22: show the wave of a bank's sample (only a plain WAV is drawn now); show which key ranges are in use and the velocity layers (a keyboard strip with each zone's span, layers stacked by velocity, the sounding one lit)
