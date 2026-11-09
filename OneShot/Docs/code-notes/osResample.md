# osResample.c

## 1. Why
A sample plays at its file's rate times the pitch ratio, so every output sample falls between the file's frames. The
first version read them by 4-point Hermite, which measured badly (2026-11-09, `alias` test: a 44.1 kHz sine played
into 48 kHz): at no transposition a 10 kHz tone left an image at -27 dB and an 18 kHz tone one at -11 dB, and 18 kHz
lost 3 dB; up an octave, 15 kHz aliased to 18 kHz at -1.7 dB. Hats and cymbals live up there.

## 2. The kernel
A windowed sinc, 32 taps each side, Kaiser window beta 8.6 (about 87 dB stopband), tabulated at 512 points a tap and
read with linear interpolation between them. At full bandwidth it passes to about 20 kHz of a 44.1 kHz file and puts
its images above the output's Nyquist far down. Measured after the change: every unwanted component at the test's own
floor (-88.6 dB, the analysis window), 15 kHz up an octave at -116 dB, 18 kHz at full level.

## 3. Playing faster than the file's rate
The kernel widens by the step (cutoff 1/step), so content that would land above the output's Nyquist is removed
instead of folding back. It widens no further than 4x (two octaves up, 256 taps); past that aliasing returns. The note
runs on for the kernel's half-width after the last frame, so the filter's ring is not cut off.

## 4. The untouched case
At full bandwidth on a whole frame (the file's rate equals the host's and nothing is transposed) the kernel is 1 at
that frame and 0 at every other, so the frame itself is returned: playback is the file, sample for sample.

## Cost
64 multiply-adds per channel per output sample at full bandwidth, 64 x step when faster; 8 stereo voices at 48 kHz
is about 50 million a second.
