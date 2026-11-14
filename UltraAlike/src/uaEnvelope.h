/*
 * UltraAlike - a sampler (a placeholder on a sample player's engine), as a VST3 and Audio Unit instrument.
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
// Notes: Docs/code-notes/uaEnvelope.md - "// notes §k" refers there.

#ifndef __UA_ENVELOPE_H__
#define __UA_ENVELOPE_H__

#include <stdbool.h>

// notes §1: the shape, in seconds and curves; hold < 0 means it holds for ever (Inf)
typedef struct {
    bool   adsr;                                  // false: AHR - attack, hold, release to silence; true: AHDSR with a hold
    double attack, hold, decay, release;
    double sustain;                               // 0..1, AHDSR only; decay is AHDSR's only too
    double attackCurve, decayCurve, releaseCurve; // -1..1, 0 straight
} tUaEnvShape;

typedef enum {
    eUaEnvAttack = 0,
    eUaEnvHold,
    eUaEnvDecay,
    eUaEnvSustain,
    eUaEnvRelease,
    eUaEnvDone
} tUaEnvStage;

typedef struct {
    tUaEnvStage stage;
    double      x;            // notes §4: how far through the stage, 0..1 - not seconds, so a changed time continues smoothly
    double      level;        // the shape's level, 0..1
    double      out;          // what is played: the level, slewed past any jump
    double      from;         // the level the release started at
    bool        started;
} tUaEnv;

void ua_env_start(tUaEnv * e);
void ua_env_release(tUaEnv * e);                                      // a note off: release from wherever it is
double ua_env_step(tUaEnv * e, const tUaEnvShape * shape, double dt); // advances dt, returns the level
bool ua_env_done(const tUaEnv * e);

// notes §2: the dials' laws, shared by the plug-in, the panel's text and its drawing
double ua_env_time(double normalized);                  // 0 .. 10 s
double ua_env_attack_time(double normalized);           // notes §2a: 0 .. 10 s, fine at the bottom
double ua_env_curve(double normalized);                 // -1 .. 1

#endif // __UA_ENVELOPE_H__
