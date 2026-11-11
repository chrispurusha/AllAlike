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
// Notes: Docs/code-notes/osPlugin.md - "// notes §k" refers there.

#include <math.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "synthlibPlugin.h"
#include "synthlibLog.h"
#include "synthlibPanelView.h"
#include "synthlibTypes.h"
#include "osCache.h"
#include "osDraw.h"
#include "osParams.h"
#include "osPrefs.h"
#include "osSample.h"
#include "osVoice.h"

// touch /tmp/oneshot-log, read /tmp/oneshot.log - see SynthLib's plugin/synthlibLog.h.

#ifndef OS_VERSION_STRING
#define OS_VERSION_STRING    "0.1.0"
#endif

#ifndef OS_AU_VERSION
#define OS_AU_VERSION    (0x00000100)
#endif

// THESE MAY NEVER CHANGE: a saved project finds the plug-in by them.
static const uint8_t gProcessorUid[16]  = {0x2C, 0xCF, 0x91, 0x2E, 0x75, 0x8C, 0x40, 0xE2, 0x84, 0x24, 0xBA, 0x80, 0x60, 0x09, 0x4E, 0xFB};
static const uint8_t gControllerUid[16] = {0x9E, 0xD2, 0xE2, 0x78, 0xA1, 0xAD, 0x43, 0x41, 0xB0, 0x66, 0x1C, 0x2F, 0xE0, 0x22, 0x67, 0xDA};

// THESE MUST MATCH do-plugin - the bundle identifier especially
#define FOUR_CC(a, b, c, d)    (((uint32_t)(a) << 24) | ((uint32_t)(b) << 16) | ((uint32_t)(c) << 8) | (uint32_t)(d))
#define OS_AU_TYPE            FOUR_CC('a', 'u', 'm', 'u')
#define OS_AU_SUBTYPE         FOUR_CC('O', 'n', 'S', 'h')
#define OS_AU_MANUFACTURER    FOUR_CC('C', 'P', 'u', 'r')
#define OS_AU_BUNDLE_ID       "com.chrispurusha.oneshot.au"

#define MAX_EVENTS            (256)
#define MAX_RETIRED           (64)
#define AUDITION_VELOCITY     (0.8f)
#define PATH_LENGTH           (2048)

typedef struct {
    uint32_t offset;
    uint8_t  note;
    bool     on;
    float    velocity;
} tOsEvent;

// notes §1: a sample replaced while the audio thread may still be playing it
typedef struct {
    tOsSample * sample;
    uint32_t    generation;    // freed once the audio thread has acknowledged this generation
} tRetired;

// notes §6: one of the four layers - all a one-sample instance was, with its own sample, settings and voices
typedef struct {
    tOsVoices         voices;
    _Atomic double    values[kOsParamCount];     // normalized

    _Atomic(tOsSample *) sample;                 // the one notes play; swapped by the main thread
    _Atomic uint32_t  generation;                // bumped after each swap
    _Atomic uint32_t  acknowledged;              // the audio thread: no voice plays anything older
    tRetired          retired[MAX_RETIRED];      // main thread
    uint32_t          retiredCount;
    const tOsSample * playing;                   // audio thread: the sample its voices were last given

    char              path[PATH_LENGTH];         // main thread: kept when the file cannot be read
    tOsCacheRequest   prefetched;                // audio thread: the last root pitch asked for
    double            prefetchedSetting[3];      // and the Stretch, Sync and Base it was asked at
} tOsLayer;

#define NO_AUDITION    (-1)

typedef struct {
    tOsLayer    layer[OS_LAYERS];
    double      hostRate;
    double      hostTempo;                       // audio thread: the host's, kept when it stops sending one
    tOsEvent    events[MAX_EVENTS];
    uint32_t    eventCount;
    _Atomic int audition;                        // notes §6: a layer, + OS_LAYERS for the stack; NO_AUDITION

    tOsCache    cache;                           // notes §4: resampled copies for every layer, built by its one thread
    _Atomic(const tOsSample *) borrowed;         // the sample the cache's thread is reading; never freed under it
} tOsPlugin;

// ---- parameters: every layer's - notes §6 ------------------------------------------------------------------

static tSynthLibParam gAllParams[OS_LAYERS * kOsParamCount];
static char           gTitles[OS_LAYERS * kOsParamCount][2][48];
static pthread_once_t gParamsOnce = PTHREAD_ONCE_INIT;

// Layer n's copy of each parameter: its id moved up by n x OS_LAYER_STRIDE, its names numbered
static void build_params(void) {
    for (uint32_t layer = 0; layer < OS_LAYERS; layer++) {
        for (uint32_t id = 0; id < kOsParamCount; id++) {
            uint32_t         at = (layer * kOsParamCount) + id;
            tSynthLibParam * p  = &gAllParams[at];

            *p            = gOsParams[id];
            p->id         = (layer * OS_LAYER_STRIDE) + id;
            snprintf(gTitles[at][0], sizeof(gTitles[at][0]), "%u %s", layer + 1, gOsParams[id].title);
            snprintf(gTitles[at][1], sizeof(gTitles[at][1]), "%u %s", layer + 1, gOsParams[id].shortTitle);
            p->title      = gTitles[at][0];
            p->shortTitle = gTitles[at][1];
        }
    }
}

// A host id's layer and parameter; false for an id that is neither
static bool split_id(uint32_t globalId, uint32_t * layer, uint32_t * id) {
    *layer = globalId / OS_LAYER_STRIDE;
    *id    = globalId % OS_LAYER_STRIDE;
    return (*layer < OS_LAYERS) && (*id < kOsParamCount);
}

static uint32_t global_id(uint32_t layer, uint32_t id) {
    return (layer * OS_LAYER_STRIDE) + id;
}

// ---- the samples, across threads - notes §1 -----------------------------------------------------------------

static void free_acknowledged(tOsPlugin * t, tOsLayer * l) {
    uint32_t ack  = atomic_load(&l->acknowledged);
    uint32_t keep = 0;

    for (uint32_t i = 0; i < l->retiredCount; i++) {
        if (((int32_t)(ack - l->retired[i].generation) >= 0) && (l->retired[i].sample != atomic_load(&t->borrowed))) {
            os_sample_free(l->retired[i].sample);
        } else {
            l->retired[keep++] = l->retired[i];
        }
    }

    l->retiredCount = keep;
}

static void install_sample(tOsPlugin * t, tOsLayer * l, tOsSample * sample) {
    tOsSample * old        = atomic_exchange(&l->sample, sample);
    uint32_t    generation = atomic_fetch_add(&l->generation, 1) + 1;

    free_acknowledged(t, l);

    if (old == NULL) {
        return;
    }

    if (l->retiredCount < MAX_RETIRED) {
        l->retired[l->retiredCount++] = (tRetired){
            old, generation
        };
    } else {
        synthlib_log_line("retire list full - %s left in memory", old->path);   // never freed under the audio thread
    }
}

static bool load_sample(tOsPlugin * t, int layer, const char * path, char * error, size_t errorLength) {
    char        unused[512];
    tOsLayer *  l      = &t->layer[layer];
    tOsSample * sample = os_sample_load(path, (error != NULL) ? error : unused, (error != NULL) ? errorLength : sizeof(unused));

    if (sample == NULL) {
        synthlib_log_line("layer %d: could not load %s: %s", layer + 1, path, (error != NULL) ? error : unused);
        return false;
    }
    snprintf(l->path, sizeof(l->path), "%s", path);
    install_sample(t, l, sample);
    synthlib_log_line("layer %d: loaded %s: %u frames, %u ch, %.0f Hz", layer + 1, path, sample->frames, sample->channels, sample->rate);
    return true;
}

// notes §4: the cache's thread reads only a layer's current sample, and marks it first so it cannot be freed meanwhile
static const tOsSample * cache_borrow(void * user, uint32_t serial) {
    tOsPlugin * t = (tOsPlugin *)user;

    for (int i = 0; i < OS_LAYERS; i++) {
        const tOsSample * s = atomic_load(&t->layer[i].sample);

        if ((s == NULL) || (s->serial != serial)) {
            continue;
        }
        atomic_store(&t->borrowed, s);

        if (s == atomic_load(&t->layer[i].sample)) {
            return s;
        }
        atomic_store(&t->borrowed, NULL);
    }

    return NULL;
}

static void cache_give_back(void * user, const tOsSample * sample) {
    (void)sample;
    atomic_store(&((tOsPlugin *)user)->borrowed, NULL);
}

// ---- lifetime -----------------------------------------------------------------------------------------------

static void * os_create(const tSynthLibPluginDesc * desc) {
    tOsPlugin * t = (tOsPlugin *)calloc(1, sizeof(tOsPlugin));
    bool        cached;

    (void)desc;

    if (t == NULL) {
        return NULL;
    }
    t->hostRate  = 48000.0;
    t->hostTempo = 120.0;   // until the host says otherwise
    atomic_store(&t->audition, NO_AUDITION);
    cached       = os_cache_init(&t->cache, (tOsCacheSource){cache_borrow, cache_give_back}, t);

    for (int i = 0; i < OS_LAYERS; i++) {
        tOsLayer * l = &t->layer[i];

        for (uint32_t id = 0; id < kOsParamCount; id++) {
            atomic_store(&l->values[id], gOsParams[id].defaultNormalized);
        }

        os_voices_init(&l->voices, t->hostRate);

        if (cached) {
            os_voices_set_cache(&l->voices, &t->cache);
        }
    }

    os_prefs_init();
    synthlib_log_line("created - OneShot %s", OS_VERSION_STRING);
    return t;
}

static void os_destroy(void * inst) {
    tOsPlugin * t = (tOsPlugin *)inst;

    for (int i = 0; i < OS_LAYERS; i++) {
        os_voices_stop(&t->layer[i].voices);
    }

    os_cache_destroy(&t->cache);

    for (int i = 0; i < OS_LAYERS; i++) {
        tOsLayer * l = &t->layer[i];

        for (uint32_t k = 0; k < l->retiredCount; k++) {
            os_sample_free(l->retired[k].sample);
        }

        os_sample_free(atomic_load(&l->sample));
    }

    free(t);
}

// notes §2: the plug-in's own state is the samples' paths - OSS2 the four layers', OSS1 (a one-sample project) the first's
#define STATE_MAGIC      "OSS2"
#define STATE_LENGTH     (4 + (OS_LAYERS * PATH_LENGTH))
#define STATE1_MAGIC     "OSS1"
#define STATE1_LENGTH    (4 + PATH_LENGTH)

static size_t os_get_state(void * inst, void * out, size_t len) {
    tOsPlugin * t = (tOsPlugin *)inst;

    if ((out == NULL) || (len < STATE_LENGTH)) {
        return STATE_LENGTH;
    }
    memcpy(out, STATE_MAGIC, 4);

    for (int i = 0; i < OS_LAYERS; i++) {
        memcpy((uint8_t *)out + 4 + (i * PATH_LENGTH), t->layer[i].path, PATH_LENGTH);
    }

    return STATE_LENGTH;
}

static void restore_layer(tOsPlugin * t, int layer, const uint8_t * saved) {
    tOsLayer * l = &t->layer[layer];
    char       path[PATH_LENGTH];

    memcpy(path, saved, PATH_LENGTH);
    path[PATH_LENGTH - 1] = '\0';

    if (strcmp(path, l->path) == 0) {
        return;
    }

    if ((path[0] == '\0') || !load_sample(t, layer, path, NULL, 0)) {
        snprintf(l->path, sizeof(l->path), "%s", path);   // shown as Missing; the project still names it
        install_sample(t, l, NULL);
    }
}

static void os_set_state(void * inst, const void * data, size_t len) {
    tOsPlugin *     t = (tOsPlugin *)inst;
    const uint8_t * p = (const uint8_t *)data;
    static uint8_t  none[PATH_LENGTH];

    if ((data != NULL) && (len >= STATE_LENGTH) && (memcmp(data, STATE_MAGIC, 4) == 0)) {
        for (int i = 0; i < OS_LAYERS; i++) {
            restore_layer(t, i, p + 4 + (i * PATH_LENGTH));
        }
    } else if ((data != NULL) && (len >= STATE1_LENGTH) && (memcmp(data, STATE1_MAGIC, 4) == 0)) {
        restore_layer(t, 0, p + 4);

        for (int i = 1; i < OS_LAYERS; i++) {
            restore_layer(t, i, none);
        }
    }
}

static void os_prepare(void * inst, const tSynthLibSetup * setup) {
    tOsPlugin * t = (tOsPlugin *)inst;

    t->hostRate = setup->sampleRate;

    for (int i = 0; i < OS_LAYERS; i++) {
        t->layer[i].voices.hostRate = setup->sampleRate;
    }
}

static void os_set_active(void * inst, bool active) {
    tOsPlugin * t = (tOsPlugin *)inst;

    (void)active;
    t->eventCount = 0;

    for (int i = 0; i < OS_LAYERS; i++) {
        os_voices_stop(&t->layer[i].voices);
    }
}

// ---- playing ------------------------------------------------------------------------------------------------

static double value(const tOsLayer * l, uint32_t id) {
    return atomic_load(&((tOsLayer *)l)->values[id]);
}

static long step(const tOsLayer * l, uint32_t id) {
    return lround(value(l, id) * gOsParams[id].stepCount);
}

static tOsPlay make_play(const tOsLayer * l) {
    double values[kOsParamCount];

    for (uint32_t id = 0; id < kOsParamCount; id++) {
        values[id] = value(l, id);
    }

    return (tOsPlay){
        .tune        = (double)(step(l, kOsParamTune) - 24) + ((double)(step(l, kOsParamFine) - 50) / 100.0),
        .velocity    = value(l, kOsParamVelocity),
        .keyTrack    = step(l, kOsParamKeyTrack) > 0,
        .rootKey     = (int)step(l, kOsParamRootKey),
        .gate        = step(l, kOsParamMode) > 0,
        .mono        = step(l, kOsParamVoices) > 0,
        .pan         = (double)(step(l, kOsParamPan) - 64) / 64.0,
        .gain        = os_volume_gain(value(l, kOsParamVolume)),
        .env         = os_env_shape(values),
        .start       = os_start_time(value(l, kOsParamStart)),
        .snap        = step(l, kOsParamSnap) > 0,
        .reverse     = step(l, kOsParamReverse) > 0,
        .protect     = os_protect_time(value(l, kOsParamProtect)),
        .protectSnap = step(l, kOsParamProtectSnap) > 0,
        .end         = os_end_fraction(value(l, kOsParamEnd)),
        .delay       = os_delay_time(value(l, kOsParamDelay)),
    };
}

// notes §5: Stretch, times the base tempo over the host's with Sync on; the envelope's times follow it
static void apply_stretch(const tOsLayer * l, double hostTempo, tOsPlay * p) {
    double f = os_stretch_multiple(value(l, kOsParamStretch));

    if (step(l, kOsParamSync) > 0) {
        f *= os_base_tempo(value(l, kOsParamBaseTempo)) / hostTempo;
    }
    p->stretch = f;

    if (f != 1.0) {
        p->env.attack  *= f;
        p->env.decay   *= f;
        p->env.release *= f;
        p->env.hold     = (p->env.hold < 0.0) ? p->env.hold : p->env.hold * f;
    }
}

// notes §7: a muted layer takes no notes, and what it is playing fades out
static bool muted(const tOsLayer * l) {
    return step(l, kOsParamMute) > 0;
}

// notes §6: the layer's keys, Low to High inclusive
static bool in_range(const tOsLayer * l, uint8_t note) {
    return (note >= step(l, kOsParamLowKey)) && (note <= step(l, kOsParamHighKey));
}

// notes §4: the root key's copy built before its first note - the only one, with Key Track off. A change of
// the host's tempo alone asks for nothing: the next note does, so instances do not all rebuild at once.
static void prefetch(tOsPlugin * t, tOsLayer * l, const tOsSample * sample, const tOsPlay * p) {
    tOsCacheRequest want       = {os_voices_key(&l->voices, sample, p, (uint8_t)p->rootKey), !p->keyTrack};
    tOsCacheKey     was        = l->prefetched.key;
    double          setting[3] = {value(l, kOsParamStretch), value(l, kOsParamSync), value(l, kOsParamBaseTempo)};
    bool            changed    = (want.key.serial != was.serial) || (want.key.reverse != was.reverse)
                                 || (want.key.step != was.step) || (want.key.hostRate != was.hostRate)
                                 || (want.key.protect != was.protect) || (want.only != l->prefetched.only)
                                 || (memcmp(setting, l->prefetchedSetting, sizeof(setting)) != 0)
                                 || ((want.key.stretch == 1.0) != (was.stretch == 1.0));

    if (changed) {
        os_cache_request(&t->cache, want.key, want.only);
        l->prefetched = want;
        memcpy(l->prefetchedSetting, setting, sizeof(setting));
    }
}

static void queue(tOsPlugin * t, tOsEvent e) {
    if (t->eventCount < MAX_EVENTS) {
        t->events[t->eventCount++] = e;
    }
}

static void os_note_on(void * inst, uint8_t channel, uint8_t note, float velocity, uint32_t sampleOffset) {
    (void)channel;
    queue((tOsPlugin *)inst, (tOsEvent){sampleOffset, note, true, velocity});
}

static void os_note_off(void * inst, uint8_t channel, uint8_t note, float velocity, uint32_t sampleOffset) {
    (void)channel;
    (void)velocity;
    queue((tOsPlugin *)inst, (tOsEvent){sampleOffset, note, false, 0.0f});
}

// notes §6: a note to every layer whose keys it falls in; a note off to all of them
static void play(tOsPlugin * t, const tOsSample *const * sample, const tOsPlay * p, const tOsEvent * e) {
    for (int i = 0; i < OS_LAYERS; i++) {
        if (!e->on) {
            os_voices_note_off(&t->layer[i].voices, &p[i], e->note);
        } else if ((sample[i] != NULL) && !muted(&t->layer[i]) && in_range(&t->layer[i], e->note)) {
            os_voices_note_on(&t->layer[i].voices, sample[i], &p[i], e->note, e->velocity);
        }
    }
}

static void render(tOsPlugin * t, const tOsPlay * p, float * left, float * right, uint32_t frames) {
    for (int i = 0; i < OS_LAYERS; i++) {
        os_voices_render(&t->layer[i].voices, &p[i], left, right, frames);
    }
}

static void os_process(void * inst, const float *const * in, uint32_t numIn, float ** out, uint32_t numOut,
                       uint32_t frames, const tSynthLibTransport * transport) {
    tOsPlugin *       t = (tOsPlugin *)inst;
    uint32_t          generation[OS_LAYERS];
    const tOsSample * sample[OS_LAYERS];
    tOsPlay           p[OS_LAYERS];

    (void)in;
    (void)numIn;

    if ((transport != NULL) && transport->tempoValid && (transport->tempo > 0.0)) {
        t->hostTempo = transport->tempo;
    }

    for (int i = 0; i < OS_LAYERS; i++) {
        tOsLayer * l = &t->layer[i];

        generation[i] = atomic_load(&l->generation);
        sample[i]     = atomic_load(&l->sample);
        p[i]          = make_play(l);
        apply_stretch(l, t->hostTempo, &p[i]);

        // notes §1: a new sample ends what the old one was playing, with a short fade
        if (sample[i] != l->playing) {
            os_voices_fade_all(&l->voices);
            l->playing = sample[i];
        }

        if (sample[i] != NULL) {
            prefetch(t, l, sample[i], &p[i]);
        }

        if (muted(l)) {
            os_voices_fade_all(&l->voices);   // notes §7
        }
    }

    // notes §6: Play - the shown layer alone at its root key, or that key through every layer it reaches
    int audition = atomic_exchange(&t->audition, NO_AUDITION);

    if (audition != NO_AUDITION) {
        int      shown = audition % OS_LAYERS;
        uint8_t  key   = (uint8_t)p[shown].rootKey;
        tOsEvent e     = {0, key, true, AUDITION_VELOCITY};

        if (audition >= OS_LAYERS) {
            play(t, sample, p, &e);
        } else if ((sample[shown] != NULL) && !muted(&t->layer[shown])) {
            os_voices_note_on(&t->layer[shown].voices, sample[shown], &p[shown], key, AUDITION_VELOCITY);
        }
    }

    if ((out == NULL) || (numOut == 0) || (out[0] == NULL)) {
        for (uint32_t i = 0; i < t->eventCount; i++) {
            play(t, sample, p, &t->events[i]);
        }

        t->eventCount = 0;
        return;
    }
    float *  left  = out[0];
    float *  right = (numOut > 1) ? out[1] : NULL;
    uint32_t done  = 0;

    memset(left, 0, (size_t)frames * sizeof(float));

    if (right != NULL) {
        memset(right, 0, (size_t)frames * sizeof(float));
    }

    // up to each note, then the note - every layer in the same block, on this thread (notes §6)
    for (uint32_t i = 0; i < t->eventCount; i++) {
        uint32_t at = (t->events[i].offset < frames) ? t->events[i].offset : frames;

        if (at > done) {
            render(t, p, left + done, (right != NULL) ? right + done : NULL, at - done);
            done = at;
        }
        play(t, sample, p, &t->events[i]);
    }

    t->eventCount = 0;

    if (done < frames) {
        render(t, p, left + done, (right != NULL) ? right + done : NULL, frames - done);
    }

    for (uint32_t ch = 2; ch < numOut; ch++) {
        if (out[ch] != NULL) {
            memset(out[ch], 0, (size_t)frames * sizeof(float));
        }
    }

    for (int i = 0; i < OS_LAYERS; i++) {
        if (!os_voices_use_other_than(&t->layer[i].voices, sample[i])) {
            atomic_store(&t->layer[i].acknowledged, generation[i]);
        }
    }
}

static void os_set_param(void * inst, uint32_t globalId, double normalized) {
    uint32_t layer, id;

    if (split_id(globalId, &layer, &id)) {
        atomic_store(&((tOsPlugin *)inst)->layer[layer].values[id], normalized);
    }
}

static double os_get_param(void * inst, uint32_t globalId) {
    uint32_t layer, id;

    return split_id(globalId, &layer, &id) ? value(&((tOsPlugin *)inst)->layer[layer], id) : 0.0;
}

static bool os_param_text_cb(const tSynthLibPluginDesc * desc, void * inst, uint32_t globalId, double normalized, char * out, size_t len) {
    uint32_t layer, id;

    (void)desc;
    (void)inst;

    if (!split_id(globalId, &layer, &id)) {
        return false;
    }
    os_param_text(id, normalized, out, len);
    return true;
}

// ---- the editor ---------------------------------------------------------------------------------------------

static bool host_load(void * user, int layer, const char * path, char * error, size_t errorLength) {
    return load_sample((tOsPlugin *)user, layer, path, error, errorLength);
}

static const tOsSample * host_sample(void * user, int layer) {
    return atomic_load(&((tOsPlugin *)user)->layer[layer].sample);
}

static const char * host_path(void * user, int layer) {
    return ((tOsPlugin *)user)->layer[layer].path;
}

static void host_audition(void * user, int layer, bool stack) {
    atomic_store(&((tOsPlugin *)user)->audition, stack ? layer + OS_LAYERS : layer);
}

// Through the host, as an edit of each, so it sees the change (and can undo it)
static void host_reset(void * user, int layer) {
    for (uint32_t id = 0; id < kOsParamCount; id++) {
        synthlib_plugin_param_edited(user, global_id((uint32_t)layer, id), gOsParams[id].defaultNormalized);
    }
}

// notes §8: the layer back to empty - its sample retired like a replaced one, its settings left alone
static void host_clear(void * user, int layer) {
    tOsPlugin * t = (tOsPlugin *)user;

    t->layer[layer].path[0] = '\0';
    install_sample(t, &t->layer[layer], NULL);
    synthlib_log_line("layer %d: cleared", layer + 1);
}

static const tOsSampleHost gSampleHost = {
    .load = host_load, .sample = host_sample, .path = host_path, .audition = host_audition, .reset = host_reset, .clear = host_clear,
};

// An edit from the panel is to the shown layer's copy of the parameter
static void edited(void * user, const tOsEdit * edit) {
    if (edit->paramId != UINT32_MAX) {
        uint32_t layer = (edit->layer >= 0) ? (uint32_t)edit->layer : (uint32_t)os_draw_layer();

        synthlib_plugin_param_edited(user, global_id(layer, edit->paramId), edit->normalized);
    }
}

static void os_panel_init(void) {
    os_draw_init();
    os_draw_set_pages(gOsPages, gOsPageCount);
    os_draw_set_host(&gSampleHost);

    for (uint32_t id = 0; id < kOsParamCount; id++) {
        os_draw_set_steps(id, gOsParams[id].stepCount);
    }
}

static void os_panel_sync(void * user) {
    tOsPlugin * t = (tOsPlugin *)user;

    os_draw_set_user(user);

    for (uint32_t layer = 0; layer < OS_LAYERS; layer++) {
        free_acknowledged(t, &t->layer[layer]);

        for (uint32_t id = 0; id < kOsParamCount; id++) {
            os_draw_set_value(global_id(layer, id), synthlib_plugin_param_value(user, global_id(layer, id)));
        }
    }
}

static void os_panel_frame(void * user, int pixelWidth, int pixelHeight) {
    (void)user;
    os_draw_frame(pixelWidth, pixelHeight);
}

static bool os_panel_press(void * user, double x, double y) {
    tOsEdit edit    = {.paramId = UINT32_MAX, .layer = -1};
    bool    grabbed = false;

    (void)os_draw_press(x, y, &edit, &grabbed);

    edited(user, &edit);
    return grabbed;
}

static bool os_panel_keeps_pointer(void * user) {
    (void)user;
    return os_dial_mode() == eDialModeRotary;
}

static void os_panel_drag_to(void * user, double x, double y) {
    tOsEdit edit = {.paramId = UINT32_MAX, .layer = -1};

    if (os_draw_drag_to(x, y, &edit)) {
        edited(user, &edit);
    }
}

static void os_panel_drag_by(void * user, double dx, double dy) {
    tOsEdit edit = {.paramId = UINT32_MAX, .layer = -1};

    if (os_draw_drag_by(dx, dy, &edit)) {
        edited(user, &edit);
    }
}

static void os_panel_release(void * user, double x, double y) {
    tOsEdit edit = {.paramId = UINT32_MAX, .layer = -1};

    os_draw_release(x, y, &edit);
    edited(user, &edit);
}

static bool os_panel_key(void * user, int glfwKey, unsigned int codepoint, bool isRepeat) {
    (void)user;
    return os_draw_key(glfwKey, codepoint, isRepeat);
}

static void os_panel_scroll(void * user, double x, double y, double dy) {
    (void)user;
    (void)x;
    (void)y;
    os_draw_scroll(dy);
}

static const tSynthLibPanel      gPanel = {
    .canvasWidth  = OS_CANVAS_W,
    .init         = os_panel_init,
    .sync         = os_panel_sync,
    .frame        = os_panel_frame,
    .press        = os_panel_press,
    .dragBy       = os_panel_drag_by,
    .keepsPointer = os_panel_keeps_pointer,
    .dragTo       = os_panel_drag_to,
    .release      = os_panel_release,
    .scroll       = os_panel_scroll,
    .key          = os_panel_key,
    .pointer      = os_draw_set_mouse,
    .menuActive   = os_draw_menu_active,
};

static void * os_create_view(const tSynthLibPluginDesc * desc, void * inst, double width, double height) {
    (void)desc;
    return synthlib_panel_view_create(&gPanel, inst, width, height);
}

static const tSynthLibBus        gOutputs[1] = {{"Out", 2, false, true}};

static const tSynthLibPluginDesc gDescriptor = {
    .name               = "OneShot",
    .vendor             = "Chris Purusha",
    .url                = "",
    .email              = "",
    .version            = OS_VERSION_STRING,

    .isInstrument       = true,
    .vst3SubCategory    = "Instrument|Sampler",
    .outputs            = gOutputs,
    .numOutputs         =                         1,
    .wantsMidiIn        = true,

    .vst3ProcessorUid   = gProcessorUid,
    .vst3ControllerUid  = gControllerUid,
    .auType             = OS_AU_TYPE,
    .auSubType          = OS_AU_SUBTYPE,
    .auManufacturer     = OS_AU_MANUFACTURER,
    .auVersion          = OS_AU_VERSION,
    .auBundleId         = OS_AU_BUNDLE_ID,

    .params             = gAllParams,
    .numParams          = OS_LAYERS * kOsParamCount,

    .editorDefaultWidth = OS_CANVAS_W,
    .editorMinWidth     = OS_CANVAS_W * 0.75,
    .editorMaxWidth     = OS_CANVAS_W * 2.0,
    .editorAspect       = OS_CANVAS_W / OS_CANVAS_H,

    .cb                 = {
        .create     = os_create,
        .destroy    = os_destroy,
        .prepare    = os_prepare,
        .setActive  = os_set_active,
        .process    = os_process,
        .noteOn     = os_note_on,
        .noteOff    = os_note_off,
        .setParam   = os_set_param,
        .getParam   = os_get_param,
        .paramText  = os_param_text_cb,
        .getState   = os_get_state,
        .setState   = os_set_state,
        .createView = os_create_view,
    }
};

static const tSynthLibPluginSet  gSet = {
    .variants = &gDescriptor,
    .count    = 1
};

static const tSynthLibPluginSet * plugin_variants(void) {
    pthread_once(&gParamsOnce, build_params);
    return &gSet;
}

SYNTHLIB_PLUGIN_REGISTER(plugin_variants, "oneshot")
