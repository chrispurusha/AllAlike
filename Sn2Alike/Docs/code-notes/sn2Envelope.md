# sn2Envelope.c - the envelopes

The amplifier envelope so far; the filter envelope will be the same code with its own five controls.

## 1. Ticks
The envelope moves in ticks, 46875 / 4 = 11718.75 a second, whatever the host's rate. Each control is worked out as a
rate per tick, then turned into a per-sample step for the host's rate: an added rate times ticks per sample, a
multiplier raised to the power of ticks per sample. So the times are the same at 44.1, 48 or 96 kHz.

## 2. Attack: a straight line
The level climbs from wherever it is to full in a straight line, adding a fixed rate each tick. The rate is a curve
read at every fourth step of the dial, straight-line between: at step i (dial = 4i) log2 rate = 0, log2 0.75, then
-1 per point to -5 at i = 6, -1/2 per point to -17 at i = 30, then -1 per point. Dial 0 is full in one tick; 64 takes
87.4 ms, 80 349.5 ms, 96 1.40 s; each 16 dial steps four times as long.

## 3. Decay and Release: an exponential fall
The level is multiplied by (1 - rate) each tick - an exponential fall towards zero. Decay stops at the sustain level
(a fall towards zero that stops, not an approach to the sustain level: with Sustain 0 or 64 the curve is the same until
it reaches 64's level). Release falls from wherever the level is. Both read one curve at every fourth dial step, log2
rate at breakpoints i = 0, 4, 7, 12, 24, 28, 33, 34, 35 of -4, -8, -9.5, -11, -14, -15.2, -17.2, -17.7, -19, straight
between, nothing past 35. Time constants: Release 4 2.7 ms, 32 76 ms, 64 350 ms, 96 1.40 s, 127 9.0 s.

## 4. Sustain, and Sustain Time
The sustain level is half a curve plus half a line: s = c(dial/4)/2 + dial/256 + 2^-15, where c is 0 at 0, then
0.1 x 2^(-2.8 - 0.8(5 - i)) up to i = 5, 0.1 x 2^(-0.4(12 - i)) up to 12 (-20 dB), then 1 dB per point to 0 dB at 32.
Sustain 64 sits at 0.33, 127 at 0.996.

Sustain Time (the panel's -64..+63; 64 on the wire is 0) moves the level once it has reached sustain:
- below the centre, an exponential fall towards 2^-15 at half the decay curve's rate at (9/16) x dial;
- above it, a straight rise at a quarter of the decay curve's rate at (9/16) x (128 - dial), stopping at 127/128;
- at the centre it holds.
So -32 falls with a 0.99 s time constant, -48 0.19 s; +16 rises 0.097 a second, +32 0.51.

## 5. The end of a note
A release ends, and frees its voice, once the level is below 2^-16 (-96 dB). Where the instrument itself lets a voice go
is not known; this is far enough down not to be heard.

## 6. Changing Sustain while a note holds
The level glides to the new sustain level in a straight line, 2^-12 a tick (about 2.9 full-scale a second), then
Sustain Time carries on from there. Not yet checked against the reference.

## 7. Checks (2026-10-27)
Against reference renders with the Init Single (oscillator 2 out, filter open, A4 held), each envelope against its own
peak: Attack 64 and 96, Decay 48 / Sustain 0, Decay 80 / Sustain 64, Sustain Time -32 and +32, Release 4, 16 and 64,
the shape within 0.1 dB from 20 ms on; times as §2-4 within 1% (Release 0 to 127). The level does not follow
velocity in the Init Single - its velocity to volume is 0 (sn2Voices notes §7).
