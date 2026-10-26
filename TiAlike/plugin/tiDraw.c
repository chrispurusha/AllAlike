/*
 * TiAlike - the editor panel.
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
// Notes: Docs/code-notes/tiDraw.md - "// notes §k" refers there. Pulse2Alike's panel, without its presets,
// hardware link and patch bar (notes §1).

#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "tiDraw.h"

#include "synthlibDefs.h"
#include "synthlibGlobals.h"
#include "geometry.h"
#include "synthlibHost.h"
#include "synthlibTypes.h"
#include "utilsGraphics.h"
#include "renderBackend.h"
#include "contextMenu.h"
#include "fileBrowser.h"
#include "menuBar.h"
#include "synthlibPopups.h"

#include "tiMenu.h"
#include "tiPrefs.h"

#define GL_SILENCE_DEPRECATION    1
#include <GLFW/glfw3.h>           // the key constants SynthLib's popups speak

#define FONT_PATH                 "/System/Library/Fonts/Supplemental/Arial.ttf"
#define FONT_PRELOAD_SIZE         (16.0)

#define MAX_PARAMS                (128)
#define MAX_PLACED                (64)
#define MAX_PAGES                 (16)
#define MARGIN_X                  (16.0)
#define TITLE_Y                   (30.0)     // below the menu bar
#define TAB_Y                     (58.0)
#define TAB_H                     (14.0)
#define TAB_GAP                   (6.0)
#define PAGE_Y                    (90.0)
#define ROW_H                     (112.0)
#define SECTION_GAP               (16.0)
#define CONTROL_W                 (58.0)
#define DIAL_SIZE                 (40.0)
#define LABEL_H                   (11.0)
#define HEADING_TO_DIAL           (34.0)
#define BUTTON_H                  (12.0)
#define MENU_BG                   {0.22, 0.22, 0.25}
#define MENU_MAX                  (32)
#define MENU_CELL_W               (96.0)
#define MENU_ROWS_PER_COLUMN      (8)
#define OPENING_RELEASE_SLOP      (4.0)   // notes §3: how far the pointer may move and still be the opening click

typedef struct {
    const tTiControl * control;
    const tTiSection * section;
    tRectangle         dial;          // the dial's square; a choice's button sits across its middle
    double             x;             // the control's column
} tPlaced;

typedef struct {
    const tTiSection * section;
    double             x, y, w;
} tPlacedSection;

static const tTiPage *    gPages;
static uint32_t           gPageCount;
static uint32_t           gPage;
static double             gValue[MAX_PARAMS];   // normalized, as the host holds them
static bool               gFontReady;
static tCoord             gMouse;

static tPlaced            gPlaced[MAX_PLACED];
static uint32_t           gPlacedCount;
static tPlacedSection     gPlacedSections[MAX_PLACED];
static uint32_t           gPlacedSectionCount;
static tRectangle         gTabs[MAX_PAGES];

// the dial being dragged; its value runs unrounded while it moves
static const tTiControl * gGrabbed;
static double             gGrabValue;
static tRectangle         gGrabDial;            // the grabbed dial's square, for a rotary drag

// notes §7: the open value menu - which control it sets, and what was chosen
static const tTiControl * gMenuFor;
static int                gMenuChoice = -1;
static tMenuItem          gMenuItems[MENU_MAX + 1];

static bool               gOpeningClick;     // notes §3: the press that opened a menu, until its release
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

void ti_draw_init(void) {
    synthlib_host_init((tSynthLibHost){.mouseCoord = mouse_coord, .pointerCaptured = pointer_captured});
    synthlib_popups_set_menu_bar(ti_menu_bar(), ti_menu_bar_rect);
    gFontReady = preload_glyph_textures(FONT_PATH, FONT_PRELOAD_SIZE);
}

void ti_draw_set_pages(const tTiPage * pages, uint32_t count) {
    gPages     = pages;
    gPageCount = (count < MAX_PAGES) ? count : MAX_PAGES;

    if (gPage >= gPageCount) {
        gPage = 0;
    }
}

void ti_draw_set_value(uint32_t paramId, double normalized) {
    if (paramId < MAX_PARAMS) {
        gValue[paramId] = normalized;
    }
}

void ti_draw_set_mouse(double x, double y) {
    gMouse.x = x;
    gMouse.y = y;
    (void)handle_file_browser_mouse_move(gMouse);
}

bool ti_draw_menu_active(void) {
    return gContextMenu.active || synthlib_popups_modal_active();
}

// A button whose label is shrunk to fit it (a mod target's name is wider than its box)
#define LABEL_MIN_H    (7.0)

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

static int32_t step_of(const tTiControl * c) {
    long step = lround(gValue[c->paramId] * c->steps);

    return (int32_t)((step < 0) ? 0 : ((step > c->steps) ? c->steps : step));
}

// notes §6: sections flow left to right and wrap onto a new row when the next would not fit
static void layout(void) {
    gPlacedCount        = 0;
    gPlacedSectionCount = 0;

    if ((gPages == NULL) || (gPage >= gPageCount)) {
        return;
    }
    const tTiPage * page = &gPages[gPage];
    double          x    = MARGIN_X;
    double          y    = PAGE_Y;

    for (uint32_t s = 0; (s < page->count) && (gPlacedSectionCount < MAX_PLACED); s++) {
        const tTiSection * section = &page->sections[s];
        double             w       = section->count * CONTROL_W;

        if ((x > MARGIN_X) && ((x + w) > (TI_CANVAS_W - MARGIN_X))) {
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

static void value_text(const tTiControl * c, char * out, size_t len) {
    int32_t step = step_of(c);

    if (c->names != NULL) {
        snprintf(out, len, "%s", c->names[step]);
    } else {
        snprintf(out, len, "%d", (int)lround(c->plainMin + step));
    }
}

static void draw_tabs(void) {
    double x = MARGIN_X;

    for (uint32_t p = 0; p < gPageCount; p++) {
        double w = get_text_width(gPages[p].title, TAB_H - 2.0, eNoCache) + 12.0;

        gTabs[p] = (tRectangle){{
                                    x, TAB_Y
                                }, {
                                    w, TAB_H
                                }
        };
        draw_button(mainArea, gTabs[p], gPages[p].title, (p == gPage) ? (tRgb)RGB_GREEN_ON : (tRgb)RGB_GREY_7);
        x       += w + TAB_GAP;
    }
}

void ti_draw_frame(int pixelWidth, int pixelHeight) {
    char text[64];

    if ((pixelWidth <= 0) || (pixelHeight <= 0)) {
        return;
    }
    render_backend_set_surface(pixelWidth, pixelHeight);
    set_render_width(pixelWidth);
    set_render_height(pixelHeight);
    gGlobalGuiScale = (double)pixelWidth / TI_CANVAS_W;
    render_backend_clear((tRgb)RGB_BACKGROUND_GREY);

    if (!gFontReady) {
        render_backend_flush();
        return;
    }
    set_rgb_colour((tRgb){0.95, 0.95, 0.97});
    render_text(mainArea, (tRectangle){{MARGIN_X, TITLE_Y}, {0.0, 18.0}}, "TiAlike");
    draw_tabs();
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
        const tTiControl * c      = pc->control;
        tRgb               colour = {pc->section->colour[0], pc->section->colour[1], pc->section->colour[2]};

        set_rgb_colour((tRgb)RGB_GREY_7);
        render_text(mainArea, (tRectangle){{pc->x, pc->dial.coord.y - 14.0}, {0.0, LABEL_H}}, c->label);
        value_text(c, text, sizeof(text));

        if (c->kind == eTiControlChoice) {
            fitted_button((tRectangle){{pc->x, pc->dial.coord.y + ((DIAL_SIZE - BUTTON_H) / 2.0)}, {CONTROL_W - 6.0, BUTTON_H}}, text, colour);
        } else {
            render_dial(mainArea, pc->dial, (uint32_t)step_of(c), (uint32_t)c->steps, 0, colour);
            set_rgb_colour((tRgb)RGB_WHITE);
            render_text(mainArea, (tRectangle){{pc->x, pc->dial.coord.y + DIAL_SIZE + 6.0}, {0.0, LABEL_H}}, text);
        }
    }

    render_menu_bar(ti_menu_bar(), ti_menu_bar_rect());
    synthlib_popups_tick();
    synthlib_popups_render();  // menus, the file browser and dialogues, front-most last
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
    const tTiControl * c       = pc->control;
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

// notes §2
bool ti_draw_press(double x, double y, tTiEdit * edit, bool * grabbed) {
    *grabbed      = false;
    edit->paramId = UINT32_MAX;

    // notes §5: the popups first, front to back - a dialogue or the file browser takes every press, a menu the
    // presses on it, the menu bar its own; a press anywhere else while a menu is open only closes it
    if (synthlib_popups_dispatch_click((tCoord){x, y}, mouseButtonLeftDown)) {
        return true;
    }

    if (gContextMenu.active) {
        close_context_menu();
        gMenuFor = NULL;
        return true;
    }

    for (uint32_t p = 0; p < gPageCount; p++) {
        if (within_rectangle((tCoord){x, y}, draw_button_bounds(gTabs[p]))) {
            gPage = p;
            return true;
        }
    }

    layout();
    const tPlaced * pc = hit(x, y);

    if (pc == NULL) {
        return false;
    }

    if (pc->control->kind == eTiControlChoice) {
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

bool ti_draw_drag_by(double dx, double dy, tTiEdit * edit) {
    // up or right raises it, as the Controls menu chose
    double move = (ti_dial_mode() == eDialModeHorizontal) ? dx : -dy;

    if (gGrabbed == NULL) {
        return false;
    }
    gGrabValue                = fmin(1.0, fmax(0.0, gGrabValue + (move / dial_drag_pixels_for_full_range((uint32_t)gGrabbed->steps))));
    edit->paramId             = gGrabbed->paramId;
    edit->normalized          = gGrabValue;
    gValue[gGrabbed->paramId] = gGrabValue;
    return true;
}

// notes §4: Rotary - the pointer stays, and the dial takes its angle
bool ti_draw_drag_to(double x, double y, tTiEdit * edit) {
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
void ti_draw_release(double x, double y, tTiEdit * edit) {
    edit->paramId = UINT32_MAX;
    gGrabbed      = NULL;
    gMenuChoice   = -1;

    // notes §3: the release of the click that opened a menu is not a choice, even where the menu was moved up
    // to fit and so opened under the pointer
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

void ti_draw_scroll(double dy) {
    // the browsers count rows, the wheel logical units
    if (!synthlib_popups_dispatch_scroll(-dy / 20.0) && gContextMenu.active) {
        context_menu_scroll(dy / 20.0);
    }
}

bool ti_draw_key(int glfwKey, unsigned int codepoint, bool isRepeat) {
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
