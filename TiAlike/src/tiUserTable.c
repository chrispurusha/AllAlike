/*
 * TiAlike - reading a user wavetable from a file: one cycle per frame.
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
// Notes: Docs/code-notes/tiUserTable.md - "// notes §k" refers there.

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "tiOsc.h"
#include "tiUserTable.h"

#define WALDORF_WAVE      (128)    // notes §3: samples in a wave
#define WALDORF_WAVES     (64)
#define WALDORF_BODY      (WALDORF_WAVE * 3)
#define MAX_FILE_BYTES    (64u << 20)

static uint8_t * read_file(const char * path, size_t * len) {
    FILE *    f = fopen(path, "rb");
    uint8_t * d = NULL;
    long      n;

    if (f == NULL) {
        return NULL;
    }

    if ((fseek(f, 0, SEEK_END) == 0) && ((n = ftell(f)) > 0) && ((size_t)n <= MAX_FILE_BYTES) && (fseek(f, 0, SEEK_SET) == 0)) {
        d    = (uint8_t *)malloc((size_t)n);

        if ((d != NULL) && (fread(d, 1, (size_t)n, f) != (size_t)n)) {
            free(d);
            d = NULL;
        }
        *len = (size_t)n;
    }
    fclose(f);
    return d;
}

static uint32_t le32(const uint8_t * p) {
    return (uint32_t)p[0] | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
}

static uint16_t le16(const uint8_t * p) {
    return (uint16_t)(p[0] | (p[1] << 8));
}

// notes §2: one sample of the first channel, -1..1
static float wav_sample(const uint8_t * p, int format, int bits) {
    if (format == 3) {
        if (bits == 32) {
            float f;

            memcpy(&f, p, 4);
            return f;
        }
        double d;

        memcpy(&d, p, 8);
        return (float)d;
    }

    switch (bits) {
        case 8:  return ((float)p[0] - 128.0f) / 128.0f;

        case 16: return (float)(int16_t)le16(p) / 32768.0f;

        case 24: return (float)((int32_t)((uint32_t)p[0] << 8 | (uint32_t)p[1] << 16 | (uint32_t)p[2] << 24) >> 8) / 8388608.0f;

        default: return (float)(int32_t)le32(p) / 2147483648.0f;
    }
}

// notes §2: the cycle length - the file's own mark if it has one, else by its rate and length
static int wav_cycle(int marked, uint32_t rate, size_t samples) {
    if ((marked >= 8) && ((size_t)marked <= samples)) {
        return marked;
    }

    if (rate == 22050u) {
        return 256;
    }

    if ((samples % 2048u) == 0u) {
        return 2048;
    }

    if ((samples % 256u) == 0u) {
        return 256;
    }
    return (samples <= 4096u) ? (int)samples : 2048;
}

static bool load_wav(const uint8_t * d, size_t len, tTiUserCycles * out) {
    int             format = 0, channels = 0, bits = 0, marked = 0;
    uint32_t        rate  = 0;
    const uint8_t * data  = NULL;
    size_t          bytes = 0;

    if ((len < 12) || (memcmp(d, "RIFF", 4) != 0) || (memcmp(d + 8, "WAVE", 4) != 0)) {
        return false;
    }

    for (size_t at = 12; at + 8 <= len;) {
        uint32_t n    = le32(d + at + 4);
        size_t   body = at + 8;

        if (body + n > len) {
            n = (uint32_t)(len - body);
        }

        if ((memcmp(d + at, "fmt ", 4) == 0) && (n >= 16)) {
            format   = le16(d + body);
            channels = le16(d + body + 2);
            rate     = le32(d + body + 4);
            bits     = le16(d + body + 14);

            if ((format == 0xFFFE) && (n >= 26)) {
                format = le16(d + body + 24);   // the extensible format's own code
            }
        } else if ((memcmp(d + at, "clm ", 4) == 0) && (n >= 4) && (memcmp(d + body, "<!>", 3) == 0)) {
            marked = atoi((const char *)d + body + 3);   // notes §2: "<!>2048 ..."
        } else if (memcmp(d + at, "data", 4) == 0) {
            data  = d + body;
            bytes = n;
        }
        at = body + n + (n & 1u);
    }

    int    step    = (bits / 8) * channels;

    if ((data == NULL) || (channels < 1) || !(((format == 1) && (bits >= 8) && (bits <= 32)) || ((format == 3) && ((bits == 32) || (bits == 64)))) || (step <= 0)) {
        return false;
    }
    size_t samples = bytes / (size_t)step;
    int    cycle   = wav_cycle(marked, rate, samples);
    int    frames  = (int)(samples / (size_t)cycle);

    if (frames < 1) {
        return false;
    }
    // notes §2: more frames than a table holds - evenly spaced ones
    int    keep    = (frames > TI_USER_TABLE_FRAMES) ? TI_USER_TABLE_FRAMES : frames;

    out->samples = (float *)malloc(sizeof(float) * (size_t)keep * (size_t)cycle);

    if (out->samples == NULL) {
        return false;
    }

    for (int k = 0; k < keep; k++) {
        int    from = (keep > 1) ? (int)(((long)k * (frames - 1)) / (keep - 1)) : 0;
        size_t base = (size_t)from * (size_t)cycle;

        for (int n = 0; n < cycle; n++) {
            out->samples[((size_t)k * cycle) + n] = wav_sample(data + ((base + n) * (size_t)step), format, bits);
        }
    }

    out->frames  = keep;
    out->cycle   = cycle;
    return true;
}

// notes §3: Waldorf user wavetables, one message a wave - in a .syx, or in a .mid as its SysEx events
static bool load_waldorf(const uint8_t * d, size_t len, tTiUserCycles * out) {
    int   frames              = 0;
    float waves[WALDORF_WAVES][WALDORF_WAVE];
    bool  have[WALDORF_WAVES] = {false};

    for (size_t i = 0; i + 7 + WALDORF_BODY <= len; i++) {
        if ((d[i] != 0x3E) || (d[i + 1] != 0x13) || (d[i + 3] != 0x12) || (d[i + 5] >= WALDORF_WAVES)) {
            continue;
        }
        int             wave = d[i + 5];
        const uint8_t * b    = d + i + 7;

        for (int n = 0; n < WALDORF_WAVE; n++) {
            int32_t v = ((int32_t)(b[3 * n] & 0x7F) << 14) | ((int32_t)(b[(3 * n) + 1] & 0x7F) << 7) | (b[(3 * n) + 2] & 0x7F);

            v              = (v >= (1 << 20)) ? (v - (1 << 21)) : v;
            waves[wave][n] = (float)v / (float)(1 << 20);
        }

        if ((out->name[0] == '\0') && (i + 7 + WALDORF_BODY + 14 <= len)) {
            const uint8_t * name = b + WALDORF_BODY;
            int             end  = 14;

            for (int c = 0; c < 14; c++) {
                out->name[c] = ((name[c] >= 0x20) && (name[c] < 0x7F)) ? (char)name[c] : ' ';
            }

            while ((end > 0) && (out->name[end - 1] == ' ')) {
                end--;
            }
            out->name[end] = '\0';
        }
        have[wave] = true;
        frames     = (wave + 1 > frames) ? (wave + 1) : frames;
        i         += 6 + WALDORF_BODY;
    }

    if (frames < 1) {
        return false;
    }
    out->samples = (float *)malloc(sizeof(float) * (size_t)frames * WALDORF_WAVE);

    if (out->samples == NULL) {
        return false;
    }

    for (int k = 0; k < frames; k++) {
        int from = k;

        while ((from > 0) && !have[from]) {
            from--;   // a wave missing from the dump holds the one before it
        }
        memcpy(out->samples + ((size_t)k * WALDORF_WAVE), waves[from], sizeof(waves[0]));
    }

    out->frames  = frames;
    out->cycle   = WALDORF_WAVE;
    return true;
}

static const char * extension(const char * path) {
    const char * dot   = strrchr(path, '.');
    const char * slash = strrchr(path, '/');

    return ((dot == NULL) || ((slash != NULL) && (dot < slash))) ? "" : dot + 1;
}

bool ti_user_table_file(const char * path) {
    const char * e = extension(path);

    return (strcasecmp(e, "wav") == 0) || (strcasecmp(e, "syx") == 0) || (strcasecmp(e, "mid") == 0);
}

bool ti_user_cycles_load(const char * path, tTiUserCycles * out) {
    size_t    len = 0;
    uint8_t * d;
    bool      ok;

    memset(out, 0, sizeof(*out));

    if (!ti_user_table_file(path) || ((d = read_file(path, &len)) == NULL)) {
        return false;
    }
    ok = (strcasecmp(extension(path), "wav") == 0) ? load_wav(d, len, out) : load_waldorf(d, len, out);
    free(d);

    if (ok && (out->name[0] == '\0')) {
        const char * slash = strrchr(path, '/');

        snprintf(out->name, sizeof(out->name), "%s", (slash != NULL) ? slash + 1 : path);
        char *       dot   = strrchr(out->name, '.');

        if (dot != NULL) {
            *dot = '\0';
        }
    }
    return ok;
}

void ti_user_cycles_free(tTiUserCycles * c) {
    free(c->samples);
    c->samples = NULL;
    c->frames  = 0;
}
