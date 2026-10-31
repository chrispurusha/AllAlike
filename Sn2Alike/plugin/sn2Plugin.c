/*
 * Sn2Alike - a reproduction inspired by the Novation Supernova II, as a VST3 and Audio Unit
 * instrument.
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
// Notes: Docs/code-notes/sn2Plugin.md - "// notes §k" refers there.

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "synthlibPlugin.h"
#include "synthlibLog.h"
#include "synthlibSwapFade.h"
#include "synthlibPanelView.h"
#include "synthlibTypes.h"
#include "inputState.h"
#include "sn2Draw.h"
#include "sn2Params.h"
#include "sn2Patch.h"
#include "sn2Prefs.h"
#include "sn2Voices.h"

// touch /tmp/sn2alike-log, read /tmp/sn2alike.log - see SynthLib's plugin/synthlibLog.h.

#ifndef SN2_VERSION_STRING
#define SN2_VERSION_STRING    "0.1.0"
#endif

#ifndef SN2_AU_VERSION
#define SN2_AU_VERSION    (0x00000100)
#endif

// THESE MAY NEVER CHANGE: a saved project finds the plug-in by them.
static const uint8_t gProcessorUid[16]  = {0xA3, 0xAF, 0x37, 0xA1, 0x94, 0xDF, 0x43, 0xD3, 0xB9, 0x53, 0xDF, 0x1E, 0x87, 0x62, 0xF5, 0xA2};
static const uint8_t gControllerUid[16] = {0x2B, 0xE4, 0x1E, 0xF9, 0xA5, 0x0F, 0x4C, 0xF3, 0xAF, 0x46, 0xF5, 0x6D, 0x8F, 0x77, 0x8D, 0x53};

// THESE MUST MATCH do-plugin - the bundle identifier especially
#define FOUR_CC(a, b, c, d)    (((uint32_t)(a) << 24) | ((uint32_t)(b) << 16) | ((uint32_t)(c) << 8) | (uint32_t)(d))
#define SN2_AU_TYPE            FOUR_CC('a', 'u', 'm', 'u')
#define SN2_AU_SUBTYPE         FOUR_CC('S', 'n', '2', 'a')
#define SN2_AU_MANUFACTURER    FOUR_CC('C', 'P', 'u', 'r')
#define SN2_AU_BUNDLE_ID       "com.chrispurusha.sn2alike.au"

// notes §2: a block's notes, played at their own sample when the block renders
#define MAX_EVENTS             (256)

typedef struct {
    uint32_t offset;
    uint8_t  note;
    bool     on;
    float    velocity;
} tSn2Event;

typedef struct {
    tSn2Voices        voices;
    tSn2Event         events[MAX_EVENTS];
    uint32_t          eventCount;
    double            values[kSn2ParamCount]; // normalized
    // notes §5: the sound, beyond its parameters - main thread, and the host's state calls
    char              soundName[SN2_SINGLE_NAME_LENGTH + 1];
    bool              haveBase;
    uint8_t           base[SN2_SINGLE_LENGTH]; // the Single it came from, whose bytes a save keeps
    char              patchPath[1024];
    int               patchIndex;              // its place in that file
    int               preset;                  // 1..SN2_PRESETS, 0 none
    double            presetsChecked;
    tSynthLibSwapFade fade;
    _Atomic bool      startPreset;  // notes: a new instance plays A001
    _Atomic int       wantPreset;   // a program change waiting for the fade
} tSn2Plugin;

static uint32_t voice_limit(const tSn2Plugin * t) {
    return 1u + (uint32_t)lround(t->values[kSn2ParamVoiceLimit] * (SN2_MAX_VOICES - 1));
}

static void * sn2_create(const tSynthLibPluginDesc * desc) {
    tSn2Plugin * t = (tSn2Plugin *)calloc(1, sizeof(tSn2Plugin));

    (void)desc;

    if (t == NULL) {
        return NULL;
    }

    for (uint32_t i = 0; i < kSn2ParamCount; i++) {
        t->values[i] = gSn2Params[i].defaultNormalized;
    }

    synthlib_swap_fade_init(&t->fade);
    sn2_presets_refresh();
    t->startPreset = true;

    snprintf(t->soundName, sizeof(t->soundName), "Init");
    sn2_voices_init(&t->voices, 48000.0);
    sn2_voices_set_limit(&t->voices, voice_limit(t));
    sn2_prefs_init();
    synthlib_log_line("created - Sn2Alike %s", SN2_VERSION_STRING);
    return t;
}

static void sn2_destroy(void * inst) {
    free(inst);
}

// notes §5: the plug-in's own state, after the parameters
#define STATE_MAGIC     "TIS1"
#define STATE_LENGTH    (4 + (SN2_SINGLE_NAME_LENGTH + 1) + 1 + SN2_SINGLE_LENGTH + 1024 + 2 + 2)

static size_t sn2_get_state(void * inst, void * out, size_t len) {
    tSn2Plugin * t = (tSn2Plugin *)inst;
    uint8_t *    o = (uint8_t *)out;

    if ((out == NULL) || (len < STATE_LENGTH)) {
        return STATE_LENGTH;
    }
    memcpy(o, STATE_MAGIC, 4);
    o   += 4;
    memcpy(o, t->soundName, SN2_SINGLE_NAME_LENGTH + 1);
    o   += SN2_SINGLE_NAME_LENGTH + 1;
    *o++ = t->haveBase ? 1 : 0;
    memcpy(o, t->base, SN2_SINGLE_LENGTH);
    o   += SN2_SINGLE_LENGTH;
    memcpy(o, t->patchPath, 1024);
    o   += 1024;
    *o++ = (uint8_t)(t->patchIndex >> 8);
    *o++ = (uint8_t)(t->patchIndex & 0xFF);
    *o++ = (uint8_t)(t->preset >> 8);
    *o   = (uint8_t)(t->preset & 0xFF);
    return STATE_LENGTH;
}

static void sn2_set_state(void * inst, const void * data, size_t len) {
    tSn2Plugin *    t = (tSn2Plugin *)inst;
    const uint8_t * d = (const uint8_t *)data;

    if ((data == NULL) || (len < STATE_LENGTH) || (memcmp(d, STATE_MAGIC, 4) != 0)) {
        return;
    }
    atomic_store(&t->startPreset, false);   // a project's sound, not A001
    d                                   += 4;
    memcpy(t->soundName, d, SN2_SINGLE_NAME_LENGTH + 1);
    t->soundName[SN2_SINGLE_NAME_LENGTH] = '\0';
    d                                   += SN2_SINGLE_NAME_LENGTH + 1;
    t->haveBase                          = (*d++ != 0);
    memcpy(t->base, d, SN2_SINGLE_LENGTH);
    d                                   += SN2_SINGLE_LENGTH;
    memcpy(t->patchPath, d, 1024);
    t->patchPath[1023]                   = '\0';
    d                                   += 1024;
    t->patchIndex                        = (d[0] << 8) | d[1];
    t->preset                            = (d[2] << 8) | d[3];
    t->preset                            = ((t->preset >= 1) && (t->preset <= SN2_PRESETS)) ? t->preset : 0;
}

static void sn2_prepare(void * inst, const tSynthLibSetup * setup) {
    tSn2Plugin * t = (tSn2Plugin *)inst;

    sn2_voices_set_rate(&t->voices, setup->sampleRate);

    if (setup->offline) {
        atomic_store(&t->startPreset, false);   // an offline render plays what it is given
    }
}

static void sn2_set_active(void * inst, bool active) {
    (void)active;
    ((tSn2Plugin *)inst)->eventCount = 0;
    sn2_voices_all_off(&((tSn2Plugin *)inst)->voices);
}

static void play(tSn2Plugin * t, const tSn2Event * e) {
    if (e->on) {
        sn2_voices_note_on(&t->voices, e->note, e->velocity);
    } else {
        sn2_voices_note_off(&t->voices, e->note);
    }
}

// notes §2: a full queue plays the note at once rather than lose it
static void queue(tSn2Plugin * t, tSn2Event e) {
    if (t->eventCount < MAX_EVENTS) {
        t->events[t->eventCount++] = e;
    } else {
        play(t, &e);
    }
}

static void sn2_note_on(void * inst, uint8_t channel, uint8_t note, float velocity, uint32_t sampleOffset) {
    (void)channel;
    queue((tSn2Plugin *)inst, (tSn2Event){sampleOffset, note, true, velocity});
}

static void sn2_note_off(void * inst, uint8_t channel, uint8_t note, float velocity, uint32_t sampleOffset) {
    (void)channel;
    (void)velocity;
    queue((tSn2Plugin *)inst, (tSn2Event){sampleOffset, note, false, 0.0f});
}

static void sn2_load_single(void * user, const uint8_t * single, const char * path, int index, int preset);

// A new instance plays A001, first audio block or first editor frame; with no presets it stays on Init
static void take_start_preset(tSn2Plugin * t) {
    if (atomic_exchange(&t->startPreset, false) && (sn2_presets_single(1)[0] == 0xF0)) {
        sn2_load_single(t, sn2_presets_single(1), "", 0, 1);
    }
}

static void take_program_change(tSn2Plugin * t) {
    int number = atomic_load(&t->wantPreset);

    if ((number != 0) && synthlib_swap_fade_silent(&t->fade)) {
        atomic_store(&t->wantPreset, 0);
        sn2_load_single(t, sn2_presets_single(number), "", 0, number);
        synthlib_swap_fade_end(&t->fade);
    }
}

static void sn2_process(void * inst, const float *const * in, uint32_t numIn, float ** out, uint32_t numOut,
                        uint32_t frames, const tSynthLibTransport * transport) {
    tSn2Plugin * t     = (tSn2Plugin *)inst;

    take_start_preset(t);
    take_program_change(t);
    double       gain  = 0.25 * pow(t->values[kSn2ParamVolume], 2.0);

    (void)in;
    (void)numIn;
    (void)transport;

    if ((out == NULL) || (numOut == 0) || (out[0] == NULL)) {
        for (uint32_t i = 0; i < t->eventCount; i++) {
            play(t, &t->events[i]);
        }

        t->eventCount = 0;
        return;
    }
    tSn2Sound    sound = {
        .shape          = (tSn2OscShape)lround(t->values[kSn2ParamOscShape] * (eTiOscShapes - 1)),
        .pulseWidth     =                           0.05 + (0.9 * t->values[kSn2ParamPulseWidth]),
        .cutoff         = t->values[kSn2ParamCutoff] * 127.0,
        .resonance      = t->values[kSn2ParamResonance] * 127.0,
        .filterType     = (uint32_t)lround(t->values[kSn2ParamFilterType] * 11.0),
        .filterSlope    = (uint32_t)lround(t->values[kSn2ParamFilterSlope] * 2.0),
        .ampAttack      = t->values[kSn2ParamAmpAttack] * 127.0,
        .ampDecay       = t->values[kSn2ParamAmpDecay] * 127.0,
        .ampSustain     = t->values[kSn2ParamAmpSustain] * 127.0,
        .ampSustainTime = t->values[kSn2ParamAmpSustainTime] * 127.0,
        .ampRelease     = t->values[kSn2ParamAmpRelease] * 127.0,
    };

    memset(out[0], 0, (size_t)frames * sizeof(float));

    // notes §2: up to each note, then the note; the host sends them in time order
    uint32_t     done  = 0;

    for (uint32_t i = 0; i < t->eventCount; i++) {
        uint32_t at = (t->events[i].offset < frames) ? t->events[i].offset : frames;

        if (at > done) {
            sn2_voices_render(&t->voices, &sound, out[0] + done, at - done, gain);
            done = at;
        }
        play(t, &t->events[i]);
    }

    t->eventCount = 0;

    if (done < frames) {
        sn2_voices_render(&t->voices, &sound, out[0] + done, frames - done, gain);
    }
    synthlib_swap_fade_apply(&t->fade, out[0], NULL, frames, t->voices.rate);

    for (uint32_t ch = 1; ch < numOut; ch++) {
        if (out[ch] != NULL) {
            memcpy(out[ch], out[0], (size_t)frames * sizeof(float));
        }
    }
}

static void sn2_set_param(void * inst, uint32_t id, double normalized) {
    tSn2Plugin * t = (tSn2Plugin *)inst;

    if (id >= kSn2ParamCount) {
        return;
    }
    t->values[id] = normalized;

    if (id == kSn2ParamProgram) {
        int bank   = (int)lround(t->values[kSn2ParamBank] * 127.0) % (SN2_PRESETS / SN2_BANK_SIZE);
        int number = (bank * SN2_BANK_SIZE) + (int)lround(normalized * 127.0) + 1;

        // already on it, nothing happens, edits and all
        if ((number != t->preset) && (sn2_presets_single(number)[0] == 0xF0)) {
            atomic_store(&t->wantPreset, number);
            synthlib_swap_fade_start(&t->fade);
        }
    } else if (id == kSn2ParamVoiceLimit) {
        sn2_voices_set_limit(&t->voices, voice_limit(t));
    }
}

static double sn2_get_param(void * inst, uint32_t id) {
    return (id < kSn2ParamCount) ? ((tSn2Plugin *)inst)->values[id] : 0.0;
}

static bool sn2_param_text(const tSynthLibPluginDesc * desc, void * inst, uint32_t id, double value, char * out, size_t len) {
    static const char *const shapes[]  = {"Saw", "Pulse", "Triangle"};
    static const char *const filters[] = {"LPF", "BPF", "HPF", "Res LPF", "Res BPF", "Res HPF", "LPF+LPF", "HPF+LPF", "HPF+HPF", "Notch", "LPF/LPF", "BPF/BPF"};
    static const char *const slopes[]  = {"12 dB", "18 dB", "24 dB"};

    (void)desc;
    (void)inst;

    if (id >= kSn2ParamCount) {
        return false;
    }
    long                     step      = lround(value * gSn2Params[id].stepCount);

    if (id == kSn2ParamOscShape) {
        snprintf(out, len, "%s", shapes[(step < 0) ? 0 : ((step > 2) ? 2 : step)]);
    } else if (id == kSn2ParamFilterType) {
        snprintf(out, len, "%s", filters[(step < 0) ? 0 : ((step > 11) ? 11 : step)]);
    } else if (id == kSn2ParamFilterSlope) {
        snprintf(out, len, "%s", slopes[(step < 0) ? 0 : ((step > 2) ? 2 : step)]);
    } else {
        snprintf(out, len, "%ld", lround(gSn2Params[id].plainMin) + step);
    }
    return true;
}

// ---- the editor - notes §3: Pulse2Alike's panel view, with this plug-in's pages --------------------------------

// ---- Singles and presets - notes §5 ------------------------------------------------------------------------

// What a sound carries: everything but the plug-in's own settings
static bool sound_param(uint32_t id) {
    return (id != kSn2ParamVoiceLimit) && (id != kSn2ParamVolume) && (id != kSn2ParamProgram) && (id != kSn2ParamBank);
}

static void current_values(void * user, double values[kSn2ParamCount]) {
    for (uint32_t id = 0; id < kSn2ParamCount; id++) {
        values[id] = synthlib_plugin_param_value(user, id);
    }
}

// The sound as it is now, as a Single: the host's values over the Single it came from
static void current_single(void * user, uint8_t single[SN2_SINGLE_LENGTH]) {
    tSn2Plugin * t = (tSn2Plugin *)user;
    double       values[kSn2ParamCount];

    current_values(user, values);
    sn2_patch_to_single(values, t->haveBase ? t->base : NULL, t->soundName, single);
}

// notes §6: every parameter the sound carries, through the host, so the change is one the host sees
static void sn2_load_single(void * user, const uint8_t * single, const char * path, int index, int preset) {
    tSn2Plugin * t = (tSn2Plugin *)user;
    double       values[kSn2ParamCount];

    current_values(user, values);
    sn2_patch_from_single(single, values);

    for (uint32_t id = 0; id < kSn2ParamCount; id++) {
        if (sound_param(id)) {
            synthlib_plugin_param_edited(user, id, values[id]);
        }
    }

    memcpy(t->base, single, SN2_SINGLE_LENGTH);
    t->haveBase   = true;
    sn2_patch_name(single, t->soundName);
    snprintf(t->patchPath, sizeof(t->patchPath), "%s", path);
    t->patchIndex = index;
    t->preset     = preset;
    synthlib_log_line("loaded \"%s\" (%s #%d, preset %d)", t->soundName, path, index, preset);
}

static bool sn2_save_patch(void * user, const char * path) {
    tSn2Plugin * t = (tSn2Plugin *)user;
    uint8_t      single[SN2_SINGLE_LENGTH];

    current_single(user, single);

    if (!sn2_patch_save_file(path, single)) {
        synthlib_log_line("could not write %s", path);
        return false;
    }
    memcpy(t->base, single, SN2_SINGLE_LENGTH);
    t->haveBase   = true;
    snprintf(t->patchPath, sizeof(t->patchPath), "%s", path);
    t->patchIndex = 0;
    t->preset     = 0;
    return true;
}

static void sn2_load_faded(void * user, const uint8_t * single, const char * path, int index, int preset) {
    tSn2Plugin * t = (tSn2Plugin *)user;

    synthlib_swap_fade_begin(&t->fade);
    sn2_load_single(user, single, path, index, preset);
    synthlib_swap_fade_end(&t->fade);
}

static bool sn2_store_preset(void * user, int number) {
    tSn2Plugin * t = (tSn2Plugin *)user;
    uint8_t      single[SN2_SINGLE_LENGTH];

    current_single(user, single);

    if (!sn2_presets_store(number, single)) {
        return false;
    }
    memcpy(t->base, sn2_presets_single(number), SN2_SINGLE_LENGTH);
    t->haveBase     = true;
    t->patchPath[0] = '\0';
    t->patchIndex   = 0;
    t->preset       = number;
    return true;
}

// every sound parameter to its default; the next save has no Single to keep bytes from
static void sn2_init_patch(void * user) {
    tSn2Plugin * t = (tSn2Plugin *)user;

    for (uint32_t id = 0; id < kSn2ParamCount; id++) {
        if (sound_param(id)) {
            synthlib_plugin_param_edited(user, id, gSn2Params[id].defaultNormalized);
        }
    }

    t->haveBase     = false;
    t->patchPath[0] = '\0';
    t->patchIndex   = 0;
    t->preset       = 0;
    snprintf(t->soundName, sizeof(t->soundName), "Init");
}

static const char * sn2_sound_name(void * user) {
    return ((tSn2Plugin *)user)->soundName;
}

// notes §5: the name only - written to a Single at the next save or store
static void sn2_rename(void * user, const char * name) {
    snprintf(((tSn2Plugin *)user)->soundName, SN2_SINGLE_NAME_LENGTH + 1, "%s", name);
}

// notes §7: edited, if a parameter it carries or its name differs from the stored preset
static bool preset_edited(tSn2Plugin * t, void * user) {
    double values[kSn2ParamCount], stored[kSn2ParamCount];
    char   name[SN2_SINGLE_NAME_LENGTH + 1];

    if (t->preset == 0) {
        return false;
    }
    current_values(user, values);
    memcpy(stored, values, sizeof(stored));
    sn2_patch_from_single(sn2_presets_single(t->preset), stored);
    sn2_patch_name(sn2_presets_single(t->preset), name);

    for (uint32_t id = 0; id < kSn2ParamCount; id++) {
        if (sound_param(id) && (fabs(stored[id] - values[id]) > 1e-6)) {
            return true;
        }
    }

    return strcmp(name, t->soundName) != 0;
}

static double now_seconds(void) {
    struct timespec ts;

    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec + (ts.tv_nsec * 1e-9);
}

static const tSn2PatchHost gPatchHost = {
    .load = sn2_load_faded, .save   = sn2_save_patch, .storePreset = sn2_store_preset, .init = sn2_init_patch,
    .name = sn2_sound_name, .rename = sn2_rename,
};

static void sn2_panel_init(void) {
    sn2_draw_init();
    sn2_draw_set_pages(gSn2Pages, gSn2PageCount);
    sn2_draw_set_host(&gPatchHost);
}

static void sn2_panel_sync(void * user) {
    tSn2Plugin * t = (tSn2Plugin *)user;

    sn2_draw_set_user(user);

    // another instance's store shows within a second
    if ((now_seconds() - t->presetsChecked) >= 1.0) {
        t->presetsChecked = now_seconds();
        (void)sn2_presets_refresh();
    }
    sn2_draw_set_patch(t->patchPath, t->patchIndex);   // this instance's, every sync
    sn2_draw_set_preset(t->preset, preset_edited(t, user));

    take_start_preset((tSn2Plugin *)user);

    for (uint32_t id = 0; id < kSn2ParamCount; id++) {
        sn2_draw_set_value(id, synthlib_plugin_param_value(user, id));
    }
}

static void sn2_panel_frame(void * user, int pixelWidth, int pixelHeight) {
    (void)user;
    sn2_draw_frame(pixelWidth, pixelHeight);
}

static bool sn2_panel_press(void * user, double x, double y) {
    tSn2Edit edit;
    bool     grabbed = false;

    (void)sn2_draw_press(x, y, &edit, &grabbed);

    if (edit.paramId != UINT32_MAX) {
        synthlib_plugin_param_edited(user, edit.paramId, edit.normalized);
    }
    return grabbed;
}

static bool sn2_panel_keeps_pointer(void * user) {
    (void)user;
    return sn2_dial_mode() == eDialModeRotary;
}

static void sn2_panel_drag_to(void * user, double x, double y) {
    tSn2Edit edit;

    if (sn2_draw_drag_to(x, y, &edit)) {
        synthlib_plugin_param_edited(user, edit.paramId, edit.normalized);
    }
}

static void sn2_panel_drag_by(void * user, double dx, double dy) {
    tSn2Edit edit;

    if (sn2_draw_drag_by(dx, dy, &edit)) {
        synthlib_plugin_param_edited(user, edit.paramId, edit.normalized);
    }
}

static void sn2_panel_release(void * user, double x, double y) {
    tSn2Edit edit;

    sn2_draw_release(x, y, &edit);

    if (edit.paramId != UINT32_MAX) {
        synthlib_plugin_param_edited(user, edit.paramId, edit.normalized);
    }
}

static bool sn2_panel_key(void * user, int glfwKey, unsigned int codepoint, bool isRepeat) {
    (void)user;
    return sn2_draw_key(glfwKey, codepoint, isRepeat);
}

static void sn2_panel_scroll(void * user, double x, double y, double dy) {
    (void)user;
    (void)x;
    (void)y;
    sn2_draw_scroll(dy);
}

static const tSynthLibPanel      gPanel = {
    .canvasWidth  = SN2_CANVAS_W,
    .init         = sn2_panel_init,
    .sync         = sn2_panel_sync,
    .frame        = sn2_panel_frame,
    .press        = sn2_panel_press,
    .dragBy       = sn2_panel_drag_by,
    .keepsPointer = sn2_panel_keeps_pointer,
    .dragTo       = sn2_panel_drag_to,
    .release      = sn2_panel_release,
    .scroll       = sn2_panel_scroll,
    .key          = sn2_panel_key,
    .pointer      = sn2_draw_set_mouse,
    .menuActive   = sn2_draw_menu_active,
};

static void * sn2_create_view(const tSynthLibPluginDesc * desc, void * inst, double width, double height) {
    (void)desc;
    return synthlib_panel_view_create(&gPanel, inst, width, height);
}

static const tSynthLibBus        gOutputs[1] = {{"Out", 2, false, true}};

static const tSynthLibPluginDesc gDescriptor = {
    .name               = "Sn2Alike",
    .vendor             = "Chris Purusha",
    .url                = "https://github.com/chrispurusha/Sn2Alike",
    .email              = "",
    .version            = SN2_VERSION_STRING,

    .isInstrument       = true,
    .vst3SubCategory    = "Instrument|Synth",
    .outputs            = gOutputs,
    .numOutputs         =                                          1,
    .wantsMidiIn        = true,

    .vst3ProcessorUid   = gProcessorUid,
    .vst3ControllerUid  = gControllerUid,
    .auType             = SN2_AU_TYPE,
    .auSubType          = SN2_AU_SUBTYPE,
    .auManufacturer     = SN2_AU_MANUFACTURER,
    .auVersion          = SN2_AU_VERSION,
    .auBundleId         = SN2_AU_BUNDLE_ID,

    .params             = gSn2Params,
    .numParams          = kSn2ParamCount,

    .editorDefaultWidth = SN2_CANVAS_W,
    .editorMinWidth     = SN2_CANVAS_W * 0.75,
    .editorMaxWidth     = SN2_CANVAS_W * 2.0,
    .editorAspect       = SN2_CANVAS_W / SN2_CANVAS_H,

    .cb                 = {
        .create     = sn2_create,
        .destroy    = sn2_destroy,
        .prepare    = sn2_prepare,
        .setActive  = sn2_set_active,
        .process    = sn2_process,
        .noteOn     = sn2_note_on,
        .noteOff    = sn2_note_off,
        .setParam   = sn2_set_param,
        .getParam   = sn2_get_param,
        .paramText  = sn2_param_text,
        .getState   = sn2_get_state,
        .setState   = sn2_set_state,
        .createView = sn2_create_view,
    }
};

static const tSynthLibPluginSet  gSet = {
    .variants = &gDescriptor,
    .count    = 1
};

static const tSynthLibPluginSet * plugin_variants(void) {
    return &gSet;
}

SYNTHLIB_PLUGIN_REGISTER(plugin_variants, "sn2alike")
