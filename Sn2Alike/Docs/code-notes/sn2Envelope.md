# sn2Envelope.c - the envelopes

## 1. Form
The instrument's own steps, one every four samples at the engine's rate. Attack adds the same amount each step, to the
top. Decay takes a part of the distance to Sustain; when what is left is under 1/128 of full scale the decay is done
and the hold begins. A key let go releases at once from wherever the envelope is: a part of the level each step
(checked on the instrument mid-attack and mid-decay, 2026-11-22).

## 2. Times
From the dials by the instrument's laws, which FIXED shares (sn2Fixed notes §8): exact, nothing fitted.

## 3. Sustain
The dial over 128, squared on the amplifier. Sustain Time adds to a clock that ends the hold at a half; 127 adds nothing.
Sustain Rate below the centre takes a part of the level each step, above it adds to it up to the top. With the rate at
the centre the level closes on Sustain as in the decay, by 1/32 of the distance a step while that is over 1/128.
One Shot (byte 237, bit 2) ends the hold as it begins and ignores the key's release.

## 4. Silence
A voice ends when its level falls below 2^-16.

## 5. Delay, repeats and the key
Env 2 and Env 3 wait for Delay's clock to fill before Attack. A repeat count (extension bytes 13-15; 127 for ever) sends
the decay's end back to Attack. Key tracking (extension bytes 4-6, 64 none) moves the time dials by the key's distance
from middle C: h = (note - 60)(track - 64) 2 / 256 rounded down off Attack, h + h/2 off Decay, Release and Sustain Time,
and off Sustain Rate's distance from its centre read as a time. Checked on the instrument 2026-11-22, repeats to the dB.
