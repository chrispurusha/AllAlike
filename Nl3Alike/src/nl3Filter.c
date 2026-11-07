/*
 * Nl3Alike - the filter.
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
// Notes: Docs/code-notes/nl3Filter.md - "// notes §k" refers there.

#include <math.h>
#include <string.h>

#include "nl3Filter.h"

#define CUTOFF_WORD_AT_0    (969.0 / NL3_Q23)   // notes §4
#define CUTOFF_WORD_OPEN    (0.8125)            // notes §4: the dial's top
#define G_PER_WORD          (8.0)
#define NORM_STEPS          (8.0)               // notes §5
#define NORM_LAST           (640)
#define Q_FLOOR_24          (1.0 / 64.0)        // notes §6
#define GAIN_TAKE_12        (0.75)
#define GAIN_TAKE_24        (0.9)
#define WORD_TOP            ((double)NL3_WORD_MAX / NL3_Q23)

tNl3FilterMode nl3_filter_mode(uint32_t type, uint32_t slope) {
    bool high  = (type == 1);
    bool steep = (slope == 1);

    return high ? (steep ? eNl3FilterHp24 : eNl3FilterHp12) : (steep ? eNl3FilterLp24 : eNl3FilterLp12);
}

// notes §5: 1 / root s, read from a table of 1/8 steps with straight lines between
static double norm_table(int i) {
    return round(NL3_Q23 / sqrt(1.0 + (i / NORM_STEPS))) / NL3_Q23;
}

static double norm(double s) {
    double pos = fmax(0.0, (s - 1.0) * NORM_STEPS);
    int    i   = (int)pos;

    if (i >= NORM_LAST) {
        return norm_table(NORM_LAST);
    }
    double lo  = norm_table(i);

    return lo + ((norm_table(i + 1) - lo) * (pos - i));
}

void nl3_filter_coeffs(tNl3FilterMode mode, double cutoff, double resonance, tNl3FilterCoeffs * c) {
    bool   steep = (mode == eNl3FilterLp24) || (mode == eNl3FilterHp24);
    double w     = (cutoff >= 127.0) ? CUTOFF_WORD_OPEN : fmin(CUTOFF_WORD_OPEN, CUTOFF_WORD_AT_0 * pow(2.0, cutoff / 12.0));
    double d     = (resonance >= 127.0) ? 0.0 : (1.0 - (fmax(0.0, resonance) / 128.0));
    double d2    = d * d;
    double q     = steep ? fmax(d2, Q_FLOOR_24) : (d2 * d2);                    // notes §6
    double g     = G_PER_WORD * w;
    double n     = norm(1.0 + (2.0 * q * g) + (g * g));
    double take  = steep ? GAIN_TAKE_24 : GAIN_TAKE_12;

    c->mode  = mode;
    c->f     = fmin(WORD_TOP, g * n);
    c->k     = fmin(WORD_TOP, q * n);
    c->n2    = fmin(WORD_TOP, n * n);
    c->gain  = fmin(WORD_TOP, 1.0 - (take * (1.0 - (steep ? fmax(d2, Q_FLOOR_24) : d2))));  // notes §8
    c->fw    = nl3_word(c->f);
    c->kw    = nl3_word(c->k);
    c->n2w   = nl3_word(c->n2);
    c->gainw = nl3_word(c->gain);
}

void nl3_filter_reset(tNl3Filter * f) {
    memset(f, 0, sizeof(*f));
}

// ------------------------------------------------------------------------------------------------ float

// notes §4: the float path takes the same steps, limiting where the chip moves a value out
static double limit(double x) {
    return fmin(WORD_TOP, fmax(-1.0, x));
}

// notes §2: one stage; b holds u on entry and leaves u - low - damping; returns the high leg
static double stage(double * low, double * band, double f, double k, double * b) {
    double x0   = *band;
    double a    = *low + (f * x0);

    *b   -= a;
    *low  = limit(a);
    double b3   = limit(limit(x0 * x0) * x0);

    a     = b3 - (k * b3);
    double y1   = limit(a);

    a    += k * x0;
    a    -= y1 * f;
    *b   -= a;
    double high = limit(*b);

    *band = limit((x0 / 4.0 + (f * high)) * 4.0);
    return high;
}

// notes §1: the input averaged over the two stored samples
static double smooth(double * in, double * mean, double b) {
    double prev = *in;

    *in   = limit(b);
    b     = (b + prev) / 2.0;
    double m    = *mean;

    *mean = limit(b);
    return (b + m) / 4.0;
}

static double out_float(double b, double gain) {
    return limit((limit(b) * gain * 32.0) / 4.0);
}

double nl3_filter_process(tNl3Filter * f, const tNl3FilterCoeffs * c, double x) {
    double b;

    switch (c->mode) {
        case eNl3FilterLp24:
            b = smooth(&f->in[0], &f->mean[0], x / 4.0);
            (void)stage(&f->low[0], &f->band[0], c->f, c->k, &b);
            b = smooth(&f->in[1], &f->mean[1], (2.0 * f->low[0]) / 8.0);
            (void)stage(&f->low[1], &f->band[1], c->f, c->k, &b);
            return out_float(2.0 * f->low[1], c->gain);

        case eNl3FilterHp12:
            b = (c->n2 * limit(x / 32.0)) / 2.0;
            (void)stage(&f->low[0], &f->band[0], c->f, c->k, &b);
            return out_float(2.0 * b, c->gain);

        case eNl3FilterHp24:
            b = (c->n2 * limit(x / 4.0)) / 2.0;
            (void)stage(&f->low[0], &f->band[0], c->f, c->k, &b);
            b = (c->n2 * limit((2.0 * b) / 8.0)) / 2.0;
            (void)stage(&f->low[1], &f->band[1], c->f, c->k, &b);
            return out_float(2.0 * b, c->gain);

        case eNl3FilterLp12:
        default:
            b = smooth(&f->in[0], &f->mean[0], x / 32.0);
            (void)stage(&f->low[0], &f->band[0], c->f, c->k, &b);
            return out_float(2.0 * f->low[0], c->gain);
    }
}

// ------------------------------------------------------------------------------------------------ fixed

// notes §3: the stage as the chip runs it; b holds u on entry and leaves u - low - damping
static tNl3Word stage_fixed(tNl3Word * low, tNl3Word * band, tNl3Word f, tNl3Word k, tNl3Acc * b) {
    tNl3Word x0   = *band;
    tNl3Acc  a    = nl3_acc(*low) + nl3_mul(f, x0);

    *b   -= a;
    *low  = nl3_move(a);
    a     = nl3_mul(x0, x0);
    tNl3Word y1   = nl3_move(a);

    a     = nl3_mul(x0, y1);
    tNl3Word b3   = nl3_move(a);

    a    -= nl3_mul(k, b3);
    y1    = nl3_move(a);
    a    += nl3_mul(k, x0);
    a    -= nl3_mul(y1, f);
    *b   -= a;
    a     = nl3_acc(x0) >> 2;
    tNl3Word high = nl3_move(*b);

    a    += nl3_mul(f, high);
    a     = nl3_asl(a, 2);
    *band = nl3_move(a);
    return high;
}

// notes §1: the input averaged over the two stored words, from a pre-shifted accumulator
static tNl3Acc smooth_fixed(tNl3Word * in, tNl3Word * mean, tNl3Acc b) {
    tNl3Word prev = *in;

    *in   = nl3_move(b);
    b    += nl3_acc(prev);
    b   >>= 1;
    tNl3Word m    = *mean;

    *mean = nl3_move(b);
    b    += nl3_acc(m);
    return b >> 2;
}

// notes §3: the output word: x1 = limit(b), times the gain rounded, up 32, down 4
static tNl3Word out_fixed(tNl3Acc b, tNl3Word gain) {
    tNl3Acc o = nl3_round(nl3_mul(nl3_move(b), gain));

    o = nl3_asl(o, 5);
    return nl3_move(o >> 2);
}

tNl3Word nl3_filter_process_fixed(tNl3Filter * f, const tNl3FilterCoeffs * c, tNl3Word x) {
    tNl3Acc b;

    switch (c->mode) {
        case eNl3FilterLp24:
            b = smooth_fixed(&f->inW[0], &f->meanW[0], nl3_acc(x) >> 2);
            (void)stage_fixed(&f->lowW[0], &f->bandW[0], c->fw, c->kw, &b);
            b = smooth_fixed(&f->inW[1], &f->meanW[1], nl3_asl(nl3_acc(f->lowW[0]), 1) >> 3);
            (void)stage_fixed(&f->lowW[1], &f->bandW[1], c->fw, c->kw, &b);
            return out_fixed(nl3_asl(nl3_acc(f->lowW[1]), 1), c->gainw);

        case eNl3FilterHp12:
            b = nl3_mul(c->n2w, nl3_move(nl3_acc(x) >> 5)) >> 1;
            (void)stage_fixed(&f->lowW[0], &f->bandW[0], c->fw, c->kw, &b);
            return out_fixed(nl3_asl(b, 1), c->gainw);

        case eNl3FilterHp24:
            b = nl3_mul(c->n2w, nl3_move(nl3_acc(x) >> 2)) >> 1;
            (void)stage_fixed(&f->lowW[0], &f->bandW[0], c->fw, c->kw, &b);
            b = nl3_mul(c->n2w, nl3_move(nl3_asl(b, 1) >> 3)) >> 1;
            (void)stage_fixed(&f->lowW[1], &f->bandW[1], c->fw, c->kw, &b);
            return out_fixed(nl3_asl(b, 1), c->gainw);

        case eNl3FilterLp12:
        default:
            b = smooth_fixed(&f->inW[0], &f->meanW[0], nl3_acc(x) >> 5);
            (void)stage_fixed(&f->lowW[0], &f->bandW[0], c->fw, c->kw, &b);
            return out_fixed(nl3_asl(nl3_acc(f->lowW[0]), 1), c->gainw);
    }
}
