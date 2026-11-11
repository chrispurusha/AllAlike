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

typedef struct {
    tOsVoices         voices;
    double            hostRate;
    tOsEvent          events[MAX_EVENTS];
    uint32_t          eventCount;
    _Atomic double    values[kOsParamCount];     // normalized

    _Atomic(tOsSample *)  sample;                // the one notes play; swapped by the main thread
    _Atomic uint32_t  generation;                // bumped after each swap
    _Atomic uint32_t  acknowledged;              // the audio thread: no voice plays anything older
    tRetired          retired[MAX_RETIRED];      // main thread
    uint32_t          retiredCount;
    const tOsSample * playing;                   // audio thread: the sample its voices were last given

    char              path[PATH_LENGTH];         // main thread: kept when the file cannot be read
    _Atomic bool      audition;

    tOsCache          cache;                     // notes §4: resampled copies, built by its own thread
    _Atomic(const tOsSample *) borrowed;         // the sample the cache's thread is reading; never freed under it
    tOsCacheRequest   prefetched;                // audio thread: the last root pitch asked for
    double            prefetchedSetting[3];      // and the Stretch, Sync and Base it was asked at
    double            hostTempo;                 // audio thread: the host's, kept when it stops sending one
} tOsPlugin;

// ---- the sample, across threads - notes §1 ------------------------------------------------------------------

static void free_acknowledged(tOsPlugin * t) {
    uint32_t ack  = atomic_load(&t->acknowledged);
    uint32_t keep = 0;

    for (uint32_t i = 0; i < t->retiredCount; i++) {
        if (((int32_t)(ack - t->retired[i].generation) >= 0) && (t->retired[i].sample != atomic_load(&t->borrowed))) {
            os_sample_free(t->retired[i].sample);
        } else {
            t->retired[keep++] = t->retired[i];
        }
    }

    t->retiredCount = keep;
}

static void install_sample(tOsPlugin * t, tOsSample * sample) {
    tOsSample * old        = atomic_exchange(&t->sample, sample);
    uint32_t    generation = atomic_fetch_add(&t->generation, 1) + 1;

    free_acknowledged(t);

    if (old == NULL) {
        return;
    }

    if (t->retiredCount < MAX_RETIRED) {
        t->retired[t->retiredCount++] = (tRetired){
            old, generation
        };
    } else {
        synthlib_log_line("retire list full - %s left in memory", old->path);   // never freed under the audio thread
    }
}

static bool load_sample(tOsPlugin * t, const char * path, char * error, size_t errorLength) {
    char        unused[512];
    tOsSample * sample = os_sample_load(path, (error != NULL) ? error : unused, (error != NULL) ? errorLength : sizeof(unused));

    if (sample == NULL) {
        synthlib_log_line("could not load %s: %s", path, (error != NULL) ? error : unused);
        return false;
    }
    snprintf(t->path, sizeof(t->path), "%s", path);
    install_sample(t, sample);
    synthlib_log_line("loaded %s: %u frames, %u ch, %.0f Hz", path, sample->frames, sample->channels, sample->rate);
    return true;
}

// notes §4: the cache's thread reads only the current sample, and marks it first so it cannot be freed meanwhile
static const tOsSample * cache_borrow(void * user, uint32_t serial) {
    tOsPlugin *       t = (tOsPlugin *)user;
    const tOsSample * s = atomic_load(&t->sample);

    atomic_store(&t->borrowed, s);

    if ((s == NULL) || (s != atomic_load(&t->sample)) || (s->serial != serial)) {
        atomic_store(&t->borrowed, NULL);
        return NULL;
    }
    return s;
}

static void cache_give_back(void * user, const tOsSample * sample) {
    (void)sample;
    atomic_store(&((tOsPlugin *)user)->borrowed, NULL);
}

// ---- lifetime -----------------------------------------------------------------------------------------------

static void * os_create(const tSynthLibPluginDesc * desc) {
    tOsPlugin * t = (tOsPlugin *)calloc(1, sizeof(tOsPlugin));

    (void)desc;

    if (t == NULL) {
        return NULL;
    }

    for (uint32_t i = 0; i < kOsParamCount; i++) {
        atomic_store(&t->values[i], gOsParams[i].defaultNormalized);
    }

    t->hostRate  = 48000.0;
    t->hostTempo = 120.0;   // until the host says otherwise
    os_voices_init(&t->voices, t->hostRate);

    if (os_cache_init(&t->cache, (tOsCacheSource){cache_borrow, cache_give_back}, t)) {
        os_voices_set_cache(&t->voices, &t->cache);
    }
    os_prefs_init();
    synthlib_log_line("created - OneShot %s", OS_VERSION_STRING);
    return t;
}

static void os_destroy(void * inst) {
    tOsPlugin * t = (tOsPlugin *)inst;

    os_voices_stop(&t->voices);
    os_cache_destroy(&t->cache);

    for (uint32_t i = 0; i < t->retiredCount; i++) {
        os_sample_free(t->retired[i].sample);
    }

    os_sample_free(atomic_load(&t->sample));
    free(t);
}

// notes §2: the plug-in's own state is the sample's path
#define STATE_MAGIC     "OSS1"
#define STATE_LENGTH    (4 + PATH_LENGTH)

static size_t os_get_state(void * inst, void * out, size_t len) {
    tOsPlugin * t = (tOsPlugin *)inst;

    if ((out == NULL) || (len < STATE_LENGTH)) {
        return STATE_LENGTH;
    }
    memcpy(out, STATE_MAGIC, 4);
    memcpy((uint8_t *)out + 4, t->path, PATH_LENGTH);
    return STATE_LENGTH;
}

static void os_set_state(void * inst, const void * data, size_t len) {
    tOsPlugin * t = (tOsPlugin *)inst;
    char        path[PATH_LENGTH];

    if ((data == NULL) || (len < STATE_LENGTH) || (memcmp(data, STATE_MAGIC, 4) != 0)) {
        return;
    }
    memcpy(path, (const uint8_t *)data + 4, PATH_LENGTH);
    path[PATH_LENGTH - 1] = '\0';

    if (strcmp(path, t->path) == 0) {
        return;
    }

    if (!load_sample(t, path, NULL, 0)) {
        snprintf(t->path, sizeof(t->path), "%s", path);   // shown as Missing; the project still names it
        install_sample(t, NULL);
    }
}

static void os_prepare(void * inst, const tSynthLibSetup * setup) {
    tOsPlugin * t = (tOsPlugin *)inst;

    t->hostRate        = setup->sampleRate;
    t->voices.hostRate = setup->sampleRate;
}

static void os_set_active(void * inst, bool active) {
    tOsPlugin * t = (tOsPlugin *)inst;

    (void)active;
    t->eventCount = 0;
    os_voices_stop(&t->voices);
}

// ---- playing ------------------------------------------------------------------------------------------------

static double value(const tOsPlugin * t, uint32_t id) {
    return atomic_load(&((tOsPlugin *)t)->values[id]);
}

static long step(const tOsPlugin * t, uint32_t id) {
    return lround(value(t, id) * gOsParams[id].stepCount);
}

static tOsPlay make_play(const tOsPlugin * t) {
    double values[kOsParamCount];

    for (uint32_t id = 0; id < kOsParamCount; id++) {
        values[id] = value(t, id);
    }

    return (tOsPlay){
        .tune     = (double)(step(t, kOsParamTune) - 24) + ((double)(step(t, kOsParamFine) - 50) / 100.0),
        .velocity = value(t, kOsParamVelocity),
        .keyTrack = step(t, kOsParamKeyTrack) > 0,
        .rootKey  = (int)step(t, kOsParamRootKey),
        .gate     = step(t, kOsParamMode) > 0,
        .mono     = step(t, kOsParamVoices) > 0,
        .pan      = (double)(step(t, kOsParamPan) - 64) / 64.0,
        .gain     = os_volume_gain(value(t, kOsParamVolume)),
        .env      = os_env_shape(values),
        .start    = os_start_time(value(t, kOsParamStart)),
        .snap     = step(t, kOsParamSnap) > 0,
        .reverse  = step(t, kOsParamReverse) > 0,
        .protect  = os_protect_time(value(t, kOsParamProtect)),
        .end      = os_end_fraction(value(t, kOsParamEnd)),
        .delay    = os_delay_time(value(t, kOsParamDelay)),
    };
}

// notes §5: Stretch, times the base tempo over the host's with Sync on; the envelope's times follow it
static void apply_stretch(const tOsPlugin * t, tOsPlay * p) {
    double f = os_stretch_multiple(value(t, kOsParamStretch));

    if (step(t, kOsParamSync) > 0) {
        f *= os_base_tempo(value(t, kOsParamBaseTempo)) / t->hostTempo;
    }
    p->stretch = f;

    if (f != 1.0) {
        p->env.attack  *= f;
        p->env.decay   *= f;
        p->env.release *= f;
        p->env.hold     = (p->env.hold < 0.0) ? p->env.hold : p->env.hold * f;
    }
}

static void play(tOsPlugin * t, const tOsSample * sample, const tOsPlay * p, const tOsEvent * e) {
    if (e->on) {
        os_voices_note_on(&t->voices, sample, p, e->note, e->velocity);
    } else {
        os_voices_note_off(&t->voices, p, e->note);
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

static void os_process(void * inst, const float *const * in, uint32_t numIn, float ** out, uint32_t numOut,
                       uint32_t frames, const tSynthLibTransport * transport) {
    tOsPlugin *       t          = (tOsPlugin *)inst;
    uint32_t          generation = atomic_load(&t->generation);
    const tOsSample * sample     = atomic_load(&t->sample);
    tOsPlay           p;

    (void)in;
    (void)numIn;

    if ((transport != NULL) && transport->tempoValid && (transport->tempo > 0.0)) {
        t->hostTempo = transport->tempo;
    }
    p = make_play(t);
    apply_stretch(t, &p);

    // notes §1: a new sample ends what the old one was playing, with a short fade
    if (sample != t->playing) {
        os_voices_fade_all(&t->voices);
        t->playing = sample;
    }

    // notes §4: the root key's copy built before its first note - the only one, with Key Track off. A change of
    // the host's tempo alone asks for nothing: the next note does, so instances do not all rebuild at once.
    if (sample != NULL) {
        tOsCacheRequest want       = {os_voices_key(&t->voices, sample, &p, (uint8_t)p.rootKey), !p.keyTrack};
        tOsCacheKey     was        = t->prefetched.key;
        double          setting[3] = {value(t, kOsParamStretch), value(t, kOsParamSync), value(t, kOsParamBaseTempo)};
        bool            changed    = (want.key.serial != was.serial) || (want.key.reverse != was.reverse)
                                     || (want.key.step != was.step) || (want.key.hostRate != was.hostRate)
                                     || (want.key.protect != was.protect) || (want.only != t->prefetched.only)
                                     || (memcmp(setting, t->prefetchedSetting, sizeof(setting)) != 0)
                                     || ((want.key.stretch == 1.0) != (was.stretch == 1.0));

        if (changed) {
            os_cache_request(&t->cache, want.key, want.only);
            t->prefetched = want;
            memcpy(t->prefetchedSetting, setting, sizeof(setting));
        }
    }

    if (atomic_exchange(&t->audition, false) && (sample != NULL)) {
        if (p.mono) {
            os_voices_fade_all(&t->voices);
        }
        os_voices_note_on(&t->voices, sample, &p, (uint8_t)p.rootKey, AUDITION_VELOCITY);
    }

    if ((out == NULL) || (numOut == 0) || (out[0] == NULL)) {
        for (uint32_t i = 0; i < t->eventCount; i++) {
            play(t, sample, &p, &t->events[i]);
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

    // up to each note, then the note; the host sends them in time order
    for (uint32_t i = 0; i < t->eventCount; i++) {
        uint32_t at = (t->events[i].offset < frames) ? t->events[i].offset : frames;

        if (at > done) {
            os_voices_render(&t->voices, &p, left + done, (right != NULL) ? right + done : NULL, at - done);
            done = at;
        }
        play(t, sample, &p, &t->events[i]);
    }

    t->eventCount = 0;

    if (done < frames) {
        os_voices_render(&t->voices, &p, left + done, (right != NULL) ? right + done : NULL, frames - done);
    }

    for (uint32_t ch = 2; ch < numOut; ch++) {
        if (out[ch] != NULL) {
            memset(out[ch], 0, (size_t)frames * sizeof(float));
        }
    }

    if (!os_voices_use_other_than(&t->voices, sample)) {
        atomic_store(&t->acknowledged, generation);
    }
}

static void os_set_param(void * inst, uint32_t id, double normalized) {
    if (id < kOsParamCount) {
        atomic_store(&((tOsPlugin *)inst)->values[id], normalized);
    }
}

static double os_get_param(void * inst, uint32_t id) {
    return (id < kOsParamCount) ? value((tOsPlugin *)inst, id) : 0.0;
}

static bool os_param_text_cb(const tSynthLibPluginDesc * desc, void * inst, uint32_t id, double normalized, char * out, size_t len) {
    (void)desc;
    (void)inst;

    if (id >= kOsParamCount) {
        return false;
    }
    os_param_text(id, normalized, out, len);
    return true;
}

// ---- the editor ---------------------------------------------------------------------------------------------

static bool host_load(void * user, const char * path, char * error, size_t errorLength) {
    return load_sample((tOsPlugin *)user, path, error, errorLength);
}

static const tOsSample * host_sample(void * user) {
    return atomic_load(&((tOsPlugin *)user)->sample);
}

static const char * host_path(void * user) {
    return ((tOsPlugin *)user)->path;
}

static void host_audition(void * user) {
    atomic_store(&((tOsPlugin *)user)->audition, true);
}

// Through the host, as an edit of each, so it sees the change (and can undo it)
static void host_reset(void * user) {
    for (uint32_t id = 0; id < kOsParamCount; id++) {
        synthlib_plugin_param_edited(user, id, gOsParams[id].defaultNormalized);
    }
}

static const tOsSampleHost gSampleHost = {
    .load = host_load, .sample = host_sample, .path = host_path, .audition = host_audition, .reset = host_reset,
};

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
    free_acknowledged(t);

    for (uint32_t id = 0; id < kOsParamCount; id++) {
        os_draw_set_value(id, synthlib_plugin_param_value(user, id));
    }
}

static void os_panel_frame(void * user, int pixelWidth, int pixelHeight) {
    (void)user;
    os_draw_frame(pixelWidth, pixelHeight);
}

static bool os_panel_press(void * user, double x, double y) {
    tOsEdit edit;
    bool    grabbed = false;

    (void)os_draw_press(x, y, &edit, &grabbed);

    if (edit.paramId != UINT32_MAX) {
        synthlib_plugin_param_edited(user, edit.paramId, edit.normalized);
    }
    return grabbed;
}

static bool os_panel_keeps_pointer(void * user) {
    (void)user;
    return os_dial_mode() == eDialModeRotary;
}

static void os_panel_drag_to(void * user, double x, double y) {
    tOsEdit edit;

    if (os_draw_drag_to(x, y, &edit)) {
        synthlib_plugin_param_edited(user, edit.paramId, edit.normalized);
    }
}

static void os_panel_drag_by(void * user, double dx, double dy) {
    tOsEdit edit;

    if (os_draw_drag_by(dx, dy, &edit)) {
        synthlib_plugin_param_edited(user, edit.paramId, edit.normalized);
    }
}

static void os_panel_release(void * user, double x, double y) {
    tOsEdit edit;

    os_draw_release(x, y, &edit);

    if (edit.paramId != UINT32_MAX) {
        synthlib_plugin_param_edited(user, edit.paramId, edit.normalized);
    }
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

    .params             = gOsParams,
    .numParams          = kOsParamCount,

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
    return &gSet;
}

SYNTHLIB_PLUGIN_REGISTER(plugin_variants, "oneshot")
