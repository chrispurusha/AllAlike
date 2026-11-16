/*
 * UltraAlike - the menu bar.
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
#include <string.h>

#include "synthlibDefs.h"
#include "synthlibGlobals.h"
#include "synthlibTypes.h"
#include "synthlibVersion.h"
#include "contextMenu.h"
#include "alertDialog.h"

#include "uaDraw.h"
#include "uaMenu.h"
#include "uaPrefs.h"

#define ITEM           (tRgb)RGB_GREY_3
#define END_OF_MENU    (tMenuItem){NULL, (tRgb)RGB_BLACK, NULL, 0, NULL, 0, 0.0, NULL}

// ---- File -------------------------------------------------------------------------------------------------

static void action_load(int index) {
    (void)index;
    ua_draw_open_sample();
}

static void action_step(int index) {
    ua_draw_step_sample((gContextMenu.items[index].param != 0) ? 1 : -1);
}

static void action_play(int index) {
    (void)index;
    ua_draw_audition(true);
}

static void action_reset(int index) {
    (void)index;
    ua_draw_reset_controls();
}

static void open_file_menu(tCoord anchor) {
    static tMenuItem items[6];

    items[0] = (tMenuItem){
        "Load Sample...", ITEM, action_load, 0, NULL, 0, 0.0, NULL
    };
    items[1] = (tMenuItem){
        "Previous in Folder", ITEM, action_step, 0, NULL, 0, 0.0, NULL
    };
    items[2] = (tMenuItem){
        "Next in Folder", ITEM, action_step, 1, NULL, 0, 0.0, NULL
    };
    items[3] = (tMenuItem){
        "Play", ITEM, action_play, 0, NULL, 0, 0.0, NULL
    };
    items[4] = (tMenuItem){
        "Reset Controls...", ITEM, action_reset, 0, NULL, 0, 0.0, NULL
    };
    items[5] = END_OF_MENU;
    open_context_menu(anchor, items, 0, 0.0);
}

// ---- Controls ---------------------------------------------------------------------------------------------

static void action_dial_mode(int index) {
    ua_set_dial_mode((tDialMode)gContextMenu.items[index].param);
}

static void action_audition(int index) {
    (void)index;
    ua_set_audition_on_browse(!ua_audition_on_browse());
}

// "* " marks the one in force, as the sibling editors' Controls menus do
static void open_controls_menu(tCoord anchor) {
    static tMenuItem items[6];
    tDialMode        mode = ua_dial_mode();

    items[0] = (tMenuItem){
        (mode == eDialModeRotary) ? "* Rotary" : "Rotary", ITEM, action_dial_mode, eDialModeRotary, NULL, 0, 0.0, NULL
    };
    items[1] = (tMenuItem){
        (mode == eDialModeVertical) ? "* Vertical" : "Vertical", ITEM, action_dial_mode, eDialModeVertical, NULL, 0, 0.0, NULL
    };
    items[2] = (tMenuItem){
        (mode == eDialModeHorizontal) ? "* Horizontal" : "Horizontal", ITEM, action_dial_mode, eDialModeHorizontal, NULL, 0, 0.0,
        NULL
    };
    items[3] = (tMenuItem){
        ua_audition_on_browse() ? "* Play on Load" : "Play on Load", ITEM, action_audition, 0, NULL, 0, 0.0, NULL
    };
    items[4] = END_OF_MENU;
    open_context_menu(anchor, items, 0, 0.0);
}

// ---- Help -------------------------------------------------------------------------------------------------

static void action_about(int index) {
    (void)index;
    SYNTHLIB_APP_INIT();
    show_alert("About", synthlib_about_text("UltraAlike"));
}

static void action_panel_help(int index) {
    (void)index;
    show_alert("The Panel",
               "One E-mu Emulator X bank preset, or one sample, per instance; Low and High limit the keys it plays. Play "
               "sounds it at its root key.\n\n"
               "Load... opens a bank (.exb, inside its folder beside SamplePool) or a WAV or AIFF file. With a bank, < Prev and "
               "Next > step through its presets; with a sample, through the other samples in its folder. Each plays as it loads "
               "(Controls > Play on Load).\n\n"
               "A bank preset plays every zone its keys and velocities reach, at each sample's own tuning, its loops held, through "
               "each voice's own amplifier envelope from the bank. Tune, Velocity, Pan, Volume and the key range apply to it too.\n\n"
               "For a single sample: Keys On plays it at the pitch of the note, from the Root key; Mode One Shot ignores the note "
               "off and Gate starts the Release at it; Voices Poly lets notes overlap, Mono cuts the last one off. The Envelope "
               "(AHR or AHDSR) shapes it, and any bank voice that has no envelope of its own.\n\n"
               "GLOBAL: Voices is how many notes sound at once; past it, Steal fades the quietest note whose key is up "
               "(Released) or the oldest (Oldest). Same Note Piano (the default) fades a key's last note when it is struck "
               "again, as a piano's damper does - the better choice with a sustain pedal, where repeated notes would otherwise "
               "pile up and use every voice. Stack lets each strike ring on, as the E5000 itself does.\n\n"
               "Dials: drag to turn, Shift-drag for fine steps; double-click one to put it back to its default.\n\n"
               "The project keeps the bank's or sample's path and the preset, not the samples: a moved file shows as Missing.\n\n"
               "For a diagnostic log: in Terminal, touch /tmp/ultraalike-log, then reload the plug-in; the log is "
               "/tmp/ultraalike.log.");
}

static void open_help_menu(tCoord anchor) {
    static tMenuItem items[3];

    items[0] = (tMenuItem){
        "The Panel...", ITEM, action_panel_help, 0, NULL, 0, 0.0, NULL
    };
    items[1] = (tMenuItem){
        "About UltraAlike...", ITEM, action_about, 0, NULL, 0, 0.0, NULL
    };
    items[2] = END_OF_MENU;
    open_context_menu(anchor, items, 0, 0.0);
}

static const tMenuBarItem gMenuBar[] = {
    {"File",     open_file_menu    },
    {"Controls", open_controls_menu},
    {"Help",     open_help_menu    },
    {NULL,       NULL              },
};

const tMenuBarItem * ua_menu_bar(void) {
    return gMenuBar;
}

tRectangle ua_menu_bar_rect(void) {
    return (tRectangle){{
                            0.0, 0.0
                        }, {
                            UA_CANVAS_W, UA_MENU_BAR_H
                        }
    };
}
