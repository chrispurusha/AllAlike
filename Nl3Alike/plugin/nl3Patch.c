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

#include <dirent.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "nl3Patch.h"

#include <sys/stat.h>

#define SOUNDS_FOLDER       "Library/Mobile Documents/com~apple~CloudDocs/Patches/Nord Lead 3"  // under $HOME
#define PRESETS_FOLDER      "Library/Application Support/Nl3Alike"                              // beside prefs.txt
#define PRESETS_FILE        "Presets.syx"

// notes §1: one message
#define DEVICE_AT           (2)
#define MODEL_AT            (3)
#define COMMAND_AT          (4)
#define BANK_AT             (5)     // the Slot for a dump from a Slot
#define PROGRAM_AT          (6)
#define NAME_AT             (7)
#define CATEGORY_AT         (23)
#define VERSION_AT          (39)
#define PACKED_AT           (41)
#define PACKED_LENGTH       (NL3_SINGLE_LENGTH - PACKED_AT - 1)

#define CLAVIA              (0x33)
#define MODEL               (0x09)
#define FROM_SLOT           (0x20)
#define FROM_MEMORY         (0x21)
#define ALL_DEVICES         (0x7F)
#define VERSION             (120)
#define BODY_LENGTH         (191)
#define CHECKSUM_AT         (BODY_LENGTH - 1)

#define MAX_FILE_BYTES      (16 * 1024 * 1024)
#define MAX_FOLDER_FILES    (4096)

static bool is_program(const uint8_t * m, size_t len) {
    return (len == NL3_SINGLE_LENGTH) && (m[0] == 0xF0) && (m[1] == CLAVIA) && (m[MODEL_AT] == MODEL)
           && ((m[COMMAND_AT] == FROM_SLOT) || (m[COMMAND_AT] == FROM_MEMORY)) && (m[len - 1] == 0xF7);
}

static int add_if_single(const uint8_t * m, size_t len, uint8_t(*singles)[NL3_SINGLE_LENGTH], int count, int capacity) {
    if ((count < capacity) && is_program(m, len)) {
        memcpy(singles[count++], m, NL3_SINGLE_LENGTH);
    }
    return count;
}

static int from_syx(const uint8_t * d, size_t n, uint8_t(*singles)[NL3_SINGLE_LENGTH], int capacity) {
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
static int from_mid(const uint8_t * d, size_t n, uint8_t(*singles)[NL3_SINGLE_LENGTH], int capacity) {
    static uint8_t pending[NL3_SINGLE_LENGTH + 1];
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

int nl3_patch_read_file(const char * path, uint8_t(*singles)[NL3_SINGLE_LENGTH], int capacity) {
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

// notes §1: the body is a stream of 7-bit bytes read as one run of bits, eight at a time
static void unpack(const uint8_t * single, uint8_t body[BODY_LENGTH]) {
    uint32_t acc = 0, have = 0, n = 0;

    for (uint32_t i = 0; (i < PACKED_LENGTH) && (n < BODY_LENGTH); i++) {
        acc   = (acc << 7) | (single[PACKED_AT + i] & 0x7Fu);
        have += 7;

        if (have >= 8) {
            have     -= 8;
            body[n++] = (uint8_t)(acc >> have);
            acc      &= (1u << have) - 1u;
        }
    }

    while (n < BODY_LENGTH) {
        body[n++] = 0;
    }
}

static void pack(const uint8_t body[BODY_LENGTH], uint8_t * single) {
    uint32_t acc = 0, have = 0, n = 0;

    for (uint32_t i = 0; (i < BODY_LENGTH) && (n < PACKED_LENGTH); i++) {
        acc   = (acc << 8) | body[i];
        have += 8;

        while ((have >= 7) && (n < PACKED_LENGTH)) {
            have                   -= 7;
            single[PACKED_AT + n++] = (uint8_t)((acc >> have) & 0x7Fu);
            acc                    &= (1u << have) - 1u;
        }
    }

    if ((have > 0) && (n < PACKED_LENGTH)) {
        single[PACKED_AT + n++] = (uint8_t)((acc << (7 - have)) & 0x7Fu);
    }

    while (n < PACKED_LENGTH) {
        single[PACKED_AT + n++] = 0;
    }
}

static uint32_t get_bits(const uint8_t * body, uint32_t at, uint32_t bits) {
    uint32_t v = 0;

    for (uint32_t i = 0; i < bits; i++) {
        v = (v << 1) | ((body[(at + i) / 8] >> (7 - ((at + i) % 8))) & 1u);
    }

    return v;
}

static void set_bits(uint8_t * body, uint32_t at, uint32_t bits, uint32_t v) {
    for (uint32_t i = 0; i < bits; i++) {
        uint32_t bit  = at + i;
        uint8_t  mask = (uint8_t)(1u << (7 - (bit % 8)));
        bool     one  = ((v >> (bits - 1 - i)) & 1u) != 0;

        body[bit / 8] = (uint8_t)(one ? (body[bit / 8] | mask) : (body[bit / 8] & ~mask));
    }
}

// notes §1: the byte sum of the body before it
static uint8_t checksum(const uint8_t body[BODY_LENGTH]) {
    uint8_t sum = 0;

    for (uint32_t i = 0; i < CHECKSUM_AT; i++) {
        sum = (uint8_t)(sum + body[i]);
    }

    return sum;
}

// notes §3: a field's stored value as the parameter's plain value
static int32_t field_plain(const uint8_t body[BODY_LENGTH], const tNl3Field * f) {
    int32_t v = (int32_t)get_bits(body, f->at, f->bits);

    if (f->isSigned && (v >= (1 << (f->bits - 1)))) {
        v -= 1 << f->bits;
    }
    return v;
}

void nl3_patch_from_single(const uint8_t single[NL3_SINGLE_LENGTH], double normalized[kNl3ParamCount]) {
    uint8_t body[BODY_LENGTH];

    unpack(single, body);

    for (uint32_t k = 0; k < gNl3FieldCount; k++) {
        const tNl3Field *      f     = &gNl3Fields[k];
        const tSynthLibParam * p     = &gNl3Params[f->paramId];
        double                 value = fmin(p->plainMax, fmax(p->plainMin, (double)field_plain(body, f)));

        normalized[f->paramId] = (value - p->plainMin) / (p->plainMax - p->plainMin);
    }
}

void nl3_patch_name(const uint8_t single[NL3_SINGLE_LENGTH], char name[NL3_SINGLE_NAME_LENGTH + 1]) {
    int n = 0;

    for (int i = 0; (i < NL3_SINGLE_NAME_LENGTH) && (single[NAME_AT + i] != 0); i++) {
        uint8_t c = single[NAME_AT + i];

        name[n++] = ((c >= 32) && (c < 127)) ? (char)c : ' ';
    }

    while ((n > 0) && (name[n - 1] == ' ')) {
        n--;
    }
    name[n] = '\0';
}

static void fresh_header(uint8_t * single) {
    single[0]                     = 0xF0;
    single[1]                     = CLAVIA;
    single[DEVICE_AT]             = ALL_DEVICES;
    single[MODEL_AT]              = MODEL;
    single[NL3_SINGLE_LENGTH - 1] = 0xF7;
}

void nl3_patch_to_single(const double normalized[kNl3ParamCount], const uint8_t * base, const char * name, uint8_t out[NL3_SINGLE_LENGTH]) {
    uint8_t body[BODY_LENGTH];

    if (base != NULL) {
        memcpy(out, base, NL3_SINGLE_LENGTH);
        unpack(base, body);
    } else {
        memset(out, 0, NL3_SINGLE_LENGTH);
        memset(body, 0, sizeof(body));
        set_bits(body, 0, 16, VERSION);
        out[VERSION_AT]     = (uint8_t)(VERSION >> 7);    // notes §4: a program keeps the version it came with
        out[VERSION_AT + 1] = (uint8_t)(VERSION & 0x7F);
    }
    fresh_header(out);

    // notes §4: every field is written; the bits no parameter covers stay as they came
    for (uint32_t k = 0; k < gNl3FieldCount; k++) {
        const tNl3Field *      f     = &gNl3Fields[k];
        const tSynthLibParam * p     = &gNl3Params[f->paramId];
        long                   value = lround(p->plainMin + (fmin(1.0, fmax(0.0, normalized[f->paramId])) * (p->plainMax - p->plainMin)));

        set_bits(body, f->at, f->bits, (uint32_t)value & ((1u << f->bits) - 1u));
    }

    body[CHECKSUM_AT] = checksum(body);
    pack(body, out);

    if ((name != NULL) || (base == NULL)) {
        size_t len = (name != NULL) ? strlen(name) : 0;

        for (size_t i = 0; i < NL3_SINGLE_NAME_LENGTH; i++) {
            char c = (i < len) ? name[i] : '\0';

            out[NAME_AT + i] = ((c >= 32) && (c < 127)) ? (uint8_t)c : 0;
        }
    }
    nl3_patch_address(out, 0);
}

// notes §5: 0 is Slot A's edit buffer; a preset is bank and program
void nl3_patch_address(uint8_t single[NL3_SINGLE_LENGTH], int number) {
    single[COMMAND_AT] = (number > 0) ? FROM_MEMORY : FROM_SLOT;
    single[BANK_AT]    = (number > 0) ? (uint8_t)((number - 1) / NL3_BANK_SIZE) : 0;
    single[PROGRAM_AT] = (number > 0) ? (uint8_t)((number - 1) % NL3_BANK_SIZE) : 0;
}

void nl3_patch_init_single(uint8_t out[NL3_SINGLE_LENGTH]) {
    double values[kNl3ParamCount];

    for (uint32_t id = 0; id < kNl3ParamCount; id++) {
        values[id] = gNl3Params[id].defaultNormalized;
    }

    nl3_patch_to_single(values, NULL, "Init", out);
}

bool nl3_patch_save_file(const char * path, const uint8_t single[NL3_SINGLE_LENGTH]) {
    FILE * f = fopen(path, "wb");
    bool   ok;

    if (f == NULL) {
        return false;
    }
    ok = fwrite(single, 1, NL3_SINGLE_LENGTH, f) == NL3_SINGLE_LENGTH;
    return (fclose(f) == 0) && ok;
}

const char * nl3_patch_sounds_folder(void) {
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

static uint8_t gPresets[NL3_PRESETS][NL3_SINGLE_LENGTH];
static char    gPresetNames[NL3_PRESETS][NL3_SINGLE_NAME_LENGTH + 1];
static bool    gPresetsRead;
static time_t  gPresetsModified;

void nl3_preset_label(int number, char * out, size_t len) {
    snprintf(out, len, "%d:%d", 1 + ((number - 1) / NL3_BANK_SIZE), 1 + ((number - 1) % NL3_BANK_SIZE));   // bank:program from 1, as the instrument shows them
}

const char * nl3_presets_path(void) {
    static char path[1024];

    snprintf(path, sizeof(path), "%s/%s/%s", getenv("HOME") ? getenv("HOME") : "", PRESETS_FOLDER, PRESETS_FILE);
    return path;
}

// slots past what was found are Init, and each is addressed to its own place
static void fill_and_address(uint8_t singles[NL3_PRESETS][NL3_SINGLE_LENGTH], int found) {
    for (int i = 0; i < NL3_PRESETS; i++) {
        if (i >= found) {
            nl3_patch_init_single(singles[i]);
        }
        nl3_patch_address(singles[i], i + 1);
    }
}

static void read_presets(void) {
    static uint8_t found[NL3_PRESETS][NL3_SINGLE_LENGTH];
    int            count = nl3_patch_read_file(nl3_presets_path(), found, NL3_PRESETS);

    fill_and_address(found, count);
    memcpy(gPresets, found, sizeof(gPresets));

    for (int i = 0; i < NL3_PRESETS; i++) {
        nl3_patch_name(gPresets[i], gPresetNames[i]);
    }
}

static time_t presets_modified(void) {
    struct stat st;

    return (stat(nl3_presets_path(), &st) == 0) ? st.st_mtime : 0;
}

bool nl3_presets_refresh(void) {
    time_t modified = presets_modified();

    if (gPresetsRead && (modified == gPresetsModified)) {
        return false;
    }
    gPresetsRead     = true;
    gPresetsModified = modified;
    read_presets();
    return true;
}

const uint8_t * nl3_presets_single(int number) {
    return gPresets[((number >= 1) && (number <= NL3_PRESETS)) ? (number - 1) : 0];
}

const char * nl3_presets_name(int number) {
    return ((number >= 1) && (number <= NL3_PRESETS)) ? gPresetNames[number - 1] : "";
}

static bool write_all(const char * path, const uint8_t singles[NL3_PRESETS][NL3_SINGLE_LENGTH]) {
    FILE * f = fopen(path, "wb");
    bool   ok;

    if (f == NULL) {
        return false;
    }
    ok = fwrite(singles, NL3_SINGLE_LENGTH, NL3_PRESETS, f) == NL3_PRESETS;
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
    ok               = write_all(nl3_presets_path(), (const uint8_t(*)[NL3_SINGLE_LENGTH]) gPresets);
    gPresetsModified = presets_modified();

    for (int i = 0; i < NL3_PRESETS; i++) {
        nl3_patch_name(gPresets[i], gPresetNames[i]);
    }

    return ok;
}

bool nl3_presets_store(int number, const uint8_t single[NL3_SINGLE_LENGTH]) {
    if ((number < 1) || (number > NL3_PRESETS)) {
        return false;
    }
    (void)nl3_presets_refresh();   // another instance's store is kept
    memcpy(gPresets[number - 1], single, NL3_SINGLE_LENGTH);
    nl3_patch_address(gPresets[number - 1], number);
    return write_presets();
}

bool nl3_presets_store_all(const uint8_t singles[NL3_PRESETS][NL3_SINGLE_LENGTH]) {
    memcpy(gPresets, singles, sizeof(gPresets));
    fill_and_address(gPresets, NL3_PRESETS);
    gPresetsRead = true;
    return write_presets();
}

// notes §6: at their own bank and number where they carry one; what is left over is Init
int nl3_presets_read_file(const char * path, uint8_t singles[NL3_PRESETS][NL3_SINGLE_LENGTH]) {
    static uint8_t found[NL3_PRESETS][NL3_SINGLE_LENGTH];
    static bool    taken[NL3_PRESETS];
    int            count    = nl3_patch_read_file(path, found, NL3_PRESETS);
    int            next     = 0;

    static int     where[NL3_PRESETS];
    bool           distinct = true;
    int            bank = 0, last = NL3_BANK_SIZE;

    memset(taken, 0, sizeof(taken));

    // notes §6: by bank and number when every program has its own; a bank file's bank is the one selected when it
    // is received, so otherwise a bank starts wherever the program number goes back down
    for (int i = 0; i < count; i++) {
        where[i] = (found[i][COMMAND_AT] == FROM_MEMORY) ? ((found[i][BANK_AT] * NL3_BANK_SIZE) + found[i][PROGRAM_AT]) : -1;

        for (int j = 0; distinct && (where[i] >= 0) && (j < i); j++) {
            distinct = (where[j] != where[i]);
        }
    }

    for (int i = 0; i < count; i++) {
        if (where[i] < 0) {
            continue;
        }

        if (!distinct) {
            bank    += (i > 0) && (found[i][PROGRAM_AT] <= last) ? 1 : 0;
            last     = found[i][PROGRAM_AT];
            where[i] = (bank * NL3_BANK_SIZE) + found[i][PROGRAM_AT];
        }
        int at = where[i];

        if ((at < NL3_PRESETS) && !taken[at]) {
            memcpy(singles[at], found[i], NL3_SINGLE_LENGTH);
            taken[at] = true;
        }
    }

    for (int i = 0; i < count; i++) {
        if (found[i][COMMAND_AT] != FROM_MEMORY) {
            while ((next < NL3_PRESETS) && taken[next]) {
                next++;
            }

            if (next < NL3_PRESETS) {
                memcpy(singles[next], found[i], NL3_SINGLE_LENGTH);
                taken[next] = true;
            }
        }
    }

    for (int i = 0; i < NL3_PRESETS; i++) {
        if (!taken[i]) {
            nl3_patch_init_single(singles[i]);
        }
        nl3_patch_address(singles[i], i + 1);
    }

    return count;
}

// notes §6: every .syx and .mid in the folder, in name order, one after the other
int nl3_presets_read_folder(const char * folder, uint8_t singles[NL3_PRESETS][NL3_SINGLE_LENGTH]) {
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
        if ((names[i] != NULL) && (count < NL3_PRESETS)) {
            snprintf(path, sizeof(path), "%s/%s", folder, names[i]);
            count += nl3_patch_read_file(path, &singles[count], NL3_PRESETS - count);
        }
        free((void *)names[i]);
    }

    fill_and_address(singles, count);
    return count;
}

bool nl3_presets_write_file(const char * path) {
    (void)nl3_presets_refresh();
    return write_all(path, (const uint8_t(*)[NL3_SINGLE_LENGTH]) gPresets);
}
