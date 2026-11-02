/*
 * TiAlike - the effects.
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
// Notes: Docs/code-notes/tiEffects.md - "// notes §k" refers there.

#include <math.h>
#include <stdbool.h>
#include <string.h>

#include "tiEffects.h"
#include "tiLfo.h"
#include "tiVoices.h"

#define SHELF_SLOPE       (0.5)      // notes §1
#define LOW_CORNER        (1.75)
#define MID_CENTRE        (0.913)
#define DIST_UNIT         (0.54)
#define DIST_HIGH_PASS    (14.7)
#define DIST_QUANTISED    (1.026)
#define VERB_LEVEL        (0.87)
#define BANK_LEVEL        (1.0)
#define BANK_FLAT         (1.0) // a section\'s damping at its lowest Q
#define PHASER_TICK       (32u)

void ti_effects_init(tTiEffects * e, double rate) {
    memset(e, 0, sizeof(*e));
    e->rate = rate;
}

// notes §15: empty lines and filters for a new sound; the rate and the plug-in's settings stay
void ti_effects_reset(tTiEffects * e) {
    double rate          = e->rate;
    double bpm           = e->bpm;
    bool   delayAllowed  = e->delayAllowed;
    bool   reverbAllowed = e->reverbAllowed;

    ti_effects_init(e, rate);
    e->bpm           = bpm;
    e->delayAllowed  = delayAllowed;
    e->reverbAllowed = reverbAllowed;
}

static void set_coeffs(tTiBiquad * q, double b0, double b1, double b2, double a0, double a1, double a2) {
    q->b0 = b0 / a0;
    q->b1 = b1 / a0;
    q->b2 = b2 / a0;
    q->a1 = a1 / a0;
    q->a2 = a2 / a0;
}

// notes §1: the cookbook's shelves and peak
static void low_shelf(tTiBiquad * q, double hz, double dB, double rate) {
    double A  = pow(10.0, dB / 40.0);
    double w  = 2.0 * M_PI * hz / rate;
    double c  = cos(w);
    double al = (sin(w) / 2.0) * sqrt(((A + (1.0 / A)) * ((1.0 / SHELF_SLOPE) - 1.0)) + 2.0);
    double s  = 2.0 * sqrt(A) * al;

    set_coeffs(q, A * ((A + 1.0) - ((A - 1.0) * c) + s), 2.0 * A * ((A - 1.0) - ((A + 1.0) * c)), A * ((A + 1.0) - ((A - 1.0) * c) - s),
               (A + 1.0) + ((A - 1.0) * c) + s, -2.0 * ((A - 1.0) + ((A + 1.0) * c)), (A + 1.0) + ((A - 1.0) * c) - s);
}

static void high_shelf(tTiBiquad * q, double hz, double dB, double rate) {
    double A  = pow(10.0, dB / 40.0);
    double w  = 2.0 * M_PI * hz / rate;
    double c  = cos(w);
    double al = (sin(w) / 2.0) * sqrt(((A + (1.0 / A)) * ((1.0 / SHELF_SLOPE) - 1.0)) + 2.0);
    double s  = 2.0 * sqrt(A) * al;

    set_coeffs(q, A * ((A + 1.0) + ((A - 1.0) * c) + s), -2.0 * A * ((A - 1.0) + ((A + 1.0) * c)), A * ((A + 1.0) + ((A - 1.0) * c) - s),
               (A + 1.0) - ((A - 1.0) * c) + s, 2.0 * ((A - 1.0) - ((A + 1.0) * c)), (A + 1.0) - ((A - 1.0) * c) - s);
}

static void peak(tTiBiquad * q, double hz, double dB, double Q, double rate) {
    double A  = pow(10.0, dB / 40.0);
    double w  = 2.0 * M_PI * hz / rate;
    double c  = cos(w);
    double al = sin(w) / (2.0 * Q);

    set_coeffs(q, 1.0 + (al * A), -2.0 * c, 1.0 - (al * A), 1.0 + (al / A), -2.0 * c, 1.0 - (al / A));
}

static void run(tTiBiquad * q, float * io, uint32_t frames, int ch) {
    double x1 = q->x1[ch], x2 = q->x2[ch], y1 = q->y1[ch], y2 = q->y2[ch];

    for (uint32_t f = 0; f < frames; f++) {
        double x = io[f];
        double y = (q->b0 * x) + (q->b1 * x1) + (q->b2 * x2) - (q->a1 * y1) - (q->a2 * y2);

        x2    = x1;
        x1    = x;
        y2    = y1;
        y1    = y + 1e-30;
        io[f] = (float)y;
    }

    q->x1[ch] = x1;
    q->x2[ch] = x2;
    q->y1[ch] = y1;
    q->y2[ch] = y2;
}

static double eq_db(uint8_t byte) {
    return ((double)byte - 64.0) * 0.25;
}

// notes §1
static void equalise(tTiEffects * e, const uint8_t * b, float * left, float * right, uint32_t frames) {
    double nyquist = 0.45 * e->rate;

    if (!e->ready || (e->lowKey[0] != b[95]) || (e->lowKey[1] != b[45])) {
        low_shelf(&e->low, LOW_CORNER * 32.0 * pow(1.02117, b[45]), eq_db(b[95]), e->rate);
        e->lowKey[0] = b[95];
        e->lowKey[1] = b[45];
    }

    if (!e->ready || (e->highKey[0] != b[96]) || (e->highKey[1] != b[46])) {
        high_shelf(&e->high, fmin((1831.0 * pow(1.02045, b[46])) / LOW_CORNER, nyquist), eq_db(b[96]), e->rate);
        e->highKey[0] = b[96];
        e->highKey[1] = b[46];
    }

    if (!e->ready || (e->midKey[0] != b[92]) || (e->midKey[1] != b[93]) || (e->midKey[2] != b[94])) {
        peak(&e->mid, fmin(MID_CENTRE * 19.0 * pow(1.0581, b[93]), nyquist), eq_db(b[92]), 0.142 * exp(0.0178 * b[94]), e->rate);
        e->midKey[0] = b[92];
        e->midKey[1] = b[93];
        e->midKey[2] = b[94];
    }
    e->ready = 1u;

    if (b[95] != 64) {
        run(&e->low, left, frames, 0);
        run(&e->low, right, frames, 1);
    }

    if (b[92] != 64) {
        run(&e->mid, left, frames, 0);
        run(&e->mid, right, frames, 1);
    }

    if (b[96] != 64) {
        run(&e->high, left, frames, 0);
        run(&e->high, right, frames, 1);
    }
}

// notes §2: Low Pass and High Pass are first-order filters, their corners moved by Intensity
static double corner_of(const double * hz, double intensity) {
    static const double kAt[6] = {0, 16, 48, 80, 112, 127};

    for (int i = 1; i < 6; i++) {
        if (intensity <= kAt[i]) {
            return hz[i - 1] * pow(hz[i] / hz[i - 1], (intensity - kAt[i - 1]) / (kAt[i] - kAt[i - 1]));
        }
    }

    return hz[5];
}

double ti_dist_corner(bool low, double intensity) {
    static const double kLow[6]  = {20000.0, 15000.0, 1700.0, 285.0, 47.0, 21.0};
    static const double kHigh[6] = {10.0, 50.0, 280.0, 1900.0, 10500.0, 44000.0};

    return corner_of(low ? kLow : kHigh, intensity);
}

static void one_pole(double hz, bool low, double mix, double rate, double * state, float * io, uint32_t frames) {
    double a = 1.0 - exp(-2.0 * M_PI * fmin(hz, 0.45 * rate) / rate);

    for (uint32_t f = 0; f < frames; f++) {
        double x = io[f];

        *state += a * (x - *state);
        io[f]   = (float)(x + (mix * ((low ? *state : (x - *state)) - x)));
    }
}

// notes §12
double ti_dist_high_pass_pole(double amount, double rate) {
    double hz = (amount <= 0.0) ? 0.0 : fmin(711.0 * pow(6.0, (amount - 64.0) / 32.0), 0.4715 * 48000.0) * rate / 48000.0;
    double t  = tan(M_PI * hz / rate);

    return (1.0 - t) / (1.0 + t);
}

static void high_pass(double amount, double mix, double rate, double * state, double * last, float * io, uint32_t frames) {
    double p = ti_dist_high_pass_pole(amount, rate);
    double g = 0.5 * (1.0 + p);

    for (uint32_t f = 0; f < frames; f++) {
        double x = io[f];

        *state = (g * (x - *last)) + (p * *state);
        *last  = x;
        io[f]  = (float)(x + (mix * (*state - x)));
    }
}

// notes §2: every curve but the filters and the Overdrives, as its parameters at one Intensity
typedef struct {
    uint8_t curve;
    double  drive;
    double  w, level;
    double  step;
    double  hold;
} tDistShape;

static double breakpoints(const double * at, const double * log2To, int n, double intensity) {
    for (int i = 1; i < n; i++) {
        if (intensity <= at[i]) {
            return exp2(log2To[i - 1] + (((intensity - at[i - 1]) / (at[i] - at[i - 1])) * (log2To[i] - log2To[i - 1])));
        }
    }

    return exp2(log2To[n - 1]);
}

#define HOLD_SCALE    (1.03)   // tiVoices notes §25

static void shape_at(uint8_t curve, double intensity, tDistShape * d) {
    static const double kOldStepAt[] = {0, 24, 32, 40, 48, 56, 64, 72, 80, 88, 96, 104, 112, 120, 127};
    static const double kOldStep[]   = {-9.5, -6.74, -6.15, -5.74, -5.16, -4.89, -4.57, -4.38, -4.16, -3.89, -3.57, -3.38, -3.16, -2.74, -2.24};
    static const double kOldHoldAt[] = {0, 8, 16, 127};
    static const double kOldHold[]   = {0.0, 1.63, 2.18, 6.99};
    static const double kHoldAt[]    = {0, 24, 112, 127};
    static const double kHold[]      = {0.0, 1.52, 8.85, 10.1};

    d->curve = curve;
    d->drive = intensity / 127.0;
    d->w     = 7.3 * pow(10.0, (intensity - 48.0) / 32.0);
    d->level = 0.14 + (0.415 * pow(3.2, (32.0 - intensity) / 16.0));

    if (curve == 6) {
        d->w     = 14.0 * pow(10.0, 0.9 * d->drive);
        d->level = 0.1148 * (1.0 - (0.245 * d->drive));
    } else if (curve == 8) {
        d->step = breakpoints(kOldStepAt, kOldStep, 15, intensity);
    } else if (curve == 9) {
        d->hold = breakpoints(kOldHoldAt, kOldHold, 4, intensity);
    } else if (curve == 18) {
        d->hold = breakpoints(kHoldAt, kHold, 4, intensity);
    } else if (curve == 19) {
        d->step = (intensity <= 64.0) ? (0.04317 * exp2((intensity - 64.0) / 12.0)) : (0.04317 * exp2((intensity - 64.0) / 24.0));
    }
}

double ti_dist_step(double intensity) {
    tDistShape d;

    shape_at(8, intensity, &d);
    return d.step;
}

double ti_dist_hold(double intensity) {
    tDistShape d;

    shape_at(9, intensity, &d);
    return HOLD_SCALE * d.hold;
}

static double triangle(double phase) {
    return (2.0 / M_PI) * asin(sin(phase));
}

// notes §2
static double dist_shape(const tDistShape * d, double x) {
    double v = d->w * x;

    switch (d->curve) {
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
            return ti_saturate_curve(x, d->curve, d->drive, DIST_UNIT);

        case 6:
        case 15:
            return d->level * sin(v);

        case 7:
            return ((1.0 - (d->drive * d->drive)) * x) + (d->drive * (1.0 + (0.95 * d->drive)) * (0.18 - fabs(x)));

        case 8:
        case 19:
            return DIST_QUANTISED * floor((x / d->step) + 0.5) * d->step;

        case 12:
        case 20:
        case 21:
        case 22:
        case 23:
        case 24:
        case 25:
            return d->level * sin(fmin(fmax(v, -0.5 * M_PI), 0.5 * M_PI));

        case 13:
            return d->level * sin(fmin(fmax(v, -1.5 * M_PI), 1.5 * M_PI));

        case 14:
            return d->level * triangle(fmin(fmax(0.5 * M_PI * v, -1.5 * M_PI), 1.5 * M_PI));

        case 16:
            return d->level * triangle(0.5 * M_PI * v);

        case 17:
            return d->level * ((2.0 * (((0.5 * v) + 0.5) - floor((0.5 * v) + 0.5))) - 1.0);

        default:
            return x;
    }
}

// notes §2: a low Intensity where nothing has reached the curve yet runs straight through
static bool dist_direct(uint8_t curve, double intensity) {
    switch (curve) {
        case 7:
        case 8:
        case 9:
        case 18:
        case 19:
            return true;

        case 14:
        case 16:
        case 17:
            return intensity <= 36.0;

        case 13:
        case 15:
            return intensity <= 16.0;

        default:
            return false;
    }
}

typedef struct {
    double b0, b1, b2, a1, a2;
} tBiquadCoeffs;

// notes §2: each Overdrive's filter ahead of the shaper
typedef struct {
    double gain, lowCut, highCut, q, afterDb, postHz;   // postHz: a pole after the shaper, 0 none
} tOverdrive;

static const tOverdrive kOverdrive[5] = {
    {12.8121, 4980.7,  5615.1, 0.1971, -0.24, 350.0},
    { 0.8958,   87.0, 22808.0, 0.0114,  0.00,   0.0},
    {40.6764, 6000.0, 18243.1, 0.0200,  8.76, 350.0},
    { 2.1389,  172.6,  2436.5, 0.2441,  6.48, 350.0},
    { 2.4389,  959.3,  5654.4, 1.0593, -0.23, 550.0},
};

static double overdrive_filter(tTiEffects * e, int ch, const tBiquadCoeffs * top, double lowCut, double gain, double x) {
    double * s = e->driveState[ch];
    double   y = (top->b0 * x) + s[0];

    s[0]  = (top->b1 * x) - (top->a1 * y) + s[1];
    s[1]  = (top->b2 * x) - (top->a2 * y);
    s[2] += lowCut * (y - s[2]);
    return gain * (y - s[2]);
}

// notes §2: the shaper, a delay and a high-pass
static void shaped(tTiEffects * e, int ch, uint8_t curve, double intensity, double mix, const uint8_t * c, float * io, uint32_t frames) {
    tDistShape         d;
    double             a       = 1.0 - exp(-2.0 * M_PI * DIST_HIGH_PASS / e->rate);
    bool               direct  = dist_direct(curve, intensity);

    const tOverdrive * od      = ((curve >= 20) && (curve <= 24)) ? &kOverdrive[curve - 20] : NULL;
    tBiquadCoeffs      top     = {0};
    double             lowCut = 0.0, after = 1.0, postPole = 0.0;

    shape_at(curve, intensity, &d);

    // notes §2: Treble Booster tilts what goes in, High Cut smooths what comes out
    double             m       = c[70] / 127.0;
    double             lows    = 1.0 - (0.75 * m);
    double             highs   = 1.0 + (2.4 * m);
    double             tilt    = 1.0 - exp(-2.0 * M_PI * 130.0 * pow(fmax(m, 0.01) / 0.25, 1.2) / e->rate);
    double             w       = 2.0 * M_PI * fmin(750.0 * pow(2.0, (127.0 - (double)c[71]) / 21.0), 0.45 * e->rate) / e->rate;
    double             al      = sin(w) / 1.414;
    double             co      = cos(w);
    double             n0      = 1.0 + al;
    double             cutB    = 0.5 * (1.0 - co) / n0;
    double             cutA1   = -2.0 * co / n0;
    double             cutA2   = (1.0 - al) / n0;
    bool               reducer = (curve == 8) || (curve == 9) || (curve == 18) || (curve == 19);   // notes §2
    bool               useTilt = (c[70] != 0) && !reducer;
    bool               useCut  = (c[71] != 0) && !reducer;

    if (od != NULL) {
        double w  = 2.0 * M_PI * od->highCut / e->rate;
        double al = sin(w) / (2.0 * od->q);
        double c  = cos(w);
        double n  = 1.0 + al;

        top.b0   = 0.5 * (1.0 - c) / n;
        top.b1   = (1.0 - c) / n;
        top.b2   = top.b0;
        top.a1   = -2.0 * c / n;
        top.a2   = (1.0 - al) / n;
        lowCut   = 1.0 - exp(-2.0 * M_PI * od->lowCut / e->rate);
        after    = pow(10.0, od->afterDb * fmin(intensity / 48.0, 1.0) / 20.0);
        postPole = (od->postHz > 0.0) ? (1.0 - exp(-2.0 * M_PI * od->postHz / e->rate)) : 0.0;
    }

    for (uint32_t f = 0; f < frames; f++) {
        double x = io[f];
        double y;

        if ((curve == 9) || (curve == 18)) {
            e->distHeld[ch] += 1.0;

            if (e->distHeld[ch] >= (d.hold * e->rate / 48000.0)) {
                e->distHeld[ch] -= d.hold * e->rate / 48000.0;
                e->distState[ch] = x;
            }
            e->distHigh[ch] += a * (e->distState[ch] - e->distHigh[ch]);
            y                = e->distState[ch] - e->distHigh[ch];
        } else {
            double in = x;

            if (useTilt) {
                e->treble[ch] += tilt * (in - e->treble[ch]);
                in             = (lows * e->treble[ch]) + (highs * (in - e->treble[ch]));
            }
            in = (od != NULL) ? overdrive_filter(e, ch, &top, lowCut, od->gain, in) : in;

            y  = after * dist_shape(&d, in);

            if (postPole > 0.0) {
                e->driveState[ch][3] += postPole * (y - e->driveState[ch][3]);
                y                     = e->driveState[ch][3];
            }
        }

        if (!direct) {
            double out = e->distLine[ch][e->distAt[ch]];

            e->distLine[ch][e->distAt[ch]] = (float)y;
            e->distAt[ch]                  = (e->distAt[ch] + 1u) % TI_DIST_DELAY;
            e->distHigh[ch]               += a * (out - e->distHigh[ch]);
            y                              = out - e->distHigh[ch];
        }

        if (useCut) {
            double * h   = e->highCut[ch];
            double   out = (cutB * y) + h[0];

            h[0] = (2.0 * cutB * y) - (cutA1 * out) + h[1];
            h[1] = (cutB * y) - (cutA2 * out);
            y    = out;
        }
        io[f] = (float)(x + (mix * (y - x)));
    }
}

static void distort(tTiEffects * e, int ch, uint8_t version, const uint8_t * b, const uint8_t * c, float * io, uint32_t frames) {
    int    curve = (int)b[100];
    double mix   = (version < 9) ? 1.0 : (c[72] / 127.0);                 // notes §2

    if ((curve < 1) || (curve > TI_DIST_CURVES) || (mix == 0.0)) {
        return;
    }

    if (curve == 10) {
        one_pole(ti_dist_corner(true, b[101]), true, mix, e->rate, &e->distState[ch], io, frames);
    } else if (curve == 11) {
        high_pass(b[101], mix, e->rate, &e->distState[ch], &e->distLast[ch], io, frames);
    } else {
        shaped(e, ch, (uint8_t)curve, b[101], mix, c, io, frames);
    }
}

// notes §3: coefficients every so often, as the corner moves
// notes §13
static double chorus_rate(double dial) {
    static const double kAt[9] = {0, 16, 28, 40, 52, 64, 76, 88, 100};
    static const double kTo[9] = {-6.299, -4.071, -3.540, -2.542, -1.694, -0.876, -0.082, 0.710, 1.635};

    return (dial >= 100.0) ? (0.5 * ti_lfo_rate_hz(dial)) : breakpoints(kAt, kTo, 9, dial);
}

#define PHASER_RIGHT    (1.11)   // notes §14

// notes §14: an octave-straight triangle up from the Frequency, starting at the top
static void phaser_tune(tTiEffects * e, const uint8_t * b, int sections) {
    double p      = e->phaseLfo;
    double up     = (p < 0.5) ? (1.0 - (2.0 * p)) : ((2.0 * p) - 1.0);
    double floor_ = 393.0 * pow(2.3, ((double)b[88] - 48.0) / 16.0);
    double centre = floor_ * exp2(0.0465 * b[87] * up);
    double spread = (b[90] <= 96) ? 0.0 : (((double)b[90] - 96.0) / 31.0);
    double span   = pow(5.0, spread);

    e->phaseLfo += chorus_rate(b[86]) * PHASER_TICK / e->rate;
    e->phaseLfo -= floor(e->phaseLfo);

    for (int ch = 0; ch < 2; ch++) {
        for (int i = 0; i < sections; i++) {
            double place = (sections > 1) ? (((2.0 * i) / (sections - 1.0)) - 1.0) : 0.0;
            double hz    = fmin(centre * ((ch == 0) ? 1.0 : PHASER_RIGHT) * pow(span, place), 0.45 * e->rate);
            double t     = tan(M_PI * hz / e->rate);

            e->apCoeff[ch][i] = (t - 1.0) / (t + 1.0);
        }
    }
}

static void phaser(tTiEffects * e, const uint8_t * b, float * left, float * right, uint32_t frames) {
    if (b[85] == 0) {
        return;
    }
    int    sections = 2 * (((b[84] > 5) ? 5 : b[84]) + 1);
    double wet      = sin(0.5 * M_PI * b[85] / 127.0);    // notes §3: equal power
    double dry      = cos(0.5 * M_PI * b[85] / 127.0);
    double amount   = ((double)b[89] - 64.0) / 64.0;
    double feedback = 0.89 * amount;
    int    tap      = (sections <= 2) ? 2 : ((sections >= 10) ? 8 : (sections - 2));   // notes §14

    for (uint32_t f = 0; f < frames; f++) {
        if (e->phaserLeft == 0u) {
            e->phaserLeft = PHASER_TICK;
            phaser_tune(e, b, sections);
        }
        e->phaserLeft--;

        for (int ch = 0; ch < 2; ch++) {
            float * io = (ch == 0) ? left : right;
            double  x  = io[f];
            double  y  = x + (feedback * e->phaserLast[ch]);

            for (int i = 0; i < sections; i++) {
                double out = (e->apCoeff[ch][i] * y) + e->apX[ch][i] - (e->apCoeff[ch][i] * e->apY[ch][i]);

                e->apX[ch][i] = y;
                e->apY[ch][i] = out + 1e-30;
                y             = out;

                if (i == (tap - 1)) {
                    e->phaserLast[ch] = out;
                }
            }

            io[f] = (float)((dry * x) + (wet * y));
        }
    }
}

// notes §13: 1 at the longest delay, all starting from phase 0
static double chorus_wave(double phase, uint8_t shape) {
    switch (shape) {
        case 0:  return cos(2.0 * M_PI * phase);

        case 2:  return 1.0 - (2.0 * ((phase < 0.5) ? (phase + 0.5) : (phase - 0.5)));

        case 3:  return (phase < 0.5) ? -1.0 : 1.0;

        default: return (phase < 0.5) ? (1.0 - (4.0 * phase)) : ((4.0 * phase) - 3.0);
    }
}

// notes §4: a line read at a moving delay, the two sides in antiphase
// notes §13: a Single older than format 12 has no Chorus Type; Mix alone turns it on
static bool chorus_active(const uint8_t * a) {
    return (a[105] != 0) && ((a[103] != 0) || (a[0] < 12));
}

static void chorus(tTiEffects * e, const uint8_t * a, float * left, float * right, uint32_t frames) {
    if (!chorus_active(a)) {
        return;
    }
    double m     = a[105] / 127.0;
    double dry   = cos(0.5 * M_PI * m);
    double wet   = sin(0.5 * M_PI * m);
    double delay = 8.0 * a[108] * e->rate / 48000.0;
    double swing = 0.5 * delay * (a[107] / 127.0);   // notes §13
    double rate  = chorus_rate(a[106]);
    double back  = 0.85 * ((double)a[109] - 64.0) / 64.0;

    for (uint32_t f = 0; f < frames; f++) {
        for (int ch = 0; ch < 2; ch++) {
            float *  io   = (ch == 0) ? left : right;
            double   wave = chorus_wave(e->chorusLfo, a[110]);
            double   at   = delay - swing + (((ch == 0) ? swing : -swing) * wave);
            double   from = (double)e->chorusAt + TI_CHORUS_LINE - fmin(fmax(at, 1.0), TI_CHORUS_LINE - 2.0);
            uint32_t i    = (uint32_t)from;
            double   frac = from - i;
            double   x    = io[f];
            double   y    = e->chorusLine[ch][i % TI_CHORUS_LINE] + (frac * (e->chorusLine[ch][(i + 1u) % TI_CHORUS_LINE] - e->chorusLine[ch][i % TI_CHORUS_LINE]));

            e->chorusLine[ch][e->chorusAt] = (float)(x + (back * y));
            io[f]                          = (float)((dry * x) + (wet * y));
        }

        e->chorusAt   = (e->chorusAt + 1u) % TI_CHORUS_LINE;
        e->chorusLfo += rate / e->rate;
        e->chorusLfo -= floor(e->chorusLfo);
    }
}

// notes §5: each type's level in dB at octaves from 33 Hz
static const double kCharacter[7][8]   = {
    { 0.0,  0.0,  0.0,  0.0,  0.0,  0.0,  0.0,  0.0},
    { 0.0,  0.2,  0.5,  0.9,  1.0,  0.9,  0.6,  0.0},
    {-1.8, -1.3, -0.1,  1.1,  1.5,  1.0, -0.4, -2.7},
    { 0.0,  0.0,  0.0,  0.0,  0.0,  0.0,  0.0,  0.0},
    {-1.3, -3.5, -1.0, -2.5, -1.7, -0.1,  1.1,  2.8},
    {-0.3,  0.9,  0.9,  0.1, -1.4, -2.1, -0.3,  4.7},
    {-5.3, -8.1, -3.3,  1.9,  2.5,  3.3,  0.5, -2.4},
};

// notes §5: Speaker Cabinet's fixed low pair, and laws in Tune for its numerator and moving pair
static const double kCabinetWarp[3]    = {0.111888, 1.345870, -2.023352};
static const double kCabinetQ[3]       = {0.001521, 0.004340, 0.698055};
static const double kCabinetSoft[3][6] = {
    {-0.03210485,  0.02930774, -0.03684868, 0.08745830, 0.17507678, 0.19011669},
    { 0.00274723, -0.00111108,  0.00660622, 0.00538059, 0.00497473, 0.00197482},
    {-0.02980966,  0.02698614, -0.03276562, 0.07901201, 0.15947394, 0.17608467},
};
static const double kCabinetHard[3][6] = {
    {-0.07503038,  0.03336402, -0.05890525, 0.24020877, 0.52565904, 0.47848226},
    { 0.00088862, -0.00033098,  0.00222439, 0.00185638, 0.00180595, 0.00077184},
    {-0.07434968,  0.03589706, -0.05980793, 0.23377588, 0.50980170, 0.46442431},
};

// notes §10: moves a filter designed at 48 kHz to the running rate, keeping its response in hertz
static void at_this_rate(double * num, double * den, int order, double rate) {
    if (rate == 48000.0) {
        return;
    }
    double k           = rate / 48000.0;
    double alpha       = (1.0 - k) / (1.0 + k);
    double power[7][7] = {{0.0}};         // (z^-1 + alpha)^i (1 + alpha z^-1)^(order - i)
    double outN[7] = {0.0}, outD[7] = {0.0};

    for (int i = 0; i <= order; i++) {
        double p[7] = {1.0};

        for (int f = 0; f < i; f++) {
            for (int j = order; j >= 0; j--) {
                p[j] = (alpha * p[j]) + ((j > 0) ? p[j - 1] : 0.0);
            }
        }

        for (int f = i; f < order; f++) {
            for (int j = order; j >= 0; j--) {
                p[j] = p[j] + (alpha * ((j > 0) ? p[j - 1] : 0.0));
            }
        }

        memcpy(power[i], p, sizeof(p));
    }

    for (int i = 0; i <= order; i++) {
        for (int j = 0; j <= order; j++) {
            outN[j] += num[i] * power[i][j];
            outD[j] += den[i] * power[i][j];
        }
    }

    for (int j = 0; j <= order; j++) {
        num[j] = outN[j] / outD[0];
        den[j] = outD[j] / outD[0];
    }
}

static double in_tune(const double * c, double t) {
    double v = c[0];

    for (int i = 1; i < 6; i++) {
        v = (v * t) + c[i];
    }

    return v;
}

static void cabinet(tTiEffects * e, const uint8_t * b, float * left, float * right, uint32_t frames) {
    double t      = b[98] / 127.0;
    double d      = b[97] / 127.0;
    double warp   = exp((((kCabinetWarp[0] * t) + kCabinetWarp[1]) * t) + kCabinetWarp[2]);
    double q      = (((kCabinetQ[0] * t) + kCabinetQ[1]) * t) + kCabinetQ[2];
    double n      = 1.0 + (warp / q) + (warp * warp);
    double hi1    = 2.0 * ((warp * warp) - 1.0) / n;
    double hi2    = (1.0 - (warp / q) + (warp * warp)) / n;
    double lo1    = -2.0 * 0.9887003 * cos(2.0 * M_PI * 83.610 / 48000.0);
    double lo2    = 0.9887003 * 0.9887003;
    double a1     = hi1 + lo1;
    double a2     = hi2 + (hi1 * lo1) + lo2;
    double a3     = (hi2 * lo1) + (hi1 * lo2);
    double a4     = hi2 * lo2;
    double z[3];

    for (int i = 0; i < 3; i++) {
        z[i] = in_tune(kCabinetSoft[i], t) + (d * (in_tune(kCabinetHard[i], t) - in_tune(kCabinetSoft[i], t)));
    }

    double middle = z[1] - z[0] - z[2];   // notes §5: the terms nearly cancel, so the law holds their sum
    double num[5] = {z[0], middle, z[2] - z[0], -middle, -z[2]};
    double den[5] = {1.0, a1, a2, a3, a4};

    at_this_rate(num, den, 4, e->rate);

    for (int ch = 0; ch < 2; ch++) {
        float *  io = (ch == 0) ? left : right;
        double * w  = e->cabinet[ch];

        for (uint32_t f = 0; f < frames; f++) {
            double x = io[f];
            double y = (num[0] * x) + w[0];

            w[0]  = (num[1] * x) - (den[1] * y) + w[1];
            w[1]  = (num[2] * x) - (den[2] * y) + w[2];
            w[2]  = (num[3] * x) - (den[3] * y) + w[3];
            w[3]  = (num[4] * x) - (den[4] * y);
            io[f] = (float)y;
        }
    }
}

// notes §5: Stereo Widener's side filter, three pole pairs and a numerator, each a law in Tune
static const double kWidenerPoles[3][6] = {
    {-0.0799941, 1.1338536, 4.5229289, -0.0093234, -0.0021558, 0.9864422},
    { 0.5881325, 0.1163568, 6.2031198, -0.0051098, -0.0595339, 0.9591141},
    { 0.0610413, 0.8428732, 7.2279988, -0.0256161, -0.1043953, 0.8983116},
};
static const double kWidenerZeros[7][6] = {
    {-0.0026156,  0.0146206, -0.0159690, -0.0239697, -0.1025519, -0.1274251},
    { 0.0190306, -0.0771527,  0.0933414,  0.0776216,  0.3937632,  0.5029760},
    {-0.0701172,  0.2020045, -0.2352061, -0.0232256, -0.4621030, -0.6186805},
    { 0.1482932, -0.3375678,  0.3402351, -0.1996537, -0.0277203, -0.0036338},
    {-0.1769985,  0.3538109, -0.3055811,  0.3207078,  0.4554851,  0.6044364},
    { 0.1108148, -0.2057345,  0.1610401, -0.1939180, -0.3328536, -0.4735733},
    {-0.0284047,  0.0500137, -0.0378571,  0.0424366,  0.0759803,  0.1159004},
};

static void widener(tTiEffects * e, const uint8_t * b, float * left, float * right, uint32_t frames) {
    double   t      = b[98] / 127.0;
    double   amount = b[97] / 127.0;
    double   den[7] = {1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
    double   num[7];
    double * w      = e->widener;

    for (int k = 0; k < 3; k++) {
        const double * c  = kWidenerPoles[k];
        double         hz = exp((((c[0] * t) + c[1]) * t) + c[2]);
        double         r  = (((c[3] * t) + c[4]) * t) + c[5];
        double         p1 = -2.0 * r * cos(2.0 * M_PI * hz / 48000.0);
        double         p2 = r * r;

        for (int i = 6; i >= 2; i--) {
            den[i] += (p1 * den[i - 1]) + (p2 * den[i - 2]);
        }

        den[1] += p1;
    }

    for (int i = 0; i < 7; i++) {
        num[i] = amount * in_tune(kWidenerZeros[i], t);
    }

    at_this_rate(num, den, 6, e->rate);

    for (uint32_t f = 0; f < frames; f++) {
        double x    = 0.5 * ((double)left[f] + (double)right[f]);
        double side = (num[0] * x) + w[0];

        for (int i = 0; i < 5; i++) {
            w[i] = (num[i + 1] * x) - (den[i + 1] * side) + w[i + 1];
        }

        w[5]      = (num[6] * x) - (den[6] * side);
        left[f]  += (float)side;
        right[f] -= (float)side;
    }
}

static void character(tTiEffects * e, const uint8_t * b, const uint8_t * c, float * left, float * right, uint32_t frames) {
    uint8_t type = c[26];

    if (type == 8) {
        cabinet(e, b, left, right, frames);
        return;
    }

    if (type == 7) {
        if (b[97] != 0) {
            widener(e, b, left, right, frames);
        }
        return;
    }

    if ((type == 0) || (type == 3)) {
        return;
    }

    if (e->characterType != type) {
        e->characterType = type;

        for (int i = 0; i < 8; i++) {
            peak(&e->character[i], 32.7 * exp2(i), kCharacter[type][i], 1.41, e->rate);
        }
    }

    for (int i = 0; i < 8; i++) {
        run(&e->character[i], left, frames, 0);
        run(&e->character[i], right, frames, 1);
    }
}

// notes §6: two allpass chains whose outputs stay 90 degrees apart across the audio band
static void hilbert(tTiEffects * e, int ch, double x, double * i, double * q) {
    static const double kA[4] = {0.6923878, 0.9360654322959, 0.9882295226860, 0.9987488452737};
    static const double kB[4] = {0.4021921162426, 0.8561710882420, 0.9722909545651, 0.9952884791278};
    double              u     = x;
    double              v     = x;

    for (int k = 0; k < 4; k++) {
        double * ua = e->hilbert[ch][k];
        double * va = e->hilbert[ch][k + 4];
        double   yu = (kA[k] * kA[k] * (u + ua[3])) - ua[1];
        double   yv = (kB[k] * kB[k] * (v + va[3])) - va[1];

        ua[1] = ua[0];
        ua[0] = u;
        ua[3] = ua[2];
        ua[2] = yu;
        va[1] = va[0];
        va[0] = v;
        va[3] = va[2];
        va[2] = yv;
        u     = yu;
        v     = yv;
    }

    *q                  = v;
    *i                  = e->hilbertDelay[ch];
    e->hilbertDelay[ch] = u;
}

// notes §6: Ring Modulator and Frequency Shifter
static void bank_shift(tTiEffects * e, const uint8_t * c, float * left, float * right, uint32_t frames) {
    double d    = (double)c[21] - 64.0;
    double hz   = (d == 0.0) ? 0.0 : (12000.0 * pow(2.0, -3.0 * (64.0 - fabs(d)) / 16.0));
    double step = ((d < 0.0) ? -hz : hz) / e->rate;
    double m    = c[20] / 127.0;
    double turn = 0.5 * M_PI * ((c[19] == 1) ? pow(m, 1.39) : m);   // notes §6
    double wet  = sin(turn);
    double dry  = cos(turn);
    double side[2];

    for (int ch = 0; ch < 2; ch++) {
        side[ch] = (c[19] == 1) ? 0.0 : (((double)c[23 + ch] - 64.0) / 64.0);
    }

    for (uint32_t f = 0; f < frames; f++) {
        double co = cos(2.0 * M_PI * e->bankPhase);
        double si = sin(2.0 * M_PI * e->bankPhase);

        for (int ch = 0; ch < 2; ch++) {
            float * io   = (ch == 0) ? left : right;
            double  x = io[f], i, q;

            hilbert(e, ch, x, &i, &q);
            double  gain = BANK_LEVEL * sqrt(2.0 / (1.0 + (side[ch] * side[ch])));

            io[f] = (float)((dry * x) + (wet * gain * ((i * co) + (side[ch] * q * si))));
        }

        e->bankPhase += step;
        e->bankPhase -= floor(e->bankPhase);
    }
}

// notes §6: a state-variable section; low, band scaled to unity at its peak, and high
typedef struct {
    double low, band, high;
} tBankTaps;

static tBankTaps bank_section(double * st, double g, double r, double x) {
    double    d  = 1.0 / (1.0 + (2.0 * r * g) + (g * g));
    double    hp = (x - (((2.0 * r) + g) * st[0]) - st[1]) * d;
    double    v1 = g * hp;
    double    bp = v1 + st[0];
    double    v2 = g * bp;
    double    lp = v2 + st[1];
    tBankTaps t  = {lp, 2.0 * r * bp, hp};

    st[0] = bp + v1;
    st[1] = lp + v2;
    return t;
}

static double bank_morph(tBankTaps t, int type, double m, double boost) {
    if (type == 5) {
        double high = fmin(2.0 * m, 1.0);
        double low  = fmin(2.0 - (2.0 * m), 1.0);

        return (low * t.low) + (high * t.high) + (boost * t.band * (0.5 + fmin(m, 1.0 - m)));
    }
    double turn = M_PI * m;   // equal power between neighbours

    if (m < 0.5) {
        return (cos(turn) * t.low) + (sin(turn) * boost * t.band);
    }
    return (sin(turn) * boost * t.band) - (cos(turn) * t.high);
}

// notes §6: VariSlope, from a section towards one more pole, or for band pass towards three sections
static double bank_slope(tTiEffects * e, int ch, int type, double m, double g, double r, double boost, double x) {
    tBankTaps a     = bank_section(e->bankState[ch][0], g, r, x);

    if (type == 11) {
        tBankTaps b = bank_section(e->bankState[ch][1], g, r, boost * a.band);
        tBankTaps c = bank_section(e->bankState[ch][2], g, r, boost * b.band);
        double    n = 2.0 * m;

        return (n < 1.0) ? ((boost * a.band) + (n * ((boost * b.band) - (boost * a.band))))
               : ((boost * b.band) + ((n - 1.0) * ((boost * c.band) - (boost * b.band))));
    }
    double    two   = (type == 9) ? a.low : a.high;
    tBankTaps b     = bank_section(e->bankState[ch][1], g, BANK_FLAT, two);
    double    three = bank_morph(b, 5, (type == 9) ? 0.0 : 1.0, 1.0);

    return two + (0.7 * m * m * (three - two));
}

// notes §6: each section's Q at no resonance, the 6 pole a Butterworth
static const double kBankQ[2][3] = {{0.578, 0.81, 0.0}, {0.518, 0.707, 1.93}};

// notes §6: 1, 2, 4 and 6 pole XFade, and the three VariSlopes
static void bank_poles(tTiEffects * e, const uint8_t * c, float * left, float * right, uint32_t frames) {
    int    type     = c[19];
    double hz       = 832.0 * pow(2.0, ((double)c[21] - 64.0) / 12.2);
    double g        = tan(M_PI * fmin(hz, 0.45 * e->rate) / e->rate);
    double res      = c[25];
    double q        = pow(2.0, res / 16.0);
    double r        = 0.5 / (0.487 * q);
    double gain     = fmax(1.0 - (0.0054 * res), 0.36);
    double boost    = gain * q;
    int    sections = (type <= 6) ? 1 : ((type == 7) ? 2 : 3);
    double turn     = 0.5 * M_PI * c[20] / 127.0;
    double wet      = sin(turn);
    double dry      = cos(turn);

    for (int ch = 0; ch < 2; ch++) {
        float * io = (ch == 0) ? left : right;
        double  m  = c[23 + ch] / 127.0;

        for (uint32_t f = 0; f < frames; f++) {
            double x = io[f];
            double y = gain * x;

            if (type >= 9) {
                y = bank_slope(e, ch, type, m, g, r, boost, y);
            } else {
                for (int k = 0; k < sections; k++) {
                    double qk = (type == 7) ? kBankQ[0][k] : ((type == 8) ? kBankQ[1][k] : 0.487);

                    double rk = (type <= 6) ? r : ((k == 0) ? (0.5 / fmax(qk, 0.487 * q)) : (0.5 / qk));   // notes §6

                    y = bank_morph(bank_section(e->bankState[ch][k], g, rk, y), type, m, (k == 0) ? boost : 1.0);
                }
            }
            io[f] = (float)((dry * x) + (wet * y));
        }
    }
}

// notes §6: Comb, tuned as a note, its two sides spread by Stereo Phase
static void bank_comb(tTiEffects * e, const uint8_t * c, float * left, float * right, uint32_t frames) {
    double hz       = 659.255 * pow(2.0, ((double)c[21] - 64.0) / 12.0);
    double delay    = e->rate / hz;
    double spread   = 0.0274 * ((double)c[22] - 64.0) / 63.0;
    double feedback = 0.0063 * c[25];
    double m        = c[20] / 127.0;

    for (int ch = 0; ch < 2; ch++) {
        float * io = (ch == 0) ? left : right;
        double  at = fmin(fmax(delay * ((ch == 0) ? (1.0 - spread) : (1.0 + spread)), 1.0), TI_BANK_LINE - 2.0);

        for (uint32_t f = 0; f < frames; f++) {
            double   from = (double)e->bankAt[ch] + TI_BANK_LINE - at;
            uint32_t i    = (uint32_t)from;
            double   frac = from - i;
            float *  line = e->bankLine[ch];
            double   late = line[i % TI_BANK_LINE] + (frac * (line[(i + 1u) % TI_BANK_LINE] - line[i % TI_BANK_LINE]));
            double   x    = io[f];

            line[e->bankAt[ch]] = (float)(x + (feedback * late));
            e->bankAt[ch]       = (e->bankAt[ch] + 1u) % TI_BANK_LINE;
            io[f]               = (float)(((1.0 - (0.293 * m)) * x) + (0.7 * m * late));
        }
    }
}

// notes §6: Vowel, two formants side by side, their places measured every 16 steps
static void bank_vowel(tTiEffects * e, const uint8_t * c, float * left, float * right, uint32_t frames) {
    static const double kFormant[9][2] = {
        {296.0, 633.0}, {585.0, 884.0}, {988.0, 1385.0}, {585.0, 2013.0}, {316.0, 3167.0}, {316.0, 1927.0}, {494.0, 1483.0}, {542.0, 1046.0}, {311.0, 646.0}
    };
    double              p              = fmin(c[21] / 16.0, 7.9375);
    int                 i              = (int)p;
    double              frac           = p - i;
    double              q              = 0.5 * pow(2.0, c[25] / 24.2);
    double              r              = 0.5 / q;
    double              g1             = tan(M_PI * kFormant[i][0] * pow(kFormant[i + 1][0] / kFormant[i][0], frac) / e->rate);
    double              g2             = tan(M_PI * kFormant[i][1] * pow(kFormant[i + 1][1] / kFormant[i][1], frac) / e->rate);
    double              level          = (0.77 - (0.0023 * c[25])) / (2.0 * r);
    double              turn           = 0.5 * M_PI * c[20] / 127.0;
    double              wet            = sin(turn);
    double              dry            = cos(turn);

    for (int ch = 0; ch < 2; ch++) {
        float * io = (ch == 0) ? left : right;

        for (uint32_t f = 0; f < frames; f++) {
            double x = io[f];
            double a = bank_section(e->bankState[ch][0], g1, r, x).band;
            double b = bank_section(e->bankState[ch][1], g2, r, x).band;

            io[f] = (float)((dry * x) + (wet * level * (a + b)));
        }
    }
}

static void bank(tTiEffects * e, const uint8_t * c, float * left, float * right, uint32_t frames) {
    if ((c[20] == 0) || (c[19] == 0)) {
        return;
    }

    if (c[19] <= 2) {
        bank_shift(e, c, left, right, frames);
    } else if (c[19] >= 5) {
        bank_poles(e, c, left, right, frames);
    } else if (c[19] == 4) {
        bank_comb(e, c, left, right, frames);
    } else {
        bank_vowel(e, c, left, right, frames);
    }
}

// notes §8: wet by Send, every 16 steps
static double delay_wet(double send) {
    static const double kWet[7] = {0.0, 0.059, 0.153, 0.271, 0.437, 0.71, 1.05};
    double              p       = fmin(send / 16.0, 5.999);
    int                 i       = (int)p;

    return kWet[i] + ((p - i) * (kWet[i + 1] - kWet[i]));
}

// notes §8: the two sides' delays, in samples
#define DELAY_TEMPO_LAW    (0.9816)

// notes §11: a whole note, in samples at the current tempo
static double delay_whole_note(const tTiEffects * e) {
    return DELAY_TEMPO_LAW * 240.0 * e->rate / ((e->bpm > 1.0) ? e->bpm : 120.0);
}

// notes §11: a clock division replaces the Time dial, folded by octaves into the line
static double delay_clocked(const tTiEffects * e, uint8_t clock) {
    static const double kDivision[17] = {
        0.0,        1.0 / 64.0, 1.0 / 32.0, 1.0 / 16.0, 1.0 / 8.0, 1.0 / 4.0, 1.0 / 2.0, 3.0 / 64.0, 3.0 / 32.0,
        3.0 / 16.0,  3.0 / 8.0, 1.0 / 24.0, 1.0 / 12.0, 1.0 / 6.0, 1.0 / 3.0, 2.0 / 3.0, 3.0 / 4.0
    };
    double              longest       = 32768.0 * e->rate / 48000.0;
    double              at            = kDivision[(clock <= 16) ? clock : 0] * delay_whole_note(e);

    while (at > longest) {
        at *= 0.5;
    }
    return at;
}

static void delay_times(const tTiEffects * e, const uint8_t * a, const uint8_t * b, double * left, double * right) {
    static const double kRatio[4]      = {0.5, 0.75, 0.25, 0.875};
    static const int    kPattern[6][2] = {{1, 1}, {2, 1}, {3, 1}, {4, 1}, {5, 1}, {2, 3}};
    double              base           = (b[20] != 0) ? delay_clocked(e, b[20]) : (fmax((double)a[114], 1.0) * 256.0 * e->rate / 48000.0);
    uint8_t             mode           = a[112];

    *left  = base;
    *right = base;

    if ((mode >= 2) && (mode <= 5)) {
        *right = base * kRatio[mode - 2];
    } else if (mode >= 6) {
        const int * p         = kPattern[(mode <= 11) ? (mode - 6) : 0];
        double      sixteenth = delay_whole_note(e) / 16.0;

        *left  = p[0] * sixteenth;
        *right = p[1] * sixteenth;
    }
}

static void delay(tTiEffects * e, const uint8_t * a, const uint8_t * b, float * left, float * right, uint32_t frames) {
    if (!e->delayAllowed || (a[112] == 0) || (a[113] == 0)) {
        return;
    }
    double   send     = a[113];
    double   wet      = delay_wet(send);
    double   dry      = (send <= 96.0) ? 1.0 : ((127.0 - send) / 31.0);
    double   feedback = 0.00772 * a[115];
    double   at[2];

    delay_times(e, a, b, &at[0], &at[1]);

    uint32_t tapL     = (uint32_t)fmin(at[0], TI_DELAY_LINE - 2.0);
    uint32_t tapR     = (uint32_t)fmin(at[1], TI_DELAY_LINE - 2.0);
    bool     apart    = (a[112] == 1);
    float *  lineL    = e->delayLine[0];
    float *  lineR    = e->delayLine[apart ? 1 : 0];

    for (uint32_t f = 0; f < frames; f++) {
        double xl    = left[f];
        double xr    = right[f];
        double lateL = lineL[(e->delayAt + TI_DELAY_LINE - tapL) % TI_DELAY_LINE];
        double lateR = lineR[(e->delayAt + TI_DELAY_LINE - tapR) % TI_DELAY_LINE];

        if (apart) {
            lineL[e->delayAt] = (float)(xl + (feedback * lateL));
            lineR[e->delayAt] = (float)(xr + (feedback * lateR));
        } else {                                      // notes §8: one line, the left tap feeding it
            lineL[e->delayAt] = (float)((0.5 * (xl + xr)) + (feedback * lateL));
        }
        left[f]    = (float)((dry * xl) + (wet * lateL));
        right[f]   = (float)((dry * xr) + (wet * lateR));
        e->delayAt = (e->delayAt + 1u) % TI_DELAY_LINE;
    }
}

// notes §9: each room's stepped read offsets; the last entries place the four read points and the input
static const int32_t kRoom[4][44] = {
    { 514,  522,  750,  547,  636,  392,  541,  512,  552,  653, 2742,  -3740, -1971,  -663,  -806, -179, -2347, -1419,  -3725,  85,  51, 134,  85, 230, 151, 2312, 2241, 1018,  689, 1036,  690, 1559, 1716, -3222,  -567, 0, 25, -1750, 2608, 338,  723, 2915, 11796,   781},
    { 781,  853, 1142,  833,  967,  595,  823,  779,  839,  994, 4170,  -5689, -2997, -1008, -1226, -273, -3571, -2159,  -5667, 130,  78, 204, 130, 351, 229, 3516, 3409, 1547, 1049, 1576, 1050, 2373, 2552, -4900,  -864, 0, 38, -2660, 3909, 514, 1100, 4435,  9423,  1048},
    {1048, 1180, 1530, 1117, 1297,  799, 1103, 1044, 1126, 1333, 5593,  -7628, -4020, -1352, -1644, -366, -4787, -2896,  -7599, 173, 103, 274, 173, 471, 308, 4715, 4572, 2075, 1406, 2114, 1407, 3182, 3386, -6571, -1157, 0, 52, -3567, 5205, 690, 1475, 5947,  7061,  1448},
    {1448, 1672, 2113, 1543, 1791, 1103, 1523, 1441, 1554, 1840, 7722, -10533, -5551, -1867, -2269, -505, -6610, -3998, -10493, 239, 143, 378, 239, 650, 425, 6511, 6313, 2866, 1942, 2918, 1943, 4393, 4633, -9073, -1597, 0, 73, -4926, 7145, 953, 2036, 8211,  3525, 65536},
};

static int verb_next(tTiEffects * e) {
    int v = e->verbTable[e->verbIndex];

    e->verbIndex = (e->verbIndex + 1) % 38;
    return v;
}

static int32_t verb_step(int32_t r, int n) {
    r += n;

    while (r < 0) {
        r += TI_REVERB_LINE;
    }

    while (r >= TI_REVERB_LINE) {
        r -= TI_REVERB_LINE;
    }
    return r;
}

// notes §9: one sample of the tank at half rate, statement for statement
static void verb_tank(tTiEffects * e, const double * c, double in, double * outL, double * outR) {
    float * X = e->verbX;
    float * Y = e->verbX;    // notes §9: one ring, both sides of the tank
    int32_t r2 = e->verbR[0], r3 = e->verbR[1], r4 = e->verbR[2], r5 = e->verbR[3];
    int     n2 = e->verbN[0], n3 = e->verbN[1], n4 = e->verbN[2], n5 = e->verbN[3];
    double  a, b, x0, x1, y0, y1, t;

    Y[e->verbIn] = (float)in;
    e->verbIn    = verb_step(e->verbIn, -1);

    x0           = X[r3];
    y1           = c[0];
    a            = x0 * y1;
    y0           = Y[r4];
    b            = y1 * y0;
    x1           = c[1];
    t            = a;
    a            = x1 * x0;
    X[r3]        = (float)t;
    r3           = verb_step(r3, n3);
    t            = b;
    b            = x1 * y0;
    Y[r4]        = (float)t;
    r4           = verb_step(r4, n4);
    n3           = verb_next(e);
    n4           = verb_next(e);
    x0           = X[r2];
    r2           = verb_step(r2, n2);
    y0           = Y[r5];
    r5           = verb_step(r5, n5);
    a           -= x1 * y0;
    n5           = verb_next(e);
    b           -= x1 * x0;
    n2           = verb_next(e);
    x0           = X[r3];
    r3           = verb_step(r3, n3);
    y0           = Y[r4];
    r4           = verb_step(r4, n4);
    a           += x1 * x0;
    n3           = verb_next(e);
    b           += x1 * y0;
    n4           = verb_next(e);
    x0           = X[r2];
    r2           = verb_step(r2, n2);
    y0           = Y[r5];
    r5           = verb_step(r5, n5);
    a           += x1 * y0;
    n5           = verb_next(e);
    b           += x1 * x0;
    n2           = verb_next(e);
    x0           = X[r3];
    r3           = verb_step(r3, n3);
    y0           = Y[r4];
    r4           = verb_step(r4, n4);
    a           -= x1 * x0;
    n3           = verb_next(e);
    b           -= x1 * y0;
    n4           = verb_next(e);
    x0           = X[r2];
    r2           = verb_step(r2, n2);
    y0           = Y[r5];
    r5           = verb_step(r5, n5);
    a           -= x1 * y0;
    n5           = verb_next(e);
    b           -= x1 * x0;
    n2           = verb_next(e);
    x0           = X[r3];
    r3           = verb_step(r3, n3);
    y0           = Y[r4];
    r4           = verb_step(r4, n4);
    a           += x1 * x0;
    n3           = verb_next(e);
    b           += x1 * y0;
    n4           = verb_next(e);
    *outL        = a;
    x1           = X[r2];
    y0           = Y[r5];
    r5           = verb_step(r5, n5);
    a            = x1;
    n5           = verb_next(e);
    a           += y1 * y0;
    x0           = X[r3];
    *outR        = b;
    b            = x1;
    b           += x0 * y1;
    X[r2]        = (float)a;
    r2           = verb_step(r2, n2);
    y0           = c[2];
    n2           = verb_next(e);
    X[r3]        = (float)b;
    r3           = verb_step(r3, n3);
    y1           = c[3];
    x1           = Y[r4];
    r4           = verb_step(r4, 1);
    a            = x1 * y0;
    x1           = Y[r4];
    r4           = verb_step(r4, -1);
    a           += y1 * x1;
    n3           = verb_next(e);
    x1           = Y[r5];
    r5           = verb_step(r5, 1);
    b            = X[r2];
    r2           = verb_step(r2, n2);
    Y[r4]        = (float)a;
    r4           = verb_step(r4, n4);
    a            = x1 * y0;
    x1           = Y[r5];
    r5           = verb_step(r5, -1);
    a           += y1 * x1;
    n4           = verb_next(e);
    x1           = X[r3];
    y0           = c[4];
    b           -= x1 * y0;
    Y[r5]        = (float)a;
    r5           = verb_step(r5, n5);
    n5           = verb_next(e);
    t            = b;
    b            = x1;
    x1           = t;
    a            = Y[r4];
    r4           = verb_step(r4, n4);
    b           += x1 * y0;
    n2           = verb_next(e);
    a           += x1 * y0;
    X[r3]        = (float)b;
    r3           = verb_step(r3, n3);
    n3           = verb_next(e);
    y1           = a;
    a            = x1;
    b            = X[r2];
    r2           = verb_step(r2, n2);
    a           -= y1 * y0;
    n4           = verb_next(e);
    b           -= y1 * y0;
    Y[r5]        = (float)a;
    r5           = verb_step(r5, n5);
    n5           = verb_next(e);
    t            = b;
    b            = y1;
    x1           = t;
    a            = Y[r4];
    b           += x1 * y0;
    n2           = verb_next(e);
    y0           = c[5];
    a           += x1 * y0;
    X[r3]        = (float)b;
    r3           = verb_step(r3, n3);
    n3           = verb_next(e);
    t            = a;
    a            = x1;
    x1           = t;
    Y[r4]        = (float)t;
    r4           = verb_step(r4, n4);
    a           -= x1 * y0;
    n4           = verb_next(e);
    b            = X[r2];
    x1           = X[r3];
    Y[r5]        = (float)a;
    r5           = verb_step(r5, n5);
    b           -= x1 * y0;
    n5           = verb_next(e);
    a            = Y[r4];
    t            = b;
    X[r2]        = (float)t;
    r2           = verb_step(r2, n2);
    y1           = t;
    b            = x1;
    b           += y1 * y0;
    n2           = verb_next(e);
    x1           = Y[r5];
    a           -= x1 * y0;
    X[r3]        = (float)b;
    r3           = verb_step(r3, n3);
    n3           = verb_next(e);
    t            = a;
    a            = x1;
    x1           = t;
    Y[r4]        = (float)t;
    r4           = verb_step(r4, n4);
    a           += x1 * y0;
    n4           = verb_next(e);
    b            = X[r2];
    y0           = c[6];
    x1           = X[r3];
    Y[r5]        = (float)a;
    r5           = verb_step(r5, n5);
    b           += x1 * y0;
    n5           = verb_next(e);
    (void)verb_next(e);                     // the slot that holds the modulation's phase
    t            = b;
    X[r2]        = (float)t;
    r2           = verb_step(r2, n2);
    y1           = t;
    b            = x1;
    b           -= y1 * y0;
    x1           = Y[r5];
    a            = Y[r4];
    a           += x1 * y0;
    X[r3]        = (float)b;
    r3           = verb_step(r3, n3);
    t            = a;
    a            = x1;
    x1           = t;
    Y[r4]        = (float)t;
    r4           = verb_step(r4, n4);
    a           -= x1 * y0;
    Y[r5]        = (float)a;
    r5           = verb_step(r5, n5);
    a            = Y[r4];
    r4           = verb_step(r4, 1);
    y0           = Y[r4];
    b            = Y[r5];
    r5           = verb_step(r5, 1);
    y1           = Y[r5];
    n4           = verb_next(e);
    n5           = verb_next(e);
    n2           = verb_next(e);
    n3           = verb_next(e);
    Y[r4]        = (float)y0;
    r4           = verb_step(r4, n4);
    n4           = verb_next(e);
    Y[r5]        = (float)y1;
    r5           = verb_step(r5, n5);
    n5           = verb_next(e);
    (void)a;
    (void)b;
    e->verbR[0]  = r2;
    e->verbR[1]  = r3;
    e->verbR[2]  = r4;
    e->verbR[3]  = r5;
    e->verbN[0]  = n2;
    e->verbN[1]  = n3;
    e->verbN[2]  = n4;
    e->verbN[3]  = n5;
}

static void verb_room(tTiEffects * e, int room) {
    const int32_t * t = kRoom[room];

    memset(e->verbX, 0, sizeof(e->verbX));

    for (int i = 0; i < 38; i++) {
        e->verbTable[i] = t[i];
    }

    e->verbRoom  = (uint8_t)(room + 1);
    e->verbIndex = 4;
    e->verbN[0]  = t[0];
    e->verbN[1]  = t[1];
    e->verbN[2]  = t[2];
    e->verbN[3]  = t[3];
    e->verbR[1]  = t[38] % TI_REVERB_LINE;
    e->verbR[2]  = t[39] % TI_REVERB_LINE;
    e->verbR[3]  = t[40] % TI_REVERB_LINE;
    e->verbR[0]  = t[41] % TI_REVERB_LINE;
    e->verbIn    = 0;    // notes §9: the read points sit relative to the input
}

// notes §9: from Time and Damping, breakpoints; the rest of the set follows them
static void verb_coefficients(const uint8_t * c, double * k) {
    static const double kAt[14]    = {0, 8, 16, 24, 32, 48, 64, 80, 96, 108, 116, 122, 126, 127};
    static const double kTime[14]  = {0, 0.03896, 0.15582, 0.25551, 0.37972, 0.57467, 0.66617, 0.75464, 0.84105, 0.90392, 0.94567, 0.97652, 0.99737, 1.0};
    static const double kDampAt[9] = {0, 16, 32, 48, 64, 80, 96, 112, 127};
    static const double kDamp[9]   = {0, 0.06239, 0.1621, 0.21955, 0.35975, 0.5239, 0.77369, 0.87122, 0.96265};
    static const double kRoomy[4]  = {0.39, 0.58, 0.76, 1.0};
    double              time = c[4], damp = c[5], g = 1.0, d = kDamp[8];

    for (int i = 1; i < 14; i++) {
        if (time <= kAt[i]) {
            g = kTime[i - 1] + (((time - kAt[i - 1]) / (kAt[i] - kAt[i - 1])) * (kTime[i] - kTime[i - 1]));
            break;
        }
    }

    for (int i = 1; i < 9; i++) {
        if (damp <= kDampAt[i]) {
            d = kDamp[i - 1] + (((damp - kDampAt[i - 1]) / (kDampAt[i] - kDampAt[i - 1])) * (kDamp[i] - kDamp[i - 1]));
            break;
        }
    }

    g    = 1.0 - ((1.0 - g) * kRoomy[(c[3] > 3) ? 3 : c[3]]);
    k[0] = g;
    k[1] = 0.7;
    k[2] = g * (1.0 - d);
    k[3] = d;
    k[4] = 0.66;
    k[5] = fmin(fmax(0.4 + (0.75 * g), 0.45), 0.62);
    k[6] = fmin(fmax(0.25 + (0.55 * g), 0.3), 0.48);
}

// notes §9: half rate in the tank, Color on the way in, Predelay ahead of it
static void reverb(tTiEffects * e, const uint8_t * c, float * left, float * right, uint32_t frames) {
    if (!e->reverbAllowed || (c[1] == 0) || (c[2] == 0)) {
        return;
    }
    int      room   = (c[3] > 3) ? 3 : c[3];
    double   send   = c[2];
    double   wet    = VERB_LEVEL * delay_wet(send);
    double   dry    = (send <= 96.0) ? 1.0 : ((127.0 - send) / 31.0);
    uint32_t stride = (e->rate > 50000.0) ? 4u : 2u;   // notes §10
    uint32_t pre    = (uint32_t)fmin(c[7] * 0.001875 * e->rate / stride, TI_REVERB_PRE - 2.0);
    double   k[7];

    if (e->verbRoom != room + 1) {
        verb_room(e, room);
    }
    verb_coefficients(c, k);
    double   color  = ((double)c[6] - 64.0) / 63.0;
    double   lp     = (color >= 0.0) ? 1.0 : (1.0 - exp(-2.0 * M_PI * 16000.0 * pow(2.0, 7.3 * color) / e->rate));
    double   hp     = (color <= 0.0) ? 0.0 : (1.0 - exp(-2.0 * M_PI * 4000.0 * pow(2.0, 8.0 * (color - 1.0)) / e->rate));

    for (uint32_t f = 0; f < frames; f++) {
        double x = 0.5 * ((double)left[f] + (double)right[f]);

        // the one-pole tilt Color sets
        e->verbTone[0] += lp * (x - e->verbTone[0]);
        x               = e->verbTone[0];
        e->verbTone[2] += hp * (x - e->verbTone[2]);
        x              -= e->verbTone[2];

        e->verbSum     += x;
        e->verbHalf++;

        if (e->verbHalf >= stride) {
            double in = e->verbSum / stride;

            e->verbHalf              = 0u;
            e->verbSum               = 0.0;
            e->verbPre[e->verbPreAt] = (float)in;
            in                       = e->verbPre[(e->verbPreAt + TI_REVERB_PRE - pre) % TI_REVERB_PRE];
            e->verbPreAt             = (e->verbPreAt + 1u) % TI_REVERB_PRE;
            e->verbMid[0]            = e->verbLast[0];
            e->verbMid[1]            = e->verbLast[1];
            verb_tank(e, k, in, &e->verbLast[0], &e->verbLast[1]);
        }
        double along = (double)(e->verbHalf + 1u) / stride;
        double l     = e->verbMid[0] + (along * (e->verbLast[0] - e->verbMid[0]));
        double r     = e->verbMid[1] + (along * (e->verbLast[1] - e->verbMid[1]));

        left[f]         = (float)((dry * left[f]) + (wet * l));
        right[f]        = (float)((dry * right[f]) + (wet * r));
    }
}

// notes §7: an effect switched off keeps nothing, so switching it on again plays no stale sound
static void forget_when_off(tTiEffects * e, const uint8_t * a, const uint8_t * b, const uint8_t * c) {
    bool distort_on = (b[100] != 0);
    bool bank_on    = (c[19] != 0) && (c[20] != 0);
    bool phaser_on  = (b[85] != 0);
    bool chorus_on  = chorus_active(a);
    bool reverb_on  = e->reverbAllowed && (c[1] != 0) && (c[2] != 0);
    bool delay_on   = e->delayAllowed && (a[112] != 0) && (a[113] != 0);

    if (!distort_on && e->wasOn[0]) {
        memset(e->distState, 0, sizeof(e->distState));
        memset(e->distLast, 0, sizeof(e->distLast));
        memset(e->distHigh, 0, sizeof(e->distHigh));
        memset(e->distHeld, 0, sizeof(e->distHeld));
        memset(e->driveState, 0, sizeof(e->driveState));
        memset(e->treble, 0, sizeof(e->treble));
        memset(e->highCut, 0, sizeof(e->highCut));
        memset(e->distLine, 0, sizeof(e->distLine));
    }

    if (!bank_on && e->wasOn[1]) {
        memset(e->hilbert, 0, sizeof(e->hilbert));
        memset(e->hilbertDelay, 0, sizeof(e->hilbertDelay));
        memset(e->bankState, 0, sizeof(e->bankState));
        memset(e->bankLine, 0, sizeof(e->bankLine));
    }

    if (!phaser_on && e->wasOn[2]) {
        memset(e->apX, 0, sizeof(e->apX));
        memset(e->apY, 0, sizeof(e->apY));
        memset(e->phaserLast, 0, sizeof(e->phaserLast));
    }

    if (!chorus_on && e->wasOn[3]) {
        memset(e->chorusLine, 0, sizeof(e->chorusLine));
    }

    if (!delay_on && e->wasOn[4]) {
        memset(e->delayLine, 0, sizeof(e->delayLine));
    }

    if (!reverb_on && e->wasOn[5]) {
        memset(e->verbX, 0, sizeof(e->verbX));
        memset(e->verbPre, 0, sizeof(e->verbPre));
        memset(e->verbTone, 0, sizeof(e->verbTone));
        memset(e->verbLast, 0, sizeof(e->verbLast));
        memset(e->verbMid, 0, sizeof(e->verbMid));
        e->verbRoom = 0u;
    }
    e->wasOn[5] = reverb_on;
    e->wasOn[4] = delay_on;
    e->wasOn[0] = distort_on;
    e->wasOn[1] = bank_on;
    e->wasOn[2] = phaser_on;
    e->wasOn[3] = chorus_on;
}

void ti_effects_render(tTiEffects * e, const uint8_t * a, const uint8_t * b, const uint8_t * c, double volume, float * left, float * right, uint32_t frames) {
    forget_when_off(e, a, b, c);
    bank(e, c, left, right, frames);
    distort(e, 0, a[0], b, c, left, frames);
    distort(e, 1, a[0], b, c, right, frames);

    // notes §2: Patch Volume comes after distortion
    for (uint32_t f = 0; f < frames; f++) {
        left[f]  *= (float)volume;
        right[f] *= (float)volume;
    }

    character(e, b, c, left, right, frames);
    phaser(e, b, left, right, frames);
    chorus(e, a, left, right, frames);
    equalise(e, b, left, right, frames);
    delay(e, a, b, left, right, frames);
    reverb(e, c, left, right, frames);
}
