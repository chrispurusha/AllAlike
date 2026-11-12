# tiLfo.c - the LFOs

## 1. Rate
The envelope's fall curve (tiEnvelope §7) read at (127 - dial) x 15/16 + 8, that scaled by 128/127 and taken at 9/32,
times 0.0008918 Hz a unit - the instrument's own law. Against reference renders (the saw's jumps timed over several
cycles, 2026-11-11) it is within 1% from 48 to 104 and 2% at 40 and 112, where the measurement has few cycles; the
table it replaces was out by up to 4.4% (at 104).

## 2. Env Mode
One cycle, lifted to sit on zero (so it swings 0..2), from the Keytrigger phase (the top with Keytrigger off), and
then it rests where the cycle ended - the value just before that phase, the end of the cycle when it is 0 - until the
next note. From the top that is the bottom, 0. Against reference renders through Filter Gain, saw and square at
Keytrigger 1 to 113: a square started in its first half rests high, in its second half low; a saw rests lower the later
it starts (2026-11-12; K030 FunkMamaBC, Keytrigger 57, was silent here once its cycle was spent). It runs at twice the LFO's rate - unless the LFO is clocked, when its cycle is the clock's note value, as without Env Mode (against reference renders, Clock 6 and 10, 2026-11-12: U076 OfcHell ended its note in half the time here). S&H and S&G are not lifted, and they end holding their value rather than falling to nothing.

## 3. Contour
Bends the phase, so a shape's ends stay where they are.

The sine is the exception (2026-11-09, osc 2's pitch tracked on the instrument, LFO 1 at rate 48): its phase is not
bent. Above the middle, Contour drives it into a limit - gain (1 - contour)^-0.8, so it holds at the top and bottom for
about half the cycle at Symmetry 96, most of it at 112, and is a square with sine-shaped edges at 127. Below the
middle it stays a sine (at 0 the instrument's is slightly skewed; not modelled). The triangle's bend was checked
against the instrument at 64, 96 and 127 and matches.

## 4. Where a cycle starts
Sine and triangle start at their minimum, saw and square at their top. A free-running LFO (Keytrigger off) starts half
a cycle in when the Single is loaded (voices §20): a note straight after loading finds the saw halfway down and the
sine at its top, at every rate, on the reference.
