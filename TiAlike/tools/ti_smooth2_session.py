#!/usr/bin/python3
# Smooth Mode's overshoot, measured cleanly (findings 2026-11-06): TiAlike's Init Single into the edit buffer, then
# osc 1 alone (Osc Balance A33 0, Sub A34 0, Noise A37 0, Filter Env Amounts A44/A45 0); a held C4 and one Cutoff
# jump by CC 40 - 30->100, 30->60, 100->30 - with Smooth Mode (B25) Off and On. S/PDIF, 48 kHz.
# usage: ti_smooth2_session.py <init .syx> <outdir>
import os, subprocess, sys, time

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from ti_smooth_session import MIDISEND, CAPTURE, PORT, CH, sysex

JUMPS = ((30, 100), (30, 60), (100, 30))


def main():
    init, out = sys.argv[1], sys.argv[2]
    os.makedirs(out, exist_ok=True)
    dump = open(init, 'rb').read().hex()
    for smooth in (0, 1):
        for a, b in JUMPS:
            items = [dump, 'sleep:300'] + [x for p, v in ((33, 0), (34, 0), (37, 0), (44, 0), (45, 0)) for x in (sysex(0x70, p, v), 'sleep:30')]
            items += [sysex(0x71, 25, smooth), 'sleep:50', '%02x%02x%02x' % (0xB0 | CH, 40, a), 'sleep:300']
            subprocess.run([MIDISEND, PORT] + items, check=True)
            path = os.path.join(out, 'smooth%d_%d_%d.wav' % (smooth, a, b))
            cap = subprocess.Popen([CAPTURE, '--device', 'Fireface', '--out', path, '--seconds', '3.5', '--rate', '48000', '--channels', '8,9'],
                                   stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
            time.sleep(0.8)
            subprocess.run([MIDISEND, PORT, '%02x3c64' % (0x90 | CH), 'sleep:1000', '%02x%02x%02x' % (0xB0 | CH, 40, b), 'sleep:1000',
                            '%02x3c00' % (0x80 | CH)], check=True)
            cap.wait()
            print(path, flush=True)


if __name__ == '__main__':
    main()
