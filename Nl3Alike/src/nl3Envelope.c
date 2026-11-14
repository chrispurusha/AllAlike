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
// Notes: Docs/code-notes/nl3Envelope.md - "// notes §k" refers there.

#include <math.h>

#include "nl3Envelope.h"

#define STEP_HZ          (22050.0)           // notes §2: the envelopes step once every two samples
#define TIME_OFFSET      (40.15)             // notes §2: steps of 1/e = scale x (dial + offset)^8
#define ATTACK_SCALE     (1.013266e-12)
#define DECAY_SCALE      (3.540686e-13)      // Decay and Release
#define ATTACK_TARGET    (1.25)              // notes §1: the attack heads past the top, and stops there
#define SILENT           (1.0 / 65536.0)     // notes §5
#define REPEAT_TARGET    (-0.05)             // notes §6

// notes §2: the fraction of the distance closed each sample at the engine's rate; Decay and Release 127 hold
static double stage_k(double dial, double scale, bool holdsAtTop, double sampleRate) {
    if (holdsAtTop && (dial >= 127.0)) {
        return 0.0;
    }
    double steps = scale * pow(dial + TIME_OFFSET, 8.0);

    return 1.0 - exp(-STEP_HZ / (steps * sampleRate));
}

void nl3_env_shape(double attack, double decay, double sustain, double release, bool squared, bool exponential, double sampleRate,
                   tNl3EnvShape * shape) {
    double level = (sustain >= 127.0) ? 1.0 : ((sustain / 128.0) * (squared ? (sustain / 128.0) : 1.0));   // notes §3

    shape->attackK  = stage_k(attack, ATTACK_SCALE, false, sampleRate);
    shape->decayK   = stage_k(decay, DECAY_SCALE, true, sampleRate);
    shape->releaseK = stage_k(release, DECAY_SCALE, true, sampleRate);
    shape->cubed    = exponential;
    shape->repeat   = false;
    shape->sustain  = exponential ? cbrt(level) : level;   // notes §4
}

void nl3_env_gate(tNl3Env * env, bool on) {
    if (on) {
        env->stage = eNl3EnvAttack;
    } else if (env->stage != eNl3EnvIdle) {
        env->stage = eNl3EnvRelease;
    }
}

double nl3_env_next(tNl3Env * env, const tNl3EnvShape * shape) {
    switch (env->stage) {
        case eNl3EnvAttack:
            env->state += shape->attackK * (ATTACK_TARGET - env->state);

            if (env->state >= 1.0) {
                env->state = 1.0;
                env->stage = eNl3EnvDecay;
            }
            break;

        case eNl3EnvDecay:
            env->state += shape->decayK * ((shape->repeat ? REPEAT_TARGET : shape->sustain) - env->state);

            // notes §6
            if (shape->repeat && (env->state <= 0.0)) {
                env->state = 0.0;
                env->stage = eNl3EnvAttack;
            }
            break;

        case eNl3EnvRelease:
            env->state -= shape->releaseK * env->state;
            break;

        case eNl3EnvIdle:
        default:
            break;
    }
    env->level = shape->cubed ? (env->state * env->state * env->state) : env->state;

    // notes §5
    if ((env->stage == eNl3EnvRelease) && (env->level < SILENT)) {
        env->state = 0.0;
        env->level = 0.0;
        env->stage = eNl3EnvIdle;
    }
    return env->level;
}
