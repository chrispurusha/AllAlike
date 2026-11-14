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
#define GAIN_TAKE_BP        (0.875)                         // notes §9
#define BP_WARP_SCALE       (1.00022)                       // notes §9
#define Q_FLOOR_BR          ((21.0 / 64.0) * (21.0 / 64.0)) // notes §14
#define WORD_TOP            ((double)NL3_WORD_MAX / NL3_Q23)

// The panel's type (LP HP BP BR Classic Dist) and slope (6 12 24 dB); the modes not yet built play as the nearest
tNl3FilterMode nl3_filter_mode(uint32_t type, uint32_t slope) {
    switch (type) {
        case 1:
            return (slope == 0) ? eNl3FilterHp6 : ((slope == 2) ? eNl3FilterHp24 : eNl3FilterHp12);

        case 2:
            return (slope == 2) ? eNl3FilterBp24 : eNl3FilterBp12;

        case 3:
            return eNl3FilterBr;   // one form at either slope (notes §14)

        default:
            return (slope == 0) ? eNl3FilterLp6 : ((slope == 2) ? eNl3FilterLp24 : eNl3FilterLp12);
    }
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
    bool   steep  = (mode == eNl3FilterLp24) || (mode == eNl3FilterHp24) || (mode == eNl3FilterBp24);
    double w      = (cutoff >= 127.0) ? CUTOFF_WORD_OPEN : fmin(CUTOFF_WORD_OPEN, CUTOFF_WORD_AT_0 * pow(2.0, cutoff / 12.0));
    double d      = (resonance >= 127.0) ? 0.0 : (1.0 - (fmax(0.0, resonance) / 128.0));
    double d2     = d * d;
    double q      = steep ? fmax(d2, Q_FLOOR_24) : (d2 * d2);                   // notes §6
    bool   band   = (mode == eNl3FilterBp12) || (mode == eNl3FilterBp24);
    bool   reject = (mode == eNl3FilterBr);
    bool   warp   = band || reject || (mode == eNl3FilterLp6) || (mode == eNl3FilterHp6); // notes §9, §11, §12, §14

    q        = reject ? fmax(d2, Q_FLOOR_BR) : q;                                         // notes §14
    double g      = G_PER_WORD * w;

    g        = (warp && (cutoff < 127.0)) ? tan(BP_WARP_SCALE * g) : g;          // notes §9, §11: prewarped
    double n      = norm(1.0 + (2.0 * q * g) + (g * g));
    double take   = steep ? GAIN_TAKE_24 : (band ? GAIN_TAKE_BP : GAIN_TAKE_12); // notes §8-10

    c->mode  = mode;
    c->f     = fmin(WORD_TOP, g * n);
    c->k     = fmin(WORD_TOP, q * n);
    c->n2    = fmin(WORD_TOP, (warp && !reject) ? n : (n * n));                            // notes §9, §11, §14: N, or N² (band-reject's high leg)
    c->gain  = fmin(WORD_TOP, 1.0 - (take * (1.0 - (steep ? fmax(d2, Q_FLOOR_24) : d2)))); // notes §8
    c->fw    = nl3_word(c->f);
    c->kw    = nl3_word(c->k);
    c->n2w   = nl3_word(c->n2);
    c->gainw = nl3_word(c->gain);
}

// notes §15: one filter of a multi mode - prewarped, q = d², N or N² as the leg takes it
static void multi_one(double cutoff, double d2, double * f, double * k, double * n, double * n2) {
    double w = (cutoff >= 127.0) ? CUTOFF_WORD_OPEN : fmin(CUTOFF_WORD_OPEN, CUTOFF_WORD_AT_0 * pow(2.0, cutoff / 12.0));
    double g = G_PER_WORD * w;

    g   = (cutoff < 127.0) ? tan(BP_WARP_SCALE * g) : g;
    *n  = norm(1.0 + (2.0 * d2 * g) + (g * g));
    *f  = fmin(WORD_TOP, g * *n);
    *k  = fmin(WORD_TOP, d2 * *n);
    *n2 = fmin(WORD_TOP, *n * *n);
}

void nl3_filter_coeffs_multi(tNl3FilterMode mode, double cutoff, double cutoff2, double resonance, tNl3FilterCoeffs * c) {
    double d  = (resonance >= 127.0) ? 0.0 : (1.0 - (fmax(0.0, resonance) / 128.0));
    double d2 = d * d;
    double n, n2;

    c->mode  = mode;

    if ((mode == eNl3FilterPeak) || (mode == eNl3FilterNotch)) {                                             // notes §16: Freq 2 sets the width, resonance the feedback
        double q = (cutoff2 >= 127.0) ? 1.0 : ((fmax(0.0, cutoff2) / 128.0) * (fmax(0.0, cutoff2) / 128.0)); // 127 counts as 128

        multi_one(cutoff, q, &c->f, &c->k, &n, &n2);
        c->fb    = ((resonance >= 127.0) ? 127.0 : fmax(0.0, resonance)) / 128.0;
        c->fw    = nl3_word(c->f);
        c->kw    = nl3_word(c->k);
        c->fbw   = nl3_word(c->fb);
        c->gainw = NL3_WORD_MAX;
        c->gain  = 1.0;
        return;
    }
    multi_one(cutoff, d2, &c->f, &c->k, &n, &n2);
    c->n2    = n2;
    multi_one(cutoff2, d2, &c->f2, &c->k2, &n, &c->n22);
    c->gain  = fmin(WORD_TOP, 1.0 - (GAIN_TAKE_BP * (1.0 - d2)));
    c->fw    = nl3_word(c->f);
    c->kw    = nl3_word(c->k);
    c->n2w   = nl3_word(c->n2);
    c->fw2   = nl3_word(c->f2);
    c->kw2   = nl3_word(c->k2);
    c->n2w2  = nl3_word(c->n22);
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

// notes §11: the 6 dB lowpass in floating point, the same steps
static double lp6(tNl3Filter * f, const tNl3FilterCoeffs * c, double x) {
    double b      = x / 16.0;
    double prev   = f->in[0];
    double band   = f->band[0];
    double lowWas = f->low[0];

    f->in[0]   = limit(b);
    b          = (b + prev) / 2.0;
    double a      = (2.0 * f->low[0]) + (2.0 * c->f * band);

    b         -= a;
    f->low[0]  = limit(a / 2.0);
    double cube   = limit(limit(band * band) * band);
    double damp   = cube - (c->k * cube);

    a          = damp - (limit(damp) * c->f) + (c->k * band);
    b         -= 2.0 * a;
    a          = f->low[0] + lowWas;
    double t      = limit(b);

    f->band[0] = limit(band + (2.0 * c->f * t));
    a         -= f->band[0] * c->n2;
    return limit(4.0 * limit(a) * c->gain);
}

static tNl3Word stage_fixed(tNl3Word * low, tNl3Word * band, tNl3Word f, tNl3Word k, tNl3Acc * b, tNl3Acc * bandAcc);
static tNl3Acc smooth_fixed(tNl3Word * in, tNl3Word * mean, tNl3Acc b);

// notes §13: a stage without the cubic term; b holds its input and leaves its high leg
static void linear_stage_fixed(tNl3Word * low, tNl3Word * band, tNl3Word f, tNl3Word k, tNl3Acc * b) {
    tNl3Acc a = nl3_acc(*low) + nl3_mul(f, *band);

    *b   -= a;
    *b   -= nl3_mul(k, *band);
    *low  = nl3_move(a);
    a     = nl3_acc(*band) >> 2;
    a    += nl3_mul(nl3_move(*b), f);
    *band = nl3_move(nl3_asl(a, 2));
}

// notes §13: Classic, two passes a sample over the held input, each a plain stage into a cubic one
static tNl3Word classic_fixed(tNl3Filter * f, const tNl3FilterCoeffs * c, tNl3Word x) {
    tNl3Acc  b    = nl3_acc(x) >> 1;
    tNl3Word held = nl3_move(b);

    for (int pass = 0; pass < 2; pass++) {
        b = (pass == 0) ? (b >> 1) : (nl3_acc(held) >> 1);
        linear_stage_fixed(&f->lowW[0], &f->bandW[0], c->fw, c->kw, &b);
        b = nl3_acc(f->lowW[0]) >> 1;
        (void)stage_fixed(&f->lowW[1], &f->bandW[1], c->fw, c->kw, &b, NULL);
    }

    b = nl3_asl(nl3_mul(f->lowW[1], c->gainw), 2);
    return nl3_move(b >> 1);
}

// notes §14: band-reject, the plain stage at 1/16; out the last N x high plus the mean of two low sums
static tNl3Word br_fixed(tNl3Filter * f, const tNl3FilterCoeffs * c, tNl3Word x) {
    tNl3Acc  b      = nl3_acc(x) >> 4;
    tNl3Word x0     = f->bandW[0];
    tNl3Word lowWas = f->lowW[0];
    tNl3Acc  a      = nl3_asl(nl3_acc(f->lowW[0]), 1) + nl3_mul(c->fw, x0) + nl3_mul(c->fw, x0);

    b          -= a;
    a         >>= 1;
    f->lowW[0]  = nl3_move(a);
    b          -= nl3_mul(c->kw, x0) + nl3_mul(c->kw, x0);
    a          += nl3_acc(lowWas);
    tNl3Word t      = nl3_move(b);

    b           = nl3_acc(x0) + nl3_mul(c->fw, t) + nl3_mul(c->fw, t);
    f->bandW[0] = nl3_move(b);
    b           = nl3_mul(c->n2w, t);
    tNl3Word last   = f->inW[0];

    f->inW[0]   = nl3_move(b);
    b           = nl3_acc(last);
    tNl3Word sumWas = f->meanW[0];

    f->meanW[0] = nl3_move(a);
    a           = (a + nl3_acc(sumWas)) >> 1;
    b          += a;
    b           = nl3_asl(b, 4);
    return nl3_move(b >> 2);
}

// notes §15: lowpass 12 on the first filter's words beside highpass 12 on the second's, the input scaled first
static tNl3Word lphp_par_fixed(tNl3Filter * f, const tNl3FilterCoeffs * c, tNl3Word x) {
    tNl3Word in  = nl3_move(nl3_round(nl3_mul(x, c->gainw)));
    tNl3Acc  b   = smooth_fixed(&f->inW[0], &f->meanW[0], nl3_acc(in) >> 2);

    (void)stage_fixed(&f->lowW[0], &f->bandW[0], c->fw, c->kw, &b, NULL);
    tNl3Word low = nl3_move(nl3_asl(nl3_acc(f->lowW[0]), 3));

    b = nl3_mul(c->n2w2, nl3_move(nl3_acc(in) >> 2)) >> 1;
    (void)stage_fixed(&f->lowW[1], &f->bandW[1], c->fw2, c->kw2, &b, NULL);
    b = nl3_asl(b, 3) + nl3_acc(low);
    return nl3_move(b >> 2);
}

// notes §16: three all-pass stages on one F and k, their output fed back to the input; peak or notch
static tNl3Word allpass3_fixed(tNl3Filter * f, const tNl3FilterCoeffs * c, tNl3Word x, bool notch) {
    tNl3Acc  a    = nl3_mul(notch ? (tNl3Word)0xA00000 - 0x1000000 : (tNl3Word)0x933333 - 0x1000000, c->fbw) + nl3_acc(NL3_WORD_MAX);
    tNl3Acc  b    = nl3_mul(x, nl3_move(a)) >> 4;
    tNl3Word dry  = nl3_move(b);

    b        += notch ? nl3_mul(f->backW, c->fbw) : -nl3_mul(f->backW, c->fbw);
    tNl3Word prev = f->inW[0];

    f->inW[0] = nl3_move(b);
    b        += nl3_acc(prev);
    b       >>= 1;

    for (int s = 0; s < 3; s++) {
        tNl3Word band = f->bandW[s];

        a           = nl3_acc(f->lowW[s]) + nl3_mul(c->fw, band) + nl3_mul(c->fw, band);
        b          -= a;
        f->lowW[s]  = nl3_move(a);
        b          -= nl3_mul(c->kw, band) + nl3_mul(c->kw, band);
        tNl3Word t    = nl3_move(b);

        b           = nl3_acc(band) + nl3_mul(c->fw, t) + nl3_mul(c->fw, t);
        a          += nl3_acc(t);
        f->bandW[s] = nl3_move(b);
        tNl3Word y1   = nl3_move(b);

        a          -= nl3_mul(y1, c->kw) + nl3_mul(y1, c->kw);
        b           = a;
    }

    f->backW  = nl3_move(b);

    if (notch) {
        b = nl3_asl(b + nl3_acc(dry), 3);
    } else {
        b = nl3_asl(b, 4);
    }
    return nl3_move(b >> 2);
}

// notes §12: the 6 dB highpass, step by step as the instrument computes it
static tNl3Word hp6_fixed(tNl3Filter * f, const tNl3FilterCoeffs * c, tNl3Word x) {
    tNl3Acc  b       = nl3_round(nl3_mul(nl3_move(nl3_acc(x) >> 4), c->n2w));
    tNl3Word x0      = f->bandW[0];
    tNl3Word y0      = c->fw;
    tNl3Word x1      = c->kw;
    tNl3Acc  a       = nl3_acc(f->lowW[0]) + nl3_mul(y0, x0) + nl3_mul(y0, x0);

    b          -= a;
    f->lowW[0]  = nl3_move(a);
    a           = nl3_mul(x0, x0);
    tNl3Word y1      = nl3_move(a);

    a           = nl3_mul(x0, y1);
    x0          = nl3_move(a);
    a          -= nl3_mul(x1, x0);
    y1          = nl3_move(a);
    a          -= nl3_mul(y1, y0);
    x0          = f->bandW[0];
    a          += nl3_mul(x1, x0);
    b          -= a;
    b          -= a;
    tNl3Word t       = nl3_move(b);
    tNl3Word bandWas = f->bandW[0];

    b           = nl3_acc(bandWas) + nl3_mul(y0, t) + nl3_mul(y0, t);
    f->bandW[0] = nl3_move(b);
    b          += nl3_acc(bandWas);
    b         >>= 1;
    b          -= nl3_mul(c->n2w, t);
    tNl3Acc  o       = nl3_round(nl3_mul(nl3_move(b), c->gainw));

    o           = nl3_asl(o, 4);
    return nl3_move(o >> 2);
}

// notes §15: the parallel lowpass and highpass in floating point
static double lphp_par(tNl3Filter * f, const tNl3FilterCoeffs * c, double x) {
    double in  = limit(x * c->gain);
    double b   = smooth(&f->in[0], &f->mean[0], in / 4.0);

    (void)stage(&f->low[0], &f->band[0], c->f, c->k, &b);
    double low = limit(8.0 * f->low[0]);

    b = (c->n22 * limit(in / 4.0)) / 2.0;
    (void)stage(&f->low[1], &f->band[1], c->f2, c->k2, &b);
    return limit(((8.0 * b) + low) / 4.0);
}

// notes §16: the peak and notch modes in floating point
static double allpass3(tNl3Filter * f, const tNl3FilterCoeffs * c, double x, bool notch) {
    double pre  = limit(1.0 + ((notch ? -0.75 : -0.85) * c->fb));
    double b    = x * pre / 16.0;
    double dry  = limit(b);

    b       += (notch ? 1.0 : -1.0) * f->back * c->fb;
    double prev = f->in[0];

    f->in[0] = limit(b);
    b        = (b + prev) / 2.0;
    double a    = 0.0;

    for (int s = 0; s < 3; s++) {
        double band = f->band[s];

        a          = f->low[s] + (2.0 * c->f * band);
        b         -= a;
        f->low[s]  = limit(a);
        b         -= 2.0 * c->k * band;
        double t    = limit(b);

        f->band[s] = limit(band + (2.0 * c->f * t));
        a         += t;
        a         -= 2.0 * c->k * f->band[s];
        b          = a;
    }

    f->back  = limit(b);
    return limit(notch ? (2.0 * (b + dry)) : (4.0 * b));
}

// notes §14: band-reject in floating point
static double br(tNl3Filter * f, const tNl3FilterCoeffs * c, double x) {
    double b      = x / 16.0;
    double band   = f->band[0];
    double lowWas = f->low[0];
    double a      = (2.0 * f->low[0]) + (2.0 * c->f * band);

    b         -= a;
    a         /= 2.0;
    f->low[0]  = limit(a);
    b         -= 2.0 * c->k * band;
    a         += lowWas;
    double t      = limit(b);

    f->band[0] = limit(band + (2.0 * c->f * t));
    double last   = f->in[0];

    f->in[0]   = limit(c->n2 * t);
    double sumWas = f->mean[0];

    f->mean[0] = limit(a);
    return limit(4.0 * (last + ((a + sumWas) / 2.0)));
}

// notes §12: the 6 dB highpass in floating point
static double hp6(tNl3Filter * f, const tNl3FilterCoeffs * c, double x) {
    double b    = c->n2 * limit(x / 16.0);
    double band = f->band[0];
    double a    = f->low[0] + (2.0 * c->f * band);

    b         -= a;
    f->low[0]  = limit(a);
    double cube = limit(limit(band * band) * band);
    double damp = cube - (c->k * cube);

    a          = damp - (limit(damp) * c->f) + (c->k * band);
    b         -= 2.0 * a;
    double t    = limit(b);

    double sum  = band + (2.0 * c->f * t);      // averaged before it is limited, as the fixed path

    f->band[0] = limit(sum);
    b          = ((sum + band) / 2.0) - (c->n2 * t);
    return limit(4.0 * limit(b) * c->gain);
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

        case eNl3FilterLp6:
            return lp6(f, c, x);

        case eNl3FilterHp6:
            return hp6(f, c, x);

        case eNl3FilterBr:
            return br(f, c, x);

        case eNl3FilterLpHpPar:
            return lphp_par(f, c, x);

        case eNl3FilterPeak:
            return allpass3(f, c, x, false);

        case eNl3FilterNotch:
            return allpass3(f, c, x, true);

        case eNl3FilterBp24:   // notes §10
        {
            double cur  = c->n2 * limit(x / 4.0);
            double prev = f->in[0];

            f->in[0] = limit(cur);
            b        = (cur + prev) / 2.0;
            (void)stage(&f->low[0], &f->band[0], c->f, c->k, &b);
            cur      = c->n2 * limit(f->band[0] / 8.0);
            prev     = f->in[1];
            f->in[1] = limit(cur);
            b        = (cur + prev) / 2.0;
            (void)stage(&f->low[1], &f->band[1], c->f, c->k, &b);
            return out_float(f->band[1], c->gain);
        }

        case eNl3FilterBp12:   // notes §9: the input at N, averaged once; the band state out
        {
            double cur  = c->n2 * limit(x / 32.0);
            double prev = f->in[0];

            f->in[0] = limit(cur);
            b        = (cur + prev) / 2.0;
            (void)stage(&f->low[0], &f->band[0], c->f, c->k, &b);
            return out_float(f->band[0], c->gain);
        }

        case eNl3FilterLp12:
        default:
            b = smooth(&f->in[0], &f->mean[0], x / 32.0);
            (void)stage(&f->low[0], &f->band[0], c->f, c->k, &b);
            return out_float(2.0 * f->low[0], c->gain);
    }
}

// ------------------------------------------------------------------------------------------------ fixed

// notes §3: the stage as the chip runs it; b holds u on entry and leaves u - low - damping
static tNl3Word stage_fixed(tNl3Word * low, tNl3Word * band, tNl3Word f, tNl3Word k, tNl3Acc * b, tNl3Acc * bandAcc) {
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

    if (bandAcc != NULL) {
        *bandAcc = a;   // notes §10: the next stage takes it before it is limited
    }
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

// notes §11: the 6 dB lowpass, step by step as the instrument computes it
static tNl3Word lp6_fixed(tNl3Filter * f, const tNl3FilterCoeffs * c, tNl3Word x) {
    tNl3Acc  b  = nl3_acc(x) >> 4;
    tNl3Word x1 = f->inW[0];

    f->inW[0]   = nl3_move(b);
    b          += nl3_acc(x1);
    b         >>= 1;
    tNl3Word x0 = f->bandW[0];
    tNl3Word y0 = c->fw;
    tNl3Word n1 = f->lowW[0];
    tNl3Acc  a  = nl3_asl(nl3_acc(f->lowW[0]), 1);

    x1          = c->kw;
    a          += nl3_mul(y0, x0);
    a          += nl3_mul(y0, x0);
    b          -= a;
    a         >>= 1;
    f->lowW[0]  = nl3_move(a);
    a           = nl3_mul(x0, x0);
    tNl3Word y1 = nl3_move(a);

    a           = nl3_mul(x0, y1);
    x0          = nl3_move(a);
    a          -= nl3_mul(x1, x0);
    y1          = nl3_move(a);
    a          -= nl3_mul(y1, y0);
    x0          = f->bandW[0];
    a          += nl3_mul(x1, x0);
    b          -= a;
    y1          = n1;
    b          -= a;
    a           = nl3_acc(f->lowW[0]) + nl3_acc(y1);
    tNl3Word t  = nl3_move(b);

    b           = nl3_acc(x0);
    x0          = t;
    b          += nl3_mul(y0, x0);
    b          += nl3_mul(y0, x0);
    f->bandW[0] = nl3_move(b);
    x0          = nl3_move(b);
    a          -= nl3_mul(x0, c->n2w);
    tNl3Acc  o  = nl3_round(nl3_mul(nl3_move(a), c->gainw));

    o           = nl3_asl(o, 4);
    return nl3_move(o >> 2);
}

tNl3Word nl3_filter_process_fixed(tNl3Filter * f, const tNl3FilterCoeffs * c, tNl3Word x) {
    tNl3Acc b;

    switch (c->mode) {
        case eNl3FilterLp24:
            b = smooth_fixed(&f->inW[0], &f->meanW[0], nl3_acc(x) >> 2);
            (void)stage_fixed(&f->lowW[0], &f->bandW[0], c->fw, c->kw, &b, NULL);
            b = smooth_fixed(&f->inW[1], &f->meanW[1], nl3_asl(nl3_acc(f->lowW[0]), 1) >> 3);
            (void)stage_fixed(&f->lowW[1], &f->bandW[1], c->fw, c->kw, &b, NULL);
            return out_fixed(nl3_asl(nl3_acc(f->lowW[1]), 1), c->gainw);

        case eNl3FilterHp12:
            b = nl3_mul(c->n2w, nl3_move(nl3_acc(x) >> 5)) >> 1;
            (void)stage_fixed(&f->lowW[0], &f->bandW[0], c->fw, c->kw, &b, NULL);
            return out_fixed(nl3_asl(b, 1), c->gainw);

        case eNl3FilterHp24:
            b = nl3_mul(c->n2w, nl3_move(nl3_acc(x) >> 2)) >> 1;
            (void)stage_fixed(&f->lowW[0], &f->bandW[0], c->fw, c->kw, &b, NULL);
            b = nl3_mul(c->n2w, nl3_move(nl3_asl(b, 1) >> 3)) >> 1;
            (void)stage_fixed(&f->lowW[1], &f->bandW[1], c->fw, c->kw, &b, NULL);
            return out_fixed(nl3_asl(b, 1), c->gainw);

        case eNl3FilterLp6:
            return lp6_fixed(f, c, x);

        case eNl3FilterClassic:
            return classic_fixed(f, c, x);

        case eNl3FilterLpHpPar:
            return lphp_par_fixed(f, c, x);

        case eNl3FilterPeak:
            return allpass3_fixed(f, c, x, false);

        case eNl3FilterNotch:
            return allpass3_fixed(f, c, x, true);

        case eNl3FilterBr:
            return br_fixed(f, c, x);

        case eNl3FilterHp6:
            return hp6_fixed(f, c, x);

        case eNl3FilterBp24:   // notes §10: two band-pass stages, the first's band into the second
        {
            tNl3Acc  bandAcc = 0;
            tNl3Acc  cur     = nl3_mul(nl3_move(nl3_acc(x) >> 2), c->n2w);
            tNl3Word prev    = f->inW[0];

            f->inW[0] = nl3_move(cur);
            b         = (cur + nl3_acc(prev)) >> 1;
            (void)stage_fixed(&f->lowW[0], &f->bandW[0], c->fw, c->kw, &b, &bandAcc);
            cur       = nl3_mul(nl3_move(bandAcc >> 3), c->n2w);
            prev      = f->inW[1];
            f->inW[1] = nl3_move(cur);
            b         = (cur + nl3_acc(prev)) >> 1;
            (void)stage_fixed(&f->lowW[1], &f->bandW[1], c->fw, c->kw, &b, NULL);
            return out_fixed(nl3_acc(f->bandW[1]), c->gainw);
        }

        case eNl3FilterBp12:   // notes §9
        {
            tNl3Acc  cur  = nl3_mul(nl3_move(nl3_acc(x) >> 5), c->n2w);
            tNl3Word prev = f->inW[0];

            f->inW[0] = nl3_move(cur);
            b         = (cur + nl3_acc(prev)) >> 1;
            (void)stage_fixed(&f->lowW[0], &f->bandW[0], c->fw, c->kw, &b, NULL);
            return out_fixed(nl3_acc(f->bandW[0]), c->gainw);
        }

        case eNl3FilterLp12:
        default:
            b = smooth_fixed(&f->inW[0], &f->meanW[0], nl3_acc(x) >> 5);
            (void)stage_fixed(&f->lowW[0], &f->bandW[0], c->fw, c->kw, &b, NULL);
            return out_fixed(nl3_asl(nl3_acc(f->lowW[0]), 1), c->gainw);
    }
}
