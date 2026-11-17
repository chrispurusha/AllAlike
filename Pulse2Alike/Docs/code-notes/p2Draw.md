# p2Draw.c / p2Draw.h - notes

The editor panel, drawn through SynthLib's renderer with SynthEdit's look: dark grey, section headings with a
rule, `render_dial()` for continuous controls, and `draw_button()` for lists, which step on each click. It is
hosted by SynthLib's `synthlibPanelView`, as MidiSyncTool's and GenBridge's panels are.

## 1. Dragging a dial
As SynthEdit and G2Alike do it. A press on a dial grabs it, and SynthLib's panel view hides the pointer and
holds it still. The raw movement turns the dial, at `dial_drag_pixels_for_full_range()` per full range, which
is SynthLib's shared policy, so Shift drags slowly. On release the pointer reappears where it was. The value
runs unrounded while it moves, so a continuous parameter (cutoff, resonance) moves smoothly.

## 2. Value menus
A list control (the oscillator shapes, the filter type) opens SynthLib's context menu under itself, as
SynthEdit's value menus and MidiSyncTool's selectors do. While the menu is open, a click belongs to it, either
choosing or dismissing, never to the control underneath.

## 3. Pages and layout
Page tabs across the top (SynthEdit's `draw_button`, green for the current page), then the page's sections
flowing left to right and wrapping onto a new row when the next would not fit. Layout is recomputed every
frame and every press, from the page table alone, so a page needs no coordinates of its own. Value menus
open in columns of 8 rows (the 32 modulation targets as 4 columns), so a long list fits under a control in
the lower row instead of scrolling.

## 4. Presets: Prev, Next, the label
`< Prev` and `Next >` step through P001-P500 and wrap at both ends, as the instrument's own buttons do; from a sound
opened as a numbered file (`004 Name.syx`) they step from its number; from any other (Init, an unnumbered file)
Next goes to P001 and Prev to P500. `Load...` opens the Load Preset browser, as Presets > Load Preset... does.
A single `.syx` is opened from the File menu, in SynthLib's in-window file browser, starting in the current file's
folder or `iCloud Drive/Patches/Pulse2`. Both browsers are modal and drawn last; while one is open every press,
release, hover and scroll is its own.

After the name: the preset (`P012`, grey; `P012*`, white, once edited - p2Plugin notes §48), or the file's name
when the sound came from a file, cut to what fits before the panel's right margin. Load Preset... and Store Preset to... choose the slot in SynthLib's bank browser,
as SynthEdit and G2Alike choose a bank location: 500 named rows numbered 1-500, sortable A-Z. They go in unbanked (bank 0, SynthLib bankBrowser.cpp notes §9);
the instrument's banks of 100 matter only to program changes (p2Patch notes §3). A store is
always confirmed, naming what the slot holds now; so is Revert.

The browsers also sort by category: each sound's CLASS, its last sound byte, under the 20 names the instrument
shows for it (ARP, ATMO, BASS, DRUM, FX, KEYS, LEAD, MONO, PAD, PERC, POLY, SEQ, DUB, LUSH, STRG, PLUK, WIND, NOIS,
EXFX, INIT). The factory presets use 0-13. A value past the list is shown as INIT.

## 5. Menus and popups
The menu bar (p2Menu.c) sits across the top of the panel; everything else is 20 units lower. Menus, the file
browser and the dialogues all go through SynthLib's synthlibPopups, as in G2Alike's editor: a press is offered
to them front to back first, and a press anywhere else while a menu is open only closes it. A menu item is chosen
on the RELEASE (contextMenu acts on mouse-up), which is why p2_draw_release returns an edit: a list control's
value menu is chosen there too. Keys come from SynthLib's panel view once a click has given it the keyboard.

## 6. The name field
The sound's name, beside Prev/Next/Load, edited in place as SynthEdit edits a program name: a click starts it,
Return or a click elsewhere keeps it, Escape drops it. Typing is upper-cased and stops at 14 characters, which
is the dump's name field and how the instrument's own sounds are named. The name goes to a file only at the
next save. The file the sound came from is shown greyed after it.

It is drawn as a button, grey with black text like the tabs, wide enough for 14 W's (the widest capital), the
longest name the dump can hold; it lightens while being edited.

## 7. The arpeggiator's steps
Below the ARP page's dials, the 16 steps, each three cells: its Event and Time (lists) and Glide (a toggle). A
step past Pattern Length is greyed. Each step is one parameter, the instrument's own byte (p2Arp notes §2), so
an edit changes only the bits it shows.

## 8. Link
Link is on the GLOBAL page only, under HARDWARE, as in the other Alikes (owner 2026-11-17: the title row's Link HW
button is gone, and Prev starts where it was). It flips the Link setting (p2Plugin notes §41). The work is the
plug-in's, through the host's `send`; the panel only reports a failure: amber, reading BUSY, while Link is on but
another instance owns the instrument (p2Plugin notes §49).

## 9. Rotary drag
Controls offers Rotary, Vertical and Horizontal, as the sibling editors do. Vertical and Horizontal hide the
pointer and count its movement; Rotary keeps it visible and free, as the siblings' Rotary does, and the dial takes
the pointer's angle about its centre (SynthLib's `calculate_mouse_angle()` / `angle_to_value()`, the 270 degree
sweep the siblings use). The panel view asks at each grab which kind of drag it is (its `keepsPointer`, SynthLib
synthlibPanelView notes §12).

## 10. Import and export
Presets > Import from File/Folder reads the sounds (p2Patch notes §7) and asks before replacing all 500, saying how
many it found and that the rest become INIT; nothing is written unless confirmed. Export writes the 500 as they
are, and reports only a failure.

## 11. Backup progress
Pulse 2 > Backup Pulse 2 to Presets... asks first (all 500 replaced; nothing on the instrument changes), then shows
SynthEdit's "Backing Up Bank" box over the panel: the sound being asked for, how many have answered, a bar. The
menu offers Cancel Backup while it runs. The end says how many were read and how many are INIT.

## 12. The opening click's release
A menu item is chosen on the release (§5), and SynthLib moves a menu up when it would run off the bottom of the
panel. A list on the bottom row (OSC 3's Shape and Routing) then opened its menu under the pointer, and the release
of the very click that opened it chose the item there: the menu flashed up and went (CT, 2026-10-23). So the
release that ends the opening press is ignored when the pointer has not moved more than 4 units; the menu stays open
for a click of its own. Dragging from the control into the menu and letting go still chooses, as before.

## 13. Store Sound to Pulse 2 Slot (2026-11-06)
The bank browser lists P001-P499 only: P500, the instrument's random sound generator, is not offered (and the
hardware layer refuses it too, p2Hardware notes §2). Choosing a slot always asks first, naming the sound and what
the slot held at the last backup (the presets file's name for it: the plug-in does not read the slot again before
writing). The store sends the sound as it is now, like Send Sound, under the same one-off lock; it does not touch
the presets file or the plug-in's preset number. A store that went out says so; a failure shows the usual Pulse 2
alert. One store is one flash write: nothing repeats it.

## 14. Double-click a dial
Two presses on the same dial within 0.4 s put it back to its default (the parameter table's, given to the panel at
start by `p2_draw_set_default()`), through the host like any edit, so it can be undone there. The second press does
not grab the dial. As OneShot's (osDraw notes §10), added across the Alikes 2026-11-14.

## 15. Prev/Next in the Sort menu's order
Prev/Next step through P001-P500 in the order the Sort menu sets (Bank / Location, Category by each sound's CLASS,
or A-Z; SynthLib's `bank_browser_sorted_order()`, as TiAlike, Sn2Alike and Nl3Alike), wrapping. From a numbered file
they start from its number; from any other sound Next is the first in that order and Prev the last.

## 16. Shift for fine adjustment
With Shift held, dragging a dial moves it a tenth as far for the same distance - the same on the Mac and Windows
(SynthLib's panel views read the modifiers on every drag event). Rotary mode, where the dial follows the pointer's
angle, is unchanged. Added across the Alikes 2026-11-14.

## 17. Keyboard tracking at 1:1
The filter's keyboard tracking dial (kP2ParamFilterKeytrack) turns gold at the one step where the cutoff follows the keys one
semitone a note, like an oscillator; a double-click goes there rather than to the Init sound's own setting. The
gold is shared by every Alike (`UNITY_COLOUR`). Added 2026-11-14 (CT 2026-11-08: the same in every Alike).

## 18. Colours and Off/On buttons (2026-11-17)
The panel follows `../../Docs/panel-layout.md` (Colours), on SynthLib's defaults (synthlibDefs.h §7). Grey means only
a control that does nothing, so no section is grey: PLUG-IN and HARDWARE take `RGB_SECTION_PLUGIN`, OUTPUT and MASTER
`RGB_SECTION_OUTPUT`, the play sections `RGB_SECTION_PLAY`, the performance controllers `RGB_SECTION_CONTROL`. A choice
of OFF and ON is a button that flips on the release that lands on it (as Nl3Alike's), lit in its section's colour when
on and that colour dimmed (`synthlib_button_off_colour()`) with light text when off. CLIP is the same when not lit.
