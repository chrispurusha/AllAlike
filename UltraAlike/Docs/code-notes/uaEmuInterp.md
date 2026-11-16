# uaEmuInterp.c

The E5000 Ultra's own sample interpolation, played when Anti-alias is Off. Measured on the owner's E5000 Ultra
(2026-11-16): test samples sent to it over MIDI Sample Dump, played from a flattened preset at many pitches, captured
at 192 kHz (Fireface) and 48 kHz (QU-24).

## 1. The kernels
Seven fixed kernels, each a sinc at `cutoff` x the source's Nyquist under a Kaiser window (`halfLength` frames,
`beta`), all in SOURCE frames - unlike uaResample's, which stretches with the pitch. Fitted per kernel to the main
tone's level across the band (median error 0.2-0.75 dB, checked on notes held out of the fit) and the images'
levels (within ~4 dB for 0-2, 7-14 dB for 3-6). Kernel 0, the one every note at or below +3 semitones uses, is flat
to 20 kHz, -2 dB at 22.05 kHz, -14 dB at 29 kHz and below -55 dB from 39 kHz (source frequency): a slow roll-off
just above Nyquist, so images of 15-20 kHz content pass at -4..-14 dB when a note is pitched down. That, and the
aliasing of notes pitched up, is the instrument's top-end "air".

## 2. Which kernel
By the playback ratio against a 44.1 kHz engine: 0 below 1.25, then one more at every 0.5 up to 6 from 3.75. Measured
as a step in the response between adjacent semitones either side of each ratio, and none within a step. A note's
kernel is fixed when it starts. Each kernel cuts lower than the last but never far enough to stop aliasing: x1.5
aliases above 14.7 kHz after ~6 dB, x8 folds 4 kHz content after ~7 dB.

The plug-in runs at the host's rate, not 44.1 kHz: the kernels act on the sample the same, but what aliases folds
at the host's Nyquist, so at 48 kHz and above it differs slightly from the instrument's.

## 3. Weights
The taps within halfLength of the position, the kernel read from a table of 256 points a frame with a straight line
between, the weights then divided by their sum so a constant passes at exactly unity - the measurements are all
relative to the passband, and a fitted kernel's sum ripples a little with the phase.
