#!/usr/bin/python3
# Wave shapes for the oscillators and the modulation step (todo: Oscillators, Filter): every take is one held note
# with the test sound (p2rig.TEST_SOUND: filter open, envelopes flat) at Volume 40, a .json beside it with the
# settings, so averaged cycles can be set against the engine's.
#   osc1_<shape>[_w<width>]_l<level>_n<note>   osc 1 alone: PWM at widths 0/64/110, SAW, TRI, APW; Level 40 and 127
#   osc3_<shape>_l<level>_n<note>              osc 3 alone: SQR, SAW, TRI
#   sync_o2saw_s<semi>[_o3]                    osc 2 SAW synced by osc 3 at osc 2 semitone +7/+12/+19, alone and
#                                              with osc 3's square in the mix (where osc 3's edges sit)
#   modstep_<target>                           LFO 1 (fast) through MOD 1 on CUTOFF or PITCH, osc 1 SAW
# usage: shapes_session.py [outdir]
import json, os, sys, time

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import p2rig

OUT = sys.argv[1] if len(sys.argv) > 1 else os.environ.get('P2_CAPTURES', '/Volumes/ext-ssd/Captures/Pulse2') + '/192k/shapes'
NOTES = (36, 48, 60, 72, 84, 96)
HOLD = 1.5
RATE = 192000


def take(name, settings, note=48, hold=HOLD, mods=None):
    path = os.path.join(OUT, name + '.wav')
    p2rig.set_sound(settings)
    if mods:
        p2rig.send(*p2rig.cc_items(mods))
    time.sleep(0.4)
    cap = p2rig.capture(path, hold + 1.0, rate=RATE)
    time.sleep(0.6)
    p2rig.send(*p2rig.note_items(note, hold))
    cap.communicate()
    json.dump({'notes': [note], 'hold_s': hold, 'settings': settings, 'mods': mods or {}}, open(path.replace('.wav', '.json'), 'w'))
    print(name, flush=True)


def main():
    os.makedirs(OUT, exist_ok=True)
    osc1 = [('pwm_w0', 0, 0), ('pwm_w64', 0, 64), ('pwm_w110', 0, 110), ('saw', 1, 64), ('tri', 2, 64), ('apw', 3, 64)]
    for level in (40, 127):
        for label, shape, width in osc1:
            for n in NOTES:
                take('osc1_%s_l%d_n%d' % (label, level, n), dict(osc1_shape=shape, osc1_pw=width, osc1_level=level, volume=40), n)
        for label, shape in (('sqr', 0), ('saw', 1), ('tri', 2)):
            for n in NOTES:
                take('osc3_%s_l%d_n%d' % (label, level, n), dict(osc1_level=0, osc3_shape=shape, osc3_level=level, volume=40), n)
    for semi in (71, 76, 83):
        base = dict(osc1_level=0, osc2_shape=1, osc2_level=100, osc2_semi=semi, osc3_shape=0, osc3_sync=1, volume=40)
        take('sync_o2saw_s%d' % semi, dict(base, osc3_level=0))
        take('sync_o2saw_s%d_o3' % semi, dict(base, osc3_level=100))
    # MOD 1: source 1 (LFO1), amount 100, target 10 (CUTOFF) or 0 (PITCH); LFO 1 fast, triangle
    for target, tnum, cutoff in (('cutoff', 10, 64), ('pitch', 0, 127)):
        take('modstep_%s' % target, dict(osc1_shape=1, cutoff=cutoff, volume=40), 48, hold=3.0,
             mods={p2rig.MOD_SOURCES[0]: 1, p2rig.MOD_AMOUNTS[0]: 100, 110: tnum, p2rig.CC_LFO1_SPEED: 100, p2rig.CC_LFO1_SHAPE: 1})


if __name__ == '__main__':
    main()
