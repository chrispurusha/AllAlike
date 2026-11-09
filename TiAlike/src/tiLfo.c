/*
 * TiAlike - the LFOs.
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
// Notes: Docs/code-notes/tiLfo.md - "// notes §k" refers there.

#include <math.h>

#include "tiLfo.h"
#include "tiOsc.h"

// notes §1: Hz at each sixteenth step of the dial, in between geometrically
double ti_lfo_rate_hz(double dial) {
    static const double kHz[9] = {0.0206, 0.0741, 0.2218, 0.5215, 1.075, 2.242, 4.93, 13.16, 60.0};
    double              p      = fmin(fmax(dial, 0.0), 127.0) / 16.0;
    int                 i      = (int)p;

    i = (i > 7) ? 7 : i;
    return kHz[i] * pow(kHz[i + 1] / kHz[i], (p - i) / ((i == 7) ? (15.0 / 16.0) : 1.0));
}

static double random_bipolar(uint32_t * s) {
    *s = (*s * 1664525u) + 1013904223u;
    return ((double)(int32_t)*s) / 2147483648.0;
}

void ti_lfo_trigger(tTiLfo * l, double phase, uint32_t seed) {
    l->phase = phase;
    l->fade  = 0.0;
    l->spent = false;
    l->seen  = 0.0;

    if (seed != 0u) {
        l->noise = seed;
    }
    l->held  = l->target = random_bipolar(&l->noise);
}

// notes §3: Contour bends the phase, so a shape's ends stay where they are
static double contoured(double p, double contour) {
    double mid = 0.5 + (0.49 * contour);

    return (p < mid) ? (0.5 * p / mid) : (0.5 + (0.5 * (p - mid) / (1.0 - mid)));
}

double ti_lfo_tick(tTiLfo * l, const tTiLfoShape * s, double seconds) {
    double inc = s->rateHz * seconds * (s->envMode ? 2.0 : 1.0);   // notes §2
    double p   = contoured(l->phase, s->contour);

    if (l->wraps) {
        l->wraps = false;
        l->seen += 1.0;
        l->spent = l->seen > 0.0;
    }
    double v;

    switch (s->shape) {
        case 0:
        {
            // notes §3: the sine is not bent - above the middle Contour drives it into a limit, below it does nothing
            double drive = (s->contour > 0.0) ? pow(1.0 - fmin(s->contour, 0.999), -0.8) : 1.0;

            v = fmax(-1.0, fmin(1.0, -drive * cos(2.0 * M_PI * l->phase)));   // notes §4: from the minimum
            break;
        }
        case 1:
            v      = (p < 0.5) ? ((4.0 * p) - 1.0) : (3.0 - (4.0 * p));
            break;
        case 2:  v = 1.0 - (2.0 * p);
            break;
        case 3:  v = (l->phase < (0.5 + (0.49 * s->contour))) ? 1.0 : -1.0;
            break;
        case 4:  v = l->held;
            break;
        case 5:  v = l->last + ((l->target - l->last) * l->phase);
            break;
        default: v = ti_osc_wave(s->shape - 4, p, 0.001);
            break;
    }
    l->phase += inc;

    if (l->phase >= 1.0) {
        l->phase -= floor(l->phase);
        l->wraps  = true;
        l->held   = random_bipolar(&l->noise);
        l->last   = l->target;
        l->target = l->held;
    }

    // notes §2: Env Mode is one cycle, lifted to sit on zero, and then nothing
    if (s->envMode) {
        if ((s->shape == 4) || (s->shape == 5)) {
            v = l->spent ? l->value : v;
        } else {
            v = l->pinned ? 2.0 : (l->spent ? 0.0 : (v + 1.0));
        }
    }

    if (s->fadeStep > 0.0) {
        l->fade = fmin(1.0, l->fade + s->fadeStep);
        v      *= l->fade;
    }
    l->value = v;
    return v;
}
