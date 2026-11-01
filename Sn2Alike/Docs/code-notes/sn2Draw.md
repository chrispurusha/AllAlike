# sn2Draw.c - the editor panel

## 1. Where it comes from
Pulse2Alike's panel (`p2Draw.c`): the page tabs, sections of dials and list buttons, the menu bar, SynthLib's
popups, and since 2026-10-27 its patch bar, name field and presets. Not here: its hardware link, backup and
arpeggiator steps. The two are close copies, so a fix in one probably belongs in the other - a SynthLib candidate.

## 2. Dragging a dial
A press on a dial grabs it, and SynthLib's panel view hides the pointer and holds it still. The raw movement turns
the dial at `dial_drag_pixels_for_full_range()` per full range, SynthLib's shared policy, so Shift drags slowly. On
release the pointer reappears where it was. The value runs unrounded while it moves, so a continuous parameter
moves smoothly.

## 3. The opening click's release
A menu item is chosen on the release (§5), and SynthLib moves a menu up when it would run off the bottom of the
panel, so a list low on the panel can open its menu under the pointer. The release of the click that opened it is
ignored when the pointer has moved no more than 4 units; the menu stays open for a click of its own. Dragging from
the control into the menu and letting go still chooses.

## 4. Rotary drag
Controls offers Rotary, Vertical and Horizontal, as the sibling editors do. Vertical and Horizontal hide the pointer
and count its movement; Rotary keeps it visible and the dial takes the pointer's angle about its centre (SynthLib's
`calculate_mouse_angle()` / `angle_to_value()`). The panel view asks at each grab which kind of drag it is.

## 5. Menus and popups
The menu bar (sn2Menu.c) sits across the top; everything else is 20 units lower. Menus, the file browser and the
dialogues go through SynthLib's synthlibPopups: a press is offered to them front to back first, and a press anywhere
else while a menu is open only closes it. A menu item is chosen on the release, which is why `sn2_draw_release`
returns an edit. Keys come from SynthLib's panel view once a click has given it the keyboard.

## 6. Pages and layout
Page tabs across the top, then the page's sections flowing left to right and wrapping onto a new row when the next
would not fit. Layout is recomputed every frame and every press from the page table (sn2Params.c) alone, so a page
needs no coordinates and a new section is one table entry. Value menus open in columns of 8 rows.

## 7. Value menus
A list control opens SynthLib's context menu under itself. While it is open, a click belongs to it, choosing or
dismissing, never to the control underneath.

## 8. A dial shown somewhere other than its value
`sn2_draw_set_display()` lets the plug-in draw a dial at another position than its own parameter, with its own text,
for a control that follows another - Filter 2's Cutoff following Filter 1's when Link is on. Only the drawing
changes: a drag still moves the parameter itself, from where it is. Nothing here knows which parameters are linked.


## 9. The patch bar
`< Prev`, `Next >`, `Load...` and the name, as Pulse2Alike's. Prev and Next step through the presets (§11). Load...
opens the Load Preset browser (§11), as Presets > Load Preset... does. Opening a file is the File menu's: SynthLib's
file browser on a `.syx` or `.mid`; one Single loads at once; several open the bank browser on that file, in banks of
128, to choose one. Beside the name: the preset (`A012`, `A012*` once edited), or the file and the Single's place in it
(`My Bank 12/128`), the file's name cut to what fits before the panel's right margin.

The file's Singles are read when it is opened and kept until another file is. The instance's file and place are set
on every frame, so that setter reads nothing: it would replace the Singles the bank browser is choosing from with
the current file's.

Save Patch (over the current file) is offered only for a file holding one Single; saving over a bank would write one
Single in place of it. Save Patch As always writes a one-Single `.syx`.

## 10. The name
Ten characters, edited in place: a click starts it, Return or a click elsewhere keeps it, Escape drops it. Either
case, as a Single's name is.

Drawn as a button, grey with black text like the tabs, wide enough for ten W's (the widest capital) - the longest
name a Single holds; it lightens while being edited.

## 11. Presets
A001-L128 through SynthLib's bank browser, banks A-L of 128 (sn2Patch notes §5). Prev/Next wrap through all 512; from
no preset, Next is A001. Store always confirms, naming what the slot holds; Revert loads the stored preset again.

## 12. Import and export
Import replaces all 1536 after a confirm naming how many were found (sn2Patch notes §6). Export writes the 512 as one
`.syx`, which Import reads back.
