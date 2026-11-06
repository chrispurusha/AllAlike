#!/usr/bin/python3
# The toggle flip's odds against the target note: from note 48 (below the long-period threshold, so the state is
# normal) up to each target, six times, back to 48 between (which resets it). osc 1 PWM, PW 64, alone.
# usage: flip_targets_session.py [outdir]
import os, sys, time

import numpy as np

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import p2rig
from flip_session import state

OUT = sys.argv[1] if len(sys.argv) > 1 else os.environ.get('P2_CAPTURES', '/Volumes/ext-ssd/Captures/Pulse2') + '/192k/flip_targets'
TARGETS = (55, 57, 60, 62, 64, 67, 69, 72, 76, 84)
REPEATS = 6


def main():
    os.makedirs(OUT, exist_ok=True)
    p2rig.set_sound(dict(osc1_shape=0, osc1_pw=64, osc1_level=40, volume=40))
    time.sleep(0.4)
    rows = []
    for k in range(REPEATS):
        for tgt in TARGETS:
            p2rig.send(*p2rig.note_items(48, 0.3))
            time.sleep(0.15)
            path = os.path.join(OUT, 'r%d_n%d.wav' % (k, tgt))
            cap = p2rig.capture(path, 1.1, rate=192000)
            time.sleep(0.45)
            p2rig.send(*p2rig.note_items(tgt, 0.5))
            cap.communicate()
            d = state(path, tgt)
            rows.append((tgt, int(d > 0.5), d))
            print('48 -> %d  duty %.3f %s' % (tgt, d, 'FLIPPED' if d > 0.5 else ''), flush=True)
    np.savetxt(os.path.join(OUT, 'states.csv'), rows, delimiter=',', fmt='%g', header='to,flipped,duty')
    for tgt in TARGETS:
        f = [r[1] for r in rows if r[0] == tgt]
        print('target %d: %d of %d flipped' % (tgt, sum(f), len(f)))


if __name__ == '__main__':
    main()
