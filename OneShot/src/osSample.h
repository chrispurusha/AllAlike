/*
 * OneShot - a one-sample player, as a VST3 and Audio Unit instrument.
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
// Notes: Docs/code-notes/osSample.md - "// notes §k" refers there.

#ifndef __OS_SAMPLE_H__
#define __OS_SAMPLE_H__

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>


// A whole sample in memory, one float array per channel (mono or stereo)
typedef struct {
    uint32_t serial;                 // unique to this load: what a cached copy is keyed by, never reused
    uint32_t frames;
    uint32_t channels;               // 1 or 2
    double   rate;                   // the file's own sample rate
    float *  data[2];                // data[1] == data[0] for mono
    char     path[2048];
} tOsSample;

// notes §1: WAV (PCM 8/16/24/32, float 32/64, extensible) and AIFF/AIFC (PCM, sowt, fl32/fl64).
// NULL on failure, with why in error.
tOsSample * os_sample_load(const char * path, char * error, size_t errorLength);
void os_sample_free(tOsSample * sample);

bool os_sample_is_audio_file(const char * name);    // by extension: what the loader can read

// notes §6: the frame nearest `frame` where the sample (left and right summed) crosses zero, within radius frames;
// `frame` itself when there is none
uint32_t os_sample_zero_crossing(const tOsSample * sample, uint32_t frame, uint32_t radius);
uint32_t os_zero_crossing(const float * left, const float * right, uint32_t frames, uint32_t frame, uint32_t radius);   // right = left for mono

#endif // __OS_SAMPLE_H__
