# prophFilter.c

The Prophecy's filter, taken from the instrument's own arithmetic (2026-11-21). One structure gives all five types;
the plug-in has it twice over, in the instrument's fixed point and in floating point.

## §1. Rate
The instrument's voice runs at 48 kHz; so does the engine, whatever the host's rate.

## §2. Cutoff and its glide
The panel sets a target for the working cutoff word f, in its top 16 bits, together with a step. Every sample f moves
by the step (in the same top 16 bits) and stops at the target rather than pass it. The step is set once per change,
from the last target to the new one: the distance divided by 128, towards zero, at least 1 - so a cutoff change
glides over about 128 samples, 2.7 ms. A change during a glide restarts it from the new distance.

## §3. Types
The filter computes every response each sample and the type picks one: 0 THRU (the input), 1 LPF, 2 HPF, 3 BPF,
4 BRF (notch).

## §4. The state-variable filter
With f the working cutoff, q the damping (negative), g the input gain, x the input:
notch = g x + 2 q band; high = notch - low; band += 2 f high; low += 2 f band (the new band).
The order matters: the low-pass integrates the band-pass computed in the same sample.

## §5. Cutoff law
The cutoff target at resonance 0 is a table of the 100 Cutoff steps - about a semitone a step above step 10,
irregular below it. Between steps (modulation) it is interpolated linearly for now; the instrument computes finer
values, not yet known.

## §6. Resonance
Resonance sets three targets at once: the damping (-1 at 0, falling to about -0.0005 at 99), an input gain that holds
at 1 up to 70 and then falls to 0.06 at 99 (which keeps the level), and a lift of the cutoff target by 1 to 2.41,
the same at every cutoff. All three are known at 0, 10 .. 90 and 99 and interpolated between.

## §7. Gain and damping glide
After each sample the input gain and the damping each glide one pole toward their targets: new = k1 target + k2 old,
with k1 = 0.0044 for the gain (about 5 ms) and 1/128 for the damping (2.7 ms).

## §8. Fixed point
32-bit words with 1.0 at 2^31; a 24-bit input sample. Products go into a 48-bit accumulator: a coefficient times the
24-bit input shifted down 7; a coefficient times a word through the multiplier's A port - the word's low 8 bits
cut - shifted down 15. A word is stored as the accumulator's bits 16 up, saturating past 48 bits; the output is the
selected word's top 24 bits. Checked word for word against the instrument's arithmetic over a cutoff change and at
Resonance 99. The floating-point path is the same structure without the truncations; the two agree to better than
-95 dB away from full scale.

## §9. Level in and out
The oscillator mix reaches the filter at a quarter of full scale and its output is scaled back by 4. The real level
into the filter is not known until the oscillators are; a half clipped the fixed-point path at moderate resonance.
