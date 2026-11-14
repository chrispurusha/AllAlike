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
// Notes: Docs/code-notes/uaResample.md - "// notes §k" refers there.

#include <math.h>
#include <pthread.h>
#include <stdbool.h>

#include "uaResample.h"

#define PHASES         (512)                                   // table points per tap
#define TABLE_SIZE     ((2 * UA_RESAMPLE_HALF * PHASES) + 2)   // one spare for the interpolation's last step
#define KAISER_BETA    (8.6)                                   // notes §2: about 87 dB stopband
#define MIN_CUTOFF     (0.25)                                  // notes §3: two octaves up, then aliasing returns
#define MAX_TAPS       ((2 * UA_RESAMPLE_HALF * 4) + 2)        // the widest kernel: 1 / MIN_CUTOFF

static float          gKernel[TABLE_SIZE];   // the windowed sinc from -HALF to +HALF taps
static pthread_once_t gOnce = PTHREAD_ONCE_INIT;

// The zeroth-order modified Bessel function, by its series
static double bessel_i0(double x) {
    double sum = 1.0, term = 1.0;

    for (int k = 1; k < 50; k++) {
        term *= (x / (2.0 * k)) * (x / (2.0 * k));
        sum  += term;
    }

    return sum;
}

static void build(void) {
    double norm = bessel_i0(KAISER_BETA);

    for (int i = 0; i < TABLE_SIZE; i++) {
        double u      = ((double)i / PHASES) - UA_RESAMPLE_HALF;   // taps from the centre
        double r      = u / UA_RESAMPLE_HALF;
        double window = (fabs(r) < 1.0) ? bessel_i0(KAISER_BETA * sqrt(1.0 - (r * r))) / norm : 0.0;
        double sinc   = (fabs(u) < 1e-12) ? 1.0 : sin(M_PI * u) / (M_PI * u);

        gKernel[i] = (float)(sinc * window);
    }
}

void ua_resample_init(void) {
    pthread_once(&gOnce, build);
}

double ua_resample_cutoff(double step) {
    return (step <= 1.0) ? 1.0 : fmax(MIN_CUTOFF, 1.0 / step);
}

double ua_resample_tail(double cutoff) {
    return UA_RESAMPLE_HALF / cutoff;
}

// notes §5: the weights for the taps around position, into w; returns the first tap's frame and sets *count
static int64_t weights(uint32_t frames, double position, double cutoff, float * w, int * count) {
    double  width  = UA_RESAMPLE_HALF / cutoff;
    int64_t from   = (int64_t)ceil(position - width);
    int64_t to     = (int64_t)floor(position + width);

    from   = (from < 0) ? 0 : from;
    to     = (to >= (int64_t)frames) ? (int64_t)frames - 1 : to;
    int     n      = (to >= from) ? (int)(to - from + 1) : 0;

    // notes §5: walking the table, one tap's distance (cutoff x PHASES points) at a time
    double  stride = cutoff * PHASES;
    double  t      = (((position - (double)from) * cutoff) + UA_RESAMPLE_HALF) * PHASES;
    float   gain   = (float)cutoff;

    for (int k = 0; k < n; k++, t -= stride) {
        int   i = (int)floor(t);
        float f = (float)(t - i);

        w[k] = ((i < 0) || (i >= TABLE_SIZE - 1)) ? 0.0f : (gKernel[i] + (f * (gKernel[i + 1] - gKernel[i]))) * gain;
    }

    *count = n;
    return from;
}

void ua_resample_read2(const float * left, const float * right, uint32_t frames, double position, double cutoff,
                       float * outL, float * outR) {
    int64_t       at   = (int64_t)floor(position);

    // notes §4: on a frame at full bandwidth the kernel is 1 there and 0 at every other frame: the sample itself
    if ((cutoff >= 1.0) && (position == (double)at)) {
        bool inside = (at >= 0) && (at < (int64_t)frames);

        *outL = inside ? left[at] : 0.0f;
        *outR = (right == NULL) ? *outL : (inside ? right[at] : 0.0f);
        return;
    }
    float         w[MAX_TAPS];
    int           n;
    int64_t       from = weights(frames, position, cutoff, w, &n);
    float         sl = 0.0f, sr = 0.0f;
    const float * l    = left + from;

    for (int k = 0; k < n; k++) {
        sl += l[k] * w[k];
    }

    if (right != NULL) {
        const float * r = right + from;

        for (int k = 0; k < n; k++) {
            sr += r[k] * w[k];
        }
    } else {
        sr = sl;
    }
    *outL = sl;
    *outR = sr;
}

float ua_resample_read(const float * d, uint32_t frames, double position, double cutoff) {
    float l, r;

    ua_resample_read2(d, NULL, frames, position, cutoff, &l, &r);
    return l;
}
