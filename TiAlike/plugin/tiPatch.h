/*
 * TiAlike - Singles: reading them out of .syx and .mid files, and writing one back.
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
// Notes: Docs/code-notes/tiPatch.md - "// notes §k" refers there.

#ifndef __TI_PATCH_H__
#define __TI_PATCH_H__

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "tiParams.h"
#include "tiVoices.h"

#define TI_SINGLE_LENGTH         (524)    // notes §1: the whole SysEx message, F0 to F7
#define TI_SINGLE_NAME_LENGTH    (10)
#define TI_MAX_SINGLES           (4096)   // in one file; a sound set holds a few hundred
#define TI_BANK_SIZE             (128)

// notes §2: every Single in a .syx or a .mid, in file order; the count, 0 if there are none
int ti_patch_read_file(const char * path, uint8_t(*singles)[TI_SINGLE_LENGTH], int capacity);

// notes §3: the parameters a Single carries, to normalized values; the others keep theirs
void ti_patch_from_single(const uint8_t single[TI_SINGLE_LENGTH], double normalized[kTiParamCount]);
void ti_patch_name(const uint8_t single[TI_SINGLE_LENGTH], char name[TI_SINGLE_NAME_LENGTH + 1]);

// notes §4: the parameters back over base (NULL: zeros), the name, the edit buffer's address and fresh checksums
void ti_patch_to_single(const double normalized[kTiParamCount], const uint8_t * base, const char * name, uint8_t out[TI_SINGLE_LENGTH]);
// notes §7: the pages of a Single, as the voices read them
void ti_patch_sound(const uint8_t single[TI_SINGLE_LENGTH], tTiSound * sound);
bool ti_patch_save_file(const char * path, const uint8_t single[TI_SINGLE_LENGTH]);

const char * ti_patch_sounds_folder(void);   // where Load... starts with no file loaded

// notes §5: the presets - A001..W128 in one file, read when it changes, main thread only
#define TI_PRESETS    (23 * TI_BANK_SIZE)

void ti_patch_init_single(uint8_t out[TI_SINGLE_LENGTH]);                                   // every carried parameter at its default, "Init"
void ti_patch_address(uint8_t single[TI_SINGLE_LENGTH], int number);                        // to preset number; 0 the edit buffer
void ti_preset_label(int number, char * out, size_t len);                                   // "A001"
const char * ti_presets_path(void);
bool ti_presets_refresh(void);                                                              // re-read if the file changed; true if so
const uint8_t * ti_presets_single(int number);                                              // 1..TI_PRESETS, as last refreshed
const char * ti_presets_name(int number);
bool ti_presets_store(int number, const uint8_t single[TI_SINGLE_LENGTH]);                  // false if not written
bool ti_presets_store_all(const uint8_t singles[TI_PRESETS][TI_SINGLE_LENGTH]);
int ti_presets_read_file(const char * path, uint8_t singles[TI_PRESETS][TI_SINGLE_LENGTH]); // notes §6
int ti_presets_read_folder(const char * folder, uint8_t singles[TI_PRESETS][TI_SINGLE_LENGTH]);
bool ti_presets_write_file(const char * path);

#endif // __TI_PATCH_H__
