# nl3Envelope.c - the envelopes

Amp, Filter and Mod envelopes share one law, measured on the instrument (2026-11-14).

## 1. Form
Each stage moves the envelope a fixed fraction of the way to its target, every step, so every segment is an exponential.
The attack heads for 1.25 and ends the moment it reaches full (1.0): a curve close to a straight line that bends a
little towards the top. The decay heads for Sustain and stays in that stage for as long as the key is held, so a
Sustain changed while a key is down is reached at the decay's own rate. The release heads for silence from wherever
the envelope is. A new note's attack starts from where the envelope is.

## 2. Times
The envelopes step 22050 times a second. The number of steps the envelope takes to close all but 1/e of the distance
is 1.013266e-12 x (dial + 40.15)^8 for Attack and 3.540686e-13 x (dial + 40.15)^8 for Decay and Release (one law for
both). Attack 0 reaches full in 0.5 ms, Attack 64 in 1.02 s, Attack 127 in 45 s; Decay and Release halve in 0.075 ms
at 0, 154 ms at 64 and 6.4 s at 126, and at 127 they hold. Within 0.2% of every setting measured.

## 3. Sustain
The amplifier's Sustain is (dial/128)^2, the Filter envelope's dial/128, 127 being full for both: Sustain 64 holds the
amplifier at exactly 0.25 and the Filter envelope at 0.5.

## 4. Exp Attack
The envelope runs as above and its output is its cube; Sustain's target becomes the cube root of its level, so the held
level is the same with the switch on or off. The attack starts slowly and ends steeply, and the decay and release
fall quickly at first and slowly towards the end.

## 5. Silence
A voice ends when its released level falls below 2^-16.

## 6. The Mod envelope's modes
AD: the attack, then a fall to nothing whether the key is held or not. AR: the attack, held at the top while the key
is down, and a fall at the same time on release. Repeat (AD only): the fall aims a little below zero (-0.05) and the
attack starts again the moment it reaches zero, for as long as the key is held; released, it falls and stays down.
Measured on the instrument, every 50 ms through a held note and its release, and at 1 ms around a repeat.
