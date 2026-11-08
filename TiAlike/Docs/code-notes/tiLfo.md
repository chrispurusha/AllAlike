# tiLfo.c - the LFOs

## 1. Rate
Measured at every sixteenth step of the dial and joined geometrically, roughly doubling each step until the top. The
lower half was measured again over long notes (2026-11-08): the instrument is much slower there than a doubling
suggests - 0.074 Hz at 16, 0.222 at 32, 0.52 at 48, against 0.12, 0.25 and 0.51 before - and 0 is set at 0.0206 Hz,
the geometric step below 16 (too slow to measure in a minute). The midpoints between the anchors, 8 to 56, then fall
within 1.5% of their own measurements. From 64 up the law was already right (0.927 s at 64, 0.204 s at 96).

## 2. Env Mode
One cycle, lifted to sit on zero (so it swings 0..2), and then nothing until the next note. It runs at twice the LFO's rate. S&H and S&G are not lifted, and they end holding their value rather than falling to nothing.

## 3. Contour
Bends the phase, so a shape's ends stay where they are.

## 4. Where a cycle starts
Sine and triangle start at their minimum, saw and square at their top.
