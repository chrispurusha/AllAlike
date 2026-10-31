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

#include <math.h>
#include <string.h>

#include "sn2Filter.h"

#define PASSES          (2)
#define REFERENCE_HZ    (48000.0)  // notes §3: where the law below was measured

// notes §2: each type's tap gains, input, band, low and high, for section 1 then section 2
typedef struct {
    bool   parallel;
    double taps[2][4];
} tSn2FilterType;

#define IN    {1, 0, 0, 0}
#define BP    {0, 1, 0, 0}
#define LP    {0, 0, 1, 0}
#define HP    {0, 0, 0, 1}

static const tSn2FilterType kTypes[SN2_FILTER_TYPES][SN2_FILTER_SLOPES] = {
    {{true,  { {   -1, 1,  1,  0}, {    1, 1,  0, 0}}}, {true, {{-1, 1, 1,  0}, {1, 1,  0, 0}}}, {true, {{-1, 1, 1,  0}, {1, 1,  0, 0}}}},
    {{true,  { {    1, 1,  1, -1}, {    0, 1, -1, 0}}}, {true, {{ 1, 1, 1, -1}, {0, 1, -1, 0}}}, {true, {{ 1, 1, 1, -1}, {0, 1, -1, 0}}}},
    {{true,  { {    1, 1,  0,  0}, {    1, 1,  1, 1}}}, {true, {{ 1, 1, 0,  0}, {1, 1,  1, 1}}}, {true, {{ 1, 1, 0,  0}, {1, 1,  1, 1}}}},
    {{false, {IN, LP}},{false, {{0, -1, 1, 0}, LP}}, {false, {LP, LP}}},
    {{false, {IN, BP}},{false, {{1, -1, 1, 0}, BP}}, {false, {BP, BP}}},
    {{false, {IN, HP}},{false, {{0, 1, 0, 1}, HP}}, {false, {HP, HP}}},
    {{false, {LP, LP}},{false, {LP, LP}}, {false, {LP, LP}}},
    {{false, {HP, LP}},{false, {HP, LP}}, {false, {HP, LP}}},
    {{false, {HP, HP}},{false, {HP, HP}}, {false, {HP, HP}}},
    {{true,  {LP, HP}},{true,  {LP,HP}}, {true, {LP, HP}}},
    {{true,  {LP, LP}},{true,  {LP,LP}}, {true, {LP, LP}}},
    {{true,  {BP, BP}},{true,  {BP,BP}}, {true, {BP, BP}}},
};

// notes §3: the dial is in semitones
void sn2_filter_coeffs(uint32_t type, uint32_t slope, double cutoff, double resonance, double rate, tSn2FilterCoeffs * c) {
    const tSn2FilterType * t = &kTypes[(type < SN2_FILTER_TYPES) ? type : 3][(slope < SN2_FILTER_SLOPES) ? slope : 0];
    double                 f = 0.0306 * pow(2.0, (cutoff - 64.0) / 12.0);

    c->f        = fmin((f * (1.0 + (0.25 * f))) * (REFERENCE_HZ / rate), 1.0);
    c->damp     = 2.0 * fmax(0.66 - (0.0043 * resonance), 0.05);
    c->parallel = t->parallel;
    memcpy(c->taps, t->taps, sizeof(c->taps));
}

void sn2_filter_reset(tSn2Filter * f) {
    memset(f, 0, sizeof(*f));
}

// notes §1: two passes a sample, the output their average
static double section(double * low, double * band, const tSn2FilterCoeffs * c, const double * tap, double x) {
    double sum = 0.0;

    for (int pass = 0; pass < PASSES; pass++) {
        *low  += c->f * *band;
        double high = x - *low - (c->damp * *band);

        sum   += (tap[0] * x) + (tap[1] * *band) + (tap[2] * *low) + (tap[3] * high);
        *band += c->f * high;
    }

    return sum / PASSES;
}

double sn2_filter_process(tSn2Filter * f, const tSn2FilterCoeffs * c, double x) {
    double first  = section(&f->low[0], &f->band[0], c, c->taps[0], x);
    double second = section(&f->low[1], &f->band[1], c, c->taps[1], c->parallel ? x : first);

    return c->parallel ? (first + second) : second;
}
