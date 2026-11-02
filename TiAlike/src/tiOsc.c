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
#include <stdatomic.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>

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
    ti_osc_wavetables_start();
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

static float * _Atomic gTable[TI_WAVETABLES];     // its frames x WT_MIPS x WT_SIZE; NULL until built
static atomic_bool     gTablesBuilt;
static pthread_once_t  gTablesOnce = PTHREAD_ONCE_INIT;

// A table takes up to ~12 ms to build, and 100 of them ~180 ms: far too long for the audio thread,
// which used to build each one the first time a sound asked for it
static float * build_table(int table) {
    int     frames = gTiWavetableCount[table];
    float * t      = (float *)malloc(sizeof(float) * frames * WT_MIPS * WT_SIZE);
    int     k      = 0;

    if (t == NULL) {
        return NULL;
    }

    for (k = 0; k < frames; k++) {
        int top = TI_WAVETABLE_PARTIALS;
        int m   = 0;

        for (m = 0; m < WT_MIPS; m++, top /= 2) {
            float * o = t + (((k * WT_MIPS) + m) * WT_SIZE);
            int     i = 0;

            for (i = 0; i < WT_SIZE; i++) {
                double p   = (double)i / WT_SIZE;
                double sum = 0.0;
                int    h   = 0;

                for (h = 1; h <= top; h++) {
                    const float * q = gTiWavetableFrames[gTiWavetableFirst[table] + k][h - 1];

                    if (q[0] != 0.0f) {
                        sum += q[0] * cos((2.0 * M_PI * h * p) + q[1]);
                    }
                }

                o[i] = (float)sum;
            }
        }
    }

    return t;
}

static void * build_tables(void * unused) {
    int table = 0;

    (void)unused;

    for (table = 0; table < TI_WAVETABLES; table++) {
        atomic_store_explicit(&gTable[table], build_table(table), memory_order_release);
    }

    atomic_store(&gTablesBuilt, true);
    return NULL;
}

static void start_tables(void) {
    pthread_t thread;

    if (pthread_create(&thread, NULL, build_tables, NULL) == 0) {
        pthread_detach(thread);
    } else {
        (void)build_tables(NULL);
    }
}

void ti_osc_wavetables_start(void) {
    pthread_once(&gTablesOnce, start_tables);
}

void ti_osc_wavetables_wait(void) {
    const struct timespec tick = {0, 1000000};

    ti_osc_wavetables_start();

    while (!atomic_load(&gTablesBuilt)) {
        nanosleep(&tick, NULL);
    }
}

static double wt_read(const float * t, double phase) {
    double x = (phase - floor(phase)) * WT_SIZE;
    int    i = (int)x;
    double f = x - i;

    return t[i & (WT_SIZE - 1)] + (f * (t[(i + 1) & (WT_SIZE - 1)] - t[i & (WT_SIZE - 1)]));
}

// notes §5: Smooth steepens the crossfade about its middle; 127 steps from frame to frame
double ti_osc_wavetable_weight(double between, double smooth) {
    double slope = 1.0 - (fmin(fmax(smooth, 0.0), 127.0) / 128.0);

    return fmin(fmax(0.5 + ((between - 0.5) / (slope * slope)), 0.0), 1.0);
}

double ti_osc_wavetable(int table, double index, double smooth, double phase, double increment) {
    table = (table < 0) ? 0 : ((table >= TI_WAVETABLES) ? (TI_WAVETABLES - 1) : table);
    const float * t      = atomic_load_explicit(&gTable[table], memory_order_acquire);

    // not built yet - only in the first moments after the first instance is made
    if (t == NULL) {
        return 0.0;
    }
    double        top    = 0.5 / fmax(fabs(increment), 1e-9);
    int           m      = 0;

    for (double h = TI_WAVETABLE_PARTIALS; (h > top) && (m < WT_MIPS - 1); h /= 2.0) {
        m++;
    }

    int           frames = gTiWavetableCount[table];

    if (frames < 2) {
        return wt_read(t + (m * WT_SIZE), phase);
    }
    double        p      = fmin(fmax(index, 0.0), 127.0) * (frames - 1) / 127.0;
    int           k      = (int)p;

    k = (k > frames - 2) ? (frames - 2) : k;
    double        w      = ti_osc_wavetable_weight(p - k, smooth);
    double        a      = wt_read(t + (((k * WT_MIPS) + m) * WT_SIZE), phase);
    double        b      = (w > 0.0) ? wt_read(t + ((((k + 1) * WT_MIPS) + m) * WT_SIZE), phase) : a;

    return a + (w * (b - a));
}

// notes §6: cents either way; linear up to 32, then rising by about 1.8 every 16
void ti_osc_pwm_detune(double detune, double * down, double * up) {
    double v     = fmin(fmax(detune, 0.0), 127.0);
    double cents = (v < 32.0) ? (0.0495 * v) : (5.04 * pow(2.0, (v - 64.0) / 19.3));

    *up   = pow(2.0, cents / 1200.0);
    *down = pow(2.0, -(cents + (0.0018 * cents * cents)) / 1200.0);
}

// notes §6: (wave(a) - wave(b + offset)) / sqrt 2, the offset half a cycle at Pulse Width 0 and none at 127
double ti_osc_wave_pwm(int table, double index, double smooth, const double twin[2], double width, double increment) {
    double offset = (127.0 - fmin(fmax(width, 0.0), 127.0)) / 254.0;

    return M_SQRT1_2 * (ti_osc_wavetable(table, index, smooth, twin[0], increment) - ti_osc_wavetable(table, index, smooth, twin[1] + offset, increment));
}

// notes §7: the grain window, 1 at a grain's centre and 0 a period away, W(x) + W(1 - x) = 1
double ti_osc_grain_window(double x) {
    double u  = fmin(fmax(x, 0.0), 1.0) - 0.5;
    double u2 = u * u;

    return 0.5 + (u * (-2.3496 + (u2 * (11.2307 + (u2 * (-33.7016 + (u2 * 41.6111)))))));
}

// notes §7: one grain a period, centred on the period's start, the wave read k times as fast
double ti_osc_grain(int table, double index, double smooth, double k, double phase, double increment) {
    double centred = (phase < 0.5) ? phase : (phase - 1.0);

    return ti_osc_grain_window(2.0 * fabs(centred)) * ti_osc_wavetable(table, index, smooth, k * centred, increment * k);
}

// notes §8: two streams of grains two periods long, one period apart, each fading into the other
double ti_osc_grain_pair(int table, double index, double smooth, const double k[2], const double grain[3], double increment) {
    double psi  = (grain[0] < 0.5) ? (2.0 * grain[0]) : ((2.0 * grain[0]) - 2.0);    // periods from stream A's centre
    double psiB = (psi < 0.0) ? (psi + 1.0) : (psi - 1.0);
    double w    = ti_osc_grain_window(fabs(psi));
    double a    = ti_osc_wavetable(table, index, smooth, (k[0] * psi) + grain[1], increment * k[0]);
    double b    = ti_osc_wavetable(table, index, smooth, (k[1] * psiB) + grain[2], increment * k[1]);

    return (w * a) + ((1.0 - w) * b);
}
