# sn2Envelope.c - the envelopes

## 1. Form
Per-sample at the engine's rate. Attack rises in a straight line to the top. Decay is an exponential approach to Sustain, a little quicker the higher Sustain is; at Sustain 0 it has the same halving time as Release. Release is the same fall, to silence.

## 2. Times
Attack is the time to the top and Decay and Release the time to half, measured at nine points of the dial and joined in the logarithm. About three times longer every 16 steps, slowing near the top of the dial. Decay and Release share their points.

## 3. Sustain
The dial squared on the amplifier, the dial itself on Env 2 and 3.

## 5. Silence
A voice ends when Release falls below 2^-16.

## 6. Sustain changed while held
Glides to the new level; the rate is not measured.

## 7. Env 2 and 3
The same law as the amplifier. Each moves the cutoff by 2.2146 dial steps for every step of its amount from 64, scaled by the same velocity curve as the amplifier. Delay and the other routes are not built.

## 8. Sustain Time and Sustain Rate
Measured on the amplifier envelope, Attack and Decay 0 unless stated; Env 2 and 3 take the same law.

- **The hold starts when the decay is done**: when its distance to Sustain, closing by the Decay halving time (without
  §1's quickening), falls to 1/128 of full scale. Sustain is taken as (S/128)², the dial itself on Env 2 and 3. So
  Sustain 127 still waits one halving time, and Sustain 0 seven. Fits Decay 48-96 at Sustain 0-127 within about 2%.
  The level carries on its own approach meanwhile.
- **Sustain Time** is then how long it holds before Release starts: 0 about 5 ms, 64 0.32 s, 126 9.0 s, about 2.35
  times longer every 16 steps, faster at the bottom. Seventeen points, every 8 steps, joined in the logarithm as §2;
  127 holds until the key is let go.
- **Sustain Rate** below 64 makes the sustain fall steadily in decibels: 14200 · 10^(-Rate/16) dB/s, so 0 is gone in
  a few milliseconds, 16 falls 1400 dB/s, 48 14 dB/s, 63 1.6 dB/s. It falls to silence and ends the voice if the hold
  outlasts it. 64 is level.
- **Above 64** the level climbs to the top in about 27 ms, the same for every value 65-127, and only when Sustain Time
  is below 127; with 127 a rising Rate is ignored.

## 9. One Shot
The program's One Shot switch (byte 237, bit 2): the key's release is ignored, and Release starts as soon as the
decay is done (§8), with no hold whatever Sustain Time says. An attack or decay longer than the note still runs to
the end. Applied to all three envelopes.
