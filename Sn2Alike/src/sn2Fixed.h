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

#define SN2_FIXED_BLOCK        (0xDE)
#define SN2_FIXED_WORDS        (SN2_FIXED_BLOCK + 24) // the voice's, then what the key needs of the program (notes §8)
#define SN2_FIXED_REPEATS      (SN2_FIXED_BLOCK)
#define SN2_FIXED_TRACKS       (SN2_FIXED_BLOCK + 3)
#define SN2_FIXED_DIALS        (SN2_FIXED_BLOCK + 6)
#define SN2_FIXED_GLIDE        (SN2_FIXED_BLOCK + 18)
#define SN2_FIXED_RATES        (SN2_FIXED_BLOCK + 20)
#define SN2_FIXED_OSC_WORDS    (6)
#define SN2_FIXED_LFO_WORDS    (4) // notes §6: a voice's words from the host side, the first its flags

// notes §6: one voice in the instrument's words - its memory, its registers
typedef struct {
    uint32_t x[SN2_DSP_WORDS], y[SN2_DSP_WORDS];
    tSn2Dsp  d;
    int      updating;   // samples left with the update raised
    uint32_t pitch[3];   // each oscillator's pitch word, without the bend
    bool     sounding;
} tSn2FixedVoice;

// notes §6: the memory of an idle voice, built once; the edge table as the float engine has it
void sn2_fixed_template(const double * edge);
void sn2_fixed_voice_clear(tSn2FixedVoice * v);
// notes §7: a note - the host's words, then the update; and its release
void sn2_fixed_voice_start(tSn2FixedVoice * v, const uint32_t words[SN2_FIXED_WORDS], bool freeOscs);
void sn2_fixed_voice_release(tSn2FixedVoice * v);
// notes §10: a free-running LFO - its phase words taken over from another voice (or the voice's own, before a clear)
void sn2_fixed_lfo_save(const tSn2FixedVoice * v, int lfo, uint32_t saved[SN2_FIXED_LFO_WORDS]);
void sn2_fixed_lfo_restore(tSn2FixedVoice * v, int lfo, const uint32_t saved[SN2_FIXED_LFO_WORDS]);
// notes §11: free oscillators - their phases and windows kept across a clear, and where they first stand
void sn2_fixed_osc_save(const tSn2FixedVoice * v, uint32_t saved[SN2_FIXED_OSC_WORDS]);
void sn2_fixed_osc_restore(tSn2FixedVoice * v, const uint32_t saved[SN2_FIXED_OSC_WORDS]);
void sn2_fixed_osc_seed(tSn2FixedVoice * v, const double phase[3]);
// notes §12: what moves while it sounds - the tempo, the wheel 0..1 and each oscillator's bend in semitones
void sn2_fixed_voice_live(tSn2FixedVoice * v, double wheel, const double bend[3], double tempo);
// notes §12: a legato note - the key and the glide to it, nothing restarted
void sn2_fixed_voice_move(tSn2FixedVoice * v, const uint32_t words[SN2_FIXED_WORDS]);
// one sample at the engine's rate: what the voice adds to its bus, -1..1
double sn2_fixed_voice_sample(tSn2FixedVoice * v);

// notes §8: what one step (four samples) of an envelope does at a dial's setting, as a fraction of full scale
typedef enum {
    eSn2EnvDelay = 0,
    eSn2EnvAttack,
    eSn2EnvDecay,
    eSn2EnvRelease,
    eSn2EnvTime,
    eSn2EnvRate
} tSn2EnvLaw;

// moved: the dial steps the key takes off
double sn2_fixed_env_step(tSn2EnvLaw law, double dial, int moved);
int sn2_fixed_key_moves(int note, int track, bool attack);
// notes §8: an LFO's smoothing, against its own step; 0 none
double sn2_fixed_lfo_soften(double dial);
// notes §8: an envelope's level at a velocity (0..1) and its velocity depth
double sn2_fixed_velocity(double velocity, double depth);
// notes §8: what one step of an LFO's delay does
double sn2_fixed_lfo_delay(double dial, bool fadeOut);
// notes §8: an LFO's speed locked to 120 beats a minute, by its setting (1..34)
double sn2_fixed_lfo_sync_hz(int sync);
// notes §8: the words for a program (a single's bytes), then the key's and velocity's on top
void sn2_fixed_program_words(const uint8_t * single, uint32_t words[SN2_FIXED_WORDS]);
// glide: semitones from the note that the voice starts at
void sn2_fixed_note_words(uint32_t words[SN2_FIXED_WORDS], uint8_t note, float velocity, double glide);

#endif // __SN2_FIXED_H__
