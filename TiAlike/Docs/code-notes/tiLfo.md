# tiLfo.c - the LFOs

## 1. Rate
Measured at every sixteenth step of the dial and joined geometrically, roughly doubling each step until the top.

## 2. Env Mode
One cycle, lifted to sit on zero (so it swings 0..2), and then nothing until the next note. It runs at twice the LFO's rate. S&H and S&G are not lifted, and they end holding their value rather than falling to nothing.

## 3. Contour
Bends the phase, so a shape's ends stay where they are.

## 4. Where a cycle starts
Sine and triangle start at their minimum, saw and square at their top.
