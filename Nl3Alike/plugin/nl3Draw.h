/*
 * Nl3Alike - the editor panel.
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
// Notes: Docs/code-notes/nl3Draw.md - "// notes §k" refers there.

#ifndef __TI_DRAW_H__
#define __TI_DRAW_H__

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define NL3_CANVAS_W    (780.0)
#define NL3_CANVAS_H    (560.0)   // four rows of controls under the tabs

typedef enum {
    eTiControlDial = 0,
    eTiControlChoice        // a button that opens a list of its values
} tNl3ControlKind;

typedef struct {
    uint32_t            paramId;
    const char *        label;
    tNl3ControlKind     kind;
    int32_t             steps;   // plain range is 0..steps for a dial, entries - 1 for a choice
    double              plainMin;
    const char *const * names;   // a choice's entries
} tNl3Control;

typedef struct {
    const char *        title;
    double              colour[3];
    const tNl3Control * controls;
    uint32_t            count;
} tNl3Section;

typedef struct {
    const char *        title;
    const tNl3Section * sections;
    uint32_t            count;
} tNl3Page;

// What a click or drag asked for: one parameter, its new normalized value.
typedef struct {
    uint32_t paramId;
    double   normalized;
} tNl3Edit;

void nl3_draw_init(void);
void nl3_draw_set_pages(const tNl3Page * pages, uint32_t count);
void nl3_draw_set_value(uint32_t paramId, double normalized);

// notes §8: a plug-in may show a dial at another position than its own value, and with its own text (a linked
// control following the one it is linked to). values[] are every parameter as the panel holds them, normalized.
// Returns false to draw the control as usual.
typedef bool (*tNl3DialDisplay)(uint32_t paramId, const double * values, int32_t * step, char * text, size_t len);
void nl3_draw_set_display(tNl3DialDisplay display);
void nl3_draw_set_clip(bool on);
void nl3_draw_read_hardware(void);     // Presets > Read from Hardware
void nl3_draw_read_tick(void);         // from the panel's sync
void nl3_draw_set_mouse(double x, double y);
void nl3_draw_frame(int pixelWidth, int pixelHeight);
bool nl3_draw_menu_active(void);

// A press: opens a list control's menu, takes a choice from an open one (edit->paramId is UINT32_MAX when
// nothing was set), or grabs a dial (*grabbed, and the pointer is then held while dragBy moves it).
bool nl3_draw_press(double x, double y, tNl3Edit * edit, bool * grabbed);
bool nl3_draw_drag_by(double dx, double dy, tNl3Edit * edit);
bool nl3_draw_drag_to(double x, double y, tNl3Edit * edit);   // Rotary: the pointer's own position
void nl3_draw_release(double x, double y, tNl3Edit * edit);   // a menu choice is taken here
bool nl3_draw_key(int glfwKey, unsigned int codepoint, bool isRepeat);
void nl3_draw_scroll(double dy);

// notes §9: Singles and presets, and what the menus do with them. The host does the work on the sound.
typedef struct {
    // a Single chosen: from a file (path, its index there) or a preset (path "", preset 1..NL3_PRESETS)
    void (*load)(void * user, const uint8_t * single, const char * path, int index, int preset);
    bool (*save)(void * user, const char * path);              // the sound as it is now; false if not written
    bool (*storePreset)(void * user, int number);              // the same, into a preset
    void (*init)(void * user);                                 // every sound parameter to its default
    const char * (*name)(void * user);
    void (*rename)(void * user, const char * name);            // notes §10: an edited name, on commit
} tNl3PatchHost;

void nl3_draw_set_host(const tNl3PatchHost * host);    // static storage
void nl3_draw_set_user(void * user);
void nl3_draw_set_patch(const char * path, int index); // this instance's file and its place in it; "" none
void nl3_draw_set_preset(int number, bool edited);     // shown as 1:12, 1:12* once edited; 0 none

void nl3_draw_open_patch(void);
bool nl3_draw_has_patch_file(void);                  // a file of one Single, which Save may overwrite
void nl3_draw_save_patch(bool chooseFile);           // false: over the current file, after a confirm
void nl3_draw_init_patch(void);
int nl3_draw_preset(void);
void nl3_draw_load_preset_browser(void);             // notes §11: SynthLib's bank browser, A-D of 128
void nl3_draw_store_preset_browser(void);
void nl3_draw_store_preset(int number);              // 0: back to the preset it came from; confirmed first
void nl3_draw_revert_preset(void);
void nl3_draw_import_presets(bool folder);           // notes §12: all 512, after a confirm
void nl3_draw_export_presets(void);

#endif // __TI_DRAW_H__
