#!/usr/bin/python3
# Smooth Mode's other values (todo): TiAlike's Init into the edit buffer, osc 1 alone (as ti_smooth2_session.py),
# a held C4 and one CC Cutoff jump 30 -> 100 sent 1.0 s or 1.3 s after the note-on, for Smooth Off, On, Auto, Note,
# Quantise 1/16, 1/4 and 1/1. When the jump lands (from the note's own onset, in the audio) and how.
# usage: ti_smooth_modes_session.py <init .syx> <outdir> [refs]   (refs: the two reference takes only)
import os, subprocess, sys, time

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from ti_smooth_session import MIDISEND, CAPTURE, PORT, CH, sysex

MODES = ((0, 'off'), (1, 'on'), (2, 'auto'), (3, 'note'), (6, 'q1-16'), (8, 'q1-4'), (20, 'q1-1'))
AT_MS = (1000, 1300)


def main():
    init, out = sys.argv[1], sys.argv[2]
    os.makedirs(out, exist_ok=True)
    dump = open(init, 'rb').read().hex()
    runs = [(v, l, a) for v, l in MODES for a in AT_MS]
    if len(sys.argv) > 3 and sys.argv[3] == 'refs':     # the note at Cutoff 30 throughout, and at 100 throughout
        runs = [(0, 'ref30', None), (0, 'ref100', None)]
    for value, label, at in runs:
        if True:
            items = [dump, 'sleep:300'] + [x for p, v in ((33, 0), (34, 0), (37, 0), (44, 0), (45, 0)) for x in (sysex(0x70, p, v), 'sleep:30')]
            start = 100 if label == 'ref100' else 30
            items += [sysex(0x71, 25, value), 'sleep:50', '%02x%02x%02x' % (0xB0 | CH, 40, start), 'sleep:300']
            subprocess.run([MIDISEND, PORT] + items, check=True)
            path = os.path.join(out, ('mode_%s_at%d.wav' % (label, at)) if at is not None else ('%s.wav' % label))
            cap = subprocess.Popen([CAPTURE, '--device', 'Fireface', '--out', path, '--seconds', '5.5', '--rate', '48000', '--channels', '8,9'],
                                   stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
            time.sleep(0.8)
            if at is None:
                subprocess.run([MIDISEND, PORT, '%02x3c64' % (0x90 | CH), 'sleep:3500', '%02x3c00' % (0x80 | CH)], check=True)
            else:
                subprocess.run([MIDISEND, PORT, '%02x3c64' % (0x90 | CH), 'sleep:%d' % at, '%02x%02x%02x' % (0xB0 | CH, 40, 100),
                                'sleep:%d' % (3500 - at), '%02x3c00' % (0x80 | CH)], check=True)
            cap.wait()
            print(path, flush=True)


if __name__ == '__main__':
    main()
