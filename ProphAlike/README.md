# ProphAlike

A work in progress inspired by the Korg Prophecy, as a VST3 and Audio Unit instrument for macOS 12 or later.
Not affiliated with or endorsed by Korg.

**Placeholder.** This is the plug-in, GUI and SynthLib scaffold for a future independently modelled sound engine.
Its current oscillator, filter, envelope, patch and hardware code is inherited scaffolding and has not been validated against the Prophecy. See
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
