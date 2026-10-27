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
- The hardware's own front panel, its own host software and USB audio streaming. A link to a hardware Virus TI over
  USB MIDI comes later (§6).

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
2. The skeleton (built 2026-10-26): the infrastructure, the panel, a placeholder oscillator and filter, the voice
   limit - §5.
3. An INIT Single, then one-control sweeps against the reference renders: the filter and the oscillator first, then
   envelopes and LFOs - the first laws.
4. The patch loader and the panel's patch bar, early and on placeholders (CT 2026-10-26): a loaded Single keeps
   all its bytes (so it saves back unchanged), every parameter the engine has is driven from it, and the rest are
   kept but do nothing until their section is built - so real patches play, partly, from the start.
5. Section by section, each confirmed against the reference renders before the next: the oscillator, the
   envelopes, the LFOs and the modulation matrix, then the rest of the voice.
6. **Effects next to last, the arpeggiator last - if at all** (CT 2026-10-26).
7. The hardware link (§6), further down the line.

## 5. The project's infrastructure (2026-10-26)

As Pulse2Alike's: SynthLib as a submodule, `do-plugin` (VST3 and Audio Unit, universal, installed as built),
`do-release` (a `.dmg` with the licences), `do-uncrustify`, the licence and `THIRD_PARTY.md`. The panel is
Pulse2Alike's, driven by tables of pages and sections (`plugin/tiParams.c`), so it grows a section at a time as the
engine does. See `findings.md` for the first build's checks.

**Kept common with Pulse2Alike (CT 2026-10-27).** The same pages by name and colour - OSCILLATORS, FILTER, AMP,
ENVELOPES, LFOS, MOD, EFFECTS (TiAlike only), ARP, GLOBAL - each shown once something on it is built; the same patch
bar (Prev / Next / Load... / name), File menu and Presets menu. Presets are A001-D128, four banks of 128 as the
instrument's RAM banks, in one file; Load... reads a Single from a `.syx` or a sound set `.mid`. Where TiAlike needs
something Pulse2Alike lacks, it is added to both if it makes sense there. See code-notes/tiDraw.md §9-12 and
tiPatch.md.

## 6. A Virus TI over USB (later; CT 2026-10-26)

Connect a hardware Virus TI over USB, as Pulse2Alike links a Pulse 2: the instrument follows the instance's sound
(controller changes as they are made, a Single dump into its edit buffer when a patch loads), and a Single can be read
back from it. It also makes the instrument a second reference to measure against, through an audio interface as the
Pulse 2 is. To settle when it starts: which of the TI's USB MIDI ports carries SysEx and controllers, its device ID,
and whether the panel's edits reach a host (to follow the instrument as well as drive it). Like Pulse2Alike, nothing is
ever written to the instrument's memory without asking.

## 7. Open questions

- **The effects.** A Single carries its own effects (delay, reverb, chorus and the rest). Next to last in the order
  (§4); which of them come in is still open.
- **Oscillator models.** The TI has several (classic, HyperSaw, wavetable, wavetable PWM, grain, formant). All of
  them, or a first set.
- **Arpeggiator, vocoder, the inputs.** Part of a Single, but not needed for most patches. The arpeggiator is the
  lowest priority of all, perhaps never (CT 2026-10-26).
- **The voice limit's range and default.**
- **A/B/C Singles.** Load them too (mapped onto the TI engine), or TI Singles only.
