# tiPlugin.c - the plug-in

## 1. The descriptor
SynthLib's format-free plug-in description, filled in for TiAlike: an instrument with one stereo output and MIDI in,
its class ids and Audio Unit codes (which may never change), the parameters of tiParams.c and the editor's geometry.

## 2. Notes in a block
For now a note starts at the beginning of the block it arrives in, not at its sample offset. To change before any
timing is compared with the reference renders (todo.md).

## 3. The editor
SynthLib's panel view around tiDraw.c, wired as Pulse2Alike's: the view syncs the parameter values into the panel
before every frame and every click, and an edit goes back to the host as a parameter change.
