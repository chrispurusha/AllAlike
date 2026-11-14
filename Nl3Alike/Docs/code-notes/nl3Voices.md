# nl3Voices.c - the voices

## 1. A voice
Oscillator, the two filters and the amplifier envelope (nl3Envelope.md), per voice. The oscillator is still a
placeholder; the filter envelope, modulation and the rest come section by section. A voice is free once its amplifier
envelope has finished its release.

## 2. The voice limit
The instance's voice limit (design §1), 1 to 32. A note takes the voice already playing its key, else a free
voice, else the oldest. Lowering the limit releases the voices above it rather than cutting them.

## 3. The sound
`tNl3Sound` is the patch as the voices play it, in the panel's units; the plug-in builds it from its parameters each
block. A loaded Single will fill the same struct.

## 4. Filter coefficients every 32 samples
The filter's coefficients are worked out every 32 samples and shared by every voice: nothing per voice moves the
cutoff yet. Every 32 rather than once a host block so that a glide (§5) is not stepped by a host's block size, which
can be 1024 samples. Keyfollow and the filter envelope will make them per voice.

## 5. Smoothing a control change
The reference glides a changed Cutoff or Resonance to its new value in a straight line over a fixed time, whatever
the size of the change: 2048 samples at 46875 Hz, 43.7 ms (measured on the Init Single, whose Parameter Smooth Mode
is On, against Off, where the change is immediate - findings.md). Here both glide that way; a new value starts a new
glide from wherever the last one had got to. The other Smooth modes (Auto, Note, the quantised ones) are not built.

## 6. The two filters: routing, Balance, Link
Filter 2 has the digital modes only (low, high, band pass, band stop). Balance runs through a centre-detented curve,
bal = -1, -0.6875, -0.375, -0.125, 0, then the mirror up to +1, at dial 0, 16, 32 ... 112, 127 and straight between
(the top two dial steps stretched to reach +1). With F1, F2 the filters, x the oscillator, and g2 a gain on Filter 2:

- **Serial 4.** bal < 0: out = g2 F2(F1(x)) + 0.707 |bal| F1(x), g2 = 0.707 (1 - |bal|) - at Balance 0 Filter 1 alone at
  0.707, and Filter 2 fades in towards the centre. bal = 0: out = 0.707 F2(F1(x)). bal > 0: out = 0.707 F2(u), with
  Filter 2 fed u = (1 - bal) F1(x) + bal x - at Balance 127 Filter 2 alone on the oscillator.
- **Parallel 4.** Both filters on the oscillator: out = g2 F2(x) + m F1(x); bal < 0: m = 0.3535 + 0.1465 |bal|,
  g2 = 0.3535 (1 - |bal|); bal >= 0: m = 0.3535 (1 - bal), g2 = 0.3535 + 0.1465 bal. Each 0.3535 at the centre, 0.5
  alone at either end.
- **Serial 6 and Split** are not yet their own: Serial 6 runs as Serial 4 (its Filter 1 should be four-pole), Split as
  Parallel 4 with its own levels (0.707 at the centre) summed to mono.
- **Close cutoffs.** In the serial routings, when Filter 2's cutoff is within 2.13 dial steps of Filter 1's (4.27 in
  Serial 6), Filter 2's resonance is taken down: res2 x (1 - 0.375 (1 - apart/limit) res1/128) - to 0.625 x res2 at
  full Filter 1 resonance and the same cutoff. Its output gain stays that of the resonance before the cut (band pass
  excepted, which takes the cut one).
- **Link.** On (the default), Filter 2's cutoff is Filter 1's plus Cutoff 2 through a second curve: -1, -0.5, -0.25,
  -0.0625, 0 ... +1 of the whole dial range at the same nine points; off, Cutoff 2 is Filter 2's cutoff itself.

Cutoff 2, Resonance 2 and Balance glide as §5. Checked against reference renders (saw at C1, harmonics against the
open filter at Balance 0): Serial 4 at Balance 0, 1, 32, 64, 96, 127, with Filter 2 offset in high and band pass, and
at close cutoffs with resonance; Parallel 4 at Balance 0, 32, 64, 127; Link off - all within 0.3 dB rms below 3 kHz,
most within 0.05.


## 7. Velocity
The level does not follow velocity: the Init Single's velocity to volume is 0, and with it the reference is the same at
velocity 1 and 127. The patch's own velocity amounts come with the modulation.

## 8. Level and pitch
In Percussive trigger mode every oscillator starts each note at the same phase, so identical oscillators add. Ensemble lets them run on. The oscillator mix reaches the Overdrive saturator at a half, fitted from its gain at four settings, and the saturator's output reaches the filter at a half again, which sets where level-dependent damping starts. The saturator is Sn2Alike's placeholder, made exactly linear at Filter Distortion 0 (it kept 1.6% of its curve there, which put a third harmonic at -63 dB on a pure sine): the Nord Lead 3's path is clean until its distortion is used. The filter's output is scaled so a wide-open Saw at Level 127 and Volume 127 matches the reference.

## 9. Tracking
Keyboard Track is a switch. On, a voice's cutoff moves one semitone per note from C2 (note 36), so the Cutoff dial sets
the cutoff C2 plays at: measured, notes 36, 48, 60, 72 and 84 at +0, +12, +24, +36 and +48 semitones exactly. Off,
the note does not move the cutoff. The coefficients are worked out per voice for that reason.

## 10. Velocity
Env Velocity 64 ignores velocity. Above it the level blends towards a curve of 0.016 + 0.98 v cubed, below it towards the inverse, in proportion to the distance from 64.

## 11. Noise and ring
At Soften 127 noise is the difference of successive white samples, rising 6 dB an octave, over a floor 27 dB down; lower, one pole of low pass whose corner doubles every 15 steps. The two have not been joined: the change at Soften 120 is a guess. Ring is twice the product of Osc 3 and Osc 1 or 2, whatever their levels.

## 12. Glide
A note starts away from its pitch and arrives in a time set by Glide Time alone, whatever the interval: 0.183 s at 64, doubling every 11.2 steps. Normal starts from the last note played, Auto likewise but only over a held note, and the fixed types start that many semitones away on every note. Linear is a straight line in pitch. Exponential closes a gap at (gap + 2.5) / 0.41T semitones a second, so it lands in a finite time. Glissando is the same path rounded to semitones. Time 0 is no glide. It applies to all voices.

## 13. Mono
Polyphony 1 in a program. One voice and a list of the keys down, last note priority: a released key returns to the newest still held. Measured: value 34 of the trigger byte, and 35-37, restart the envelope on each change including the return; 32, 33 and 38 do not. Glide Auto counts a key still held on the one voice.

## 14. Volume and the output
A program's Volume is part of the sound. Measured: gain is Volume/84, and 127 plays as 84, not as the loudest; the Nord Lead 3 gives 18.6 dB between 9 and 127, against 19.4 from this law. The output saturates hard at 0.97675 of full scale: flat tops, with no gain reduction, whatever the voice count, with the effects on or bypassed.

## 15. LFOs
Two per voice, stepped every sub-block. Rate 2.5 Hz x (Speed/64)^2, Range x1, x10, x100. Square, rising saw from -1, triangle rising from 0 and sample and hold, all +-1; Offset adds Offset/127 of the amount. Key restarts the phase with a note, Free runs on. Delay is a straight fade in: 0.125 s at 40, 0.34 s at 64, 1.33 s at 90. Pitch and cutoff go as amount^2/57.6 semitones, width in step with the amount, one unit in the panel's 64 a unit. Measured.
Fade Out, in place of the fade in: from full, an exponential fall after 12.5 ms, time constant the fade in's time over 5.7. A square at Speed 0 this way is a pitch or filter blip at the note's start.

## 16. Pan
After the effects, in nl3Effects.c. A share of 0.5 + (64 - Pan)/128 + 0.5 Depth/127 sin, left 2 x share, right 2 x (1 - share), a sine at Speed^2/642.5 Hz. Bypass leaves it out. Pan Effects takes the wet signal with it. Measured. Read a panned sound by its sum: one channel of an auto-pan looks peaky, and misled a kurtosis comparison.

## 12a. Where a glide starts (instrument, by ear, 2026-11-02)
Glide Normal on the instrument: a key pressed while another is held glides (the reference does not; the instrument
decides). Going back to the held key glides from the pitch the first glide had reached, not from the key let go; a
new note after all keys are up glides from whatever pitch was last sounding - Normal against Auto, which glides only
legato. So a glide starts from the pitch the voice had reached. In poly the instrument seems to glide each voice from
that voice's own last pitch (hard to follow by ear); a voice that has never played starts from the last note.

## 19. Arithmetic and the filter's level
The Arithmetic parameter chooses, per instance, between the instrument's fixed-point arithmetic and floating point for
the stages built both ways (so far the filter, `nl3Filter.md` §3). It is an instance setting, not part of a sound.
The filter leaves a quarter of its input at DC on every mode, as the instrument's does, so the voice scales its output
by 4 to keep the level it had. Where the oscillators stand on the instrument's full scale is not measured yet; the
filter's non-linear damping depends on it.

## 20. Anti-alias
The Anti-alias switch, per instance like Arithmetic, On by default (owner, 2026-11-12): On uses the longer correction
of `nl3Osc.md` §7; Off plays the oscillators band-limited as the instrument does, its aliases included. Stored as
0 = Off, 1 = On, so a project saved before the rename (0 = Instrument, 1 = Clean) sounds as it did. The plug-in sets
the engine from the default when it starts, before the host sends anything. The Saw takes its phase advanced
before it is read, as the instrument's does; the placeholder shapes still read before advancing.


## 21. The LFOs: rate, waves and the pitch amount (instrument, 2026-11-14)
Each voice's LFO steps once every two samples (22050 times a second) by a phase word of 2^24 a cycle, the word being
round(22.493 x 2^(Rate/9)): 0.0296 Hz (one cycle in 33.8 s) at 0, a ninth of an octave a step, 523 Hz at 127 - the
manual's range, rounded as measured. On its phase p (-1..1) the waves are: Saw -p, falling; Triangle
2|p| - 1, starting at its top; Square +1 while p > 0, else -1; Random a new held value at each wrap.
To an oscillator's pitch a full swing is Amount^3 / 16384 semitones, 127 counting as 128: 0.03 at 8, 2 at 32, 16 at
64, 54 at 96, 128 at the top - measured, the peak pitch offset at each Amount with a fast saw, within the sampled
saw's own shortfall of its peak. Invert turns the sign.
Not yet the instrument's: Smooth Random and Special (a table the instrument reads), where a keyed LFO starts, the
Env (single cycle) mode, Mono, and the amounts to the other destinations (filter, shape, Osc Mod, Osc Mix, resonance,
amplitude, pan), which are still the interim scales.

## 22. The LFOs' other destinations (instrument, 2026-11-14)
The Destination positions, in order: Osc 1, Osc 2 and both (pitch), Filter 1, Filter 2 and both, Shape 1, Shape 2 and
both, Osc Mod, then Osc Mix (LFO 1) or Resonance (LFO 2), Amp (LFO 1) or Pan (LFO 2). Measured with a square LFO,
which sits at +1 or -1 on every sample, against the knob each destination belongs to:
- Pitch: both ways, cubic (§21).
- Filter: upward only - +Amount semitones at the wave's top, none at its bottom (Amount 16 moved the cutoff as 16
  steps of its own knob, 32 as 32, 64 as 64), clamped where the knob runs out.
- Shape and Osc Mod: upward only, an Amount step a step of the knob, clamped at the top.
- Osc Mix: both ways, an Amount step a step of the knob.
- Amp: downward only - the level times 1 - (Amount/64) at the wave's bottom: silence at 64, and past it the signal
  turns over, as the manual says it may.
So a positive-only destination follows (wave + 1)/2. Not yet the instrument's: LFO 2's Resonance and Pan, what Invert
does to a positive-only destination, and Osc Mod's effect (Osc Mod itself is not built).

## 23. The Filter envelope's Amount (instrument, 2026-11-14)
At full envelope the cutoff rises Amount semitones - a step of its own Freq knob for every step of Amount (measured
against the knob at Amounts 0 to 127 in steps of 8, within 0.25 of a step), and in proportion below full. The Mod
envelope to the filter is taken to follow the same law. With the Filter envelope's Velocity on, the Amount is scaled
by velocity/128 - a straight line (Amount 64: 0.5 semitones at velocity 1, 32 at 64, 50 at 100, 63.5 at 127). The
amplifier itself takes no velocity: its level is the same at every velocity (velocity reaches it through the
velocity morph). Keyboard Track adds a semitone a note above C2 (note 36), measured at C2 to C6.

## 24. The Mod envelope's destinations (instrument, 2026-11-14)
Its twelve positions are the LFOs' to Osc Mod, then Osc Mix and LFO 1's Amount. To pitch it follows the LFOs' cube
exactly (Amount 16: 0.25 semitones at the top of the envelope, 32: 2, 64: 16, 96: 54); to the filter a semitone an
Amount step (§23); Shape and Osc Mix are taken as the LFOs' (§22). Not yet built: LFO 1's Amount, Osc Mod; and at
Amount 127 the instrument's pitch offset reads zero rather than 128 semitones - not reproduced.

## 25. The morph groups (instrument, 2026-11-14)
Each group adds its range times its source to the knob's own value, in steps of that knob, before the knob's law is
applied - so a morphed cutoff moves a semitone a step, a morphed attack along the envelope law - and groups on the
same knob add. Measured on Filter 1 Freq, against the knob itself:
- Wheel: wheel/128, 127 counting as the whole range (+64 range: 16 steps at wheel 32, 64 at 127).
- Velocity: velocity/128 (+64 range: 0.5 at velocity 1, 32 at 64, 63.5 at 127).
- Keyboard: (note - 36)/48 - nothing at C2, the whole range at C6 (the instrument's four octaves), and on beyond
  either end (1.25 at C7).
The move is not rounded to a step. Each sounding voice gets its own sound with its morphs (Velocity and Keyboard differ
from voice to voice), built once a block; a knob is held to its own range. Aftertouch (and the pedal it can be
swapped for) has no source yet.

## 26. Unison (instrument, 2026-11-14)
Each voice's output goes into a 256-sample ring and is read back twice at fractional delays, through a 6-tap
interpolation (sinc x Hann over three samples a side). Each read's delay comes from a 24-bit word: 128 - word x 244/2²⁴
samples (the word taken signed), so as the word climbs the delay shrinks from 250 samples to 6 and then jumps back - a
read sweeping through the ring is a pitch shift, upward. With Unison on, the first word steps every sample by
k²/8 - 0.08k + 7.45 (k the Detune dial; within 1.5 of every setting measured) and the second by 0.7 of that: at 64,
+12.8 and +9.0 cents; at 127, +49.5 and +34.8. Left is the second word's copies with the voice, right the first's (§27: two copies a side); with
Unison off the voice goes to both sides undelayed. The jump at each wrap is not smoothed - it is the instrument's, and
shows as faint sidebands. The voices are now stereo for it; each starts its reads somewhere of its own.
Not yet the instrument's: the interpolation kernel's small correction term, and where the reads start on a new note.

## 27. Unison: a mirrored pair each side (measured on the instrument, 2026-11-14)
Captured with Unison at 40 and 81 on one saw (Init) and on String Pad and Happy ending, and compared harmonic by
harmonic: each side carries two copies, not one - one read ahead of 128 samples by the word's offset and one behind
by as much, so one goes sharp while the other goes as far flat. Seen as sidebands on both sides of every harmonic, at
equal strength in every quarter second, their pattern line for line that of a mirrored ramp (not a triangle) and the
copies' power 3 dB above the voice's. A side is (voice + copy ahead + copy behind)/2: Unison on is 1.2 dB quieter
than off on the instrument, as here. The second word's (0.7) pair is on the left, the first's on the right, as §26.
With one copy a side (§26 as first built) the copy sat sharp of the voice and beat against it, about 10 dB deeper
than the instrument at the fundamental; switching Unison off on both made the beating match, which is how it was found.

## 28. Voice stealing
With every voice busy, a note takes the quietest voice already let go, and a held one (the oldest) only when none
is. Long releases keep voices busy for a long time - String Pad's (103) for half a minute - so this is common. A note
on a key still sounding restarts that voice, its envelopes from where they are; whether the instrument does the
same is not yet settled (the beating hid it in the one comparison made).
