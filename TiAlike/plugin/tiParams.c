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
    {kTiParamVoiceLimit,                      "Voice Limit",             "Voices",            eSynthLibUnitGeneric,   1.0, TI_MAX_VOICES, 15.0 / (TI_MAX_VOICES - 1), TI_MAX_VOICES - 1, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamVolume,                          "Volume",                  "Volume",            eSynthLibUnitGeneric,   0.0,         127.0,              100.0 / 127.0,               127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamOscShape,                        "Osc Shape",               "Shape",             eSynthLibUnitGeneric,   0.0,         127.0,               64.0 / 127.0,               127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamPulseWidth,                      "Osc Pulse Width",         "PW",                eSynthLibUnitGeneric,   0.0,         127.0,                        0.0,               127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamCutoff,                          "Cutoff",                  "Cutoff",            eSynthLibUnitGeneric,   0.0,         127.0,                        1.0,               127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamResonance,                       "Resonance",               "Reso",              eSynthLibUnitGeneric,   0.0,         127.0,                        0.0,               127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamFilterMode,                      "Filter Mode",             "Mode",              eSynthLibUnitIndexed,   0.0,           7.0,                        0.0,                 7, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kTiParamCutoff2,                         "Cutoff 2",                "Cut 2",             eSynthLibUnitGeneric,   0.0,         127.0,               64.0 / 127.0,               127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamResonance2,                      "Resonance 2",             "Reso 2",            eSynthLibUnitGeneric,   0.0,         127.0,                        0.0,               127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamFilter2Mode,                     "Filter 2 Mode",           "Mode 2",            eSynthLibUnitIndexed,   0.0,           3.0,                        0.0,                 3, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kTiParamRouting,                         "Filter Routing",          "Route",             eSynthLibUnitIndexed,   0.0,           3.0,                        0.0,                 3, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kTiParamBalance,                         "Filter Balance",          "Bal",               eSynthLibUnitGeneric,   0.0,         127.0,               64.0 / 127.0,               127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamFilterLink,                      "Filter Link",             "Link",              eSynthLibUnitIndexed,   0.0,           1.0,                        1.0,                 1, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kTiParamAmpAttack,                       "Amp Attack",              "Attack",            eSynthLibUnitGeneric,   0.0,         127.0,                        0.0,               127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamAmpDecay,                        "Amp Decay",               "Decay",             eSynthLibUnitGeneric,   0.0,         127.0,                        1.0,               127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamAmpSustain,                      "Amp Sustain",             "Sustain",           eSynthLibUnitGeneric,   0.0,         127.0,                        1.0,               127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamAmpSustainTime,                  "Amp Sustain Time",        "Time",              eSynthLibUnitGeneric, -64.0,          63.0,               64.0 / 127.0,               127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamAmpRelease,                      "Amp Release",             "Release",           eSynthLibUnitGeneric,   0.0,         127.0,                4.0 / 127.0,               127, SYNTHLIB_MIDI_NONE,                                                                             0},
    // notes §3: not on the panel; the matrix reads them
    {kTiParamBend,                            "Pitch Bend",              "Pitch Bend",        eSynthLibUnitGeneric,   0.0,           1.0,                        0.5,                 0, SYNTHLIB_MIDI_PITCH_BEND, SYNTHLIB_PARAM_HIDDEN | SYNTHLIB_PARAM_NO_SAVE                         },
    {kTiParamPressure,                        "Channel Pressure",        "Channel Pressure",  eSynthLibUnitGeneric,   0.0,           1.0,                          0,                 0, SYNTHLIB_MIDI_AFTERTOUCH, SYNTHLIB_PARAM_HIDDEN | SYNTHLIB_PARAM_NO_SAVE                         },
    {kTiParamWheel,                           "Mod Wheel",               "Mod Wheel",         eSynthLibUnitGeneric,   0.0,           1.0,                          0,                 0, SYNTHLIB_MIDI_MOD_WHEEL,  SYNTHLIB_PARAM_HIDDEN | SYNTHLIB_PARAM_NO_SAVE                         },
    {kTiParamBreath,                          "Breath",                  "Breath",            eSynthLibUnitGeneric,   0.0,           1.0,                          0,                 0, SYNTHLIB_MIDI_BREATH,     SYNTHLIB_PARAM_HIDDEN | SYNTHLIB_PARAM_NO_SAVE                         },
    {kTiParamControl3,                        "Controller 3",            "Controller 3",      eSynthLibUnitGeneric,   0.0,           1.0,                          0,                 0,                        3, SYNTHLIB_PARAM_HIDDEN | SYNTHLIB_PARAM_NO_SAVE                         },
    {kTiParamFoot,                            "Foot Pedal",              "Foot Pedal",        eSynthLibUnitGeneric,   0.0,           1.0,                          0,                 0, SYNTHLIB_MIDI_FOOT,       SYNTHLIB_PARAM_HIDDEN | SYNTHLIB_PARAM_NO_SAVE                         },
    {kTiParamData,                            "Data Entry",              "Data Entry",        eSynthLibUnitGeneric,   0.0,           1.0,                          0,                 0,                        6, SYNTHLIB_PARAM_HIDDEN | SYNTHLIB_PARAM_NO_SAVE                         },
    {kTiParamBalanceCc,                       "Balance",                 "Balance",           eSynthLibUnitGeneric,   0.0,           1.0,                          0,                 0,                        8, SYNTHLIB_PARAM_HIDDEN | SYNTHLIB_PARAM_NO_SAVE                         },
    {kTiParamControl9,                        "Controller 9",            "Controller 9",      eSynthLibUnitGeneric,   0.0,           1.0,                          0,                 0,                        9, SYNTHLIB_PARAM_HIDDEN | SYNTHLIB_PARAM_NO_SAVE                         },
    {kTiParamExpression,                      "Expression",              "Expression",        eSynthLibUnitGeneric,   0.0,           1.0,                          0,                 0, SYNTHLIB_MIDI_EXPRESSION, SYNTHLIB_PARAM_HIDDEN | SYNTHLIB_PARAM_NO_SAVE                         },
    {kTiParamControl12,                       "Controller 12",           "Controller 12",     eSynthLibUnitGeneric,   0.0,           1.0,                          0,                 0,                       12, SYNTHLIB_PARAM_HIDDEN | SYNTHLIB_PARAM_NO_SAVE                         },
    {kTiParamControl13,                       "Controller 13",           "Controller 13",     eSynthLibUnitGeneric,   0.0,           1.0,                          0,                 0,                       13, SYNTHLIB_PARAM_HIDDEN | SYNTHLIB_PARAM_NO_SAVE                         },
    {kTiParamControl14,                       "Controller 14",           "Controller 14",     eSynthLibUnitGeneric,   0.0,           1.0,                          0,                 0,                       14, SYNTHLIB_PARAM_HIDDEN | SYNTHLIB_PARAM_NO_SAVE                         },
    {kTiParamControl15,                       "Controller 15",           "Controller 15",     eSynthLibUnitGeneric,   0.0,           1.0,                          0,                 0,                       15, SYNTHLIB_PARAM_HIDDEN | SYNTHLIB_PARAM_NO_SAVE                         },
    {kTiParamControl16,                       "Controller 16",           "Controller 16",     eSynthLibUnitGeneric,   0.0,           1.0,                          0,                 0,                       16, SYNTHLIB_PARAM_HIDDEN | SYNTHLIB_PARAM_NO_SAVE                         },
    {kTiParamHold,                            "Hold Pedal",              "Hold Pedal",        eSynthLibUnitGeneric,   0.0,           1.0,                          0,                 0, SYNTHLIB_MIDI_SUSTAIN,    SYNTHLIB_PARAM_HIDDEN | SYNTHLIB_PARAM_NO_SAVE                         },
    {kTiParamPortamentoSw,                    "Portamento Switch",       "Portamento Switch", eSynthLibUnitGeneric,   0.0,           1.0,                          0,                 0,                       65, SYNTHLIB_PARAM_HIDDEN | SYNTHLIB_PARAM_NO_SAVE                         },
    {kTiParamSostenuto,                       "Sostenuto",               "Sostenuto",         eSynthLibUnitGeneric,   0.0,           1.0,                          0,                 0,                       66, SYNTHLIB_PARAM_HIDDEN | SYNTHLIB_PARAM_NO_SAVE                         },
    {kTiParamOsc1Wave,                        "Osc 1 Wave",              "Wave",              eSynthLibUnitIndexed,   0.0,         127.0,                   0.000000,               127, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kTiParamOsc1Semi,                        "Osc 1 Semitone",          "Semi",              eSynthLibUnitGeneric, -64.0,          63.0,                   0.503937,               127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamOsc2Shape,                       "Osc 2 Shape",             "Shape",             eSynthLibUnitGeneric,   0.0,         127.0,                   0.503937,               127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamOsc2Width,                       "Osc 2 Pulse Width",       "PW",                eSynthLibUnitGeneric,   0.0,         127.0,                   0.000000,               127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamOsc2Wave,                        "Osc 2 Wave",              "Wave",              eSynthLibUnitIndexed,   0.0,         127.0,                   0.000000,               127, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kTiParamOsc2Semi,                        "Osc 2 Semitone",          "Semi",              eSynthLibUnitGeneric, -64.0,          63.0,                   0.503937,               127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamOsc2Detune,                      "Osc 2 Detune",            "Detune",            eSynthLibUnitGeneric,   0.0,         127.0,                   0.251969,               127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamFm,                              "FM Amount",               "FM",                eSynthLibUnitGeneric,   0.0,         127.0,                   0.000000,               127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamSync,                            "Osc Sync",                "Sync",              eSynthLibUnitIndexed,   0.0,           1.0,                   0.000000,                 1, SYNTHLIB_MIDI_NONE,       SYNTHLIB_PARAM_LIST                                                    },
    {kTiParamOscBalance,                      "Osc Balance",             "Bal",               eSynthLibUnitGeneric,   0.0,         127.0,                   0.503937,               127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamSub,                             "Sub Volume",              "Sub",               eSynthLibUnitGeneric,   0.0,         127.0,                   0.000000,               127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamSubShape,                        "Sub Shape",               "Shape",             eSynthLibUnitIndexed,   0.0,           1.0,                   0.000000,                 1, SYNTHLIB_MIDI_NONE,       SYNTHLIB_PARAM_LIST                                                    },
    {kTiParamOscVolume,                       "Osc Volume",              "Volume",            eSynthLibUnitGeneric,   0.0,         127.0,                   0.503937,               127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamNoise,                           "Noise Volume",            "Noise",             eSynthLibUnitGeneric,   0.0,         127.0,                   0.000000,               127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamNoiseColor,                      "Noise Color",             "Color",             eSynthLibUnitGeneric,   0.0,         127.0,                   0.503937,               127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamRing,                            "Ring Volume",             "Ring",              eSynthLibUnitGeneric,   0.0,         127.0,                   0.000000,               127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamEnvAmount1,                      "Filter 1 Env Amount",     "Env",               eSynthLibUnitGeneric,   0.0,         127.0,                   0.000000,               127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamEnvAmount2,                      "Filter 2 Env Amount",     "Env",               eSynthLibUnitGeneric,   0.0,         127.0,                   0.000000,               127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamKey1,                            "Filter 1 Keyfollow",      "Key",               eSynthLibUnitGeneric, -64.0,          63.0,                   0.503937,               127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamKey2,                            "Filter 2 Keyfollow",      "Key",               eSynthLibUnitGeneric, -64.0,          63.0,                   0.503937,               127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamSaturation,                      "Saturation Curve",        "Sat",               eSynthLibUnitIndexed,   0.0,          14.0,                   0.000000,                14, SYNTHLIB_MIDI_NONE,       SYNTHLIB_PARAM_LIST                                                    },
    {kTiParamFilterAttack,                    "Filter Attack",           "Attack",            eSynthLibUnitGeneric,   0.0,         127.0,                   0.000000,               127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamFilterDecay,                     "Filter Decay",            "Decay",             eSynthLibUnitGeneric,   0.0,         127.0,                   0.362205,               127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamFilterSustain,                   "Filter Sustain",          "Sustain",           eSynthLibUnitGeneric,   0.0,         127.0,                   0.000000,               127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamFilterSustainTime,               "Filter Sustain Time",     "Time",              eSynthLibUnitGeneric, -64.0,          63.0,                   0.503937,               127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamFilterRelease,                   "Filter Release",          "Release",           eSynthLibUnitGeneric,   0.0,         127.0,                   1.000000,               127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamLfo1Rate,                        "LFO 1 Rate",              "Rate",              eSynthLibUnitGeneric,   0.0,         127.0,                   0.377953,               127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamLfo1Shape,                       "LFO 1 Shape",             "Shape",             eSynthLibUnitIndexed,   0.0,          67.0,                   0.014925,                67, SYNTHLIB_MIDI_NONE,       SYNTHLIB_PARAM_LIST                                                    },
    {kTiParamLfo1Pitch,                       "LFO 1 to Osc 1",          "Pitch",             eSynthLibUnitGeneric, -64.0,          63.0,                   0.503937,               127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamLfo2Rate,                        "LFO 2 Rate",              "Rate",              eSynthLibUnitGeneric,   0.0,         127.0,                   0.377953,               127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamLfo2Shape,                       "LFO 2 Shape",             "Shape",             eSynthLibUnitIndexed,   0.0,          67.0,                   0.014925,                67, SYNTHLIB_MIDI_NONE,       SYNTHLIB_PARAM_LIST                                                    },
    {kTiParamLfo2Cutoff,                      "LFO 2 to Cutoff",         "Cutoff",            eSynthLibUnitGeneric, -64.0,          63.0,                   0.503937,               127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamKeyMode,                         "Key Mode",                "Mode",              eSynthLibUnitIndexed,   0.0,           5.0,                   0.000000,                 5, SYNTHLIB_MIDI_NONE,       SYNTHLIB_PARAM_LIST                                                    },
    {kTiParamPortamento,                      "Portamento",              "Porta",             eSynthLibUnitGeneric,   0.0,         127.0,                   0.000000,               127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamPan,                             "Panorama",                "Pan",               eSynthLibUnitGeneric, -64.0,          63.0,                   0.503937,               127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamTranspose,                       "Transpose",               "Trans",             eSynthLibUnitGeneric, -64.0,          63.0,                   0.503937,               127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamPunch,                           "Punch",                   "Punch",             eSynthLibUnitGeneric,   0.0,         127.0,                   0.503937,               127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamOsc3Mode,                        "Osc 3 Mode",              "Mode",              eSynthLibUnitGeneric,   0.0,          67.0,                   0.000000,                67, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamOsc3Volume,                      "Osc 3 Volume",            "Volume",            eSynthLibUnitGeneric,   0.0,         127.0,                   0.503937,               127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamOsc3Semi,                        "Osc 3 Semitone",          "Semi",              eSynthLibUnitGeneric, -64.0,          63.0,                   0.503937,               127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamOsc3Detune,                      "Osc 3 Detune",            "Detune",            eSynthLibUnitGeneric,   0.0,         127.0,                   0.314961,               127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamLfo3Rate,                        "LFO 3 Rate",              "Rate",              eSynthLibUnitGeneric,   0.0,         127.0,                   0.740157,               127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamLfo3Shape,                       "LFO 3 Shape",             "Shape",             eSynthLibUnitIndexed,   0.0,          67.0,                   0.014925,                67, SYNTHLIB_MIDI_NONE,       SYNTHLIB_PARAM_LIST                                                    },
    {kTiParamLfo3Amount,                      "LFO 3 Amount",            "Amount",            eSynthLibUnitGeneric,   0.0,         127.0,                   0.000000,               127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamLfo3Fade,                        "LFO 3 Fade In",           "Fade",              eSynthLibUnitGeneric,   0.0,         127.0,                   0.000000,               127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamEnv3Attack,                      "Env 3 Attack",            "Attack",            eSynthLibUnitGeneric,   0.0,         127.0,                   0.157480,               127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamEnv3Decay,                       "Env 3 Decay",             "Decay",             eSynthLibUnitGeneric,   0.0,         127.0,                   0.551181,               127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamEnv3Sustain,                     "Env 3 Sustain",           "Sustain",           eSynthLibUnitGeneric,   0.0,         127.0,                   0.000000,               127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamEnv3SustainTime,                 "Env 3 Sustain Time",      "Time",              eSynthLibUnitGeneric, -64.0,          63.0,                   0.000000,               127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamEnv3Release,                     "Env 3 Release",           "Release",           eSynthLibUnitGeneric,   0.0,         127.0,                   0.551181,               127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamEnv4Attack,                      "Env 4 Attack",            "Attack",            eSynthLibUnitGeneric,   0.0,         127.0,                   0.157480,               127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamEnv4Decay,                       "Env 4 Decay",             "Decay",             eSynthLibUnitGeneric,   0.0,         127.0,                   0.551181,               127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamEnv4Sustain,                     "Env 4 Sustain",           "Sustain",           eSynthLibUnitGeneric,   0.0,         127.0,                   0.000000,               127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamEnv4SustainTime,                 "Env 4 Sustain Time",      "Time",              eSynthLibUnitGeneric, -64.0,          63.0,                   0.000000,               127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamEnv4Release,                     "Env 4 Release",           "Release",           eSynthLibUnitGeneric,   0.0,         127.0,                   0.551181,               127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamUnisonVoices,                    "Unison Voices",           "Voices",            eSynthLibUnitIndexed,   0.0,          15.0,                   0.000000,                15, SYNTHLIB_MIDI_NONE,       SYNTHLIB_PARAM_LIST                                                    },
    {kTiParamUnisonDetune,                    "Unison Detune",           "Detune",            eSynthLibUnitGeneric,   0.0,         127.0,                   0.377953,               127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamUnisonSpread,                    "Unison Pan Spread",       "Spread",            eSynthLibUnitGeneric,   0.0,         127.0,                   1.000000,               127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamUnisonPhase,                     "Unison LFO Phase",        "Phase",             eSynthLibUnitGeneric, -64.0,          63.0,                   0.503937,               127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamOsc1Model,                       "Osc 1 Model",             "Model",             eSynthLibUnitIndexed,   0.0,           7.0,                   0.000000,                 7, SYNTHLIB_MIDI_NONE,       SYNTHLIB_PARAM_LIST                                                    },
    {kTiParamOsc2Model,                       "Osc 2 Model",             "Model",             eSynthLibUnitIndexed,   0.0,           7.0,                   0.000000,                 7, SYNTHLIB_MIDI_NONE,       SYNTHLIB_PARAM_LIST                                                    },
    {kTiParamMod1Source,                      "Slot 1 Source",           "Source",            eSynthLibUnitGeneric,   0.0,          39.0,                   0.076923,                39, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamMod1Dest,                        "Slot 1 Destination",      "Dest",              eSynthLibUnitGeneric,   0.0,         127.0,                   0.456693,               127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamMod1Amount,                      "Slot 1 Amount",           "Amount",            eSynthLibUnitGeneric, -64.0,          63.0,                   0.787402,               127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamMod2Source,                      "Slot 2 Source",           "Source",            eSynthLibUnitGeneric,   0.0,          39.0,                   0.000000,                39, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamMod2Dest,                        "Slot 2 Destination",      "Dest",              eSynthLibUnitGeneric,   0.0,         127.0,                   0.000000,               127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamMod2Amount,                      "Slot 2 Amount",           "Amount",            eSynthLibUnitGeneric, -64.0,          63.0,                   0.503937,               127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamMod3Source,                      "Slot 3 Source",           "Source",            eSynthLibUnitGeneric,   0.0,          39.0,                   0.000000,                39, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamMod3Dest,                        "Slot 3 Destination",      "Dest",              eSynthLibUnitGeneric,   0.0,         127.0,                   0.000000,               127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamMod3Amount,                      "Slot 3 Amount",           "Amount",            eSynthLibUnitGeneric, -64.0,          63.0,                   0.503937,               127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamMod4Source,                      "Slot 4 Source",           "Source",            eSynthLibUnitGeneric,   0.0,          39.0,                   0.076923,                39, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamMod4Dest,                        "Slot 4 Destination",      "Dest",              eSynthLibUnitGeneric,   0.0,         127.0,                   0.000000,               127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamMod4Amount,                      "Slot 4 Amount",           "Amount",            eSynthLibUnitGeneric, -64.0,          63.0,                   0.503937,               127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamMod5Source,                      "Slot 5 Source",           "Source",            eSynthLibUnitGeneric,   0.0,          39.0,                   0.076923,                39, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamMod5Dest,                        "Slot 5 Destination",      "Dest",              eSynthLibUnitGeneric,   0.0,         127.0,                   0.000000,               127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamMod5Amount,                      "Slot 5 Amount",           "Amount",            eSynthLibUnitGeneric, -64.0,          63.0,                   0.503937,               127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamMod6Source,                      "Slot 6 Source",           "Source",            eSynthLibUnitGeneric,   0.0,          39.0,                   0.076923,                39, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamMod6Dest,                        "Slot 6 Destination",      "Dest",              eSynthLibUnitGeneric,   0.0,         127.0,                   0.000000,               127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamMod6Amount,                      "Slot 6 Amount",           "Amount",            eSynthLibUnitGeneric, -64.0,          63.0,                   0.503937,               127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamArithmetic,                      "Arithmetic",              "Maths",             eSynthLibUnitIndexed,   0.0,           1.0,                        0.0,                 1, SYNTHLIB_MIDI_NONE,       SYNTHLIB_PARAM_LIST                                                    },
    {kTiParamHwLink,                          "Link Hardware",           "Link",              eSynthLibUnitIndexed,   0.0,           1.0,                        0.0,                 1, SYNTHLIB_MIDI_NONE,       SYNTHLIB_PARAM_LIST                                                    },
    {kTiParamDelayOn,                         "Global Delay",            "Delay",             eSynthLibUnitIndexed,   0.0,           1.0,                        1.0,                 1, SYNTHLIB_MIDI_NONE,       SYNTHLIB_PARAM_LIST                                                    },
    {kTiParamReverbOn,                        "Global Reverb",           "Reverb",            eSynthLibUnitIndexed,   0.0,           1.0,                        1.0,                 1, SYNTHLIB_MIDI_NONE,       SYNTHLIB_PARAM_LIST                                                    },
    {kTiParamA21Osc1Keyfollow,                "Osc 1 Key Follow",        "Key 1",             eSynthLibUnitGeneric, -64.0,          63.0,                   0.755906,               127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamA31Osc2Keyfollow,                "Osc 2 Key Follow",        "Key 2",             eSynthLibUnitGeneric, -64.0,          63.0,                   0.755906,               127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamA29Osc2FiltEnvAmt,               "Osc 2 Filter Env Amount", "2 Env",             eSynthLibUnitGeneric, -64.0,          63.0,                   0.503937,               127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamA30FmFiltEnvAmt,                 "FM Filter Env Amount",    "FM Env",            eSynthLibUnitGeneric, -64.0,          63.0,                   0.503937,               127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamB34OscFmMode,                    "FM Mode",                 "FM Mode",           eSynthLibUnitIndexed,   0.0,           6.0,                   0.000000,                 6, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kTiParamB35OscInitPhase,                 "Osc Init Phase",          "Phase",             eSynthLibUnitGeneric,   0.0,         127.0,                   0.000000,               127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamB30Filter1EnvPolarity,           "Filter 1 Env Polarity",   "Env 1",             eSynthLibUnitIndexed,   0.0,           1.0,                   1.000000,                 1, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kTiParamB31Filter2EnvPolarity,           "Filter 2 Env Polarity",   "Env 2",             eSynthLibUnitIndexed,   0.0,           1.0,                   1.000000,                 1, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kTiParamB33FilterKeytrackBase,           "Filter Key Base",         "Key Base",          eSynthLibUnitGeneric,   0.0,         127.0,                   0.283465,               127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamB47Osc1ShapeVelocity,            "Velocity Shape 1",        "Shape 1",           eSynthLibUnitGeneric, -64.0,          63.0,                   0.503937,               127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamB48Osc2ShapeVelocity,            "Velocity Shape 2",        "Shape 2",           eSynthLibUnitGeneric, -64.0,          63.0,                   0.503937,               127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamB49PulsewidthVelocity,           "Velocity PW",             "PW",                eSynthLibUnitGeneric, -64.0,          63.0,                   0.503937,               127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamB50FmAmountVelocity,             "Velocity FM",             "FM",                eSynthLibUnitGeneric, -64.0,          63.0,                   0.503937,               127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamB54Flt1EnvamtVelocity,           "Velocity Env 1",          "Env 1",             eSynthLibUnitGeneric, -64.0,          63.0,                   0.503937,               127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamB55Flt2EnvamtVelocity,           "Velocity Env 2",          "Env 2",             eSynthLibUnitGeneric, -64.0,          63.0,                   0.503937,               127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamB56Resonance1Velocity,           "Velocity Reso 1",         "Reso 1",            eSynthLibUnitGeneric, -64.0,          63.0,                   0.503937,               127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamB57Resonance2Velocity,           "Velocity Reso 2",         "Reso 2",            eSynthLibUnitGeneric, -64.0,          63.0,                   0.503937,               127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamB60AmpVelocity,                  "Velocity Amp",            "Amp",               eSynthLibUnitGeneric, -64.0,          63.0,                   0.503937,               127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamB61PanoramaVelocity,             "Velocity Pan",            "Pan",               eSynthLibUnitGeneric, -64.0,          63.0,                   0.503937,               127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamA69Lfo1EnvMode,                  "LFO 1 Env Mode",          "Env Mode",          eSynthLibUnitIndexed,   0.0,           1.0,                   0.000000,                 1, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kTiParamA70Lfo1Mode,                     "LFO 1 Mode",              "Mode",              eSynthLibUnitIndexed,   0.0,           1.0,                   0.000000,                 1, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kTiParamA71Lfo1Symmetry,                 "LFO 1 Symmetry",          "Symmetry",          eSynthLibUnitGeneric, -64.0,          63.0,                   0.503937,               127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamA72Lfo1Keyfollow,                "LFO 1 Key",               "Key",               eSynthLibUnitGeneric,   0.0,         127.0,                   0.000000,               127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamA73Lfo1Keytrigger,               "LFO 1 Trigger",           "Trigger",           eSynthLibUnitGeneric,   0.0,         127.0,                   0.000000,               127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamB18Lfo1Clock,                    "LFO 1 Clock",             "Clock",             eSynthLibUnitIndexed,   0.0,          21.0,                   0.000000,                21, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kTiParamA75Osc2Lfo1Amount,               "LFO 1 Osc 2",             "Osc 2",             eSynthLibUnitGeneric, -64.0,          63.0,                   0.503937,               127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamA76PwLfo1Amount,                 "LFO 1 PW",                "PW",                eSynthLibUnitGeneric, -64.0,          63.0,                   0.503937,               127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamA77ResoLfo1Amount,               "LFO 1 Reso",              "Reso",              eSynthLibUnitGeneric, -64.0,          63.0,                   0.503937,               127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamA78FiltgainLfo1Amount,           "LFO 1 Gain",              "Gain",              eSynthLibUnitGeneric, -64.0,          63.0,                   0.503937,               127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamA81Lfo2EnvMode,                  "LFO 2 Env Mode",          "Env Mode",          eSynthLibUnitIndexed,   0.0,           1.0,                   0.000000,                 1, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kTiParamA82Lfo2Mode,                     "LFO 2 Mode",              "Mode",              eSynthLibUnitIndexed,   0.0,           1.0,                   0.000000,                 1, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kTiParamA83Lfo2Symmetry,                 "LFO 2 Symmetry",          "Symmetry",          eSynthLibUnitGeneric, -64.0,          63.0,                   0.503937,               127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamA84Lfo2Keyfollow,                "LFO 2 Key",               "Key",               eSynthLibUnitGeneric,   0.0,         127.0,                   0.000000,               127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamA85Lfo2Keytrigger,               "LFO 2 Trigger",           "Trigger",           eSynthLibUnitGeneric,   0.0,         127.0,                   0.000000,               127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamB19Lfo2Clock,                    "LFO 2 Clock",             "Clock",             eSynthLibUnitIndexed,   0.0,          21.0,                   0.000000,                21, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kTiParamA86ShapeLfo2Amount,              "LFO 2 Shape",             "Shape",             eSynthLibUnitGeneric, -64.0,          63.0,                   0.503937,               127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamA87FmLfo2Amount,                 "LFO 2 FM",                "FM",                eSynthLibUnitGeneric, -64.0,          63.0,                   0.503937,               127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamA89Cutoff2Lfo2Amount,            "LFO 2 Cutoff 2",          "Cutoff 2",          eSynthLibUnitGeneric, -64.0,          63.0,                   0.503937,               127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamA90PanLfo2Amount,                "LFO 2 Pan",               "Pan",               eSynthLibUnitGeneric, -64.0,          63.0,                   0.503937,               127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamB9Lfo3Mode,                      "LFO 3 Mode",              "Mode",              eSynthLibUnitIndexed,   0.0,           1.0,                   0.000000,                 1, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kTiParamB10Lfo3Keyfollow,                "LFO 3 Key",               "Key",               eSynthLibUnitGeneric,   0.0,         127.0,                   0.000000,               127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamB11Lfo3Destination,              "LFO 3 Dest",              "Dest",              eSynthLibUnitIndexed,   0.0,           6.0,                   0.166667,                 6, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kTiParamB21Lfo3Clock,                    "LFO 3 Clock",             "Clock",             eSynthLibUnitIndexed,   0.0,          21.0,                   0.000000,                21, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kTiParamB79Lfo1AssignDest,               "LFO Assign 1 Dest",       "1 Dest",            eSynthLibUnitIndexed,   0.0,         127.0,                   0.000000,               127, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kTiParamB80Lfo1AssignAmount,             "LFO Assign 1 Amount",     "1 Amount",          eSynthLibUnitGeneric, -64.0,          63.0,                   0.503937,               127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamB81Lfo2AssignDest,               "LFO Assign 2 Dest",       "2 Dest",            eSynthLibUnitIndexed,   0.0,         127.0,                   0.000000,               127, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kTiParamB82Lfo2AssignAmount,             "LFO Assign 2 Amount",     "2 Amount",          eSynthLibUnitGeneric, -64.0,          63.0,                   0.503937,               127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamB16ClockTempo,                   "Sound Tempo",             "Tempo",             eSynthLibUnitGeneric,   0.0,         127.0,                   0.448819,               127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamB25ControlSmoothMode,            "Control Smooth",          "Smooth",            eSynthLibUnitIndexed,   0.0,          20.0,                   0.050000,                20, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kTiParamB26BenderRangeUp,                "Control Bend Up",         "Bend Up",           eSynthLibUnitGeneric, -64.0,          63.0,                   0.519685,               127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamB27BenderRangeDown,              "Control Bend Dn",         "Bend Dn",           eSynthLibUnitGeneric, -64.0,          63.0,                   0.488189,               127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamC37Osc1WavetableSync,            "Osc 1 Table Sync",        "Sync",              eSynthLibUnitGeneric,   0.0,         127.0,                   0.000000,               127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamC42Osc1WavetableFormantshift,    "Osc 1 Table Formant",     "Formant",           eSynthLibUnitGeneric, -64.0,          63.0,                   0.503937,               127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamC43Osc1WavetableInternalDetune,  "Osc 1 Table Detune",      "Detune",            eSynthLibUnitGeneric,   0.0,         127.0,                   0.000000,               127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamC44Osc1WavetableInterpolation,   "Osc 1 Table Smooth",      "Smooth",            eSynthLibUnitGeneric,   0.0,         127.0,                   0.000000,               127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamC57Osc2WavetableSync,            "Osc 2 Table Sync",        "Sync",              eSynthLibUnitGeneric,   0.0,         127.0,                   0.000000,               127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamC62Osc2WavetableFormantshift,    "Osc 2 Table Formant",     "Formant",           eSynthLibUnitGeneric, -64.0,          63.0,                   0.503937,               127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamC63Osc2WavetableInternalDetune,  "Osc 2 Table Detune",      "Detune",            eSynthLibUnitGeneric,   0.0,         127.0,                   0.000000,               127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamC64Osc2WavetableInterpolation,   "Osc 2 Table Smooth",      "Smooth",            eSynthLibUnitGeneric,   0.0,         127.0,                   0.000000,               127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamB100DistortionCurve,             "Distortion Curve",        "Curve",             eSynthLibUnitIndexed,   0.0,          25.0,                   0.000000,                25, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kTiParamB101DistortionIntensity,         "Distortion Amount",       "Amount",            eSynthLibUnitGeneric,   0.0,         127.0,                   0.000000,               127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamC70PatchDistortionTrebleBooster, "Distortion Treble",       "Treble",            eSynthLibUnitGeneric,   0.0,         127.0,                   0.000000,               127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamC71PatchDistortionHighCut,       "Distortion High Cut",     "High Cut",          eSynthLibUnitGeneric,   0.0,         127.0,                   0.000000,               127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamC72PatchDistortionMix,           "Distortion Mix",          "Mix",               eSynthLibUnitGeneric,   0.0,         127.0,                   1.000000,               127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamC73PatchDistortionQuality,       "Distortion Quality",      "Quality",           eSynthLibUnitGeneric,   0.0,         127.0,                   1.000000,               127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamC74PatchDistortionTone127,       "Distortion Tone",         "Tone",              eSynthLibUnitGeneric, -64.0,          63.0,                   0.000000,               127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamC26CharacterType,                "Character Type",          "Type",              eSynthLibUnitIndexed,   0.0,           8.0,                   0.000000,                 8, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kTiParamB97BassIntensity,                "Character Amount",        "Amount",            eSynthLibUnitGeneric,   0.0,         127.0,                   0.314961,               127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamB98BassTune,                     "Character Tune",          "Tune",              eSynthLibUnitGeneric, -64.0,          63.0,                   0.251969,               127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamC19FilterBankType,               "Filterbank Type",         "Type",              eSynthLibUnitIndexed,   0.0,          11.0,                   0.000000,                11, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kTiParamC20FilterBankMix,                "Filterbank Mix",          "Mix",               eSynthLibUnitGeneric,   0.0,         127.0,                   1.000000,               127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamC21FilterBankCombFrequency,      "Filterbank Freq",         "Freq",              eSynthLibUnitGeneric,   0.0,          96.0,                   0.000000,                96, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamC22FilterBankStereoPhase,        "Filterbank Stereo",       "Stereo",            eSynthLibUnitGeneric,   0.0,         127.0,                   0.503937,               127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamC23FilterBankFilterType,         "Filterbank Shape L",      "Shape L",           eSynthLibUnitGeneric,   0.0,         127.0,                   0.000000,               127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamC24FilterBankShapeR,             "Filterbank Shape R",      "Shape R",           eSynthLibUnitGeneric, -64.0,          63.0,                   0.000000,               127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamC25FilterBankResonance,          "Filterbank Reso",         "Reso",              eSynthLibUnitGeneric,   0.0,         127.0,                   0.503937,               127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamA103ChorusType,                  "Chorus Type",             "Type",              eSynthLibUnitIndexed,   0.0,           6.0,                   0.166667,                 6, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kTiParamA104ChorusMix2,                  "Chorus Mix 2",            "Mix 2",             eSynthLibUnitGeneric,   0.0,         127.0,                   1.000000,               127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamA105ChorusMix,                   "Chorus Mix",              "Mix",               eSynthLibUnitGeneric,   0.0,         127.0,                   0.000000,               127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamA106ChorusRate,                  "Chorus Rate",             "Rate",              eSynthLibUnitGeneric,   0.0,         127.0,                   0.543307,               127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamA107ChorusDistance,              "Chorus Distance",         "Distance",          eSynthLibUnitGeneric,   0.0,         127.0,                   0.125984,               127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamA108ChorusAmount,                "Chorus Depth",            "Depth",             eSynthLibUnitGeneric, -64.0,          63.0,                   1.000000,               127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamA109ChorusFeedback,              "Chorus Feedback",         "Feedback",          eSynthLibUnitGeneric, -64.0,          63.0,                   0.503937,               127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamA110ChorusLfoShape,              "Chorus LFO",              "LFO",               eSynthLibUnitIndexed,   0.0,           5.0,                   0.200000,                 5, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kTiParamA111ChorusXOver,                 "Chorus X Over",           "X Over",            eSynthLibUnitGeneric,   0.0,         127.0,                   0.000000,               127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamB84PhaserMode,                   "Phaser Stages",           "Stages",            eSynthLibUnitIndexed,   0.0,           5.0,                   0.600000,                 5, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kTiParamB85PhaserMix,                    "Phaser Mix",              "Mix",               eSynthLibUnitGeneric,   0.0,         127.0,                   0.000000,               127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamB86PhaserRate,                   "Phaser Rate",             "Rate",              eSynthLibUnitGeneric,   0.0,         127.0,                   0.283465,               127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamB87PhaserDepth,                  "Phaser Depth",            "Depth",             eSynthLibUnitGeneric,   0.0,         127.0,                   0.881890,               127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamB88PhaserFrequency,              "Phaser Freq",             "Freq",              eSynthLibUnitGeneric,   0.0,         127.0,                   0.377953,               127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamB89PhaserFeedback,               "Phaser Feedback",         "Feedback",          eSynthLibUnitGeneric, -64.0,          63.0,                   0.503937,               127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamB90PhaserSpread,                 "Phaser Spread",           "Spread",            eSynthLibUnitGeneric,   0.0,         127.0,                   1.000000,               127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamB95LoweqGain,                    "Eq Low",                  "Low",               eSynthLibUnitGeneric, -64.0,          63.0,                   0.503937,               127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamB45LoweqFrequency,               "Eq Low Hz",               "Low Hz",            eSynthLibUnitGeneric,   0.0,         127.0,                   0.338583,               127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamB92MideqGain,                    "Eq Mid",                  "Mid",               eSynthLibUnitGeneric, -64.0,          63.0,                   0.503937,               127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamB93MideqFrequency,               "Eq Mid Hz",               "Mid Hz",            eSynthLibUnitGeneric,   0.0,         127.0,                   0.559055,               127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamB94MideqQFactor,                 "Eq Mid Q",                "Mid Q",             eSynthLibUnitGeneric,   0.0,         127.0,                   0.401575,               127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamB96HigheqGain,                   "Eq High",                 "High",              eSynthLibUnitGeneric, -64.0,          63.0,                   0.503937,               127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamB46HigheqFrequency,              "Eq High Hz",              "High Hz",           eSynthLibUnitGeneric,   0.0,         127.0,                   0.669291,               127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamA112DelayMode,                   "Delay Mode",              "Mode",              eSynthLibUnitIndexed,   0.0,          22.0,                   0.045455,                22, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kTiParamA113DelaySend,                   "Delay Send",              "Send",              eSynthLibUnitGeneric,   0.0,         127.0,                   0.000000,               127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamA114DelayTapeDelayTime,          "Delay Time",              "Time",              eSynthLibUnitGeneric,   0.0,         127.0,                   0.000000,               127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamA115DelayFeedback,               "Delay Feedback",          "Feedback",          eSynthLibUnitGeneric,   0.0,         127.0,                   0.000000,               127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamA116DlyRateRevDecay,             "Delay Rate",              "Rate",              eSynthLibUnitGeneric,   0.0,         127.0,                   0.125984,               127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamA117DlyDepth,                    "Delay Depth",             "Depth",             eSynthLibUnitGeneric,   0.0,         127.0,                   0.094488,               127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamA118DelayLfoShape,               "Delay LFO",               "LFO",               eSynthLibUnitIndexed,   0.0,           5.0,                   0.200000,                 5, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kTiParamA119DelayColor,                  "Delay Color",             "Color",             eSynthLibUnitGeneric, -64.0,          63.0,                   0.503937,               127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamB20DelayClock,                   "Delay Clock",             "Clock",             eSynthLibUnitIndexed,   0.0,          16.0,                   0.000000,                16, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kTiParamC10DelayType,                    "Tape Type",               "Type",              eSynthLibUnitIndexed,   0.0,           3.0,                   0.000000,                 3, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kTiParamC12DelayTapeDelayRatio,          "Tape Ratio",              "Ratio",             eSynthLibUnitIndexed,   0.0,           6.0,                   0.500000,                 6, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kTiParamC13DelayTapeDelayLeftClock,      "Tape Left",               "Left",              eSynthLibUnitIndexed,   0.0,           5.0,                   0.800000,                 5, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kTiParamC14DelayTapeDelayRightClock,     "Tape Right",              "Right",             eSynthLibUnitIndexed,   0.0,           5.0,                   0.400000,                 5, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kTiParamC17DelayTapeDelayBandwidth,      "Tape Band",               "Band",              eSynthLibUnitGeneric,   0.0,         127.0,                   0.000000,               127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamC1ReverbMode,                    "Reverb Mode",             "Mode",              eSynthLibUnitIndexed,   0.0,           3.0,                   0.333333,                 3, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kTiParamC2ReverbSend,                    "Reverb Send",             "Send",              eSynthLibUnitGeneric,   0.0,         127.0,                   0.000000,               127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamC3ReverbType,                    "Reverb Type",             "Type",              eSynthLibUnitIndexed,   0.0,           3.0,                   0.666667,                 3, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kTiParamC4ReverbTime,                    "Reverb Time",             "Time",              eSynthLibUnitGeneric,   0.0,         127.0,                   0.535433,               127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamC5ReverbDamping,                 "Reverb Damping",          "Damping",           eSynthLibUnitGeneric,   0.0,         127.0,                   0.157480,               127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamC6ReverbColor,                   "Reverb Color",            "Color",             eSynthLibUnitGeneric, -64.0,          63.0,                   0.503937,               127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamC7ReverbPredelay,                "Reverb Predelay",         "Predelay",          eSynthLibUnitGeneric,   0.0,          92.0,                   0.010870,                92, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamC8ReverbClock,                   "Reverb Clock",            "Clock",             eSynthLibUnitIndexed,   0.0,          16.0,                   0.000000,                16, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kTiParamC9ReverbFeedback,                "Reverb Feedback",         "Feedback",          eSynthLibUnitGeneric,   0.0,         127.0,                   0.000000,               127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamC90Assign1Destination2,          "Slot 1 Destination 2",    "Dest 2",            eSynthLibUnitIndexed,   0.0,         127.0,                   0.000000,               127, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kTiParamC91Assign1Amount2,               "Slot 1 Amount 2",         "Amt 2",             eSynthLibUnitGeneric, -64.0,          63.0,                   0.503937,               127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamC92Assign1Destination3,          "Slot 1 Destination 3",    "Dest 3",            eSynthLibUnitIndexed,   0.0,         127.0,                   0.000000,               127, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kTiParamC93Assign1Amount3,               "Slot 1 Amount 3",         "Amt 3",             eSynthLibUnitGeneric, -64.0,          63.0,                   0.503937,               127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamB70Assign2Destination2,          "Slot 2 Destination 2",    "Dest 2",            eSynthLibUnitIndexed,   0.0,         127.0,                   0.000000,               127, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kTiParamB71Assign2Amount2,               "Slot 2 Amount 2",         "Amt 2",             eSynthLibUnitGeneric, -64.0,          63.0,                   0.503937,               127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamC94Assign2Destination3,          "Slot 2 Destination 3",    "Dest 3",            eSynthLibUnitIndexed,   0.0,         127.0,                   0.000000,               127, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kTiParamC95Assign2Amount3,               "Slot 2 Amount 3",         "Amt 3",             eSynthLibUnitGeneric, -64.0,          63.0,                   0.503937,               127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamB75Assign3Destination2,          "Slot 3 Destination 2",    "Dest 2",            eSynthLibUnitIndexed,   0.0,         127.0,                   0.000000,               127, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kTiParamB76Assign3Amount2,               "Slot 3 Amount 2",         "Amt 2",             eSynthLibUnitGeneric, -64.0,          63.0,                   0.503937,               127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamB77Assign3Destination3,          "Slot 3 Destination 3",    "Dest 3",            eSynthLibUnitIndexed,   0.0,         127.0,                   0.000000,               127, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kTiParamB78Assign3Amount3,               "Slot 3 Amount 3",         "Amt 3",             eSynthLibUnitGeneric, -64.0,          63.0,                   0.503937,               127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamC96Assign4Destination2,          "Slot 4 Destination 2",    "Dest 2",            eSynthLibUnitIndexed,   0.0,         127.0,                   0.000000,               127, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kTiParamC97Assign4Amount2,               "Slot 4 Amount 2",         "Amt 2",             eSynthLibUnitGeneric, -64.0,          63.0,                   0.503937,               127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamC98Assign4Destination3,          "Slot 4 Destination 3",    "Dest 3",            eSynthLibUnitIndexed,   0.0,         127.0,                   0.000000,               127, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kTiParamC99Assign4Amount3,               "Slot 4 Amount 3",         "Amt 3",             eSynthLibUnitGeneric, -64.0,          63.0,                   0.503937,               127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamC100Assign5Destination2,         "Slot 5 Destination 2",    "Dest 2",            eSynthLibUnitIndexed,   0.0,         127.0,                   0.000000,               127, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kTiParamC101Assign5Amount2,              "Slot 5 Amount 2",         "Amt 2",             eSynthLibUnitGeneric, -64.0,          63.0,                   0.503937,               127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamC102Assign5Destination3,         "Slot 5 Destination 3",    "Dest 3",            eSynthLibUnitIndexed,   0.0,         127.0,                   0.000000,               127, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kTiParamC103Assign5Amount3,              "Slot 5 Amount 3",         "Amt 3",             eSynthLibUnitGeneric, -64.0,          63.0,                   0.503937,               127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamC104Assign6Destination2,         "Slot 6 Destination 2",    "Dest 2",            eSynthLibUnitIndexed,   0.0,         127.0,                   0.000000,               127, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kTiParamC105Assign6Amount2,              "Slot 6 Amount 2",         "Amt 2",             eSynthLibUnitGeneric, -64.0,          63.0,                   0.503937,               127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamC106Assign6Destination3,         "Slot 6 Destination 3",    "Dest 3",            eSynthLibUnitIndexed,   0.0,         127.0,                   0.000000,               127, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kTiParamC107Assign6Amount3,              "Slot 6 Amount 3",         "Amt 3",             eSynthLibUnitGeneric, -64.0,          63.0,                   0.503937,               127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kTiParamProgram,                         "Program",                 "Program",           eSynthLibUnitGeneric,   0.0,         127.0,                   0.000000,               127, SYNTHLIB_MIDI_NONE,       SYNTHLIB_PARAM_PROGRAM | SYNTHLIB_PARAM_HIDDEN | SYNTHLIB_PARAM_NO_SAVE},
    {kTiParamBank,                            "Bank Select",             "Bank",              eSynthLibUnitGeneric,   0.0,         127.0,                   0.000000,               127,                       32, SYNTHLIB_PARAM_HIDDEN | SYNTHLIB_PARAM_NO_SAVE                         },
    {kTiParamMaster,                          "Master Volume",           "Master",            eSynthLibUnitGeneric,   0.0,         127.0,                        1.0,               127, SYNTHLIB_MIDI_NONE,                                                                             0},
};

static const char *const kFilters[]                     = {"LP", "HP", "BP", "BS", "A1", "A2", "A3", "A4"};
static const char *const kFilters2[]                    = {"LP", "HP", "BP", "BS"};
static const char *const kRoutings[]                    = {"SER4", "SER6", "PAR4", "SPLIT"};
static const char *const kOffOn[]                       = {"OFF", "ON"};
static const char *const kArithmetic[]                  = {"FLOAT", "FIXED"};
static const char *const kModels[]                      = {"CLASSIC", "HYPER", "WAVE", "PWM", "GRAIN", "GRAIN+", "FORMANT", "FORM+"};
static const char *const kUnison[]                      = {"OFF", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "13", "14", "15", "16"};
static const char *const kSubShapes[]                   = {"SQUARE", "TRI"};
static const char *const kSaturations[]                 = {"OFF", "LIGHT", "SOFT", "MIDDLE", "HARD", "DIGITAL", "SHAPER", "RECT", "BITS", "RATE", "RATE+", "LOW", "LOW+", "HIGH", "HIGH+"};
static const char *const kKeyModes[]                    = {"POLY", "MONO 1", "MONO 2", "MONO 3", "MONO 4", "HOLD"};

static const char *const kListWave[128]                 = {"Sine", "Triangle", "Wave 3", "Wave 4", "Wave 5", "Wave 6", "Wave 7", "Wave 8", "Wave 9", "Wave 10", "Wave 11", "Wave 12", "Wave 13", "Wave 14", "Wave 15", "Wave 16", "Wave 17", "Wave 18", "Wave 19", "Wave 20", "Wave 21", "Wave 22", "Wave 23", "Wave 24", "Wave 25", "Wave 26", "Wave 27", "Wave 28", "Wave 29", "Wave 30", "Wave 31", "Wave 32", "Wave 33", "Wave 34", "Wave 35", "Wave 36", "Wave 37", "Wave 38", "Wave 39", "Wave 40", "Wave 41", "Wave 42", "Wave 43", "Wave 44", "Wave 45", "Wave 46", "Wave 47", "Wave 48", "Wave 49", "Wave 50", "Wave 51", "Wave 52", "Wave 53", "Wave 54", "Wave 55", "Wave 56", "Wave 57", "Wave 58", "Wave 59", "Wave 60", "Wave 61", "Wave 62", "Wave 63", "Wave 64"};
static const char *const kListWavetable[128]            = {"Sine", "HarmncSweep", "Glass Sweep", "Draw Bars", "Clusters", "Insine Out", "Landing", "Liquid Metal", "Opposition", "Overtunes 1", "Overtunes 2", "Scale Trix", "Sine Rider", "Sqr Series", "Upsine Down", "Thumbs Up", "Waterphone", "E-Chime", "Tinkabell", "Bellfizz", "Bellentine", "Robot Wars", "Alternator", "Finger Bass", "Fizzybar", "Flutes", "HP Love", "Majestix", "Hotch Potch", "Resynater", "Smooth Rough", "Sawsalito", "Bells 1", "Bells 2", "SportReport", "Metal Guru", "Bat Cave", "Acetate", "Buzzbizz", "Buzzpartout", "Vanish", "Overbones", "Pulsechecker", "Stratosfear", "Sooty Sweep", "Throaty", "Didgitalis", "Evil", "Chords", "FM Grit", "Bellsarnie", "Octavius", "Eat Pulse", "Sinzin", "Sine System", "Clip Sweep", "Roughage", "Waving", "Pling Saw", "E-Peas", "Bump Sweep", "Filter Sqr", "Fourmant", "Formantera", "Sundial 1", "Sundial 2", "Sundial 3", "Clipdial 1", "Clipdial 2", "Voxonix", "Solenoid", "KlingKlang", "Violator", "Potassium", "Pile Up", "Tincanali", "Sniper", "Squeezy", "Decomposer", "Morfants", "Pingvox", "Adenoids", "Nasal", "Partialism", "TableDance", "Cascade", "Prismism", "Friction", "Robotix", "Whizzfizz", "Spangly", "Fluxbin", "Fiboglide", "Fibonice", "Fibonasty", "Penetrator", "Blinder", "Element 5", "Bad Signs", "Domina7rix"};
static const char *const kListSource[40]                = {"Off", "Pitch Bend", "Chan Pressure", "Mod Wheel", "Breath", "Controller 3", "Foot Pedal", "Data Entry", "Balance", "Controller 9", "Expression", "Controller 12", "Controller 13", "Controller 14", "Controller 15", "Controller 16", "Hold Pedal", "Portamento Sw", "Sost Pedal", "Amp Envelope", "Filter Envelope", "LFO 1 bipolar", "LFO 2 bipolar", "LFO 3 bipolar", "Velocity On", "Velocity Off", "Key Follow", "Random", "Arp Input", "LFO 1 unipolar", "LFO 2 unipolar", "LFO 3 unipolar", "1% constant", "10% constant", "AnaKey1 Fine", "AnaKey2 Fine", "AnaKey1Coarse", "AnaKey2Coarse", "Envelope 3", "Envelope 4"};
static const char *const kListDest[128]                 = {"Off", "Patch Volume", "Osc 1 Interpolation", "Panorama", "Transpose", "Portamento", "Osc 1 Shape/Index", "Osc 1 Pulse Width", "Osc 1 Wave Select", "Osc 1 Pitch", "Slot 6 Amount 3", "Osc 2 Shape/Index", "Osc 2 Pulse Width", "Osc 2 Wave Select", "Osc 2 Pitch", "Osc 2 Detune", "Osc 2 FM Amount", "FiltEnv > Osc 2 Pitch", "FiltEnv>FM/Sync", "Osc 2 Interpolation", "Osc Balance", "Sub Osc Volume", "Osc Volume", "Noise Volume", "Filter 1 Cutoff", "Filter 2 Cutoff", "Filter 1 Resonance", "Filter 2 Resonance", "Filter 1 Env Amount", "Filter 2 Env Amount", "Slot 5 Amount 2", "Slot 5 Amount 3", "Filter Balance", "Filter Env Attack", "Filter Env Decay", "Filter Env Sustain", "Filter Env Slope", "Filter Env Release", "Amp Env Attack", "Amp Env Decay", "Amp Env Sustain", "Amp Env Slope", "Amp Env Release", "LFO 1 Rate", "LFO 1 Contour", "LFO 1>Osc 1 Pitch", "LFO 1>Osc 2 Pitch", "LFO 1>Pulse Width", "LFO 1>Resonance", "LFO 1>Filter Gain", "LFO 2 Rate", "LFO 2 Contour", "LFO 2>Shape", "LFO 2>FM Amount", "LFO 2>Cutoff 1", "LFO 2>Cutoff 2", "LFO 2>Panorama", "LFO 3 Rate", "LFO 3 Assign Amt", "Unison Detune", "Pan Spread", "Unison LFO Phase", "Chorus Mix", "Chorus Mod Rate", "Chorus Mod Depth", "Chorus Delay", "Chorus Feedback", "Delay Send", "Delay Time", "Delay Feedback", "Delay Mod Rate", "Delay Mod Depth", "Reverb Send", "Osc 1 Wavetable Index", "Osc 2 Wavetable Index", "Slot 6 Amount 2", "Slot 4 Amount 2", "Slot 4 Amount 3", "Filterbank Reso", "Filterbank Poles", "Slot 2 Amount 3", "Filterbank Slope", "Slot 1 Amount 1", "Slot 2 Amount 1", "Slot 2 Amount 2", "Slot 3 Amount 1", "Slot 3 Amount 2", "Slot 3 Amount 3", "-", "Punch Intensity", "Ring Modulator", "Noise Color", "Delay Coloration", "Slot 1 Amount 2", "Slot 1 Amount 3", "Distortion Intensity", "Filterbank Freq", "Osc 3 Volume", "Osc 3 Pitch", "Osc 3 Detune", "LFO 1 Assign Amt", "LFO 2 Assign Amt", "Phaser Mix", "Phaser Mod Rate", "Phaser Mod Depth", "Phaser Frequency", "Phaser Feedback", "-", "Reverb Time", "Reverb Damping", "Reverb Color", "Reverb PreDelay", "-", "Surround Balance", "Arp Note Length", "Arp Swing Factor", "Arp Pattern", "EQ Mid Gain", "EQ Mid Frequency", "-", "Slot 4 Amount 1", "Slot 5 Amount 1", "Slot 6 Amount 1", "Osc 1 F-Shift", "Osc 2 F-Shift", "Osc 1 F-Spread", "Osc 2 F-Spread", "Distortion Mix"};
static const char *const kListLfoShape[68]              = {"Sine", "Triangle", "Saw", "Square", "S&H", "S&G", "Wave 3", "Wave 4", "Wave 5", "Wave 6", "Wave 7", "Wave 8", "Wave 9", "Wave 10", "Wave 11", "Wave 12", "Wave 13", "Wave 14", "Wave 15", "Wave 16", "Wave 17", "Wave 18", "Wave 19", "Wave 20", "Wave 21", "Wave 22", "Wave 23", "Wave 24", "Wave 25", "Wave 26", "Wave 27", "Wave 28", "Wave 29", "Wave 30", "Wave 31", "Wave 32", "Wave 33", "Wave 34", "Wave 35", "Wave 36", "Wave 37", "Wave 38", "Wave 39", "Wave 40", "Wave 41", "Wave 42", "Wave 43", "Wave 44", "Wave 45", "Wave 46", "Wave 47", "Wave 48", "Wave 49", "Wave 50", "Wave 51", "Wave 52", "Wave 53", "Wave 54", "Wave 55", "Wave 56", "Wave 57", "Wave 58", "Wave 59", "Wave 60", "Wave 61", "Wave 62", "Wave 63", "Wave 64"};
static const char *const kListNameDistortionModes[26]   = {"Off", "Light", "Soft", "Medium", "Hard", "Digital", "Wave Shaper", "Rectifier", "Bit Reducer Old", "Rate Reducer Old", "Low Pass", "High Pass", "Wide", "Soft Bounce", "Hard Bounce", "Sine Fold", "Triangle Fold", "Sawtooth Fold", "Rate Reducer", "Bit Reducer", "Mint Overdrive", "Curry Overdrive", "Saffron Overdrive", "Onion Overdrive", "Pepper Overdrive", "Chili Overdrive"};
static const char *const kListCharacters[9]             = {"Analog Boost", "Vintage 1", "Vintage 2", "Vintage 3", "Pad Opener", "Lead Enhancer", "Bass Enhancer", "Stereo Widener", "Speaker Cabinet"};
static const char *const kListChorusType[7]             = {"Off", "Classic", "Vintage", "Hyper Chorus", "Air Chorus", "Vibrato", "Rotary Speaker"};
static const char *const kListControlSmoothMode[21]     = {"Off", "On", "Auto", "Note", "Quantise 1/64", "Quantise 1/32", "Quantise 1/16", "Quantise 1/8", "Quantise 1/4", "Quantise 1/2", "Quantise 3/64", "Quantise 3/32", "Quantise 3/16", "Quantise 3/8", "Quantise 1/24", "Quantise 1/12", "Quantise 1/6", "Quantise 1/3", "Quantise 2/3", "Quantise 3/4", "Quantise 1/1"};
static const char *const kListDelayLfoShape[6]          = {"Sine", "Triangle", "Saw", "Square", "S&H", "S&G"};
static const char *const kListDelayMode[23]             = {"Off", "Simple Delay", "Ping Pong 2:1", "Ping Pong 4:3", "Ping Pong 4:1", "Ping Pong 8:7", "Pattern 1+1", "Pattern 2+1", "Pattern 3+1", "Pattern 4+1", "Pattern 5+1", "Pattern 2+3", "Pattern 2+5", "Pattern 3+2", "Pattern 3+3", "Pattern 3+4", "Pattern 3+5", "Pattern 4+3", "Pattern 4+5", "Pattern 5+2", "Pattern 5+3", "Pattern 5+4", "Pattern 5+5"};
static const char *const kListDelayRatio[7]             = {"1/4", "2/4", "3/4", "4/4", "4/3", "4/2", "4/1"};
static const char *const kListDelayType[4]              = {"Classic", "Tape Clocked", "Tape Free", "Tape Doppler"};
static const char *const kListFilterBankType[12]        = {"Off", "Ring Modulator", "Frequency Shifter", "Vowel Filter", "Comb Filter", "1 Pole XFade", "2 Pole XFade", "4 Pole XFade", "6 Pole XFade", "LP VariSlope", "HP VariSlope", "BP VariSlope"};
static const char *const kListLfoDest[7]                = {"Osc 1 Pitch", "Osc 1+2 Pitch", "Osc 2 Pitch", "Osc 1 Pulse Width", "Osc 1+2 Pulse Width", "Osc 2 Pulse Width", "Sync Phase"};
static const char *const kListLfoMode[2]                = {"Poly", "Mono"};
static const char *const kListNegPos[2]                 = {"Negative", "Positive"};
static const char *const kListNumDelayClockdividers[17] = {"Off", "1/64", "1/32", "1/16", "1/8", "1/4", "1/2", "3/64", "3/32", "3/16", "3/8", "1/24", "1/12", "1/6", "1/3", "2/3", "3/4"};
static const char *const kListNumLFOClockdividers[22]   = {"Off", "1/64", "1/32", "1/16", "1/8", "1/4", "1/2", "3/64", "3/32", "3/16", "3/8", "1/24", "1/12", "1/6", "1/3", "2/3", "3/4", "1/1", "2/1", "4/1", "8/1", "16/1"};
static const char *const kListOscFMMode[7]              = {"Pos Triangle", "Triangle", "Wave", "Noise", "In L", "In L+R", "In R"};
static const char *const kListPhaserMode[6]             = {"1 Stage", "2 Stages", "3 Stages", "4 Stages", "5 Stages", "6 Stages"};
static const char *const kListReverbMode[4]             = {"Off", "Reverb", "Feedback 1", "Feedback 2"};
static const char *const kListReverbType[4]             = {"Ambience", "Small Room", "Large Room", "Hall"};
static const char *const kListTapeDelayClock[9]         = {"1/32", "1/16", "2/16", "3/16", "4/16", "5/16", "6/16", "7/16", "8/16"};
static const tTiControl  kNewKeyAndMod[]                = {
    {kTiParamA21Osc1Keyfollow,  "Key 1",   eTiControlDial,   127, -64.0, NULL          },
    {kTiParamA31Osc2Keyfollow,  "Key 2",   eTiControlDial,   127, -64.0, NULL          },
    {kTiParamA29Osc2FiltEnvAmt, "2 Env",   eTiControlDial,   127, -64.0, NULL          },
    {kTiParamA30FmFiltEnvAmt,   "FM Env",  eTiControlDial,   127, -64.0, NULL          },
    {kTiParamB34OscFmMode,      "FM Mode", eTiControlChoice,   6,   0.0, kListOscFMMode},
    {kTiParamB35OscInitPhase,   "Phase",   eTiControlDial,   127,   0.0, NULL          },
};
static const tTiControl  kNewEnvelope[]                 = {
    {kTiParamB30Filter1EnvPolarity, "Env 1",    eTiControlChoice,   1, 0.0, kListNegPos},
    {kTiParamB31Filter2EnvPolarity, "Env 2",    eTiControlChoice,   1, 0.0, kListNegPos},
    {kTiParamB33FilterKeytrackBase, "Key Base", eTiControlDial,   127, 0.0, NULL       },
};
static const tTiControl  kNewVelocity[]                 = {
    {kTiParamB47Osc1ShapeVelocity,  "Shape 1", eTiControlDial, 127, -64.0, NULL},
    {kTiParamB48Osc2ShapeVelocity,  "Shape 2", eTiControlDial, 127, -64.0, NULL},
    {kTiParamB49PulsewidthVelocity, "PW",      eTiControlDial, 127, -64.0, NULL},
    {kTiParamB50FmAmountVelocity,   "FM",      eTiControlDial, 127, -64.0, NULL},
    {kTiParamB54Flt1EnvamtVelocity, "Env 1",   eTiControlDial, 127, -64.0, NULL},
    {kTiParamB55Flt2EnvamtVelocity, "Env 2",   eTiControlDial, 127, -64.0, NULL},
    {kTiParamB56Resonance1Velocity, "Reso 1",  eTiControlDial, 127, -64.0, NULL},
    {kTiParamB57Resonance2Velocity, "Reso 2",  eTiControlDial, 127, -64.0, NULL},
    {kTiParamB60AmpVelocity,        "Amp",     eTiControlDial, 127, -64.0, NULL},
    {kTiParamB61PanoramaVelocity,   "Pan",     eTiControlDial, 127, -64.0, NULL},
};
static const tTiControl  kNewLfoAssign[]                = {
    {kTiParamB79Lfo1AssignDest,   "1 Dest",   eTiControlChoice, 127,   0.0, kListDest},
    {kTiParamB80Lfo1AssignAmount, "1 Amount", eTiControlDial,   127, -64.0, NULL     },
    {kTiParamB81Lfo2AssignDest,   "2 Dest",   eTiControlChoice, 127,   0.0, kListDest},
    {kTiParamB82Lfo2AssignAmount, "2 Amount", eTiControlDial,   127, -64.0, NULL     },
};
static const tTiControl  kNewControl[]                  = {
    {kTiParamB16ClockTempo,        "Tempo",   eTiControlDial,   127,   0.0, NULL                  },
    {kTiParamB25ControlSmoothMode, "Smooth",  eTiControlChoice,  20,   0.0, kListControlSmoothMode},
    {kTiParamB26BenderRangeUp,     "Bend Up", eTiControlDial,   127, -64.0, NULL                  },
    {kTiParamB27BenderRangeDown,   "Bend Dn", eTiControlDial,   127, -64.0, NULL                  },
};
static const tTiControl  kNewOsc1Table[]                = {
    {kTiParamC37Osc1WavetableSync,           "Sync",    eTiControlDial, 127,   0.0, NULL},
    {kTiParamC42Osc1WavetableFormantshift,   "Formant", eTiControlDial, 127, -64.0, NULL},
    {kTiParamC43Osc1WavetableInternalDetune, "Detune",  eTiControlDial, 127,   0.0, NULL},
    {kTiParamC44Osc1WavetableInterpolation,  "Smooth",  eTiControlDial, 127,   0.0, NULL},
};
static const tTiControl  kNewOsc2Table[]                = {
    {kTiParamC57Osc2WavetableSync,           "Sync",    eTiControlDial, 127,   0.0, NULL},
    {kTiParamC62Osc2WavetableFormantshift,   "Formant", eTiControlDial, 127, -64.0, NULL},
    {kTiParamC63Osc2WavetableInternalDetune, "Detune",  eTiControlDial, 127,   0.0, NULL},
    {kTiParamC64Osc2WavetableInterpolation,  "Smooth",  eTiControlDial, 127,   0.0, NULL},
};
static const tTiControl  kNewDistortion[]               = {
    {kTiParamB100DistortionCurve,             "Curve",    eTiControlChoice,  25,   0.0, kListNameDistortionModes},
    {kTiParamB101DistortionIntensity,         "Amount",   eTiControlDial,   127,   0.0, NULL                    },
    {kTiParamC70PatchDistortionTrebleBooster, "Treble",   eTiControlDial,   127,   0.0, NULL                    },
    {kTiParamC71PatchDistortionHighCut,       "High Cut", eTiControlDial,   127,   0.0, NULL                    },
    {kTiParamC72PatchDistortionMix,           "Mix",      eTiControlDial,   127,   0.0, NULL                    },
    {kTiParamC73PatchDistortionQuality,       "Quality",  eTiControlDial,   127,   0.0, NULL                    },
    {kTiParamC74PatchDistortionTone127,       "Tone",     eTiControlDial,   127, -64.0, NULL                    },
};
static const tTiControl  kNewCharacter[]                = {
    {kTiParamC26CharacterType, "Type",   eTiControlChoice,   8,   0.0, kListCharacters},
    {kTiParamB97BassIntensity, "Amount", eTiControlDial,   127,   0.0, NULL           },
    {kTiParamB98BassTune,      "Tune",   eTiControlDial,   127, -64.0, NULL           },
};
static const tTiControl  kNewFilterbank[]               = {
    {kTiParamC19FilterBankType,          "Type",    eTiControlChoice,  11,   0.0, kListFilterBankType},
    {kTiParamC20FilterBankMix,           "Mix",     eTiControlDial,   127,   0.0, NULL               },
    {kTiParamC21FilterBankCombFrequency, "Freq",    eTiControlDial,    96,   0.0, NULL               },
    {kTiParamC22FilterBankStereoPhase,   "Stereo",  eTiControlDial,   127,   0.0, NULL               },
    {kTiParamC23FilterBankFilterType,    "Shape L", eTiControlDial,   127,   0.0, NULL               },
    {kTiParamC24FilterBankShapeR,        "Shape R", eTiControlDial,   127, -64.0, NULL               },
    {kTiParamC25FilterBankResonance,     "Reso",    eTiControlDial,   127,   0.0, NULL               },
};
static const tTiControl  kNewChorus[]                   = {
    {kTiParamA103ChorusType,     "Type",     eTiControlChoice,   6,   0.0, kListChorusType},
    {kTiParamA104ChorusMix2,     "Mix 2",    eTiControlDial,   127,   0.0, NULL           },
    {kTiParamA105ChorusMix,      "Mix",      eTiControlDial,   127,   0.0, NULL           },
    {kTiParamA106ChorusRate,     "Rate",     eTiControlDial,   127,   0.0, NULL           },
    {kTiParamA107ChorusDistance, "Distance", eTiControlDial,   127,   0.0, NULL           },
    {kTiParamA108ChorusAmount,   "Depth",    eTiControlDial,   127, -64.0, NULL           },
    {kTiParamA109ChorusFeedback, "Feedback", eTiControlDial,   127, -64.0, NULL           },
    {kTiParamA110ChorusLfoShape, "LFO",      eTiControlChoice,   5,   0.0, kListLfoShape  },
    {kTiParamA111ChorusXOver,    "X Over",   eTiControlDial,   127,   0.0, NULL           },
};
static const tTiControl  kNewPhaser[]                   = {
    {kTiParamB84PhaserMode,      "Stages",   eTiControlChoice,   5,   0.0, kListPhaserMode},
    {kTiParamB85PhaserMix,       "Mix",      eTiControlDial,   127,   0.0, NULL           },
    {kTiParamB86PhaserRate,      "Rate",     eTiControlDial,   127,   0.0, NULL           },
    {kTiParamB87PhaserDepth,     "Depth",    eTiControlDial,   127,   0.0, NULL           },
    {kTiParamB88PhaserFrequency, "Freq",     eTiControlDial,   127,   0.0, NULL           },
    {kTiParamB89PhaserFeedback,  "Feedback", eTiControlDial,   127, -64.0, NULL           },
    {kTiParamB90PhaserSpread,    "Spread",   eTiControlDial,   127,   0.0, NULL           },
};
static const tTiControl  kNewEq[]                       = {
    {kTiParamB95LoweqGain,       "Low",     eTiControlDial, 127, -64.0, NULL},
    {kTiParamB45LoweqFrequency,  "Low Hz",  eTiControlDial, 127,   0.0, NULL},
    {kTiParamB92MideqGain,       "Mid",     eTiControlDial, 127, -64.0, NULL},
    {kTiParamB93MideqFrequency,  "Mid Hz",  eTiControlDial, 127,   0.0, NULL},
    {kTiParamB94MideqQFactor,    "Mid Q",   eTiControlDial, 127,   0.0, NULL},
    {kTiParamB96HigheqGain,      "High",    eTiControlDial, 127, -64.0, NULL},
    {kTiParamB46HigheqFrequency, "High Hz", eTiControlDial, 127,   0.0, NULL},
};
static const tTiControl  kNewDelay[]                    = {
    {kTiParamA112DelayMode,          "Mode",     eTiControlChoice,  22,   0.0, kListDelayMode            },
    {kTiParamA113DelaySend,          "Send",     eTiControlDial,   127,   0.0, NULL                      },
    {kTiParamA114DelayTapeDelayTime, "Time",     eTiControlDial,   127,   0.0, NULL                      },
    {kTiParamA115DelayFeedback,      "Feedback", eTiControlDial,   127,   0.0, NULL                      },
    {kTiParamA116DlyRateRevDecay,    "Rate",     eTiControlDial,   127,   0.0, NULL                      },
    {kTiParamA117DlyDepth,           "Depth",    eTiControlDial,   127,   0.0, NULL                      },
    {kTiParamA118DelayLfoShape,      "LFO",      eTiControlChoice,   5,   0.0, kListDelayLfoShape        },
    {kTiParamA119DelayColor,         "Color",    eTiControlDial,   127, -64.0, NULL                      },
    {kTiParamB20DelayClock,          "Clock",    eTiControlChoice,  16,   0.0, kListNumDelayClockdividers},
};
static const tTiControl  kNewTape[]                     = {
    {kTiParamC10DelayType,                "Type",  eTiControlChoice,   3, 0.0, kListDelayType     },
    {kTiParamC12DelayTapeDelayRatio,      "Ratio", eTiControlChoice,   6, 0.0, kListDelayRatio    },
    {kTiParamC13DelayTapeDelayLeftClock,  "Left",  eTiControlChoice,   5, 0.0, kListTapeDelayClock},
    {kTiParamC14DelayTapeDelayRightClock, "Right", eTiControlChoice,   5, 0.0, kListTapeDelayClock},
    {kTiParamC17DelayTapeDelayBandwidth,  "Band",  eTiControlDial,   127, 0.0, NULL               },
};
static const tTiControl  kNewReverb[]                   = {
    {kTiParamC1ReverbMode,     "Mode",     eTiControlChoice,   3,   0.0, kListReverbMode           },
    {kTiParamC2ReverbSend,     "Send",     eTiControlDial,   127,   0.0, NULL                      },
    {kTiParamC3ReverbType,     "Type",     eTiControlChoice,   3,   0.0, kListReverbType           },
    {kTiParamC4ReverbTime,     "Time",     eTiControlDial,   127,   0.0, NULL                      },
    {kTiParamC5ReverbDamping,  "Damping",  eTiControlDial,   127,   0.0, NULL                      },
    {kTiParamC6ReverbColor,    "Color",    eTiControlDial,   127, -64.0, NULL                      },
    {kTiParamC7ReverbPredelay, "Predelay", eTiControlDial,    92,   0.0, NULL                      },
    {kTiParamC8ReverbClock,    "Clock",    eTiControlChoice,  16,   0.0, kListNumDelayClockdividers},
    {kTiParamC9ReverbFeedback, "Feedback", eTiControlDial,   127,   0.0, NULL                      },
};
static const tTiControl  kOsc[]                         = {
    {kTiParamOscShape,   "Shape", eTiControlDial,   127,   0.0, NULL     },
    {kTiParamPulseWidth, "Width", eTiControlDial,   127,   0.0, NULL     },
    {kTiParamOsc1Wave,   "Wave",  eTiControlChoice, 127,   0.0, kListWave},
    {kTiParamOsc1Semi,   "Semi",  eTiControlDial,   127, -64.0, NULL     },
};
static const tTiControl  kFilter[]                      = {
    {kTiParamCutoff,     "Cutoff", eTiControlDial,   127, 0.0, NULL    },
    {kTiParamResonance,  "Reso",   eTiControlDial,   127, 0.0, NULL    },
    {kTiParamFilterMode, "Mode",   eTiControlChoice,   7, 0.0, kFilters},
};
static const tTiControl  kFilter2[]                     = {
    {kTiParamCutoff2,     "Cutoff", eTiControlDial,   127, 0.0, NULL     },
    {kTiParamResonance2,  "Reso",   eTiControlDial,   127, 0.0, NULL     },
    {kTiParamFilter2Mode, "Mode",   eTiControlChoice,   3, 0.0, kFilters2},
};
static const tTiControl  kRouting[]                     = {
    {kTiParamRouting,    "Route",   eTiControlChoice,   3, 0.0, kRoutings},
    {kTiParamBalance,    "Balance", eTiControlDial,   127, 0.0, NULL     },
    {kTiParamFilterLink, "Link",    eTiControlChoice,   1, 0.0, kOffOn   },
};
static const tTiControl  kAmpEnv[]                      = {
    {kTiParamAmpAttack,      "Attack",  eTiControlDial, 127,   0.0, NULL},
    {kTiParamAmpDecay,       "Decay",   eTiControlDial, 127,   0.0, NULL},
    {kTiParamAmpSustain,     "Sustain", eTiControlDial, 127,   0.0, NULL},
    {kTiParamAmpSustainTime, "Time",    eTiControlDial, 127, -64.0, NULL},
    {kTiParamAmpRelease,     "Release", eTiControlDial, 127,   0.0, NULL},
};
static const tTiControl  kAmp[]                         = {
    {kTiParamVolume, "Volume", eTiControlDial, 127, 0.0, NULL},
    {kTiParamMaster, "Master", eTiControlDial, 127, 0.0, NULL},
};
static const tTiControl  kVoices[]                      = {
    {kTiParamVoiceLimit, "Limit",  eTiControlDial,   TI_MAX_VOICES - 1, 1.0, NULL       },
    {kTiParamArithmetic, "Maths",  eTiControlChoice,                 1, 0.0, kArithmetic},
    {kTiParamHwLink,     "Link",   eTiControlChoice,                 1, 0.0, kOffOn     },
    {kTiParamDelayOn,    "Delay",  eTiControlChoice,                 1, 0.0, kOffOn     },
    {kTiParamReverbOn,   "Reverb", eTiControlChoice,                 1, 0.0, kOffOn     },
};

static const tTiControl  kOsc2[]                        = {
    {kTiParamOsc2Shape,  "Shape",  eTiControlDial,   127,   0.0, NULL     },
    {kTiParamOsc2Width,  "Width",  eTiControlDial,   127,   0.0, NULL     },
    {kTiParamOsc2Wave,   "Wave",   eTiControlChoice, 127,   0.0, kListWave},
    {kTiParamOsc2Semi,   "Semi",   eTiControlDial,   127, -64.0, NULL     },
    {kTiParamOsc2Detune, "Detune", eTiControlDial,   127,   0.0, NULL     },
    {kTiParamFm,         "FM",     eTiControlDial,   127,   0.0, NULL     },
    {kTiParamSync,       "Sync",   eTiControlChoice,   1,   0.0, kOffOn   },
};
static const tTiControl  kMix[]                         = {
    {kTiParamOscBalance, "Balance", eTiControlDial,   127, 0.0, NULL      },
    {kTiParamOscVolume,  "Volume",  eTiControlDial,   127, 0.0, NULL      },
    {kTiParamSub,        "Sub",     eTiControlDial,   127, 0.0, NULL      },
    {kTiParamSubShape,   "Shape",   eTiControlChoice,   1, 0.0, kSubShapes},
    {kTiParamNoise,      "Noise",   eTiControlDial,   127, 0.0, NULL      },
    {kTiParamNoiseColor, "Color",   eTiControlDial,   127, 0.0, NULL      },
    {kTiParamRing,       "Ring",    eTiControlDial,   127, 0.0, NULL      },
    {kTiParamPunch,      "Punch",   eTiControlDial,   127, 0.0, NULL      },
};
static const tTiControl  kFilterMod[]                   = {
    {kTiParamEnvAmount1, "Env 1", eTiControlDial,   127,   0.0, NULL        },
    {kTiParamEnvAmount2, "Env 2", eTiControlDial,   127,   0.0, NULL        },
    {kTiParamKey1,       "Key 1", eTiControlDial,   127, -64.0, NULL        },
    {kTiParamKey2,       "Key 2", eTiControlDial,   127, -64.0, NULL        },
    {kTiParamSaturation, "Sat",   eTiControlChoice,  14,   0.0, kSaturations},
};
static const tTiControl  kFilterEnv[]                   = {
    {kTiParamFilterAttack,      "Attack",  eTiControlDial, 127,   0.0, NULL},
    {kTiParamFilterDecay,       "Decay",   eTiControlDial, 127,   0.0, NULL},
    {kTiParamFilterSustain,     "Sustain", eTiControlDial, 127,   0.0, NULL},
    {kTiParamFilterSustainTime, "Time",    eTiControlDial, 127, -64.0, NULL},
    {kTiParamFilterRelease,     "Release", eTiControlDial, 127,   0.0, NULL},
};
static const tTiControl  kLfo1[]                        = {
    {kTiParamLfo1Rate,              "Rate",     eTiControlDial,   127,   0.0, NULL                    },
    {kTiParamLfo1Shape,             "Shape",    eTiControlChoice,  67,   0.0, kListLfoShape           },
    {kTiParamLfo1Pitch,             "Pitch",    eTiControlDial,   127, -64.0, NULL                    },
    {kTiParamA69Lfo1EnvMode,        "Env Mode", eTiControlChoice,   1,   0.0, kOffOn                  },
    {kTiParamA70Lfo1Mode,           "Mode",     eTiControlChoice,   1,   0.0, kListLfoMode            },
    {kTiParamA71Lfo1Symmetry,       "Symmetry", eTiControlDial,   127, -64.0, NULL                    },
    {kTiParamA72Lfo1Keyfollow,      "Key",      eTiControlDial,   127,   0.0, NULL                    },
    {kTiParamA73Lfo1Keytrigger,     "Trigger",  eTiControlDial,   127,   0.0, NULL                    },
    {kTiParamB18Lfo1Clock,          "Clock",    eTiControlChoice,  21,   0.0, kListNumLFOClockdividers},
    {kTiParamA75Osc2Lfo1Amount,     "Osc 2",    eTiControlDial,   127, -64.0, NULL                    },
    {kTiParamA76PwLfo1Amount,       "PW",       eTiControlDial,   127, -64.0, NULL                    },
    {kTiParamA77ResoLfo1Amount,     "Reso",     eTiControlDial,   127, -64.0, NULL                    },
    {kTiParamA78FiltgainLfo1Amount, "Gain",     eTiControlDial,   127, -64.0, NULL                    },
};
static const tTiControl  kLfo2[]                        = {
    {kTiParamLfo2Rate,             "Rate",     eTiControlDial,   127,   0.0, NULL                    },
    {kTiParamLfo2Shape,            "Shape",    eTiControlChoice,  67,   0.0, kListLfoShape           },
    {kTiParamLfo2Cutoff,           "Cutoff",   eTiControlDial,   127, -64.0, NULL                    },
    {kTiParamA81Lfo2EnvMode,       "Env Mode", eTiControlChoice,   1,   0.0, kOffOn                  },
    {kTiParamA82Lfo2Mode,          "Mode",     eTiControlChoice,   1,   0.0, kListLfoMode            },
    {kTiParamA83Lfo2Symmetry,      "Symmetry", eTiControlDial,   127, -64.0, NULL                    },
    {kTiParamA84Lfo2Keyfollow,     "Key",      eTiControlDial,   127,   0.0, NULL                    },
    {kTiParamA85Lfo2Keytrigger,    "Trigger",  eTiControlDial,   127,   0.0, NULL                    },
    {kTiParamB19Lfo2Clock,         "Clock",    eTiControlChoice,  21,   0.0, kListNumLFOClockdividers},
    {kTiParamA86ShapeLfo2Amount,   "Shape",    eTiControlDial,   127, -64.0, NULL                    },
    {kTiParamA87FmLfo2Amount,      "FM",       eTiControlDial,   127, -64.0, NULL                    },
    {kTiParamA89Cutoff2Lfo2Amount, "Cutoff 2", eTiControlDial,   127, -64.0, NULL                    },
    {kTiParamA90PanLfo2Amount,     "Pan",      eTiControlDial,   127, -64.0, NULL                    },
};
static const tTiControl  kPlay[]                        = {
    {kTiParamKeyMode,    "Keys",  eTiControlChoice,   5,   0.0, kKeyModes},
    {kTiParamPortamento, "Porta", eTiControlDial,   127,   0.0, NULL     },
    {kTiParamPan,        "Pan",   eTiControlDial,   127, -64.0, NULL     },
    {kTiParamTranspose,  "Trans", eTiControlDial,   127, -64.0, NULL     },
};

static const tTiControl  kOsc3[]                        = {
    {kTiParamOsc3Mode,   "Mode",   eTiControlDial,  67,   0.0, NULL},
    {kTiParamOsc3Volume, "Volume", eTiControlDial, 127,   0.0, NULL},
    {kTiParamOsc3Semi,   "Semi",   eTiControlDial, 127, -64.0, NULL},
    {kTiParamOsc3Detune, "Detune", eTiControlDial, 127,   0.0, NULL},
};
static const tTiControl  kModelsCtl[]                   = {
    {kTiParamOsc1Model, "Osc 1", eTiControlChoice, 7, 0.0, kModels},
    {kTiParamOsc2Model, "Osc 2", eTiControlChoice, 7, 0.0, kModels},
};
static const tTiControl  kLfo3[]                        = {
    {kTiParamLfo3Rate,           "Rate",   eTiControlDial,   127, 0.0, NULL                    },
    {kTiParamLfo3Shape,          "Shape",  eTiControlChoice,  67, 0.0, kListLfoShape           },
    {kTiParamLfo3Amount,         "Amount", eTiControlDial,   127, 0.0, NULL                    },
    {kTiParamLfo3Fade,           "Fade",   eTiControlDial,   127, 0.0, NULL                    },
    {kTiParamB9Lfo3Mode,         "Mode",   eTiControlChoice,   1, 0.0, kListLfoMode            },
    {kTiParamB10Lfo3Keyfollow,   "Key",    eTiControlDial,   127, 0.0, NULL                    },
    {kTiParamB11Lfo3Destination, "Dest",   eTiControlChoice,   6, 0.0, kListLfoDest            },
    {kTiParamB21Lfo3Clock,       "Clock",  eTiControlChoice,  21, 0.0, kListNumLFOClockdividers},
};
static const tTiControl  kEnv3[]                        = {
    {kTiParamEnv3Attack,      "Attack",  eTiControlDial, 127,   0.0, NULL},
    {kTiParamEnv3Decay,       "Decay",   eTiControlDial, 127,   0.0, NULL},
    {kTiParamEnv3Sustain,     "Sustain", eTiControlDial, 127,   0.0, NULL},
    {kTiParamEnv3SustainTime, "Time",    eTiControlDial, 127, -64.0, NULL},
    {kTiParamEnv3Release,     "Release", eTiControlDial, 127,   0.0, NULL},
};
static const tTiControl  kEnv4[]                        = {
    {kTiParamEnv4Attack,      "Attack",  eTiControlDial, 127,   0.0, NULL},
    {kTiParamEnv4Decay,       "Decay",   eTiControlDial, 127,   0.0, NULL},
    {kTiParamEnv4Sustain,     "Sustain", eTiControlDial, 127,   0.0, NULL},
    {kTiParamEnv4SustainTime, "Time",    eTiControlDial, 127, -64.0, NULL},
    {kTiParamEnv4Release,     "Release", eTiControlDial, 127,   0.0, NULL},
};
static const tTiControl  kUnisonCtl[]                   = {
    {kTiParamUnisonVoices, "Voices", eTiControlChoice,  15,   0.0, kUnison},
    {kTiParamUnisonDetune, "Detune", eTiControlDial,   127,   0.0, NULL   },
    {kTiParamUnisonSpread, "Spread", eTiControlDial,   127,   0.0, NULL   },
    {kTiParamUnisonPhase,  "Phase",  eTiControlDial,   127, -64.0, NULL   },
};
static const tTiControl  kMod1[]                        = {
    {kTiParamMod1Source,             "Source", eTiControlChoice,  39,   0.0, kListSource},
    {kTiParamMod1Dest,               "Dest",   eTiControlChoice, 127,   0.0, kListDest  },
    {kTiParamMod1Amount,             "Amount", eTiControlDial,   127, -64.0, NULL       },
    {kTiParamC90Assign1Destination2, "Dest 2", eTiControlChoice, 127,   0.0, kListDest  },
    {kTiParamC91Assign1Amount2,      "Amt 2",  eTiControlDial,   127, -64.0, NULL       },
    {kTiParamC92Assign1Destination3, "Dest 3", eTiControlChoice, 127,   0.0, kListDest  },
    {kTiParamC93Assign1Amount3,      "Amt 3",  eTiControlDial,   127, -64.0, NULL       },
};
static const tTiControl  kMod2[]                        = {
    {kTiParamMod2Source,             "Source", eTiControlChoice,  39,   0.0, kListSource},
    {kTiParamMod2Dest,               "Dest",   eTiControlChoice, 127,   0.0, kListDest  },
    {kTiParamMod2Amount,             "Amount", eTiControlDial,   127, -64.0, NULL       },
    {kTiParamB70Assign2Destination2, "Dest 2", eTiControlChoice, 127,   0.0, kListDest  },
    {kTiParamB71Assign2Amount2,      "Amt 2",  eTiControlDial,   127, -64.0, NULL       },
    {kTiParamC94Assign2Destination3, "Dest 3", eTiControlChoice, 127,   0.0, kListDest  },
    {kTiParamC95Assign2Amount3,      "Amt 3",  eTiControlDial,   127, -64.0, NULL       },
};
static const tTiControl  kMod3[]                        = {
    {kTiParamMod3Source,             "Source", eTiControlChoice,  39,   0.0, kListSource},
    {kTiParamMod3Dest,               "Dest",   eTiControlChoice, 127,   0.0, kListDest  },
    {kTiParamMod3Amount,             "Amount", eTiControlDial,   127, -64.0, NULL       },
    {kTiParamB75Assign3Destination2, "Dest 2", eTiControlChoice, 127,   0.0, kListDest  },
    {kTiParamB76Assign3Amount2,      "Amt 2",  eTiControlDial,   127, -64.0, NULL       },
    {kTiParamB77Assign3Destination3, "Dest 3", eTiControlChoice, 127,   0.0, kListDest  },
    {kTiParamB78Assign3Amount3,      "Amt 3",  eTiControlDial,   127, -64.0, NULL       },
};
static const tTiControl  kMod4[]                        = {
    {kTiParamMod4Source,             "Source", eTiControlChoice,  39,   0.0, kListSource},
    {kTiParamMod4Dest,               "Dest",   eTiControlChoice, 127,   0.0, kListDest  },
    {kTiParamMod4Amount,             "Amount", eTiControlDial,   127, -64.0, NULL       },
    {kTiParamC96Assign4Destination2, "Dest 2", eTiControlChoice, 127,   0.0, kListDest  },
    {kTiParamC97Assign4Amount2,      "Amt 2",  eTiControlDial,   127, -64.0, NULL       },
    {kTiParamC98Assign4Destination3, "Dest 3", eTiControlChoice, 127,   0.0, kListDest  },
    {kTiParamC99Assign4Amount3,      "Amt 3",  eTiControlDial,   127, -64.0, NULL       },
};
static const tTiControl  kMod5[]                        = {
    {kTiParamMod5Source,              "Source", eTiControlChoice,  39,   0.0, kListSource},
    {kTiParamMod5Dest,                "Dest",   eTiControlChoice, 127,   0.0, kListDest  },
    {kTiParamMod5Amount,              "Amount", eTiControlDial,   127, -64.0, NULL       },
    {kTiParamC100Assign5Destination2, "Dest 2", eTiControlChoice, 127,   0.0, kListDest  },
    {kTiParamC101Assign5Amount2,      "Amt 2",  eTiControlDial,   127, -64.0, NULL       },
    {kTiParamC102Assign5Destination3, "Dest 3", eTiControlChoice, 127,   0.0, kListDest  },
    {kTiParamC103Assign5Amount3,      "Amt 3",  eTiControlDial,   127, -64.0, NULL       },
};
static const tTiControl  kMod6[]                        = {
    {kTiParamMod6Source,              "Source", eTiControlChoice,  39,   0.0, kListSource},
    {kTiParamMod6Dest,                "Dest",   eTiControlChoice, 127,   0.0, kListDest  },
    {kTiParamMod6Amount,              "Amount", eTiControlDial,   127, -64.0, NULL       },
    {kTiParamC104Assign6Destination2, "Dest 2", eTiControlChoice, 127,   0.0, kListDest  },
    {kTiParamC105Assign6Amount2,      "Amt 2",  eTiControlDial,   127, -64.0, NULL       },
    {kTiParamC106Assign6Destination3, "Dest 3", eTiControlChoice, 127,   0.0, kListDest  },
    {kTiParamC107Assign6Amount3,      "Amt 3",  eTiControlDial,   127, -64.0, NULL       },
};

// notes §2: Pulse2Alike's page names and section colours, a page only once something on it is built
static const tTiSection  kOscillatorsPage[]             = {
    {"OSC 1",       {0.30, 0.55, 0.85}, kOsc,          4},
    {"OSC 2",       {0.30, 0.55, 0.85}, kOsc2,         7},
    {"MIX",         {0.30, 0.55, 0.85}, kMix,          8},
    {"OSC 3",       {0.30, 0.55, 0.85}, kOsc3,         4},
    {"MODEL",       {0.30, 0.55, 0.85}, kModelsCtl,    2},
    {"KEY AND MOD", {0.30, 0.55, 0.85}, kNewKeyAndMod, 6},
};
static const tTiSection  kFilterPage[]                  = {
    {"FILTER 1", {0.90, 0.55, 0.20}, kFilter,      3},
    {"FILTER 2", {0.90, 0.55, 0.20}, kFilter2,     3},
    {"ROUTING",  {0.90, 0.55, 0.20}, kRouting,     3},
    {"MOD",      {0.90, 0.55, 0.20}, kFilterMod,   5},
    {"ENVELOPE", {0.90, 0.55, 0.20}, kNewEnvelope, 3},
};
static const tTiSection  kAmpPage[]                     = {
    {"OUTPUT",   {0.65, 0.65, 0.65}, kAmp,          2},
    {"VELOCITY", {0.65, 0.65, 0.65}, kNewVelocity, 10},
};
static const tTiSection  kEnvelopesPage[]               = {
    {"AMP ENV",    {0.85, 0.75, 0.25}, kAmpEnv,    5},
    {"FILTER ENV", {0.85, 0.75, 0.25}, kFilterEnv, 5},
    {"ENV 3",      {0.85, 0.75, 0.25}, kEnv3,      5},
    {"ENV 4",      {0.85, 0.75, 0.25}, kEnv4,      5},
};
static const tTiSection  kLfoPage[]                     = {
    {"LFO 1",      {0.45, 0.75, 0.45}, kLfo1,         13},
    {"LFO 2",      {0.45, 0.75, 0.45}, kLfo2,         13},
    {"LFO 3",      {0.45, 0.75, 0.45}, kLfo3,          8},
    {"LFO ASSIGN", {0.45, 0.75, 0.45}, kNewLfoAssign,  4},
};
static const tTiSection  kModPage[]                     = {
    {"SLOT 1", {0.70, 0.45, 0.80}, kMod1, 7},
    {"SLOT 2", {0.70, 0.45, 0.80}, kMod2, 7},
    {"SLOT 3", {0.70, 0.45, 0.80}, kMod3, 7},
};
static const tTiSection  kModPage2[]                    = {
    {"SLOT 4", {0.70, 0.45, 0.80}, kMod4, 7},
    {"SLOT 5", {0.70, 0.45, 0.80}, kMod5, 7},
    {"SLOT 6", {0.70, 0.45, 0.80}, kMod6, 7},
};
static const tTiSection  kGlobalPage[]                  = {
    {"PLUG-IN", {0.65, 0.65, 0.65}, kVoices,     5},
    {"PLAY",    {0.65, 0.65, 0.65}, kPlay,       4},
    {"UNISON",  {0.65, 0.65, 0.65}, kUnisonCtl,  4},
    {"CONTROL", {0.65, 0.65, 0.65}, kNewControl, 4},
};

static const tTiSection  kTablesPage[]                  = {
    {"OSC 1 TABLE", {0.30, 0.55, 0.85}, kNewOsc1Table, 4},
    {"OSC 2 TABLE", {0.30, 0.55, 0.85}, kNewOsc2Table, 4},
};
static const tTiSection  kDistortionPage[]              = {
    {"DISTORTION", {0.85, 0.35, 0.30}, kNewDistortion, 7},
    {"CHARACTER",  {0.85, 0.35, 0.30}, kNewCharacter,  3},
    {"FILTERBANK", {0.85, 0.35, 0.30}, kNewFilterbank, 7},
};
static const tTiSection  kModulationPage[]              = {
    {"CHORUS", {0.35, 0.75, 0.75}, kNewChorus, 9},
    {"PHASER", {0.35, 0.75, 0.75}, kNewPhaser, 7},
    {"EQ",     {0.35, 0.75, 0.75}, kNewEq,     7},
};
static const tTiSection  kDelayPage[]                   = {
    {"DELAY",  {0.70, 0.60, 0.90}, kNewDelay,  9},
    {"TAPE",   {0.70, 0.60, 0.90}, kNewTape,   5},
    {"REVERB", {0.70, 0.60, 0.90}, kNewReverb, 9},
};
const tTiTableNames      gTiTableNames[2]               = {
    {kTiParamOsc1Wave, kTiParamOsc1Model, kListWavetable},
    {kTiParamOsc2Wave, kTiParamOsc2Model, kListWavetable},
};
const tTiPage            gTiPages[]                     = {
    {"OSC",     kOscillatorsPage, 6},
    {"TABLES",  kTablesPage,      2},
    {"FILTER",  kFilterPage,      5},
    {"AMP",     kAmpPage,         2},
    {"ENV",     kEnvelopesPage,   4},
    {"LFO",     kLfoPage,         4},
    {"MOD 1-3", kModPage,         3},
    {"MOD 4-6", kModPage2,        3},
    {"DRIVE",   kDistortionPage,  3},
    {"MOD FX",  kModulationPage,  3},
    {"EFFECTS", kDelayPage,       3},
    {"GLOBAL",  kGlobalPage,      4},
};
const uint32_t           gTiPageCount                   = sizeof(gTiPages) / sizeof(gTiPages[0]);
