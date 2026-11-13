/*
 * ModAlike - the panel's menu bar: Controls and Help.
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

#include <stdio.h>

#include "synthlibDefs.h"
#include "synthlibGlobals.h"
#include "synthlibTypes.h"
#include "synthlibVersion.h"
#include "contextMenu.h"
#include "alertDialog.h"

#include "maDraw.h"
#include "maMenu.h"
#include "maPrefs.h"

#define ITEM           (tRgb)RGB_GREY_3
#define END_OF_MENU    (tMenuItem){NULL, (tRgb)RGB_BLACK, NULL, 0, NULL, 0, 0.0, NULL}

// ---- Controls ---------------------------------------------------------------------------------------------

static void action_drag_rotary(int index) {
    (void)index;
    ma_set_dial_mode(eDialModeRotary);
}

static void action_drag_vertical(int index) {
    (void)index;
    ma_set_dial_mode(eDialModeVertical);
}

static void action_drag_horizontal(int index) {
    (void)index;
    ma_set_dial_mode(eDialModeHorizontal);
}

// "* " marks the one in force, as the sibling editors' Controls menus do
static void open_controls_menu(tCoord anchor) {
    static tMenuItem items[4];
    tDialMode        mode = ma_dial_mode();

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
    show_alert("About", synthlib_about_text("ModAlike"));
}

// Each fits the 320-point panel: SynthLib's alert is 420 wide and 76 + 16 a line tall, so 15 lines at most
static void action_panel_help(int index) {
    (void)index;
    show_alert("The Panel",
               "Effect chooses Phaser, Chorus or Flanger; the other two are dimmed and keep their settings.\n\n"
               "Stereo: Linked runs a module a side in step; Wide puts the right side's LFO behind the left by "
               "LFO Offset; Mid/Side processes only the mid. Chorus Input: Stereo keeps the source's image, Mono "
               "sums both sides into one.\n\n"
               "Mix blends the dry signal back, lined up. Dials drag; Controls sets how. Lists open on a click.");
}

static void action_engine_help(int index) {
    (void)index;
    show_alert("Engine",
               "Rate: 96 kHz, or Host rate - no latency, close to but not exactly the same.\n\n"
               "Maths: Float, or Fixed - every word on a 24-bit step. Stage: On rolls the top off as an output "
               "converter does (off at the host rate). Anti-alias: host rate only, a low-pass into the effect.\n\n"
               "A diagnostic log: touch /tmp/modalike-log, reload; read /tmp/modalike.log.");
}

static void open_help_menu(tCoord anchor) {
    static tMenuItem items[4];

    items[0] = (tMenuItem){"The Panel...", ITEM, action_panel_help, 0, NULL, 0, 0.0, NULL};
    items[1] = (tMenuItem){"Engine...", ITEM, action_engine_help, 0, NULL, 0, 0.0, NULL};
    items[2] = (tMenuItem){"About ModAlike...", ITEM, action_about, 0, NULL, 0, 0.0, NULL};
    items[3] = END_OF_MENU;
    open_context_menu(anchor, items, 0, 0.0);
}

static const tMenuBarItem gMenuBar[] = {
    {"Controls", open_controls_menu},
    {"Help",     open_help_menu    },
    {NULL,       NULL              },
};

const tMenuBarItem * ma_menu_bar(void) {
    return gMenuBar;
}

tRectangle ma_menu_bar_rect(void) {
    return (tRectangle){{
                            0.0, 0.0
                        }, {
                            MA_CANVAS_W, MA_MENU_BAR_H
                        }
    };
}
