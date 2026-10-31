# sn2Osc.c - the oscillator

## 1. Band-limiting
The saw and the pulse take a PolyBLEP correction at each step: a two-sample polynomial residual that removes most of
the aliasing a naive step makes, cheaply. A placeholder, as the oscillator's own waves are still to be measured.

## 2. The triangle
Naive: its discontinuities are in the slope, not the level, so its aliasing falls off far faster than the saw's.
