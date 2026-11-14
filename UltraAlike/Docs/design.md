# UltraAlike design

Placeholder, started 2026-11-14 from OneShot's skeleton (renamed, new VST3 class ids and AU subtype `Ultr`), so it
builds and plays a sample as OneShot does. A reproduction inspired by the E-mu Ultra samplers, measured against an
E5000 Ultra. `Docs/code-notes/` came across from OneShot: rewrite each as its file changes.

## The instrument to reproduce

An EOS 4 sampler: presets of voices, each voice a set of zones (a sample mapped across a key and velocity range,
with root key, fine tune, volume and pan); per voice a Z-plane filter, amplifier, filter and auxiliary envelopes,
two LFOs, and patch cords routing modulation sources to destinations. Banks hold presets and the samples they use.

## What already exists to build on

- G2Alike's Sampler module plays E-mu Emulator X banks (`.exb` with its `SamplePool` of `.ebl`):
  `G2Alike/src/nordSample.c`, G2Alike `Docs/code-notes/nordSample.c.md` §11. Emulator X is the software successor of
  the EOS banks, so its zones, voices and envelope chunks are a close map. Envelopes and filter are not decoded there.
- The EOS `.E4B` bank format is documented publicly (mpc2emu's `docs/E4B_FORMAT.md`, GPL-2.0-or-later; emu3bm, GPL-3):
  preset header, 284-byte voices, 22-byte zones.
- OneShot (this repo): a one-sample player on SynthLib - sample loading, a note played from a resampled copy, an
  envelope - the nearest skeleton for a sampler.

## Emulator X3 first (owner, 2026-11-14)

Until the E5000 Ultra is on the rig, reproduce E-mu's Emulator X3 - the software sampler the Ultra line led to,
whose banks (`.exb` with their `.ebl` samples) G2Alike already reads. Its presets, voices and zones are the same
structure; its filters, envelopes and modulation are what to measure first, then compare with the E5000 Ultra.

## Open decisions

- Skeleton: OneShot (a sample player already) or a synth skeleton with sample playback added.
- Content: reading `.E4B` banks (and Emulator X `.exb`) directly; how samples are found and kept in memory.
- Validation rig: the E5000 Ultra's outputs into the QU-24, and how banks reach it (SCSI or MIDI sample dump).

## Bank playback (2026-11-14)

Built: SynthLib's `emuBank.c` reads an Emulator X bank (from G2Alike's Sampler reader) and a layer plays one of its
presets in place of a sample (`code-notes/uaPlugin.md` §12, `uaBankVoice.md`). Next, each voice's own settings from
the bank: tuning, level, pan, then its envelopes - compared with Emulator X3, then the E5000.

## Order of work (proposed)

1. Bank reading: `.E4B` presets, voices, zones and samples into a playable map - shared with G2Alike's Emulator X
   reader where it can be (a SynthLib candidate).
2. Playback: zone selection, pitch, loops, the amplifier envelope.
3. The Z-plane filters, envelopes, LFOs and cords - measured on the instrument.
