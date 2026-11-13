# AllAlike

Reproductions of synthesizers and their effects, and a sample player, as VST3 and Audio Unit plug-ins, each built on
[SynthLib](https://github.com/chrispurusha/SynthLib).

| Plug-in | What it is | State |
|---|---|---|
| [Pulse2Alike](Pulse2Alike/) | A reproduction inspired by the Waldorf Pulse 2 | Beta |
| [TiAlike](TiAlike/) | A reproduction inspired by the Access Virus TI's single-patch engine | Work in progress |
| [Sn2Alike](Sn2Alike/) | A reproduction inspired by the Novation Supernova II | Work in progress |
| [Nl3Alike](Nl3Alike/) | A reproduction inspired by the Clavia Nord Lead 3 | Work in progress: the filter (LP/HP 12/24 dB) is done |
| [ModAlike](ModAlike/) | A Phaser, Chorus and Flanger as an effect, with stereo modes and Mix | Work in progress: plays, with its panel; not yet tried in a host |
| [OneShot](OneShot/) | A one-sample player: one drum hit per instance, with an envelope | Work in progress: plays, browses its folder |

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

`./do-release` here builds all of them into one `.dmg`, `AllAlike-<version>.dmg`, tagged `AllAlike-V<version>`, with the same arguments.

GPLv3. See [LICENSE](LICENSE).
