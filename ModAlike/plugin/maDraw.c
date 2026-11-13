/*
 * ModAlike - the editor panel.
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
// Notes: Docs/code-notes/maDraw.md - "// notes §k" refers there.

// notes §1 - Nl3Alike's panel (Pulse2Alike's look) without its presets, patch files, tabs and hardware link

#include <math.h>
#include <stdio.h>
#include <string.h>

#include "maDraw.h"
#include "maMenu.h"
#include "maPrefs.h"

#include "synthlibDefs.h"
#include "synthlibGlobals.h"
#include "geometry.h"
#include "synthlibHost.h"
#include "synthlibTypes.h"
#include "utilsGraphics.h"
#include "renderBackend.h"
#include "contextMenu.h"
#include "menuBar.h"
#include "synthlibPopups.h"

#include <GLFW/glfw3.h>           // the key constants SynthLib's popups speak

#define FONT_PATH               "/System/Library/Fonts/Supplemental/Arial.ttf"
#define FONT_PRELOAD_SIZE       (16.0)

#define MAX_PARAMS              (64)
#define MAX_PLACED              (64)
#define MARGIN_X                (16.0)
#define TITLE_Y                 (30.0)       // below the menu bar
#define PAGE_Y                  (70.0)
#define ROW_H                   (112.0)
#define SECTION_GAP             (16.0)
#define CONTROL_W               (58.0)
#define DIAL_SIZE               (40.0)
#define LABEL_H                 (11.0)
#define LABEL_MIN_H             (7.0)
#define HEADING_TO_DIAL         (34.0)
#define BUTTON_H                (12.0)
#define MENU_BG                 {0.22, 0.22, 0.25}
#define MENU_MAX                (32)
#define MENU_CELL_W             (96.0)
#define MENU_ROWS_PER_COLUMN    (8)
#define OPENING_RELEASE_SLOP    (4.0)        // how far the pointer may move and still be the opening click
#define DIM                     (0.35)       // notes §2: how much of its colour a section not in use keeps

typedef struct {
    const tMaControl * control;
    const tMaSection * section;
    tRectangle         dial;         // the dial's square; a choice's button sits across its middle
    double             x;            // the control's column
} tPlaced;

typedef struct {
    const tMaSection * section;
    double             x, y, w;
} tPlacedSection;

static const tMaSection * gSections;
static uint32_t           gSectionCount;
static uint32_t           gEffectParam;
static int32_t            gEffectSteps;
static double             gValue[MAX_PARAMS];  // normalized, as the host holds them
static bool               gFontReady;
static tCoord             gMouse;

static tPlaced            gPlaced[MAX_PLACED];
static uint32_t           gPlacedCount;
static tPlacedSection     gPlacedSections[MAX_PLACED];
static uint32_t           gPlacedSectionCount;

// the dial being dragged; its value runs unrounded while it moves
static const tMaControl * gGrabbed;
static double             gGrabValue;
static tRectangle         gGrabDial;

// the open value menu - which control it sets, and what was chosen
static const tMaControl * gMenuFor;
static int                gMenuChoice = -1;
static tMenuItem          gMenuItems[MENU_MAX + 1];

static bool               gOpeningClick;   // the press that opened a menu, until its release
static tCoord             gOpeningAt;

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

void ma_draw_init(void) {
    synthlib_host_init((tSynthLibHost){.mouseCoord = mouse_coord, .pointerCaptured = pointer_captured});
    synthlib_popups_set_menu_bar(ma_menu_bar(), ma_menu_bar_rect);
    gFontReady = preload_glyph_textures(FONT_PATH, FONT_PRELOAD_SIZE);
}

void ma_draw_set_sections(const tMaSection * sections, uint32_t count, uint32_t effectParam, int32_t effectSteps) {
    gSections     = sections;
    gSectionCount = count;
    gEffectParam  = effectParam;
    gEffectSteps  = effectSteps;
}

void ma_draw_set_value(uint32_t paramId, double normalized) {
    if (paramId < MAX_PARAMS) {
        gValue[paramId] = normalized;
    }
}

void ma_draw_set_mouse(double x, double y) {
    gMouse.x = x;
    gMouse.y = y;
}

bool ma_draw_menu_active(void) {
    return gContextMenu.active || synthlib_popups_modal_active();
}

static void fitted_button(tRectangle r, const char * text, tRgb colour) {
    double h = r.size.h;

    while ((h > LABEL_MIN_H) && (get_text_width(text, h, eNoCache) > r.size.w)) {
        h -= 0.5;
    }
    // where draw_button puts its own label: the box grows by its margin all round, the text starts a margin in
    draw_button(mainArea, r, "", colour);
    set_rgb_colour((tRgb)RGB_BLACK);
    render_text(mainArea, (tRectangle){{r.coord.x + DRAW_BUTTON_MARGIN, r.coord.y + DRAW_BUTTON_MARGIN + ((r.size.h - h) / 2.0)}, {0.0, h}}, text);
}

static int32_t step_of(const tMaControl * c) {
    long step = lround(gValue[c->paramId] * c->steps);

    return (int32_t)((step < 0) ? 0 : ((step > c->steps) ? c->steps : step));
}

// notes §4
static bool control_lit(const tMaControl * c) {
    return (c->litSteps == 0) || (lround(gValue[c->litParam] * c->litSteps) == c->litStep);
}

static bool section_lit(const tMaSection * s) {
    if (s->onlyWhen < 0) {
        return true;
    }
    long effect = lround(gValue[gEffectParam] * gEffectSteps);

    return effect == s->onlyWhen;
}

// Sections flow left to right; one marked newRow, or one that would not fit, starts the next row
static void layout(void) {
    double x = MARGIN_X;
    double y = PAGE_Y;

    gPlacedCount        = 0;
    gPlacedSectionCount = 0;

    for (uint32_t s = 0; (s < gSectionCount) && (gPlacedSectionCount < MAX_PLACED); s++) {
        const tMaSection * section = &gSections[s];
        double             w       = section->count * CONTROL_W;

        if ((x > MARGIN_X) && (section->newRow || ((x + w) > (MA_CANVAS_W - MARGIN_X)))) {
            x  = MARGIN_X;
            y += ROW_H;
        }
        gPlacedSections[gPlacedSectionCount++] = (tPlacedSection){
            section, x, y, w
        };

        for (uint32_t i = 0; (i < section->count) && (gPlacedCount < MAX_PLACED); i++) {
            double cx = x + (i * CONTROL_W);

            gPlaced[gPlacedCount++] = (tPlaced){
                &section->controls[i], section,
                {{
                     cx + ((CONTROL_W - DIAL_SIZE) / 2.0), y + HEADING_TO_DIAL
                 }, {
                     DIAL_SIZE, DIAL_SIZE
                 }
                },
                cx
            };
        }

        x                                     += w + SECTION_GAP;
    }
}

static void value_text(const tMaControl * c, char * out, size_t len) {
    int32_t step = step_of(c);

    if (c->names != NULL) {
        snprintf(out, len, "%s", c->names[step]);
    } else {
        snprintf(out, len, (c->format != NULL) ? c->format : "%.0f", c->plainMin + (step * c->plainStep));
    }
}

static tRgb scaled(tRgb c, double by) {
    return (tRgb){
        c.red * by, c.green * by, c.blue * by
    };
}

void ma_draw_frame(int pixelWidth, int pixelHeight) {
    char text[64];

    if ((pixelWidth <= 0) || (pixelHeight <= 0)) {
        return;
    }
    render_backend_set_surface(pixelWidth, pixelHeight);
    set_render_width(pixelWidth);
    set_render_height(pixelHeight);
    gGlobalGuiScale = (double)pixelWidth / MA_CANVAS_W;
    render_backend_clear((tRgb)RGB_BACKGROUND_GREY);

    if (!gFontReady) {
        render_backend_flush();
        return;
    }
    set_rgb_colour((tRgb){0.95, 0.95, 0.97});
    render_text(mainArea, (tRectangle){{MARGIN_X, TITLE_Y}, {0.0, 18.0}}, "ModAlike");

    layout();

    for (uint32_t s = 0; s < gPlacedSectionCount; s++) {
        tPlacedSection * ps  = &gPlacedSections[s];
        bool             lit = section_lit(ps->section);

        set_rgb_colour(lit ? (tRgb)RGB_WHITE : (tRgb)RGB_GREY_5);
        render_text(mainArea, (tRectangle){{ps->x, ps->y}, {0.0, 12.0}}, ps->section->title);
        set_rgb_colour((tRgb)RGB_GREY_5);
        render_line(mainArea, (tCoord){ps->x, ps->y + 15.0}, (tCoord){ps->x + ps->w - 6.0, ps->y + 15.0}, 1.0);
    }

    for (uint32_t i = 0; i < gPlacedCount; i++) {
        tPlaced *          pc     = &gPlaced[i];
        const tMaControl * c      = pc->control;
        bool               lit    = section_lit(pc->section) && control_lit(c);
        tRgb               colour = {pc->section->colour[0], pc->section->colour[1], pc->section->colour[2]};

        colour = lit ? colour : scaled(colour, DIM);
        set_rgb_colour(lit ? (tRgb)RGB_GREY_7 : (tRgb)RGB_GREY_5);
        render_text(mainArea, (tRectangle){{pc->x, pc->dial.coord.y - 14.0}, {0.0, LABEL_H}}, c->label);
        value_text(c, text, sizeof(text));

        if (c->kind == eMaControlChoice) {
            fitted_button((tRectangle){{pc->x, pc->dial.coord.y + ((DIAL_SIZE - BUTTON_H) / 2.0)}, {CONTROL_W - 6.0, BUTTON_H}}, text, colour);
        } else {
            render_dial(mainArea, pc->dial, (uint32_t)step_of(c), (uint32_t)c->steps + 1u, 0, colour);
            set_rgb_colour(lit ? (tRgb)RGB_WHITE : (tRgb)RGB_GREY_5);
            render_text(mainArea, (tRectangle){{pc->x, pc->dial.coord.y + DIAL_SIZE + 6.0}, {0.0, LABEL_H}}, text);
        }
    }

    render_menu_bar(ma_menu_bar(), ma_menu_bar_rect());
    synthlib_popups_tick();
    synthlib_popups_render();  // menus and dialogues, front-most last
    render_backend_flush();
}

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
    const tMaControl * c       = pc->control;
    int                count   = (c->steps + 1 < MENU_MAX) ? (c->steps + 1) : MENU_MAX;
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

bool ma_draw_press(double x, double y, tMaEdit * edit, bool * grabbed) {
    *grabbed      = false;
    edit->paramId = UINT32_MAX;

    // the popups first: a dialogue takes every press, a menu the presses on it, the menu bar its own; a press
    // anywhere else while a menu is open only closes it
    if (synthlib_popups_dispatch_click((tCoord){x, y}, mouseButtonLeftDown)) {
        return true;
    }

    if (gContextMenu.active) {
        close_context_menu();
        gMenuFor = NULL;
        return true;
    }
    layout();
    const tPlaced * pc = hit(x, y);

    if (pc == NULL) {
        return false;
    }

    if (pc->control->kind == eMaControlChoice) {
        open_value_menu(pc);
        gOpeningClick = true;
        gOpeningAt    = (tCoord){
            x, y
        };
        return true;
    }
    gGrabbed   = pc->control;
    gGrabValue = gValue[pc->control->paramId];
    gGrabDial  = pc->dial;
    *grabbed   = true;
    return true;
}

bool ma_draw_drag_by(double dx, double dy, tMaEdit * edit) {
    double move = (ma_dial_mode() == eDialModeHorizontal) ? dx : -dy;   // up or right raises it

    if (gGrabbed == NULL) {
        return false;
    }
    gGrabValue                = fmin(1.0, fmax(0.0, gGrabValue + (move / dial_drag_pixels_for_full_range((uint32_t)gGrabbed->steps))));
    edit->paramId             = gGrabbed->paramId;
    edit->normalized          = gGrabValue;
    gValue[gGrabbed->paramId] = gGrabValue;
    return true;
}

// Rotary: the pointer stays, and the dial takes its angle
bool ma_draw_drag_to(double x, double y, tMaEdit * edit) {
    if ((gGrabbed == NULL) || (gGrabbed->steps <= 0)) {
        return false;
    }
    uint32_t range = (uint32_t)gGrabbed->steps + 1;

    gGrabValue                = (double)angle_to_value(calculate_mouse_angle((tCoord){x, y}, gGrabDial), range) / (double)gGrabbed->steps;
    edit->paramId             = gGrabbed->paramId;
    edit->normalized          = gGrabValue;
    gValue[gGrabbed->paramId] = gGrabValue;
    return true;
}

// A release ends a drag, and is where a menu's choice is taken (the press opened it, the release on an item picks)
void ma_draw_release(double x, double y, tMaEdit * edit) {
    edit->paramId = UINT32_MAX;
    gGrabbed      = NULL;
    gMenuChoice   = -1;

    // the release of the click that opened a menu is not a choice, even where the menu opened under the pointer
    if (gOpeningClick) {
        gOpeningClick = false;

        if (gContextMenu.active && (fabs(x - gOpeningAt.x) <= OPENING_RELEASE_SLOP) && (fabs(y - gOpeningAt.y) <= OPENING_RELEASE_SLOP)) {
            return;
        }
    }

    if (synthlib_popups_dispatch_click((tCoord){x, y}, mouseButtonLeftUp) && (gMenuChoice >= 0) && (gMenuFor != NULL)) {
        edit->paramId             = gMenuFor->paramId;
        edit->normalized          = (double)gMenuChoice / gMenuFor->steps;
        gValue[gMenuFor->paramId] = edit->normalized;
    }

    if (!gContextMenu.active) {
        gMenuFor = NULL;
    }
}

void ma_draw_scroll(double dy) {
    if (!synthlib_popups_dispatch_scroll(-dy / 20.0) && gContextMenu.active) {
        context_menu_scroll(dy / 20.0);
    }
}

bool ma_draw_key(int glfwKey, unsigned int codepoint, bool isRepeat) {
    int action = isRepeat ? GLFW_REPEAT : GLFW_PRESS;

    if ((glfwKey == GLFW_KEY_ESCAPE) && gContextMenu.active && !synthlib_popups_modal_active()) {
        close_context_menu();
        gMenuFor = NULL;
        return true;
    }

    if (glfwKey != GLFW_KEY_UNKNOWN) {
        return synthlib_popups_dispatch_key(glfwKey, 0, action);
    }
    return (codepoint >= 0x20u) && (codepoint != 0x7Fu) && synthlib_popups_dispatch_char(codepoint);
}
