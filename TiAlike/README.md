# TiAlike

A reproduction inspired by the Access Virus TI's single-patch sound engine, as a VST3 and Audio Unit instrument.

One instance plays one Single: patches are loaded per instance, with an adjustable voice limit in poly mode. There
is no Multi or performance mode. Each section of the engine is reproduced by measurement and checked against
reference renders sample by sample (see [`Docs/design.md`](Docs/design.md)).

**Beta.** A loaded Single plays through every oscillator model (Classic, HyperSaw, Wavetable, Wave PWM, Grain and Formant), Osc 3, sub, noise, ring, sync and FM, both filters (the analog modes included), the envelopes, three LFOs, the matrix, unison, the mono modes and the arpeggiator, then Distortion, the Filter Bank, Phaser, Chorus, Delay (Tape included), Reverb and the EQ, in stereo. Each oscillator can also play a wavetable of your own, loaded from a PPG, Serum-style or Waldorf file, and a Global switch turns anti-aliasing off for the instrument's own aliasing. Not yet: the vocoder and input follower, soft knobs, the other Chorus and Character types, Reverb Feedback, Distortion Quality and Tone. What is built but still being matched to the instrument is in [`Docs/todo.md`](Docs/todo.md).

## Relationship to the sibling projects

One of a family that shares [SynthLib](https://github.com/chrispurusha/SynthLib) and its format-free plug-in
wrappers; the panel follows Pulse2Alike's.

- **Pulse2Alike**: a reproduction inspired by the Waldorf Pulse 2
- **G2Alike**: editor for the Nord G2, and the G2Alike plug-in. The capture and analysis tools live in its `tools/`.
- **SynthEdit**, **EmuUtility**, **GenBridge**, **MidiSyncTool**, **MidiSeqTool**: editors and plug-ins on the same
  library.

## Building

```
git clone --recurse-submodules https://github.com/chrispurusha/SynthLib.git ../../SynthLib
./do-plugin            # VST3 and Audio Unit, and installs them
./do-plugin au         # one format
./do-plugin --no-install
./do-uncrustify        # after editing sources
```

The VST3 half needs the VST3 SDK at `~/Documents/vst3sdk` (or `$VST3_SDK`); the Audio Unit needs none.

## Installing

From a release's `.dmg`: copy `TiAlike.vst3` into `/Library/Audio/Plug-Ins/VST3` and `TiAlike.component` into
`/Library/Audio/Plug-Ins/Components`. The builds are not notarized, so clear the quarantine before a host will load
them:

    sudo xattr -dr com.apple.quarantine /Library/Audio/Plug-Ins/VST3/TiAlike.vst3
    sudo xattr -dr com.apple.quarantine /Library/Audio/Plug-Ins/Components/TiAlike.component

## Releasing

    ./do-release 0.1.0-beta.1     # the first release (there is no tag yet)
    ./do-release beta             # after that: beta / patch / minor / major / final bump the newest TiAlike-V* tag

Builds both plug-ins (universal, ad-hoc signed) into a `.dmg` on the Desktop with a `Read Me First.txt`, the licence
and the third-party licences. It never tags: it prints the `git tag` command to run once the `.dmg` has been checked.

## Licence

GPLv3 - see [`LICENSE`](LICENSE). Third-party components and their redistribution obligations are in
[`THIRD_PARTY.md`](THIRD_PARTY.md).

Portions of this software are copyright (C) The FreeType Project (www.freetype.org). All rights reserved. This
software is based in part on the work of the FreeType Team.
