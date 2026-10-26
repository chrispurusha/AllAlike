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

#include "synthlibPlugin.h"
#include "synthlibLog.h"
#include "synthlibPanelView.h"
#include "synthlibTypes.h"
#include "inputState.h"
#include "tiDraw.h"
#include "tiParams.h"
#include "tiPrefs.h"
#include "tiVoices.h"

// touch /tmp/tialike-log, read /tmp/tialike.log - see SynthLib's plugin/synthlibLog.h.
const char           gSynthLibLogName[] = "tialike";

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

typedef struct {
    tTiVoices voices;
    double    values[kTiParamCount];   // normalized
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

    ti_voices_init(&t->voices, 48000.0);
    ti_voices_set_limit(&t->voices, voice_limit(t));
    ti_prefs_init();
    synthlib_log_line("created - TiAlike %s", TIA_VERSION_STRING);
    return t;
}

static void ti_destroy(void * inst) {
    free(inst);
}

static void ti_prepare(void * inst, const tSynthLibSetup * setup) {
    tTiPlugin * t = (tTiPlugin *)inst;

    ti_voices_set_rate(&t->voices, setup->sampleRate);
}

static void ti_set_active(void * inst, bool active) {
    (void)active;
    ti_voices_all_off(&((tTiPlugin *)inst)->voices);
}

// notes §2: notes land at the block's start for now
static void ti_note_on(void * inst, uint8_t channel, uint8_t note, float velocity, uint32_t sampleOffset) {
    (void)channel;
    (void)sampleOffset;
    ti_voices_note_on(&((tTiPlugin *)inst)->voices, note, velocity);
}

static void ti_note_off(void * inst, uint8_t channel, uint8_t note, float velocity, uint32_t sampleOffset) {
    (void)channel;
    (void)velocity;
    (void)sampleOffset;
    ti_voices_note_off(&((tTiPlugin *)inst)->voices, note);
}

static void ti_process(void * inst, const float *const * in, uint32_t numIn, float ** out, uint32_t numOut,
                       uint32_t frames, const tSynthLibTransport * transport) {
    tTiPlugin * t     = (tTiPlugin *)inst;
    double      gain  = 0.25 * pow(t->values[kTiParamVolume], 2.0);

    (void)in;
    (void)numIn;
    (void)transport;

    if ((out == NULL) || (numOut == 0) || (out[0] == NULL)) {
        return;
    }
    tTiSound    sound = {
        .shape      = (tTiOscShape)lround(t->values[kTiParamOscShape] * (eTiOscShapes - 1)),
        .pulseWidth =                                0.05 + (0.9 * t->values[kTiParamPulseWidth]),
        .cutoff     = t->values[kTiParamCutoff] * 127.0,
        .resonance  = t->values[kTiParamResonance] * 127.0,
        .filterMode = (tTiFilterMode)lround(t->values[kTiParamFilterMode] * (eTiFilterModes - 1)),
    };

    memset(out[0], 0, (size_t)frames * sizeof(float));
    ti_voices_render(&t->voices, &sound, out[0], frames, gain);

    for (uint32_t ch = 1; ch < numOut; ch++) {
        if (out[ch] != NULL) {
            memcpy(out[ch], out[0], (size_t)frames * sizeof(float));
        }
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
    }
}

static double ti_get_param(void * inst, uint32_t id) {
    return (id < kTiParamCount) ? ((tTiPlugin *)inst)->values[id] : 0.0;
}

static bool ti_param_text(const tSynthLibPluginDesc * desc, void * inst, uint32_t id, double value, char * out, size_t len) {
    static const char *const shapes[]  = {"Saw", "Pulse", "Triangle"};
    static const char *const filters[] = {"Low Pass", "High Pass", "Band Pass", "Band Stop", "Analog 1 Pole", "Analog 2 Pole", "Analog 3 Pole", "Analog 4 Pole"};

    (void)desc;
    (void)inst;

    if (id >= kTiParamCount) {
        return false;
    }
    long                     step      = lround(value * gTiParams[id].stepCount);

    if (id == kTiParamOscShape) {
        snprintf(out, len, "%s", shapes[(step < 0) ? 0 : ((step > 2) ? 2 : step)]);
    } else if (id == kTiParamFilterMode) {
        snprintf(out, len, "%s", filters[(step < 0) ? 0 : ((step > 7) ? 7 : step)]);
    } else {
        snprintf(out, len, "%ld", lround(gTiParams[id].plainMin) + step);
    }
    return true;
}

// ---- the editor - notes §3: Pulse2Alike's panel view, with this plug-in's pages --------------------------------

static void ti_panel_init(void) {
    ti_draw_init();
    ti_draw_set_pages(gTiPages, gTiPageCount);
}

static void ti_panel_sync(void * user) {
    for (uint32_t id = 0; id < kTiParamCount; id++) {
        ti_draw_set_value(id, synthlib_plugin_param_value(user, id));
    }
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
        .createView = ti_create_view,
    }
};

static const tSynthLibPluginSet  gSet = {
    .variants = &gDescriptor,
    .count    = 1
};

const tSynthLibPluginSet * synthlib_plugin_variants(void) {
    return &gSet;
}
