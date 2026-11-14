/*
 * UltraAlike - a sampler (a placeholder on a sample player's engine), as a VST3 and Audio Unit instrument.
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
// Notes: Docs/code-notes/uaFx.md - "// notes §k" refers there.

#ifndef __UA_FX_H__
#define __UA_FX_H__

#include <stdbool.h>
#include <stdint.h>

// notes §5: Soft is a gentle shape - Low Cut 12 dB/octave from two one-pole stages, High Cut 6 dB/octave from one
typedef enum {
    eUaSlopeSoft = 0,
    eUaSlope12,
    eUaSlope24
} tUaSlope;

typedef enum {
    eUaSatClassic = 0,
    eUaSatDrums,
    eUaSatTape
} tUaSatMode;

// What a layer's effects are set to, read once a block
typedef struct {
    bool       filter;          // notes §1: Low Cut and High Cut
    double     lowHz, highHz;
    tUaSlope   slope;
    bool       saturate;        // notes §3
    tUaSatMode mode;
    double     gainDb, outputDb;
    double     warmthDb;        // Tape: the lows, up or down
    double     hfHz;            // Tape: its low-pass
    bool       oversample;      // notes §6: the curve at twice the rate - and every layer UA_FX_LATENCY late
} tUaFx;

#define UA_FX_LATENCY    (23)   // notes §6: frames, with oversampling on, saturating or not
#define UA_FX_HALF       (24)   // the half-band filter's taps a phase

// A 2-pole state-variable filter's two integrators
typedef struct {
    double ic1, ic2;
} tUaSvf;

// One layer's effects, both channels
typedef struct {
    tUaSvf low[2][2], high[2][2];   // [channel][stage]
    tUaSvf tapeHf[2];
    double shelf[2];                // Tape Warmth's one-pole low-pass
    double dc[2][2];                // Tape's DC blocker: last in, last out
    double last[2];                 // notes §4: the shaper's previous input, for its antiderivative
    double lowHz, highHz, hfHz;     // smoothed
    double gain, output, warmth;    // smoothed, linear
    bool   started;
    // notes §6: oversampling - the base-rate inputs, the curve's two output phases, and the plain delay
    double up[2][UA_FX_HALF], even[2][UA_FX_HALF], odd[2][UA_FX_HALF];
    double delay[2][UA_FX_LATENCY];
    int    at;                      // where the newest of each ring is
    int    delayAt;
} tUaFxState;

void ua_fx_reset(tUaFxState * s);

// In place; right may be NULL (mono)
void ua_fx_process(tUaFxState * s, const tUaFx * fx, double rate, float * left, float * right, uint32_t frames);

#endif // __UA_FX_H__
