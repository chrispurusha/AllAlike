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

#define BUDGET_BYTES     ((size_t)256 * 1024 * 1024)     // notes §5
#define SAFETY_WAKE_S    (1)                             // a request whose signal was missed waits no longer than this

static bool same_key(tOsCacheKey a, tOsCacheKey b) {
    return (a.serial == b.serial) && (a.reverse == b.reverse) && (a.step == b.step);
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

static void build(tOsCache * c, tOsCacheRequest request) {
    tOsCacheKey           key   = request.key;

    if (request.only) {
        drop_others(c, key);
    }

    if (key.step == 1.0) {
        return;   // the sample itself, read straight: nothing to copy
    }
    const tOsCacheEntry * have  = os_cache_acquire(c, key);

    if (have != NULL) {
        os_cache_release(have);
        return;
    }
    const tOsSample *     s     = c->source.borrow(c->user, key.serial);

    if (s == NULL) {
        return;   // a sample since replaced
    }
    uint32_t              n     = os_cache_length(s, key.step);
    size_t                bytes = (size_t)n * sizeof(float) * s->channels;
    tOsCacheEntry *       e     = (bytes <= c->budget) ? make_room(c, key.serial, bytes) : NULL;

    if (e != NULL) {
        e->data[0] = (float *)malloc((size_t)n * sizeof(float));
        e->data[1] = (s->channels == 2) ? (float *)malloc((size_t)n * sizeof(float)) : e->data[0];

        if ((e->data[0] == NULL) || (e->data[1] == NULL)) {
            free_entry(e);
        } else {
            os_cache_render(s, key.reverse, key.step, e->data[0], (s->channels == 2) ? e->data[1] : NULL, n);
            e->key    = key;
            e->frames = n;
            e->bytes  = bytes;
            atomic_store(&e->lastUsed, atomic_fetch_add(&c->clock, 1) + 1);
            atomic_store(&e->state, 0);   // published: the audio thread may now find it
        }
    }
    c->source.give_back(c->user, s);
}

static void * worker(void * arg) {
    tOsCache * c = (tOsCache *)arg;

    while (!atomic_load(&c->quit)) {
        uint32_t        tail = atomic_load(&c->tail);

        if (tail != atomic_load(&c->head)) {
            tOsCacheRequest request = c->request[tail % OS_CACHE_REQUESTS];

            atomic_store(&c->tail, tail + 1);
            build(c, request);
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
}
