# osFx.c

Each layer's own effects, after its voices and before the layers are summed: the filter, then saturation.

## 1. Filter
Low Cut (a high-pass) and High Cut (a low-pass), 20 Hz to 20 kHz each, at 12 or 24 dB an octave: Butterworth, one
2-pole stage (Q 0.707) or two (Q 0.541 and 1.307), so the response is flat to the cutoff, -3 dB there, and falls at
the slope beyond. Measured (`tools/osFxTest.c`, 48 kHz): High Cut 1 kHz -3.0 dB at 1 kHz, then -12.4 / -24.5 dB at 2 and
4 kHz (12 dB) or -24.2 / -48.9 (24 dB); Low Cut the mirror image.

## 2. Moving the cutoffs
Each stage is a trapezoidal state-variable filter, which stays stable and click-free while its cutoff moves. The
cutoffs glide toward their dials in octaves (10 ms) and the coefficients are worked out every 32 frames.

## 3. Saturation
Gain into a curve, then Output. Classic: tanh, a smooth soft clip - odd harmonics. Drums: a hard clip, flat-topping the
peaks of a hit. Tape: tanh leaning by a fixed offset, which adds even harmonics too (the offset taken out after by a
DC blocker at 8 Hz); Warmth turns the lows up or down before it (a shelf at 150 Hz, +-12 dB) and HF is a 12 dB/octave
low-pass after it (1 to 20 kHz).

## 4. Aliasing
A curve makes harmonics, and those above half the sample rate fold back as tones that are not harmonics. Each curve
is applied through its antiderivative (first-order antiderivative anti-aliasing): the output is the curve's mean
between this input and the last, which smooths what would fold. It adds no latency beyond half a sample, so layers stay
in step. Measured on a 5 kHz sine driven +18 dB at 48 kHz, worst alias against the same curve applied sample by sample:
Classic -21.9 dB (-17.0 plain), Drums -19.1 (-14.1), Tape -34.5 (-17.2, its HF helping). Near half the sample rate the
method helps least; §6 is the remedy, as an option.

## 5. Soft
A third slope, gentler and lopsided: Low Cut is two trapezoidal one-pole high-passes in series - 12 dB/octave with a
soft knee, -6 dB at the dial - and High Cut a single one-pole low-pass - 6 dB/octave, -3 dB at the dial, falling to
nothing at half the sample rate. It thins a sound's bottom firmly but only shades its top, which suits drums. Measured:
High Cut 1 kHz -3.0 / -7.0 / -12.5 dB at 1, 2, 4 kHz; Low Cut 1 kHz -6.0 dB at 1 kHz, -24.6 at 250 Hz.

## 6. 2x oversampling (optional)
Controls > Oversample Saturation 2x, for the whole instance (owner, 2026-11-11). The curve runs at twice the rate
between two 47-tap half-band filters (Kaiser 8; every other tap 0, so 24 multiplies a phase), with the antiderivative
kept at the doubled rate: without it the hard clip still aliased at -47.7 dB below 20 kHz. The filters make the
output 23 frames late (0.48 ms at 48 kHz), so while it is in force every layer is 23 frames late - a layer not
saturating through a plain delay - and the plug-in reports 23 to the host, which lines it up. It is in force only
while some layer with a sample has saturation on; otherwise nothing is delayed and 0 is reported (owner, 2026-11-11). The antiderivative adds a quarter
of a frame on a saturating layer (5 us; a comb under 0.1 dB below 20 kHz against an unsaturated twin).
Measured on a 5 kHz sine at +18 dB, worst alias below 20 kHz: Classic -35.1 dB -> -129.0, Drums -31.0 -> -82.7, Tape
-35.4 -> -122.5. What is left is at 23 kHz (the 25 kHz harmonic, at the half-band's corner): -28.6 / -25.9 / -41.1 dB,
above hearing. Cost per stereo layer: Tape 0.3% of a core, 0.7% oversampled; a 24 dB filter 0.1%.
Through the VST3: a note's sound starts at frame 389 without, 412 with it - 23 later - saturating or not.
