#!/usr/bin/python3
# ti_smooth_session.py's takes: the energy above 1.5 kHz (dB, a 2 ms rms) through the Cutoff jump - a jump of tens of
# dB, which the sound's own beating cannot hide - its 10-90% rise in dB, and a plot of all four.  usage: ti_smooth_analyse.py <dir>
import glob, os, sys

import numpy as np

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from spdif_bits import words


def centroid_track(x, r, frame=256, hop=48):
    w = np.hanning(frame)
    f = np.fft.rfftfreq(frame, 1 / r)
    t, c, lv = [], [], []
    for i in range(0, len(x) - frame, hop):
        s = np.abs(np.fft.rfft(x[i:i + frame] * w)) ** 2
        t.append((i + frame / 2) / r)
        c.append(np.sum(f * s) / max(1e-30, np.sum(s)))
        lv.append(np.sum(s))
    return np.array(t), np.array(c), np.array(lv)


def hf_track(x, r, hz=1500.0, smooth_s=0.002):
    sp = np.fft.rfft(x)
    sp[np.fft.rfftfreq(len(x), 1 / r) < hz] = 0
    h = np.fft.irfft(sp, len(x))
    n = max(1, int(smooth_s * r))
    e = np.sqrt(np.convolve(h ** 2, np.ones(n) / n, 'same'))
    return np.arange(len(x)) / r, 20 * np.log10(e + 1e-12)


def main():
    d = sys.argv[1]
    import matplotlib
    matplotlib.use('Agg')
    import matplotlib.pyplot as plt
    fig, ax = plt.subplots(figsize=(9, 3.6))
    for path in sorted(glob.glob(os.path.join(d, 'smooth*.wav'))):
        v, _, r = words(path)
        x = (v[:, 0] & ~1).astype(float) / (1 << 23)        # bit 0 is the sign, not audio (findings 2026-11-06)
        t, e = hf_track(x, r)
        on = t[np.argmax(np.abs(x) > 0.01)]
        before = np.median(e[(t > on + 0.4) & (t < on + 0.9)])
        after = np.median(e[(t > on + 1.3) & (t < on + 1.8)])
        win = (t > on + 0.9) & (t < on + 1.3)
        frac = (e[win] - before) / (after - before)
        tw = t[win]
        t10 = tw[np.argmax(frac > 0.1)]
        t90 = tw[np.argmax(frac > 0.9)]
        print('%-18s above 1.5 kHz %6.1f -> %6.1f dB, 10-90%% in %5.1f ms' % (os.path.basename(path), before, after, (t90 - t10) * 1000))
        sel = (t > t10 - 0.03) & (t < t10 + 0.12)
        ax.plot((t[sel] - t10) * 1000, (e[sel] - before) / (after - before), label=os.path.basename(path)[:-4], lw=1.2)
    ax.set_xlabel('ms from the 10% point'); ax.set_ylabel('above 1.5 kHz, 0 = before, 1 = after (dB scale)')
    ax.legend(fontsize=8); ax.grid(alpha=0.3); fig.tight_layout()
    fig.savefig(os.path.join(d, 'smooth.png'), dpi=110)
    print('plot', os.path.join(d, 'smooth.png'))


if __name__ == '__main__':
    main()
