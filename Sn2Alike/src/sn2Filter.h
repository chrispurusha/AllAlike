/*
 * Sn2Alike - the filter.
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
// Notes: Docs/code-notes/sn2Filter.md - "// notes §k" refers there.

#ifndef __SN2_FILTER_H__
#define __SN2_FILTER_H__

#include <stdbool.h>
#include <stdint.h>

#define SN2_FILTER_TYPES     (12)
#define SN2_FILTER_SLOPES    (3)

typedef struct {
    double f, damp;
    bool   parallel;
    double taps[2][4];
} tSn2FilterCoeffs;

typedef struct {
    double low[2], band[2];
} tSn2Filter;

// cutoff and resonance in the panel's 0..127 (fractions allowed)
void sn2_filter_coeffs(uint32_t type, uint32_t slope, double cutoff, double resonance, double rate, tSn2FilterCoeffs * c);
void sn2_filter_reset(tSn2Filter * f);
double sn2_filter_process(tSn2Filter * f, const tSn2FilterCoeffs * c, double x);

#endif // __SN2_FILTER_H__
