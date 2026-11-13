/*
 * ModAlike - a Phaser, Chorus and Flanger as an effect plug-in.
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
#include "synthlibLog.h"
#include "synthlibPanelView.h"
#include "maDraw.h"
#include "maPrefs.h"
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

#define MA_MAKEUP             (1.0 / 0.5894) // notes §4 - 2-In -> 2-Out loses 4.6 dB
#define MA_APPLY_POLL_US      (5000)         // notes §2
#define MA_START_RATE         (44100.0)
#define MA_START_FRAMES       (1024)
#define MA_CHUNK              (256)          // frames processed at a time, on the stack
#define MA_DELAY_MAX          (64)           // notes §11 - the dry paths' delay line, past the engine's 31
#define MA_LINK_REPEATS       (40)           // notes §14 - polls (200 ms) the link is asked for after a change...
#define MA_LINK_EVERY         (10)           // ...on every tenth (50 ms), so one lands after both slots have the new patch
#define MA_LINK_AFTER_RATE    MA_LINK_REPEATS

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
    kStereo,           // notes §12
    kOffset,
    kChorusInput,
    kMix,
    kEconomy,          // notes §16
    kFixedPoint,       // notes §17
    kOutputStage,
    kAntiAlias,
    kParamCount
};

enum {
    eStereoLinked = 0,
    eStereoWide,
    eStereoMidSide,
    eStereoCount
};

enum {
    eEffectPhaser = 0,
    eEffectChorus,
    eEffectFlanger,
    eEffectCount
};

#define DIAL(v)    ((double)(v) / 127.0)

// Defaults are the modules' own
static const tSynthLibParam gParams[kParamCount] = {
    {kEffect,          "Effect",           "Effect",     eSynthLibUnitIndexed,    0.0,   2.0,         0.5,   2, SYNTHLIB_MIDI_NONE, SYNTHLIB_PARAM_LIST},   // the Chorus
    {kPhaserType,      "Phaser Type",      "Type",       eSynthLibUnitIndexed,    0.0,   1.0,         0.0,   1, SYNTHLIB_MIDI_NONE, SYNTHLIB_PARAM_LIST},
    {kPhaserRate,      "Phaser Rate",      "Rate",       eSynthLibUnitGeneric,    0.0, 127.0, DIAL(64),    127, SYNTHLIB_MIDI_NONE,                   0},
    {kPhaserFeedback,  "Phaser Feedback",  "FB",         eSynthLibUnitGeneric,    0.0, 127.0, DIAL(0),     127, SYNTHLIB_MIDI_NONE,                   0},
    {kChorusDetune,    "Chorus Detune",    "Detune",     eSynthLibUnitGeneric,    0.0, 127.0, DIAL(50),    127, SYNTHLIB_MIDI_NONE,                   0},
    {kChorusAmount,    "Chorus Amount",    "Amount",     eSynthLibUnitGeneric,    0.0, 127.0, DIAL(127),   127, SYNTHLIB_MIDI_NONE,                   0},
    {kFlangerRate,     "Flanger Rate",     "Rate",       eSynthLibUnitGeneric,    0.0, 127.0, DIAL(64),    127, SYNTHLIB_MIDI_NONE,                   0},
    {kFlangerRange,    "Flanger Range",    "Range",      eSynthLibUnitGeneric,    0.0, 127.0, DIAL(64),    127, SYNTHLIB_MIDI_NONE,                   0},
    {kFlangerFeedback, "Flanger Feedback", "Feedback",   eSynthLibUnitGeneric,    0.0, 127.0, DIAL(64),    127, SYNTHLIB_MIDI_NONE,                   0},
    {kOutput,          "Output",           "Output",     eSynthLibUnitDecibels, -24.0,  12.0, 24.0 / 36.0,   0, SYNTHLIB_MIDI_NONE,                   0},
    {kStereo,          "Stereo",           "Stereo",     eSynthLibUnitIndexed,    0.0,   2.0,         0.0,   2, SYNTHLIB_MIDI_NONE, SYNTHLIB_PARAM_LIST},
    {kOffset,          "LFO Offset",       "Offset",     eSynthLibUnitGeneric,    0.0, 180.0,         0.5, 180, SYNTHLIB_MIDI_NONE,                   0},
    {kChorusInput,     "Chorus Input",     "Input",      eSynthLibUnitIndexed,    0.0,   1.0,         0.0,   1, SYNTHLIB_MIDI_NONE, SYNTHLIB_PARAM_LIST},
    {kMix,             "Mix",              "Mix",        eSynthLibUnitPercent,    0.0, 100.0,         1.0, 100, SYNTHLIB_MIDI_NONE,                   0},
    {kEconomy,         "Engine Rate",      "Rate",       eSynthLibUnitIndexed,    0.0,   1.0,         0.0,   1, SYNTHLIB_MIDI_NONE, SYNTHLIB_PARAM_LIST},
    {kFixedPoint,      "Maths",            "Maths",      eSynthLibUnitIndexed,    0.0,   1.0,         0.0,   1, SYNTHLIB_MIDI_NONE, SYNTHLIB_PARAM_LIST},
    {kOutputStage,     "Output Stage",     "Stage",      eSynthLibUnitIndexed,    0.0,   1.0,         0.0,   1, SYNTHLIB_MIDI_NONE, SYNTHLIB_PARAM_LIST},
    {kAntiAlias,       "Anti-alias",       "Anti-alias", eSynthLibUnitIndexed,    0.0,   1.0,         1.0,   1, SYNTHLIB_MIDI_NONE, SYNTHLIB_PARAM_LIST},
};

static const char *         kEffectName[eEffectCount] = {"Phaser", "Chorus", "Flanger"};
static const char *         kPhaserTypeName[2]        = {"Type I", "Type II"};
static const char *         kStereoName[eStereoCount] = {"Linked", "Wide", "Mid/Side"};
static const char *         kChorusInputName[2]       = {"Stereo", "Mono"};
static const char *         kEconomyName[2]           = {"96 kHz", "Host rate (no latency)"};
static const char *         kFixedName[2]             = {"Floating point", "Fixed point"};
static const char *         kStageName[2]             = {"Off", "On"};
static const char *         kOffOnName[2]             = {"Off", "On"};
// the panel's buttons are narrow: short names there, the full ones in the host's parameter list
static const char *         kEconomyShort[2]          = {"96 kHz", "Host rate"};
static const char *         kFixedShort[2]            = {"Float", "Fixed"};
static const char *         kStageShort[2]            = {"Off", "On"};

// ---- The panel's layout - notes §15 ----------------------------------------------------------------------------

#define DIAL_127(id, label)    {id, label, eMaControlDial, 127, 0.0, 1.0, "%.0f", NULL, 0, 0, 0}

static const tMaControl     kEffectControls[]         = {
    {kEffect, "Effect", eMaControlChoice, 2, 0.0, 1.0, NULL, kEffectName, 0, 0, 0},
};
static const tMaControl     kPhaserControls[]         = {
    {kPhaserType, "Type", eMaControlChoice, 1, 0.0, 1.0, NULL, kPhaserTypeName, 0, 0, 0},
    DIAL_127(kPhaserRate, "Rate"),
    DIAL_127(kPhaserFeedback, "Feedback"),
};
static const tMaControl     kChorusControls[]         = {
    DIAL_127(kChorusDetune, "Detune"),
    DIAL_127(kChorusAmount, "Amount"),
    {kChorusInput,          "Input",  eMaControlChoice,1, 0.0, 1.0, NULL, kChorusInputName, 0, 0, 0},
};
static const tMaControl     kFlangerControls[]        = {
    DIAL_127(kFlangerRate,     "Rate"),
    DIAL_127(kFlangerRange,    "Range"),
    DIAL_127(kFlangerFeedback, "Feedback"),
};
static const tMaControl     kStereoControls[]         = {
    {kStereo, "Mode",       eMaControlChoice,   2, 0.0, 1.0, NULL,       kStereoName, 0, 0, 0},
    {kOffset, "LFO Offset", eMaControlDial,   180, 0.0, 1.0, "%.0f deg", NULL,        0, 0, 0},
};
static const tMaControl     kOutputControls[]         = {
    {kMix,    "Mix",    eMaControlDial, 100,   0.0, 1.0, "%.0f%%",   NULL, 0, 0, 0},
    {kOutput, "Output", eMaControlDial,  72, -24.0, 0.5, "%+.1f dB", NULL, 0, 0, 0},
};

static const tMaControl     kEngineControls[]         = {
    {kEconomy,     "Rate",       eMaControlChoice, 1, 0.0, 1.0, NULL, kEconomyShort,        0, 0, 0},
    {kFixedPoint,  "Maths",      eMaControlChoice, 1, 0.0, 1.0, NULL, kFixedShort,          0, 0, 0},
    {kOutputStage, "Stage",      eMaControlChoice, 1, 0.0, 1.0, NULL, kStageShort,          0, 0, 0},
    {kAntiAlias,   "Anti-alias", eMaControlChoice, 1, 0.0, 1.0, NULL, kOffOnName,    kEconomy, 1, 1},  // only at the host rate
};

#define COUNT(a)    ((uint32_t)(sizeof(a) / sizeof((a)[0])))

// onlyWhen: the Effect entry a section belongs to, so the other two show dimmed
static const tMaSection     kSections[]               = {
    {"EFFECT",  {0.75, 0.75, 0.78}, kEffectControls,  COUNT(kEffectControls),  false, -1},
    {"PHASER",  {0.90, 0.60, 0.30}, kPhaserControls,  COUNT(kPhaserControls),  false,  0},
    {"CHORUS",  {0.40, 0.70, 0.90}, kChorusControls,  COUNT(kChorusControls),  false,  1},
    {"FLANGER", {0.60, 0.85, 0.40}, kFlangerControls, COUNT(kFlangerControls), false,  2},
    {"STEREO",  {0.75, 0.55, 0.90}, kStereoControls,  COUNT(kStereoControls),  true,  -1},
    {"OUTPUT",  {0.85, 0.85, 0.85}, kOutputControls,  COUNT(kOutputControls),  false, -1},
    {"ENGINE",  {0.85, 0.75, 0.45}, kEngineControls,  COUNT(kEngineControls),  false, -1},
};

// ---- The patch - notes §1 ------------------------------------------------------------------------------------

// Slot A plays the left side, slot B the right, each with its module at the same index: the engine draws an LFO's
// start phase from a seed every slot has its own copy of, so the two sides start, and stay, in step (notes §1)
#define MOD_IN          (1)
#define MOD_EFFECT      (2)   // the effect, one a slot: slot A the left, slot B the right; a mono-in Chorus in slot A only
#define MOD_MIX         (3)   // the Chorus's mono sum
#define MOD_OUT         (8)

#define IN_PAD_PARAM    (2)   // 2-In Pad: 0 is the loudest
#define OUT_L           (0)
#define OUT_R           (1)

static const char *         kModuleType[eEffectCount] = {"Phaser", "StChorus", "Flanger"};

// Which module parameter each plug-in parameter is, in its effect's module(s)
typedef struct {
    int effect;
    int param;
    int steps;    // 127 for a dial, n-1 for a list
} tTarget;

static const tTarget        kTarget[kParamCount]      = {
    [kPhaserType]      = {eEffectPhaser,  0,   1},
    [kPhaserRate]      = {eEffectPhaser,  1, 127},
    [kPhaserFeedback]  = {eEffectPhaser,  2, 127},
    [kChorusDetune]    = {eEffectChorus,  0, 127},
    [kChorusAmount]    = {eEffectChorus,  1, 127},
    [kFlangerRate]     = {eEffectFlanger, 0, 127},
    [kFlangerRange]    = {eEffectFlanger, 1, 127},
    [kFlangerFeedback] = {eEffectFlanger, 2, 127},
};

// A short delay, for the dry paths beside the engine (notes §11)
typedef struct {
    float    ring[MA_DELAY_MAX];
    uint32_t write;
} tDelay;

static float delay_step(tDelay * d, float x, uint32_t frames) {
    d->ring[d->write] = x;
    float y = d->ring[(d->write + MA_DELAY_MAX - frames) % MA_DELAY_MAX];

    d->write          = (d->write + 1u) % MA_DELAY_MAX;
    return y;
}

typedef struct {
    g2am_engine *    engine;
    _Atomic double   value[kParamCount];
    _Atomic uint32_t dirty;            // a bit per parameter
    _Atomic uint32_t latency;          // the engine's, at the current rate
    tDelay           dryL, dryR, side; // audio thread only
    double           aa[5];            // notes §18 - the anti-alias low-pass, b0 b1 b2 a1 a2; set in prepare
    double           aaState[2][2];    // audio thread only, a side each
    int              built;            // the effect the patch holds; apply thread only
    int              builtMonoChorus;
    int              linkPending;      // links still to make; apply thread only
    pthread_mutex_t  lock;             // the engine's control calls
    pthread_t        applyThread;
    bool             applyRunning;
    atomic_bool      applyStop;
} tModAlike;

static int effect_of(tModAlike * m) {
    long e = lround(atomic_load(&m->value[kEffect]) * (eEffectCount - 1));

    return (e < 0) ? 0 : ((e >= eEffectCount) ? (eEffectCount - 1) : (int)e);
}

static int list_of(tModAlike * m, int id, int count) {
    long v = lround(atomic_load(&m->value[id]) * (count - 1));

    return (v < 0) ? 0 : ((v >= count) ? (count - 1) : (int)v);
}

static int wire_value(tModAlike * m, int id) {
    return (int)lround(atomic_load(&m->value[id]) * kTarget[id].steps);
}

// notes §10 - a refusal here is a fault in this file; it is logged, and the patch built as far as it goes
static void check(tModAlike * m, int result, const char * what) {
    if (result < 0) {
        synthlib_log_line("ModAlike: %s refused: %s", what, g2am_last_error(m->engine));
    }
}

static void cable(tModAlike * m, int slot, int from, int fromOut, int to, int toIn) {
    check(m, g2am_cable_add(m->engine, slot, "fx", from, fromOut, 1, to, toIn), "cable");
}

// notes §13 - one module a side, except a Chorus asked to take a mono sum, as the module was made to
static int sides(tModAlike * m, int effect) {
    return ((effect == eEffectChorus) && (list_of(m, kChorusInput, 2) == 1)) ? 1 : 2;
}

static void apply_param(tModAlike * m, int id) {
    const tTarget * t = &kTarget[id];

    if ((t->steps == 0) || (t->effect != m->built)) {
        return;   // Effect and Output, or another effect's setting: kept, applied when that effect is built
    }

    for (int slot = 0; slot < sides(m, t->effect); slot++) {
        check(m, g2am_param_set(m->engine, slot, "fx", MOD_EFFECT, t->param, 0, wire_value(m, id)), "parameter");
    }
}

// One slot's patch: In -> effect -> Out. For a two-sided effect, `slot` picks the side.
static void build_slot(tModAlike * m, int slot, int effect) {
    g2am_engine * e    = m->engine;
    int           side = (sides(m, effect) == 2) ? slot : -1;

    check(m, g2am_clear(e, slot), "clear");
    check(m, g2am_module_add(e, slot, "fx", MOD_IN, "2-In"), "2-In");
    check(m, g2am_param_set(e, slot, "fx", MOD_IN, IN_PAD_PARAM, 0, 0), "pad");
    check(m, g2am_module_add(e, slot, "fx", MOD_OUT, "2-Out"), "2-Out");
    check(m, g2am_module_add(e, slot, "fx", MOD_EFFECT, kModuleType[effect]), kModuleType[effect]);

    if (side < 0) {   // notes §5 - the Chorus: mono in, as the module is, L+R summed at half each; stereo out
        check(m, g2am_module_add(e, slot, "fx", MOD_MIX, "Mix2-1A"), "Mix2-1A");
        cable(m, slot, MOD_IN, OUT_L, MOD_MIX, 0);
        cable(m, slot, MOD_IN, OUT_R, MOD_MIX, 1);
        cable(m, slot, MOD_MIX, 0, MOD_EFFECT, 0);
        cable(m, slot, MOD_EFFECT, OUT_L, MOD_OUT, 0);
        cable(m, slot, MOD_EFFECT, OUT_R, MOD_OUT, 1);
    } else {   // a Chorus a side takes only its own side's output: left from slot A, right from slot B
        cable(m, slot, MOD_IN, side, MOD_EFFECT, 0);
        cable(m, slot, MOD_EFFECT, (effect == eEffectChorus) ? side : 0, MOD_OUT, side);
    }
}

// The whole patch for one effect; the caller commits
static void build(tModAlike * m, int effect) {
    int two = (sides(m, effect) == 2);

    build_slot(m, 0, effect);

    if (two) {
        build_slot(m, 1, effect);
    } else {
        check(m, g2am_clear(m->engine, 1), "clear");
    }
    check(m, g2am_performance_setting(m->engine, "enabled_b", two ? 1 : 0), "enabled_b");
    m->built           = effect;
    m->builtMonoChorus = !two;
    m->linkPending     = MA_LINK_REPEATS;   // a new patch starts both sides in step; Wide moves the right

    for (int id = 0; id < kParamCount; id++) {
        apply_param(m, id);
    }
}

// notes §2 - parameters reach the engine from here, never from the audio thread
// notes §17 - the output stage, never at the host rate: it cannot be placed right below 88.2 kHz
static void apply_output_stage(tModAlike * m) {
    int on = (list_of(m, kOutputStage, 2) == 1) && (list_of(m, kEconomy, 2) == 0);

    check(m, g2am_set_output_stage(m->engine, on), "output stage");
}

// notes §18 - a two-pole Butterworth low-pass at 18 kHz (no higher than 0.4 of the rate), the RBJ form
static void set_anti_alias(tModAlike * m, double rate) {
    double fc    = fmin(18000.0, 0.4 * rate);
    double w     = 2.0 * M_PI * fc / rate;
    double alpha = sin(w) / (2.0 * M_SQRT1_2);
    double a0    = 1.0 + alpha;

    m->aa[0] = ((1.0 - cos(w)) / 2.0) / a0;
    m->aa[1] = (1.0 - cos(w)) / a0;
    m->aa[2] = m->aa[0];
    m->aa[3] = (-2.0 * cos(w)) / a0;
    m->aa[4] = (1.0 - alpha) / a0;
    memset(m->aaState, 0, sizeof(m->aaState));
}

static void anti_alias(tModAlike * m, int side, const float * in, float * out, uint32_t frames) {
    double * z = m->aaState[side];

    for (uint32_t i = 0; i < frames; i++) {
        double x = in[i];
        double y = (m->aa[0] * x) + z[0];

        z[0]   = (m->aa[1] * x) - (m->aa[3] * y) + z[1];
        z[1]   = (m->aa[2] * x) - (m->aa[4] * y);
        out[i] = (float)y;
    }
}

// The engine's latency, kept for the audio thread's dry paths; true when it moved
static bool update_latency(tModAlike * m) {
    uint32_t latency = (uint32_t)g2am_latency(m->engine);

    return atomic_exchange(&m->latency, latency) != latency;
}

static void apply_pending(tModAlike * m) {
    uint32_t bits           = atomic_exchange(&m->dirty, 0u);
    bool     latencyChanged = false;

    if ((bits == 0u) && (m->linkPending == 0)) {
        return;
    }
    pthread_mutex_lock(&m->lock);

    if (bits != 0u) {
        int effect = effect_of(m);
        int mono   = (sides(m, effect) == 1);

        if ((effect != m->built) || (mono != m->builtMonoChorus)) {
            build(m, effect);
        } else {
            for (int id = 0; id < kParamCount; id++) {
                if ((bits & (1u << id)) != 0u) {
                    apply_param(m, id);
                }
            }
        }

        if ((bits & ((1u << kStereo) | (1u << kOffset))) != 0u) {
            m->linkPending = MA_LINK_REPEATS;
        }
        g2am_commit(m->engine);

        // notes §16 - economy: the host's rate, no resampling and so no latency; the patch's state is cleared
        if ((bits & (1u << kEconomy)) != 0u) {
            check(m, g2am_set_economy(m->engine, list_of(m, kEconomy, 2)), "economy");
            m->linkPending = MA_LINK_AFTER_RATE;
            latencyChanged = update_latency(m);
        }

        if ((bits & (1u << kFixedPoint)) != 0u) {   // notes §17
            check(m, g2am_set_fixed_point(m->engine, list_of(m, kFixedPoint, 2)), "fixed point");
        }

        if ((bits & ((1u << kOutputStage) | (1u << kEconomy))) != 0u) {
            apply_output_stage(m);
        }
    }

    // notes §14 - Wide: the right side's LFO behind the left by the offset; otherwise in step
    if ((m->linkPending > 0) && (m->built != eEffectChorus)) {
        double offset = (list_of(m, kStereo, eStereoCount) == eStereoWide) ? (atomic_load(&m->value[kOffset]) / 2.0) : 0.0;

        if ((m->linkPending % MA_LINK_EVERY) == 0) {
            check(m, g2am_lfo_link(m->engine, 1, "fx", MOD_EFFECT, 0, offset), "LFO link");
        }
        m->linkPending--;
    } else {
        m->linkPending = 0;
    }
    pthread_mutex_unlock(&m->lock);

    if (latencyChanged) {
        synthlib_plugin_latency_changed(m);   // any thread
    }
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
    g2am_config config = {G2AM_API_VERSION, MA_START_RATE, MA_START_FRAMES, 1};   // extensions: g2am_lfo_link

    (void)desc;

    if (m == NULL) {
        return NULL;
    }
    ma_prefs_init();   // the dial mode, before the panel opens

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
    apply_output_stage(m);   // the library's default is on; ModAlike's is off
    set_anti_alias(m, MA_START_RATE);
    atomic_store(&m->latency, (uint32_t)g2am_latency(m->engine));
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
    tModAlike * m       = (tModAlike *)inst;

    pthread_mutex_lock(&m->lock);
    g2am_prepare(m->engine, setup->sampleRate, (int)((setup->maxFrames > 0) ? setup->maxFrames : MA_START_FRAMES));
    set_anti_alias(m, setup->sampleRate);
    uint32_t    latency = (uint32_t)g2am_latency(m->engine);

    m->linkPending = MA_LINK_REPEATS;   // a restarted engine starts its LFOs again
    pthread_mutex_unlock(&m->lock);

    if (atomic_exchange(&m->latency, latency) != latency) {
        synthlib_plugin_latency_changed(inst);   // notes §3 - none at 88.2 kHz and above
    }
}

static void ma_reset(void * inst) {
    g2am_all_notes_off(((tModAlike *)inst)->engine);
}

static uint32_t ma_latency(void * inst) {
    return atomic_load(&((tModAlike *)inst)->latency);
}

// ---- Audio -----------------------------------------------------------------------------------------------------

// notes §11 - Mid/Side splits the input around the engine; Mix blends a dry copy back. Both dry paths are delayed by
// the engine's latency so they line up with what comes out of it.
static void process_chunk(tModAlike * m, const float * inL, const float * inR, float * outL, float * outR, uint32_t frames) {
    float         mid[MA_CHUNK], side[MA_CHUNK], wetL[MA_CHUNK], wetR[MA_CHUNK];
    bool          midSide = (list_of(m, kStereo, eStereoCount) == eStereoMidSide);
    uint32_t      latency = atomic_load(&m->latency);
    const float * jack[2] = {inL, inR};
    float *       wet[2]  = {wetL, wetR};

    if ((inL != NULL) && midSide) {
        for (uint32_t i = 0; i < frames; i++) {
            mid[i]  = 0.5f * (inL[i] + inR[i]);
            side[i] = 0.5f * (inL[i] - inR[i]);
        }

        jack[0] = jack[1] = mid;
    }
    // notes §18 - at the host rate, Anti-alias filters what goes into the effect; the dry paths stay as they came
    float         aaL[MA_CHUNK], aaR[MA_CHUNK];

    if ((inL != NULL) && (list_of(m, kEconomy, 2) == 1) && (list_of(m, kAntiAlias, 2) == 1)) {
        bool same = (jack[0] == jack[1]);

        anti_alias(m, 0, jack[0], aaL, frames);

        if (!same) {
            anti_alias(m, 1, jack[1], aaR, frames);
        }
        jack[0] = aaL;
        jack[1] = same ? aaL : aaR;
    }
    g2am_render(m->engine, jack, (inL != NULL) ? 2 : 0, wet, 2, (int)frames);

    double        db    = gParams[kOutput].plainMin + (atomic_load(&m->value[kOutput]) * (gParams[kOutput].plainMax - gParams[kOutput].plainMin));
    float         level = (float)pow(10.0, db / 20.0);
    float         mix   = (float)atomic_load(&m->value[kMix]);

    for (uint32_t i = 0; i < frames; i++) {
        float l     = (float)MA_MAKEUP * wetL[i];
        float r     = (float)MA_MAKEUP * wetR[i];
        float dryL  = delay_step(&m->dryL, (inL != NULL) ? inL[i] : 0.0f, latency);
        float dryR  = delay_step(&m->dryR, (inR != NULL) ? inR[i] : 0.0f, latency);
        float sideD = delay_step(&m->side, ((inL != NULL) && midSide) ? side[i] : 0.0f, latency);

        l      += sideD;    // zero unless Mid/Side
        r      -= sideD;
        outL[i] = level * ((mix * l) + ((1.0f - mix) * dryL));

        if (outR != NULL) {
            outR[i] = level * ((mix * r) + ((1.0f - mix) * dryR));
        }
    }
}

static void ma_process(void * inst, const float *const * in, uint32_t numIn, float ** out, uint32_t numOut,
                       uint32_t frames, const tSynthLibTransport * transport) {
    tModAlike *   m   = (tModAlike *)inst;
    const float * inL = NULL;
    const float * inR = NULL;

    (void)transport;

    if ((out == NULL) || (numOut == 0u)) {
        return;
    }

    if ((in != NULL) && (numIn > 0u)) {
        inL = in[0];
        inR = (numIn > 1u) ? in[1] : in[0];   // a mono track feeds both sides
    }

    for (uint32_t done = 0; done < frames; done += MA_CHUNK) {
        uint32_t n = ((frames - done) < MA_CHUNK) ? (frames - done) : MA_CHUNK;

        process_chunk(m, (inL != NULL) ? (inL + done) : NULL, (inR != NULL) ? (inR + done) : NULL,
                      out[0] + done, (numOut > 1u) ? (out[1] + done) : NULL, n);
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

    if (id == kStereo) {
        long v = lround(normalized * (eStereoCount - 1));

        snprintf(out, len, "%s", kStereoName[(v < 0) ? 0 : ((v >= eStereoCount) ? eStereoCount - 1 : v)]);
        return true;
    }

    if (id == kChorusInput) {
        snprintf(out, len, "%s", kChorusInputName[(normalized >= 0.5) ? 1 : 0]);
        return true;
    }

    if (id == kEconomy) {
        snprintf(out, len, "%s", kEconomyName[(normalized >= 0.5) ? 1 : 0]);
        return true;
    }

    if ((id == kFixedPoint) || (id == kOutputStage) || (id == kAntiAlias)) {
        const char *const * names = (id == kFixedPoint) ? kFixedName : ((id == kOutputStage) ? kStageName : kOffOnName);

        snprintf(out, len, "%s", names[(normalized >= 0.5) ? 1 : 0]);
        return true;
    }

    if (id == kOffset) {
        snprintf(out, len, "%ld deg", lround(normalized * 180.0));
        return true;
    }

    if ((id < kParamCount) && (kTarget[id].steps == 127)) {
        snprintf(out, len, "%ld", lround(normalized * 127.0));   // notes §7 - the modules' own 0-127, for now
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

// ---- The editor - notes §15: Nl3Alike's panel view, with ModAlike's sections ----------------------------------

static void ma_panel_init(void) {
    ma_draw_init();
    ma_draw_set_sections(kSections, COUNT(kSections), kEffect, eEffectCount - 1);
}

static void ma_panel_sync(void * user) {
    for (uint32_t id = 0; id < kParamCount; id++) {
        ma_draw_set_value(id, synthlib_plugin_param_value(user, id));
    }
}

static void ma_panel_frame(void * user, int pixelWidth, int pixelHeight) {
    (void)user;
    ma_draw_frame(pixelWidth, pixelHeight);
}

static bool ma_panel_press(void * user, double x, double y) {
    tMaEdit edit;
    bool    grabbed = false;

    (void)ma_draw_press(x, y, &edit, &grabbed);

    if (edit.paramId != UINT32_MAX) {
        synthlib_plugin_param_edited(user, edit.paramId, edit.normalized);
    }
    return grabbed;
}

static bool ma_panel_keeps_pointer(void * user) {
    (void)user;
    return ma_dial_mode() == eDialModeRotary;
}

static void ma_panel_drag_to(void * user, double x, double y) {
    tMaEdit edit;

    if (ma_draw_drag_to(x, y, &edit)) {
        synthlib_plugin_param_edited(user, edit.paramId, edit.normalized);
    }
}

static void ma_panel_drag_by(void * user, double dx, double dy) {
    tMaEdit edit;

    if (ma_draw_drag_by(dx, dy, &edit)) {
        synthlib_plugin_param_edited(user, edit.paramId, edit.normalized);
    }
}

static void ma_panel_release(void * user, double x, double y) {
    tMaEdit edit;

    ma_draw_release(x, y, &edit);

    if (edit.paramId != UINT32_MAX) {
        synthlib_plugin_param_edited(user, edit.paramId, edit.normalized);
    }
}

static bool ma_panel_key(void * user, int glfwKey, unsigned int codepoint, bool isRepeat) {
    (void)user;
    return ma_draw_key(glfwKey, codepoint, isRepeat);
}

static void ma_panel_scroll(void * user, double x, double y, double dy) {
    (void)user;
    (void)x;
    (void)y;
    ma_draw_scroll(dy);
}

static const tSynthLibPanel      gPanel = {
    .canvasWidth  = MA_CANVAS_W,
    .init         = ma_panel_init,
    .sync         = ma_panel_sync,
    .frame        = ma_panel_frame,
    .press        = ma_panel_press,
    .dragBy       = ma_panel_drag_by,
    .keepsPointer = ma_panel_keeps_pointer,
    .dragTo       = ma_panel_drag_to,
    .release      = ma_panel_release,
    .scroll       = ma_panel_scroll,
    .key          = ma_panel_key,
    .pointer      = ma_draw_set_mouse,
    .menuActive   = ma_draw_menu_active,
};

static void * ma_create_view(const tSynthLibPluginDesc * desc, void * inst, double width, double height) {
    (void)desc;
    return synthlib_panel_view_create(&gPanel, inst, width, height);
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
    .numInputs          =                         1,
    .outputs            = gOutputs,
    .numOutputs         =                         1,

    .vst3ProcessorUid   = gProcessorUid,
    .vst3ControllerUid  = gControllerUid,
    .auType             = MA_AU_TYPE,
    .auSubType          = MA_AU_SUBTYPE,
    .auManufacturer     = MA_AU_MANUFACTURER,
    .auVersion          = MA_AU_VERSION,
    .auBundleId         = MA_AU_BUNDLE_ID,

    .params             = gParams,
    .numParams          = kParamCount,

    .editorDefaultWidth = MA_CANVAS_W,
    .editorMinWidth     = MA_CANVAS_W * 0.75,
    .editorMaxWidth     = MA_CANVAS_W * 2.0,
    .editorAspect       = MA_CANVAS_W / MA_CANVAS_H,

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
        .createView     = ma_create_view,
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
