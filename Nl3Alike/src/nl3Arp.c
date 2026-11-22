/*
 * Nl3Alike - the arpeggiator: the held keys stepped through as the instrument does, Sub Arpeggio and Mask included.
 *
 * Copyright (C) 2026 Chris Turner <chris_purusha@icloud.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
// Notes: Docs/code-notes/nl3Arp.md - "// notes §k" refers there.

#include <math.h>
#include <string.h>

#include "nl3Arp.h"

#define TICK_HZ        (4096.0)          // notes §2: the instrument's step clock
#define RANDOM_SEED    (0x1267u)         // notes §4
#define RANDOM_TAPS    (0x1D872B41u)

static const uint16_t kRateTicks[24] = {   // notes §2: an octave of step lengths, scaled by 100 and shifted
    32768, 33728, 34716, 35734, 36781, 37859, 38968, 40110, 41285, 42495, 43740, 45022,
    46341, 47699, 49097, 50535, 52016, 53540, 55109, 56724, 58386, 60097, 61858, 63670,
};
static const double   kSyncBeats[10] = {4.0, 2.0, 4.0 / 3.0, 1.0, 2.0 / 3.0, 0.5, 1.0 / 3.0, 0.25, 1.0 / 6.0, 0.125}; // notes §2

static int sync_index(int divisor) {
    return (divisor < 10) ? 0 : ((divisor < 26) ? 1 : ((divisor / 13 > 9) ? 9 : (divisor / 13)));
}

// notes §2: the free-running step, in ticks of 1/4096 s
static uint32_t rate_ticks(int rate) {
    int      x     = 127 - ((rate < 0) ? 0 : ((rate > 127) ? 127 : rate));
    uint32_t ticks = (uint32_t)kRateTicks[x % 24] * 100u;
    int      shift = (x / 24) - 15;

    ticks = (shift < 0) ? (ticks >> -shift) : (ticks << shift);
    return (ticks > 0xFFFFu) ? 0xFFFFu : ticks;
}

int nl3_arp_division(int divisor) {
    return sync_index(divisor);
}

double nl3_arp_step_seconds(const tNl3ArpSettings * s, double tempo) {
    if (s->clockSync) {
        return kSyncBeats[sync_index(s->divisor)] * 60.0 / ((tempo > 0.0) ? tempo : 120.0);
    }
    return (double)rate_ticks((int)lround(s->rate)) / TICK_HZ;
}

void nl3_arp_init(tNl3Arp * a) {
    memset(a, 0, sizeof(*a));
    a->sounding = -1;
    a->randomAt = -1;
    a->random   = RANDOM_SEED;
}

static uint16_t next_random(tNl3Arp * a) {   // notes §4: a 32-bit shift register; its low 16 bits are used
    bool carry = (a->random & 0x80000000u) != 0;

    a->random = (a->random == 0) ? RANDOM_SEED : a->random;
    a->random = (a->random << 1) ^ (carry ? RANDOM_TAPS : 0u);
    return (uint16_t)a->random;
}

static int scaled_random(tNl3Arp * a, int n) {
    return (int)(((uint32_t)next_random(a) * (uint32_t)n) >> 16);
}

// notes §3, §4: the held key and octave this step plays, as an index into the keys lowest first, octave by octave
static int step_index(tNl3Arp * a, const tNl3ArpSettings * s, int * octave) {
    int n       = a->keys;
    int octaves = (s->octaves < 1) ? 1 : ((s->octaves > 4) ? 4 : s->octaves);
    int total   = n * octaves;
    int range   = (s->subRange < 1) ? 1 : ((s->subRange > 8) ? 8 : s->subRange);
    int length, width, reach, positions, half, type;

    // the Sub Arpeggio's length over a window, the width its directions turn on, and how far it reaches past its start
    if (!s->sub) {
        length = 1;
        width  = (s->subDirection == eNl3ArpUpDown) ? 0 : 1;
        reach  = 0;
    } else if (s->subDirection == eNl3ArpUpDown) {
        length = ((2 * range) - 1 < (2 * n) - 1) ? ((2 * range) - 1) : ((2 * n) - 1);
        width  = length >> 1;
        reach  = width;
    } else if (s->subDirection == eNl3ArpStill) {
        length = range;
        width  = 1;
        reach  = 0;
    } else {
        length = (range < n) ? range : n;
        width  = length;
        reach  = length - 1;
    }

    // the main pattern: Mono with a note priority plays only the lowest or highest key
    if (((s->voiceMode == 1) || (s->voiceMode == 2)) && (s->priority != 0)) {
        type      = (s->priority == 2) ? -2 : -1;
        positions = 1;
    } else if (s->direction == eNl3ArpUpDown) {
        type      = eNl3ArpUpDown;
        positions = (total < 2) ? total : ((2 * total) - 2);
        positions = ((2 * reach) < positions) ? (positions - (2 * reach)) : positions;
    } else {
        type      = s->direction;
        positions = (reach < total) ? (total - reach) : total;
    }
    half = (reach < total) ? (total - reach) : total;

    int cycle    = (length * positions) & 0xFF;   // the instrument keeps these in bytes
    int at       = (cycle != 0) ? (int)(((uint32_t)a->count % (uint32_t)cycle) & 0xFF) : 0;
    int position = (cycle != 0) ? (at / length) : 0;
    int sub      = (cycle != 0) ? (at % length) : 0;
    int offset, place;

    switch (s->sub ? s->subDirection : eNl3ArpUp) {
        case eNl3ArpDown:
            offset = width - 1 - sub;
            break;

        case eNl3ArpUpDown:
            offset = (width < sub) ? ((2 * width) - sub) : sub;
            break;

        case eNl3ArpRandom:
            offset = scaled_random(a, width);
            break;

        case eNl3ArpStill:
            offset = 0;
            break;

        default:
            offset = sub;
            break;
    }

    switch (type) {
        case -2:
            place = half - 1;
            break;

        case -1:
            place = 0;
            break;

        case eNl3ArpDown:
            place = half - 1 - position;
            break;

        case eNl3ArpUpDown:
            place = (half <= position) ? ((2 * half) - position - 2) : position;
            break;

        case eNl3ArpRandom:

            if (position != a->randomAt) {
                a->randomIndex = scaled_random(a, half);
                a->randomAt    = position;
            }
            place = a->randomIndex;
            break;

        default:
            place = position;
            break;
    }
    int index = (((place + offset) % total) + total) % total;

    *octave = index / n;
    return index % n;
}

// notes §5: host frames from now to the clock's next line of the step's note value - now, if it is on one
static double to_grid(const tNl3ArpSettings * s, const tNl3ArpClock * c, bool after) {
    double beats = kSyncBeats[sync_index(s->divisor)];
    double lines = c->beats / beats;
    double next  = after ? (floor(lines + 1e-6) + 1.0) : ceil(lines - 1e-6);

    return ((next * beats) - c->beats) * 60.0 / c->tempo * c->hostRate;
}

// notes §5: the next step - with Clock on, the next line of the clock's grid
static void schedule(tNl3Arp * a, const tNl3ArpSettings * s, const tNl3ArpClock * c) {
    if (s->clockSync && c->locked && (c->tempo > 0.0)) {
        a->untilStep = to_grid(s, c, true);
        return;
    }
    a->untilStep += nl3_arp_step_seconds(s, c->tempo) * c->hostRate;
}

void nl3_arp_step(tNl3Arp * a, const tNl3ArpSettings * s, const tNl3ArpClock * c, tNl3ArpNote play, void * user) {
    if (!a->running) {
        return;
    }
    int  length = (s->maskLength < 1) ? 1 : ((s->maskLength > NL3_ARP_MASK_STEPS) ? NL3_ARP_MASK_STEPS : s->maskLength);
    bool heard  = !s->mask || (((s->maskOn >> ((uint32_t)a->count % (uint32_t)length)) & 1u) != 0);

    if (heard && (a->keys > 0)) {   // notes §6: a muted step does nothing at all - the note before it plays on
        int octave = 0;
        int key    = step_index(a, s, &octave);
        int note   = a->key[key] + (12 * octave);
        int was    = a->sounding;

        note = (note > 127) ? 127 : note;

        if ((s->voiceMode != 0) && (note != was)) {   // notes §3: mono and legato start the new note before ending the old
            play(user, (uint8_t)note, a->velocity[key], true);

            if (was >= 0) {
                play(user, (uint8_t)was, 0.0f, false);
            }
            a->sounding = note;
        } else if (((s->voiceMode != 2) && (s->voiceMode != 3)) || (note != was)) {   // legato holds the same note
            if (was >= 0) {
                play(user, (uint8_t)was, 0.0f, false);
            }
            play(user, (uint8_t)note, a->velocity[key], true);
            a->sounding = note;
        }
    }
    a->count++;
    schedule(a, s, c);
}

void nl3_arp_key(tNl3Arp * a, const tNl3ArpSettings * s, const tNl3ArpClock * c, uint8_t note, float velocity, bool on, tNl3ArpNote play, void * user) {
    int  at   = 0;

    while ((at < a->keys) && (a->key[at] < note)) {
        at++;
    }
    bool held = (at < a->keys) && (a->key[at] == note);

    if (on && !held && (a->keys < NL3_ARP_KEYS)) {
        memmove(&a->key[at + 1], &a->key[at], (size_t)(a->keys - at) * sizeof(a->key[0]));
        memmove(&a->velocity[at + 1], &a->velocity[at], (size_t)(a->keys - at) * sizeof(a->velocity[0]));
        a->key[at]      = note;
        a->velocity[at] = velocity;
        a->keys++;
    } else if (!on && held) {
        memmove(&a->key[at], &a->key[at + 1], (size_t)(a->keys - at - 1) * sizeof(a->key[0]));
        memmove(&a->velocity[at], &a->velocity[at + 1], (size_t)(a->keys - at - 1) * sizeof(a->velocity[0]));
        a->keys--;
    }

    if (on && !a->running && (a->keys > 0)) {   // notes §5: the first key starts it - at once, or with Clock on at the grid's next line
        a->running   = true;
        a->count     = 0;
        a->randomAt  = -1;
        a->untilStep = (s->clockSync && c->locked && (c->tempo > 0.0)) ? to_grid(s, c, false) : 0.0;

        if (a->untilStep <= 0.0) {
            a->untilStep = 0.0;
            nl3_arp_step(a, s, c, play, user);
        }
    } else if (a->keys == 0) {
        nl3_arp_stop(a, play, user);
    }
}

uint32_t nl3_arp_frames_to_step(const tNl3Arp * a) {
    if (!a->running) {
        return UINT32_MAX;
    }
    return (a->untilStep <= 0.0) ? 0u : (uint32_t)ceil(a->untilStep);
}

void nl3_arp_elapse(tNl3Arp * a, uint32_t frames) {
    if (a->running) {
        a->untilStep -= (double)frames;
    }
}

void nl3_arp_stop(tNl3Arp * a, tNl3ArpNote play, void * user) {
    if (a->sounding >= 0) {
        play(user, (uint8_t)a->sounding, 0.0f, false);
    }
    a->sounding = -1;
    a->running  = false;
    a->keys     = 0;
}
