/*
 * TiAlike - the envelopes.
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
// Notes: Docs/code-notes/tiEnvelope.md - "// notes §k" refers there.

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
} tTiEnvStage;

// notes §1: the five controls as per-sample steps at one sample rate
typedef struct {
    double attackStep;     // added
    double decayMul;       // multiplied, down to sustain
    double sustain;        // level
    double slopeStep;      // notes §4: rising, added up to the top; 0 when falling or held
    double slopeMul;       // falling, multiplied down to the floor; 1 when rising or held
    double releaseMul;
    double glideStep;
} tTiEnvShape;

typedef struct {
    tTiEnvStage stage;
    double      level;     // 0..1, the amplifier's gain
    double      held;      // the sustain level the sustain stage started from
} tTiEnv;

// The panel's 0..127 for Attack, Decay, Sustain, Sustain Time (64 holds) and Release
void ti_env_shape(double attack, double decay, double sustain, double sustainTime, double release, double sampleRate, tTiEnvShape * shape);
void ti_env_gate(tTiEnv * env, bool on);                     // on: attack from where it is; off: release
double ti_env_next(tTiEnv * env, const tTiEnvShape * shape); // one sample on

static inline bool ti_env_idle(const tTiEnv * env) {
    return env->stage == eTiEnvIdle;
}

#endif // __TI_ENVELOPE_H__
