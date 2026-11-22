# ProphAlike

A work in progress inspired by the Korg Prophecy, as a VST3 and Audio Unit instrument for macOS 12 or later.
Not affiliated with or endorsed by Korg.

**Early work.** Filter 1 is the instrument's, in fixed and floating point. The oscillator and the amp envelope are stand-ins;
Filter 2, the mixer, LFOs, effects and programs are not built. See
[`Docs/design.md`](Docs/design.md).

One of the plug-ins in [AllAlike](../README.md), built on [SynthLib](https://github.com/chrispurusha/SynthLib).

## Building

```
./do-plugin            # VST3 and Audio Unit, and installs them
./do-plugin au         # one format
./do-plugin --no-install
./do-uncrustify        # after editing sources
```

GPLv3. See [LICENSE](LICENSE).
