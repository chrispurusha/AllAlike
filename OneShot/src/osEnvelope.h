/*
 * OneShot - a one-sample player, as a VST3 and Audio Unit instrument.
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
// Notes: Docs/code-notes/osEnvelope.md - "// notes §k" refers there.

#ifndef __OS_ENVELOPE_H__
#define __OS_ENVELOPE_H__

#include <stdbool.h>

// notes §1: the shape, in seconds and curves; hold < 0 means it holds for ever (Inf)
typedef struct {
    bool   adsr;                                  // false: AHR - attack, hold, release to silence; true: AHDSR with a hold
    double attack, hold, decay, release;
    double sustain;                               // 0..1, AHDSR only; decay is AHDSR's only too
    double attackCurve, decayCurve, releaseCurve; // -1..1, 0 straight
} tOsEnvShape;

typedef enum {
    eOsEnvAttack = 0,
    eOsEnvHold,
    eOsEnvDecay,
    eOsEnvSustain,
    eOsEnvRelease,
    eOsEnvDone
} tOsEnvStage;

typedef struct {
    tOsEnvStage stage;
    double      x;            // notes §4: how far through the stage, 0..1 - not seconds, so a changed time continues smoothly
    double      level;        // the shape's level, 0..1
    double      out;          // what is played: the level, slewed past any jump
    double      from;         // the level the release started at
    bool        started;
} tOsEnv;

void os_env_start(tOsEnv * e);
void os_env_release(tOsEnv * e);                                      // a note off: release from wherever it is
double os_env_step(tOsEnv * e, const tOsEnvShape * shape, double dt); // advances dt, returns the level
bool os_env_done(const tOsEnv * e);

// notes §2: the dials' laws, shared by the plug-in, the panel's text and its drawing
double os_env_time(double normalized);                  // 0 .. 10 s
double os_env_attack_time(double normalized);           // notes §2a: 0 .. 10 s, fine at the bottom
double os_env_curve(double normalized);                 // -1 .. 1

#endif // __OS_ENVELOPE_H__
