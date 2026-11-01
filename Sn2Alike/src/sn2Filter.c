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

#define PASSES                 (2)
#define DRIVE_LEAST            (0.98447)
#define DRIVE_TRIM             (0.012)
#define NORMALISE_TRIM_FROM    (120.0)
#define F_AT_64                (0.037465)
#define F_MAX                  (0.99863)
#define PLAIN_FIRST_D          (0.5)
#define FIRST_RES_TYPE         (3u)
#define LEVEL_DAMP_FROM        (0.25)

// notes §2: each type's tap gains, input, band, low and high, for section 1 then section 2
typedef struct {
    bool   parallel;
    double taps[2][4];
} tSn2FilterType;

#define IN    {1, 0, 0, 0}
#define BP    {0, 1, 0, 0}
#define LP    {0, 0, 1, 0}
#define HP    {0, 0, 0, 1}

// notes §2: in panel order; the stored byte is this index + 3 (sn2Patch notes §7)
static const tSn2FilterType kTypes[SN2_FILTER_TYPES][SN2_FILTER_SLOPES] = {
    {{false, {IN, LP}}, {false, { {    0, -1, 1, 0}, LP}}, {false, {LP, LP}}}, // LPF
    {{false, {IN, BP}}, {false, { {    1, -1, 1, 0}, BP}}, {false, {BP, BP}}}, // BPF
    {{false, {IN, HP}}, {false, { {    0,  1, 0, 1}, HP}}, {false, {HP, HP}}}, // HPF
    {{false, {LP, LP}}, {false, {LP, LP}},{false, {LP, LP}}},              // Res LPF
    {{false, {HP, LP}}, {false, {HP, LP}},{false, {HP, LP}}},              // Res BPF
    {{false, {HP, HP}}, {false, {HP, HP}},{false, {HP, HP}}},              // Res HPF
    {{true,  {LP, HP}}, {true,  {LP, HP}},{true,  {LP,HP}}},               // Notch
    {{true,  {LP, LP}}, {true,  {LP, LP}},{true,  {LP,LP}}},               // LPF+LPF
    {{true,  {BP, BP}}, {true,  {BP, BP}},{true,  {BP,BP}}},               // BPF+BPF
    {{true,  {HP, HP}}, {true,  {HP, HP}},{true,  {HP,HP}}},               // HPF+HPF
    {{true,  {LP, BP}}, {true,  {LP, BP}},{true,  {LP,BP}}},               // LPF+BPF
    {{true,  {BP, HP}}, {true,  {BP, HP}},{true,  {BP,HP}}},               // BPF+HPF
};

// notes §3: the dial is in semitones
static double section_damp(double f, double d) {
    return 2.0 * 0.7 * d * (1.0 - (f * d / 2.0));
}

void sn2_filter_coeffs(uint32_t type, uint32_t slope, double cutoff, double resonance, double normalise, tSn2FilterCoeffs * c) {
    const tSn2FilterType * t     = &kTypes[(type < SN2_FILTER_TYPES) ? type : 0][(slope < SN2_FILTER_SLOPES) ? slope : 0];
    double                 f     = fmin(F_AT_64 * pow(2.0, (cutoff - 64.0) / 12.0), F_MAX);
    double                 d     = (127.0 - resonance) / 127.0;
    bool                   plain = (type < FIRST_RES_TYPE);

    c->f        = f;
    c->damp[0]  = section_damp(f, plain ? fmax(d, PLAIN_FIRST_D) : d);  // notes §3
    c->damp[1]  = section_damp(f, d);
    c->gain     = 1.0 - ((normalise / 128.0) * (resonance / 127.0));
    c->trim     = 2.0 * (fmax(normalise - NORMALISE_TRIM_FROM, 0.0) / 128.0) * (1.0 - f);
    c->parallel = t->parallel;
    memcpy(c->taps, t->taps, sizeof(c->taps));
}

// notes §5
double sn2_filter_drive_word(double overdrive) {
    double v = overdrive / 128.0;

    return 1.0 - (DRIVE_LEAST * (1.0 - v) * (1.0 - v)) - (DRIVE_TRIM * v * (1.0 - v));
}

double sn2_filter_drive(double x, double word) {
    return x / (1.0 + (word * (fabs(x) - 1.0)));
}

void sn2_filter_reset(tSn2Filter * f) {
    memset(f, 0, sizeof(*f));
}

// notes §4
static double saturate(double x) {
    return fmin(fmax(x, -1.0), 1.0);
}

// notes §1: two passes a sample, the output their average
static double section(double * low, double * band, const tSn2FilterCoeffs * c, const double * tap, double damp, double x) {
    double sum = 0.0;

    for (int pass = 0; pass < PASSES; pass++) {
        *low  = saturate(*low + (c->f * *band));
        double high = x - *low - ((fmax(damp, 2.0 * (fabs(*band) - LEVEL_DAMP_FROM)) - c->trim) * *band);  // notes §4

        sum  += (tap[0] * x) + (tap[1] * *band) + (tap[2] * *low) + (tap[3] * high);
        *band = saturate(*band + (c->f * high));
    }

    return sum / PASSES;
}

double sn2_filter_process(tSn2Filter * f, const tSn2FilterCoeffs * c, double x) {
    double first  = section(&f->low[0], &f->band[0], c, c->taps[0], c->damp[0], x);
    double second = section(&f->low[1], &f->band[1], c, c->taps[1], c->damp[1], c->parallel ? x : first);

    return c->parallel ? (first + second) : second;
}
