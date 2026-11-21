/*
 * ProphAlike - the filter: one state-variable filter, as the instrument's, in its fixed point and in floating point.
 *
 * Copyright (C) 2026 Chris Turner <chris_purusha@icloud.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
// Notes: Docs/code-notes/prophFilter.md - "// notes §k" refers there.

#ifndef __PROPH_FILTER_H__
#define __PROPH_FILTER_H__

#include <stdbool.h>
#include <stdint.h>

#define PROPH_ENGINE_HZ       (48000.0) // notes §1: the rate the instrument's voice runs at
#define PROPH_FILTER_TYPES    (5)       // notes §3: THRU, LPF, HPF, BPF, BRF

// notes §2: what the panel asks for, as the instrument hands it over - cutoff target and step, gain and damping targets
typedef struct {
    uint32_t type;
    int32_t  target;        // the cutoff word's target, in its top 16 bits
    int32_t  gainTarget;    // the input gain's
    int32_t  dampTarget;    // the damping's (negative)
} tProphFilterSet;

typedef struct {
    // fixed point: 32-bit words with 1.0 at 2^31, as the instrument holds them
    int32_t f, notch, high, band, low, thru, gain, damp;
    int32_t step, lastTarget;
    // floating point
    double  ff, fband, flow, fgain, fdamp;
    bool    primed;
} tProphFilter;

// cutoff and resonance in the panel's 0..99 (cutoff fractions allowed, for modulation)
void proph_filter_set(tProphFilter * f, tProphFilterSet * s, uint32_t type, double cutoff, double resonance);
void proph_filter_reset(tProphFilter * f);
// one sample; x and the result in -1..1 of the instrument's full scale
double proph_filter_process(tProphFilter * f, const tProphFilterSet * s, double x, bool fixedPoint);

#endif // __PROPH_FILTER_H__
