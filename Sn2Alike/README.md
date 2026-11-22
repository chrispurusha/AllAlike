# Sn2Alike

A reproduction inspired by the Novation Supernova II, as a VST3 and Audio Unit instrument for macOS 12 or later.
Not affiliated with or endorsed by Novation.

**Beta.** Programs load and play through the three oscillators (sync, skew, formant width, soften, FM, ring and noise),
the filter in all its types, the envelopes, both LFOs, the modulation routes, unison, glide, EQ, distortion, chorus, delay
and reverb, in floating point or in the instrument's own fixed point (Maths). A Supernova can follow along (Link).

Not built yet: the arpeggiator, the Phaser (it plays as a chorus), the comb, Hardness, the audio inputs and
Performances. Known problems are at the top of [`Docs/todo.md`](Docs/todo.md).

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
