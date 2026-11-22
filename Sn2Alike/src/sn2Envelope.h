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

#ifndef __TI_ENVELOPE_H__
#define __TI_ENVELOPE_H__

#include <stdbool.h>
#include <stdint.h>

typedef enum {
    eTiEnvIdle = 0,
    eTiEnvAttack,
    eTiEnvDecay,
    eTiEnvSustain     // notes §3: the hold, the fall or the rise, and at its end the release
} tSn2EnvStage;

#define SN2_ENV_TICK    (4)   // notes §1: samples at the engine's rate

// notes §2: a step of every control, as fractions of full scale
typedef struct {
    double delay;         // added to the delay's clock
    double attack;        // added
    double decay;         // of the distance to sustain
    double release;       // of the level
    double sustain;
    double hold;          // added to the hold's clock; 0 holds until the key is let go
    double rate;          // below 0 of the level, above 0 added
    bool   oneShot;
} tSn2EnvShape;

typedef struct {
    tSn2EnvStage stage;
    double       level;   // 0..1
    double       waited;  // the delay's clock
    double       held;    // the hold's clock
    int          again;   // repeats to go
    uint32_t     sample;
} tSn2Env;

// The panel's dials: Delay, Attack, Decay, Sustain, Release, Sustain Time and Rate; amplifier: the first envelope
typedef struct {
    double delay, attack, decay, sustain, release, time, rate;
    int    track;         // 64 none: how a key moves the times
} tSn2EnvDials;

void sn2_env_shape(const tSn2EnvDials * dials, bool amplifier, bool oneShot, int note, tSn2EnvShape * shape);
void sn2_env_start(tSn2Env * env, int repeats);                 // from where it is
void sn2_env_release(tSn2Env * env);
double sn2_env_next(tSn2Env * env, const tSn2EnvShape * shape); // one sample on

static inline bool sn2_env_idle(const tSn2Env * env) {
    return env->stage == eTiEnvIdle;
}

#endif // __TI_ENVELOPE_H__
