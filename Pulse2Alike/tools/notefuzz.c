// Random keys, releases, pedal and program changes through the plug-in's own engine, checking after each event that
// nothing sounds or is held that the keys do not account for: a stray note. Built with the plug-in compiled in, so it
// can look at the engine's state.
//
// usage: notefuzz [events] [seed] [first last]     programs first..last (P001.., default 1..30)

#include "../plugin/p2Plugin.c"

#include <stdio.h>

#define BLOCK    (256)
#define ECHO_MAX (4096)

// a host's copy of an edited parameter comes back as setParam a block or more later; the harness does the same
static uint32_t gEchoId[ECHO_MAX];
static double   gEchoValue[ECHO_MAX];
static int      gEchoCount = 0;

static void echo_param_edited(void * inst, uint32_t id, double normalized) {
    (void)inst;

    if (gEchoCount < ECHO_MAX) {
        gEchoId[gEchoCount]    = id;
        gEchoValue[gEchoCount] = normalized;
        gEchoCount++;
    }
}

static const tSynthLibPluginFormat kEchoFormat = {
    .paramEdited = echo_param_edited,
};

static uint32_t gSeed = 1;

static uint32_t rnd(uint32_t n) {
    gSeed = (gSeed * 1103515245u) + 12345u;
    return (gSeed >> 8) % n;
}

static int gFailures = 0;

static void fail(int event, const char * what, int preset, int voices, int arp) {
    if (gFailures < 40) {
        printf("event %d: %s (P%03d, Voices %d, arp %d)\n", event, what, preset, voices, arp);
    }
    gFailures++;
}

int main(int argc, char ** argv) {
    int                         events = (argc > 1) ? atoi(argv[1]) : 20000;
    int                         first  = (argc > 3) ? atoi(argv[3]) : 1;
    int                         last   = (argc > 4) ? atoi(argv[4]) : 30;

    gSeed = (argc > 2) ? (uint32_t)atoi(argv[2]) : 1u;
    synthlib_plugin_set_format(&kEchoFormat);
    const tSynthLibPluginDesc * d      = &synthlib_plugin_variants()->variants[0];
    void *                      inst   = d->cb.create(d);
    tP2Plugin *                 p      = (tP2Plugin *)inst;
    tSynthLibSetup              setup  = {.sampleRate = 48000, .maxFrames = BLOCK, .offline = true};
    bool                        held[128];
    bool                        pedal  = false;
    float                       left[BLOCK], right[BLOCK];
    float *                     outs[2] = {left, right};

    memset(held, 0, sizeof(held));
    d->cb.prepare(inst, &setup);
    d->cb.setActive(inst, true);

    for (int e = 0; e < events; e++) {
        int  choice   = (int)rnd(100);
        int  keys     = 0;
        bool released = false;

        for (int n = 0; n < 128; n++) {
            keys += held[n] ? 1 : 0;
        }

        if (choice < 40) {                                   // a key down
            int note = 48 + (int)rnd(13);

            if (!held[note]) {
                held[note] = true;
                d->cb.noteOn(inst, 0, (uint8_t)note, 0.8f, 0);
            }
        } else if (choice < 85) {                            // a key up
            if (keys > 0) {
                int k = (int)rnd((uint32_t)keys);

                for (int n = 0; n < 128; n++) {
                    if (held[n] && (k-- == 0)) {
                        held[n]  = false;
                        released = true;
                        d->cb.noteOff(inst, 0, (uint8_t)n, 0.0f, 0);
                        break;
                    }
                }
            }
        } else if (choice < 90) {                            // the pedal
            pedal = !pedal;
            d->cb.setParam(inst, kP2ParamSustain, pedal ? 1.0 : 0.0);
        } else {                                             // Prev / Next or a program change
            int to = first + (int)rnd((uint32_t)(last - first + 1));

            d->cb.setParam(inst, kP2ParamBank, (double)((to - 1) / 100) / 127.0);
            d->cb.setParam(inst, kP2ParamProgram, (double)((to - 1) % 100) / 127.0);
        }

        // a few blocks, so the fade can finish and the voices follow
        int blocks = 1 + (int)rnd(40);

        for (int b = 0; b < blocks; b++) {
            bool attackBefore[POLY_VOICES_MAX];

            for (int i = 0; i < POLY_VOICES_MAX; i++) {
                tP2Plugin * v = poly_engine(p, i);

                attackBefore[i] = (v != NULL) && (v->ampEnv.stage == eP2EnvAttack);
            }
            d->cb.process(inst, NULL, 0, outs, 2, BLOCK, NULL);

            // the host's echo, now and then a block late
            if ((gEchoCount > 0) && (rnd(3) != 0)) {
                int count = gEchoCount;

                gEchoCount = 0;

                for (int k = 0; k < count; k++) {
                    d->cb.setParam(inst, gEchoId[k], gEchoValue[k]);
                }
            }

            int nowKeys = 0;

            for (int n = 0; n < 128; n++) {
                nowKeys += held[n] ? 1 : 0;
            }
            int voices = poly_voices(p);
            int arp    = arp_on(p) ? plain_int(p, kP2ParamArpActive) : eP2ArpOff;
            int shown  = atomic_load(&p->programShown);

            // no keys, no pedal, no arpeggiator: every engine must be empty and no envelope may start
            if ((nowKeys == 0) && !pedal && (arp == eP2ArpOff)) {
                for (int i = 0; i < POLY_VOICES_MAX; i++) {
                    tP2Plugin * v = poly_engine(p, i);

                    if (v == NULL) {
                        continue;
                    }

                    if (v->noteCount > 0) {
                        char what[120];

                        snprintf(what, sizeof(what), "voice %d still lists %d note(s), first %d, with no key down", i, v->noteCount, v->notes[0]);
                        fail(e, what, shown, voices, arp);
                    }

                    if (p->poly[i].held) {
                        char what[120];

                        snprintf(what, sizeof(what), "voice %d marked held (note %d) with no key down", i, p->poly[i].note);
                        fail(e, what, shown, voices, arp);
                    }

                    if (!attackBefore[i] && (v->ampEnv.stage == eP2EnvAttack)) {
                        char what[120];

                        snprintf(what, sizeof(what), "voice %d started a note with no key down%s", i, released ? " (just after a release)" : "");
                        fail(e, what, shown, voices, arp);
                    }
                }
            }

            // the arpeggiator on (not HOLD) with no keys: it must have none either
            if ((nowKeys == 0) && !pedal && (arp == eP2ArpOn) && (p->arp.keyCount > 0)) {
                fail(e, "arpeggiator ON still lists keys with none down", shown, voices, arp);
            }
        }
    }

    printf("%d events, %d failures\n", events, gFailures);
    return (gFailures > 0) ? 1 : 0;
}
