# TODO

One line per item. Measurements and reasoning go in `findings.md`, design in `design.md`.

## First
- P001-P015 close by ear (CT 2026-11-06) apart from P003, the only one of them with drive on (FUZZ, Drive 17) - the drive rebuild below is next; then listen on from P016
- Presets, still to do (design §7): Restore Presets to Pulse 2 (writes only slots that differ, never P500) - waits for CT; Store Sound to Pulse 2 Slot is built (2026-11-06, in to-test.md)

## Measured, close by ear (2026-11-06) - only if a later sound needs them
- P012: the instrument's filter drifted up since 10-17 (+0.6 semitone at cutoff 0 to +0.1 at 88; findings 2026-10-23); TUNE FILTER, then re-measure cutoff 0/40/64/88 at note 60, before any filter refit
- P012: after ~10 s idle the first note seemed to glide its filter in (slow LFO 1 on VCF ENV AMT and RESONANCE; glide 10)
- LEVEL2 below osc 2 Level 101 1.5-4 dB loud (findings 2026-10-25); the synced pulse against osc 3's square (0.78); P046, P073, P160, P228 slightly worse with sync/LEVEL2 in
- LEVEL2 (P006, P015): the engine's slew and loop saturation compress loud fast signals more than the instrument (2 kHz takes, open triangle) - refit with the filter
- P002: a steady +2.2 dB loud against a fresh take (osc 3 on VCF at Level 51, cutoff 13); its attack
- P001: its release ~3.6 dB short
- P006: 1-4 kHz 3-7 dB bright; first 150 ms ~3 dB quiet; 2nd-4th harmonics 4/5/10 dB under over 7.5 s; its tail's VCA knee ~0.1 s early
- Filter envelope short attacks: measure with noise through the filter (p2Plugin notes §43); P004's burst; redo `fenv_attack/` with Filter Velocity set (at 0 the envelope did not open)
- The first ~4 ms of a note over a release: the instrument's first half-cycle is smoother (P004)
- The thump (VCA feedthrough, p2Plugin notes §43): Decay 0 too big (0.149 against 0.103), Attack 20's small positive bump

## Drive
- NOW (planned CT 2026-10-26): rebuild the drive model as an OTA-style gain stage plus a fixed per-curve clipper, every per-Drive value one smooth function of Drive - plan in p2Drive notes §9; the waveform refit (§8) is in and "sounding much better", not identical
- The output gate is in (p2Plugin notes §55) and the fitted quiet-input loss is off (p2Drive notes §7); P003 gates off as the instrument's does
- FUZZ overall (CT 2026-10-25): better but still not close by ear; after the §8 waveform refit (10-26) Drive 0/64 close, 127 slightly worse, even harmonics still not growing with Drive
- FUZZ at quiet Levels (14-20): the instrument still clips and lifts quiet signals (+3.8 dB against NONE at -45 dBFS in, BP12), the engine does not (0) - more gain into the clipper; refit running (`drive_shape.py 2 gated`)
- TUBE: the gated refit gained nothing (8.2 dB rms, a parameter at its bound) - its structure (§9: Drive 127 is a near-switch the present chain cannot make); a level trend of -3..-5 dB at Levels 14-24 easing to 0 by 80
- Noise through the drive (2026-11-06, `192k/drive_noise/`): TUBE 1-3 dB quiet (4-5 against NONE), worst 1-4 kHz; FUZZ close but 15 dB short at 125 Hz through BP12 at Drive 0; NONE's noise level and colour fixed (p2Plugin notes §64) - check the rebuilt drive on these
- FUZZ at quiet EXT inputs: even harmonics ~15 dB above the instrument's (p2Drive notes §6)
- Rig: osc 3 on DRIVE raises the baseline by about 8 drive units at any level - find why
- The drive-on output gate's thresholds were measured with FUZZ; check one TUBE point

## Rig
- Dedicated capture machine (2026-11-06): the Intel MacBook, Fireface UC inputs 7/8 (indices 6/7), USB MIDI channel 12 (design §5); tools built there, Python in ~/.venvs/p2rig; captures copied to ~/Captures/Pulse2 (set P2_CAPTURES)
- Regression references: use `192k/patches1025/` (fresh 2026-10-25); the 10-17 takes in `patches/` misled on P001 and P010
- Build a known test sound over CCs (one osc, semitone/detune centred, filter open, envelopes flat) before every capture
- INPUT GAIN is 0 (CT 2026-11-06); record it with every EXT capture, and sweep the EXT level per step

## Oscillators
- The toggle flip (p2Dco notes §12): built for the plain pulse under Anomalies, at 0.68 per crossing from the long-period range; still to do: APW, osc 3's square and its stepped triangle, and the odds' dependence on the note it came from (the timer's phase)
- Amplitude against pitch and Level: corrected (p2Plugin notes §63, 2026-11-06); left: the high-Level squeeze is a saturation whose wave shape is not modelled, only its level; unison/para/X-PWM levels against pitch unmeasured
- The triangle (findings 2026-10-25): at Level 127 the filter's open-cutoff saturation rounds it (h41-61 15-22 dB under its own Level-40 shape; the instrument keeps 12 dB/oct to h61), and the instrument's own shape has weaker low odd harmonics (h3 -25, ideal -19) and even ones at 6-10 dB/oct (h16 -57, h32 -67: an asymmetry or a small step at the reset) - rework the open filter's saturation with the pulses' top end
- Normal-mode pulses at full Level keep +2.4 dB at 16 kHz: the filter's saturation re-sharpens the softened edge (p2Dco notes §11)
- Osc 3's square against a synced osc 2 (polarity audit, findings 2026-10-24): where its edges sit relative to the sync point
- P014 3.6 -> 4.1 after APW's polarity fix (APW mixed with other sources) - check APW's own shape against its two-cycle period
- Osc 3 EXT is not built: the plug-in has no audio input - a side-chain input, as G2Alike's
- X3-PWM with sync (P053, bands 11-13 dB) and P052's attack: X1/X3 are built (p2Plugin notes §8); look at P053's filter (cutoff 28, env 127) and the sync's first-edge rule with X3
- The saw's reset: each ramp ends in a ~0.3-0.5 ms flat stretch (seen synced, findings 2026-10-24) - measure it unsynced against pitch
- Model the ramp's shape and reset spike against pitch (the pulse's duty is settled - design §2.2)
- How PWM from the mod matrix adds to the width word (the dial is dial << 8, confirmed)

## Filter
- Retake the LP12/BP12/HP12 resonance-92 and LP24 resonance-88 sweeps at −45 dBFS (LP24 confirmed compressed at −30)
- Level sweeps (−40..−6 dBFS) per mode at a few settings, for the nonlinearity
- LP12's stopband: ~2.5 dB high at 1-2.5 kHz at cutoff 0
- The modulation step (findings 2026-11-06): matches on cutoff; on pitch the engine's 1 ms step is a little more abrupt (63% in its first quarter against 54%) - soften pitch's glide slightly if it is ever heard

## Engine
- The VCA's release tail (p2Env notes §4a): 3-4 dB low 0.2-0.3 s into a release at low volume; the knee reached a little early
- Library sweep leftovers (findings 2026-10-24): P017 3-7 dB short after the thump (findings 2026-10-25); P018 UNISON-P with Attack 97: attack 13 dB and sustain 4 dB quiet; P051 harmonics (osc 2 synced 29 st over osc 3, LFO 2 on osc 2 pitch: the instrument's energy higher); P102 cannot be compared (unison at Detune 0 is a random draw), P052 X3 attack (9)
- Mod matrix: every target takes the modulation in its own dial steps (p2Plugin notes §51, §54); checked on cutoff, pitch, pan and the envelope rates - spot-check osc levels, volume, LFO 1 speed, PW, resonance against captures
- Capture to confirm the math sources: M-DELAY's time against amount (16 ms per step?), M-SMOOTH's rate, M-PRODUCT/MIN/MAX on two LFOs
- Osc 3's LEVEL2 / VCF / DRIVE depths and FB (depths are guesses beyond LEVEL2 at 8 Hz)
- FB alone sings on the instrument from FB 64 (cutoff 64, ~950 Hz, -40 dB); the engine does not - frequency-dependent feedback?
- Amp Velocity: the engine moved P011 1.7 dB more than the instrument (velocity 100, Amp Velocity 95) - check the law away from velocity 64/127
- Arp Pattern (0-14): what it selects
- PITCH FLW now follows the glide and the bend (the manual; 2026-11-06) - 64 presets use it, P004 and P006 among them: recheck those two by ear and in the regression
- CLASS (sound byte 127, 0-21: ARP, ATMO, BASS ... INIT) could fill the preset browser's Category column
- Arp SOFT and HARD: measure their velocity offsets (the plug-in uses -32 and +64)
- Arp, by program change: does a chord kept by one HOLD sound start again after a non-arp sound, in the next HOLD sound? (the plug-in does so with Anomalies on - p2Plugin notes §45)
- The 1 kHz leak (−96 dBFS with a note sounding): decide whether to model it

## Plug-in
- Value text in real units (times, rates, semitones) where the laws are measured; the panel shows raw values now, as the instrument does
- SynthLib: maximise the C and reduce the C++ (CT 2026-10-21) - port src/fileBrowser.cpp, bankBrowser.cpp and prefs.cpp to C; the VST3 wrapper and views stay C++/Obj-C++ as the SDK requires

## DO NOT RE-TRY
- Four one-pole stages with LP24 at the end (per-stage poles): worse than the free-mix fit, implausible stage ratios, no HP notch (findings, first linear filter fit)
- Fitting an engine constant to one take of a patch with detuned oscillators: their slow beat moves a band by up to 10 dB take to take (P006's LEVEL2 depth of 3, reverted)
- Drive structures already rejected against the level sweep - p2Drive notes §4
