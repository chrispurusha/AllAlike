/*
 * Sn2Alike - the plug-in\'s own preferences.
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
// Notes: Docs/code-notes/sn2Prefs.md - "// notes §k" refers there.

#include <stdbool.h>
#include <stdio.h>

#include "prefs.h"
#include "sn2Prefs.h"

static tDialMode gDialMode = eDialModeVertical;
static char      gHwPort[256];

tDialMode sn2_dial_mode(void) {
    return gDialMode;
}

void sn2_set_dial_mode(tDialMode mode) {
    gDialMode = mode;
    prefs_set_int(SN2_PREF_DIAL_MODE, (long)mode);
}

const char * sn2_hw_port(void) {
    return gHwPort;
}

void sn2_set_hw_port(const char * name) {
    snprintf(gHwPort, sizeof(gHwPort), "%s", name);
    prefs_set_string(SN2_PREF_HW_PORT, gHwPort);
}

void sn2_prefs_init(void) {
    static bool done = false;

    if (done) {
        return;
    }
    done = true;
    prefs_init("Sn2Alike");
    snprintf(gHwPort, sizeof(gHwPort), "%s", prefs_get_string(SN2_PREF_HW_PORT, ""));
    long        mode = prefs_get_int(SN2_PREF_DIAL_MODE, (long)eDialModeVertical);

    if ((mode == (long)eDialModeRotary) || (mode == (long)eDialModeVertical) || (mode == (long)eDialModeHorizontal)) {
        gDialMode = (tDialMode)mode;
    }
}
