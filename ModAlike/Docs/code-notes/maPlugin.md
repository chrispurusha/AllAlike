# maPlugin.c - notes

1. **The patch.** FX area. Phaser or Flanger: slot A is In L -> module 2 -> Out L, slot B the same for the right,
   and slot B enabled. Chorus: slot A only, In L+R -> Mix2-1A -> StChorus -> Out L/R. Why two slots: design §2.
2. **The apply thread** polls every 5 ms for parameters marked dirty and makes the engine library's calls and the
   commit. `setParam` only stores the value and sets a bit, so it is safe from the audio thread.
3. **Latency** is the engine library's resampling: 31 samples below 88.2 kHz, none above. Asked of the library
   (`g2am_latency()`) after create and every prepare; a change is reported to the host. `tools/maTest.c` measures it.
4. **Make-up gain**: 2-In -> 2-Out loses 4.6 dB (0.5894) at Pad 0; the output is multiplied back by its inverse.
5. **The Chorus's sum**: Mix2-1A at its default level 100 on both inputs gives the average of L and R (measured 0.98).
6. **Engines**: each instance creates an engine; a process holds at most 32, and the 33rd create fails,
   which a host reports as a plug-in that would not load.
7. **Dial text** is the wire value 0-127 for now.
8. **State**: the wrapper saves the parameters; the plug-in's own block is a four-byte marker kept for later use.
9. **The editor** is SynthLib's panel view drawing `maDraw.c` (§15).
10. **Refusals** from the engine library while building are a fault in this file, never the user's: logged to
    `/tmp/modalike.log` (`touch /tmp/modalike-log`). The first build named the Chorus "Chorus" - its short name is
    StChorus - and the refusal went unseen until this was added.
11. **Mid/Side and Mix** are done around the engine, in chunks of 256 frames on the stack. Mid/Side: the engine is
    fed (L+R)/2 on both sides, and (L-R)/2 is added to the left and taken from the right after it. Mix blends the
    input back. Both dry paths go through a 64-sample delay set to the engine's latency, so they line up exactly
    (the test compares to 0).
12. **The four stereo parameters** were added after the first ten, with new ids, so earlier projects still open.
13. **Two modules or one**: everything is a module a side except a Chorus with Chorus Input "Mono"; changing that
    setting rebuilds the patch.
14. **Wide** asks `g2am_lfo_link()` for slot B's LFO at slot A's plus Offset/360 of a cycle, and Linked and Mid/Side
    for 0, after every build, prepare, and change of Stereo or Offset. The library makes the link on the audio
    thread between blocks (its notes §8), over 8 blocks. Each slot's engine takes a rebuilt patch on its own, so slot
    A's new module can start a block before slot B's; a single request could spend its 8 blocks before both had it
    and the Phaser's sides came out a block apart (intermittently - two runs in six, once the Chorus became the
    default and the test's first change was a rebuild). So the request is repeated every 50 ms for 200 ms: one lands
    after both. Re-making a link that already holds changes nothing. Not for the Chorus.
15. **The panel**: `kSections` lays it out - EFFECT, PHASER, CHORUS, FLANGER on the first row, STEREO and OUTPUT on
    the second. A section's `onlyWhen` names its Effect entry, so the two effects not in use draw dimmed. The panel
    reads every parameter from the host at each sync and edits through `synthlib_plugin_param_edited()`.
16. **Engine Rate** calls `g2am_set_economy()`; the latency changes with it, and the host is told from the apply
    thread. The engine clears the patch's state on the switch, so the LFO link is asked for again on every poll for
    100 ms (MA_LINK_AFTER_RATE), past that reset.
17. **Maths** calls `g2am_set_fixed_point()`. **Stage** calls `g2am_set_output_stage()` with the switch AND not the
    host rate - the library's default is on, so ModAlike sets it at create.
18. **Anti-alias** is ModAlike's own: an RBJ Butterworth low-pass at min(18 kHz, 0.4 x rate), set in prepare, run on
    the engine's input in `process_chunk()` only at the host rate. On Mid/Side the one mid feeds both sides and is
    filtered once.
