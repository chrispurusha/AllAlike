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

#include <stdbool.h>
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

// notes §9: the classic oscillator with Anti-alias off - the instrument's own points, filter and interpolation
typedef struct {
    double point[5];     // the newest point first
    double out[2];       // the filter's last two outputs, older first
    double place;        // points into the cycle, last pass
    int    points;       // per cycle, frozen while a cycle plays
    double ratio;        // synced: the slave's frequency over the master's, whose phase drives it; 0 when free
    double subPoint[5];  // notes §9: the sub square on the same points, flipping at each wrap
    double subOut[2];
    double subSign;
    bool   started;
} tTiOscPoints;

int ti_osc_points_per_cycle(double frequency);
double ti_osc_classic_points(tTiOscPoints * g, int points, int wave, double shape, double width, double phase, double increment);
// notes §9: Osc 2 synced to Osc 1, made on Osc 1's points from Osc 1's phase and increment
double ti_osc_classic_points_synced(tTiOscPoints * g, int points, int wave, double shape, double width, double phase, double increment, double ratio);
// notes §9: the sub square made on the same points as the last call, -1..1
double ti_osc_points_sub(const tTiOscPoints * g);

// notes §10: Osc 3 with Anti-alias off - the plain wave averaged over each sample's span of phase
double ti_osc_box_saw(double phase, double increment);
double ti_osc_box_pulse(double phase, double increment, double duty);
double ti_osc_classic_box(int wave, double shape, double width, double phase, double increment);

// notes §4: the HyperSaw, up to nine saws set against one another
#define TI_HYPER_SAWS    (9)

typedef struct {
    int    count;
    double ratio[TI_HYPER_SAWS];   // of the note's frequency
    double gain[TI_HYPER_SAWS];
} tTiHyper;

void ti_osc_hyper(double density, double spread, tTiHyper * h);
double ti_osc_hyper_sample(const tTiHyper * h, double * phases, double increment);

// notes §5: the Wavetable oscillator. index 0..127 runs through the table's frames, smooth 0..127 (Smooth)
// The tables are built once, on a thread of their own that ti_osc_init() starts; until a table is ready
// it plays silence. An offline render waits for them first.
void ti_osc_wavetables_start(void);
void ti_osc_wavetables_wait(void);
double ti_osc_wavetable_weight(double between, double smooth);
double ti_osc_wavetable(int table, double index, double smooth, double phase, double increment);

// notes §6: Wave PWM - the table read twice, the copies' rates from Detune, offset by Pulse Width
void ti_osc_pwm_detune(double detune, double * down, double * up);   // the two copies' rates, against the note's
double ti_osc_wave_pwm(int table, double index, double smooth, const double twin[2], double width, double increment);

// notes §7-§8: Grain and Formant. k: how much faster than the note the wave is read within a grain
double ti_osc_grain_window(double x);
double ti_osc_grain(int table, double index, double smooth, double k, double phase, double increment);
// grain: the half-rate phase, then streams A and B's detune drift (cycles); k: A's and B's
double ti_osc_grain_pair(int table, double index, double smooth, const double k[2], const double grain[3], double increment);

double ti_osc_sine(double phase);
double ti_osc_triangle(double phase);
double ti_osc_saw(double phase, double increment);
double ti_osc_square(double phase, double increment);
double ti_osc_pulse(double phase, double increment, double duty);

#endif // __TI_OSC_H__
