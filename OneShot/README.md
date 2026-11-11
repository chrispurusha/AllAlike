# OneShot

A one-sample player as a VST3 and Audio Unit instrument, built on SynthLib (macOS, arm64 + x86_64).

One sample per instance, held in memory, played by any note. Meant for drum and percussion hits - one instance per
pad in a Drum Rack, with ordinary effect plug-ins after it.

```
./do-plugin              # both formats, installed to /Library/Audio/Plug-Ins/VST3 and .../Components
./do-plugin vst3         # one format
./do-plugin --no-install
```

The VST3 half needs the VST3 SDK at `~/Documents/vst3sdk` (or `$VST3_SDK`); `./do-plugin au` does not.
Run `./do-uncrustify` after editing.

## The panel
- **Load...** opens SynthLib's file browser. WAV (PCM 8/16/24/32, float) and AIFF/AIFC.
- **< Prev / Next >** (and the arrow keys) step through the other samples in the same folder, wrapping; each plays
  as it loads (Controls > Play on Load). **Play**, a click on the waveform or Space plays it again. **Reset** returns every control
  to its default, keeping the sample.
- Pitch: Tune (±24 st), Fine (±50 ct), Keys (play at the note's pitch from Root), Root (C3 = 60).
- Play: Mode (One Shot ignores the note off, Gate releases at it), Voices (Mono, the default, cuts the last hit; Poly overlaps), Reverse
  (backwards; Start and End count in the order it plays).
- Trim: Delay (0-2 s before a note sounds, microsecond steps at the bottom - for lining up layers), Start (0-2 s into
  the sample, fine at the bottom), End (0-100% of its length, shown as a time), Snap (Start and End to the nearest zero
  crossing; Off fades in and out over 1 ms instead). The waveform shows Start to End, with a sliver either side.
- Velocity (how far velocity moves the level).
- Output: Pan, Volume (-57..+6 dB in 0.5 dB steps; default -6 dB).
- Stretch: the length x0.25..x4 with the pitch kept; Sync stretches it from the sample's own tempo (BPM) to the host's
  as well; Protect (default 5 ms) keeps the start untouched while the rest takes up the change, and its Snap (default On)
  moves Protect's end to the nearest zero crossing. While stretching, Tune
  and Keys change the pitch but not the length, and the envelope's times stretch with the sample. Time-stretching is
  the Rubber Band Library's (GPL; see `THIRD_PARTY.md`).
- Double-click any dial to put it back to its default.
- Envelope (Shift-drag any dial for fine steps; Attack has 1000, 195 of them in its first 5 ms): AHR (Attack, Hold, then Release to silence - the usual one-shot drum envelope) or ADSR, with a curve on
  Attack, Decay and Release (-100..+100; Decay and Release default to +50). Hold at its top is Inf: the whole sample
  plays, which with Attack 0 is the default, so a loaded sample sounds as it is. Decay and Sustain are ADSR's and
  greyed in AHR; ADSR's Release needs Gate mode. In Gate mode a note off always starts the Release. The envelope is
  drawn over the waveform.

The project saves the sample's path, not the sample.

## Docs
- `Docs/todo.md` - what is next; `Docs/to-test.md` - built but not yet tried in a host.
- `Docs/code-notes/<file>.md` - the longer comments, numbered; code points there as `// notes §k`.
- `tools/osTest.c` - offline check of the loader and the voice; `tools/osAliasTest.c` - the interpolator's aliasing; `tools/osCacheTest.c` - the resampled copies (see their headers).
