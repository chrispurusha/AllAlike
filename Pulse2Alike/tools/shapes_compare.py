#!/usr/bin/python3
# The engine's wave shapes against shapes_session.py's takes. Each take's sound is rebuilt from a base dump (the
# edit buffer read back with the test sound in it) with the take's CCs applied as sound bytes, rendered by
# tools/p2render, and both are folded into one averaged cycle (phase-locked on the instrument's refined f0, the
# engine's on its own), aligned by circular shift. Reported per take: the cycle error as % of the instrument's peak,
# the level, the harmonic rms, and the cycle overlays as one PNG per group.
# usage: shapes_compare.py <takes dir> <base .syx> <work dir> [name-prefix ...]
import glob, json, os, subprocess, sys

import numpy as np

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import p2rig
from osc_analyse import read, refine_f0

HERE = os.path.dirname(os.path.abspath(__file__))
# CC -> sound byte, for the parameters the rig sets
CC_BYTE = {34: 0, 35: 1, 70: 2, 71: 3, 73: 4, 45: 5, 38: 6, 39: 7, 36: 8, 37: 9, 40: 10, 46: 11, 44: 12, 72: 13, 42: 14,
           43: 15, 41: 16, 47: 17, 24: 18, 25: 19, 26: 20, 27: 21, 5: 22, 62: 23, 14: 24, 15: 25, 16: 26, 17: 27, 74: 28,
           29: 29, 18: 30, 19: 31, 20: 32, 21: 33, 75: 34, 31: 35, 79: 36, 80: 37, 81: 38, 82: 39, 83: 41, 50: 42, 56: 43,
           52: 44, 49: 45, 51: 46, 53: 47, 76: 48, 77: 49, 59: 50, 48: 51, 57: 52, 58: 53, 108: 54, 109: 55, 110: 56,
           111: 57, 112: 58, 113: 59, 114: 60, 115: 61, 116: 62, 117: 63, 118: 64, 119: 65, 90: 66, 91: 67, 92: 68,
           93: 69, 94: 70, 95: 71, 96: 72, 97: 73, 98: 74, 99: 75, 100: 76, 101: 77, 102: 78, 103: 79, 104: 80, 105: 85,
           106: 86, 84: 87, 85: 88, 86: 89, 87: 106, 88: 107}
N = 512
WINDOW = (0.3, 1.2)          # seconds after the onset
HARMONICS = 24


def sound_for(base, meta):
    d = bytearray(base)
    settings = dict(p2rig.TEST_SOUND)
    settings.update(meta['settings'])
    ccs = {p2rig.CC[k]: v for k, v in settings.items()}
    ccs.update({c: 0 for c in p2rig.MOD_SOURCES})
    ccs.update({c: 64 for c in p2rig.MOD_AMOUNTS})
    ccs.update({int(k): v for k, v in meta.get('mods', {}).items()})
    for cc, v in ccs.items():
        if cc in CC_BYTE:
            d[7 + CC_BYTE[cc]] = v
    d[5], d[6] = 0x7F, 0x00
    d[135] = sum(d[1:135]) & 0x7F
    return bytes(d)


def onset(x):
    e = np.abs(x)
    return int(np.argmax(e > 0.1 * e.max()))


def cycle(s, rate, f0):
    t = np.arange(len(s)) / rate
    b = (((t * f0) % 1.0) * N).astype(int)
    c = np.bincount(b, s, N) / np.maximum(np.bincount(b, None, N), 1)
    return c - c.mean()


def harmonics(s, rate, f0):
    t = np.arange(len(s)) / rate
    w = np.hanning(len(s))
    return np.array([20 * np.log10(abs(np.sum(s * w * np.exp(-2j * np.pi * f0 * k * t))) * 2 / w.sum() + 1e-12) for k in range(1, HARMONICS + 1)])


def analyse(path, rate_scale=1.0):
    x, r = read(path)
    x = x[:, 0] * rate_scale
    a = onset(x)
    return x[a + int(WINDOW[0] * r):a + int(WINDOW[1] * r)], r


def main():
    takes, base_syx, work = sys.argv[1:4]
    prefixes = sys.argv[4:] or ['osc1_', 'osc3_', 'sync_']
    base = open(base_syx, 'rb').read()
    os.makedirs(work, exist_ok=True)
    groups = {}
    print('%-26s %7s %7s %6s %7s' % ('take', 'cycle%', 'level', 'harm', 'f0 Hz'))
    for jp in sorted(glob.glob(os.path.join(takes, '*.json'))):
        name = os.path.basename(jp)[:-5]
        if not any(name.startswith(p) for p in prefixes):
            continue
        meta = json.load(open(jp))
        note = meta['notes'][0]
        sp = os.path.join(work, name + '.syx')
        open(sp, 'wb').write(sound_for(base, meta))
        ep = os.path.join(work, name + '_p2.wav')
        if not os.path.exists(ep):
            subprocess.run([os.path.join(HERE, 'p2render'), sp, str(note), str(meta['hold_s']), '0.3', ep, '192000'],
                           check=True, stdout=subprocess.DEVNULL)
        h, rh = analyse(os.path.join(takes, name + '.wav'))
        e, re = analyse(ep, 10.0)
        guess = 440.0 * 2 ** ((note - 69) / 12.0)
        fh = refine_f0(h[:int(0.4 * rh)], rh, guess)
        fe = refine_f0(e[:int(0.4 * re)], re, fh)
        ch, ce = cycle(h, rh, fh), cycle(e, re, fe)
        ce = np.roll(ce, int(np.argmax([np.dot(np.roll(ce, k), ch) for k in range(N)])))
        peak = max(1e-9, np.max(np.abs(ch)))
        err = np.sqrt(np.mean((ce - ch) ** 2)) / peak * 100
        lv = 20 * np.log10(np.sqrt(np.mean(e ** 2)) / np.sqrt(np.mean(h ** 2)))
        hh, he = harmonics(h, rh, fh), harmonics(e, re, fe)
        keep = hh > hh[0] - 60
        hr = np.sqrt(np.mean(((he - he[0]) - (hh - hh[0]))[keep] ** 2))
        print('%-26s %6.1f%% %+6.1f %5.1f %8.2f' % (name, err, lv, hr, fh), flush=True)
        g = name.rsplit('_n', 1)[0] if name.startswith(('osc1_', 'osc3_')) else 'sync'
        groups.setdefault(g, []).append((name, ch, ce, err))
    import matplotlib
    matplotlib.use('Agg')
    import matplotlib.pyplot as plt
    for g, items in groups.items():
        fig, axes = plt.subplots(1, len(items), figsize=(2.6 * len(items), 2.4), squeeze=False)
        for ax, (name, ch, ce, err) in zip(axes[0], items):
            ax.plot(ch, color='#1f4e9c', lw=1.4, label='Pulse 2')
            ax.plot(ce, color='#d1602a', lw=1.0, label='engine')
            ax.set_title('%s  %.1f%%' % (name.replace(g + '_', '') if g != 'sync' else name.replace('sync_', ''), err), fontsize=8)
            ax.set_xticks([]); ax.tick_params(labelsize=6)
        axes[0][0].legend(fontsize=6, loc='best')
        fig.suptitle(g, fontsize=9)
        fig.tight_layout()
        fig.savefig(os.path.join(work, 'cycles_%s.png' % g), dpi=110)
        plt.close(fig)
    print('plots in', work)


if __name__ == '__main__':
    main()
