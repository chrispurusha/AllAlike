/*
 * ModAlike - its preferences.
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

#include <stdbool.h>

#include "prefs.h"
#include "maPrefs.h"

static tDialMode gDialMode = eDialModeVertical;

tDialMode ma_dial_mode(void) {
    return gDialMode;
}

void ma_set_dial_mode(tDialMode mode) {
    gDialMode = mode;
    prefs_set_int(MA_PREF_DIAL_MODE, (long)mode);
}

void ma_prefs_init(void) {
    static bool done = false;

    if (done) {
        return;
    }
    done = true;
    prefs_init("ModAlike");
    long        mode = prefs_get_int(MA_PREF_DIAL_MODE, (long)eDialModeVertical);

    if ((mode == (long)eDialModeRotary) || (mode == (long)eDialModeVertical) || (mode == (long)eDialModeHorizontal)) {
        gDialMode = (tDialMode)mode;
    }
}
