# TiAlike - design

Living design note, started 2026-10-26.

## 1. What it is, and what it is not

A plug-in (VST3 and AU, on SynthLib like Pulse2Alike and G2Alike): a reproduction inspired by the Access Virus TI's
single-patch sound engine.

**In scope (CT, 2026-10-26):**
- **One single patch per instance.** An instance plays one Single, the way Pulse2Alike plays one sound. Another
  sound is another instance.
- **Loading patches** from the formats Virus banks come in: `.syx` dumps and Standard MIDI files (`.mid`) carrying
  them, a whole bank browsable and any Single loadable into the instance.
- **An adjustable voice limit** for the patch in poly mode, per instance. Mono stays a property of the patch.
- The instance's state - the patch and the voice limit - is saved in the host's project, as Pulse2Alike's is.

**Out of scope:**
- **Multi mode and performance mode** - no parts, no multi-timbral setup, no Multi dumps.
- The hardware's own front panel, its USB/host integration, and anything that only exists to drive the hardware.

**Sources.** This repo states only what has been measured: renders of the reference instrument, made in the box
and compared sample by sample with this engine (§3), and what the patch format itself shows. A law in the code is
there because it reproduces those renders.

## 2. Patches

**2.1 The Single.** A Single dump is `F0 00 20 33 01 <device> 10 <bank> <program> <data> F7`. A/B/C-era Singles
carry 256 data bytes and a checksum; TI Singles carry 512, two 256-byte pages each followed by a checksum, 524 bytes
in all. The name is data bytes 240-249. Bank 0, program 0x40 is the single-mode edit buffer; device 0x10 is omni.

**2.2 The banks on hand.** CT's `Patches/VirusPatches` (iCloud): 46 `.mid` sound sets and 5 `.syx` banks
(`VirChrisBankA`-`D`, 128 TI Singles each), 30 of the files a full 128-Single bank. Which of them are TI Singles
and which A/B/C (256 bytes) decides how much of the older format the loader must take.

## 3. Validation: in the box

The engine is checked against renders of the reference instrument made by an offline VST3 renderer: the same
script of notes, controllers and SysEx played into both, written to WAV, compared sample by sample. There is no
converter, rig offset or noise floor in the way, and the reference renders are bit-identical run to run, so a
difference is a difference in the engine.

Every comparison starts from a known Single sent as SysEx into the edit buffer, then changes one control at a time.

## 4. Order of work

1. The renderer and a patch extractor (built; kept outside this repo for now).
2. An INIT Single, then one-control sweeps: oscillators, filters, envelopes, LFOs - the first laws.
3. The plug-in skeleton, on Pulse2Alike's layout: one engine per instance, the patch loader, the voice limit.
4. Section by section, each confirmed against the reference renders before the next.

## 5. Open questions

- **The effects.** A Single carries its own effects (delay, reverb, chorus and the rest). Which of them come in,
  and when.
- **Oscillator models.** The TI has several (classic, HyperSaw, wavetable, wavetable PWM, grain, formant). All of
  them, or a first set.
- **Arpeggiator, vocoder, the inputs.** Part of a Single, but not needed for most patches.
- **The voice limit's range and default.**
- **A/B/C Singles.** Load them too (mapped onto the TI engine), or TI Singles only.
