/*
 * Sn2Alike - the oscillator.
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
// Notes: Docs/code-notes/sn2Osc.md - "// notes §k" refers there.

#include <math.h>

#include "sn2Osc.h"

#include <math.h>

#define EDGE_LAST        (1007.0)    // notes §1
#define STRETCH_SCALE    (64.0)      // notes §2
#define STRETCH_TRIM     (30.0)
#define SQUARE_TRIM      (15.0)
#define SOFT_LAG         (2.0)       // dial steps
#define PITCH_KNOT       (0.0071649) // the increment whose limit is SCALE x 2^6
#define PITCH_OCTAVES    (6.0)
#define SYNC_TOP         (120.0)     // notes §5: a ratio of 32

// notes §1: a saw's ramp, shaped by the table towards its jump
static double ramp(const double * edge, double phase, double stretch) {
    double place = fmin((1.0 - fabs(phase)) * stretch, EDGE_LAST);
    int    i     = (int)place;
    double f     = place - i;

    return phase * (edge[i] + (f * (edge[i + 1] - edge[i])));
}

double sn2_osc_stretch(tSn2OscShape shape, double increment, double soften) {
    double pitch   = PITCH_OCTAVES - log2(increment / PITCH_KNOT);
    double soft    = ((soften * 128.0 / 127.0) - SOFT_LAG) / 16.0;
    double octaves = fmin(pitch, soft);
    double whole   = floor(octaves);

    return fmax(floor((STRETCH_SCALE * exp2(whole) * (1.0 + octaves - whole)) - ((shape == eTiOscSquare) ? SQUARE_TRIM : STRETCH_TRIM)), 1.0);
}

double sn2_sync_ratio(double sync) {
    return pow(2.0, fmin(sync, SYNC_TOP) / 24.0);
}

// notes §5: the slave restarts with each of the oscillator's cycles; its last, partial cycle is centred on zero
double sn2_sync_phase(double phase, double ratio) {
    double cycles = ratio * (phase + 1.0) / 2.0;
    double whole  = floor(cycles);
    double slave  = (2.0 * (cycles - whole)) - 1.0;
    double part   = ratio - floor(ratio);

    return ((whole >= floor(ratio)) && (part > 0.0)) ? (slave + 1.0 - part) : slave;
}

double sn2_osc_sample_synced(const double * edge, tSn2OscShape shape, double phase, double ratio, double stretch, double offset) {
    double out = ramp(edge, sn2_sync_phase(phase, ratio), stretch);

    if (shape != eTiOscSaw) {   // the second ramp's own slave, its master moved on by the offset over the ratio
        double other  = phase + (((shape == eTiOscSquare) ? offset : (offset - 1.0)) / ratio);

        other = (other >= 1.0) ? (other - 2.0) : ((other < -1.0) ? (other + 2.0) : other);
        double second = ramp(edge, sn2_sync_phase(other, ratio), stretch);

        out  += (shape == eTiOscSquare) ? -second : second;
    }
    return out / 2.0;
}

double sn2_osc_sample(const double * edge, tSn2OscShape shape, double phase, double stretch, double offset) {
    double out = ramp(edge, phase, stretch);

    if (shape != eTiOscSaw) {
        double other  = phase + ((shape == eTiOscSquare) ? offset : (offset - 1.0));

        other = (other >= 1.0) ? (other - 2.0) : ((other < -1.0) ? (other + 2.0) : other);
        double second = ramp(edge, other, stretch);

        out  += (shape == eTiOscSquare) ? -second : second;
    }
    return out / 2.0;
}
