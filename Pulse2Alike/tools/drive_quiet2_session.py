#!/usr/bin/python3
# FUZZ's quiet end and TUBE on noise, through EXT IN (p2Drive notes §11): static lopsidedness or a gain that follows
# the envelope? A single 1 kHz tone, then 1000 + 1300 Hz, each at -48/-42/-36/-30 dBFS for 1 s, through FUZZ at Drive
# 0, 17 (P003's) and 64 and through NONE. A static curve gives H2 on the single tone; a level-following gain shows
# only with two tones (at f2-f1). Then white noise at -30 and -18 dBFS rms, 1.5 s each, through TUBE 0/64/127 and NONE
# - the same file the engine can play through its side-chain. Osc 3 EXT at Level 127, filter open, Volume 40.
# usage: drive_quiet2_session.py [outdir]
import os, subprocess, sys, time

import numpy as np

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import p2rig
from drive_imd_session import RATE, PLAY, fade, write

OUT = sys.argv[1] if len(sys.argv) > 1 else os.environ.get('P2_CAPTURES', '/Volumes/ext-ssd/Captures/Pulse2') + '/192k/drive_quiet2'
LEVELS = (-48, -42, -36, -30)


def signals():
    t = np.arange(int(1.0 * RATE)) / RATE
    one = np.concatenate([10 ** (db / 20) * np.sin(2 * np.pi * 1000 * t) * fade(len(t)) for db in LEVELS])
    two = np.concatenate([10 ** (db / 20) * (np.sin(2 * np.pi * 1000 * t) + np.sin(2 * np.pi * 1300 * t)) * fade(len(t)) for db in LEVELS])
    rng = np.random.default_rng(11)
    tn = int(1.5 * RATE)
    noise = np.concatenate([10 ** (db / 20) * rng.standard_normal(tn) * fade(tn) for db in (-30, -18)])
    return {'one': one, 'two': two, 'noise': noise}


def main():
    os.makedirs(OUT, exist_ok=True)
    sigs = signals()
    for name, x in sigs.items():
        write(os.path.join(OUT, 'sig_%s.wav' % name), x)
    runs = [(n, 2, d) for n in ('one', 'two') for d in (0, 17, 64)] + [(n, 0, 0) for n in ('one', 'two', 'noise')] + [('noise', 1, d) for d in (0, 64, 127)]
    for name, curve, drive in runs:
        dur = len(sigs[name]) / RATE
        p2rig.set_sound(dict(osc1_level=0, osc3_shape=3, osc3_routing=0, osc3_level=127, volume=40, drive=drive, drive_curve=curve))
        time.sleep(0.4)
        path = os.path.join(OUT, '%s_c%d_d%d.wav' % (name, curve, drive))
        cap = p2rig.capture(path, dur + 2.0, rate=RATE)
        time.sleep(0.8)
        note = subprocess.Popen([p2rig.MIDISEND, p2rig.PORT, *p2rig.note_items(48, dur + 0.6)])
        time.sleep(0.3)
        subprocess.run([PLAY, '--device', 'Fireface', '--channels', str(p2rig.EXT_OUT_CHANNEL), '--file', os.path.join(OUT, 'sig_%s.wav' % name)],
                       check=True, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        note.wait()
        cap.communicate()
        print(path, flush=True)


if __name__ == '__main__':
    main()
