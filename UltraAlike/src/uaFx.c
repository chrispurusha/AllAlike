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
// Notes: Docs/code-notes/uaFx.md - "// notes §k" refers there.

#include <math.h>
#include <pthread.h>
#include <string.h>

#include "uaFx.h"

#define SUB             (32)        // notes §2: frames between coefficient updates
#define SMOOTH_S        (0.010)     // how fast a cutoff or a gain follows its dial
#define TAPE_BIAS       (0.15)      // notes §3: Tape's lean, for its even harmonics
#define WARMTH_HZ       (150.0)
#define DC_HZ           (8.0)
#define ADAA_EPSILON    (1e-5)

// notes §1: Butterworth Qs - one stage for 12 dB/octave, two for 24
static const double   kQ2[1]    = {0.70710678};
static const double   kQ4[2]    = {0.54119610, 1.30656296};

void ua_fx_reset(tUaFxState * s) {
    memset(s, 0, sizeof(*s));
}

static inline double adaa(tUaSatMode mode, double x, double * last);

// ---- 2x oversampling - notes §6 -----------------------------------------------------------------------------

#define HB_TAPS    ((2 * UA_FX_HALF) - 1)    // 47: a half-band low-pass, its centre at 23
#define HB_BETA    (8.0)

static double         gHalf[UA_FX_HALF];     // the even taps; every odd one is 0 but the centre, which is 1/2
static pthread_once_t gHalfOnce = PTHREAD_ONCE_INIT;

static double bessel_i0(double x) {
    double sum = 1.0, term = 1.0;

    for (int k = 1; k < 50; k++) {
        term *= (x / (2.0 * k)) * (x / (2.0 * k));
        sum  += term;
    }

    return sum;
}

static void build_half(void) {
    double centre = (HB_TAPS - 1) / 2.0, sum = 0.5;

    for (int j = 0; j < UA_FX_HALF; j++) {
        double u = (2.0 * j) - centre;   // odd: a half-band's taps at even distances from the centre are all 0
        double r = u / centre;

        gHalf[j] = 0.5 * (sin(M_PI * u / 2.0) / (M_PI * u / 2.0)) * bessel_i0(HB_BETA * sqrt(1.0 - (r * r))) / bessel_i0(HB_BETA);
        sum     += gHalf[j];
    }

    for (int j = 0; j < UA_FX_HALF; j++) {
        gHalf[j] /= sum;   // unity at DC, with the centre's 1/2
    }
}

static inline double ring(const double * r, int newest, int back) {
    return r[(newest - back + UA_FX_HALF) % UA_FX_HALF];
}

// One base-rate sample through the curve at twice the rate: up, the curve twice, down. UA_FX_LATENCY late.
static double oversampled(tUaFxState * s, int c, tUaSatMode mode, double x) {
    int    n = s->at;
    double e = 0.0, y = 0.0;

    s->up[c][n]   = x;

    for (int j = 0; j < UA_FX_HALF; j++) {
        e += gHalf[j] * ring(s->up[c], n, j);
    }

    // the even phase is the filter's even taps; the odd phase is only its centre - the input, 11 frames back
    s->even[c][n] = adaa(mode, 2.0 * e, &s->last[c]);
    s->odd[c][n]  = adaa(mode, ring(s->up[c], n, 11), &s->last[c]);

    for (int j = 0; j < UA_FX_HALF; j++) {
        y += gHalf[j] * ring(s->even[c], n, j);
    }

    return y + (0.5 * ring(s->odd[c], n, 12));
}

// The same lateness, for a layer not saturating while oversampling is on
static double delayed(tUaFxState * s, int c, double x) {
    double y = s->delay[c][s->delayAt];

    s->delay[c][s->delayAt] = x;
    return y;
}

// ---- the filter - notes §1, §2 ------------------------------------------------------------------------------

typedef struct {
    double a1, a2, a3, k;
} tCoef;

static tCoef coef(double hz, double q, double rate) {
    double g  = tan(M_PI * fmin(hz, 0.49 * rate) / rate);
    double k  = 1.0 / q;
    double a1 = 1.0 / (1.0 + (g * (g + k)));

    return (tCoef){
        a1, g * a1, g * g * a1, k
    };
}

// One sample through a stage: its low-pass and high-pass outputs
static inline void svf(tUaSvf * f, const tCoef * c, double x, double * lp, double * hp) {
    double v3 = x - f->ic2;
    double v1 = (c->a1 * f->ic1) + (c->a2 * v3);
    double v2 = f->ic2 + (c->a2 * f->ic1) + (c->a3 * v3);

    f->ic1 = (2.0 * v1) - f->ic1;
    f->ic2 = (2.0 * v2) - f->ic2;
    *lp    = v2;
    *hp    = x - (c->k * v1) - v2;
}

// notes §5: a trapezoidal one-pole stage - its low-pass, or its high-pass
static inline double one_pole(double * state, double g, double x, bool lowPass) {
    double v  = (x - *state) * (g / (1.0 + g));
    double lp = v + *state;

    *state = lp + v;
    return lowPass ? lp : x - lp;
}

// ---- the shapers and their antiderivatives - notes §4 -------------------------------------------------------

static inline double shape(tUaSatMode mode, double x) {
    switch (mode) {
        case eUaSatDrums:   return fmax(-1.0, fmin(1.0, x));

        case eUaSatTape:    return tanh(x + TAPE_BIAS) - tanh(TAPE_BIAS);

        default:            return tanh(x);
    }
}

static inline double log_cosh(double x) {
    double a = fabs(x);

    return a + log1p(exp(-2.0 * a)) - M_LN2;
}

static inline double antiderivative(tUaSatMode mode, double x) {
    switch (mode) {
        case eUaSatDrums:
        {
            double a = fabs(x);

            return (a <= 1.0) ? 0.5 * x * x : a - 0.5;
        }

        case eUaSatTape:    return log_cosh(x + TAPE_BIAS) - (x * tanh(TAPE_BIAS));

        default:            return log_cosh(x);
    }
}

// First-order antiderivative anti-aliasing: the mean of the curve between this input and the last
static inline double adaa(tUaSatMode mode, double x, double * last) {
    double x0 = *last;
    double d  = x - x0;
    double y  = (fabs(d) < ADAA_EPSILON) ? shape(mode, 0.5 * (x + x0))
                                         : (antiderivative(mode, x) - antiderivative(mode, x0)) / d;

    *last = x;
    return y;
}

// ---- the whole chain ----------------------------------------------------------------------------------------

static double db(double decibels) {
    return pow(10.0, decibels / 20.0);
}

void ua_fx_process(tUaFxState * s, const tUaFx * fx, double rate, float * left, float * right, uint32_t frames) {
    int    channels = (right != NULL) ? 2 : 1;
    double k        = 1.0 - exp(-1.0 / (SMOOTH_S * rate));
    double kSub     = 1.0 - exp(-(double)SUB / (SMOOTH_S * rate));
    double gain = db(fx->gainDb), output = db(fx->outputDb), warmth = db(fx->warmthDb);
    double shelfK   = 1.0 - exp(-2.0 * M_PI * WARMTH_HZ / rate);
    double dcR      = 1.0 - (2.0 * M_PI * DC_HZ / rate);

    pthread_once(&gHalfOnce, build_half);

    if (!s->started) {
        s->lowHz   = fx->lowHz;
        s->highHz  = fx->highHz;
        s->hfHz    = fx->hfHz;
        s->gain    = gain;
        s->output  = output;
        s->warmth  = warmth;
        s->started = true;
    }

    for (uint32_t at = 0; at < frames; at += SUB) {
        uint32_t n      = ((frames - at) < SUB) ? (frames - at) : SUB;
        bool     soft   = (fx->slope == eUaSlopeSoft);
        int      stages = (fx->slope == eUaSlope24) ? 2 : 1;
        double   gLow   = tan(M_PI * fmin(s->lowHz, 0.49 * rate) / rate);    // Soft's one-pole stages
        double   gHigh  = tan(M_PI * fmin(s->highHz, 0.49 * rate) / rate);
        tCoef    low[2], high[2], hf;

        // notes §2: cutoffs glide in octaves, the coefficients worked out once a sub-block
        s->lowHz  = exp(log(s->lowHz) + ((log(fx->lowHz) - log(s->lowHz)) * kSub));
        s->highHz = exp(log(s->highHz) + ((log(fx->highHz) - log(s->highHz)) * kSub));
        s->hfHz   = exp(log(s->hfHz) + ((log(fx->hfHz) - log(s->hfHz)) * kSub));

        for (int st = 0; st < stages; st++) {
            double q = (fx->slope == eUaSlope24) ? kQ4[st] : kQ2[0];

            low[st]  = coef(s->lowHz, q, rate);
            high[st] = coef(s->highHz, q, rate);
        }

        hf        = coef(s->hfHz, kQ2[0], rate);

        for (uint32_t i = 0; i < n; i++) {
            s->gain   += (gain - s->gain) * k;
            s->output += (output - s->output) * k;
            s->warmth += (warmth - s->warmth) * k;

            for (int c = 0; c < channels; c++) {
                float * io = (c == 0) ? left : right;
                double  x = io[at + i], lp, hp;

                if (fx->filter && soft) {
                    x = one_pole(&s->low[c][0].ic1, gLow, x, false);   // notes §5: two for Low Cut, one for High Cut
                    x = one_pole(&s->low[c][1].ic1, gLow, x, false);
                    x = one_pole(&s->high[c][0].ic1, gHigh, x, true);
                } else if (fx->filter) {
                    for (int st = 0; st < stages; st++) {
                        svf(&s->low[c][st], &low[st], x, &lp, &hp);
                        x = hp;   // Low Cut: the high-pass
                    }

                    for (int st = 0; st < stages; st++) {
                        svf(&s->high[c][st], &high[st], x, &lp, &hp);
                        x = lp;   // High Cut: the low-pass
                    }
                }

                if (fx->saturate) {
                    if (fx->mode == eUaSatTape) {
                        s->shelf[c] += (x - s->shelf[c]) * shelfK;   // notes §3: Warmth - the lows up or down
                        x           += (s->warmth - 1.0) * s->shelf[c];
                    }
                    x  = fx->oversample ? oversampled(s, c, fx->mode, x * s->gain) : adaa(fx->mode, x * s->gain, &s->last[c]);

                    if (fx->mode == eUaSatTape) {
                        double y = x - s->dc[c][0] + (dcR * s->dc[c][1]);   // the lean's offset taken out

                        s->dc[c][0] = x;
                        s->dc[c][1] = y;
                        svf(&s->tapeHf[c], &hf, y, &lp, &hp);
                        x           = lp;
                    }
                    x *= s->output;
                } else if (fx->oversample) {
                    x = delayed(s, c, x);
                }
                io[at + i] = (float)x;
            }

            if (fx->oversample) {
                s->at      = (s->at + 1) % UA_FX_HALF;
                s->delayAt = (s->delayAt + 1) % UA_FX_LATENCY;
            }
        }
    }
}
