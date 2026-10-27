/*
 * TiAlike - the filter.
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
// Notes: Docs/code-notes/tiFilter.md - "// notes §k" refers there.

#ifndef __TI_FILTER_H__
#define __TI_FILTER_H__

#define TI_FILTER_POINTS    (32)
#define TI_FILTER_ROWS      (9)

#include <stdint.h>

typedef enum {
    eTiFilterLowPass = 0,
    eTiFilterHighPass,
    eTiFilterBandPass,
    eTiFilterBandStop,
    eTiFilterAnalog1,     // notes §5: the ladder, tapped after 1..4 stages
    eTiFilterAnalog2,
    eTiFilterAnalog3,
    eTiFilterAnalog4,
    eTiFilterModes
} tTiFilterMode;

// notes §1: the cutoff points and resonance rows at the running sample rate
typedef struct {
    double rate;
    double cosine[TI_FILTER_POINTS];
    double halfR2[TI_FILTER_ROWS][TI_FILTER_POINTS];   // r^2 / 2 of each row's pole pair
} tTiFilterTables;

typedef struct {
    double   b0, b1, b2;  // feed-forward
    double   a1, a2;      // feedback, y[n] = ... - a1 y[n-1] - a2 y[n-2]

    // notes §5: the ladder; poles 0 selects the biquad above
    uint32_t poles;
    double   g;           // each stage's coefficient, at twice the sample rate
    double   feedback;    // into stage 1, from stage 4
    double   inGain, preGain, postGain, tapGain;
} tTiFilterCoeffs;

typedef struct {
    double   x1, x2, y1, y2;
    double   s[4];
    double   lastIn;
    uint32_t noise;
} tTiFilter;

void ti_filter_tables(tTiFilterTables * t, double rate);  // also keeps the rate for the ladder
// cutoff and resonance in the panel's 0..127 (fractions allowed)
void ti_filter_coeffs(const tTiFilterTables * t, tTiFilterMode mode, double cutoff, double resonance, double drive, tTiFilterCoeffs * c);
double ti_filter_cutoff_hz(double cutoff);
// the output gain a digital mode takes at a resonance (notes §2)
double ti_filter_row_gain(tTiFilterMode mode, double resonance);
void ti_filter_reset(tTiFilter * f);
double ti_filter_process(tTiFilter * f, const tTiFilterCoeffs * c, double x);

#endif // __TI_FILTER_H__
