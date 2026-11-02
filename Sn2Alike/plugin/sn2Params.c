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

#include <math.h>

#include "sn2Params.h"
#include "sn2Voices.h"

#define LIST    SYNTHLIB_PARAM_LIST

// notes §1: the panel's 0..127 is each dial's plain range, as on the instrument
const tSynthLibParam     gSn2Params[kSn2ParamCount] = {
    {kSn2ParamVoiceLimit,     "Voice Limit",             "Voices",   eSynthLibUnitGeneric,   1.0, SN2_MAX_VOICES, 23.0 / (SN2_MAX_VOICES - 1), SN2_MAX_VOICES - 1, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kSn2ParamVolume,         "Prog Level",              "Level",    eSynthLibUnitGeneric,   0.0,          127.0,               100.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kSn2ParamOscShape,       "Osc Shape",               "Shape",    eSynthLibUnitIndexed,   0.0,            2.0,                         0.0,                  2, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kSn2ParamPulseWidth,     "Osc Pulse Width",         "PW",       eSynthLibUnitGeneric,   0.0,          127.0,                64.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kSn2ParamCutoff,         "Cutoff",                  "Cutoff",   eSynthLibUnitGeneric,   0.0,          127.0,                         1.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kSn2ParamResonance,      "Resonance",               "Reso",     eSynthLibUnitGeneric,   0.0,          127.0,                         0.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kSn2ParamFilterType,     "Filter Type",             "Type",     eSynthLibUnitIndexed,   0.0,           11.0,                         0.0,                 11, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kSn2ParamFilterSlope,    "Filter Slope",            "Slope",    eSynthLibUnitIndexed,   0.0,            2.0,                         0.0,                  2, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kSn2ParamAmpAttack,      "Amp Attack",              "Attack",   eSynthLibUnitGeneric,   0.0,          127.0,                         0.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kSn2ParamAmpDecay,       "Amp Decay",               "Decay",    eSynthLibUnitGeneric,   0.0,          127.0,                         1.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kSn2ParamAmpSustain,     "Amp Sustain",             "Sustain",  eSynthLibUnitGeneric,   0.0,          127.0,                         1.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kSn2ParamAmpSustainTime, "Amp Sustain Time",        "Time",     eSynthLibUnitGeneric, -64.0,           63.0,                64.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kSn2ParamAmpRelease,     "Amp Release",             "Release",  eSynthLibUnitGeneric,   0.0,          127.0,                 4.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kSn2ParamProgram,        "Program",                 "Program",  eSynthLibUnitGeneric,   0.0,          127.0,                    0.000000,                127, SYNTHLIB_MIDI_NONE,       SYNTHLIB_PARAM_PROGRAM | SYNTHLIB_PARAM_HIDDEN | SYNTHLIB_PARAM_NO_SAVE},
    {kSn2ParamBank,           "Bank Select",             "Bank",     eSynthLibUnitGeneric,   0.0,          127.0,                    0.000000,                127,                       32, SYNTHLIB_PARAM_HIDDEN | SYNTHLIB_PARAM_NO_SAVE                         },
    {kSn2ParamOscSoften,      "Osc Soften",              "Soften",   eSynthLibUnitGeneric,   0.0,          127.0,                         1.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kSn2ParamOscLevel,       "Osc Level",               "Level",    eSynthLibUnitGeneric,   0.0,          127.0,                         1.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kSn2ParamOsc2Shape,      "Osc 2 Shape",             "Shape",    eSynthLibUnitIndexed,   0.0,            2.0,                         0.0,                  2, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kSn2ParamOsc2Width,      "Osc 2 Pulse Width",       "PW",       eSynthLibUnitGeneric,   0.0,          127.0,                64.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kSn2ParamOsc2Soften,     "Osc 2 Soften",            "Soften",   eSynthLibUnitGeneric,   0.0,          127.0,                         1.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kSn2ParamOsc2Level,      "Osc 2 Level",             "Level",    eSynthLibUnitGeneric,   0.0,          127.0,                         0.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kSn2ParamOsc3Shape,      "Osc 3 Shape",             "Shape",    eSynthLibUnitIndexed,   0.0,            2.0,                         0.0,                  2, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kSn2ParamOsc3Width,      "Osc 3 Pulse Width",       "PW",       eSynthLibUnitGeneric,   0.0,          127.0,                64.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kSn2ParamOsc3Soften,     "Osc 3 Soften",            "Soften",   eSynthLibUnitGeneric,   0.0,          127.0,                         1.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kSn2ParamOsc3Level,      "Osc 3 Level",             "Level",    eSynthLibUnitGeneric,   0.0,          127.0,                         0.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kSn2ParamOsc1Octave,     "Osc 1 Octave",            "Octave",   eSynthLibUnitGeneric,  -2.0,            2.0,                         0.5,                  4, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kSn2ParamOsc1Semitone,   "Osc 1 Semitone",          "Semi",     eSynthLibUnitGeneric, -12.0,           12.0,                         0.5,                 24, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kSn2ParamOsc1Fine,       "Osc 1 Fine Tune",         "Fine",     eSynthLibUnitGeneric, -64.0,           63.0,                64.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kSn2ParamOsc2Octave,     "Osc 2 Octave",            "Octave",   eSynthLibUnitGeneric,  -2.0,            2.0,                         0.5,                  4, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kSn2ParamOsc2Semitone,   "Osc 2 Semitone",          "Semi",     eSynthLibUnitGeneric, -12.0,           12.0,                         0.5,                 24, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kSn2ParamOsc2Fine,       "Osc 2 Fine Tune",         "Fine",     eSynthLibUnitGeneric, -64.0,           63.0,                64.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kSn2ParamOsc3Octave,     "Osc 3 Octave",            "Octave",   eSynthLibUnitGeneric,  -2.0,            2.0,                         0.5,                  4, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kSn2ParamOsc3Semitone,   "Osc 3 Semitone",          "Semi",     eSynthLibUnitGeneric, -12.0,           12.0,                         0.5,                 24, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kSn2ParamOsc3Fine,       "Osc 3 Fine Tune",         "Fine",     eSynthLibUnitGeneric, -64.0,           63.0,                64.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kSn2ParamFilterTrack,    "Filter Tracking",         "Track",    eSynthLibUnitGeneric,   0.0,          127.0,                         1.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kSn2ParamFilterDrive,    "Filter Overdrive",        "Drive",    eSynthLibUnitGeneric,   0.0,          127.0,                         0.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kSn2ParamAmpVelocity,    "Amp Velocity",            "Velocity", eSynthLibUnitGeneric,   0.0,          127.0,                64.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kSn2ParamEnv2Attack,     "Env 2 Attack",            "Attack",   eSynthLibUnitGeneric,   0.0,          127.0,                         0.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kSn2ParamEnv2Decay,      "Env 2 Decay",             "Decay",    eSynthLibUnitGeneric,   0.0,          127.0,                         0.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kSn2ParamEnv2Sustain,    "Env 2 Sustain",           "Sustain",  eSynthLibUnitGeneric,   0.0,          127.0,                         1.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kSn2ParamEnv2Release,    "Env 2 Release",           "Release",  eSynthLibUnitGeneric,   0.0,          127.0,                         0.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kSn2ParamEnv2Velocity,   "Env 2 Velocity",          "Vel",      eSynthLibUnitGeneric,   0.0,          127.0,                64.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kSn2ParamEnv2Cutoff,     "Env 2 to Cutoff",         "Cutoff",   eSynthLibUnitGeneric, -64.0,           63.0,                64.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kSn2ParamEnv3Attack,     "Env 3 Attack",            "Attack",   eSynthLibUnitGeneric,   0.0,          127.0,                         0.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kSn2ParamEnv3Decay,      "Env 3 Decay",             "Decay",    eSynthLibUnitGeneric,   0.0,          127.0,                         0.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kSn2ParamEnv3Sustain,    "Env 3 Sustain",           "Sustain",  eSynthLibUnitGeneric,   0.0,          127.0,                         1.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kSn2ParamEnv3Release,    "Env 3 Release",           "Release",  eSynthLibUnitGeneric,   0.0,          127.0,                         0.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kSn2ParamEnv3Velocity,   "Env 3 Velocity",          "Vel",      eSynthLibUnitGeneric,   0.0,          127.0,                64.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kSn2ParamEnv3Cutoff,     "Env 3 to Cutoff",         "Cutoff",   eSynthLibUnitGeneric, -64.0,           63.0,                64.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kSn2ParamNoiseLevel,     "Noise Level",             "Noise",    eSynthLibUnitGeneric,   0.0,          127.0,                         0.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kSn2ParamNoiseSoften,    "Noise Soften",            "Soften",   eSynthLibUnitGeneric,   0.0,          127.0,                         1.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kSn2ParamRing13,         "Ring 1x3",                "1x3",      eSynthLibUnitGeneric,   0.0,          127.0,                         0.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kSn2ParamRing23,         "Ring 2x3",                "2x3",      eSynthLibUnitGeneric,   0.0,          127.0,                         0.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kSn2ParamFilterQ,        "Filter Q Normalise",      "Q Norm",   eSynthLibUnitGeneric,   0.0,          127.0,                         0.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kSn2ParamEffectsDry,     "Effects Dry",             "Dry",      eSynthLibUnitGeneric,   0.0,          127.0,                         1.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kSn2ParamChorusSend,     "Chorus Send",             "Send",     eSynthLibUnitGeneric,   0.0,          127.0,                         0.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kSn2ParamChorusSpeed,    "Chorus Speed",            "Speed",    eSynthLibUnitGeneric,   0.0,          127.0,                27.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kSn2ParamChorusDepth,    "Chorus Depth",            "Depth",    eSynthLibUnitGeneric,   0.0,          127.0,                33.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kSn2ParamChorusFeedback, "Chorus Feedback",         "Fback",    eSynthLibUnitGeneric,   0.0,          127.0,                64.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kSn2ParamChorusType,     "Chorus Type",             "Type",     eSynthLibUnitIndexed,   0.0,            2.0,                         0.5,                  2, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kSn2ParamDelayOn,        "Global Delay",            "Delay",    eSynthLibUnitIndexed,   0.0,            1.0,                         1.0,                  1, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kSn2ParamReverbOn,       "Global Reverb",           "Reverb",   eSynthLibUnitIndexed,   0.0,            1.0,                         1.0,                  1, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kSn2ParamDelaySend,      "Delay Send",              "Send",     eSynthLibUnitGeneric,   0.0,          127.0,                         0.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kSn2ParamDelayTime,      "Delay Time",              "Time",     eSynthLibUnitGeneric,   0.0,          127.0,                75.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kSn2ParamDelayFeedback,  "Delay Feedback",          "Fback",    eSynthLibUnitGeneric,   0.0,          127.0,                50.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kSn2ParamDelayDamp,      "Delay HF Damp",           "Damp",     eSynthLibUnitGeneric,   0.0,          127.0,                20.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kSn2ParamDelayWidth,     "Delay Width",             "Width",    eSynthLibUnitGeneric,   0.0,          127.0,                         1.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kSn2ParamDelayRatio,     "Delay Ratio",             "Ratio",    eSynthLibUnitIndexed,   0.0,           12.0,                         0.0,                 12, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kSn2ParamDelaySync,      "Delay Sync",              "Sync",     eSynthLibUnitIndexed,   0.0,           34.0,                         0.0,                 34, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kSn2ParamGlideTime,      "Glide Time",              "Time",     eSynthLibUnitGeneric,   0.0,          127.0,                         0.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kSn2ParamGlideStyle,     "Glide Style",             "Style",    eSynthLibUnitIndexed,   0.0,            1.0,                         0.0,                  1, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kSn2ParamGlideCurve,     "Glide Curve",             "Curve",    eSynthLibUnitIndexed,   0.0,            1.0,                         1.0,                  1, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kSn2ParamGlideType,      "Glide Type",              "Type",     eSynthLibUnitIndexed,   0.0,            9.0,                   1.0 / 9.0,                  9, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kSn2ParamReverbSend,     "Reverb Send",             "Send",     eSynthLibUnitGeneric,   0.0,          127.0,                         0.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kSn2ParamReverbDecay,    "Reverb Decay",            "Decay",    eSynthLibUnitGeneric,   0.0,          127.0,                80.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kSn2ParamReverbDamp,     "Reverb HF Damp",          "Damp",     eSynthLibUnitGeneric,   0.0,          127.0,                18.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kSn2ParamReverbType,     "Reverb Type",             "Type",     eSynthLibUnitIndexed,   0.0,           15.0,                  9.0 / 15.0,                 15, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kSn2ParamEffectsOrder,   "Effects Order",           "Order",    eSynthLibUnitIndexed,   0.0,           18.0,                         0.0,                 18, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kSn2ParamOscTrigger,     "Osc Trigger",             "Trigger",  eSynthLibUnitIndexed,   0.0,            1.0,                         0.0,                  1, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kSn2ParamDistLevel,      "Distortion Level",        "Level",    eSynthLibUnitGeneric,   0.0,          127.0,                         0.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kSn2ParamDistComp,       "Distortion Compensation", "Comp",     eSynthLibUnitGeneric,   0.0,          127.0,               112.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kSn2ParamDistOutput,     "Distortion Output",       "Output",   eSynthLibUnitGeneric,   0.0,          127.0,                64.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kSn2ParamEqBass,         "EQ Bass",                 "Bass",     eSynthLibUnitGeneric,   0.0,          127.0,                64.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kSn2ParamEqTreble,       "EQ Treble",               "Treble",   eSynthLibUnitGeneric,   0.0,          127.0,                64.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kSn2ParamPitchBend,      "Pitch Bend",              "Bend",     eSynthLibUnitGeneric,   0.0,            1.0,                         0.5,                  0, SYNTHLIB_MIDI_PITCH_BEND, SYNTHLIB_PARAM_HIDDEN | SYNTHLIB_PARAM_NO_SAVE                         },
    {kSn2ParamModWheel,       "Mod Wheel",               "Wheel",    eSynthLibUnitGeneric,   0.0,            1.0,                         0.0,                  0, SYNTHLIB_MIDI_MOD_WHEEL,  SYNTHLIB_PARAM_HIDDEN | SYNTHLIB_PARAM_NO_SAVE                         },
    {kSn2ParamOsc1BendRange,  "Osc 1 Bend Range",        "Bend",     eSynthLibUnitGeneric, -12.0,           12.0,                 14.0 / 24.0,                 24, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kSn2ParamOsc2BendRange,  "Osc 2 Bend Range",        "Bend",     eSynthLibUnitGeneric, -12.0,           12.0,                 14.0 / 24.0,                 24, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kSn2ParamOsc3BendRange,  "Osc 3 Bend Range",        "Bend",     eSynthLibUnitGeneric, -12.0,           12.0,                 14.0 / 24.0,                 24, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kSn2ParamOsc1WheelPitch, "Osc 1 Wheel Pitch",       "Wheel",    eSynthLibUnitGeneric, -64.0,           63.0,                64.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kSn2ParamOsc2WheelPitch, "Osc 2 Wheel Pitch",       "Wheel",    eSynthLibUnitGeneric, -64.0,           63.0,                64.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kSn2ParamOsc3WheelPitch, "Osc 3 Wheel Pitch",       "Wheel",    eSynthLibUnitGeneric, -64.0,           63.0,                64.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kSn2ParamOsc1WheelLevel, "Osc 1 Wheel Level",       "Wheel",    eSynthLibUnitGeneric, -64.0,           63.0,                64.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kSn2ParamOsc2WheelLevel, "Osc 2 Wheel Level",       "Wheel",    eSynthLibUnitGeneric, -64.0,           63.0,                64.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kSn2ParamOsc3WheelLevel, "Osc 3 Wheel Level",       "Wheel",    eSynthLibUnitGeneric, -64.0,           63.0,                64.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kSn2ParamWheelCutoff,    "Wheel Cutoff",            "Wheel",    eSynthLibUnitGeneric, -64.0,           63.0,                64.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kSn2ParamWheelResonance, "Wheel Resonance",         "Wheel",    eSynthLibUnitGeneric, -64.0,           63.0,                64.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kSn2ParamPolyMode,       "Poly Mode",               "Mode",     eSynthLibUnitIndexed,   0.0,            1.0,                         0.0,                  1, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kSn2ParamEnvTrigger,     "Env Trigger",             "Trigger",  eSynthLibUnitIndexed,   0.0,            1.0,                         0.0,                  1, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kSn2ParamMaster,         "Master Volume",           "Master",   eSynthLibUnitGeneric,   0.0,          127.0,                         1.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kSn2ParamEffectsBypass,  "Effects Bypass",          "Effects",  eSynthLibUnitIndexed,   0.0,            1.0,                         0.0,                  1, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kSn2ParamSustainPedal,   "Sustain Pedal",           "Pedal",    eSynthLibUnitGeneric,   0.0,            1.0,                         0.0,                  0, SYNTHLIB_MIDI_SUSTAIN,    SYNTHLIB_PARAM_HIDDEN | SYNTHLIB_PARAM_NO_SAVE                         },
    {kSn2ParamLfo1Shape,      "LFO 1 Shape",             "Shape",    eSynthLibUnitIndexed,   0.0,            3.0,                   2.0 / 3.0,                  3, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kSn2ParamLfo1Speed,      "LFO 1 Speed",             "Speed",    eSynthLibUnitGeneric,   0.0,          127.0,                64.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kSn2ParamLfo1Range,      "LFO 1 Range",             "Range",    eSynthLibUnitIndexed,   0.0,            2.0,                         0.0,                  2, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kSn2ParamLfo1Delay,      "LFO 1 Delay",             "Delay",    eSynthLibUnitGeneric,   0.0,          127.0,                         0.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kSn2ParamLfo1Offset,     "LFO 1 Offset",            "Offset",   eSynthLibUnitGeneric,   0.0,          127.0,                         0.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kSn2ParamLfo1Trigger,    "LFO 1 Trigger",           "Trigger",  eSynthLibUnitIndexed,   0.0,            1.0,                         1.0,                  1, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kSn2ParamLfo1Cutoff,     "LFO 1 to Cutoff",         "Cutoff",   eSynthLibUnitGeneric, -64.0,           63.0,                64.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kSn2ParamOsc1Lfo1Pitch,  "Osc 1 LFO 1 Pitch",       "Pitch",    eSynthLibUnitGeneric, -64.0,           63.0,                64.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kSn2ParamOsc1Lfo1Width,  "Osc 1 LFO 1 Width",       "Width",    eSynthLibUnitGeneric, -64.0,           63.0,                64.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kSn2ParamOsc2Lfo1Pitch,  "Osc 2 LFO 1 Pitch",       "Pitch",    eSynthLibUnitGeneric, -64.0,           63.0,                64.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kSn2ParamOsc2Lfo1Width,  "Osc 2 LFO 1 Width",       "Width",    eSynthLibUnitGeneric, -64.0,           63.0,                64.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kSn2ParamOsc3Lfo1Pitch,  "Osc 3 LFO 1 Pitch",       "Pitch",    eSynthLibUnitGeneric, -64.0,           63.0,                64.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kSn2ParamOsc3Lfo1Width,  "Osc 3 LFO 1 Width",       "Width",    eSynthLibUnitGeneric, -64.0,           63.0,                64.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kSn2ParamLfo2Shape,      "LFO 2 Shape",             "Shape",    eSynthLibUnitIndexed,   0.0,            3.0,                   2.0 / 3.0,                  3, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kSn2ParamLfo2Speed,      "LFO 2 Speed",             "Speed",    eSynthLibUnitGeneric,   0.0,          127.0,                64.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kSn2ParamLfo2Range,      "LFO 2 Range",             "Range",    eSynthLibUnitIndexed,   0.0,            2.0,                         0.0,                  2, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kSn2ParamLfo2Delay,      "LFO 2 Delay",             "Delay",    eSynthLibUnitGeneric,   0.0,          127.0,                         0.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kSn2ParamLfo2Offset,     "LFO 2 Offset",            "Offset",   eSynthLibUnitGeneric,   0.0,          127.0,                         0.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kSn2ParamLfo2Trigger,    "LFO 2 Trigger",           "Trigger",  eSynthLibUnitIndexed,   0.0,            1.0,                         1.0,                  1, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kSn2ParamLfo2Cutoff,     "LFO 2 to Cutoff",         "Cutoff",   eSynthLibUnitGeneric, -64.0,           63.0,                64.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kSn2ParamOsc1Lfo2Pitch,  "Osc 1 LFO 2 Pitch",       "Pitch",    eSynthLibUnitGeneric, -64.0,           63.0,                64.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kSn2ParamOsc1Lfo2Width,  "Osc 1 LFO 2 Width",       "Width",    eSynthLibUnitGeneric, -64.0,           63.0,                64.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kSn2ParamOsc2Lfo2Pitch,  "Osc 2 LFO 2 Pitch",       "Pitch",    eSynthLibUnitGeneric, -64.0,           63.0,                64.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kSn2ParamOsc2Lfo2Width,  "Osc 2 LFO 2 Width",       "Width",    eSynthLibUnitGeneric, -64.0,           63.0,                64.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kSn2ParamOsc3Lfo2Pitch,  "Osc 3 LFO 2 Pitch",       "Pitch",    eSynthLibUnitGeneric, -64.0,           63.0,                64.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kSn2ParamOsc3Lfo2Width,  "Osc 3 LFO 2 Width",       "Width",    eSynthLibUnitGeneric, -64.0,           63.0,                64.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kSn2ParamLfo1Fade,       "LFO 1 Fade",              "Fade",     eSynthLibUnitIndexed,   0.0,            1.0,                         0.0,                  1, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kSn2ParamLfo2Fade,       "LFO 2 Fade",              "Fade",     eSynthLibUnitIndexed,   0.0,            1.0,                         0.0,                  1, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kSn2ParamHwLink,         "Link Hardware",           "Link",     eSynthLibUnitIndexed,   0.0,            1.0,                         0.0,                  1, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kSn2ParamHwChannel,      "Hardware MIDI Channel",   "Channel",  eSynthLibUnitGeneric,   1.0,           16.0,                  6.0 / 15.0,                 15, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kSn2ParamHwDevice,       "Hardware SysEx ID",       "SysEx ID", eSynthLibUnitGeneric,   0.0,          127.0,                 6.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kSn2ParamPan,            "Pan",                     "Pan",      eSynthLibUnitGeneric,   0.0,          127.0,                64.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kSn2ParamPanSpeed,       "Pan Speed",               "Speed",    eSynthLibUnitGeneric,   0.0,          127.0,                34.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kSn2ParamPanDepth,       "Pan Depth",               "Depth",    eSynthLibUnitGeneric,   0.0,          127.0,                         0.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kSn2ParamPanEffects,     "Pan Effects",             "Effects",  eSynthLibUnitIndexed,   0.0,            1.0,                         1.0,                  1, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
};

static const char *const kShapes[]          = {"SAW", "SQUARE", "DOUBLE"};
static const char *const kFilters[]         = {"LPF", "BPF", "HPF", "Res LPF", "Res BPF", "Res HPF", "Notch", "LPF+LPF", "BPF+BPF", "HPF+HPF", "LPF+BPF", "BPF+HPF"};
static const char *const kSlopes[]          = {"12", "18", "24"};

static const tSn2Control kOsc1[]            = {
    {kSn2ParamOscShape,     "Shape",  eTiControlChoice,   2,   0.0, kShapes},
    {kSn2ParamOsc1Octave,   "Octave", eTiControlDial,     4,  -2.0, NULL   },
    {kSn2ParamOsc1Semitone, "Semi",   eTiControlDial,    24, -12.0, NULL   },
    {kSn2ParamOsc1Fine,     "Fine",   eTiControlDial,   127, -64.0, NULL   },
    {kSn2ParamPulseWidth,   "Width",  eTiControlDial,   127,   0.0, NULL   },
    {kSn2ParamOscSoften,    "Soften", eTiControlDial,   127,   0.0, NULL   },
    {kSn2ParamOscLevel,     "Level",  eTiControlDial,   127,   0.0, NULL   },
};
static const tSn2Control kOsc2[]            = {
    {kSn2ParamOsc2Shape,    "Shape",  eTiControlChoice,   2,   0.0, kShapes},
    {kSn2ParamOsc2Octave,   "Octave", eTiControlDial,     4,  -2.0, NULL   },
    {kSn2ParamOsc2Semitone, "Semi",   eTiControlDial,    24, -12.0, NULL   },
    {kSn2ParamOsc2Fine,     "Fine",   eTiControlDial,   127, -64.0, NULL   },
    {kSn2ParamOsc2Width,    "Width",  eTiControlDial,   127,   0.0, NULL   },
    {kSn2ParamOsc2Soften,   "Soften", eTiControlDial,   127,   0.0, NULL   },
    {kSn2ParamOsc2Level,    "Level",  eTiControlDial,   127,   0.0, NULL   },
};
static const tSn2Control kOsc3[]            = {
    {kSn2ParamOsc3Shape,    "Shape",  eTiControlChoice,   2,   0.0, kShapes},
    {kSn2ParamOsc3Octave,   "Octave", eTiControlDial,     4,  -2.0, NULL   },
    {kSn2ParamOsc3Semitone, "Semi",   eTiControlDial,    24, -12.0, NULL   },
    {kSn2ParamOsc3Fine,     "Fine",   eTiControlDial,   127, -64.0, NULL   },
    {kSn2ParamOsc3Width,    "Width",  eTiControlDial,   127,   0.0, NULL   },
    {kSn2ParamOsc3Soften,   "Soften", eTiControlDial,   127,   0.0, NULL   },
    {kSn2ParamOsc3Level,    "Level",  eTiControlDial,   127,   0.0, NULL   },
};
static const tSn2Control kFilter[]          = {
    {kSn2ParamCutoff,      "Cutoff", eTiControlDial,   127, 0.0, NULL    },
    {kSn2ParamResonance,   "Reso",   eTiControlDial,   127, 0.0, NULL    },
    {kSn2ParamFilterType,  "Type",   eTiControlChoice,  11, 0.0, kFilters},
    {kSn2ParamFilterSlope, "Slope",  eTiControlChoice,   2, 0.0, kSlopes },
    {kSn2ParamFilterTrack, "Track",  eTiControlDial,   127, 0.0, NULL    },
    {kSn2ParamFilterDrive, "Drive",  eTiControlDial,   127, 0.0, NULL    },
    {kSn2ParamFilterQ,     "Q Norm", eTiControlDial,   127, 0.0, NULL    },
};
static const tSn2Control kAmpEnv[]          = {
    {kSn2ParamAmpAttack,      "Attack",  eTiControlDial, 127,   0.0, NULL},
    {kSn2ParamAmpDecay,       "Decay",   eTiControlDial, 127,   0.0, NULL},
    {kSn2ParamAmpSustain,     "Sustain", eTiControlDial, 127,   0.0, NULL},
    {kSn2ParamAmpSustainTime, "Time",    eTiControlDial, 127, -64.0, NULL},
    {kSn2ParamAmpRelease,     "Release", eTiControlDial, 127,   0.0, NULL},
    {kSn2ParamAmpVelocity,    "Vel",     eTiControlDial, 127,   0.0, NULL},
};
static const tSn2Control kAmp[]             = {
    {kSn2ParamVolume, "Prog Level", eTiControlDial, 127, 0.0, NULL},
    {kSn2ParamMaster, "Master",     eTiControlDial, 127, 0.0, NULL},
};
static const char *const kPolyModes[]       = {"Poly", "Mono"};
static const char *const kEnvTriggers[]     = {"Retrigger", "Legato"};
static const tSn2Control kVoices[]          = {
    {kSn2ParamVoiceLimit, "Limit",   eTiControlDial,   SN2_MAX_VOICES - 1, 1.0, NULL        },
    {kSn2ParamPolyMode,   "Mode",    eTiControlChoice,                  1, 0.0, kPolyModes  },
    {kSn2ParamEnvTrigger, "Trigger", eTiControlChoice,                  1, 0.0, kEnvTriggers},
};
static const tSn2Control kMixer[]           = {
    {kSn2ParamNoiseLevel,  "Noise",  eTiControlDial, 127, 0.0, NULL},
    {kSn2ParamNoiseSoften, "Soften", eTiControlDial, 127, 0.0, NULL},
    {kSn2ParamRing13,      "1x3",    eTiControlDial, 127, 0.0, NULL},
    {kSn2ParamRing23,      "2x3",    eTiControlDial, 127, 0.0, NULL},
};
static const char *const kChorusTypes[]     = {"QUAD", "CHORUS", "PHASER"};
static const char *const kOffOn[]           = {"OFF", "ON"};
static const tSn2Control kChorus[]          = {
    {kSn2ParamChorusType,     "Type",  eTiControlChoice,   2,   0.0, kChorusTypes},
    {kSn2ParamChorusSend,     "Send",  eTiControlDial,   127,   0.0, NULL        },
    {kSn2ParamChorusSpeed,    "Speed", eTiControlDial,   127,   0.0, NULL        },
    {kSn2ParamChorusDepth,    "Depth", eTiControlDial,   127,   0.0, NULL        },
    {kSn2ParamChorusFeedback, "Fback", eTiControlDial,   127, -64.0, NULL        },
};
static const char *const kDelayRatios[]     = {"1:1", "1:.75", ".75:1", "1:.66", ".66:1", "1:.5", ".5:1", "1:.33", ".33:1", "1:.25", ".25:1", "1:OFF", "OFF:1"};
static const char *const kDelaySyncs[]      = {"OFF", "32 T", "32", "16 T", "16", "8 T", "16 .", "8", "4 T", "8 .", "4", "2 T", "4 .", "2", "1B T", "2 .", "1B", "2B T", "1B .", "2B", "4B T", "3B", "5B T", "4B", "3B .", "7B T", "5B", "8B T", "6B", "7B", "5B .", "8B", "6B .", "7B .", "8B ."};
static const tSn2Control kDelay[]           = {
    {kSn2ParamDelaySync,     "Sync",  eTiControlChoice,  34, 0.0, kDelaySyncs },
    {kSn2ParamDelayRatio,    "Ratio", eTiControlChoice,  12, 0.0, kDelayRatios},
    {kSn2ParamDelaySend,     "Send",  eTiControlDial,   127, 0.0, NULL        },
    {kSn2ParamDelayTime,     "Time",  eTiControlDial,   127, 0.0, NULL        },
    {kSn2ParamDelayFeedback, "Fback", eTiControlDial,   127, 0.0, NULL        },
    {kSn2ParamDelayDamp,     "Damp",  eTiControlDial,   127, 0.0, NULL        },
    {kSn2ParamDelayWidth,    "Width", eTiControlDial,   127, 0.0, NULL        },
};
static const char *const kReverbTypes[]     = {"GATE REV", "GATE RISE", "GATE GENT", "GATE FALL", "DRY CHMB", "ECHO CHMB", "SMALL RM", "BIG RM", "MEDIUM 1", "MEDIUM 2", "M PLATE 1", "M PLATE 2", "LARGE 1", "LARGE 2", "L PLATE 1", "L PLATE 2"};
static const tSn2Control kReverb[]          = {
    {kSn2ParamReverbType,  "Type",  eTiControlChoice,  15, 0.0, kReverbTypes},
    {kSn2ParamReverbSend,  "Send",  eTiControlDial,   127, 0.0, NULL        },
    {kSn2ParamReverbDecay, "Decay", eTiControlDial,   127, 0.0, NULL        },
    {kSn2ParamReverbDamp,  "Damp",  eTiControlDial,   127, 0.0, NULL        },
};
static const char *const kOrders[]          = {"D+R+C", "D>R>C", "D>C>R", "R>D>C", "R>C>D", "C>D>R", "C>R>D", "D+[R>C]", "D+[C>R]", "R+[D>C]", "R+[C>D]", "C+[D>R]", "C+[R>D]", "D>[R+C]", "R>[D+C]", "C>[D+R]", "[R+C]>D", "[D+C]>R", "[D+R]>C"};
static const tSn2Control kDrive[]           = {
    {kSn2ParamDistLevel,  "Level",  eTiControlDial, 127, 0.0, NULL},
    {kSn2ParamDistComp,   "Comp",   eTiControlDial, 127, 0.0, NULL},
    {kSn2ParamDistOutput, "Output", eTiControlDial, 127, 0.0, NULL},
};
static const tSn2Control kEq[]              = {
    {kSn2ParamEqBass,   "Bass",   eTiControlDial, 127, -64.0, NULL},
    {kSn2ParamEqTreble, "Treble", eTiControlDial, 127, -64.0, NULL},
};
static const tSn2Control kEffectsMain[]     = {
    {kSn2ParamEffectsOrder, "Order", eTiControlChoice,  18, 0.0, kOrders},
    {kSn2ParamEffectsDry,   "Dry",   eTiControlDial,   127, 0.0, NULL   },
};
static const tSn2Section kEffectsPage[]     = {
    {"CHORUS", {0.30, 0.70, 0.85}, kChorus,      5},
    {"DELAY",  {0.85, 0.65, 0.30}, kDelay,       7},
    {"REVERB", {0.55, 0.45, 0.85}, kReverb,      4},
    {"DRIVE",  {0.90, 0.40, 0.30}, kDrive,       3},
    {"EQ",     {0.50, 0.75, 0.40}, kEq,          2},
    {"OUTPUT", {0.60, 0.60, 0.65}, kEffectsMain, 2},
};
static const char *const kFxBypass[]        = {"On", "Bypass"};
static const tSn2Control kGlobalFx[]        = {
    {kSn2ParamEffectsBypass, "Effects", eTiControlChoice, 1, 0.0, kFxBypass},
    {kSn2ParamDelayOn,       "Delay",   eTiControlChoice, 1, 0.0, kOffOn   },
    {kSn2ParamReverbOn,      "Reverb",  eTiControlChoice, 1, 0.0, kOffOn   },
};
static const char *const kGlideStyles[]     = {"PORTA", "GLISS"};
static const char *const kGlideCurves[]     = {"LINEAR", "EXP"};
static const char *const kGlideTypes[]      = {"NORMAL", "AUTO", "2 DOWN", "2 UP", "5 DOWN", "5 UP", "7 DOWN", "7 UP", "12 DOWN", "12 UP"};
static const char *const kTriggers[]        = {"PERCUSS", "ENSEMBLE"};
static const tSn2Control kGlide[]           = {
    {kSn2ParamGlideTime,  "Time",    eTiControlDial,   127, 0.0, NULL        },
    {kSn2ParamGlideStyle, "Style",   eTiControlChoice,   1, 0.0, kGlideStyles},
    {kSn2ParamGlideCurve, "Curve",   eTiControlChoice,   1, 0.0, kGlideCurves},
    {kSn2ParamGlideType,  "Type",    eTiControlChoice,   9, 0.0, kGlideTypes },
    {kSn2ParamOscTrigger, "Trigger", eTiControlChoice,   1, 0.0, kTriggers   },
};
static const tSn2Section kOscillatorsPage[] = {
    {"OSC 1", {0.30, 0.55, 0.85}, kOsc1,  7},
    {"MIXER", {0.60, 0.60, 0.65}, kMixer, 4},
    {"OSC 2", {0.30, 0.70, 0.60}, kOsc2,  7},
    {"GLIDE", {0.60, 0.60, 0.65}, kGlide, 5},
    {"OSC 3", {0.85, 0.55, 0.30}, kOsc3,  7},
};
static const tSn2Section kFilterPage[]      = {
    {"FILTER", {0.90, 0.55, 0.20}, kFilter, 7},
};
static const tSn2Control kEnv2[]            = {
    {kSn2ParamEnv2Attack,   "Attack",  eTiControlDial, 127,   0.0, NULL},
    {kSn2ParamEnv2Decay,    "Decay",   eTiControlDial, 127,   0.0, NULL},
    {kSn2ParamEnv2Sustain,  "Sustain", eTiControlDial, 127,   0.0, NULL},
    {kSn2ParamEnv2Release,  "Release", eTiControlDial, 127,   0.0, NULL},
    {kSn2ParamEnv2Velocity, "Vel",     eTiControlDial, 127,   0.0, NULL},
    {kSn2ParamEnv2Cutoff,   "Cutoff",  eTiControlDial, 127, -64.0, NULL},
};
static const tSn2Control kEnv3[]            = {
    {kSn2ParamEnv3Attack,   "Attack",  eTiControlDial, 127,   0.0, NULL},
    {kSn2ParamEnv3Decay,    "Decay",   eTiControlDial, 127,   0.0, NULL},
    {kSn2ParamEnv3Sustain,  "Sustain", eTiControlDial, 127,   0.0, NULL},
    {kSn2ParamEnv3Release,  "Release", eTiControlDial, 127,   0.0, NULL},
    {kSn2ParamEnv3Velocity, "Vel",     eTiControlDial, 127,   0.0, NULL},
    {kSn2ParamEnv3Cutoff,   "Cutoff",  eTiControlDial, 127, -64.0, NULL},
};
static const tSn2Section kAmpPage[]         = {
    {"OUTPUT", {0.65, 0.65, 0.65}, kAmp, 2},
};
static const tSn2Section kEnvelopesPage[]   = {
    {"AMP ENV", {0.85, 0.75, 0.25}, kAmpEnv, 6},
    {"ENV 2",   {0.90, 0.55, 0.20}, kEnv2,   6},
    {"ENV 3",   {0.70, 0.45, 0.80}, kEnv3,   6},
};
static const char *const kLfoShapes[]       = {"Square", "Saw", "Triangle", "S+H"};
static const char *const kLfoRanges[]       = {"x1", "x10", "x100"};
static const char *const kLfoTriggers[]     = {"Free", "Key"};
static const char *const kLfoFades[]        = {"In", "Out"};
#define LFO_CONTROLS(n)    {                                                              \
       {kSn2ParamLfo ## n ## Shape, "Shape", eTiControlChoice, 3, 0.0, kLfoShapes},       \
       {kSn2ParamLfo ## n ## Speed, "Speed", eTiControlDial, 127, 0.0, NULL},             \
       {kSn2ParamLfo ## n ## Range, "Range", eTiControlChoice, 2, 0.0, kLfoRanges},       \
       {kSn2ParamLfo ## n ## Delay, "Delay", eTiControlDial, 127, 0.0, NULL},             \
       {kSn2ParamLfo ## n ## Fade, "Fade", eTiControlChoice, 1, 0.0, kLfoFades},          \
       {kSn2ParamLfo ## n ## Offset, "Offset", eTiControlDial, 127, 0.0, NULL},           \
       {kSn2ParamLfo ## n ## Trigger, "Trigger", eTiControlChoice, 1, 0.0, kLfoTriggers}, \
       {kSn2ParamLfo ## n ## Cutoff, "Cutoff", eTiControlDial, 127, -64.0, NULL},         \
}
static const tSn2Control kLfo1[]            = LFO_CONTROLS(1);
static const tSn2Control kLfo2[]            = LFO_CONTROLS(2);
static const tSn2Section kLfoPage[]         = {
    {"LFO 1", {0.80, 0.55, 0.85}, kLfo1, 8},
    {"LFO 2", {0.55, 0.80, 0.85}, kLfo2, 8},
};
static const tSn2Control kHardware[]        = {
    {kSn2ParamHwLink,    "Link",     eTiControlChoice,   1, 0.0, kOffOn},
    {kSn2ParamHwChannel, "Channel",  eTiControlDial,    15, 1.0, NULL  },
    {kSn2ParamHwDevice,  "SysEx ID", eTiControlDial,   127, 0.0, NULL  },
};
static const tSn2Control kPan[]             = {
    {kSn2ParamPan,        "Pan",     eTiControlDial,   127, 0.0, NULL  },
    {kSn2ParamPanSpeed,   "Speed",   eTiControlDial,   127, 0.0, NULL  },
    {kSn2ParamPanDepth,   "Depth",   eTiControlDial,   127, 0.0, NULL  },
    {kSn2ParamPanEffects, "Effects", eTiControlChoice,   1, 0.0, kOffOn},
};
static const tSn2Section kGlobalPage[]      = {
    {"PLUG-IN",  {0.65, 0.65, 0.65}, kVoices,   3},
    {"EFFECTS",  {0.65, 0.65, 0.65}, kGlobalFx, 3},
    {"HARDWARE", {0.65, 0.65, 0.65}, kHardware, 3},
    {"PAN",      {0.65, 0.65, 0.65}, kPan,      4},
};

const tSn2Page           gSn2Pages[]        = {
    {"OSCILLATORS", kOscillatorsPage, 5},
    {"FILTER",      kFilterPage,      1},
    {"AMP",         kAmpPage,         1},
    {"ENVELOPES",   kEnvelopesPage,   3},
    {"LFOS",        kLfoPage,         2},
    {"EFFECTS",     kEffectsPage,     6},
    {"GLOBAL",      kGlobalPage,      4},
};
const uint32_t           gSn2PageCount      = sizeof(gSn2Pages) / sizeof(gSn2Pages[0]);

tSn2EffectsSound sn2_effects_sound(const double * v, double tempo) {
    return (tSn2EffectsSound){
        .dry            = v[kSn2ParamEffectsDry] * 127.0,
        .chorusSend     = v[kSn2ParamChorusSend] * 127.0,
        .chorusSpeed    = v[kSn2ParamChorusSpeed] * 127.0,
        .chorusDepth    = v[kSn2ParamChorusDepth] * 127.0,
        .chorusFeedback = v[kSn2ParamChorusFeedback] * 127.0,
        .chorusType     = (int)lround(v[kSn2ParamChorusType] * 2.0),
        .delayOn        = v[kSn2ParamDelayOn] > 0.5,
        .delaySend      = v[kSn2ParamDelaySend] * 127.0,
        .delayTime      = v[kSn2ParamDelayTime] * 127.0,
        .delayFeedback  = v[kSn2ParamDelayFeedback] * 127.0,
        .delayDamp      = v[kSn2ParamDelayDamp] * 127.0,
        .delayWidth     = v[kSn2ParamDelayWidth] * 127.0,
        .delayRatio     = (int)lround(v[kSn2ParamDelayRatio] * 12.0),
        .delaySync      = (int)lround(v[kSn2ParamDelaySync] * 34.0),
        .tempo          = tempo,
        .reverbOn       = v[kSn2ParamReverbOn] > 0.5,
        .order          = (int)lround(v[kSn2ParamEffectsOrder] * 18.0),
        .reverbSend     = v[kSn2ParamReverbSend] * 127.0,
        .reverbDecay    = v[kSn2ParamReverbDecay] * 127.0,
        .reverbDamp     = v[kSn2ParamReverbDamp] * 127.0,
        .reverbType     = (int)lround(v[kSn2ParamReverbType] * 15.0),
        .distLevel      = v[kSn2ParamDistLevel] * 127.0,
        .distComp       = v[kSn2ParamDistComp] * 127.0,
        .distOutput     = v[kSn2ParamDistOutput] * 127.0,
        .eqBass         = v[kSn2ParamEqBass] * 127.0,
        .eqTreble       = v[kSn2ParamEqTreble] * 127.0,
        .pan            = v[kSn2ParamPan] * 127.0,
        .panSpeed       = v[kSn2ParamPanSpeed] * 127.0,
        .panDepth       = v[kSn2ParamPanDepth] * 127.0,
        .panEffects     = v[kSn2ParamPanEffects] > 0.5,
    };
}

// voices notes §15: pitch and cutoff go as the square of the amount, width in step with it
static double lfo_semis(double normalized) {
    double d = (normalized * 127.0) - 64.0;

    return ((d < 0.0) ? -1.0 : 1.0) * d * d / 57.6;
}

static double lfo_delay(double dial) {
    if (dial <= 0.0) {
        return 0.0;
    }
    return (dial < 64.0) ? exp(-1.08 + (0.0417 * (dial - 64.0))) : exp(-1.08 + (0.0525 * (dial - 64.0)));
}

void sn2_controls_sound(const double * v, tSn2Sound * sound) {
    for (int l = 0; l < SN2_LFOS; l++) {
        int    at    = l * (kSn2ParamLfo2Shape - kSn2ParamLfo1Shape);
        double speed = v[kSn2ParamLfo1Speed + at] * 127.0;

        sound->lfo[l]       = (tSn2LfoSound){
            .shape   = (int)lround(v[kSn2ParamLfo1Shape + at] * 3.0),
            .hz      = (speed * speed / 1638.4) * pow(10.0, lround(v[kSn2ParamLfo1Range + at] * 2.0)),
            .delay   = lfo_delay(v[kSn2ParamLfo1Delay + at] * 127.0),
            .fadeOut = v[kSn2ParamLfo1Fade + l] > 0.5,
            .offset  = v[kSn2ParamLfo1Offset + at],
            .keyed   = v[kSn2ParamLfo1Trigger + at] > 0.5,
        };
        sound->lfoCutoff[l] = lfo_semis(v[kSn2ParamLfo1Cutoff + at]);

        for (int o = 0; o < SN2_OSCS; o++) {
            sound->osc[o].lfoPitch[l] = lfo_semis(v[kSn2ParamOsc1Lfo1Pitch + at + (2 * o)]);
            sound->osc[o].lfoWidth[l] = ((v[kSn2ParamOsc1Lfo1Width + at + (2 * o)] * 127.0) - 64.0) / 64.0;
        }
    }

    sound->bend           = (2.0 * v[kSn2ParamPitchBend]) - 1.0;
    sound->wheel          = v[kSn2ParamModWheel];
    sound->pedal          = v[kSn2ParamSustainPedal] >= 0.5;
    sound->mono           = v[kSn2ParamPolyMode] > 0.5;
    sound->legato         = v[kSn2ParamEnvTrigger] > 0.5;
    sound->wheelCutoff    = (v[kSn2ParamWheelCutoff] * 127.0) - 64.0;
    sound->wheelResonance = (v[kSn2ParamWheelResonance] * 127.0) - 64.0;

    for (int o = 0; o < SN2_OSCS; o++) {
        sound->osc[o].bendRange  = (v[kSn2ParamOsc1BendRange + o] * 24.0) - 12.0;
        sound->osc[o].wheelPitch = (v[kSn2ParamOsc1WheelPitch + o] * 127.0) - 64.0;
        sound->osc[o].wheelLevel = ((v[kSn2ParamOsc1WheelLevel + o] * 127.0) - 64.0) / 64.0;
    }
}
