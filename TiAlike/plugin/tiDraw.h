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
// Notes: Docs/code-notes/tiDraw.md - "// notes §k" refers there.

#ifndef __TI_DRAW_H__
#define __TI_DRAW_H__

#include <stdbool.h>
#include <stdint.h>

#define TI_CANVAS_W    (780.0)
#define TI_CANVAS_H    (320.0)

typedef enum {
    eTiControlDial = 0,
    eTiControlChoice        // a button that opens a list of its values
} tTiControlKind;

typedef struct {
    uint32_t            paramId;
    const char *        label;
    tTiControlKind      kind;
    int32_t             steps;   // plain range is 0..steps for a dial, entries - 1 for a choice
    double              plainMin;
    const char *const * names;   // a choice's entries
} tTiControl;

typedef struct {
    const char *       title;
    double             colour[3];
    const tTiControl * controls;
    uint32_t           count;
} tTiSection;

typedef struct {
    const char *       title;
    const tTiSection * sections;
    uint32_t           count;
} tTiPage;

// What a click or drag asked for: one parameter, its new normalized value.
typedef struct {
    uint32_t paramId;
    double   normalized;
} tTiEdit;

void ti_draw_init(void);
void ti_draw_set_pages(const tTiPage * pages, uint32_t count);
void ti_draw_set_value(uint32_t paramId, double normalized);
void ti_draw_set_mouse(double x, double y);
void ti_draw_frame(int pixelWidth, int pixelHeight);
bool ti_draw_menu_active(void);

// A press: opens a list control's menu, takes a choice from an open one (edit->paramId is UINT32_MAX when
// nothing was set), or grabs a dial (*grabbed, and the pointer is then held while dragBy moves it).
bool ti_draw_press(double x, double y, tTiEdit * edit, bool * grabbed);
bool ti_draw_drag_by(double dx, double dy, tTiEdit * edit);
bool ti_draw_drag_to(double x, double y, tTiEdit * edit);   // Rotary: the pointer's own position
void ti_draw_release(double x, double y, tTiEdit * edit);   // a menu choice is taken here
bool ti_draw_key(int glfwKey, unsigned int codepoint, bool isRepeat);
void ti_draw_scroll(double dy);

#endif // __TI_DRAW_H__
