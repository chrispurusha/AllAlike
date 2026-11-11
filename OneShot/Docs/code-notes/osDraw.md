# osDraw.c

## 1. The panel and its instance
The panel is shared by every instance in the process (SynthLib's panel model); `os_draw_set_user()` names the one
being drawn on each sync, and the panel reaches its sample only through `tOsSampleHost`.

## 2. Prev / Next
Step through the audio files in the current sample's folder, sorted case-insensitively, wrapping. A file that will
not load is passed over silently. The folder is listed again only when it changes. With no sample, Prev/Next opens
the file browser.

## 3. Waveform
A 512-column min/max of the left and right averaged, over the frames in view (§12) - worked out by the panel when the view
changes, no longer at load. A click on it plays the sample.

## 4. Keys
With no popup open: Left/Up previous, Right/Down next, Space plays. A host may keep some of these for itself.

## 5. The envelope on the waveform
The envelope is stepped over the sample's own length (32 steps a column, so a short attack shows) as though the note
were held throughout. In Gate mode a nominal note off is marked halfway along and the release drawn from it, so the
Release dials show; the real release comes at the real note off. The original sample is drawn dim, the sample
shaped by the envelope over it, and the envelope as a green line. Tune and Keys are not taken into account.

## 6. Start on the waveform
A yellow line where the note starts - snapped as the voice snaps it, through the same `os_voices_start_frame()` - with
the part before it dim and the envelope starting there. Gate's nominal note off is halfway from the start to the end.
The controls are 50 units wide so that PITCH, PLAY and OUTPUT (eleven controls) fit one row.

## 7. Reverse on the waveform
The overview is drawn as the sample plays, so with Reverse on it is mirrored, and the envelope and the Start marker
are laid over it in played order.

## 8. Size
860 wide since 2026-11-11, for the third row (STRETCH, FILTER, SATURATION); MAX_PLACED is 64 controls.
760 x 540 since 2026-11-11 (was 640 x 420): the first row of controls takes Reverse, there is room for a third row, and
the file browser lays itself out below the menu bar with all its rows.

## 9. Protect on the waveform
A purple line where the protected part ends, while stretching - snapped, as the copy is built (osVoice §11). The waveform and the envelope over it are not redrawn
stretched: the envelope's times stretch with the sample, so the picture is the same.

## 10. Double-click a dial
Two presses on the same dial within 0.4 s put it back to its default (the parameter table's), through the host like
any edit, so it can be undone there. The second press does not grab the dial; the first one does, as any press,
so a little movement between the two is undone by the reset. Added 2026-11-11, when Stretch's x1 proved hard to
find by dragging.

## 11. Toggles
Off / On controls (Keys, Reverse, Snap, Sync) are one button, flipped by a press: lit in the section's colour when On,
grey when Off. A menu of two entries only got in the way (owner, 2026-11-11). Two-valued controls whose values are
names (Mode, Voices, Type) keep their menu.

## 12. The view: Start to End
The waveform shows what plays - Start to End, snapped as the voice snaps them - with 6% of that span either side,
darker, so what is cut off still shows. The envelope runs from Start and stops at End; Gate's nominal note off is
halfway between. The view's frames go in the text line ("showing ..."). With End at or before Start the whole sample
shows and nothing is lit. End's dial text is a time worked out from the sample here (osParams §7).

## 13. Delay on the waveform
With a Delay, a grey "note on" line where the note is played and a tinted band from it to Start, labelled with the
Delay - the time before anything sounds. The waveform is drawn in the sample's own time, so the Delay (real time) is
turned into the sample's frames and divided by the Stretch multiple; Sync's tempo and Tune are not counted, as for the
envelope (§5). A Delay longer than the sliver before Start widens the view to the left, past the sample's first frame
if need be ("showing" then starts below 0). The band is never thinner than a line, so a microsecond Delay still shows.

## 14. Layer tabs
Four tabs between the sample bar and the waveform: the layer's number, its sample (or "empty") and its keys when not
the whole keyboard; the shown one lit. Everything else on the panel - the sample bar, Load, Prev/Next, the waveform,
every control, Reset - is the shown layer's. The panel keeps every layer's values (`gValues`) and points `gValue` at
the shown one, so the drawing code reads one layer as before; an edit leaves as the shown layer's host id
(osPlugin `edited`). The canvas grew to 566 high for the tab row.

## 15. Mute and Clear row
Under each tab, side by side: Mute, lit orange ("Muted") when on, and Clear, which empties that layer after a
confirm (osPlugin §8) and is greyed on an empty one. A press flips that layer's, whichever is shown, so the edit
names its layer (`tOsEdit.layer`; the plug-in's callers start it at -1, the shown one). The canvas grew to 584 high.

## 16. Oversampling
Not on the panel - it is the instance's, not the shown layer's: Controls > Oversample Saturation 2x, through the host
interface (`oversampling`, `setOversampling`).
