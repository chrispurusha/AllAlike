# Nl3Alike

A reproduction inspired by the Clavia Nord Lead 3, as a VST3 and Audio Unit instrument for macOS 12 or later.
Not affiliated with or endorsed by Clavia.

**Beta.** Built: Saw, Pulse, Triangle, Sine, Noise and Dual Sine, with sync, Osc 2's
partials and fixed pitch, and all six Osc Mod types; every filter type (Classic and Distorted LP among them) and the
multi modes; the envelopes, both LFOs and their destinations, the four morph groups, Unison, vibrato, glide, Chord Memory
and the arpeggiator with its Sub and Mask modes. Programs load, are written back byte for byte, and can follow on the instrument (Link).
The sound can be played in floating point or in the instrument's own fixed point, where a stage has it.

Not working yet: the LFOs' Clock, Divisor and Mono and their Smooth and Special waves, Transpose, the arpeggiator's
Keyboard Sync, Mono's note priority and Poly Legato. See [`Docs/todo.md`](Docs/todo.md) and [`Docs/design.md`](Docs/design.md).

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
