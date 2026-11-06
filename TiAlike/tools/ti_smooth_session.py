#!/usr/bin/python3
# Parameter Smooth Mode on the instrument (todo: the reference renders jump, TiAlike glides ~45 ms): a held note on
# whatever sound is in the edit buffer, Cutoff 30 -> 100 one second in, by CC 40 or by SysEx (page A, parameter 40),
# with Smooth Mode (page B, 25) Off and On. S/PDIF into the Fireface (inputs 9/10), 48 kHz from the Virus.
# Only the edit buffer changes. usage: ti_smooth_session.py <outdir>
import os, subprocess, sys, time

HERE = os.path.dirname(os.path.abspath(__file__))
MIDISEND = os.path.join(HERE, '..', '..', 'Pulse2Alike', 'tools', 'midisend')
CAPTURE = os.path.join(HERE, '..', '..', '..', 'G2Alike', 'tools', 'capture')
PORT = 'Virus TI USB Plugin'
CH = 0                      # MIDI channel 1


def sysex(page, param, value):
    return 'f000203301%02x%02x40%02x%02xf7' % (0x10, page, param, value)


def take(path, smooth, by):
    items = [sysex(0x71, 25, smooth), 'sleep:100', sysex(0x70, 40, 30), 'sleep:200']
    subprocess.run([MIDISEND, PORT] + items, check=True)
    cap = subprocess.Popen([CAPTURE, '--device', 'Fireface', '--out', path, '--seconds', '4', '--rate', '48000', '--channels', '8,9'],
                           stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    time.sleep(0.8)
    jump = ('%02x%02x%02x' % (0xB0 | CH, 40, 100)) if by == 'cc' else sysex(0x70, 40, 100)
    subprocess.run([MIDISEND, PORT, '%02x3c64' % (0x90 | CH), 'sleep:1000', jump, 'sleep:1200', '%02x3c00' % (0x80 | CH)], check=True)
    cap.wait()
    print(path, flush=True)


def main():
    out = sys.argv[1]
    os.makedirs(out, exist_ok=True)
    for smooth in (0, 1):
        for by in ('cc', 'sysex'):
            take(os.path.join(out, 'smooth%d_%s.wav' % (smooth, by)), smooth, by)


if __name__ == '__main__':
    main()
