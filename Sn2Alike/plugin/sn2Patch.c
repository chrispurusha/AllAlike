/*
 * Sn2Alike - Singles: reading them out of .syx and .mid files, and writing one back.
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
// Notes: Docs/code-notes/sn2Patch.md - "// notes §k" refers there.

#include <dirent.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "sn2Patch.h"

#include <sys/stat.h>

#define SOUNDS_FOLDER     "Library/Mobile Documents/com~apple~CloudDocs/Patches/Supernova2"      // under $HOME
#define PRESETS_FOLDER    "Library/Application Support/Sn2Alike"                                 // beside prefs.txt
#define PRESETS_FILE      "Presets.syx"

// notes §1: where things sit in the two messages
#define MAIN_LENGTH       (297)
#define EXT_LENGTH        (99)
#define EXT               (MAIN_LENGTH)
#define DEVICE_AT         (6)
#define COMMAND_AT        (7)
#define BANK_AT           (8)
#define PROGRAM_AT        (9)
#define FIELDS_AT         (10)
#define NAME_AT           (FIELDS_AT)
#define MAIN(field)     (FIELDS_AT + (field))
#define EXTRA(field)    (EXT + FIELDS_AT + (field))

#define MAIN_DUMP           (0x02)
#define EXT_DUMP            (0x1F)
#define DEVICE              (0x01)
#define OSC1_SQUARE         (0)

#define MAX_FILE_BYTES      (16 * 1024 * 1024)
#define MAX_FOLDER_FILES    (4096)

static const uint8_t kHeader[6] = {0xF0, 0x00, 0x20, 0x29, 0x01, 0x24};

// notes §3: the bits of a byte a parameter takes; the rest of the byte is kept when it is written back
static const struct {
    uint32_t paramId;
    uint16_t at;
    uint8_t  mask;
}                    kCarried[] = {
    {kSn2ParamCutoff,         MAIN(211), 0x7F},
    {kSn2ParamResonance,      MAIN(221), 0x7F},
    {kSn2ParamFilterType,     MAIN(207), 0x7F},
    {kSn2ParamFilterSlope,    MAIN(208), 0x7F},
    {kSn2ParamPulseWidth,     MAIN(41),  0x7F},
    {kSn2ParamAmpAttack,      MAIN(194), 0x7F},
    {kSn2ParamAmpDecay,       MAIN(195), 0x7F},
    {kSn2ParamAmpSustain,     MAIN(196), 0x7F},
    {kSn2ParamAmpRelease,     MAIN(197), 0x7F},
    {kSn2ParamAmpSustainTime, EXTRA(16), 0x7F},
};

static bool is_message(const uint8_t * m, size_t len, size_t want, uint8_t command) {
    return (len == want) && (memcmp(m, kHeader, sizeof(kHeader)) == 0) && (m[COMMAND_AT] == command) && (m[want - 1] == 0xF7);
}

static void fresh_header(uint8_t * m, size_t len, uint8_t command) {
    memcpy(m, kHeader, sizeof(kHeader));
    m[DEVICE_AT]  = DEVICE;
    m[COMMAND_AT] = command;
    m[len - 1]    = 0xF7;
}

// notes §1: a main dump starts a program, and the extension with its address completes it
static int add_if_single(const uint8_t * m, size_t len, uint8_t(*singles)[SN2_SINGLE_LENGTH], int count, int capacity) {
    if ((count < capacity) && is_message(m, len, MAIN_LENGTH, MAIN_DUMP)) {
        uint8_t * out = singles[count++];

        memset(out, 0, SN2_SINGLE_LENGTH);
        memcpy(out, m, MAIN_LENGTH);
        fresh_header(out + EXT, EXT_LENGTH, EXT_DUMP);
        out[EXT + BANK_AT]    = m[BANK_AT];
        out[EXT + PROGRAM_AT] = m[PROGRAM_AT];
    } else if (  (count > 0) && is_message(m, len, EXT_LENGTH, EXT_DUMP)
              && (singles[count - 1][BANK_AT] == m[BANK_AT]) && (singles[count - 1][PROGRAM_AT] == m[PROGRAM_AT])) {
        memcpy(singles[count - 1] + EXT, m, EXT_LENGTH);
    }
    return count;
}

static int from_syx(const uint8_t * d, size_t n, uint8_t(*singles)[SN2_SINGLE_LENGTH], int capacity) {
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
static int from_mid(const uint8_t * d, size_t n, uint8_t(*singles)[SN2_SINGLE_LENGTH], int capacity) {
    static uint8_t pending[SN2_SINGLE_LENGTH + 1];
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

int sn2_patch_read_file(const char * path, uint8_t(*singles)[SN2_SINGLE_LENGTH], int capacity) {
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

void sn2_patch_from_single(const uint8_t single[SN2_SINGLE_LENGTH], double normalized[kSn2ParamCount]) {
    for (size_t k = 0; k < sizeof(kCarried) / sizeof(kCarried[0]); k++) {
        int32_t steps = gSn2Params[kCarried[k].paramId].stepCount;
        int32_t value = single[kCarried[k].at] & kCarried[k].mask;

        value                           = (value > steps) ? steps : value;
        normalized[kCarried[k].paramId] = (double)value / steps;
    }

    // the placeholder oscillator's nearest shape
    normalized[kSn2ParamOscShape] = (single[MAIN(22)] == OSC1_SQUARE) ? 0.5 : 0.0;
}

void sn2_patch_name(const uint8_t single[SN2_SINGLE_LENGTH], char name[SN2_SINGLE_NAME_LENGTH + 1]) {
    int n = SN2_SINGLE_NAME_LENGTH;

    for (int i = 0; i < SN2_SINGLE_NAME_LENGTH; i++) {
        uint8_t c = single[NAME_AT + i];

        name[i] = ((c >= 32) && (c < 127)) ? (char)c : ' ';
    }

    while ((n > 0) && (name[n - 1] == ' ')) {
        n--;
    }
    name[n] = '\0';
}

void sn2_patch_to_single(const double normalized[kSn2ParamCount], const uint8_t * base, const char * name, uint8_t out[SN2_SINGLE_LENGTH]) {
    size_t len                                 = (name != NULL) ? strlen(name) : 0;
    double was[kSn2ParamCount];
    char   wasName[SN2_SINGLE_NAME_LENGTH + 1] = "";

    if (base != NULL) {
        sn2_patch_from_single(base, was);
        sn2_patch_name(base, wasName);
    }

    if (base != NULL) {
        memcpy(out, base, SN2_SINGLE_LENGTH);
    } else {
        memset(out, 0, SN2_SINGLE_LENGTH);
    }
    fresh_header(out, MAIN_LENGTH, MAIN_DUMP);
    fresh_header(out + EXT, EXT_LENGTH, EXT_DUMP);

    for (size_t k = 0; k < sizeof(kCarried) / sizeof(kCarried[0]); k++) {
        int32_t steps = gSn2Params[kCarried[k].paramId].stepCount;
        long    value = lround(fmin(1.0, fmax(0.0, normalized[kCarried[k].paramId])) * steps);

        // notes §4: a byte is kept as it came unless its parameter changed - it may hold more than the panel shows
        if ((base != NULL) && (value == lround(was[kCarried[k].paramId] * steps))) {
            continue;
        }
        out[kCarried[k].at] = (uint8_t)((out[kCarried[k].at] & ~kCarried[k].mask) | ((uint8_t)value & kCarried[k].mask));
    }

    for (size_t i = 0; (base == NULL || strcmp(wasName, (name != NULL) ? name : "") != 0) && (i < SN2_SINGLE_NAME_LENGTH); i++) {
        char c = (i < len) ? name[i] : ' ';

        out[NAME_AT + i] = ((c >= 32) && (c < 127)) ? (uint8_t)c : ' ';
    }

    sn2_patch_address(out, 0);
}

void sn2_patch_address(uint8_t single[SN2_SINGLE_LENGTH], int number) {
    uint8_t bank    = (number > 0) ? (uint8_t)((number - 1) / SN2_BANK_SIZE) : 0;
    uint8_t program = (number > 0) ? (uint8_t)((number - 1) % SN2_BANK_SIZE) : 0;

    single[BANK_AT]          = bank;
    single[PROGRAM_AT]       = program;
    single[EXT + BANK_AT]    = bank;
    single[EXT + PROGRAM_AT] = program;
}

void sn2_patch_init_single(uint8_t out[SN2_SINGLE_LENGTH]) {
    double values[kSn2ParamCount];

    for (uint32_t id = 0; id < kSn2ParamCount; id++) {
        values[id] = gSn2Params[id].defaultNormalized;
    }

    sn2_patch_to_single(values, NULL, "Init", out);
}

bool sn2_patch_save_file(const char * path, const uint8_t single[SN2_SINGLE_LENGTH]) {
    FILE * f = fopen(path, "wb");
    bool   ok;

    if (f == NULL) {
        return false;
    }
    ok = fwrite(single, 1, SN2_SINGLE_LENGTH, f) == SN2_SINGLE_LENGTH;
    return (fclose(f) == 0) && ok;
}

const char * sn2_patch_sounds_folder(void) {
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

static uint8_t gPresets[SN2_PRESETS][SN2_SINGLE_LENGTH];
static char    gPresetNames[SN2_PRESETS][SN2_SINGLE_NAME_LENGTH + 1];
static bool    gPresetsRead;
static time_t  gPresetsModified;

void sn2_preset_label(int number, char * out, size_t len) {
    snprintf(out, len, "%c%03d", 'A' + ((number - 1) / SN2_BANK_SIZE), ((number - 1) % SN2_BANK_SIZE) + 1);
}

const char * sn2_presets_path(void) {
    static char path[1024];

    snprintf(path, sizeof(path), "%s/%s/%s", getenv("HOME") ? getenv("HOME") : "", PRESETS_FOLDER, PRESETS_FILE);
    return path;
}

// slots past what was found are Init, and each is addressed to its own place
static void fill_and_address(uint8_t singles[SN2_PRESETS][SN2_SINGLE_LENGTH], int found) {
    for (int i = 0; i < SN2_PRESETS; i++) {
        if (i >= found) {
            sn2_patch_init_single(singles[i]);
        }
        sn2_patch_address(singles[i], i + 1);
    }
}

static void read_presets(void) {
    static uint8_t found[SN2_PRESETS][SN2_SINGLE_LENGTH];
    int            count = sn2_patch_read_file(sn2_presets_path(), found, SN2_PRESETS);

    fill_and_address(found, count);
    memcpy(gPresets, found, sizeof(gPresets));

    for (int i = 0; i < SN2_PRESETS; i++) {
        sn2_patch_name(gPresets[i], gPresetNames[i]);
    }
}

static time_t presets_modified(void) {
    struct stat st;

    return (stat(sn2_presets_path(), &st) == 0) ? st.st_mtime : 0;
}

bool sn2_presets_refresh(void) {
    time_t modified = presets_modified();

    if (gPresetsRead && (modified == gPresetsModified)) {
        return false;
    }
    gPresetsRead     = true;
    gPresetsModified = modified;
    read_presets();
    return true;
}

const uint8_t * sn2_presets_single(int number) {
    return gPresets[((number >= 1) && (number <= SN2_PRESETS)) ? (number - 1) : 0];
}

const char * sn2_presets_name(int number) {
    return ((number >= 1) && (number <= SN2_PRESETS)) ? gPresetNames[number - 1] : "";
}

static bool write_all(const char * path, const uint8_t singles[SN2_PRESETS][SN2_SINGLE_LENGTH]) {
    FILE * f = fopen(path, "wb");
    bool   ok;

    if (f == NULL) {
        return false;
    }
    ok = fwrite(singles, SN2_SINGLE_LENGTH, SN2_PRESETS, f) == SN2_PRESETS;
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
    ok               = write_all(sn2_presets_path(), (const uint8_t(*)[SN2_SINGLE_LENGTH]) gPresets);
    gPresetsModified = presets_modified();

    for (int i = 0; i < SN2_PRESETS; i++) {
        sn2_patch_name(gPresets[i], gPresetNames[i]);
    }

    return ok;
}

bool sn2_presets_store(int number, const uint8_t single[SN2_SINGLE_LENGTH]) {
    if ((number < 1) || (number > SN2_PRESETS)) {
        return false;
    }
    (void)sn2_presets_refresh();   // another instance's store is kept
    memcpy(gPresets[number - 1], single, SN2_SINGLE_LENGTH);
    sn2_patch_address(gPresets[number - 1], number);
    return write_presets();
}

bool sn2_presets_store_all(const uint8_t singles[SN2_PRESETS][SN2_SINGLE_LENGTH]) {
    memcpy(gPresets, singles, sizeof(gPresets));
    fill_and_address(gPresets, SN2_PRESETS);
    gPresetsRead = true;
    return write_presets();
}

// notes §6: in file order from A001; what is left over is Init
int sn2_presets_read_file(const char * path, uint8_t singles[SN2_PRESETS][SN2_SINGLE_LENGTH]) {
    int count = sn2_patch_read_file(path, singles, SN2_PRESETS);

    fill_and_address(singles, count);
    return count;
}

// notes §6: every .syx and .mid in the folder, in name order, one after the other
int sn2_presets_read_folder(const char * folder, uint8_t singles[SN2_PRESETS][SN2_SINGLE_LENGTH]) {
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
        if ((names[i] != NULL) && (count < SN2_PRESETS)) {
            snprintf(path, sizeof(path), "%s/%s", folder, names[i]);
            count += sn2_patch_read_file(path, &singles[count], SN2_PRESETS - count);
        }
        free((void *)names[i]);
    }

    fill_and_address(singles, count);
    return count;
}

bool sn2_presets_write_file(const char * path) {
    (void)sn2_presets_refresh();
    return write_all(path, (const uint8_t(*)[SN2_SINGLE_LENGTH]) gPresets);
}
