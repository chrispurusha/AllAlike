# uaPlugin.c

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
The instance owns an uaCache and its thread. That thread reads only the current sample: it marks it in `borrowed`,
then checks it is still current, and `free_acknowledged` never frees the marked one - so whichever comes first, the
swap or the mark, the sample is not freed while it is read. Each block the audio thread asks for the root key's pitch
when it changes (sample, direction, Tune, rate, Key Track), so with Key Track off the copy is usually ready before the
first note.

## 5. Stretch
The multiple is Stretch, times Base BPM over the host's tempo with Sync on. The host's tempo is taken from the
transport each block and kept when the host stops sending one; 120 until it first does. The envelope's times are
multiplied by the same amount (owner, 2026-11-11), so the envelope keeps its place against the sample. The look-ahead
(§4) asks for a new copy when the sample, pitch, direction, rate, Protect or the Stretch, Sync or Base settings
change - not when only the host's tempo does (uaCache §8).

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

## 9. Effects
Each layer's voices are rendered into a buffer of their own, put through the layer's filter and saturation (uaFx)
and added to the output; a layer with both off goes straight in, as before. Settings are read once a block; the
effects glide to them. Checked through the VST3 (middle C held): plain -38.3 dBFS; High Cut 200 Hz -49.5 (12 dB) and
-57.1 (24 dB); saturation at +24 dB gain -16.2 / -15.0 / -16.3 (Classic / Drums / Tape).

## 10. Oversampling
One switch for the instance, not a layer - every layer must be equally late to stay in step. Saved in the state
(OSS3: OSS2 and four bytes of flags; OSS2 and OSS1 open with it off). It is in force - every layer 23 frames late, 23
reported - only while some layer with a sample has saturation on (`oversampling_in_force`, used by both the audio
thread and `latencySamples`). The audio thread compares it each block; when it changes - the button, a layer's
saturation switched or automated, a sample loaded or cleared - it starts every layer's effects afresh and tells the
host to read the latency again (`synthlib_plugin_latency_changed`, which posts to the main thread). Checked: switch on
with nothing saturating, a note sounds from frame 389 as with it off; with a layer saturating, from 411.

## 11. Automation
Every parameter is read once a block, so automation takes effect from the next block. What a copy depends on - the
sample, Reverse, Tune, Fine, Keys and Root (the step), the host's rate, Stretch, Sync, Base and Protect - is compared
each block with what was last asked for, and a change asks for a new copy (§4): a sweep asks every block, the cache
builds only the newest (uaCache §6), and notes meanwhile play live, or stretched notes the nearest copy (uaCache §8).
These are taken when a note starts: a sounding note keeps its pitch, stretch, Start, End and Delay. The envelope,
Volume, Pan, the filter and saturation are read each block and shape sounding notes too, gliding where a step would
click. A bounce builds what it needs before each note (uaVoice §12), so it does not depend on how fast the cache is.

## 12. Bank presets
A layer plays either a sample or a preset of an E-mu Emulator X bank (`.exb`, read by SynthLib's `emuBank.c`),
never both. Loading a `.exb` opens the bank and its first preset; Prev/Next then steps the presets (uaDraw §18).
The bank is shared by every program made from it (`tUaBankRef`, counted on the main thread), and a program - a
preset's zones - is retired like a replaced sample, freed once the audio thread no longer plays it; the bank and
its samples go with the last program. The project keeps the bank's path in the layer's slot and its preset in
`UAS4`'s extra words.
A preset's notes play through the bank voices (uaBankVoice), with the layer's Tune, Velocity, Volume, Pan, envelope,
keys and effects; the layer's sample controls (Start, End, Stretch, Reverse...) do not apply. Play holds the root
key for a second and lets go, since a looped preset would otherwise sound for ever.
