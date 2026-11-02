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

#ifndef __TI_PREFS_H__
#define __TI_PREFS_H__

#define NL3_PREF_DIAL_MODE    "dialMode"
#define NL3_PREF_HW_PORT      "hardwarePort"

#include "synthlibTypes.h"

void nl3_prefs_init(void);    // once per process; restores the drag mode

// the plug-in's own, since SynthLib's plug-in stubs keep no dial mode
tDialMode nl3_dial_mode(void);
void nl3_set_dial_mode(tDialMode mode);    // and saves it

// The MIDI output the instrument hangs from, by name; "" none. Machine-wide, not per project.
const char * nl3_hw_port(void);
void nl3_set_hw_port(const char * name);

#endif // __TI_PREFS_H__
