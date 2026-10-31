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

#ifndef __TI_PREFS_H__
#define __TI_PREFS_H__

#define SN2_PREF_DIAL_MODE    "dialMode"

#include "synthlibTypes.h"

void sn2_prefs_init(void);    // once per process; restores the drag mode

// the plug-in's own, since SynthLib's plug-in stubs keep no dial mode
tDialMode sn2_dial_mode(void);
void sn2_set_dial_mode(tDialMode mode);    // and saves it

#endif // __TI_PREFS_H__
