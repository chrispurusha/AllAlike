/*
 * TiAlike - the parameters and the panel pages.
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
// Notes: Docs/code-notes/tiParams.md - "// notes §k" refers there.

#include "tiParams.h"
#include "tiVoices.h"

#define LIST    SYNTHLIB_PARAM_LIST

// notes §1: the panel's 0..127 is each dial's plain range, as on the instrument
const tSynthLibParam     gTiParams[kTiParamCount] = {
    {kTiParamVoiceLimit,  "Voice Limit",     "Voices", eSynthLibUnitGeneric, 1.0, TI_MAX_VOICES, 15.0 / (TI_MAX_VOICES - 1), TI_MAX_VOICES - 1, SYNTHLIB_MIDI_NONE,    0},
    {kTiParamVolume,      "Volume",          "Volume", eSynthLibUnitGeneric, 0.0,         127.0,              100.0 / 127.0,               127, SYNTHLIB_MIDI_NONE,    0},
    {kTiParamOscShape,    "Osc Shape",       "Shape",  eSynthLibUnitIndexed, 0.0,           2.0,                        0.0,                 2, SYNTHLIB_MIDI_NONE, LIST},
    {kTiParamPulseWidth,  "Osc Pulse Width", "PW",     eSynthLibUnitGeneric, 0.0,         127.0,               64.0 / 127.0,               127, SYNTHLIB_MIDI_NONE,    0},
    {kTiParamCutoff,      "Cutoff",          "Cutoff", eSynthLibUnitGeneric, 0.0,         127.0,                        1.0,               127, SYNTHLIB_MIDI_NONE,    0},
    {kTiParamResonance,   "Resonance",       "Reso",   eSynthLibUnitGeneric, 0.0,         127.0,                        0.0,               127, SYNTHLIB_MIDI_NONE,    0},
    {kTiParamFilterMode,  "Filter Mode",     "Mode",   eSynthLibUnitIndexed, 0.0,           7.0,                        0.0,                 7, SYNTHLIB_MIDI_NONE, LIST},
    {kTiParamCutoff2,     "Cutoff 2",        "Cut 2",  eSynthLibUnitGeneric, 0.0,         127.0,               64.0 / 127.0,               127, SYNTHLIB_MIDI_NONE,    0},
    {kTiParamResonance2,  "Resonance 2",     "Reso 2", eSynthLibUnitGeneric, 0.0,         127.0,                        0.0,               127, SYNTHLIB_MIDI_NONE,    0},
    {kTiParamFilter2Mode, "Filter 2 Mode",   "Mode 2", eSynthLibUnitIndexed, 0.0,           3.0,                        0.0,                 3, SYNTHLIB_MIDI_NONE, LIST},
    {kTiParamRouting,     "Filter Routing",  "Route",  eSynthLibUnitIndexed, 0.0,           3.0,                        0.0,                 3, SYNTHLIB_MIDI_NONE, LIST},
    {kTiParamBalance,     "Filter Balance",  "Bal",    eSynthLibUnitGeneric, 0.0,         127.0,               64.0 / 127.0,               127, SYNTHLIB_MIDI_NONE,    0},
    {kTiParamFilterLink,  "Filter Link",     "Link",   eSynthLibUnitIndexed, 0.0,           1.0,                        1.0,                 1, SYNTHLIB_MIDI_NONE, LIST},
};

static const char *const kShapes[]                = {"SAW", "PULSE", "TRI"};
static const char *const kFilters[]               = {"LP", "HP", "BP", "BS", "A1", "A2", "A3", "A4"};
static const char *const kFilters2[]              = {"LP", "HP", "BP", "BS"};
static const char *const kRoutings[]              = {"SER4", "SER6", "PAR4", "SPLIT"};
static const char *const kOffOn[]                 = {"OFF", "ON"};

static const tTiControl  kOsc[]                   = {
    {kTiParamOscShape,   "Shape", eTiControlChoice,   2, 0.0, kShapes},
    {kTiParamPulseWidth, "Width", eTiControlDial,   127, 0.0, NULL   },
};
static const tTiControl  kFilter[]                = {
    {kTiParamCutoff,     "Cutoff", eTiControlDial,   127, 0.0, NULL    },
    {kTiParamResonance,  "Reso",   eTiControlDial,   127, 0.0, NULL    },
    {kTiParamFilterMode, "Mode",   eTiControlChoice,   7, 0.0, kFilters},
};
static const tTiControl  kFilter2[]               = {
    {kTiParamCutoff2,     "Cutoff", eTiControlDial,   127, 0.0, NULL     },
    {kTiParamResonance2,  "Reso",   eTiControlDial,   127, 0.0, NULL     },
    {kTiParamFilter2Mode, "Mode",   eTiControlChoice,   3, 0.0, kFilters2},
};
static const tTiControl  kRouting[]               = {
    {kTiParamRouting,    "Route",   eTiControlChoice,   3, 0.0, kRoutings},
    {kTiParamBalance,    "Balance", eTiControlDial,   127, 0.0, NULL     },
    {kTiParamFilterLink, "Link",    eTiControlChoice,   1, 0.0, kOffOn   },
};
static const tTiControl  kAmp[]                   = {
    {kTiParamVolume, "Volume", eTiControlDial, 127, 0.0, NULL},
};
static const tTiControl  kVoices[]                = {
    {kTiParamVoiceLimit, "Limit", eTiControlDial, TI_MAX_VOICES - 1, 1.0, NULL},
};

// notes §2: Pulse2Alike's page names and section colours, a page only once something on it is built
static const tTiSection  kOscillatorsPage[]       = {
    {"OSC 1", {0.30, 0.55, 0.85}, kOsc, 2},
};
static const tTiSection  kFilterPage[]            = {
    {"FILTER 1", {0.90, 0.55, 0.20}, kFilter,  3},
    {"FILTER 2", {0.90, 0.55, 0.20}, kFilter2, 3},
    {"ROUTING",  {0.90, 0.55, 0.20}, kRouting, 3},
};
static const tTiSection  kAmpPage[]               = {
    {"MASTER", {0.65, 0.65, 0.65}, kAmp, 1},
};
static const tTiSection  kGlobalPage[]            = {
    {"PLUG-IN", {0.65, 0.65, 0.65}, kVoices, 1},
};

const tTiPage            gTiPages[]               = {
    {"OSCILLATORS", kOscillatorsPage, 1},
    {"FILTER",      kFilterPage,      3},
    {"AMP",         kAmpPage,         1},
    {"GLOBAL",      kGlobalPage,      1},
};
const uint32_t           gTiPageCount             = sizeof(gTiPages) / sizeof(gTiPages[0]);
