#!/usr/bin/python3
# ti_smooth2_session.py's takes: the energy above 1.5 kHz in dB over exactly one period of C4 (so the wave's own
# ripple averages out), each take aligned on its jump (the steepest 2 ms change), Off against On per jump size;
# the overshoot's peak over the settled level, when it peaks, and when it is back within 1 dB. One plot per jump.
# usage: ti_smooth2_analyse.py <dir>
import glob, os, sys

import numpy as np

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from spdif_bits import words

C4 = 261.6256


def track(x, r, hz=1500.0):
    sp = np.fft.rfft(x)
    sp[np.fft.rfftfreq(len(x), 1 / r) < hz] = 0
    h = np.fft.irfft(sp, len(x))
    n = int(round(r / C4))
    return 20 * np.log10(np.sqrt(np.convolve(h ** 2, np.ones(n) / n, 'same')) + 1e-12)


def main():
    d = sys.argv[1]
    import matplotlib
    matplotlib.use('Agg')
    import matplotlib.pyplot as plt
    jumps = sorted({os.path.basename(p)[8:-4] for p in glob.glob(os.path.join(d, 'smooth*_*_*.wav'))})
    fig, axes = plt.subplots(1, len(jumps), figsize=(4.2 * len(jumps), 3.2), squeeze=False)
    for ax, j in zip(axes[0], jumps):
        for smooth in (0, 1):
            v, _, r = words(os.path.join(d, 'smooth%d_%s.wav' % (smooth, j)))
            x = (v[:, 0] & ~1).astype(float) / (1 << 23)
            e = track(x, r)
            t = np.arange(len(x)) / r
            on = t[np.argmax(np.abs(x) > 0.01)]
            win = np.flatnonzero((t > on + 0.8) & (t < on + 1.6))
            k = int(0.002 * r)
            step = win[np.argmax(np.abs(e[win + k] - e[win]))]
            before = np.median(e[step - int(0.3 * r):step - int(0.02 * r)])
            after = np.median(e[step + int(0.25 * r):step + int(0.6 * r)])
            seg = e[step:step + int(0.2 * r)] - after
            peak = seg.max() if after > before else seg.min()
            tp = np.argmax(seg) if after > before else np.argmin(seg)
            settle = np.flatnonzero(np.abs(seg) > 1.0)
            ts = (settle[-1] / r * 1000) if len(settle) else 0.0
            print('jump %-7s smooth %s: %6.1f -> %6.1f dB; overshoot %+5.1f dB at %5.1f ms; within 1 dB after %5.1f ms' % (
                j.replace('_', '->'), 'On ' if smooth else 'Off', before, after, peak, tp / r * 1000, ts))
            sel = slice(step - int(0.03 * r), step + int(0.15 * r))
            ax.plot((np.arange(len(e))[sel] - step) / r * 1000, e[sel] - after, label='Smooth %s' % ('On' if smooth else 'Off'), lw=1.2)
        ax.set_title('Cutoff %s' % j.replace('_', ' -> '), fontsize=9); ax.set_xlabel('ms from the jump'); ax.grid(alpha=0.3)
    axes[0][0].set_ylabel('above 1.5 kHz, dB re settled'); axes[0][0].legend(fontsize=8)
    fig.tight_layout(); fig.savefig(os.path.join(d, 'overshoot.png'), dpi=110)
    print('plot', os.path.join(d, 'overshoot.png'))


if __name__ == '__main__':
    main()
