# AllAlike

Reproductions of synthesizers and their effects, and a sample player, as VST3 and Audio Unit plug-ins, each built on
[SynthLib](https://github.com/chrispurusha/SynthLib).

| Plug-in | What it is | State |
|---|---|---|
| [Pulse2Alike](Pulse2Alike/) | A reproduction inspired by the Waldorf Pulse 2 | Beta |
| [TiAlike](TiAlike/) | A reproduction inspired by the Access Virus TI | Beta |
| [Sn2Alike](Sn2Alike/) | A reproduction inspired by the Novation Supernova II | Beta |
| [Nl3Alike](Nl3Alike/) | A reproduction inspired by the Clavia Nord Lead 3 | Beta: every oscillator waveform and Osc Mod type, every filter type and multi mode, the envelopes, LFOs, morphs, Unison, vibrato, glide, Chord Memory and the arpeggiator. A few controls do nothing yet |
| [UltraAlike](UltraAlike/) | A reproduction inspired by the E-mu Ultra samplers | Alpha: plays EOS `.E4B` and Emulator X `.exb` banks, SFZ instruments, and loose WAV and AIFF, with the E5000's envelope and interpolation, measured. Filters, LFOs and modulation to come |
| [ModAlike](ModAlike/) | A Phaser, Chorus and Flanger as an effect, with stereo modes and Mix | Work in progress: plays in a host (Ableton Live) |
| [ProphAlike](ProphAlike/) | An instrument inspired by the Korg Prophecy | Work in progress: the Prophecy's Filter 1 is built (exact in fixed point, to -95 dB in floating point); its oscillators and the rest are still placeholders |
| [OneShot](OneShot/) | A layered sample player (WAV and AIFF): four samples an instance, each with its key range, envelope, filter, saturation and time-stretch |

What each one is missing, and what needs more work: [Docs/feature-status.md](Docs/feature-status.md).

## Building

SynthLib sits beside this repo:

```
git clone --recurse-submodules https://github.com/chrispurusha/SynthLib.git
git clone https://github.com/chrispurusha/AllAlike.git
cd AllAlike/Pulse2Alike && ./do-plugin
```

Each plug-in builds and releases on its own, from its own folder. See
[SynthLib's build notes](https://github.com/chrispurusha/SynthLib/blob/main/Docs/build.md) for what it needs.

## Releases

Every plug-in carries AllAlike's own version, from the `VERSION` file here: a build (`./do-plugin`, `./do-plugins`,
the Windows scripts) reads it unless a version is given. Change it there, once, for a new version.

`./do-release` builds the eight products into one `.dmg`, `AllAlike-<version>.dmg`, then the same eight for Windows into one `AllAlike-<version>-windows.zip` (an `x64` and an `arm64` folder of VST3s; `./do-release-windows` alone builds just that), tagged `V<version>`, with the same arguments.

**Updating Sn2Alike from 1.0.9 or earlier:** its parameters have moved, so load the preset again in each project. This and what
to expect from each plug-in: [Docs/release-expectations.txt](Docs/release-expectations.txt), which both `Read Me First.txt`s
include - edit it for each release.

GPLv3. See [LICENSE](LICENSE).
