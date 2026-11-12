# tiFixed.h - fixed-point arithmetic

The instrument computes in 24-bit words with 56-bit accumulators. The Arithmetic parameter, per instance, runs the stages that have been converted that way. Float, the default, is the same laws in doubles. Tests run with Fixed; `tools/tirender` uses it unless TI_FLOAT is set.

## 1. The scale
A voice's sine of 1 stands at 1 / (4 x root 2), 0.1768, of full scale. Three separate measurements settle on it: the saturation curves, the ladder's response, and its self-oscillation level.

## 2. Multiplying
Two words multiply as fractions, so the product is doubled.

## 3. Rounding
Convergent: exact halves go to the even word. Moving a word out limits it, rather than wrapping.

## 7. Stages converted
The digital biquad: coefficients held at half scale in words, scaled up when read, state in words. All four
envelopes (tiEnvelope §7). Everything else still runs in doubles.
