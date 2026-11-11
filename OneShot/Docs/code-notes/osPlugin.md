# osPlugin.c

## 1. The sample across threads
The main thread loads a sample whole, then swaps it into an atomic pointer and bumps a generation counter. The
audio thread reads the generation, then the pointer, at the start of each block; when the pointer has changed it
fades out every voice. After the block, if no voice still plays anything but the current sample, it stores the
generation it read as acknowledged. The main thread keeps each replaced sample, tagged with the generation of its
replacement, and frees it once that generation is acknowledged - so nothing is freed while a voice might read it,
and the audio thread never allocates, frees or touches a file. Retired samples are also freed on destroy.

## 2. State
OSS2 since 2026-11-11: the four layers' paths. OSS1 - one path, a project from before the layers - loads into layer
1 and empties the others.

`OSS1` and the sample's path (2048 bytes). Parameters are saved by SynthLib's wrapper. A path that no longer loads is
kept and shown as Missing, so re-saving the project does not lose it.

## 3. Reset
Reset (the panel's button, or File > Reset Controls..., after a confirm) sends every parameter's default through
`synthlib_plugin_param_edited()`, so the host records it as an edit like any other and can undo it. The sample stays.

## 4. The resampled copies
The instance owns an osCache and its thread. That thread reads only the current sample: it marks it in `borrowed`,
then checks it is still current, and `free_acknowledged` never frees the marked one - so whichever comes first, the
swap or the mark, the sample is not freed while it is read. Each block the audio thread asks for the root key's pitch
when it changes (sample, direction, Tune, rate, Key Track), so with Key Track off the copy is usually ready before the
first note.

## 5. Stretch
The multiple is Stretch, times Base BPM over the host's tempo with Sync on. The host's tempo is taken from the
transport each block and kept when the host stops sending one; 120 until it first does. The envelope's times are
multiplied by the same amount (owner, 2026-11-11), so the envelope keeps its place against the sample. The look-ahead
(§4) asks for a new copy when the sample, pitch, direction, rate, Protect or the Stretch, Sync or Base settings
change - not when only the host's tempo does (osCache §8).

## 6. Layers
Four layers, each all a one-sample instance was: its own sample (with its own retire list, §1), its own settings and
eight voices of its own, so Mono, stealing and Gate act within a layer. They share the one cache and its thread - a
copy is keyed by the sample's load, so layers never mix theirs up - and `borrow` looks for the serial in every layer.
A note goes to every layer whose Low..High takes it (default the whole keyboard); a note off to all. Every layer
renders into the same block on the audio thread, between the same events, so the stack is sample-locked and
predictable (owner, 2026-11-11). Play (button, Space, menu) sends the shown layer's root key through every layer it
reaches; a click on the waveform plays the shown layer alone at its root key, whatever its range.
Checked through the VST3 (vst3host --bus-test, middle C held): layer 1 alone -38.3 dBFS, layer 2 alone -39.9, both
-36.1 (their power sum), and layer 2 set to start above the note -38.3 again.

## 7. Mute
A layer's Mute (a parameter, so it automates and saves): the layer takes no notes, Play included, and what it is
playing fades out over 3 ms (the voices' steal fade). Checked: layers 1 and 2 on middle C -36.1 dBFS; layer 2 muted
-38.3, layer 1 alone.

## 8. Clear
Empties a layer: its path goes, and its sample is retired as a replaced one is (§1), so a voice still playing it
fades out and the memory is freed once the audio thread lets go. Its settings stay - Reset is for those. Not a
parameter, so the host cannot undo it; the panel asks first.
