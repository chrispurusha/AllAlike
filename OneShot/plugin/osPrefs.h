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

#ifndef __OS_PREFS_H__
#define __OS_PREFS_H__

#include <stdbool.h>

#include "synthlibTypes.h"

void os_prefs_init(void);    // once per process

tDialMode os_dial_mode(void);
void os_set_dial_mode(tDialMode mode);

bool os_audition_on_browse(void);    // a sample loaded from the panel plays once
void os_set_audition_on_browse(bool on);

const char * os_last_folder(void);   // where the file browser opens with no sample loaded
void os_set_last_folder(const char * path);

#endif // __OS_PREFS_H__
