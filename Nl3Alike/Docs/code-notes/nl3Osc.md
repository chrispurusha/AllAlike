# nl3Osc.c - the oscillator

## 1. Shape
Phase runs -1..1. A saw is that ramp multiplied by a curve (§4) of the distance from the jump, which gives the edge its band limit and its ripple. A square is two such saws subtracted, the second offset by Width, so Width 64 is square and 0 silence. Double Saw adds them, the offset counted from Width 64. The curve stops at point 1007, which is the level of the body of the wave.

## 2. Stretch
How many curve points a unit of distance covers, and so how wide the edge is: the lower of a limit set by pitch and one set by Soften. The pitch limit is 30 / increment less 25, within about 2% of its stepped measurements. The Soften limit doubles every 16 steps and runs nearly straight between them; it is low above Soften 112, which only matters below about note 30.

## 3. Not built
Sync, Hardness, Skew and Formant Width.

## 4. The edge curve
Calculated when an instance starts, in three parts fitted to measurements. The rise to point 128 is a sum of sines. To point 768 it is a level of 0.82 with a ripple of period 64 points, its size falling as 0.036 + 5/i. The last part fades that ripple out. Together within about 0.0003 rms of the measured curve.
