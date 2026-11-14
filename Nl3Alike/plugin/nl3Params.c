/*
 * Nl3Alike - the parameters and the panel pages.
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
// Notes: Docs/code-notes/nl3Params.md - "// notes §k" refers there.

#include <math.h>
#include <stddef.h>
#include <string.h>

#include "nl3Params.h"
#include "nl3Voices.h"

#define LIST    SYNTHLIB_PARAM_LIST

// notes §1: the panel's 0..127 is each dial's plain range, as on the instrument; morph offsets -128..127
const tSynthLibParam     gNl3Params[kNl3ParamCount] = {
    {kNl3ParamVoiceLimit,                       "Voice Limit",                            "Voices",   eSynthLibUnitGeneric,    1.0, NL3_MAX_VOICES, 23.0 / (NL3_MAX_VOICES - 1), NL3_MAX_VOICES - 1, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamMaster,                           "Master Volume",                          "Master",   eSynthLibUnitGeneric,    0.0,          127.0,                         1.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamArithmetic,                       "Arithmetic",                             "Maths",    eSynthLibUnitIndexed,    0.0,            1.0,                         0.0,                  1, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kNl3ParamAliasing,                         "Anti-alias",                             "AA",       eSynthLibUnitIndexed,    0.0,            1.0,                         1.0,                  1, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kNl3ParamProgram,                          "Program",                                "Program",  eSynthLibUnitGeneric,    0.0,          127.0,                         0.0,                127, SYNTHLIB_MIDI_NONE,       SYNTHLIB_PARAM_PROGRAM | SYNTHLIB_PARAM_HIDDEN | SYNTHLIB_PARAM_NO_SAVE},
    {kNl3ParamBank,                             "Bank Select",                            "Bank",     eSynthLibUnitGeneric,    0.0,          127.0,                         0.0,                127,                       32, SYNTHLIB_PARAM_HIDDEN | SYNTHLIB_PARAM_NO_SAVE                         },
    {kNl3ParamPitchBend,                        "Pitch Bend",                             "Bend",     eSynthLibUnitGeneric,    0.0,            1.0,                         0.5,                  0, SYNTHLIB_MIDI_PITCH_BEND, SYNTHLIB_PARAM_HIDDEN | SYNTHLIB_PARAM_NO_SAVE                         },
    {kNl3ParamModWheel,                         "Mod Wheel",                              "Wheel",    eSynthLibUnitGeneric,    0.0,            1.0,                         0.0,                  0, SYNTHLIB_MIDI_MOD_WHEEL,  SYNTHLIB_PARAM_HIDDEN | SYNTHLIB_PARAM_NO_SAVE                         },
    {kNl3ParamSustainPedal,                     "Sustain Pedal",                          "Pedal",    eSynthLibUnitGeneric,    0.0,            1.0,                         0.0,                  0, SYNTHLIB_MIDI_SUSTAIN,    SYNTHLIB_PARAM_HIDDEN | SYNTHLIB_PARAM_NO_SAVE                         },
    {kNl3ParamHwLink,                           "Link Hardware",                          "Link",     eSynthLibUnitIndexed,    0.0,            1.0,                         0.0,                  1, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kNl3ParamHwChannel,                        "Hardware MIDI Channel",                  "Channel",  eSynthLibUnitGeneric,    1.0,           16.0,                  1.0 / 15.0,                 15, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamHwDevice,                         "Hardware Device ID",                     "Device",   eSynthLibUnitGeneric,    0.0,          127.0,                         1.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamOsc1Waveform,                     "Osc 1 Waveform",                         "Wave",     eSynthLibUnitIndexed,    0.0,            5.0,                    0.000000,                  5, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kNl3ParamOsc1Shape,                        "Osc 1 Shape",                            "Shape",    eSynthLibUnitGeneric,    0.0,          127.0,                    0.000000,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamOsc1Sync,                         "Osc 1 Sync",                             "Sync",     eSynthLibUnitIndexed,    0.0,            1.0,                    0.000000,                  1, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kNl3ParamOsc1SyncTune,                     "Osc 1 Sync Detune",                      "Detune",   eSynthLibUnitGeneric,    0.0,          127.0,                    0.000000,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamOsc1NoiseSeed,                    "Osc 1 Sync Noise Timbre",                "Timbre",   eSynthLibUnitGeneric,    0.0,          127.0,                    0.000000,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamOsc1FmAmount,                     "Osc 1 FM Amount",                        "FM",       eSynthLibUnitGeneric,    0.0,          127.0,                    0.000000,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamOsc2Waveform,                     "Osc 2 Waveform",                         "Wave",     eSynthLibUnitIndexed,    0.0,            5.0,                    0.000000,                  5, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kNl3ParamOsc2Shape,                        "Osc 2 Shape",                            "Shape",    eSynthLibUnitGeneric,    0.0,          127.0,                    0.000000,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamOsc2Sync,                         "Osc 2 Sync",                             "Sync",     eSynthLibUnitIndexed,    0.0,            1.0,                    0.000000,                  1, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kNl3ParamOsc2Coarse,                       "Osc 2 Coarse Tune",                      "Coarse",   eSynthLibUnitGeneric,    0.0,          127.0,                    0.503937,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamOsc2Fine,                         "Osc 2 Fine Tune",                        "Fine",     eSynthLibUnitGeneric,    0.0,          127.0,                    0.503937,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamOsc2Kbt,                          "Osc 2 Keyboard Track Off",               "KBT Off",  eSynthLibUnitIndexed,    0.0,            1.0,                    0.000000,                  1, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kNl3ParamOsc2Partial,                      "Osc 2 Partial",                          "Partial",  eSynthLibUnitIndexed,    0.0,            1.0,                    0.000000,                  1, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kNl3ParamOsc2SyncTune,                     "Osc 2 Sync Detune",                      "Detune",   eSynthLibUnitGeneric,    0.0,          127.0,                    0.000000,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamOsc2NoiseSeed,                    "Osc 2 Sync Noise Timbre",                "Timbre",   eSynthLibUnitGeneric,    0.0,          127.0,                    0.000000,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamOsc2FmAmount,                     "Osc 2 FM Amount",                        "FM",       eSynthLibUnitGeneric,    0.0,          127.0,                    0.000000,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamOsc2Carrier,                      "Osc 2 Dual Sine Carrier",                "Carrier",  eSynthLibUnitGeneric,    0.0,          127.0,                    0.157480,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamOsc2Modulator,                    "Osc 2 Dual Sine Modulator",              "Mod",      eSynthLibUnitGeneric,    0.0,          127.0,                    0.165354,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamOsc2NoiseType,                    "Osc 2 Noise Type",                       "N Type",   eSynthLibUnitGeneric,    0.0,          127.0,                    0.503937,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamOsc2NoiseFreq,                    "Osc 2 Noise Frequency",                  "N Freq",   eSynthLibUnitGeneric,    0.0,          127.0,                    0.787402,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamOscModAmount,                     "Osc Mod Amount",                         "Amount",   eSynthLibUnitGeneric,    0.0,          127.0,                    0.000000,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamOscModType,                       "Osc Mod Type",                           "Type",     eSynthLibUnitIndexed,    0.0,            5.0,                    0.600000,                  5, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kNl3ParamOscMix,                           "Osc Mix",                                "Mix",      eSynthLibUnitGeneric,    0.0,          127.0,                    0.000000,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamOscMixNoise,                      "Osc Mix Noise 3",                        "Noise3",   eSynthLibUnitIndexed,    0.0,            1.0,                    0.000000,                  1, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kNl3ParamFilterMode,                       "Filter Mode",                            "Mode",     eSynthLibUnitIndexed,    0.0,            1.0,                    0.000000,                  1, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kNl3ParamFilterType,                       "Filter Type",                            "Type",     eSynthLibUnitIndexed,    0.0,            5.0,                    0.000000,                  5, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kNl3ParamFilterSlope,                      "Filter Slope",                           "Slope",    eSynthLibUnitIndexed,    0.0,            2.0,                    0.500000,                  2, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kNl3ParamFilter2Type,                      "Multi Filter Type",                      "Multi",    eSynthLibUnitIndexed,    0.0,            5.0,                    0.000000,                  5, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kNl3ParamCutoff,                           "Filter Frequency 1",                     "Freq 1",   eSynthLibUnitGeneric,    0.0,          127.0,                    1.000000,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamCutoff2,                          "Filter Frequency 2",                     "Freq 2",   eSynthLibUnitGeneric,    0.0,          127.0,                    0.787402,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamResonance,                        "Filter Resonance",                       "Res",      eSynthLibUnitGeneric,    0.0,          127.0,                    0.000000,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamFilterDist,                       "Filter Distortion",                      "Dist",     eSynthLibUnitGeneric,    0.0,          127.0,                    0.000000,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamFilterTrack,                      "Filter Keyboard Track",                  "KB Track", eSynthLibUnitIndexed,    0.0,            1.0,                    0.000000,                  1, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kNl3ParamFilterBypass,                     "Filter Osc 2 Bypass",                    "Osc2 Byp", eSynthLibUnitIndexed,    0.0,            1.0,                    0.000000,                  1, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kNl3ParamFilter2Env,                       "Filter 2 Envelope",                      "F2 Env",   eSynthLibUnitIndexed,    0.0,            1.0,                    0.000000,                  1, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kNl3ParamFEnvAttack,                       "Filter Env Attack",                      "Attack",   eSynthLibUnitGeneric,    0.0,          127.0,                    0.000000,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamFEnvDecay,                        "Filter Env Decay",                       "Decay",    eSynthLibUnitGeneric,    0.0,          127.0,                    0.503937,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamFEnvSustain,                      "Filter Env Sustain",                     "Sustain",  eSynthLibUnitGeneric,    0.0,          127.0,                    1.000000,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamFEnvRelease,                      "Filter Env Release",                     "Release",  eSynthLibUnitGeneric,    0.0,          127.0,                    0.078740,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamFEnvAmount,                       "Filter Env Amount",                      "Amount",   eSynthLibUnitGeneric,    0.0,          127.0,                    0.000000,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamFEnvVelocity,                     "Filter Env Velocity",                    "Vel",      eSynthLibUnitIndexed,    0.0,            1.0,                    0.000000,                  1, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kNl3ParamFEnvInvert,                       "Filter Env Invert",                      "Invert",   eSynthLibUnitIndexed,    0.0,            1.0,                    0.000000,                  1, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kNl3ParamFEnvExpAttack,                    "Filter Env Exp Attack",                  "Exp Att",  eSynthLibUnitIndexed,    0.0,            1.0,                    0.000000,                  1, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kNl3ParamAmpAttack,                        "Amp Env Attack",                         "Attack",   eSynthLibUnitGeneric,    0.0,          127.0,                    0.000000,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamAmpDecay,                         "Amp Env Decay",                          "Decay",    eSynthLibUnitGeneric,    0.0,          127.0,                    0.503937,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamAmpSustain,                       "Amp Env Sustain",                        "Sustain",  eSynthLibUnitGeneric,    0.0,          127.0,                    1.000000,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamAmpRelease,                       "Amp Env Release",                        "Release",  eSynthLibUnitGeneric,    0.0,          127.0,                    0.078740,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamAmpExpAttack,                     "Amp Env Exp Attack",                     "Exp Att",  eSynthLibUnitIndexed,    0.0,            1.0,                    0.000000,                  1, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kNl3ParamLevel,                            "Output Level",                           "Level",    eSynthLibUnitGeneric,    0.0,          127.0,                    0.787402,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamLfo1Rate,                         "LFO 1 Rate",                             "Rate",     eSynthLibUnitGeneric,    0.0,          127.0,                    0.393701,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamLfo1Amount,                       "LFO 1 Amount",                           "Amount",   eSynthLibUnitGeneric,    0.0,          127.0,                    0.000000,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamLfo1Waveform,                     "LFO 1 Waveform",                         "Wave",     eSynthLibUnitIndexed,    0.0,            5.0,                    0.200000,                  5, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kNl3ParamLfo1Dest,                         "LFO 1 Destination",                      "Dest",     eSynthLibUnitIndexed,    0.0,           11.0,                    0.545455,                 11, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kNl3ParamLfo1EnvKbs,                       "LFO 1 Env/KB Sync",                      "Env/KBS",  eSynthLibUnitIndexed,    0.0,            2.0,                    0.000000,                  2, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kNl3ParamLfo1Mono,                         "LFO 1 Mono",                             "Mono",     eSynthLibUnitIndexed,    0.0,            1.0,                    0.000000,                  1, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kNl3ParamLfo1Invert,                       "LFO 1 Invert",                           "Invert",   eSynthLibUnitIndexed,    0.0,            1.0,                    0.000000,                  1, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kNl3ParamLfo1ClockSync,                    "LFO 1 Clock Sync",                       "Clock",    eSynthLibUnitIndexed,    0.0,            1.0,                    0.000000,                  1, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kNl3ParamLfo1Divisor,                      "LFO 1 Clock Divisor",                    "Divisor",  eSynthLibUnitGeneric,    0.0,          127.0,                    0.543307,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamLfo2Rate,                         "LFO 2 Rate",                             "Rate",     eSynthLibUnitGeneric,    0.0,          127.0,                    0.393701,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamLfo2Amount,                       "LFO 2 Amount",                           "Amount",   eSynthLibUnitGeneric,    0.0,          127.0,                    0.000000,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamLfo2Waveform,                     "LFO 2 Waveform",                         "Wave",     eSynthLibUnitIndexed,    0.0,            5.0,                    0.000000,                  5, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kNl3ParamLfo2Dest,                         "LFO 2 Destination",                      "Dest",     eSynthLibUnitIndexed,    0.0,           11.0,                    0.636364,                 11, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kNl3ParamLfo2EnvKbs,                       "LFO 2 Env/KB Sync",                      "Env/KBS",  eSynthLibUnitIndexed,    0.0,            2.0,                    0.000000,                  2, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kNl3ParamLfo2Mono,                         "LFO 2 Mono",                             "Mono",     eSynthLibUnitIndexed,    0.0,            1.0,                    0.000000,                  1, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kNl3ParamLfo2Invert,                       "LFO 2 Invert",                           "Invert",   eSynthLibUnitIndexed,    0.0,            1.0,                    0.000000,                  1, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kNl3ParamLfo2ClockSync,                    "LFO 2 Clock Sync",                       "Clock",    eSynthLibUnitIndexed,    0.0,            1.0,                    0.000000,                  1, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kNl3ParamLfo2Divisor,                      "LFO 2 Clock Divisor",                    "Divisor",  eSynthLibUnitGeneric,    0.0,          127.0,                    0.543307,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamMEnvAttack,                       "Mod Env Attack",                         "Attack",   eSynthLibUnitGeneric,    0.0,          127.0,                    0.000000,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamMEnvDecay,                        "Mod Env Decay/Release",                  "Dec/Rel",  eSynthLibUnitGeneric,    0.0,          127.0,                    0.503937,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamMEnvAmount,                       "Mod Env Amount",                         "Amount",   eSynthLibUnitGeneric,    0.0,          127.0,                    0.000000,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamMEnvDest,                         "Mod Env Destination",                    "Dest",     eSynthLibUnitIndexed,    0.0,           11.0,                    0.818182,                 11, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kNl3ParamMEnvInvert,                       "Mod Env Invert",                         "Invert",   eSynthLibUnitIndexed,    0.0,            1.0,                    0.000000,                  1, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kNl3ParamMEnvRelease,                      "Mod Env Decay or Release",               "Mode",     eSynthLibUnitIndexed,    0.0,            1.0,                    0.000000,                  1, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kNl3ParamMEnvRepeat,                       "Mod Env Repeat",                         "Repeat",   eSynthLibUnitIndexed,    0.0,            1.0,                    0.000000,                  1, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kNl3ParamMEnvExpAttack,                    "Mod Env Exp Attack",                     "Exp Att",  eSynthLibUnitIndexed,    0.0,            1.0,                    0.000000,                  1, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kNl3ParamGlideRate,                        "Glide Rate",                             "Rate",     eSynthLibUnitGeneric,    0.0,          127.0,                    0.503937,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamGlideMode,                        "Glide Mode",                             "Glide",    eSynthLibUnitIndexed,    0.0,            2.0,                    0.000000,                  2, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kNl3ParamVibratoRate,                      "Vibrato Rate",                           "Rate",     eSynthLibUnitGeneric,    0.0,          127.0,                    0.503937,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamVibratoAmount,                    "Vibrato Amount",                         "Amount",   eSynthLibUnitGeneric,    0.0,          127.0,                    0.472441,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamVibratoSource,                    "Vibrato Source",                         "Source",   eSynthLibUnitIndexed,    0.0,            2.0,                    0.000000,                  2, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kNl3ParamMono,                             "Mono",                                   "Mono",     eSynthLibUnitIndexed,    0.0,            1.0,                    0.000000,                  1, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kNl3ParamLegato,                           "Legato",                                 "Legato",   eSynthLibUnitIndexed,    0.0,            1.0,                    0.000000,                  1, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kNl3ParamMonoPriority,                     "Mono Note Priority",                     "Priority", eSynthLibUnitIndexed,    0.0,            2.0,                    0.000000,                  2, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kNl3ParamUnison,                           "Unison",                                 "Unison",   eSynthLibUnitIndexed,    0.0,            1.0,                    0.000000,                  1, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kNl3ParamUnisonDetune,                     "Unison Detune",                          "Detune",   eSynthLibUnitGeneric,    0.0,          127.0,                    0.503937,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamOctaveShift,                      "Octave Shift",                           "Octave",   eSynthLibUnitIndexed,    0.0,            4.0,                    0.500000,                  4, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kNl3ParamTranspose,                        "Transpose",                              "Transp",   eSynthLibUnitGeneric,    0.0,          127.0,                    0.377953,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamChordMemory,                      "Chord Memory",                           "Chord",    eSynthLibUnitIndexed,    0.0,            1.0,                    0.000000,                  1, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kNl3ParamArpRun,                           "Arpeggio Run",                           "Run",      eSynthLibUnitIndexed,    0.0,            1.0,                    0.000000,                  1, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kNl3ParamArpRate,                          "Arpeggio Rate",                          "Rate",     eSynthLibUnitGeneric,    0.0,          127.0,                    0.503937,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamArpMode,                          "Arpeggio Mode",                          "Mode",     eSynthLibUnitIndexed,    0.0,            3.0,                    0.000000,                  3, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kNl3ParamArpRange,                         "Arpeggio Range",                         "Range",    eSynthLibUnitIndexed,    0.0,            3.0,                    0.000000,                  3, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kNl3ParamArpKbdSync,                       "Arpeggio Keyboard Sync",                 "KB Sync",  eSynthLibUnitIndexed,    0.0,            1.0,                    0.000000,                  1, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kNl3ParamArpClockSync,                     "Arpeggio Clock Sync",                    "Clock",    eSynthLibUnitIndexed,    0.0,            1.0,                    0.000000,                  1, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kNl3ParamArpDivisor,                       "Arpeggio Clock Divisor",                 "Divisor",  eSynthLibUnitGeneric,    0.0,          127.0,                    0.543307,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamMorphWheel_Lfo1Rate,              "Wheel Morph Lfo1 Rate",                  "L1 Rate",  eSynthLibUnitGeneric, -128.0,          127.0,                    0.501961,                255, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamMorphWheel_Lfo1Amount,            "Wheel Morph Lfo1 Amount",                "L1 Amt",   eSynthLibUnitGeneric, -128.0,          127.0,                    0.501961,                255, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamMorphWheel_Lfo2Rate,              "Wheel Morph Lfo2 Rate",                  "L2 Rate",  eSynthLibUnitGeneric, -128.0,          127.0,                    0.501961,                255, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamMorphWheel_Lfo2Amount,            "Wheel Morph Lfo2 Amount",                "L2 Amt",   eSynthLibUnitGeneric, -128.0,          127.0,                    0.501961,                255, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamMorphWheel_ModEnvAttack,          "Wheel Morph Mod Env Attack",             "ME Att",   eSynthLibUnitGeneric, -128.0,          127.0,                    0.501961,                255, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamMorphWheel_ModEnvDecayRelease,    "Wheel Morph Mod Env Decay Release",      "ME Dec",   eSynthLibUnitGeneric, -128.0,          127.0,                    0.501961,                255, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamMorphWheel_ModEnvAmount,          "Wheel Morph Mod Env Amount",             "ME Amt",   eSynthLibUnitGeneric, -128.0,          127.0,                    0.501961,                255, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamMorphWheel_Osc2FinePitch,         "Wheel Morph Osc2 Fine Pitch",            "O2 Fine",  eSynthLibUnitGeneric, -128.0,          127.0,                    0.501961,                255, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamMorphWheel_Osc2CoarsePitch,       "Wheel Morph Osc2 Coarse Pitch",          "O2 Crs",   eSynthLibUnitGeneric, -128.0,          127.0,                    0.501961,                255, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamMorphWheel_Oscmod,                "Wheel Morph Oscmod",                     "OscMod",   eSynthLibUnitGeneric, -128.0,          127.0,                    0.501961,                255, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamMorphWheel_Oscmix,                "Wheel Morph Oscmix",                     "OscMix",   eSynthLibUnitGeneric, -128.0,          127.0,                    0.501961,                255, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamMorphWheel_Osc1Shape,             "Wheel Morph Osc1 Shape",                 "O1 Shp",   eSynthLibUnitGeneric, -128.0,          127.0,                    0.501961,                255, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamMorphWheel_Osc2Shape,             "Wheel Morph Osc2 Shape",                 "O2 Shp",   eSynthLibUnitGeneric, -128.0,          127.0,                    0.501961,                255, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamMorphWheel_AmpEnvAttack,          "Wheel Morph Amp Env Attack",             "A Att",    eSynthLibUnitGeneric, -128.0,          127.0,                    0.501961,                255, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamMorphWheel_AmpEnvDecay,           "Wheel Morph Amp Env Decay",              "A Dec",    eSynthLibUnitGeneric, -128.0,          127.0,                    0.501961,                255, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamMorphWheel_AmpEnvSustain,         "Wheel Morph Amp Env Sustain",            "A Sus",    eSynthLibUnitGeneric, -128.0,          127.0,                    0.501961,                255, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamMorphWheel_AmpEnvRelease,         "Wheel Morph Amp Env Release",            "A Rel",    eSynthLibUnitGeneric, -128.0,          127.0,                    0.501961,                255, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamMorphWheel_FiltEnvAttack,         "Wheel Morph Filt Env Attack",            "F Att",    eSynthLibUnitGeneric, -128.0,          127.0,                    0.501961,                255, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamMorphWheel_FiltEnvDecay,          "Wheel Morph Filt Env Decay",             "F Dec",    eSynthLibUnitGeneric, -128.0,          127.0,                    0.501961,                255, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamMorphWheel_FiltEnvSustain,        "Wheel Morph Filt Env Sustain",           "F Sus",    eSynthLibUnitGeneric, -128.0,          127.0,                    0.501961,                255, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamMorphWheel_FiltEnvRelease,        "Wheel Morph Filt Env Release",           "F Rel",    eSynthLibUnitGeneric, -128.0,          127.0,                    0.501961,                255, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamMorphWheel_FiltEnvAmount,         "Wheel Morph Filt Env Amount",            "F Amt",    eSynthLibUnitGeneric, -128.0,          127.0,                    0.501961,                255, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamMorphWheel_FiltFrequency1,        "Wheel Morph Filt Frequency1",            "Freq 1",   eSynthLibUnitGeneric, -128.0,          127.0,                    0.501961,                255, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamMorphWheel_FiltFrequency2,        "Wheel Morph Filt Frequency2",            "Freq 2",   eSynthLibUnitGeneric, -128.0,          127.0,                    0.501961,                255, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamMorphWheel_FiltResonance,         "Wheel Morph Filt Resonance",             "Res",      eSynthLibUnitGeneric, -128.0,          127.0,                    0.501961,                255, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamMorphWheel_OutputLevel,           "Wheel Morph Output Level",               "Level",    eSynthLibUnitGeneric, -128.0,          127.0,                    0.501961,                255, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamMorphTouch_Lfo1Rate,              "Aftertouch Morph Lfo1 Rate",             "L1 Rate",  eSynthLibUnitGeneric, -128.0,          127.0,                    0.501961,                255, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamMorphTouch_Lfo1Amount,            "Aftertouch Morph Lfo1 Amount",           "L1 Amt",   eSynthLibUnitGeneric, -128.0,          127.0,                    0.501961,                255, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamMorphTouch_Lfo2Rate,              "Aftertouch Morph Lfo2 Rate",             "L2 Rate",  eSynthLibUnitGeneric, -128.0,          127.0,                    0.501961,                255, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamMorphTouch_Lfo2Amount,            "Aftertouch Morph Lfo2 Amount",           "L2 Amt",   eSynthLibUnitGeneric, -128.0,          127.0,                    0.501961,                255, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamMorphTouch_ModEnvAttack,          "Aftertouch Morph Mod Env Attack",        "ME Att",   eSynthLibUnitGeneric, -128.0,          127.0,                    0.501961,                255, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamMorphTouch_ModEnvDecayRelease,    "Aftertouch Morph Mod Env Decay Release", "ME Dec",   eSynthLibUnitGeneric, -128.0,          127.0,                    0.501961,                255, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamMorphTouch_ModEnvAmount,          "Aftertouch Morph Mod Env Amount",        "ME Amt",   eSynthLibUnitGeneric, -128.0,          127.0,                    0.501961,                255, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamMorphTouch_Osc2FinePitch,         "Aftertouch Morph Osc2 Fine Pitch",       "O2 Fine",  eSynthLibUnitGeneric, -128.0,          127.0,                    0.501961,                255, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamMorphTouch_Osc2CoarsePitch,       "Aftertouch Morph Osc2 Coarse Pitch",     "O2 Crs",   eSynthLibUnitGeneric, -128.0,          127.0,                    0.501961,                255, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamMorphTouch_Oscmod,                "Aftertouch Morph Oscmod",                "OscMod",   eSynthLibUnitGeneric, -128.0,          127.0,                    0.501961,                255, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamMorphTouch_Oscmix,                "Aftertouch Morph Oscmix",                "OscMix",   eSynthLibUnitGeneric, -128.0,          127.0,                    0.501961,                255, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamMorphTouch_Osc1Shape,             "Aftertouch Morph Osc1 Shape",            "O1 Shp",   eSynthLibUnitGeneric, -128.0,          127.0,                    0.501961,                255, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamMorphTouch_Osc2Shape,             "Aftertouch Morph Osc2 Shape",            "O2 Shp",   eSynthLibUnitGeneric, -128.0,          127.0,                    0.501961,                255, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamMorphTouch_AmpEnvAttack,          "Aftertouch Morph Amp Env Attack",        "A Att",    eSynthLibUnitGeneric, -128.0,          127.0,                    0.501961,                255, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamMorphTouch_AmpEnvDecay,           "Aftertouch Morph Amp Env Decay",         "A Dec",    eSynthLibUnitGeneric, -128.0,          127.0,                    0.501961,                255, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamMorphTouch_AmpEnvSustain,         "Aftertouch Morph Amp Env Sustain",       "A Sus",    eSynthLibUnitGeneric, -128.0,          127.0,                    0.501961,                255, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamMorphTouch_AmpEnvRelease,         "Aftertouch Morph Amp Env Release",       "A Rel",    eSynthLibUnitGeneric, -128.0,          127.0,                    0.501961,                255, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamMorphTouch_FiltEnvAttack,         "Aftertouch Morph Filt Env Attack",       "F Att",    eSynthLibUnitGeneric, -128.0,          127.0,                    0.501961,                255, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamMorphTouch_FiltEnvDecay,          "Aftertouch Morph Filt Env Decay",        "F Dec",    eSynthLibUnitGeneric, -128.0,          127.0,                    0.501961,                255, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamMorphTouch_FiltEnvSustain,        "Aftertouch Morph Filt Env Sustain",      "F Sus",    eSynthLibUnitGeneric, -128.0,          127.0,                    0.501961,                255, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamMorphTouch_FiltEnvRelease,        "Aftertouch Morph Filt Env Release",      "F Rel",    eSynthLibUnitGeneric, -128.0,          127.0,                    0.501961,                255, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamMorphTouch_FiltEnvAmount,         "Aftertouch Morph Filt Env Amount",       "F Amt",    eSynthLibUnitGeneric, -128.0,          127.0,                    0.501961,                255, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamMorphTouch_FiltFrequency1,        "Aftertouch Morph Filt Frequency1",       "Freq 1",   eSynthLibUnitGeneric, -128.0,          127.0,                    0.501961,                255, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamMorphTouch_FiltFrequency2,        "Aftertouch Morph Filt Frequency2",       "Freq 2",   eSynthLibUnitGeneric, -128.0,          127.0,                    0.501961,                255, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamMorphTouch_FiltResonance,         "Aftertouch Morph Filt Resonance",        "Res",      eSynthLibUnitGeneric, -128.0,          127.0,                    0.501961,                255, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamMorphTouch_OutputLevel,           "Aftertouch Morph Output Level",          "Level",    eSynthLibUnitGeneric, -128.0,          127.0,                    0.501961,                255, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamMorphVelocity_Lfo1Rate,           "Velocity Morph Lfo1 Rate",               "L1 Rate",  eSynthLibUnitGeneric, -128.0,          127.0,                    0.501961,                255, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamMorphVelocity_Lfo1Amount,         "Velocity Morph Lfo1 Amount",             "L1 Amt",   eSynthLibUnitGeneric, -128.0,          127.0,                    0.501961,                255, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamMorphVelocity_Lfo2Rate,           "Velocity Morph Lfo2 Rate",               "L2 Rate",  eSynthLibUnitGeneric, -128.0,          127.0,                    0.501961,                255, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamMorphVelocity_Lfo2Amount,         "Velocity Morph Lfo2 Amount",             "L2 Amt",   eSynthLibUnitGeneric, -128.0,          127.0,                    0.501961,                255, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamMorphVelocity_ModEnvAttack,       "Velocity Morph Mod Env Attack",          "ME Att",   eSynthLibUnitGeneric, -128.0,          127.0,                    0.501961,                255, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamMorphVelocity_ModEnvDecayRelease, "Velocity Morph Mod Env Decay Release",   "ME Dec",   eSynthLibUnitGeneric, -128.0,          127.0,                    0.501961,                255, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamMorphVelocity_ModEnvAmount,       "Velocity Morph Mod Env Amount",          "ME Amt",   eSynthLibUnitGeneric, -128.0,          127.0,                    0.501961,                255, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamMorphVelocity_Osc2FinePitch,      "Velocity Morph Osc2 Fine Pitch",         "O2 Fine",  eSynthLibUnitGeneric, -128.0,          127.0,                    0.501961,                255, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamMorphVelocity_Osc2CoarsePitch,    "Velocity Morph Osc2 Coarse Pitch",       "O2 Crs",   eSynthLibUnitGeneric, -128.0,          127.0,                    0.501961,                255, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamMorphVelocity_Oscmod,             "Velocity Morph Oscmod",                  "OscMod",   eSynthLibUnitGeneric, -128.0,          127.0,                    0.501961,                255, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamMorphVelocity_Oscmix,             "Velocity Morph Oscmix",                  "OscMix",   eSynthLibUnitGeneric, -128.0,          127.0,                    0.501961,                255, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamMorphVelocity_Osc1Shape,          "Velocity Morph Osc1 Shape",              "O1 Shp",   eSynthLibUnitGeneric, -128.0,          127.0,                    0.501961,                255, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamMorphVelocity_Osc2Shape,          "Velocity Morph Osc2 Shape",              "O2 Shp",   eSynthLibUnitGeneric, -128.0,          127.0,                    0.501961,                255, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamMorphVelocity_AmpEnvAttack,       "Velocity Morph Amp Env Attack",          "A Att",    eSynthLibUnitGeneric, -128.0,          127.0,                    0.501961,                255, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamMorphVelocity_AmpEnvDecay,        "Velocity Morph Amp Env Decay",           "A Dec",    eSynthLibUnitGeneric, -128.0,          127.0,                    0.501961,                255, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamMorphVelocity_AmpEnvSustain,      "Velocity Morph Amp Env Sustain",         "A Sus",    eSynthLibUnitGeneric, -128.0,          127.0,                    0.501961,                255, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamMorphVelocity_AmpEnvRelease,      "Velocity Morph Amp Env Release",         "A Rel",    eSynthLibUnitGeneric, -128.0,          127.0,                    0.501961,                255, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamMorphVelocity_FiltEnvAttack,      "Velocity Morph Filt Env Attack",         "F Att",    eSynthLibUnitGeneric, -128.0,          127.0,                    0.501961,                255, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamMorphVelocity_FiltEnvDecay,       "Velocity Morph Filt Env Decay",          "F Dec",    eSynthLibUnitGeneric, -128.0,          127.0,                    0.501961,                255, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamMorphVelocity_FiltEnvSustain,     "Velocity Morph Filt Env Sustain",        "F Sus",    eSynthLibUnitGeneric, -128.0,          127.0,                    0.501961,                255, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamMorphVelocity_FiltEnvRelease,     "Velocity Morph Filt Env Release",        "F Rel",    eSynthLibUnitGeneric, -128.0,          127.0,                    0.501961,                255, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamMorphVelocity_FiltEnvAmount,      "Velocity Morph Filt Env Amount",         "F Amt",    eSynthLibUnitGeneric, -128.0,          127.0,                    0.501961,                255, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamMorphVelocity_FiltFrequency1,     "Velocity Morph Filt Frequency1",         "Freq 1",   eSynthLibUnitGeneric, -128.0,          127.0,                    0.501961,                255, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamMorphVelocity_FiltFrequency2,     "Velocity Morph Filt Frequency2",         "Freq 2",   eSynthLibUnitGeneric, -128.0,          127.0,                    0.501961,                255, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamMorphVelocity_FiltResonance,      "Velocity Morph Filt Resonance",          "Res",      eSynthLibUnitGeneric, -128.0,          127.0,                    0.501961,                255, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamMorphVelocity_OutputLevel,        "Velocity Morph Output Level",            "Level",    eSynthLibUnitGeneric, -128.0,          127.0,                    0.501961,                255, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamMorphKbd_Lfo1Rate,                "Keyboard Morph Lfo1 Rate",               "L1 Rate",  eSynthLibUnitGeneric, -128.0,          127.0,                    0.501961,                255, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamMorphKbd_Lfo1Amount,              "Keyboard Morph Lfo1 Amount",             "L1 Amt",   eSynthLibUnitGeneric, -128.0,          127.0,                    0.501961,                255, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamMorphKbd_Lfo2Rate,                "Keyboard Morph Lfo2 Rate",               "L2 Rate",  eSynthLibUnitGeneric, -128.0,          127.0,                    0.501961,                255, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamMorphKbd_Lfo2Amount,              "Keyboard Morph Lfo2 Amount",             "L2 Amt",   eSynthLibUnitGeneric, -128.0,          127.0,                    0.501961,                255, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamMorphKbd_ModEnvAttack,            "Keyboard Morph Mod Env Attack",          "ME Att",   eSynthLibUnitGeneric, -128.0,          127.0,                    0.501961,                255, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamMorphKbd_ModEnvDecayRelease,      "Keyboard Morph Mod Env Decay Release",   "ME Dec",   eSynthLibUnitGeneric, -128.0,          127.0,                    0.501961,                255, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamMorphKbd_ModEnvAmount,            "Keyboard Morph Mod Env Amount",          "ME Amt",   eSynthLibUnitGeneric, -128.0,          127.0,                    0.501961,                255, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamMorphKbd_Osc2FinePitch,           "Keyboard Morph Osc2 Fine Pitch",         "O2 Fine",  eSynthLibUnitGeneric, -128.0,          127.0,                    0.501961,                255, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamMorphKbd_Osc2CoarsePitch,         "Keyboard Morph Osc2 Coarse Pitch",       "O2 Crs",   eSynthLibUnitGeneric, -128.0,          127.0,                    0.501961,                255, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamMorphKbd_Oscmod,                  "Keyboard Morph Oscmod",                  "OscMod",   eSynthLibUnitGeneric, -128.0,          127.0,                    0.501961,                255, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamMorphKbd_Oscmix,                  "Keyboard Morph Oscmix",                  "OscMix",   eSynthLibUnitGeneric, -128.0,          127.0,                    0.501961,                255, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamMorphKbd_Osc1Shape,               "Keyboard Morph Osc1 Shape",              "O1 Shp",   eSynthLibUnitGeneric, -128.0,          127.0,                    0.501961,                255, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamMorphKbd_Osc2Shape,               "Keyboard Morph Osc2 Shape",              "O2 Shp",   eSynthLibUnitGeneric, -128.0,          127.0,                    0.501961,                255, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamMorphKbd_AmpEnvAttack,            "Keyboard Morph Amp Env Attack",          "A Att",    eSynthLibUnitGeneric, -128.0,          127.0,                    0.501961,                255, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamMorphKbd_AmpEnvDecay,             "Keyboard Morph Amp Env Decay",           "A Dec",    eSynthLibUnitGeneric, -128.0,          127.0,                    0.501961,                255, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamMorphKbd_AmpEnvSustain,           "Keyboard Morph Amp Env Sustain",         "A Sus",    eSynthLibUnitGeneric, -128.0,          127.0,                    0.501961,                255, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamMorphKbd_AmpEnvRelease,           "Keyboard Morph Amp Env Release",         "A Rel",    eSynthLibUnitGeneric, -128.0,          127.0,                    0.501961,                255, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamMorphKbd_FiltEnvAttack,           "Keyboard Morph Filt Env Attack",         "F Att",    eSynthLibUnitGeneric, -128.0,          127.0,                    0.501961,                255, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamMorphKbd_FiltEnvDecay,            "Keyboard Morph Filt Env Decay",          "F Dec",    eSynthLibUnitGeneric, -128.0,          127.0,                    0.501961,                255, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamMorphKbd_FiltEnvSustain,          "Keyboard Morph Filt Env Sustain",        "F Sus",    eSynthLibUnitGeneric, -128.0,          127.0,                    0.501961,                255, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamMorphKbd_FiltEnvRelease,          "Keyboard Morph Filt Env Release",        "F Rel",    eSynthLibUnitGeneric, -128.0,          127.0,                    0.501961,                255, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamMorphKbd_FiltEnvAmount,           "Keyboard Morph Filt Env Amount",         "F Amt",    eSynthLibUnitGeneric, -128.0,          127.0,                    0.501961,                255, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamMorphKbd_FiltFrequency1,          "Keyboard Morph Filt Frequency1",         "Freq 1",   eSynthLibUnitGeneric, -128.0,          127.0,                    0.501961,                255, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamMorphKbd_FiltFrequency2,          "Keyboard Morph Filt Frequency2",         "Freq 2",   eSynthLibUnitGeneric, -128.0,          127.0,                    0.501961,                255, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamMorphKbd_FiltResonance,           "Keyboard Morph Filt Resonance",          "Res",      eSynthLibUnitGeneric, -128.0,          127.0,                    0.501961,                255, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamMorphKbd_OutputLevel,             "Keyboard Morph Output Level",            "Level",    eSynthLibUnitGeneric, -128.0,          127.0,                    0.501961,                255, SYNTHLIB_MIDI_NONE,                                                                             0},
};

// notes §3: measured on the instrument
const tNl3Field          gNl3Fields[] = {
    {kNl3ParamOsc1Waveform,                      370, 3, false},
    {kNl3ParamOsc1Shape,                          16, 7, false},
    {kNl3ParamOsc1Sync,                          373, 1, false},
    {kNl3ParamOsc1SyncTune,                      212, 7, false},
    {kNl3ParamOsc1NoiseSeed,                     226, 7, false},
    {kNl3ParamOsc1FmAmount,                      240, 7, false},
    {kNl3ParamOsc2Waveform,                      374, 3, false},
    {kNl3ParamOsc2Shape,                          37, 7, false},
    {kNl3ParamOsc2Sync,                          377, 1, false},
    {kNl3ParamOsc2Coarse,                         23, 7, false},
    {kNl3ParamOsc2Fine,                           30, 7, false},
    {kNl3ParamOsc2Kbt,                           378, 1, false},
    {kNl3ParamOsc2Partial,                       379, 1, false},
    {kNl3ParamOsc2SyncTune,                      219, 7, false},
    {kNl3ParamOsc2NoiseSeed,                     233, 7, false},
    {kNl3ParamOsc2FmAmount,                      247, 7, false},
    {kNl3ParamOsc2Carrier,                       254, 7, false},
    {kNl3ParamOsc2Modulator,                     268, 7, false},
    {kNl3ParamOsc2NoiseType,                     261, 7, false},
    {kNl3ParamOsc2NoiseFreq,                     275, 7, false},
    {kNl3ParamOscModAmount,                       51, 7, false},
    {kNl3ParamOscModType,                        380, 3, false},
    {kNl3ParamOscMix,                             44, 7, false},
    {kNl3ParamOscMixNoise,                       437, 1, false},
    {kNl3ParamFilterMode,                        428, 1, false},
    {kNl3ParamFilterType,                        417, 3, false},
    {kNl3ParamFilterSlope,                       420, 2, false},
    {kNl3ParamFilter2Type,                       430, 3, false},
    {kNl3ParamCutoff,                            177, 7, false},
    {kNl3ParamCutoff2,                           191, 7, false},
    {kNl3ParamResonance,                         184, 7, false},
    {kNl3ParamFilterDist,                        205, 7, false},
    {kNl3ParamFilterTrack,                       423, 1, false},
    {kNl3ParamFilterBypass,                      433, 1, false},
    {kNl3ParamFilter2Env,                        429, 1, false},
    {kNl3ParamFEnvAttack,                        121, 7, false},
    {kNl3ParamFEnvDecay,                         128, 7, false},
    {kNl3ParamFEnvSustain,                       135, 7, false},
    {kNl3ParamFEnvRelease,                       142, 7, false},
    {kNl3ParamFEnvAmount,                        170, 7, false},
    {kNl3ParamFEnvVelocity,                      422, 1, false},
    {kNl3ParamFEnvInvert,                        424, 1, false},
    {kNl3ParamFEnvExpAttack,                     427, 1, false},
    {kNl3ParamAmpAttack,                          86, 7, false},
    {kNl3ParamAmpDecay,                           93, 7, false},
    {kNl3ParamAmpSustain,                        100, 7, false},
    {kNl3ParamAmpRelease,                        107, 7, false},
    {kNl3ParamAmpExpAttack,                      425, 1, false},
    {kNl3ParamLevel,                             114, 7, false},
    {kNl3ParamLfo1Rate,                           58, 7, false},
    {kNl3ParamLfo1Amount,                         65, 7, false},
    {kNl3ParamLfo1Waveform,                      383, 3, false},
    {kNl3ParamLfo1Dest,                          386, 4, false},
    {kNl3ParamLfo1EnvKbs,                        390, 2, false},
    {kNl3ParamLfo1Mono,                          393, 1, false},
    {kNl3ParamLfo1Invert,                        395, 1, false},
    {kNl3ParamLfo1ClockSync,                     434, 1, false},
    {kNl3ParamLfo1Divisor,                       333, 7, false},
    {kNl3ParamLfo2Rate,                           72, 7, false},
    {kNl3ParamLfo2Amount,                         79, 7, false},
    {kNl3ParamLfo2Waveform,                      396, 3, false},
    {kNl3ParamLfo2Dest,                          399, 4, false},
    {kNl3ParamLfo2EnvKbs,                        403, 2, false},
    {kNl3ParamLfo2Mono,                          406, 1, false},
    {kNl3ParamLfo2Invert,                        408, 1, false},
    {kNl3ParamLfo2ClockSync,                     435, 1, false},
    {kNl3ParamLfo2Divisor,                       340, 7, false},
    {kNl3ParamMEnvAttack,                        149, 7, false},
    {kNl3ParamMEnvDecay,                         156, 7, false},
    {kNl3ParamMEnvAmount,                        163, 7, false},
    {kNl3ParamMEnvDest,                          411, 4, false},
    {kNl3ParamMEnvInvert,                        410, 1, false},
    {kNl3ParamMEnvRelease,                       415, 1, false},
    {kNl3ParamMEnvRepeat,                        416, 1, false},
    {kNl3ParamMEnvExpAttack,                     426, 1, false},
    {kNl3ParamGlideRate,                         298, 7, false},
    {kNl3ParamGlideMode,                         438, 2, false},
    {kNl3ParamVibratoRate,                       312, 7, false},
    {kNl3ParamVibratoAmount,                     319, 7, false},
    {kNl3ParamVibratoSource,                     440, 2, false},
    {kNl3ParamMono,                              442, 1, false},
    {kNl3ParamLegato,                            475, 1, false},
    {kNl3ParamMonoPriority,                      476, 2, false},
    {kNl3ParamUnison,                            445, 1, false},
    {kNl3ParamUnisonDetune,                      198, 7, false},
    {kNl3ParamOctaveShift,                       446, 3, false},
    {kNl3ParamTranspose,                         347, 7, false},
    {kNl3ParamChordMemory,                       449, 1, false},
    {kNl3ParamArpRun,                            443, 1, false},
    {kNl3ParamArpRate,                           305, 7, false},
    {kNl3ParamArpMode,                           450, 3, false},
    {kNl3ParamArpRange,                          453, 3, false},
    {kNl3ParamArpKbdSync,                        456, 1, false},
    {kNl3ParamArpClockSync,                      436, 1, false},
    {kNl3ParamArpDivisor,                        326, 7, false},
    {kNl3ParamMorphWheel_Lfo1Rate,               478, 8, true },
    {kNl3ParamMorphWheel_Lfo1Amount,             486, 8, true },
    {kNl3ParamMorphWheel_Lfo2Rate,               494, 8, true },
    {kNl3ParamMorphWheel_Lfo2Amount,             502, 8, true },
    {kNl3ParamMorphWheel_ModEnvAttack,           510, 8, true },
    {kNl3ParamMorphWheel_ModEnvDecayRelease,     518, 8, true },
    {kNl3ParamMorphWheel_ModEnvAmount,           526, 8, true },
    {kNl3ParamMorphWheel_Osc2FinePitch,          534, 8, true },
    {kNl3ParamMorphWheel_Osc2CoarsePitch,        542, 8, true },
    {kNl3ParamMorphWheel_Oscmod,                 550, 8, true },
    {kNl3ParamMorphWheel_Oscmix,                 558, 8, true },
    {kNl3ParamMorphWheel_Osc1Shape,              566, 8, true },
    {kNl3ParamMorphWheel_Osc2Shape,              574, 8, true },
    {kNl3ParamMorphWheel_AmpEnvAttack,           582, 8, true },
    {kNl3ParamMorphWheel_AmpEnvDecay,            590, 8, true },
    {kNl3ParamMorphWheel_AmpEnvSustain,          598, 8, true },
    {kNl3ParamMorphWheel_AmpEnvRelease,          606, 8, true },
    {kNl3ParamMorphWheel_FiltEnvAttack,          614, 8, true },
    {kNl3ParamMorphWheel_FiltEnvDecay,           622, 8, true },
    {kNl3ParamMorphWheel_FiltEnvSustain,         630, 8, true },
    {kNl3ParamMorphWheel_FiltEnvRelease,         638, 8, true },
    {kNl3ParamMorphWheel_FiltEnvAmount,          646, 8, true },
    {kNl3ParamMorphWheel_FiltFrequency1,         654, 8, true },
    {kNl3ParamMorphWheel_FiltFrequency2,         662, 8, true },
    {kNl3ParamMorphWheel_FiltResonance,          670, 8, true },
    {kNl3ParamMorphWheel_OutputLevel,            678, 8, true },
    {kNl3ParamMorphTouch_Lfo1Rate,               686, 8, true },
    {kNl3ParamMorphTouch_Lfo1Amount,             694, 8, true },
    {kNl3ParamMorphTouch_Lfo2Rate,               702, 8, true },
    {kNl3ParamMorphTouch_Lfo2Amount,             710, 8, true },
    {kNl3ParamMorphTouch_ModEnvAttack,           718, 8, true },
    {kNl3ParamMorphTouch_ModEnvDecayRelease,     726, 8, true },
    {kNl3ParamMorphTouch_ModEnvAmount,           734, 8, true },
    {kNl3ParamMorphTouch_Osc2FinePitch,          742, 8, true },
    {kNl3ParamMorphTouch_Osc2CoarsePitch,        750, 8, true },
    {kNl3ParamMorphTouch_Oscmod,                 758, 8, true },
    {kNl3ParamMorphTouch_Oscmix,                 766, 8, true },
    {kNl3ParamMorphTouch_Osc1Shape,              774, 8, true },
    {kNl3ParamMorphTouch_Osc2Shape,              782, 8, true },
    {kNl3ParamMorphTouch_AmpEnvAttack,           790, 8, true },
    {kNl3ParamMorphTouch_AmpEnvDecay,            798, 8, true },
    {kNl3ParamMorphTouch_AmpEnvSustain,          806, 8, true },
    {kNl3ParamMorphTouch_AmpEnvRelease,          814, 8, true },
    {kNl3ParamMorphTouch_FiltEnvAttack,          822, 8, true },
    {kNl3ParamMorphTouch_FiltEnvDecay,           830, 8, true },
    {kNl3ParamMorphTouch_FiltEnvSustain,         838, 8, true },
    {kNl3ParamMorphTouch_FiltEnvRelease,         846, 8, true },
    {kNl3ParamMorphTouch_FiltEnvAmount,          854, 8, true },
    {kNl3ParamMorphTouch_FiltFrequency1,         862, 8, true },
    {kNl3ParamMorphTouch_FiltFrequency2,         870, 8, true },
    {kNl3ParamMorphTouch_FiltResonance,          878, 8, true },
    {kNl3ParamMorphTouch_OutputLevel,            886, 8, true },
    {kNl3ParamMorphVelocity_Lfo1Rate,            894, 8, true },
    {kNl3ParamMorphVelocity_Lfo1Amount,          902, 8, true },
    {kNl3ParamMorphVelocity_Lfo2Rate,            910, 8, true },
    {kNl3ParamMorphVelocity_Lfo2Amount,          918, 8, true },
    {kNl3ParamMorphVelocity_ModEnvAttack,        926, 8, true },
    {kNl3ParamMorphVelocity_ModEnvDecayRelease,  934, 8, true },
    {kNl3ParamMorphVelocity_ModEnvAmount,        942, 8, true },
    {kNl3ParamMorphVelocity_Osc2FinePitch,       950, 8, true },
    {kNl3ParamMorphVelocity_Osc2CoarsePitch,     958, 8, true },
    {kNl3ParamMorphVelocity_Oscmod,              966, 8, true },
    {kNl3ParamMorphVelocity_Oscmix,              974, 8, true },
    {kNl3ParamMorphVelocity_Osc1Shape,           982, 8, true },
    {kNl3ParamMorphVelocity_Osc2Shape,           990, 8, true },
    {kNl3ParamMorphVelocity_AmpEnvAttack,        998, 8, true },
    {kNl3ParamMorphVelocity_AmpEnvDecay,        1006, 8, true },
    {kNl3ParamMorphVelocity_AmpEnvSustain,      1014, 8, true },
    {kNl3ParamMorphVelocity_AmpEnvRelease,      1022, 8, true },
    {kNl3ParamMorphVelocity_FiltEnvAttack,      1030, 8, true },
    {kNl3ParamMorphVelocity_FiltEnvDecay,       1038, 8, true },
    {kNl3ParamMorphVelocity_FiltEnvSustain,     1046, 8, true },
    {kNl3ParamMorphVelocity_FiltEnvRelease,     1054, 8, true },
    {kNl3ParamMorphVelocity_FiltEnvAmount,      1062, 8, true },
    {kNl3ParamMorphVelocity_FiltFrequency1,     1070, 8, true },
    {kNl3ParamMorphVelocity_FiltFrequency2,     1078, 8, true },
    {kNl3ParamMorphVelocity_FiltResonance,      1086, 8, true },
    {kNl3ParamMorphVelocity_OutputLevel,        1094, 8, true },
    {kNl3ParamMorphKbd_Lfo1Rate,                1102, 8, true },
    {kNl3ParamMorphKbd_Lfo1Amount,              1110, 8, true },
    {kNl3ParamMorphKbd_Lfo2Rate,                1118, 8, true },
    {kNl3ParamMorphKbd_Lfo2Amount,              1126, 8, true },
    {kNl3ParamMorphKbd_ModEnvAttack,            1134, 8, true },
    {kNl3ParamMorphKbd_ModEnvDecayRelease,      1142, 8, true },
    {kNl3ParamMorphKbd_ModEnvAmount,            1150, 8, true },
    {kNl3ParamMorphKbd_Osc2FinePitch,           1158, 8, true },
    {kNl3ParamMorphKbd_Osc2CoarsePitch,         1166, 8, true },
    {kNl3ParamMorphKbd_Oscmod,                  1174, 8, true },
    {kNl3ParamMorphKbd_Oscmix,                  1182, 8, true },
    {kNl3ParamMorphKbd_Osc1Shape,               1190, 8, true },
    {kNl3ParamMorphKbd_Osc2Shape,               1198, 8, true },
    {kNl3ParamMorphKbd_AmpEnvAttack,            1206, 8, true },
    {kNl3ParamMorphKbd_AmpEnvDecay,             1214, 8, true },
    {kNl3ParamMorphKbd_AmpEnvSustain,           1222, 8, true },
    {kNl3ParamMorphKbd_AmpEnvRelease,           1230, 8, true },
    {kNl3ParamMorphKbd_FiltEnvAttack,           1238, 8, true },
    {kNl3ParamMorphKbd_FiltEnvDecay,            1246, 8, true },
    {kNl3ParamMorphKbd_FiltEnvSustain,          1254, 8, true },
    {kNl3ParamMorphKbd_FiltEnvRelease,          1262, 8, true },
    {kNl3ParamMorphKbd_FiltEnvAmount,           1270, 8, true },
    {kNl3ParamMorphKbd_FiltFrequency1,          1278, 8, true },
    {kNl3ParamMorphKbd_FiltFrequency2,          1286, 8, true },
    {kNl3ParamMorphKbd_FiltResonance,           1294, 8, true },
    {kNl3ParamMorphKbd_OutputLevel,             1302, 8, true },
};
const uint32_t           gNl3FieldCount      = sizeof(gNl3Fields) / sizeof(gNl3Fields[0]);

// notes §4: the names of each switch's positions
static const char *const kChoiceArithmetic[] = {"FLOAT", "FIXED"};
static const char *const kChoiceAliasing[]   = {"OFF", "ON"};
static const char *const kChoiceWave[]       = {"SAW", "PULSE", "TRI", "SINE", "NOISE", "DUAL SINE"};
static const char *const kChoiceOffon[]      = {"OFF", "ON"};
static const char *const kChoiceOscmod[]     = {"DM", "DIST", "FILT", "FM", "DFM", "RM"};
static const char *const kChoiceLfowave[]    = {"TRI", "SAW", "SQUARE", "RANDOM", "SMOOTH", "SPECIAL"};
static const char *const kChoiceLfo1dest[]   = {"OSC 1", "OSC 2", "OSC 1+2", "FILT 1", "FILT 2", "FILT 1+2", "SHAPE 1", "SHAPE 2", "SHAPE 1+2", "OSCMOD", "OSCMIX", "AMP"};
static const char *const kChoiceLfo2dest[]   = {"OSC 1", "OSC 2", "OSC 1+2", "FILT 1", "FILT 2", "FILT 1+2", "SHAPE 1", "SHAPE 2", "SHAPE 1+2", "OSCMOD", "RES", "PAN"};
static const char *const kChoiceModenvdest[] = {"OSC 1", "OSC 2", "OSC 1+2", "FILT 1", "FILT 2", "FILT 1+2", "SHAPE 1", "SHAPE 2", "SHAPE 1+2", "OSCMOD", "OSCMIX", "LFO 1"};
static const char *const kChoiceEnvkbs[]     = {"OFF", "ENV", "KB SYNC"};
static const char *const kChoiceDecrel[]     = {"DECAY", "RELEASE"};
static const char *const kChoiceFtype[]      = {"LP", "HP", "BP", "BR", "CLASSIC", "DIST LP"};
static const char *const kChoiceSlope[]      = {"6 dB", "12 dB", "24 dB"};
static const char *const kChoiceFmode[]      = {"SINGLE", "MULTI"};
static const char *const kChoiceF2type[]     = {"LP-BP", "LP-HP", "HP>LP", "BP-BP", "PEAK", "NOTCH"};
static const char *const kChoiceGlide[]      = {"OFF", "ON", "AUTO"};
static const char *const kChoiceVibsrc[]     = {"OFF", "WHEEL", "A.TOUCH"};
static const char *const kChoicePrio[]       = {"OFF", "HI", "LO"};
static const char *const kChoiceOctave[]     = {"-2", "-1", "0", "+1", "+2"};
static const char *const kChoiceArpmode[]    = {"UP", "DOWN", "UP/DOWN", "RANDOM"};
static const char *const kChoiceArprange[]   = {"1", "2", "3", "4"};

const char * const * nl3_param_choices(uint32_t id) {
    switch (id) {
        case kNl3ParamArithmetic:
            return kChoiceArithmetic;

        case kNl3ParamAliasing:
            return kChoiceAliasing;

        case kNl3ParamHwLink:
            return kChoiceOffon;

        case kNl3ParamOsc1Waveform:
        case kNl3ParamOsc2Waveform:
            return kChoiceWave;

        case kNl3ParamOsc1Sync:
        case kNl3ParamOsc2Sync:
        case kNl3ParamOsc2Kbt:
        case kNl3ParamOsc2Partial:
        case kNl3ParamOscMixNoise:
        case kNl3ParamFilterTrack:
        case kNl3ParamFilterBypass:
        case kNl3ParamFilter2Env:
        case kNl3ParamFEnvVelocity:
        case kNl3ParamFEnvInvert:
        case kNl3ParamFEnvExpAttack:
        case kNl3ParamAmpExpAttack:
        case kNl3ParamLfo1Mono:
        case kNl3ParamLfo1Invert:
        case kNl3ParamLfo1ClockSync:
        case kNl3ParamLfo2Mono:
        case kNl3ParamLfo2Invert:
        case kNl3ParamLfo2ClockSync:
        case kNl3ParamMEnvInvert:
        case kNl3ParamMEnvRepeat:
        case kNl3ParamMEnvExpAttack:
        case kNl3ParamMono:
        case kNl3ParamLegato:
        case kNl3ParamUnison:
        case kNl3ParamChordMemory:
        case kNl3ParamArpRun:
        case kNl3ParamArpKbdSync:
        case kNl3ParamArpClockSync:
            return kChoiceOffon;

        case kNl3ParamOscModType:
            return kChoiceOscmod;

        case kNl3ParamFilterMode:
            return kChoiceFmode;

        case kNl3ParamFilterType:
            return kChoiceFtype;

        case kNl3ParamFilterSlope:
            return kChoiceSlope;

        case kNl3ParamFilter2Type:
            return kChoiceF2type;

        case kNl3ParamLfo1Waveform:
        case kNl3ParamLfo2Waveform:
            return kChoiceLfowave;

        case kNl3ParamLfo1Dest:
            return kChoiceLfo1dest;

        case kNl3ParamLfo1EnvKbs:
        case kNl3ParamLfo2EnvKbs:
            return kChoiceEnvkbs;

        case kNl3ParamLfo2Dest:
            return kChoiceLfo2dest;

        case kNl3ParamMEnvDest:
            return kChoiceModenvdest;

        case kNl3ParamMEnvRelease:
            return kChoiceDecrel;

        case kNl3ParamGlideMode:
            return kChoiceGlide;

        case kNl3ParamVibratoSource:
            return kChoiceVibsrc;

        case kNl3ParamMonoPriority:
            return kChoicePrio;

        case kNl3ParamOctaveShift:
            return kChoiceOctave;

        case kNl3ParamArpMode:
            return kChoiceArpmode;

        case kNl3ParamArpRange:
            return kChoiceArprange;

        default:
            return NULL;
    }
}

#define nl3_choices_Osc1Waveform     kChoiceWave
#define nl3_choices_Osc2Waveform     kChoiceWave
#define nl3_choices_Osc1Sync         kChoiceOffon
#define nl3_choices_Osc2Sync         kChoiceOffon
#define nl3_choices_Osc2Kbt          kChoiceOffon
#define nl3_choices_Osc2Partial      kChoiceOffon
#define nl3_choices_OscMixNoise      kChoiceOffon
#define nl3_choices_FilterTrack      kChoiceOffon
#define nl3_choices_FilterBypass     kChoiceOffon
#define nl3_choices_Filter2Env       kChoiceOffon
#define nl3_choices_FEnvVelocity     kChoiceOffon
#define nl3_choices_FEnvInvert       kChoiceOffon
#define nl3_choices_FEnvExpAttack    kChoiceOffon
#define nl3_choices_AmpExpAttack     kChoiceOffon
#define nl3_choices_Lfo1Mono         kChoiceOffon
#define nl3_choices_Lfo1Invert       kChoiceOffon
#define nl3_choices_Lfo1ClockSync    kChoiceOffon
#define nl3_choices_Lfo2Mono         kChoiceOffon
#define nl3_choices_Lfo2Invert       kChoiceOffon
#define nl3_choices_Lfo2ClockSync    kChoiceOffon
#define nl3_choices_MEnvInvert       kChoiceOffon
#define nl3_choices_MEnvRepeat       kChoiceOffon
#define nl3_choices_MEnvExpAttack    kChoiceOffon
#define nl3_choices_Mono             kChoiceOffon
#define nl3_choices_Legato           kChoiceOffon
#define nl3_choices_Unison           kChoiceOffon
#define nl3_choices_ChordMemory      kChoiceOffon
#define nl3_choices_ArpRun           kChoiceOffon
#define nl3_choices_ArpKbdSync       kChoiceOffon
#define nl3_choices_ArpClockSync     kChoiceOffon
#define nl3_choices_OscModType       kChoiceOscmod
#define nl3_choices_FilterMode       kChoiceFmode
#define nl3_choices_FilterType       kChoiceFtype
#define nl3_choices_FilterSlope      kChoiceSlope
#define nl3_choices_Filter2Type      kChoiceF2type
#define nl3_choices_Lfo1Waveform     kChoiceLfowave
#define nl3_choices_Lfo2Waveform     kChoiceLfowave
#define nl3_choices_Lfo1Dest         kChoiceLfo1dest
#define nl3_choices_Lfo1EnvKbs       kChoiceEnvkbs
#define nl3_choices_Lfo2EnvKbs       kChoiceEnvkbs
#define nl3_choices_Lfo2Dest         kChoiceLfo2dest
#define nl3_choices_MEnvDest         kChoiceModenvdest
#define nl3_choices_MEnvRelease      kChoiceDecrel
#define nl3_choices_GlideMode        kChoiceGlide
#define nl3_choices_VibratoSource    kChoiceVibsrc
#define nl3_choices_MonoPriority     kChoicePrio
#define nl3_choices_OctaveShift      kChoiceOctave
#define nl3_choices_ArpMode          kChoiceArpmode
#define nl3_choices_ArpRange         kChoiceArprange
#define nl3_choices_Arithmetic       kChoiceArithmetic
#define nl3_choices_Aliasing         kChoiceAliasing
#define nl3_choices_HwLink           kChoiceOffon

// ---- the panel - notes §2 ------------------------------------------------------------------------------------
static const tNl3Control kOsc1[] = {
    {kNl3ParamOsc1Waveform,  "Wave",   eTiControlChoice,   5, 0.0, nl3_choices_Osc1Waveform},
    {kNl3ParamOsc1Shape,     "Shape",  eTiControlDial,   127, 0.0, NULL                    },
    {kNl3ParamOsc1Sync,      "Sync",   eTiControlChoice,   1, 0.0, nl3_choices_Osc1Sync    },
    {kNl3ParamOsc1SyncTune,  "Detune", eTiControlDial,   127, 0.0, NULL                    },
    {kNl3ParamOsc1NoiseSeed, "Timbre", eTiControlDial,   127, 0.0, NULL                    },
    {kNl3ParamOsc1FmAmount,  "FM",     eTiControlDial,   127, 0.0, NULL                    },
};
static const tNl3Control kOsc2[]                = {
    {kNl3ParamOsc2Waveform, "Wave",    eTiControlChoice,   5, 0.0, nl3_choices_Osc2Waveform},
    {kNl3ParamOsc2Shape,    "Shape",   eTiControlDial,   127, 0.0, NULL                    },
    {kNl3ParamOsc2Sync,     "Sync",    eTiControlChoice,   1, 0.0, nl3_choices_Osc2Sync    },
    {kNl3ParamOsc2Coarse,   "Coarse",  eTiControlDial,   127, 0.0, NULL                    },
    {kNl3ParamOsc2Fine,     "Fine",    eTiControlDial,   127, 0.0, NULL                    },
    {kNl3ParamOsc2Kbt,      "KBT Off", eTiControlChoice,   1, 0.0, nl3_choices_Osc2Kbt     },
    {kNl3ParamOsc2Partial,  "Partial", eTiControlChoice,   1, 0.0, nl3_choices_Osc2Partial },
};
static const tNl3Control kOsc2More[]            = {
    {kNl3ParamOsc2SyncTune,  "Detune",  eTiControlDial, 127, 0.0, NULL},
    {kNl3ParamOsc2NoiseSeed, "Timbre",  eTiControlDial, 127, 0.0, NULL},
    {kNl3ParamOsc2FmAmount,  "FM",      eTiControlDial, 127, 0.0, NULL},
    {kNl3ParamOsc2Carrier,   "Carrier", eTiControlDial, 127, 0.0, NULL},
    {kNl3ParamOsc2Modulator, "Mod",     eTiControlDial, 127, 0.0, NULL},
    {kNl3ParamOsc2NoiseType, "N Type",  eTiControlDial, 127, 0.0, NULL},
    {kNl3ParamOsc2NoiseFreq, "N Freq",  eTiControlDial, 127, 0.0, NULL},
};
static const tNl3Control kOscMod[]              = {
    {kNl3ParamOscModAmount, "Amount", eTiControlDial,   127, 0.0, NULL                   },
    {kNl3ParamOscModType,   "Type",   eTiControlChoice,   5, 0.0, nl3_choices_OscModType },
    {kNl3ParamOscMix,       "Mix",    eTiControlDial,   127, 0.0, NULL                   },
    {kNl3ParamOscMixNoise,  "Noise3", eTiControlChoice,   1, 0.0, nl3_choices_OscMixNoise},
};
static const tNl3Control kFilter[]              = {
    {kNl3ParamFilterMode,   "Mode",     eTiControlChoice,   1, 0.0, nl3_choices_FilterMode  },
    {kNl3ParamFilterType,   "Type",     eTiControlChoice,   5, 0.0, nl3_choices_FilterType  },
    {kNl3ParamFilterSlope,  "Slope",    eTiControlChoice,   2, 0.0, nl3_choices_FilterSlope },
    {kNl3ParamFilter2Type,  "Multi",    eTiControlChoice,   5, 0.0, nl3_choices_Filter2Type },
    {kNl3ParamCutoff,       "Freq 1",   eTiControlDial,   127, 0.0, NULL                    },
    {kNl3ParamCutoff2,      "Freq 2",   eTiControlDial,   127, 0.0, NULL                    },
    {kNl3ParamResonance,    "Res",      eTiControlDial,   127, 0.0, NULL                    },
    {kNl3ParamFilterDist,   "Dist",     eTiControlDial,   127, 0.0, NULL                    },
    {kNl3ParamFilterTrack,  "KB Track", eTiControlChoice,   1, 0.0, nl3_choices_FilterTrack },
    {kNl3ParamFilterBypass, "Osc2 Byp", eTiControlChoice,   1, 0.0, nl3_choices_FilterBypass},
    {kNl3ParamFilter2Env,   "F2 Env",   eTiControlChoice,   1, 0.0, nl3_choices_Filter2Env  },
};
static const tNl3Control kFilterEnv[]           = {
    {kNl3ParamFEnvAttack,    "Attack",  eTiControlDial,   127, 0.0, NULL                     },
    {kNl3ParamFEnvDecay,     "Decay",   eTiControlDial,   127, 0.0, NULL                     },
    {kNl3ParamFEnvSustain,   "Sustain", eTiControlDial,   127, 0.0, NULL                     },
    {kNl3ParamFEnvRelease,   "Release", eTiControlDial,   127, 0.0, NULL                     },
    {kNl3ParamFEnvAmount,    "Amount",  eTiControlDial,   127, 0.0, NULL                     },
    {kNl3ParamFEnvVelocity,  "Vel",     eTiControlChoice,   1, 0.0, nl3_choices_FEnvVelocity },
    {kNl3ParamFEnvInvert,    "Invert",  eTiControlChoice,   1, 0.0, nl3_choices_FEnvInvert   },
    {kNl3ParamFEnvExpAttack, "Exp Att", eTiControlChoice,   1, 0.0, nl3_choices_FEnvExpAttack},
};
static const tNl3Control kAmpEnv[]              = {
    {kNl3ParamAmpAttack,    "Attack",  eTiControlDial,   127, 0.0, NULL                    },
    {kNl3ParamAmpDecay,     "Decay",   eTiControlDial,   127, 0.0, NULL                    },
    {kNl3ParamAmpSustain,   "Sustain", eTiControlDial,   127, 0.0, NULL                    },
    {kNl3ParamAmpRelease,   "Release", eTiControlDial,   127, 0.0, NULL                    },
    {kNl3ParamAmpExpAttack, "Exp Att", eTiControlChoice,   1, 0.0, nl3_choices_AmpExpAttack},
};
static const tNl3Control kLfo1[]                = {
    {kNl3ParamLfo1Rate,      "Rate",    eTiControlDial,   127, 0.0, NULL                     },
    {kNl3ParamLfo1Amount,    "Amount",  eTiControlDial,   127, 0.0, NULL                     },
    {kNl3ParamLfo1Waveform,  "Wave",    eTiControlChoice,   5, 0.0, nl3_choices_Lfo1Waveform },
    {kNl3ParamLfo1Dest,      "Dest",    eTiControlChoice,  11, 0.0, nl3_choices_Lfo1Dest     },
    {kNl3ParamLfo1EnvKbs,    "Env/KBS", eTiControlChoice,   2, 0.0, nl3_choices_Lfo1EnvKbs   },
    {kNl3ParamLfo1Mono,      "Mono",    eTiControlChoice,   1, 0.0, nl3_choices_Lfo1Mono     },
    {kNl3ParamLfo1Invert,    "Invert",  eTiControlChoice,   1, 0.0, nl3_choices_Lfo1Invert   },
    {kNl3ParamLfo1ClockSync, "Clock",   eTiControlChoice,   1, 0.0, nl3_choices_Lfo1ClockSync},
    {kNl3ParamLfo1Divisor,   "Divisor", eTiControlDial,   127, 0.0, NULL                     },
};
static const tNl3Control kLfo2[]                = {
    {kNl3ParamLfo2Rate,      "Rate",    eTiControlDial,   127, 0.0, NULL                     },
    {kNl3ParamLfo2Amount,    "Amount",  eTiControlDial,   127, 0.0, NULL                     },
    {kNl3ParamLfo2Waveform,  "Wave",    eTiControlChoice,   5, 0.0, nl3_choices_Lfo2Waveform },
    {kNl3ParamLfo2Dest,      "Dest",    eTiControlChoice,  11, 0.0, nl3_choices_Lfo2Dest     },
    {kNl3ParamLfo2EnvKbs,    "Env/KBS", eTiControlChoice,   2, 0.0, nl3_choices_Lfo2EnvKbs   },
    {kNl3ParamLfo2Mono,      "Mono",    eTiControlChoice,   1, 0.0, nl3_choices_Lfo2Mono     },
    {kNl3ParamLfo2Invert,    "Invert",  eTiControlChoice,   1, 0.0, nl3_choices_Lfo2Invert   },
    {kNl3ParamLfo2ClockSync, "Clock",   eTiControlChoice,   1, 0.0, nl3_choices_Lfo2ClockSync},
    {kNl3ParamLfo2Divisor,   "Divisor", eTiControlDial,   127, 0.0, NULL                     },
};
static const tNl3Control kModEnv[]              = {
    {kNl3ParamMEnvAttack,    "Attack",  eTiControlDial,   127, 0.0, NULL                     },
    {kNl3ParamMEnvDecay,     "Dec/Rel", eTiControlDial,   127, 0.0, NULL                     },
    {kNl3ParamMEnvAmount,    "Amount",  eTiControlDial,   127, 0.0, NULL                     },
    {kNl3ParamMEnvDest,      "Dest",    eTiControlChoice,  11, 0.0, nl3_choices_MEnvDest     },
    {kNl3ParamMEnvInvert,    "Invert",  eTiControlChoice,   1, 0.0, nl3_choices_MEnvInvert   },
    {kNl3ParamMEnvRelease,   "Mode",    eTiControlChoice,   1, 0.0, nl3_choices_MEnvRelease  },
    {kNl3ParamMEnvRepeat,    "Repeat",  eTiControlChoice,   1, 0.0, nl3_choices_MEnvRepeat   },
    {kNl3ParamMEnvExpAttack, "Exp Att", eTiControlChoice,   1, 0.0, nl3_choices_MEnvExpAttack},
};
static const tNl3Control kGlide[]               = {
    {kNl3ParamGlideRate, "Rate",  eTiControlDial,   127, 0.0, NULL                 },
    {kNl3ParamGlideMode, "Glide", eTiControlChoice,   2, 0.0, nl3_choices_GlideMode},
};
static const tNl3Control kVibrato[]             = {
    {kNl3ParamVibratoRate,   "Rate",   eTiControlDial,   127, 0.0, NULL                     },
    {kNl3ParamVibratoAmount, "Amount", eTiControlDial,   127, 0.0, NULL                     },
    {kNl3ParamVibratoSource, "Source", eTiControlChoice,   2, 0.0, nl3_choices_VibratoSource},
};
static const tNl3Control kVoice[]               = {
    {kNl3ParamMono,         "Mono",     eTiControlChoice,   1, 0.0, nl3_choices_Mono        },
    {kNl3ParamLegato,       "Legato",   eTiControlChoice,   1, 0.0, nl3_choices_Legato      },
    {kNl3ParamMonoPriority, "Priority", eTiControlChoice,   2, 0.0, nl3_choices_MonoPriority},
    {kNl3ParamUnison,       "Unison",   eTiControlChoice,   1, 0.0, nl3_choices_Unison      },
    {kNl3ParamUnisonDetune, "Detune",   eTiControlDial,   127, 0.0, NULL                    },
    {kNl3ParamOctaveShift,  "Octave",   eTiControlChoice,   4, 0.0, nl3_choices_OctaveShift },
    {kNl3ParamTranspose,    "Transp",   eTiControlDial,   127, 0.0, NULL                    },
    {kNl3ParamChordMemory,  "Chord",    eTiControlChoice,   1, 0.0, nl3_choices_ChordMemory },
};
static const tNl3Control kArp[]                 = {
    {kNl3ParamArpRun,       "Run",     eTiControlChoice,   1, 0.0, nl3_choices_ArpRun      },
    {kNl3ParamArpRate,      "Rate",    eTiControlDial,   127, 0.0, NULL                    },
    {kNl3ParamArpMode,      "Mode",    eTiControlChoice,   3, 0.0, nl3_choices_ArpMode     },
    {kNl3ParamArpRange,     "Range",   eTiControlChoice,   3, 0.0, nl3_choices_ArpRange    },
    {kNl3ParamArpKbdSync,   "KB Sync", eTiControlChoice,   1, 0.0, nl3_choices_ArpKbdSync  },
    {kNl3ParamArpClockSync, "Clock",   eTiControlChoice,   1, 0.0, nl3_choices_ArpClockSync},
    {kNl3ParamArpDivisor,   "Divisor", eTiControlDial,   127, 0.0, NULL                    },
};
static const tNl3Control kMorphWheelOsc[]       = {
    {kNl3ParamMorphWheel_Osc2FinePitch,   "O2 Fine", eTiControlDial, 255, -128.0, NULL},
    {kNl3ParamMorphWheel_Osc2CoarsePitch, "O2 Crs",  eTiControlDial, 255, -128.0, NULL},
    {kNl3ParamMorphWheel_Oscmod,          "OscMod",  eTiControlDial, 255, -128.0, NULL},
    {kNl3ParamMorphWheel_Oscmix,          "OscMix",  eTiControlDial, 255, -128.0, NULL},
    {kNl3ParamMorphWheel_Osc1Shape,       "O1 Shp",  eTiControlDial, 255, -128.0, NULL},
    {kNl3ParamMorphWheel_Osc2Shape,       "O2 Shp",  eTiControlDial, 255, -128.0, NULL},
};
static const tNl3Control kMorphWheelAmp[]       = {
    {kNl3ParamMorphWheel_AmpEnvAttack,  "A Att", eTiControlDial, 255, -128.0, NULL},
    {kNl3ParamMorphWheel_AmpEnvDecay,   "A Dec", eTiControlDial, 255, -128.0, NULL},
    {kNl3ParamMorphWheel_AmpEnvSustain, "A Sus", eTiControlDial, 255, -128.0, NULL},
    {kNl3ParamMorphWheel_AmpEnvRelease, "A Rel", eTiControlDial, 255, -128.0, NULL},
    {kNl3ParamMorphWheel_OutputLevel,   "Level", eTiControlDial, 255, -128.0, NULL},
};
static const tNl3Control kMorphWheelLfo[]       = {
    {kNl3ParamMorphWheel_Lfo1Rate,           "L1 Rate", eTiControlDial, 255, -128.0, NULL},
    {kNl3ParamMorphWheel_Lfo1Amount,         "L1 Amt",  eTiControlDial, 255, -128.0, NULL},
    {kNl3ParamMorphWheel_Lfo2Rate,           "L2 Rate", eTiControlDial, 255, -128.0, NULL},
    {kNl3ParamMorphWheel_Lfo2Amount,         "L2 Amt",  eTiControlDial, 255, -128.0, NULL},
    {kNl3ParamMorphWheel_ModEnvAttack,       "ME Att",  eTiControlDial, 255, -128.0, NULL},
    {kNl3ParamMorphWheel_ModEnvDecayRelease, "ME Dec",  eTiControlDial, 255, -128.0, NULL},
    {kNl3ParamMorphWheel_ModEnvAmount,       "ME Amt",  eTiControlDial, 255, -128.0, NULL},
};
static const tNl3Control kMorphWheelFilter[]    = {
    {kNl3ParamMorphWheel_FiltEnvAttack,  "F Att",  eTiControlDial, 255, -128.0, NULL},
    {kNl3ParamMorphWheel_FiltEnvDecay,   "F Dec",  eTiControlDial, 255, -128.0, NULL},
    {kNl3ParamMorphWheel_FiltEnvSustain, "F Sus",  eTiControlDial, 255, -128.0, NULL},
    {kNl3ParamMorphWheel_FiltEnvRelease, "F Rel",  eTiControlDial, 255, -128.0, NULL},
    {kNl3ParamMorphWheel_FiltEnvAmount,  "F Amt",  eTiControlDial, 255, -128.0, NULL},
    {kNl3ParamMorphWheel_FiltFrequency1, "Freq 1", eTiControlDial, 255, -128.0, NULL},
    {kNl3ParamMorphWheel_FiltFrequency2, "Freq 2", eTiControlDial, 255, -128.0, NULL},
    {kNl3ParamMorphWheel_FiltResonance,  "Res",    eTiControlDial, 255, -128.0, NULL},
};
static const tNl3Control kMorphTouchOsc[]       = {
    {kNl3ParamMorphTouch_Osc2FinePitch,   "O2 Fine", eTiControlDial, 255, -128.0, NULL},
    {kNl3ParamMorphTouch_Osc2CoarsePitch, "O2 Crs",  eTiControlDial, 255, -128.0, NULL},
    {kNl3ParamMorphTouch_Oscmod,          "OscMod",  eTiControlDial, 255, -128.0, NULL},
    {kNl3ParamMorphTouch_Oscmix,          "OscMix",  eTiControlDial, 255, -128.0, NULL},
    {kNl3ParamMorphTouch_Osc1Shape,       "O1 Shp",  eTiControlDial, 255, -128.0, NULL},
    {kNl3ParamMorphTouch_Osc2Shape,       "O2 Shp",  eTiControlDial, 255, -128.0, NULL},
};
static const tNl3Control kMorphTouchAmp[]       = {
    {kNl3ParamMorphTouch_AmpEnvAttack,  "A Att", eTiControlDial, 255, -128.0, NULL},
    {kNl3ParamMorphTouch_AmpEnvDecay,   "A Dec", eTiControlDial, 255, -128.0, NULL},
    {kNl3ParamMorphTouch_AmpEnvSustain, "A Sus", eTiControlDial, 255, -128.0, NULL},
    {kNl3ParamMorphTouch_AmpEnvRelease, "A Rel", eTiControlDial, 255, -128.0, NULL},
    {kNl3ParamMorphTouch_OutputLevel,   "Level", eTiControlDial, 255, -128.0, NULL},
};
static const tNl3Control kMorphTouchLfo[]       = {
    {kNl3ParamMorphTouch_Lfo1Rate,           "L1 Rate", eTiControlDial, 255, -128.0, NULL},
    {kNl3ParamMorphTouch_Lfo1Amount,         "L1 Amt",  eTiControlDial, 255, -128.0, NULL},
    {kNl3ParamMorphTouch_Lfo2Rate,           "L2 Rate", eTiControlDial, 255, -128.0, NULL},
    {kNl3ParamMorphTouch_Lfo2Amount,         "L2 Amt",  eTiControlDial, 255, -128.0, NULL},
    {kNl3ParamMorphTouch_ModEnvAttack,       "ME Att",  eTiControlDial, 255, -128.0, NULL},
    {kNl3ParamMorphTouch_ModEnvDecayRelease, "ME Dec",  eTiControlDial, 255, -128.0, NULL},
    {kNl3ParamMorphTouch_ModEnvAmount,       "ME Amt",  eTiControlDial, 255, -128.0, NULL},
};
static const tNl3Control kMorphTouchFilter[]    = {
    {kNl3ParamMorphTouch_FiltEnvAttack,  "F Att",  eTiControlDial, 255, -128.0, NULL},
    {kNl3ParamMorphTouch_FiltEnvDecay,   "F Dec",  eTiControlDial, 255, -128.0, NULL},
    {kNl3ParamMorphTouch_FiltEnvSustain, "F Sus",  eTiControlDial, 255, -128.0, NULL},
    {kNl3ParamMorphTouch_FiltEnvRelease, "F Rel",  eTiControlDial, 255, -128.0, NULL},
    {kNl3ParamMorphTouch_FiltEnvAmount,  "F Amt",  eTiControlDial, 255, -128.0, NULL},
    {kNl3ParamMorphTouch_FiltFrequency1, "Freq 1", eTiControlDial, 255, -128.0, NULL},
    {kNl3ParamMorphTouch_FiltFrequency2, "Freq 2", eTiControlDial, 255, -128.0, NULL},
    {kNl3ParamMorphTouch_FiltResonance,  "Res",    eTiControlDial, 255, -128.0, NULL},
};
static const tNl3Control kMorphVelocityOsc[]    = {
    {kNl3ParamMorphVelocity_Osc2FinePitch,   "O2 Fine", eTiControlDial, 255, -128.0, NULL},
    {kNl3ParamMorphVelocity_Osc2CoarsePitch, "O2 Crs",  eTiControlDial, 255, -128.0, NULL},
    {kNl3ParamMorphVelocity_Oscmod,          "OscMod",  eTiControlDial, 255, -128.0, NULL},
    {kNl3ParamMorphVelocity_Oscmix,          "OscMix",  eTiControlDial, 255, -128.0, NULL},
    {kNl3ParamMorphVelocity_Osc1Shape,       "O1 Shp",  eTiControlDial, 255, -128.0, NULL},
    {kNl3ParamMorphVelocity_Osc2Shape,       "O2 Shp",  eTiControlDial, 255, -128.0, NULL},
};
static const tNl3Control kMorphVelocityAmp[]    = {
    {kNl3ParamMorphVelocity_AmpEnvAttack,  "A Att", eTiControlDial, 255, -128.0, NULL},
    {kNl3ParamMorphVelocity_AmpEnvDecay,   "A Dec", eTiControlDial, 255, -128.0, NULL},
    {kNl3ParamMorphVelocity_AmpEnvSustain, "A Sus", eTiControlDial, 255, -128.0, NULL},
    {kNl3ParamMorphVelocity_AmpEnvRelease, "A Rel", eTiControlDial, 255, -128.0, NULL},
    {kNl3ParamMorphVelocity_OutputLevel,   "Level", eTiControlDial, 255, -128.0, NULL},
};
static const tNl3Control kMorphVelocityLfo[]    = {
    {kNl3ParamMorphVelocity_Lfo1Rate,           "L1 Rate", eTiControlDial, 255, -128.0, NULL},
    {kNl3ParamMorphVelocity_Lfo1Amount,         "L1 Amt",  eTiControlDial, 255, -128.0, NULL},
    {kNl3ParamMorphVelocity_Lfo2Rate,           "L2 Rate", eTiControlDial, 255, -128.0, NULL},
    {kNl3ParamMorphVelocity_Lfo2Amount,         "L2 Amt",  eTiControlDial, 255, -128.0, NULL},
    {kNl3ParamMorphVelocity_ModEnvAttack,       "ME Att",  eTiControlDial, 255, -128.0, NULL},
    {kNl3ParamMorphVelocity_ModEnvDecayRelease, "ME Dec",  eTiControlDial, 255, -128.0, NULL},
    {kNl3ParamMorphVelocity_ModEnvAmount,       "ME Amt",  eTiControlDial, 255, -128.0, NULL},
};
static const tNl3Control kMorphVelocityFilter[] = {
    {kNl3ParamMorphVelocity_FiltEnvAttack,  "F Att",  eTiControlDial, 255, -128.0, NULL},
    {kNl3ParamMorphVelocity_FiltEnvDecay,   "F Dec",  eTiControlDial, 255, -128.0, NULL},
    {kNl3ParamMorphVelocity_FiltEnvSustain, "F Sus",  eTiControlDial, 255, -128.0, NULL},
    {kNl3ParamMorphVelocity_FiltEnvRelease, "F Rel",  eTiControlDial, 255, -128.0, NULL},
    {kNl3ParamMorphVelocity_FiltEnvAmount,  "F Amt",  eTiControlDial, 255, -128.0, NULL},
    {kNl3ParamMorphVelocity_FiltFrequency1, "Freq 1", eTiControlDial, 255, -128.0, NULL},
    {kNl3ParamMorphVelocity_FiltFrequency2, "Freq 2", eTiControlDial, 255, -128.0, NULL},
    {kNl3ParamMorphVelocity_FiltResonance,  "Res",    eTiControlDial, 255, -128.0, NULL},
};
static const tNl3Control kMorphKbdOsc[]         = {
    {kNl3ParamMorphKbd_Osc2FinePitch,   "O2 Fine", eTiControlDial, 255, -128.0, NULL},
    {kNl3ParamMorphKbd_Osc2CoarsePitch, "O2 Crs",  eTiControlDial, 255, -128.0, NULL},
    {kNl3ParamMorphKbd_Oscmod,          "OscMod",  eTiControlDial, 255, -128.0, NULL},
    {kNl3ParamMorphKbd_Oscmix,          "OscMix",  eTiControlDial, 255, -128.0, NULL},
    {kNl3ParamMorphKbd_Osc1Shape,       "O1 Shp",  eTiControlDial, 255, -128.0, NULL},
    {kNl3ParamMorphKbd_Osc2Shape,       "O2 Shp",  eTiControlDial, 255, -128.0, NULL},
};
static const tNl3Control kMorphKbdAmp[]         = {
    {kNl3ParamMorphKbd_AmpEnvAttack,  "A Att", eTiControlDial, 255, -128.0, NULL},
    {kNl3ParamMorphKbd_AmpEnvDecay,   "A Dec", eTiControlDial, 255, -128.0, NULL},
    {kNl3ParamMorphKbd_AmpEnvSustain, "A Sus", eTiControlDial, 255, -128.0, NULL},
    {kNl3ParamMorphKbd_AmpEnvRelease, "A Rel", eTiControlDial, 255, -128.0, NULL},
    {kNl3ParamMorphKbd_OutputLevel,   "Level", eTiControlDial, 255, -128.0, NULL},
};
static const tNl3Control kMorphKbdLfo[]         = {
    {kNl3ParamMorphKbd_Lfo1Rate,           "L1 Rate", eTiControlDial, 255, -128.0, NULL},
    {kNl3ParamMorphKbd_Lfo1Amount,         "L1 Amt",  eTiControlDial, 255, -128.0, NULL},
    {kNl3ParamMorphKbd_Lfo2Rate,           "L2 Rate", eTiControlDial, 255, -128.0, NULL},
    {kNl3ParamMorphKbd_Lfo2Amount,         "L2 Amt",  eTiControlDial, 255, -128.0, NULL},
    {kNl3ParamMorphKbd_ModEnvAttack,       "ME Att",  eTiControlDial, 255, -128.0, NULL},
    {kNl3ParamMorphKbd_ModEnvDecayRelease, "ME Dec",  eTiControlDial, 255, -128.0, NULL},
    {kNl3ParamMorphKbd_ModEnvAmount,       "ME Amt",  eTiControlDial, 255, -128.0, NULL},
};
static const tNl3Control kMorphKbdFilter[]      = {
    {kNl3ParamMorphKbd_FiltEnvAttack,  "F Att",  eTiControlDial, 255, -128.0, NULL},
    {kNl3ParamMorphKbd_FiltEnvDecay,   "F Dec",  eTiControlDial, 255, -128.0, NULL},
    {kNl3ParamMorphKbd_FiltEnvSustain, "F Sus",  eTiControlDial, 255, -128.0, NULL},
    {kNl3ParamMorphKbd_FiltEnvRelease, "F Rel",  eTiControlDial, 255, -128.0, NULL},
    {kNl3ParamMorphKbd_FiltEnvAmount,  "F Amt",  eTiControlDial, 255, -128.0, NULL},
    {kNl3ParamMorphKbd_FiltFrequency1, "Freq 1", eTiControlDial, 255, -128.0, NULL},
    {kNl3ParamMorphKbd_FiltFrequency2, "Freq 2", eTiControlDial, 255, -128.0, NULL},
    {kNl3ParamMorphKbd_FiltResonance,  "Res",    eTiControlDial, 255, -128.0, NULL},
};
static const tNl3Control kOutput[]              = {
    {kNl3ParamLevel,  "Level",  eTiControlDial, 127, 0.0, NULL},
    {kNl3ParamMaster, "Master", eTiControlDial, 127, 0.0, NULL},
};
static const tNl3Control kPlugin[]              = {
    {kNl3ParamVoiceLimit, "Limit",      eTiControlDial,   NL3_MAX_VOICES - 1, 1.0, NULL                  },
    {kNl3ParamArithmetic, "Maths",      eTiControlChoice,                  1, 0.0, nl3_choices_Arithmetic},
    {kNl3ParamAliasing,   "Anti-alias", eTiControlChoice,                  1, 0.0, nl3_choices_Aliasing  },
};
static const tNl3Control kHardware[]            = {
    {kNl3ParamHwLink,    "Link",    eTiControlChoice,   1, 0.0, nl3_choices_HwLink},
    {kNl3ParamHwChannel, "Channel", eTiControlDial,    15, 1.0, NULL              },
    {kNl3ParamHwDevice,  "Device",  eTiControlDial,   127, 0.0, NULL              },
};

#define SECTION(title, r, g, b, controls)    {title, {r, g, b}, controls, sizeof(controls) / sizeof(controls[0])}
static const tNl3Section kOscPage[]             = {SECTION("OSC 1", 0.30, 0.55, 0.85, kOsc1), SECTION("MOD / MIX", 0.60, 0.60, 0.65, kOscMod), SECTION("OSC 2", 0.30, 0.70, 0.60, kOsc2), SECTION("OSC 2 SYNC / FM / NOISE", 0.30, 0.70, 0.60, kOsc2More)};
static const tNl3Section kFilterPage[]          = {SECTION("FILTER", 0.90, 0.55, 0.20, kFilter), SECTION("FILTER ENV", 0.90, 0.70, 0.30, kFilterEnv)};
static const tNl3Section kAmpPage[]             = {SECTION("AMP ENV", 0.85, 0.75, 0.25, kAmpEnv), SECTION("OUTPUT", 0.65, 0.65, 0.65, kOutput)};
static const tNl3Section kLfoPage[]             = {SECTION("LFO 1", 0.80, 0.55, 0.85, kLfo1), SECTION("LFO 2", 0.55, 0.80, 0.85, kLfo2)};
static const tNl3Section kModPage[]             = {SECTION("MOD ENV", 0.70, 0.45, 0.80, kModEnv)};
static const tNl3Section kPlayPage[]            = {SECTION("GLIDE", 0.60, 0.60, 0.65, kGlide), SECTION("VIBRATO", 0.60, 0.60, 0.65, kVibrato), SECTION("VOICE", 0.60, 0.60, 0.65, kVoice), SECTION("ARP", 0.60, 0.60, 0.65, kArp)};
static const tNl3Section kMorphWheelPage[]      = {SECTION("WHEEL: LFO / MOD ENV", 0.85, 0.55, 0.30, kMorphWheelLfo), SECTION("WHEEL: AMP", 0.85, 0.55, 0.30, kMorphWheelAmp), SECTION("WHEEL: OSC", 0.85, 0.55, 0.30, kMorphWheelOsc), SECTION("WHEEL: FILTER", 0.85, 0.55, 0.30, kMorphWheelFilter)};
static const tNl3Section kMorphTouchPage[]      = {SECTION("AFTERTOUCH: LFO / MOD ENV", 0.85, 0.55, 0.30, kMorphTouchLfo), SECTION("AFTERTOUCH: AMP", 0.85, 0.55, 0.30, kMorphTouchAmp), SECTION("AFTERTOUCH: OSC", 0.85, 0.55, 0.30, kMorphTouchOsc), SECTION("AFTERTOUCH: FILTER", 0.85, 0.55, 0.30, kMorphTouchFilter)};
static const tNl3Section kMorphVelocityPage[]   = {SECTION("VELOCITY: LFO / MOD ENV", 0.85, 0.55, 0.30, kMorphVelocityLfo), SECTION("VELOCITY: AMP", 0.85, 0.55, 0.30, kMorphVelocityAmp), SECTION("VELOCITY: OSC", 0.85, 0.55, 0.30, kMorphVelocityOsc), SECTION("VELOCITY: FILTER", 0.85, 0.55, 0.30, kMorphVelocityFilter)};
static const tNl3Section kMorphKbdPage[]        = {SECTION("KEYBOARD: LFO / MOD ENV", 0.85, 0.55, 0.30, kMorphKbdLfo), SECTION("KEYBOARD: AMP", 0.85, 0.55, 0.30, kMorphKbdAmp), SECTION("KEYBOARD: OSC", 0.85, 0.55, 0.30, kMorphKbdOsc), SECTION("KEYBOARD: FILTER", 0.85, 0.55, 0.30, kMorphKbdFilter)};
static const tNl3Section kGlobalPage[]          = {SECTION("PLUG-IN", 0.65, 0.65, 0.65, kPlugin), SECTION("HARDWARE", 0.65, 0.65, 0.65, kHardware)};

#define PAGE(title, sections)    {title, sections, sizeof(sections) / sizeof(sections[0])}
const tNl3Page           gNl3Pages[]            = {
    PAGE("OSC",  kOscPage),  PAGE("FILTER", kFilterPage),     PAGE("AMP",   kAmpPage),        PAGE("LFO", kLfoPage),           PAGE("MOD", kModPage),
    PAGE("PLAY", kPlayPage), PAGE("WHEEL",  kMorphWheelPage), PAGE("TOUCH", kMorphTouchPage), PAGE("VEL", kMorphVelocityPage), PAGE("KBD", kMorphKbdPage),PAGE("GLOBAL", kGlobalPage),
};
const uint32_t           gNl3PageCount          = sizeof(gNl3Pages) / sizeof(gNl3Pages[0]);

// ---- the sound - notes §5: an interim mapping onto the engine as it stands, until each block is rebuilt -------

#define LFO_RATE_WORD       (22.493)        // voices notes §21: the phase step at Rate 0, words of 2^24 a cycle
#define LFO_RATE_STEPS      (9.0)           // Rate steps an octave
#define LFO_TICK_HZ         (22050.0)       // the LFOs step once every two samples
#define LFO_PITCH_CUBE      (16384.0)       // voices notes §21: semitones = Amount^3 / this
#define ENV_CUTOFF_SCALE    (1.0)           // voices notes §23: a semitone of cutoff an Amount step, at full envelope
#define BEND_SEMIS          (2.0)

static double plain(const double * v, uint32_t id) {
    return v[id] * gNl3Params[id].stepCount;
}

static int choice(const double * v, uint32_t id) {
    return (int)lround(plain(v, id));
}

static bool on(const double * v, uint32_t id) {
    return v[id] >= 0.5;
}

// voices notes §21: the phase step, rounded as measured, in hertz
static double lfo_hz(double rate) {
    return round(LFO_RATE_WORD * exp2(rate / LFO_RATE_STEPS)) * LFO_TICK_HZ / 16777216.0;
}

// voices notes §21: a full swing's pitch, cubic in the Amount, 127 counting as 128
static double lfo_pitch_semis(const double * v, int l) {
    uint32_t at     = (uint32_t)l * (kNl3ParamLfo2Rate - kNl3ParamLfo1Rate);
    double   amount = plain(v, kNl3ParamLfo1Amount + at);

    amount = (amount >= 127.0) ? 128.0 : amount;
    return (on(v, kNl3ParamLfo1Invert + at) ? -1.0 : 1.0) * amount * amount * amount / LFO_PITCH_CUBE;
}

static tNl3OscShape osc_shape(int waveform) {
    static const tNl3OscShape kShapes[] = {eNl3OscSaw, eNl3OscSquare, eNl3OscTriangle, eNl3OscSine}; // the instrument's Saw, Pulse, Triangle and Sine

    return ((waveform >= 0) && (waveform < 4)) ? kShapes[waveform] : eNl3OscSaw;                     // Noise and Dual Sine play as Saw for now
}

void nl3_sound_from_values(const double * v, tNl3Sound * s) {
    static const int kLfoShape[6] = {2, 1, 0, 3, 3, 2};    // the engine's Square, Saw, Triangle, S+H
    double           mix          = plain(v, kNl3ParamOscMix) / 127.0;
    bool             noise3       = on(v, kNl3ParamOscMixNoise);
    int              slope        = choice(v, kNl3ParamFilterSlope);

    memset(s, 0, sizeof(*s));
    s->osc[0].shape     = osc_shape(choice(v, kNl3ParamOsc1Waveform));
    s->osc[0].width     = nl3_word_value(nl3_pulse_width_word(plain(v, kNl3ParamOsc1Shape)));   // osc notes §11
    s->osc[0].level     = 1.0 - mix;
    s->osc[1].shape     = osc_shape(choice(v, kNl3ParamOsc2Waveform));
    s->osc[1].width     = nl3_word_value(nl3_pulse_width_word(plain(v, kNl3ParamOsc2Shape)));
    s->osc[1].semitones = fmin(60.0, fmax(-60.0, plain(v, kNl3ParamOsc2Coarse) - 64.0));
    s->osc[1].cents     = (plain(v, kNl3ParamOsc2Fine) - 64.0) * 50.0 / 64.0;
    s->osc[1].level     = noise3 ? 0.0 : mix;
    s->osc[2].width     = 0.0;
    s->noiseLevel       = noise3 ? mix : 0.0;

    for (int o = 0; o < NL3_OSCS; o++) {
        s->osc[o].bendRange = BEND_SEMIS;
    }

    s->cutoff           = plain(v, kNl3ParamCutoff);
    s->resonance        = plain(v, kNl3ParamResonance);
    s->tracking         = on(v, kNl3ParamFilterTrack) ? 1.0 : 0.0;
    s->filterType       = (uint32_t)choice(v, kNl3ParamFilterType);   // LP HP BP BR Classic Dist (filter notes)
    s->filterSlope      = (uint32_t)slope;                            // 6, 12, 24 dB
    s->filterMulti      = choice(v, kNl3ParamFilterMode) == 1;
    s->filter2Type      = (uint32_t)choice(v, kNl3ParamFilter2Type);
    s->cutoff2          = plain(v, kNl3ParamCutoff2);
    s->filter2Env       = on(v, kNl3ParamFilter2Env);

    s->ampAttack        = plain(v, kNl3ParamAmpAttack);
    s->ampDecay         = plain(v, kNl3ParamAmpDecay);
    s->ampSustain       = plain(v, kNl3ParamAmpSustain);
    s->ampRelease       = plain(v, kNl3ParamAmpRelease);
    s->ampVelocity      = 64.0;
    s->ampExponential   = on(v, kNl3ParamAmpExpAttack);
    s->unison           = on(v, kNl3ParamUnison);   // voices notes §26
    {
        double k = plain(v, kNl3ParamUnisonDetune);

        s->unisonStep = (k * k / 8.0) - (0.08 * k) + 7.45;
    }

    s->env[0]           = (tNl3ModEnv){
        .attack   = plain(v, kNl3ParamFEnvAttack), .decay = plain(v, kNl3ParamFEnvDecay),
        .sustain  = plain(v, kNl3ParamFEnvSustain), .release = plain(v, kNl3ParamFEnvRelease),
        .velocity = on(v, kNl3ParamFEnvVelocity) ? 127.0 : 64.0, .exponential = on(v, kNl3ParamFEnvExpAttack),
        .amount   = (on(v, kNl3ParamFEnvInvert) ? -1.0 : 1.0) * plain(v, kNl3ParamFEnvAmount) * ENV_CUTOFF_SCALE,
    };
    int  dest     = choice(v, kNl3ParamMEnvDest);
    bool modEnvAr = choice(v, kNl3ParamMEnvRelease) == 1;   // 1: AR

    s->env[1]           = (tNl3ModEnv){
        // envelope notes §6: AD falls to nothing held or not (Repeat starting it again while held); AR holds at the top
        .attack  = plain(v, kNl3ParamMEnvAttack), .decay = plain(v, kNl3ParamMEnvDecay), .sustain = modEnvAr ? 127.0 : 0.0,
        .release = plain(v, kNl3ParamMEnvDecay), .velocity = 64.0, .exponential = on(v, kNl3ParamMEnvExpAttack),
        .repeat  = !modEnvAr && on(v, kNl3ParamMEnvRepeat),
        .amount  = ((dest == 3) || (dest == 5)) ? ((on(v, kNl3ParamMEnvInvert) ? -1.0 : 1.0) * plain(v, kNl3ParamMEnvAmount) * ENV_CUTOFF_SCALE) : 0.0,
    };
    {
        // voices notes §24: pitch cubic in the Amount as the LFOs', Shape and Osc Mix a step a step
        double amount = plain(v, kNl3ParamMEnvAmount), sign = on(v, kNl3ParamMEnvInvert) ? -1.0 : 1.0;
        double cube = (amount >= 127.0) ? 128.0 : amount;

        s->modEnvPitch[0] = ((dest == 0) || (dest == 2)) ? (sign * cube * cube * cube / LFO_PITCH_CUBE) : 0.0;
        s->modEnvPitch[1] = ((dest == 1) || (dest == 2)) ? (sign * cube * cube * cube / LFO_PITCH_CUBE) : 0.0;
        s->modEnvWidth[0] = ((dest == 6) || (dest == 8)) ? (sign * amount / 128.0) : 0.0;
        s->modEnvWidth[1] = ((dest == 7) || (dest == 8)) ? (sign * amount / 128.0) : 0.0;
        s->modEnvMix      = (dest == 10) ? (sign * amount / 127.0) : 0.0;
    }

    for (int l = 0; l < NL3_LFOS; l++) {
        uint32_t at     = (uint32_t)l * (kNl3ParamLfo2Rate - kNl3ParamLfo1Rate);
        double   amount = (on(v, kNl3ParamLfo1Invert + at) ? -1.0 : 1.0) * plain(v, kNl3ParamLfo1Amount + at) / 127.0;
        int      to     = choice(v, kNl3ParamLfo1Dest + at);

        s->lfo[l]             = (tNl3LfoSound){
            .shape = kLfoShape[choice(v, kNl3ParamLfo1Waveform + at) % 6],
            .hz    = lfo_hz(plain(v, kNl3ParamLfo1Rate + at)),
            .keyed = choice(v, kNl3ParamLfo1EnvKbs + at) == 2,
        };
        s->lfoCutoff[l]       = ((to == 3) || (to == 5)) ? (amount * 127.0) : 0.0; // voices notes §22: a semitone an Amount step
        s->lfoMix[l]          = ((l == 0) && (to == 10)) ? amount : 0.0;           // LFO 2's 10 and 11 are Resonance and Pan: not yet
        s->lfoAmp[l]          = ((l == 0) && (to == 11)) ? (amount * 127.0 / 64.0) : 0.0;
        s->osc[0].lfoPitch[l] = ((to == 0) || (to == 2)) ? lfo_pitch_semis(v, l) : 0.0;
        s->osc[1].lfoPitch[l] = ((to == 1) || (to == 2)) ? lfo_pitch_semis(v, l) : 0.0;
        s->osc[0].lfoWidth[l] = ((to == 6) || (to == 8)) ? (amount * 127.0 / 128.0) : 0.0;   // voices notes §22: a Shape step an Amount step
        s->osc[1].lfoWidth[l] = ((to == 7) || (to == 8)) ? (amount * 127.0 / 128.0) : 0.0;
    }

    s->glideTime  = (choice(v, kNl3ParamGlideMode) == 0) ? 0.0 : plain(v, kNl3ParamGlideRate);
    s->glideType  = (choice(v, kNl3ParamGlideMode) == 2) ? 1 : 0;
    s->glideCurve = 1;
    s->mono       = on(v, kNl3ParamMono);
    s->legato     = on(v, kNl3ParamLegato);
    s->bend       = (2.0 * v[kNl3ParamPitchBend]) - 1.0;
    s->wheel      = v[kNl3ParamModWheel];
    s->pedal      = on(v, kNl3ParamSustainPedal);
}

// voices notes §25: the knob each of a group's 26 morph ranges moves, in the group's order
static const uint32_t kMorphTargets[26] = {
    kNl3ParamLfo1Rate,   kNl3ParamLfo1Amount,  kNl3ParamLfo2Rate,    kNl3ParamLfo2Amount,   kNl3ParamMEnvAttack, kNl3ParamMEnvDecay,
    kNl3ParamMEnvAmount, kNl3ParamOsc2Fine,    kNl3ParamOsc2Coarse,  kNl3ParamOscModAmount, kNl3ParamOscMix,     kNl3ParamOsc1Shape,
    kNl3ParamOsc2Shape,  kNl3ParamAmpAttack,   kNl3ParamAmpDecay,    kNl3ParamAmpSustain,   kNl3ParamAmpRelease, kNl3ParamFEnvAttack,
    kNl3ParamFEnvDecay,  kNl3ParamFEnvSustain, kNl3ParamFEnvRelease, kNl3ParamFEnvAmount,   kNl3ParamCutoff,     kNl3ParamCutoff2,
    kNl3ParamResonance,  kNl3ParamLevel,
};

// voices notes §25: range x source in knob steps, the groups adding, each knob held to its own range
bool nl3_morph_values(const double * v, float velocity, int note, double * out) {
    static const uint32_t kGroups[4] = {kNl3ParamMorphWheel_Lfo1Rate, kNl3ParamMorphTouch_Lfo1Rate, kNl3ParamMorphVelocity_Lfo1Rate, kNl3ParamMorphKbd_Lfo1Rate};
    double                wheel      = v[kNl3ParamModWheel];
    double                source[4]  = {
        (wheel >= 1.0) ? 1.0 : (wheel * 127.0 / 128.0),   // 127 is the whole range
        0.0,                                              // aftertouch: not received yet
        (double)velocity * 127.0 / 128.0,                 // 127 is not quite all of it
        ((double)note - 36.0) / 48.0,                     // C2 to C6, and on past either end
    };
    bool                  any        = false;

    memcpy(out, v, sizeof(double) * kNl3ParamCount);

    for (int g = 0; g < 4; g++) {
        for (int k = 0; k < 26; k++) {
            double range = plain(v, kGroups[g] + (uint32_t)k) - 128.0;

            if ((range != 0.0) && (source[g] != 0.0)) {
                uint32_t id    = kMorphTargets[k];
                double   steps = gNl3Params[id].stepCount;

                out[id] = fmin(1.0, fmax(0.0, out[id] + (range * source[g] / steps)));
                any     = true;
            }
        }
    }

    return any;
}
