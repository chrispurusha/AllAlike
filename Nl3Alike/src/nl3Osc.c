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

#define STEP_OVERSAMPLE    (64)      // notes §5: integration points a table point
#define CLEAN_CUTOFF       (0.92)    // notes §7: of the Nyquist frequency
#define SAW_TILT           (256.0)   // notes §5
#define INDEX_LAST         (NL3_STEP_POINTS - 1)

static double blackman(double u) {
    return 0.42 + (0.5 * cos(M_PI * u)) + (0.08 * cos(2.0 * M_PI * u));
}

static double sinc(double x) {
    return (x == 0.0) ? 1.0 : (sin(M_PI * x) / (M_PI * x));
}

// notes §5: the integral of a windowed sinc, as what remains of a jump of 2 at each table point
static void step_build(double * table, int samples, double cutoff) {
    int    points = samples * NL3_STEP_RESOLUTION;
    int    half   = points * STEP_OVERSAMPLE;
    double total = 0.0, below = 0.0;

    for (int j = -half; j < half; j++) {
        double x = (j + 0.5) / STEP_OVERSAMPLE;

        total += sinc(cutoff * x / NL3_STEP_RESOLUTION) * blackman(x / points);
    }

    for (int j = -half, i = 0; i < points; j++) {
        if (j == (i * STEP_OVERSAMPLE)) {
            table[i++] = 2.0 * (1.0 - (below / total));
        }
        double x = (j + 0.5) / STEP_OVERSAMPLE;

        below += sinc(cutoff * x / NL3_STEP_RESOLUTION) * blackman(x / points);
    }

    table[points] = 0.0;
}

void nl3_steps_build(tNl3Steps * s) {
    step_build(s->step, NL3_STEP_SAMPLES, 1.0);
    step_build(s->clean, NL3_CLEAN_SAMPLES, CLEAN_CUTOFF);

    for (int i = 0; i < NL3_STEP_POINTS; i++) {
        s->stepWord[i] = nl3_word(s->step[i]);
    }
}

static double step_at(const double * table, int points, double distance) {
    double place = fabs(distance) * NL3_STEP_RESOLUTION;

    if (place >= points) {
        return 0.0;
    }
    int    i     = (int)place;
    double v     = table[i] + ((place - i) * (table[i + 1] - table[i]));

    return (distance < 0.0) ? -v : v;
}

double nl3_saw(const tNl3Steps * s, double phase, double increment, bool clean) {
    if (increment <= 0.0) {
        return 0.5 * phase;
    }
    double from = phase + 1.0;    // notes §5: the signed distance to the jump, in phase

    from = (from >= 1.0) ? (from - 2.0) : from;
    double near = from / increment;
    double step;

    if (!clean) {
        step = step_at(s->step, NL3_STEP_POINTS, near);
    } else {
        // notes §7: every jump within reach
        double period = 2.0 / increment;
        int    reach  = (int)ceil(NL3_CLEAN_SAMPLES / period);

        step = 0.0;

        for (int k = -reach; k <= reach; k++) {
            step += step_at(s->clean, NL3_CLEAN_POINTS, near + (k * period));
        }
    }
    return (0.5 * (phase + step)) - (clean ? 0.0 : (increment / SAW_TILT));
}

#define TUNE_WORD         (0x3D376D)  // notes §8: the instrument's tuning word
#define PITCH_E           (64.0)      // the note whose mantissa is 0.25
#define MANTISSA_ONE      (32768.0)   // 0.25 in the 16-bit mantissa
#define MANTISSA_SHIFT    (6)
#define PITCH_SHIFT       (3)         // the exponent at the octave from E4

double nl3_pitch_increment(double note) {
    return nl3_word_value(TUNE_WORD) * pow(2.0, ((note - PITCH_E) / 12.0) - PITCH_SHIFT - 2.0);
}

tNl3Word nl3_pitch_increment_fixed(double note) {
    double  octave   = floor((note - PITCH_E) / 12.0);
    double  semis    = note - PITCH_E - (12.0 * octave);
    int64_t mantissa = (int64_t)floor(MANTISSA_ONE * pow(2.0, semis / 12.0)) << MANTISSA_SHIFT;
    int     shift    = PITCH_SHIFT - (int)octave;
    tNl3Acc product  = nl3_mul((tNl3Word)mantissa, TUNE_WORD);

    product = (shift >= 0) ? (product >> shift) : nl3_asl(product, -shift);

    return nl3_word_limit(product >> 24);
}

tNl3Word nl3_phase_advance(tNl3Word phase, tNl3Word increment) {
    return (tNl3Word)((((uint32_t)phase + (uint32_t)increment) & 0xFFFFFFu) ^ 0x800000u) - 0x800000;
}

tNl3Word nl3_saw_fixed(const tNl3Steps * s, tNl3Word phase, tNl3Word increment) {
    if (increment <= 0) {
        return phase / 2;
    }
    tNl3Word from  = nl3_phase_advance(phase, 0x800000);
    int      shift = 0;

    // notes §6: the increment's reciprocal to nine bits, and where it was normalised
    while ((increment << shift) < 0x400000) {
        shift++;
    }
    int64_t  scale = (INT64_C(0x200000) << 9) / ((int64_t)increment << shift);
    int64_t  index = (2 * (int64_t)from * scale) >> (24 - shift);

    if ((index > INDEX_LAST) || (index < -INDEX_LAST)) {
        index = (from >= 0) ? INDEX_LAST : -INDEX_LAST;
    }
    int64_t  step  = (index >= 0) ? s->stepWord[index] : -s->stepWord[-index];
    tNl3Acc  acc   = ((((tNl3Acc)phase + step) * (INT64_C(1) << 24)) >> 1) - ((tNl3Acc)increment * (INT64_C(1) << 16));

    return nl3_move(acc);
}

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

    if (shape != eNl3OscSaw) {
        double other  = phase + ((shape == eNl3OscSquare) ? offset : (offset - 1.0));

        other = (other >= 1.0) ? (other - 2.0) : ((other < -1.0) ? (other + 2.0) : other);
        double second = ramp(edge, other, stretch);

        out  += (shape == eNl3OscSquare) ? -second : second;
    }
    return out / 2.0;
}
