#!/usr/bin/python3
# noise_law_session.py's takes against the engine: per Level the held part's rms and octave bands, engine minus
# instrument, and the instrument's floor before the note (so a quiet Level is not read against the hiss). Also the
# amplitude distribution's crest and skew, the noise's shape.
# usage: noise_law_compare.py <takes dir> <base .syx> <work dir>
import glob, json, os, subprocess, sys

import numpy as np

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import shapes_compare as sc
import drive_noise_compare as dn
from osc_analyse import read

HERE = os.path.dirname(os.path.abspath(__file__))


def main():
    takes, base_syx, work = sys.argv[1:4]
    base = open(base_syx, 'rb').read()
    os.makedirs(work, exist_ok=True)
    print('level  hw dBFS  floor   eng   diff   crest hw/eng   bands engine-hw dB (%s Hz)' % ','.join(str(b) for b in dn.BANDS))
    metas = sorted(glob.glob(os.path.join(takes, 'noise_l*.json')), key=lambda f: int(f.split('_l')[-1][:-5]))
    for jp in metas:
        meta = json.load(open(jp))
        name = os.path.basename(jp)[:-5]
        sp = os.path.join(work, name + '.syx'); open(sp, 'wb').write(sc.sound_for(base, meta))
        ep = os.path.join(work, name + '_p2.wav')
        subprocess.run([os.path.join(HERE, 'p2render'), sp, '48', '2.0', '0.3', ep, '192000'], check=True, stdout=subprocess.DEVNULL)
        h, rh = read(os.path.join(takes, name + '.wav')); h = h[:, 0]
        e, re = read(ep); e = e[:, 0] * 10.0
        hs, es = dn.held(h, rh), dn.held(e, re)
        a = dn.onset(h, rh)
        floor = h[max(0, a - int(0.5 * rh)):max(0, a - int(0.05 * rh))]
        lh, le = [20 * np.log10(np.sqrt(np.mean(s ** 2))) for s in (hs, es)]
        lf = 20 * np.log10(np.sqrt(np.mean(floor ** 2)) + 1e-12)
        bd = (dn.bands(es, re) - 10 * np.log10(len(es))) - (dn.bands(hs, rh) - 10 * np.log10(len(hs)))
        print('%5d  %7.1f %6.1f %6.1f %+6.1f   %5.2f / %5.2f   %s' % (meta['settings']['noise'], lh, lf, le, le - lh,
              dn.shape(hs)[0], dn.shape(es)[0], ' '.join('%+5.1f' % v for v in bd)))


if __name__ == '__main__':
    main()
