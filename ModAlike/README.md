# ModAlike

A Phaser, Chorus and Flanger as a VST3 and Audio Unit effect, built on SynthLib (macOS, arm64 + x86_64), with the
effects played by a sound engine library ([G2AlikeMaths](https://github.com/chrispurusha/G2AlikeMaths)).

```
./do-plugin              # both formats, installed to /Library/Audio/Plug-Ins/VST3 and .../Components
./do-plugin au           # one format
./do-plugin --no-install
./tools/do-matest        # the offline test: the plug-in driven as a host drives it
```

The engine library sits beside AllAlike (`../../G2AlikeMaths`, or `$G2AM_DIR`), as SynthLib does. The VST3 half needs the
VST3 SDK at `~/Documents/vst3sdk` (or `$VST3_SDK`); `./do-plugin au` does not. Run `./do-uncrustify` after editing.

## The panel

One page: the effect chosen at the top left, then the Phaser, Chorus and Flanger side by side - the two not in use
dimmed, keeping their settings - and below them Stereo and Output. Dials drag; Controls chooses Rotary, Vertical or
Horizontal dragging, remembered between sessions. Lists open a menu of their values. Help > The Panel and Help > Engine explain it.

## The parameters

- **Effect**: Phaser, Chorus (the default) or Flanger. Each keeps its own settings while another is chosen.
- **Phaser**: Type (I, II), Rate, Feedback.
- **Chorus**: Detune (default 50), Amount.
- **Flanger**: Rate, Range, Feedback.
- **Stereo**: Linked (one module a side, LFOs in step), Wide (the right side's LFO behind the left by **LFO
  Offset**, 0-180 degrees - Phaser and Flanger), Mid/Side (only the mid is processed; the side passes untouched).
- **Chorus Input**: Stereo (a Chorus a side, keeping the source's image) or Mono (both sides summed into one, as the module was made).
- **Mix**: 0-100%, the dry signal delayed to line up. **Output**: -24..+12 dB.
- **Engine**: **Rate** 96 kHz (the effects' own; 31 samples late below 88.2 kHz) or Host rate (no latency, close to
  but not exactly the same); **Maths** Float or Fixed (every word on a 24-bit step); **Stage**, an output
  converter's top-end roll-off, off by default and always off at the host rate; **Anti-alias** (host rate only, on by
  default), an 18 kHz low-pass into the effect.

Dials read the modules' own 0-127, with their defaults. Latency is reported to the host, and follows Rate and the host's rate.

GPLv3. See [LICENSE](LICENSE).
