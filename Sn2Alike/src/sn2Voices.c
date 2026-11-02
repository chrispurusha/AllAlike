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

#define SMOOTH_S                 (2048.0 / 46875.0) // notes §5
#define MIDDLE_C_HZ              (261.663)
#define VELOCITY_FLOOR           (0.0157)
#define VELOCITY_RANGE           (0.98)
#define RING_GAIN                (2.0)
#define NOISE_DIFFERENCE         (0.85)
#define NOISE_FLOOR              (0.04)
#define NOISE_CORNER_HZ          (20.0)
#define NOISE_DIFFERENCE_FROM    (120.0)
#define GLIDE_AT_64              (0.183) // voices notes §12
#define GLIDE_DOUBLING           (11.2)
#define GLIDE_CURVE              (0.41)
#define GLIDE_LANDING            (2.5)
#define ENV_TO_CUTOFF            (2.2146)
#define DRIVE_IN                 (0.5) // voices notes §8
#define FILTER_IN                (0.5)
#define LFO_FADE_HOLD            (0.0125)
#define LFO_FADE_FALL            (5.7)
#define SUB_BLOCK                (32u) // notes §4

void sn2_voices_init(tSn2Voices * v) {
    memset(v, 0, sizeof(*v));
    v->limit    = 24;
    v->rate     = SN2_ENGINE_HZ;
    v->lastNote = -1;
    sn2_edge_build(v->edge);
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

static double osc_increment(const tSn2OscSound * o, double note, double bend, double wheel, double moved, double rate) {
    double semitones = note - 60.0 + (12.0 * o->octaves) + o->semitones + (o->cents / 100.0) + (bend * o->bendRange) + (wheel * o->wheelPitch) + moved;

    return 2.0 * MIDDLE_C_HZ * pow(2.0, semitones / 12.0) / rate;  // phase runs -1..1
}

// voices notes §12: seconds from one pitch to the next, whatever the interval
static double glide_seconds(double time) {
    return GLIDE_AT_64 * pow(2.0, (time - 64.0) / GLIDE_DOUBLING);
}

static double glide_heard(const tSn2Voice * vc, const tSn2Sound * sound) {
    return (sound->glideStyle == 1) ? round(vc->glide) : vc->glide;
}

static void set_pitch(tSn2Voice * vc, const tSn2Sound * sound, double rate) {
    double note = (double)vc->note + glide_heard(vc, sound);

    for (int o = 0; o < SN2_OSCS; o++) {
        vc->increment[o] = osc_increment(&sound->osc[o], note, sound->bend, sound->wheel, vc->lfoSemis[o], rate);
    }
}

// voices notes §15
static double next_random(uint32_t * seed) {
    uint32_t x = *seed;

    x    ^= x << 13;
    x    ^= x >> 17;
    x    ^= x << 5;
    *seed = x;
    return ((double)x / 2147483648.0) - 1.0;
}

static void lfo_restart(tSn2Voice * vc, const tSn2Sound * sound) {
    vc->lfoAge = 0.0;

    for (int l = 0; l < SN2_LFOS; l++) {
        vc->lfoHeld[l] = next_random(&vc->noiseSeed);

        if (sound->lfo[l].keyed) {
            vc->lfoPhase[l] = 0.0;
        }
    }
}

static double lfo_wave(int shape, double phase, double held) {
    switch (shape) {
        case 0:
            return (phase < 0.5) ? 1.0 : -1.0;

        case 1:
            return (2.0 * phase) - 1.0;

        case 2:
            return (phase < 0.25) ? (4.0 * phase) : ((phase < 0.75) ? (2.0 - (4.0 * phase)) : ((4.0 * phase) - 4.0));

        default:
            return held;
    }
}

static double lfo_fade(const tSn2LfoSound * lfo, double age) {
    if (lfo->fadeOut) {
        return (lfo->delay > 0.0) ? exp(-fmax(age - LFO_FADE_HOLD, 0.0) * LFO_FADE_FALL / lfo->delay) : 0.0;
    }
    return (lfo->delay > 0.0) ? fmin(1.0, age / lfo->delay) : 1.0;
}

// voices notes §15: every LFO moved on by one sub-block, and what it does to this voice
static void lfo_advance(tSn2Voices * v, tSn2Voice * vc, const tSn2Sound * sound, double seconds, double * cutoff, double width[SN2_OSCS]) {
    *cutoff = 0.0;

    for (int o = 0; o < SN2_OSCS; o++) {
        vc->lfoSemis[o] = 0.0;
        width[o]        = sound->osc[o].width;
    }

    for (int l = 0; l < SN2_LFOS; l++) {
        const tSn2LfoSound * lfo   = &sound->lfo[l];
        double               phase = lfo->keyed ? vc->lfoPhase[l] : v->lfoFree[l];
        double               next  = phase + (lfo->hz * seconds);
        double               fade  = lfo_fade(lfo, vc->lfoAge);

        if (next >= 1.0) {
            next          -= floor(next);
            vc->lfoHeld[l] = next_random(&vc->noiseSeed);
        }
        vc->lfoPhase[l] = next;
        double               value = (lfo_wave(lfo->shape, next, vc->lfoHeld[l]) * fade) + lfo->offset;

        *cutoff        += value * sound->lfoCutoff[l];

        for (int o = 0; o < SN2_OSCS; o++) {
            vc->lfoSemis[o] += value * sound->osc[o].lfoPitch[l];
            width[o]        += value * sound->osc[o].lfoWidth[l];
        }
    }

    vc->lfoAge += seconds;
}

// voices notes §12
static void start_glide(tSn2Voices * v, const tSn2Sound * sound, tSn2Voice * pick, uint8_t note, bool over) {
    static const double kFixed[8] = {-2.0, 2.0, -5.0, 5.0, -7.0, 7.0, -12.0, 12.0};
    bool                held      = over;

    for (uint32_t i = 0; i < SN2_MAX_VOICES; i++) {
        held = held || (v->voice[i].gate && (&v->voice[i] != pick));
    }

    // voices notes §12a: from the pitch this voice had reached, mid-glide or not; a voice never played, from the last note
    double              from      = (pick->note > 0) ? ((double)pick->note + pick->glide) : (double)v->lastNote;

    pick->glide     = 0.0;

    if (sound->glideTime > 0.0) {
        if (sound->glideType >= 2) {
            pick->glide = kFixed[(sound->glideType - 2) & 7];
        } else if ((v->lastNote >= 0) && ((sound->glideType == 0) || held)) {
            pick->glide = from - (double)note;
        }
    }
    pick->glideRate = fabs(pick->glide) / glide_seconds(sound->glideTime);
    v->lastNote     = note;
}

static void advance_glide(tSn2Voice * vc, const tSn2Sound * sound, double seconds) {
    double size = fabs(vc->glide);
    double move = vc->glideRate * seconds;

    if (sound->glideCurve == 1) {
        move = (size + GLIDE_LANDING) * seconds / (GLIDE_CURVE * glide_seconds(sound->glideTime));
    }
    vc->glide = (move >= size) ? 0.0 : ((vc->glide > 0.0) ? (vc->glide - move) : (vc->glide + move));
}

static void held_remove(tSn2Voices * v, uint8_t note) {
    for (uint32_t i = 0; i < v->heldCount; i++) {
        if (v->heldNote[i] == note) {
            memmove(&v->heldNote[i], &v->heldNote[i + 1], (v->heldCount - i - 1) * sizeof(v->heldNote[0]));
            memmove(&v->heldVelocity[i], &v->heldVelocity[i + 1], (v->heldCount - i - 1) * sizeof(v->heldVelocity[0]));
            v->heldCount--;
            return;
        }
    }
}

// voices notes §13: the one voice moves to a note; the envelopes restart unless it is legato over a held note
static void mono_play(tSn2Voices * v, const tSn2Sound * sound, uint8_t note, float velocity) {
    tSn2Voice * vc      = &v->voice[0];
    bool        over    = vc->active && vc->gate;
    bool        restart = !over || !sound->legato;

    vc->active = true;
    vc->gate   = true;
    vc->note   = note;
    start_glide(v, sound, vc, note, over);

    if (restart) {
        vc->velocity  = velocity;

        if (!sound->oscsFree) {
            memset(vc->phase, 0, sizeof(vc->phase));
        }
        vc->started   = ++v->clock;
        vc->noiseSeed = vc->noiseSeed ? vc->noiseSeed : (uint32_t)((v->clock * 2654435761u) | 1u);
        lfo_restart(vc, sound);
        sn2_env_gate(&vc->amp, true);

        for (int e = 0; e < SN2_MOD_ENVS; e++) {
            sn2_env_gate(&vc->env[e], true);
        }

        if (vc->amp.level <= 0.0) {
            sn2_filter_reset(&vc->filter);
        }
    }
    set_pitch(vc, sound, v->rate);
}

void sn2_voices_note_on(tSn2Voices * v, const tSn2Sound * sound, uint8_t note, float velocity) {
    tSn2Voice * pick = NULL;

    v->pedalled[note & 0x7F] = false;

    if (sound->mono) {
        held_remove(v, note);
        v->heldNote[v->heldCount]     = note;
        v->heldVelocity[v->heldCount] = velocity;
        v->heldCount++;
        mono_play(v, sound, note, velocity);
        return;
    }
    v->heldCount             = 0;

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

    // voices notes §8
    if (!sound->oscsFree) {
        memset(pick->phase, 0, sizeof(pick->phase));
    }
    start_glide(v, sound, pick, note, false);
    set_pitch(pick, sound, v->rate);

    pick->started   = ++v->clock;
    pick->noiseSeed = pick->noiseSeed ? pick->noiseSeed : (uint32_t)((v->clock * 2654435761u) | 1u);
    lfo_restart(pick, sound);
    sn2_env_gate(&pick->amp, true);

    for (int e = 0; e < SN2_MOD_ENVS; e++) {
        sn2_env_gate(&pick->env[e], true);
    }

    if (pick->amp.level <= 0.0) {
        sn2_filter_reset(&pick->filter);    // a voice starting from silence starts with a quiet filter
    }
}

static void release_note(tSn2Voices * v, const tSn2Sound * sound, uint8_t note);

// A key let go with the pedal down waits for the pedal
void sn2_voices_note_off(tSn2Voices * v, const tSn2Sound * sound, uint8_t note) {
    if (sound->pedal) {
        v->pedalled[note & 0x7F] = true;
        return;
    }
    release_note(v, sound, note);
}

static void release_note(tSn2Voices * v, const tSn2Sound * sound, uint8_t note) {
    if (sound->mono) {
        held_remove(v, note);

        if (v->heldCount > 0) {
            uint32_t last = v->heldCount - 1;

            if (v->voice[0].note == note) {
                mono_play(v, sound, v->heldNote[last], v->heldVelocity[last]);
            }
            return;
        }
    }

    for (uint32_t i = 0; i < SN2_MAX_VOICES; i++) {
        if (v->voice[i].active && (v->voice[i].note == note)) {
            v->voice[i].gate = false;
            sn2_env_gate(&v->voice[i].amp, false);

            for (int e = 0; e < SN2_MOD_ENVS; e++) {
                sn2_env_gate(&v->voice[i].env[e], false);
            }
        }
    }
}

void sn2_voices_all_off(tSn2Voices * v) {
    v->heldCount = 0;
    memset(v->pedalled, 0, sizeof(v->pedalled));

    for (uint32_t i = 0; i < SN2_MAX_VOICES; i++) {
        v->voice[i].active = false;
        v->voice[i].gate   = false;
        v->voice[i].amp    = (tSn2Env){
            0
        };
        sn2_filter_reset(&v->voice[i].filter);
        memset(v->voice[i].env, 0, sizeof(v->voice[i].env));
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

// voices notes §11: white noise in -1..1
static double next_white(tSn2Voice * vc) {
    uint32_t x = vc->noiseSeed;

    x            ^= x << 13;
    x            ^= x >> 17;
    x            ^= x << 5;
    vc->noiseSeed = x;
    return ((double)x / 2147483648.0) - 1.0;
}

// voices notes §11: the low-passed noise's level, measured at these Softens and joined
static double noise_gain(double soften) {
    static const double kSoften[] = {0.0, 32.0, 64.0, 96.0, 112.0, 120.0};
    static const double kGain[]   = {1.2, 1.075, 0.95, 0.75, 0.54, 0.54};

    for (int k = 1; k < 6; k++) {
        if (soften <= kSoften[k]) {
            return kGain[k - 1] + ((kGain[k] - kGain[k - 1]) * (soften - kSoften[k - 1]) / (kSoften[k] - kSoften[k - 1]));
        }
    }

    return kGain[5];
}

// voices notes §11
static double next_noise(tSn2Voice * vc, double soften, double corner) {
    double white = next_white(vc);

    if (soften >= NOISE_DIFFERENCE_FROM) {
        double out = white - vc->noiseLast;

        vc->noiseLast = white;
        return (out * NOISE_DIFFERENCE) + (white * NOISE_FLOOR);
    }
    vc->noiseLow += corner * (white - vc->noiseLow);
    return vc->noiseLow * noise_gain(soften);
}

// voices notes §10
static double velocity_level(double velocity, double amount) {
    double curve = VELOCITY_FLOOR + (VELOCITY_RANGE * velocity * velocity * velocity);

    if (amount >= 64.0) {
        double a = (amount - 64.0) / 63.0;

        return 1.0 - a + (a * curve);
    }
    return 1.0 - (((64.0 - amount) / 64.0) * curve);
}

static void render_sub_block(tSn2Voices * v, const tSn2Sound * sound, double cutoff, double res, const tSn2EnvShape * amp, const tSn2EnvShape * mods,
                             float * out, uint32_t frames, double gain) {
    double drive  = sn2_filter_drive_word(sound->overdrive);
    double corner = fmin(1.0, 2.0 * M_PI * NOISE_CORNER_HZ * pow(2.0, sound->noiseSoften / 15.0) / v->rate);

    for (uint32_t i = 0; i < SN2_MAX_VOICES; i++) {
        tSn2Voice *      vc    = &v->voice[i];

        if (!vc->active) {
            continue;
        }
        double           lfoCut;
        double           width[SN2_OSCS];

        lfo_advance(v, vc, sound, (double)frames / v->rate, &lfoCut, width);

        if (vc->glide != 0.0) {
            advance_glide(vc, sound, (double)frames / v->rate);
        }
        set_pitch(vc, sound, v->rate);
        double           stretch[SN2_OSCS];
        double           level = velocity_level(vc->velocity, sound->ampVelocity);
        tSn2FilterCoeffs coeffs;

        // voices notes §9
        double           moved = (((double)vc->note - 60.0) * sound->tracking) + (sound->wheel * sound->wheelCutoff * ENV_TO_CUTOFF) + lfoCut;

        for (int e = 0; e < SN2_MOD_ENVS; e++) {
            moved += vc->env[e].level * sound->env[e].amount * ENV_TO_CUTOFF * velocity_level(vc->velocity, sound->env[e].velocity);
        }

        sn2_filter_coeffs(sound->filterType, sound->filterSlope, cutoff + moved, fmin(127.0, fmax(0.0, res + (sound->wheel * sound->wheelResonance))), sound->normalise, &coeffs);

        double           oscLevel[SN2_OSCS];

        for (int o = 0; o < SN2_OSCS; o++) {
            stretch[o]  = sn2_osc_stretch(vc->increment[o], sound->osc[o].soften);
            oscLevel[o] = fmin(1.0, fmax(0.0, sound->osc[o].level + (sound->wheel * sound->osc[o].wheelLevel)));
        }

        for (uint32_t f = 0; f < frames; f++) {
            double mix = 0.0;

            for (int e = 0; e < SN2_MOD_ENVS; e++) {
                (void)sn2_env_next(&vc->env[e], &mods[e]);
            }

            double wave[SN2_OSCS];

            for (int o = 0; o < SN2_OSCS; o++) {
                const tSn2OscSound * os = &sound->osc[o];

                wave[o]       = sn2_osc_sample(v->edge, os->shape, vc->phase[o], stretch[o], fmin(2.0, fmax(0.0, width[o])));
                mix          += wave[o] * oscLevel[o];
                vc->phase[o] += vc->increment[o];

                if (vc->phase[o] >= 1.0) {
                    vc->phase[o] -= 2.0;
                }
            }

            mix    += RING_GAIN * wave[2] * ((wave[0] * sound->ring13) + (wave[1] * sound->ring23));
            mix    += next_noise(vc, sound->noiseSoften, corner) * sound->noiseLevel;

            double y = sn2_filter_process(&vc->filter, &coeffs, sn2_filter_drive(mix * DRIVE_IN, drive) * FILTER_IN * coeffs.gain);

            out[f] += (float)(y * sn2_env_next(&vc->amp, amp) * level * gain);
        }

        if (sn2_env_idle(&vc->amp)) {
            vc->active = false;
            vc->gate   = false;
        }
    }
}

void sn2_voices_render(tSn2Voices * v, const tSn2Sound * sound, float * out, uint32_t frames, double gain) {
    if (v->pedal && !sound->pedal) {
        for (uint32_t n = 0; n < 128; n++) {
            if (v->pedalled[n]) {
                v->pedalled[n] = false;
                release_note(v, sound, (uint8_t)n);
            }
        }
    }
    v->pedal = sound->pedal;

    smooth_toward(&v->cutoff, sound->cutoff, v->rate);
    smooth_toward(&v->resonance, sound->resonance, v->rate);
    tSn2EnvShape amp;

    sn2_env_shape(sound->ampAttack, sound->ampDecay, sound->ampSustain, sound->ampRelease, true, v->rate, &amp);
    tSn2EnvShape mods[SN2_MOD_ENVS];

    for (int e = 0; e < SN2_MOD_ENVS; e++) {
        const tSn2ModEnv * m = &sound->env[e];

        sn2_env_shape(m->attack, m->decay, m->sustain, m->release, false, v->rate, &mods[e]);
    }

    // notes §4: the coefficients every sub-block, the same for every voice until keyfollow and envelopes come
    for (uint32_t done = 0; done < frames; done += SUB_BLOCK) {
        uint32_t n      = ((frames - done) < SUB_BLOCK) ? (frames - done) : SUB_BLOCK;
        double   cutoff = smooth_advance(&v->cutoff, n);
        double   res    = smooth_advance(&v->resonance, n);

        render_sub_block(v, sound, cutoff, res, &amp, mods, out + done, n, gain);

        for (int l = 0; l < SN2_LFOS; l++) {
            v->lfoFree[l] += sound->lfo[l].hz * ((double)n / v->rate);
            v->lfoFree[l] -= floor(v->lfoFree[l]);
        }
    }
}
