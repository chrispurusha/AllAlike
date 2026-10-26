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
    {kTiParamVoiceLimit, "Voice Limit",     "Voices", eSynthLibUnitGeneric, 1.0, TI_MAX_VOICES, 15.0 / (TI_MAX_VOICES - 1), TI_MAX_VOICES - 1, SYNTHLIB_MIDI_NONE,    0},
    {kTiParamVolume,     "Volume",          "Volume", eSynthLibUnitGeneric, 0.0,         127.0,              100.0 / 127.0,               127, SYNTHLIB_MIDI_NONE,    0},
    {kTiParamOscShape,   "Osc Shape",       "Shape",  eSynthLibUnitIndexed, 0.0,           2.0,                        0.0,                 2, SYNTHLIB_MIDI_NONE, LIST},
    {kTiParamPulseWidth, "Osc Pulse Width", "PW",     eSynthLibUnitGeneric, 0.0,         127.0,               64.0 / 127.0,               127, SYNTHLIB_MIDI_NONE,    0},
    {kTiParamCutoff,     "Cutoff",          "Cutoff", eSynthLibUnitGeneric, 0.0,         127.0,                        1.0,               127, SYNTHLIB_MIDI_NONE,    0},
    {kTiParamResonance,  "Resonance",       "Reso",   eSynthLibUnitGeneric, 0.0,         127.0,                        0.0,               127, SYNTHLIB_MIDI_NONE,    0},
    {kTiParamFilterMode, "Filter Mode",     "Mode",   eSynthLibUnitIndexed, 0.0,           7.0,                        0.0,                 7, SYNTHLIB_MIDI_NONE, LIST},
};

static const char *const kShapes[]                = {"SAW", "PULSE", "TRI"};
static const char *const kFilters[]               = {"LP", "HP", "BP", "BS", "A1", "A2", "A3", "A4"};

static const tTiControl  kOsc[]                   = {
    {kTiParamOscShape,   "Shape", eTiControlChoice,   2, 0.0, kShapes},
    {kTiParamPulseWidth, "Width", eTiControlDial,   127, 0.0, NULL   },
};
static const tTiControl  kFilter[]                = {
    {kTiParamCutoff,     "Cutoff", eTiControlDial,   127, 0.0, NULL    },
    {kTiParamResonance,  "Reso",   eTiControlDial,   127, 0.0, NULL    },
    {kTiParamFilterMode, "Mode",   eTiControlChoice,   7, 0.0, kFilters},
};
static const tTiControl  kAmp[]                   = {
    {kTiParamVolume, "Volume", eTiControlDial, 127, 0.0, NULL},
};
static const tTiControl  kVoices[]                = {
    {kTiParamVoiceLimit, "Limit", eTiControlDial, TI_MAX_VOICES - 1, 1.0, NULL},
};

static const tTiSection  kSoundPage[]             = {
    {"OSCILLATOR", {0.85, 0.65, 0.30}, kOsc,    2},
    {"FILTER",     {0.40, 0.70, 0.90}, kFilter, 3},
    {"AMPLIFIER",  {0.60, 0.80, 0.45}, kAmp,    1},
};
static const tTiSection  kGlobalPage[]            = {
    {"VOICES", {0.75, 0.75, 0.75}, kVoices, 1},
};

const tTiPage            gTiPages[]               = {
    {"SOUND",  kSoundPage,  3},
    {"GLOBAL", kGlobalPage, 1},
};
const uint32_t           gTiPageCount             = 2;
