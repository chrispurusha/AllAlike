#!/usr/bin/python3
# The modulation step's shape, the instrument against the engine (modstep2_session.py's takes): each take's
# frequency tracked cycle by cycle from interpolated zero crossings, as semitones; its rate of change resampled on a
# 20 us grid; the update clock found as the autocorrelation peak near 1 ms; the rate folded on that clock, giving
# the step's shape (where in the period the change happens) and its size.
# usage: modstep_compare.py <takes dir> <base .syx> <work dir>
import glob, json, os, subprocess, sys

import numpy as np

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import shapes_compare as sc
from osc_analyse import read

HERE = os.path.dirname(os.path.abspath(__file__))
GRID = 20e-6
FOLD = 50


def track(x, r):
    x = x - np.convolve(x, np.ones(r // 5) / (r // 5), 'same')
    i = np.flatnonzero((x[:-1] < 0) & (x[1:] >= 0))
    t = (i + (-x[i] / (x[i + 1] - x[i]))) / r
    f = 1.0 / np.diff(t)
    return t[1:], 12 * np.log2(f / 440.0) + 69


def study(x, r):
    a = sc.onset(x)
    x = x[a + int(0.2 * r):a + int(2.8 * r)]
    t, s = track(x, r)
    ok = np.isfinite(s) & (s > 20) & (s < 140)
    t, s = t[ok], s[ok]
    g = np.arange(t[0], t[-1], GRID)
    sg = np.interp(g, t, s)
    rate = np.gradient(sg, GRID)
    rate = rate - np.convolve(rate, np.ones(250) / 250, 'same')          # the LFO's own slope out (5 ms)
    ac = [np.dot(rate[:-k], rate[k:]) / np.dot(rate, rate) for k in range(30, 70)]
    k = 30 + int(np.argmax(ac))
    period = k * GRID
    ph = ((g - g[0]) % period) / period
    b = (ph * FOLD).astype(int)
    prof = np.bincount(b, np.abs(np.gradient(sg, GRID)), FOLD) / np.maximum(np.bincount(b, None, FOLD), 1)
    prof = prof - prof.min()
    prof = np.roll(prof, -int(np.argmax(prof)) + 2)
    share = np.cumsum(prof) / prof.sum()
    slope = np.median(np.abs(np.gradient(sg, GRID)))
    return period * 1000, ac[k - 30], slope * period, share, (sg.min(), sg.max())


def main():
    takes, base_syx, work = sys.argv[1:4]
    base = open(base_syx, 'rb').read()
    os.makedirs(work, exist_ok=True)
    print('%-22s %-7s %8s %6s %10s %8s %8s  %s' % ('take', 'who', 'clock ms', 'r', 'step st', 'q1 share', 'half', 'range st'))
    for jp in sorted(glob.glob(os.path.join(takes, 'modstep_*.json'))):
        name = os.path.basename(jp)[:-5]
        meta = json.load(open(jp))
        sp = os.path.join(work, name + '.syx'); open(sp, 'wb').write(sc.sound_for(base, meta))
        ep = os.path.join(work, name + '_p2.wav')
        if not os.path.exists(ep):
            subprocess.run([os.path.join(HERE, 'p2render'), sp, str(meta['notes'][0]), str(meta['hold_s']), '0.3', ep, '192000'],
                           check=True, stdout=subprocess.DEVNULL)
        for who, path, gain in (('Pulse2', os.path.join(takes, name + '.wav'), 1.0), ('engine', ep, 10.0)):
            x, r = read(path)
            ms, rr, step, share, rng = study(x[:, 0] * gain, r)
            print('%-22s %-7s %8.3f %6.2f %10.4f %7.0f%% %7.0f%%  %.1f..%.1f' % (name, who, ms, rr, step, share[FOLD // 4 - 1] * 100,
                  share[FOLD // 2 - 1] * 100, rng[0], rng[1]))


if __name__ == '__main__':
    main()
