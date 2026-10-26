# tiFilter.c - the filter

## 1. Cutoff
A biquad whose cutoff runs over 32 points, the third-octave centres f(p) = 1000 Hz x 2^((p - 17)/3) (1 kHz at point
17), the last fixed at 22968.75 Hz and every point held below 0.49 of the sample rate. The cutoff in hertz does not
depend on the sample rate. The Cutoff dial maps linearly onto points 1 to about 30.8: p = 1 + 30 x cutoff/128, so one
dial step is 0.234 points, 0.078 octave. Between points, cos(w) and the pole radius are interpolated linearly in the
point (not in frequency), as the instrument does.

## 2. Resonance
Nine rows, the dial's 0..127 spread over them as row = 8 x resonance/128 and interpolated. Each row is one Q at every
cutoff, in dB: -3 (1/sqrt 2), -1, 4, 9, 14, 19, 24, 40, 48. Each row also carries an output gain: for low and high
pass 1, 7/8, 7/8, 3/4, 5/8, 0.515625, 0.40625, 0.34375, 1/4 - the level falls as the peak rises; for band pass 1,
1.125, 2, 3, 4, 8, 12, 28, 64 - it rises.

## 3. The coefficients
With C = cos(w) and r^2 = (1 - a)/(1 + a), a = sin(w)/(2Q), both interpolated as above, the denominator is
1 - C(1 + r^2) z^-1 + r^2 z^-2, and the numerators are the cookbook ones, scaled by the row's gain:
low pass g(1 + r^2)(1 - C)/4 x (1, 2, 1); high pass g(1 + r^2)(1 + C)/4 x (1, -2, 1); band pass g(1 - r^2)/2 x
(1, 0, -1); band stop (1 + r^2)/2 x (1, -2C, 1) - the notch, with no gain row (its gain is 1 at every
resonance). Checked against reference renders through Filter 1 alone (saw at C1, harmonics against the open filter):
Cutoff 60 at Resonance 0, 64 and 100 within 0.2 dB from 200 Hz to 5 kHz, the resonant peak included; band stop
at Resonance 0, 64 and 120 within 0.6 dB, the notch included (it narrows to nothing as the resonance rises).

## 4. Not yet
Not here yet: Filter 2 and the routings, the 4-pole
digital forms, keyfollow, the envelope, the Saturation curves after the filter, and whether the instrument smooths a
Cutoff change (one step is nearly a semitone - todo.md).

## 5. The analog modes: a four-stage ladder
Analog 1 to 4 Pole are one filter, four one-pole stages in series with feedback from the fourth into the first,
tapped after the first, second, third or fourth stage. The feedback is always taken from the fourth, so the 1-pole
tap is a resonant ladder's first stage, which is what gives it a notch beside its peak.

It runs at twice the sample rate, two passes per output sample: the first fed the mean of this input and the last,
the second this input, the output taken from the second. One pass, with g each stage's coefficient and F the
fourth stage from the pass before (plus a little noise, about -84 dB, on the first pass, which is what lets a silent
filter start to self-oscillate):
  s1 = (1-g) s1 + g (in x u - k F)
  t  = sat(pre x s1),  sat(x) = 2x - x|x| with x held to +-1
  s2 = (1-g) s2 + g post t;  s3 = (1-g) s3 + g s2;  s4 = (1-g) s4 + g s3
The 1-pole tap is t itself, scaled by post x 0.707.

**Cutoff.** g is chosen so that the loop resonates at the digital filter's cutoff (§1): each stage turns the phase
by (pi - w)/4 at w = 2 pi f / (2 x rate), the -w being the loop's one-pass delay. That has a closed form:
1 - g = tan(phi) / (sin w + tan(phi) cos w), phi = (pi - w)/4.

**Resonance.** The loop gain k x pre x 2 x post (the saturator's slope at zero is 2) runs over 16 steps of the dial,
interpolated: 0.5 at 0, rising evenly to 3.75 at 64, then 4 (where it starts to self-oscillate), 4.25, 4.5, 5, 6, 8,
16 and 32. Above a cutoff of about 60 the dial is held back: the step used is 16 x min(res/128, 1 - (7/17)(c - 15/32))
with c = 0.9375 cutoff/128 - 5/256, so the resonance cannot run away at the top.

**Drive.** d = the Osc Volume above its centre, 0..1: in = 2^(2d-4), pre = 2^(d+7), post = 2^(-3-d). The loop gain does
not depend on d; the small-signal gain through the ladder is 2^(2d+1)/(1+K), K the loop gain.

Checked against reference renders (Init, Filter 1 alone, a saw at C1, harmonics against the open filter): 1 to 4
Pole at Cutoff 60, Resonance 0 and 64, within about 0.5 dB from 100 Hz to 3 kHz (the 1-pole notch included); at
Resonance 100 and 127 the filter self-oscillates at 637.8 / 634.4 Hz against the reference's 636.7 / 640.0, its tone
the same 28-30 dB above the note.

## 6. The ladder's input level
The saturator works on the instrument's own signal level, which the panel does not show, so the oscillator is scaled
into the ladder by LADDER_SCALE and back out after it. 0.115 was fitted to the renders of §5: the error at
Resonance 0 and 64, all four pole counts, is 0.1-0.7 dB rms there and grows either side (0.9 dB at 0.10 and 0.13). It
belongs to the oscillator's level rather than the filter, and is to be revisited when the oscillator and Osc Volume
are measured (todo.md).
