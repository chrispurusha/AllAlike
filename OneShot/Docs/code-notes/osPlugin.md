# osPlugin.c

## 1. The sample across threads
The main thread loads a sample whole, then swaps it into an atomic pointer and bumps a generation counter. The
audio thread reads the generation, then the pointer, at the start of each block; when the pointer has changed it
fades out every voice. After the block, if no voice still plays anything but the current sample, it stores the
generation it read as acknowledged. The main thread keeps each replaced sample, tagged with the generation of its
replacement, and frees it once that generation is acknowledged - so nothing is freed while a voice might read it,
and the audio thread never allocates, frees or touches a file. Retired samples are also freed on destroy.

## 2. State
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
