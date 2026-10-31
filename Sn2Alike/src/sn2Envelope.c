/*
 * Sn2Alike - the envelopes.
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
// Notes: Docs/code-notes/sn2Envelope.md - "// notes §k" refers there.

#include <math.h>

#include "sn2Envelope.h"

#define TICKS_PER_SECOND    (46875.0 / 4.0)   // notes §1
#define TOP                 (1.0)
#define RISE_TOP            (127.0 / 128.0)   // notes §4
#define FLOOR               (1.0 / 32768.0)
#define SILENT              (1.0 / 65536.0)   // notes §5
#define GLIDE_RATE          (1.0 / 4096.0)    // notes §6, per tick

// notes §2: log2 of the attack rate at each fourth step of the dial
static double attack_log2(int i) {
    if (i <= 0) {
        return 0.0;
    }

    if (i == 1) {
        return log2(0.75);
    }

    if (i <= 6) {
        return -(double)(i - 1);
    }

    if (i <= 30) {
        return -5.0 - ((i - 6) / 2.0);
    }
    return -17.0 - (i - 30);
}

// notes §3: the same for decay and release, as breakpoints in log2; past the last, no movement
static double fall_rate(int i) {
    static const double kAt[]   = {0, 4, 7, 12, 24, 28, 33, 34, 35};
    static const double kLog2[] = {-4.0, -8.0, -9.5, -11.0, -14.0, -15.2, -17.2, -17.7, -19.0};
    const int           n       = (int)(sizeof(kAt) / sizeof(kAt[0]));

    if (i > kAt[n - 1]) {
        return 0.0;
    }

    for (int k = 1; k < n; k++) {
        if (i <= kAt[k]) {
            return exp2(kLog2[k - 1] + ((kLog2[k] - kLog2[k - 1]) * (i - kAt[k - 1]) / (kAt[k] - kAt[k - 1])));
        }
    }

    return exp2(kLog2[0]);
}

static double attack_rate(int i) {
    return exp2(attack_log2(i));
}

// notes §4: the sustain level's curve, at each fourth step of the dial
static double sustain_curve(int i) {
    if (i <= 0) {
        return 0.0;
    }

    if (i < 5) {
        return 0.1 * exp2(-2.8 - (0.8 * (5 - i)));
    }

    if (i < 12) {
        return 0.1 * exp2(-0.4 * (12 - i));
    }
    return pow(10.0, (i - 32) / 20.0);
}

// Between the points, a straight line - as the instrument reads its curves
static double between(double ( *curve )(int), double index) {
    int    i = (int)floor(index);
    double f = index - i;

    return (curve(i) * (1.0 - f)) + (curve(i + 1) * f);
}

void sn2_env_shape(double attack, double decay, double sustain, double sustainTime, double release, double sampleRate,
                   tSn2EnvShape * shape) {
    double ticks = TICKS_PER_SECOND / sampleRate;   // per sample

    shape->attackStep = between(attack_rate, attack / 4.0) * ticks;
    shape->decayMul   = pow(1.0 - between(fall_rate, decay / 4.0), ticks);
    shape->releaseMul = pow(1.0 - between(fall_rate, release / 4.0), ticks);
    shape->sustain    = (between(sustain_curve, sustain / 4.0) / 2.0) + (sustain / 256.0) + FLOOR;
    shape->glideStep  = GLIDE_RATE * ticks;
    shape->slopeStep  = 0.0;
    shape->slopeMul   = 1.0;

    // notes §4: below 64 a fall, above it a rise, both off the decay curve
    if (sustainTime < 64.0) {
        shape->slopeMul = pow(1.0 - (between(fall_rate, (9.0 / 16.0) * sustainTime) / 2.0), ticks);
    } else if (sustainTime > 64.0) {
        shape->slopeStep = (between(fall_rate, (9.0 / 16.0) * (128.0 - sustainTime)) / 4.0) * ticks;
    }
}

void sn2_env_gate(tSn2Env * env, bool on) {
    if (on) {
        env->stage = eTiEnvAttack;
    } else if (env->stage != eTiEnvIdle) {
        env->stage = eTiEnvRelease;
    }
}

static void hold(tSn2Env * env, double level) {
    env->level = level;
    env->held  = level;
    env->stage = eTiEnvSustain;
}

double sn2_env_next(tSn2Env * env, const tSn2EnvShape * shape) {
    switch (env->stage) {
        case eTiEnvAttack:
            env->level += shape->attackStep;

            if (env->level >= TOP) {
                env->level = TOP;
                env->stage = eTiEnvDecay;
            }
            break;

        case eTiEnvDecay:
            env->level *= shape->decayMul;

            if (env->level <= shape->sustain) {
                hold(env, shape->sustain);
            }
            break;

        case eTiEnvSustain:

            // notes §6
            if (shape->sustain != env->held) {
                env->held  = shape->sustain;
                env->stage = eTiEnvGlide;
            } else if (shape->slopeStep > 0.0) {
                env->level = (env->level < RISE_TOP) ? fmin(RISE_TOP, env->level + shape->slopeStep) : env->level;
            } else {
                env->level = fmax(FLOOR, env->level * shape->slopeMul);
            }
            break;

        case eTiEnvGlide:

            if (env->level < shape->sustain) {
                env->level = fmin(shape->sustain, env->level + shape->glideStep);
            } else {
                env->level = fmax(shape->sustain, env->level - shape->glideStep);
            }

            if (env->level == shape->sustain) {
                hold(env, shape->sustain);
            }
            break;

        case eTiEnvRelease:
            env->level *= shape->releaseMul;

            // notes §5
            if (env->level < SILENT) {
                env->level = 0.0;
                env->stage = eTiEnvIdle;
            }
            break;

        case eTiEnvIdle:
        default:
            break;
    }
    return env->level;
}
