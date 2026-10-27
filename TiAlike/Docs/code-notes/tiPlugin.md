# tiPlugin.c - the plug-in

## 1. The descriptor
SynthLib's format-free plug-in description, filled in for TiAlike: an instrument with one stereo output and MIDI in,
its class ids and Audio Unit codes (which may never change), the parameters of tiParams.c and the editor's geometry.

## 2. Notes in a block
The host hands over a block's notes before the block, each with its sample offset. They are queued and the block is
rendered in pieces, up to each note and then the note, so a note starts at its own sample whatever the host's block
size. Up to 256 a block; past that a note plays at once rather than be lost.

## 3. The editor
SynthLib's panel view around tiDraw.c, wired as Pulse2Alike's: the view syncs the parameter values into the panel
before every frame and every click, and an edit goes back to the host as a parameter change.

## 4. Filter 2's Cutoff when linked
With Link on, Cutoff 2 is an offset (tiVoices notes §6), so its own dial would sit still while Filter 2's cutoff
moved with Filter 1's. The panel shows the dial at Filter 2's cutoff instead - it turns with Cutoff - and the text
gives that cutoff and the offset, "72 (+8)". Dragging it sets the offset, so the pointer moves by the offset curve's
slope: finely near the centre, coarsely towards the ends. Above 127 the pointer stays at the end.


## 5. The sound beyond its parameters
Each instance holds the sound's name, the Single it came from (whose bytes a save keeps), the file and its place in
it, and the preset. They go into the project after the parameters as `TIS1`: name, whether there is a Single, the
Single, the path, the index and the preset. A project saved before has no record and starts as Init. Volume and the
voice limit are the instance's, not the sound's: a Single neither sets nor saves them, and Init leaves them.

## 6. Loading
Every parameter a sound carries goes through the host as an edit, so the host sees the change (automation, undo) and
the processor gets it - as Pulse2Alike loads.

## 7. Edited
A preset shows as `A012*` once a parameter it carries or its name differs from what is stored in that slot. Checked
on each editor frame; the presets file itself at most once a second.
