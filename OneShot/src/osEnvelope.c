/*
 * OneShot - a one-sample player, as a VST3 and Audio Unit instrument.
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
// Notes: Docs/code-notes/osEnvelope.md - "// notes §k" refers there.

#include <math.h>

#include "osEnvelope.h"

#define MAX_TIME_S       (10.0)
#define CURVE_K          (6.0)       // notes §3: how far a full curve bends
#define MIN_RELEASE_S    (0.001)     // a release of 0 would click
#define ATTACK_KNEE_S    (0.001)     // notes §2a
#define SLEW_S           (0.0005)    // notes §4: the fastest the level may move from 0 to 1

// notes §3: progress through a segment, x 0..1 -> 0..1, bent by c: c > 0 moves fast first, c < 0 slow first
static double bend(double x, double c) {
    double k = c * CURVE_K;

    if (fabs(k) < 1e-6) {
        return x;
    }
    return (1.0 - exp(-k * x)) / (1.0 - exp(-k));
}

void os_env_start(tOsEnv * e) {
    *e = (tOsEnv){
        .stage = eOsEnvAttack
    };
}

void os_env_release(tOsEnv * e) {
    if (e->stage < eOsEnvRelease) {
        e->from  = e->out;   // from what is heard, so the release starts where the sound is
        e->stage = eOsEnvRelease;
        e->x     = 0.0;
    }
}

bool os_env_done(const tOsEnv * e) {
    return (e->stage == eOsEnvDone) && (e->out <= 0.0);
}

// A stage's length in seconds: 0 passes straight through, INFINITY never ends
static double length_of(tOsEnvStage stage, const tOsEnvShape * s) {
    switch (stage) {
        case eOsEnvAttack:  return s->attack;

        case eOsEnvHold:    return (s->hold < 0.0) ? INFINITY : s->hold;

        case eOsEnvDecay:   return s->decay;

        case eOsEnvSustain: return INFINITY;

        case eOsEnvRelease: return fmax(MIN_RELEASE_S, s->release);

        default:            return INFINITY;
    }
}

// Moves on while the stage is spent, carrying the time left over into the next
static void next_stage(tOsEnv * e, const tOsEnvShape * s) {
    while ((e->stage != eOsEnvDone) && (e->x >= 1.0)) {
        double over = (e->x - 1.0) * length_of(e->stage, s);   // seconds past the end (0 for a zero-length stage)

        if (e->stage == eOsEnvRelease) {
            e->stage = eOsEnvDone;
            break;
        }

        if ((e->stage == eOsEnvHold) && !s->adsr) {
            e->from  = 1.0;         // notes §1: AHR - the hold, at full level, ends in the release
            e->stage = eOsEnvRelease;
        } else {
            e->stage = (tOsEnvStage)(e->stage + 1);
        }
        double next = length_of(e->stage, s);

        e->x = (next <= 0.0) ? 1.0 : (isinf(next) ? 0.0 : (isnan(over) ? 0.0 : over / next));
    }
}

double os_env_step(tOsEnv * e, const tOsEnvShape * s, double dt) {
    double floor = s->adsr ? s->sustain : 0.0;

    if ((e->stage != eOsEnvDone) && (length_of(e->stage, s) <= 0.0)) {
        e->x = 1.0;
    }
    next_stage(e, s);

    switch (e->stage) {
        case eOsEnvAttack:  e->level = bend(e->x, s->attackCurve);
            break;

        case eOsEnvHold:    e->level = 1.0;
            break;

        case eOsEnvDecay:   e->level = 1.0 - ((1.0 - floor) * bend(e->x, s->decayCurve));
            break;

        case eOsEnvSustain: e->level = floor;
            break;

        case eOsEnvRelease: e->level = e->from * (1.0 - bend(e->x, s->releaseCurve));
            break;

        case eOsEnvDone:    e->level = 0.0;
            break;
    }

    // notes §4: the first value is taken as it is (an attack of 0 starts at full level); after that no jump is faster
    // than the slew, so a dial turned during a note never clicks
    if (!e->started) {
        e->out     = e->level;
        e->started = true;
    } else {
        double most = dt / SLEW_S;

        e->out += fmax(-most, fmin(most, e->level - e->out));
    }
    double length = length_of(e->stage, s);

    if ((e->stage != eOsEnvDone) && isfinite(length) && (length > 0.0)) {
        e->x += dt / length;
    }
    return e->out;
}

// notes §2: a fourth-power law, so the short times every drum wants get most of the dial
double os_env_time(double normalized) {
    double v = fmin(1.0, fmax(0.0, normalized));

    return MAX_TIME_S * v * v * v * v;
}

// notes §2a: exponential above a 1 ms knee - every step about 0.9% longer than the last, the first 5 ms a fifth of the dial
double os_env_attack_time(double normalized) {
    double v = fmin(1.0, fmax(0.0, normalized));

    return ATTACK_KNEE_S * (pow((MAX_TIME_S / ATTACK_KNEE_S) + 1.0, v) - 1.0);
}

double os_env_curve(double normalized) {
    return (fmin(1.0, fmax(0.0, normalized)) * 2.0) - 1.0;
}
