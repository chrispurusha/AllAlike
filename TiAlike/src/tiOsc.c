/*
 * TiAlike - the oscillator.
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

#include "tiOsc.h"

// notes §1: PolyBLEP, the step's band-limiting correction
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

double ti_osc_sample(tTiOscShape shape, double phase, double increment, double width) {
    switch (shape) {
        case eTiOscPulse:
        {
            double shifted = fmod(phase + 1.0 - width, 1.0);
            double v       = (phase < width) ? 1.0 : -1.0;

            return v + poly_blep(phase, increment) - poly_blep(shifted, increment);
        }
        case eTiOscTriangle:
            // notes §2: naive - its steps are in the slope, so aliasing is far below the saw's
            return (phase < 0.5) ? ((4.0 * phase) - 1.0) : (3.0 - (4.0 * phase));

        default:
            return (2.0 * phase) - 1.0 - poly_blep(phase, increment);
    }
}
