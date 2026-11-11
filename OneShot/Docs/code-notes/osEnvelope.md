# osEnvelope.c

## 1. Stages
**AHR** (CT 2026-11-09: the usual one-shot drum envelope; it replaced an ADR here): Attack (0 to 1), Hold at 1, then Release
to 0, and the note ends - the Release follows the Hold whatever the Mode. **AHDSR**: Attack, Hold at 1, Decay to
Sustain, held until a note off. In Gate mode a note off starts the Release from wherever the level is, in any stage
and either mode; One Shot ignores the note off. Hold at its top position is Inf: it holds for ever (in AHR the whole
sample plays). A zero-length stage is passed straight through, so Attack 0 starts at full level; AHR's release from
the hold always starts at 1, which is the hold's level even when Attack and Hold are both 0.

## 2. Time law
`10 s x v^4` for v = dial/127: 64 is 0.64 s, 32 is 40 ms, 10 is 0.4 ms - most of the dial on the short times drums use.
Release is never shorter than 1 ms, which would click.

## 2a. Attack's law (CT 2026-11-09: finer in the first 5 ms)
Attack has 1000 steps and its own law, `1 ms x (10001^v - 1)` for v = step/1000: exponential above a 1 ms knee, so each
step is about 0.9% longer than the last. 195 steps cover 0-5 ms (9 us a step at the bottom, 0.06 ms near 5 ms); the
middle of the dial is 99 ms, three quarters 1 s, the top 10 s. A Shift-drag moves one step a pixel (SynthLib), so the
fine end is reachable by hand. Under the fourth-power law it had 19 steps for 0-5 ms, eight of them under 0.2 ms.

## 3. Curves
Each segment's progress x (0..1) is bent as `(1 - e^(-kx)) / (1 - e^(-k))`, k = 6 x curve, curve -1..1 (dial -100..+100).
Positive moves fast first: a punchy attack, a decay or release that drops quickly and then tails off, as a natural
decay does. Negative moves slowly first. 0 is a straight line. Decay and Release default to +50 (CT 2026-11-09: a drum
envelope wants a curve by default). Checked 2026-11-09 with a stepping test: linear, +-50, Hold,
AHDSR with release, release from Inf, release mid-attack.

## 4. Turning a dial during a note
The envelope is computed every output sample, so it has no block steps. It keeps how far through each stage it is
(0..1), not seconds, so a time changed mid-note carries on from the same point at the new rate - no jump (Decay
400 -> 50 ms mid-decay: at most 0.0006 a sample). A curve or Sustain changed mid-note does move the level, and the
output is slewed so no move is faster than 0 to 1 in 0.5 ms (24 samples at 48 kHz). The first value of a note is not
slewed, so an attack of 0 still starts at full level, and a note is done only once the slewed output reaches 0.
