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

#include <math.h>
#include <string.h>

#include "sn2Voices.h"

#define SMOOTH_S     (2048.0 / 46875.0)    // notes §5
#define SUB_BLOCK    (32u)                 // notes §4

void sn2_voices_init(tSn2Voices * v, double rate) {
    memset(v, 0, sizeof(*v));
    v->limit = 16;
    sn2_voices_set_rate(v, rate);
}

void sn2_voices_set_rate(tSn2Voices * v, double rate) {
    v->rate = rate;
}

void sn2_voices_set_limit(tSn2Voices * v, uint32_t limit) {
    limit    = (limit < 1u) ? 1u : ((limit > SN2_MAX_VOICES) ? SN2_MAX_VOICES : limit);
    v->limit = limit;

    // notes §2: voices above a lowered limit are released, not cut
    for (uint32_t i = limit; i < SN2_MAX_VOICES; i++) {
        v->voice[i].gate = false;
        sn2_env_gate(&v->voice[i].amp, false);
    }
}

static double note_hz(uint8_t note) {
    return 440.0 * pow(2.0, ((double)note - 69.0) / 12.0);
}

void sn2_voices_note_on(tSn2Voices * v, uint8_t note, float velocity) {
    tSn2Voice * pick = NULL;

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
    sn2_env_gate(&pick->amp, true);

    if (pick->amp.level <= 0.0) {
        sn2_filter_reset(&pick->filter);    // a voice starting from silence starts with a quiet filter
    }
}

void sn2_voices_note_off(tSn2Voices * v, uint8_t note) {
    for (uint32_t i = 0; i < SN2_MAX_VOICES; i++) {
        if (v->voice[i].active && (v->voice[i].note == note)) {
            v->voice[i].gate = false;
            sn2_env_gate(&v->voice[i].amp, false);
        }
    }
}

void sn2_voices_all_off(tSn2Voices * v) {
    for (uint32_t i = 0; i < SN2_MAX_VOICES; i++) {
        v->voice[i].active = false;
        v->voice[i].gate   = false;
        v->voice[i].amp    = (tSn2Env){
            0
        };
        sn2_filter_reset(&v->voice[i].filter);
    }
}

static void smooth_toward(tSn2Smooth * s, double target, double rate) {
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

static double smooth_advance(tSn2Smooth * s, uint32_t frames) {
    if (s->left > (double)frames) {
        s->current += s->step * frames;
        s->left    -= frames;
    } else {
        s->current = s->target;
        s->left    = 0.0;
    }
    return s->current;
}

static void render_sub_block(tSn2Voices * v, const tSn2Sound * sound, const tSn2FilterCoeffs * coeffs, const tSn2EnvShape * amp,
                             float * out, uint32_t frames, double gain) {
    for (uint32_t i = 0; i < SN2_MAX_VOICES; i++) {
        tSn2Voice * vc = &v->voice[i];

        if (!vc->active) {
            continue;
        }

        for (uint32_t f = 0; f < frames; f++) {
            double osc = sn2_osc_sample(sound->shape, vc->phase, vc->increment, sound->pulseWidth);
            double y   = sn2_filter_process(&vc->filter, coeffs, osc);

            out[f]    += (float)(y * sn2_env_next(&vc->amp, amp) * gain);   // notes §7: no velocity yet
            vc->phase += vc->increment;

            if (vc->phase >= 1.0) {
                vc->phase -= 1.0;
            }
        }

        if (sn2_env_idle(&vc->amp)) {
            vc->active = false;
            vc->gate   = false;
        }
    }
}

void sn2_voices_render(tSn2Voices * v, const tSn2Sound * sound, float * out, uint32_t frames, double gain) {
    smooth_toward(&v->cutoff, sound->cutoff, v->rate);
    smooth_toward(&v->resonance, sound->resonance, v->rate);
    tSn2EnvShape amp;

    sn2_env_shape(sound->ampAttack, sound->ampDecay, sound->ampSustain, sound->ampSustainTime, sound->ampRelease, v->rate, &amp);

    // notes §4: the coefficients every sub-block, the same for every voice until keyfollow and envelopes come
    for (uint32_t done = 0; done < frames; done += SUB_BLOCK) {
        uint32_t         n      = ((frames - done) < SUB_BLOCK) ? (frames - done) : SUB_BLOCK;
        double           cutoff = smooth_advance(&v->cutoff, n);
        double           res    = smooth_advance(&v->resonance, n);
        tSn2FilterCoeffs coeffs;

        sn2_filter_coeffs(sound->filterType, sound->filterSlope, cutoff, res, v->rate, &coeffs);
        render_sub_block(v, sound, &coeffs, &amp, out + done, n, gain);
    }
}
