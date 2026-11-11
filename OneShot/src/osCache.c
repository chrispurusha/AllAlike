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
// Notes: Docs/code-notes/osCache.md - "// notes §k" refers there.

#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "osCache.h"
#include "osResample.h"
#include "osStretch.h"

#define BUDGET_BYTES     ((size_t)256 * 1024 * 1024)     // notes §5
#define SAFETY_WAKE_S    (1)                             // a request whose signal was missed waits no longer than this
#define MIN_RATIO        (1.0 / 16.0)                    // notes §7: the stretcher's limits, either way
#define MAX_RATIO        (16.0)
#define CROSSFADE_S      (0.003)                         // notes §7: from the protected part into the stretched

static bool same_key(tOsCacheKey a, tOsCacheKey b) {
    return (a.serial == b.serial) && (a.reverse == b.reverse) && (a.step == b.step) && (a.hostRate == b.hostRate)
           && (a.stretch == b.stretch) && (a.protect == b.protect);
}

// ---- the audio thread ---------------------------------------------------------------------------------------

static bool hold(tOsCacheEntry * e) {
    int32_t s = atomic_load(&e->state);

    while (s >= 0) {
        if (atomic_compare_exchange_weak(&e->state, &s, s + 1)) {
            return true;
        }
    }
    return false;
}

const tOsCacheEntry * os_cache_acquire(tOsCache * c, tOsCacheKey key) {
    if (c == NULL) {
        return NULL;
    }

    for (int i = 0; i < OS_CACHE_ENTRIES; i++) {
        tOsCacheEntry * e = &c->entry[i];

        if (!hold(e)) {
            continue;
        }

        if (same_key(e->key, key)) {
            atomic_store(&e->lastUsed, atomic_fetch_add(&c->clock, 1) + 1);
            return e;
        }
        atomic_fetch_sub(&e->state, 1);
    }

    return NULL;
}

// notes §8: the most recently played copy of this sample at this pitch stretched by any amount - what a note plays
// while its own copy is built, rather than playing unstretched
const tOsCacheEntry * os_cache_acquire_similar(tOsCache * c, tOsCacheKey key) {
    tOsCacheEntry * best = NULL;

    if (c == NULL) {
        return NULL;
    }

    for (int i = 0; i < OS_CACHE_ENTRIES; i++) {
        tOsCacheEntry * e       = &c->entry[i];

        if (!hold(e)) {
            continue;
        }
        tOsCacheKey     k       = e->key;
        bool            similar = (k.serial == key.serial) && (k.reverse == key.reverse) && (k.step == key.step)
                                  && (k.hostRate == key.hostRate) && (k.stretch != 1.0);

        if (similar && ((best == NULL) || (atomic_load(&e->lastUsed) > atomic_load(&best->lastUsed)))) {
            if (best != NULL) {
                atomic_fetch_sub(&best->state, 1);
            }
            best = e;
        } else {
            atomic_fetch_sub(&e->state, 1);
        }
    }

    return best;
}

void os_cache_release(const tOsCacheEntry * e) {
    if (e != NULL) {
        atomic_fetch_sub(&((tOsCacheEntry *)e)->state, 1);
    }
}

void os_cache_request(tOsCache * c, tOsCacheKey key, bool only) {
    if ((c == NULL) || !c->running) {
        return;
    }
    uint32_t head = atomic_load(&c->head);

    if (head - atomic_load(&c->tail) >= OS_CACHE_REQUESTS) {
        return;   // full: the note plays live, and asks again next time
    }
    c->request[head % OS_CACHE_REQUESTS] = (tOsCacheRequest){
        key, only
    };
    atomic_store(&c->head, head + 1);
    pthread_cond_signal(&c->wake);   // notes §3: without the lock, so the audio thread never waits on it
}

// ---- what a copy holds - notes §4 ---------------------------------------------------------------------------

uint32_t os_cache_length(const tOsSample * s, double step) {
    double end = (double)s->frames + os_resample_tail(os_resample_cutoff(step));

    return (uint32_t)ceil(end / step) + 1;
}

void os_cache_render(const tOsSample * s, bool reverse, double step, float * left, float * right, uint32_t frames) {
    double cutoff = os_resample_cutoff(step);
    double last   = (double)s->frames - 1.0;

    for (uint32_t n = 0; n < frames; n++) {
        double at = (double)n * step;
        float  l, r;

        os_resample_read2(s->data[0], (s->channels == 2) ? s->data[1] : NULL, s->frames, reverse ? last - at : at,
                          cutoff, &l, &r);
        left[n] = l;

        if (right != NULL) {
            right[n] = r;
        }
    }
}

// notes §7: the protected part resampled (tape-style, so it lasts protect / pitch), the rest stretched to fill
// what remains of stretch x the original's length, never past the stretcher's limits - the protected part wins
tOsCopyPlan os_cache_plan(const tOsSample * s, tOsCacheKey key) {
    tOsCopyPlan p       = {.step = key.step};

    if (key.stretch == 1.0) {
        p.frames     = os_cache_length(s, key.step);
        p.protectIn  = s->frames;
        p.protectOut = p.frames;
        return p;
    }
    double      length  = (double)s->frames / s->rate;
    double      protect = fmin(fmax(0.0, key.protect), length);

    p.protectIn  = (uint32_t)lround(protect * s->rate);
    p.protectIn  = (p.protectIn > s->frames) ? s->frames : p.protectIn;
    p.protectOut = (uint32_t)lround((double)p.protectIn / key.step);
    p.regionIn   = s->frames - p.protectIn;

    if (p.regionIn > 0) {
        double rest  = (double)p.regionIn / s->rate;
        double fill  = (length * key.stretch) - ((double)p.protectOut / key.hostRate);
        double ratio = fmin(MAX_RATIO, fmax(MIN_RATIO, fill / rest));

        p.regionOut = (uint32_t)lround(rest * ratio * key.hostRate);
    }
    p.frames     = p.protectOut + p.regionOut;
    return p;
}

double os_cache_position(const tOsCacheEntry * e, uint32_t sourceFrame) {
    const tOsCopyPlan * p = &e->plan;

    if ((sourceFrame < p->protectIn) || (p->regionIn == 0)) {
        return (double)lround((double)sourceFrame / p->step);
    }
    return (double)p->protectOut + (double)lround((double)(sourceFrame - p->protectIn) * p->regionOut / p->regionIn);
}

// ---- the worker ---------------------------------------------------------------------------------------------

static void free_entry(tOsCacheEntry * e) {
    if (e->data[1] != e->data[0]) {
        free(e->data[1]);
    }
    free(e->data[0]);
    e->data[0] = e->data[1] = NULL;
    e->bytes   = 0;
}

// Takes an entry out of use if no voice holds it: it is then the worker's to free or fill
static bool claim(tOsCacheEntry * e) {
    int32_t idle = 0;

    return atomic_compare_exchange_strong(&e->state, &idle, -1);
}

static size_t bytes_in_use(tOsCache * c) {
    size_t total = 0;

    for (int i = 0; i < OS_CACHE_ENTRIES; i++) {
        total += c->entry[i].bytes;   // only the worker changes bytes
    }

    return total;
}

// notes §5: copies of any other sample first, then the least recently played, until there is room
static tOsCacheEntry * make_room(tOsCache * c, uint32_t serial, size_t needed) {
    tOsCacheEntry * free_slot = NULL;

    for (int i = 0; i < OS_CACHE_ENTRIES; i++) {
        tOsCacheEntry * e = &c->entry[i];

        if ((e->data[0] != NULL) && (e->key.serial != serial) && claim(e)) {
            free_entry(e);
        }

        if ((free_slot == NULL) && (e->data[0] == NULL) && (atomic_load(&e->state) < 0)) {
            free_slot = e;
        }
    }

    while ((free_slot == NULL) || (bytes_in_use(c) + needed > c->budget)) {
        tOsCacheEntry * oldest = NULL;

        for (int i = 0; i < OS_CACHE_ENTRIES; i++) {
            tOsCacheEntry * e = &c->entry[i];

            if (  (e->data[0] != NULL) && (atomic_load(&e->state) == 0)
               && ((oldest == NULL) || (atomic_load(&e->lastUsed) < atomic_load(&oldest->lastUsed)))) {
                oldest = e;
            }
        }

        if ((oldest == NULL) || !claim(oldest)) {
            return NULL;   // everything left is playing: this pitch plays live
        }
        free_entry(oldest);
        free_slot = (free_slot == NULL) ? oldest : free_slot;
    }
    return free_slot;
}

// notes §6: every copy but this one's, that no voice is reading
static void drop_others(tOsCache * c, tOsCacheKey key) {
    for (int i = 0; i < OS_CACHE_ENTRIES; i++) {
        tOsCacheEntry * e = &c->entry[i];

        if ((e->data[0] != NULL) && !same_key(e->key, key) && claim(e)) {
            free_entry(e);
        }
    }
}

static bool cancelled(void * user) {
    return atomic_load(&((tOsCache *)user)->quit);
}

// The source frames [from, from + count) as played - backwards when reversed - into a new buffer for the stretcher
static float * played(const tOsSample * s, int channel, bool reverse, uint32_t from, uint32_t count) {
    float * out = (float *)malloc(((size_t)count + 1) * sizeof(float));

    if (out != NULL) {
        for (uint32_t i = 0; i < count; i++) {
            uint32_t at = from + i;

            out[i] = s->data[channel][reverse ? (s->frames - 1 - at) : at];
        }
    }
    return out;
}

// notes §7: resampled up to the end of the protected part and a crossfade on, then the stretched rest over it
static bool render(tOsCache * c, const tOsSample * s, tOsCacheKey key, const tOsCopyPlan * plan, float * left, float * right) {
    if (key.stretch == 1.0) {
        os_cache_render(s, key.reverse, key.step, left, right, plan->frames);
        return true;
    }
    uint32_t fade   = (uint32_t)(CROSSFADE_S * key.hostRate);

    fade = (plan->protectOut == 0) ? 0 : ((fade > plan->regionOut) ? plan->regionOut : fade);
    os_cache_render(s, key.reverse, key.step, left, right, plan->protectOut + fade);

    if (plan->regionOut == 0) {
        return true;
    }
    bool     stereo = (right != NULL);
    float *  inL    = played(s, 0, key.reverse, plan->protectIn, plan->regionIn);
    float *  inR    = stereo ? played(s, 1, key.reverse, plan->protectIn, plan->regionIn) : NULL;
    float *  keepL  = (float *)malloc(((size_t)fade + 1) * sizeof(float));
    float *  keepR  = (float *)malloc(((size_t)fade + 1) * sizeof(float));
    bool     ok     = (inL != NULL) && (!stereo || (inR != NULL)) && (keepL != NULL) && (keepR != NULL);

    if (ok) {
        double pitch = key.step * key.hostRate / s->rate;

        memcpy(keepL, left + plan->protectOut, fade * sizeof(float));

        if (stereo) {
            memcpy(keepR, right + plan->protectOut, fade * sizeof(float));
        }
        ok = os_stretch_render(inL, inR, plan->regionIn, s->rate, key.hostRate, pitch, left + plan->protectOut,
                               stereo ? right + plan->protectOut : NULL, plan->regionOut, cancelled, c);
    }

    for (uint32_t i = 0; ok && (i < fade); i++) {
        float w = ((float)i + 0.5f) / (float)fade;

        left[plan->protectOut + i] = (keepL[i] * (1.0f - w)) + (left[plan->protectOut + i] * w);

        if (stereo) {
            right[plan->protectOut + i] = (keepR[i] * (1.0f - w)) + (right[plan->protectOut + i] * w);
        }
    }

    free(inL);
    free(inR);
    free(keepL);
    free(keepR);
    return ok;
}

// notes §6: an only request with a later one queued behind it is already out of date
static bool superseded(tOsCache * c, uint32_t tail) {
    uint32_t head = atomic_load(&c->head);

    for (uint32_t at = tail + 1; at != head; at++) {
        if (c->request[at % OS_CACHE_REQUESTS].only) {
            return true;
        }
    }

    return false;
}

static void build(tOsCache * c, tOsCacheRequest request) {
    tOsCacheKey           key   = request.key;

    if ((key.step == 1.0) && (key.stretch == 1.0)) {
        if (request.only) {
            drop_others(c, key);
        }
        return;   // the sample itself, read straight: nothing to copy
    }
    const tOsCacheEntry * have  = os_cache_acquire(c, key);

    if (have != NULL) {
        os_cache_release(have);

        if (request.only) {
            drop_others(c, key);
        }
        return;
    }
    const tOsSample *     s     = c->source.borrow(c->user, key.serial);

    if (s == NULL) {
        return;   // a sample since replaced
    }
    tOsCopyPlan           plan  = os_cache_plan(s, key);
    uint32_t              n     = plan.frames;
    size_t                bytes = (size_t)n * sizeof(float) * s->channels;
    tOsCacheEntry *       e     = ((n > 0) && (bytes <= c->budget)) ? make_room(c, key.serial, bytes) : NULL;

    if (e != NULL) {
        e->data[0] = (float *)malloc((size_t)n * sizeof(float));
        e->data[1] = (s->channels == 2) ? (float *)malloc((size_t)n * sizeof(float)) : e->data[0];

        if (  (e->data[0] == NULL) || (e->data[1] == NULL)
           || !render(c, s, key, &plan, e->data[0], (s->channels == 2) ? e->data[1] : NULL)) {
            free_entry(e);
        } else {
            e->key    = key;
            e->plan   = plan;
            e->frames = n;
            e->bytes  = bytes;
            atomic_store(&e->lastUsed, atomic_fetch_add(&c->clock, 1) + 1);
            atomic_store(&e->state, 0);   // published: the audio thread may now find it
        }
    }
    c->source.give_back(c->user, s);

    // notes §6: only once the new copy is there, so notes meanwhile still play the old one
    if (request.only) {
        drop_others(c, key);
    }
}

const tOsCacheEntry * os_cache_build_now(tOsCache * c, tOsCacheKey key) {
    if ((c == NULL) || !c->running) {
        return NULL;
    }
    pthread_mutex_lock(&c->building);
    build(c, (tOsCacheRequest){key, false});
    pthread_mutex_unlock(&c->building);
    return os_cache_acquire(c, key);
}

static void * worker(void * arg) {
    tOsCache * c = (tOsCache *)arg;

    while (!atomic_load(&c->quit)) {
        uint32_t        tail = atomic_load(&c->tail);

        if (tail != atomic_load(&c->head)) {
            tOsCacheRequest request = c->request[tail % OS_CACHE_REQUESTS];
            bool            stale   = request.only && superseded(c, tail);

            atomic_store(&c->tail, tail + 1);

            if (!stale) {
                pthread_mutex_lock(&c->building);
                build(c, request);
                pthread_mutex_unlock(&c->building);
            }
            continue;
        }
        struct timespec until;

        clock_gettime(CLOCK_REALTIME, &until);
        until.tv_sec += SAFETY_WAKE_S;
        pthread_mutex_lock(&c->lock);

        if ((atomic_load(&c->tail) == atomic_load(&c->head)) && !atomic_load(&c->quit)) {
            (void)pthread_cond_timedwait(&c->wake, &c->lock, &until);
        }
        pthread_mutex_unlock(&c->lock);
    }
    return NULL;
}

// ---- lifetime -----------------------------------------------------------------------------------------------

bool os_cache_init(tOsCache * c, tOsCacheSource source, void * user) {
    memset(c, 0, sizeof(*c));
    c->source  = source;
    c->user    = user;
    c->budget  = BUDGET_BYTES;
    os_resample_init();

    for (int i = 0; i < OS_CACHE_ENTRIES; i++) {
        atomic_store(&c->entry[i].state, -1);
    }

    pthread_mutex_init(&c->lock, NULL);
    pthread_mutex_init(&c->building, NULL);
    pthread_cond_init(&c->wake, NULL);
    c->running = (pthread_create(&c->thread, NULL, worker, c) == 0);
    return c->running;
}

void os_cache_destroy(tOsCache * c) {
    if (c->running) {
        atomic_store(&c->quit, true);
        pthread_mutex_lock(&c->lock);
        pthread_cond_signal(&c->wake);
        pthread_mutex_unlock(&c->lock);
        pthread_join(c->thread, NULL);
        c->running = false;
    }

    for (int i = 0; i < OS_CACHE_ENTRIES; i++) {
        free_entry(&c->entry[i]);
    }

    pthread_cond_destroy(&c->wake);
    pthread_mutex_destroy(&c->lock);
    pthread_mutex_destroy(&c->building);
}
