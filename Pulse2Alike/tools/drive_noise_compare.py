#!/usr/bin/python3
# The engine against drive_noise_session.py's takes: each take's sound (the edit buffer read after the session, with
# its filter and drive bytes set per take) rendered by tools/p2render, then the held part's level and its octave
# bands compared - and the wave's shape, which for noise is its amplitude distribution: the crest factor (the
# 99.9th percentile of |x| over the rms), the skew, and the positive peak over the negative one. A clipper shows
# as a lower crest; an asymmetric one as skew and unequal peaks.
# usage: drive_noise_compare.py <takes dir> <edit-buffer .syx> <work dir>
import os, subprocess, sys

import numpy as np

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from osc_analyse import read

HERE = os.path.dirname(os.path.abspath(__file__))
FILTERS = {'lp24open': (0, 127, 0), 'bp12p030': (2, 53, 83)}
POINTS = [(0, 0)] + [(c, d) for c in (1, 2) for d in (0, 32, 64, 127)]
BANDS = [125, 250, 500, 1000, 2000, 4000, 8000, 16000]
HOLD = (0.4, 1.8)            # seconds after the onset


def onset(x, r):
    e = np.abs(x)
    return int(np.argmax(e > 0.05 * e.max()))


def held(x, r):
    a = onset(x, r)
    return x[a + int(HOLD[0] * r):a + int(HOLD[1] * r)]


def bands(s, r):
    sp = np.abs(np.fft.rfft(s * np.hanning(len(s)))) ** 2
    f = np.fft.rfftfreq(len(s), 1 / r)
    return np.array([10 * np.log10(sp[(f >= b / np.sqrt(2)) & (f < b * np.sqrt(2))].sum() + 1e-30) for b in BANDS])


def shape(s):
    s = s - np.mean(s)
    rms = np.sqrt(np.mean(s ** 2))
    return (np.percentile(np.abs(s), 99.9) / rms, np.mean(s ** 3) / rms ** 3,
            np.percentile(s, 99.9) / -np.percentile(s, 0.1))


def main():
    takes, syx, work = sys.argv[1:4]
    base = bytearray(open(syx, 'rb').read())
    os.makedirs(work, exist_ok=True)
    print('%-9s %-5s %5s  %7s %7s %6s   %-17s %-17s %-17s  band diffs engine-instrument dB (%s Hz)' % ('filter', 'curve', 'drive', 'hw dBFS', 'eng', 'diff',
          'crest hw/eng', 'skew hw/eng', '+pk/-pk hw/eng', ','.join(str(b) for b in BANDS)))
    for fname, (vtype, cutoff, res) in FILTERS.items():
        for curve, drive in POINTS:
            d = bytearray(base)
            for i, v in ((42, cutoff), (43, res), (45, vtype), (48, drive), (49, curve)):
                d[7 + i] = v
            d[135] = sum(d[1:135]) & 0x7F
            sp = os.path.join(work, '%s_c%d_d%d.syx' % (fname, curve, drive))
            open(sp, 'wb').write(d)
            ep = sp.replace('.syx', '_p2.wav')
            subprocess.run([os.path.join(HERE, 'p2render'), sp, '48', '2.0', '0.6', ep, '192000'], check=True, stdout=subprocess.DEVNULL)
            h, rh = read(os.path.join(takes, '%s_c%d_d%d.wav' % (fname, curve, drive)))
            e, re = read(ep)
            hs, es = held(h[:, 0], rh), held(e[:, 0] * 10.0, re)        # p2render's gain of 0.1 restored
            lh, le = 20 * np.log10(np.sqrt(np.mean(hs ** 2))), 20 * np.log10(np.sqrt(np.mean(es ** 2)))
            bd = (bands(es, re) - 10 * np.log10(len(es))) - (bands(hs, rh) - 10 * np.log10(len(hs)))
            sh, se = shape(hs), shape(es)
            print('%-9s %-5s %5d  %7.1f %7.1f %+6.1f   %5.2f / %5.2f     %+5.2f / %+5.2f     %5.2f / %5.2f      %s' % (
                fname, ['NONE', 'TUBE', 'FUZZ'][curve], drive, lh, le, le - lh, sh[0], se[0], sh[1], se[1], sh[2], se[2],
                ' '.join('%+5.1f' % v for v in bd)))


if __name__ == '__main__':
    main()
