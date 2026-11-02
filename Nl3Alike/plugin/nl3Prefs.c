/*
 * Nl3Alike - the plug-in\'s own preferences.
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
// Notes: Docs/code-notes/nl3Prefs.md - "// notes §k" refers there.

#include <stdbool.h>
#include <stdio.h>

#include "prefs.h"
#include "nl3Prefs.h"

static tDialMode gDialMode = eDialModeVertical;
static char      gHwPort[256];

tDialMode nl3_dial_mode(void) {
    return gDialMode;
}

void nl3_set_dial_mode(tDialMode mode) {
    gDialMode = mode;
    prefs_set_int(NL3_PREF_DIAL_MODE, (long)mode);
}

const char * nl3_hw_port(void) {
    return gHwPort;
}

void nl3_set_hw_port(const char * name) {
    snprintf(gHwPort, sizeof(gHwPort), "%s", name);
    prefs_set_string(NL3_PREF_HW_PORT, gHwPort);
}

void nl3_prefs_init(void) {
    static bool done = false;

    if (done) {
        return;
    }
    done = true;
    prefs_init("Nl3Alike");
    snprintf(gHwPort, sizeof(gHwPort), "%s", prefs_get_string(NL3_PREF_HW_PORT, ""));
    long        mode = prefs_get_int(NL3_PREF_DIAL_MODE, (long)eDialModeVertical);

    if ((mode == (long)eDialModeRotary) || (mode == (long)eDialModeVertical) || (mode == (long)eDialModeHorizontal)) {
        gDialMode = (tDialMode)mode;
    }
}
