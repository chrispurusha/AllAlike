/*
 * ModAlike - G2Alike's Phaser, Chorus and Flanger as an effect plug-in.
 *
 * Copyright (C) 2026 Chris Turner <chris_purusha@icloud.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
// Notes: Docs/code-notes/maPlugin.md - "// notes §k" refers there.

#include <math.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "synthlibPlugin.h"
#include "g2alikemaths.h"

#ifndef MA_VERSION_STRING
#define MA_VERSION_STRING     "0.0.0"
#endif
#ifndef MA_AU_VERSION
#define MA_AU_VERSION         (0)
#endif
#define MA_AU_TYPE            ('aufx')
#define MA_AU_SUBTYPE         ('MdAl')
#define MA_AU_MANUFACTURER    ('CPur')
#define MA_AU_BUNDLE_ID       "com.chrispurusha.modalike.au"

#define MA_LATENCY            (31u)          // notes §3 - measured through 2-In -> 2-Out at 48 kHz
#define MA_MAKEUP             (1.0 / 0.5894) // notes §4 - 2-In -> 2-Out loses 4.6 dB
#define MA_APPLY_POLL_US      (5000)         // notes §2
#define MA_START_RATE         (44100.0)
#define MA_START_FRAMES       (1024)

static const uint8_t gProcessorUid[16]  = {0x28, 0x57, 0x73, 0x81, 0xCC, 0x0B, 0x44, 0x49, 0xB4, 0x87, 0xD2, 0xF3, 0x1A, 0x79, 0xCA, 0x9D};
static const uint8_t gControllerUid[16] = {0x94, 0x64, 0x07, 0x81, 0x4B, 0x91, 0x4A, 0x50, 0xB8, 0x7D, 0x8C, 0xDC, 0xB0, 0x46, 0x59, 0x42};

// ---- Parameters - ids are saved in projects and never change -------------------------------------------------

enum {
    kEffect = 0,
    kPhaserType,
    kPhaserRate,
    kPhaserFeedback,
    kChorusDetune,
    kChorusAmount,
    kFlangerRate,
    kFlangerRange,
    kFlangerFeedback,
    kOutput,
    kParamCount
};

enum {
    eEffectPhaser = 0,
    eEffectChorus,
    eEffectFlanger,
    eEffectCount
};

#define DIAL(v)    ((double)(v) / 127.0)

// Defaults are the G2 modules' own
static const tSynthLibParam gParams[kParamCount] = {
    {kEffect,          "Effect",           "Effect",   eSynthLibUnitIndexed,  0.0,   2.0, 0.0,              2, SYNTHLIB_MIDI_NONE, SYNTHLIB_PARAM_LIST},
    {kPhaserType,      "Phaser Type",      "Type",     eSynthLibUnitIndexed,  0.0,   1.0, 0.0,              1, SYNTHLIB_MIDI_NONE, SYNTHLIB_PARAM_LIST},
    {kPhaserRate,      "Phaser Rate",      "Rate",     eSynthLibUnitGeneric,  0.0, 127.0, DIAL(64),       127, SYNTHLIB_MIDI_NONE, 0},
    {kPhaserFeedback,  "Phaser Feedback",  "FB",       eSynthLibUnitGeneric,  0.0, 127.0, DIAL(0),        127, SYNTHLIB_MIDI_NONE, 0},
    {kChorusDetune,    "Chorus Detune",    "Detune",   eSynthLibUnitGeneric,  0.0, 127.0, DIAL(20),       127, SYNTHLIB_MIDI_NONE, 0},
    {kChorusAmount,    "Chorus Amount",    "Amount",   eSynthLibUnitGeneric,  0.0, 127.0, DIAL(127),      127, SYNTHLIB_MIDI_NONE, 0},
    {kFlangerRate,     "Flanger Rate",     "Rate",     eSynthLibUnitGeneric,  0.0, 127.0, DIAL(64),       127, SYNTHLIB_MIDI_NONE, 0},
    {kFlangerRange,    "Flanger Range",    "Range",    eSynthLibUnitGeneric,  0.0, 127.0, DIAL(64),       127, SYNTHLIB_MIDI_NONE, 0},
    {kFlangerFeedback, "Flanger Feedback", "Feedback", eSynthLibUnitGeneric,  0.0, 127.0, DIAL(64),       127, SYNTHLIB_MIDI_NONE, 0},
    {kOutput,          "Output",           "Output",   eSynthLibUnitDecibels, -24.0, 12.0, 24.0 / 36.0,      0, SYNTHLIB_MIDI_NONE, 0},
};

static const char * kEffectName[eEffectCount] = {"Phaser", "Chorus", "Flanger"};
static const char * kPhaserTypeName[2]        = {"Type I", "Type II"};

// ---- The patch - notes §1 ------------------------------------------------------------------------------------

#define MOD_IN          (1)
#define MOD_PHASER_L    (2)
#define MOD_PHASER_R    (3)
#define MOD_MIX         (4)
#define MOD_CHORUS      (5)
#define MOD_FLANGER_L   (6)
#define MOD_FLANGER_R   (7)
#define MOD_OUT         (8)

#define IN_PAD_PARAM    (2)   // 2-In Pad: 0 is the loudest
#define OUT_L           (0)
#define OUT_R           (1)

// Which G2 module parameter each plug-in parameter is, in its effect's module(s)
typedef struct {
    int effect;
    int param;
    int steps;    // 127 for a dial, n-1 for a list
} tTarget;

static const tTarget kTarget[kParamCount] = {
    [kPhaserType]      = {eEffectPhaser,  0,   1},
    [kPhaserRate]      = {eEffectPhaser,  1, 127},
    [kPhaserFeedback]  = {eEffectPhaser,  2, 127},
    [kChorusDetune]    = {eEffectChorus,  0, 127},
    [kChorusAmount]    = {eEffectChorus,  1, 127},
    [kFlangerRate]     = {eEffectFlanger, 0, 127},
    [kFlangerRange]    = {eEffectFlanger, 1, 127},
    [kFlangerFeedback] = {eEffectFlanger, 2, 127},
};

typedef struct {
    g2am_engine *    engine;
    _Atomic double   value[kParamCount];
    _Atomic uint32_t dirty;            // a bit per parameter
    int              built;            // the effect the patch holds; apply thread only
    pthread_mutex_t  lock;             // the engine's control calls
    pthread_t        applyThread;
    bool             applyRunning;
    atomic_bool      applyStop;
} tModAlike;

static int effect_of(tModAlike * m) {
    long e = lround(atomic_load(&m->value[kEffect]) * (eEffectCount - 1));

    return (e < 0) ? 0 : ((e >= eEffectCount) ? (eEffectCount - 1) : (int)e);
}

static int wire_value(tModAlike * m, int id) {
    return (int)lround(atomic_load(&m->value[id]) * kTarget[id].steps);
}

static void cable(tModAlike * m, int from, int fromOut, int to, int toIn) {
    g2am_cable_add(m->engine, 0, "fx", from, fromOut, 1, to, toIn);
}

static void apply_param(tModAlike * m, int id) {
    static const int modules[eEffectCount][2] = {
        {MOD_PHASER_L,  MOD_PHASER_R },
        {MOD_CHORUS,    0            },
        {MOD_FLANGER_L, MOD_FLANGER_R},
    };
    const tTarget *  t = &kTarget[id];

    if ((t->steps == 0) || (t->effect != m->built)) {
        return;   // Effect and Output, or another effect's setting: kept, applied when that effect is built
    }

    for (int side = 0; side < 2; side++) {
        if (modules[t->effect][side] != 0) {
            g2am_param_set(m->engine, 0, "fx", modules[t->effect][side], t->param, 0, wire_value(m, id));
        }
    }
}

// The whole patch for one effect; the caller commits
static void build(tModAlike * m, int effect) {
    g2am_engine * e = m->engine;

    g2am_clear(e, 0);
    g2am_module_add(e, 0, "fx", MOD_IN, "2-In");
    g2am_param_set(e, 0, "fx", MOD_IN, IN_PAD_PARAM, 0, 0);
    g2am_module_add(e, 0, "fx", MOD_OUT, "2-Out");

    switch (effect) {
        case eEffectChorus:   // notes §5 - mono in, as the G2's: L+R summed at half each
            g2am_module_add(e, 0, "fx", MOD_MIX, "Mix2-1A");
            g2am_module_add(e, 0, "fx", MOD_CHORUS, "Chorus");
            cable(m, MOD_IN, OUT_L, MOD_MIX, 0);
            cable(m, MOD_IN, OUT_R, MOD_MIX, 1);
            cable(m, MOD_MIX, 0, MOD_CHORUS, 0);
            cable(m, MOD_CHORUS, OUT_L, MOD_OUT, 0);
            cable(m, MOD_CHORUS, OUT_R, MOD_OUT, 1);
            break;
        case eEffectFlanger:
        case eEffectPhaser:
        default:
        {
            int         left  = (effect == eEffectFlanger) ? MOD_FLANGER_L : MOD_PHASER_L;
            int         right = (effect == eEffectFlanger) ? MOD_FLANGER_R : MOD_PHASER_R;
            const char *type  = (effect == eEffectFlanger) ? "Flanger" : "Phaser";

            g2am_module_add(e, 0, "fx", left, type);
            g2am_module_add(e, 0, "fx", right, type);
            cable(m, MOD_IN, OUT_L, left, 0);
            cable(m, MOD_IN, OUT_R, right, 0);
            cable(m, left, 0, MOD_OUT, 0);
            cable(m, right, 0, MOD_OUT, 1);
            break;
        }
    }

    m->built = effect;

    for (int id = 0; id < kParamCount; id++) {
        apply_param(m, id);
    }
}

// notes §2 - parameters reach the engine from here, never from the audio thread
static void apply_pending(tModAlike * m) {
    uint32_t bits = atomic_exchange(&m->dirty, 0u);

    if (bits == 0u) {
        return;
    }
    pthread_mutex_lock(&m->lock);
    int effect = effect_of(m);

    if (effect != m->built) {
        build(m, effect);
    } else {
        for (int id = 0; id < kParamCount; id++) {
            if ((bits & (1u << id)) != 0u) {
                apply_param(m, id);
            }
        }
    }

    g2am_commit(m->engine);
    pthread_mutex_unlock(&m->lock);
}

static void * apply_worker(void * arg) {
    tModAlike * m = (tModAlike *)arg;

    pthread_setname_np("ModAlike apply");

    while (!atomic_load(&m->applyStop)) {
        apply_pending(m);
        usleep(MA_APPLY_POLL_US);
    }

    return NULL;
}

// ---- Lifecycle -------------------------------------------------------------------------------------------------

static void * ma_create(const tSynthLibPluginDesc * desc) {
    tModAlike * m      = (tModAlike *)calloc(1, sizeof(tModAlike));
    g2am_config config = {G2AM_API_VERSION, MA_START_RATE, MA_START_FRAMES, 0};

    (void)desc;

    if (m == NULL) {
        return NULL;
    }
    m->engine = g2am_create(&config);

    if (m->engine == NULL) {   // notes §6 - at most 32 engines in one host process
        free(m);
        return NULL;
    }
    pthread_mutex_init(&m->lock, NULL);

    for (int id = 0; id < kParamCount; id++) {
        atomic_store(&m->value[id], gParams[id].defaultNormalized);
    }
    pthread_mutex_lock(&m->lock);
    build(m, effect_of(m));
    g2am_commit(m->engine);
    pthread_mutex_unlock(&m->lock);

    atomic_store(&m->applyStop, false);
    m->applyRunning = (pthread_create(&m->applyThread, NULL, apply_worker, m) == 0);
    return m;
}

static void ma_destroy(void * inst) {
    tModAlike * m = (tModAlike *)inst;

    if (m->applyRunning) {
        atomic_store(&m->applyStop, true);
        pthread_join(m->applyThread, NULL);
    }
    g2am_destroy(m->engine);
    pthread_mutex_destroy(&m->lock);
    free(m);
}

static void ma_prepare(void * inst, const tSynthLibSetup * setup) {
    tModAlike * m = (tModAlike *)inst;

    pthread_mutex_lock(&m->lock);
    g2am_prepare(m->engine, setup->sampleRate, (int)((setup->maxFrames > 0) ? setup->maxFrames : MA_START_FRAMES));
    pthread_mutex_unlock(&m->lock);
}

static void ma_reset(void * inst) {
    g2am_all_notes_off(((tModAlike *)inst)->engine);
}

static uint32_t ma_latency(void * inst) {
    (void)inst;
    return MA_LATENCY;
}

// ---- Audio -----------------------------------------------------------------------------------------------------

static void ma_process(void * inst, const float * const * in, uint32_t numIn, float ** out, uint32_t numOut,
                       uint32_t frames, const tSynthLibTransport * transport) {
    tModAlike *   m        = (tModAlike *)inst;
    const float * jack[2]  = {NULL, NULL};

    (void)transport;

    if ((out == NULL) || (numOut == 0u)) {
        return;
    }

    if ((in != NULL) && (numIn > 0u)) {
        jack[0] = in[0];
        jack[1] = (numIn > 1u) ? in[1] : in[0];   // a mono track feeds both sides
    }
    float * dest[2] = {out[0], (numOut > 1u) ? out[1] : NULL};

    g2am_render(m->engine, jack, 2, dest, 2, (int)frames);

    double db   = gParams[kOutput].plainMin + (atomic_load(&m->value[kOutput]) * (gParams[kOutput].plainMax - gParams[kOutput].plainMin));
    float  gain = (float)(MA_MAKEUP * pow(10.0, db / 20.0));

    for (uint32_t c = 0; c < numOut && c < 2u; c++) {
        for (uint32_t i = 0; i < frames; i++) {
            out[c][i] *= gain;
        }
    }
}

// ---- Parameters ------------------------------------------------------------------------------------------------

static void ma_set_param(void * inst, uint32_t id, double normalized) {
    tModAlike * m = (tModAlike *)inst;

    if (id >= kParamCount) {
        return;
    }
    normalized = (normalized < 0.0) ? 0.0 : ((normalized > 1.0) ? 1.0 : normalized);

    if (atomic_exchange(&m->value[id], normalized) != normalized) {
        atomic_fetch_or(&m->dirty, 1u << id);
    }
}

static double ma_get_param(void * inst, uint32_t id) {
    return (id < kParamCount) ? atomic_load(&((tModAlike *)inst)->value[id]) : 0.0;
}

static bool ma_param_text(const tSynthLibPluginDesc * desc, void * inst, uint32_t id, double normalized, char * out, size_t len) {
    (void)desc;
    (void)inst;

    if (id == kEffect) {
        long e = lround(normalized * (eEffectCount - 1));

        snprintf(out, len, "%s", kEffectName[(e < 0) ? 0 : ((e >= eEffectCount) ? eEffectCount - 1 : e)]);
        return true;
    }

    if (id == kPhaserType) {
        snprintf(out, len, "%s", kPhaserTypeName[(normalized >= 0.5) ? 1 : 0]);
        return true;
    }

    if ((id < kParamCount) && (kTarget[id].steps == 127)) {
        snprintf(out, len, "%ld", lround(normalized * 127.0));   // notes §7 - the G2's own 0-127, for now
        return true;
    }
    return false;
}

// The parameters are the whole state; the wrapper saves them. These four bytes mark ModAlike's own (notes §8).
#define MA_STATE_MAGIC    "MAS1"

static size_t ma_get_state(void * inst, void * out, size_t len) {
    (void)inst;

    if ((out != NULL) && (len >= 4)) {
        memcpy(out, MA_STATE_MAGIC, 4);
    }
    return 4;
}

static void ma_set_state(void * inst, const void * data, size_t len) {
    (void)inst;
    (void)data;
    (void)len;
}

// ---- The plug-in ------------------------------------------------------------------------------------------------

static const tSynthLibBus        gInputs[1]  = {{"In", 2, false, true}};
static const tSynthLibBus        gOutputs[1] = {{"Out", 2, false, true}};

static const tSynthLibPluginDesc gDescriptor = {
    .name               = "ModAlike",
    .vendor             = "Chris Purusha",
    .url                = "",
    .email              = "",
    .version            = MA_VERSION_STRING,

    .isInstrument       = false,
    .vst3SubCategory    = "Fx|Modulation",
    .inputs             = gInputs,
    .numInputs          = 1,
    .outputs            = gOutputs,
    .numOutputs         = 1,

    .vst3ProcessorUid   = gProcessorUid,
    .vst3ControllerUid  = gControllerUid,
    .auType             = MA_AU_TYPE,
    .auSubType          = MA_AU_SUBTYPE,
    .auManufacturer     = MA_AU_MANUFACTURER,
    .auVersion          = MA_AU_VERSION,
    .auBundleId         = MA_AU_BUNDLE_ID,

    .params             = gParams,
    .numParams          = kParamCount,

    .editorDefaultWidth = 0,   // notes §9 - no panel yet: the host's own

    .cb                 = {
        .create         = ma_create,
        .destroy        = ma_destroy,
        .prepare        = ma_prepare,
        .reset          = ma_reset,
        .latencySamples = ma_latency,
        .process        = ma_process,
        .setParam       = ma_set_param,
        .getParam       = ma_get_param,
        .paramText      = ma_param_text,
        .getState       = ma_get_state,
        .setState       = ma_set_state,
    }
};

static const tSynthLibPluginSet  gSet = {
    .variants = &gDescriptor,
    .count    = 1
};

static const tSynthLibPluginSet * plugin_variants(void) {
    return &gSet;
}

SYNTHLIB_PLUGIN_REGISTER(plugin_variants, "modalike")
