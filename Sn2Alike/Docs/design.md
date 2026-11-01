# Sn2Alike design

Started 2026-10-30 as a reproduction inspired by the Novation Supernova II. The skeleton comes from TiAlike:
same layout, same SynthLib wrappers, new identifiers.

## State

The plug-in builds as VST3 and AU (`aumu Sn2a CPur`). All 12 filter types and all three oscillators (Saw, Square, Double Saw, with pitch, Width, Soften and Level) are built and measured (see `code-notes/sn2Filter.md`, `sn2Osc.md`). Noise, ring, FM, modulation and the envelope laws are still to come. Compare filters with `tools/sn2filtertest`.

## The voice to build

Three oscillators, each Square, Saw or Double Saw (or an audio input), with Sync, Hardness, Soften, pulse Width, Sync Skew and Formant Width; noise; ring modulators 1x3 and 2x3; FM 1x3; two LFOs (square, saw, triangle, sample and hold); three envelopes with AD repeat; a filter of 12 types (low, band and high pass, their resonant forms, notch, and the doubled forms) at 12, 18 and 24 dB, with Overdrive; 566 parameters in all. It differs enough from TiAlike that only the skeleton, not the engine, carries over.

## Patches

A program is a 297-byte main dump and a 99-byte extension, Novation SysEx. Every field is listed in `program-format.md`. A performance is command 07, 383 bytes, not decoded yet.

## Validation

In the box, starting from a real program with the fields under test edited, as a program left unset is not neutral. Bypass effects through the live parameter: editing the program does not turn them off. Oscillators free-run, so align each render to a fraction of a sample; renders then repeat to about -44 dB at any block size, and nulls are a real check.

## Measured so far

- Square is two saws, one offset by Width, so Width runs from silence at 0 through square at 64.
- Soften widens the edge of the wave, doubling its reach every 16 steps, so the tone holds across the keyboard; above that limit pitch sets the width.
- Sidebands seen on every partial in early renders were the chorus. With effects bypassed through the live parameter, L and R are identical and Saw harmonics fall as 1/k.

## Sample rate

The reference runs at one fixed rate whatever the host asks: 39062 Hz, solved from where its aliases fall at five notes, with a top end stopping near 19.5 kHz. Its output is converted to the host rate, so levels and spectra do not change with the host. Sn2Alike does the same, through SynthLib's resampler, so the sound and its top end hold at any host rate.

## To decide

- The engine's scope: parts and performances, or one program per instance as TiAlike.
- What the panel should look like once its controls are known.

Each section of the engine should be reproduced by measurement and recorded here as it is settled.
