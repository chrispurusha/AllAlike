# To test

Built but not yet checked in a host. Confirmed: delete the line. Failed: move it to `todo.md`.

- In Live: load OneShot, Load... a sample, play it from a MIDI clip; Prev/Next and the arrow keys; Play on Load
- Drum Rack: one instance per pad, choke group on two hats (Mono within an instance)
- Reopen a project: each instance reloads its sample; a moved file shows Missing
- Changing sample while a note sounds: the old one fades out, no click
- Gate mode against note lengths; Keys On across the keyboard; offline bounce
- Space and the arrow keys reach the panel in Live (a host may take them for its own transport)
- Envelope by ear: AHR on a long sample (Hold and Release shortening it, the curve's tail), Gate cutting AHR short, ADSR with Gate, Attack on a hit
- The envelope drawn on the waveform follows every dial; Hold at its top reads Inf and leaves the sample whole
- By ear, hats and cymbals in a 48 kHz project and pitched up: no grit (the interpolator, osResample notes)
- Automating Decay, Sustain and the curves while notes sound: no clicks
- Reset in Live: every control back to default, the sample kept, and Live's Undo brings the settings back
- Pan and Volume automated fast in Live: no zipper
- Attack's fine end by Shift-drag in Live: 0-5 ms in small even steps, the text in 0.01 ms under 1 ms
- Reverse in Live: plays backwards, the waveform mirrors, Start counts from the end, Snap still clickless
- CPU in Live's meter: a 44.1 kHz sample in a 48 kHz set, Key Track Off, fast hits - lower than before; Key Track On across the keyboard
- The file browser fits the larger panel, below the menu bar
- Stretch by ear in Live: x0.25/x0.5/x2/x4 on a hit and on a sustained sample; Protect at 5 ms keeps the click of the attack; the seam is inaudible
- Sync On: a loop at its Base BPM follows Live's tempo; a tempo change rebuilds on the next note (the first plays the old tempo), many instances not all at once
- Keys On with Stretch: pitch moves, length stays; Reverse with Stretch
- Envelope with Stretch: AHR Hold/Release scale with the stretch
- Double-click a dial in Live (Stretch especially): back to its default, one Undo step, in both dial modes
- Toggles (Keys, Reverse, Snap, Sync): one click flips them, one Undo step in Live
- End by ear with Snap On and Off: no click at the cut, live and stretched; Reverse; End before Start is silent
- The waveform zooms to Start..End with the slivers either side, following the dials
- Delay: layers on two instances line up to the sample; Gate note shorter than the Delay stays silent
- Delay on the waveform: note-on line and band follow the dial; a long Delay reaches back before the sample
- A new instance comes up Mono; an older project keeps its Poly
- Protect with STRETCH's Snap On (TRIM's Snap not affecting it): the purple line jumps to a crossing; the attack-to-stretch seam by ear, Snap On vs Off
- Start by ear with Snap On and Off (no click either way); the yellow marker follows the dial and snaps
