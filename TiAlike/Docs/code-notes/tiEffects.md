# tiEffects.c - the effects

## 1. EQ
A low shelf, a parametric middle and a high shelf, 0.25 dB a step about 64. The shelves are gentle, the cookbook's with slope 0.5, their corners 1.75 times further in than the frequency shown.

The middle is not the cookbook's peak, whose cut is as wide as its boost: it is a band pass of unit peak added to the signal, (G - 1) of it, so a cut is much narrower than the boost of the same size (at Q 64, 0.87 octaves at -12 dB against 3.0 at +12, half-gain width). It is built as the digital filters are (tiFilter notes): on their third-octave points, the point Frequency / 4 - 1 (a semitone a step, 630 Hz at 64), and on their Q rows with two broader ones below them, Q -11 and -7 dB, then -3, -1, 4 and 9 dB; the row is 5 Q / 128, so the dial spans Q 0.28 to about 2.7. Point and row interpolate as the filter's do, the cosine linearly and r^2 / 2 between rows, which is why the Q does not climb smoothly. Against reference renders, every Frequency, Q and Gain tried within 0.24 dB (0.01-0.04 rms), Frequency 127 within 0.64.

## 2. Distortion
After the voices and before Patch Volume and the EQ, on each side, so its character does not change with Patch Volume. A Single older than format 9 has no Distortion Mix, and is all distortion whatever that byte holds.

Nearly every curve is a shaper with no memory, then a delay of 26 samples, then a 14.7 Hz high-pass. A plain input-to-output curve cannot hold that, and is what the first version tried.

- Light, Soft, Medium, Hard and Digital are saturation curves 1-5, with Intensity as drive.
- Wide, the Bounces and the Folds are one curve: level times a sine at a rate rising 10 dB every 16 steps. They differ in how far they follow it before holding: a quarter turn for Wide, three quarters for the Bounces, without limit for the Folds. Triangle, Hard Bounce and Sawtooth use the same rate with their own wave. The level keeps the slope near 1 at low Intensity, so one formula covers the whole dial.
- Wave Shaper is a sine whose rate climbs 18 dB across Intensity.
- Rectifier crossfades from the signal to a folded form of it, and has no delay.
- Bit Reducer and its older form round to a step. The newer step doubles every 12 steps of Intensity up to 64 and every 24 above it.
- Treble Booster tilts the signal ahead of the shaper, cutting lows and lifting highs; High Cut is a two-pole low pass after it, its corner an octave lower every 21 steps.
- The Rate Reducers are a sample and hold. The older hold doubles every 23 steps, the newer every 12. Neither reducer takes Treble Booster or High Cut, and neither goes through the delay and high-pass.
- Low Pass and High Pass are first-order filters with the corner moved by Intensity.
- At a low Intensity, where the signal has not reached the curve, some curves run straight through.
- The Overdrives are the Wide curve between filters: a first-order low cut and a two-pole top in front, then a level, ramped in over the first 48 steps of Intensity, then one pole after it, 350 Hz or 550 Hz, except Curry. The pole shapes the buzz at high drive, so it cannot sit ahead of the curve, though at Intensity 0 the response is the same. Measured against a saw: a near-sine leaves a fit with almost nothing to hold it.
- The Rate Reducers pass no DC: a hold landing on the same point of each cycle gives silence, not a constant.

## 3. Phaser
Stages n is 2n first-order allpass sections, so n notches once mixed with the dry signal; Mix is an equal-power crossfade, all wet at full. The corner is 393 Hz at Frequency 48 and 2.3 times higher every 16 steps. Feedback goes as the 1.5th power of its amount. The LFO swings the corner, up to 1.7 octaves each way. Spread is approximate: 5 times across the chain at full and none up to 96, which fits the two ends but not the middle.

## 4. Chorus
A delay line read at a moving point, the two sides in antiphase, Mix an equal-power crossfade. Delay sets the base time, 0.156 ms a step, and Depth the movement, up to half the base either way; at Delay 0 nothing is left to hear. Vintage, Hyper, Air, Vibrato and Rotary use the same for now.

## 5. Character
Analog Boost is in tiVoices (§16). Vintage 1-3, Pad Opener, Lead Enhancer and Bass Enhancer are fixed tone curves that Intensity and Tune do not change, built as octave bands; Vintage 3 is flat. Speaker Cabinet is one fourth-order filter: a pole pair fixed near 84 Hz and a Butterworth pair that Tune moves, over a numerator with zeros at DC and Nyquist that is a straight line in Intensity, so Intensity crossfades between two filters. Stereo Widener leaves the middle alone: left gains a side signal and right loses it. The side is the middle through a sixth-order filter, its level a straight line in Intensity, its poles and zeros following Tune.

## 6. Filter bank
Before Distortion, as a clipped signal after a low pass shows.

- Ring Modulator multiplies by a sine. Frequency Shifter uses a phase splitter, so Shape L and Shape R move each side from one sideband through both to the other. Both run at 12 kHz times 2^(-3(64 - d)/16), d being the distance of Frequency from 64.
- Vowel is two formants in parallel, sharing one level. Their places were measured every 16 steps of Frequency and the dial wraps round to where it began. Q is 0.5 times 2^(res/24.2).
- Comb is a delay tuned as a note, Frequency plus 12, Stereo Phase spreading the two sides around it. Feedback is a straight line in Resonance.
- 1, 2, 4 and 6 Pole XFade and the VariSlopes are bilinear state-variable sections. Cutoff is in semitones and Q is 0.487 times 2^(res/16). At no resonance the 1 Pole type is low pass plus half the band output, which is exactly first order, and the 6 Pole is a Butterworth. Shape runs low, band, high, equal-power between neighbours; resonance sits on the first section.
- VariSlope band pass moves through one to three sections. Low and high pass move from two poles towards three, and are the least exact of these.
- Mix is an equal-power crossfade, except on Comb, which keeps its dry path, and Ring, which leans towards dry.

Filter bank values are measured on a mono signal at 48 kHz.

## 7. Off means off
An effect that is off does no work: Distortion at Mix 0 or Off, the Filter Bank at Mix 0, the Widener at Intensity 0, the EQ's flat bands. When one goes off its delay lines and filters are cleared, so switching it on again plays nothing stale. The voices do the same (tiVoices §7).

## 8. Delay
A step of Delay Time is 256 samples at 48 kHz. Send leaves the dry signal alone up to 96 and then takes it to nothing at 127, while the wet rises to 0.92. Feedback is a straight line, reaching just under 1. Ping Pong and the Patterns read one line from two places, the left tap feeding it back, so both sides repeat at the left's period: Ping Pong sets the right to a ratio of the left, and Patterns count sixteenths of the tempo for each side. Their sixteenth measures about 8% short of the tempo's, at 120 BPM only. Color, modulation, Tape delay and where Delay sits against the EQ are not measured.

The plug-in has global Delay and Reverb switches, outside the sound, so a DAW's effects can take over. Off means the dry signal, untouched, whatever Send says.

All seventeen patterns (2026-11-09): Pattern x+y is x sixteenths on the left and y on the right, modes 6-22 in the
instrument's list order. Only the first six (1+1 to 2+3) had been built; the eleven after them (2+5 to 5+5) played 1+1,
which is why A006 (Pattern 5+3) sounded nothing like the instrument.

## 9. Reverb
A tank of tapped delays and allpasses in one ring of 16382 words, run at half the sample rate, each Type its own list of read offsets. These lists are the one table here: their entries must sum exactly for the read points to keep their places, so scaling them does not work. Two values drive it: one from Time, scaled by the room, and one from Damping; the other coefficients follow them with simple limits. Send follows Delay's law, and Predelay is 1.875 ms a step. Color is a one-pole tilt on the way in, low pass one way and high pass the other. The tank's slow modulation and the Feedback modes are not built.

## 10. Sample rates
Nothing is resampled: the engine runs at the host's rate. The tables stay as they are and values are scaled to hold their hertz and seconds, as checked at 44.1 kHz against 48 kHz. The Reverb tank counts samples, so above 50 kHz it runs at a quarter of the rate to keep its decay. Speaker Cabinet and Stereo Widener were measured at 48 kHz, so their coefficients are moved to the running rate by an allpass substitution in z^-1, which keeps the response in hertz.

## 11. Delay Clock
A Delay Clock division replaces the Time dial. It is a fraction of a whole note at the host's tempo, scaled by 0.9816 as measured at 90, 132 and 160 BPM. A time longer than 32768 samples at 48 kHz is halved until it fits, so 1/2 can play as 1/4. The sequencer patterns use the same tempo law. At 60, 120 and 200 BPM the reference's times wander from this by up to 9%, which is left out.

## 12. Distortion High Pass
A bilinear one pole: the corner is 711 Hz at Amount 64 and six times higher for every 32 steps, up to 22.6 kHz. Measured at 48 kHz, and held in proportion to the rate.

## 13. Chorus
A Single older than format 12 has no Chorus Type: Mix alone turns the chorus on, and the type byte holds nothing useful.

The delay is 8 samples per step of Delay at 48 kHz, and swings down from there, reaching nothing at full Depth: d x (1 - Depth x (1 - wave) / 2), with the two sides mirror images. Rate is half an LFO's at the same dial value from the middle of the dial up, and measured below that. Sine and triangle start at the top; the saw falls and jumps half way; the square starts low.

The chorus's delay and depth glide toward their values over 10 ms, every sample (2026-11-09). Modulation reaches
the effects by rewriting their bytes once per block (tiVoices notes §22), and a step of Chorus Delay is 8 samples of
delay time, so a modulated chorus jumped at every block boundary - a fast zipper on a held note (A053), worse at
larger host blocks. Reading the line between samples cannot hide a jump in the delay itself.

## 14. Phaser sweep
The sweep is a triangle, straight in octaves, starting at the top. It rises from the Frequency by 0.0465 octave per step of Depth, and its rate is the Chorus's. The right side stands 0.15 octave above the left at all times. Spread sets how far apart the stages lie, not the stereo. Feedback is 0.89 of the dial either side of centre, and is taken from the second section of the chain from the end, or the eighth at most.

## 15. Reset
`ti_effects_reset` empties every line and filter, as a new instance has them, and keeps the rate, the tempo and the plug-in's Delay and Reverb switches. The plug-in calls it on the audio thread when a sound loads (tiPlugin §12).

## 25. Modulated effects glide
Modulation reaches the effects by rewriting their bytes once per block (tiVoices notes §22). The phaser (mix, depth,
frequency, feedback, spread), the delay (send, feedback, both taps) and the reverb (send, pre-delay, the colour's
filters, the tank's coefficients) glide toward those values over 10 ms, every sample, as the chorus does (§13); the
delay's taps and the reverb's pre-delay are read between samples, so a moving time slides. A value starts on its
target, and again whenever its effect is switched off, so a sound that does not modulate them is unchanged; 13 of 20
Bank A Singles render identically, the rest differ only where a clocked delay time falls between samples, which had
been rounded down (2026-11-09).

## 25a. Delay Colour
A bilinear one-pole on what goes into the line (the input with the feedback), so every repeat is coloured again: a low
pass below 64, a high pass above, nothing at 64. The corner sits on the digital filters' third-octave points (19.7 Hz
x 2^(p/3)): with x = 2 (0.75 Colour / 128 - 0.375), p = 31 (x + 1) for the low pass and 31 x for the high pass, half a
point added (fitted at 44.1 kHz) - about 133 Hz at 0, 1.9 kHz at 32, 320 Hz at 96 and 4.4 kHz at 127. Against reference
renders, octave bands of a saw's echo against Colour 64: within about 1 dB from 125 Hz to 8 kHz at 0, 32 and 96 (at 0
the reference levels off near -29 dB above 4 kHz where ours keeps falling); at 127 the lowest band is 9 dB short.

