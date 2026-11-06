#!/usr/bin/python3
# The modulation step's shape, closer (todo, Filter): LFO 1 triangle through MOD 1 at amount 100, fast (speed 100)
# and medium (40). Cutoff: the filter self-oscillating (resonance 127, no oscillator, cutoff 64), its pitch the
# cutoff. Pitch: osc 1 TRI at note 84, filter open, its instantaneous frequency tracked within each cycle.
# usage: modstep2_session.py [outdir]
import json, os, sys, time

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import p2rig

OUT = sys.argv[1] if len(sys.argv) > 1 else os.environ.get('P2_CAPTURES', '/Volumes/ext-ssd/Captures/Pulse2') + '/192k/modstep2'
CASES = {'cutoff': (dict(osc1_level=0, resonance=127, cutoff=64, volume=40), 10, 60),
         'pitch': (dict(osc1_shape=2, osc1_level=100, volume=40), 0, 84)}


def main():
    os.makedirs(OUT, exist_ok=True)
    for name, (sound, target, note) in CASES.items():
        for speed in (40, 100):
            mods = {p2rig.MOD_SOURCES[0]: 1, p2rig.MOD_AMOUNTS[0]: 100, 110: target, p2rig.CC_LFO1_SPEED: speed, p2rig.CC_LFO1_SHAPE: 1}
            p2rig.set_sound(sound)
            p2rig.send(*p2rig.cc_items(mods))
            time.sleep(0.4)
            path = os.path.join(OUT, 'modstep_%s_s%d.wav' % (name, speed))
            cap = p2rig.capture(path, 4.0, rate=192000)
            time.sleep(0.6)
            p2rig.send(*p2rig.note_items(note, 3.0))
            cap.communicate()
            json.dump({'notes': [note], 'hold_s': 3.0, 'settings': sound, 'mods': mods}, open(path[:-4] + '.json', 'w'))
            print(path, flush=True)


if __name__ == '__main__':
    main()
