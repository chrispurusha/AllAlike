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
// Notes: Docs/code-notes/uaEmuInterp.md - "// notes §k" refers there.

#include <math.h>
#include <pthread.h>

#include "uaEmuInterp.h"

#define ENGINE_RATE    (44100.0)   // notes §2: the rate the ratios are measured against
#define PHASES         (256)       // table points per source frame
#define MAX_HALF       (5)         // the widest kernel reaches under 5 frames either side
#define TABLE_SIZE     ((2 * MAX_HALF * PHASES) + 2)
#define MAX_TAPS       (2 * MAX_HALF + 1)

// notes §1: each kernel is sinc(cutoff x t) under a Kaiser window of half-length halfLength, t in source frames
typedef struct {
    double cutoff;       // of the source's Nyquist
    double halfLength;   // source frames
    double beta;
} tEmuKernel;

static const tEmuKernel kKernel[UA_EMU_SETTINGS]        = {
    {1.06930, 3.3900, 7.1900},
    {1.14920, 4.7700, 7.3200},
    {0.89630, 4.1700, 6.9500},
    {0.70510, 4.0000, 5.6700},
    {0.35230, 2.5800, 5.6000},
    {0.13810, 2.7000, 7.0500},
    {0.27180, 3.1900, 4.5200},
};
static const double     kStepRatio[UA_EMU_SETTINGS - 1] = {1.25, 1.75, 2.25, 2.75, 3.25, 3.75}; // notes §2

static float            gTable[UA_EMU_SETTINGS][TABLE_SIZE];                                    // each kernel from -MAX_HALF to +MAX_HALF frames
static pthread_once_t   gOnce                           = PTHREAD_ONCE_INIT;

static double bessel_i0(double x) {
    double sum = 1.0, term = 1.0;

    for (int k = 1; k < 50; k++) {
        term *= (x / (2.0 * k)) * (x / (2.0 * k));
        sum  += term;
    }

    return sum;
}

static void build(void) {
    for (int s = 0; s < UA_EMU_SETTINGS; s++) {
        const tEmuKernel * k    = &kKernel[s];
        double             norm = bessel_i0(k->beta);

        for (int i = 0; i < TABLE_SIZE; i++) {
            double t      = ((double)i / PHASES) - MAX_HALF;
            double r      = t / k->halfLength;
            double window = (fabs(r) < 1.0) ? bessel_i0(k->beta * sqrt(1.0 - (r * r))) / norm : 0.0;
            double x      = k->cutoff * t;
            double sinc   = (fabs(x) < 1e-12) ? 1.0 : sin(M_PI * x) / (M_PI * x);

            gTable[s][i] = (float)(sinc * window);
        }
    }
}

void ua_emu_init(void) {
    pthread_once(&gOnce, build);
}

int ua_emu_setting(double step, double hostRate) {
    double ratio   = step * hostRate / ENGINE_RATE;
    int    setting = 0;

    while ((setting < UA_EMU_SETTINGS - 1) && (ratio >= kStepRatio[setting])) {
        setting++;
    }
    return setting;
}

double ua_emu_tail(int setting) {
    return kKernel[setting].halfLength;
}

// notes §3: the taps around position weighted by the kernel, the weights summed to 1
void ua_emu_read2(const float * left, const float * right, uint32_t frames, double position, int setting,
                  float * outL, float * outR) {
    const float * table = gTable[setting];
    double        half  = kKernel[setting].halfLength;
    int64_t       from  = (int64_t)ceil(position - half);
    int64_t       to    = (int64_t)floor(position + half);
    float         w[MAX_TAPS];
    float         sum = 0.0f, sl = 0.0f, sr = 0.0f;
    int           n     = 0;

    for (int64_t i = from; (i <= to) && (n < MAX_TAPS); i++, n++) {
        double t = ((position - (double)i) + MAX_HALF) * PHASES;
        int    j = (int)floor(t);
        float  f = (float)(t - j);

        w[n] = ((j < 0) || (j >= TABLE_SIZE - 1)) ? 0.0f : table[j] + (f * (table[j + 1] - table[j]));
        sum += w[n];
    }

    float         g     = (sum > 1e-6f) ? 1.0f / sum : 0.0f;

    for (int k = 0; k < n; k++) {
        int64_t i = from + k;

        if ((i >= 0) && (i < (int64_t)frames)) {
            sl += left[i] * w[k];
            sr += ((right != NULL) ? right[i] : left[i]) * w[k];
        }
    }

    *outL = sl * g;
    *outR = sr * g;
}
