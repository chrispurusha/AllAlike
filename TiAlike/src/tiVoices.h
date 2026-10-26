/*
 * TiAlike - the voices.
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
// Notes: Docs/code-notes/tiVoices.md - "// notes §k" refers there.

#ifndef __TI_VOICES_H__
#define __TI_VOICES_H__

#include <stdbool.h>
#include <stdint.h>

#include "tiFilter.h"
#include "tiOsc.h"

#define TI_MAX_VOICES    (32)

typedef struct {
    bool      active;
    bool      gate;
    uint8_t   note;
    float     velocity;
    double    phase;
    double    increment;
    double    level;     // the gate envelope, 0..1
    tTiFilter filter;
    uint64_t  started;   // for stealing the oldest
} tTiVoice;

// The sound as the voices play it, in the panel's units (notes §3)
typedef struct {
    tTiOscShape   shape;
    double        pulseWidth;   // 0..1
    double        cutoff;       // 0..127
    double        resonance;    // 0..127
    tTiFilterMode filterMode;
    double        drive;        // 0..1, the Osc Volume above its centre (filter notes §5)
} tTiSound;

// notes §5: a control gliding to its new value as the instrument's Smooth On does
typedef struct {
    bool   started;
    double current, target, step;
    double left;          // samples to go
} tTiSmooth;

typedef struct {
    tTiVoice        voice[TI_MAX_VOICES];
    tTiSmooth       cutoff, resonance;
    uint32_t        limit;    // 1..TI_MAX_VOICES (design §1: the instance's voice limit)
    double          rate;
    tTiFilterTables filterTables;
    uint64_t        clock;
} tTiVoices;

void ti_voices_init(tTiVoices * v, double rate);
void ti_voices_set_rate(tTiVoices * v, double rate);
void ti_voices_set_limit(tTiVoices * v, uint32_t limit);
void ti_voices_note_on(tTiVoices * v, uint8_t note, float velocity);
void ti_voices_note_off(tTiVoices * v, uint8_t note);
void ti_voices_all_off(tTiVoices * v);
// Adds frames of every voice into out (mono); gain applied
void ti_voices_render(tTiVoices * v, const tTiSound * sound, float * out, uint32_t frames, double gain);

#endif // __TI_VOICES_H__
