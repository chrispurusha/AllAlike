# Nl3Alike

A reproduction inspired by the Clavia Nord Lead 3, as a VST3 and Audio Unit instrument for macOS 12 or later.
Not affiliated with or endorsed by Clavia.

**Work in progress.** So far the filter is built and measured and real programs load; the plug-in plays and saves, using a placeholder oscillator,
filter and envelopes. None of it is meant to sound like a Nord Lead 3 yet. See [`Docs/design.md`](Docs/design.md).

One of the plug-ins in [AllAlike](../README.md), built on [SynthLib](https://github.com/chrispurusha/SynthLib).

## Building

```
git clone --recurse-submodules https://github.com/chrispurusha/SynthLib.git ../../SynthLib
./do-plugin            # VST3 and Audio Unit, and installs them
./do-plugin au         # one format
./do-plugin --no-install
./do-uncrustify        # after editing sources
```

GPLv3. See [LICENSE](LICENSE).
