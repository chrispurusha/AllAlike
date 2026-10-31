/*
 * TiAlike - the oscillators.
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

#ifndef __TI_OSC_H__
#define __TI_OSC_H__

#include <stdint.h>

#define TI_WAVE_SIZE     (2048)
#define TI_WAVE_MIPS     (10)
#define TI_WAVE_COUNT    (64)

// Builds the tables once; call before any audio
void ti_osc_init(void);

// One sample of a wave (0 sine, 1 triangle, 2.. spectral) at phase 0..1, band-limited for increment
double ti_osc_wave(int wave, double phase, double increment);

// notes §1: the classic oscillator. shape 0..127 runs wave > saw > pulse, width 0..127
double ti_osc_classic(int wave, double shape, double width, double phase, double increment);

// notes §4: the HyperSaw, up to nine saws set against one another
#define TI_HYPER_SAWS    (9)

typedef struct {
    int    count;
    double ratio[TI_HYPER_SAWS];   // of the note's frequency
    double gain[TI_HYPER_SAWS];
} tTiHyper;

void ti_osc_hyper(double density, double spread, tTiHyper * h);
double ti_osc_hyper_sample(const tTiHyper * h, double * phases, double increment);

// notes §5: the Wavetable oscillator. index 0..127 runs through the table's steps
void ti_osc_wavetable_prepare(int table);   // builds a table on first use
double ti_osc_wavetable(int table, double index, double phase, double increment);

double ti_osc_sine(double phase);
double ti_osc_triangle(double phase);
double ti_osc_saw(double phase, double increment);
double ti_osc_square(double phase, double increment);
double ti_osc_pulse(double phase, double increment, double duty);

#endif // __TI_OSC_H__
