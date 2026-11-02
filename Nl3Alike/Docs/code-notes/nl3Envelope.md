# nl3Envelope.c - the envelopes

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
