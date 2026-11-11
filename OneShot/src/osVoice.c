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
// Notes: Docs/code-notes/osVoice.md - "// notes §k" refers there.

#include <math.h>
#include <string.h>

#include "osResample.h"
#include "osVoice.h"

#define OUTPUT_SMOOTH_S    (0.005) // notes §4: Pan and Volume reach a new setting over a few ms
#define SNAP_RADIUS_S      (0.005) // notes §5: how far either side of Start a zero crossing is looked for
#define START_FADE_S       (0.001)
#define OUTPUT_CHUNK       (64)
#define STEAL_FADE_S       (0.003) // notes §2: a stolen or replaced voice, and a change of sample

void os_voices_init(tOsVoices * v, double hostRate) {
    os_resample_init();
    memset(v, 0, sizeof(*v));
    v->hostRate = hostRate;
}

void os_voices_set_cache(tOsVoices * v, tOsCache * cache) {
    v->cache = cache;
}

// Silences a voice and lets its copy go
static void end_voice(tOsVoice * voice) {
    os_cache_release(voice->copy);
    voice->copy   = NULL;
    voice->active = false;
}

void os_voices_stop(tOsVoices * v) {
    for (int i = 0; i < OS_VOICES; i++) {
        end_voice(&v->voice[i]);
    }
}

static void fade_out(tOsVoices * v, tOsVoice * voice, double seconds) {
    double step = 1.0 / fmax(1.0, seconds * v->hostRate);

    if ((voice->fadeStep == 0.0) || (step > voice->fadeStep)) {
        voice->fadeStep = step;
    }
}

void os_voices_fade_all(tOsVoices * v) {
    for (int i = 0; i < OS_VOICES; i++) {
        if (v->voice[i].active) {
            fade_out(v, &v->voice[i], STEAL_FADE_S);
        }
    }
}

// notes §1: a free voice, else the oldest, faded out under the new note
static tOsVoice * take_voice(tOsVoices * v) {
    tOsVoice * oldest = &v->voice[0];

    for (int i = 0; i < OS_VOICES; i++) {
        if (!v->voice[i].active) {
            return &v->voice[i];
        }

        if (v->voice[i].age < oldest->age) {
            oldest = &v->voice[i];
        }
    }

    // the stolen voice cannot fade under the new one in the same slot: it is cut, which a free voice avoids
    return oldest;
}

// notes §6: in frames as played - from the end when reversed
bool os_voices_stretched(const tOsPlay * play) {
    return (play->stretch > 0.0) && (play->stretch != 1.0);
}

tOsCacheKey os_voices_key(const tOsVoices * v, const tOsSample * sample, const tOsPlay * play, uint8_t note) {
    bool stretched = os_voices_stretched(play);

    return (tOsCacheKey){
        sample->serial, play->reverse, os_voices_step(v, sample, play, note), v->hostRate,
        stretched ? play->stretch : 1.0, stretched ? play->protect : 0.0
    };
}

uint32_t os_voices_start_frame(const tOsSample * sample, const tOsPlay * play) {
    double   at     = play->start * sample->rate;

    if ((at <= 0.0) || (sample->frames == 0)) {
        return 0;
    }

    if (at >= (double)sample->frames) {
        return sample->frames;
    }
    uint32_t frame  = (uint32_t)lround(at);
    uint32_t last   = sample->frames - 1;

    if (!play->snap) {
        return frame;
    }
    uint32_t radius = (uint32_t)(SNAP_RADIUS_S * sample->rate);

    return play->reverse ? last - os_sample_zero_crossing(sample, last - frame, radius)
                         : os_sample_zero_crossing(sample, frame, radius);
}

double os_voices_step(const tOsVoices * v, const tOsSample * sample, const tOsPlay * play, uint8_t note) {
    double semitones = play->tune + (play->keyTrack ? (double)((int)note - play->rootKey) : 0.0);

    return (sample->rate / v->hostRate) * pow(2.0, semitones / 12.0);
}

void os_voices_note_on(tOsVoices * v, const tOsSample * sample, const tOsPlay * play, uint8_t note, float velocity) {
    if ((sample == NULL) || (sample->frames == 0)) {
        return;
    }

    if (play->mono) {
        os_voices_fade_all(v);
    }
    tOsVoice * voice     = take_voice(v);

    end_voice(voice);
    *voice          = (tOsVoice){
        .sample   = sample,
        .position = 0.0,   // set below, from Start
        .step     = os_voices_step(v, sample, play, note),
        .reverse  = play->reverse,
        .level    = (1.0 - play->velocity) + (play->velocity * velocity),
        .fade     = 1.0,
        .fadeStep = 0.0,
        .note     = note,
        .active   = true,
        .age      = ++v->started,
    };
    uint32_t   start     = os_voices_start_frame(sample, play);

    voice->position = (double)start;
    voice->rise     = ((start > 0) && !play->snap) ? 0.0 : 1.0;
    voice->riseStep = 1.0 / (START_FADE_S * v->hostRate);
    voice->cutoff   = os_resample_cutoff(voice->step);
    voice->end      = (double)sample->frames + os_resample_tail(voice->cutoff); // notes §3: the filter's ring after the last frame

    // notes §7, §8: through the copy if it is ready, else (stretched) the nearest one, else live - and ask for it
    bool       stretched = os_voices_stretched(play);

    if (stretched || (voice->step != 1.0)) {
        tOsCacheKey key = os_voices_key(v, sample, play, note);

        voice->copy = os_cache_acquire(v->cache, key);

        if (voice->copy == NULL) {
            os_cache_request(v->cache, key, !play->keyTrack);
            voice->copy = stretched ? os_cache_acquire_similar(v->cache, key) : NULL;
        }

        if (voice->copy != NULL) {
            voice->position = os_cache_position(voice->copy, start);
            voice->end      = (double)voice->copy->frames;
            voice->step     = 1.0;
        }
    }
    os_env_start(&voice->env);
}

void os_voices_note_off(tOsVoices * v, const tOsPlay * play, uint8_t note) {
    if (!play->gate) {
        return;
    }

    for (int i = 0; i < OS_VOICES; i++) {
        if (v->voice[i].active && (v->voice[i].note == note)) {
            os_env_release(&v->voice[i].env);
        }
    }
}

void os_voices_render(tOsVoices * v, const tOsPlay * play, float * left, float * right, uint32_t frames) {
    // equal-power pan, times the volume: the targets the output gains move toward
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

    // notes §4: the smoothed gains for a chunk, then every voice through the same chunk
    for (uint32_t at = 0; at < frames; at += OUTPUT_CHUNK) {
        uint32_t count = ((frames - at) < OUTPUT_CHUNK) ? (frames - at) : OUTPUT_CHUNK;

        for (uint32_t i = 0; i < count; i++) {
            v->outL += (targetL - v->outL) * k;
            v->outR += (targetR - v->outR) * k;
            gainL[i] = (float)v->outL;
            gainR[i] = (float)v->outR;
        }

        for (int n = 0; n < OS_VOICES; n++) {
            tOsVoice *        voice = &v->voice[n];
            const tOsSample * s     = voice->sample;

            if (!voice->active) {
                continue;
            }

            for (uint32_t i = 0; i < count; i++) {
                double e = os_env_step(&voice->env, &play->env, dt);

                if ((voice->position >= voice->end) || (voice->fade <= 0.0) || os_env_done(&voice->env)) {
                    end_voice(voice);
                    break;
                }
                float  f = (float)(voice->level * voice->fade * e * voice->rise);

                voice->rise      = fmin(1.0, voice->rise + voice->riseStep);
                float  l, r;

                if (voice->copy != NULL) {
                    uint32_t n = (uint32_t)voice->position;

                    l = voice->copy->data[0][n];
                    r = voice->copy->data[1][n];
                } else {
                    double at = voice->reverse ? ((double)s->frames - 1.0) - voice->position : voice->position;

                    os_resample_read2(s->data[0], (s->channels == 2) ? s->data[1] : NULL, s->frames, at, voice->cutoff, &l, &r);
                }
                left[at + i]    += l * f * gainL[i];

                if (right != NULL) {
                    right[at + i] += r * f * gainR[i];
                }
                voice->position += voice->step;
                voice->fade     -= voice->fadeStep;
            }
        }
    }
}

bool os_voices_use_other_than(const tOsVoices * v, const tOsSample * sample) {
    for (int i = 0; i < OS_VOICES; i++) {
        if (v->voice[i].active && (v->voice[i].sample != sample)) {
            return true;
        }
    }

    return false;
}
