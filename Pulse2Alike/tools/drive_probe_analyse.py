#!/usr/bin/python3
# drive_imd_session.py's probe takes: the 7.3 kHz probe's amplitude through each cycle of the 250 Hz wave, against the
# 250 Hz wave's value at the drive's input (from the NONE take, the same signal without the drive), folded on the
# 250 Hz cycle: the drive curve's slope at each input, normalised to its largest. The 250 Hz wave's own harmonics sit
# beside the probe (29 x 250 = 7250 Hz), so each take's average 250 Hz cycle (finely folded) is subtracted first: what
# repeats every cycle goes, the probe (not a harmonic) and its modulation stay. Where it peaks is the curve's centre;
# off zero, a bias. The instrument's, and the engine's (as drive_imd_analyse.py). One plot per curve.
# usage: drive_probe_analyse.py <dir>
import os, sys

import numpy as np

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from drive_imd_analyse import engine
from osc_analyse import read, refine_f0

N = 64
STEP_S = 1.2
LEVELS = (-24, -18, -12)


def demod(x, r, f, ms=0.4):
    t = np.arange(len(x)) / r
    z = x * np.exp(-2j * np.pi * f * t)
    n = int(ms * 1e-3 * r)
    return np.abs(np.convolve(z, np.ones(n) / n, 'same')) * 2


def slow(x, r, hz=2000.0):
    sp = np.fft.rfft(x)
    sp[np.fft.rfftfreq(len(x), 1 / r) > hz] = 0
    return np.fft.irfft(sp, len(x))


def fold(v, r, f0, ref_phase):
    t = np.arange(len(v)) / r
    b = (((t * f0 - ref_phase) % 1.0) * N).astype(int)
    return np.bincount(b, v, N) / np.maximum(np.bincount(b, None, N), 1)


def without_cycle(x, r, f0, n=4096):
    t = np.arange(len(x)) / r
    b = (((t * f0) % 1.0) * n).astype(int)
    mean = np.bincount(b, x, n) / np.maximum(np.bincount(b, None, n), 1)
    return x - mean[b]


def phase(x, r, f0):
    t = np.arange(len(x)) / r
    return np.angle(np.sum(x * np.exp(-2j * np.pi * f0 * t))) / (2 * np.pi)


def segments(x, r):
    a = int(np.argmax(np.abs(x) > 1e-3))
    return [x[a + int((k * STEP_S + 0.2) * r):a + int((k * STEP_S + 1.0) * r)] for k in range(len(LEVELS))]


def main():
    d = sys.argv[1]
    import matplotlib
    matplotlib.use('Agg')
    import matplotlib.pyplot as plt
    none, r = read(os.path.join(d, 'probe_c0_d0.wav')); none = none[:, 0]
    nseg = segments(none, r)
    for curve, name in ((1, 'TUBE'), (2, 'FUZZ')):
        fig, axes = plt.subplots(1, 4, figsize=(15, 3.4))
        for ax, drive in zip(axes, (0, 32, 64, 127)):
            hw, _ = read(os.path.join(d, 'probe_c%d_d%d.wav' % (curve, drive)))
            for k, (lvl, ns, hs, es) in enumerate(zip(LEVELS, nseg, segments(hw[:, 0], r), segments(engine(none, r, curve, drive), r))):
                f0 = refine_f0(ns, r, 250.0)
                ph = phase(slow(ns, r), r, f0)
                xin = fold(slow(ns, r), r, f0, ph)
                out = []
                for who, s in (('hw', hs), ('eng', es)):
                    fs = refine_f0(s, r, f0)
                    # each take on its own 250 Hz phase (captures start at different moments); the drive shifts
                    # the fundamental's phase only slightly against its input's
                    g = fold(demod(without_cycle(s, r, fs), r, 7300.0), r, fs, phase(slow(s, r), r, fs))
                    out.append(g / g.max())
                if k == len(LEVELS) - 1:
                    for (lab, g, st) in (('Pulse 2', out[0], '-'), ('engine', out[1], '--')):
                        ax.plot(xin / np.max(np.abs(xin)), g, st, lw=1.3, label=lab)
                peak_h = xin[np.argmax(out[0])] / np.max(np.abs(xin))
                peak_e = xin[np.argmax(out[1])] / np.max(np.abs(xin))
                print('%s Drive %3d, 250 Hz at %d dBFS: slope peaks at input %+.2f (Pulse 2), %+.2f (engine), of the swing; '
                      'slope at the + / - extremes %.2f/%.2f (hw) %.2f/%.2f (eng)' % (
                          name, drive, lvl, peak_h, peak_e, out[0][np.argmax(xin)], out[0][np.argmin(xin)], out[1][np.argmax(xin)], out[1][np.argmin(xin)]))
            ax.set_title('%s Drive %d (250 Hz at -12 dBFS)' % (name, drive), fontsize=8)
            ax.set_xlabel('input, of its swing'); ax.grid(alpha=0.3)
        axes[0].set_ylabel('slope, of its largest'); axes[0].legend(fontsize=7)
        fig.tight_layout(); fig.savefig(os.path.join(d, 'slope_%s.png' % name.lower()), dpi=100)
    print('plots in', d)


if __name__ == '__main__':
    main()
