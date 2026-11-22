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

#include <math.h>
#include <string.h>

#include "sn2Fixed.h"
#include "sn2FixedCode.h"
#include "sn2Osc.h"

#define BLOCK          (0x33)    // notes §6: the voice's words
#define FLAGS_HELD     (0xFFFF40u)
#define FLAG_UPDATE    (0x1Fu)   // notes §7: everything recomputed, the phases restarted
#define FLAG_CLEAR     (0x80u)
#define STATE          (0x64)    // notes §7: the voice's envelope states
#define BUS            (0x1D)

static uint32_t gX[SN2_DSP_WORDS], gY[SN2_DSP_WORDS];
static bool     gBuilt;

static uint32_t w24(double v) {
    return (uint32_t)lround(v) & 0xFFFFFFu;
}

static void registers(tSn2Dsp * d, uint32_t * x, uint32_t * y) {
    static const uint32_t m[8] = {0xFFFF, 0x7FFF, 0x7FFF, 0xFFFF, 0xFFFF, 0xFFFF, 0x81FF, 0xFFFF};

    memset(d, 0, sizeof(*d));
    memcpy(d->m, m, sizeof(m));
    d->n[4] = 8;
    d->n[5] = 5;
    d->n[7] = 9;
    d->r[3] = 0x31;
    d->xm   = x;
    d->ym   = y;
}

static void run(tSn2Dsp * d) {
    sn2_fixed_head(d);
    d->lc = 1;   // notes §2: one voice to a memory
    sn2_fixed_pass(d);
}

// notes §6: the constants the voice processor sets itself, the tables calculated, the host's few fixed words
void sn2_fixed_template(const double * edge) {
    static const uint32_t kConstants[] = {
        0x7FFFFF, 0x555555, 0x400000, 0x040000, 0x79999A, 0x59999A, 0x400000, 0x200000, 0x000001, 0x000002, 0x002C64, 0xFF0000,
        0x00000F, 0x00000F, 0x007FFE, 0x6C7E28, 0x199990, 0x000400, 0x0081FF, 0x10624E, 0x000300, 0x000600, 0x0003EF, 0x000800, 0x000A00,
    };
    tSn2Dsp               d;

    memset(gX, 0, sizeof(gX));
    memset(gY, 0, sizeof(gY));
    memcpy(gY, kConstants, sizeof(kConstants));
    gY[0x10C] = 0x180;
    gY[0x10D] = 4;
    gY[0x128] = 0x48000;

    for (int i = 0; i < 512; i++) {   // notes §3: exponential, reciprocal
        gY[0x400 + i] = w24(4194304.0 * pow(2.0, i / 512.0));
        gY[0x600 + i] = w24(floor(2147483648.0 / (512 + i)));
    }

    for (int i = 0; i < SN2_EDGE_POINTS; i++) {   // the oscillators' edge
        gY[0x800 + i] = w24(edge[i] * 8388608.0);
    }

    gX[0x02]  = 1;
    gX[0x05]  = 0x312700;
    gX[0x30]  = 0x010000;

    for (int i = 0x0D; i <= 0x1C; i++) {
        gX[i] = 0x7FFFFF;
    }

    gY[0xC06] = 0x7FFFFE;

    registers(&d, gX, gY);
    gX[BLOCK] = FLAG_CLEAR;

    for (int s = 0; s < 64; s++) {   // cleared, then idle long enough for its slow words
        run(&d);
    }

    gBuilt    = true;
}

void sn2_fixed_voice_clear(tSn2FixedVoice * v) {
    memcpy(v->x, gX, sizeof(gX));
    memcpy(v->y, gY, sizeof(gY));
    registers(&v->d, v->x, v->y);
    v->updating = 0;
    v->sounding = false;
}

void sn2_fixed_voice_start(tSn2FixedVoice * v, const uint32_t words[SN2_FIXED_WORDS]) {
    if (!gBuilt) {
        return;
    }

    for (int i = 1; i < SN2_FIXED_WORDS; i++) {
        v->x[BLOCK + i] = words[i] & 0xFFFFFFu;
    }

    v->x[BLOCK] = FLAGS_HELD | FLAG_UPDATE;
    v->updating = 1;
    v->sounding = true;
}

// notes §10: each LFO's running words; the second LFO's are five on from the first's, its last one on
static const int kLfoWords[2][SN2_FIXED_LFO_WORDS] = {{0x86, 0x89, 0x8A, 0x9A}, {0x8B, 0x8E, 0x8F, 0x9B}};

void sn2_fixed_lfo_save(const tSn2FixedVoice * v, int lfo, uint32_t saved[SN2_FIXED_LFO_WORDS]) {
    for (int i = 0; i < SN2_FIXED_LFO_WORDS; i++) {
        saved[i] = v->y[kLfoWords[lfo][i]];
    }
}

void sn2_fixed_lfo_restore(tSn2FixedVoice * v, int lfo, const uint32_t saved[SN2_FIXED_LFO_WORDS]) {
    for (int i = 0; i < SN2_FIXED_LFO_WORDS; i++) {
        v->y[kLfoWords[lfo][i]] = saved[i];
    }
}

// notes §7: the three sustain timers run out at once
void sn2_fixed_voice_release(tSn2FixedVoice * v) {
    v->y[STATE + 0x16] = v->y[STATE + 0x18] = v->y[STATE + 0x1A] = 0x7FFFFF;
}

double sn2_fixed_voice_sample(tSn2FixedVoice * v) {
    run(&v->d);

    if (v->updating > 0 && --v->updating == 0) {
        v->x[BLOCK] = FLAGS_HELD;
    }
    return sn2_s24(v->x[BUS]) / 8388608.0;
}
