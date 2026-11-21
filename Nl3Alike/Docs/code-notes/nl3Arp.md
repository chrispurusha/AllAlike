# nl3Arp.c - the arpeggiator

Everything here was measured on the instrument (OS 1.20) on 2026-11-21: a plain sine with a short decay, a held
chord, and each step's time and pitch read from the capture. `tools/nl3arptest.c` replays those sequences (§7).

## 1. The settings, and where a program keeps them
The panel's Run, Rate, Clock and Divisor, and the Sound menu's direction (Up, Down, Up/Down, Random), Range (1-4
octaves), Mode (Normal, Sub, Mask, Sub+Mask), Sub direction (Up, Down, Up/Down, Random, Still), Sub range (1-8
notes), Mask length (1-16) and the 16 Mask steps. The manual's program table lists the panel ones; the Sound menu's
sit in bits it calls spare, found by setting them and listening, and by what the instrument stores when sent a value
out of range (it clamps):

| Setting | Bits | |
|---|---|---|
| Mask length - 1 | 354-357 | 15 (16 steps) in nearly every program |
| Sub direction | 358-361 | 0-4; 5 is stored as 4 |
| Sub range - 1 | 362-366 | 0-7; larger is stored as 7 |
| Mode | 367-369 | 0-3; 4 is stored as 3 |
| Mask step k | 466 - (k - 1) for steps 1-8, 474 - (k - 9) for 9-16 | 1 plays, 0 rests |

The step bits run backwards and the two halves of the mask are swapped: step 1 is the low bit of the second byte.
The bit the manual calls Keyboard Sync (456) is cleared by the instrument whenever a program is sent with it set, and
the manual's arpeggiator chapter has no such setting, so it does nothing here and is left off the panel.

## 2. The step
Free running, a step is a whole number of ticks of 1/4096 s: with x = 127 - Rate, an octave table of 24 lengths
(32768 rising to 63670, each 2^(1/24) above the last) indexed by x mod 24, times 100, shifted by (x / 24) - 15 bits
and held below 65536. Rate 0 is 3916 ticks (0.9561 s), 64 is 616 (150.4 ms), 127 is 100 (24.4 ms) - halving every 24
steps with the table's rounding; every measured rate (0 to 127) agrees within a millisecond. The manual's 30-240 BPM
is not this range.

With Clock on, a step is a note value at the tempo - the host's here, the master clock on the instrument:

| Divisor | Step | Divisor | Step |
|---|---|---|---|
| 0-9 | 1/1 | 65-77 | 1/8 |
| 10-25 | 1/2 | 78-90 | 1/8T |
| 26-38 | 1/2T | 91-103 | 1/16 |
| 39-51 | 1/4 | 104-116 | 1/16T |
| 52-64 | 1/4T | 117-127 | 1/32 |

From 26 each value is exactly 13 wide; the boundaries were found by halving on the instrument.

## 3. The pattern
One step count drives everything: it starts at 0 with the first key and goes up by one every step, muted ones
included, whatever is held. The keys are kept lowest first (at most 20). Each step picks an index into those keys
octave by octave - index i is key i mod n (n keys held), i / n octaves up - from the count:

- the main pattern has P places: the keys times the octaves (`total`), less the Sub Arpeggio's reach (§4); Up/Down
  has 2 x total - 2 places, less twice the reach;
- the count, wrapped round (Sub length x P) - a value kept in a byte, so wrapped again at 256 - gives the place
  (it / Sub length) and the step within the Sub Arpeggio (it mod Sub length);
- the place becomes a start index: Up the place, Down (half - 1 - place), Up/Down the place up to half - 1 and back
  down without repeating either end, Random a random start drawn whenever the place moves, where half = total less
  the reach;
- the index is (start + the Sub Arpeggio's offset) mod total.

So a key added or let go changes the lengths and the count carries on: holding C E G and adding C an octave up just
after the sixth step plays C E G C E G G C' C, as the instrument did, and a chord's first steps catch its keys
arriving one by one (Down plays C, E, then G E C).

Each note lasts until the next step and plays at its key's velocity. Poly ends the old note then starts the new one;
Mono and either Legato start the new note first; with Legato the same note twice is simply held on.

## 4. The Sub Arpeggio, and Mono priority
The Sub Arpeggio adds an offset to the start index, so its window always runs upward in pitch from the main note,
whatever the main direction - C E G A, Down, Sub Up over 2 plays G A, E G, C E. Over a Sub range R (never wider
than the keys held, n):

| Sub direction | Steps per place | Offsets | Reach |
|---|---|---|---|
| Up | min(R, n) | 0, 1, ... | steps - 1 |
| Down | min(R, n) | ..., 1, 0 | steps - 1 |
| Up/Down | min(2R - 1, 2n - 1) | 0 .. w .. 0, w = steps / 2 | w |
| Random | min(R, n) | a random one each step | steps - 1 |
| Still | R | 0 | 0 |

The reach keeps the windows from wrapping: once a window has held the top note the pattern starts again. Random draws
from a 32-bit shift-register generator (shifted left, 0x1D872B41 folded in when a bit falls off the top, from 0x1267,
its low 16 bits scaled to the range); the instrument shares its generator with other things, so its sequence is not
reproduced, only its kind.

In Mono (with or without Legato) with a note priority set, the arpeggio plays one key over and over: priority 1 the
lowest, 2 the highest. Stored 1 is the lowest-note priority and 2 the highest - the manual's table has them the other
way round, and the panel follows the instrument.

## 5. Starting and stopping
Free running, the first key starts the arpeggio at once, the count from 0. With Clock on the steps keep to the
clock's grid of the note value: a key pressed between two lines waits for the next one (measured: a chord pressed
mid-step started 136 ms later at 1/8, one pressed just before a line 14 ms later, and every step fell on the grid). The
grid is the host's beat while it plays, and runs on at its tempo while it is stopped, as the instrument's master
clock does. Letting go of the last key ends the note playing and the arpeggio. Run switched off ends the note playing
and forgets the keys.

## 6. The Mask
In Mask or Sub+Mask the step whose bit (the count mod the length) is off does nothing at all: no new note, and the one
before it is not ended - it ties over, sounding two steps (measured with full sustain: E held 600 ms at 300 ms
steps). The pattern moves on under it, since the count does, and the Mask does not start again with the pattern
(length 3 over a 4-note pattern, step 2 muted, plays C - G A - E G C - ...).

## 7. The check
`tools/nl3arptest.c` replays twenty sequences taken from the instrument - each direction, two octaves, every Sub
direction, Sub over Down, Up/Down and two octaves, a Sub range wider than the chord, three Masks, Mono priority, a key
added - feeding the chord one key at a time as MIDI did; checks that a muted step ties, and that with Clock on a key
waits for the grid; and prints the step at Rate 0, 64 and 127. All pass.
