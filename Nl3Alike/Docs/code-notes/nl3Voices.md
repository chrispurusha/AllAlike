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
The oscillators run free: no note sets them back to a starting phase, in poly or mono (measured 2026-11-16: two sines a fine step apart, note after note - each note starts at a different point of their beat, 0.06 to 0.97 of full level; set back, every note would start in phase). An earlier version set them back at every note, a rule carried over from another instrument's Percussive mode that the Nord Lead 3 does not have; it ticked wherever a sounding voice was restarted. Each voice's oscillators start from phases of their own. The oscillator mix reaches the Overdrive saturator at a half and the filter at 2/3 in all (§19). The saturator is Sn2Alike's placeholder, made exactly linear at Filter Distortion 0 (it kept 1.6% of its curve there, which put a third harmonic at -63 dB on a pure sine): the Nord Lead 3's path is clean until its distortion is used. The filter's output is scaled so a wide-open Saw at Level 127 and Volume 127 matches the reference.

## 9. Tracking
Keyboard Track is a switch. On, a voice's cutoff moves one semitone per note from C2 (note 36), so the Cutoff dial sets
the cutoff C2 plays at: measured, notes 36, 48, 60, 72 and 84 at +0, +12, +24, +36 and +48 semitones exactly. Off,
the note does not move the cutoff. The coefficients are worked out per voice for that reason.

## 10. Velocity
Env Velocity 64 ignores velocity. Above it the level blends towards a curve of 0.016 + 0.98 v cubed, below it towards the inverse, in proportion to the distance from 64.

## 11. Noise and ring
At Soften 127 noise is the difference of successive white samples, rising 6 dB an octave, over a floor 27 dB down; lower, one pole of low pass whose corner doubles every 15 steps. The two have not been joined: the change at Soften 120 is a guess. Ring is twice the product of Osc 3 and Osc 1 or 2, whatever their levels.

## 12. Glide
A constant rate, as the instrument's manual says: a straight line in semitones, so a wider interval takes longer.
Rate 0 is no glide; Rate 1 is about 580 semitones a second, 32 about 255, 64 about 98, 96 about 36 and 127 about 1.9 -
measured on the instrument's arithmetic at all 127 settings (2026-11-20: a legato C3 to C6, every write of Osc 1's
pitch word logged; the pitch moves in equal steps about every 4.3 ms, alternating single and double), smoothed by at
most 3% and held as a table, since no closed form fits both ends (low down it falls about a third every 16 steps; the
top 15 settings fall in a straight line towards zero). Glide Mode: 0 Off, 1 On, 2 Auto - Auto glides only over a held
key, checked on the instrument (1:9 Early lead, Mode 2: detached notes jump, legato ones glide 12 semitones in 60 ms
at Rate 32). Normal starts from the last note played; the fixed types start that many semitones away. It applies to
all voices. Replaces the first version's fixed time (0.183 s at 64, an exponential approach), from the Sn2Alike skeleton.

## 13. Mono
Polyphony 1 in a program. One voice and a list of the keys down, last note priority: a released key returns to the newest still held. Measured: value 34 of the trigger byte, and 35-37, restart the envelope on each change including the return; 32, 33 and 38 do not. Glide Auto counts a key still held on the one voice.

## 14. Volume and the output
A program's Volume is part of the sound. Measured (2026-11-16, 1:5 with its level morph off, Volume 23/46/92/115/126/127, peaks under 0.14 of full scale): gain is the square of Volume, within 0.2 dB everywhere, 127 included - halving Volume costs 12 dB. NL3_VOLUME_UNITY (91.2) keeps Volume 100 exactly as loud as the earlier law did. That law (Volume/84, 127 playing as 84) came from one 9-against-127 reading and was wrong. The output saturates hard at 0.97675 of full scale: flat tops, with no gain reduction, whatever the voice count, with the effects on or bypassed.

## 15. LFOs
Two per voice, stepped every sub-block. Rate 2.5 Hz x (Speed/64)^2, Range x1, x10, x100. Square, rising saw from -1, triangle rising from 0 and sample and hold, all +-1; Offset adds Offset/127 of the amount. Key restarts the phase with a note, Free runs on. Delay is a straight fade in: 0.125 s at 40, 0.34 s at 64, 1.33 s at 90. Pitch and cutoff go as amount^2/57.6 semitones, width in step with the amount, one unit in the panel's 64 a unit. Measured.
Fade Out, in place of the fade in: from full, an exponential fall after 12.5 ms, time constant the fade in's time over 5.7. A square at Speed 0 this way is a pitch or filter blip at the note's start.

## 16. Pan
After the effects, in nl3Effects.c. A share of 0.5 + (64 - Pan)/128 + 0.5 Depth/127 sin, left 2 x share, right 2 x (1 - share), a sine at Speed^2/642.5 Hz. Bypass leaves it out. Pan Effects takes the wet signal with it. Measured. Read a panned sound by its sum: one channel of an auto-pan looks peaky, and misled a kurtosis comparison.

## 12a. Where a glide starts (instrument, by ear, 2026-11-02)
Glide Normal on the instrument: a key pressed while another is held glides (the reference does not; the instrument
decides). Going back to the held key glides from the pitch the first glide had reached, not from the key let go; a
new note after all keys are up glides from whatever pitch was last sounding - Normal against Auto, which glides only
legato. So a glide starts from the pitch the voice had reached.
In poly (Mono off) there is no glide from note to note, Legato on or off (instrument, by ear, 2026-11-17), although the
manual describes each voice gliding from its own last note. The fixed-interval glide types are untouched in poly; not
checked on the instrument. Poly Legato, with its parallel glide (manual p.108), is not implemented; as a stopgap, a
poly note with Legato on glides (from that voice's own last pitch) while another key is held, and its envelopes still
restart.

## 19. Arithmetic and the filter's level
The Arithmetic parameter chooses, per instance, between the instrument's fixed-point arithmetic and floating point for
the stages built both ways (so far the filter, `nl3Filter.md` §3). It is an instance setting, not part of a sound.
The filter leaves a quarter of its input at DC on every mode, as the instrument's does. The oscillator mix reaches the
filter at 2/3 of itself (DRIVE_IN x FILTER_IN): traced in the reference (2026-11-21), the filter's input for a
full-level saw at Init peaks at 1/3, and the ratio to ours was the same for saw and square (x2.68 when ours took the
mix at 1/4) and held across Osc Mix 0, 64 and 127 (x2.60-2.67) - a scale, not a shape. The output is scaled by
1.5 = 4 x (1/4) / (2/3), so the level heard is unchanged; what changes is how hard the level-dependent parts work: the
Dist LP shaper (filter notes §17), and the stages' non-linear damping. Until then ours drove Dist LP about 8 dB
softer than the instrument, so 1:8, 1:14, 1:17 and 1:18 were darker (owner, 2026-11-21, confirmed by captures: at
Distortion 50 and 97 the instrument sounded as ours did 18 and 13 settings higher, at 32 and 127 the same).

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

Destination LFO 1 adds the Mod Env's Amount to LFO 1's, knob step for knob step, and LFO 1 then does whatever its
Amount does at its own destination (the laws are one function, nl3_lfo_targets, for both). Measured on the
instrument: SawLead (1:21, LFO 1 Amount 0 to Osc 1+2, Mod Env AR Amount 14 to LFO 1) fades a vibrato in to +-15
cents at 5.5 Hz in under a second, +-3/11/15 cents at 0.35/0.6/0.85 s; Nl3Alike now does the same to within a cent.
It did nothing before, in the 56 programs that use it.

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

## 28. Which voice a note takes
After every note event (a key down or up, a key the pedal holds included) the voices are ranked, and the notes until
the next event take them in that order:
1. the voices whose key is up - silent or releasing - quietest first (the amp envelope's level, to 16 bits), the
   lowest number on a tie;
2. then, if every voice left is held, the oldest held voice, never the one playing the lowest key held;
3. then whatever is left.
The ranking uses the levels as they were at the last event, not when the next note comes: a voice let go stays near
the end of the order until another event ranks it again, however quiet it has become meanwhile. Measured on the
instrument's own voice placement (notes run through it with the voice each one reached recorded): short notes in a
row alternate between voices 0 and 1, between 1 and 2 with a key held on 0; notes with a long release take 0, 1, 2,
3; with all 24 held, the 25th note takes voice 1 when voice 0 holds the lowest key. Nl3Alike placed all of these
runs identically. Before this a note took the first silent voice, else the quietest released, else the oldest held.

## 29. Every voice its own LFO
A free-running LFO (not keyed) runs in each voice at that voice's own rate, so a morph that moves the rate with the
key gives each note its own speed. Measured on the instrument with String Pad's keyboard morph (Rate +8, Amount +117)
on one oscillator, C2 to C6: the LFO quickens 2.22 → 2.67 → 3.11 → 3.56 → 4.00 Hz, a straight line in hertz ending at
Rate 64's speed, while its depth pins the width at its limit from about C5 up (movement 0.69, 0.47, 0.18, 0.00, 0.01;
the plug-in 0.70, 0.47, 0.19, 0.02, 0.01). Until 2026-11-14 one shared phase ran at the unmorphed rate, so every note
of String Pad swept its pulse width at 2.2 Hz - the "LFO beating" heard up the keyboard. A voice starting from
silence takes up the shared phase (`lfoFree`), so a chord's voices begin together. The LFO Mono mode (one LFO for
every voice) is not built.

## 30. Aftertouch reaches the morph
Channel pressure arrives as a hidden parameter mapped to the host's aftertouch (as Pulse2Alike's and TiAlike's) and
is the third morph group's source, taken with the wheel's law (pressure × 127/128, full pressure the whole range) -
not yet measured on the instrument. Before 2026-11-14 that group did nothing.

## 31. Vibrato
One sine for every voice, added to the pitch of oscillators 1 and 2. Its depth is a cent an Amount step up to 100,
taken as min(Amount, 100) × 127/128 cents (±29.6 at 30, ±59.3 at 60, ±89.3 at 90, ±99 at 127), times its source: the
pressure or the wheel, as value/128. With source Off there is none - not a fixed vibrato. The stored source values
are 1 for aftertouch and 2 for the wheel (the manual lists the wheel first); the panel's names follow the stored
order. Rate 3.98 Hz at 0 to 8.00 Hz at 127, measured at 0, 32, 64, 96 and 127 and drawn straight between, the
plug-in within 0.25% of every point. Measured on the instrument 2026-11-14 with the Init saw at A4, the pressure and
the wheel sent over MIDI.

## 32. Morph offsets per voice
A morph moves a voice's own sound (§25); its Filter Frequency 1, Resonance and Output Level offsets from the panel's
values are applied as they are. They glided over 12 ms for a while (2026-11-16, for Mod Wheel Sweep's stepping);
that went when the wheel itself was made to step as the instrument does (plugin notes §18), which smooths every
wheel target the same way. Output Level is a morph target like the rest, additive in knob steps and through the same
square law (§14): measured on 1:5, +46 at full wheel adds 12.0 dB, at half wheel 7.0.

## 33. Octave Shift
A program's Octave Shift (-2..+2) moves every note by whole octaves: measured on 1:5 (Octave Shift -1), C4 sounds at
130.8 Hz on the instrument. It was read from the program and never applied, so 354 of the 1024 programs played in the
wrong octave. It is added to the note for pitch and for the filter's Keyboard Track (assumed - the track with a shift
not yet measured); the keyboard morph still takes the key played. Transpose is 48 in every program and not applied.

## 34. The same key again
A key played again while its last note still sounds takes another voice; the old one is let go (if it was still
held) and releases to its end beside the new one. Restarting the old voice instead - the first version - set its
oscillators back to their starting phase mid-cycle, a tick at every repeated note of a program with a long release
(1:4), which the instrument does not make (owner, 2026-11-16). A run of notes rendered through 1:4 now starts every
note without a step. Which voice it takes is §28's order.
Same Note Piano (a plug-in setting, not the instrument's; default Stack, which is the above) gives the key its newest
sounding voice back instead - held, pedal-held or releasing - so a sustain pedal does not pile up a voice for every
strike. Its envelopes restart from where they are and its oscillators run on, as in mono (§35), so there is no tick;
another voice still left on the key from before is let go. No sounding voice on the key: §28's order as usual.

## 35. Mono: one voice, its oscillators running on
Mono is one voice: every note goes to the same one, a key played again included, and so does the note a released
key falls back to. Each note restarts the voice's envelopes; its oscillators run on (§8) - measured on the instrument (1:4 with Mono on,
C4 played again in its release): the fundamental's phase runs straight through the new note, where setting it back
made a jump of about 3 radians and a tick.
Poly takes each note's voice in §28's order, the same key included (§34).

## 36. Osc Mix
Osc Mix is x = value / 128 (127 is all the way, 1.0), and the two oscillators' gains are 1 - x^2 and 2x - x^2 -
each 2.5 dB down at the middle, not 6 dB as a straight crossfade gives. Measured on the instrument: a saw on Osc 1
and a sine a fifth up on Osc 2, Osc Mix swept under a held note, each oscillator's level read off the spectrum - Osc 1
-0.6 / -2.5 / -7.2 / -12.7 dB at 32 / 64 / 96 / 112, the sine its mirror, matching 1 - x^2 to a tenth of a dB. LFO
and Mod Env modulation of Osc Mix move x before the gains are formed; Osc Mix Noise 3 gives the second gain to the
noise in place of Osc 2. The first version crossfaded straight, which left programs balancing both oscillators up
to 3.5 dB quiet against the instrument (owner, 2026-11-16).

## 37. LFO Env mode
Env/KB Sync 1 is Env, 2 KB Sync (the manual: Shift + Waveform once for single-cycle, twice for keyboard sync). Both
restart the LFO on a key; Env then runs one cycle and holds where the cycle ends, as an envelope. Seen on the
instrument's arithmetic with 1:15 Open Air (LFO 2, Saw, inverted, Amount 41, to Filter 1, 2026-11-20): Filter 1's
cutoff rises steadily in semitones from the note, about 10.5 a second, then stays put from about 3.8 s - where ours
looped, dropping back each cycle. Ours takes 4.2 s for the same rise (Rate 27, 0.237 Hz), about 8% longer: the rate
law at Rate 27, or where Env mode's cycle starts, is still to check.

## 38. Osc Bypass
Osc 2 goes round the filter and is added to its output; with Noise3 on, the noise goes through the filter alone and
both oscillators go round it (the manual's rule). Osc Mod then works on Osc 1 alone when Osc 2 is bypassed, as the
instrument's code leaves Osc 2 out of what it hands Osc Mod. What goes round is added at a quarter of the filter's
word, after the filter's own output (filter notes §3) - checked against the instrument's arithmetic (2026-11-20):
a sine on Osc 2 with Bypass on and the filter closed comes out at the level of the same sine through the open filter
with Bypass off, on both (0.0 dB).

## 39. LFO and Mod Env to Filter 2 and Osc Mod
Destination 4 is Filter 2's cutoff and 5 both cutoffs, for the LFOs and the Mod Env alike, by the same law as Filter
1 (Amount knob steps at the top, rising from 0 - the private measurements of the LFO's square at each destination);
with Filter 2 Env on, Filter 2 already follows Filter 1's modulation, so "both" is not added twice. Destination 9,
Osc Mod, adds up to the LFO's Amount in steps to the Osc Mod Amount, before its square (osc notes §17-18), as the Mod
Env's OSCMOD does. Filter 2 counts only in the multi modes. Not yet compared with the instrument's sound.

## 40. LFO Invert
On the one-way destinations (Filter, Shape, Osc Mod - up from where the knob is - and Amp, down), Invert flips the
wave and keeps it one way: the modulation still runs from 0 to the Amount, the wave upside down. On the both-ways
ones (pitch, Osc Mix) flipping and negating are the same. Found by blame on 1:15 Open Air (2026-11-20; LFO 2, an
inverted Saw in Env mode, Amount 41, to Filter 1): on the instrument the cutoff rises from the knob to about +39 and
holds; Invert as a negative amount had it start 41 semitones below the knob instead - the filter far more closed and
9-13 dB quiet against the instrument's arithmetic. With the wave flipped the blame score fell from 11.5 to 2.7. Mod Env
Invert is still a negative amount - to check the same way.

## 41. Mod Env AR measured from the top
In AR mode the Mod Env reaches the both-ways destinations (pitch, Osc Mix) as Amount x (level - 1): nothing while the key is held at the top,
the destination bent below on the way up and again on release. Seen on the instrument's arithmetic with 1:14 ElecGit
(2026-11-20; Mod Env AR, Attack 33, Amount 63, to Osc 1's pitch): Osc 1 starts about 13 semitones under the note,
reaches it as the attack ends, holds in tune, and falls away on release; in AD mode the same settings rise 14
semitones over the note and decay back. Ours held +15 semitones for the whole note. Blame score 19.2 -> 14.3.
On the one-way destinations (filter, Shape, Osc Mod) AR is the plain level - up to the Amount, held there while the
key is down, back to nothing on release: seen with the Mod Env on the filter (Amount 64, 2026-11-20), the cutoff
rising to knob + 64, holding, and falling after the key. Applying the pitch rule there first had AR do nothing.

## 42. LFO Env mode on the both-ways destinations
In Env mode the LFO runs its one cycle one way at every destination: on pitch and Osc Mix the wave's one-way form
(0 at its bottom, the Amount at its top), Invert taking the sign, so a saw runs from the Amount to 0 and ends in tune
(Invert: from minus the Amount up to 0). Free-running it swings both ways, +-Amount, as before. Seen on 1:14's LFO 1
(Saw, Env, Amount 26, both oscillators' pitch, 2026-11-20): 1.07 semitones to 0 in 45 ms, then in tune - with and
without Invert, of opposite sign - where ours swung +-1.07 and held at the far end. Blame score 14.3 -> 7.1.

## 43. Envelope Invert
The Filter Env's and Mod Env's Invert work as the LFO's (§40): on a one-way destination (filter, Shape, Osc Mod, LFO 1
Amount) the shape is flipped and still one way - Amount x (1 - level) - and on pitch and Osc Mix it is a sign. Seen on
the instrument's arithmetic (2026-11-20, Filter Env and Mod Env to the filter, Amount 64, decay 60, sustain 0): the
cutoff starts near the knob and rises to knob + 64, where Invert as a negative amount had it dip below and come back.

From round 2 until 2026-11-21 Invert was applied twice on the both-ways destinations - as a sign in the Mod Env's
pitch and Osc Mix amounts (nl3Params.c) and again in `env_both_ways()` - so the two cancelled and Invert did
nothing there. On the reference, Mod Env (Amount 96, Decay 60) to Osc 2 pitch starts 33.7 semitones above and
settles; inverted, 33.5 below and rises to the same pitch. 1:25 MilkyWay (inverted Mod Env on Osc 2 pitch), which
the owner heard with one tone out of tune: blame 11.8 -> 4.9.

## 44. Noise3
The noise that takes Osc 2's place in the mix: the generator's words (osc notes §15), each averaged with the one before
- nearly white, 2 dB down at 6-9 kHz and 4 dB at 9-13 kHz - 6.1 dB under a saw at the same Osc Mix. Measured against
the instrument's arithmetic (2026-11-20): half-octave bands within about 0.5 dB, the level matched. The first version
was a low-passed placeholder from the Sn2Alike skeleton, 15 dB quiet. Blame on 1:17 Icisematta: 21.8 -> 8.7 (then 1.6
with the multi modes, filter notes §20).

## 45. What a morph moves when the knob edits another field
A morph adds to the knob, and on the instrument some knobs edit a different field depending on the mode, so the morph
follows the knob. Osc Shape: the sync pitch with Sync on, the FM Amount in Dual Sine FM, the timbre in synched noise.
Osc 2 Coarse and Fine: Carrier and Modulator in Dual Sine (Coarse stays Coarse with Keyboard Track off), Noise
Frequency and Type in Noise. 1:28 DXexpress, whose wheel morphs Osc 2 Coarse on a Dual Sine Osc 2, went from 36.7 to
8.8 on the blame score (2026-11-21) once the morph reached the Carrier instead of a Coarse nothing reads.

## 46. LFO 2 to Resonance and Pan
LFO 2's destinations 10 and 11. Resonance takes the LFO one way, like the filter destinations, adding Amount steps to
the Resonance knob. Pan is both ways and linear: left x (1 + p), right x (1 - p), p = Amount/128 x the LFO. At Amount
32 and 64, with the LFO held at its top, the two sides measured +1.9/-2.5 dB and +3.5/-6.0 dB about the centre,
which is (1 +- p) to a tenth of a dB, and at 127 the far side went silent (2026-11-21).

## 47. Chord Memory
With Chord Memory on, a key starts one voice per stored chord note. The program holds a count (twice the number of
notes) and then one byte per note, the interval above the key in the byte's top 7 bits: 1:35 Tonewheel holds 0 and
24, an octave; 1:63 FifthPad 0, 7, 14. Every voice of a key carries it as `key`, so letting the key go releases the
whole chord and another key landing on one of its pitches is a voice of its own. The intervals arrive as hidden
parameters (Chord Count, Chord Note 1-12) loaded with the program. Odd counts (14 of the 194 programs using a chord)
are not understood: on the reference 3 played nothing and 5 played only the second note; here they round down.
Chords in Mono mode are not done. 1:35's blame score went from 25 to 13.4.
