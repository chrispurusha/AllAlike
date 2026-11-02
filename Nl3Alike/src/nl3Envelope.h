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
} tNl3EnvStage;

// notes §1: the four controls as per-sample steps at one sample rate
typedef struct {
    double attackStep;     // added
    double decayMul;       // multiplied, down to sustain
    double sustain;        // level
    double releaseMul;
    double glideStep;
} tNl3EnvShape;

typedef struct {
    tNl3EnvStage stage;
    double       level;    // 0..1, the amplifier's gain
    double       held;     // the sustain level the sustain stage started from
} tNl3Env;

// The panel's 0..127 for Attack, Decay, Sustain and Release; squared: the amplifier's Sustain (notes §3)
void nl3_env_shape(double attack, double decay, double sustain, double release, bool squared, double sampleRate, tNl3EnvShape * shape);
void nl3_env_gate(tNl3Env * env, bool on);                      // on: attack from where it is; off: release
double nl3_env_next(tNl3Env * env, const tNl3EnvShape * shape); // one sample on

static inline bool nl3_env_idle(const tNl3Env * env) {
    return env->stage == eTiEnvIdle;
}

#endif // __TI_ENVELOPE_H__
