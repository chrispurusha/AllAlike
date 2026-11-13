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

#ifndef __MA_DRAW_H__
#define __MA_DRAW_H__

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define MA_CANVAS_W    (780.0)
#define MA_CANVAS_H    (320.0)   // the title and two rows of controls

typedef enum {
    eMaControlDial = 0,
    eMaControlChoice        // a button that opens a list of its values
} tMaControlKind;

typedef struct {
    uint32_t            paramId;
    const char *        label;
    tMaControlKind      kind;
    int32_t             steps;     // a dial's positions less one; a choice's entries less one
    double              plainMin;
    double              plainStep; // a dial's plain value is plainMin + step x plainStep
    const char *        format;    // a dial's text, printf of its plain value
    const char *const * names;     // a choice's entries
    uint32_t            litParam;  // notes §4: drawn dimmed unless this list parameter...
    int32_t             litSteps;  // ...of this many entries less one (0: always lit)...
    int32_t             litStep;   // ...stands at this entry
} tMaControl;

typedef struct {
    const char *       title;
    double             colour[3];
    const tMaControl * controls;
    uint32_t           count;
    bool               newRow;       // starts a row of its own
    int32_t            onlyWhen;     // notes §2: drawn dimmed unless the Effect list stands here; -1 always lit
} tMaSection;

// What a click or drag asked for: one parameter, its new normalized value.
typedef struct {
    uint32_t paramId;
    double   normalized;
} tMaEdit;

void ma_draw_init(void);
void ma_draw_set_sections(const tMaSection * sections, uint32_t count, uint32_t effectParam, int32_t effectSteps);
void ma_draw_set_value(uint32_t paramId, double normalized);
void ma_draw_set_mouse(double x, double y);
void ma_draw_frame(int pixelWidth, int pixelHeight);
bool ma_draw_menu_active(void);

// A press: opens a list control's menu, takes a choice from an open one (edit->paramId is UINT32_MAX when
// nothing was set), or grabs a dial (*grabbed, and the pointer is then held while dragBy moves it).
bool ma_draw_press(double x, double y, tMaEdit * edit, bool * grabbed);
bool ma_draw_drag_by(double dx, double dy, tMaEdit * edit);
bool ma_draw_drag_to(double x, double y, tMaEdit * edit);   // Rotary: the pointer's own position
void ma_draw_release(double x, double y, tMaEdit * edit);   // a menu choice is taken here
bool ma_draw_key(int glfwKey, unsigned int codepoint, bool isRepeat);
void ma_draw_scroll(double dy);

#endif // __MA_DRAW_H__
