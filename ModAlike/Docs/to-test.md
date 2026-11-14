# To test

Built, not yet tried in a host or against the instrument. Confirmed: delete the line. Failed: move it to todo.md.

- Loads in Ableton as an effect (VST3 and AU); its panel opens, resizes, and its dials, lists and menus work; automation moves the panel's dials
- The panel follows a change made in the host (automation, the generic parameter list) and a reopened project
- A saved project reopens with its effect and settings, Stereo, Offset, Chorus Input and Mix included
- Wide at 90 and 180 degrees on headphones; Mid/Side keeps a wide mix's image; Mix 50% has no comb filtering (the dry path lines up)
- Changing LFO Offset jumps the sweep (expected): check it is not a loud click
- Engine Rate switched while playing: a moment's silence, then no latency; Ableton re-aligns the track
- Maths Fixed against Float on a long Flanger feedback tail; Stage On on bright material
- The latency (31 samples) is compensated: a dry and a processed track stay aligned
- Each effect against the instrument itself, same settings, through its inputs (the engine's modules are Confirmed; the plug-in's
  level and stereo handling are not)
- Flanger Range (2026-11-14, G2Alike reference §70.2): turning Range no longer zippers - Range and FB glide over 125 ms as every dial does
