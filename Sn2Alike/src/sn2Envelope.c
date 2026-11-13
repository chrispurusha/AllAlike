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

#define TOP              (1.0)
#define SETTLED          (1.0 / 65536.0)
#define DECAY_QUICKER    (0.24)          // notes §1
#define SILENT           (1.0 / 65536.0) // notes §5
#define GLIDE_S          (0.35)          // notes §6: seconds across the whole range
#define GAP_DONE         (1.0 / 128.0)   // notes §8
#define RISE_S           (0.027)
#define FALL_DB_S        (14200.0)       // notes §8: at Sustain Rate 0, a decade slower every 16 steps
#define LEVEL_RATE       (64.0)
#define FOREVER          (127.0)

// notes §2: seconds at points every `step` of the dial (the last at `last`), a straight line in the logarithm between them
static double law_points(const double * seconds, int n, double step, double last, double dial) {
    for (int k = 1; k < n; k++) {
        double at = (k == n - 1) ? last : k * step;

        if (dial <= at) {
            double from = (k - 1) * step;
            double t    = (dial - from) / (at - from);

            return exp(log(seconds[k - 1]) + (t * (log(seconds[k]) - log(seconds[k - 1]))));
        }
    }

    return seconds[n - 1];
}

static double law(const double * seconds, double dial) {
    return law_points(seconds, 9, 16.0, 127.0, dial);
}

// notes §8: the hold after the decay, 0..126; 127 holds for ever
static double hold_seconds(double dial) {
    static const double kHold[] = {
        0.0048, 0.0064, 0.0166, 0.0403, 0.0607, 0.0904,  0.136, 0.217, 0.316,
        0.4919, 0.7703, 1.1461, 1.8248, 2.7579, 4.2149, 6.8215, 8.9527
    };

    if (dial < 0.0) {
        return 0.0;   // notes §9: One Shot
    }
    return (dial >= FOREVER - 0.5) ? INFINITY : law_points(kHold, 17, 8.0, 126.0, dial);
}

void sn2_env_shape(double attack, double decay, double sustain, double release, double sustainTime, double sustainRate, bool squared, double sampleRate, tSn2EnvShape * shape) {
    static const double kAttack[] = {0.0006, 0.007, 0.021, 0.062, 0.198, 0.596, 1.913, 6.405, 13.07};    // to the top
    static const double kHalf[]   = {0.0003, 0.0016, 0.0044, 0.0144, 0.045, 0.150, 0.460, 1.42, 4.45};   // to half

    shape->attackStep  = TOP / (law(kAttack, attack) * sampleRate);
    shape->decayMul    = pow(0.5, 1.0 / (law(kHalf, decay) * (1.0 - (DECAY_QUICKER * sustain / 127.0)) * sampleRate));
    shape->releaseMul  = pow(0.5, 1.0 / (law(kHalf, release) * sampleRate));
    shape->sustain     = (sustain / 127.0) * (squared ? (sustain / 127.0) : 1.0);
    shape->glideStep   = TOP / (GLIDE_S * sampleRate);
    // notes §8
    shape->gapMul      = pow(0.5, 1.0 / (law(kHalf, decay) * sampleRate));
    shape->gapSustain  = (sustain / 128.0) * (squared ? (sustain / 128.0) : 1.0);
    shape->holdSamples = hold_seconds(sustainTime) * sampleRate;
    shape->fallMul     = (sustainRate < LEVEL_RATE - 0.5) ? pow(10.0, -FALL_DB_S * pow(10.0, -sustainRate / 16.0) / (20.0 * sampleRate)) : 1.0;
    shape->riseStep    = ((sustainRate > LEVEL_RATE + 0.5) && isfinite(shape->holdSamples)) ? TOP / (RISE_S * sampleRate) : 0.0;
}

void sn2_env_gate(tSn2Env * env, bool on) {
    if (on) {
        env->stage   = eTiEnvAttack;
        env->holding = false;
        env->gap     = -1.0;
    } else if (env->stage != eTiEnvIdle) {
        env->stage = eTiEnvRelease;
    }
}

static void hold(tSn2Env * env, double level) {
    env->level = level;
    env->held  = level;
    env->stage = eTiEnvSustain;
}

// notes §8: the decay's own clock starts the hold; then the level falls or rises at the Sustain Rate until the hold ends
static void sustain_stage(tSn2Env * env, const tSn2EnvShape * shape) {
    if (!env->holding) {
        if (env->gap < 0.0) {
            env->gap = fmax(env->level - shape->gapSustain, 0.0);
        }
        env->gap    *= shape->gapMul;

        if (env->gap > GAP_DONE) {
            return;
        }
        env->holding = true;
        env->left    = shape->holdSamples;
    }

    if (shape->riseStep > 0.0) {
        env->level = fmin(TOP, env->level + shape->riseStep);
        env->stage = eTiEnvSustain;
        env->held  = shape->sustain;
    } else if (shape->fallMul < 1.0) {
        env->level *= shape->fallMul;
        env->stage  = eTiEnvSustain;
        env->held   = shape->sustain;

        if (env->level < SILENT) {
            env->level = 0.0;
            env->stage = eTiEnvIdle;
            return;
        }
    }

    if ((env->left -= 1.0) <= 0.0) {
        env->stage = eTiEnvRelease;
    }
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
            env->level  = shape->sustain + ((env->level - shape->sustain) * shape->decayMul);

            if ((env->level - shape->sustain) <= SETTLED) {
                hold(env, shape->sustain);
            }
            break;

        case eTiEnvSustain:

            // notes §6
            if (shape->sustain != env->held) {
                env->held  = shape->sustain;
                env->stage = eTiEnvGlide;
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

    if ((env->stage == eTiEnvDecay) || (env->stage == eTiEnvSustain) || (env->stage == eTiEnvGlide)) {
        sustain_stage(env, shape);
    }
    return env->level;
}
