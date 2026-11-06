#!/usr/bin/python3
# The noise's level law and colour: noise alone, filter open, Volume 40, at several Levels; one 2 s note each, a
# .json beside it (shapes_compare / drive_noise_compare style).  usage: noise_law_session.py [outdir]
import json, os, sys, time

import numpy as np

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import p2rig
from osc_analyse import read

OUT = sys.argv[1] if len(sys.argv) > 1 else os.environ.get('P2_CAPTURES', '/Volumes/ext-ssd/Captures/Pulse2') + '/192k/noise_law'
LEVELS = (12, 20, 30, 40, 64, 90, 127)


def main():
    os.makedirs(OUT, exist_ok=True)
    for level in LEVELS:
        sound = dict(osc1_level=0, noise=level, volume=40)
        p2rig.set_sound(sound)
        time.sleep(0.4)
        path = os.path.join(OUT, 'noise_l%d.wav' % level)
        for attempt in range(4):
            cap = p2rig.capture(path, 3.0, rate=192000)
            time.sleep(0.6)
            p2rig.send(*p2rig.note_items(48, 2.0))
            cap.communicate()
            x, r = read(path)
            before = 20 * np.log10(np.sqrt(np.mean(x[:int(0.05 * r), 0] ** 2)) + 1e-12)   # the take's first 50 ms: silence
            if before < -60.0:
                break
            print('  not quiet before the note (%.0f dBFS) - again' % before, flush=True)
            time.sleep(2.0)
        json.dump({'notes': [48], 'hold_s': 2.0, 'settings': sound, 'mods': {}}, open(path[:-4] + '.json', 'w'))
        print(path, flush=True)


if __name__ == '__main__':
    main()
