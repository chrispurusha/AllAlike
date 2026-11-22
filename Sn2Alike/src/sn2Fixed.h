/*
 * Sn2Alike - a voice in the instrument's own words: its memory, the host side's writes, a sample at a time.
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
// Notes: Docs/code-notes/sn2Filter.md - "// notes §k" refers there.


// Notes: Docs/code-notes/sn2Fixed.md - "// notes §k" refers there.

#ifndef __SN2_FIXED_H__
#define __SN2_FIXED_H__

#include <stdbool.h>
#include <stdint.h>

#include "sn2Dsp.h"

#define SN2_FIXED_WORDS    (0xDE)   // notes §6: a voice's words from the host side, the first its flags

// notes §6: one voice in the instrument's words - its memory, its registers
typedef struct {
    uint32_t x[SN2_DSP_WORDS], y[SN2_DSP_WORDS];
    tSn2Dsp  d;
    int      updating;   // samples left with the update raised
    bool     sounding;
} tSn2FixedVoice;

// notes §6: the memory of an idle voice, built once; the edge table as the float engine has it
void sn2_fixed_template(const double * edge);
void sn2_fixed_voice_clear(tSn2FixedVoice * v);
// notes §7: a note - the host's words, then the update; and its release
void sn2_fixed_voice_start(tSn2FixedVoice * v, const uint32_t words[SN2_FIXED_WORDS]);
void sn2_fixed_voice_release(tSn2FixedVoice * v);
// one sample at the engine's rate: what the voice adds to its bus, -1..1
double sn2_fixed_voice_sample(tSn2FixedVoice * v);

// notes §8: the words for a program (a single's bytes), then the key's and velocity's on top
void sn2_fixed_program_words(const uint8_t * single, uint32_t words[SN2_FIXED_WORDS]);
void sn2_fixed_note_words(uint32_t words[SN2_FIXED_WORDS], uint8_t note, float velocity);

#endif // __SN2_FIXED_H__
