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

#ifndef __TI_OSC_H__
#define __TI_OSC_H__

#include <stdint.h>

#define SN2_EDGE_POINTS    (1024)

typedef enum {
    eTiOscSaw = 0,
    eTiOscSquare,
    eTiOscDoubleSaw,
    eTiOscShapes
} tSn2OscShape;

// notes §4: fills SN2_EDGE_POINTS values
void sn2_edge_build(double * edge);

// notes §2: the edge table's stretch for a note's increment (cycles of -1..1 a sample) and a Soften of 0..127
double sn2_osc_stretch(double increment, double soften);
// notes §1: one sample at phase -1..1 (advanced by the caller); offset, 0..2, is the second saw's
double sn2_osc_sample(const double * edge, tSn2OscShape shape, double phase, double stretch, double offset);

#endif // __TI_OSC_H__
