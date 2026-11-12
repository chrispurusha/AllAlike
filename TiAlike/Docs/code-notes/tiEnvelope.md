# tiEnvelope.c - the envelopes

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


## 4a. The mix levels
Sub, Noise and Ring take their level from the same table as the amplifier's sustain, mixed differently: a quarter of the
dial (over 127) and three quarters of the table read there, `ti_env_mix_level()`. Full at 127, 0.247 at 64. It replaced
a fitted curve (the sustain level to the power 1.25) that was within 0.2 dB of it everywhere (2026-11-12).
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
velocity in the Init Single - its velocity to volume is 0 (tiVoices notes §7).

## 7. Fixed: the instrument's own arithmetic
With Arithmetic at Fixed (and in tools/tirender) all four envelopes - amplifier, filter, 3 and 4 - are computed as the
instrument computes them, word for word at 44.1 kHz. The update, the level and the Decay, Release and Sustain Time rates
are the same for all four; the amplifier differs from the other three in its Attack, its sustain level, where it starts
and where Attack stops (see the end of this section). The amplifier's:
- The level is a 48-bit word, updated once every 16 samples. Attack adds its rate to the level's high word and stops
  at $7F0000; Decay, Release and a falling Sustain Time take the level's high word times their rate off the whole
  48-bit level, so the bits below the word carry from update to update; Decay stops exactly on the sustain level, a
  falling Sustain Time on $100, a rising one on $7F0000. A note from silence starts at $1000.
- The rates: the curve's points (33 for Attack, 37 for the falls), read at dial / 4 with a straight line between points
  (the fraction as a word, the product's lower bits dropped), times a scale of 0.5 x 46875 / the sample rate, doubled,
  the product's lower bits dropped towards minus; four times that per update. Sustain Time reads the fall curve at
  9/8 of twice its distance from 64, halved for a fall, quartered for a rise.
- The sustain level: the dial over 127 as a word (dial x 66052); half the sustain curve there, half that word, plus $100,
  limited at full scale - so Sustain 127 is full scale, above the attack's $7F0000, and holds there.
Checked against reference renders: every rate for every dial value 0-127 the same word; a whole note (Attack, Decay,
Sustain, Sustain Time 40 and Release 64) the same level word at all 1944 updates; all 128 sustain levels. Sustain Time
now starts falling as the note does (it waited 150 ms behind a sustain level 2% low), and Sustain Time 0 sits on the
floor instead of 12 dB over it. At other sample rates the scale follows the rate; the update stays 16 samples.
The filter's envelope and envelopes 3 and 4:
- Attack reads the fall curve, not the attack curve: its rate there (as above, before the four) halved, plus that half
  times the curve's point as a fraction; Attack 0 jumps straight to the top. About eight times slower than the
  amplifier's at 64, the same at 120.
- The sustain level is the dial over 127 as a word, plus $100: a straight line, no curve.
- A note from silence starts at 0, and Attack rises to full scale ($7FFFFF).
Checked: Attack and sustain words for all 128 dial values, all three envelopes; three whole notes (filter, envelope 3,
envelope 4, different settings each) the same level word at every update. Filter-envelope sweep against the reference:
Attack 0 band error 6.3 -> 0.5 dB; what is left is common to every setting, the filter's answer to the envelope.
Between updates the level moves in a straight line, sample by sample, from one update's level to the next's, as the
reference's does (an attack measured sample by sample rises at a constant 0.002 a sample); a note from silence takes
its first update at once, so Attack 0 starts at full level on the first sample. The amplifier uses it directly in Fixed
(voices §32's block-wide line remains for Float). Amp envelope sweep: worst level 2.16 -> 0.43 dB.
Float keeps the closed forms of §2-§4.
