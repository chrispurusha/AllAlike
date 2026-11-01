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

typedef enum {
    eTiEnvIdle = 0,
    eTiEnvAttack,
    eTiEnvDecay,
    eTiEnvSustain,
    eTiEnvGlide,      // notes §6: to a sustain level changed while it is held
    eTiEnvRelease
} tSn2EnvStage;

// notes §1: the four controls as per-sample steps at one sample rate
typedef struct {
    double attackStep;     // added
    double decayMul;       // multiplied, down to sustain
    double sustain;        // level
    double releaseMul;
    double glideStep;
} tSn2EnvShape;

typedef struct {
    tSn2EnvStage stage;
    double       level;    // 0..1, the amplifier's gain
    double       held;     // the sustain level the sustain stage started from
} tSn2Env;

// The panel's 0..127 for Attack, Decay, Sustain and Release; squared: the amplifier's Sustain (notes §3)
void sn2_env_shape(double attack, double decay, double sustain, double release, bool squared, double sampleRate, tSn2EnvShape * shape);
void sn2_env_gate(tSn2Env * env, bool on);                      // on: attack from where it is; off: release
double sn2_env_next(tSn2Env * env, const tSn2EnvShape * shape); // one sample on

static inline bool sn2_env_idle(const tSn2Env * env) {
    return env->stage == eTiEnvIdle;
}

#endif // __TI_ENVELOPE_H__
