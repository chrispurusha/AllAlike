# Panel layout, every Alike

The same option sits in the same place, under the same name, in every plug-in (CT 2026-11-17). When a plug-in gains an
option another already has, it goes where this says; when this changes, every plug-in follows.

## Left to right is the signal's order

Within a row, controls follow the signal: what acts first on the left. A sample's own gain (Normalise) before the
note's (Velocity), placement (Pan) after, and the main Volume last.

## The GLOBAL page

The last tab. Its sections, in this order:

1. **PLUG-IN** - the instance's own settings, never part of a sound, in this order:

   | Control | Choices | What it is |
   |---|---|---|
   | Voices | 1-32 (or the plug-in's most) | how many voices the instance plays |
   | Same Note | STACK / PIANO | a key struck again: a new voice beside the last, or its own voice back |
   | Anti-alias | OFF / ON | ON, the default, clean oscillators; OFF the instrument's own aliasing |
   | Maths | FLOAT / FIXED | where the instrument's fixed-point arithmetic is built |
   | then the plug-in's own | | e.g. Oversample, Anomalies, Hiss, Thump (Pulse2Alike); Delay, Reverb (TiAlike) |

   A plug-in without Anti-alias leaves its place empty, so Maths and the rest stay where they are elsewhere
   (Pulse2Alike: its Oversample does that job). Maths is the last of the four, so a plug-in without it just goes on
   to its own (Pulse2Alike, Sn2Alike).

2. **HARDWARE** - only in a plug-in that can follow its instrument: Link, Channel, Device (the instrument's SysEx
   device id).

3. Then the instrument's own global settings (performance, effects, play modes and so on), in the plug-in's order.

A sound's own settings (poly/mono, envelope trigger, unison) never go in PLUG-IN.

## In the code

Each section's and page's count comes from `sizeof` of its table (Nl3Alike's `SECTION()` and `PAGE()`, `COUNT()` in
the others), never a number written by hand: a control added past a written count is silently not drawn, which is how Same Note first went missing in
Sn2Alike and TiAlike.

Pulse2Alike's empty place is a control of kind `eP2ControlGap`: it keeps its width and is never drawn or clicked.

## Where each plug-in stands (2026-11-17)

| | PLUG-IN | HARDWARE | then |
|---|---|---|---|
| Nl3Alike | Voices, Same Note, Anti-alias, Maths | Link, Channel, Device | - |
| Sn2Alike | Voices, Same Note, Anti-alias | Link, Channel, Device | PLAY (Mode, Trigger), EFFECTS, PAN |
| Pulse2Alike | Voices, Same Note, (empty), OvrSmp, Anomalies, Hiss, Thump | Link, Channel | PERFORMANCE |
| TiAlike | Voices, Same Note, Anti-alias, Maths, Delay, Reverb | Link | PLAY, UNISON, CONTROL |

Not yet checked against this: UltraAlike, OneShot, ModAlike (different panels; `Docs/todo.md`). Saved projects from
before 2026-11-17 load the moved parameters wrongly - say so in the release notes.
