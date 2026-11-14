/*
 * UltraAlike - a sampler (a placeholder on a sample player's engine), as a VST3 and Audio Unit instrument.
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
// Notes: Docs/code-notes/uaSample.md - "// notes §k" refers there.

#include <math.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "uaSample.h"

#define MAX_FILE_BYTES    (1024u * 1024u * 1024u)   // notes §2

static _Atomic uint32_t gSerial;

typedef enum {
    eIntLE = 0,
    eIntBE,
    eUInt8,
    eFloatLE,
    eFloatBE
} tEncoding;

typedef struct {
    uint32_t        channels;
    uint32_t        bits;
    double          rate;
    tEncoding       encoding;
    const uint8_t * data;
    size_t          bytes;
} tFormat;

static uint32_t le16(const uint8_t * p) {
    return (uint32_t)p[0] | ((uint32_t)p[1] << 8);
}

static uint32_t le32(const uint8_t * p) {
    return (uint32_t)p[0] | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
}

static uint32_t be16(const uint8_t * p) {
    return ((uint32_t)p[0] << 8) | (uint32_t)p[1];
}

static uint32_t be32(const uint8_t * p) {
    return ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) | ((uint32_t)p[2] << 8) | (uint32_t)p[3];
}

// notes §3: AIFF's sample rate is an 80-bit IEEE extended
static double extended80(const uint8_t * p) {
    int      exponent = (int)(((p[0] & 0x7F) << 8) | p[1]);
    uint64_t mantissa = 0;

    for (int i = 0; i < 8; i++) {
        mantissa = (mantissa << 8) | p[2 + i];
    }

    if ((exponent == 0) && (mantissa == 0)) {
        return 0.0;
    }
    double   value    = ldexp((double)mantissa, exponent - 16383 - 63);

    return (p[0] & 0x80) ? -value : value;
}

static bool parse_wav(const uint8_t * f, size_t n, tFormat * fmt, char * error, size_t errorLength) {
    bool   haveFmt = false;
    size_t at      = 12;

    while (at + 8 <= n) {
        const uint8_t * id   = f + at;
        size_t          size = le32(f + at + 4);
        const uint8_t * body = f + at + 8;

        if (size > n - at - 8) {
            size = n - at - 8;   // notes §4: a truncated last chunk is read as far as it goes
        }

        if ((memcmp(id, "fmt ", 4) == 0) && (size >= 16)) {
            uint32_t tag = le16(body);

            fmt->channels = le16(body + 2);
            fmt->rate     = (double)le32(body + 4);
            fmt->bits     = le16(body + 14);

            if ((tag == 0xFFFE) && (size >= 26)) {
                tag = le16(body + 24);   // the sub-format GUID's first two bytes
            }

            if (tag == 1) {
                fmt->encoding = (fmt->bits == 8) ? eUInt8 : eIntLE;
            } else if (tag == 3) {
                fmt->encoding = eFloatLE;
            } else {
                snprintf(error, errorLength, "WAV format %u is not supported (PCM and float only)", tag);
                return false;
            }
            haveFmt       = true;
        } else if (memcmp(id, "data", 4) == 0) {
            fmt->data  = body;
            fmt->bytes = size;
        }
        at += 8 + size + (size & 1);
    }

    if (!haveFmt || (fmt->data == NULL)) {
        snprintf(error, errorLength, "WAV file has no %s chunk", haveFmt ? "data" : "fmt");
        return false;
    }
    return true;
}

static bool parse_aiff(const uint8_t * f, size_t n, bool aifc, tFormat * fmt, char * error, size_t errorLength) {
    bool   haveComm = false;
    size_t at       = 12;

    while (at + 8 <= n) {
        const uint8_t * id   = f + at;
        size_t          size = be32(f + at + 4);
        const uint8_t * body = f + at + 8;

        if (size > n - at - 8) {
            size = n - at - 8;
        }

        if ((memcmp(id, "COMM", 4) == 0) && (size >= 18)) {
            fmt->channels = be16(body);
            fmt->bits     = be16(body + 6);
            fmt->rate     = extended80(body + 8);
            fmt->encoding = eIntBE;

            if (aifc && (size >= 22)) {
                const uint8_t * type = body + 18;

                if ((memcmp(type, "sowt", 4) == 0)) {
                    fmt->encoding = eIntLE;
                } else if ((memcmp(type, "fl32", 4) == 0) || (memcmp(type, "FL32", 4) == 0)) {
                    fmt->encoding = eFloatBE;
                    fmt->bits     = 32;
                } else if ((memcmp(type, "fl64", 4) == 0) || (memcmp(type, "FL64", 4) == 0)) {
                    fmt->encoding = eFloatBE;
                    fmt->bits     = 64;
                } else if (memcmp(type, "NONE", 4) != 0) {
                    snprintf(error, errorLength, "AIFC compression '%.4s' is not supported", type);
                    return false;
                }
            }
            haveComm      = true;
        } else if ((memcmp(id, "SSND", 4) == 0) && (size >= 8)) {
            uint32_t offset = be32(body);

            if (offset <= size - 8) {
                fmt->data  = body + 8 + offset;
                fmt->bytes = size - 8 - offset;
            }
        }
        at += 8 + size + (size & 1);
    }

    if (!haveComm || (fmt->data == NULL)) {
        snprintf(error, errorLength, "AIFF file has no %s chunk", haveComm ? "SSND" : "COMM");
        return false;
    }
    return true;
}

// One sample of one channel, as a float in -1..1
static float read_value(const uint8_t * p, const tFormat * fmt) {
    switch (fmt->encoding) {
        case eUInt8:
            return ((float)p[0] - 128.0f) / 128.0f;

        case eIntLE:
        case eIntBE:
        {
            uint32_t bytes = fmt->bits / 8;
            int32_t  v     = 0;

            for (uint32_t i = 0; i < bytes; i++) {
                uint32_t b = (fmt->encoding == eIntLE) ? p[bytes - 1 - i] : p[i];

                v = (int32_t)(((uint32_t)v << 8) | b);
            }

            v = (int32_t)((uint32_t)v << (32 - (bytes * 8)));   // sign in the top bit
            return (float)((double)v / 2147483648.0);
        }

        case eFloatLE:
        case eFloatBE:
        {
            uint8_t  b[8];
            uint32_t bytes = fmt->bits / 8;

            for (uint32_t i = 0; i < bytes; i++) {
                b[i] = (fmt->encoding == eFloatLE) ? p[i] : p[bytes - 1 - i];   // to the Mac's little-endian
            }

            if (bytes == 8) {
                double d;

                memcpy(&d, b, 8);
                return (float)d;
            }
            float    f;

            memcpy(&f, b, 4);
            return f;
        }
    }
    return 0.0f;
}

static uint8_t * read_file(const char * path, size_t * length, char * error, size_t errorLength) {
    FILE *    file = fopen(path, "rb");
    uint8_t * data = NULL;
    long      size;

    if (file == NULL) {
        snprintf(error, errorLength, "Cannot open %s", path);
        return NULL;
    }

    if ((fseek(file, 0, SEEK_END) == 0) && ((size = ftell(file)) > 0) && ((unsigned long)size <= MAX_FILE_BYTES) && (fseek(file, 0, SEEK_SET) == 0)) {
        data = (uint8_t *)malloc((size_t)size);

        if ((data != NULL) && (fread(data, 1, (size_t)size, file) == (size_t)size)) {
            *length = (size_t)size;
        } else {
            free(data);
            data = NULL;
            snprintf(error, errorLength, "Cannot read %s", path);
        }
    } else {
        snprintf(error, errorLength, "%s is empty or larger than 1 GB", path);
    }
    fclose(file);
    return data;
}

tUaSample * ua_sample_load(const char * path, char * error, size_t errorLength) {
    size_t      n          = 0;
    uint8_t *   f          = read_file(path, &n, error, errorLength);
    tFormat     fmt        = {0};
    bool        ok         = false;
    tUaSample * s          = NULL;

    if (f == NULL) {
        return NULL;
    }

    if ((n >= 12) && (memcmp(f, "RIFF", 4) == 0) && (memcmp(f + 8, "WAVE", 4) == 0)) {
        ok = parse_wav(f, n, &fmt, error, errorLength);
    } else if ((n >= 12) && (memcmp(f, "FORM", 4) == 0) && ((memcmp(f + 8, "AIFF", 4) == 0) || (memcmp(f + 8, "AIFC", 4) == 0))) {
        ok = parse_aiff(f, n, memcmp(f + 8, "AIFC", 4) == 0, &fmt, error, errorLength);
    } else {
        snprintf(error, errorLength, "Not a WAV or AIFF file");
    }
    bool        validBits  = ((fmt.encoding == eFloatLE) || (fmt.encoding == eFloatBE)) ? ((fmt.bits == 32) || (fmt.bits == 64))
                                                                                : ((fmt.bits == 8) || (fmt.bits == 16) || (fmt.bits == 24) || (fmt.bits == 32));

    if (ok && (!validBits || (fmt.channels == 0) || (fmt.rate < 1000.0) || (fmt.rate > 768000.0))) {
        snprintf(error, errorLength, "Unsupported: %u channels, %u bits, %.0f Hz", fmt.channels, fmt.bits, fmt.rate);
        ok = false;
    }
    uint32_t    frameBytes = ok ? (fmt.channels * (fmt.bits / 8)) : 0;
    uint32_t    frames     = ok ? (uint32_t)(fmt.bytes / frameBytes) : 0;

    if (ok && (frames == 0)) {
        snprintf(error, errorLength, "The file holds no audio");
        ok = false;
    }

    if (ok) {
        s = (tUaSample *)calloc(1, sizeof(tUaSample));
    }

    if (s != NULL) {
        uint32_t keep = (fmt.channels >= 2) ? 2 : 1;   // notes §5: a file of more channels plays its first two

        s->serial   = atomic_fetch_add(&gSerial, 1) + 1;
        s->frames   = frames;
        s->channels = keep;
        s->rate     = fmt.rate;
        s->data[0]  = (float *)malloc((size_t)frames * sizeof(float));
        s->data[1]  = (keep == 2) ? (float *)malloc((size_t)frames * sizeof(float)) : s->data[0];

        if ((s->data[0] == NULL) || (s->data[1] == NULL)) {
            ua_sample_free(s);
            s = NULL;
            snprintf(error, errorLength, "Out of memory");
        } else {
            for (uint32_t i = 0; i < frames; i++) {
                const uint8_t * frame = fmt.data + ((size_t)i * frameBytes);

                for (uint32_t c = 0; c < keep; c++) {
                    s->data[c][i] = read_value(frame + (c * (fmt.bits / 8)), &fmt);
                }
            }

            snprintf(s->path, sizeof(s->path), "%s", path);
        }
    }
    free(f);
    return s;
}

void ua_sample_free(tUaSample * sample) {
    if (sample != NULL) {
        if (sample->data[1] != sample->data[0]) {
            free(sample->data[1]);
        }
        free(sample->data[0]);
        free(sample);
    }
}

bool ua_sample_is_audio_file(const char * name) {
    static const char *const kExtensions[] = {".wav", ".wave", ".aif", ".aiff", ".aifc"};
    const char *             dot           = strrchr(name, '.');

    if ((dot == NULL) || (name[0] == '.')) {
        return false;
    }

    for (size_t i = 0; i < sizeof(kExtensions) / sizeof(kExtensions[0]); i++) {
        if (strcasecmp(dot, kExtensions[i]) == 0) {
            return true;
        }
    }

    return false;
}

static inline float mixed(const float * left, const float * right, uint32_t i) {
    return left[i] + right[i];
}

// notes §6: outward from the frame, the first sign change either side; of its two frames, the one nearer zero
uint32_t ua_sample_zero_crossing(const tUaSample * sample, uint32_t frame, uint32_t radius) {
    return (sample == NULL) ? frame : ua_zero_crossing(sample->data[0], sample->data[1], sample->frames, frame, radius);
}

uint32_t ua_zero_crossing(const float * left, const float * right, uint32_t frames, uint32_t frame, uint32_t radius) {
    if (frame + 1 >= frames) {
        return frame;
    }

    if (mixed(left, right, frame) == 0.0f) {
        return frame;
    }

    for (uint32_t d = 0; d <= radius; d++) {
        for (int side = 0; side < 2; side++) {
            int64_t a = (side == 0) ? (int64_t)frame + d : (int64_t)frame - d - 1;

            if ((a < 0) || (a + 1 >= (int64_t)frames)) {
                continue;
            }
            float   x = mixed(left, right, (uint32_t)a), y = mixed(left, right, (uint32_t)a + 1);

            if ((x <= 0.0f) != (y <= 0.0f)) {
                return (fabsf(x) <= fabsf(y)) ? (uint32_t)a : (uint32_t)a + 1;
            }
        }
    }

    return frame;
}
