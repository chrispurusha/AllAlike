# sn2Voices.c - the voices

## 1. A voice
Oscillator, the two filters and the amplifier envelope (sn2Envelope.md), per voice. The oscillator is still a
placeholder; the filter envelope, modulation and the rest come section by section. A voice is free once its amplifier
envelope has finished its release.

## 2. The voice limit
The instance's voice limit (design §1), 1 to 32. A note takes the voice already playing its key (Same Note Piano, §21), else a free
voice, else the oldest. Lowering the limit releases the voices above it rather than cutting them.

## 3. The sound
`tSn2Sound` is the patch as the voices play it, in the panel's units; the plug-in builds it from its parameters each
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
In Percussive trigger mode every oscillator starts each note at the same phase, so identical oscillators add. Ensemble lets them run on. Each voice's oscillators first stand at scattered phases, as on an instrument whose voices never stop running. The oscillator mix reaches the Overdrive saturator at a half, fitted from its gain at four settings, and the saturator's output reaches the filter at a half again, which sets where level-dependent damping starts. The filter's output is scaled so a wide-open Saw at Level 127 and Volume 127 matches the reference.

## 9. Tracking
A voice's cutoff moves (note - 60) x Tracking dial steps, so Tracking 127 follows the keyboard one to one around middle C. The coefficients are worked out per voice for that reason.

## 10. Velocity
Env Velocity 64 ignores velocity. Above it the level blends towards a curve of 0.016 + 0.98 v cubed, below it towards the inverse, in proportion to the distance from 64.

## 11. Noise and ring
Noise is the instrument's own generator (a counter multiplying a word, not quite white) through one pole with a corner
c = 2^n (1 + f) / 256, Soften being 16 (n + f), less c times what the pole held; the step towards the generator is held to full scale, which dulls the top at high Soften. Ring is twice the product of Osc 3 and Osc 1 or 2, whatever their levels.

## 12. Glide
A note starts away from its pitch and, Linear, arrives in a time set by Glide Time alone, whatever the interval: 0.183 s at 64 (the exact law is §12c). Normal starts from the last note played, Auto likewise but only over a held note, and the fixed types start that many semitones away on every note. Linear is a straight line in pitch. Exponential moves by a fixed step and a part of the gap, so it lands in a finite time (§12c). Glissando is the same path rounded to semitones. Time 0 is no glide. It applies to all voices.

## 13. Mono
Polyphony 1 in a program. One voice and a list of the keys down, last note priority: a released key returns to the newest still held. Measured: value 34 of the trigger byte, and 35-37, restart the envelope on each change including the return; 32, 33 and 38 do not. Glide Auto counts a key still held on the one voice.

## 14. Volume and the output
A program's Volume is part of the sound. Gain is Volume/84, straight to 127 (the instrument 2026-11-22: +3.6 dB from 84 to 127; an earlier note that 127 plays as 84 was wrong). The output saturates hard at 0.97675 of full scale: flat tops, with no gain reduction, whatever the voice count, with the effects on or bypassed.

## 15. LFOs
Two per voice, stepped every sub-block. Rate 2.5 Hz x (Speed/64)^2, Range x1, x10, x100. Square, rising saw from -1, triangle rising from 0 and sample and hold, all +-1; Offset adds Offset/127 of the amount, and the Delay fades the offset in with the wave (measured through a level route: with Offset 127 and Delay 66 the whole output rises from 0 over about 0.4 s). Key restarts the phase with a note; Free runs on, through silence too - on the instrument (2026-11-22, notes 0.53 s apart with no voice sounding between them) each note met the LFO somewhere new. An earlier model restarted a free LFO whenever a note found no voice sounding; the instrument does not. Delay is a straight fade in: 0.125 s at 40, 0.34 s at 64, 1.33 s at 90. Pitch and cutoff go as amount^2/57.6 semitones, width in step with the amount, one unit in the panel's 64 a unit. Measured.
Fade Out, in place of the fade in: from full, an exponential fall after 12.5 ms, time constant the fade in's time over 5.7. A square at Speed 0 this way is a pitch or filter blip at the note's start.

LFO Soften smooths the LFO's output, offset and fade included: each step it moves a part of the way, 256 x Soften's word x
the LFO's own step (the whole way with Soften 0), so a slow LFO is smoothed for longer. Within 0.8 dB of FIXED on cutoff.

## 16. Pan
After the effects, in sn2Effects.c. A share of 0.5 + (64 - Pan)/128 + 0.5 Depth/127 sin, left 2 x share, right 2 x (1 - share), a sine at Speed^2/642.5 Hz. Bypass leaves it out. Pan Effects takes the wet signal with it. Measured. Read a panned sound by its sum: one channel of an auto-pan looks peaky, and misled a kurtosis comparison.

## 12a. Where a glide starts (instrument, by ear, 2026-11-02)
Glide Normal on the instrument: a key pressed while another is held glides (the reference does not; the instrument
decides). Going back to the held key glides from the pitch the first glide had reached, not from the key let go; a
new note after all keys are up glides from whatever pitch was last sounding - Normal against Auto, which glides only
legato. So a glide starts from the pitch the voice had reached. In poly the instrument seems to glide each voice from
that voice's own last pitch (hard to follow by ear); a voice that has never played starts from the last note.

## 12b. Auto in poly (2026-11-22)
Auto holds a glide back only in mono, where the key before must still be down. A poly program glides on every note with
Auto as with Normal (A016, against the instrument by ear; it was legato only here, so A016 did not glide).

## 12c. One law for both (2026-11-22)
A glide moves every four samples: Linear by a step set from its interval, Exponential by a step and a part of what is left
(`sn2_fixed_glide`, the words the fixed path plays). FLOAT had fitted times; it now follows FIXED within 0.004 semitones on slow
glides, and starts about 0.3 ms before it on fast ones. The share drops its fraction at each step, half a step on average, which is taken
off. The glide's place is moved on whichever maths plays the voice: in FIXED it stood still until this date, so a third
note began its glide where the one before had begun.

## 17. Anti-alias
The GLOBAL page's Anti-alias, On by default (owner, 2026-11-12), as on the other Alikes. Off plays the oscillators as
the instrument makes them at its own rate, aliases included. On runs the same law at four times the rate - the same
edge, the stretch still taken from the note's own increment, so the tone is the instrument's - and brings it back down
through a 48-tap windowed-sinc low-pass at 0.45 of the engine's rate. A restarted wave clears that filter. Against the
Off path, a lone saw: the harmonics identical, the energy between them down 7-22 dB, to -63 to -84 dB from C5 to G7;
Soften 64, where the instrument aliases most (-47 to -56 dB), comes down to -63 to -73. It costs four oscillator
steps and a 48-tap sum per oscillator a sample. Stored 0 = Off, 1 = On.

## 18. FM
The FM 1x3, FM 2x3 and FM Noise switches each turn a mixer slot into frequency modulation of Osc 3: Osc 1, Osc 2 or
the noise modulates it, at the slot's own level (1x3, 2x3, Noise) as the depth, and that slot is then not heard as ring
or noise. The modulator is the oscillator's own output before its mix level (Osc 1's mix makes no difference); Osc 3 is
heard through its own mix level as usual.

Linear and through zero: each sample Osc 3's phase step moves by step x (level^2 / 256) x modulator, by no more than half
a cycle's half either way. The modulator is the oscillator's output, or the noise's pole. Exact against FIXED (2026-11-22)
to 0.2 dB up to level 60. With Anti-alias on, Osc 3's sub-steps take Osc 1's and Osc 2's own.

## 19. Mixer level routes
Every mixer slot - Osc 1-3, ring 1x3 and 2x3, noise - has five routes onto its level, each bipolar about 64: the
wheel, Env 2, Env 3, LFO 1 and LFO 2 (program bytes 124 + 6 x slot + 1..5, the slot's level at + 0). The law is the
wheel's: level + 2 x (route - 64) x source, in the slot's 0..127, clamped. Measured with Env 2 held at its top on
Osc 1: route 72 plays as level 16, 96 as 63, 127 as 124, and 32 over a level of 64 silences it. An LFO counts half here:
+-0.5, plus 0.5 x Offset/127, so Offset 127 makes it 0..1 - measured at Speed 0 and on a moving saw and triangle.
A moving square through a level route measured louder than full level on the reference; not understood, not modelled. Env 2 and 3 are scaled by their own Velocity, as on the cutoff (§9) - it brought the Open High Hats from silence to
close. The levels are worked out once a sub-block. The FM depths (§18) take the moved levels.
Many programs play only through these - all levels 0, the sound coming in on Env 2 or an LFO.

## 20. Filter Oscs Bypass
The program's switch (ext byte 3, bit 5): Osc 1-3 and the ring products go around the filter and its Overdrive and join
its output at 0.771 of the level they would have through a fully open filter. Noise alone goes through the filter, at
twice its usual level.

## 21. The same key again (Same Note)
A plug-in setting, not part of a sound. Piano, the default and what the plug-in has always done, gives a key struck
again the voice already on it - held, pedal-held or releasing - and restarts it as any new note (its oscillators from
zero unless Osc Free, §8). Stack lets that voice go (unless held by a one-shot envelope) and takes a free voice, else
the oldest, so each strike rings on; with a sustain pedal and a long release that piles up voices, which is what Piano
is for. What the instrument itself does with a repeated key is not yet checked.

## 22. Envelope to pitch
Env 2 and Env 3 move each oscillator's pitch from the program (bytes 29 and 30 for Osc 1, 34 on for each oscillator),
with no dials yet: amount a = depth - 64, a x |a| / 28.8 semitones at a full envelope - twice the LFOs' law (§15), as
an LFO counts half - times the envelope's level and its velocity setting. Checked 2026-11-22 against the instrument's
arithmetic (Maths FIXED): steady offsets within 0.1 semitone up and down, and Env 3 decays tracked step for step. Past
about 45 semitones the instrument's pitch word runs out of range (FIXED wraps; ours does not) - only extreme settings
reach it.

## 23. Routes to Soften, Resonance and pulse width
From the program. Noise Soften moves 2 dial steps for each step of a route's amount from 64 at a full source; Soften and Resonance each move 131072/66048 for each step of a route's amount from 64, at a
full source: the wheel, Env 2, Env 3, LFO 1, LFO 2 (an LFO half, as §19). Pulse width moves a 32nd of its 0..2 for each
step from the wheel, Env 2 and Env 3 (Osc 1's wheel an eighth of that), a 64th from an LFO.

## 24. Unison
The program's switch (byte 238, bit 5) and count (bits 2-4, the voices less 2): 2 to 8 voices to a key. They are numbered
from 0; voice w of n is off by (8w + 5 - 4n) 256 / (8n + 16), plus 1, times Detune (extension byte 2) / 256, whole cents: even
steps about the pitch, a little sharp of it. Every voice is (16 - n) / 16 of its level. Mono moves them together. The
program's own limit is the plug-in's Voices. A voice to steal is the oldest already let go, else the oldest; a key never
takes a voice it has just taken. Against the instrument 2026-11-22, 2 to 8 voices at five settings: every voice's pitch
within half a cent, the level within 0.6 dB.
A glide starts where the voice's pitch was before the new note (until 2026-11-22 the note was set first, and nothing
glided); its rate was checked on the instrument in both curves.
