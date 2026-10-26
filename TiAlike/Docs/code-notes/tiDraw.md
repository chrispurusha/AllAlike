# tiDraw.c - the editor panel

## 1. Where it comes from
Pulse2Alike's panel (`p2Draw.c`) cut down to what TiAlike has so far: the page tabs, sections of dials and list
buttons, the menu bar and SynthLib's popups. Its presets, hardware link, backup, arpeggiator steps, name field and
patch bar are not here; the patch bar comes back with patch loading. The two are close copies, so a fix in one
probably belongs in the other - a SynthLib candidate.

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
The menu bar (tiMenu.c) sits across the top; everything else is 20 units lower. Menus, the file browser and the
dialogues go through SynthLib's synthlibPopups: a press is offered to them front to back first, and a press anywhere
else while a menu is open only closes it. A menu item is chosen on the release, which is why `ti_draw_release`
returns an edit. Keys come from SynthLib's panel view once a click has given it the keyboard.

## 6. Pages and layout
Page tabs across the top, then the page's sections flowing left to right and wrapping onto a new row when the next
would not fit. Layout is recomputed every frame and every press from the page table (tiParams.c) alone, so a page
needs no coordinates and a new section is one table entry. Value menus open in columns of 8 rows.

## 7. Value menus
A list control opens SynthLib's context menu under itself. While it is open, a click belongs to it, choosing or
dismissing, never to the control underneath.
