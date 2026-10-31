# tiOsc.c - the oscillators

## 1. The classic oscillator
Shape 0..64 crossfades the wave to a saw, 64..127 the saw to a pulse, each a straight line. The saw falls, in phase with the sine and the pulse, so the crossfades add. Pulse width: duty 1/2 + width / 254, silent at the far end.

## 2. PolyBLEP
Band-limits the saw's and the pulse's steps.

## 3. The waves
Sine, triangle and 62 spectral waves, the last held as 64 measured partials each (`tiWaves.c`, generated), their phases measured against the saw so that a wave blends with it correctly. Tables are built once, each with half the harmonics of the last, and the highest whose top partial stays under Nyquist is read.

## 4. The HyperSaw
Up to nine saws. Density reaches 2, 3, 4, 5, 6, 7, 8 and 9 saws at 32, 64, 80, 96, 104, 112, 120 and 127, and in between the next saw fades in, a little faster than a straight line in the dial, while the stack slides to stay centred. Power stays constant, each saw at 1 / sqrt(n). Spread sets the spacing, 3.47 cents at 64 and 1.79 times as much every 16 steps; fewer saws sit further apart (`kHyperWiden`). Measured at note 60; a lone saw is 1.066 of a classic one.

## 5. The Wavetable oscillator
A hundred tables, each measured as 64 partials at 16 positions, read with Wave Select choosing the table and Index moving through it, crossfading between neighbours. The positions sit every 127/15 of Index, between whole dial values, so each is taken from a straight line through the dials either side, and the phases are measured against the saw so the partials of neighbouring positions add correctly. A table's waves are built when first used, in five band-limited versions, because all of them together would take far too much memory.
