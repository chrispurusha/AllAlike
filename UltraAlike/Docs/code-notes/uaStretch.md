# uaStretch.cpp

## 1. What it is
The one file that speaks to Rubber Band (SynthLib `ThirdParty/rubberband`, v4.0.0, GPL v2 or later - `THIRD_PARTY.md`),
behind a C function so the rest stays C. It renders a whole stretched copy at once, on the cache's thread: never on
the audio thread, which it would stall for a tenth of a second or more. The cache's thread asks it to stop between
blocks when the instance closes.

## 2. Settings
Offline, which is two passes (study, then process) over material that is all there - a one-shot always is. The R3
engine (`OptionEngineFiner`): smoother on tones than R2 and it keeps transients. Measured 2026-11-11 on a 1.5 s stereo
sample: 55-120 ms for x0.5 to x4, 25-50 times faster than the output's own length (a 6-minute file: 11-33 s).

## 3. Rates and ratios
The stretcher sees only the file's rate. Its time ratio is the frames asked for over the frames given, so the output,
played at the host's rate, lasts what was asked; its pitch scale is the wanted pitch times the file's rate over the
host's, which is what the rate change itself would otherwise shift. The ratio is met to within a few frames; any
shortfall is silence at the end. Checked (`tools/uaCacheTest.c`): a 440 Hz tone at x0.5, x2 and x4 comes out exactly
that long at 440.0 Hz, and +12 st at x2 is 880.0 Hz, still 2 s.
