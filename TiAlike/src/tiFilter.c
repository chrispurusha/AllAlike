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

#include <math.h>
#include <string.h>

#include "tiFilter.h"
#include "tiFixed.h"

#define POINT_1KHZ          (17)        // notes §1: third-octave centres, 1 kHz at point 17
#define TOP_POINT_HZ        (22968.75)
#define NYQUIST_SHARE       (0.49)
#define LADDER_G_LIFT       (1.00817)                      // notes §5: 2^((32/3) x 9/8192), the coefficient a shade above the exact resonance
#define LADDER_SCALE        TI_FIXED_UNIT                  // notes §6: the oscillator's level inside the ladder
#define LADDER_TAP1_GAIN    (0.70703125)                   // notes §5: the 1-pole tap's own scale
#define LADDER_NOISE        (1.0 / 16384.0 / 2147483648.0) // the LCG word >> 14, as a fraction

// notes §2: each resonance row's Q in dB, and its output gain
static const double kRowQdB[TI_FILTER_ROWS] = {-3.0103, -1.0, 4.0, 9.0, 14.0, 19.0, 24.0, 40.0, 48.0};
static const double kRowGainLpHp[TI_FILTER_ROWS] = {1.0, 0.875, 0.875, 0.75, 0.625, 0.515625, 0.40625, 0.34375, 0.25};
static const double kRowGainBp[TI_FILTER_ROWS]   = {1.0, 1.125, 2.0, 3.0, 4.0, 8.0, 12.0, 28.0, 64.0};

double ti_filter_cutoff_hz(double cutoff) {
    double p = 1.0 + (30.0 * cutoff / 128.0);

    return 1000.0 * pow(2.0, (p - POINT_1KHZ) / 3.0);
}

void ti_filter_tables(tTiFilterTables * t, double rate) {
    t->rate = rate;

    for (int p = 0; p < TI_FILTER_POINTS; p++) {
        double hz = (p == TI_FILTER_POINTS - 1) ? TOP_POINT_HZ : 1000.0 * pow(2.0, (p - POINT_1KHZ) / 3.0);
        double w  = 2.0 * M_PI * fmin(hz, NYQUIST_SHARE * rate) / rate;

        t->cosine[p] = cos(w);

        for (int r = 0; r < TI_FILTER_ROWS; r++) {
            double q     = pow(10.0, kRowQdB[r] / 20.0);
            double alpha = sin(w) / (2.0 * q);

            t->halfR2[r][p] = 0.5 * (1.0 - alpha) / (1.0 + alpha);
        }
    }
}

static double lerp(double a, double b, double f) {
    return a + (f * (b - a));
}

double ti_filter_row_gain(tTiFilterMode mode, double resonance) {
    double row = 8.0 * fmin(fmax(resonance, 0.0), 127.999) / 128.0;
    int    r   = (int)row;

    if (r > TI_FILTER_ROWS - 2) {
        r = TI_FILTER_ROWS - 2;
    }

    if (mode == eTiFilterBandPass) {
        return lerp(kRowGainBp[r], kRowGainBp[r + 1], row - r);
    }
    return (mode == eTiFilterBandStop) ? 1.0 : lerp(kRowGainLpHp[r], kRowGainLpHp[r + 1], row - r);
}

// notes §5: each resonance step's loop gain, the panel's 0..127 spread over 16 steps
static const double kLadderLoopGain[17] = {
    0.5, 0.90625, 1.3125, 1.71875, 2.125, 2.53125, 2.9375, 3.34375, 3.75, 4.0, 4.25, 4.5, 5.0, 6.0, 8.0, 16.0, 32.0
};

static void ladder_coeffs(const tTiFilterTables * t, tTiFilterMode mode, double cutoff, double resonance, double drive,
                          tTiFilterCoeffs * c) {
    double ctrl = fmin(fmax(cutoff, 0.0), 127.999) / 128.0;
    double hz   = fmin(1000.0 * pow(2.0, ((1.0 + 30.0 * ctrl) - POINT_1KHZ) / 3.0), NYQUIST_SHARE * 2.0 * t->rate);
    double w    = 2.0 * M_PI * hz / (2.0 * t->rate);
    double phi  = (M_PI - w) / 4.0;
    double c32  = (0.9375 * ctrl) - (5.0 / 256.0);
    double res  = fmin(fmax(resonance, 0.0), 127.999) / 128.0;
    double step = 16.0 * fmin(fmax(fmin(1.0 - ((7.0 / 17.0) * (c32 - (15.0 / 32.0))), res), 0.0), 0.9999);
    int    i    = (int)step;
    double d    = fmin(fmax(drive, 0.0), 1.0);

    c->poles    = (uint32_t)(mode - eTiFilterAnalog1) + 1u;
    c->g        = (1.0 - (tan(phi) / (sin(w) + (tan(phi) * cos(w))))) * LADDER_G_LIFT;   // notes §5: the loop resonates at hz
    c->feedback = lerp(kLadderLoopGain[i], kLadderLoopGain[i + 1], step - i) / 32.0;
    c->inGain   = pow(2.0, (2.0 * d) - 4.0);
    c->preGain  = pow(2.0, d + 7.0);
    c->postGain = pow(2.0, -3.0 - d);
    c->tapGain  = (c->poles == 1u) ? (c->postGain * LADDER_TAP1_GAIN) : 1.0;
}

// notes §3
void ti_filter_coeffs(const tTiFilterTables * t, tTiFilterMode mode, double cutoff, double resonance, double drive,
                      tTiFilterCoeffs * c) {
    if (mode >= eTiFilterAnalog1) {
        ladder_coeffs(t, mode, cutoff, resonance, drive, c);
        return;
    }
    c->poles = 0u;

    double point = 1.0 + (30.0 * fmin(fmax(cutoff, 0.0), 127.999) / 128.0);
    double row   = 8.0 * fmin(fmax(resonance, 0.0), 127.999) / 128.0;
    int    p = (int)point, r = (int)row;
    double pf = point - p, rf = row - r;

    if (r > TI_FILTER_ROWS - 2) {
        r  = TI_FILTER_ROWS - 2;
        rf = 1.0;
    }
    double cs    = lerp(t->cosine[p], t->cosine[p + 1], pf);
    double half  = lerp(lerp(t->halfR2[r][p], t->halfR2[r][p + 1], pf), lerp(t->halfR2[r + 1][p], t->halfR2[r + 1][p + 1], pf), rf);
    double r2    = 2.0 * half;

    c->a1    = -cs * (1.0 + r2);
    c->a2    = r2;

    switch (mode) {
        case eTiFilterHighPass:
        {
            double k = lerp(kRowGainLpHp[r], kRowGainLpHp[r + 1], rf) * (1.0 + r2) * (1.0 + cs) / 4.0;

            c->b0 = k;
            c->b1 = -2.0 * k;
            c->b2 = k;
            break;
        }
        case eTiFilterBandPass:
        {
            double k = lerp(kRowGainBp[r], kRowGainBp[r + 1], rf) * (1.0 - r2) / 2.0;

            c->b0 = k;
            c->b1 = 0.0;
            c->b2 = -k;
            break;
        }
        case eTiFilterBandStop:
        {
            // notes §3: the cookbook notch, its gain 1 at every resonance
            double k = (1.0 + r2) / 2.0;

            c->b0 = k;
            c->b1 = -2.0 * cs * k;
            c->b2 = k;
            break;
        }
        default:
        {
            double k = lerp(kRowGainLpHp[r], kRowGainLpHp[r + 1], rf) * (1.0 + r2) * (1.0 - cs) / 4.0;

            c->b0 = k;
            c->b1 = 2.0 * k;
            c->b2 = k;
            break;
        }
    }
}

void ti_filter_reset(tTiFilter * f) {
    memset(f, 0, sizeof(*f));
}

static double ladder_saturate(double x) {
    x = fmin(fmax(x, -1.0), 1.0);
    return (2.0 * x) - (x * fabs(x));
}

// notes §5: one pass of the ladder at twice the rate; returns the tap the mode names
static double ladder_pass(tTiFilter * f, const tTiFilterCoeffs * c, double u, double noise) {
    double q  = 1.0 - c->g;
    double fb = f->s[3] + noise;
    double t;

    f->s[0] = (q * f->s[0]) + (c->g * ((c->inGain * u) - (c->feedback * fb)));
    t       = ladder_saturate(c->preGain * f->s[0]);
    f->s[1] = (q * f->s[1]) + (c->g * c->postGain * t);
    f->s[2] = (q * f->s[2]) + (c->g * f->s[1]);
    f->s[3] = (q * f->s[3]) + (c->g * f->s[2]);
    return (c->poles == 1u) ? t : f->s[c->poles - 1u];
}

static double ladder_process(tTiFilter * f, const tTiFilterCoeffs * c, double x) {
    double u     = LADDER_SCALE * x;
    double noise = 0.0;

    f->noise  = (f->noise * 1664525u) + 1013904223u;
    noise     = (double)(int32_t)f->noise * LADDER_NOISE;

    (void)ladder_pass(f, c, 0.5 * (u + f->lastIn), noise);
    f->lastIn = u;
    return c->tapGain * ladder_pass(f, c, u, 0.0) / LADDER_SCALE;
}

// notes §7: after any change to a digital mode's coefficients
void ti_filter_set_fixed(tTiFilterCoeffs * c, bool fixed) {
    c->fixed = fixed && (c->poles == 0u);

    if (c->fixed) {
        c->w0  = ti_word(0.5 * c->b0);
        c->w1  = ti_word(0.5 * c->b1);
        c->w2  = ti_word(0.5 * c->b2);
        c->wa1 = ti_word(-0.5 * c->a1);
        c->wa2 = ti_word(-0.5 * c->a2);
    }
}

// notes §7
static double fixed_biquad(tTiFilter * f, const tTiFilterCoeffs * c, double x) {
    int32_t in  = ti_word(x * TI_FIXED_UNIT);
    int64_t acc = ti_mac(c->w0, in) + ti_mac(c->w1, f->wx1) + ti_mac(c->w2, f->wx2) + ti_mac(c->wa1, f->wy1) + ti_mac(c->wa2, f->wy2);
    int32_t y   = ti_round(acc * 2);    // scaled up: the coefficients are held at half

    f->wx2 = f->wx1;
    f->wx1 = in;
    f->wy2 = f->wy1;
    f->wy1 = y;
    return ((double)y / TI_Q23) / TI_FIXED_UNIT;
}

double ti_filter_process(tTiFilter * f, const tTiFilterCoeffs * c, double x) {
    if (c->poles != 0u) {
        return ladder_process(f, c, x);
    }

    if (c->fixed) {
        return fixed_biquad(f, c, x);
    }
    double y = (c->b0 * x) + (c->b1 * f->x1) + (c->b2 * f->x2) - (c->a1 * f->y1) - (c->a2 * f->y2);

    f->x2 = f->x1;
    f->x1 = x;
    f->y2 = f->y1;
    f->y1 = y;
    return y;
}
