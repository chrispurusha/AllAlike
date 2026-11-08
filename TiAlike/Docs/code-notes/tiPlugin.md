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

## 8. The Single the voices read
The host's values written over the Single they came from, redone only when something changed. A mono output hears both sides. A project from before a parameter existed takes that parameter from its Single.

## 9. Link
With Link on, the editor's frame compares the sound with what the instrument was last given and sends the difference, so only while the editor is open and never on the audio thread. Notes, bend, wheel and pedal are not sent: the instrument takes them from the host's own MIDI route, so a key sounds once (sent both ways, every note played twice - two voices beating in Poly; CT 2026-11-08). Link takes the machine's one hardware lock; if another instance has it, this one stays idle. The local engine keeps sounding: use the Volume to choose what is heard.

## 10. Program changes
A program change selects a stored sound: Bank Select picks the bank, as on the instrument. If that sound is already loaded nothing happens, so edits survive a sequencer resending the same program. Otherwise the output fades down (`synthlibSwapFade`) and the sound changes at the top of the next block once it is silent, because a host may deliver the change on the audio thread. Loads from the panel use the same fade, and the effects are emptied behind it (§12).

## 11. A new instance
A new instance plays A001, whichever comes first of its first audio block and its editor's first frame, so a panel opened before audio runs shows the sound it will play. A project's state cancels this, and with no presets stored it stays on Init.

## 12. A new sound starts with empty effects
The delay, reverb, chorus and the other effects kept running across a change of sound, so a long delay left by one Single went on echoing under the next (2026-11-02, as Sn2Alike before it). Loading a sound sets a flag, and the audio thread empties the effects (`ti_effects_reset`, tiEffects §15) before its next block - never the loading thread, which may be the main one. The swap is already faded to silence, so nothing audible is cut. Whether the instrument stops its echoes on a program change has not been checked.

## 13. The rate
The voices run at the host's rate when the instrument runs at it too - 32, 44.1, 48, 64, 88.2 or 96 kHz, the rates
its audio interface offers - and write straight into the host's buffers. At any other rate (176.4 or 192 kHz, say)
they run at the nearest of those in the same family (88.2 or 96 kHz), and SynthLib's resampler converts their output.

## 14. Output headroom
The output is 6 dB under the instrument's own level (OUTPUT_HEADROOM 0.5, CT 2026-11-08, as Pulse2Alike's and
Sn2Alike's), so the loudest sounds stay under a host's full scale. It is applied after the voices' own limit and with Master, so the instrument's clipping is unchanged; `tools/tirender` renders the voices itself
and so keeps the instrument's level for comparisons.
