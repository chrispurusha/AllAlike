/*
 * UltraAlike - a preset's voices: every zone a note reaches, from an E-mu bank.
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
// Notes: Docs/code-notes/uaBankVoice.md - "// notes §k" refers there.

#include <math.h>
#include <string.h>

#include "uaBankVoice.h"
#include "uaResample.h"

#define OUTPUT_SMOOTH_S    (0.005)
#define OUTPUT_CHUNK       (64)
#define STEAL_FADE_S       (0.003)
#define GATHER_HALF        ((int64_t)(UA_RESAMPLE_HALF * 4) + 2)   // the widest kernel's reach, 1 / its narrowest cutoff
#define GATHER_SIZE        ((2 * GATHER_HALF) + 1)

void ua_bank_voices_init(tUaBankVoices * v, double hostRate) {
    memset(v, 0, sizeof(*v));
    v->hostRate = hostRate;
}

void ua_bank_voices_stop(tUaBankVoices * v) {
    for (int i = 0; i < UA_BANK_VOICES; i++) {
        v->voice[i].active = false;
    }
}

static void fade_out(tUaBankVoices * v, tUaBankVoice * voice) {
    double step = 1.0 / fmax(1.0, STEAL_FADE_S * v->hostRate);

    if ((voice->fadeStep == 0.0) || (step > voice->fadeStep)) {
        voice->fadeStep = step;
    }
}

void ua_bank_voices_fade_all(tUaBankVoices * v) {
    for (int i = 0; i < UA_BANK_VOICES; i++) {
        if (v->voice[i].active) {
            fade_out(v, &v->voice[i]);
        }
    }
}

// A free voice, else the oldest, faded out under the new note
static tUaBankVoice * take_voice(tUaBankVoices * v) {
    tUaBankVoice * oldest = &v->voice[0];

    for (int i = 0; i < UA_BANK_VOICES; i++) {
        if (!v->voice[i].active) {
            return &v->voice[i];
        }

        if ((int32_t)(v->voice[i].age - oldest->age) < 0) {
            oldest = &v->voice[i];
        }
    }

    return oldest;
}

void ua_bank_voices_note_on(tUaBankVoices * v, const tEmuPreset * preset, const tUaPlay * play, uint8_t note, float velocity) {
    int vel = (int)lroundf(velocity * 127.0f);

    for (uint32_t z = 0; (preset != NULL) && (z < preset->zoneCount); z++) {
        const tEmuZone * zone  = &preset->zone[z];

        if ((note < zone->keyLow) || (note > zone->keyHigh) || (vel < zone->velLow) || (vel > zone->velHigh)) {
            continue;
        }
        tUaBankVoice *   voice = take_voice(v);
        double           step  = (zone->sample->rate / v->hostRate) * pow(2.0, ((double)note - zone->rootNote + play->tune) / 12.0);

        *voice = (tUaBankVoice){
            .preset = preset,
            .sample = zone->sample,
            .step   = step,
            .cutoff = ua_resample_cutoff(step),
            .level  = (1.0 - play->velocity) + (play->velocity * velocity),
            .fade   = 1.0,
            .note   = note,
            .active = true,
            .age    = v->started++,
        };
        ua_env_start(&voice->env);
    }
}

// notes §1: every note off releases, whatever the layer's Mode
void ua_bank_voices_note_off(tUaBankVoices * v, uint8_t note) {
    for (int i = 0; i < UA_BANK_VOICES; i++) {
        if (v->voice[i].active && (v->voice[i].note == note)) {
            ua_env_release(&v->voice[i].env);
        }
    }
}

// notes §2: a frame of the sample as it sounds - past the loop's end, round the loop again
static int64_t looped_index(const tEmuSample * s, bool looping, int64_t i) {
    int64_t start = s->loopStart;
    int64_t span  = (int64_t)s->loopEnd - start;

    if (!s->looped || ((i < (int64_t)s->loopEnd) && (!looping || (i >= start)))) {
        return i;
    }
    int64_t k     = (i - start) % span;

    return start + ((k < 0) ? (k + span) : k);
}

static void read_frame(const tUaBankVoice * voice, float * l, float * r) {
    const tEmuSample * s     = voice->sample;
    const float *      right = (s->channels == 2) ? s->data[1] : NULL;
    int64_t            reach = (int64_t)ceil(ua_resample_tail(voice->cutoff)) + 2;
    int64_t            at    = (int64_t)floor(voice->position);
    bool               seam  = s->looped && (((at + reach) >= (int64_t)s->loopEnd) || (voice->looping && ((at - reach) < (int64_t)s->loopStart)));

    if (!seam) {
        ua_resample_read2(s->data[0], right, s->frames, voice->position, voice->cutoff, l, r);
        return;
    }
    // notes §2: the frames around the seam gathered in the order they sound, and read from there
    float              gl[GATHER_SIZE], gr[GATHER_SIZE];
    int64_t            from  = at - GATHER_HALF;

    for (int64_t k = 0; k < GATHER_SIZE; k++) {
        int64_t i  = looped_index(s, voice->looping, from + k);
        bool    in = (i >= 0) && (i < (int64_t)s->frames);

        gl[k] = in ? s->data[0][i] : 0.0f;
        gr[k] = (in && (right != NULL)) ? right[i] : gl[k];
    }

    ua_resample_read2(gl, (right != NULL) ? gr : NULL, GATHER_SIZE, voice->position - (double)from, voice->cutoff, l, r);
}

void ua_bank_voices_render(tUaBankVoices * v, const tUaPlay * play, float * left, float * right, uint32_t frames) {
    double angle   = (play->pan + 1.0) * 0.25 * M_PI;
    double targetL = cos(angle) * M_SQRT2 * play->gain;
    double targetR = sin(angle) * M_SQRT2 * play->gain;
    double dt      = 1.0 / v->hostRate;
    double k       = 1.0 - exp(-dt / OUTPUT_SMOOTH_S);
    float  gainL[OUTPUT_CHUNK], gainR[OUTPUT_CHUNK];

    if (!v->outSet) {
        v->outL   = targetL;
        v->outR   = targetR;
        v->outSet = true;
    }

    for (uint32_t at = 0; at < frames; at += OUTPUT_CHUNK) {
        uint32_t count = ((frames - at) < OUTPUT_CHUNK) ? (frames - at) : OUTPUT_CHUNK;

        for (uint32_t i = 0; i < count; i++) {
            v->outL += (targetL - v->outL) * k;
            v->outR += (targetR - v->outR) * k;
            gainL[i] = (float)v->outL;
            gainR[i] = (float)v->outR;
        }

        for (int n = 0; n < UA_BANK_VOICES; n++) {
            tUaBankVoice *     voice = &v->voice[n];
            const tEmuSample * s     = voice->sample;

            if (!voice->active) {
                continue;
            }
            double             end   = (double)s->frames + ua_resample_tail(voice->cutoff);

            for (uint32_t i = 0; i < count; i++) {
                double e = ua_env_step(&voice->env, &play->env, dt);

                if ((!s->looped && (voice->position >= end)) || (voice->fade <= 0.0) || ua_env_done(&voice->env)) {
                    voice->active = false;
                    break;
                }
                float  f = (float)(voice->level * voice->fade * e);
                float  l, r;

                read_frame(voice, &l, &r);
                left[at + i]    += l * f * gainL[i];

                if (right != NULL) {
                    right[at + i] += r * f * gainR[i];
                }
                voice->position += voice->step;
                voice->fade     -= voice->fadeStep;

                if (s->looped && (voice->position >= (double)s->loopEnd)) {   // notes §2
                    voice->position -= (double)(s->loopEnd - s->loopStart) * floor((voice->position - s->loopStart) / (double)(s->loopEnd - s->loopStart));
                    voice->looping   = true;
                }
            }
        }
    }
}

bool ua_bank_voices_use_other_than(const tUaBankVoices * v, const tEmuPreset * preset) {
    for (int i = 0; i < UA_BANK_VOICES; i++) {
        if (v->voice[i].active && (v->voice[i].preset != preset)) {
            return true;
        }
    }

    return false;
}
