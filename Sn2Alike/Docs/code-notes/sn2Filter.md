# sn2Filter.c - the filter

## 1. Structure
Two state-variable sections, each run twice per output sample, the output the average of the two passes. Each pass: low += f band; high = input - low - damp band; band += f high.

## 2. Types
All twelve types and three slopes are this one structure. They differ only in how each section's input, band, low and high outputs are summed, and in whether section 2 follows section 1 or runs beside it. Slope changes LPF, BPF and HPF only. At 24 dB they use the same taps as Res LPF and Res HPF, and BPF the band of each section. The "+" types are parallel.

## 3. Laws
The voices run at 39062 Hz, whatever the host's rate. f is exponential in the Cutoff dial, 0.037465 at 64, doubling every 12 steps and stopping at 0.99863. With d = (127 - Resonance) / 127, a section's damping is 2 x 0.7 d (1 - f d / 2). In LPF, BPF and HPF section 1 takes d no lower than 0.5, so only section 2 resonates; that is the whole difference from the Res types at 24 dB. Special Width is not built; at 0 it does nothing. 

## 4. Level
Damping is at least 2 (|band| - 0.25), which holds down a resonance as it grows. That depends on the level reaching the filter, not yet set against the reference. The low and band states saturate at full scale, as a fixed-point register does.

## 5. Overdrive
A saturator ahead of the filter: y = x / (1 + W (|x| - 1)), so full scale stays full scale while lower levels rise by up to 1 / (1 - W). W climbs from 0.016 at Overdrive 0 to 1 at 127, nearly as 1 - (1 - v) squared where v is Overdrive / 128.

## 6. Q Normalise
The filter's input is scaled by 1 - (Q / 128) (Resonance / 127), so resonance does not raise the level. Above Q 120 the damping is also trimmed by twice (Q - 120) / 128 x (1 - f).
