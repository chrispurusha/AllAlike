# nl3Filter.c - the filter

The Nord Lead 3's single-mode lowpass and highpass at 6, 12 and 24 dB band-pass at 12 and 24 dB and band-reject, in
two arithmetics: FIXED is the instrument's
own, word for word (24-bit words, 56-bit accumulator, see `nl3Fixed.md`), FLOAT is the same steps in doubles. The
other modes (Classic, Distorted, the multi modes) are not built yet, and play as the
nearest built one.

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
down 2. Checked sample for sample on noise against the measured output: LP and HP, 12 and 24 dB, cutoff 20,
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

## 9. Band-pass 12 dB
The same stage (§2) with the band state as the output, 8·gain·band. The input is taken at N rather than N² and
averaged once - (u + u')/2 with u = N·x/32 - where the lowpass averages twice. Its coefficients differ in two ways:
the frequency is prewarped, g = tan(1.00022 · 8w) below the dial's top (at the top the fixed open word, as the
lowpass), so the peak stays where the dial says as the cutoff rises - 1.9% above the lowpass's g at cutoff 96, 47% at
120; and the gain's take is 0.875, so G = 1 − 0.875(1 − d²), 0.125 at full resonance. Q is the 12 dB lowpass's, d⁴.
Measured over a grid of 150 cutoff and resonance settings: F, k and N within 37 of 2²³, gain within 3; and the fixed
path reproduces the measured output exactly on noise at every cutoff and resonance of §3's test.

## 10. Band-pass 24 dB
Two band-pass stages (§9) in series: the input at 1/4 into the first, the first stage's band at 1/8 into the second -
taken before it is limited to a word - each stage with its own input
average; the second's band is the output. Both stages share F, k and N. Q and the gain are the 24 dB lowpass's -
q = max(d², 1/64), take 0.9 - with the band-pass's prewarp and N. Within 36 of 2²³ over the grid, and exact on noise as
§3.

## 11. Lowpass 6 dB
The state-variable stage again (§2), run differently: the input is (x + x')/2 at 1/16, the low state takes
2·F·band' and is stored halved, and the output is the new low plus the last one, less N times the new band - a
first-order response from the second-order stage. Out 4·gain·that. Its coefficients are the band-pass's frequency and
N (§9: prewarped, N rather than N²) with the 12 dB lowpass's damping (d⁴) and gain take (0.75). Within 37 of 2²³ over
the grid, and exact on noise as §3.

## 12. Highpass 6 dB
The same stage with the input N·x/16 (rounded), no averaging; the output is the mean of the new band and the last,
less N times the high leg, 4·gain·that. Coefficients as §11. Within 37 of 2²³ over the grid, exact on noise as §3.
In floating point both 6 dB modes part from the fixed path with the cutoff fully open and the input near full scale,
where the stage runs into saturation.

## 13. Classic (not yet playable)
Two passes a sample over the held input - the filter runs at twice the voice's rate - each a stage without the cubic
term (§2's stage less its non-linearity) feeding a full stage at half level; out 4·gain·low of the second stage, at
half scale. The sample path is built and exact on noise. Its coefficients are its own: the first stage's damping is
fixed near 0.895 and its frequency rises with resonance (to about 2.4 times the second stage's at resonance 32,
settling at 2.23 above 96); the second stage takes d⁴ at about half the lowpass's frequency, shifted a little with
resonance. Until those laws are fitted, Classic plays as the 24 dB lowpass.

## 14. Band-reject
§2's stage without the cubic term, input at 1/16; the output is the previous sample's N²·high plus the mean of this
and the last low sum (2·low + F·band halved, plus the last low), at 4× - no gain stage. One form serves both slopes.
Coefficients: the band-pass's prewarp (§9), q = max(d², (21/64)²) - a notch never sharper than that - and N² for the
high leg. Within 38 of 2²³ over the grid; exact on noise as §3.

## 15. Multi mode, Filter 2 type 1: lowpass beside highpass
The input, scaled by the gain first, goes to two 12 dB stages side by side: a lowpass on the first cutoff (the input
averaged twice, as §1, at 1/4) and a highpass on the second (the input at N², §1, at 1/4); out (8·high + 8·low)/4, the
low leg limited to a word before the sum. Both filters are prewarped (§9), take q = d² - not the 12 dB lowpass's d⁴ -
and the gain is the band-pass's, 1 − 0.875(1 − d²). Measured over the grid with the two cutoffs moving in opposite
directions: every coefficient within 30 of 2²³; exact on noise as §3, and the floating-point path within 5·10⁻⁴.
The second cutoff takes the envelope, LFO and morph moves only when Filter 2 Env is on; otherwise only keyboard
tracking - taken from the manual's description, not yet measured. The other five multi modes still play as the first
filter alone.

## 16. Multi modes, Filter 2 types 4 and 5: three all-pass stages with feedback
Three linear stages (§2 without the cubic term) taken as all-passes - low + high − 2k·band - in series, all on one F
and k, their output fed back to the input. The input is first scaled by 1 + c·R (c = −0.85 for type 4, −0.75 for type
5) and taken at 1/16, the feedback subtracted (type 4) or added (type 5) at R, then averaged once. Type 4 puts out the
all-pass chain, 4×; type 5 the chain plus the scaled input, 2× - the dry and phase-turned signals cancelling into
notches. Coefficients, measured over the grid: F from Filter 1 Freq, prewarped (§9); the damping from Filter 2 Freq,
q = (Freq2/128)², 127 counting as 128 - so Freq 2 sets how wide the peaks or notches are; and R = Resonance/128.
F within 23 of 2²³, k within 8, R exact; exact on noise as §3, floating point within 3·10⁻³.
