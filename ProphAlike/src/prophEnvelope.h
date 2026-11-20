/*
 * ProphAlike - the envelopes.
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
// Notes: Docs/code-notes/prophEnvelope.md - "// notes §k" refers there.

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
} tProphEnvStage;

// notes §1: the four controls as per-sample steps at one sample rate
typedef struct {
    double attackStep;     // added
    double decayMul;       // multiplied, down to sustain
    double sustain;        // level
    double releaseMul;
    double glideStep;
    double gapMul;         // notes §8: the decay's own approach, which starts the hold
    double gapSustain;
    double holdSamples;    // INFINITY: held until the key is let go
    double fallMul;        // 1: a level sustain
    double riseStep;       // 0: no rise
} tProphEnvShape;

typedef struct {
    tProphEnvStage stage;
    double       level;    // 0..1, the amplifier's gain
    double       held;     // the sustain level the sustain stage started from
    double       gap;      // notes §8: decay distance still to close before the hold starts
    double       left;     // samples of hold to go, once it has started
    bool         holding;
} tProphEnv;

// The panel's 0..127 for Attack, Decay, Sustain, Release, Sustain Time and Rate; squared: the amplifier's Sustain (notes §3)
void proph_env_shape(double attack, double decay, double sustain, double release, double sustainTime, double sustainRate, bool squared, double sampleRate, tProphEnvShape * shape);
void proph_env_gate(tProphEnv * env, bool on);                      // on: attack from where it is; off: release
double proph_env_next(tProphEnv * env, const tProphEnvShape * shape); // one sample on

static inline bool proph_env_idle(const tProphEnv * env) {
    return env->stage == eTiEnvIdle;
}

#endif // __TI_ENVELOPE_H__
