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
- Start by ear with Snap On and Off (no click either way); the yellow marker follows the dial and snaps
