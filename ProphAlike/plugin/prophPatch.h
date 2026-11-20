/*
 * ProphAlike - Singles: reading them out of .syx and .mid files, and writing one back.
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
// Notes: Docs/code-notes/prophPatch.md - "// notes §k" refers there.

#ifndef __TI_PATCH_H__
#define __TI_PATCH_H__

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "prophParams.h"

#define PROPH_SINGLE_LENGTH         (396)    // notes §1: a program's two SysEx messages, one after the other
#define PROPH_SINGLE_NAME_LENGTH    (16)
#define PROPH_MAX_SINGLES           (2048)   // in one file; a full backup holds 1424
#define PROPH_BANK_SIZE             (128)

// notes §2: every Single in a .syx or a .mid, in file order; the count, 0 if there are none
int proph_patch_read_file(const char * path, uint8_t(*singles)[PROPH_SINGLE_LENGTH], int capacity);

// notes §3: the parameters a Single carries, to normalized values; the others keep theirs
void proph_patch_from_single(const uint8_t single[PROPH_SINGLE_LENGTH], double normalized[kProphParamCount]);
void proph_patch_name(const uint8_t single[PROPH_SINGLE_LENGTH], char name[PROPH_SINGLE_NAME_LENGTH + 1]);

// notes §4: the parameters back over base (NULL: zeros), the name, the edit buffer's address and fresh checksums
void proph_patch_to_single(const double normalized[kProphParamCount], const uint8_t * base, const char * name, uint8_t out[PROPH_SINGLE_LENGTH]);
bool proph_patch_save_file(const char * path, const uint8_t single[PROPH_SINGLE_LENGTH]);

const char * proph_patch_sounds_folder(void);   // where Load... starts with no file loaded

// notes §5: the presets - A001..L128 in one file, read when it changes, main thread only
#define PROPH_PRESETS    (12 * PROPH_BANK_SIZE)

void proph_patch_init_single(uint8_t out[PROPH_SINGLE_LENGTH]);                                    // every carried parameter at its default, "Init"
void proph_patch_address(uint8_t single[PROPH_SINGLE_LENGTH], int number);                         // to preset number; 0 the edit buffer
void proph_preset_label(int number, char * out, size_t len);                                     // "A001"
const char * proph_presets_path(void);
bool proph_presets_refresh(void);                                                                // re-read if the file changed; true if so
const uint8_t * proph_presets_single(int number);                                                // 1..PROPH_PRESETS, as last refreshed
const char * proph_presets_name(int number);
bool proph_presets_store(int number, const uint8_t single[PROPH_SINGLE_LENGTH]);                   // false if not written
bool proph_presets_store_all(const uint8_t singles[PROPH_PRESETS][PROPH_SINGLE_LENGTH]);
int proph_presets_read_file(const char * path, uint8_t singles[PROPH_PRESETS][PROPH_SINGLE_LENGTH]); // notes §6
int proph_presets_read_folder(const char * folder, uint8_t singles[PROPH_PRESETS][PROPH_SINGLE_LENGTH]);
bool proph_presets_write_file(const char * path);

#endif // __TI_PATCH_H__
