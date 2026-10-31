/*
 * Sn2Alike - the voices.
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
// Notes: Docs/code-notes/sn2Voices.md - "// notes §k" refers there.

#ifndef __TI_VOICES_H__
#define __TI_VOICES_H__

#include <stdbool.h>
#include <stdint.h>

#include "sn2Envelope.h"
#include "sn2Filter.h"
#include "sn2Osc.h"

#define SN2_MAX_VOICES    (32)

// notes §6: how the two filters are put together
typedef struct {
    bool       active;
    bool       gate;
    uint8_t    note;
    float      velocity;
    double     phase;
    double     increment;
    tSn2Env    amp;      // the amplifier envelope
    tSn2Filter filter;
    uint64_t   started;  // for stealing the oldest
} tSn2Voice;

// The sound as the voices play it, in the panel's units (notes §3)
typedef struct {
    tSn2OscShape shape;
    double       pulseWidth;                                                   // 0..1
    double       cutoff;                                                       // 0..127
    double       resonance;                                                    // 0..127
    uint32_t     filterType, filterSlope;
    double       ampAttack, ampDecay, ampSustain, ampSustainTime, ampRelease;  // 0..127 (envelope notes)
} tSn2Sound;

// notes §5: a control gliding to its new value as the instrument's Smooth On does
typedef struct {
    bool   started;
    double current, target, step;
    double left;          // samples to go
} tSn2Smooth;

typedef struct {
    tSn2Voice  voice[SN2_MAX_VOICES];
    tSn2Smooth cutoff, resonance;
    uint32_t   limit;         // 1..SN2_MAX_VOICES (design §1: the instance's voice limit)
    double     rate;
    uint64_t   clock;
} tSn2Voices;

void sn2_voices_init(tSn2Voices * v, double rate);
void sn2_voices_set_rate(tSn2Voices * v, double rate);
void sn2_voices_set_limit(tSn2Voices * v, uint32_t limit);
void sn2_voices_note_on(tSn2Voices * v, uint8_t note, float velocity);
void sn2_voices_note_off(tSn2Voices * v, uint8_t note);
void sn2_voices_all_off(tSn2Voices * v);
// Adds frames of every voice into out (mono); gain applied
void sn2_voices_render(tSn2Voices * v, const tSn2Sound * sound, float * out, uint32_t frames, double gain);

#endif // __TI_VOICES_H__
