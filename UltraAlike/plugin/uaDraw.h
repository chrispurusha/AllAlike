/*
 * UltraAlike - the editor panel.
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
// Notes: Docs/code-notes/uaDraw.md - "// notes §k" refers there.

#ifndef __UA_DRAW_H__
#define __UA_DRAW_H__

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "uaSample.h"

#define UA_CANVAS_W    (860.0)   // notes §8
#define UA_CANVAS_H    (584.0)

typedef enum {
    eUaControlDial = 0,
    eUaControlChoice,       // a button that opens a list of its values
    eUaControlToggle        // notes §11: Off / On, flipped by a click
} tUaControlKind;

typedef struct {
    uint32_t            paramId;
    const char *        label;
    tUaControlKind      kind;
    const char *const * names;   // a choice's entries
} tUaControl;

typedef struct {
    const char *       title;
    double             colour[3];
    const tUaControl * controls;
    uint32_t           count;
} tUaSection;

typedef struct {
    const char *       title;
    const tUaSection * sections;
    uint32_t           count;
} tUaPage;

// What a click or drag asked for: one parameter, its new normalized value.
typedef struct {
    uint32_t paramId;
    double   normalized;
    int32_t  layer;        // notes §15: the layer it is for; -1 (set by the caller) the shown one
} tUaEdit;

// notes §1: what the panel asks of the plug-in instance it shows
// notes §14: each for one layer, 0..UA_LAYERS-1
typedef struct {
    bool (*load)(void * user, int layer, const char * path, char * error, size_t errorLength); // a sample chosen
    const tUaSample * (*sample)(void * user, int layer);                                       // the one loaded, or NULL; main thread
    const char * (*path)(void * user, int layer);                                              // its path, kept even when the file could not be read
    void (*audition)(void * user, int layer, bool stack);                                      // at its root key: that layer alone, or every layer the key reaches
    void (*reset)(void * user, int layer);                                                     // every control to its default, through the host
    void (*clear)(void * user, int layer);                                                     // notes §15: back to empty; its settings kept
    bool (*oversampling)(void * user);                                                         // notes §16: the whole instance's, not a layer's
    void (*setOversampling)(void * user, bool on);
    bool (*stepPreset)(void * user, int layer, int direction);                                 // notes §18: false unless it plays a bank
    bool (*presetText)(void * user, int layer, char * out, size_t size);                       // the bank preset it plays, if it does
} tUaSampleHost;

void ua_draw_init(void);
void ua_draw_set_pages(const tUaPage * pages, uint32_t count);
void ua_draw_set_steps(uint32_t paramId, int32_t steps);     // from the parameter table
void ua_draw_set_value(uint32_t paramId, double normalized); // notes §14: a host id - layer x UA_LAYER_STRIDE + the parameter
int ua_draw_layer(void);                                     // the tab shown
void ua_draw_set_mouse(double x, double y);
void ua_draw_frame(int pixelWidth, int pixelHeight);
bool ua_draw_menu_active(void);

// A press: opens a list control's menu, takes a choice from an open one (edit->paramId is UINT32_MAX when
// nothing was set), or grabs a dial (*grabbed, and the pointer is then held while dragBy moves it).
bool ua_draw_press(double x, double y, tUaEdit * edit, bool * grabbed);
bool ua_draw_drag_by(double dx, double dy, tUaEdit * edit);
bool ua_draw_drag_to(double x, double y, tUaEdit * edit);   // Rotary: the pointer's own position
void ua_draw_release(double x, double y, tUaEdit * edit);   // a menu choice is taken here
bool ua_draw_key(int glfwKey, unsigned int codepoint, bool isRepeat);
void ua_draw_scroll(double dy);

void ua_draw_set_host(const tUaSampleHost * host);    // static storage
void ua_draw_set_user(void * user);

void ua_draw_open_sample(void);                       // the file browser
void ua_draw_step_sample(int direction);              // notes §2: the previous or next sample in its folder
void ua_draw_audition(bool stack);                    // the tab's layer alone, or the stack
void ua_draw_reset_controls(void);                    // after a confirm
bool ua_draw_oversampling(void);                      // notes §16: Controls > Oversample Saturation 2x
void ua_draw_set_oversampling(bool on);

#endif // __UA_DRAW_H__
