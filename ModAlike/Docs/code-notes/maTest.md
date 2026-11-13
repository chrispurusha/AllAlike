# tools/maTest.c - notes

Links `plugin/maPlugin.c` with the engine library and SynthLib and drives it through `synthlib_plugin_variants()`, the
way both wrappers do. Checks: the impulse comes out where `latencySamples` says (within 4); each effect sounds at
0.3-2.5 of its input on noise; the Phaser and Flanger move a sine's level by more than 1 dB and their sides agree
exactly on a mono input; the Chorus's sides differ; Output -6 dB moves the level 6 dB (within 1). A parameter change
reaches the engine through the apply thread, so `settle()` keeps rendering and then waits 30 ms.
