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

#define SOUNDS_FOLDER       "Library/Mobile Documents/com~apple~CloudDocs/Patches/Nord Lead 32"  // under $HOME
#define PRESETS_FOLDER      "Library/Application Support/Nl3Alike"                               // beside prefs.txt
#define PRESETS_FILE        "Presets.syx"

// notes §1: where things sit in the two messages
#define MAIN_LENGTH         (297)
#define EXT_LENGTH          (99)
#define EXT                 (MAIN_LENGTH)
#define DEVICE_AT           (6)
#define COMMAND_AT          (7)
#define BANK_AT             (8)
#define PROGRAM_AT          (9)
#define FIELDS_AT           (10)
#define NAME_AT             (FIELDS_AT)
#define MAIN(field)     (FIELDS_AT + (field))
#define EXTRA(field)    (EXT + FIELDS_AT + (field))
#define FADE_AT             EXTRA(3)

#define MAIN_DUMP           (0x02)
#define EXT_DUMP            (0x1F)
#define DEVICE              (0x01)
#define OSC_SQUARE          (0)
#define OSC_SAW             (1)
#define OSC_DOUBLE_SAW      (4)

#define MAX_FILE_BYTES      (16 * 1024 * 1024)
#define MAX_FOLDER_FILES    (4096)

static const uint8_t kHeader[6] = {0xF0, 0x00, 0x20, 0x29, 0x01, 0x24};

// notes §3: the bits of a byte a parameter takes; the rest of the byte is kept when it is written back
static const struct {
    uint32_t paramId;
    uint16_t at;
    uint8_t  mask;
    int8_t   stored;  // notes §7: the byte's value at parameter value 0
}                    kCarried[] = {
    {kNl3ParamCutoff,         MAIN(211), 0x7F,   0},
    {kNl3ParamResonance,      MAIN(221), 0x7F,   0},
    {kNl3ParamFilterType,     MAIN(207), 0x7F,   3},
    {kNl3ParamFilterSlope,    MAIN(208), 0x7F,   0},
    {kNl3ParamOscSoften,      MAIN(35),  0x7F,   0},
    {kNl3ParamOscLevel,       MAIN(124), 0x7F,   0},
    {kNl3ParamVolume,         MAIN(236), 0x7F,   0},
    {kNl3ParamOsc2Soften,     MAIN(69),  0x7F,   0},
    {kNl3ParamOsc2Width,      MAIN(75),  0x7F,   0},
    {kNl3ParamOsc2Level,      MAIN(130), 0x7F,   0},
    {kNl3ParamOsc3Soften,     MAIN(103), 0x7F,   0},
    {kNl3ParamOsc3Width,      MAIN(109), 0x7F,   0},
    {kNl3ParamOsc3Level,      MAIN(136), 0x7F,   0},
    {kNl3ParamOsc1Octave,     MAIN(23),  0x7F,  30},
    {kNl3ParamOsc1Semitone,   MAIN(24),  0x7F,  45},
    {kNl3ParamOsc1Fine,       MAIN(25),  0x7F,   0},
    {kNl3ParamOsc2Octave,     MAIN(57),  0x7F,  35},
    {kNl3ParamOsc2Semitone,   MAIN(58),  0x7F,  70},
    {kNl3ParamOsc2Fine,       MAIN(59),  0x7F,   0},
    {kNl3ParamOsc3Octave,     MAIN(91),  0x7F,  40},
    {kNl3ParamOsc3Semitone,   MAIN(92),  0x7F,  95},
    {kNl3ParamOsc3Fine,       MAIN(93),  0x7F,   0},
    {kNl3ParamFilterTrack,    MAIN(220), 0x7F,   0},
    {kNl3ParamFilterDrive,    MAIN(210), 0x7F,   0},
    {kNl3ParamAmpVelocity,    MAIN(198), 0x7F,   0},
    {kNl3ParamEnv2Attack,     MAIN(188), 0x7F,   0},
    {kNl3ParamEnv2Decay,      MAIN(189), 0x7F,   0},
    {kNl3ParamEnv2Sustain,    MAIN(190), 0x7F,   0},
    {kNl3ParamEnv2Release,    MAIN(191), 0x7F,   0},
    {kNl3ParamEnv2Velocity,   MAIN(192), 0x7F,   0},
    {kNl3ParamEnv2Cutoff,     MAIN(214), 0x7F,   0},
    {kNl3ParamEnv3Attack,     MAIN(182), 0x7F,   0},
    {kNl3ParamEnv3Decay,      MAIN(183), 0x7F,   0},
    {kNl3ParamEnv3Sustain,    MAIN(184), 0x7F,   0},
    {kNl3ParamEnv3Release,    MAIN(185), 0x7F,   0},
    {kNl3ParamEnv3Velocity,   MAIN(186), 0x7F,   0},
    {kNl3ParamEnv3Cutoff,     MAIN(215), 0x7F,   0},
    {kNl3ParamNoiseLevel,     MAIN(154), 0x7F,   0},
    {kNl3ParamNoiseSoften,    MAIN(201), 0x7F,   0},
    {kNl3ParamRing13,         MAIN(142), 0x7F,   0},
    {kNl3ParamRing23,         MAIN(148), 0x7F,   0},
    {kNl3ParamFilterQ,        MAIN(230), 0x7F,   0},
    {kNl3ParamEffectsDry,     MAIN(260), 0x7F,   0},
    {kNl3ParamChorusSend,     MAIN(270), 0x7F,   0},
    {kNl3ParamChorusSpeed,    MAIN(272), 0x7F,   0},
    {kNl3ParamChorusDepth,    MAIN(273), 0x7F,   0},
    {kNl3ParamChorusFeedback, MAIN(274), 0x7F,   0},
    {kNl3ParamChorusType,     MAIN(275), 0x03,   0},
    {kNl3ParamDelaySend,      MAIN(276), 0x7F,   0},
    {kNl3ParamDelayTime,      MAIN(278), 0x7F,   0},
    {kNl3ParamDelayFeedback,  MAIN(279), 0x7F,   0},
    {kNl3ParamDelayDamp,      MAIN(280), 0x7F,   0},
    {kNl3ParamDelayRatio,     MAIN(281), 0x7F,  96},
    {kNl3ParamDelayWidth,     MAIN(282), 0x7F,   0},
    {kNl3ParamDelaySync,      MAIN(283), 0x7F,   0},
    {kNl3ParamGlideTime,      MAIN(231), 0x7F,   0},
    {kNl3ParamGlideType,      MAIN(232), 0x7F,  42},
    {kNl3ParamGlideStyle,     MAIN(233), 0x7F, 120},
    {kNl3ParamGlideCurve,     MAIN(234), 0x7F, 122},
    {kNl3ParamReverbSend,     MAIN(265), 0x7F,   0},
    {kNl3ParamReverbDecay,    MAIN(267), 0x7F,   0},
    {kNl3ParamReverbDamp,     MAIN(268), 0x7F,   0},
    {kNl3ParamOscTrigger,     MAIN(237), 0x01,   0},
    {kNl3ParamDistLevel,      MAIN(254), 0x7F,   0},
    {kNl3ParamDistComp,       EXTRA(59), 0x7F,   0},
    {kNl3ParamDistOutput,     EXTRA(58), 0x7F,   0},
    {kNl3ParamEqBass,         MAIN(256), 0x7F,   0},
    {kNl3ParamEqTreble,       MAIN(257), 0x7F,   0},
    {kNl3ParamPulseWidth,     MAIN(41),  0x7F,   0},
    {kNl3ParamAmpAttack,      MAIN(194), 0x7F,   0},
    {kNl3ParamAmpDecay,       MAIN(195), 0x7F,   0},
    {kNl3ParamAmpSustain,     MAIN(196), 0x7F,   0},
    {kNl3ParamAmpRelease,     MAIN(197), 0x7F,   0},
    {kNl3ParamAmpSustainTime, EXTRA(16), 0x7F,   0},
    {kNl3ParamOsc1BendRange,  MAIN(26),  0x7F,  52},
    {kNl3ParamOsc2BendRange,  MAIN(60),  0x7F,  52},
    {kNl3ParamOsc3BendRange,  MAIN(94),  0x7F,  52},
    {kNl3ParamOsc1WheelPitch, MAIN(28),  0x7F,   0},
    {kNl3ParamOsc2WheelPitch, MAIN(62),  0x7F,   0},
    {kNl3ParamOsc3WheelPitch, MAIN(96),  0x7F,   0},
    {kNl3ParamOsc1WheelLevel, MAIN(125), 0x7F,   0},
    {kNl3ParamOsc2WheelLevel, MAIN(131), 0x7F,   0},
    {kNl3ParamOsc3WheelLevel, MAIN(137), 0x7F,   0},
    {kNl3ParamWheelCutoff,    MAIN(212), 0x7F,   0},
    {kNl3ParamWheelResonance, MAIN(222), 0x7F,   0},
    {kNl3ParamPan,            MAIN(261), 0x7F,   0},
    {kNl3ParamPanSpeed,       MAIN(262), 0x7F,   0},
    {kNl3ParamPanDepth,       MAIN(263), 0x7F,   0},
    {kNl3ParamLfo1Shape,      MAIN(161), 0x7F,  16},
    {kNl3ParamLfo1Speed,      MAIN(163), 0x7F,   0},
    {kNl3ParamLfo1Range,      MAIN(180), 0x7F,  28},
    {kNl3ParamLfo1Delay,      MAIN(160), 0x7F,   0},
    {kNl3ParamLfo1Offset,     MAIN(162), 0x7F,   0},
    {kNl3ParamLfo1Trigger,    MAIN(168), 0x7F,  24},
    {kNl3ParamLfo1Cutoff,     MAIN(216), 0x7F,   0},
    {kNl3ParamLfo2Shape,      MAIN(171), 0x7F,  20},
    {kNl3ParamLfo2Speed,      MAIN(173), 0x7F,   0},
    {kNl3ParamLfo2Range,      MAIN(181), 0x7F,  31},
    {kNl3ParamLfo2Delay,      MAIN(170), 0x7F,   0},
    {kNl3ParamLfo2Offset,     MAIN(172), 0x7F,   0},
    {kNl3ParamLfo2Trigger,    MAIN(178), 0x7F,  26},
    {kNl3ParamLfo2Cutoff,     MAIN(217), 0x7F,   0},
    {kNl3ParamOsc1Lfo1Pitch,  MAIN(31),  0x7F,   0},
    {kNl3ParamOsc1Lfo1Width,  MAIN(45),  0x7F,   0},
    {kNl3ParamOsc1Lfo2Pitch,  MAIN(32),  0x7F,   0},
    {kNl3ParamOsc1Lfo2Width,  MAIN(46),  0x7F,   0},
    {kNl3ParamOsc2Lfo1Pitch,  MAIN(65),  0x7F,   0},
    {kNl3ParamOsc2Lfo1Width,  MAIN(79),  0x7F,   0},
    {kNl3ParamOsc2Lfo2Pitch,  MAIN(66),  0x7F,   0},
    {kNl3ParamOsc2Lfo2Width,  MAIN(80),  0x7F,   0},
    {kNl3ParamOsc3Lfo1Pitch,  MAIN(99),  0x7F,   0},
    {kNl3ParamOsc3Lfo1Width,  MAIN(113), 0x7F,   0},
    {kNl3ParamOsc3Lfo2Pitch,  MAIN(100), 0x7F,   0},
    {kNl3ParamOsc3Lfo2Width,  MAIN(114), 0x7F,   0},
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
static int add_if_single(const uint8_t * m, size_t len, uint8_t(*singles)[NL3_SINGLE_LENGTH], int count, int capacity) {
    if ((count < capacity) && is_message(m, len, MAIN_LENGTH, MAIN_DUMP)) {
        uint8_t * out = singles[count++];

        memset(out, 0, NL3_SINGLE_LENGTH);
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

// notes §8: audio inputs are not built, and play as a Saw
static double shape_from_type(int type) {
    return (type == OSC_SQUARE) ? 0.5 : ((type == OSC_DOUBLE_SAW) ? 1.0 : 0.0);
}

void nl3_patch_from_single(const uint8_t single[NL3_SINGLE_LENGTH], double normalized[kNl3ParamCount]) {
    for (size_t k = 0; k < sizeof(kCarried) / sizeof(kCarried[0]); k++) {
        int32_t steps = gNl3Params[kCarried[k].paramId].stepCount;
        int32_t value = (single[kCarried[k].at] & kCarried[k].mask) - kCarried[k].stored;

        value                           = (value < 0) ? 0 : ((value > steps) ? steps : value);
        normalized[kCarried[k].paramId] = (double)value / steps;
    }

    normalized[kNl3ParamPanEffects] = ((single[MAIN(275)] >> 4) & 1) ? 1.0 : 0.0;
    normalized[kNl3ParamLfo1Fade]   = ((single[FADE_AT] >> 3) & 1) ? 1.0 : 0.0;
    normalized[kNl3ParamLfo2Fade]   = ((single[FADE_AT] >> 4) & 1) ? 1.0 : 0.0;
    normalized[kNl3ParamPolyMode]   = ((single[MAIN(238)] & 0x03) == 1) ? 1.0 : 0.0;    // above these bits, the Unison voices
    normalized[kNl3ParamEnvTrigger] = ((single[MAIN(235)] <= 33) || (single[MAIN(235)] == 38)) ? 1.0 : 0.0;
    normalized[kNl3ParamReverbType] = (double)((single[MAIN(269)] >> 3) & 0x0F) / 15.0; // shares its byte with Early Reflections
    normalized[kNl3ParamOscShape]   = shape_from_type(single[MAIN(22)]);
    normalized[kNl3ParamOsc2Shape]  = shape_from_type(single[MAIN(56)] - 10);
    normalized[kNl3ParamOsc3Shape]  = shape_from_type(single[MAIN(90)] - 20);
}

void nl3_patch_name(const uint8_t single[NL3_SINGLE_LENGTH], char name[NL3_SINGLE_NAME_LENGTH + 1]) {
    int n = NL3_SINGLE_NAME_LENGTH;

    for (int i = 0; i < NL3_SINGLE_NAME_LENGTH; i++) {
        uint8_t c = single[NAME_AT + i];

        name[i] = ((c >= 32) && (c < 127)) ? (char)c : ' ';
    }

    while ((n > 0) && (name[n - 1] == ' ')) {
        n--;
    }
    name[n] = '\0';
}

void nl3_patch_to_single(const double normalized[kNl3ParamCount], const uint8_t * base, const char * name, uint8_t out[NL3_SINGLE_LENGTH]) {
    size_t len                                 = (name != NULL) ? strlen(name) : 0;
    double was[kNl3ParamCount];
    char   wasName[NL3_SINGLE_NAME_LENGTH + 1] = "";

    if (base != NULL) {
        nl3_patch_from_single(base, was);
        nl3_patch_name(base, wasName);
    }

    if (base != NULL) {
        memcpy(out, base, NL3_SINGLE_LENGTH);
    } else {
        memset(out, 0, NL3_SINGLE_LENGTH);
    }
    fresh_header(out, MAIN_LENGTH, MAIN_DUMP);
    fresh_header(out + EXT, EXT_LENGTH, EXT_DUMP);

    for (size_t k = 0; k < sizeof(kCarried) / sizeof(kCarried[0]); k++) {
        int32_t steps = gNl3Params[kCarried[k].paramId].stepCount;
        long    value = lround(fmin(1.0, fmax(0.0, normalized[kCarried[k].paramId])) * steps);

        // notes §4: a byte is kept as it came unless its parameter changed - it may hold more than the panel shows
        if ((base != NULL) && (value == lround(was[kCarried[k].paramId] * steps))) {
            continue;
        }
        out[kCarried[k].at] = (uint8_t)((out[kCarried[k].at] & ~kCarried[k].mask) | ((uint8_t)(value + kCarried[k].stored) & kCarried[k].mask));
    }

    long reverbType = lround(fmin(1.0, fmax(0.0, normalized[kNl3ParamReverbType])) * 15.0);

    if ((base == NULL) || (reverbType != lround(was[kNl3ParamReverbType] * 15.0))) {
        out[MAIN(269)] = (uint8_t)((out[MAIN(269)] & 0x07) | ((uint8_t)reverbType << 3));
    }
    out[MAIN(275)] = (uint8_t)((out[MAIN(275)] & ~0x10) | ((normalized[kNl3ParamPanEffects] > 0.5) ? 0x10 : 0));
    out[FADE_AT]   = (uint8_t)((out[FADE_AT] & ~0x18) | ((normalized[kNl3ParamLfo1Fade] > 0.5) ? 0x08 : 0) | ((normalized[kNl3ParamLfo2Fade] > 0.5) ? 0x10 : 0));

    if ((base == NULL) || ((normalized[kNl3ParamPolyMode] > 0.5) != (was[kNl3ParamPolyMode] > 0.5))) {
        out[MAIN(238)] = (uint8_t)(((base != NULL) ? (out[MAIN(238)] & 0x7C) : 0x40) | ((normalized[kNl3ParamPolyMode] > 0.5) ? 1 : 2));
    }

    if ((base == NULL) || ((normalized[kNl3ParamEnvTrigger] > 0.5) != (was[kNl3ParamEnvTrigger] > 0.5))) {
        out[MAIN(235)] = (normalized[kNl3ParamEnvTrigger] > 0.5) ? 32 : 34;
    }

    for (size_t i = 0; (base == NULL || strcmp(wasName, (name != NULL) ? name : "") != 0) && (i < NL3_SINGLE_NAME_LENGTH); i++) {
        char c = (i < len) ? name[i] : ' ';

        out[NAME_AT + i] = ((c >= 32) && (c < 127)) ? (uint8_t)c : ' ';
    }

    nl3_patch_address(out, 0);
}

void nl3_patch_address(uint8_t single[NL3_SINGLE_LENGTH], int number) {
    uint8_t bank    = (number > 0) ? (uint8_t)((number - 1) / NL3_BANK_SIZE) : 0;
    uint8_t program = (number > 0) ? (uint8_t)((number - 1) % NL3_BANK_SIZE) : 0;

    single[BANK_AT]          = bank;
    single[PROGRAM_AT]       = program;
    single[EXT + BANK_AT]    = bank;
    single[EXT + PROGRAM_AT] = program;
}

void nl3_patch_init_single(uint8_t out[NL3_SINGLE_LENGTH]) {
    double values[kNl3ParamCount];

    for (uint32_t id = 0; id < kNl3ParamCount; id++) {
        values[id] = gNl3Params[id].defaultNormalized;
    }

    nl3_patch_to_single(values, NULL, "Init", out);
    out[MAIN(22)] = OSC_SAW;
    out[MAIN(56)] = OSC_SAW + 10;
    out[MAIN(90)] = OSC_SAW + 20;
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
    snprintf(out, len, "%c%03d", 'A' + ((number - 1) / NL3_BANK_SIZE), (number - 1) % NL3_BANK_SIZE);   // the instrument counts from 000
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

// notes §6: in file order from A000; what is left over is Init
int nl3_presets_read_file(const char * path, uint8_t singles[NL3_PRESETS][NL3_SINGLE_LENGTH]) {
    int count = nl3_patch_read_file(path, singles, NL3_PRESETS);

    fill_and_address(singles, count);
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
