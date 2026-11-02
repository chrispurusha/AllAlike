/*
 * Nl3Alike - the oscillator.
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
// Notes: Docs/code-notes/nl3Osc.md - "// notes §k" refers there.

#include <math.h>

#include "nl3Osc.h"

#include <math.h>

#define EDGE_LAST        (1007.0)    // notes §1
#define PITCH_STRETCH    (30.0)      // notes §2
#define PITCH_TRIM       (25.0)
#define SOFT_LEAST       (29.0)
#define SOFT_SCALE       (61.0)
#define SOFT_STEP        (0.96)

// notes §1: a saw's ramp, shaped by the table towards its jump
static double ramp(const double * edge, double phase, double stretch) {
    double place = fmin((1.0 - fabs(phase)) * stretch, EDGE_LAST);
    int    i     = (int)place;
    double f     = place - i;

    return phase * (edge[i] + (f * (edge[i + 1] - edge[i])));
}

double nl3_osc_stretch(double increment, double soften) {
    double pitch = (PITCH_STRETCH / increment) - PITCH_TRIM;
    double whole = floor(soften / 16.0);
    double soft  = SOFT_LEAST + (SOFT_SCALE * ((pow(2.0, whole) * (1.0 + (SOFT_STEP * ((soften / 16.0) - whole)))) - 1.0));

    return fmax(fmin(pitch, soft), 1.0);
}

double nl3_osc_sample(const double * edge, tNl3OscShape shape, double phase, double stretch, double offset) {
    double out = ramp(edge, phase, stretch);

    if (shape != eTiOscSaw) {
        double other  = phase + ((shape == eTiOscSquare) ? offset : (offset - 1.0));

        other = (other >= 1.0) ? (other - 2.0) : ((other < -1.0) ? (other + 2.0) : other);
        double second = ramp(edge, other, stretch);

        out  += (shape == eTiOscSquare) ? -second : second;
    }
    return out / 2.0;
}
