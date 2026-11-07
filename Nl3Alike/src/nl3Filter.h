/*
 * Nl3Alike - the filter.
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
// Notes: Docs/code-notes/nl3Filter.md - "// notes §k" refers there.

#ifndef __NL3_FILTER_H__
#define __NL3_FILTER_H__

#include <stdbool.h>
#include <stdint.h>

#include "nl3Fixed.h"

#define NL3_ENGINE_HZ    (44100.0)      // notes §7: the rate the voices run at, whatever the host's

typedef enum {
    eNl3FilterLp12,
    eNl3FilterLp24,
    eNl3FilterHp12,
    eNl3FilterHp24,
    eNl3FilterModes
} tNl3FilterMode;

#define NL3_FILTER_TYPES     (2)        // LP, HP
#define NL3_FILTER_SLOPES    (2)        // 12, 24 dB

// notes §3: the words the filter reads, a set per voice
typedef struct {
    tNl3FilterMode mode;
    double         f, k, n2, gain;
    tNl3Word       fw, kw, n2w, gainw;
} tNl3FilterCoeffs;

// notes §2: two stages of state, in both arithmetics
typedef struct {
    double   low[2], band[2], in[2], mean[2];
    tNl3Word lowW[2], bandW[2], inW[2], meanW[2];
} tNl3Filter;

tNl3FilterMode nl3_filter_mode(uint32_t type, uint32_t slope);
// cutoff on the panel's 0..127, plus modulation in semitones; resonance 0..127
void nl3_filter_coeffs(tNl3FilterMode mode, double cutoff, double resonance, tNl3FilterCoeffs * c);
void nl3_filter_reset(tNl3Filter * f);
// x and the result on the instrument's full scale
double nl3_filter_process(tNl3Filter * f, const tNl3FilterCoeffs * c, double x);
tNl3Word nl3_filter_process_fixed(tNl3Filter * f, const tNl3FilterCoeffs * c, tNl3Word x);

#endif // __NL3_FILTER_H__
