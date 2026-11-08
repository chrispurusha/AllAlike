/*
 * Nl3Alike - Singles: reading them out of .syx and .mid files, and writing one back.
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
// Notes: Docs/code-notes/nl3Patch.md - "// notes §k" refers there.

#ifndef __NL3_PATCH_H__
#define __NL3_PATCH_H__

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "nl3Params.h"

#define NL3_SINGLE_LENGTH         (261)    // notes §1: a program dump, one SysEx message
#define NL3_SINGLE_NAME_LENGTH    (16)
#define NL3_MAX_SINGLES           (2048)   // in one file; the instrument holds 1024
#define NL3_BANK_SIZE             (128)

// notes §2: every Single in a .syx or a .mid, in file order; the count, 0 if there are none
int nl3_patch_read_file(const char * path, uint8_t(*singles)[NL3_SINGLE_LENGTH], int capacity);

// notes §3: the parameters a Single carries, to normalized values; the others keep theirs
void nl3_patch_from_single(const uint8_t single[NL3_SINGLE_LENGTH], double normalized[kNl3ParamCount]);
void nl3_patch_name(const uint8_t single[NL3_SINGLE_LENGTH], char name[NL3_SINGLE_NAME_LENGTH + 1]);

// notes §4: the parameters back over base (NULL: zeros), the name, the edit buffer's address and fresh checksums
void nl3_patch_to_single(const double normalized[kNl3ParamCount], const uint8_t * base, const char * name, uint8_t out[NL3_SINGLE_LENGTH]);
bool nl3_patch_save_file(const char * path, const uint8_t single[NL3_SINGLE_LENGTH]);

const char * nl3_patch_sounds_folder(void);   // where Load... starts with no file loaded

// notes §5: the presets - 1:001..8:128 in one file, read when it changes, main thread only
#define NL3_PRESETS    (8 * NL3_BANK_SIZE)

void nl3_patch_init_single(uint8_t out[NL3_SINGLE_LENGTH]);                                    // every carried parameter at its default, "Init"
void nl3_patch_address(uint8_t single[NL3_SINGLE_LENGTH], int number);                         // to preset number; 0 the edit buffer
void nl3_preset_label(int number, char * out, size_t len);                                     // "1:001"
const char * nl3_presets_path(void);
bool nl3_presets_refresh(void);                                                                // re-read if the file changed; true if so
const uint8_t * nl3_presets_single(int number);                                                // 1..NL3_PRESETS, as last refreshed
const char * nl3_presets_name(int number);
bool nl3_presets_store(int number, const uint8_t single[NL3_SINGLE_LENGTH]);                   // false if not written
bool nl3_presets_store_all(const uint8_t singles[NL3_PRESETS][NL3_SINGLE_LENGTH]);
int nl3_presets_read_file(const char * path, uint8_t singles[NL3_PRESETS][NL3_SINGLE_LENGTH]); // notes §6
int nl3_presets_read_folder(const char * folder, uint8_t singles[NL3_PRESETS][NL3_SINGLE_LENGTH]);
bool nl3_presets_write_file(const char * path);

#endif // __TI_PATCH_H__
