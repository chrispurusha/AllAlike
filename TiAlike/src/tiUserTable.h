/*
 * TiAlike - reading a user wavetable from a file.
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
// Notes: Docs/code-notes/tiUserTable.md

#ifndef __TI_USER_TABLE_H__
#define __TI_USER_TABLE_H__

#include <stdbool.h>

typedef struct {
    int     frames, cycle;
    float * samples;   // frames x cycle
    char    name[64];
} tTiUserCycles;

bool ti_user_table_file(const char * path);   // a kind of file it reads: .wav, .syx, .mid
bool ti_user_cycles_load(const char * path, tTiUserCycles * out);
void ti_user_cycles_free(tTiUserCycles * c);

#endif // __TI_USER_TABLE_H__
