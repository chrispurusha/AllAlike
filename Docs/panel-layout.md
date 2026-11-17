# Panel layout, every Alike

The same option sits in the same place, under the same name, in every plug-in (CT 2026-11-17). When a plug-in gains an
option another already has, it goes where this says; when this changes, every plug-in follows.

## Left to right is the signal's order

Within a row, controls follow the signal: what acts first on the left. A sample's own gain (Normalise) before the
note's (Velocity), placement (Pan) after, and the main Volume last.

## Colours

Defaults in SynthLib's `synthlibDefs.h` (its notes §7); a plug-in may override any by defining it first.

- **Grey means unavailable, and only that.** A control that does nothing at the moment is drawn in
  `RGB_PANEL_UNAVAILABLE` with a grey label (OneShot, UltraAlike). Nothing usable is grey, so no section is grey.
  The patch bar's buttons and the tabs are furniture, a light grey with black text, and stay as they are.
- **Off is dimmed, not grey.** An Off button keeps `PANEL_OFF_DIM` (35%) of its section's colour, with light text
  (`RGB_PANEL_OFF_TEXT`); On is the full colour with black text. `synthlib_button_off_colour()` in `buttonPress.h`.
  The same for an indicator such as CLIP, which is `PANEL_CLIP_W` x `PANEL_CLIP_H` (25 x 10) beside the title.
- **Shared sections share a colour:** PLUG-IN and HARDWARE `RGB_SECTION_PLUGIN` (steel blue); OUTPUT
  `RGB_SECTION_OUTPUT` (sage); PLAY, VOICE, GLIDE, VIBRATO, UNISON, ARP `RGB_SECTION_PLAY` (rose). Use the
  `SECTION_RGB()` form with these names rather than copying the numbers.

Done in every Alike (2026-11-17). Off/On choices are flip buttons in Nl3Alike, Sn2Alike, Pulse2Alike, TiAlike and
ModAlike (OneShot and UltraAlike had them already). Shared colours also: `RGB_SECTION_CONTROL` (wheat) for the
performance controllers - BEND / WHEEL, VELOCITY, CONTROL, PERFORMANCE. In a table that mixes a shared colour with
numbers, name the numbers (`#define COLOUR_X {r, g, b}`): uncrustify cannot align a table of mixed shapes.

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
   device id). Link is here and nowhere else - no Link HW button in the patch bar (CT 2026-11-17). When Link is on
   but cannot work, its button turns amber and says why: NO PORT (no MIDI output chosen), BUSY (another instance
   holds the instrument).

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
| ModAlike | Anti-alias, Maths, Rate, Stage (an effect: no Voices or Same Note) | - | - |

UltraAlike waits for its GUI overhaul (its `Docs/todo.md`: plug-in settings are a section called GLOBAL). OneShot checked 2026-11-17: one page, no plug-in settings or hardware, so no GLOBAL page. Saved projects from
before 2026-11-17 load the moved parameters wrongly - say so in the release notes.
