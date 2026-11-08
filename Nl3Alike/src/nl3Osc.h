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

#ifndef __NL3_OSC_H__
#define __NL3_OSC_H__

#include <stdbool.h>
#include <stdint.h>

#include "nl3Fixed.h"

#define NL3_EDGE_POINTS        (1024)

#define NL3_STEP_RESOLUTION    (128)                                         // notes §5: table points a sample
#define NL3_STEP_SAMPLES       (4)                                           // notes §5: each side of the jump
#define NL3_CLEAN_SAMPLES      (16)                                          // notes §7
#define NL3_STEP_POINTS        (NL3_STEP_SAMPLES * NL3_STEP_RESOLUTION)
#define NL3_CLEAN_POINTS       (NL3_CLEAN_SAMPLES * NL3_STEP_RESOLUTION)

typedef enum {
    eNl3OscSaw = 0,
    eNl3OscSquare,
    eNl3OscDoubleSaw,
    eNl3OscShapes
} tNl3OscShape;

// notes §5, §7: what is left of a band-limited jump of 2, by distance from it
typedef struct {
    double   step[NL3_STEP_POINTS + 1];
    double   clean[NL3_CLEAN_POINTS + 1];
    tNl3Word stepWord[NL3_STEP_POINTS];
} tNl3Steps;

void nl3_steps_build(tNl3Steps * s);

// notes §5: the saw at a phase already advanced by increment (cycles of -1..1 a sample)
double nl3_saw(const tNl3Steps * s, double phase, double increment, bool clean);
// notes §6
tNl3Word nl3_saw_fixed(const tNl3Steps * s, tNl3Word phase, tNl3Word increment);
// notes §8: a pitch (MIDI notes, fractions allowed) as the increment, in both arithmetics
double nl3_pitch_increment(double note);
tNl3Word nl3_pitch_increment_fixed(double note);
// a phase word advanced, wrapping as the instrument's does
tNl3Word nl3_phase_advance(tNl3Word phase, tNl3Word increment);

// notes §4: fills NL3_EDGE_POINTS values
void nl3_edge_build(double * edge);

// notes §2: the edge table's stretch for a note's increment and a Soften of 0..127
double nl3_osc_stretch(double increment, double soften);
// notes §1: one sample at phase -1..1 (advanced by the caller); offset, 0..2, is the second saw's
double nl3_osc_sample(const double * edge, tNl3OscShape shape, double phase, double stretch, double offset);

#endif // __NL3_OSC_H__
