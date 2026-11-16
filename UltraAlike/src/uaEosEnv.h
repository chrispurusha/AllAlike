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
// Notes: Docs/code-notes/uaEosEnv.md - "// notes §k" refers there.

#ifndef __UA_EOS_ENV_H__
#define __UA_EOS_ENV_H__

#include <stdbool.h>
#include <stdint.h>

#include "emuBank.h"

#define UA_EOS_STAGES    (6)   // Attack 1, Attack 2, Decay 1, Decay 2 (held), Release 1, Release 2

// notes §1: an envelope as the E5000 holds it - a rate (0-127, a speed) and a level (0-100%) a stage
typedef struct {
    uint8_t rate[UA_EOS_STAGES];
    uint8_t level[UA_EOS_STAGES];
} tUaEosEnvelope;

typedef struct {
    int    stage;   // 0-3 attack and decay, 4 held, 5-6 release (Release 1, 2), 7 done
    double gain;
} tUaEosEnvState;

extern const tUaEosEnvelope gUaEosDefaultEnvelope;                                   // a new voice's

double ua_eos_level_gain(double percent, int depth);                                 // notes §2
double ua_eos_speed_db(int rate, int depth);                                         // notes §3: dB a second, decay and release
double ua_eos_attack_time(int rate, int depth);                                      // notes §4: seconds for silence to full
void ua_eos_from_times(const tEmuEnvelope * times, int depth, tUaEosEnvelope * out); // notes §5

void ua_eos_env_start(tUaEosEnvState * s, double fromGain);
void ua_eos_env_release(tUaEosEnvState * s);
double ua_eos_env_step(tUaEosEnvState * s, const tUaEosEnvelope * e, int depth, double dt);
bool ua_eos_env_done(const tUaEosEnvState * s);

#endif
