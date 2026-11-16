# uaEosEnv.c

The E5000 Ultra's amplifier envelope, from measurements on the owner's E5000 Ultra (2026-11-16: a looped 1 kHz sine,
the six stages set over SysEx, the level read by a detector locked to the tone).

## 1. Stages, rates and levels
Six stages - Attack 1, Attack 2, Decay 1, Decay 2 (whose level is held while the key is down), Release 1, Release 2 -
each a rate 0-127 and a level 0-100%, as the E5000 shows them. A rate is a speed, not a time: a stage moves toward
its level at that speed and the next begins when it arrives. Rate 0 jumps.

## 2. Levels
A level is a fraction of the envelope's range, which Depth sets: 96 dB at 0, 3 dB less a step, 48 dB at 16. Below 90%
the level falls in a straight line, 0.958 - 0.00984 x percent of the range below full; from 90% to 100% it eases to
full along 0.0072 x (100 - percent). Within 0.4 dB of the measured levels at every tenth. 0% is silence.

## 3. Decay and release speed
Straight in dB, at a speed that halves every ~12 rate steps from 30 up and rises much faster below: 135 dB/s at 30,
23.5 at 60, 4.37 at 90 (depth 16), straight in log between those and the ends (3670 at 0, 0.39 at 127). The speed
scales with the range: twice the dB a second at depth 0.

## 4. Attack
Close to straight in gain, on the same speed scale: silence to full in about 34 dB's worth of the decay speed at depth
16, a fifth longer at depth 0. The curve's exact shape changes with the depth and is not modelled.

## 5. A bank's times as rates
An Emulator X bank stores each stage as a time and a level. On load each stage becomes the rate whose speed covers the
stage's distance (in gain for attacks, in dB for the rest, from the previous stage's level; the releases from the held
level) in that time, at the default depth.

## 6. Running
`ua_eos_env_step()` moves the gain toward each stage's level at its rate, attacks in gain and the rest in dB; a stage to
0% ends at the bottom of the range and the voice with it.
