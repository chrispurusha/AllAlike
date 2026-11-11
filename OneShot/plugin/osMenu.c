/*
 * OneShot - the menu bar.
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

#include "osDraw.h"
#include "osMenu.h"
#include "osPrefs.h"

#define ITEM           (tRgb)RGB_GREY_3
#define END_OF_MENU    (tMenuItem){NULL, (tRgb)RGB_BLACK, NULL, 0, NULL, 0, 0.0, NULL}

// ---- File -------------------------------------------------------------------------------------------------

static void action_load(int index) {
    (void)index;
    os_draw_open_sample();
}

static void action_step(int index) {
    os_draw_step_sample((gContextMenu.items[index].param != 0) ? 1 : -1);
}

static void action_play(int index) {
    (void)index;
    os_draw_audition(true);
}

static void action_reset(int index) {
    (void)index;
    os_draw_reset_controls();
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
    os_set_dial_mode((tDialMode)gContextMenu.items[index].param);
}

static void action_oversample(int index) {
    (void)index;
    os_draw_set_oversampling(!os_draw_oversampling());
}

static void action_audition(int index) {
    (void)index;
    os_set_audition_on_browse(!os_audition_on_browse());
}

// "* " marks the one in force, as the sibling editors' Controls menus do
static void open_controls_menu(tCoord anchor) {
    static tMenuItem items[6];
    tDialMode        mode = os_dial_mode();

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
        os_audition_on_browse() ? "* Play on Load" : "Play on Load", ITEM, action_audition, 0, NULL, 0, 0.0, NULL
    };
    items[4] = (tMenuItem){
        os_draw_oversampling() ? "* Oversample Saturation 2x" : "Oversample Saturation 2x", ITEM, action_oversample, 0, NULL, 0, 0.0, NULL
    };
    items[5] = END_OF_MENU;
    open_context_menu(anchor, items, 0, 0.0);
}

// ---- Help -------------------------------------------------------------------------------------------------

static void action_about(int index) {
    (void)index;
    SYNTHLIB_APP_INIT();
    show_alert("About", synthlib_about_text("OneShot"));
}

static void action_panel_help(int index) {
    (void)index;
    show_alert("The Panel",
               "Up to four samples, one a layer on the tabs, held in memory. Each layer has its own controls and a key range "
               "(Low to High); a note plays every layer it falls in, all in step; Mute under a tab silences that layer, Clear empties it. Play sounds the stack at the shown layer's root "
               "key; a click on the waveform plays that layer alone.\n\n"
               "Load... opens a WAV or AIFF file. < Prev and Next > step through the other samples in its folder, "
               "as do the arrow keys; each one plays as it loads (Controls > Play on Load). Play, a click on the "
               "waveform or Space plays it again.\n\n"
               "Mode: One Shot ignores the note off; Gate starts the Release at it. Voices: Poly lets hits "
               "overlap, Mono cuts the last one off. Keys On plays it at the pitch of the note, from the Root key.\n\n"
               "Trim: Delay holds a note back before it sounds. Start and End (the yellow lines) are where in the sample it "
               "begins and stops, in the sample's own time; Snap moves both to the nearest zero crossing. The waveform shows "
               "Start to End. Reverse plays it backwards, Start and End counting from its end.\n\n"
               "Stretch: the sample's length times x0.25 to x4, its pitch kept. Sync On stretches it from its own tempo (BPM) "
               "to the host's as well. Protect keeps the start as it is (the purple line) - the rest takes up the change - and its Snap moves its end to a zero crossing. "
               "With either on, Tune and Keys change the pitch but not the length, and the envelope's times stretch too. "
               "After a tempo change the first note still plays at the old tempo while the new one is prepared.\n\n"
               "Envelope: AHR is Attack, Hold, then Release to silence; Hold at its top is Inf, the whole sample. ADSR adds Decay "
               "to Sustain; its Release needs Gate mode. In Gate mode a note off always starts the Release. The curves bend each "
               "stage: + drops fast then tails off, - the reverse. The green line on the waveform is the envelope.\n\n"
               "Filter: Low Cut and High Cut; Slope Soft (a gentle 12 dB Low Cut, a 6 dB High Cut), 12 or 24 dB an octave. Saturation: Classic (soft), Drums (hard) or Tape (Warmth "
               "shapes the lows, HF the highs), with Gain in and Output after. Both are per layer. Controls > Oversample Saturation 2x is cleaner, for 0.5 ms of latency the host makes up.\n\n"
               "Dials: drag to turn, Shift-drag for fine steps (Attack has 1000, a fifth of them in its first 5 ms); "
               "double-click one to put it back to its default.\n\n"
               "The project keeps the sample's path, not the sample: a moved file shows as Missing.\n\n"
               "For a diagnostic log: in Terminal, touch /tmp/oneshot-log, then reload the plug-in; the log is "
               "/tmp/oneshot.log.");
}

static void open_help_menu(tCoord anchor) {
    static tMenuItem items[3];

    items[0] = (tMenuItem){
        "The Panel...", ITEM, action_panel_help, 0, NULL, 0, 0.0, NULL
    };
    items[1] = (tMenuItem){
        "About OneShot...", ITEM, action_about, 0, NULL, 0, 0.0, NULL
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

const tMenuBarItem * os_menu_bar(void) {
    return gMenuBar;
}

tRectangle os_menu_bar_rect(void) {
    return (tRectangle){{
                            0.0, 0.0
                        }, {
                            OS_CANVAS_W, OS_MENU_BAR_H
                        }
    };
}
