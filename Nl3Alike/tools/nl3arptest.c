// Nl3Alike - checks the arpeggiator's note order against sequences taken from the instrument (arp notes §7).
// Build: clang -Wno-initializer-overrides -I../src nl3arptest.c ../src/nl3Arp.c -lm
#include <stdio.h>
#include <string.h>

#include "nl3Arp.h"

static char gOut[1024];
static int  gOffs;

static void note(void * user, uint8_t n, float velocity, bool on) {
    (void)user;
    (void)velocity;

    gOffs += !on;

    if (on) {
        snprintf(gOut + strlen(gOut), sizeof(gOut) - strlen(gOut), "%s%d", gOut[0] ? " " : "", n);
    }
}

typedef struct {
    const char *    name;
    tNl3ArpSettings s;
    int             keys[4], count;
    int             steps;
    const char *    want;          // the instrument's, from the start
    int             addAt, addKey; // a key added before this step, or 0
} tCase;

#define S(...)    ((tNl3ArpSettings){.run = true, .octaves = 1, .subRange = 2, .maskLength = 16, .maskOn = 0xFFFF, __VA_ARGS__})

int main(void) {
    const tCase cases[] = {
        {"up",                       S(.direction =    0), {60, 64, 67}, 3, 12, "60 64 67 60 64 67 60 64 67 60 64 67",                   0,  0},
        {"down",                     S(.direction =    1), {60, 64, 67}, 3, 11, "60 64 60 67 64 60 67 64 60 67 64",                      0,  0},
        {"up/down",                  S(.direction =    2), {60, 64, 67}, 3, 12, "60 64 67 64 60 64 67 64 60 64 67 64",                   0,  0},
        {"up 2 oct",                 S(.direction =     0, .octaves = 2),{60, 64, 67}, 3, 12, "60 64 67 72 76 79 60 64 67 72 76 79",                   0,  0},
        {"up/down 2 oct",            S(.direction =     2, .octaves = 2),{60, 64, 67}, 3, 14, "60 64 67 72 76 79 76 72 67 64 60 64 67 72",             0,  0},
        {"sub up 2",                 S(.sub       = true), {60, 64,67, 69}, 4, 12, "60 64 64 67 67 69 60 64 64 67 67 69",                   0,  0},
        {"sub down 2",               S(.sub       = true,  .subDirection = 1),{60, 64, 67, 69}, 4, 12, "60 60 67 64 69 67 64 60 67 64 69 67",               0,  0},
        {"sub up/down 2",            S(.sub       = true,  .subDirection = 2),{60, 64, 67, 69}, 4, 12, "60 64 60 64 67 64 67 69 67 60 64 60",               0,  0},
        {"sub still 2",              S(.sub       = true,  .subDirection = 4),{60, 64, 67, 69}, 4, 12, "60 60 64 64 67 67 69 69 60 60 64 64",               0,  0},
        {"sub up 4",                 S(.sub       = true,  .subRange = 4),{60, 64, 67, 69}, 4, 8, "60 64 67 69 60 64 67 69",                            0,  0},
        {"mask 1010",                S(.mask      = true,  .maskOn = 0xAAAA),{60, 64, 67, 69}, 4, 8, "64 69 64 69",                                        0,  0},
        {"mask len 3 rest 2",        S(.mask      = true,  .maskLength = 3, .maskOn = 0xFFFD),{60, 64, 67, 69}, 4, 12, "60 67 69 64 67 60 64 69",                           0,  0},
        {"3 keys len 4 rest 4",      S(.mask      = true,  .maskLength = 4, .maskOn = 0xFFF7),{60, 64, 67}, 3, 12, "60 64 67 64 67 60 67 60 64",                            0,  0},
        {"down + sub up 2",          S(.direction =     1, .sub = true),{60, 64, 67, 69}, 4, 12, "60 69 64 67 60 64 67 69 64 67 60 64",               0,  0},
        {"up/down + sub down 2",     S(.direction =     2, .sub = true, .subDirection = 1),{60, 64, 67, 69}, 4, 12, "60 60 67 64 69 67 67 64 64 60 67 64",               0,  0},
        {"up 2 oct + sub up/down 3", S(.octaves   =     2, .sub = true, .subDirection = 2, .subRange = 3),{60, 64, 67, 69}, 4, 18, "60 64 67 64 60 64 67 69 67 64 67 69 72 69 67 69 72 76", 0, 0},
        {"down 2 oct + sub up 3",    S(.direction =     1, .octaves = 2, .sub = true, .subRange = 3),{60, 64, 67, 69}, 4, 18, "72 79 81 72 76 79 69 72 76 67 69 72 64 67 69 60 64 67", 0, 0},
        {"mono, priority 1",         S(.voiceMode =     1, .priority = 1),{60, 64, 67, 69}, 4, 6, "60 60 60 60 60 60",                                  0,  0},
        {"mono, priority 2",         S(.voiceMode =     1, .priority = 2),{60, 64, 67, 69}, 4, 6, "60 69 69 69 69 69",                                  0,  0},
        {"key added",                S(.direction =    0), {60, 64, 67}, 3,  9, "60 64 67 60 64 67 67 72 60",                            6, 72},
    };
    int         fails = 0;

    for (size_t c = 0; c < sizeof(cases) / sizeof(cases[0]); c++) {
        const tCase * k     = &cases[c];
        tNl3Arp       a;
        tNl3ArpClock  clock = {48000.0, 120.0, false, 0.0};

        nl3_arp_init(&a);
        gOut[0] = '\0';
        gOffs   = 0;

        for (int i = 0; i < k->count; i++) {   // one at a time, as over MIDI; the first starts the first step
            nl3_arp_key(&a, &k->s, &clock, (uint8_t)k->keys[i], 0.8f, true, note, NULL);
        }

        for (int step = 1; step < k->steps; step++) {
            if ((k->addAt != 0) && (step == k->addAt)) {
                nl3_arp_key(&a, &k->s, &clock, (uint8_t)k->addKey, 0.8f, true, note, NULL);
            }
            nl3_arp_step(&a, &k->s, &clock, note, NULL);
        }

        bool          ok    = strcmp(gOut, k->want) == 0;

        fails  += !ok;
        printf("%s  %-20s %s%s%s\n", ok ? "PASS" : "FAIL", k->name, gOut, ok ? "" : "\n      instrument: ", ok ? "" : k->want);
    }

    {   // notes §6: a muted step holds the note before it - 8 steps of 1010 end only 3 notes
        tNl3Arp         a;
        tNl3ArpClock    clock = {48000.0, 120.0, false, 0.0};
        tNl3ArpSettings m     = S(.mask = true, .maskOn = 0xAAAA);

        nl3_arp_init(&a);
        gOut[0] = '\0';
        gOffs   = 0;

        for (int k = 0; k < 4; k++) {
            nl3_arp_key(&a, &m, &clock, (uint8_t)(60 + (k * 3)), 0.8f, true, note, NULL);
        }

        for (int step = 1; step < 8; step++) {
            nl3_arp_step(&a, &m, &clock, note, NULL);
        }

        printf("%s  %-20s %d note-offs in 8 steps (the instrument ties: 3)\n", (gOffs == 3) ? "PASS" : "FAIL", "mask tie", gOffs);
        fails  += (gOffs != 3);
    }

    {                                                                // notes §5: Clock on - a key off the beat waits for the grid's next line; the steps keep to the grid
        tNl3Arp         a;
        tNl3ArpClock    clock = {48000.0, 120.0, true, 0.3};         // 0.3 of a beat in
        tNl3ArpSettings c     = S(.clockSync = true, .divisor = 69); // 1/8

        nl3_arp_init(&a);
        gOut[0]     = '\0';
        nl3_arp_key(&a, &c, &clock, 60, 0.8f, true, note, NULL);
        uint32_t        first = nl3_arp_frames_to_step(&a);

        nl3_arp_elapse(&a, first);
        clock.beats = 0.5;
        nl3_arp_step(&a, &c, &clock, note, NULL);
        uint32_t        next  = nl3_arp_frames_to_step(&a);
        bool            ok    = (first == 4800) && (next == 12000) && (strcmp(gOut, "60") == 0);

        printf("%s  %-20s first step %u frames after the key (4800), the next %u later (12000)\n", ok ? "PASS" : "FAIL", "clock grid", first, next);
        fails      += !ok;
    }

    tNl3ArpSettings r = S(.rate = 0.0);
    printf("rate 0 / 64 / 127: %.1f / %.1f / %.1f ms (instrument 956 / 150 / 24.6)\n", 1000 * nl3_arp_step_seconds(&r, 120),
           (r.rate = 64, 1000 * nl3_arp_step_seconds(&r, 120)), (r.rate = 127, 1000 * nl3_arp_step_seconds(&r, 120)));
    return fails;
}
