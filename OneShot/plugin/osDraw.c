/*
 * OneShot - the editor panel.
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
// Notes: Docs/code-notes/osDraw.md - "// notes §k" refers there. 3pAlike's panel without its presets and
// name field, with a sample bar and the sample's waveform.

#include <dirent.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <time.h>

#include "osDraw.h"

#include "synthlibDefs.h"
#include "synthlibGlobals.h"
#include "geometry.h"
#include "synthlibHost.h"
#include "synthlibTypes.h"
#include "utilsGraphics.h"
#include "renderBackend.h"
#include "contextMenu.h"
#include "fileBrowser.h"
#include "alertDialog.h"
#include "menuBar.h"
#include "synthlibPopups.h"

#include "osEnvelope.h"
#include "osMenu.h"
#include "osParams.h"
#include "osPrefs.h"
#include "osVoice.h"

#define GL_SILENCE_DEPRECATION    1
#include <GLFW/glfw3.h>           // the key constants SynthLib's popups speak

#define FONT_PATH                 "/System/Library/Fonts/Supplemental/Arial.ttf"
#define FONT_PRELOAD_SIZE         (16.0)

#define MAX_PARAMS                (64)
#define MAX_PLACED                (64)
#define MARGIN_X                  (16.0)
#define TITLE_Y                   (30.0)     // below the menu bar
#define BAR_X                     (130.0)
#define BAR_BUTTON_H              (14.0)
#define TAB_Y                     (54.0)     // notes §14: the layer tabs, between the sample bar and the waveform
#define TAB_H                     (18.0)
#define TAB_GAP                   (6.0)
#define MUTE_Y                    (TAB_Y + TAB_H + 3.0)   // notes §15: a Mute under each tab
#define MUTE_H                    (13.0)
#define WAVE_Y                    (98.0)
#define WAVE_H                    (130.0)
#define PAGE_Y                    (248.0)
#define ROW_H                     (112.0)
#define SECTION_GAP               (16.0)
#define CONTROL_W                 (50.0)   // twelve controls across the first row
#define DIAL_SIZE                 (40.0)
#define LABEL_H                   (11.0)
#define HEADING_TO_DIAL           (34.0)
#define BUTTON_H                  (12.0)
#define MENU_BG                   {0.22, 0.22, 0.25}
#define MENU_MAX                  (32)
#define MENU_CELL_W               (96.0)
#define MENU_ROWS_PER_COLUMN      (8)
#define OPENING_RELEASE_SLOP      (4.0)   // how far the pointer may move and still be the opening click
#define MAX_FOLDER_FILES          (4096)
#define INACTIVE_COLOUR           {0.42, 0.42, 0.44}
#define START_COLOUR              {0.95, 0.80, 0.25}
#define PROTECT_COLOUR            {0.80, 0.55, 0.85}
#define TOGGLE_OFF_COLOUR         {0.56, 0.56, 0.58}
#define OUTSIDE_COLOUR            {0.07, 0.07, 0.08} // notes §12: the parts of the view that do not play
#define TAB_COLOUR_TEXT_OFF       {0.40, 0.40, 0.42}
#define MUTE_ON_COLOUR            {0.95, 0.55, 0.25}
#define TAB_COLOUR                {0.22, 0.22, 0.25}
#define TAB_SHOWN_COLOUR          {0.30, 0.42, 0.55}
#define DELAY_COLOUR              {0.20, 0.18, 0.09} // notes §13: the wait between the note on and Start
#define WAVE_COLUMNS              (512)
#define VIEW_MARGIN               (0.06)             // notes §12: the sliver shown either side of Start..End
#define NOTE_OFF_AT               (0.5)              // notes §5: Gate's drawing releases halfway along
#define ENV_SUBSTEPS              (32)               // envelope steps per waveform column, so a short attack shows
#define DOUBLE_CLICK_S            (0.4)              // notes §10: a second press on the same dial within this resets it

typedef struct {
    const tOsControl * control;
    const tOsSection * section;
    tRectangle         dial;         // the dial's square; a choice's button sits across its middle
    double             x;            // the control's column
} tPlaced;

typedef struct {
    const tOsSection * section;
    double             x, y, w;
} tPlacedSection;

static const tOsPage *       gPages;
static uint32_t              gPageCount;
static double                gValues[OS_LAYERS][MAX_PARAMS]; // notes §14: normalized, as the host holds them, per layer
static double *              gValue = gValues[0];            // the shown layer's
static int                   gLayer;
static tRectangle            gTabRect[OS_LAYERS];
static tRectangle            gMuteRect[OS_LAYERS];
static tRectangle            gClearRect[OS_LAYERS];
static int                   gClearing;   // the layer a Clear is being confirmed for
static int32_t               gSteps[MAX_PARAMS];
static bool                  gFontReady;
static tCoord                gMouse;

static tPlaced               gPlaced[MAX_PLACED];
static uint32_t              gPlacedCount;
static tPlacedSection        gPlacedSections[MAX_PLACED];
static uint32_t              gPlacedSectionCount;

// the dial being dragged; its value runs unrounded while it moves
static const tOsControl *    gGrabbed;
static double                gGrabValue;
static tRectangle            gGrabDial;

// the open value menu - which control it sets, and what was chosen
static const tOsControl *    gMenuFor;
static int                   gMenuChoice = -1;
static tMenuItem             gMenuItems[MENU_MAX + 1];
static bool                  gOpeningClick;  // the press that opened a menu, until its release
static tCoord                gOpeningAt;

// notes §10: the last press on a dial, for a double-click
static const tOsControl *    gLastPressed;
static double                gLastPressAt;

static const tOsSampleHost * gHost;
static void *                gUser;
static tRectangle            gPrevRect, gNextRect, gLoadRect, gPlayRect, gResetRect, gWaveRect;

// notes §2: the folder of the sample shown, listed again only when the folder changes
static char                  gFolder[2048];
static char *                gFolderFiles[MAX_FOLDER_FILES];
static int                   gFolderCount;

static void menu_action(int index) {
    gMenuChoice = index;
}

static void mouse_coord(tCoord * coord) {
    if (coord != NULL) {
        *coord = gMouse;
    }
}

static bool pointer_captured(void) {
    return gGrabbed != NULL;
}

void os_draw_init(void) {
    synthlib_host_init((tSynthLibHost){.mouseCoord = mouse_coord, .pointerCaptured = pointer_captured});
    synthlib_popups_set_menu_bar(os_menu_bar(), os_menu_bar_rect);
    gFontReady = preload_glyph_textures(FONT_PATH, FONT_PRELOAD_SIZE);
}

void os_draw_set_pages(const tOsPage * pages, uint32_t count) {
    gPages     = pages;
    gPageCount = count;
}

void os_draw_set_steps(uint32_t paramId, int32_t steps) {
    if (paramId < MAX_PARAMS) {
        gSteps[paramId] = steps;
    }
}

void os_draw_set_value(uint32_t paramId, double normalized) {
    uint32_t layer = paramId / OS_LAYER_STRIDE, id = paramId % OS_LAYER_STRIDE;

    if ((layer < OS_LAYERS) && (id < MAX_PARAMS)) {
        gValues[layer][id] = normalized;
    }
}

int os_draw_layer(void) {
    return gLayer;
}

static void select_layer(int layer) {
    gLayer = layer;
    gValue = gValues[layer];
}

void os_draw_set_mouse(double x, double y) {
    gMouse.x = x;
    gMouse.y = y;
    (void)handle_file_browser_mouse_move(gMouse);
}

bool os_draw_menu_active(void) {
    return gContextMenu.active || synthlib_popups_modal_active();
}

void os_draw_set_host(const tOsSampleHost * host) {
    gHost = host;
}

void os_draw_set_user(void * user) {
    gUser = user;
}

static bool have_host(void) {
    return (gHost != NULL) && (gUser != NULL);
}

static const char * current_path(void) {
    return have_host() ? gHost->path(gUser, gLayer) : "";
}

// ---- loading - notes §1, §2 ---------------------------------------------------------------------------------

static void on_reset_confirmed(bool confirmed) {
    if (confirmed && have_host()) {
        gHost->reset(gUser, gLayer);
    }
}

bool os_draw_oversampling(void) {
    return have_host() && gHost->oversampling(gUser);
}

void os_draw_set_oversampling(bool on) {
    if (have_host()) {
        gHost->setOversampling(gUser, on);
    }
}

static void on_clear_confirmed(bool confirmed) {
    if (confirmed && have_host()) {
        gHost->clear(gUser, gClearing);
    }
}

void os_draw_reset_controls(void) {
    static char message[160];

    snprintf(message, sizeof(message), "Set every dial and switch of layer %d to its default? Its sample stays loaded.", gLayer + 1);
    show_confirm("Reset Controls", message, "Reset", on_reset_confirmed);
}

void os_draw_audition(bool stack) {
    if (have_host()) {
        gHost->audition(gUser, gLayer, stack);
    }
}

static bool load(const char * path, bool reportFailure) {
    static char message[2400];
    char        error[512] = "";

    if (!have_host()) {
        return false;
    }

    if (gHost->load(gUser, gLayer, path, error, sizeof(error))) {
        if (os_audition_on_browse()) {
            gHost->audition(gUser, gLayer, false);
        }
        return true;
    }

    if (reportFailure) {
        snprintf(message, sizeof(message), "%s\n\n%s", strrchr(path, '/') ? strrchr(path, '/') + 1 : path, error);
        show_alert("Load Sample", message);
    }
    return false;
}

static void on_file_chosen(const char * path) {
    if ((path != NULL) && (path[0] != '\0')) {
        (void)load(path, true);
    }
}

static const char * browser_start(void) {
    static char folder[2048];

    snprintf(folder, sizeof(folder), "%s", current_path());
    char *      slash = strrchr(folder, '/');

    if (slash != NULL) {
        *slash = '\0';
        return folder;
    }
    return os_last_folder();
}

static void on_folder_changed(const char * path) {
    os_set_last_folder(path);
}

void os_draw_open_sample(void) {
    set_file_browser_start_directory_provider(browser_start);
    set_file_browser_directory_changed_callback(on_folder_changed);
    open_file_browser_read(on_file_chosen);
}

static int compare_names(const void * a, const void * b) {
    return strcasecmp(*(char *const *)a, *(char *const *)b);
}

static void list_folder(const char * folder) {
    DIR *           dir;
    struct dirent * entry;

    if (strcmp(folder, gFolder) == 0) {
        return;
    }

    for (int i = 0; i < gFolderCount; i++) {
        free(gFolderFiles[i]);
    }

    gFolderCount = 0;
    snprintf(gFolder, sizeof(gFolder), "%s", folder);
    dir          = opendir(folder);

    if (dir == NULL) {
        return;
    }

    while (((entry = readdir(dir)) != NULL) && (gFolderCount < MAX_FOLDER_FILES)) {
        if (os_sample_is_audio_file(entry->d_name)) {
            gFolderFiles[gFolderCount] = strdup(entry->d_name);
            gFolderCount              += (gFolderFiles[gFolderCount] != NULL) ? 1 : 0;
        }
    }
    closedir(dir);
    qsort(gFolderFiles, (size_t)gFolderCount, sizeof(gFolderFiles[0]), compare_names);
}

// The current sample's folder, listed, and its place there (-1 when not found)
static int place_in_folder(void) {
    char         folder[2048];
    const char * path  = current_path();
    const char * slash = strrchr(path, '/');

    if (slash == NULL) {
        return -1;
    }
    snprintf(folder, sizeof(folder), "%.*s", (int)(slash - path), path);
    list_folder(folder);

    for (int i = 0; i < gFolderCount; i++) {
        if (strcmp(gFolderFiles[i], slash + 1) == 0) {
            return i;
        }
    }

    return -1;
}

// notes §2: wraps, and passes over a file that will not load
void os_draw_step_sample(int direction) {
    int  at = place_in_folder();
    char path[4200];

    if (gFolderCount == 0) {
        os_draw_open_sample();
        return;
    }

    if (at < 0) {
        at = (direction > 0) ? -1 : 0;
    }

    for (int tries = 0; tries < gFolderCount; tries++) {
        at = (((at + direction) % gFolderCount) + gFolderCount) % gFolderCount;
        snprintf(path, sizeof(path), "%s/%s", gFolder, gFolderFiles[at]);

        if (load(path, false)) {
            return;
        }
    }
}

// ---- drawing ------------------------------------------------------------------------------------------------

static void fitted_button(tRectangle r, const char * text, tRgb colour) {
    double h = r.size.h;

    while ((h > 7.0) && (get_text_width(text, h, eNoCache) > r.size.w)) {
        h -= 0.5;
    }
    draw_button(mainArea, r, "", colour);
    set_rgb_colour((tRgb)RGB_BLACK);
    render_text(mainArea, (tRectangle){{r.coord.x + DRAW_BUTTON_MARGIN, r.coord.y + DRAW_BUTTON_MARGIN + ((r.size.h - h) / 2.0)}, {0.0, h}}, text);
}

// The text fitted to a width by cutting its end
static void fit_text(char * text, double height, double width) {
    while ((text[0] != '\0') && (get_text_width(text, height, eNoCache) > width)) {
        text[strlen(text) - 1] = '\0';
    }
}

// notes §14: one tab a layer - its number, its sample, and its keys when not the whole keyboard; the shown one lit
static void draw_tabs(void) {
    double width = (OS_CANVAS_W - (2.0 * MARGIN_X) - ((OS_LAYERS - 1) * TAB_GAP)) / OS_LAYERS;

    for (int i = 0; i < OS_LAYERS; i++) {
        const char * path = have_host() ? gHost->path(gUser, i) : "";
        const char * base = strrchr(path, '/');
        bool         full = (lround(gValues[i][kOsParamLowKey] * 127.0) == 0) && (lround(gValues[i][kOsParamHighKey] * 127.0) == 127);
        char         keys[32] = "", low[8], high[8], text[600];

        gTabRect[i] = (tRectangle){{
                                       MARGIN_X + (i * (width + TAB_GAP)), TAB_Y
                                   }, {
                                       width, TAB_H
                                   }
        };
        set_rgb_colour((i == gLayer) ? (tRgb)TAB_SHOWN_COLOUR : (tRgb)TAB_COLOUR);
        (void)render_rectangle(mainArea, gTabRect[i]);

        if (!full) {
            os_param_text(kOsParamLowKey, gValues[i][kOsParamLowKey], low, sizeof(low));
            os_param_text(kOsParamHighKey, gValues[i][kOsParamHighKey], high, sizeof(high));
            snprintf(keys, sizeof(keys), "  %s-%s", low, high);
        }
        snprintf(text, sizeof(text), "%d  %s", i + 1, ((base != NULL) && (base[1] != '\0')) ? base + 1 : "empty");
        fit_text(text, 11.0, width - 10.0 - get_text_width(keys, 11.0, eNoCache));
        strcat(text, keys);
        set_rgb_colour(((base != NULL) && (base[1] != '\0')) ? ((i == gLayer) ? (tRgb)RGB_WHITE : (tRgb)RGB_GREY_7) : (tRgb)RGB_GREY_5);
        render_text(mainArea, (tRectangle){{gTabRect[i].coord.x + 5.0, TAB_Y + 3.0}, {0.0, 11.0}}, text);

        // notes §15: its Mute, lit when muted
        bool   muted = gValues[i][kOsParamMute] > 0.5;

        double half  = (width - TAB_GAP) / 2.0;
        bool   empty = (base == NULL) || (base[1] == '\0');

        gMuteRect[i]  = (tRectangle){{
                                         gTabRect[i].coord.x, MUTE_Y
                                     }, {
                                         half, MUTE_H
                                     }
        };
        gClearRect[i] = (tRectangle){{
                                         gTabRect[i].coord.x + half + TAB_GAP, MUTE_Y
                                     }, {
                                         half, MUTE_H
                                     }
        };
        set_rgb_colour((tRgb)TAB_COLOUR);
        (void)render_rectangle(mainArea, gClearRect[i]);
        set_rgb_colour(empty ? (tRgb)TAB_COLOUR_TEXT_OFF : (tRgb)RGB_GREY_7);
        render_text(mainArea, (tRectangle){{gClearRect[i].coord.x + 5.0, MUTE_Y + 2.0}, {0.0, 9.0}}, "Clear");
        set_rgb_colour(muted ? (tRgb)MUTE_ON_COLOUR : (tRgb)TAB_COLOUR);
        (void)render_rectangle(mainArea, gMuteRect[i]);
        set_rgb_colour(muted ? (tRgb)RGB_BLACK : (tRgb)RGB_GREY_5);
        render_text(mainArea, (tRectangle){{gMuteRect[i].coord.x + 5.0, MUTE_Y + 2.0}, {0.0, 9.0}}, muted ? "Muted" : "Mute");
    }
}

static void draw_sample_bar(void) {
    char         label[600];
    char         place[32] = "";
    const char * path      = current_path();
    const char * base      = strrchr(path, '/');
    int          at        = place_in_folder();
    double       x;

    gPrevRect  = (tRectangle){{
                                  BAR_X, TITLE_Y + 2.0
                              }, {
                                  52.0, BAR_BUTTON_H
                              }
    };
    gNextRect  = (tRectangle){{
                                  BAR_X + 58.0, TITLE_Y + 2.0
                              }, {
                                  52.0, BAR_BUTTON_H
                              }
    };
    gLoadRect  = (tRectangle){{
                                  BAR_X + 116.0, TITLE_Y + 2.0
                              }, {
                                  56.0, BAR_BUTTON_H
                              }
    };
    gPlayRect  = (tRectangle){{
                                  BAR_X + 178.0, TITLE_Y + 2.0
                              }, {
                                  40.0, BAR_BUTTON_H
                              }
    };
    gResetRect = (tRectangle){{
                                  BAR_X + 224.0, TITLE_Y + 2.0
                              }, {
                                  44.0, BAR_BUTTON_H
                              }
    };
    draw_button(mainArea, gPrevRect, "< Prev", (tRgb)RGB_GREY_7);
    draw_button(mainArea, gNextRect, "Next >", (tRgb)RGB_GREY_7);
    draw_button(mainArea, gLoadRect, "Load...", (tRgb)RGB_GREY_7);
    draw_button(mainArea, gPlayRect, "Play", (tRgb)RGB_GREEN_ON);
    draw_button(mainArea, gResetRect, "Reset", (tRgb)RGB_GREY_7);

    x          = gResetRect.coord.x + draw_button_bounds(gResetRect).size.w + 10.0;

    if ((base == NULL) || (base[1] == '\0')) {
        set_rgb_colour((tRgb)RGB_GREY_5);
        render_text(mainArea, (tRectangle){{x, TITLE_Y + 4.0}, {0.0, 11.0}}, "No sample - Load... one");
        return;
    }

    if (at >= 0) {
        snprintf(place, sizeof(place), "  %d/%d", at + 1, gFolderCount);
    }
    snprintf(label, sizeof(label), "%s", base + 1);
    fit_text(label, 11.0, OS_CANVAS_W - MARGIN_X - x - get_text_width(place, 11.0, eNoCache));
    strcat(label, place);
    set_rgb_colour((tRgb)RGB_WHITE);
    render_text(mainArea, (tRectangle){{x, TITLE_Y + 4.0}, {0.0, 11.0}}, label);
}

// notes §12: the peaks of the frames [from, to) as played, a column each - worked out again only when the view moves
static struct {
    uint32_t serial;
    int64_t  from, to;          // may start before the sample: a long Delay is drawn ahead of it (§13)
    bool     reverse;
    float    lo[WAVE_COLUMNS], hi[WAVE_COLUMNS];
} gView;

static void view_peaks(const tOsSample * s, int64_t from, int64_t to, bool reverse) {
    if ((gView.serial == s->serial) && (gView.from == from) && (gView.to == to) && (gView.reverse == reverse)) {
        return;
    }
    gView.serial  = s->serial;
    gView.from    = from;
    gView.to      = to;
    gView.reverse = reverse;

    for (int i = 0; i < WAVE_COLUMNS; i++) {
        int64_t a = from + (((to - from) * i) / WAVE_COLUMNS);
        int64_t b = from + (((to - from) * (i + 1)) / WAVE_COLUMNS);
        float   lo = 0.0f, hi = 0.0f;

        b           = (b <= a) ? a + 1 : b;
        a           = (a < 0) ? 0 : a; // before the sample: nothing

        for (int64_t j = a; (j < b) && (j < (int64_t)s->frames); j++) {
            uint32_t k = reverse ? (uint32_t)(s->frames - 1 - j) : (uint32_t)j;
            float    v = 0.5f * (s->data[0][k] + s->data[1][k]);

            lo = fminf(lo, v);
            hi = fmaxf(hi, v);
        }

        gView.lo[i] = lo;
        gView.hi[i] = hi;
    }
}

static void wave_marker(double x, tRgb colour, double width) {
    set_rgb_colour(colour);
    render_line(mainArea, (tCoord){x, gWaveRect.coord.y + 2.0}, (tCoord){x, gWaveRect.coord.y + WAVE_H - 2.0}, width);
}

// notes §3, §12: what plays, from Start to End, with a sliver either side of what does not
static void draw_waveform(void) {
    const tOsSample * s        = have_host() ? gHost->sample(gUser, gLayer) : NULL;
    char              text[600];

    gWaveRect = (tRectangle){{
                                 MARGIN_X, WAVE_Y
                             }, {
                                 OS_CANVAS_W - (2.0 * MARGIN_X), WAVE_H
                             }
    };
    set_rgb_colour((tRgb){0.12, 0.12, 0.14});
    (void)render_rectangle(mainArea, gWaveRect);

    double            mid      = gWaveRect.coord.y + (WAVE_H / 2.0);
    double            half     = (WAVE_H / 2.0) - 4.0;

    if (s == NULL) {
        const char * path = current_path();

        set_rgb_colour((tRgb)RGB_GREY_5);
        render_line(mainArea, (tCoord){gWaveRect.coord.x, mid}, (tCoord){gWaveRect.coord.x + gWaveRect.size.w, mid}, 1.0);

        if (path[0] != '\0') {
            snprintf(text, sizeof(text), "Missing: %s", path);
            fit_text(text, 11.0, gWaveRect.size.w - 12.0);
            set_rgb_colour((tRgb){0.95, 0.45, 0.35});
            render_text(mainArea, (tRectangle){{gWaveRect.coord.x + 6.0, mid - 6.0}, {0.0, 11.0}}, text);
        }
        return;
    }
    // notes §6: Start and End snapped as the voice snaps them, in frames as played
    bool              reverse  = gValue[kOsParamReverse] > 0.5;
    tOsPlay           play     = {
        .start = os_start_time(gValue[kOsParamStart]), .end     = os_end_fraction(gValue[kOsParamEnd]),
        .snap  = gValue[kOsParamSnap] > 0.5,           .reverse = reverse
    };
    uint32_t          start    = os_voices_start_frame(s, &play);
    uint32_t          end      = os_voices_end_frame(s, &play);
    bool              silent   = (end <= start);
    uint32_t          span     = silent ? s->frames : (end - start);
    int64_t           margin   = (int64_t)(span * VIEW_MARGIN);
    // notes §13: the Delay in the sample's frames - real time, so shorter by the Stretch multiple (Sync not counted)
    double            wait     = os_delay_time(gValue[kOsParamDelay]) * s->rate / os_stretch_multiple(gValue[kOsParamStretch]);
    int64_t           noteOn   = (int64_t)start - (int64_t)llround(wait);
    int64_t           from     = silent ? 0 : (((int64_t)start > margin) ? (int64_t)start - margin : 0);
    int64_t           to       = silent ? s->frames : (((int64_t)end + margin < (int64_t)s->frames) ? (int64_t)end + margin : s->frames);
    bool              delayed  = !silent && (wait > 0.0);

    if (delayed && (noteOn - (margin / 2) < from)) {
        from = noteOn - (margin / 2);   // the note-on line in view, with a little before it
    }
    to = (to <= from) ? from + 1 : to;
    view_peaks(s, from, to, reverse);
    double            perFrame = gWaveRect.size.w / (double)(to - from);
    double            xStart   = gWaveRect.coord.x + ((double)((int64_t)start - from) * perFrame);
    double            xEnd     = gWaveRect.coord.x + ((double)((int64_t)end - from) * perFrame);
    double            xNoteOn  = gWaveRect.coord.x + ((double)(noteOn - from) * perFrame);

    // the parts that do not play, darker; the Delay's wait between the note on and Start, tinted (§13)
    set_rgb_colour((tRgb)OUTSIDE_COLOUR);

    if (!silent && (xStart > gWaveRect.coord.x)) {
        (void)render_rectangle(mainArea, (tRectangle){gWaveRect.coord, {xStart - gWaveRect.coord.x, WAVE_H}});
    }

    if (delayed) {
        double x0 = fmin(xNoteOn, xStart - 2.0);   // never thinner than a line

        set_rgb_colour((tRgb)DELAY_COLOUR);
        (void)render_rectangle(mainArea, (tRectangle){{x0, gWaveRect.coord.y}, {xStart - x0, WAVE_H}});
    }

    if (!silent && (xEnd < gWaveRect.coord.x + gWaveRect.size.w)) {
        set_rgb_colour((tRgb)OUTSIDE_COLOUR);
        (void)render_rectangle(mainArea, (tRectangle){{xEnd, gWaveRect.coord.y}, {gWaveRect.coord.x + gWaveRect.size.w - xEnd, WAVE_H}});
    }
    set_rgb_colour((tRgb)RGB_GREY_5);
    render_line(mainArea, (tCoord){gWaveRect.coord.x, mid}, (tCoord){gWaveRect.coord.x + gWaveRect.size.w, mid}, 1.0);

    // notes §5: the envelope from Start, as a note held throughout, stopped at End; Gate's note off halfway between
    tOsEnvShape shape    = os_env_shape(gValue);
    tOsEnv      env;
    double      level[WAVE_COLUMNS];
    double      column   = (double)(to - from) / WAVE_COLUMNS;         // frames a column
    double      dt       = column / s->rate / ENV_SUBSTEPS;
    bool        gate     = os_is_gate(gValue);
    double      noteOff  = (double)start + (NOTE_OFF_AT * (double)span);
    bool        released = false;

    os_env_start(&env);

    for (int i = 0; i < WAVE_COLUMNS; i++) {
        double at   = (double)from + ((i + 0.5) * column);
        double peak = 0.0;

        if (silent || (at < (double)start) || (at >= (double)end)) {
            level[i] = 0.0;
            continue;
        }

        if (gate && !released && (at >= noteOff)) {
            os_env_release(&env);
            released = true;
        }

        for (int k = 0; k < ENV_SUBSTEPS; k++) {
            peak = fmax(peak, os_env_step(&env, &shape, dt));
        }

        level[i] = peak;
    }

    for (int i = 0; i < WAVE_COLUMNS; i++) {
        double x = gWaveRect.coord.x + ((i + 0.5) * gWaveRect.size.w / WAVE_COLUMNS);
        double lo = fmax(-1.0, gView.lo[i]), hi = fmin(1.0, gView.hi[i]);

        set_rgb_colour((tRgb){0.28, 0.32, 0.38});
        render_line(mainArea, (tCoord){x, mid - (hi * half)}, (tCoord){x, mid - (lo * half) + 0.5}, 1.0);

        if (level[i] > 0.0) {
            set_rgb_colour((tRgb){0.40, 0.75, 0.95});
            render_line(mainArea, (tCoord){x, mid - (hi * level[i] * half)}, (tCoord){x, mid - (lo * level[i] * half) + 0.5}, 1.0);
        }
    }

    set_rgb_colour((tRgb){0.55, 0.85, 0.40});

    for (int i = 1; i < WAVE_COLUMNS; i++) {
        double x0 = gWaveRect.coord.x + ((i - 0.5) * gWaveRect.size.w / WAVE_COLUMNS);
        double x1 = gWaveRect.coord.x + ((i + 0.5) * gWaveRect.size.w / WAVE_COLUMNS);
        double top = gWaveRect.coord.y + 4.0, h = WAVE_H - 8.0;

        render_line(mainArea, (tCoord){x0, top + ((1.0 - level[i - 1]) * h)}, (tCoord){x1, top + ((1.0 - level[i]) * h)}, 1.5);
    }

    if (gate && !silent) {
        double x = gWaveRect.coord.x + ((noteOff - (double)from) * perFrame);

        set_rgb_colour((tRgb)RGB_GREY_7);
        render_line(mainArea, (tCoord){x, gWaveRect.coord.y + 16.0}, (tCoord){x, gWaveRect.coord.y + WAVE_H - 2.0}, 1.0);
        render_text(mainArea, (tRectangle){{x + 4.0, gWaveRect.coord.y + 16.0}, {0.0, 10.0}}, "note off");
    }

    if ((start > 0) || delayed) {
        wave_marker(xStart, (tRgb)START_COLOUR, 1.5);
    }

    if (delayed) {
        char   label[64] = "delay ";
        double x0        = fmin(xNoteOn, xStart - 2.0);

        wave_marker(x0, (tRgb)RGB_GREY_7, 1.0);
        os_param_text(kOsParamDelay, gValue[kOsParamDelay], label + 6, sizeof(label) - 6);
        set_rgb_colour((tRgb)RGB_GREY_7);
        render_text(mainArea, (tRectangle){{x0 + 4.0, gWaveRect.coord.y + WAVE_H - 16.0}, {0.0, 10.0}}, label);
        render_text(mainArea, (tRectangle){{x0 + 4.0, gWaveRect.coord.y + WAVE_H - 28.0}, {0.0, 10.0}}, "note on");
    }

    if (end < s->frames) {
        wave_marker(xEnd, (tRgb)START_COLOUR, 1.5);
    }

    // notes §9: where the protected part ends, while stretching
    if (os_param_active(kOsParamProtect, gValue)) {
        tOsPlay snapped = {.protect = os_protect_time(gValue[kOsParamProtect]), .protectSnap = gValue[kOsParamProtectSnap] > 0.5, .reverse = reverse};
        double  protect = (double)os_voices_protect_frame(s, &snapped);   // notes §9: snapped as the copy is built

        if ((protect >= (double)from) && (protect <= (double)to)) {
            wave_marker(gWaveRect.coord.x + ((protect - (double)from) * perFrame), (tRgb)PROTECT_COLOUR, 1.0);
        }
    }
    snprintf(text, sizeof(text), "%.3f s   %.0f Hz   %s", s->frames / s->rate, s->rate, (s->channels == 2) ? "stereo" : "mono");

    if ((from != 0) || (to < (int64_t)s->frames)) {
        size_t used = strlen(text);

        snprintf(text + used, sizeof(text) - used, "   showing %.3f - %.3f s", from / s->rate, to / s->rate);
    }
    set_rgb_colour((tRgb)RGB_GREY_7);
    render_text(mainArea, (tRectangle){{gWaveRect.coord.x + 6.0, gWaveRect.coord.y + 4.0}, {0.0, 10.0}}, text);
}

static int32_t step_of(const tOsControl * c) {
    int32_t steps = gSteps[c->paramId];
    long    step  = lround(gValue[c->paramId] * steps);

    return (int32_t)((step < 0) ? 0 : ((step > steps) ? steps : step));
}

// sections flow left to right and wrap onto a new row when the next would not fit
static void layout(void) {
    gPlacedCount        = 0;
    gPlacedSectionCount = 0;

    if ((gPages == NULL) || (gPageCount == 0)) {
        return;
    }
    const tOsPage * page = &gPages[0];
    double          x    = MARGIN_X;
    double          y    = PAGE_Y;

    for (uint32_t s = 0; (s < page->count) && (gPlacedSectionCount < MAX_PLACED); s++) {
        const tOsSection * section = &page->sections[s];
        double             w       = section->count * CONTROL_W;

        if ((x > MARGIN_X) && ((x + w) > (OS_CANVAS_W - MARGIN_X))) {
            x  = MARGIN_X;
            y += ROW_H;
        }
        gPlacedSections[gPlacedSectionCount++] = (tPlacedSection){
            section, x, y, w
        };

        for (uint32_t i = 0; (i < section->count) && (gPlacedCount < MAX_PLACED); i++) {
            double cx = x + (i * CONTROL_W);

            gPlaced[gPlacedCount++] = (tPlaced){
                &section->controls[i], section, {{
                                                     cx + ((CONTROL_W - DIAL_SIZE) / 2.0), y + HEADING_TO_DIAL
                                                 }, {
                                                     DIAL_SIZE, DIAL_SIZE
                                                 }
                }, cx
            };
        }

        x                                     += w + SECTION_GAP;
    }
}

void os_draw_frame(int pixelWidth, int pixelHeight) {
    char text[64];

    if ((pixelWidth <= 0) || (pixelHeight <= 0)) {
        return;
    }
    render_backend_set_surface(pixelWidth, pixelHeight);
    set_render_width(pixelWidth);
    set_render_height(pixelHeight);
    gGlobalGuiScale = (double)pixelWidth / OS_CANVAS_W;
    render_backend_clear((tRgb)RGB_BACKGROUND_GREY);

    if (!gFontReady) {
        render_backend_flush();
        return;
    }
    set_rgb_colour((tRgb){0.95, 0.95, 0.97});
    render_text(mainArea, (tRectangle){{MARGIN_X, TITLE_Y}, {0.0, 18.0}}, "OneShot");
    draw_sample_bar();
    draw_tabs();
    draw_waveform();
    layout();

    for (uint32_t s = 0; s < gPlacedSectionCount; s++) {
        tPlacedSection * ps = &gPlacedSections[s];

        set_rgb_colour((tRgb)RGB_WHITE);
        render_text(mainArea, (tRectangle){{ps->x, ps->y}, {0.0, 12.0}}, ps->section->title);
        set_rgb_colour((tRgb)RGB_GREY_5);
        render_line(mainArea, (tCoord){ps->x, ps->y + 15.0}, (tCoord){ps->x + ps->w - 6.0, ps->y + 15.0}, 1.0);
    }

    for (uint32_t i = 0; i < gPlacedCount; i++) {
        tPlaced *          pc     = &gPlaced[i];
        const tOsControl * c      = pc->control;
        bool               active = os_param_active(c->paramId, gValue);
        tRgb               colour = active ? (tRgb){
            pc->section->colour[0], pc->section->colour[1], pc->section->colour[2]
        }
                                                                                                                            : (tRgb)INACTIVE_COLOUR;

        set_rgb_colour(active ? (tRgb)RGB_GREY_7 : (tRgb)RGB_GREY_5);
        render_text(mainArea, (tRectangle){{pc->x, pc->dial.coord.y - 14.0}, {0.0, LABEL_H}}, c->label);
        os_param_text(c->paramId, gValue[c->paramId], text, sizeof(text));

        if ((c->paramId == kOsParamEnd) && have_host() && (gHost->sample(gUser, gLayer) != NULL)) {
            const tOsSample * s    = gHost->sample(gUser, gLayer);
            tOsPlay           play = {
                .end     = os_end_fraction(gValue[kOsParamEnd]), .snap = gValue[kOsParamSnap] > 0.5,
                .reverse = gValue[kOsParamReverse] > 0.5
            };

            os_time_text(os_voices_end_frame(s, &play) / s->rate, text, sizeof(text));   // notes §12: End as a time
        }

        if (c->kind == eOsControlToggle) {
            tRgb lit = (step_of(c) > 0) ? colour : (tRgb)TOGGLE_OFF_COLOUR;

            fitted_button((tRectangle){{pc->x, pc->dial.coord.y + ((DIAL_SIZE - BUTTON_H) / 2.0)}, {CONTROL_W - 6.0, BUTTON_H}}, text, lit);
        } else if (c->kind == eOsControlChoice) {
            fitted_button((tRectangle){{pc->x, pc->dial.coord.y + ((DIAL_SIZE - BUTTON_H) / 2.0)}, {CONTROL_W - 6.0, BUTTON_H}}, text, colour);
        } else {
            render_dial(mainArea, pc->dial, (uint32_t)step_of(c), (uint32_t)gSteps[c->paramId] + 1u, 0, colour);
            set_rgb_colour(active ? (tRgb)RGB_WHITE : (tRgb)RGB_GREY_5);
            render_text(mainArea, (tRectangle){{pc->x, pc->dial.coord.y + DIAL_SIZE + 6.0}, {0.0, LABEL_H}}, text);
        }
    }

    render_menu_bar(os_menu_bar(), os_menu_bar_rect());
    synthlib_popups_tick();
    synthlib_popups_render();  // menus, the file browser and dialogues, front-most last
    render_backend_flush();
}

// ---- input --------------------------------------------------------------------------------------------------

static const tPlaced * hit(double x, double y) {
    for (uint32_t i = 0; i < gPlacedCount; i++) {
        tPlaced * pc = &gPlaced[i];

        if ((x >= pc->x) && (x < pc->x + CONTROL_W) && (y >= pc->dial.coord.y - 16.0) && (y < pc->dial.coord.y + DIAL_SIZE + 20.0)) {
            return pc;
        }
    }

    return NULL;
}

static void open_value_menu(const tPlaced * pc) {
    const tOsControl * c       = pc->control;
    int                count   = (gSteps[c->paramId] + 1 < MENU_MAX) ? (gSteps[c->paramId] + 1) : MENU_MAX;
    uint32_t           columns = (uint32_t)((count + MENU_ROWS_PER_COLUMN - 1) / MENU_ROWS_PER_COLUMN);

    for (int i = 0; i < count; i++) {
        gMenuItems[i] = (tMenuItem){
            .label = c->names[i], .colour = (tRgb)MENU_BG, .action = menu_action, .param = (uint32_t)i
        };
    }

    gMenuItems[count] = (tMenuItem){
        .label = NULL, .action = NULL
    };
    gMenuFor          = c;
    gMenuChoice       = -1;
    open_context_menu(below_rect(pc->dial), gMenuItems, columns, MENU_CELL_W);
}

bool os_draw_press(double x, double y, tOsEdit * edit, bool * grabbed) {
    tCoord at = {x, y};

    *grabbed      = false;
    edit->paramId = UINT32_MAX;

    // the popups first, front to back; a press anywhere else while a menu is open only closes it
    if (synthlib_popups_dispatch_click(at, mouseButtonLeftDown)) {
        return true;
    }

    if (gContextMenu.active) {
        close_context_menu();
        gMenuFor = NULL;
        return true;
    }

    if (within_rectangle(at, draw_button_bounds(gPrevRect))) {
        os_draw_step_sample(-1);
        return true;
    }

    if (within_rectangle(at, draw_button_bounds(gNextRect))) {
        os_draw_step_sample(1);
        return true;
    }

    if (within_rectangle(at, draw_button_bounds(gLoadRect))) {
        os_draw_open_sample();
        return true;
    }

    if (within_rectangle(at, draw_button_bounds(gResetRect))) {
        os_draw_reset_controls();
        return true;
    }

    // notes §15: a Mute flips its own layer's, whichever is shown
    for (int i = 0; i < OS_LAYERS; i++) {
        if (within_rectangle(at, gMuteRect[i])) {
            edit->paramId            = kOsParamMute;
            edit->normalized         = (gValues[i][kOsParamMute] > 0.5) ? 0.0 : 1.0;
            edit->layer              = i;
            gValues[i][kOsParamMute] = edit->normalized;
            return true;
        }
    }

    // notes §15: Clear empties its layer, once confirmed
    for (int i = 0; i < OS_LAYERS; i++) {
        if (within_rectangle(at, gClearRect[i])) {
            const char * path = have_host() ? gHost->path(gUser, i) : "";

            if (path[0] != '\0') {
                static char message[160];

                gClearing = i;
                snprintf(message, sizeof(message), "Empty layer %d? Its settings stay; Load... fills it again.", i + 1);
                show_confirm("Clear Layer", message, "Clear", on_clear_confirmed);
            }
            return true;
        }
    }

    // notes §14: a tab shows its layer
    for (int i = 0; i < OS_LAYERS; i++) {
        if (within_rectangle(at, gTabRect[i])) {
            select_layer(i);
            return true;
        }
    }

    if (within_rectangle(at, draw_button_bounds(gPlayRect))) {
        os_draw_audition(true);
        return true;
    }

    if (within_rectangle(at, gWaveRect)) {
        os_draw_audition(false);
        return true;
    }
    layout();
    const tPlaced * pc = hit(x, y);

    if (pc == NULL) {
        return false;
    }

    if (!os_param_active(pc->control->paramId, gValue)) {
        return true;   // greyed: it does nothing in this mode
    }

    // notes §11: a toggle flips at the press
    if (pc->control->kind == eOsControlToggle) {
        edit->paramId                = pc->control->paramId;
        edit->normalized             = (step_of(pc->control) > 0) ? 0.0 : 1.0;
        gValue[pc->control->paramId] = edit->normalized;
        return true;
    }

    if (pc->control->kind == eOsControlChoice) {
        open_value_menu(pc);
        gOpeningClick = true;
        gOpeningAt    = at;
        return true;
    }
    // notes §10: a double-click puts the dial back to its default instead of grabbing it
    struct timespec now;

    clock_gettime(CLOCK_MONOTONIC, &now);
    double          seconds = (double)now.tv_sec + ((double)now.tv_nsec * 1e-9);

    if ((pc->control == gLastPressed) && ((seconds - gLastPressAt) < DOUBLE_CLICK_S)) {
        edit->paramId    = pc->control->paramId;
        edit->normalized = gOsParams[pc->control->paramId].defaultNormalized;
        gLastPressed     = NULL;
        return true;
    }
    gLastPressed = pc->control;
    gLastPressAt = seconds;
    gGrabbed     = pc->control;
    gGrabValue   = gValue[pc->control->paramId];
    gGrabDial    = pc->dial;
    *grabbed     = true;
    return true;
}

bool os_draw_drag_by(double dx, double dy, tOsEdit * edit) {
    // up or right raises it, as the Controls menu chose
    double move = (os_dial_mode() == eDialModeHorizontal) ? dx : -dy;

    if (gGrabbed == NULL) {
        return false;
    }
    gGrabValue                = fmin(1.0, fmax(0.0, gGrabValue + (move / dial_drag_pixels_for_full_range((uint32_t)gSteps[gGrabbed->paramId]))));
    edit->paramId             = gGrabbed->paramId;
    edit->normalized          = gGrabValue;
    gValue[gGrabbed->paramId] = gGrabValue;
    return true;
}

// Rotary - the pointer stays, and the dial takes its angle
bool os_draw_drag_to(double x, double y, tOsEdit * edit) {
    if ((gGrabbed == NULL) || (gSteps[gGrabbed->paramId] <= 0)) {
        return false;
    }
    int32_t steps = gSteps[gGrabbed->paramId];

    gGrabValue                = (double)angle_to_value(calculate_mouse_angle((tCoord){x, y}, gGrabDial), (uint32_t)steps + 1u) / (double)steps;
    edit->paramId             = gGrabbed->paramId;
    edit->normalized          = gGrabValue;
    gValue[gGrabbed->paramId] = gGrabValue;
    return true;
}

// A release ends a drag, and is where a menu's choice is taken
void os_draw_release(double x, double y, tOsEdit * edit) {
    edit->paramId = UINT32_MAX;
    gGrabbed      = NULL;
    gMenuChoice   = -1;

    if (gOpeningClick) {
        gOpeningClick = false;

        if (gContextMenu.active && (fabs(x - gOpeningAt.x) <= OPENING_RELEASE_SLOP) && (fabs(y - gOpeningAt.y) <= OPENING_RELEASE_SLOP)) {
            return;
        }
    }

    if (synthlib_popups_dispatch_click((tCoord){x, y}, mouseButtonLeftUp) && (gMenuChoice >= 0) && (gMenuFor != NULL)) {
        edit->paramId             = gMenuFor->paramId;
        edit->normalized          = (double)gMenuChoice / gSteps[gMenuFor->paramId];
        gValue[gMenuFor->paramId] = edit->normalized;
    }

    if (!gContextMenu.active) {
        gMenuFor = NULL;
    }
}

void os_draw_scroll(double dy) {
    // the browsers count rows, the wheel logical units
    if (!synthlib_popups_dispatch_scroll(-dy / 20.0) && gContextMenu.active) {
        context_menu_scroll(dy / 20.0);
    }
}

// notes §4: with no popup open, the arrow keys step through the folder and Space plays
bool os_draw_key(int glfwKey, unsigned int codepoint, bool isRepeat) {
    int action = isRepeat ? GLFW_REPEAT : GLFW_PRESS;

    if ((glfwKey == GLFW_KEY_ESCAPE) && gContextMenu.active && !synthlib_popups_modal_active()) {
        close_context_menu();
        gMenuFor = NULL;
        return true;
    }

    if (!synthlib_popups_modal_active() && !gContextMenu.active) {
        if ((glfwKey == GLFW_KEY_RIGHT) || (glfwKey == GLFW_KEY_DOWN)) {
            os_draw_step_sample(1);
            return true;
        }

        if ((glfwKey == GLFW_KEY_LEFT) || (glfwKey == GLFW_KEY_UP)) {
            os_draw_step_sample(-1);
            return true;
        }

        if ((glfwKey == GLFW_KEY_SPACE) || ((glfwKey == GLFW_KEY_UNKNOWN) && (codepoint == ' '))) {
            os_draw_audition(true);
            return true;
        }
    }

    if (glfwKey != GLFW_KEY_UNKNOWN) {
        return synthlib_popups_dispatch_key(glfwKey, 0, action);
    }
    return (codepoint >= 0x20u) && (codepoint != 0x7Fu) && synthlib_popups_dispatch_char(codepoint);
}
