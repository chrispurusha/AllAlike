/*
 * Sn2Alike - the curve that shapes an oscillator edge.
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

#define RIPPLE_POINTS    (64.0)    // notes §4
#define START_END        (128)
#define MIDDLE_END       (768)

static const double kStart[]  = {-0.00184105, 0.742803, 0.580156, 0.232611, 0.22055, 0.0604005, -0.0233161, 0.00285925, -0.00426551, 0.000777436, -0.00145391, 0.00027063};
static const double kMiddle[] = {0.821549, -0.0362723, -5.02905, -4.11836, -0.227926, 0.0110584};
static const double kEnd[]    = {0.821503, 2.83066e-06, -0.0427244, 0.0621014, -0.0191101, 0.00384213, -0.00247302, 0.00152425, -0.000456329};

static double start_curve(double i) {
    double y = kStart[0] + (kStart[1] * i / START_END);

    for (int k = 1; k <= 10; k++) {
        y += kStart[k + 1] * sin(k * M_PI * i / START_END);
    }

    return fmax(y, 0.0);
}

static double middle_curve(double i, double t) {
    return kMiddle[0] + (cos(t) * (kMiddle[1] + (kMiddle[2] / i) + (kMiddle[3] / (i * i)))) + (kMiddle[4] * sin(t) / i) + (kMiddle[5] * cos(2.0 * t) / i);
}

static double end_curve(double i, double t) {
    double x = (i - MIDDLE_END) / (SN2_EDGE_POINTS - MIDDLE_END);

    return kEnd[0] + (kEnd[1] * x)
           + (cos(t) * (kEnd[2] + (x * (kEnd[3] + (x * (kEnd[4] + (x * kEnd[5])))))))
           + (sin(t) * (kEnd[6] + (x * (kEnd[7] + (x * kEnd[8])))));
}

void sn2_edge_build(double * edge) {
    for (int i = 0; i < SN2_EDGE_POINTS; i++) {
        double t = 2.0 * M_PI * i / RIPPLE_POINTS;

        edge[i] = (i < START_END) ? start_curve(i) : ((i < MIDDLE_END) ? middle_curve(i, t) : end_curve(i, t));
    }
}
