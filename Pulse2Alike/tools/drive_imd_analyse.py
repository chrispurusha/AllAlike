#!/usr/bin/python3
# drive_imd_session.py's two-tone takes: per curve, Drive and step, the even products (300 = f2-f1, 2300 = f1+f2 Hz:
# asymmetry) and odd ones (700 = 2f1-f2, 1600 = 2f2-f1: the curve's shape) in dB under the tones - the instrument's
# and the engine's: with a base .syx, the whole engine (p2render, the signal into the side-chain as osc 3 EXT);
# without, src/p2Drive.c's model alone run on the NONE take as the drive's input.
# usage: drive_imd_analyse.py <dir> [base .syx work-dir]
import os, sys

import numpy as np

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import drive_wave as dw
import drive_ota as do
from osc_analyse import read

STEPS = (-36, -30, -24, -18, -12)
LINES = {'tone': (1000, 1300), 'even': (300, 2300), 'odd': (700, 1600)}


def level(seg, r, f):
    t = np.arange(len(seg)) / r
    w = np.hanning(len(seg))
    return 20 * np.log10(abs(np.sum(seg * w * np.exp(-2j * np.pi * f * t))) * 2 / w.sum() + 1e-12)


def steps(x, r):
    a = int(np.argmax(np.abs(x) > 1e-3))
    return [x[a + int((k + 0.15) * r):a + int((k + 0.85) * r)] for k in range(len(STEPS))]


def products(seg, r):
    tone = np.mean([level(seg, r, f) for f in LINES['tone']])
    return [max(level(seg, r, f) for f in LINES[k]) - tone for k in ('even', 'odd')], tone


def engine(x, r, curve, drive):
    sh, g6, t6 = do.ENGINE8[curve]
    g = float(np.interp(drive, dw.P2DRIVE_AT, g6)); t = float(np.interp(drive, dw.P2DRIVE_AT, t6))
    return dw.run(x, r, sh, 10 ** (g / 20), [-1000.0, 1e-9, t])


def rendered(d, base, work, curve, drive):
    import subprocess
    import shapes_compare as sc
    meta = {'settings': dict(osc1_level=0, osc3_shape=3, osc3_routing=0, osc3_level=127, volume=40, drive=drive, drive_curve=curve), 'mods': {}}
    sp = os.path.join(work, 'imd_c%d_d%d.syx' % (curve, drive))
    open(sp, 'wb').write(sc.sound_for(base, meta))
    ep = os.path.join(work, 'imd_c%d_d%d_p2.wav' % (curve, drive))
    subprocess.run([os.path.join(os.path.dirname(os.path.abspath(__file__)), 'p2render'), sp, '48', '5.6', '0.3', ep, '192000',
                    'ext=' + os.path.join(d, 'sig_imd.wav')], check=True, stdout=subprocess.DEVNULL)
    x, _ = read(ep)
    return x[:, 0] * 10.0


def main():
    d = sys.argv[1]
    base = open(sys.argv[2], 'rb').read() if len(sys.argv) > 3 else None
    work = sys.argv[3] if len(sys.argv) > 3 else None
    if work:
        os.makedirs(work, exist_ok=True)
    none, r = read(os.path.join(d, 'imd_c0_d0.wav'))
    none = none[:, 0]
    print('even / odd products, dB under the tones, per step (per-tone level in)')
    print('%-10s ' % '' + ''.join('%17d' % s for s in STEPS))
    for curve, name in ((1, 'TUBE'), (2, 'FUZZ')):
        for drive in (0, 32, 64, 127):
            hw, _ = read(os.path.join(d, 'imd_c%d_d%d.wav' % (curve, drive)))
            sh = steps(hw[:, 0], r)
            se = steps(rendered(d, base, work, curve, drive) if base else engine(none, r, curve, drive), r)
            row_h = '  '.join('%+5.0f/%+5.0f' % tuple(products(s, r)[0]) for s in sh)
            row_e = '  '.join('%+5.0f/%+5.0f' % tuple(products(s, r)[0]) for s in se)
            print('%s %3d hw   %s' % (name, drive, row_h))
            print('%s %3d eng  %s' % (name, drive, row_e))
    sn = steps(none, r)
    print('NONE     hw   ' + '  '.join('%+5.0f/%+5.0f' % tuple(products(s, r)[0]) for s in sn))
    if base:
        print('NONE     eng  ' + '  '.join('%+5.0f/%+5.0f' % tuple(products(s, r)[0]) for s in steps(rendered(d, base, work, 0, 0), r)))


if __name__ == '__main__':
    main()
