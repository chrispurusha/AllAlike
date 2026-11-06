#!/usr/bin/python3
# ti_smooth_modes_session.py's takes: per Smooth mode and send time, when the Cutoff jump lands after the note's own
# onset, its 10-90% rise, and the lag against when it was sent. The Init sound sweeps its own treble every ~350 ms,
# locked to the note, so each take is read against two references played the same way (ref30: Cutoff 30 throughout,
# ref100: 100 throughout): at each millisecond 0 = ref30's level, 1 = ref100's (where they differ by 3 dB or more). A plot of the tracks.  usage: ti_smooth_modes_analyse.py <dir>
import glob, os, re, sys

import numpy as np

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from spdif_bits import words
from ti_smooth2_analyse import track


def median_filter(e, n):
    pad = np.pad(e, n // 2, mode='edge')
    return np.array([np.median(pad[i:i + n]) for i in range(0, len(e))])


def track_from_onset(path):
    v, _, r = words(path)
    x = (v[:, 0] & ~1).astype(float) / (1 << 23)
    on = int(np.argmax(np.abs(x) > 0.01))
    return median_filter(track(x, r)[on::int(0.001 * r)], 11)


def main():
    d = sys.argv[1]
    import matplotlib
    matplotlib.use('Agg')
    import matplotlib.pyplot as plt
    lo, hi = track_from_onset(os.path.join(d, 'ref30.wav')), track_from_onset(os.path.join(d, 'ref100.wav'))
    fig, ax = plt.subplots(figsize=(10, 4))
    order = ['off', 'on', 'auto', 'note', 'q1-16', 'q1-4', 'q1-1']
    paths = sorted(glob.glob(os.path.join(d, 'mode_*_at*.wav')), key=lambda p: (order.index(re.search(r'mode_(.+)_at', p).group(1)), p))
    print('%-8s %6s  %9s %8s %10s' % ('mode', 'sent', 'landed', 'lag', 'rise 10-90'))
    for p in paths:
        label, at = re.search(r'mode_(.+)_at(\d+)', p).groups()
        e = track_from_onset(p)
        n = min(len(e), len(lo), len(hi))
        e, l, h = e[:n], lo[:n], hi[:n]
        ok = (h - l) > 3.0
        frac = np.where(ok, (e - l) / np.where(ok, h - l, 1.0), np.nan)
        t = np.arange(n) / 1000.0
        good = np.nan_to_num(frac, nan=-1.0)
        cand = np.flatnonzero((good > 0.5) & (t > 0.5) & (t < 3.4))
        land = next((c for c in cand if np.nanmean(frac[c:c + 60]) > 0.7), None)
        if land is None:
            print('%-8s %5s ms  no jump seen' % (label, at))
            continue
        pre = np.flatnonzero((good < 0.1) & (t < t[land]) & (t > 0.5))
        post = np.flatnonzero((good > 0.9) & (t > t[land]))
        t10 = t[pre[-1]] if len(pre) else np.nan
        t90 = t[post[0]] if len(post) else np.nan
        print('%-8s %5s ms  %7.0f ms %+7.0f ms %8.0f ms' % (label, at, t[land] * 1000, t[land] * 1000 - int(at), (t90 - t10) * 1000))
        ax.plot(t * 1000, frac, lw=1, label='%s @%s' % (label, at))
    ax.set_xlim(800, 3400); ax.set_ylim(-0.3, 1.4); ax.set_xlabel('ms from the note onset'); ax.set_ylabel('0 = as Cutoff 30, 1 = as Cutoff 100')
    ax.legend(fontsize=7, ncol=2); ax.grid(alpha=0.3); fig.tight_layout(); fig.savefig(os.path.join(d, 'modes.png'), dpi=110)
    print('plot', os.path.join(d, 'modes.png'))


if __name__ == '__main__':
    main()
