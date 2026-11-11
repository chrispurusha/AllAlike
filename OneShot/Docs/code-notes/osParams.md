# osParams.c

## 1. Volume
0 is silence; 1-127 run from -57 to +6 dB in 0.5 dB steps, so 0 dB is step 115 and the default, -6 dB (CT 2026-11-09),
step 103 - each a step of its own. It was a square law on dial/100 at first, on which no step reached -6 dB.

## 2. Note names
Middle C (60) is C3, as Ableton names it.

## 3. Envelope defaults
Chosen so a newly loaded sample sounds as it is (but 6 dB down, from Volume): AHR, Attack 0, Hold Inf, Decay 645 ms (dial 64), Sustain 100%, Release 58 ms
(dial 35), curves 0 / +50 / +50. The curve dials have 201 steps so +50 is a step of its own.

## 4. Greyed controls
`os_param_active()`: Decay, its curve and Sustain only in ADSR; Release and its curve always in AHR (the release follows
the hold) and in ADSR only in Gate mode, since One Shot ignores the note off that would start it; Root only with Keys
On. A greyed control is drawn grey and ignores clicks; a host can still automate it, and it still does nothing.

## 6. Stretch
Stretch is 2^(k/100) for k from -200 to +200: x0.25 to x4, a hundred steps an octave, x1 in the middle. Base BPM is
40 to 240 in half-BPM steps, default 120. Protect has Start's law (0 to 2 s, exponential above 1 ms) and defaults to
5 ms - step 236, 5.01 ms. Base is greyed with Sync Off, Protect and its Snap when nothing is stretched. Protect's Snap is its own switch, apart
from TRIM's (owner, 2026-11-11), appended after Delay, default On. Appended after Reverse, so
older projects keep their settings and open unstretched.

## 7. End
0 to 100% of the sample's length in tenths of a percent, default 100%. The host sees a percentage - the parameter's text
has no sample to measure - and the panel a time (osDraw §12).

## 8. Delay
Start's law (0 to 2 s, exponential above 1 ms): its first steps are microseconds, for lining up layers. Appended after
End, with End, so older projects open with neither.
