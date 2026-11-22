# sn2Filter.c - the filter

## 1. Structure
Two state-variable sections, each run twice per output sample, the output the average of the two passes. Each pass: low += f band; high = input - low - damp band; band += f high.

## 2. Types
All twelve types and three slopes are this one structure. They differ only in how each section's input, band, low and high outputs are summed, and in whether section 2 follows section 1 or runs beside it. Slope changes LPF, BPF and HPF only. At 24 dB they use the same taps as Res LPF and Res HPF, and BPF the band of each section. The "+" types are parallel.

## 3. Laws
The voices run at 39062 Hz, whatever the host's rate. f is exponential in the Cutoff dial, 0.037465 at 64, doubling every 12 steps and stopping at 0.99863. With d = (127 - Resonance) / 127, a section's damping is 2 x 0.7 d (1 - f d / 2). In LPF, BPF and HPF section 1 takes d no lower than 0.5, so only section 2 resonates; that is the whole difference from the Res types at 24 dB. Special Width is not built; at 0 it does nothing. 

## 4. Level
Damping is at least 2 (|band| - 0.25), which holds down a resonance as it grows. It is worked out once a sample for each section, from the band as the sample starts, and both passes use it; the high-pass term is limited to full scale before it is used, and the low and band states saturate at full scale, as fixed-point registers do. Set against the instrument's own arithmetic (2026-11-22, sn2Fixed notes): recomputing the damping in each pass, from the band the first pass had just moved, damped a driven resonance far too hard - a Notch driven near full scale agreed to 10 dB, now 98 dB; Res LPF at Resonance 127 and Overdrive 127 54 dB, now 81 dB - and an unlimited high-pass let a driven high-pass run away. At low levels, where neither applies, the two agreed to 52-111 dB before. The level reaching the filter matches the instrument's within 0.2 dB on seven settings, Overdrive 0 to 127.

## 5. Overdrive
A saturator ahead of the filter: y = x / (1 + W (|x| - 1)), so full scale stays full scale while lower levels rise by up to 1 / (1 - W). W climbs from 0.016 at Overdrive 0 to 1 at 127, nearly as 1 - (1 - v) squared where v is Overdrive / 128.

## 6. Q Normalise
The filter's input is scaled by 1 - (Q / 128) (Resonance / 127), so resonance does not raise the level. Above Q 120 the damping is also trimmed by twice (Q - 120) / 128 x (1 - f).

## 7. Special Width
The resonant and the "+" types set their two sections apart: the first Width / 2 Cutoff steps below the cutoff, the
second as far above (each with its own damping, from its own f, and its own Q Normalise trim); the plain LPF, BPF and
HPF do not. Width 64 puts them 2 2/3 octaves either side. Set from the instrument's own arithmetic (2026-11-22,
sn2Fixed notes §5), it was the cause of the float engine playing 19 dB loud on a Res LPF program at Width 64 - one
section 2 2/3 octaves under the cutoff takes far more out than one at it. Now within 0.1 dB of the instrument at
Width 0, 64 and 127, and on the Notch; on the 13 programs in the backup that use it, median level error 3.4 -> 1.8 dB,
spectrum 4.1 -> 1.6 dB. Width comes from the program (extension byte 7; the main dump's 3-bit copy is not used) and
is on the Filter page.
