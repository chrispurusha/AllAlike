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

#define SAW_LEVEL         (2.0 / 3.0)                                                // notes §1: against a sine's 1
#define PULSE_LEVEL       (0.5)
#define HYPER_LEVEL       (1.066)                                                    // notes §4: a lone saw of the stack, against the classic saw
#define HYPER_AT_64       (3.50)                                                     // notes §4: cents between saws at Spread 64
#define HYPER_PER_16      (1.79)                                                     // and the growth every 16 steps above the knee
#define HYPER_KNEE        (32.0)
#define HYPER_TUNE        (-0.27)                                                    // notes §4: cents, the stack against the classic oscillator
#define HYPER_PER_STEP    (HYPER_AT_64 / (HYPER_PER_16 * HYPER_PER_16 * HYPER_KNEE)) // below it a straight line from 0

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

// notes §1: the share of the wave (below 64) or the pulse (above), the rest saw
static double classic_mix(double shape) {
    return fabs(127.0 - (2.0 * fmin(fmax(shape, 0.0), 127.0))) / 127.0;
}

static double classic_duty(double width) {
    return 0.5 + (0.5 * fmin(fmax(width, -127.0), 127.0) / 127.0);
}

double ti_osc_classic(int wave, double shape, double width, double phase, double increment) {
    double f = classic_mix(shape);

    if (shape < 63.5) {
        double w = (f > 0.0) ? ti_osc_wave(wave, phase, increment) : 0.0;

        return (f * w) + ((1.0 - f) * ti_osc_saw(phase, increment));
    }
    return ((1.0 - f) * ti_osc_saw(phase, increment)) + (f * ti_osc_pulse(phase, increment, classic_duty(width)));
}

double ti_osc_points_sub(const tTiOscPoints * g) {
    double u = g->place - floor(g->place);

    return g->subOut[0] + (u * (g->subOut[1] - g->subOut[0]));
}

// notes §10: Osc 3 with Anti-alias off - each sample the plain wave's average over the phase it spans, from its integral
static double saw_integral(double p) {   // of 1 - 2p over 0..p
    return p - (p * p);
}

static double pulse_integral(double p, double duty) {   // of +1 below duty, -1 above, over 0..p
    return (p < duty) ? p : ((2.0 * duty) - p);
}

static double box_average(double ( *integral )(double, double), double arg, double phase, double increment) {
    double from  = phase - floor(phase);
    double to    = from + increment;

    if (increment <= 1e-12) {
        return (integral(from + 1e-9, arg) - integral(from, arg)) / 1e-9;
    }

    if (to <= 1.0) {
        return (integral(to, arg) - integral(from, arg)) / increment;
    }
    double whole = integral(1.0, arg);
    double turns = floor(to);

    return ((whole - integral(from, arg)) + ((turns - 1.0) * whole) + integral(to - turns, arg)) / increment;
}

static double saw_integral_arg(double p, double unused) {
    (void)unused;
    return saw_integral(p);
}

double ti_osc_box_saw(double phase, double increment) {
    return SAW_LEVEL * box_average(saw_integral_arg, 0.0, phase, increment);
}

double ti_osc_box_pulse(double phase, double increment, double duty) {
    return PULSE_LEVEL * (box_average(pulse_integral, duty, phase, increment) + 1.0 - (2.0 * duty));
}

double ti_osc_classic_box(int wave, double shape, double width, double phase, double increment) {
    double f = classic_mix(shape);

    if (shape < 63.5) {
        return ((1.0 - f) * ti_osc_box_saw(phase, increment)) + ((f > 0.0) ? (f * ti_osc_wave(wave, phase + (0.5 * increment), 1e-9)) : 0.0);
    }
    return ((1.0 - f) * ti_osc_box_saw(phase, increment)) + (f * ti_osc_box_pulse(phase, increment, classic_duty(width)));
}

// notes §9: points per cycle by note, base x 2^(10 - octave) counting octaves from A
int ti_osc_points_per_cycle(double frequency) {
    static const double kBase[12] = {5.0, 5.0, 5.0, 5.0, 4.0, 4.0, 4.0, 4.0, 3.5, 3.5, 3.0, 3.0};
    int                 note      = (int)lround(69.0 + (12.0 * log2(fmax(frequency, 1.0) / 440.0))) - 9;
    int                 octave    = (int)floor(note / 12.0);
    double              points    = kBase[note - (12 * octave)] * ldexp(1.0, 10 - octave);

    return (int)fmin(fmax(points, 2.0), 1 << 20);
}

// notes §9: one point - the saw at its own place, the wave and the pulse at the point before
// notes §9: synced, the points are the master's and the slave's place is the ratio times the master's, so it
// restarts on the master's wrap
static double point_place(int k, int points, double ratio) {
    double place = (double)(((k % points) + points) % points) / points;

    place *= (ratio > 0.0) ? ratio : 1.0;
    return place - floor(place);
}

static double classic_point(int wave, double shape, double width, int k, int points, double ratio) {
    double here = point_place(k, points, ratio);
    double back = point_place(k - 1, points, ratio);
    double f    = classic_mix(shape);
    double saw  = SAW_LEVEL * (1.0 - (2.0 * here));

    if (shape < 63.5) {
        return ((1.0 - f) * saw) + ((f > 0.0) ? (f * ti_osc_wave(wave, back, 1e-9)) : 0.0);
    }
    double duty = classic_duty(width);

    return ((1.0 - f) * saw) + (f * PULSE_LEVEL * (((back < duty) ? 1.0 : -1.0) + 1.0 - (2.0 * duty)));
}

static void points_take(tTiOscPoints * g, int wave, double shape, double width, int k) {
    double p = classic_point(wave, shape, width, k, g->points, g->ratio);

    g->point[4]    = g->point[3];
    g->point[3]    = g->point[2];
    g->point[2]    = g->point[1];
    g->point[1]    = g->point[0];
    g->point[0]    = p;
    g->out[0]      = g->out[1];
    g->out[1]      = (g->point[0] + (4.0 * g->point[1]) + (5.0 * g->point[2]) + (4.0 * g->point[3]) + g->point[4]) / 16.0;

    if ((((k % g->points) + g->points) % g->points) == 0) {
        g->subSign = (g->subSign > 0.0) ? -1.0 : 1.0;   // notes §9: the sub flips on the wrap's own point
    }
    g->subPoint[4] = g->subPoint[3];
    g->subPoint[3] = g->subPoint[2];
    g->subPoint[2] = g->subPoint[1];
    g->subPoint[1] = g->subPoint[0];
    g->subPoint[0] = g->subSign;
    g->subOut[0]   = g->subOut[1];
    g->subOut[1]   = (g->subPoint[0] + (4.0 * g->subPoint[1]) + (5.0 * g->subPoint[2]) + (4.0 * g->subPoint[3]) + g->subPoint[4]) / 16.0;
}

static void points_pass(tTiOscPoints * g, int wave, double shape, double width, double phase) {
    double place = phase * g->points;
    double moved = place - g->place;

    if (moved < 0.0) {
        moved += g->points;   // the cycle wrapped, or a sync restarted it
    }

    for (int k = (int)floor(g->place) + 1; (k <= (int)floor(g->place + moved)) && (k <= (int)floor(g->place) + g->points); k++) {
        points_take(g, wave, shape, width, k);
    }

    g->place = place;
}

double ti_osc_classic_points(tTiOscPoints * g, int points, int wave, double shape, double width, double phase, double increment) {
    if (g->ratio != 0.0) {
        g->ratio   = 0.0;
        g->started = false;
    }
    return ti_osc_classic_points_synced(g, points, wave, shape, width, phase, increment, 0.0);
}

double ti_osc_classic_points_synced(tTiOscPoints * g, int points, int wave, double shape, double width, double phase, double increment, double ratio) {
    g->ratio = ratio;

    if (!g->started || (points != g->points)) {
        g->points  = points;
        g->place   = (phase - floor(phase)) * points;
        g->started = true;
    }
    double half  = phase + (0.5 * increment);

    points_pass(g, wave, shape, width, half - floor(half));   // two passes a sample, the second one heard
    double whole = phase + increment;

    whole   -= floor(whole);
    points_pass(g, wave, shape, width, whole);
    double u     = g->place - floor(g->place);

    return g->out[0] + (u * (g->out[1] - g->out[0]));
}

// notes §4: the voices at each breakpoint of Density
static const double kHyperDial[9]   = {0, 32, 64, 80, 96, 104, 112, 120, 127};
static const double kHyperWiden[10] = {0, 2.9, 2.9, 1.81, 1.46, 1.26, 1.16, 1.09, 1.04, 1.0};   // the spacing's share with N saws

void ti_osc_hyper(double density, double spread, tTiHyper * h) {
    double d     = fmin(fmax(density, 0.0), 127.0);
    double sp    = fmin(fmax(spread, 0.0), 127.0);
    double base  = (sp <= HYPER_KNEE) ? (HYPER_PER_STEP * sp) : (HYPER_AT_64 * pow(HYPER_PER_16, (sp - 64.0) / 16.0));   // notes §4
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
        h->ratio[i] = pow(2.0, (first + (i * step) + HYPER_TUNE) / 1200.0);
        h->gain[i]  = (i < n) ? (l0 + (f * (l1 - l0))) : (pow(f, 0.78) * l1);
    }
}

// notes §9: Anti-alias off, each saw of the stack made on points of its own, as the classic saw is
double ti_osc_hyper_points(const tTiHyper * h, tTiOscPoints * g, double * phases, double increment, double rate) {
    double sum = 0.0;

    for (int i = 0; i < h->count; i++) {
        double inc = increment * h->ratio[i];

        sum       += h->gain[i] * ti_osc_classic_points(&g[i], ti_osc_points_per_cycle(inc * rate), 0, 63.5, 64.0, phases[i], inc);
        phases[i] += inc;
        phases[i] -= floor(phases[i]);
    }

    return HYPER_LEVEL * sum;
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

#define WT_SIZE        (512)    // notes §5: the longest cycle
#define WT_VERSIONS    (9)      // each frame band-limited to 256, 128 ... 1 partials
#define WT_FRAME       (1193)   // the words of a frame's versions, guards included

static const int       kVersionSize[WT_VERSIONS] = {512, 256, 128, 64, 64, 64, 32, 32, 32};

static float * _Atomic gTable[TI_WAVETABLES];     // its frames x WT_FRAME; NULL until built
static atomic_bool     gTablesBuilt;
static pthread_once_t  gTablesOnce               = PTHREAD_ONCE_INIT;

// notes §5: a version's cycle, no longer than the table's own
static int size_version(int tableSize, int v) {
    return (kVersionSize[v] < tableSize) ? kVersionSize[v] : tableSize;
}

static int size_version_at(int tableSize, int v) {
    int at = 0;

    for (int u = 0; u < v; u++) {
        at += size_version(tableSize, u) + 1;
    }

    return at;
}

static int version_size(int table, int v) {
    return size_version(gTiWavetableSize[table], v);
}

static int version_at(int table, int v) {
    return size_version_at(gTiWavetableSize[table], v);
}

// A table takes up to ~12 ms to build, and 100 of them ~180 ms: far too long for the audio thread,
// which used to build each one the first time a sound asked for it
static float * build_table(int table) {
    int           frames = gTiWavetableCount[table];
    float *       t      = (float *)malloc(sizeof(float) * frames * WT_FRAME);
    static double cosine[WT_SIZE], sine[WT_SIZE];

    if (t == NULL) {
        return NULL;
    }

    for (int i = 0; i < WT_SIZE; i++) {
        cosine[i] = cos(2.0 * M_PI * i / WT_SIZE);
        sine[i]   = sin(2.0 * M_PI * i / WT_SIZE);
    }

    for (int k = 0; k < frames; k++) {
        int frame = gTiWavetableFirst[table] + k;

        for (int v = 0; v < WT_VERSIONS; v++) {
            int     size = version_size(table, v);
            int     top  = TI_WAVETABLE_PARTIALS >> v;
            int     step = WT_SIZE / size;
            float * o    = t + (k * WT_FRAME) + version_at(table, v);
            double  sum[WT_SIZE];

            top = (top < (size / 2)) ? top : (size / 2);

            for (int i = 0; i < size; i++) {
                sum[i] = 0.0;
            }

            for (unsigned int at = gTiWavetableStart[frame]; at < gTiWavetableStart[frame + 1]; at++) {
                const tTiWavetablePartial * q = &gTiWavetablePartials[at];

                if (q->partial > top) {
                    continue;
                }
                double                      c = q->amp * cos(q->phase), s = q->amp * sin(q->phase);

                for (int i = 0; i < size; i++) {
                    int place = (q->partial * i * step) & (WT_SIZE - 1);   // cos(2 pi h i / size + phase)

                    sum[i] += (c * cosine[place]) - (s * sine[place]);
                }
            }

            for (int i = 0; i < size; i++) {
                o[i] = (float)sum[i];
            }

            o[size] = o[0];   // the guard, for the straight line past the end
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

#define WT_FIRST_STEP    (1.0 / 256.0)    // notes §5: the increment at which version 1 takes over, half-cycles a sample
#define WT_STEEP_LAST    (4)              // the last version below the top read steepened

// notes §5: a straight line between samples whose step is steepened by m, and held once it arrives
static double wt_read(const float * t, int size, double phase, double m) {
    double x = (phase - floor(phase)) * size;
    int    i = (int)x;
    double w = fmin((x - i) * m, 1.0);

    i = (i < size) ? i : (size - 1);
    return t[i] + (w * (t[i + 1] - t[i]));
}

static int top_version(int tableSize) {
    int v = 0;

    while ((v < WT_VERSIONS - 1) && (kVersionSize[v] > tableSize)) {
        v++;
    }
    return v;
}

// notes §5: Smooth steepens the crossfade about its middle; 127 steps from frame to frame
double ti_osc_wavetable_weight(double between, double smooth) {
    double slope = 1.0 - (fmin(fmax(smooth, 0.0), 127.0) / 128.0);

    return fmin(fmax(0.5 + ((between - 0.5) / (slope * slope)), 0.0), 1.0);
}

// notes §5a: the user tables this thread is rendering with, Osc 1's and Osc 2's
static _Thread_local const tTiUserTable * tUser[2];

void ti_osc_set_user_tables(const tTiUserTable * osc1, const tTiUserTable * osc2) {
    tUser[0] = osc1;
    tUser[1] = osc2;
}

static double table_read(const float * t, int frames, int tableSize, bool sharpTop, double index, double smooth, double phase, double increment) {
    // notes §5: one version an octave, by the increment in half-cycles a sample
    double step  = 2.0 * fabs(increment);
    int    top   = top_version(tableSize);
    int    v     = top;

    while ((v < WT_VERSIONS - 1) && (step >= (WT_FIRST_STEP * pow(2.0, v)))) {
        v++;
    }
    bool   steep = (v == top) ? sharpTop : (v <= WT_STEEP_LAST);
    double m     = 1.0;

    if (steep && (step > 0.0)) {
        // the next version's threshold over the increment, in 1/128ths
        m = floor(WT_FIRST_STEP * pow(2.0, v) / step * 128.0) / 128.0;
    }
    int    size  = size_version(tableSize, v);
    int    at    = size_version_at(tableSize, v);

    if (frames < 2) {
        return wt_read(t + at, size, phase, m);
    }
    double p     = fmin(fmax(index, 0.0), 127.0) * (frames - 1) / 127.0;
    int    k     = (int)p;

    k = (k > frames - 2) ? (frames - 2) : k;
    double w     = ti_osc_wavetable_weight(p - k, smooth);
    double a     = wt_read(t + (k * WT_FRAME) + at, size, phase, m);
    double b     = (w > 0.0) ? wt_read(t + ((k + 1) * WT_FRAME) + at, size, phase, m) : a;

    return a + (w * (b - a));
}

double ti_osc_wavetable(int table, double index, double smooth, double phase, double increment) {
    if (table >= TI_WAVETABLE_USER) {
        const tTiUserTable * u = tUser[(table > TI_WAVETABLE_USER) ? 1 : 0];

        if (u != NULL) {
            return table_read(u->data, u->frames, u->size, false, index, smooth, phase, increment);
        }
        table = 0;   // notes §5a: no table loaded - the sine table, not silence
    }
    table = (table < 0) ? 0 : ((table >= TI_WAVETABLES) ? (TI_WAVETABLES - 1) : table);
    const float * t = atomic_load_explicit(&gTable[table], memory_order_acquire);

    // not built yet - only in the first moments after the first instance is made
    if (t == NULL) {
        return 0.0;
    }
    return table_read(t, gTiWavetableCount[table], gTiWavetableSize[table], gTiWavetableSharpTop[table] != 0, index, smooth, phase, increment);
}

// notes §5a: a table from one cycle per frame - each cycle's partials, then the same versions as the built-in tables
tTiUserTable * ti_osc_user_table(const float * cycles, int frames, int cycle) {
    if ((cycles == NULL) || (frames < 1) || (cycle < 8)) {
        return NULL;
    }
    frames   = (frames > TI_USER_TABLE_FRAMES) ? TI_USER_TABLE_FRAMES : frames;
    int            partials  = (cycle / 2) - 1;

    partials = (partials > (TI_WAVETABLE_PARTIALS - 1)) ? (TI_WAVETABLE_PARTIALS - 1) : partials;   // notes §5a: below the longest cycle's Nyquist, so a saved table rebuilds the same
    int            tableSize = 64;

    while ((tableSize < WT_SIZE) && ((tableSize / 2) < partials)) {
        tableSize *= 2;
    }
    tTiUserTable * u         = (tTiUserTable *)calloc(1, sizeof(tTiUserTable) + (sizeof(float) * frames * WT_FRAME));
    double *       re        = (double *)malloc(sizeof(double) * (partials + 1));
    double *       im        = (double *)malloc(sizeof(double) * (partials + 1));
    double *       turn      = (double *)malloc(sizeof(double) * 2 * cycle); // cos and sin round the cycle
    double         cosine[WT_SIZE], sine[WT_SIZE];                           // on the stack: instances may build at once

    if ((u == NULL) || (re == NULL) || (im == NULL) || (turn == NULL)) {
        free(u);
        free(re);
        free(im);
        free(turn);
        return NULL;
    }

    for (int n = 0; n < cycle; n++) {
        turn[2 * n]       = cos(2.0 * M_PI * n / cycle);
        turn[(2 * n) + 1] = sin(2.0 * M_PI * n / cycle);
    }

    for (int i = 0; i < WT_SIZE; i++) {
        cosine[i] = cos(2.0 * M_PI * i / WT_SIZE);
        sine[i]   = sin(2.0 * M_PI * i / WT_SIZE);
    }

    u->frames = frames;
    u->size   = tableSize;
    double peak = 0.0;

    for (int k = 0; k < frames; k++) {
        const float * x = cycles + ((size_t)k * cycle);

        // each partial as c cos + s sin of its own angle: 2/cycle of the transform
        for (int h = 1; h <= partials; h++) {
            double c = 0.0, d = 0.0;
            int    place = 0;

            for (int n = 0; n < cycle; n++) {
                c     += x[n] * turn[2 * place];
                d     += x[n] * turn[(2 * place) + 1];
                place += h;
                place -= (place >= cycle) ? cycle : 0;
            }

            re[h] = 2.0 * c / cycle;
            im[h] = 2.0 * d / cycle;
        }

        for (int v = 0; v < WT_VERSIONS; v++) {
            int     size = size_version(tableSize, v);
            int     top  = TI_WAVETABLE_PARTIALS >> v;
            float * o    = u->data + ((size_t)k * WT_FRAME) + size_version_at(tableSize, v);

            top     = (top < (size / 2)) ? top : (size / 2);
            top     = (top < partials) ? top : partials;

            int     step = WT_SIZE / size;

            for (int i = 0; i < size; i++) {
                double sum = 0.0;

                for (int h = 1; h <= top; h++) {
                    int place = (h * i * step) & (WT_SIZE - 1);

                    sum += (re[h] * cosine[place]) + (im[h] * sine[place]);
                }

                o[i] = (float)sum;

                if (v == 0) {
                    peak = fmax(peak, fabs(sum));
                }
            }

            o[size] = o[0];
        }
    }

    free(re);
    free(im);
    free(turn);

    // notes §5a: the loudest frame's peak at 1, a sine's level
    if (peak > 0.0) {
        for (size_t i = 0; i < (size_t)frames * WT_FRAME; i++) {
            u->data[i] = (float)(u->data[i] / peak);
        }
    }
    return u;
}

// notes §5a: a frame's cycle as built, u->size samples - what a saved project keeps
const float * ti_osc_user_table_cycle(const tTiUserTable * u, int frame) {
    return u->data + ((size_t)frame * WT_FRAME);
}

void ti_osc_user_table_free(tTiUserTable * u) {
    free(u);
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
