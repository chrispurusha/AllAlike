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

#include "tiDraw.h"
#include "tiMenu.h"
#include "tiPrefs.h"

#define ITEM           (tRgb)RGB_GREY_3
#define END_OF_MENU    (tMenuItem){NULL, (tRgb)RGB_BLACK, NULL, 0, NULL, 0, 0.0, NULL}

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
    show_alert("About", synthlib_about_text("TiAlike"));
}

static void action_panel_help(int index) {
    (void)index;
    show_alert("The Panel",
               "An early build: one oscillator, the filter and the amplifier, and the voice limit on the GLOBAL page.\n\n"
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

static const tMenuBarItem gMenuBar[] = {
    {"Controls", open_controls_menu},
    {"Help",     open_help_menu    },
    {NULL,       NULL              },
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
