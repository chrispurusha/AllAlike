# uaParams.c

## 1. Volume
-96 to +10 dB in 1 dB steps, as the E5000's; -96 is silence, 0 dB the default.

## 2. Note names
Middle C (60) is C3, as Ableton names it.

## 3. Envelope defaults
Chosen so a newly loaded sample sounds as it is (but 6 dB down, from Volume): AHR, Attack 0, Hold Inf, Decay 645 ms (dial 64), Sustain 100%, Release 58 ms
(dial 35), curves 0 / +50 / +50. The curve dials have 201 steps so +50 is a step of its own.

## 4. Greyed controls
`ua_param_active()`: Decay, its curve and Sustain only in AHDSR; Release and its curve always in AHR (the release follows
the hold) and in AHDSR only in Gate mode, since One Shot ignores the note off that would start it; Root only with Keys
On. A greyed control is drawn grey and ignores clicks; a host can still automate it, and it still does nothing.

## 6. Stretch
Stretch is 2^(k/100) for k from -200 to +200: x0.25 to x4, a hundred steps an octave, x1 in the middle. Base BPM is
40 to 240 in half-BPM steps, default 120. Protect has Start's law (0 to 2 s, exponential above 1 ms) and defaults to
5 ms - step 236, 5.01 ms. Base is greyed with Sync Off, Protect and its Snap when nothing is stretched. Protect's Snap is its own switch, apart
from TRIM's (owner, 2026-11-11), appended after Delay, default On. Appended after Reverse, so
older projects keep their settings and open unstretched.

## 7. End
0 to 100% of the sample's length in tenths of a percent, default 100%. The host sees a percentage - the parameter's text
has no sample to measure - and the panel a time (uaDraw §12).

## 8. Delay
Start's law (0 to 2 s, exponential above 1 ms): its first steps are microseconds, for lining up layers. Appended after
End, with End, so older projects open with neither.

## 9. Layers
Every parameter here exists once a layer. Layer n's id is n x 256 + the id in the table (`UA_LAYERS`,
`UA_LAYER_STRIDE`), so layer 1's ids are what a one-layer project saved, and it opens as layer 1 with the rest at
their defaults. The host sees 4 x 30, named "1 Volume" to "4 Volume" and so on; the table is built once, when the
plug-in is first asked for its description. 256 leaves room to append parameters to every layer at once.

## 10. Low and High, Mute
Mute (Off / On) is appended after High; the panel draws it under the tabs, not with the controls (uaDraw §15).

Each layer's keys, MIDI 0 to 127 named as Root is (C3 = 60), default the whole keyboard. A note plays the layer when
Low <= note <= High; High below Low takes no notes.

## 11. Filter and saturation
Filter On, Low Cut and High Cut (20 Hz x 1000^v - three decades in 1000 steps, about 0.7% each; defaults 20 Hz and
20 kHz), Slope Soft / 12 dB / 24 dB (default 12 dB; uaFx §5). Saturation On, Mode (Classic, Drums, Tape), Gain 0 to +30 dB and Output -30 to +6 dB in
half-dB steps, Warmth -12 to +12 dB and HF 1 to 20 kHz (Tape only, greyed otherwise). All appended, all per layer.

## 12. What the panel shows
PITCH, PLAY (Mode and Voices), OUTPUT and the layer ENVELOPE. OneShot's trim (Delay, Start, End, Snap), Reverse, the
envelope curves, Stretch, its filter and saturation came across with the skeleton and mean nothing to an E-mu
sampler, so they are off the panel and hidden from hosts (2026-11-14); they stay in the parameter list at their
defaults, where they do nothing, and the engine code that serves them is untouched. The Z-plane filters will take the
filter's place.

## 13. Global
Anti-alias, On/Off, default On (clean): Off plays a bank's notes through the E5000's own interpolation, images and
aliasing included (uaEmuInterp). Polarity, Normal/Inverted, default Normal: the E5000's main outputs are inverted
(measured 2026-11-16; its sub outputs are not), which owners often correct, so Normal is the corrected sound and
Inverted the main outputs as they come. Both appended at the end, so older projects open with the defaults. A
section of their own on the panel, as GLOBAL.

## 14. The E5000's controls (2026-11-16)
The panel is the E5000's: PITCH - Transpose (+/-24), Coarse (+/-36), Fine (+/-64, 1/64 semitone each, as E-mu's),
Non-Transpose, Root (a loose sample's), Low and High keys; PLAY - Solo with the E5000's nine modes; OUTPUT - Volume,
Pan, Velocity; AMP ENV RATE and LEVEL - the six stages and Depth (uaEosEnv); GLOBAL. Root greys while Non-Transpose
is on. OneShot's Mode, Voices, envelope type, Attack/Hold/Decay/Sustain/Release and curves are hidden and do nothing;
Coarse, Fine, Volume and Non-Transpose reuse old ids with new ranges - projects from before do not carry over.

## 15. Sustain Pedal
Hidden, not saved, driven by CC 64; 0-127 as the controller sends it.
