#!/usr/bin/python3
# Noise through the drive (todo: TUBE on noise, P030 10 dB loud): noise 127 alone, Volume 40 (the drive's units,
# p2Drive notes §2), two filters - LP24 open, and P030's BP12 (cutoff 53, resonance 83) - each with NONE, then TUBE and
# FUZZ at Drive 0/32/64/127. One 2 s note per take at 192 kHz. Files: <filter>_c<curve>_d<drive>.wav
# usage: drive_noise_session.py [outdir]
import os, sys, time

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import p2rig

OUT = sys.argv[1] if len(sys.argv) > 1 else os.environ.get('P2_CAPTURES', '/Volumes/ext-ssd/Captures/Pulse2') + '/192k/drive_noise'
FILTERS = {'lp24open': dict(vcf_type=0, cutoff=127, resonance=0), 'bp12p030': dict(vcf_type=2, cutoff=53, resonance=83)}
POINTS = [(0, 0)] + [(c, d) for c in (1, 2) for d in (0, 32, 64, 127)]

os.makedirs(OUT, exist_ok=True)
for fname, filt in FILTERS.items():
    for curve, drive in POINTS:
        path = os.path.join(OUT, '%s_c%d_d%d.wav' % (fname, curve, drive))
        sound = dict(osc1_level=0, noise=127, volume=40, drive=drive, drive_curve=curve)
        sound.update(filt)
        p2rig.set_sound(sound)
        time.sleep(0.4)
        cap = p2rig.capture(path, 3.2, rate=192000)
        time.sleep(0.7)
        p2rig.send(*p2rig.note_items(48, 2.0))
        print(cap.communicate()[0].strip().splitlines()[-1][:100], flush=True)
