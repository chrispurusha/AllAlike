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
// Notes: Docs/code-notes/osDraw.md - "// notes §k" refers there.

#ifndef __OS_DRAW_H__
#define __OS_DRAW_H__

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "osSample.h"

#define OS_CANVAS_W    (760.0)   // notes §8
#define OS_CANVAS_H    (540.0)

typedef enum {
    eOsControlDial = 0,
    eOsControlChoice,       // a button that opens a list of its values
    eOsControlToggle        // notes §11: Off / On, flipped by a click
} tOsControlKind;

typedef struct {
    uint32_t            paramId;
    const char *        label;
    tOsControlKind      kind;
    const char *const * names;   // a choice's entries
} tOsControl;

typedef struct {
    const char *       title;
    double             colour[3];
    const tOsControl * controls;
    uint32_t           count;
} tOsSection;

typedef struct {
    const char *       title;
    const tOsSection * sections;
    uint32_t           count;
} tOsPage;

// What a click or drag asked for: one parameter, its new normalized value.
typedef struct {
    uint32_t paramId;
    double   normalized;
} tOsEdit;

// notes §1: what the panel asks of the plug-in instance it shows
typedef struct {
    bool (*load)(void * user, const char * path, char * error, size_t errorLength); // a sample chosen
    const tOsSample * (*sample)(void * user);                                       // the one loaded, or NULL; main thread
    const char * (*path)(void * user);                                              // its path, kept even when the file could not be read
    void (*audition)(void * user);
    void (*reset)(void * user);                                                     // every control to its default, through the host                                                  // play it once, at its root key
} tOsSampleHost;

void os_draw_init(void);
void os_draw_set_pages(const tOsPage * pages, uint32_t count);
void os_draw_set_steps(uint32_t paramId, int32_t steps);   // from the parameter table
void os_draw_set_value(uint32_t paramId, double normalized);
void os_draw_set_mouse(double x, double y);
void os_draw_frame(int pixelWidth, int pixelHeight);
bool os_draw_menu_active(void);

// A press: opens a list control's menu, takes a choice from an open one (edit->paramId is UINT32_MAX when
// nothing was set), or grabs a dial (*grabbed, and the pointer is then held while dragBy moves it).
bool os_draw_press(double x, double y, tOsEdit * edit, bool * grabbed);
bool os_draw_drag_by(double dx, double dy, tOsEdit * edit);
bool os_draw_drag_to(double x, double y, tOsEdit * edit);   // Rotary: the pointer's own position
void os_draw_release(double x, double y, tOsEdit * edit);   // a menu choice is taken here
bool os_draw_key(int glfwKey, unsigned int codepoint, bool isRepeat);
void os_draw_scroll(double dy);

void os_draw_set_host(const tOsSampleHost * host);    // static storage
void os_draw_set_user(void * user);

void os_draw_open_sample(void);                       // the file browser
void os_draw_step_sample(int direction);              // notes §2: the previous or next sample in its folder
void os_draw_audition(void);
void os_draw_reset_controls(void);                    // after a confirm

#endif // __OS_DRAW_H__
