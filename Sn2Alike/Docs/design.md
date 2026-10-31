# Sn2Alike design

Started 2026-10-30 as a reproduction inspired by the Novation Supernova II. The skeleton comes from TiAlike:
same layout, same SynthLib wrappers, new identifiers.

## State

The plug-in builds as VST3 and AU (`aumu Sn2a CPur`). The filter is built and measured (see `code-notes/sn2Filter.md`); oscillator, envelope and most of the panel are still placeholders. Compare filters with `tools/sn2filtertest`.

## The voice to build

Three oscillators, each Square, Saw or Double Saw (or an audio input), with Sync, Hardness, Soften, pulse Width, Sync Skew and Formant Width; noise; ring modulators 1x3 and 2x3; FM 1x3; two LFOs (square, saw, triangle, sample and hold); three envelopes with AD repeat; a filter of 12 types (low, band and high pass, their resonant forms, notch, and the doubled forms) at 12, 18 and 24 dB, with Overdrive; 566 parameters in all. It differs enough from TiAlike that only the skeleton, not the engine, carries over.

## Patches

A program is a 297-byte main dump and a 99-byte extension, Novation SysEx. Every field is listed in `program-format.md`. A performance is command 07, 383 bytes, not decoded yet.

## Validation

By spectrum and level over a note, in the box, starting from a real program with the fields under test edited, as a program left unset is not neutral. Renders repeat to about -25 dB at one block size, so nulls are only a rough check.

## Measured so far

- Square and Saw share their odd harmonics. Square is a pulse, and Width runs from silence at 0 through square at 64.
- Soften is a steep cut at a harmonic number, not a slope: about the 2^(Soften/16)th harmonic of the note, so it doubles every 16 steps and the tone holds across the keyboard. It is above the 47th harmonic of the note from about 80 up.
- Every harmonic carries small sidebands about 2 Hz away. Their source is not known.

## To decide

- The engine's scope: parts and performances, or one program per instance as TiAlike.
- What the panel should look like once its controls are known.

Each section of the engine should be reproduced by measurement and recorded here as it is settled.
