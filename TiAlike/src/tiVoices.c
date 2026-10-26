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

#include <math.h>
#include <string.h>

#include "tiVoices.h"

// notes §1: a placeholder voice until the engine's own sections replace it
#define ATTACK_S     (0.002)
#define RELEASE_S    (0.120)
#define SMOOTH_S     (2048.0 / 46875.0)    // notes §5
#define SUB_BLOCK    (32u)                 // notes §4

void ti_voices_init(tTiVoices * v, double rate) {
    memset(v, 0, sizeof(*v));
    v->limit = 16;
    ti_voices_set_rate(v, rate);
}

void ti_voices_set_rate(tTiVoices * v, double rate) {
    v->rate = rate;
    ti_filter_tables(&v->filterTables, rate);
}

void ti_voices_set_limit(tTiVoices * v, uint32_t limit) {
    limit    = (limit < 1u) ? 1u : ((limit > TI_MAX_VOICES) ? TI_MAX_VOICES : limit);
    v->limit = limit;

    // notes §2: voices above a lowered limit are released, not cut
    for (uint32_t i = limit; i < TI_MAX_VOICES; i++) {
        v->voice[i].gate = false;
    }
}

static double note_hz(uint8_t note) {
    return 440.0 * pow(2.0, ((double)note - 69.0) / 12.0);
}

void ti_voices_note_on(tTiVoices * v, uint8_t note, float velocity) {
    tTiVoice * pick = NULL;

    // the same key again restarts its voice; else a free voice; else the oldest
    for (uint32_t i = 0; (i < v->limit) && (pick == NULL); i++) {
        if (v->voice[i].active && (v->voice[i].note == note)) {
            pick = &v->voice[i];
        }
    }

    for (uint32_t i = 0; (i < v->limit) && (pick == NULL); i++) {
        if (!v->voice[i].active) {
            pick = &v->voice[i];
        }
    }

    if (pick == NULL) {
        pick = &v->voice[0];

        for (uint32_t i = 1; i < v->limit; i++) {
            if (v->voice[i].started < pick->started) {
                pick = &v->voice[i];
            }
        }
    }
    pick->active    = true;
    pick->gate      = true;
    pick->note      = note;
    pick->velocity  = velocity;
    pick->increment = note_hz(note) / v->rate;
    pick->started   = ++v->clock;

    if (pick->level <= 0.0) {
        ti_filter_reset(&pick->filter);    // a voice starting from silence starts with a quiet filter
    }
}

void ti_voices_note_off(tTiVoices * v, uint8_t note) {
    for (uint32_t i = 0; i < TI_MAX_VOICES; i++) {
        if (v->voice[i].active && (v->voice[i].note == note)) {
            v->voice[i].gate = false;
        }
    }
}

void ti_voices_all_off(tTiVoices * v) {
    for (uint32_t i = 0; i < TI_MAX_VOICES; i++) {
        v->voice[i].active = false;
        v->voice[i].gate   = false;
        v->voice[i].level  = 0.0;
        ti_filter_reset(&v->voice[i].filter);
    }
}

static void smooth_toward(tTiSmooth * s, double target, double rate) {
    if (!s->started) {
        s->started = true;
        s->current = s->target = target;
        s->left    = 0.0;
    } else if (target != s->target) {
        s->target = target;
        s->left   = SMOOTH_S * rate;
        s->step   = (target - s->current) / s->left;
    }
}

static double smooth_advance(tTiSmooth * s, uint32_t frames) {
    if (s->left > (double)frames) {
        s->current += s->step * frames;
        s->left    -= frames;
    } else {
        s->current = s->target;
        s->left    = 0.0;
    }
    return s->current;
}

static void render_sub_block(tTiVoices * v, const tTiSound * sound, const tTiFilterCoeffs * coeffs, float * out,
                             uint32_t frames, double gain) {
    double up   = 1.0 / (ATTACK_S * v->rate);
    double down = 1.0 / (RELEASE_S * v->rate);

    for (uint32_t i = 0; i < TI_MAX_VOICES; i++) {
        tTiVoice * vc = &v->voice[i];

        if (!vc->active) {
            continue;
        }

        for (uint32_t f = 0; f < frames; f++) {
            double osc = ti_osc_sample(sound->shape, vc->phase, vc->increment, sound->pulseWidth);
            double y   = ti_filter_process(&vc->filter, coeffs, osc);

            vc->level  = vc->gate ? fmin(1.0, vc->level + up) : fmax(0.0, vc->level - down);
            out[f]    += (float)(y * vc->level * vc->velocity * gain);
            vc->phase += vc->increment;

            if (vc->phase >= 1.0) {
                vc->phase -= 1.0;
            }
        }

        if (!vc->gate && (vc->level <= 0.0)) {
            vc->active = false;
        }
    }
}

void ti_voices_render(tTiVoices * v, const tTiSound * sound, float * out, uint32_t frames, double gain) {
    smooth_toward(&v->cutoff, sound->cutoff, v->rate);
    smooth_toward(&v->resonance, sound->resonance, v->rate);

    // notes §4: the coefficients every sub-block, the same for every voice until keyfollow and envelopes come
    for (uint32_t done = 0; done < frames; done += SUB_BLOCK) {
        uint32_t        n = ((frames - done) < SUB_BLOCK) ? (frames - done) : SUB_BLOCK;
        tTiFilterCoeffs coeffs;

        ti_filter_coeffs(&v->filterTables, sound->filterMode, smooth_advance(&v->cutoff, n),
                         smooth_advance(&v->resonance, n), sound->drive, &coeffs);
        render_sub_block(v, sound, &coeffs, out + done, n, gain);
    }
}
