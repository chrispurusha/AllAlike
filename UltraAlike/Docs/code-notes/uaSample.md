# uaSample.c

## 1. Formats
WAV: format 1 (PCM: 8-bit unsigned, 16/24/32 signed), 3 (float 32/64), and 0xFFFE (extensible), whose real
format is the first two bytes of its sub-format GUID. AIFF: big-endian PCM. AIFC: NONE, `sowt` (little-endian PCM),
`fl32`/`FL32`, `fl64`/`FL64`. Anything compressed is refused with its name. Checked 2026-11-09 against 402 samples
from commercial drum libraries (16/24-bit, float, mono/stereo, 32-48 kHz, big-endian AIFF): each played back sample-exact.
Ableton's factory-pack `.aif` files are deliberately unreadable (macOS's `afinfo` cannot open them either).

## 2. Size
The whole file is read into memory first, then converted to float; files over 1 GB are refused.

## 3. AIFF sample rate
An 80-bit IEEE extended: sign and 15-bit exponent, then a 64-bit mantissa with an explicit integer bit.

## 4. Truncated files
A chunk claiming more bytes than the file holds is read to the end of the file, so a file cut short still plays.

## 5. Channels
Files of more than two channels play their first two.

## 6. Zero crossings
`ua_zero_crossing` works on any two buffers, so Snap can look in a stretched copy too (uaVoice §9).
For Snap: from the Start (or End) frame outward, the first place either side where left + right changes sign; of the two
frames there, the one nearer zero. A frame that is exactly 0 is taken as it is. With none in reach, the frame itself.
