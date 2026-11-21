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

## Reference order (owner, 2026-11-14)

Emulator X3 first for everything - envelopes, LFOs, cords, the Z-plane filters - and the E5000 Ultra later. Its
filters are said not to match the hardware exactly, so a filter matched to X3 is marked as such and checked again on
the E5000 before it counts as the instrument's.

## Open decisions

- Layering (owner, 2026-11-14: "some kind of layering, but different"; the four tabs REMOVED the same day - one preset or sample an instance): the tabs were OneShot's and not the
  instrument's. On an EOS sampler the layers are a preset's voices (key and velocity windows with crossfades, each its
  own tuning, filter and envelopes), then up to eight linked presets, then the 16-channel Multisetup. Likely: one
  preset per instance with its voices as the layering, links next, Multisetup left to the host; the tabs replaced by a
  view of the voices or links.

- Skeleton: OneShot (a sample player already) or a synth skeleton with sample playback added.
- Content: reading `.E4B` banks (and Emulator X `.exb`) directly; how samples are found and kept in memory.
- Validation rig: the E5000 Ultra's outputs into the QU-24, and how banks reach it (SCSI or MIDI sample dump).

## Bank playback (2026-11-14)

Built: SynthLib's `emuBank.c` reads an Emulator X bank (from G2Alike's Sampler reader) and a layer plays one of its
presets in place of a sample (`code-notes/uaPlugin.md` §12, `uaBankVoice.md`). Next, each voice's own settings from
the bank: tuning, level, pan, then its envelopes - compared with Emulator X3, then the E5000.

EOS `.E4B` banks too (2026-11-21): the same reader opens them (SynthLib `Docs/code-notes/emuBank.c.md` §8) - presets,
voices, zones, the samples held inside the bank (mono and stereo), loops, the amp envelope, and each zone's tuning
(its voice's transpose, coarse and fine tune plus its own, applied when a note starts). `tools/uaBankTest.c` plays a
preset's notes through the bank voices, checks every pitch step against the zone's root and tuning, and writes a WAV.
Still read from the bank but not yet played: each voice's level and pan, the filter, the other envelopes, LFOs and
cords.

## Numbering (owner, 2026-11-16)

EOS counts presets and samples from zero (P000, S000): wherever UltraAlike shows a preset's or sample's number, it
counts the same way.

## Measuring the E5000 Ultra (2026-11-16)

The rig: the E5000's main outputs into the QU-24 (inputs 17/18) and, for 192 kHz work, a Fireface UC; MIDI through
the Elektron TM-1 on the E5000's basic channel 9. Everything is driven over MIDI, without touching the panel:

- E-mu's published EOS remote SysEx ("Remote Preset Editing via MIDI SysEx", `F0 18 21 <dev> 55 ...`) selects a
  preset and voice and sets any parameter by id, reads it back, dumps a preset, names, copies and deletes presets
  and erases samples. Pace every message (50 ms or more; erasing a large sample takes longer) and read the unit's
  identity back between batches - bursts sent without a gap can hang it. Preset dumps use the original dump format.
- Test samples go in by MIDI Sample Dump (EmuUtility's backdoor `SENDSAMPLE`), about 70 ms a packet. SDS words are
  offset binary, not signed.
- A preset is made playable with Bank Select (MSB 0, LSB = preset / 128) and Program Change on the basic channel;
  selecting it by SysEx edits it without playing it.
- One preset (P000 "Test Neutral") and three samples (S001-S003) are kept on the instrument as a bank: test signal 1 (impulse,
  log sweep, 1 kHz tones at -1 and -20 dBFS, noise), test signal 2 (an impulse, then 32 single tones 500 Hz-21 kHz)
  and a looped 1 kHz sine for envelopes and LFOs.

Measured so far: at the sample's own pitch the output is clean (THD 0.004%, noise -101 dBFS) and within -1.3 dB to
16 kHz; the master volume is a plain digital gain on every output; the main outputs are inverted and the sub outputs
are not; and the interpolation is seven fixed kernels chosen by playback ratio (`code-notes/uaEmuInterp.md`), which
the plug-in reproduces to 0.1-0.4 dB in the passband and without bias in the images up to the fifth.

## Order of work (proposed)

1. Bank reading: `.E4B` presets, voices, zones and samples into a playable map - shared with G2Alike's Emulator X
   reader where it can be (a SynthLib candidate).
2. Playback: zone selection, pitch, loops, the amplifier envelope.
3. The Z-plane filters, envelopes, LFOs and cords - measured on the instrument.
