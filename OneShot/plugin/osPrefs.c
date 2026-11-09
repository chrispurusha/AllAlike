/*
 * OneShot - preferences shared by every instance.
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
#include <stdlib.h>

#include "prefs.h"
#include "osPrefs.h"

#define PREF_DIAL_MODE      "dialMode"
#define PREF_AUDITION       "auditionOnBrowse"
#define PREF_LAST_FOLDER    "lastFolder"
#define DEFAULT_FOLDER      "/Users/Shared"

static tDialMode gDialMode = eDialModeVertical;
static bool      gAudition = true;
static char      gLastFolder[2048];

tDialMode os_dial_mode(void) {
    return gDialMode;
}

void os_set_dial_mode(tDialMode mode) {
    gDialMode = mode;
    prefs_set_int(PREF_DIAL_MODE, (long)mode);
}

bool os_audition_on_browse(void) {
    return gAudition;
}

void os_set_audition_on_browse(bool on) {
    gAudition = on;
    prefs_set_int(PREF_AUDITION, on ? 1 : 0);
}

const char * os_last_folder(void) {
    return gLastFolder;
}

void os_set_last_folder(const char * path) {
    if ((path != NULL) && (path[0] != '\0')) {
        snprintf(gLastFolder, sizeof(gLastFolder), "%s", path);
        prefs_set_string(PREF_LAST_FOLDER, gLastFolder);
    }
}

void os_prefs_init(void) {
    static bool  done   = false;

    if (done) {
        return;
    }
    done      = true;
    prefs_init("OneShot");
    long         mode   = prefs_get_int(PREF_DIAL_MODE, (long)eDialModeVertical);
    const char * folder = prefs_get_string(PREF_LAST_FOLDER, DEFAULT_FOLDER);

    if ((mode == (long)eDialModeRotary) || (mode == (long)eDialModeVertical) || (mode == (long)eDialModeHorizontal)) {
        gDialMode = (tDialMode)mode;
    }
    gAudition = prefs_get_int(PREF_AUDITION, 1) != 0;
    snprintf(gLastFolder, sizeof(gLastFolder), "%s", (folder != NULL) ? folder : DEFAULT_FOLDER);
}
