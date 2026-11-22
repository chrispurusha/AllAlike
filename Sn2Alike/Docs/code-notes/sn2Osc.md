# sn2Osc.c - the oscillator

## 1. Shape
Phase runs -1..1. A saw is that ramp multiplied by a curve (§4) of the distance from the jump, which gives the edge its band limit and its ripple. A square is two such saws subtracted, the second offset by Width, so Width 64 is square and 0 silence. Double Saw adds them, the offset counted from Width 64. The curve stops at point 1007, which is the level of the body of the wave.

## 2. Stretch
How many curve points a unit of distance covers, and so how wide the edge is: the lower of a limit set by pitch and one set by Soften. The pitch limit is 30 / increment less 25, within about 2% of its stepped measurements. The Soften limit doubles every 16 steps and runs nearly straight between them; it is low above Soften 112, which only matters below about note 30.

## 3. Not built
The audio inputs.

## 4. The edge curve
Calculated when an instance starts, in three parts fitted to measurements. The rise to point 128 is a sum of sines. To point 768 it is a level of 0.82 with a ripple of period 64 points, its size falling as 0.036 + 5/i. The last part fades that ripple out. Together within about 0.0003 rms of the measured curve.

## 5. Sync
Hard sync with no second oscillator: each sample the slave's phase is worked out from the oscillator's own (both
-1..1, a cycle 2.0) as the instrument's arithmetic does it. With r the ratio, the slave has run r(p + 1)/2 cycles since
the oscillator's cycle began; its phase is twice the part past the whole cycles, less 1 - it restarts with every one
of the oscillator's cycles - except in the last, partial cycle (the part f of r past a whole number), which is moved up
by 1 - f so that it is centred on zero rather than starting at -1.

- **Ratio**: r = 2^(Sync / 24), Sync/2 semitones (24 an octave, 48 two), held at 32 from Sync 120 up. At a whole ratio
  the slave's restarts fall in step and the oscillator simply plays r times higher - 24, 48 and 96 give one, two and
  four octaves up with no fundamental.
- **Edge**: the slave's ramp is shaped as any saw's (§1), its stretch (§2) taken from the slave's increment, r times
  the oscillator's, with the ratio not held at 32 - above Sync 120 the pitch stops rising but the edge keeps softening.
- **Square, pulse, double saw**: each of the two ramps is a slave of its own, the second's oscillator phase moved on by
  the offset (the width, or the width - 1 for the double saw) over r - not the first slave's phase moved on by the width.
- **Moving it**: Env 2, Env 3, the wheel and the LFOs add to Sync from the program: (depth - 64) x 2 Sync steps at full
  swing, the envelopes scaled by their velocity, an LFO counting half as on the levels. A Sync of 0 with any route set
  still runs through the slave (r = 1 is the oscillator itself), so a route can sweep a sound into sync. Env 2 and
  Env 3 checked against Maths FIXED; the wheel not (FIXED has no wheel yet).

Checked 2026-11-22 against the instrument's arithmetic (Maths FIXED), harmonic by harmonic with the filter open: saw,
square, a narrow pulse and the double saw within 0.1-0.9 dB median from Sync 12 to 96, levels within 0.1 dB; to about 1
dB (0.5-1.7 dB in level) above the cap; above 8 kHz ours is up to 1.8 dB brighter (the instrument softens the restart
slightly more). Found through Supernova program A007 (FM EP), whose Sync 63 with LFO 2 on it puts its energy near the
32nd harmonic, under its low cutoff: it now decays as the instrument's does.

## 6. Sync Skew
Skew bends the slave's pitch through each of the oscillator's cycles: Sync - 24 (Skew - 64) / 64 x phase, phase the
oscillator's own (-1..1) - an octave either way at the extremes - not below 0. It is worked out every fourth sample,
with the voice's other slow words, and the edge's stretch follows it. Being in phase, its effect depends on nothing but
Skew and Sync. With Sync 0 the slave runs above the oscillator for half of each cycle, so Skew is heard with Sync off.
Found 2026-11-22 from the words FIXED works out: the law gives its ratio to within its table's step (1/512 octave) at
three keys. Program A001 (Osc 2 at 124) came from up to 21 dB out to within 2 dB in every octave band.

## 7. Sync Formant Width
A window on the oscillator's output: 1 as each cycle starts, then losing (127 - Width) / 4096 of itself a sample, so
the cycle dies away. 127, the usual value, leaves it open. It works with or without Sync. The instrument never
closes or opens it at a note, so a voice starts with it open.

## 8. Key Sync
An amount, not a switch: each key above middle C takes 2 x amount / 128 Sync steps off, each below adds them, so at 127
the slave's pitch stays put as the keyboard moves. It follows the glide. Checked 2026-11-22 against the instrument, steady
tones at two keys and three amounts: FIXED to 0.1 dB above 250 Hz, FLOAT to about 1 dB.
