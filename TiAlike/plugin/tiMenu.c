/*
 * TiAlike - the menu bar.
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
// Notes: Docs/code-notes/tiMenu.md - "// notes §k" refers there.

#include <stdio.h>
#include <string.h>

#include "synthlibDefs.h"
#include "synthlibGlobals.h"
#include "synthlibTypes.h"
#include "synthlibVersion.h"
#include "contextMenu.h"
#include "alertDialog.h"
#include "bankBrowser.h"

#include "tiDraw.h"
#include "tiHardware.h"
#include "tiPrefs.h"
#include "tiMenu.h"
#include "tiPatch.h"
#include "tiPrefs.h"

#define ITEM             (tRgb)RGB_GREY_3
#define ITEM_DISABLED    (tRgb)RGB_GREY_5
#define END_OF_MENU      (tMenuItem){NULL, (tRgb)RGB_BLACK, NULL, 0, NULL, 0, 0.0, NULL}

// ---- File -------------------------------------------------------------------------------------------------

static void action_open(int index) {
    (void)index;
    ti_draw_open_patch();
}

static void action_save(int index) {
    (void)index;
    ti_draw_save_patch(false);
}

static void action_save_as(int index) {
    (void)index;
    ti_draw_save_patch(true);
}

static void action_init(int index) {
    (void)index;
    ti_draw_init_patch();
}

static void open_file_menu(tCoord anchor) {
    static tMenuItem items[5];
    bool             haveFile = ti_draw_has_patch_file();

    items[0] = (tMenuItem){
        "Open Patch...", ITEM, action_open, 0, NULL, 0, 0.0, NULL
    };
    items[1] = (tMenuItem){
        "Save Patch", haveFile ? ITEM : ITEM_DISABLED, haveFile ? action_save : NULL, 0, NULL, 0, 0.0, NULL
    };
    items[2] = (tMenuItem){
        "Save Patch As...", ITEM, action_save_as, 0, NULL, 0, 0.0, NULL
    };
    items[3] = (tMenuItem){
        "Init Patch...", ITEM, action_init, 0, NULL, 0, 0.0, NULL
    };
    items[4] = END_OF_MENU;
    open_context_menu(anchor, items, 0, 0.0);
}

// ---- Presets - notes §1 -----------------------------------------------------------------------------------

static void action_load_preset(int index) {
    (void)index;
    ti_draw_load_preset_browser();
}

static void action_store(int index) {
    (void)index;
    ti_draw_store_preset(0);
}

static void action_store_to(int index) {
    (void)index;
    ti_draw_store_preset_browser();
}

static void action_revert(int index) {
    (void)index;
    ti_draw_revert_preset();
}

static void action_import(int index) {
    ti_draw_import_presets(gContextMenu.items[index].param != 0);
}

static void action_export(int index) {
    (void)index;
    ti_draw_export_presets();
}

static void open_presets_menu(tCoord anchor) {
    static tMenuItem items[8];
    static char      storeLabel[32], revertLabel[40];
    int              preset   = ti_draw_preset();
    char             label[8] = "";

    if (preset > 0) {
        ti_preset_label(preset, label, sizeof(label));
    }
    snprintf(storeLabel, sizeof(storeLabel), (preset > 0) ? "Store Preset to %s" : "Store Preset", label);
    snprintf(revertLabel, sizeof(revertLabel), (preset > 0) ? "Revert to Stored %s" : "Revert to Stored Preset", label);
    items[0] = (tMenuItem){
        "Load Preset...", ITEM, action_load_preset, 0, NULL, 0, 0.0, NULL
    };
    items[1] = (tMenuItem){
        storeLabel, (preset > 0) ? ITEM : ITEM_DISABLED, (preset > 0) ? action_store : NULL, 0, NULL, 0, 0.0, NULL
    };
    items[2] = (tMenuItem){
        "Store Preset to...", ITEM, action_store_to, 0, NULL, 0, 0.0, NULL
    };
    items[3] = (tMenuItem){
        revertLabel, (preset > 0) ? ITEM : ITEM_DISABLED, (preset > 0) ? action_revert : NULL, 0, NULL, 0, 0.0, NULL
    };
    items[4] = (tMenuItem){
        "Import Presets from File...", ITEM, action_import, 0, NULL, 0, 0.0, NULL
    };
    items[5] = (tMenuItem){
        "Import Presets from Folder...", ITEM, action_import, 1, NULL, 0, 0.0, NULL
    };
    items[6] = (tMenuItem){
        "Export Presets to File...", ITEM, action_export, 0, NULL, 0, 0.0, NULL
    };
    items[7] = END_OF_MENU;
    open_context_menu(anchor, items, 0, 0.0);
}

// ---- Sort - notes §3: the order the preset browser lists in and Prev/Next step through -------------------------

static void action_sort(int index) {
    bank_browser_set_sort_mode((int32_t)gContextMenu.items[index].param);
}

static void open_sort_menu(tCoord anchor) {
    static tMenuItem         items[4];
    static const char *const kLabels[3][2] = {
        {"Bank / Location", "* Bank / Location"}, {"Category", "* Category"}, {"A-Z", "* A-Z"}
    };
    int32_t                  mode          = bank_browser_sort_mode();

    for (int i = 0; i < 3; i++) {
        items[i] = (tMenuItem){
            kLabels[i][mode == i], ITEM, action_sort, (uint32_t)i, NULL, 0, 0.0, NULL
        };
    }

    items[3] = END_OF_MENU;
    open_context_menu(anchor, items, 0, 0.0);
}

// ---- Wavetable - notes §4: a user wavetable for each oscillator; Prev/Next are beside its Wave on the panel ---

static void action_table_load(int index) {
    ti_draw_load_table((int)gContextMenu.items[index].param);
}

static void open_table_menu(tCoord anchor) {
    static tMenuItem items[3];

    items[0] = (tMenuItem){
        "Osc 1 Load Wavetable...", ITEM, action_table_load, 0, NULL, 0, 0.0, NULL
    };
    items[1] = (tMenuItem){
        "Osc 2 Load Wavetable...", ITEM, action_table_load, 1, NULL, 0, 0.0, NULL
    };
    items[2] = END_OF_MENU;
    open_context_menu(anchor, items, 0, 0.0);
}

// ---- Virus TI - notes §2 ----------------------------------------------------------------------------------

static void action_read_hardware(int index) {
    ti_draw_read_hardware(gContextMenu.items[index].param != 0);
}

static void action_write_hardware(int index) {
    ti_draw_write_hardware((int)gContextMenu.items[index].param);
}

static void open_hardware_menu(tCoord anchor) {
    static tMenuItem items[4 + TI_HW_RAM_BANKS];
    static char      labels[TI_HW_RAM_BANKS][48];
    int              n = 0;

    items[n++] = (tMenuItem){
        "Read Banks A-D into Presets...", ITEM, action_read_hardware, 0, NULL, 0, 0.0, NULL
    };
    items[n++] = (tMenuItem){
        "Read All Banks A-W into Presets...", ITEM, action_read_hardware, 1, NULL, 0, 0.0, NULL
    };

    for (int b = 1; b <= TI_HW_RAM_BANKS; b++) {
        snprintf(labels[b - 1], sizeof(labels[0]), "Write Presets %c to Bank %c...", 'A' + b - 1, 'A' + b - 1);
        items[n++] = (tMenuItem){
            labels[b - 1], ITEM, action_write_hardware, (uint32_t)b, NULL, 0, 0.0, NULL
        };
    }

    items[n]   = END_OF_MENU;
    open_context_menu(anchor, items, 0, 0.0);
}

// ---- Controls ---------------------------------------------------------------------------------------------

static void action_drag_rotary(int index) {
    (void)index;
    ti_set_dial_mode(eDialModeRotary);
}

static void action_drag_vertical(int index) {
    (void)index;
    ti_set_dial_mode(eDialModeVertical);
}

static void action_drag_horizontal(int index) {
    (void)index;
    ti_set_dial_mode(eDialModeHorizontal);
}

// "* " marks the one in force, as the sibling editors' Controls menus do
static void open_controls_menu(tCoord anchor) {
    static tMenuItem items[4];
    tDialMode        mode = ti_dial_mode();

    items[0] = (tMenuItem){
        (mode == eDialModeRotary) ? "* Rotary" : "Rotary", ITEM, action_drag_rotary, 0, NULL, 0, 0.0, NULL
    };
    items[1] = (tMenuItem){
        (mode == eDialModeVertical) ? "* Vertical" : "Vertical", ITEM, action_drag_vertical, 0, NULL, 0, 0.0, NULL
    };
    items[2] = (tMenuItem){
        (mode == eDialModeHorizontal) ? "* Horizontal" : "Horizontal", ITEM, action_drag_horizontal, 0, NULL, 0, 0.0, NULL
    };
    items[3] = END_OF_MENU;
    open_context_menu(anchor, items, 0, 0.0);
}

// ---- Help -------------------------------------------------------------------------------------------------

static void action_about(int index) {
    (void)index;
    SYNTHLIB_APP_INIT();
    show_alert("About", synthlib_about_text("TiAlike"));
}

static void action_panel_help(int index) {
    (void)index;
    show_alert("The Panel",
               "An early build: one oscillator, the two filters, the amplifier envelope (ENV) and the voice limit on the "
               "GLOBAL page.\n\n"
               "Patches: < Prev and Next > step through the presets, A001-W128, in the order the Sort menu sets; Load... opens a .syx or .mid file of "
               "Singles, choosing one when it holds several. The name beside them edits on a click. A Single keeps "
               "every byte it came with, though only what is built so far plays.\n\n"
               "Dials: drag to turn, Shift to turn finely. Controls chooses how a drag turns them: Rotary follows the "
               "pointer round the dial, Vertical and Horizontal count its movement.\n\n"
               "Lists (the oscillator's shape, the filter's mode) open a menu of their values on a click.\n\n"
               "For a diagnostic log: in Terminal, touch /tmp/tialike-log, then reload the plug-in; the log is "
               "/tmp/tialike.log.");
}

static void open_help_menu(tCoord anchor) {
    static tMenuItem items[3];

    items[0] = (tMenuItem){
        "The Panel...", ITEM, action_panel_help, 0, NULL, 0, 0.0, NULL
    };
    items[1] = (tMenuItem){
        "About TiAlike...", ITEM, action_about, 0, NULL, 0, 0.0, NULL
    };
    items[2] = END_OF_MENU;
    open_context_menu(anchor, items, 0, 0.0);
}

// ---- MIDI: where Link sends (tiHardware notes §1) ------------------------------------------------------------

static char gPortNames[TI_HARDWARE_MAX_PORTS][256];
static char gPortLabels[TI_HARDWARE_MAX_PORTS][260];
static char gDefaultLabel[64];

static void action_port(int index) {
    int i = (int)gContextMenu.items[index].param;

    ti_set_hw_port((i < 0) ? "" : gPortNames[i]);
}

static void open_midi_menu(tCoord anchor) {
    static tMenuItem items[TI_HARDWARE_MAX_PORTS + 3];
    int              count = 0, ports = 0;

    snprintf(gDefaultLabel, sizeof(gDefaultLabel), "%sVirus TI (its own USB port)", (ti_hw_port()[0] == '\0') ? "* " : "");
    items[count++] = (tMenuItem){
        gDefaultLabel, ITEM, action_port, (uint32_t)-1, NULL, 0, 0.0, NULL
    };

    for (int i = 0; i < ti_hardware_port_count(); i++) {
        if (ti_hardware_port_name(i, gPortNames[ports], sizeof(gPortNames[ports]))) {
            snprintf(gPortLabels[ports], sizeof(gPortLabels[ports]), "%s%s", (strcmp(gPortNames[ports], ti_hw_port()) == 0) ? "* " : "", gPortNames[ports]);
            items[count++] = (tMenuItem){
                gPortLabels[ports], ITEM, action_port, (uint32_t)ports, NULL, 0, 0.0, NULL
            };
            ports++;
        }
    }

    items[count]   = END_OF_MENU;
    open_context_menu(anchor, items, 0, 0.0);
}

static const tMenuBarItem gMenuBar[] = {
    {"File",      open_file_menu    },
    {"Presets",   open_presets_menu },
    {"Sort",      open_sort_menu    },
    {"Wavetable", open_table_menu   },
    {"Virus TI",  open_hardware_menu},
    {"Controls",  open_controls_menu},
    {"MIDI",      open_midi_menu    },
    {"Help",      open_help_menu    },
    {NULL,        NULL              },
};

const tMenuBarItem * ti_menu_bar(void) {
    return gMenuBar;
}

tRectangle ti_menu_bar_rect(void) {
    return (tRectangle){{
                            0.0, 0.0
                        }, {
                            TI_CANVAS_W, TI_MENU_BAR_H
                        }
    };
}
