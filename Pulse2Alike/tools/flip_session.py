#!/usr/bin/python3
# What flips a DCO's toggle state (findings 2026-11-06, "wave shapes"): osc 1 PWM at PW 64 alone, a random sequence
# of notes (fixed seed), one capture each; the state read from the averaged cycle's duty (0.129 normal, 0.871
# flipped). Prints each transition and the state after it.
# usage: flip_session.py [outdir] [count]
import os, random, sys, time

import numpy as np

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import p2rig
import shapes_compare as sc
from osc_analyse import refine_f0

OUT = sys.argv[1] if len(sys.argv) > 1 else os.environ.get('P2_CAPTURES', '/Volumes/ext-ssd/Captures/Pulse2') + '/192k/flip'
COUNT = int(sys.argv[2]) if len(sys.argv) > 2 else 60
NOTES = (36, 43, 48, 55, 60, 67, 72, 84)


def state(path, note):
    s, r = sc.analyse(path)
    s = s[:int(0.4 * r)]
    c = sc.cycle(s, r, refine_f0(s, r, 440 * 2 ** ((note - 69) / 12)))
    return float(np.mean(c > (c.max() + c.min()) / 2))


def main():
    os.makedirs(OUT, exist_ok=True)
    rng = random.Random(7)
    p2rig.set_sound(dict(osc1_shape=0, osc1_pw=64, osc1_level=40, volume=40))
    time.sleep(0.4)
    prev = None
    rows = []
    for i in range(COUNT):
        note = rng.choice(NOTES)
        path = os.path.join(OUT, 'f%03d_n%d.wav' % (i, note))
        cap = p2rig.capture(path, 1.3, rate=192000)
        time.sleep(0.5)
        p2rig.send(*p2rig.note_items(note, 0.6))
        cap.communicate()
        d = state(path, note)
        flipped = d > 0.5
        rows.append((prev, note, flipped, d))
        print('%3d  %4s -> %-3d duty %.3f %s' % (i, prev if prev is not None else '-', note, d, 'FLIPPED' if flipped else ''), flush=True)
        prev = note
    np.savetxt(os.path.join(OUT, 'states.csv'), [(r[0] if r[0] is not None else -1, r[1], int(r[2]), r[3]) for r in rows],
               delimiter=',', fmt='%g', header='from,to,flipped,duty')


if __name__ == '__main__':
    main()
