# nl3Filter.c - the filter

The Nord Lead 3's single-mode lowpass and highpass at 12 and 24 dB, in two arithmetics: FIXED is the instrument's
own, word for word (24-bit words, 56-bit accumulator, see `nl3Fixed.md`), FLOAT is the same steps in doubles. The
other modes (6 dB, band-pass, band-reject, Classic, Distorted, the multi modes) are not built yet.

## 1. The input
The lowpass modes average the input over two stored samples before the filter: the mean of this sample and the last,
then the mean of that and the last mean, which is (x + 2x' + x'')/4 overall. A zero at Nyquist, so the top octave is
softened as on the instrument. 12 dB takes the input at 1/32, 24 dB at 1/4. The highpass modes take no average; their
input is scaled by N² (§5), 1/32 at 12 dB and 1/4 at 24 dB.

## 2. A stage
A state-variable stage with the low state updated from the previous band:

    low  = low' + F·band'
    high = u − low − (k·band' + (1 − k)(1 − F)·band'³)
    band = band' + 4F·high

The cubic term is the damping's own non-linearity: it is negligible at normal levels and takes over as the band state
nears full scale, which is what holds a self-oscillating filter at a steady level. Every stored state is limited to a
word. 24 dB is two identical stages: LP feeds stage 2 with 2·low₁/8 (averaged again, §1), HP with N²·2·high₁/8.

## 3. Fixed point
`stage_fixed` is the stage as the instrument computes it, each step the same operation in the same order: products
doubled, sums kept in the 56-bit accumulator, a value limited to a word only where the instrument moves one out. The
output is 2·low (LP) or 2·(u − low − damping) (HP) as a word, times the gain rounded convergently, shifted up 5 and
down 2. Checked sample for sample on noise against the instrument's arithmetic: LP and HP, 12 and 24 dB, cutoff 20,
64, 100 and 127, resonance 0, 64, 120 and 127, two input levels, 3000 samples each - no difference in any word.

## 4. Cutoff
The dial sets a word w = (969/2²³)·2^(v/12), a semitone a step, with modulation added in semitones. The top of the
dial is a fixed word, 0.8125, well above the 127th step: fully open. g = 8w, which is π f / 44100 for the dial's
440·2^((v − 61)/12) Hz.

## 5. The coefficients
With q the damping (§6) and N = 1 / √(1 + 2qg + g²):

    F = g·N        k = q·N        and N² for the highpass input

At resonance 0 this is F = g/(1 + g) and F + k = 1; at the top of the resonance, F = g/√(1 + g²). N is read from a
table of 1/√(1 + i/8), i = 0..640, rounded to words, with straight lines between entries; s = 1 + 2qg + g² beyond 81
takes the last entry. The table is computed, not stored. Against the instrument's coefficients over 145 settings
(cutoff 0-127, resonance 0-127): within 12 of 2²³ for F, 10 for k, 19 for N².

## 6. Resonance
d = 1 − Resonance/128, with 127 counting as 128 (d = 0, no damping at all). 12 dB: q = d⁴. 24 dB: q = d², held at
1/64 or more, so the two stages together do not ring past self-oscillation. Both stages of 24 dB take the same F and k.

## 7. The rate
The voices run at 44.1 kHz. The coefficient law of §4-5 puts the 12 dB filter's resonant peak at the dial's displayed
frequency only at that rate: measured, 523.25 Hz at cutoff 64 places the peak at 0.011856 of the rate, 44.13 kHz.
The instrument's DACs run at 44.1 kHz.

## 8. Gain
The output is scaled by G = 1 − c(1 − r), where c is 0.75 at 12 dB and 0.9 at 24 dB, and r is d² (12 dB) or
max(d², 1/64) (24 dB). It falls as resonance rises - 0.25 and 0.114 at full resonance - so the peak does not jump in
level. Within 2.3 of 2²³ against the instrument's over the 145 settings.
