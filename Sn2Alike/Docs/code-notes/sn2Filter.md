# sn2Filter.c - the filter

## 1. Structure
Two state-variable sections, each run twice per output sample, the output the average of the two passes. Each pass: low += f band; high = input - low - damp band; band += f high.

## 2. Types
All twelve types and three slopes are this one structure. They differ only in how each section's input, band, low and high outputs are summed, and in whether section 2 follows section 1 or runs beside it. Slope changes the tables of types 3-5 only.

## 3. Laws
The Cutoff dial is in semitones: f doubles every 12 steps, 0.0306 at 64, rising a little faster near the top. Damping falls in a straight line with Resonance. Both were measured at 48 kHz, and f scales with the sample rate. Overdrive and Key Tracking are not built, nor Special Width, which measured no effect.
