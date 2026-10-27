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
        ti_filter_reset(&pick->filter2);
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
        ti_filter_reset(&v->voice[i].filter2);
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

// notes §6: the two filters' coefficients and how their outputs mix, for one sub-block
typedef struct {
    tTiFilterCoeffs f1, f2;
    tTiRouting      routing;
    double          mix;       // serial: Filter 1's share in or around Filter 2; parallel: Filter 1's level
} tTiFilterSet;

#define MIX_F1_ONLY     (0.99951171875)     // notes §6: at Balance 0 in series Filter 2 is left out
#define MIX_DRY_GAIN    (0.70703125)

// notes §6: the centre-detented 9-point curves Balance and the linked Cutoff 2 run through
static const double kBalanceCurve[9] = {-1.0, -0.6875, -0.375, -0.125, 0.0, 0.125, 0.375, 0.6875, 1.0};
static const double kOffsetCurve[9]  = {-1.0, -0.5, -0.25, -0.0625, 0.0, 0.0625, 0.25, 0.5, 1.0};

static double curve9(const double * curve, double value) {
    double v = fmin(fmax(value, 0.0), 127.0) / 128.0;

    if (v >= (126.0 / 128.0)) {
        v = (2.0 * v) - (126.0 / 128.0);    // the top two steps stretched to reach the curve's end
    }
    double p = fmin(8.0 * v, 7.9999);
    int    i = (int)p;

    return curve[i] + ((p - i) * (curve[i + 1] - curve[i]));
}

double ti_voices_cutoff2(double cutoff, double cutoff2, bool link) {
    return fmax(link ? (cutoff + (128.0 * curve9(kOffsetCurve, cutoff2))) : cutoff2, 0.0);
}

static void scale_numerator(tTiFilterCoeffs * c, double g) {
    c->b0 *= g;
    c->b1 *= g;
    c->b2 *= g;
}

static void filter_set(const tTiVoices * v, const tTiSound * sound, double cutoff, double res, double cutoff2,
                       double res2, double balance, tTiFilterSet * set) {
    double bal     = curve9(kBalanceCurve, balance);
    double c2      = ti_voices_cutoff2(cutoff, cutoff2, sound->filterLink);
    bool   serial  = (sound->routing == eTiRoutingSerial4) || (sound->routing == eTiRoutingSerial6);
    double reduced = res2;


    if (serial) {
        // notes §6: two cutoffs close together take Filter 2's resonance down
        double window = (sound->routing == eTiRoutingSerial6) ? 256.0 : 128.0;
        double apart  = 60.0 * fabs(c2 - cutoff);

        if (apart < window) {
            reduced *= 1.0 + ((3.0 * (apart - window) / 1024.0) * (res / 128.0));
        }
    }
    ti_filter_coeffs(&v->filterTables, sound->filterMode, cutoff, res, sound->drive, &set->f1);
    ti_filter_coeffs(&v->filterTables, sound->filter2Mode, c2, reduced, 0.0, &set->f2);

    // notes §6: the reduced resonance shapes the peak, but the output gain stays the unreduced one's (not in band pass)
    if ((reduced != res2) && (sound->filter2Mode != eTiFilterBandPass)) {
        scale_numerator(&set->f2, ti_filter_row_gain(sound->filter2Mode, res2) / ti_filter_row_gain(sound->filter2Mode, reduced));
    }
    set->routing = sound->routing;

    // notes §6: Filter 1's share (mix) and Filter 2's gain, each a straight line in bal on either side of the centre
    if (serial) {
        set->mix = -bal;
        scale_numerator(&set->f2, (bal < 0.0) ? (0.70703 * (1.0 + bal)) : 0.70703);
    } else if (sound->routing == eTiRoutingParallel4) {
        set->mix = 2.0 * ((bal < 0.0) ? (0.17675 - (0.07324 * bal)) : (0.17675 * (1.0 - bal)));
        scale_numerator(&set->f2, (bal < 0.0) ? (0.35352 * (1.0 + bal)) : (0.35352 + (0.14648 * bal)));
    } else {
        set->mix = (bal < 0.0) ? (0.70703 - (0.29297 * bal)) : (0.70703 * (1.0 - bal));
        scale_numerator(&set->f2, (bal < 0.0) ? (0.70703 * (1.0 + bal)) : (0.70703 + (0.29297 * bal)));
    }
}

// notes §6
static double filters_process(tTiVoice * vc, const tTiFilterSet * set, double raw) {
    double y1 = ti_filter_process(&vc->filter, &set->f1, raw);
    double m  = set->mix;

    if ((set->routing == eTiRoutingSerial4) || (set->routing == eTiRoutingSerial6)) {
        if (m >= MIX_F1_ONLY) {
            return MIX_DRY_GAIN * m * y1;
        }

        if (m > 0.0) {
            return ti_filter_process(&vc->filter2, &set->f2, y1) + (MIX_DRY_GAIN * m * y1);
        }
        return ti_filter_process(&vc->filter2, &set->f2, ((1.0 + m) * y1) - (m * raw));
    }
    return ti_filter_process(&vc->filter2, &set->f2, raw) + (m * y1);
}

static void render_sub_block(tTiVoices * v, const tTiSound * sound, const tTiFilterSet * set, float * out,
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
            double y   = filters_process(vc, set, osc);

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
    smooth_toward(&v->cutoff2, sound->cutoff2, v->rate);
    smooth_toward(&v->resonance2, sound->resonance2, v->rate);
    smooth_toward(&v->balance, sound->balance, v->rate);

    // notes §4: the coefficients every sub-block, the same for every voice until keyfollow and envelopes come
    for (uint32_t done = 0; done < frames; done += SUB_BLOCK) {
        uint32_t     n       = ((frames - done) < SUB_BLOCK) ? (frames - done) : SUB_BLOCK;
        double       cutoff  = smooth_advance(&v->cutoff, n);
        double       res     = smooth_advance(&v->resonance, n);
        double       cutoff2 = smooth_advance(&v->cutoff2, n);
        double       res2    = smooth_advance(&v->resonance2, n);
        double       balance = smooth_advance(&v->balance, n);
        tTiFilterSet set;

        filter_set(v, sound, cutoff, res, cutoff2, res2, balance, &set);
        render_sub_block(v, sound, &set, out + done, n, gain);
    }
}
