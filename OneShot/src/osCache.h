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

#ifndef __OS_CACHE_H__
#define __OS_CACHE_H__

#include <pthread.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stdint.h>

#include "osSample.h"

#define OS_CACHE_ENTRIES     (24)
#define OS_CACHE_REQUESTS    (32)

// What a copy is of: one load of a sample, its direction, the playback step it was resampled for (the file's rate
// over the host's, times the pitch), and - stretch not 1 - how far it is stretched, keeping its first protect seconds
typedef struct {
    uint32_t serial;
    bool     reverse;
    double   step;
    double   hostRate;
    double   stretch;   // 1: resampled only
    double   protect;   // seconds of the original; 0 unless stretched
} tOsCacheKey;

// notes §7: how the copy is laid out - source frames [0, protectIn) resampled into [0, protectOut), the rest
// stretched into [protectOut, frames)
typedef struct {
    uint32_t frames;
    uint32_t protectIn, protectOut;
    uint32_t regionIn, regionOut;
    double   step;
} tOsCopyPlan;

// notes §6: only - no other pitch is wanted (Key Track off): every other copy goes
typedef struct {
    tOsCacheKey key;
    bool        only;
} tOsCacheRequest;

// notes §2: state >= 0 is ready, with that many voices reading it; -1 is free or being built
typedef struct {
    _Atomic int32_t  state;
    _Atomic uint64_t lastUsed;
    tOsCacheKey      key;          // these are written only while state is -1
    float *          data[2];      // data[1] == data[0] for mono
    tOsCopyPlan      plan;
    uint32_t         frames;
    size_t           bytes;
} tOsCacheEntry;

// notes §3: how the worker reaches the sample - borrow returns it only if it is still the one with that serial,
// and keeps it from being freed until give_back
typedef struct {
    const tOsSample * (*borrow)(void * user, uint32_t serial);
    void (*give_back)(void * user, const tOsSample * sample);
} tOsCacheSource;

typedef struct {
    tOsCacheEntry    entry[OS_CACHE_ENTRIES];
    tOsCacheRequest  request[OS_CACHE_REQUESTS];   // single producer (the audio thread), single consumer (the worker)
    _Atomic uint32_t head, tail;
    _Atomic uint64_t clock;
    tOsCacheSource   source;
    void *           user;
    size_t           budget;                       // bytes, all entries together
    pthread_t        thread;
    pthread_mutex_t  lock;
    pthread_mutex_t  building;                     // notes §9: one builder at a time - the worker, or an offline note
    pthread_cond_t   wake;
    _Atomic bool     quit;
    bool             running;
} tOsCache;

bool os_cache_init(tOsCache * c, tOsCacheSource source, void * user);   // starts the worker
void os_cache_destroy(tOsCache * c);                                    // stops it; no voice may still hold an entry

// Audio thread. acquire returns a ready copy (held until release) or NULL; request asks the worker for one.
const tOsCacheEntry * os_cache_acquire(tOsCache * c, tOsCacheKey key);
const tOsCacheEntry * os_cache_acquire_similar(tOsCache * c, tOsCacheKey key);   // notes §8: stretched, any amount
void os_cache_release(const tOsCacheEntry * e);
void os_cache_request(tOsCache * c, tOsCacheKey key, bool only);

// notes §9: offline only - builds the copy on the calling thread, waiting for the worker if it is building, and
// returns it held (or NULL if it cannot be made). Never in real time: it takes as long as the build.
const tOsCacheEntry * os_cache_build_now(tOsCache * c, tOsCacheKey key);

// notes §4: what the copy holds - the step's resampling of the sample, read forwards or backwards. Frame n is
// played position n x step; the copy runs on through the filter's ring after the last frame.
uint32_t os_cache_length(const tOsSample * s, double step);
void os_cache_render(const tOsSample * s, bool reverse, double step, float * left, float * right, uint32_t frames);

tOsCopyPlan os_cache_plan(const tOsSample * s, tOsCacheKey key);
double os_cache_position(const tOsCacheEntry * e, uint32_t sourceFrame);   // a source frame (as played), in the copy

#endif // __OS_CACHE_H__
