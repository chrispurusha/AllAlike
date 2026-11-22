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
#include "sn2Fixed.h"

#define SILENT       (1.0 / 65536.0) // notes §4
#define NEAR         (1.0 / 128.0)   // notes §3: the decay is done
#define CATCH_UP     (1.0 / 32.0)    // notes §3: a step towards a sustain level moved while held
#define HOLD_DONE    (0.5)

void sn2_env_shape(const tSn2EnvDials * dials, bool amplifier, bool oneShot, int note, tSn2EnvShape * shape) {
    int moved = sn2_fixed_key_moves(note, dials->track, false);   // notes §5

    shape->delay   = sn2_fixed_env_step(eSn2EnvDelay, dials->delay, 0);
    shape->attack  = sn2_fixed_env_step(eSn2EnvAttack, dials->attack, sn2_fixed_key_moves(note, dials->track, true));
    shape->decay   = sn2_fixed_env_step(eSn2EnvDecay, dials->decay, moved);
    shape->release = sn2_fixed_env_step(amplifier ? eSn2EnvRelease : eSn2EnvDecay, dials->release, moved);
    shape->hold    = sn2_fixed_env_step(eSn2EnvTime, dials->time, moved);
    shape->rate    = sn2_fixed_env_step(eSn2EnvRate, dials->rate, moved);
    shape->sustain = dials->sustain / 128.0;
    shape->sustain = amplifier ? (shape->sustain * shape->sustain) : shape->sustain;
    shape->oneShot = oneShot;
}

void sn2_env_start(tSn2Env * env, int repeats) {
    env->stage  = eTiEnvAttack;
    env->waited = 0.0;
    env->held   = 0.0;
    env->again  = repeats;
    env->sample = 0;
}

void sn2_env_release(tSn2Env * env) {
    if (env->stage != eTiEnvIdle) {
        env->stage = eTiEnvSustain;
        env->held  = 1.0;
    }
}

static void attack(tSn2Env * env, const tSn2EnvShape * shape) {
    env->waited = fmin(1.0, env->waited + shape->delay);

    if (env->waited >= 1.0) {
        env->level += shape->attack;
    }

    if (env->level >= 1.0) {
        env->level = 1.0;
        env->stage = eTiEnvDecay;
    }
}

static void decay(tSn2Env * env, const tSn2EnvShape * shape) {
    double gap = (env->level - shape->sustain) * (1.0 - shape->decay);

    env->level = shape->sustain + gap;

    if (gap < NEAR) {
        env->stage = (env->again > 0) ? eTiEnvAttack : eTiEnvSustain;
        env->again--;
        env->held  = shape->oneShot ? 1.0 : 0.0;
    }
}

static void sustain(tSn2Env * env, const tSn2EnvShape * shape) {
    env->held += shape->hold;

    if (env->held >= HOLD_DONE) {
        env->level -= env->level * shape->release;

        if (env->level < SILENT) {
            env->level = 0.0;
            env->stage = eTiEnvIdle;
        }
    } else if (shape->rate < 0.0) {
        env->level += env->level * shape->rate;
    } else if (shape->rate > 0.0) {
        env->level = fmin(1.0, env->level + shape->rate);
    } else {
        double gap = env->level - shape->sustain;

        env->level = shape->sustain + (gap * (1.0 - ((fabs(gap) > NEAR) ? CATCH_UP : shape->decay)));
    }
}

double sn2_env_next(tSn2Env * env, const tSn2EnvShape * shape) {
    if ((env->sample++ % SN2_ENV_TICK) == 0) {
        switch (env->stage) {
            case eTiEnvAttack:
                attack(env, shape);
                break;

            case eTiEnvDecay:
                decay(env, shape);
                break;

            case eTiEnvSustain:
                sustain(env, shape);
                break;

            case eTiEnvIdle:
            default:
                break;
        }
    }
    return env->level;
}
