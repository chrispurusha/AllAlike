/*
 * UltraAlike - a sampler (a placeholder on a sample player's engine), as a VST3 and Audio Unit instrument.
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
// Notes: Docs/code-notes/uaPlugin.md - "// notes §k" refers there.

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
#include "uaCache.h"
#include "uaDraw.h"
#include "uaFx.h"
#include "uaParams.h"
#include "uaPrefs.h"
#include "uaSample.h"
#include "uaVoice.h"
#include "uaBankVoice.h"
#include "emuBank.h"

// touch /tmp/ultraalike-log, read /tmp/ultraalike.log - see SynthLib's plugin/synthlibLog.h.

#ifndef UA_VERSION_STRING
#define UA_VERSION_STRING    "0.1.0"
#endif

#ifndef UA_AU_VERSION
#define UA_AU_VERSION    (0x00000100)
#endif

// THESE MAY NEVER CHANGE: a saved project finds the plug-in by them.
static const uint8_t gProcessorUid[16]  = {0x59, 0x77, 0xA1, 0x18, 0xD5, 0xD1, 0x4B, 0x19, 0x88, 0xBD, 0x72, 0xD9, 0x3F, 0xE8, 0x7D, 0x92};
static const uint8_t gControllerUid[16] = {0x89, 0x27, 0xA3, 0xC2, 0x6D, 0x97, 0x48, 0x9B, 0x90, 0x2C, 0xF3, 0x66, 0x33, 0x2D, 0x7F, 0x72};

// THESE MUST MATCH do-plugin - the bundle identifier especially
#define FOUR_CC(a, b, c, d)    (((uint32_t)(a) << 24) | ((uint32_t)(b) << 16) | ((uint32_t)(c) << 8) | (uint32_t)(d))
#define UA_AU_TYPE            FOUR_CC('a', 'u', 'm', 'u')
#define UA_AU_SUBTYPE         FOUR_CC('U', 'l', 't', 'r')
#define UA_AU_MANUFACTURER    FOUR_CC('C', 'P', 'u', 'r')
#define UA_AU_BUNDLE_ID       "com.chrispurusha.ultraalike.au"

#define MAX_EVENTS            (256)
#define MAX_RETIRED           (64)
#define AUDITION_VELOCITY     (0.8f)
#define AUDITION_HOLD_S       (1.0)   // notes §12: a program's audition, held
#define PATH_LENGTH           (2048)

typedef struct {
    uint32_t offset;
    uint8_t  note;
    bool     on;
    float    velocity;
} tUaEvent;

// notes §12: an E-mu bank, shared by every program made from it; main thread only
typedef struct {
    tEmuBank * bank;
    int        refs;
} tUaBankRef;

// notes §12: a bank's preset as a layer plays it, in place of a sample
typedef struct {
    tUaBankRef * ref;
    uint32_t     preset;
    tEmuPreset   zones;
} tUaProgram;

// notes §1: a sample or program replaced while the audio thread may still be playing it
typedef struct {
    tUaSample *  sample;
    tUaProgram * program;
    uint32_t     generation;    // freed once the audio thread has acknowledged this generation
} tRetired;

// notes §6: one of the four layers - all a one-sample instance was, with its own sample, settings and voices
typedef struct {
    tUaVoices      voices;
    _Atomic double values[kUaParamCount];        // normalized

    _Atomic(tUaSample *) sample;                 // the one notes play; swapped by the main thread
    _Atomic(tUaProgram *) program;               // notes §12: or a bank's preset; never both
    tUaBankVoices      bankVoices;
    const tUaProgram * playingProgram;           // audio thread: the program its bank voices were last given
    _Atomic uint32_t   generation;               // bumped after each swap
    _Atomic uint32_t   acknowledged;             // the audio thread: no voice plays anything older
    tRetired           retired[MAX_RETIRED];     // main thread
    uint32_t           retiredCount;
    const tUaSample *  playing;                  // audio thread: the sample its voices were last given

    char               path[PATH_LENGTH];        // main thread: kept when the file cannot be read
    tUaFxState         fx;                       // notes §9: the layer's filter and saturation, after its voices
    tUaCacheRequest    prefetched;               // audio thread: the last root pitch asked for
    double             prefetchedSetting[3];     // and the Stretch, Sync and Base it was asked at
} tUaLayer;

#define NO_AUDITION    (-1)

typedef struct {
    tUaLayer     layer[UA_LAYERS];
    double       hostRate;
    double       hostTempo;                      // audio thread: the host's, kept when it stops sending one
    tUaEvent     events[MAX_EVENTS];
    uint32_t     eventCount;
    _Atomic int  audition;                       // notes §6: a layer, + UA_LAYERS for the stack; NO_AUDITION

    tUaCache     cache;                          // notes §4: resampled copies for every layer, built by its one thread
    _Atomic(const tUaSample *) borrowed;         // the sample the cache's thread is reading; never freed under it
    _Atomic bool oversample;                     // notes §10: saturation at 2x, every layer UA_FX_LATENCY late
    uint32_t     auditionLeft;                   // notes §12: audio thread - frames until a program's audition is let go
    uint8_t      auditionKey;
    bool         oversampling;                   // audio thread: what it last ran with
} tUaPlugin;

// ---- parameters: every layer's - notes §6 ------------------------------------------------------------------

static tSynthLibParam gAllParams[UA_LAYERS * kUaParamCount];
static char           gTitles[UA_LAYERS * kUaParamCount][2][48];
static pthread_once_t gParamsOnce = PTHREAD_ONCE_INIT;

// Layer n's copy of each parameter: its id moved up by n x UA_LAYER_STRIDE, its names numbered
static void build_params(void) {
    for (uint32_t layer = 0; layer < UA_LAYERS; layer++) {
        for (uint32_t id = 0; id < kUaParamCount; id++) {
            uint32_t         at = (layer * kUaParamCount) + id;
            tSynthLibParam * p  = &gAllParams[at];

            *p            = gUaParams[id];
            p->id         = (layer * UA_LAYER_STRIDE) + id;
            snprintf(gTitles[at][0], sizeof(gTitles[at][0]), "%u %s", layer + 1, gUaParams[id].title);
            snprintf(gTitles[at][1], sizeof(gTitles[at][1]), "%u %s", layer + 1, gUaParams[id].shortTitle);
            p->title      = gTitles[at][0];
            p->shortTitle = gTitles[at][1];
        }
    }
}

// A host id's layer and parameter; false for an id that is neither
static bool split_id(uint32_t globalId, uint32_t * layer, uint32_t * id) {
    *layer = globalId / UA_LAYER_STRIDE;
    *id    = globalId % UA_LAYER_STRIDE;
    return (*layer < UA_LAYERS) && (*id < kUaParamCount);
}

static uint32_t global_id(uint32_t layer, uint32_t id) {
    return (layer * UA_LAYER_STRIDE) + id;
}

// ---- the samples, across threads - notes §1 -----------------------------------------------------------------

static void program_free(tUaProgram * program) {
    if (program == NULL) {
        return;
    }

    if (--program->ref->refs == 0) {
        emu_bank_free(program->ref->bank);
        free(program->ref);
    }
    free(program);
}

// notes §12: a preset of the bank, its samples read now if no other preset has read them
static tUaProgram * make_program(tUaBankRef * ref, uint32_t preset) {
    uint32_t     count   = emu_bank_preset_count(ref->bank);
    tUaProgram * program = calloc(1, sizeof(tUaProgram));

    if (program == NULL) {
        return NULL;
    }
    program->ref    = ref;
    program->preset = (preset < count) ? preset : (count - 1u);
    (void)emu_bank_preset(ref->bank, program->preset, &program->zones);
    ref->refs++;
    return program;
}

static void free_acknowledged(tUaPlugin * t, tUaLayer * l) {
    uint32_t ack  = atomic_load(&l->acknowledged);
    uint32_t keep = 0;

    for (uint32_t i = 0; i < l->retiredCount; i++) {
        if (((int32_t)(ack - l->retired[i].generation) >= 0) && (l->retired[i].sample != atomic_load(&t->borrowed))) {
            ua_sample_free(l->retired[i].sample);
            program_free(l->retired[i].program);
        } else {
            l->retired[keep++] = l->retired[i];
        }
    }

    l->retiredCount = keep;
}

// A layer's sample or program (one of them, or neither) put in place; what it replaces retired
static void install(tUaPlugin * t, tUaLayer * l, tUaSample * sample, tUaProgram * program) {
    tUaSample *  old        = atomic_exchange(&l->sample, sample);
    tUaProgram * oldProgram = atomic_exchange(&l->program, program);
    uint32_t     generation = atomic_fetch_add(&l->generation, 1) + 1;

    free_acknowledged(t, l);

    if ((old == NULL) && (oldProgram == NULL)) {
        return;
    }

    if (l->retiredCount < MAX_RETIRED) {
        l->retired[l->retiredCount++] = (tRetired){
            old, oldProgram, generation
        };
    } else {
        synthlib_log_line("retire list full - left in memory");   // never freed under the audio thread
    }
}

static void install_sample(tUaPlugin * t, tUaLayer * l, tUaSample * sample) {
    install(t, l, sample, NULL);
}

// notes §12: an Emulator X bank in place of a sample, at one of its presets
static bool load_bank(tUaPlugin * t, int layer, const char * path, uint32_t preset, char * error, size_t errorLength) {
    tUaLayer *   l       = &t->layer[layer];
    tEmuBank *   bank    = emu_bank_open(path, error, errorLength);
    tUaBankRef * ref     = (bank != NULL) ? calloc(1, sizeof(tUaBankRef)) : NULL;
    tUaProgram * program = (ref != NULL) ? (ref->bank = bank, make_program(ref, preset)) : NULL;

    if (program == NULL) {
        synthlib_log_line("layer %d: could not load bank %s: %s", layer + 1, path, error);
        free(ref);
        emu_bank_free(bank);
        return false;
    }
    snprintf(l->path, sizeof(l->path), "%s", path);
    install(t, l, NULL, program);
    synthlib_log_line("layer %d: bank %s, preset %u of %u, %u zones", layer + 1, path, program->preset + 1, emu_bank_preset_count(bank), program->zones.zoneCount);
    return true;
}

static bool load_sample(tUaPlugin * t, int layer, const char * path, uint32_t preset, char * error, size_t errorLength) {
    char        unused[512];
    tUaLayer *  l      = &t->layer[layer];

    if (emu_bank_is_bank_file(path)) {
        return load_bank(t, layer, path, preset, (error != NULL) ? error : unused, (error != NULL) ? errorLength : sizeof(unused));
    }
    tUaSample * sample = ua_sample_load(path, (error != NULL) ? error : unused, (error != NULL) ? errorLength : sizeof(unused));

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
static const tUaSample * cache_borrow(void * user, uint32_t serial) {
    tUaPlugin * t = (tUaPlugin *)user;

    for (int i = 0; i < UA_LAYERS; i++) {
        const tUaSample * s = atomic_load(&t->layer[i].sample);

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

static void cache_give_back(void * user, const tUaSample * sample) {
    (void)sample;
    atomic_store(&((tUaPlugin *)user)->borrowed, NULL);
}

// ---- lifetime -----------------------------------------------------------------------------------------------

static void * ua_create(const tSynthLibPluginDesc * desc) {
    tUaPlugin * t = (tUaPlugin *)calloc(1, sizeof(tUaPlugin));
    bool        cached;

    (void)desc;

    if (t == NULL) {
        return NULL;
    }
    t->hostRate  = 48000.0;
    t->hostTempo = 120.0;   // until the host says otherwise
    atomic_store(&t->audition, NO_AUDITION);
    cached       = ua_cache_init(&t->cache, (tUaCacheSource){cache_borrow, cache_give_back}, t);

    for (int i = 0; i < UA_LAYERS; i++) {
        tUaLayer * l = &t->layer[i];

        for (uint32_t id = 0; id < kUaParamCount; id++) {
            atomic_store(&l->values[id], gUaParams[id].defaultNormalized);
        }

        ua_voices_init(&l->voices, t->hostRate);
        ua_bank_voices_init(&l->bankVoices, t->hostRate);

        if (cached) {
            ua_voices_set_cache(&l->voices, &t->cache);
        }
    }

    ua_prefs_init();
    synthlib_log_line("created - UltraAlike %s", UA_VERSION_STRING);
    return t;
}

static void ua_destroy(void * inst) {
    tUaPlugin * t = (tUaPlugin *)inst;

    for (int i = 0; i < UA_LAYERS; i++) {
        ua_voices_stop(&t->layer[i].voices);
    }

    ua_cache_destroy(&t->cache);

    for (int i = 0; i < UA_LAYERS; i++) {
        tUaLayer * l = &t->layer[i];

        for (uint32_t k = 0; k < l->retiredCount; k++) {
            ua_sample_free(l->retired[k].sample);
            program_free(l->retired[k].program);
        }

        ua_sample_free(atomic_load(&l->sample));
        program_free(atomic_load(&l->program));
    }

    free(t);
}

// notes §10: the whole instance's; the host is told the new lateness
// The audio thread notices the change at its next block and tells the host (ua_process)
static void set_oversampling(tUaPlugin * t, bool on) {
    atomic_store(&t->oversample, on);
}

// notes §10: oversampling is in force - and every layer late - only while some layer with a sample is saturating
static bool oversampling_in_force(tUaPlugin * t) {
    if (!atomic_load(&t->oversample)) {
        return false;
    }

    for (int i = 0; i < UA_LAYERS; i++) {
        if (((atomic_load(&t->layer[i].sample) != NULL) || (atomic_load(&t->layer[i].program) != NULL)) && (lround(atomic_load(&t->layer[i].values[kUaParamSaturate])) > 0)) {
            return true;
        }
    }

    return false;
}

static uint32_t ua_latency(void * inst) {
    return oversampling_in_force((tUaPlugin *)inst) ? UA_FX_LATENCY : 0;
}

// notes §2: the plug-in's own state - UAS4 as OSS3 then each layer's bank preset (u32, little-endian); OSS3 the four
// layers' paths and the instance's flags, OSS2 the paths alone, OSS1 (a one-sample project) the first's
#define STATE_MAGIC        "UAS4"
#define STATE_LENGTH       (STATE3_LENGTH + (UA_LAYERS * 4))
#define STATE3_MAGIC       "OSS3"
#define STATE3_LENGTH      (4 + (UA_LAYERS * PATH_LENGTH) + 4)
#define STATE2_MAGIC       "OSS2"
#define STATE2_LENGTH      (4 + (UA_LAYERS * PATH_LENGTH))
#define FLAG_OVERSAMPLE    (1u)
#define STATE1_MAGIC       "OSS1"
#define STATE1_LENGTH      (4 + PATH_LENGTH)

static size_t ua_get_state(void * inst, void * out, size_t len) {
    tUaPlugin * t     = (tUaPlugin *)inst;

    if ((out == NULL) || (len < STATE_LENGTH)) {
        return STATE_LENGTH;
    }
    memcpy(out, STATE_MAGIC, 4);

    for (int i = 0; i < UA_LAYERS; i++) {
        memcpy((uint8_t *)out + 4 + (i * PATH_LENGTH), t->layer[i].path, PATH_LENGTH);
    }

    uint32_t    flags = atomic_load(&t->oversample) ? FLAG_OVERSAMPLE : 0u;
    uint8_t *   f     = (uint8_t *)out + STATE2_LENGTH;

    f[0] = (uint8_t)flags;
    f[1] = f[2] = f[3] = 0;

    for (int i = 0; i < UA_LAYERS; i++) {
        const tUaProgram * program = atomic_load(&t->layer[i].program);
        uint32_t           preset  = (program != NULL) ? program->preset : 0u;
        uint8_t *          q       = (uint8_t *)out + STATE3_LENGTH + (4 * i);

        q[0] = (uint8_t)preset;
        q[1] = (uint8_t)(preset >> 8);
        q[2] = q[3] = 0;
    }

    return STATE_LENGTH;
}

static void restore_layer(tUaPlugin * t, int layer, const uint8_t * saved, uint32_t preset) {
    tUaLayer *         l       = &t->layer[layer];
    const tUaProgram * program = atomic_load(&l->program);
    char               path[PATH_LENGTH];

    memcpy(path, saved, PATH_LENGTH);
    path[PATH_LENGTH - 1] = '\0';

    if ((strcmp(path, l->path) == 0) && ((program == NULL) || (program->preset == preset))) {
        return;
    }

    if ((path[0] == '\0') || !load_sample(t, layer, path, preset, NULL, 0)) {
        snprintf(l->path, sizeof(l->path), "%s", path);   // shown as Missing; the project still names it
        install_sample(t, l, NULL);
    }
}

static void ua_set_state(void * inst, const void * data, size_t len) {
    tUaPlugin *     t  = (tUaPlugin *)inst;
    const uint8_t * p  = (const uint8_t *)data;
    static uint8_t  none[PATH_LENGTH];

    bool            v4 = (data != NULL) && (len >= STATE_LENGTH) && (memcmp(data, STATE_MAGIC, 4) == 0);
    bool            v3 = v4 || ((data != NULL) && (len >= STATE3_LENGTH) && (memcmp(data, STATE3_MAGIC, 4) == 0));

    if (v3 || ((data != NULL) && (len >= STATE2_LENGTH) && (memcmp(data, STATE2_MAGIC, 4) == 0))) {
        for (int i = 0; i < UA_LAYERS; i++) {
            const uint8_t * q = p + STATE3_LENGTH + (4 * i);

            restore_layer(t, i, p + 4 + (i * PATH_LENGTH), v4 ? ((uint32_t)q[0] | ((uint32_t)q[1] << 8)) : 0u);
        }

        set_oversampling(t, v3 && ((p[STATE2_LENGTH] & FLAG_OVERSAMPLE) != 0));
    } else if ((data != NULL) && (len >= STATE1_LENGTH) && (memcmp(data, STATE1_MAGIC, 4) == 0)) {
        restore_layer(t, 0, p + 4, 0);

        for (int i = 1; i < UA_LAYERS; i++) {
            restore_layer(t, i, none, 0);
        }
    }
}

static void ua_prepare(void * inst, const tSynthLibSetup * setup) {
    tUaPlugin * t = (tUaPlugin *)inst;

    t->hostRate = setup->sampleRate;

    for (int i = 0; i < UA_LAYERS; i++) {
        t->layer[i].voices.hostRate     = setup->sampleRate;
        t->layer[i].bankVoices.hostRate = setup->sampleRate;
        ua_voices_set_offline(&t->layer[i].voices, setup->offline);   // notes §11
    }
}

static void ua_set_active(void * inst, bool active) {
    tUaPlugin * t = (tUaPlugin *)inst;

    (void)active;
    t->eventCount = 0;

    for (int i = 0; i < UA_LAYERS; i++) {
        ua_voices_stop(&t->layer[i].voices);
        ua_bank_voices_stop(&t->layer[i].bankVoices);
        ua_fx_reset(&t->layer[i].fx);
    }
}

// ---- playing ------------------------------------------------------------------------------------------------

static double value(const tUaLayer * l, uint32_t id) {
    return atomic_load(&((tUaLayer *)l)->values[id]);
}

static long step(const tUaLayer * l, uint32_t id) {
    return lround(value(l, id) * gUaParams[id].stepCount);
}

static tUaPlay make_play(const tUaLayer * l) {
    double values[kUaParamCount];

    for (uint32_t id = 0; id < kUaParamCount; id++) {
        values[id] = value(l, id);
    }

    return (tUaPlay){
        .tune        = (double)(step(l, kUaParamTune) - 24) + ((double)(step(l, kUaParamFine) - 50) / 100.0),
        .velocity    = value(l, kUaParamVelocity),
        .keyTrack    = step(l, kUaParamKeyTrack) > 0,
        .rootKey     = (int)step(l, kUaParamRootKey),
        .gate        = step(l, kUaParamMode) > 0,
        .mono        = step(l, kUaParamVoices) > 0,
        .pan         = (double)(step(l, kUaParamPan) - 64) / 64.0,
        .gain        = ua_volume_gain(value(l, kUaParamVolume)),
        .env         = ua_env_shape(values),
        .start       = ua_start_time(value(l, kUaParamStart)),
        .snap        = step(l, kUaParamSnap) > 0,
        .reverse     = step(l, kUaParamReverse) > 0,
        .protect     = ua_protect_time(value(l, kUaParamProtect)),
        .protectSnap = step(l, kUaParamProtectSnap) > 0,
        .end         = ua_end_fraction(value(l, kUaParamEnd)),
        .delay       = ua_delay_time(value(l, kUaParamDelay)),
    };
}

// notes §5: Stretch, times the base tempo over the host's with Sync on; the envelope's times follow it
static void apply_stretch(const tUaLayer * l, double hostTempo, tUaPlay * p) {
    double f = ua_stretch_multiple(value(l, kUaParamStretch));

    if (step(l, kUaParamSync) > 0) {
        f *= ua_base_tempo(value(l, kUaParamBaseTempo)) / hostTempo;
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
static bool muted(const tUaLayer * l) {
    return step(l, kUaParamMute) > 0;
}

// notes §6: the layer's keys, Low to High inclusive
static bool in_range(const tUaLayer * l, uint8_t note) {
    return (note >= step(l, kUaParamLowKey)) && (note <= step(l, kUaParamHighKey));
}

// notes §4: the root key's copy built before its first note - the only one, with Key Track off. A change of
// the host's tempo alone asks for nothing: the next note does, so instances do not all rebuild at once.
static void prefetch(tUaPlugin * t, tUaLayer * l, const tUaSample * sample, const tUaPlay * p) {
    tUaCacheRequest want       = {ua_voices_key(&l->voices, sample, p, (uint8_t)p->rootKey), !p->keyTrack};
    tUaCacheKey     was        = l->prefetched.key;
    double          setting[3] = {value(l, kUaParamStretch), value(l, kUaParamSync), value(l, kUaParamBaseTempo)};
    bool            changed    = (want.key.serial != was.serial) || (want.key.reverse != was.reverse)
                                 || (want.key.step != was.step) || (want.key.hostRate != was.hostRate)
                                 || (want.key.protect != was.protect) || (want.only != l->prefetched.only)
                                 || (memcmp(setting, l->prefetchedSetting, sizeof(setting)) != 0)
                                 || ((want.key.stretch == 1.0) != (was.stretch == 1.0));

    if (changed) {
        ua_cache_request(&t->cache, want.key, want.only);
        l->prefetched = want;
        memcpy(l->prefetchedSetting, setting, sizeof(setting));
    }
}

static void queue(tUaPlugin * t, tUaEvent e) {
    if (t->eventCount < MAX_EVENTS) {
        t->events[t->eventCount++] = e;
    }
}

static void ua_note_on(void * inst, uint8_t channel, uint8_t note, float velocity, uint32_t sampleOffset) {
    (void)channel;
    queue((tUaPlugin *)inst, (tUaEvent){sampleOffset, note, true, velocity});
}

static void ua_note_off(void * inst, uint8_t channel, uint8_t note, float velocity, uint32_t sampleOffset) {
    (void)channel;
    (void)velocity;
    queue((tUaPlugin *)inst, (tUaEvent){sampleOffset, note, false, 0.0f});
}

// notes §6: a note to every layer whose keys it falls in; a note off to all of them
static void play(tUaPlugin * t, const tUaSample *const * sample, const tUaProgram *const * program, const tUaPlay * p, const tUaEvent * e) {
    for (int i = 0; i < UA_LAYERS; i++) {
        tUaLayer * l = &t->layer[i];

        if (!e->on) {
            ua_voices_note_off(&l->voices, &p[i], e->note);
            ua_bank_voices_note_off(&l->bankVoices, e->note);
        } else if (muted(l) || !in_range(l, e->note)) {
            continue;
        } else if (sample[i] != NULL) {
            ua_voices_note_on(&l->voices, sample[i], &p[i], e->note, e->velocity);
        } else if (program[i] != NULL) {
            ua_bank_voices_note_on(&l->bankVoices, &program[i]->zones, &p[i], e->note, e->velocity);   // notes §12
        }
    }
}

// notes §9: the layer's effects as set
static tUaFx make_fx(const tUaLayer * l) {
    return (tUaFx){
        .filter   = step(l, kUaParamFilter) > 0,
        .lowHz    = ua_cut_hz(value(l, kUaParamLowCut)),
        .highHz   = ua_cut_hz(value(l, kUaParamHighCut)),
        .slope    = (tUaSlope)step(l, kUaParamSlope),
        .saturate = step(l, kUaParamSaturate) > 0,
        .mode     = (tUaSatMode)step(l, kUaParamSatMode),
        .gainDb   = ua_param_db(kUaParamSatGain, value(l, kUaParamSatGain)),
        .outputDb = ua_param_db(kUaParamSatOutput, value(l, kUaParamSatOutput)),
        .warmthDb = ua_param_db(kUaParamSatWarmth, value(l, kUaParamSatWarmth)),
        .hfHz     = ua_hf_hz(value(l, kUaParamSatHf)),
    };
}

#define FX_CHUNK    (256)

// notes §9: each layer's voices, through its effects when it has any on, into the shared output
static void render(tUaPlugin * t, const tUaPlay * p, const tUaFx * fx, float * left, float * right, uint32_t frames) {
    float l[FX_CHUNK], r[FX_CHUNK];

    for (int i = 0; i < UA_LAYERS; i++) {
        tUaLayer * layer = &t->layer[i];

        if (!fx[i].filter && !fx[i].saturate && !fx[i].oversample) {
            ua_voices_render(&layer->voices, &p[i], left, right, frames);
            ua_bank_voices_render(&layer->bankVoices, &p[i], left, right, frames);
            continue;
        }

        for (uint32_t at = 0; at < frames; at += FX_CHUNK) {
            uint32_t n = ((frames - at) < FX_CHUNK) ? (frames - at) : FX_CHUNK;

            memset(l, 0, n * sizeof(float));
            memset(r, 0, n * sizeof(float));
            ua_voices_render(&layer->voices, &p[i], l, r, n);
            ua_bank_voices_render(&layer->bankVoices, &p[i], l, r, n);
            ua_fx_process(&layer->fx, &fx[i], t->hostRate, l, (right != NULL) ? r : NULL, n);

            for (uint32_t k = 0; k < n; k++) {
                left[at + k] += l[k];

                if (right != NULL) {
                    right[at + k] += r[k];
                }
            }
        }
    }
}

static void ua_process(void * inst, const float *const * in, uint32_t numIn, float ** out, uint32_t numOut,
                       uint32_t frames, const tSynthLibTransport * transport) {
    tUaPlugin *        t            = (tUaPlugin *)inst;
    uint32_t           generation[UA_LAYERS];
    const tUaSample *  sample[UA_LAYERS];
    const tUaProgram * program[UA_LAYERS];
    tUaPlay            p[UA_LAYERS];
    tUaFx              fx[UA_LAYERS];

    (void)in;
    (void)numIn;

    if ((transport != NULL) && transport->tempoValid && (transport->tempo > 0.0)) {
        t->hostTempo = transport->tempo;
    }
    // notes §10: a change of lateness starts every layer's effects afresh, and the host is told (it reads the
    // figure again on the main thread; the message is posted, so nothing re-enters the plug-in from here)
    bool               oversampling = oversampling_in_force(t);

    if (oversampling != t->oversampling) {
        for (int i = 0; i < UA_LAYERS; i++) {
            ua_fx_reset(&t->layer[i].fx);
        }

        t->oversampling = oversampling;
        synthlib_plugin_latency_changed(t);
    }

    for (int i = 0; i < UA_LAYERS; i++) {
        tUaLayer * l = &t->layer[i];

        generation[i]    = atomic_load(&l->generation);
        sample[i]        = atomic_load(&l->sample);
        program[i]       = atomic_load(&l->program);
        p[i]             = make_play(l);
        fx[i]            = make_fx(l);
        fx[i].oversample = oversampling;
        apply_stretch(l, t->hostTempo, &p[i]);

        // notes §1: a new sample ends what the old one was playing, with a short fade
        if (sample[i] != l->playing) {
            ua_voices_fade_all(&l->voices);
            l->playing = sample[i];
        }

        if (program[i] != l->playingProgram) {
            ua_bank_voices_fade_all(&l->bankVoices);
            l->playingProgram = program[i];
        }

        if (sample[i] != NULL) {
            prefetch(t, l, sample[i], &p[i]);
        }

        if (muted(l)) {
            ua_voices_fade_all(&l->voices);   // notes §7
            ua_bank_voices_fade_all(&l->bankVoices);
        }
    }

    // notes §6: Play - the shown layer alone at its root key, or that key through every layer it reaches
    int audition = atomic_exchange(&t->audition, NO_AUDITION);

    if (audition != NO_AUDITION) {
        int      shown = audition % UA_LAYERS;
        uint8_t  key   = (uint8_t)p[shown].rootKey;
        tUaEvent e     = {0, key, true, AUDITION_VELOCITY};

        if (audition >= UA_LAYERS) {
            play(t, sample, program, p, &e);
        } else if ((sample[shown] != NULL) && !muted(&t->layer[shown])) {
            ua_voices_note_on(&t->layer[shown].voices, sample[shown], &p[shown], key, AUDITION_VELOCITY);
        } else if ((program[shown] != NULL) && !muted(&t->layer[shown])) {
            ua_bank_voices_note_on(&t->layer[shown].bankVoices, &program[shown]->zones, &p[shown], key, AUDITION_VELOCITY);
        }
        t->auditionKey  = key;
        t->auditionLeft = (uint32_t)(AUDITION_HOLD_S * t->hostRate);
    }

    // notes §12: a program's audition is a held note, let go after a while
    if (t->auditionLeft > 0) {
        t->auditionLeft = (t->auditionLeft > frames) ? (t->auditionLeft - frames) : 0;

        for (int i = 0; (t->auditionLeft == 0) && (i < UA_LAYERS); i++) {
            ua_bank_voices_note_off(&t->layer[i].bankVoices, t->auditionKey);
        }
    }

    if ((out == NULL) || (numOut == 0) || (out[0] == NULL)) {
        for (uint32_t i = 0; i < t->eventCount; i++) {
            play(t, sample, program, p, &t->events[i]);
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
            render(t, p, fx, left + done, (right != NULL) ? right + done : NULL, at - done);
            done = at;
        }
        play(t, sample, program, p, &t->events[i]);
    }

    t->eventCount = 0;

    if (done < frames) {
        render(t, p, fx, left + done, (right != NULL) ? right + done : NULL, frames - done);
    }

    for (uint32_t ch = 2; ch < numOut; ch++) {
        if (out[ch] != NULL) {
            memset(out[ch], 0, (size_t)frames * sizeof(float));
        }
    }

    for (int i = 0; i < UA_LAYERS; i++) {
        if (  !ua_voices_use_other_than(&t->layer[i].voices, sample[i])
           && !ua_bank_voices_use_other_than(&t->layer[i].bankVoices, (program[i] != NULL) ? &program[i]->zones : NULL)) {
            atomic_store(&t->layer[i].acknowledged, generation[i]);
        }
    }
}

static void ua_set_param(void * inst, uint32_t globalId, double normalized) {
    uint32_t layer, id;

    if (split_id(globalId, &layer, &id)) {
        atomic_store(&((tUaPlugin *)inst)->layer[layer].values[id], normalized);
    }
}

static double ua_get_param(void * inst, uint32_t globalId) {
    uint32_t layer, id;

    return split_id(globalId, &layer, &id) ? value(&((tUaPlugin *)inst)->layer[layer], id) : 0.0;
}

static bool ua_param_text_cb(const tSynthLibPluginDesc * desc, void * inst, uint32_t globalId, double normalized, char * out, size_t len) {
    uint32_t layer, id;

    (void)desc;
    (void)inst;

    if (!split_id(globalId, &layer, &id)) {
        return false;
    }
    ua_param_text(id, normalized, out, len);
    return true;
}

// ---- the editor ---------------------------------------------------------------------------------------------

static bool host_load(void * user, int layer, const char * path, char * error, size_t errorLength) {
    return load_sample((tUaPlugin *)user, layer, path, 0, error, errorLength);
}

// notes §12: Prev / Next through a bank's presets, when the layer plays one
static bool host_step_preset(void * user, int layer, int direction) {
    tUaPlugin *        t       = (tUaPlugin *)user;
    tUaLayer *         l       = &t->layer[layer];
    const tUaProgram * program = atomic_load(&l->program);
    uint32_t           count   = (program != NULL) ? emu_bank_preset_count(program->ref->bank) : 0u;
    tUaProgram *       next;

    if (count == 0u) {
        return false;
    }
    next = make_program(program->ref, (uint32_t)((((int64_t)program->preset + direction) % count + count) % count));

    if (next != NULL) {
        install(t, l, NULL, next);
    }
    return true;
}

static bool host_preset_text(void * user, int layer, char * out, size_t size) {
    const tUaProgram * program = atomic_load(&((tUaPlugin *)user)->layer[layer].program);

    if (program == NULL) {
        return false;
    }
    snprintf(out, size, "Preset %u of %u: %s - %u zones%s", program->preset + 1, emu_bank_preset_count(program->ref->bank),
             emu_bank_preset_name(program->ref->bank, program->preset), program->zones.zoneCount,
             (program->zones.zoneCount == 0) ? " (its samples were not found)" : "");
    return true;
}

static const tUaSample * host_sample(void * user, int layer) {
    return atomic_load(&((tUaPlugin *)user)->layer[layer].sample);
}

static const char * host_path(void * user, int layer) {
    return ((tUaPlugin *)user)->layer[layer].path;
}

static void host_audition(void * user, int layer, bool stack) {
    atomic_store(&((tUaPlugin *)user)->audition, stack ? layer + UA_LAYERS : layer);
}

// Through the host, as an edit of each, so it sees the change (and can undo it)
static void host_reset(void * user, int layer) {
    for (uint32_t id = 0; id < kUaParamCount; id++) {
        synthlib_plugin_param_edited(user, global_id((uint32_t)layer, id), gUaParams[id].defaultNormalized);
    }
}

// notes §8: the layer back to empty - its sample retired like a replaced one, its settings left alone
static void host_clear(void * user, int layer) {
    tUaPlugin * t = (tUaPlugin *)user;

    t->layer[layer].path[0] = '\0';
    install_sample(t, &t->layer[layer], NULL);
    synthlib_log_line("layer %d: cleared", layer + 1);
}

static bool host_oversampling(void * user) {
    return atomic_load(&((tUaPlugin *)user)->oversample);
}

static void host_set_oversampling(void * user, bool on) {
    set_oversampling((tUaPlugin *)user, on);
}

static const tUaSampleHost gSampleHost = {
    .load         = host_load,         .sample          = host_sample,           .path = host_path, .audition = host_audition, .reset = host_reset, .clear = host_clear,
    .oversampling = host_oversampling, .setOversampling = host_set_oversampling,
    .stepPreset   = host_step_preset,  .presetText      = host_preset_text,
};

// An edit from the panel is to the shown layer's copy of the parameter
static void edited(void * user, const tUaEdit * edit) {
    if (edit->paramId != UINT32_MAX) {
        uint32_t layer = (edit->layer >= 0) ? (uint32_t)edit->layer : (uint32_t)ua_draw_layer();

        synthlib_plugin_param_edited(user, global_id(layer, edit->paramId), edit->normalized);
    }
}

static void ua_panel_init(void) {
    ua_draw_init();
    ua_draw_set_pages(gUaPages, gUaPageCount);
    ua_draw_set_host(&gSampleHost);

    for (uint32_t id = 0; id < kUaParamCount; id++) {
        ua_draw_set_steps(id, gUaParams[id].stepCount);
    }
}

static void ua_panel_sync(void * user) {
    tUaPlugin * t = (tUaPlugin *)user;

    ua_draw_set_user(user);

    for (uint32_t layer = 0; layer < UA_LAYERS; layer++) {
        free_acknowledged(t, &t->layer[layer]);

        for (uint32_t id = 0; id < kUaParamCount; id++) {
            ua_draw_set_value(global_id(layer, id), synthlib_plugin_param_value(user, global_id(layer, id)));
        }
    }
}

static void ua_panel_frame(void * user, int pixelWidth, int pixelHeight) {
    (void)user;
    ua_draw_frame(pixelWidth, pixelHeight);
}

static bool ua_panel_press(void * user, double x, double y) {
    tUaEdit edit    = {.paramId = UINT32_MAX, .layer = -1};
    bool    grabbed = false;

    (void)ua_draw_press(x, y, &edit, &grabbed);

    edited(user, &edit);
    return grabbed;
}

static bool ua_panel_keeps_pointer(void * user) {
    (void)user;
    return ua_dial_mode() == eDialModeRotary;
}

static void ua_panel_drag_to(void * user, double x, double y) {
    tUaEdit edit = {.paramId = UINT32_MAX, .layer = -1};

    if (ua_draw_drag_to(x, y, &edit)) {
        edited(user, &edit);
    }
}

static void ua_panel_drag_by(void * user, double dx, double dy) {
    tUaEdit edit = {.paramId = UINT32_MAX, .layer = -1};

    if (ua_draw_drag_by(dx, dy, &edit)) {
        edited(user, &edit);
    }
}

static void ua_panel_release(void * user, double x, double y) {
    tUaEdit edit = {.paramId = UINT32_MAX, .layer = -1};

    ua_draw_release(x, y, &edit);
    edited(user, &edit);
}

static bool ua_panel_key(void * user, int glfwKey, unsigned int codepoint, bool isRepeat) {
    (void)user;
    return ua_draw_key(glfwKey, codepoint, isRepeat);
}

static void ua_panel_scroll(void * user, double x, double y, double dy) {
    (void)user;
    (void)x;
    (void)y;
    ua_draw_scroll(dy);
}

static const tSynthLibPanel      gPanel = {
    .canvasWidth  = UA_CANVAS_W,
    .init         = ua_panel_init,
    .sync         = ua_panel_sync,
    .frame        = ua_panel_frame,
    .press        = ua_panel_press,
    .dragBy       = ua_panel_drag_by,
    .keepsPointer = ua_panel_keeps_pointer,
    .dragTo       = ua_panel_drag_to,
    .release      = ua_panel_release,
    .scroll       = ua_panel_scroll,
    .key          = ua_panel_key,
    .pointer      = ua_draw_set_mouse,
    .menuActive   = ua_draw_menu_active,
};

static void * ua_create_view(const tSynthLibPluginDesc * desc, void * inst, double width, double height) {
    (void)desc;
    return synthlib_panel_view_create(&gPanel, inst, width, height);
}

static const tSynthLibBus        gOutputs[1] = {{"Out", 2, false, true}};

static const tSynthLibPluginDesc gDescriptor = {
    .name               = "UltraAlike",
    .vendor             = "Chris Purusha",
    .url                = "",
    .email              = "",
    .version            = UA_VERSION_STRING,

    .isInstrument       = true,
    .vst3SubCategory    = "Instrument|Sampler",
    .outputs            = gOutputs,
    .numOutputs         =                         1,
    .wantsMidiIn        = true,

    .vst3ProcessorUid   = gProcessorUid,
    .vst3ControllerUid  = gControllerUid,
    .auType             = UA_AU_TYPE,
    .auSubType          = UA_AU_SUBTYPE,
    .auManufacturer     = UA_AU_MANUFACTURER,
    .auVersion          = UA_AU_VERSION,
    .auBundleId         = UA_AU_BUNDLE_ID,

    .params             = gAllParams,
    .numParams          = UA_LAYERS * kUaParamCount,

    .editorDefaultWidth = UA_CANVAS_W,
    .editorMinWidth     = UA_CANVAS_W * 0.75,
    .editorMaxWidth     = UA_CANVAS_W * 2.0,
    .editorAspect       = UA_CANVAS_W / UA_CANVAS_H,

    .cb                 = {
        .create         = ua_create,
        .destroy        = ua_destroy,
        .prepare        = ua_prepare,
        .setActive      = ua_set_active,
        .process        = ua_process,
        .noteOn         = ua_note_on,
        .noteOff        = ua_note_off,
        .setParam       = ua_set_param,
        .getParam       = ua_get_param,
        .paramText      = ua_param_text_cb,
        .getState       = ua_get_state,
        .setState       = ua_set_state,
        .latencySamples = ua_latency,
        .createView     = ua_create_view,
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

SYNTHLIB_PLUGIN_REGISTER(plugin_variants, "ultraalike")
