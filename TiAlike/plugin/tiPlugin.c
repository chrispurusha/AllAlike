/*
 * TiAlike - a reproduction inspired by the Access Virus TI's single-patch engine, as a VST3 and Audio Unit
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
// Notes: Docs/code-notes/tiPlugin.md - "// notes §k" refers there.

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "synthlibPlugin.h"
#include "synthlibBackdoor.h"
#include "synthlibLog.h"
#include "synthlibSwapFade.h"
#include "synthlibPanelView.h"
#include "synthlibTypes.h"
#include "inputState.h"
#include "tiDraw.h"
#include "tiParams.h"
#include "tiHardware.h"
#include "tiPatch.h"
#include "tiPrefs.h"
#include "tiVoices.h"

// touch /tmp/tialike-log, read /tmp/tialike.log - see SynthLib's plugin/synthlibLog.h.

#ifndef TIA_VERSION_STRING
#define TIA_VERSION_STRING    "0.1.0"
#endif

#ifndef TIA_AU_VERSION
#define TIA_AU_VERSION    (0x00000100)
#endif

// THESE MAY NEVER CHANGE: a saved project finds the plug-in by them.
static const uint8_t gProcessorUid[16]  = {0x53, 0x8A, 0xA3, 0x55, 0x5C, 0x3A, 0x41, 0x46, 0x9F, 0xB9, 0x39, 0xEC, 0x9A, 0x23, 0x65, 0x91};
static const uint8_t gControllerUid[16] = {0x30, 0x99, 0xDF, 0x1B, 0xE1, 0x40, 0x46, 0xCC, 0x81, 0xB5, 0xBF, 0x53, 0xDE, 0x05, 0xF8, 0x30};

// THESE MUST MATCH do-plugin - the bundle identifier especially
#define FOUR_CC(a, b, c, d)    (((uint32_t)(a) << 24) | ((uint32_t)(b) << 16) | ((uint32_t)(c) << 8) | (uint32_t)(d))
#define TIA_AU_TYPE            FOUR_CC('a', 'u', 'm', 'u')
#define TIA_AU_SUBTYPE         FOUR_CC('T', 'I', 'a', 'l')
#define TIA_AU_MANUFACTURER    FOUR_CC('C', 'P', 'u', 'r')
#define TIA_AU_BUNDLE_ID       "com.chrispurusha.tialike.au"

// notes §2: a block's notes, played at their own sample when the block renders
#define MAX_EVENTS             (256)
#define LINK_MAX_BYTES         (8)
#define LINK_SOUND_SECONDS     (1.0)   // what a whole Single takes on this port
#define MAX_MONO_FRAMES        (8192)

typedef struct {
    uint32_t offset;
    uint8_t  note;
    bool     on;
    float    velocity;
} tTiEvent;

typedef struct {
    tTiVoices         voices;
    tTiEvent          events[MAX_EVENTS];
    float             mono[MAX_MONO_FRAMES]; // notes §8: the right side, when the host gives one output
    uint32_t          eventCount;
    double            values[kTiParamCount]; // normalized
    tTiSound          sound;                 // notes §8: the Single as the voices read it
    bool              soundStale;
    // notes §5: the sound, beyond its parameters - main thread, and the host's state calls
    char              soundName[TI_SINGLE_NAME_LENGTH + 1];
    bool              haveBase;
    uint8_t           base[TI_SINGLE_LENGTH]; // the Single it came from, whose bytes a save keeps
    char              patchPath[1024];
    int               patchIndex;             // its place in that file
    int               preset;                 // 1..TI_PRESETS, 0 none
    double            presetsChecked;
    // notes §9: Link, on the editor's frame
    int               hwLock;
    bool              hwSent;
    double            hwSentAt;
    uint8_t           hwLast[TI_SINGLE_LENGTH];
    tSynthLibSwapFade fade;
    _Atomic bool      clipped;      // the audio thread saw full scale; the panel holds the lamp
    double            clipUntil;
    _Atomic bool      startPreset;  // notes §11
    _Atomic int       wantPreset;   // a program change waiting for the fade
} tTiPlugin;

static uint32_t voice_limit(const tTiPlugin * t) {
    return 1u + (uint32_t)lround(t->values[kTiParamVoiceLimit] * (TI_MAX_VOICES - 1));
}

static void * ti_create(const tSynthLibPluginDesc * desc) {
    tTiPlugin * t = (tTiPlugin *)calloc(1, sizeof(tTiPlugin));

    (void)desc;

    if (t == NULL) {
        return NULL;
    }

    for (uint32_t i = 0; i < kTiParamCount; i++) {
        t->values[i] = gTiParams[i].defaultNormalized;
    }

    synthlib_swap_fade_init(&t->fade);
    ti_presets_refresh();
    t->startPreset = true;

    snprintf(t->soundName, sizeof(t->soundName), "Init");
    t->hwLock      = -1;
    t->soundStale  = true;
    ti_voices_init(&t->voices, 48000.0);
    ti_voices_set_limit(&t->voices, voice_limit(t));
    ti_prefs_init();
    synthlib_log_line("created - TiAlike %s", TIA_VERSION_STRING);
    return t;
}

static void ti_destroy(void * inst) {
    ti_hardware_release(&((tTiPlugin *)inst)->hwLock);
    free(inst);
}

// notes §5: the plug-in's own state, after the parameters
#define STATE_MAGIC     "TIS1"
#define STATE_LENGTH    (4 + (TI_SINGLE_NAME_LENGTH + 1) + 1 + TI_SINGLE_LENGTH + 1024 + 2 + 2)

// What a project keeps: the Single with the edits made since it was loaded
static void current_single_of(const tTiPlugin * t, uint8_t * single) {
    ti_patch_to_single(t->values, t->haveBase ? t->base : NULL, t->soundName, single);
}

static size_t ti_get_state(void * inst, void * out, size_t len) {
    tTiPlugin * t = (tTiPlugin *)inst;
    uint8_t *   o = (uint8_t *)out;

    if ((out == NULL) || (len < STATE_LENGTH)) {
        return STATE_LENGTH;
    }
    memcpy(o, STATE_MAGIC, 4);
    o   += 4;
    memcpy(o, t->soundName, TI_SINGLE_NAME_LENGTH + 1);
    o   += TI_SINGLE_NAME_LENGTH + 1;
    *o++ = t->haveBase ? 1 : 0;
    current_single_of(t, o);
    o   += TI_SINGLE_LENGTH;
    memcpy(o, t->patchPath, 1024);
    o   += 1024;
    *o++ = (uint8_t)(t->patchIndex >> 8);
    *o++ = (uint8_t)(t->patchIndex & 0xFF);
    *o++ = (uint8_t)(t->preset >> 8);
    *o   = (uint8_t)(t->preset & 0xFF);
    return STATE_LENGTH;
}

static void ti_set_state(void * inst, const void * data, size_t len) {
    tTiPlugin *     t = (tTiPlugin *)inst;
    const uint8_t * d = (const uint8_t *)data;

    if ((data == NULL) || (len < STATE_LENGTH) || (memcmp(d, STATE_MAGIC, 4) != 0)) {
        return;
    }
    atomic_store(&t->startPreset, false);   // a project's sound, not A001
    d                                  += 4;
    memcpy(t->soundName, d, TI_SINGLE_NAME_LENGTH + 1);
    t->soundName[TI_SINGLE_NAME_LENGTH] = '\0';
    d                                  += TI_SINGLE_NAME_LENGTH + 1;
    t->haveBase                         = (*d++ != 0);
    memcpy(t->base, d, TI_SINGLE_LENGTH);
    d                                  += TI_SINGLE_LENGTH;
    memcpy(t->patchPath, d, 1024);
    t->patchPath[1023]                  = '\0';
    d                                  += 1024;
    t->patchIndex                       = (d[0] << 8) | d[1];
    t->preset                           = (d[2] << 8) | d[3];
    t->preset                           = ((t->preset >= 1) && (t->preset <= TI_PRESETS)) ? t->preset : 0;
    t->soundStale                       = true;

    // notes §8: a project from before a parameter existed has no value for it; its Single does
    if (t->haveBase) {
        double fromSingle[kTiParamCount];

        memcpy(fromSingle, t->values, sizeof(fromSingle));
        ti_patch_from_single(t->base, fromSingle);

        for (uint32_t id = kTiParamSostenuto + 1u; id < kTiParamCount; id++) {
            t->values[id] = fromSingle[id];
        }
    }
}

static void ti_prepare(void * inst, const tSynthLibSetup * setup) {
    tTiPlugin * t = (tTiPlugin *)inst;

    ti_voices_set_rate(&t->voices, setup->sampleRate);

    if (setup->offline) {
        atomic_store(&t->startPreset, false);   // an offline render plays what it is given
    }
}

static void ti_set_active(void * inst, bool active) {
    (void)active;
    ((tTiPlugin *)inst)->eventCount = 0;
    ti_voices_all_off(&((tTiPlugin *)inst)->voices);
}

static void play(tTiPlugin * t, const tTiEvent * e) {
    if (e->on) {
        ti_voices_note_on(&t->voices, e->note, e->velocity);
    } else {
        ti_voices_note_off(&t->voices, e->note, e->velocity);
    }
}

// notes §2: a full queue plays the note at once rather than lose it
static void queue(tTiPlugin * t, tTiEvent e) {
    if (t->eventCount < MAX_EVENTS) {
        t->events[t->eventCount++] = e;
    } else {
        play(t, &e);
    }
}

static void ti_note_on(void * inst, uint8_t channel, uint8_t note, float velocity, uint32_t sampleOffset) {
    if (((tTiPlugin *)inst)->values[kTiParamHwLink] >= 0.5) {
        ti_hardware_note(note, (uint8_t)lroundf(velocity * 127.0f));   // notes §9
    }
    (void)channel;
    queue((tTiPlugin *)inst, (tTiEvent){sampleOffset, note, true, velocity});
}

static void ti_note_off(void * inst, uint8_t channel, uint8_t note, float velocity, uint32_t sampleOffset) {
    if (((tTiPlugin *)inst)->values[kTiParamHwLink] >= 0.5) {
        ti_hardware_note(note, 0);   // notes §9
    }
    (void)channel;
    queue((tTiPlugin *)inst, (tTiEvent){sampleOffset, note, false, velocity});
}

// notes §8: the host's values written over the Single they came from, when anything changed
static void refresh_sound(tTiPlugin * t) {
    uint8_t single[TI_SINGLE_LENGTH];

    if (t->soundStale) {
        t->soundStale   = false;
        ti_patch_to_single(t->values, t->haveBase ? t->base : NULL, t->soundName, single);
        ti_patch_sound(single, &t->sound);
        t->voices.sound = t->sound;
    }
}

static void take_start_preset(tTiPlugin * t);
static void take_program_change(tTiPlugin * t);

static void ti_process(void * inst, const float *const * in, uint32_t numIn, float ** out, uint32_t numOut,
                       uint32_t frames, const tSynthLibTransport * transport) {
    tTiPlugin * t = (tTiPlugin *)inst;

    (void)in;
    (void)numIn;
    take_start_preset(t);
    take_program_change(t);
    refresh_sound(t);

    ti_voices_set_effects(&t->voices, t->values[kTiParamDelayOn] >= 0.5, t->values[kTiParamReverbOn] >= 0.5);

    if ((transport != NULL) && transport->tempoValid) {
        ti_voices_set_tempo(&t->voices, transport->tempo);
    }

    if ((out == NULL) || (numOut == 0) || (out[0] == NULL)) {
        for (uint32_t i = 0; i < t->eventCount; i++) {
            play(t, &t->events[i]);
        }

        t->eventCount = 0;
        return;
    }
    float *  left  = out[0];
    float *  right = ((numOut > 1) && (out[1] != NULL)) ? out[1] : t->mono;

    if (frames > MAX_MONO_FRAMES) {
        frames = MAX_MONO_FRAMES;
    }
    memset(left, 0, (size_t)frames * sizeof(float));
    memset(right, 0, (size_t)frames * sizeof(float));

    // notes §2: up to each note, then the note; the host sends them in time order
    uint32_t done  = 0;

    for (uint32_t i = 0; i < t->eventCount; i++) {
        uint32_t at = (t->events[i].offset < frames) ? t->events[i].offset : frames;

        if (at > done) {
            ti_voices_render(&t->voices, &t->sound, left + done, right + done, at - done);
            done = at;
        }
        play(t, &t->events[i]);
    }

    t->eventCount = 0;

    if (done < frames) {
        ti_voices_render(&t->voices, &t->sound, left + done, right + done, frames - done);
    }

    if (t->voices.clipped) {
        t->voices.clipped = false;
        atomic_store(&t->clipped, true);
    }

    // after the ceiling: it turns the output down and never saves the sound from it
    for (uint32_t f = 0; f < frames; f++) {
        left[f]  *= (float)t->values[kTiParamMaster];
        right[f] *= (float)t->values[kTiParamMaster];
    }

    synthlib_swap_fade_apply(&t->fade, left, right, frames, t->voices.rate);

    // notes §8: a mono output hears both sides
    if (right == t->mono) {
        for (uint32_t f = 0; f < frames; f++) {
            left[f] = 0.5f * (left[f] + right[f]);
        }
    }

    for (uint32_t ch = 2; ch < numOut; ch++) {
        if (out[ch] != NULL) {
            memset(out[ch], 0, (size_t)frames * sizeof(float));
        }
    }
}

static void ti_load_single(void * user, const uint8_t * single, const char * path, int index, int preset);

// notes §10: already on it, nothing happens, edits and all
static void program_change(tTiPlugin * t, int program) {
    int bank   = (int)lround(t->values[kTiParamBank] * 127.0) % (TI_PRESETS / TI_BANK_SIZE);
    int number = (bank * TI_BANK_SIZE) + program + 1;

    if ((number != t->preset) && (ti_presets_single(number)[0] == 0xF0)) {
        atomic_store(&t->wantPreset, number);
        synthlib_swap_fade_start(&t->fade);
    }
}

// notes §11
static void take_start_preset(tTiPlugin * t) {
    if (atomic_exchange(&t->startPreset, false) && (ti_presets_single(1)[0] == 0xF0)) {
        ti_load_single(t, ti_presets_single(1), "", 0, 1);
    }
}

static void take_program_change(tTiPlugin * t) {
    int number = atomic_load(&t->wantPreset);

    if ((number != 0) && synthlib_swap_fade_silent(&t->fade)) {
        atomic_store(&t->wantPreset, 0);
        ti_load_single(t, ti_presets_single(number), "", 0, number);
        synthlib_swap_fade_end(&t->fade);
    }
}

static void ti_set_param(void * inst, uint32_t id, double normalized) {
    tTiPlugin * t = (tTiPlugin *)inst;

    if (id >= kTiParamCount) {
        return;
    }
    t->values[id] = normalized;

    if (id == kTiParamVoiceLimit) {
        ti_voices_set_limit(&t->voices, voice_limit(t));
    } else if (id == kTiParamProgram) {
        program_change(t, (int)lround(normalized * 127.0));
    } else if (id == kTiParamBank) {
    } else if (id == kTiParamArithmetic) {
        ti_voices_set_fixed(&t->voices, normalized >= 0.5);
    } else if ((id >= kTiParamBend) && (id <= kTiParamSostenuto)) {
        ti_voices_control(&t->voices, (tTiControlId)(id - kTiParamBend), (id == kTiParamBend) ? ((2.0 * normalized) - 1.0) : normalized);
    } else {
        t->soundStale = true;
    }
}

static double ti_get_param(void * inst, uint32_t id) {
    return (id < kTiParamCount) ? ((tTiPlugin *)inst)->values[id] : 0.0;
}

static bool ti_param_text(const tSynthLibPluginDesc * desc, void * inst, uint32_t id, double value, char * out, size_t len) {
    static const char *const filters[] = {"Low Pass", "High Pass", "Band Pass", "Band Stop", "Analog 1 Pole", "Analog 2 Pole", "Analog 3 Pole", "Analog 4 Pole"};
    static const char *const routes[]  = {"Serial 4", "Serial 6", "Parallel 4", "Split"};

    (void)desc;
    (void)inst;

    if (id >= kTiParamCount) {
        return false;
    }
    long                     step      = lround(value * gTiParams[id].stepCount);

    if (id == kTiParamFilterMode) {
        snprintf(out, len, "%s", filters[(step < 0) ? 0 : ((step > 7) ? 7 : step)]);
    } else if (id == kTiParamFilter2Mode) {
        snprintf(out, len, "%s", filters[(step < 0) ? 0 : ((step > 3) ? 3 : step)]);
    } else if (id == kTiParamRouting) {
        snprintf(out, len, "%s", routes[(step < 0) ? 0 : ((step > 3) ? 3 : step)]);
    } else if (id == kTiParamFilterLink) {
        snprintf(out, len, "%s", (step != 0) ? "On" : "Off");
    } else {
        snprintf(out, len, "%ld", lround(gTiParams[id].plainMin) + step);
    }
    return true;
}

// ---- the editor - notes §3: Pulse2Alike's panel view, with this plug-in's pages --------------------------------

// notes §4: with Link on, Filter 2's Cutoff dial shows where Filter 2's cutoff is, and its offset beside it
static bool ti_dial_display(uint32_t paramId, const double * values, int32_t * step, char * text, size_t len) {
    if ((paramId != kTiParamCutoff2) || (values[kTiParamFilterLink] < 0.5)) {
        return false;
    }
    double cutoff = values[kTiParamCutoff] * 127.0;
    double offset = values[kTiParamCutoff2] * 127.0;
    long   shown  = lround(fmin(ti_voices_cutoff2(cutoff, offset, true), 127.0));

    *step = (int32_t)shown;
    snprintf(text, len, "%ld (%+ld)", shown, lround(offset) - 64);
    return true;
}

// ---- Singles and presets - notes §5 ------------------------------------------------------------------------

// What a sound carries: everything but the plug-in's own settings
static bool sound_param(uint32_t id) {
    return (id != kTiParamProgram) && (id != kTiParamBank) && (id != kTiParamMaster) && (id != kTiParamVoiceLimit) && (id != kTiParamArithmetic) && (id != kTiParamHwLink) && (id != kTiParamDelayOn) && (id != kTiParamReverbOn) && ((id < kTiParamBend) || (id > kTiParamSostenuto));
}

static void current_values(void * user, double values[kTiParamCount]) {
    for (uint32_t id = 0; id < kTiParamCount; id++) {
        values[id] = synthlib_plugin_param_value(user, id);
    }
}

// The sound as it is now, as a Single: the host's values over the Single it came from
static void current_single(void * user, uint8_t single[TI_SINGLE_LENGTH]) {
    tTiPlugin * t = (tTiPlugin *)user;
    double      values[kTiParamCount];

    current_values(user, values);
    ti_patch_to_single(values, t->haveBase ? t->base : NULL, t->soundName, single);
}

// notes §6: every parameter the sound carries, through the host, so the change is one the host sees
static void ti_load_single(void * user, const uint8_t * single, const char * path, int index, int preset) {
    tTiPlugin * t = (tTiPlugin *)user;
    double      values[kTiParamCount];

    current_values(user, values);
    ti_patch_from_single(single, values);

    for (uint32_t id = 0; id < kTiParamCount; id++) {
        if (sound_param(id)) {
            synthlib_plugin_param_edited(user, id, values[id]);
        }
    }

    memcpy(t->base, single, TI_SINGLE_LENGTH);
    t->haveBase   = true;
    t->soundStale = true;
    ti_patch_name(single, t->soundName);
    snprintf(t->patchPath, sizeof(t->patchPath), "%s", path);
    t->patchIndex = index;
    t->preset     = preset;
    synthlib_log_line("loaded \"%s\" (%s #%d, preset %d)", t->soundName, path, index, preset);
}

static bool ti_save_patch(void * user, const char * path) {
    tTiPlugin * t = (tTiPlugin *)user;
    uint8_t     single[TI_SINGLE_LENGTH];

    current_single(user, single);

    if (!ti_patch_save_file(path, single)) {
        synthlib_log_line("could not write %s", path);
        return false;
    }
    memcpy(t->base, single, TI_SINGLE_LENGTH);
    t->haveBase   = true;
    snprintf(t->patchPath, sizeof(t->patchPath), "%s", path);
    t->patchIndex = 0;
    t->preset     = 0;
    return true;
}

static bool ti_store_preset(void * user, int number) {
    tTiPlugin * t = (tTiPlugin *)user;
    uint8_t     single[TI_SINGLE_LENGTH];

    current_single(user, single);

    if (!ti_presets_store(number, single)) {
        return false;
    }
    memcpy(t->base, ti_presets_single(number), TI_SINGLE_LENGTH);
    t->haveBase     = true;
    t->patchPath[0] = '\0';
    t->patchIndex   = 0;
    t->preset       = number;
    return true;
}

static void ti_load_faded(void * user, const uint8_t * single, const char * path, int index, int preset) {
    tTiPlugin * t = (tTiPlugin *)user;

    synthlib_swap_fade_begin(&t->fade);
    ti_load_single(user, single, path, index, preset);
    synthlib_swap_fade_end(&t->fade);
}

// every sound parameter to its default; the next save has no Single to keep bytes from
static void ti_init_patch(void * user) {
    tTiPlugin * t = (tTiPlugin *)user;

    for (uint32_t id = 0; id < kTiParamCount; id++) {
        if (sound_param(id)) {
            synthlib_plugin_param_edited(user, id, gTiParams[id].defaultNormalized);
        }
    }

    t->haveBase     = false;
    t->soundStale   = true;
    t->patchPath[0] = '\0';
    t->patchIndex   = 0;
    t->preset       = 0;
    snprintf(t->soundName, sizeof(t->soundName), "Init");
}

static const char * ti_sound_name(void * user) {
    return ((tTiPlugin *)user)->soundName;
}

// notes §5: the name only - written to a Single at the next save or store
static void ti_rename(void * user, const char * name) {
    snprintf(((tTiPlugin *)user)->soundName, TI_SINGLE_NAME_LENGTH + 1, "%s", name);
}

// notes §7: edited, if a parameter it carries or its name differs from the stored preset
static bool preset_edited(tTiPlugin * t, void * user) {
    double values[kTiParamCount], stored[kTiParamCount];
    char   name[TI_SINGLE_NAME_LENGTH + 1];

    if (t->preset == 0) {
        return false;
    }
    current_values(user, values);
    memcpy(stored, values, sizeof(stored));
    ti_patch_from_single(ti_presets_single(t->preset), stored);
    ti_patch_name(ti_presets_single(t->preset), name);

    for (uint32_t id = 0; id < kTiParamCount; id++) {
        if (sound_param(id) && (fabs(stored[id] - values[id]) > 1e-6)) {
            return true;
        }
    }

    return strcmp(name, t->soundName) != 0;
}

#define CLIP_HOLD_S    (1.5)

static double now_seconds(void) {
    struct timespec ts;

    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec + (ts.tv_nsec * 1e-9);
}

static const tTiPatchHost gPatchHost = {
    .load = ti_load_faded, .save   = ti_save_patch, .storePreset = ti_store_preset, .init = ti_init_patch,
    .name = ti_sound_name, .rename = ti_rename,
};

static void ti_panel_init(void) {
    ti_draw_init();
    ti_draw_set_pages(gTiPages, gTiPageCount);
    ti_draw_set_display(ti_dial_display);
    ti_draw_set_host(&gPatchHost);
}

// notes §9: the instrument follows the sound; a few changed bytes go singly, anything more as the whole sound
static void hardware_link(tTiPlugin * t, void * user) {
    if (synthlib_plugin_param_value(user, kTiParamHwLink) < 0.5) {
        ti_hardware_release(&t->hwLock);
        t->hwSent = false;
        return;
    }

    if (!ti_hardware_claim(&t->hwLock)) {
        return;
    }
    uint8_t single[TI_SINGLE_LENGTH];
    int     changed = 0;

    current_single(user, single);

    for (int i = 9; (i < TI_SINGLE_LENGTH - 2) && t->hwSent; i++) {
        changed += ((i != 265) && (single[i] != t->hwLast[i]));
    }

    if (t->hwSent && (changed == 0)) {
        return;
    }

    if (t->hwSent && (changed <= LINK_MAX_BYTES)) {
        for (int i = 9; i < TI_SINGLE_LENGTH - 2; i++) {
            if ((i != 265) && (single[i] != t->hwLast[i])) {
                (void)ti_hardware_send_byte(i, single[i]);
            }
        }
    } else if ((now_seconds() - t->hwSentAt) >= LINK_SOUND_SECONDS) {
        tTiHardwareResult r = ti_hardware_send_sound(single);

        t->hwSentAt = now_seconds();
        synthlib_log_line("Link: sound sent (result %d)", (int)r);
    } else {
        return;
    }
    memcpy(t->hwLast, single, TI_SINGLE_LENGTH);
    t->hwSent = true;
}

// notes: a plain-text channel, off until /tmp/tialike-backdoor exists
static double bd_get(void * user, uint32_t id) {
    return ti_get_param(user, id);
}

static void bd_set(void * user, uint32_t id, double normalized) {
    ti_set_param(user, id, normalized);
    synthlib_plugin_param_edited(user, id, normalized);
}

static bool bd_text(void * user, uint32_t id, double normalized, char * out, size_t len) {
    return ti_param_text(NULL, user, id, normalized, out, len);
}

static bool bd_preset(void * user, int number) {
    if ((number < 1) || (number > TI_PRESETS)) {
        return false;
    }
    (void)ti_presets_refresh();
    ti_load_faded(user, ti_presets_single(number), "", 0, number);
    return true;
}

static const tSynthLibBackdoor gBackdoor = {
    .params = gTiParams,     .count  = kTiParamCount,
    .get    = bd_get,        .set    = bd_set,       .text  = bd_text,
    .name   = ti_sound_name, .rename = ti_rename,
    .preset = bd_preset,     .save   = ti_save_patch,.init  = ti_init_patch,
};

static void ti_panel_sync(void * user) {
    synthlib_backdoor_poll("tialike", &gBackdoor, user);
    tTiPlugin * t = (tTiPlugin *)user;

    ti_draw_set_user(user);

    if (atomic_exchange(&((tTiPlugin *)user)->clipped, false)) {
        ((tTiPlugin *)user)->clipUntil = now_seconds() + CLIP_HOLD_S;
    }
    ti_draw_set_clip(now_seconds() < ((tTiPlugin *)user)->clipUntil);

    // another instance's store shows within a second
    if ((now_seconds() - t->presetsChecked) >= 1.0) {
        t->presetsChecked = now_seconds();
        (void)ti_presets_refresh();
    }
    ti_draw_set_patch(t->patchPath, t->patchIndex);   // this instance's, every sync
    ti_draw_set_preset(t->preset, preset_edited(t, user));

    take_start_preset((tTiPlugin *)user);   // notes §11

    for (uint32_t id = 0; id < kTiParamCount; id++) {
        ti_draw_set_value(id, synthlib_plugin_param_value(user, id));
    }

    hardware_link(t, user);
    ti_draw_transfer_tick();
}

static void ti_panel_frame(void * user, int pixelWidth, int pixelHeight) {
    (void)user;
    ti_draw_frame(pixelWidth, pixelHeight);
}

static bool ti_panel_press(void * user, double x, double y) {
    tTiEdit edit;
    bool    grabbed = false;

    (void)ti_draw_press(x, y, &edit, &grabbed);

    if (edit.paramId != UINT32_MAX) {
        synthlib_plugin_param_edited(user, edit.paramId, edit.normalized);
    }
    return grabbed;
}

static bool ti_panel_keeps_pointer(void * user) {
    (void)user;
    return ti_dial_mode() == eDialModeRotary;
}

static void ti_panel_drag_to(void * user, double x, double y) {
    tTiEdit edit;

    if (ti_draw_drag_to(x, y, &edit)) {
        synthlib_plugin_param_edited(user, edit.paramId, edit.normalized);
    }
}

static void ti_panel_drag_by(void * user, double dx, double dy) {
    tTiEdit edit;

    if (ti_draw_drag_by(dx, dy, &edit)) {
        synthlib_plugin_param_edited(user, edit.paramId, edit.normalized);
    }
}

static void ti_panel_release(void * user, double x, double y) {
    tTiEdit edit;

    ti_draw_release(x, y, &edit);

    if (edit.paramId != UINT32_MAX) {
        synthlib_plugin_param_edited(user, edit.paramId, edit.normalized);
    }
}

static bool ti_panel_key(void * user, int glfwKey, unsigned int codepoint, bool isRepeat) {
    (void)user;
    return ti_draw_key(glfwKey, codepoint, isRepeat);
}

static void ti_panel_scroll(void * user, double x, double y, double dy) {
    (void)user;
    (void)x;
    (void)y;
    ti_draw_scroll(dy);
}

static const tSynthLibPanel      gPanel = {
    .canvasWidth  = TI_CANVAS_W,
    .init         = ti_panel_init,
    .sync         = ti_panel_sync,
    .frame        = ti_panel_frame,
    .press        = ti_panel_press,
    .dragBy       = ti_panel_drag_by,
    .keepsPointer = ti_panel_keeps_pointer,
    .dragTo       = ti_panel_drag_to,
    .release      = ti_panel_release,
    .scroll       = ti_panel_scroll,
    .key          = ti_panel_key,
    .pointer      = ti_draw_set_mouse,
    .menuActive   = ti_draw_menu_active,
};

static void * ti_create_view(const tSynthLibPluginDesc * desc, void * inst, double width, double height) {
    (void)desc;
    return synthlib_panel_view_create(&gPanel, inst, width, height);
}

static const tSynthLibBus        gOutputs[1] = {{"Out", 2, false, true}};

static const tSynthLibPluginDesc gDescriptor = {
    .name               = "TiAlike",
    .vendor             = "Chris Purusha",
    .url                = "https://github.com/chrispurusha/TiAlike",
    .email              = "",
    .version            = TIA_VERSION_STRING,

    .isInstrument       = true,
    .vst3SubCategory    = "Instrument|Synth",
    .outputs            = gOutputs,
    .numOutputs         =                                         1,
    .wantsMidiIn        = true,

    .vst3ProcessorUid   = gProcessorUid,
    .vst3ControllerUid  = gControllerUid,
    .auType             = TIA_AU_TYPE,
    .auSubType          = TIA_AU_SUBTYPE,
    .auManufacturer     = TIA_AU_MANUFACTURER,
    .auVersion          = TIA_AU_VERSION,
    .auBundleId         = TIA_AU_BUNDLE_ID,

    .params             = gTiParams,
    .numParams          = kTiParamCount,

    .editorDefaultWidth = TI_CANVAS_W,
    .editorMinWidth     = TI_CANVAS_W * 0.75,
    .editorMaxWidth     = TI_CANVAS_W * 2.0,
    .editorAspect       = TI_CANVAS_W / TI_CANVAS_H,

    .cb                 = {
        .create     = ti_create,
        .destroy    = ti_destroy,
        .prepare    = ti_prepare,
        .setActive  = ti_set_active,
        .process    = ti_process,
        .noteOn     = ti_note_on,
        .noteOff    = ti_note_off,
        .setParam   = ti_set_param,
        .getParam   = ti_get_param,
        .paramText  = ti_param_text,
        .getState   = ti_get_state,
        .setState   = ti_set_state,
        .createView = ti_create_view,
    }
};

static const tSynthLibPluginSet  gSet = {
    .variants = &gDescriptor,
    .count    = 1
};

static const tSynthLibPluginSet * plugin_variants(void) {
    return &gSet;
}

SYNTHLIB_PLUGIN_REGISTER(plugin_variants, "tialike")
