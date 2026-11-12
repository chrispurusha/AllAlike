/*
 * TiAlike - the LFOs.
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
// Notes: Docs/code-notes/tiLfo.md - "// notes §k" refers there.

#ifndef __TI_LFO_H__
#define __TI_LFO_H__

#include <stdbool.h>
#include <stdint.h>

typedef struct {
    double   phase;
    double   held, last, target;   // S&H and S&G
    double   seen;                 // cycles begun, and notes §2: Env Mode has run its one
    double   start, run;           // notes §2: Env Mode's start phase, and how far it has gone since
    bool     wraps, spent;
    bool     pinned;               // tiVoices notes §13: held at the top of Env Mode
    double   fade;                 // LFO 3's fade-in, 0..1
    uint32_t noise;
    double   value;                // -1..1 as last computed
} tTiLfo;

typedef struct {
    double rateHz;
    int    shape;        // the panel's list: sine, triangle, saw, square, S&H, S&G, then the waves
    double contour;      // -1..1
    bool   envMode;
    double fadeStep;     // per tick; 0 none
} tTiLfoShape;

double ti_lfo_rate_hz(double dial);
void ti_lfo_trigger(tTiLfo * l, double phase, uint32_t seed);
double ti_lfo_tick(tTiLfo * l, const tTiLfoShape * shape, double seconds);

#endif // __TI_LFO_H__
