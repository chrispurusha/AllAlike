#!/usr/bin/python3
# drive_quiet2_session.py's takes against the engine playing the same files through its side-chain: per step, on
# the single tone H2 and H3 under H1, on the two tones f2-f1 (300) and f1+f2 (2300) under the tones, on the noise its
# level and crest. dB.  usage: drive_quiet2_analyse.py <dir> <base .syx> <work dir>
import os, subprocess, sys

import numpy as np

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import shapes_compare as sc
from drive_imd_analyse import level
from osc_analyse import read

HERE = os.path.dirname(os.path.abspath(__file__))


def steps(x, r, n, length):
    a = int(np.argmax(np.abs(x) > 3e-4))
    return [x[a + int((k * length + 0.15) * r):a + int((k * length + length - 0.15) * r)] for k in range(n)]


def render(d, base, work, name, curve, drive, dur):
    meta = {'settings': dict(osc1_level=0, osc3_shape=3, osc3_routing=0, osc3_level=127, volume=40, drive=drive, drive_curve=curve), 'mods': {}}
    sp = os.path.join(work, '%s_c%d_d%d.syx' % (name, curve, drive)); open(sp, 'wb').write(sc.sound_for(base, meta))
    ep = os.path.join(work, '%s_c%d_d%d_p2.wav' % (name, curve, drive))
    subprocess.run([os.path.join(HERE, 'p2render'), sp, '48', '%.2f' % (dur + 0.4), '0.3', ep, '192000', 'ext=' + os.path.join(d, 'sig_%s.wav' % name)],
                   check=True, stdout=subprocess.DEVNULL)
    x, _ = read(ep)
    return x[:, 0] * 10.0


def main():
    d, base, work = sys.argv[1], open(sys.argv[2], 'rb').read(), sys.argv[3]
    os.makedirs(work, exist_ok=True)
    print('single tone: H2 / H3 under H1, steps -48 -42 -36 -30 dBFS')
    for curve, drive in ((0, 0), (2, 0), (2, 17), (2, 64)):
        h, r = read(os.path.join(d, 'one_c%d_d%d.wav' % (curve, drive)))
        e = render(d, base, work, 'one', curve, drive, 4.0)
        for who, x in (('Pulse 2', h[:, 0]), ('engine', e)):
            row = ['%+4.0f/%+4.0f' % (level(s, r, 2000) - level(s, r, 1000), level(s, r, 3000) - level(s, r, 1000)) for s in steps(x, r, 4, 1.0)]
            print('  %s %3d %-8s %s' % (['NONE', 'TUBE', 'FUZZ'][curve], drive, who, '   '.join(row)))
    print('two tones: f2-f1 / f1+f2 under the tones')
    for curve, drive in ((0, 0), (2, 0), (2, 17), (2, 64)):
        h, r = read(os.path.join(d, 'two_c%d_d%d.wav' % (curve, drive)))
        e = render(d, base, work, 'two', curve, drive, 4.0)
        for who, x in (('Pulse 2', h[:, 0]), ('engine', e)):
            row = []
            for s in steps(x, r, 4, 1.0):
                tone = (level(s, r, 1000) + level(s, r, 1300)) / 2
                row.append('%+4.0f/%+4.0f' % (level(s, r, 300) - tone, level(s, r, 2300) - tone))
            print('  %s %3d %-8s %s' % (['NONE', 'TUBE', 'FUZZ'][curve], drive, who, '   '.join(row)))
    print('noise at -30 / -18 dBFS in: level dBFS (crest)')
    for curve, drive in ((0, 0), (1, 0), (1, 64), (1, 127)):
        h, r = read(os.path.join(d, 'noise_c%d_d%d.wav' % (curve, drive)))
        e = render(d, base, work, 'noise', curve, drive, 3.0)
        out = []
        for who, x in (('Pulse 2', h[:, 0]), ('engine', e)):
            row = []
            for s in steps(x, r, 2, 1.5):
                s = s - s.mean(); rms = np.sqrt(np.mean(s ** 2))
                row.append('%6.1f (%.2f)' % (20 * np.log10(rms), np.percentile(np.abs(s), 99.9) / rms))
            out.append('  '.join(row))
        print('  %s %3d  Pulse 2 %s   engine %s' % (['NONE', 'TUBE', 'FUZZ'][curve], drive, out[0], out[1]))


if __name__ == '__main__':
    main()
