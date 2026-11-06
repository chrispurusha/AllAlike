#!/usr/bin/python3
# The drive through EXT IN with two designed signals (p2Drive notes §9's "two tones and a DC step"; EXT IN is AC
# coupled, so the probe stands in for the step):
#   imd   - 1000 + 1300 Hz, equal, each -36/-30/-24/-18/-12 dBFS for 1 s: even products (300, 2300 Hz) are asymmetry,
#           odd ones (700, 1600 Hz) the curve's shape
#   probe - 250 Hz at -24/-18/-12 dBFS plus a 7.3 kHz probe at -46 dBFS, 1.2 s each: the probe's gain through the
#           big wave's cycle is the curve's slope at each point of it - off centre where there is a bias
# Osc 3 EXT into the mix at Level 127, the others off, filter open, Volume 40; NONE, then TUBE and FUZZ at Drive
# 0/32/64/127. Files: <signal>_c<curve>_d<drive>.wav, and the signals as sig_<signal>.wav.
# usage: drive_imd_session.py [outdir]
import os, subprocess, sys, time, wave

import numpy as np

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import p2rig

OUT = sys.argv[1] if len(sys.argv) > 1 else os.environ.get('P2_CAPTURES', '/Volumes/ext-ssd/Captures/Pulse2') + '/192k/drive_imd'
RATE = 192000
PLAY = os.path.join(p2rig.HERE, 'play')
POINTS = [(0, 0)] + [(c, d) for c in (1, 2) for d in (0, 32, 64, 127)]


def fade(n):
    w = np.ones(n)
    k = int(0.005 * RATE)
    w[:k] = np.linspace(0, 1, k)
    w[-k:] = np.linspace(1, 0, k)
    return w


def signals():
    t1 = np.arange(int(1.0 * RATE)) / RATE
    imd = np.concatenate([10 ** (db / 20) * (np.sin(2 * np.pi * 1000 * t1) + np.sin(2 * np.pi * 1300 * t1)) * fade(len(t1))
                          for db in (-36, -30, -24, -18, -12)])
    t2 = np.arange(int(1.2 * RATE)) / RATE
    probe = np.concatenate([(10 ** (db / 20) * np.sin(2 * np.pi * 250 * t2) + 10 ** (-46 / 20) * np.sin(2 * np.pi * 7300 * t2)) * fade(len(t2))
                            for db in (-24, -18, -12)])
    return {'imd': imd, 'probe': probe}


def write(path, x):
    w = wave.open(path, 'wb')
    w.setnchannels(1)
    w.setsampwidth(4)
    w.setframerate(RATE)
    w.writeframes((np.clip(x, -1, 1) * (2 ** 31 - 1)).astype('<i4').tobytes())
    w.close()


def main():
    os.makedirs(OUT, exist_ok=True)
    sigs = signals()
    for name, x in sigs.items():
        write(os.path.join(OUT, 'sig_%s.wav' % name), x)
    for name, x in sigs.items():
        dur = len(x) / RATE
        for curve, drive in POINTS:
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
