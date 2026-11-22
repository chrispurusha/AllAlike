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
    eNl3FilterBp12,        // notes §9
    eNl3FilterBp24,        // notes §10
    eNl3FilterLp6,         // notes §11
    eNl3FilterHp6,         // notes §12
    eNl3FilterClassic,     // notes §13
    eNl3FilterBr,          // notes §14
    eNl3FilterLpHpPar,     // notes §15: a multi mode - lowpass 12 on cutoff 1 beside highpass 12 on cutoff 2
    eNl3FilterPeak,        // notes §16: multi type 4, three all-pass stages with feedback
    eNl3FilterNotch,       // notes §16: multi type 5, the same plus the dry input
    eNl3FilterDistLp,      // notes §17: Dist LP - the 12 dB lowpass driven harder, then a clip
    eNl3FilterBp6,         // notes §19: band pass at the 6 dB slope - its own routine, damping d^2
    eNl3FilterLpBpPar,     // notes §20: multi type 0 - lowpass on cutoff 1 beside band pass on cutoff 2
    eNl3FilterHpLpSer,     // notes §20: multi type 2 - highpass on cutoff 1 into lowpass on cutoff 2
    eNl3FilterBpBpPar,     // notes §20: multi type 3 - band pass on each cutoff, side by side
    eNl3FilterModes
} tNl3FilterMode;

#define NL3_FILTER_TYPES     (2)        // LP, HP
#define NL3_FILTER_SLOPES    (2)        // 12, 24 dB

// notes §3: the words the filter reads, a set per voice
typedef struct {
    tNl3FilterMode mode;
    double         f, k, n2, gain;
    tNl3Word       fw, kw, n2w, gainw;
    double         f2, k2, n22;            // notes §15: the multi modes' second filter
    double         n1, n21;                // notes §20: N for each filter of a multi mode
    tNl3Word       fw2, kw2, n2w2;
    double         fb;                     // notes §16: the peak and notch modes' feedback, from the resonance
    double         dist;                   // notes §17: Dist LP's Distortion, 0..1 (Distortion / 128, 127 as 1)
    tNl3Word       fbw;
} tNl3FilterCoeffs;

// notes §2: two stages of state, in both arithmetics
typedef struct {
    double   low[3], band[3], in[2], mean[2], back;   // notes §16: three stages and the fed-back output
    double   held;                                    // notes §19: Bp6's N x high, a sample late
    tNl3Word lowW[3], bandW[3], inW[2], meanW[2], backW;
} tNl3Filter;

tNl3FilterMode nl3_filter_mode(uint32_t type, uint32_t slope);                                                     // type 0-5 LP HP BP BR Classic Dist, slope 0-2: 6 12 24 dB
// cutoff on the panel's 0..127, plus modulation in semitones; resonance 0..127
double nl3_filter_hz(double cutoff);                                                                               // notes §4: the frequency the dial stands for
double nl3_filter_knob(double knob);                                                                               // notes §4: a Freq knob as the cutoff modulation adds to
void nl3_filter_coeffs(tNl3FilterMode mode, double cutoff, double resonance, double offset, tNl3FilterCoeffs * c); // offset: Osc Mod FILT's, in cutoff-word units (notes §18)
void nl3_noise_coeffs(double shape, double * f, double * k);                                                       // osc notes §15: the Noise waveform's filter, Shape 0..127
void nl3_noise2_coeffs(double increment, double shape, double * f, double * k, double * n);                        // osc notes §20: Osc 2 Noise's filter
// notes §15: a multi mode's two filters, on cutoff and cutoff2
void nl3_filter_coeffs_multi(tNl3FilterMode mode, double cutoff, double cutoff2, double resonance, double offset, tNl3FilterCoeffs * c);
void nl3_filter_reset(tNl3Filter * f);
// x and the result on the instrument's full scale
double nl3_filter_process(tNl3Filter * f, const tNl3FilterCoeffs * c, double x);
tNl3Word nl3_filter_process_fixed(tNl3Filter * f, const tNl3FilterCoeffs * c, tNl3Word x);

#endif // __NL3_FILTER_H__
