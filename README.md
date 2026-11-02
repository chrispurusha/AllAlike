# AllAlike

Reproductions of synthesizers as VST3 and Audio Unit plug-ins, each built on
[SynthLib](https://github.com/chrispurusha/SynthLib).

| Plug-in | What it is | State |
|---|---|---|
| [Pulse2Alike](Pulse2Alike/) | A reproduction inspired by the Waldorf Pulse 2 | Beta |
| [TiAlike](TiAlike/) | A reproduction inspired by the Access Virus TI's single-patch engine | Work in progress |
| [Sn2Alike](Sn2Alike/) | A reproduction inspired by the Novation Supernova II | Work in progress |
| [Nl3Alike](Nl3Alike/) | A reproduction inspired by the Clavia Nord Lead 3 | Just started |

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

Each plug-in has its own versions, so tags carry its name: `Pulse2Alike-V0.1.0-beta.7`, `TiAlike-V0.0.1-beta.1`.
`./do-release` in a plug-in's folder works out the next one and prints the `git tag` command.

`./do-release` here builds all of them into one `.dmg`, `AllAlike-<version>.dmg`, tagged `AllAlike-V<version>`, with the same arguments.

GPLv3. See [LICENSE](LICENSE).
