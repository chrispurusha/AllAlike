/*
 * Sn2Alike - the parameters and the panel pages.
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
// Notes: Docs/code-notes/sn2Params.md - "// notes §k" refers there.

#include "sn2Params.h"
#include "sn2Voices.h"

#define LIST    SYNTHLIB_PARAM_LIST

// notes §1: the panel's 0..127 is each dial's plain range, as on the instrument
const tSynthLibParam     gSn2Params[kSn2ParamCount] = {
    {kSn2ParamVoiceLimit,     "Voice Limit",      "Voices",  eSynthLibUnitGeneric,   1.0, SN2_MAX_VOICES, 15.0 / (SN2_MAX_VOICES - 1), SN2_MAX_VOICES - 1, SYNTHLIB_MIDI_NONE,                                                                       0},
    {kSn2ParamVolume,         "Volume",           "Volume",  eSynthLibUnitGeneric,   0.0,          127.0,               100.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                       0},
    {kSn2ParamOscShape,       "Osc Shape",        "Shape",   eSynthLibUnitIndexed,   0.0,            2.0,                         0.0,                  2, SYNTHLIB_MIDI_NONE, LIST                                                                   },
    {kSn2ParamPulseWidth,     "Osc Pulse Width",  "PW",      eSynthLibUnitGeneric,   0.0,          127.0,                64.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                       0},
    {kSn2ParamCutoff,         "Cutoff",           "Cutoff",  eSynthLibUnitGeneric,   0.0,          127.0,                         1.0,                127, SYNTHLIB_MIDI_NONE,                                                                       0},
    {kSn2ParamResonance,      "Resonance",        "Reso",    eSynthLibUnitGeneric,   0.0,          127.0,                         0.0,                127, SYNTHLIB_MIDI_NONE,                                                                       0},
    {kSn2ParamFilterType,     "Filter Type",      "Type",    eSynthLibUnitIndexed,   0.0,           11.0,                  3.0 / 11.0,                 11, SYNTHLIB_MIDI_NONE, LIST                                                                   },
    {kSn2ParamFilterSlope,    "Filter Slope",     "Slope",   eSynthLibUnitIndexed,   0.0,            2.0,                         0.0,                  2, SYNTHLIB_MIDI_NONE, LIST                                                                   },
    {kSn2ParamAmpAttack,      "Amp Attack",       "Attack",  eSynthLibUnitGeneric,   0.0,          127.0,                         0.0,                127, SYNTHLIB_MIDI_NONE,                                                                       0},
    {kSn2ParamAmpDecay,       "Amp Decay",        "Decay",   eSynthLibUnitGeneric,   0.0,          127.0,                         1.0,                127, SYNTHLIB_MIDI_NONE,                                                                       0},
    {kSn2ParamAmpSustain,     "Amp Sustain",      "Sustain", eSynthLibUnitGeneric,   0.0,          127.0,                         1.0,                127, SYNTHLIB_MIDI_NONE,                                                                       0},
    {kSn2ParamAmpSustainTime, "Amp Sustain Time", "Time",    eSynthLibUnitGeneric, -64.0,           63.0,                64.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                       0},
    {kSn2ParamAmpRelease,     "Amp Release",      "Release", eSynthLibUnitGeneric,   0.0,          127.0,                 4.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                       0},
    {kSn2ParamProgram,        "Program",          "Program", eSynthLibUnitGeneric,   0.0,          127.0,                    0.000000,                127, SYNTHLIB_MIDI_NONE, SYNTHLIB_PARAM_PROGRAM | SYNTHLIB_PARAM_HIDDEN | SYNTHLIB_PARAM_NO_SAVE},
    {kSn2ParamBank,           "Bank Select",      "Bank",    eSynthLibUnitGeneric,   0.0,          127.0,                    0.000000,                127,                 32, SYNTHLIB_PARAM_HIDDEN | SYNTHLIB_PARAM_NO_SAVE                         },
};

static const char *const kShapes[]                  = {"SAW", "PULSE", "TRI"};
static const char *const kFilters[]                 = {"LPF", "BPF", "HPF", "RLP", "RBP", "RHP", "L+L", "H+L", "H+H", "NOT", "L/L", "B/B"};
static const char *const kSlopes[]                  = {"12", "18", "24"};

static const tSn2Control kOsc[]                     = {
    {kSn2ParamOscShape,   "Shape", eTiControlChoice,   2, 0.0, kShapes},
    {kSn2ParamPulseWidth, "Width", eTiControlDial,   127, 0.0, NULL   },
};
static const tSn2Control kFilter[]                  = {
    {kSn2ParamCutoff,      "Cutoff", eTiControlDial,   127, 0.0, NULL    },
    {kSn2ParamResonance,   "Reso",   eTiControlDial,   127, 0.0, NULL    },
    {kSn2ParamFilterType,  "Type",   eTiControlChoice,  11, 0.0, kFilters},
    {kSn2ParamFilterSlope, "Slope",  eTiControlChoice,   2, 0.0, kSlopes },
};
static const tSn2Control kAmpEnv[]                  = {
    {kSn2ParamAmpAttack,      "Attack",  eTiControlDial, 127,   0.0, NULL},
    {kSn2ParamAmpDecay,       "Decay",   eTiControlDial, 127,   0.0, NULL},
    {kSn2ParamAmpSustain,     "Sustain", eTiControlDial, 127,   0.0, NULL},
    {kSn2ParamAmpSustainTime, "Time",    eTiControlDial, 127, -64.0, NULL},
    {kSn2ParamAmpRelease,     "Release", eTiControlDial, 127,   0.0, NULL},
};
static const tSn2Control kAmp[]                     = {
    {kSn2ParamVolume, "Volume", eTiControlDial, 127, 0.0, NULL},
};
static const tSn2Control kVoices[]                  = {
    {kSn2ParamVoiceLimit, "Limit", eTiControlDial, SN2_MAX_VOICES - 1, 1.0, NULL},
};
static const tSn2Section kOscillatorsPage[]         = {
    {"OSC 1", {0.30, 0.55, 0.85}, kOsc, 2},
};
static const tSn2Section kFilterPage[]              = {
    {"FILTER", {0.90, 0.55, 0.20}, kFilter, 4},
};
static const tSn2Section kAmpPage[]                 = {
    {"MASTER", {0.65, 0.65, 0.65}, kAmp, 1},
};
static const tSn2Section kEnvelopesPage[]           = {
    {"AMP ENV", {0.85, 0.75, 0.25}, kAmpEnv, 5},
};
static const tSn2Section kGlobalPage[]              = {
    {"PLUG-IN", {0.65, 0.65, 0.65}, kVoices, 1},
};

const tSn2Page           gSn2Pages[]                = {
    {"OSCILLATORS", kOscillatorsPage, 1},
    {"FILTER",      kFilterPage,      1},
    {"AMP",         kAmpPage,         1},
    {"ENVELOPES",   kEnvelopesPage,   1},
    {"GLOBAL",      kGlobalPage,      1},
};
const uint32_t           gSn2PageCount              = sizeof(gSn2Pages) / sizeof(gSn2Pages[0]);
