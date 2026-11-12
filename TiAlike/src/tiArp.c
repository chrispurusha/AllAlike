/*
 * TiAlike - the arpeggiator.
 *
 * Copyright (C) 2026 Chris Turner <chris_purusha@icloud.com>
 *
 * This program is free software: you can redistribute it and/or modify it under the terms of the
 * GNU General Public License as published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without
 * even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with this program. If
 * not, see <https://www.gnu.org/licenses/>.
 */
// Notes: Docs/code-notes/tiArp.md - "// notes §k" refers there.

#include <math.h>
#include <string.h>

#include "tiArp.h"

#define TICKS_PER_QUARTER    (96.0)
#define FORMAT_TI            (12)     // a Single older than this keeps its mode in page B 1
#define STEP_UNIT            (65536.0)
#define VELOCITY_CENTRE      (100)    // notes §3: a step at 100 leaves the key's velocity as it is

// notes §2: Arp Clock - ticks a step, 96 a quarter note
static const int kClockTicks[18] = {24, 3, 6, 12, 24, 48, 96, 9, 18, 36, 72, 8, 16, 32, 64, 128, 144, 192};

void ti_arp_reset(tTiArp * a) {
    memset(a, 0, sizeof(*a));
    a->last   = -1;
    a->random = 0x13579bdu;
}

int ti_arp_mode(const uint8_t * pageA, const uint8_t * pageB) {
    int mode = (pageA[0] < FORMAT_TI) ? pageB[1] : pageB[15];

    return (mode > 7) ? 0 : mode;
}

static void forget(tTiArp * a, uint32_t i) {
    memmove(&a->key[i], &a->key[i + 1], (a->keys - i - 1) * sizeof(a->key[0]));
    memmove(&a->keyVelocity[i], &a->keyVelocity[i + 1], (a->keys - i - 1) * sizeof(a->keyVelocity[0]));
    memmove(&a->keyDown[i], &a->keyDown[i + 1], (a->keys - i - 1) * sizeof(a->keyDown[0]));
    a->keys--;
}

static bool any_down(const tTiArp * a) {
    for (uint32_t i = 0; i < a->keys; i++) {
        if (a->keyDown[i]) {
            return true;
        }
    }

    return false;
}

// notes §1: keys in the order played; Hold keeps released keys until a new one goes down with none held
void ti_arp_key(tTiArp * a, const uint8_t * pageB, uint8_t note, float velocity, bool down) {
    bool hold = (pageB[4] != 0) || a->pedal;

    for (uint32_t i = 0; i < a->keys; i++) {
        if (a->key[i] == note) {
            if (down) {
                forget(a, i);
                break;
            }

            if (hold) {
                a->keyDown[i] = false;
            } else {
                forget(a, i);
            }
            return;
        }
    }

    if (!down) {
        return;
    }

    if (hold && !any_down(a)) {
        a->keys = 0;   // a new chord replaces the one held
    }

    if (a->keys < TI_ARP_KEYS) {
        a->key[a->keys]         = note;
        a->keyVelocity[a->keys] = velocity;
        a->keyDown[a->keys]     = true;
        a->keys++;
    }
}

// notes §2: the pattern's steps, the user pattern (page D) as 0
static uint32_t pattern(const uint8_t * pageB, const uint8_t * pageC, const uint8_t * pageD, tTiArpStep * user, const tTiArpStep ** steps) {
    int      p     = (pageB[2] > 63) ? 63 : pageB[2];

    if (p > 0) {
        *steps = &gTiArpSteps[gTiArpPatternStart[p - 1]];
        return (uint32_t)(gTiArpPatternStart[p] - gTiArpPatternStart[p - 1]);
    }
    uint32_t count = (uint32_t)(pageC[127] & 31u) + 1u;

    for (uint32_t k = 0; k < count; k++) {
        user[k].length   = (int32_t)lround(pageD[3 * k] * STEP_UNIT / 127.0);
        user[k].duration = (int32_t)STEP_UNIT;
        user[k].velocity = (pageD[(3 * k) + 2] & 1u) ? pageD[(3 * k) + 1] : 0;
    }

    *steps = user;
    return count;
}

// notes §2: Swing - within each pair of steps the first takes (64 + swing / 4) / 128 of the pair
static double swung(int64_t ticks, int stepTicks, int swing) {
    double w    = (64.0 + (swing / 4)) / 128.0;
    double pair = 2.0 * stepTicks;
    double at   = floor((double)ticks / pair) * pair;
    double q    = (double)ticks - at;

    return at + ((q < stepTicks) ? (q * 2.0 * w) : ((2.0 * w * stepTicks) + ((q - stepTicks) * (2.0 - (2.0 * w)))));
}

static uint32_t next_random(tTiArp * a) {
    a->random = (a->random * 1664525u) + 1013904223u;
    return a->random >> 8;
}

// notes §4: the nearest key above (or below) a note, the lowest (highest) of all with none, -1 when there is none
static int above(const tTiArp * a, int from) {
    int best = 128;

    for (uint32_t i = 0; i < a->keys; i++) {
        best = ((a->key[i] > from) && (a->key[i] < best)) ? a->key[i] : best;
    }

    return (best == 128) ? -1 : best;
}

static int below(const tTiArp * a, int from) {
    int best = -1;

    for (uint32_t i = 0; i < a->keys; i++) {
        best = ((a->key[i] < from) && (a->key[i] > best)) ? a->key[i] : best;
    }

    return best;
}

// notes §4: the key or keys a step plays, with their octave
static uint32_t choose(tTiArp * a, int mode, int range, uint8_t * notes, float * velocities) {
    int key = -1;

    switch (mode) {
        case 1:   // notes §4: Up - the lowest key above the last; past the highest, an octave up, round after the range
            key = above(a, a->last);

            if (key < 0) {
                a->octave = (a->octave >= range) ? 0 : (a->octave + 1);
                key       = above(a, -1);
            }
            break;

        case 2:
            key = below(a, a->last);

            if (key < 0) {
                a->octave = (a->octave <= 0) ? range : (a->octave - 1);
                key       = below(a, 128);
            }
            break;

        case 3:   // notes §4: Up&Down turns at either end without playing the end twice
            key = a->down ? below(a, a->last) : above(a, a->last);

            if ((key < 0) && !a->down && (a->octave < range)) {
                a->octave++;
                key = above(a, -1);
            } else if ((key < 0) && !a->down) {
                a->down = true;
                key     = below(a, a->last);
                key     = (key < 0) ? a->last : key;   // a single key: it simply repeats
            } else if ((key < 0) && (a->octave > 0)) {
                a->octave--;
                key = below(a, 128);
            } else if (key < 0) {
                a->down = false;
                key     = above(a, a->last);
                key     = (key < 0) ? a->last : key;
            }
            break;

        case 4:   // notes §4: As Played - the keys in order, then an octave up

            if (a->played >= a->keys) {
                a->played = 0;
                a->octave = (a->octave >= range) ? 0 : (a->octave + 1);
            }
            key       = a->key[a->played++];
            break;

        case 5:
            key       = a->key[next_random(a) % a->keys];
            a->octave = (int)(next_random(a) % (uint32_t)(range + 1));
            break;

        default:  // notes §4: Chord - every key at once

            for (uint32_t i = 0; i < a->keys; i++) {
                notes[i]      = a->key[i];
                velocities[i] = a->keyVelocity[i];
            }

            return a->keys;
    }

    if (key < 0) {
        return 0;
    }
    a->last  = key;

    for (uint32_t i = 0; i < a->keys; i++) {
        if (a->key[i] == key) {
            velocities[0] = a->keyVelocity[i];
        }
    }

    int note = key + (12 * a->octave);

    notes[0] = (uint8_t)((note > 127) ? 127 : note);
    return 1;
}

static uint32_t emit(tTiArpEvent * events, uint32_t n, uint32_t frame, uint8_t note, float velocity, bool on) {
    if (n < TI_ARP_EVENTS) {
        events[n] = (tTiArpEvent){
            frame, note, velocity, on
        };
        n++;
    }
    return n;
}

uint32_t ti_arp_stop(tTiArp * a, tTiArpEvent * events) {
    uint32_t n = 0;

    for (uint32_t i = 0; i < a->soundingCount; i++) {
        n = emit(events, n, 0, a->sounding[i], 0.0f, false);
    }

    a->soundingCount = 0;
    return n;
}

uint32_t ti_arp_run(tTiArp * a, const uint8_t * pageA, const uint8_t * pageB, const uint8_t * pageC, const uint8_t * pageD,
                    uint32_t frames, double rate, double bpm, tTiArpEvent * events) {
    tTiArpStep         user[32];
    const tTiArpStep * steps;
    uint32_t           count     = pattern(pageB, pageC, pageD, user, &steps);
    int                mode      = ti_arp_mode(pageA, pageB);
    int                clock     = (pageB[17] > 17) ? 4 : pageB[17];
    int                stepTicks = kClockTicks[clock];
    int                range     = pageB[3] & 3;
    double             perFrame  = (bpm * TICKS_PER_QUARTER / 60.0) / rate;
    uint32_t           n         = 0;
    double             end;

    if ((pageB[4] == 0) && !a->pedal) {   // notes §1: Hold off (and the pedal up) lets go of keys it was keeping
        for (uint32_t i = a->keys; i > 0; i--) {
            if (!a->keyDown[i - 1]) {
                forget(a, i - 1);
            }
        }
    }

    if (a->keys == 0) {
        a->running = false;
    } else if (!a->running) {   // notes §5: the first key starts the pattern there and then, from its first step
        a->running   = true;
        a->origin    = a->clock;
        a->nextTick  = 0;
        a->stepTicks = 0;
        a->entry     = 0;
        a->octave    = 0;
        a->last      = (mode == 2) ? 128 : -1;
        a->octave    = (mode == 2) ? range : 0;
        a->down      = false;
        a->played    = 1;     // notes §4: As Played steps on before it plays
    }
    a->tick = a->clock - a->origin;
    end     = a->tick + (perFrame * frames);

    for (int guard = 0; guard < 256; guard++) {
        double             stepAt = a->running ? swung(a->nextTick, stepTicks, pageB[6]) : INFINITY;
        double             offAt  = (a->soundingCount > 0) ? a->offTick : INFINITY;
        double             at     = (offAt <= stepAt) ? offAt : stepAt;

        if (at >= end) {
            break;
        }
        uint32_t           frame  = (at <= a->tick) ? 0u : (uint32_t)((at - a->tick) / perFrame);

        frame            = (frame >= frames) ? (frames - 1u) : frame;

        if (offAt <= stepAt) {
            for (uint32_t i = 0; i < a->soundingCount; i++) {
                n = emit(events, n, frame, a->sounding[i], 0.0f, false);
            }

            a->soundingCount = 0;
            continue;
        }
        const tTiArpStep * st     = &steps[a->entry % count];

        a->stepTicks     = a->nextTick;
        a->nextTick     += (int64_t)floor(((double)st->duration * stepTicks / STEP_UNIT) + (1.0 / 64.0)); // notes §2: whole ticks, cut short
        a->entry         = (a->entry + 1u) % count;

        if ((st->velocity < 1) || (a->keys == 0)) {
            continue;
        }
        uint8_t            notes[TI_ARP_KEYS];
        float              vels[TI_ARP_KEYS];
        uint32_t           got    = choose(a, mode, range, notes, vels);

        if (got == 0) {
            continue;
        }

        for (uint32_t i = 0; i < a->soundingCount; i++) {   // the last step's notes stop as the next starts
            n = emit(events, n, frame, a->sounding[i], 0.0f, false);
        }

        a->soundingCount = 0;

        for (uint32_t i = 0; i < got; i++) {
            // notes §3: the key's velocity, moved by the step's from 100
            double v = (vels[i] * 127.0) + st->velocity - VELOCITY_CENTRE;

            v                               = (v < 1.0) ? 1.0 : ((v > 127.0) ? 127.0 : v);
            n                               = emit(events, n, frame, notes[i], (float)(v / 127.0), true);
            a->sounding[a->soundingCount++] = notes[i];
        }

        // notes §3: the gate - the step's length times Note Length / 64, in whole ticks, at least one
        double gate = fmax(1.0, (double)lround(st->length * stepTicks / STEP_UNIT * pageB[5] / 64.0));

        a->offTick       = stepAt + gate;
    }

    a->clock += perFrame * frames;
    a->tick   = end;
    return n;
}
