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

#include "tiEnvelope.h"
#include "tiLfo.h"
#include "tiOsc.h"

#define RATE_HZ_PER_POINT    (0.00089180)   // notes §1: Hz for each unit of the fall curve

// notes §1: the envelope's fall curve, read at (127 - dial) x 15/16 + 8, scaled by 128/127 and taken at 9/32 of that
double ti_lfo_rate_hz(double dial) {
    double  a = ((127.0 - fmin(fmax(dial, 0.0), 127.0)) * 0.9375) + 8.0;
    int64_t v = llround(a * 65536.0);

    v += (v >> 7) + (v >> 14);
    return RATE_HZ_PER_POINT * (double)ti_env_fall_curve(((v >> 2) + (v >> 5)) << 2);
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
    l->start = phase;
    l->run   = 0.0;

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

// notes §3-§4: the shape's value at a place in its cycle
static double shape_value(const tTiLfo * l, const tTiLfoShape * s, double phase) {
    double p = contoured(phase, s->contour);
    double v;

    switch (s->shape) {
        case 0:
        {
            // notes §3: the sine is not bent - above the middle Contour drives it into a limit, below it does nothing
            double drive = (s->contour > 0.0) ? pow(1.0 - fmin(s->contour, 0.999), -0.8) : 1.0;

            v = fmax(-1.0, fmin(1.0, -drive * cos(2.0 * M_PI * phase)));   // notes §4: from the minimum
            break;
        }
        case 1:
            v      = (p < 0.5) ? ((4.0 * p) - 1.0) : (3.0 - (4.0 * p));
            break;
        case 2:  v = 1.0 - (2.0 * p);
            break;
        case 3:  v = (phase < (0.5 + (0.49 * s->contour))) ? 1.0 : -1.0;
            break;
        case 4:  v = l->held;
            break;
        case 5:  v = l->last + ((l->target - l->last) * phase);
            break;
        default: v = ti_osc_wave(s->shape - 4, p, 0.001);
            break;
    }
    return v;
}

double ti_lfo_tick(tTiLfo * l, const tTiLfoShape * s, double seconds) {
    double inc = s->rateHz * seconds * (s->envMode ? 2.0 : 1.0);   // notes §2

    if (l->wraps) {
        l->wraps = false;
        l->seen += 1.0;
    }
    l->spent  = l->run >= 1.0;   // notes §2: one whole cycle from where it started
    double v   = shape_value(l, s, l->phase);

    l->run   += inc;
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
            // notes §2: spent, it rests where its cycle ended - the value just before its start
            v = l->pinned ? 2.0 : ((l->spent ? shape_value(l, s, (l->start > 0.0) ? (l->start - 1e-9) : (1.0 - 1e-9)) : v) + 1.0);
        }
    }

    if (s->fadeStep > 0.0) {
        l->fade = fmin(1.0, l->fade + s->fadeStep);
        v      *= l->fade;
    }
    l->value = v;
    return v;
}
