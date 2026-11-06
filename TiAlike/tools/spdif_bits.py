#!/usr/bin/python3
# The bits of a digital capture (S/PDIF or USB audio from the Virus): the raw integer words of each channel - how
# many distinct values, the range, each bit's share of ones (24-bit words: bit 0 the least significant), the lowest
# bit in use, and the low bits' run pattern. A healthy stream either holds still in silence or dithers every low bit
# at about half; a stuck bit sits at 0 or 1 whatever the signal does.
# usage: spdif_bits.py <wav> [<wav> ...]
import sys, wave

import numpy as np


def words(path):
    w = wave.open(path)
    ch, n, width = w.getnchannels(), w.getnframes(), w.getsampwidth()
    raw = np.frombuffer(w.readframes(n), dtype=np.uint8)
    if width == 3:
        b = raw.reshape(-1, 3).astype(np.int32)
        v = b[:, 0] | (b[:, 1] << 8) | (b[:, 2] << 16)
        v = np.where(v >= (1 << 23), v - (1 << 24), v)
        return v.reshape(-1, ch), 24, w.getframerate()
    if width == 4:
        v = np.frombuffer(raw.tobytes(), '<i4').reshape(-1, ch)
        return v >> 8, 24, w.getframerate()                    # 32-bit words: the top 24 bits are the audio
    return np.frombuffer(raw.tobytes(), '<i2').reshape(-1, ch).astype(np.int32), 16, w.getframerate()


for path in sys.argv[1:]:
    v, bits, rate = words(path)
    print(path.split('/')[-1], '- %d frames, %d-bit words, %d Hz' % (len(v), bits, rate))
    for c in range(v.shape[1]):
        x = v[:, c]
        u = x & ((1 << bits) - 1)
        ones = [(u >> b & 1).mean() for b in range(bits)]
        nz = np.flatnonzero(np.array(ones) > 0)
        lowest = int(nz[0]) if len(nz) else -1
        peak = np.max(np.abs(x)) / float(1 << (bits - 1))
        print('  ch %d: %d distinct values, range %d..%d (peak %.1f dBFS), lowest bit in use %d' % (
            c, len(np.unique(x)), x.min(), x.max(), 20 * np.log10(peak) if peak > 0 else -np.inf, lowest))
        print('        share of ones, bit 0..%d: %s' % (min(11, bits - 1), ' '.join('%.2f' % o for o in ones[:12])))
        if len(np.unique(x)) < 12:
            vals, counts = np.unique(x, return_counts=True)
            print('        values: %s' % ', '.join('%d x%d' % (a, b) for a, b in zip(vals, counts)))
