/*
 * Nl3Alike - the envelopes.
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
// Notes: Docs/code-notes/nl3Envelope.md - "// notes §k" refers there.

#include <math.h>

#include "nl3Envelope.h"

#define TOP              (1.0)
#define SETTLED          (1.0 / 65536.0)
#define DECAY_QUICKER    (0.24)          // notes §1
#define SILENT           (1.0 / 65536.0) // notes §5
#define GLIDE_S          (0.35)          // notes §6: seconds across the whole range

// notes §2: seconds, between the points a straight line in the logarithm
static double law(const double * seconds, double dial) {
    static const double kDial[] = {0, 16, 32, 48, 64, 80, 96, 112, 127};
    const int           n       = (int)(sizeof(kDial) / sizeof(kDial[0]));

    for (int k = 1; k < n; k++) {
        if (dial <= kDial[k]) {
            double t = (dial - kDial[k - 1]) / (kDial[k] - kDial[k - 1]);

            return exp(log(seconds[k - 1]) + (t * (log(seconds[k]) - log(seconds[k - 1]))));
        }
    }

    return seconds[n - 1];
}

void nl3_env_shape(double attack, double decay, double sustain, double release, bool squared, double sampleRate, tNl3EnvShape * shape) {
    static const double kAttack[] = {0.0006, 0.007, 0.021, 0.062, 0.198, 0.596, 1.913, 6.405, 13.07};    // to the top
    static const double kHalf[]   = {0.0003, 0.0016, 0.0044, 0.0144, 0.045, 0.150, 0.460, 1.42, 4.45};   // to half

    shape->attackStep = TOP / (law(kAttack, attack) * sampleRate);
    shape->decayMul   = pow(0.5, 1.0 / (law(kHalf, decay) * (1.0 - (DECAY_QUICKER * sustain / 127.0)) * sampleRate));
    shape->releaseMul = pow(0.5, 1.0 / (law(kHalf, release) * sampleRate));
    shape->sustain    = (sustain / 127.0) * (squared ? (sustain / 127.0) : 1.0);
    shape->glideStep  = TOP / (GLIDE_S * sampleRate);
}

void nl3_env_gate(tNl3Env * env, bool on) {
    if (on) {
        env->stage = eTiEnvAttack;
    } else if (env->stage != eTiEnvIdle) {
        env->stage = eTiEnvRelease;
    }
}

static void hold(tNl3Env * env, double level) {
    env->level = level;
    env->held  = level;
    env->stage = eTiEnvSustain;
}

double nl3_env_next(tNl3Env * env, const tNl3EnvShape * shape) {
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
    return env->level;
}
