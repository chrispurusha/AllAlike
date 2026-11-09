/*
 * OneShot - a one-sample player, as a VST3 and Audio Unit instrument.
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
// Notes: Docs/code-notes/osResample.md - "// notes §k" refers there.

#include <math.h>
#include <pthread.h>

#include "osResample.h"

#define PHASES         (512)                                   // table points per tap
#define TABLE_SIZE     ((2 * OS_RESAMPLE_HALF * PHASES) + 2)   // one spare for the interpolation's last step
#define KAISER_BETA    (8.6)                                   // notes §2: about 87 dB stopband
#define MIN_CUTOFF     (0.25)                                  // notes §3: two octaves up, then aliasing returns

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
        double u      = ((double)i / PHASES) - OS_RESAMPLE_HALF;   // taps from the centre
        double r      = u / OS_RESAMPLE_HALF;
        double window = (fabs(r) < 1.0) ? bessel_i0(KAISER_BETA * sqrt(1.0 - (r * r))) / norm : 0.0;
        double sinc   = (fabs(u) < 1e-12) ? 1.0 : sin(M_PI * u) / (M_PI * u);

        gKernel[i] = (float)(sinc * window);
    }
}

void os_resample_init(void) {
    pthread_once(&gOnce, build);
}

double os_resample_cutoff(double step) {
    return (step <= 1.0) ? 1.0 : fmax(MIN_CUTOFF, 1.0 / step);
}

double os_resample_tail(double cutoff) {
    return OS_RESAMPLE_HALF / cutoff;
}

static inline float kernel(double u) {
    double t = (u + OS_RESAMPLE_HALF) * PHASES;
    int    i = (int)t;

    if ((i < 0) || (i >= TABLE_SIZE - 1)) {
        return 0.0f;
    }
    float  f = (float)(t - i);

    return gKernel[i] + (f * (gKernel[i + 1] - gKernel[i]));
}

float os_resample_read(const float * d, uint32_t frames, double position, double cutoff) {
    int64_t at    = (int64_t)floor(position);

    // notes §4: on a frame at full bandwidth the kernel is 1 there and 0 at every other frame: the sample itself
    if ((cutoff >= 1.0) && (position == (double)at)) {
        return ((at < 0) || (at >= (int64_t)frames)) ? 0.0f : d[at];
    }
    double  width = OS_RESAMPLE_HALF / cutoff;
    int64_t from  = (int64_t)ceil(position - width);
    int64_t to    = (int64_t)floor(position + width);
    float   sum   = 0.0f;

    from = (from < 0) ? 0 : from;
    to   = (to >= (int64_t)frames) ? (int64_t)frames - 1 : to;

    for (int64_t k = from; k <= to; k++) {
        sum += d[k] * kernel((position - (double)k) * cutoff);
    }

    return sum * (float)cutoff;
}
