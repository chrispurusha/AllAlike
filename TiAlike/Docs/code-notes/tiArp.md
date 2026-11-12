# tiArp.c / tiArpPatterns.c - code notes

The arpeggiator, as the instrument's: the same note choices, the same 63 patterns, the same clock. Checked against
reference renders throughout (2026-11-11): every mode note for note with chords in and out of order and one to four
octaves; all 64 patterns onset for onset within 6 ms; swing, the clock divisions and note velocities.

## 1. Keys and Hold
The keys are kept in the order played. A key let go is forgotten, unless Hold is on: then it stays until a new key goes
down with none held, which replaces the chord. Turning Hold off lets go of the keys it was keeping. The sustain pedal is a second Hold while the arpeggiator runs: keys let go with it down stay, lifting it lets go of those no longer down, and it never holds the arpeggiator's own notes, which still stop at each step (the reference, rendered with and without the pedal, 2026-11-12). The mode is page B
15 (page B 1 in a Single older than the TI format): Off, Up, Down, Up&Down, As Played, Random, Chord, Arp>Matrix.

## 2. Patterns and the clock
A pattern is a list of steps: a note length and a duration, both in 1/65536ths of a step, and a velocity, 0 being a
rest. Patterns 1-63 are the instrument's own (`tiArpPatterns.c`, generated); pattern 0 is the user pattern from page D
(step k: length, velocity, and bit 0 of its third byte for on), as many steps as page C 127 says plus one.
The clock runs at 96 ticks a quarter note from the host's tempo. Arp Clock sets a step's ticks: 1/16 is 24, from 3
(1/128) to 192 (1/2), triplets and dotted values between. A step's duration is cut to whole ticks (with a margin of
1/64 tick, so a third of a step lands on 8 and 3.2 steps on 76, as on the instrument).
Swing works on pairs of steps: the first takes (64 + Swing / 4) / 128 of the pair - 50% at 0, 74.8% at 127.

## 3. Note length and velocity
A note sounds for its step's length times Note Length / 64, in whole ticks and at least one - half its length at 32,
twice at 127, so a long step at 127 runs into the next. A step's notes stop as the next step's start. The velocity is
the key's plus the step's less 100, at least 1 and at most 127, so a step at 100 plays the key as struck.

## 4. The modes
- Up: the lowest key above the last one played; past the highest, the next octave up, back to the first after the
  range (Octaves 1-4). Down mirrors it, starting from the top octave.
- Up&Down: as Up, then at the top as Down, turning at either end without playing the end note twice.
- As Played: the keys in the order played, stepping on before it plays (so the second key comes first), then the next
  octave.
- Random: any key, in any octave of the range.
- Chord: every key at once.
- Arp>Matrix: the keys play as they are (its matrix source is not built).

## 5. Starting
The first key starts the pattern from its first step there and then: a key a millisecond later misses that first
step (Down on C-E-G struck 1 ms apart starts on the C an octave up, as the reference does). Keys struck at the same
moment are all in it. When the last key is let go the pattern stops, and the next key starts it again.
A renderer that hands MIDI over a block at a time gathers keys a few milliseconds apart into one moment; compare
against renders that keep each key at its own time.
