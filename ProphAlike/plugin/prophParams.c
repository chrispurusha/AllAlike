/*
 * ProphAlike - the parameters and the panel pages.
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
// Notes: Docs/code-notes/prophParams.md - "// notes §k" refers there.

#include <math.h>

#include "prophParams.h"
#include "synthlibDefs.h"
#include "prophVoices.h"

#define LIST    SYNTHLIB_PARAM_LIST

// notes §1: the panel's 0..127 is each dial's plain range, as on the instrument
const tSynthLibParam     gProphParams[kProphParamCount] = {
    {kProphParamVoiceLimit,       "Voices",                  "Voices",   eSynthLibUnitGeneric,   1.0, PROPH_MAX_VOICES, 15.0 / (PROPH_MAX_VOICES - 1), PROPH_MAX_VOICES - 1, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kProphParamVolume,           "Prog Level",              "Level",    eSynthLibUnitGeneric,   0.0,          127.0,               100.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kProphParamOscShape,         "Osc Shape",               "Shape",    eSynthLibUnitIndexed,   0.0,            2.0,                         0.0,                  2, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kProphParamPulseWidth,       "Osc Pulse Width",         "PW",       eSynthLibUnitGeneric,   0.0,          127.0,                64.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kProphParamCutoff,           "Cutoff",                  "Cutoff",   eSynthLibUnitGeneric,   0.0,          127.0,                         1.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kProphParamResonance,        "Resonance",               "Reso",     eSynthLibUnitGeneric,   0.0,          127.0,                         0.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kProphParamFilterType,       "Filter Type",             "Type",     eSynthLibUnitIndexed,   0.0,           11.0,                         0.0,                 11, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kProphParamFilterSlope,      "Filter Slope",            "Slope",    eSynthLibUnitIndexed,   0.0,            2.0,                         0.0,                  2, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kProphParamAmpAttack,        "Amp Attack",              "Attack",   eSynthLibUnitGeneric,   0.0,          127.0,                         0.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kProphParamAmpDecay,         "Amp Decay",               "Decay",    eSynthLibUnitGeneric,   0.0,          127.0,                         1.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kProphParamAmpSustain,       "Amp Sustain",             "Sustain",  eSynthLibUnitGeneric,   0.0,          127.0,                         1.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kProphParamAmpSustainTime,   "Amp Sustain Time",        "Time",     eSynthLibUnitGeneric,   0.0,          127.0,                         1.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kProphParamAmpRelease,       "Amp Release",             "Release",  eSynthLibUnitGeneric,   0.0,          127.0,                 4.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kProphParamProgram,          "Program",                 "Program",  eSynthLibUnitGeneric,   0.0,          127.0,                    0.000000,                127, SYNTHLIB_MIDI_NONE,       SYNTHLIB_PARAM_PROGRAM | SYNTHLIB_PARAM_HIDDEN | SYNTHLIB_PARAM_NO_SAVE},
    {kProphParamBank,             "Bank Select",             "Bank",     eSynthLibUnitGeneric,   0.0,          127.0,                    0.000000,                127,                       32, SYNTHLIB_PARAM_HIDDEN | SYNTHLIB_PARAM_NO_SAVE                         },
    {kProphParamOscSoften,        "Osc Soften",              "Soften",   eSynthLibUnitGeneric,   0.0,          127.0,                         1.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kProphParamOscLevel,         "Osc Level",               "Level",    eSynthLibUnitGeneric,   0.0,          127.0,                         1.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kProphParamOsc2Shape,        "Osc 2 Shape",             "Shape",    eSynthLibUnitIndexed,   0.0,            2.0,                         0.0,                  2, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kProphParamOsc2Width,        "Osc 2 Pulse Width",       "PW",       eSynthLibUnitGeneric,   0.0,          127.0,                64.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kProphParamOsc2Soften,       "Osc 2 Soften",            "Soften",   eSynthLibUnitGeneric,   0.0,          127.0,                         1.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kProphParamOsc2Level,        "Osc 2 Level",             "Level",    eSynthLibUnitGeneric,   0.0,          127.0,                         0.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kProphParamOsc3Shape,        "Osc 3 Shape",             "Shape",    eSynthLibUnitIndexed,   0.0,            2.0,                         0.0,                  2, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kProphParamOsc3Width,        "Osc 3 Pulse Width",       "PW",       eSynthLibUnitGeneric,   0.0,          127.0,                64.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kProphParamOsc3Soften,       "Osc 3 Soften",            "Soften",   eSynthLibUnitGeneric,   0.0,          127.0,                         1.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kProphParamOsc3Level,        "Osc 3 Level",             "Level",    eSynthLibUnitGeneric,   0.0,          127.0,                         0.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kProphParamOsc1Octave,       "Osc 1 Octave",            "Octave",   eSynthLibUnitGeneric,  -2.0,            2.0,                         0.5,                  4, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kProphParamOsc1Semitone,     "Osc 1 Semitone",          "Semi",     eSynthLibUnitGeneric, -12.0,           12.0,                         0.5,                 24, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kProphParamOsc1Fine,         "Osc 1 Fine Tune",         "Fine",     eSynthLibUnitGeneric, -64.0,           63.0,                64.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kProphParamOsc2Octave,       "Osc 2 Octave",            "Octave",   eSynthLibUnitGeneric,  -2.0,            2.0,                         0.5,                  4, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kProphParamOsc2Semitone,     "Osc 2 Semitone",          "Semi",     eSynthLibUnitGeneric, -12.0,           12.0,                         0.5,                 24, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kProphParamOsc2Fine,         "Osc 2 Fine Tune",         "Fine",     eSynthLibUnitGeneric, -64.0,           63.0,                64.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kProphParamOsc3Octave,       "Osc 3 Octave",            "Octave",   eSynthLibUnitGeneric,  -2.0,            2.0,                         0.5,                  4, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kProphParamOsc3Semitone,     "Osc 3 Semitone",          "Semi",     eSynthLibUnitGeneric, -12.0,           12.0,                         0.5,                 24, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kProphParamOsc3Fine,         "Osc 3 Fine Tune",         "Fine",     eSynthLibUnitGeneric, -64.0,           63.0,                64.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kProphParamFilterTrack,      "Filter Tracking",         "Track",    eSynthLibUnitGeneric,   0.0,          127.0,                         1.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kProphParamFilterDrive,      "Filter Overdrive",        "Drive",    eSynthLibUnitGeneric,   0.0,          127.0,                         0.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kProphParamAmpVelocity,      "Amp Velocity",            "Velocity", eSynthLibUnitGeneric,   0.0,          127.0,                64.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kProphParamEnv2Attack,       "Env 2 Attack",            "Attack",   eSynthLibUnitGeneric,   0.0,          127.0,                         0.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kProphParamEnv2Decay,        "Env 2 Decay",             "Decay",    eSynthLibUnitGeneric,   0.0,          127.0,                         0.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kProphParamEnv2Sustain,      "Env 2 Sustain",           "Sustain",  eSynthLibUnitGeneric,   0.0,          127.0,                         1.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kProphParamEnv2Release,      "Env 2 Release",           "Release",  eSynthLibUnitGeneric,   0.0,          127.0,                         0.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kProphParamEnv2Velocity,     "Env 2 Velocity",          "Vel",      eSynthLibUnitGeneric,   0.0,          127.0,                64.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kProphParamEnv2Cutoff,       "Env 2 to Cutoff",         "Cutoff",   eSynthLibUnitGeneric, -64.0,           63.0,                64.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kProphParamEnv3Attack,       "Env 3 Attack",            "Attack",   eSynthLibUnitGeneric,   0.0,          127.0,                         0.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kProphParamEnv3Decay,        "Env 3 Decay",             "Decay",    eSynthLibUnitGeneric,   0.0,          127.0,                         0.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kProphParamEnv3Sustain,      "Env 3 Sustain",           "Sustain",  eSynthLibUnitGeneric,   0.0,          127.0,                         1.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kProphParamEnv3Release,      "Env 3 Release",           "Release",  eSynthLibUnitGeneric,   0.0,          127.0,                         0.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kProphParamEnv3Velocity,     "Env 3 Velocity",          "Vel",      eSynthLibUnitGeneric,   0.0,          127.0,                64.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kProphParamEnv3Cutoff,       "Env 3 to Cutoff",         "Cutoff",   eSynthLibUnitGeneric, -64.0,           63.0,                64.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kProphParamNoiseLevel,       "Noise Level",             "Noise",    eSynthLibUnitGeneric,   0.0,          127.0,                         0.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kProphParamNoiseSoften,      "Noise Soften",            "Soften",   eSynthLibUnitGeneric,   0.0,          127.0,                         1.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kProphParamRing13,           "Ring 1x3",                "1x3",      eSynthLibUnitGeneric,   0.0,          127.0,                         0.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kProphParamRing23,           "Ring 2x3",                "2x3",      eSynthLibUnitGeneric,   0.0,          127.0,                         0.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kProphParamFilterQ,          "Filter Q Normalise",      "Q Norm",   eSynthLibUnitGeneric,   0.0,          127.0,                         0.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kProphParamEffectsDry,       "Effects Dry",             "Dry",      eSynthLibUnitGeneric,   0.0,          127.0,                         1.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kProphParamChorusSend,       "Chorus Send",             "Send",     eSynthLibUnitGeneric,   0.0,          127.0,                         0.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kProphParamChorusSpeed,      "Chorus Speed",            "Speed",    eSynthLibUnitGeneric,   0.0,          127.0,                27.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kProphParamChorusDepth,      "Chorus Depth",            "Depth",    eSynthLibUnitGeneric,   0.0,          127.0,                33.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kProphParamChorusFeedback,   "Chorus Feedback",         "Fback",    eSynthLibUnitGeneric,   0.0,          127.0,                64.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kProphParamChorusType,       "Chorus Type",             "Type",     eSynthLibUnitIndexed,   0.0,            2.0,                         0.5,                  2, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kProphParamDelayOn,          "Global Delay",            "Delay",    eSynthLibUnitIndexed,   0.0,            1.0,                         1.0,                  1, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kProphParamReverbOn,         "Global Reverb",           "Reverb",   eSynthLibUnitIndexed,   0.0,            1.0,                         1.0,                  1, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kProphParamDelaySend,        "Delay Send",              "Send",     eSynthLibUnitGeneric,   0.0,          127.0,                         0.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kProphParamDelayTime,        "Delay Time",              "Time",     eSynthLibUnitGeneric,   0.0,          127.0,                75.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kProphParamDelayFeedback,    "Delay Feedback",          "Fback",    eSynthLibUnitGeneric,   0.0,          127.0,                50.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kProphParamDelayDamp,        "Delay HF Damp",           "Damp",     eSynthLibUnitGeneric,   0.0,          127.0,                20.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kProphParamDelayWidth,       "Delay Width",             "Width",    eSynthLibUnitGeneric,   0.0,          127.0,                         1.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kProphParamDelayRatio,       "Delay Ratio",             "Ratio",    eSynthLibUnitIndexed,   0.0,           12.0,                         0.0,                 12, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kProphParamDelaySync,        "Delay Sync",              "Sync",     eSynthLibUnitIndexed,   0.0,           34.0,                         0.0,                 34, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kProphParamGlideTime,        "Glide Time",              "Time",     eSynthLibUnitGeneric,   0.0,          127.0,                         0.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kProphParamGlideStyle,       "Glide Style",             "Style",    eSynthLibUnitIndexed,   0.0,            1.0,                         0.0,                  1, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kProphParamGlideCurve,       "Glide Curve",             "Curve",    eSynthLibUnitIndexed,   0.0,            1.0,                         1.0,                  1, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kProphParamGlideType,        "Glide Type",              "Type",     eSynthLibUnitIndexed,   0.0,            9.0,                   1.0 / 9.0,                  9, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kProphParamReverbSend,       "Reverb Send",             "Send",     eSynthLibUnitGeneric,   0.0,          127.0,                         0.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kProphParamReverbDecay,      "Reverb Decay",            "Decay",    eSynthLibUnitGeneric,   0.0,          127.0,                80.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kProphParamReverbDamp,       "Reverb HF Damp",          "Damp",     eSynthLibUnitGeneric,   0.0,          127.0,                18.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kProphParamReverbType,       "Reverb Type",             "Type",     eSynthLibUnitIndexed,   0.0,           15.0,                  9.0 / 15.0,                 15, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kProphParamEffectsOrder,     "Effects Order",           "Order",    eSynthLibUnitIndexed,   0.0,           18.0,                         0.0,                 18, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kProphParamOscTrigger,       "Osc Trigger",             "Trigger",  eSynthLibUnitIndexed,   0.0,            1.0,                         0.0,                  1, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kProphParamDistLevel,        "Distortion Level",        "Level",    eSynthLibUnitGeneric,   0.0,          127.0,                         0.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kProphParamDistComp,         "Distortion Compensation", "Comp",     eSynthLibUnitGeneric,   0.0,          127.0,               112.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kProphParamDistOutput,       "Distortion Output",       "Output",   eSynthLibUnitGeneric,   0.0,          127.0,                64.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kProphParamEqBass,           "EQ Bass",                 "Bass",     eSynthLibUnitGeneric,   0.0,          127.0,                64.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kProphParamEqTreble,         "EQ Treble",               "Treble",   eSynthLibUnitGeneric,   0.0,          127.0,                64.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kProphParamPitchBend,        "Pitch Bend",              "Bend",     eSynthLibUnitGeneric,   0.0,            1.0,                         0.5,                  0, SYNTHLIB_MIDI_PITCH_BEND, SYNTHLIB_PARAM_HIDDEN | SYNTHLIB_PARAM_NO_SAVE                         },
    {kProphParamModWheel,         "Mod Wheel",               "Wheel",    eSynthLibUnitGeneric,   0.0,            1.0,                         0.0,                  0, SYNTHLIB_MIDI_MOD_WHEEL,  SYNTHLIB_PARAM_HIDDEN | SYNTHLIB_PARAM_NO_SAVE                         },
    {kProphParamOsc1BendRange,    "Osc 1 Bend Range",        "Bend",     eSynthLibUnitGeneric, -12.0,           12.0,                 14.0 / 24.0,                 24, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kProphParamOsc2BendRange,    "Osc 2 Bend Range",        "Bend",     eSynthLibUnitGeneric, -12.0,           12.0,                 14.0 / 24.0,                 24, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kProphParamOsc3BendRange,    "Osc 3 Bend Range",        "Bend",     eSynthLibUnitGeneric, -12.0,           12.0,                 14.0 / 24.0,                 24, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kProphParamOsc1WheelPitch,   "Osc 1 Wheel Pitch",       "Wheel",    eSynthLibUnitGeneric, -64.0,           63.0,                64.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kProphParamOsc2WheelPitch,   "Osc 2 Wheel Pitch",       "Wheel",    eSynthLibUnitGeneric, -64.0,           63.0,                64.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kProphParamOsc3WheelPitch,   "Osc 3 Wheel Pitch",       "Wheel",    eSynthLibUnitGeneric, -64.0,           63.0,                64.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kProphParamOsc1WheelLevel,   "Osc 1 Wheel Level",       "Wheel",    eSynthLibUnitGeneric, -64.0,           63.0,                64.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kProphParamOsc2WheelLevel,   "Osc 2 Wheel Level",       "Wheel",    eSynthLibUnitGeneric, -64.0,           63.0,                64.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kProphParamOsc3WheelLevel,   "Osc 3 Wheel Level",       "Wheel",    eSynthLibUnitGeneric, -64.0,           63.0,                64.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kProphParamWheelCutoff,      "Wheel Cutoff",            "Wheel",    eSynthLibUnitGeneric, -64.0,           63.0,                64.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kProphParamWheelResonance,   "Wheel Resonance",         "Wheel",    eSynthLibUnitGeneric, -64.0,           63.0,                64.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kProphParamPolyMode,         "Poly Mode",               "Mode",     eSynthLibUnitIndexed,   0.0,            1.0,                         0.0,                  1, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kProphParamEnvTrigger,       "Env Trigger",             "Trigger",  eSynthLibUnitIndexed,   0.0,            1.0,                         0.0,                  1, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kProphParamMaster,           "Master Volume",           "Master",   eSynthLibUnitGeneric,   0.0,          127.0,                         1.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kProphParamEffectsBypass,    "Effects Bypass",          "Effects",  eSynthLibUnitIndexed,   0.0,            1.0,                         0.0,                  1, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kProphParamSustainPedal,     "Sustain Pedal",           "Pedal",    eSynthLibUnitGeneric,   0.0,            1.0,                         0.0,                  0, SYNTHLIB_MIDI_SUSTAIN,    SYNTHLIB_PARAM_HIDDEN | SYNTHLIB_PARAM_NO_SAVE                         },
    {kProphParamLfo1Shape,        "LFO 1 Shape",             "Shape",    eSynthLibUnitIndexed,   0.0,            3.0,                   2.0 / 3.0,                  3, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kProphParamLfo1Speed,        "LFO 1 Speed",             "Speed",    eSynthLibUnitGeneric,   0.0,          127.0,                64.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kProphParamLfo1Range,        "LFO 1 Range",             "Range",    eSynthLibUnitIndexed,   0.0,            2.0,                         0.0,                  2, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kProphParamLfo1Delay,        "LFO 1 Delay",             "Delay",    eSynthLibUnitGeneric,   0.0,          127.0,                         0.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kProphParamLfo1Offset,       "LFO 1 Offset",            "Offset",   eSynthLibUnitGeneric,   0.0,          127.0,                         0.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kProphParamLfo1Trigger,      "LFO 1 Trigger",           "Trigger",  eSynthLibUnitIndexed,   0.0,            1.0,                         1.0,                  1, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kProphParamLfo1Cutoff,       "LFO 1 to Cutoff",         "Cutoff",   eSynthLibUnitGeneric, -64.0,           63.0,                64.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kProphParamOsc1Lfo1Pitch,    "Osc 1 LFO 1 Pitch",       "Pitch",    eSynthLibUnitGeneric, -64.0,           63.0,                64.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kProphParamOsc1Lfo1Width,    "Osc 1 LFO 1 Width",       "Width",    eSynthLibUnitGeneric, -64.0,           63.0,                64.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kProphParamOsc2Lfo1Pitch,    "Osc 2 LFO 1 Pitch",       "Pitch",    eSynthLibUnitGeneric, -64.0,           63.0,                64.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kProphParamOsc2Lfo1Width,    "Osc 2 LFO 1 Width",       "Width",    eSynthLibUnitGeneric, -64.0,           63.0,                64.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kProphParamOsc3Lfo1Pitch,    "Osc 3 LFO 1 Pitch",       "Pitch",    eSynthLibUnitGeneric, -64.0,           63.0,                64.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kProphParamOsc3Lfo1Width,    "Osc 3 LFO 1 Width",       "Width",    eSynthLibUnitGeneric, -64.0,           63.0,                64.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kProphParamLfo2Shape,        "LFO 2 Shape",             "Shape",    eSynthLibUnitIndexed,   0.0,            3.0,                   2.0 / 3.0,                  3, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kProphParamLfo2Speed,        "LFO 2 Speed",             "Speed",    eSynthLibUnitGeneric,   0.0,          127.0,                64.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kProphParamLfo2Range,        "LFO 2 Range",             "Range",    eSynthLibUnitIndexed,   0.0,            2.0,                         0.0,                  2, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kProphParamLfo2Delay,        "LFO 2 Delay",             "Delay",    eSynthLibUnitGeneric,   0.0,          127.0,                         0.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kProphParamLfo2Offset,       "LFO 2 Offset",            "Offset",   eSynthLibUnitGeneric,   0.0,          127.0,                         0.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kProphParamLfo2Trigger,      "LFO 2 Trigger",           "Trigger",  eSynthLibUnitIndexed,   0.0,            1.0,                         1.0,                  1, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kProphParamLfo2Cutoff,       "LFO 2 to Cutoff",         "Cutoff",   eSynthLibUnitGeneric, -64.0,           63.0,                64.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kProphParamOsc1Lfo2Pitch,    "Osc 1 LFO 2 Pitch",       "Pitch",    eSynthLibUnitGeneric, -64.0,           63.0,                64.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kProphParamOsc1Lfo2Width,    "Osc 1 LFO 2 Width",       "Width",    eSynthLibUnitGeneric, -64.0,           63.0,                64.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kProphParamOsc2Lfo2Pitch,    "Osc 2 LFO 2 Pitch",       "Pitch",    eSynthLibUnitGeneric, -64.0,           63.0,                64.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kProphParamOsc2Lfo2Width,    "Osc 2 LFO 2 Width",       "Width",    eSynthLibUnitGeneric, -64.0,           63.0,                64.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kProphParamOsc3Lfo2Pitch,    "Osc 3 LFO 2 Pitch",       "Pitch",    eSynthLibUnitGeneric, -64.0,           63.0,                64.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kProphParamOsc3Lfo2Width,    "Osc 3 LFO 2 Width",       "Width",    eSynthLibUnitGeneric, -64.0,           63.0,                64.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kProphParamLfo1Fade,         "LFO 1 Fade",              "Fade",     eSynthLibUnitIndexed,   0.0,            1.0,                         0.0,                  1, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kProphParamLfo2Fade,         "LFO 2 Fade",              "Fade",     eSynthLibUnitIndexed,   0.0,            1.0,                         0.0,                  1, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kProphParamHwLink,           "Link Hardware",           "Link",     eSynthLibUnitIndexed,   0.0,            1.0,                         0.0,                  1, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kProphParamHwChannel,        "Hardware MIDI Channel",   "Channel",  eSynthLibUnitGeneric,   1.0,           16.0,                  6.0 / 15.0,                 15, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kProphParamHwDevice,         "Hardware Device ID",      "Device",   eSynthLibUnitGeneric,   0.0,          127.0,                 6.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kProphParamPan,              "Pan",                     "Pan",      eSynthLibUnitGeneric,   0.0,          127.0,                64.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kProphParamPanSpeed,         "Pan Speed",               "Speed",    eSynthLibUnitGeneric,   0.0,          127.0,                34.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kProphParamPanDepth,         "Pan Depth",               "Depth",    eSynthLibUnitGeneric,   0.0,          127.0,                         0.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kProphParamPanEffects,       "Pan Effects",             "Effects",  eSynthLibUnitIndexed,   0.0,            1.0,                         1.0,                  1, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kProphParamAntiAlias,        "Anti-alias",              "AA",       eSynthLibUnitIndexed,   0.0,            1.0,                         1.0,                  1, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kProphParamSameNote,         "Same Note",               "Same",     eSynthLibUnitIndexed,   0.0,            1.0,                         1.0,                  1, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kProphParamFm13,             "FM 1x3",                  "FM 1x3",   eSynthLibUnitIndexed,   0.0,            1.0,                         0.0,                  1, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kProphParamFm23,             "FM 2x3",                  "FM 2x3",   eSynthLibUnitIndexed,   0.0,            1.0,                         0.0,                  1, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kProphParamFmNoise,          "FM Noise",                "FM Noise", eSynthLibUnitIndexed,   0.0,            1.0,                         0.0,                  1, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kProphParamAmpSustainRate,   "Amp Sustain Rate",        "Rate",     eSynthLibUnitGeneric, -64.0,           63.0,                64.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kProphParamEnv2SustainTime,  "Env 2 Sustain Time",      "Time",     eSynthLibUnitGeneric,   0.0,          127.0,                         1.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kProphParamEnv2SustainRate,  "Env 2 Sustain Rate",      "Rate",     eSynthLibUnitGeneric, -64.0,           63.0,                64.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kProphParamEnv3SustainTime,  "Env 3 Sustain Time",      "Time",     eSynthLibUnitGeneric,   0.0,          127.0,                         1.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kProphParamEnv3SustainRate,  "Env 3 Sustain Rate",      "Rate",     eSynthLibUnitGeneric, -64.0,           63.0,                64.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kProphParamOneShot,          "One Shot",                "One Shot", eSynthLibUnitIndexed,   0.0,            1.0,                         0.0,                  1, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kProphParamOscsBypass,       "Filter Oscs Bypass",      "Bypass",   eSynthLibUnitIndexed,   0.0,            1.0,                         0.0,                  1, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kProphParamRing13WheelLevel, "1x3 Wheel Level",         "Wheel",    eSynthLibUnitGeneric, -64.0,           63.0,                64.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kProphParamRing23WheelLevel, "2x3 Wheel Level",         "Wheel",    eSynthLibUnitGeneric, -64.0,           63.0,                64.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kProphParamNoiseWheelLevel,  "Noise Wheel Level",       "Wheel",    eSynthLibUnitGeneric, -64.0,           63.0,                64.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kProphParamOsc1Env2Level,    "Osc 1 Env 2 Level",       "Env 2",    eSynthLibUnitGeneric, -64.0,           63.0,                64.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kProphParamOsc2Env2Level,    "Osc 2 Env 2 Level",       "Env 2",    eSynthLibUnitGeneric, -64.0,           63.0,                64.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kProphParamOsc3Env2Level,    "Osc 3 Env 2 Level",       "Env 2",    eSynthLibUnitGeneric, -64.0,           63.0,                64.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kProphParamRing13Env2Level,  "1x3 Env 2 Level",         "Env 2",    eSynthLibUnitGeneric, -64.0,           63.0,                64.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kProphParamRing23Env2Level,  "2x3 Env 2 Level",         "Env 2",    eSynthLibUnitGeneric, -64.0,           63.0,                64.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kProphParamNoiseEnv2Level,   "Noise Env 2 Level",       "Env 2",    eSynthLibUnitGeneric, -64.0,           63.0,                64.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kProphParamOsc1Env3Level,    "Osc 1 Env 3 Level",       "Env 3",    eSynthLibUnitGeneric, -64.0,           63.0,                64.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kProphParamOsc2Env3Level,    "Osc 2 Env 3 Level",       "Env 3",    eSynthLibUnitGeneric, -64.0,           63.0,                64.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kProphParamOsc3Env3Level,    "Osc 3 Env 3 Level",       "Env 3",    eSynthLibUnitGeneric, -64.0,           63.0,                64.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kProphParamRing13Env3Level,  "1x3 Env 3 Level",         "Env 3",    eSynthLibUnitGeneric, -64.0,           63.0,                64.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kProphParamRing23Env3Level,  "2x3 Env 3 Level",         "Env 3",    eSynthLibUnitGeneric, -64.0,           63.0,                64.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kProphParamNoiseEnv3Level,   "Noise Env 3 Level",       "Env 3",    eSynthLibUnitGeneric, -64.0,           63.0,                64.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kProphParamOsc1Lfo1Level,    "Osc 1 LFO 1 Level",       "LFO 1",    eSynthLibUnitGeneric, -64.0,           63.0,                64.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kProphParamOsc2Lfo1Level,    "Osc 2 LFO 1 Level",       "LFO 1",    eSynthLibUnitGeneric, -64.0,           63.0,                64.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kProphParamOsc3Lfo1Level,    "Osc 3 LFO 1 Level",       "LFO 1",    eSynthLibUnitGeneric, -64.0,           63.0,                64.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kProphParamRing13Lfo1Level,  "1x3 LFO 1 Level",         "LFO 1",    eSynthLibUnitGeneric, -64.0,           63.0,                64.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kProphParamRing23Lfo1Level,  "2x3 LFO 1 Level",         "LFO 1",    eSynthLibUnitGeneric, -64.0,           63.0,                64.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kProphParamNoiseLfo1Level,   "Noise LFO 1 Level",       "LFO 1",    eSynthLibUnitGeneric, -64.0,           63.0,                64.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kProphParamOsc1Lfo2Level,    "Osc 1 LFO 2 Level",       "LFO 2",    eSynthLibUnitGeneric, -64.0,           63.0,                64.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kProphParamOsc2Lfo2Level,    "Osc 2 LFO 2 Level",       "LFO 2",    eSynthLibUnitGeneric, -64.0,           63.0,                64.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kProphParamOsc3Lfo2Level,    "Osc 3 LFO 2 Level",       "LFO 2",    eSynthLibUnitGeneric, -64.0,           63.0,                64.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kProphParamRing13Lfo2Level,  "1x3 LFO 2 Level",         "LFO 2",    eSynthLibUnitGeneric, -64.0,           63.0,                64.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kProphParamRing23Lfo2Level,  "2x3 LFO 2 Level",         "LFO 2",    eSynthLibUnitGeneric, -64.0,           63.0,                64.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kProphParamNoiseLfo2Level,   "Noise LFO 2 Level",       "LFO 2",    eSynthLibUnitGeneric, -64.0,           63.0,                64.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
};

#define COUNT(table)    (sizeof(table) / sizeof((table)[0]))   // ../Docs/panel-layout.md: never a count by hand

static const char *const kShapes[]          = {"SAW", "SQUARE", "DOUBLE"};
static const char *const kFilters[]         = {"LPF", "BPF", "HPF", "Res LPF", "Res BPF", "Res HPF", "Notch", "LPF+LPF", "BPF+BPF", "HPF+HPF", "LPF+BPF", "BPF+HPF"};
static const char *const kSlopes[]          = {"12", "18", "24"};

static const tProphControl kOsc1[]            = {
    {kProphParamOscShape,     "Shape",  eTiControlChoice,   2,   0.0, kShapes},
    {kProphParamOsc1Octave,   "Octave", eTiControlDial,     4,  -2.0, NULL   },
    {kProphParamOsc1Semitone, "Semi",   eTiControlDial,    24, -12.0, NULL   },
    {kProphParamOsc1Fine,     "Fine",   eTiControlDial,   127, -64.0, NULL   },
    {kProphParamPulseWidth,   "Width",  eTiControlDial,   127,   0.0, NULL   },
    {kProphParamOscSoften,    "Soften", eTiControlDial,   127,   0.0, NULL   },
    {kProphParamOscLevel,     "Level",  eTiControlDial,   127,   0.0, NULL   },
};
static const tProphControl kOsc2[]            = {
    {kProphParamOsc2Shape,    "Shape",  eTiControlChoice,   2,   0.0, kShapes},
    {kProphParamOsc2Octave,   "Octave", eTiControlDial,     4,  -2.0, NULL   },
    {kProphParamOsc2Semitone, "Semi",   eTiControlDial,    24, -12.0, NULL   },
    {kProphParamOsc2Fine,     "Fine",   eTiControlDial,   127, -64.0, NULL   },
    {kProphParamOsc2Width,    "Width",  eTiControlDial,   127,   0.0, NULL   },
    {kProphParamOsc2Soften,   "Soften", eTiControlDial,   127,   0.0, NULL   },
    {kProphParamOsc2Level,    "Level",  eTiControlDial,   127,   0.0, NULL   },
};
static const char *const kFmOnOff[]         = {"OFF", "ON"};
static const tProphControl kOsc3[]            = {
    {kProphParamOsc3Shape,    "Shape",  eTiControlChoice,   2,   0.0, kShapes },
    {kProphParamOsc3Octave,   "Octave", eTiControlDial,     4,  -2.0, NULL    },
    {kProphParamOsc3Semitone, "Semi",   eTiControlDial,    24, -12.0, NULL    },
    {kProphParamOsc3Fine,     "Fine",   eTiControlDial,   127, -64.0, NULL    },
    {kProphParamOsc3Width,    "Width",  eTiControlDial,   127,   0.0, NULL    },
    {kProphParamOsc3Soften,   "Soften", eTiControlDial,   127,   0.0, NULL    },
    {kProphParamOsc3Level,    "Level",  eTiControlDial,   127,   0.0, NULL    },
    {kProphParamFm13,         "FM 1x3", eTiControlChoice,   1,   0.0, kFmOnOff},   // prophVoices notes §18: Osc 3 is what they modulate
    {kProphParamFm23,         "FM 2x3", eTiControlChoice,   1,   0.0, kFmOnOff},
    {kProphParamFmNoise,      "FM Nse", eTiControlChoice,   1,   0.0, kFmOnOff},
};
static const tProphControl kFilter[]          = {
    {kProphParamCutoff,      "Cutoff",  eTiControlDial,   127, 0.0, NULL    },
    {kProphParamResonance,   "Reso",    eTiControlDial,   127, 0.0, NULL    },
    {kProphParamFilterType,  "Type",    eTiControlChoice,  11, 0.0, kFilters},
    {kProphParamFilterSlope, "Slope",   eTiControlChoice,   2, 0.0, kSlopes },
    {kProphParamFilterTrack, "Track",   eTiControlDial,   127, 0.0, NULL    },
    {kProphParamFilterDrive, "Drive",   eTiControlDial,   127, 0.0, NULL    },
    {kProphParamFilterQ,     "Q Norm",  eTiControlDial,   127, 0.0, NULL    },
    {kProphParamOscsBypass,  "Osc Byp", eTiControlChoice,   1, 0.0, kFmOnOff},
};
static const tProphControl kAmpEnv[]          = {
    {kProphParamAmpAttack,      "Attack",  eTiControlDial,   127,   0.0, NULL    },
    {kProphParamAmpDecay,       "Decay",   eTiControlDial,   127,   0.0, NULL    },
    {kProphParamAmpSustain,     "Sustain", eTiControlDial,   127,   0.0, NULL    },
    {kProphParamAmpSustainTime, "Time",    eTiControlDial,   127,   0.0, NULL    },
    {kProphParamAmpSustainRate, "Rate",    eTiControlDial,   127, -64.0, NULL    },
    {kProphParamAmpRelease,     "Release", eTiControlDial,   127,   0.0, NULL    },
    {kProphParamAmpVelocity,    "Vel",     eTiControlDial,   127,   0.0, NULL    },
    {kProphParamOneShot,        "1 Shot",  eTiControlChoice,   1,   0.0, kFmOnOff},
};
static const tProphControl kAmp[]             = {
    {kProphParamVolume, "Prog Level", eTiControlDial, 127, 0.0, NULL},
    {kProphParamMaster, "Master",     eTiControlDial, 127, 0.0, NULL},
};
static const char *const kPolyModes[]       = {"Poly", "Mono"};
static const char *const kEnvTriggers[]     = {"Retrigger", "Legato"};
static const char *const kAntiAlias[]       = {"OFF", "ON"};
static const char *const kSameNote[]        = {"STACK", "PIANO"};
static const tProphControl kVoices[]          = {
    {kProphParamVoiceLimit, "Voices",     eTiControlDial,   PROPH_MAX_VOICES - 1, 1.0, NULL      },     // ../Docs/panel-layout.md
    {kProphParamSameNote,   "Same Note",  eTiControlChoice,                  1, 0.0, kSameNote },
    {kProphParamAntiAlias,  "Anti-alias", eTiControlChoice,                  1, 0.0, kAntiAlias},
};
static const tProphControl kPlay[]            = {
    {kProphParamPolyMode,   "Mode",    eTiControlChoice, 1, 0.0, kPolyModes  },
    {kProphParamEnvTrigger, "Trigger", eTiControlChoice, 1, 0.0, kEnvTriggers},
};
static const tProphControl kMixer[]           = {
    {kProphParamNoiseLevel,  "Noise",  eTiControlDial, 127, 0.0, NULL},
    {kProphParamNoiseSoften, "Soften", eTiControlDial, 127, 0.0, NULL},
    {kProphParamRing13,      "1x3",    eTiControlDial, 127, 0.0, NULL},
    {kProphParamRing23,      "2x3",    eTiControlDial, 127, 0.0, NULL},
};
static const char *const kChorusTypes[]     = {"QUAD", "CHORUS", "PHASER"};
static const char *const kOffOn[]           = {"OFF", "ON"};
static const tProphControl kChorus[]          = {
    {kProphParamChorusType,     "Type",  eTiControlChoice,   2,   0.0, kChorusTypes},
    {kProphParamChorusSend,     "Send",  eTiControlDial,   127,   0.0, NULL        },
    {kProphParamChorusSpeed,    "Speed", eTiControlDial,   127,   0.0, NULL        },
    {kProphParamChorusDepth,    "Depth", eTiControlDial,   127,   0.0, NULL        },
    {kProphParamChorusFeedback, "Fback", eTiControlDial,   127, -64.0, NULL        },
};
static const char *const kDelayRatios[]     = {"1:1", "1:.75", ".75:1", "1:.66", ".66:1", "1:.5", ".5:1", "1:.33", ".33:1", "1:.25", ".25:1", "1:OFF", "OFF:1"};
static const char *const kDelaySyncs[]      = {"OFF", "32 T", "32", "16 T", "16", "8 T", "16 .", "8", "4 T", "8 .", "4", "2 T", "4 .", "2", "1B T", "2 .", "1B", "2B T", "1B .", "2B", "4B T", "3B", "5B T", "4B", "3B .", "7B T", "5B", "8B T", "6B", "7B", "5B .", "8B", "6B .", "7B .", "8B ."};
static const tProphControl kDelay[]           = {
    {kProphParamDelaySync,     "Sync",  eTiControlChoice,  34, 0.0, kDelaySyncs },
    {kProphParamDelayRatio,    "Ratio", eTiControlChoice,  12, 0.0, kDelayRatios},
    {kProphParamDelaySend,     "Send",  eTiControlDial,   127, 0.0, NULL        },
    {kProphParamDelayTime,     "Time",  eTiControlDial,   127, 0.0, NULL        },
    {kProphParamDelayFeedback, "Fback", eTiControlDial,   127, 0.0, NULL        },
    {kProphParamDelayDamp,     "Damp",  eTiControlDial,   127, 0.0, NULL        },
    {kProphParamDelayWidth,    "Width", eTiControlDial,   127, 0.0, NULL        },
};
static const char *const kReverbTypes[]     = {"GATE REV", "GATE RISE", "GATE GENT", "GATE FALL", "DRY CHMB", "ECHO CHMB", "SMALL RM", "BIG RM", "MEDIUM 1", "MEDIUM 2", "M PLATE 1", "M PLATE 2", "LARGE 1", "LARGE 2", "L PLATE 1", "L PLATE 2"};
static const tProphControl kReverb[]          = {
    {kProphParamReverbType,  "Type",  eTiControlChoice,  15, 0.0, kReverbTypes},
    {kProphParamReverbSend,  "Send",  eTiControlDial,   127, 0.0, NULL        },
    {kProphParamReverbDecay, "Decay", eTiControlDial,   127, 0.0, NULL        },
    {kProphParamReverbDamp,  "Damp",  eTiControlDial,   127, 0.0, NULL        },
};
static const char *const kOrders[]          = {"D+R+C", "D>R>C", "D>C>R", "R>D>C", "R>C>D", "C>D>R", "C>R>D", "D+[R>C]", "D+[C>R]", "R+[D>C]", "R+[C>D]", "C+[D>R]", "C+[R>D]", "D>[R+C]", "R>[D+C]", "C>[D+R]", "[R+C]>D", "[D+C]>R", "[D+R]>C"};
static const tProphControl kDrive[]           = {
    {kProphParamDistLevel,  "Level",  eTiControlDial, 127, 0.0, NULL},
    {kProphParamDistComp,   "Comp",   eTiControlDial, 127, 0.0, NULL},
    {kProphParamDistOutput, "Output", eTiControlDial, 127, 0.0, NULL},
};
static const tProphControl kEq[]              = {
    {kProphParamEqBass,   "Bass",   eTiControlDial, 127, -64.0, NULL},
    {kProphParamEqTreble, "Treble", eTiControlDial, 127, -64.0, NULL},
};
static const tProphControl kEffectsMain[]     = {
    {kProphParamEffectsOrder, "Order", eTiControlChoice,  18, 0.0, kOrders},
    {kProphParamEffectsDry,   "Dry",   eTiControlDial,   127, 0.0, NULL   },
};
#define COLOUR_CHORUS    {0.30, 0.70, 0.85}
#define COLOUR_DELAY     {0.85, 0.65, 0.30}
#define COLOUR_REVERB    {0.55, 0.45, 0.85}
#define COLOUR_DRIVE     {0.90, 0.40, 0.30}
#define COLOUR_EQ        {0.50, 0.75, 0.40}
static const tProphSection kEffectsPage[]     = {
    {"CHORUS", COLOUR_CHORUS,      kChorus,      COUNT(kChorus)     },
    {"DELAY",  COLOUR_DELAY,       kDelay,       COUNT(kDelay)      },
    {"REVERB", COLOUR_REVERB,      kReverb,      COUNT(kReverb)     },
    {"DRIVE",  COLOUR_DRIVE,       kDrive,       COUNT(kDrive)      },
    {"EQ",     COLOUR_EQ,          kEq,          COUNT(kEq)         },
    {"OUTPUT", RGB_SECTION_OUTPUT, kEffectsMain, COUNT(kEffectsMain)},
};
static const char *const kFxBypass[]        = {"On", "Bypass"};
static const tProphControl kGlobalFx[]        = {
    {kProphParamEffectsBypass, "Effects", eTiControlChoice, 1, 0.0, kFxBypass},
    {kProphParamDelayOn,       "Delay",   eTiControlChoice, 1, 0.0, kOffOn   },
    {kProphParamReverbOn,      "Reverb",  eTiControlChoice, 1, 0.0, kOffOn   },
};
static const char *const kGlideStyles[]     = {"PORTA", "GLISS"};
static const char *const kGlideCurves[]     = {"LINEAR", "EXP"};
static const char *const kGlideTypes[]      = {"NORMAL", "AUTO", "2 DOWN", "2 UP", "5 DOWN", "5 UP", "7 DOWN", "7 UP", "12 DOWN", "12 UP"};
static const char *const kTriggers[]        = {"PERCUSS", "ENSEMBLE"};
static const tProphControl kGlide[]           = {
    {kProphParamGlideTime,  "Time",    eTiControlDial,   127, 0.0, NULL        },
    {kProphParamGlideStyle, "Style",   eTiControlChoice,   1, 0.0, kGlideStyles},
    {kProphParamGlideCurve, "Curve",   eTiControlChoice,   1, 0.0, kGlideCurves},
    {kProphParamGlideType,  "Type",    eTiControlChoice,   9, 0.0, kGlideTypes },
    {kProphParamOscTrigger, "Trigger", eTiControlChoice,   1, 0.0, kTriggers   },
};
#define COLOUR_OSC_1    {0.30, 0.55, 0.85}
#define COLOUR_MIXER    {0.30, 0.62, 0.72}
#define COLOUR_OSC_2    {0.30, 0.70, 0.60}
#define COLOUR_OSC_3    {0.85, 0.55, 0.30}
static const tProphSection kOscillatorsPage[] = {
    {"OSC 1", COLOUR_OSC_1,     kOsc1,  COUNT(kOsc1) },
    {"MIXER", COLOUR_MIXER,     kMixer, COUNT(kMixer)},
    {"OSC 2", COLOUR_OSC_2,     kOsc2,  COUNT(kOsc2) },
    {"GLIDE", RGB_SECTION_PLAY, kGlide, COUNT(kGlide)},
    {"OSC 3", COLOUR_OSC_3,     kOsc3,  COUNT(kOsc3) },
};
static const tProphSection kFilterPage[]      = {
    {"FILTER", {0.90, 0.55, 0.20}, kFilter, COUNT(kFilter)},
};
static const tProphControl kEnv2[]            = {
    {kProphParamEnv2Attack,      "Attack",  eTiControlDial, 127,   0.0, NULL},
    {kProphParamEnv2Decay,       "Decay",   eTiControlDial, 127,   0.0, NULL},
    {kProphParamEnv2Sustain,     "Sustain", eTiControlDial, 127,   0.0, NULL},
    {kProphParamEnv2SustainTime, "Time",    eTiControlDial, 127,   0.0, NULL},
    {kProphParamEnv2SustainRate, "Rate",    eTiControlDial, 127, -64.0, NULL},
    {kProphParamEnv2Release,     "Release", eTiControlDial, 127,   0.0, NULL},
    {kProphParamEnv2Velocity,    "Vel",     eTiControlDial, 127,   0.0, NULL},
    {kProphParamEnv2Cutoff,      "Cutoff",  eTiControlDial, 127, -64.0, NULL},
};
static const tProphControl kEnv3[]            = {
    {kProphParamEnv3Attack,      "Attack",  eTiControlDial, 127,   0.0, NULL},
    {kProphParamEnv3Decay,       "Decay",   eTiControlDial, 127,   0.0, NULL},
    {kProphParamEnv3Sustain,     "Sustain", eTiControlDial, 127,   0.0, NULL},
    {kProphParamEnv3SustainTime, "Time",    eTiControlDial, 127,   0.0, NULL},
    {kProphParamEnv3SustainRate, "Rate",    eTiControlDial, 127, -64.0, NULL},
    {kProphParamEnv3Release,     "Release", eTiControlDial, 127,   0.0, NULL},
    {kProphParamEnv3Velocity,    "Vel",     eTiControlDial, 127,   0.0, NULL},
    {kProphParamEnv3Cutoff,      "Cutoff",  eTiControlDial, 127, -64.0, NULL},
};
static const tProphSection kAmpPage[]         = {
    {"OUTPUT", RGB_SECTION_OUTPUT, kAmp, COUNT(kAmp)},
};
static const tProphSection kEnvelopesPage[]   = {
    {"AMP ENV", {0.85, 0.75, 0.25}, kAmpEnv, COUNT(kAmpEnv)},
    {"ENV 2",   {0.90, 0.55, 0.20}, kEnv2,   COUNT(kEnv2)  },
    {"ENV 3",   {0.70, 0.45, 0.80}, kEnv3,   COUNT(kEnv3)  },
};
static const char *const kLfoShapes[]       = {"Square", "Saw", "Triangle", "S+H"};
static const char *const kLfoRanges[]       = {"x1", "x10", "x100"};
static const char *const kLfoTriggers[]     = {"Free", "Key"};
static const char *const kLfoFades[]        = {"In", "Out"};
#define LFO_CONTROLS(n)    {                                                              \
       {kProphParamLfo ## n ## Shape, "Shape", eTiControlChoice, 3, 0.0, kLfoShapes},       \
       {kProphParamLfo ## n ## Speed, "Speed", eTiControlDial, 127, 0.0, NULL},             \
       {kProphParamLfo ## n ## Range, "Range", eTiControlChoice, 2, 0.0, kLfoRanges},       \
       {kProphParamLfo ## n ## Delay, "Delay", eTiControlDial, 127, 0.0, NULL},             \
       {kProphParamLfo ## n ## Fade, "Fade", eTiControlChoice, 1, 0.0, kLfoFades},          \
       {kProphParamLfo ## n ## Offset, "Offset", eTiControlDial, 127, 0.0, NULL},           \
       {kProphParamLfo ## n ## Trigger, "Trigger", eTiControlChoice, 1, 0.0, kLfoTriggers}, \
       {kProphParamLfo ## n ## Cutoff, "Cutoff", eTiControlDial, 127, -64.0, NULL},         \
}
static const tProphControl kLfo1[]            = LFO_CONTROLS(1);
static const tProphControl kLfo2[]            = LFO_CONTROLS(2);
static const tProphSection kLfoPage[]         = {
    {"LFO 1", {0.80, 0.55, 0.85}, kLfo1, COUNT(kLfo1)},
    {"LFO 2", {0.55, 0.80, 0.85}, kLfo2, COUNT(kLfo2)},
};
static const tProphControl kBendWheel[]       = {
    {kProphParamPitchBend, "Bend",  eTiControlDial, 16383, -8192.0, NULL},
    {kProphParamModWheel,  "Wheel", eTiControlDial,   127,     0.0, NULL},
};
static const tProphControl kBendRange[]       = {
    {kProphParamOsc1BendRange, "Osc 1", eTiControlDial, 24, -12.0, NULL},
    {kProphParamOsc2BendRange, "Osc 2", eTiControlDial, 24, -12.0, NULL},
    {kProphParamOsc3BendRange, "Osc 3", eTiControlDial, 24, -12.0, NULL},
};
static const tProphControl kWheelPitch[]      = {
    {kProphParamOsc1WheelPitch, "Osc 1", eTiControlDial, 127, -64.0, NULL},
    {kProphParamOsc2WheelPitch, "Osc 2", eTiControlDial, 127, -64.0, NULL},
    {kProphParamOsc3WheelPitch, "Osc 3", eTiControlDial, 127, -64.0, NULL},
};
static const tProphControl kWheelLevel[]      = {
    {kProphParamOsc1WheelLevel,   "Osc 1", eTiControlDial, 127, -64.0, NULL},
    {kProphParamOsc2WheelLevel,   "Osc 2", eTiControlDial, 127, -64.0, NULL},
    {kProphParamOsc3WheelLevel,   "Osc 3", eTiControlDial, 127, -64.0, NULL},
    {kProphParamRing13WheelLevel, "1x3",   eTiControlDial, 127, -64.0, NULL},
    {kProphParamRing23WheelLevel, "2x3",   eTiControlDial, 127, -64.0, NULL},
    {kProphParamNoiseWheelLevel,  "Noise", eTiControlDial, 127, -64.0, NULL},
};
static const tProphControl kWheelFilter[]     = {
    {kProphParamWheelCutoff,    "Cutoff", eTiControlDial, 127, -64.0, NULL},
    {kProphParamWheelResonance, "Res",    eTiControlDial, 127, -64.0, NULL},
};
#define COLOUR_BEND_RANGE      {0.30, 0.55, 0.85}
#define COLOUR_WHEEL_PITCH     {0.30, 0.70, 0.60}
#define COLOUR_WHEEL_LEVEL     {0.85, 0.55, 0.30}
#define COLOUR_WHEEL_FILTER    {0.90, 0.55, 0.20}
static const tProphSection kWheelPage[]       = {
    {"BEND / WHEEL", RGB_SECTION_CONTROL, kBendWheel,   COUNT(kBendWheel)  },
    {"BEND RANGE",   COLOUR_BEND_RANGE,   kBendRange,   COUNT(kBendRange)  },
    {"WHEEL PITCH",  COLOUR_WHEEL_PITCH,  kWheelPitch,  COUNT(kWheelPitch) },
    {"WHEEL LEVEL",  COLOUR_WHEEL_LEVEL,  kWheelLevel,  COUNT(kWheelLevel) },
    {"WHEEL FILTER", COLOUR_WHEEL_FILTER, kWheelFilter, COUNT(kWheelFilter)},
};
static const tProphControl kEnv2Level[]       = {
    {kProphParamOsc1Env2Level,   "Osc 1", eTiControlDial, 127, -64.0, NULL},
    {kProphParamOsc2Env2Level,   "Osc 2", eTiControlDial, 127, -64.0, NULL},
    {kProphParamOsc3Env2Level,   "Osc 3", eTiControlDial, 127, -64.0, NULL},
    {kProphParamRing13Env2Level, "1x3",   eTiControlDial, 127, -64.0, NULL},
    {kProphParamRing23Env2Level, "2x3",   eTiControlDial, 127, -64.0, NULL},
    {kProphParamNoiseEnv2Level,  "Noise", eTiControlDial, 127, -64.0, NULL},
};
static const tProphControl kEnv3Level[]       = {
    {kProphParamOsc1Env3Level,   "Osc 1", eTiControlDial, 127, -64.0, NULL},
    {kProphParamOsc2Env3Level,   "Osc 2", eTiControlDial, 127, -64.0, NULL},
    {kProphParamOsc3Env3Level,   "Osc 3", eTiControlDial, 127, -64.0, NULL},
    {kProphParamRing13Env3Level, "1x3",   eTiControlDial, 127, -64.0, NULL},
    {kProphParamRing23Env3Level, "2x3",   eTiControlDial, 127, -64.0, NULL},
    {kProphParamNoiseEnv3Level,  "Noise", eTiControlDial, 127, -64.0, NULL},
};
static const tProphControl kLfo1Level[]       = {
    {kProphParamOsc1Lfo1Level,   "Osc 1", eTiControlDial, 127, -64.0, NULL},
    {kProphParamOsc2Lfo1Level,   "Osc 2", eTiControlDial, 127, -64.0, NULL},
    {kProphParamOsc3Lfo1Level,   "Osc 3", eTiControlDial, 127, -64.0, NULL},
    {kProphParamRing13Lfo1Level, "1x3",   eTiControlDial, 127, -64.0, NULL},
    {kProphParamRing23Lfo1Level, "2x3",   eTiControlDial, 127, -64.0, NULL},
    {kProphParamNoiseLfo1Level,  "Noise", eTiControlDial, 127, -64.0, NULL},
};
static const tProphControl kLfo2Level[]       = {
    {kProphParamOsc1Lfo2Level,   "Osc 1", eTiControlDial, 127, -64.0, NULL},
    {kProphParamOsc2Lfo2Level,   "Osc 2", eTiControlDial, 127, -64.0, NULL},
    {kProphParamOsc3Lfo2Level,   "Osc 3", eTiControlDial, 127, -64.0, NULL},
    {kProphParamRing13Lfo2Level, "1x3",   eTiControlDial, 127, -64.0, NULL},
    {kProphParamRing23Lfo2Level, "2x3",   eTiControlDial, 127, -64.0, NULL},
    {kProphParamNoiseLfo2Level,  "Noise", eTiControlDial, 127, -64.0, NULL},
};
static const tProphSection kLevelsPage[]      = {
    {"Env 2 TO LEVEL", {0.90, 0.55, 0.20}, kEnv2Level, COUNT(kEnv2Level)},
    {"Env 3 TO LEVEL", {0.70, 0.45, 0.80}, kEnv3Level, COUNT(kEnv3Level)},
    {"LFO 1 TO LEVEL", {0.30, 0.70, 0.60}, kLfo1Level, COUNT(kLfo1Level)},
    {"LFO 2 TO LEVEL", {0.30, 0.55, 0.85}, kLfo2Level, COUNT(kLfo2Level)},
};
static const tProphControl kHardware[]        = {
    {kProphParamHwLink,    "Link",    eTiControlChoice,   1, 0.0, kOffOn},
    {kProphParamHwChannel, "Channel", eTiControlDial,    15, 1.0, NULL  },
    {kProphParamHwDevice,  "Device",  eTiControlDial,   127, 0.0, NULL  },
};
static const tProphControl kPan[]             = {
    {kProphParamPan,        "Pan",     eTiControlDial,   127, 0.0, NULL  },
    {kProphParamPanSpeed,   "Speed",   eTiControlDial,   127, 0.0, NULL  },
    {kProphParamPanDepth,   "Depth",   eTiControlDial,   127, 0.0, NULL  },
    {kProphParamPanEffects, "Effects", eTiControlChoice,   1, 0.0, kOffOn},
};
#define COLOUR_EFFECTS    {0.65, 0.55, 0.85}
static const tProphSection kGlobalPage[]      = {
    {"PLUG-IN",  RGB_SECTION_PLUGIN, kVoices,   COUNT(kVoices)  },
    {"HARDWARE", RGB_SECTION_PLUGIN, kHardware, COUNT(kHardware)},
    {"PLAY",     RGB_SECTION_PLAY,   kPlay,     COUNT(kPlay)    },
    {"EFFECTS",  COLOUR_EFFECTS,     kGlobalFx, COUNT(kGlobalFx)},
    {"PAN",      RGB_SECTION_OUTPUT, kPan,      COUNT(kPan)     },
};

const tProphPage           gProphPages[]        = {
    {"GLOBAL",  kGlobalPage,      COUNT(kGlobalPage)     },
};
const uint32_t           gProphPageCount      = 1;

tProphEffectsSound proph_effects_sound(const double * v, double tempo) {
    return (tProphEffectsSound){
        .dry            = v[kProphParamEffectsDry] * 127.0,
        .chorusSend     = v[kProphParamChorusSend] * 127.0,
        .chorusSpeed    = v[kProphParamChorusSpeed] * 127.0,
        .chorusDepth    = v[kProphParamChorusDepth] * 127.0,
        .chorusFeedback = v[kProphParamChorusFeedback] * 127.0,
        .chorusType     = (int)lround(v[kProphParamChorusType] * 2.0),
        .delayOn        = v[kProphParamDelayOn] > 0.5,
        .delaySend      = v[kProphParamDelaySend] * 127.0,
        .delayTime      = v[kProphParamDelayTime] * 127.0,
        .delayFeedback  = v[kProphParamDelayFeedback] * 127.0,
        .delayDamp      = v[kProphParamDelayDamp] * 127.0,
        .delayWidth     = v[kProphParamDelayWidth] * 127.0,
        .delayRatio     = (int)lround(v[kProphParamDelayRatio] * 12.0),
        .delaySync      = (int)lround(v[kProphParamDelaySync] * 34.0),
        .tempo          = tempo,
        .reverbOn       = v[kProphParamReverbOn] > 0.5,
        .order          = (int)lround(v[kProphParamEffectsOrder] * 18.0),
        .reverbSend     = v[kProphParamReverbSend] * 127.0,
        .reverbDecay    = v[kProphParamReverbDecay] * 127.0,
        .reverbDamp     = v[kProphParamReverbDamp] * 127.0,
        .reverbType     = (int)lround(v[kProphParamReverbType] * 15.0),
        .distLevel      = v[kProphParamDistLevel] * 127.0,
        .distComp       = v[kProphParamDistComp] * 127.0,
        .distOutput     = v[kProphParamDistOutput] * 127.0,
        .eqBass         = v[kProphParamEqBass] * 127.0,
        .eqTreble       = v[kProphParamEqTreble] * 127.0,
        .pan            = v[kProphParamPan] * 127.0,
        .panSpeed       = v[kProphParamPanSpeed] * 127.0,
        .panDepth       = v[kProphParamPanDepth] * 127.0,
        .panEffects     = v[kProphParamPanEffects] > 0.5,
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

void proph_controls_sound(const double * v, tProphSound * sound) {
    for (int l = 0; l < PROPH_LFOS; l++) {
        int    at    = l * (kProphParamLfo2Shape - kProphParamLfo1Shape);
        double speed = v[kProphParamLfo1Speed + at] * 127.0;

        sound->lfo[l]       = (tProphLfoSound){
            .shape   = (int)lround(v[kProphParamLfo1Shape + at] * 3.0),
            .hz      = (speed * speed / 1638.4) * pow(10.0, lround(v[kProphParamLfo1Range + at] * 2.0)),
            .delay   = lfo_delay(v[kProphParamLfo1Delay + at] * 127.0),
            .fadeOut = v[kProphParamLfo1Fade + l] > 0.5,
            .offset  = v[kProphParamLfo1Offset + at],
            .keyed   = v[kProphParamLfo1Trigger + at] > 0.5,
        };
        sound->lfoCutoff[l] = lfo_semis(v[kProphParamLfo1Cutoff + at]);

        for (int o = 0; o < PROPH_OSCS; o++) {
            sound->osc[o].lfoPitch[l] = lfo_semis(v[kProphParamOsc1Lfo1Pitch + at + (2 * o)]);
            sound->osc[o].lfoWidth[l] = ((v[kProphParamOsc1Lfo1Width + at + (2 * o)] * 127.0) - 64.0) / 64.0;
        }
    }

    sound->bend           = (2.0 * v[kProphParamPitchBend]) - 1.0;
    sound->wheel          = v[kProphParamModWheel];
    sound->pedal          = v[kProphParamSustainPedal] >= 0.5;
    sound->piano          = v[kProphParamSameNote] >= 0.5;
    sound->mono           = v[kProphParamPolyMode] > 0.5;
    sound->fm13           = v[kProphParamFm13] > 0.5;       // prophVoices notes §18
    sound->oneShot        = v[kProphParamOneShot] > 0.5;    // prophEnvelope notes §9
    sound->oscsAround     = v[kProphParamOscsBypass] > 0.5; // prophVoices notes §20

    // prophVoices notes §19
    static const int kWheel[PROPH_LEVEL_SLOTS] = {kProphParamOsc1WheelLevel, kProphParamOsc2WheelLevel, kProphParamOsc3WheelLevel, kProphParamRing13WheelLevel, kProphParamRing23WheelLevel, kProphParamNoiseWheelLevel};

    for (int s = 0; s < PROPH_LEVEL_SLOTS; s++) {
        sound->levelRoute[s][0] = ((v[kWheel[s]] * 127.0) - 64.0) / 64.0;

        for (int r = 1; r < PROPH_LEVEL_ROUTES; r++) {
            sound->levelRoute[s][r] = ((v[kProphParamOsc1Env2Level + ((r - 1) * PROPH_LEVEL_SLOTS) + s] * 127.0) - 64.0) / 64.0;
        }
    }

    sound->fm23           = v[kProphParamFm23] > 0.5;
    sound->fmNoise        = v[kProphParamFmNoise] > 0.5;
    sound->legato         = v[kProphParamEnvTrigger] > 0.5;
    sound->wheelCutoff    = (v[kProphParamWheelCutoff] * 127.0) - 64.0;
    sound->wheelResonance = (v[kProphParamWheelResonance] * 127.0) - 64.0;

    for (int o = 0; o < PROPH_OSCS; o++) {
        sound->osc[o].bendRange  = (v[kProphParamOsc1BendRange + o] * 24.0) - 12.0;
        sound->osc[o].wheelPitch = (v[kProphParamOsc1WheelPitch + o] * 127.0) - 64.0;
    }
}
