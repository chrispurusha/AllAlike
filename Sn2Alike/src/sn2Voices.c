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
#include <stdlib.h>
#include <string.h>

#include "sn2Voices.h"

#define SMOOTH_S          (2048.0 / 46875.0)        // notes §5
#define MIDDLE_C_HZ       (261.663)
#define RING_GAIN         (2.0)
#define GLIDE_AT_64       (0.183)        // voices notes §12
#define GLIDE_DOUBLING    (11.2)
#define GLIDE_CURVE       (0.41)
#define GLIDE_LANDING     (2.5)
#define ENV_TO_CUTOFF     (98304.0 / 43691.0)
#define DRIVE_IN          (0.5)        // voices notes §8
#define FILTER_IN         (0.5)
#define SUB_BLOCK         (32u)        // notes §4

void sn2_voices_init(tSn2Voices * v) {
    memset(v, 0, sizeof(*v));
    v->limit    = 24;
    v->rate     = SN2_ENGINE_HZ;
    v->lastNote = -1;
    sn2_edge_build(v->edge);

    uint32_t seed = 1;

    for (uint32_t i = 0; i < SN2_MAX_VOICES; i++) {   // notes §8: where free oscillators first stand
        for (int o = 0; o < SN2_OSCS; o++) {
            seed                 = (seed * 1664525u) + 1013904223u;
            v->voice[i].phase[o] = ((double)seed / 2147483648.0) - 1.0;
        }
    }
}

#define FIXED_GAIN     (2.580 * 63.0 / SN2_VOLUME_UNITY) // sn2Fixed notes §9: a voice's bus word to the output level, its volume in it
#define FIXED_QUIET    (4096)                            // samples silent after its release before a fixed voice is free

// sn2Fixed notes §9: Maths FIXED - every voice gets its twin; the memory is allocated once, on first use
void sn2_voices_set_fixed(tSn2Voices * v, bool fixed) {
    if (fixed && (v->fixed == NULL)) {
        static bool built;

        if (!built) {
            sn2_fixed_template(v->edge);
            built = true;
        }
        v->fixed = calloc(SN2_MAX_VOICES, sizeof(tSn2FixedVoice));

        for (uint32_t i = 0; (v->fixed != NULL) && (i < SN2_MAX_VOICES); i++) {
            sn2_fixed_voice_clear(&v->fixed[i]);
            sn2_fixed_osc_seed(&v->fixed[i], v->voice[i].phase);   // sn2Fixed notes §11
            v->voice[i].fixed = &v->fixed[i];
        }
    }
}

void sn2_voices_free(tSn2Voices * v) {
    free(v->fixed);
    v->fixed = NULL;

    for (uint32_t i = 0; i < SN2_MAX_VOICES; i++) {
        v->voice[i].fixed = NULL;
    }
}

#define CLEAN_FACTOR      (4)                  // notes §17
#define BYPASS_GAIN       (0.249)              // notes §20: 0.771 of the open filter's level
#define FREE_LFO_START    (0.5)                // notes §15: a free LFO restarts half a cycle in when a note finds no voice sounding
#define VOICE_LEVEL       (1.0162)             // notes §10: an envelope at its top is 127/128, the gain was set without it
#define ROUTE_STEPS       (131072.0 / 66048.0) // notes §23: dial steps a full source moves Resonance or Soften for each of its amount
#define LFO_TO_LEVEL      (0.5)                // notes §19: an LFO moves a level half as far as the wheel
#define FM_DEPTH          (1.0 / 256.0)        // notes §18: the change in Osc 3's phase step, a fraction of it, per level squared per unit of modulator
#define FM_MOST           (0.5)                // notes §18: the step moves by no more than this

static double gCleanFir[SN2_CLEAN_TAPS];   // a windowed sinc, built once - the same for every instance

void sn2_voices_set_clean(tSn2Voices * v, bool clean) {
    if (gCleanFir[SN2_CLEAN_TAPS / 2] == 0.0) {
        double sum = 0.0;

        for (int n = 0; n < SN2_CLEAN_TAPS; n++) {
            double x = n - ((SN2_CLEAN_TAPS - 1) / 2.0);
            double c = 0.45 / CLEAN_FACTOR;     // cycles a sample at four times the rate: 0.45 of the engine's
            double w = 0.42 - (0.5 * cos(2.0 * M_PI * n / (SN2_CLEAN_TAPS - 1))) + (0.08 * cos(4.0 * M_PI * n / (SN2_CLEAN_TAPS - 1)));

            gCleanFir[n] = w * ((x == 0.0) ? (2.0 * c) : (sin(2.0 * M_PI * c * x) / (M_PI * x)));
            sum         += gCleanFir[n];
        }

        for (int n = 0; n < SN2_CLEAN_TAPS; n++) {
            gCleanFir[n] /= sum;
        }
    }
    v->clean = clean;
}

void sn2_voices_set_limit(tSn2Voices * v, uint32_t limit) {
    limit    = (limit < 1u) ? 1u : ((limit > SN2_MAX_VOICES) ? SN2_MAX_VOICES : limit);
    v->limit = limit;

    // notes §2: voices above a lowered limit are released, not cut
    for (uint32_t i = limit; i < SN2_MAX_VOICES; i++) {
        v->voice[i].gate = false;
        sn2_env_release(&v->voice[i].amp);
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
    memset(vc->lfoValue, 0, sizeof(vc->lfoValue));

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
    double steps = age * SN2_ENGINE_HZ / SN2_ENV_TICK;

    return lfo->fadeOut ? pow(fmax(0.0, 1.0 + lfo->fade), steps) : fmin(1.0, steps * lfo->fade);
}

// voices notes §15: every LFO moved on by one sub-block, and what it does to this voice
static void lfo_advance(tSn2Voices * v, tSn2Voice * vc, const tSn2Sound * sound, double seconds, double * cutoff, double width[SN2_OSCS]) {
    *cutoff = 0.0;

    for (int o = 0; o < SN2_OSCS; o++) {
        vc->lfoSemis[o] = 0.0;
        vc->syncMod[o]  = 0.0;
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
        double               value = (lfo_wave(lfo->shape, next, vc->lfoHeld[l]) + lfo->offset) * fade;   // notes §15: the offset fades in too

        if (lfo->soften > 0.0) {   // notes §15: smoothed, the faster the LFO the less
            double steps = seconds * SN2_ENGINE_HZ / SN2_ENV_TICK;
            double part  = fmin(1.0, 256.0 * lfo->soften * lfo->hz * 2.0 * SN2_ENV_TICK / SN2_ENGINE_HZ);

            value = vc->lfoValue[l] + ((value - vc->lfoValue[l]) * (1.0 - pow(1.0 - part, steps)));
        }
        vc->lfoValue[l] = value;

        *cutoff        += value * sound->lfoCutoff[l];

        for (int o = 0; o < SN2_OSCS; o++) {
            vc->lfoSemis[o] += value * sound->osc[o].lfoPitch[l];
            vc->syncMod[o]  += LFO_TO_LEVEL * value * sound->osc[o].syncLfo[l];   // sn2Osc notes §5: an LFO counts half, as on the levels
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

// sn2Fixed notes §9: the voice's twin begins the note, where Maths is FIXED
static void start_fixed(tSn2Voices * v, const tSn2Sound * sound, tSn2Voice * pick, uint8_t note, float velocity) {
    if (!sound->fixedPoint || (pick->fixed == NULL)) {
        return;
    }
    uint32_t    words[SN2_FIXED_WORDS];

    uint32_t    carried[SN2_LFOS][SN2_FIXED_LFO_WORDS];
    uint32_t    oscs[SN2_FIXED_OSC_WORDS];
    tSn2Voice * from = pick;   // sn2Fixed notes §10: a free LFO runs on from the voice started last

    for (uint32_t i = 0; i < SN2_MAX_VOICES; i++) {
        tSn2Voice * o = &v->voice[i];

        if ((o != pick) && (o->fixed != NULL) && ((from == pick) || (o->started > from->started))) {
            from = o;
        }
    }

    for (int l = 0; l < SN2_LFOS; l++) {
        sn2_fixed_lfo_save(from->fixed, l, carried[l]);
    }

    memcpy(words, sound->fixedWords, sizeof(words));
    sn2_fixed_note_words(words, note, velocity, pick->glide);
    sn2_fixed_osc_save(pick->fixed, oscs);
    sn2_fixed_voice_clear(pick->fixed);
    sn2_fixed_voice_start(pick->fixed, words, sound->oscsFree);

    if (sound->oscsFree) {
        sn2_fixed_osc_restore(pick->fixed, oscs);   // sn2Fixed notes §11
    }

    for (int l = 0; l < SN2_LFOS; l++) {
        if (!sound->lfo[l].keyed) {
            sn2_fixed_lfo_restore(pick->fixed, l, carried[l]);
        }
    }

    pick->quiet = 0;
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
        vc->velocity   = velocity;

        if (!sound->oscsFree) {
            memset(vc->phase, 0, sizeof(vc->phase));
            memset(vc->cleanHist, 0, sizeof(vc->cleanHist));   // notes §17: a restarted wave starts its filter clean
        }
        vc->started    = ++v->clock;
        vc->noiseSeed  = vc->noiseSeed ? vc->noiseSeed : (uint32_t)((v->clock * 2654435761u) | 1u);
        vc->noiseCount = vc->noiseCount ? vc->noiseCount : (uint32_t)(0x41u + (v->clock * 7919u));
        start_fixed(v, sound, vc, note, velocity);
        lfo_restart(vc, sound);
        sn2_env_start(&vc->amp, sound->envRepeats[0]);

        for (int e = 0; e < SN2_MOD_ENVS; e++) {
            sn2_env_start(&vc->env[e], sound->envRepeats[1 + e]);
        }

        if (vc->amp.level <= 0.0) {
            sn2_filter_reset(&vc->filter);
        }
    } else if (sound->fixedPoint && (vc->fixed != NULL)) {   // sn2Fixed notes §12: legato
        uint32_t words[SN2_FIXED_WORDS];

        memcpy(words, sound->fixedWords, sizeof(words));
        sn2_fixed_note_words(words, note, (float)vc->velocity, vc->glide);
        sn2_fixed_voice_move(vc->fixed, words);
    }
    set_pitch(vc, sound, v->rate);
}

// A key let go: the envelopes release, except a one-shot's (sn2Envelope notes §9)
static void let_go(const tSn2Sound * sound, tSn2Voice * voice) {
    voice->gate = false;

    if ((voice->fixed != NULL) && voice->fixed->sounding && !sound->oneShot) {
        sn2_fixed_voice_release(voice->fixed);
    }

    if (!sound->oneShot) {
        sn2_env_release(&voice->amp);

        for (int e = 0; e < SN2_MOD_ENVS; e++) {
            sn2_env_release(&voice->env[e]);
        }
    }
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

    // notes §21: Piano restarts the key's own voice, Stack lets it go and takes another; else a free voice; else the oldest
    for (uint32_t i = 0; i < v->limit; i++) {
        if (v->voice[i].active && (v->voice[i].note == note)) {
            if (sound->piano && (pick == NULL)) {
                pick = &v->voice[i];
            } else if (v->voice[i].gate) {
                let_go(sound, &v->voice[i]);
            }
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
    pick->active     = true;
    pick->gate       = true;
    pick->note       = note;
    pick->velocity   = velocity;

    for (int o = 0; o < SN2_OSCS; o++) {
        pick->window[o] = 1.0;   // sn2Osc notes §7
    }

    // voices notes §8
    if (!sound->oscsFree) {
        memset(pick->phase, 0, sizeof(pick->phase));
        memset(pick->cleanHist, 0, sizeof(pick->cleanHist));   // notes §17: a restarted wave starts its filter clean
    }
    start_glide(v, sound, pick, note, false);
    set_pitch(pick, sound, v->rate);

    pick->started    = ++v->clock;

    start_fixed(v, sound, pick, note, velocity);
    pick->noiseSeed  = pick->noiseSeed ? pick->noiseSeed : (uint32_t)((v->clock * 2654435761u) | 1u);
    pick->noiseCount = pick->noiseCount ? pick->noiseCount : (uint32_t)(0x41u + (v->clock * 7919u));
    lfo_restart(pick, sound);
    sn2_env_start(&pick->amp, sound->envRepeats[0]);

    for (int e = 0; e < SN2_MOD_ENVS; e++) {
        sn2_env_start(&pick->env[e], sound->envRepeats[1 + e]);
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
            let_go(sound, &v->voice[i]);
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

// voices notes §11: one pole of low pass, less its corner's part of what it held
static double next_noise(tSn2Voice * vc, double corner) {
    double was = vc->noiseLow;

    vc->noise     = ((2 * vc->noiseCount * vc->noise) + vc->noiseCount + 1) & 0xFFFFFFu;   // the instrument's own, not quite white
    vc->noiseCount++;
    vc->noiseLow += corner * (((vc->noise >= 0x800000u) ? ((double)vc->noise - 16777216.0) : (double)vc->noise) / 8388608.0 - was);
    vc->noise     = (vc->noise * 0xAAAAAAu) & 0xFFFFFFu;
    return vc->noiseLow - (corner * was);
}

// voices notes §10
static double velocity_level(double velocity, double amount) {
    return sn2_fixed_velocity(velocity, amount);   // notes §10
}

// sn2Osc notes §5: one sample of an oscillator, free or synced
static double osc_sample(const double * edge, tSn2OscShape shape, double phase, double ratio, double stretch, double width) {
    return (ratio > 0.0) ? sn2_osc_sample_synced(edge, shape, phase, ratio, stretch, width) : sn2_osc_sample(edge, shape, phase, stretch, width);
}

// sn2Osc notes §5, §6: the slave's ratio and its edge, with Skew from where the oscillator is in its cycle
static double sync_now(const tSn2OscSound * os, double soften, double sync, double phase, double increment, double * stretch) {
    sync     = fmax(0.0, sync - (24.0 * os->skew * phase));
    *stretch = sn2_osc_stretch(increment * pow(2.0, sync / 24.0), soften);   // past the cap too
    return sn2_sync_ratio(sync);
}

// The three envelopes' steps, for a key (envelope notes §5)
static void env_shapes(const tSn2Sound * sound, int note, tSn2EnvShape * amp, tSn2EnvShape mods[SN2_MOD_ENVS]) {
    const tSn2EnvDials dials = {0.0, sound->ampAttack, sound->ampDecay, sound->ampSustain, sound->ampRelease, sound->ampSustainTime, sound->ampSustainRate, sound->envTrack[0]};

    sn2_env_shape(&dials, true, sound->oneShot, note, amp);

    for (int e = 0; e < SN2_MOD_ENVS; e++) {
        const tSn2ModEnv * m = &sound->env[e];
        const tSn2EnvDials d = {m->delay, m->attack, m->decay, m->sustain, m->release, m->sustainTime, m->sustainRate, sound->envTrack[1 + e]};

        sn2_env_shape(&d, false, sound->oneShot, note, &mods[e]);
    }
}

static void render_sub_block(tSn2Voices * v, const tSn2Sound * sound, double cutoff, double res, const tSn2EnvShape * amp, const tSn2EnvShape * mods,
                             float * out, uint32_t frames, double gain) {
    double drive   = sn2_filter_drive_word(sound->overdrive);
    bool   tracked = (sound->envTrack[0] != 64) || (sound->envTrack[1] != 64) || (sound->envTrack[2] != 64);
    double octave  = floor(sound->noiseSoften / 16.0);
    double corner  = fmin(1.0, pow(2.0, octave) * (1.0 + (sound->noiseSoften / 16.0) - octave) / 256.0);  // notes §11

    for (uint32_t i = 0; i < SN2_MAX_VOICES; i++) {
        tSn2Voice * vc = &v->voice[i];

        if (!vc->active) {
            continue;
        }

        if (sound->fixedPoint && (vc->fixed != NULL) && vc->fixed->sounding) {   // sn2Fixed notes §9: the voice in the instrument's own words
            double bend[SN2_OSCS];

            for (int o = 0; o < SN2_OSCS; o++) {
                bend[o] = sound->bend * sound->osc[o].bendRange;
            }

            sn2_fixed_voice_live(vc->fixed, sound->wheel, bend, (sound->tempo > 0.0) ? sound->tempo : 120.0);

            for (uint32_t f = 0; f < frames; f++) {
                double y = sn2_fixed_voice_sample(vc->fixed) * FIXED_GAIN;

                out[f]   += (float)y;   // sn2Fixed notes §9: the program's volume is already in the voice's words
                vc->quiet = (vc->gate || (fabs(y) > 1e-6)) ? 0 : (vc->quiet + 1);
            }

            if (vc->quiet >= FIXED_QUIET) {
                vc->active          = false;
                vc->fixed->sounding = false;
            }
            continue;
        }
        tSn2EnvShape         ownAmp, ownMods[SN2_MOD_ENVS];
        const tSn2EnvShape * ampShape = amp;
        const tSn2EnvShape * modShape = mods;

        if (tracked) {   // envelope notes §5
            env_shapes(sound, vc->note, &ownAmp, ownMods);
            ampShape = &ownAmp;
            modShape = ownMods;
        }
        double               lfoCut;
        double               width[SN2_OSCS];

        lfo_advance(v, vc, sound, (double)frames / v->rate, &lfoCut, width);

        for (int o = 0; o < SN2_OSCS; o++) {   // notes §22: Env 2 and Env 3 to pitch, as the LFOs
            for (int e = 0; e < SN2_MOD_ENVS; e++) {
                vc->lfoSemis[o] += vc->env[e].level * velocity_level(vc->velocity, sound->env[e].velocity) * sound->osc[o].envPitch[e];
            }
        }

        if (vc->glide != 0.0) {
            advance_glide(vc, sound, (double)frames / v->rate);
        }
        set_pitch(vc, sound, v->rate);
        double               stretch[SN2_OSCS];
        const double         source[SN2_LEVEL_ROUTES] = {sound->wheel, vc->env[0].level * velocity_level(vc->velocity, sound->env[0].velocity), vc->env[1].level * velocity_level(vc->velocity, sound->env[1].velocity), LFO_TO_LEVEL * vc->lfoValue[0], LFO_TO_LEVEL * vc->lfoValue[1]}; // what the routes follow

        for (int o = 0; o < SN2_OSCS; o++) {   // notes §23: the wheel, Env 2 and Env 3 to pulse width, twice as far as the LFOs
            for (int r = 0; r < 3; r++) {
                width[o] += source[r] * sound->osc[o].widthRoute[r];
            }
        }

        double               level                    = velocity_level(vc->velocity, sound->ampVelocity);
        tSn2FilterCoeffs     coeffs;

        // voices notes §9
        double               moved                    = (((double)vc->note - 60.0) * sound->tracking) + (sound->wheel * sound->wheelCutoff * ENV_TO_CUTOFF) + lfoCut;

        for (int e = 0; e < SN2_MOD_ENVS; e++) {
            moved += source[1 + e] * sound->env[e].amount * ENV_TO_CUTOFF;
        }

        double               resonance                = sound->wheel * sound->wheelResonance; // notes §23

        for (int r = 0; r < 4; r++) {
            resonance += source[1 + r] * sound->resRoute[r];
        }

        sn2_filter_coeffs(sound->filterType, sound->filterSlope, cutoff + moved, fmin(127.0, fmax(0.0, res + (resonance * ROUTE_STEPS))), sound->normalise, sound->filterWidth, &coeffs);

        // notes §19: each mixer level moved by the wheel, Env 2, Env 3 and the LFOs
        const double         base[SN2_LEVEL_SLOTS]    = {sound->osc[0].level, sound->osc[1].level, sound->osc[2].level, sound->ring13, sound->ring23, sound->noiseLevel};
        double               slot[SN2_LEVEL_SLOTS];

        for (int s = 0; s < SN2_LEVEL_SLOTS; s++) {
            slot[s] = base[s];

            for (int r = 0; r < SN2_LEVEL_ROUTES; r++) {
                slot[s] += source[r] * sound->levelRoute[s][r];
            }

            slot[s] = fmin(1.0, fmax(0.0, slot[s]));
        }

        double               oscLevel[SN2_OSCS];
        double               ratio[SN2_OSCS];  // sn2Osc notes §5: the sync slave's, 0 free
        double               syncAt[SN2_OSCS]; // its Sync, before Skew
        double               soften[SN2_OSCS];

        for (int o = 0; o < SN2_OSCS; o++) {
            const tSn2OscSound * os = &sound->osc[o];
            bool                 on = (os->sync > 0.0) || (os->syncLfo[0] != 0.0) || (os->syncLfo[1] != 0.0) || (os->syncEnv2 != 0.0) || (os->syncEnv3 != 0.0)
                                      || (os->syncWheel != 0.0) || (os->skew != 0.0) || (os->keySync != 0.0);

            syncAt[o]   = os->sync + vc->syncMod[o] + (source[1] * os->syncEnv2) + (source[2] * os->syncEnv3) + (sound->wheel * os->syncWheel) - (os->keySync * ((double)vc->note + glide_heard(vc, sound) - 60.0));   // sn2Osc notes §5, §8
            soften[o]   = os->soften;

            for (int r = 0; r < SN2_LEVEL_ROUTES; r++) {   // notes §23
                soften[o] += source[r] * os->softenRoute[r] * ROUTE_STEPS;
            }

            soften[o]   = fmin(127.0, fmax(0.0, soften[o]));
            stretch[o]  = sn2_osc_stretch(vc->increment[o], soften[o]);
            ratio[o]    = on ? sync_now(os, soften[o], syncAt[o], vc->phase[o], vc->increment[o], &stretch[o]) : 0.0;
            oscLevel[o] = slot[o];
        }

        double               ring13     = slot[3];
        double               ring23     = slot[4];
        double               noiseLevel = slot[5];

        // notes §18: FM depth from each slot's level, squared; a slot set to FM is not heard as ring or noise
        double               fmD13      = sound->fm13 ? (FM_DEPTH * pow(ring13 * 128.0, 2.0)) : 0.0;
        double               fmD23      = sound->fm23 ? (FM_DEPTH * pow(ring23 * 128.0, 2.0)) : 0.0;
        double               fmDN       = sound->fmNoise ? (FM_DEPTH * pow(noiseLevel * 128.0, 2.0)) : 0.0;
        bool                 fm         = (fmD13 != 0.0) || (fmD23 != 0.0) || (fmDN != 0.0);

        for (uint32_t f = 0; f < frames; f++) {
            double mix      = 0.0;
            double noiseFm  = vc->noiseLow;   // notes §18: before this sample's
            double noiseNow = next_noise(vc, corner);

            for (int e = 0; e < SN2_MOD_ENVS; e++) {
                (void)sn2_env_next(&vc->env[e], &modShape[e]);
            }

            double wave[SN2_OSCS];

            for (int o = 0; o < SN2_OSCS; o++) {
                const tSn2OscSound * os  = &sound->osc[o];

                double               wid = fmin(2.0, fmax(0.0, width[o]));

                bool                 fm3 = fm && (o == 2);   // notes §18: Osc 3 is the one modulated

                if ((os->skew != 0.0) && ((f & 3) == 0)) {   // sn2Osc notes §6: as often as the instrument's slow words
                    ratio[o] = sync_now(os, soften[o], syncAt[o], vc->phase[o], vc->increment[o], &stretch[o]);
                }

                if (v->clean) {   // notes §17: the same law at four times the rate, then the low-pass back down
                    double * h  = vc->cleanHist[o];

                    for (int k = 0; k < CLEAN_FACTOR; k++) {
                        h[(vc->cleanAt + k) % SN2_CLEAN_TAPS] = osc_sample(v->edge, os->shape, vc->phase[o], ratio[o], stretch[o], wid) * vc->window[o];
                        double sub = vc->increment[o];

                        if (fm3) {   // Osc 1's and Osc 2's own sub-steps, not their filtered output
                            uint32_t at = (vc->cleanAt + k) % SN2_CLEAN_TAPS;

                            sub += fmin(fmax(sub * ((fmD13 * vc->cleanHist[0][at]) + (fmD23 * vc->cleanHist[1][at]) + (fmDN * noiseFm)), -FM_MOST), FM_MOST);
                        }
                        vc->phase[o]                         += sub / CLEAN_FACTOR;
                        vc->window[o]                        -= vc->window[o] * os->windowFall / CLEAN_FACTOR;

                        if ((vc->phase[o] >= 1.0) || (vc->phase[o] < -1.0)) {   // a step through zero runs backwards
                            vc->phase[o] += (vc->phase[o] >= 1.0) ? -2.0 : 2.0;
                            vc->window[o] = 1.0;
                        }
                    }

                    double   y  = 0.0;
                    uint32_t at = (vc->cleanAt + CLEAN_FACTOR) % SN2_CLEAN_TAPS;   // the oldest sample

                    for (int n = 0; n < SN2_CLEAN_TAPS; n++) {
                        y += gCleanFir[n] * h[(at + n) % SN2_CLEAN_TAPS];
                    }

                    wave[o] = y;
                } else {
                    wave[o]        = osc_sample(v->edge, os->shape, vc->phase[o], ratio[o], stretch[o], wid) * vc->window[o];
                    vc->window[o] -= vc->window[o] * os->windowFall;
                    double inc = vc->increment[o];

                    if (fm3) {   // through zero, within a cycle a sample
                        inc += fmin(fmax(inc * ((fmD13 * wave[0]) + (fmD23 * wave[1]) + (fmDN * noiseFm)), -FM_MOST), FM_MOST);
                    }
                    vc->phase[o]  += inc;

                    if ((vc->phase[o] >= 1.0) || (vc->phase[o] < -1.0)) {
                        vc->phase[o] += (vc->phase[o] >= 1.0) ? -2.0 : 2.0;
                        vc->window[o] = 1.0;
                    }
                }
                mix += wave[o] * oscLevel[o];
            }

            double around = 0.0;   // notes §20: Filter OSCs Bypass
            double noise  = sound->fmNoise ? 0.0 : (noiseNow * noiseLevel);

            vc->cleanAt = (vc->cleanAt + CLEAN_FACTOR) % SN2_CLEAN_TAPS;
            mix        += RING_GAIN * wave[2] * ((sound->fm13 ? 0.0 : (wave[0] * ring13)) + (sound->fm23 ? 0.0 : (wave[1] * ring23)));

            if (sound->oscsAround) {
                around = mix;
                mix    = 2.0 * noise;
            } else {
                mix += noise;
            }
            double y      = sn2_filter_process(&vc->filter, &coeffs, sn2_filter_drive(mix * DRIVE_IN, drive) * FILTER_IN * coeffs.gain) + (around * BYPASS_GAIN);

            out[f]     += (float)(y * sn2_env_next(&vc->amp, ampShape) * level * gain * VOICE_LEVEL);
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
    tSn2EnvShape amp, mods[SN2_MOD_ENVS];

    env_shapes(sound, 60, &amp, mods);

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
