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

#include <dirent.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "tiPatch.h"

#include <sys/stat.h>

#define SOUNDS_FOLDER          "Library/Mobile Documents/com~apple~CloudDocs/Patches/VirusPatches" // under $HOME
#define PRESETS_FOLDER         "Library/Application Support/TiAlike"                               // beside prefs.txt
#define PRESETS_FILE           "Presets.syx"

// notes §1: where things sit in the message
#define DEVICE_AT              (5)
#define COMMAND_AT             (6)
#define BANK_AT                (7)
#define PROGRAM_AT             (8)
#define DATA_AT                (9)
#define CHECKSUM1_AT           (DATA_AT + 256)
#define CHECKSUM2_AT           (CHECKSUM1_AT + 1 + 256)
#define OLDER_SINGLE_LENGTH    (CHECKSUM1_AT + 2)
#define PAGE_A                 (DATA_AT)
#define PAGE_B                 (DATA_AT + 128)
#define NAME_AT                (PAGE_B + 112)

#define SINGLE_DUMP            (0x10)
#define OMNI_DEVICE            (0x10)
#define EDIT_BUFFER_BANK       (0x00)
#define EDIT_BUFFER_PROG       (0x40)

#define MAX_FILE_BYTES         (16 * 1024 * 1024)
#define MAX_FOLDER_FILES       (4096)

static const uint8_t kHeader[5] = {0xF0, 0x00, 0x20, 0x33, 0x01};

// notes §3: the bits of a byte a parameter takes; the rest of the byte is kept when it is written back
static const struct {
    uint32_t paramId;
    uint16_t at;
    uint8_t  mask;
}                    kCarried[] = {
    {kTiParamCutoff,      PAGE_A + 40, 0x7F},
    {kTiParamCutoff2,     PAGE_A + 41, 0x7F},
    {kTiParamResonance,   PAGE_A + 42, 0x7F},
    {kTiParamResonance2,  PAGE_A + 43, 0x7F},
    {kTiParamBalance,     PAGE_A + 48, 0x7F},
    {kTiParamFilterMode,  PAGE_A + 51, 0x7F},
    {kTiParamFilter2Mode, PAGE_A + 52, 0x7F},
    {kTiParamRouting,     PAGE_A + 53, 0x03},
    {kTiParamFilterLink,  PAGE_B + 32, 0x7F},
};

static bool is_single(const uint8_t * m, size_t len) {
    return (len == TI_SINGLE_LENGTH) && (memcmp(m, kHeader, sizeof(kHeader)) == 0) && (m[COMMAND_AT] == SINGLE_DUMP)
           && (m[TI_SINGLE_LENGTH - 1] == 0xF7);
}

// notes §1: an older Single - one 256-byte block and its checksum
static bool is_older_single(const uint8_t * m, size_t len) {
    return (len == OLDER_SINGLE_LENGTH) && (memcmp(m, kHeader, sizeof(kHeader)) == 0) && (m[COMMAND_AT] == SINGLE_DUMP)
           && (m[OLDER_SINGLE_LENGTH - 1] == 0xF7);
}

static uint8_t checksum(const uint8_t * m, int from, int to);

static int add_if_single(const uint8_t * m, size_t len, uint8_t(*singles)[TI_SINGLE_LENGTH], int count, int capacity) {
    if ((count < capacity) && is_single(m, len)) {
        memcpy(singles[count++], m, TI_SINGLE_LENGTH);
    } else if ((count < capacity) && is_older_single(m, len)) {
        uint8_t * out = singles[count++];

        memset(out, 0, TI_SINGLE_LENGTH);
        memcpy(out, m, CHECKSUM1_AT + 1);
        out[CHECKSUM2_AT]         = checksum(out, DEVICE_AT, CHECKSUM2_AT);
        out[TI_SINGLE_LENGTH - 1] = 0xF7;
    }
    return count;
}

static int from_syx(const uint8_t * d, size_t n, uint8_t(*singles)[TI_SINGLE_LENGTH], int capacity) {
    int count = 0;

    for (size_t i = 0; i < n; i++) {
        if (d[i] != 0xF0) {
            continue;
        }
        size_t end = i + 1;

        while ((end < n) && (d[end] != 0xF7)) {
            end++;
        }

        if (end >= n) {
            break;
        }
        count = add_if_single(&d[i], end + 1 - i, singles, count, capacity);
        i     = end;
    }

    return count;
}

static bool vlq(const uint8_t * d, size_t n, size_t * i, uint32_t * value) {
    *value = 0;

    for (int k = 0; (k < 4) && (*i < n); k++) {
        uint8_t b = d[(*i)++];

        *value = (*value << 7) | (b & 0x7Fu);

        if ((b & 0x80u) == 0) {
            return true;
        }
    }

    return false;
}

// notes §2: every SysEx event of every track; an F7 event continues the message before it
static int from_mid(const uint8_t * d, size_t n, uint8_t(*singles)[TI_SINGLE_LENGTH], int capacity) {
    static uint8_t pending[TI_SINGLE_LENGTH + 1];
    int            count = 0;
    size_t         i     = 14;

    while ((i + 8 <= n) && (count < capacity)) {
        uint32_t length = ((uint32_t)d[i + 4] << 24) | ((uint32_t)d[i + 5] << 16) | ((uint32_t)d[i + 6] << 8) | d[i + 7];
        size_t   j      = i + 8;
        size_t   end    = (j + length <= n) ? (j + length) : n;
        size_t   have   = 0;
        bool     open   = false;
        uint8_t  status = 0;

        if (memcmp(&d[i], "MTrk", 4) != 0) {
            i = end;
            continue;
        }

        while (j < end) {
            uint32_t delta, size;

            if (!vlq(d, end, &j, &delta) || (j >= end)) {
                break;
            }
            uint8_t  b = d[j];

            if (b == 0xFF) {
                j += 2;

                if (!vlq(d, end, &j, &size)) {
                    break;
                }
                j += size;
            } else if ((b == 0xF0) || (b == 0xF7)) {
                j++;

                if (!vlq(d, end, &j, &size) || (j + size > end)) {
                    break;
                }

                if (b == 0xF0) {
                    pending[0] = 0xF0;
                    have       = 1;
                    open       = true;
                }

                for (uint32_t k = 0; open && (k < size); k++) {
                    if (have >= sizeof(pending)) {
                        open = false;   // too long for a Single
                    } else {
                        pending[have++] = d[j + k];
                    }
                }

                j += size;

                if (open && (have > 0) && (pending[have - 1] == 0xF7)) {
                    count = add_if_single(pending, have, singles, count, capacity);
                    open  = false;
                }
            } else {
                if ((b & 0x80u) != 0) {
                    status = b;
                    j++;
                }
                j += (((status & 0xF0u) == 0xC0u) || ((status & 0xF0u) == 0xD0u)) ? 1 : 2;
            }
        }
        i = end;
    }
    return count;
}

int ti_patch_read_file(const char * path, uint8_t(*singles)[TI_SINGLE_LENGTH], int capacity) {
    FILE *    f = fopen(path, "rb");
    uint8_t * d;
    long      n;
    int       count;

    if (f == NULL) {
        return 0;
    }

    if ((fseek(f, 0, SEEK_END) != 0) || ((n = ftell(f)) <= 0) || (n > MAX_FILE_BYTES) || (fseek(f, 0, SEEK_SET) != 0)) {
        fclose(f);
        return 0;
    }
    d     = (uint8_t *)malloc((size_t)n);

    if ((d == NULL) || (fread(d, 1, (size_t)n, f) != (size_t)n)) {
        free(d);
        fclose(f);
        return 0;
    }
    fclose(f);
    count = ((n >= 14) && (memcmp(d, "MThd", 4) == 0)) ? from_mid(d, (size_t)n, singles, capacity) : from_syx(d, (size_t)n, singles, capacity);
    free(d);
    return count;
}

void ti_patch_from_single(const uint8_t single[TI_SINGLE_LENGTH], double normalized[kTiParamCount]) {
    for (size_t k = 0; k < sizeof(kCarried) / sizeof(kCarried[0]); k++) {
        int32_t steps = gTiParams[kCarried[k].paramId].stepCount;
        int32_t value = single[kCarried[k].at] & kCarried[k].mask;

        value                           = (value > steps) ? steps : value;
        normalized[kCarried[k].paramId] = (double)value / steps;
    }
}

void ti_patch_name(const uint8_t single[TI_SINGLE_LENGTH], char name[TI_SINGLE_NAME_LENGTH + 1]) {
    int n = TI_SINGLE_NAME_LENGTH;

    for (int i = 0; i < TI_SINGLE_NAME_LENGTH; i++) {
        uint8_t c = single[NAME_AT + i];

        name[i] = ((c >= 32) && (c < 127)) ? (char)c : ' ';
    }

    while ((n > 0) && (name[n - 1] == ' ')) {
        n--;
    }
    name[n] = '\0';
}

static uint8_t checksum(const uint8_t * m, int from, int to) {
    unsigned sum = 0;

    for (int i = from; i < to; i++) {
        sum += m[i];
    }

    return (uint8_t)(sum & 0x7Fu);
}

void ti_patch_to_single(const double normalized[kTiParamCount], const uint8_t * base, const char * name, uint8_t out[TI_SINGLE_LENGTH]) {
    size_t len                                = (name != NULL) ? strlen(name) : 0;
    double was[kTiParamCount];
    char   wasName[TI_SINGLE_NAME_LENGTH + 1] = "";

    if (base != NULL) {
        ti_patch_from_single(base, was);
        ti_patch_name(base, wasName);
    }

    if (base != NULL) {
        memcpy(out, base, TI_SINGLE_LENGTH);
    } else {
        memset(out, 0, TI_SINGLE_LENGTH);
    }
    memcpy(out, kHeader, sizeof(kHeader));
    out[DEVICE_AT]            = OMNI_DEVICE;
    out[COMMAND_AT]           = SINGLE_DUMP;
    out[BANK_AT]              = EDIT_BUFFER_BANK;
    out[PROGRAM_AT]           = EDIT_BUFFER_PROG;
    out[TI_SINGLE_LENGTH - 1] = 0xF7;

    for (size_t k = 0; k < sizeof(kCarried) / sizeof(kCarried[0]); k++) {
        int32_t steps = gTiParams[kCarried[k].paramId].stepCount;
        long    value = lround(fmin(1.0, fmax(0.0, normalized[kCarried[k].paramId])) * steps);

        // notes §4: a byte is kept as it came unless its parameter changed - it may hold more than the panel shows
        if ((base != NULL) && (value == lround(was[kCarried[k].paramId] * steps))) {
            continue;
        }
        out[kCarried[k].at] = (uint8_t)((out[kCarried[k].at] & ~kCarried[k].mask) | ((uint8_t)value & kCarried[k].mask));
    }

    for (size_t i = 0; (base == NULL || strcmp(wasName, (name != NULL) ? name : "") != 0) && (i < TI_SINGLE_NAME_LENGTH); i++) {
        char c = (i < len) ? name[i] : ' ';

        out[NAME_AT + i] = ((c >= 32) && (c < 127)) ? (uint8_t)c : ' ';
    }

    ti_patch_address(out, 0);
}

void ti_patch_address(uint8_t single[TI_SINGLE_LENGTH], int number) {
    single[BANK_AT]      = (number > 0) ? (uint8_t)(1 + ((number - 1) / TI_BANK_SIZE)) : EDIT_BUFFER_BANK;
    single[PROGRAM_AT]   = (number > 0) ? (uint8_t)((number - 1) % TI_BANK_SIZE) : EDIT_BUFFER_PROG;
    single[CHECKSUM1_AT] = checksum(single, DEVICE_AT, CHECKSUM1_AT);
    single[CHECKSUM2_AT] = checksum(single, DEVICE_AT, CHECKSUM2_AT);
}

void ti_patch_init_single(uint8_t out[TI_SINGLE_LENGTH]) {
    double values[kTiParamCount];

    for (uint32_t id = 0; id < kTiParamCount; id++) {
        values[id] = gTiParams[id].defaultNormalized;
    }

    ti_patch_to_single(values, NULL, "Init", out);
}

bool ti_patch_save_file(const char * path, const uint8_t single[TI_SINGLE_LENGTH]) {
    FILE * f = fopen(path, "wb");
    bool   ok;

    if (f == NULL) {
        return false;
    }
    ok = fwrite(single, 1, TI_SINGLE_LENGTH, f) == TI_SINGLE_LENGTH;
    return (fclose(f) == 0) && ok;
}

const char * ti_patch_sounds_folder(void) {
    static char folder[1024];

    snprintf(folder, sizeof(folder), "%s/%s", getenv("HOME") ? getenv("HOME") : "", SOUNDS_FOLDER);
    return folder;
}

static bool patch_file_name(const char * name) {
    size_t n = strlen(name);

    return (name[0] != '.') && (n > 4) && ((strcasecmp(name + n - 4, ".syx") == 0) || (strcasecmp(name + n - 4, ".mid") == 0));
}

static int by_name(const void * a, const void * b) {
    return strcasecmp(*(const char *const *)a, *(const char *const *)b);
}

// ---- the presets - notes §5 ---------------------------------------------------------------------------------

static uint8_t gPresets[TI_PRESETS][TI_SINGLE_LENGTH];
static char    gPresetNames[TI_PRESETS][TI_SINGLE_NAME_LENGTH + 1];
static bool    gPresetsRead;
static time_t  gPresetsModified;

void ti_preset_label(int number, char * out, size_t len) {
    snprintf(out, len, "%c%03d", 'A' + ((number - 1) / TI_BANK_SIZE), ((number - 1) % TI_BANK_SIZE) + 1);
}

const char * ti_presets_path(void) {
    static char path[1024];

    snprintf(path, sizeof(path), "%s/%s/%s", getenv("HOME") ? getenv("HOME") : "", PRESETS_FOLDER, PRESETS_FILE);
    return path;
}

// slots past what was found are Init, and each is addressed to its own place
static void fill_and_address(uint8_t singles[TI_PRESETS][TI_SINGLE_LENGTH], int found) {
    for (int i = 0; i < TI_PRESETS; i++) {
        if (i >= found) {
            ti_patch_init_single(singles[i]);
        }
        ti_patch_address(singles[i], i + 1);
    }
}

static void read_presets(void) {
    static uint8_t found[TI_PRESETS][TI_SINGLE_LENGTH];
    int            count = ti_patch_read_file(ti_presets_path(), found, TI_PRESETS);

    fill_and_address(found, count);
    memcpy(gPresets, found, sizeof(gPresets));

    for (int i = 0; i < TI_PRESETS; i++) {
        ti_patch_name(gPresets[i], gPresetNames[i]);
    }
}

static time_t presets_modified(void) {
    struct stat st;

    return (stat(ti_presets_path(), &st) == 0) ? st.st_mtime : 0;
}

bool ti_presets_refresh(void) {
    time_t modified = presets_modified();

    if (gPresetsRead && (modified == gPresetsModified)) {
        return false;
    }
    gPresetsRead     = true;
    gPresetsModified = modified;
    read_presets();
    return true;
}

const uint8_t * ti_presets_single(int number) {
    return gPresets[((number >= 1) && (number <= TI_PRESETS)) ? (number - 1) : 0];
}

const char * ti_presets_name(int number) {
    return ((number >= 1) && (number <= TI_PRESETS)) ? gPresetNames[number - 1] : "";
}

static bool write_all(const char * path, const uint8_t singles[TI_PRESETS][TI_SINGLE_LENGTH]) {
    FILE * f = fopen(path, "wb");
    bool   ok;

    if (f == NULL) {
        return false;
    }
    ok = fwrite(singles, TI_SINGLE_LENGTH, TI_PRESETS, f) == TI_PRESETS;
    return (fclose(f) == 0) && ok;
}

static bool write_presets(void) {
    char folder[1024];
    bool ok;

    snprintf(folder, sizeof(folder), "%s/%s", getenv("HOME") ? getenv("HOME") : "", PRESETS_FOLDER);

    for (char * p = folder + 1; *p != '\0'; p++) {
        if (*p == '/') {
            *p = '\0';
            mkdir(folder, 0755);
            *p = '/';
        }
    }

    mkdir(folder, 0755);
    ok               = write_all(ti_presets_path(), (const uint8_t(*)[TI_SINGLE_LENGTH]) gPresets);
    gPresetsModified = presets_modified();

    for (int i = 0; i < TI_PRESETS; i++) {
        ti_patch_name(gPresets[i], gPresetNames[i]);
    }

    return ok;
}

bool ti_presets_store(int number, const uint8_t single[TI_SINGLE_LENGTH]) {
    if ((number < 1) || (number > TI_PRESETS)) {
        return false;
    }
    (void)ti_presets_refresh();   // another instance's store is kept
    memcpy(gPresets[number - 1], single, TI_SINGLE_LENGTH);
    ti_patch_address(gPresets[number - 1], number);
    return write_presets();
}

bool ti_presets_store_all(const uint8_t singles[TI_PRESETS][TI_SINGLE_LENGTH]) {
    memcpy(gPresets, singles, sizeof(gPresets));
    fill_and_address(gPresets, TI_PRESETS);
    gPresetsRead = true;
    return write_presets();
}

// notes §6: in file order from A001; what is left over is Init
int ti_presets_read_file(const char * path, uint8_t singles[TI_PRESETS][TI_SINGLE_LENGTH]) {
    int count = ti_patch_read_file(path, singles, TI_PRESETS);

    fill_and_address(singles, count);
    return count;
}

// notes §6: every .syx and .mid in the folder, in name order, one after the other
int ti_presets_read_folder(const char * folder, uint8_t singles[TI_PRESETS][TI_SINGLE_LENGTH]) {
    const char *    names[MAX_FOLDER_FILES];
    int             files = 0, count = 0;
    DIR *           dir = opendir(folder);
    struct dirent * e;
    char            path[2048];

    if (dir == NULL) {
        return 0;
    }

    while (((e = readdir(dir)) != NULL) && (files < MAX_FOLDER_FILES)) {
        if (patch_file_name(e->d_name)) {
            names[files++] = strdup(e->d_name);
        }
    }
    closedir(dir);
    qsort(names, (size_t)files, sizeof(names[0]), by_name);

    for (int i = 0; i < files; i++) {
        if ((names[i] != NULL) && (count < TI_PRESETS)) {
            snprintf(path, sizeof(path), "%s/%s", folder, names[i]);
            count += ti_patch_read_file(path, &singles[count], TI_PRESETS - count);
        }
        free((void *)names[i]);
    }

    fill_and_address(singles, count);
    return count;
}

bool ti_presets_write_file(const char * path) {
    (void)ti_presets_refresh();
    return write_all(path, (const uint8_t(*)[TI_SINGLE_LENGTH]) gPresets);
}
