/*
 * TiAlike - the oscillators.
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
// Notes: Docs/code-notes/tiOsc.md - "// notes §k" refers there.

#include <math.h>
#include <pthread.h>
#include <stdlib.h>

#include "tiOsc.h"
#include "tiWaves.h"

#define SAW_LEVEL      (2.0 / 3.0)   // notes §1: against a sine's 1
#define PULSE_LEVEL    (0.5)
#define HYPER_LEVEL    (1.066)       // notes §4: a lone saw of the stack, against the classic saw

static float *        gMips[TI_WAVE_COUNT];   // TI_WAVE_MIPS tables of TI_WAVE_SIZE, each with half the harmonics
static pthread_once_t gOnce = PTHREAD_ONCE_INIT;

static void build_wave(int wave, float * table) {
    int harmonics = TI_WAVE_SIZE / 2;

    for (int m = 0; m < TI_WAVE_MIPS; m++, harmonics /= 2) {
        float * t = table + (m * TI_WAVE_SIZE);

        for (int i = 0; i < TI_WAVE_SIZE; i++) {
            double p   = (double)i / TI_WAVE_SIZE;
            double sum = 0.0;

            if (wave == 0) {
                sum = sin(2.0 * M_PI * p);
            } else if (wave == 1) {
                for (int k = 1; (k <= harmonics) && (k < 256); k += 2) {
                    sum += (((k % 4) == 1) ? 1.0 : -1.0) * sin(2.0 * M_PI * k * p) * (8.0 / (M_PI * M_PI * k * k));
                }
            } else {
                for (int k = 1; (k <= harmonics) && (k <= TI_WAVE_PARTIALS); k++) {
                    sum += gTiWavePartials[wave - 2][k - 1].amp * sin((2.0 * M_PI * k * p) + gTiWavePartials[wave - 2][k - 1].phase);
                }
            }
            t[i] = (float)sum;
        }
    }
}

static void build_all(void) {
    for (int w = 0; w < TI_WAVE_COUNT; w++) {
        gMips[w] = (float *)malloc(sizeof(float) * TI_WAVE_SIZE * TI_WAVE_MIPS);
        build_wave(w, gMips[w]);
    }
}

void ti_osc_init(void) {
    pthread_once(&gOnce, build_all);
}

double ti_osc_sine(double phase) {
    return sin(2.0 * M_PI * phase);
}

double ti_osc_triangle(double phase) {
    double p = phase + 0.25;

    p -= floor(p);
    return (p < 0.5) ? ((4.0 * p) - 1.0) : (3.0 - (4.0 * p));
}

double ti_osc_wave(int wave, double phase, double increment) {
    if (wave <= 0) {
        return ti_osc_sine(phase);
    }
    wave = (wave >= TI_WAVE_COUNT) ? (TI_WAVE_COUNT - 1) : wave;

    // notes §3: the highest table whose top harmonic stays under Nyquist
    double        top = 0.5 / fmax(fabs(increment), 1e-9);
    int           m   = 0;

    for (double h = TI_WAVE_SIZE / 2.0; (h > top) && (m < TI_WAVE_MIPS - 1); h /= 2.0) {
        m++;
    }

    const float * t   = gMips[wave] + (m * TI_WAVE_SIZE);
    double        x   = (phase - floor(phase)) * TI_WAVE_SIZE;
    int           i   = (int)x;
    double        f   = x - i;

    return t[i & (TI_WAVE_SIZE - 1)] + (f * (t[(i + 1) & (TI_WAVE_SIZE - 1)] - t[i & (TI_WAVE_SIZE - 1)]));
}

// notes §2: PolyBLEP, the step's band-limiting correction
static double poly_blep(double t, double dt) {
    if (t < dt) {
        t /= dt;
        return t + t - (t * t) - 1.0;
    }

    if (t > (1.0 - dt)) {
        t = (t - 1.0) / dt;
        return (t * t) + t + t + 1.0;
    }
    return 0.0;
}

double ti_osc_saw(double phase, double increment) {
    return SAW_LEVEL * (1.0 - (2.0 * phase) + poly_blep(phase, increment));   // notes §1: falling, in phase with the sine
}

double ti_osc_pulse(double phase, double increment, double duty) {
    double shifted = phase + 1.0 - duty;

    shifted -= floor(shifted);
    return PULSE_LEVEL * (((phase < duty) ? 1.0 : -1.0) + poly_blep(phase, increment) - poly_blep(shifted, increment) + 1.0 - (2.0 * duty));
}

double ti_osc_square(double phase, double increment) {
    return ti_osc_pulse(phase, increment, 0.5);
}

double ti_osc_classic(int wave, double shape, double width, double phase, double increment) {
    if (shape <= 64.0) {
        double f = shape / 64.0;
        double w = (f < 1.0) ? ti_osc_wave(wave, phase, increment) : 0.0;

        return ((1.0 - f) * w) + (f * ti_osc_saw(phase, increment));
    }
    double f    = (shape - 64.0) / 63.0;
    double duty = 0.5 + (0.5 * fmin(fmax(width, -127.0), 127.0) / 127.0);

    return ((1.0 - f) * ti_osc_saw(phase, increment)) + (f * ti_osc_pulse(phase, increment, duty));
}

// notes §4: the voices at each breakpoint of Density
static const double kHyperDial[9]   = {0, 32, 64, 80, 96, 104, 112, 120, 127};
static const double kHyperWiden[10] = {0, 2.9, 2.9, 1.81, 1.46, 1.26, 1.16, 1.09, 1.04, 1.0};   // the spacing's share with N saws

void ti_osc_hyper(double density, double spread, tTiHyper * h) {
    double d     = fmin(fmax(density, 0.0), 127.0);
    double base  = 3.47 * pow(2.0, (fmin(fmax(spread, 0.0), 127.0) - 64.0) / 19.0);
    int    n     = 1;

    while ((n < 8) && (d >= kHyperDial[n])) {
        n++;
    }
    double f     = (d - kHyperDial[n - 1]) / (kHyperDial[n] - kHyperDial[n - 1]);
    double step0 = base * kHyperWiden[n], step1 = base * kHyperWiden[n + 1];
    double first0 = -0.5 * (n - 1) * step0, first1 = -0.5 * n * step1;
    double step  = step0 + (f * (step1 - step0));
    double first = first0 + (f * (first1 - first0));
    double l0 = 1.0 / sqrt((double)n), l1 = 1.0 / sqrt(n + 1.0);

    h->count = (f > 0.0) ? (n + 1) : n;

    for (int i = 0; i < h->count; i++) {
        h->ratio[i] = pow(2.0, (first + (i * step)) / 1200.0);
        h->gain[i]  = (i < n) ? (l0 + (f * (l1 - l0))) : (pow(f, 0.78) * l1);
    }
}

double ti_osc_hyper_sample(const tTiHyper * h, double * phases, double increment) {
    double sum = 0.0;

    for (int i = 0; i < h->count; i++) {
        double inc = increment * h->ratio[i];

        sum       += h->gain[i] * ti_osc_saw(phases[i], inc);
        phases[i] += inc;
        phases[i] -= floor(phases[i]);
    }

    return HYPER_LEVEL * sum;
}

#define WT_SIZE    (256)
#define WT_MIPS    (5)

static float * gTable[TI_WAVETABLES];     // TI_WAVETABLE_STEPS x WT_MIPS x WT_SIZE

void ti_osc_wavetable_prepare(int table) {
    table = (table < 0) ? 0 : ((table >= TI_WAVETABLES) ? (TI_WAVETABLES - 1) : table);

    if (gTable[table] != NULL) {
        return;
    }
    float * t = (float *)malloc(sizeof(float) * TI_WAVETABLE_STEPS * WT_MIPS * WT_SIZE);

    for (int k = 0; k < TI_WAVETABLE_STEPS; k++) {
        int top = TI_WAVETABLE_PARTIALS;

        for (int m = 0; m < WT_MIPS; m++, top /= 2) {
            float * o = t + (((k * WT_MIPS) + m) * WT_SIZE);

            for (int i = 0; i < WT_SIZE; i++) {
                double p   = (double)i / WT_SIZE;
                double sum = 0.0;

                for (int h = 1; h <= top; h++) {
                    const float * q = gTiWavetables[table][k][h - 1];

                    if (q[0] != 0.0f) {
                        sum += q[0] * cos((2.0 * M_PI * h * p) + q[1]);
                    }
                }

                o[i] = (float)sum;
            }
        }
    }

    gTable[table] = t;
}

static double wt_read(const float * t, double phase) {
    double x = (phase - floor(phase)) * WT_SIZE;
    int    i = (int)x;
    double f = x - i;

    return t[i & (WT_SIZE - 1)] + (f * (t[(i + 1) & (WT_SIZE - 1)] - t[i & (WT_SIZE - 1)]));
}

double ti_osc_wavetable(int table, double index, double phase, double increment) {
    table = (table < 0) ? 0 : ((table >= TI_WAVETABLES) ? (TI_WAVETABLES - 1) : table);

    if (gTable[table] == NULL) {
        ti_osc_wavetable_prepare(table);
    }
    double top = 0.5 / fmax(fabs(increment), 1e-9);
    int    m   = 0;

    for (double h = TI_WAVETABLE_PARTIALS; (h > top) && (m < WT_MIPS - 1); h /= 2.0) {
        m++;
    }

    double p   = fmin(fmax(index, 0.0), 127.0) * (TI_WAVETABLE_STEPS - 1) / 127.0;
    int    k   = (int)p;

    k     = (k > TI_WAVETABLE_STEPS - 2) ? (TI_WAVETABLE_STEPS - 2) : k;
    double a   = wt_read(gTable[table] + (((k * WT_MIPS) + m) * WT_SIZE), phase);
    double b   = wt_read(gTable[table] + ((((k + 1) * WT_MIPS) + m) * WT_SIZE), phase);

    return a + ((p - k) * (b - a));
}
