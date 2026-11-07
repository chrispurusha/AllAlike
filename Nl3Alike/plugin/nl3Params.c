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

#include "nl3Params.h"
#include "nl3Voices.h"

#define LIST    SYNTHLIB_PARAM_LIST

// notes §1: the panel's 0..127 is each dial's plain range, as on the instrument
const tSynthLibParam     gNl3Params[kNl3ParamCount] = {
    {kNl3ParamVoiceLimit,     "Voice Limit",             "Voices",   eSynthLibUnitGeneric,   1.0, NL3_MAX_VOICES, 23.0 / (NL3_MAX_VOICES - 1), NL3_MAX_VOICES - 1, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamVolume,         "Prog Level",              "Level",    eSynthLibUnitGeneric,   0.0,          127.0,               100.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamOscShape,       "Osc Shape",               "Shape",    eSynthLibUnitIndexed,   0.0,            2.0,                         0.0,                  2, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kNl3ParamPulseWidth,     "Osc Pulse Width",         "PW",       eSynthLibUnitGeneric,   0.0,          127.0,                64.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamCutoff,         "Cutoff",                  "Cutoff",   eSynthLibUnitGeneric,   0.0,          127.0,                         1.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamResonance,      "Resonance",               "Reso",     eSynthLibUnitGeneric,   0.0,          127.0,                         0.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamFilterType,     "Filter Type",             "Type",     eSynthLibUnitIndexed,   0.0,            1.0,                         0.0,                  1, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kNl3ParamFilterSlope,    "Filter Slope",            "Slope",    eSynthLibUnitIndexed,   0.0,            1.0,                         0.0,                  1, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kNl3ParamAmpAttack,      "Amp Attack",              "Attack",   eSynthLibUnitGeneric,   0.0,          127.0,                         0.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamAmpDecay,       "Amp Decay",               "Decay",    eSynthLibUnitGeneric,   0.0,          127.0,                         1.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamAmpSustain,     "Amp Sustain",             "Sustain",  eSynthLibUnitGeneric,   0.0,          127.0,                         1.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamAmpSustainTime, "Amp Sustain Time",        "Time",     eSynthLibUnitGeneric, -64.0,           63.0,                64.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamAmpRelease,     "Amp Release",             "Release",  eSynthLibUnitGeneric,   0.0,          127.0,                 4.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamProgram,        "Program",                 "Program",  eSynthLibUnitGeneric,   0.0,          127.0,                    0.000000,                127, SYNTHLIB_MIDI_NONE,       SYNTHLIB_PARAM_PROGRAM | SYNTHLIB_PARAM_HIDDEN | SYNTHLIB_PARAM_NO_SAVE},
    {kNl3ParamBank,           "Bank Select",             "Bank",     eSynthLibUnitGeneric,   0.0,          127.0,                    0.000000,                127,                       32, SYNTHLIB_PARAM_HIDDEN | SYNTHLIB_PARAM_NO_SAVE                         },
    {kNl3ParamOscSoften,      "Osc Soften",              "Soften",   eSynthLibUnitGeneric,   0.0,          127.0,                         1.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamOscLevel,       "Osc Level",               "Level",    eSynthLibUnitGeneric,   0.0,          127.0,                         1.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamOsc2Shape,      "Osc 2 Shape",             "Shape",    eSynthLibUnitIndexed,   0.0,            2.0,                         0.0,                  2, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kNl3ParamOsc2Width,      "Osc 2 Pulse Width",       "PW",       eSynthLibUnitGeneric,   0.0,          127.0,                64.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamOsc2Soften,     "Osc 2 Soften",            "Soften",   eSynthLibUnitGeneric,   0.0,          127.0,                         1.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamOsc2Level,      "Osc 2 Level",             "Level",    eSynthLibUnitGeneric,   0.0,          127.0,                         0.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamOsc3Shape,      "Osc 3 Shape",             "Shape",    eSynthLibUnitIndexed,   0.0,            2.0,                         0.0,                  2, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kNl3ParamOsc3Width,      "Osc 3 Pulse Width",       "PW",       eSynthLibUnitGeneric,   0.0,          127.0,                64.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamOsc3Soften,     "Osc 3 Soften",            "Soften",   eSynthLibUnitGeneric,   0.0,          127.0,                         1.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamOsc3Level,      "Osc 3 Level",             "Level",    eSynthLibUnitGeneric,   0.0,          127.0,                         0.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamOsc1Octave,     "Osc 1 Octave",            "Octave",   eSynthLibUnitGeneric,  -2.0,            2.0,                         0.5,                  4, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamOsc1Semitone,   "Osc 1 Semitone",          "Semi",     eSynthLibUnitGeneric, -12.0,           12.0,                         0.5,                 24, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamOsc1Fine,       "Osc 1 Fine Tune",         "Fine",     eSynthLibUnitGeneric, -64.0,           63.0,                64.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamOsc2Octave,     "Osc 2 Octave",            "Octave",   eSynthLibUnitGeneric,  -2.0,            2.0,                         0.5,                  4, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamOsc2Semitone,   "Osc 2 Semitone",          "Semi",     eSynthLibUnitGeneric, -12.0,           12.0,                         0.5,                 24, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamOsc2Fine,       "Osc 2 Fine Tune",         "Fine",     eSynthLibUnitGeneric, -64.0,           63.0,                64.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamOsc3Octave,     "Osc 3 Octave",            "Octave",   eSynthLibUnitGeneric,  -2.0,            2.0,                         0.5,                  4, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamOsc3Semitone,   "Osc 3 Semitone",          "Semi",     eSynthLibUnitGeneric, -12.0,           12.0,                         0.5,                 24, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamOsc3Fine,       "Osc 3 Fine Tune",         "Fine",     eSynthLibUnitGeneric, -64.0,           63.0,                64.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamFilterTrack,    "Filter Keyboard Track",   "Track",    eSynthLibUnitIndexed,   0.0,            1.0,                         0.0,                  1, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kNl3ParamFilterDrive,    "Filter Distortion",       "Dist",     eSynthLibUnitGeneric,   0.0,          127.0,                         0.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamAmpVelocity,    "Amp Velocity",            "Velocity", eSynthLibUnitGeneric,   0.0,          127.0,                64.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamEnv2Attack,     "Env 2 Attack",            "Attack",   eSynthLibUnitGeneric,   0.0,          127.0,                         0.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamEnv2Decay,      "Env 2 Decay",             "Decay",    eSynthLibUnitGeneric,   0.0,          127.0,                         0.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamEnv2Sustain,    "Env 2 Sustain",           "Sustain",  eSynthLibUnitGeneric,   0.0,          127.0,                         1.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamEnv2Release,    "Env 2 Release",           "Release",  eSynthLibUnitGeneric,   0.0,          127.0,                         0.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamEnv2Velocity,   "Env 2 Velocity",          "Vel",      eSynthLibUnitGeneric,   0.0,          127.0,                64.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamEnv2Cutoff,     "Env 2 to Cutoff",         "Cutoff",   eSynthLibUnitGeneric, -64.0,           63.0,                64.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamEnv3Attack,     "Env 3 Attack",            "Attack",   eSynthLibUnitGeneric,   0.0,          127.0,                         0.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamEnv3Decay,      "Env 3 Decay",             "Decay",    eSynthLibUnitGeneric,   0.0,          127.0,                         0.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamEnv3Sustain,    "Env 3 Sustain",           "Sustain",  eSynthLibUnitGeneric,   0.0,          127.0,                         1.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamEnv3Release,    "Env 3 Release",           "Release",  eSynthLibUnitGeneric,   0.0,          127.0,                         0.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamEnv3Velocity,   "Env 3 Velocity",          "Vel",      eSynthLibUnitGeneric,   0.0,          127.0,                64.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamEnv3Cutoff,     "Env 3 to Cutoff",         "Cutoff",   eSynthLibUnitGeneric, -64.0,           63.0,                64.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamNoiseLevel,     "Noise Level",             "Noise",    eSynthLibUnitGeneric,   0.0,          127.0,                         0.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamNoiseSoften,    "Noise Soften",            "Soften",   eSynthLibUnitGeneric,   0.0,          127.0,                         1.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamRing13,         "Ring 1x3",                "1x3",      eSynthLibUnitGeneric,   0.0,          127.0,                         0.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamRing23,         "Ring 2x3",                "2x3",      eSynthLibUnitGeneric,   0.0,          127.0,                         0.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamFilterQ,        "Filter Q Normalise",      "Q Norm",   eSynthLibUnitGeneric,   0.0,          127.0,                         0.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamEffectsDry,     "Effects Dry",             "Dry",      eSynthLibUnitGeneric,   0.0,          127.0,                         1.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamChorusSend,     "Chorus Send",             "Send",     eSynthLibUnitGeneric,   0.0,          127.0,                         0.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamChorusSpeed,    "Chorus Speed",            "Speed",    eSynthLibUnitGeneric,   0.0,          127.0,                27.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamChorusDepth,    "Chorus Depth",            "Depth",    eSynthLibUnitGeneric,   0.0,          127.0,                33.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamChorusFeedback, "Chorus Feedback",         "Fback",    eSynthLibUnitGeneric,   0.0,          127.0,                64.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamChorusType,     "Chorus Type",             "Type",     eSynthLibUnitIndexed,   0.0,            2.0,                         0.5,                  2, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kNl3ParamDelayOn,        "Global Delay",            "Delay",    eSynthLibUnitIndexed,   0.0,            1.0,                         1.0,                  1, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kNl3ParamReverbOn,       "Global Reverb",           "Reverb",   eSynthLibUnitIndexed,   0.0,            1.0,                         1.0,                  1, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kNl3ParamDelaySend,      "Delay Send",              "Send",     eSynthLibUnitGeneric,   0.0,          127.0,                         0.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamDelayTime,      "Delay Time",              "Time",     eSynthLibUnitGeneric,   0.0,          127.0,                75.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamDelayFeedback,  "Delay Feedback",          "Fback",    eSynthLibUnitGeneric,   0.0,          127.0,                50.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamDelayDamp,      "Delay HF Damp",           "Damp",     eSynthLibUnitGeneric,   0.0,          127.0,                20.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamDelayWidth,     "Delay Width",             "Width",    eSynthLibUnitGeneric,   0.0,          127.0,                         1.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamDelayRatio,     "Delay Ratio",             "Ratio",    eSynthLibUnitIndexed,   0.0,           12.0,                         0.0,                 12, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kNl3ParamDelaySync,      "Delay Sync",              "Sync",     eSynthLibUnitIndexed,   0.0,           34.0,                         0.0,                 34, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kNl3ParamGlideTime,      "Glide Time",              "Time",     eSynthLibUnitGeneric,   0.0,          127.0,                         0.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamGlideStyle,     "Glide Style",             "Style",    eSynthLibUnitIndexed,   0.0,            1.0,                         0.0,                  1, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kNl3ParamGlideCurve,     "Glide Curve",             "Curve",    eSynthLibUnitIndexed,   0.0,            1.0,                         1.0,                  1, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kNl3ParamGlideType,      "Glide Type",              "Type",     eSynthLibUnitIndexed,   0.0,            9.0,                   1.0 / 9.0,                  9, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kNl3ParamReverbSend,     "Reverb Send",             "Send",     eSynthLibUnitGeneric,   0.0,          127.0,                         0.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamReverbDecay,    "Reverb Decay",            "Decay",    eSynthLibUnitGeneric,   0.0,          127.0,                80.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamReverbDamp,     "Reverb HF Damp",          "Damp",     eSynthLibUnitGeneric,   0.0,          127.0,                18.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamReverbType,     "Reverb Type",             "Type",     eSynthLibUnitIndexed,   0.0,           15.0,                  9.0 / 15.0,                 15, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kNl3ParamEffectsOrder,   "Effects Order",           "Order",    eSynthLibUnitIndexed,   0.0,           18.0,                         0.0,                 18, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kNl3ParamOscTrigger,     "Osc Trigger",             "Trigger",  eSynthLibUnitIndexed,   0.0,            1.0,                         0.0,                  1, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kNl3ParamDistLevel,      "Distortion Level",        "Level",    eSynthLibUnitGeneric,   0.0,          127.0,                         0.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamDistComp,       "Distortion Compensation", "Comp",     eSynthLibUnitGeneric,   0.0,          127.0,               112.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamDistOutput,     "Distortion Output",       "Output",   eSynthLibUnitGeneric,   0.0,          127.0,                64.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamEqBass,         "EQ Bass",                 "Bass",     eSynthLibUnitGeneric,   0.0,          127.0,                64.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamEqTreble,       "EQ Treble",               "Treble",   eSynthLibUnitGeneric,   0.0,          127.0,                64.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamPitchBend,      "Pitch Bend",              "Bend",     eSynthLibUnitGeneric,   0.0,            1.0,                         0.5,                  0, SYNTHLIB_MIDI_PITCH_BEND, SYNTHLIB_PARAM_HIDDEN | SYNTHLIB_PARAM_NO_SAVE                         },
    {kNl3ParamModWheel,       "Mod Wheel",               "Wheel",    eSynthLibUnitGeneric,   0.0,            1.0,                         0.0,                  0, SYNTHLIB_MIDI_MOD_WHEEL,  SYNTHLIB_PARAM_HIDDEN | SYNTHLIB_PARAM_NO_SAVE                         },
    {kNl3ParamOsc1BendRange,  "Osc 1 Bend Range",        "Bend",     eSynthLibUnitGeneric, -12.0,           12.0,                 14.0 / 24.0,                 24, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamOsc2BendRange,  "Osc 2 Bend Range",        "Bend",     eSynthLibUnitGeneric, -12.0,           12.0,                 14.0 / 24.0,                 24, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamOsc3BendRange,  "Osc 3 Bend Range",        "Bend",     eSynthLibUnitGeneric, -12.0,           12.0,                 14.0 / 24.0,                 24, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamOsc1WheelPitch, "Osc 1 Wheel Pitch",       "Wheel",    eSynthLibUnitGeneric, -64.0,           63.0,                64.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamOsc2WheelPitch, "Osc 2 Wheel Pitch",       "Wheel",    eSynthLibUnitGeneric, -64.0,           63.0,                64.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamOsc3WheelPitch, "Osc 3 Wheel Pitch",       "Wheel",    eSynthLibUnitGeneric, -64.0,           63.0,                64.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamOsc1WheelLevel, "Osc 1 Wheel Level",       "Wheel",    eSynthLibUnitGeneric, -64.0,           63.0,                64.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamOsc2WheelLevel, "Osc 2 Wheel Level",       "Wheel",    eSynthLibUnitGeneric, -64.0,           63.0,                64.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamOsc3WheelLevel, "Osc 3 Wheel Level",       "Wheel",    eSynthLibUnitGeneric, -64.0,           63.0,                64.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamWheelCutoff,    "Wheel Cutoff",            "Wheel",    eSynthLibUnitGeneric, -64.0,           63.0,                64.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamWheelResonance, "Wheel Resonance",         "Wheel",    eSynthLibUnitGeneric, -64.0,           63.0,                64.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamPolyMode,       "Poly Mode",               "Mode",     eSynthLibUnitIndexed,   0.0,            1.0,                         0.0,                  1, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kNl3ParamEnvTrigger,     "Env Trigger",             "Trigger",  eSynthLibUnitIndexed,   0.0,            1.0,                         0.0,                  1, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kNl3ParamMaster,         "Master Volume",           "Master",   eSynthLibUnitGeneric,   0.0,          127.0,                         1.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamEffectsBypass,  "Effects Bypass",          "Effects",  eSynthLibUnitIndexed,   0.0,            1.0,                         0.0,                  1, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kNl3ParamSustainPedal,   "Sustain Pedal",           "Pedal",    eSynthLibUnitGeneric,   0.0,            1.0,                         0.0,                  0, SYNTHLIB_MIDI_SUSTAIN,    SYNTHLIB_PARAM_HIDDEN | SYNTHLIB_PARAM_NO_SAVE                         },
    {kNl3ParamLfo1Shape,      "LFO 1 Shape",             "Shape",    eSynthLibUnitIndexed,   0.0,            3.0,                   2.0 / 3.0,                  3, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kNl3ParamLfo1Speed,      "LFO 1 Speed",             "Speed",    eSynthLibUnitGeneric,   0.0,          127.0,                64.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamLfo1Range,      "LFO 1 Range",             "Range",    eSynthLibUnitIndexed,   0.0,            2.0,                         0.0,                  2, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kNl3ParamLfo1Delay,      "LFO 1 Delay",             "Delay",    eSynthLibUnitGeneric,   0.0,          127.0,                         0.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamLfo1Offset,     "LFO 1 Offset",            "Offset",   eSynthLibUnitGeneric,   0.0,          127.0,                         0.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamLfo1Trigger,    "LFO 1 Trigger",           "Trigger",  eSynthLibUnitIndexed,   0.0,            1.0,                         1.0,                  1, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kNl3ParamLfo1Cutoff,     "LFO 1 to Cutoff",         "Cutoff",   eSynthLibUnitGeneric, -64.0,           63.0,                64.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamOsc1Lfo1Pitch,  "Osc 1 LFO 1 Pitch",       "Pitch",    eSynthLibUnitGeneric, -64.0,           63.0,                64.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamOsc1Lfo1Width,  "Osc 1 LFO 1 Width",       "Width",    eSynthLibUnitGeneric, -64.0,           63.0,                64.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamOsc2Lfo1Pitch,  "Osc 2 LFO 1 Pitch",       "Pitch",    eSynthLibUnitGeneric, -64.0,           63.0,                64.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamOsc2Lfo1Width,  "Osc 2 LFO 1 Width",       "Width",    eSynthLibUnitGeneric, -64.0,           63.0,                64.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamOsc3Lfo1Pitch,  "Osc 3 LFO 1 Pitch",       "Pitch",    eSynthLibUnitGeneric, -64.0,           63.0,                64.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamOsc3Lfo1Width,  "Osc 3 LFO 1 Width",       "Width",    eSynthLibUnitGeneric, -64.0,           63.0,                64.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamLfo2Shape,      "LFO 2 Shape",             "Shape",    eSynthLibUnitIndexed,   0.0,            3.0,                   2.0 / 3.0,                  3, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kNl3ParamLfo2Speed,      "LFO 2 Speed",             "Speed",    eSynthLibUnitGeneric,   0.0,          127.0,                64.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamLfo2Range,      "LFO 2 Range",             "Range",    eSynthLibUnitIndexed,   0.0,            2.0,                         0.0,                  2, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kNl3ParamLfo2Delay,      "LFO 2 Delay",             "Delay",    eSynthLibUnitGeneric,   0.0,          127.0,                         0.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamLfo2Offset,     "LFO 2 Offset",            "Offset",   eSynthLibUnitGeneric,   0.0,          127.0,                         0.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamLfo2Trigger,    "LFO 2 Trigger",           "Trigger",  eSynthLibUnitIndexed,   0.0,            1.0,                         1.0,                  1, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kNl3ParamLfo2Cutoff,     "LFO 2 to Cutoff",         "Cutoff",   eSynthLibUnitGeneric, -64.0,           63.0,                64.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamOsc1Lfo2Pitch,  "Osc 1 LFO 2 Pitch",       "Pitch",    eSynthLibUnitGeneric, -64.0,           63.0,                64.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamOsc1Lfo2Width,  "Osc 1 LFO 2 Width",       "Width",    eSynthLibUnitGeneric, -64.0,           63.0,                64.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamOsc2Lfo2Pitch,  "Osc 2 LFO 2 Pitch",       "Pitch",    eSynthLibUnitGeneric, -64.0,           63.0,                64.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamOsc2Lfo2Width,  "Osc 2 LFO 2 Width",       "Width",    eSynthLibUnitGeneric, -64.0,           63.0,                64.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamOsc3Lfo2Pitch,  "Osc 3 LFO 2 Pitch",       "Pitch",    eSynthLibUnitGeneric, -64.0,           63.0,                64.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamOsc3Lfo2Width,  "Osc 3 LFO 2 Width",       "Width",    eSynthLibUnitGeneric, -64.0,           63.0,                64.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamLfo1Fade,       "LFO 1 Fade",              "Fade",     eSynthLibUnitIndexed,   0.0,            1.0,                         0.0,                  1, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kNl3ParamLfo2Fade,       "LFO 2 Fade",              "Fade",     eSynthLibUnitIndexed,   0.0,            1.0,                         0.0,                  1, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kNl3ParamHwLink,         "Link Hardware",           "Link",     eSynthLibUnitIndexed,   0.0,            1.0,                         0.0,                  1, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kNl3ParamHwChannel,      "Hardware MIDI Channel",   "Channel",  eSynthLibUnitGeneric,   1.0,           16.0,                  6.0 / 15.0,                 15, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamHwDevice,       "Hardware Device ID",      "Device",   eSynthLibUnitGeneric,   0.0,          127.0,                 6.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamPan,            "Pan",                     "Pan",      eSynthLibUnitGeneric,   0.0,          127.0,                64.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamPanSpeed,       "Pan Speed",               "Speed",    eSynthLibUnitGeneric,   0.0,          127.0,                34.0 / 127.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamPanDepth,       "Pan Depth",               "Depth",    eSynthLibUnitGeneric,   0.0,          127.0,                         0.0,                127, SYNTHLIB_MIDI_NONE,                                                                             0},
    {kNl3ParamPanEffects,     "Pan Effects",             "Effects",  eSynthLibUnitIndexed,   0.0,            1.0,                         1.0,                  1, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
    {kNl3ParamArithmetic,     "Arithmetic",              "Maths",    eSynthLibUnitIndexed,   0.0,            1.0,                         0.0,                  1, SYNTHLIB_MIDI_NONE,       LIST                                                                   },
};

static const char *const kShapes[]          = {"SAW", "SQUARE", "DOUBLE"};
static const char *const kFilters[]         = {"LP", "HP"};
static const char *const kSlopes[]          = {"12", "24"};
static const char *const kArithmetic[]      = {"FLOAT", "FIXED"};
static const char *const kTrack[]           = {"OFF", "ON"};

static const tNl3Control kOsc1[]            = {
    {kNl3ParamOscShape,     "Shape",  eTiControlChoice,   2,   0.0, kShapes},
    {kNl3ParamOsc1Octave,   "Octave", eTiControlDial,     4,  -2.0, NULL   },
    {kNl3ParamOsc1Semitone, "Semi",   eTiControlDial,    24, -12.0, NULL   },
    {kNl3ParamOsc1Fine,     "Fine",   eTiControlDial,   127, -64.0, NULL   },
    {kNl3ParamPulseWidth,   "Width",  eTiControlDial,   127,   0.0, NULL   },
    {kNl3ParamOscSoften,    "Soften", eTiControlDial,   127,   0.0, NULL   },
    {kNl3ParamOscLevel,     "Level",  eTiControlDial,   127,   0.0, NULL   },
};
static const tNl3Control kOsc2[]            = {
    {kNl3ParamOsc2Shape,    "Shape",  eTiControlChoice,   2,   0.0, kShapes},
    {kNl3ParamOsc2Octave,   "Octave", eTiControlDial,     4,  -2.0, NULL   },
    {kNl3ParamOsc2Semitone, "Semi",   eTiControlDial,    24, -12.0, NULL   },
    {kNl3ParamOsc2Fine,     "Fine",   eTiControlDial,   127, -64.0, NULL   },
    {kNl3ParamOsc2Width,    "Width",  eTiControlDial,   127,   0.0, NULL   },
    {kNl3ParamOsc2Soften,   "Soften", eTiControlDial,   127,   0.0, NULL   },
    {kNl3ParamOsc2Level,    "Level",  eTiControlDial,   127,   0.0, NULL   },
};
static const tNl3Control kOsc3[]            = {
    {kNl3ParamOsc3Shape,    "Shape",  eTiControlChoice,   2,   0.0, kShapes},
    {kNl3ParamOsc3Octave,   "Octave", eTiControlDial,     4,  -2.0, NULL   },
    {kNl3ParamOsc3Semitone, "Semi",   eTiControlDial,    24, -12.0, NULL   },
    {kNl3ParamOsc3Fine,     "Fine",   eTiControlDial,   127, -64.0, NULL   },
    {kNl3ParamOsc3Width,    "Width",  eTiControlDial,   127,   0.0, NULL   },
    {kNl3ParamOsc3Soften,   "Soften", eTiControlDial,   127,   0.0, NULL   },
    {kNl3ParamOsc3Level,    "Level",  eTiControlDial,   127,   0.0, NULL   },
};
static const tNl3Control kFilter[]          = {
    {kNl3ParamCutoff,      "Cutoff", eTiControlDial,   127, 0.0, NULL    },
    {kNl3ParamResonance,   "Reso",   eTiControlDial,   127, 0.0, NULL    },
    {kNl3ParamFilterType,  "Type",   eTiControlChoice,   1, 0.0, kFilters},
    {kNl3ParamFilterSlope, "Slope",  eTiControlChoice,   1, 0.0, kSlopes },
    {kNl3ParamFilterTrack, "Track",  eTiControlChoice,   1, 0.0, kTrack  },
    {kNl3ParamFilterDrive, "Dist",   eTiControlDial,   127, 0.0, NULL    },
};
static const tNl3Control kAmpEnv[]          = {
    {kNl3ParamAmpAttack,      "Attack",  eTiControlDial, 127,   0.0, NULL},
    {kNl3ParamAmpDecay,       "Decay",   eTiControlDial, 127,   0.0, NULL},
    {kNl3ParamAmpSustain,     "Sustain", eTiControlDial, 127,   0.0, NULL},
    {kNl3ParamAmpSustainTime, "Time",    eTiControlDial, 127, -64.0, NULL},
    {kNl3ParamAmpRelease,     "Release", eTiControlDial, 127,   0.0, NULL},
    {kNl3ParamAmpVelocity,    "Vel",     eTiControlDial, 127,   0.0, NULL},
};
static const tNl3Control kAmp[]             = {
    {kNl3ParamVolume, "Prog Level", eTiControlDial, 127, 0.0, NULL},
    {kNl3ParamMaster, "Master",     eTiControlDial, 127, 0.0, NULL},
};
static const char *const kPolyModes[]       = {"Poly", "Mono"};
static const char *const kEnvTriggers[]     = {"Retrigger", "Legato"};
static const tNl3Control kVoices[]          = {
    {kNl3ParamVoiceLimit, "Limit",   eTiControlDial,   NL3_MAX_VOICES - 1, 1.0, NULL        },
    {kNl3ParamArithmetic, "Maths",   eTiControlChoice,                  1, 0.0, kArithmetic },
    {kNl3ParamPolyMode,   "Mode",    eTiControlChoice,                  1, 0.0, kPolyModes  },
    {kNl3ParamEnvTrigger, "Trigger", eTiControlChoice,                  1, 0.0, kEnvTriggers},
};
static const tNl3Control kMixer[]           = {
    {kNl3ParamNoiseLevel,  "Noise",  eTiControlDial, 127, 0.0, NULL},
    {kNl3ParamNoiseSoften, "Soften", eTiControlDial, 127, 0.0, NULL},
    {kNl3ParamRing13,      "1x3",    eTiControlDial, 127, 0.0, NULL},
    {kNl3ParamRing23,      "2x3",    eTiControlDial, 127, 0.0, NULL},
};
static const char *const kChorusTypes[]     = {"QUAD", "CHORUS", "PHASER"};
static const char *const kOffOn[]           = {"OFF", "ON"};
static const tNl3Control kChorus[]          = {
    {kNl3ParamChorusType,     "Type",  eTiControlChoice,   2,   0.0, kChorusTypes},
    {kNl3ParamChorusSend,     "Send",  eTiControlDial,   127,   0.0, NULL        },
    {kNl3ParamChorusSpeed,    "Speed", eTiControlDial,   127,   0.0, NULL        },
    {kNl3ParamChorusDepth,    "Depth", eTiControlDial,   127,   0.0, NULL        },
    {kNl3ParamChorusFeedback, "Fback", eTiControlDial,   127, -64.0, NULL        },
};
static const char *const kDelayRatios[]     = {"1:1", "1:.75", ".75:1", "1:.66", ".66:1", "1:.5", ".5:1", "1:.33", ".33:1", "1:.25", ".25:1", "1:OFF", "OFF:1"};
static const char *const kDelaySyncs[]      = {"OFF", "32 T", "32", "16 T", "16", "8 T", "16 .", "8", "4 T", "8 .", "4", "2 T", "4 .", "2", "1B T", "2 .", "1B", "2B T", "1B .", "2B", "4B T", "3B", "5B T", "4B", "3B .", "7B T", "5B", "8B T", "6B", "7B", "5B .", "8B", "6B .", "7B .", "8B ."};
static const tNl3Control kDelay[]           = {
    {kNl3ParamDelaySync,     "Sync",  eTiControlChoice,  34, 0.0, kDelaySyncs },
    {kNl3ParamDelayRatio,    "Ratio", eTiControlChoice,  12, 0.0, kDelayRatios},
    {kNl3ParamDelaySend,     "Send",  eTiControlDial,   127, 0.0, NULL        },
    {kNl3ParamDelayTime,     "Time",  eTiControlDial,   127, 0.0, NULL        },
    {kNl3ParamDelayFeedback, "Fback", eTiControlDial,   127, 0.0, NULL        },
    {kNl3ParamDelayDamp,     "Damp",  eTiControlDial,   127, 0.0, NULL        },
    {kNl3ParamDelayWidth,    "Width", eTiControlDial,   127, 0.0, NULL        },
};
static const char *const kReverbTypes[]     = {"GATE REV", "GATE RISE", "GATE GENT", "GATE FALL", "DRY CHMB", "ECHO CHMB", "SMALL RM", "BIG RM", "MEDIUM 1", "MEDIUM 2", "M PLATE 1", "M PLATE 2", "LARGE 1", "LARGE 2", "L PLATE 1", "L PLATE 2"};
static const tNl3Control kReverb[]          = {
    {kNl3ParamReverbType,  "Type",  eTiControlChoice,  15, 0.0, kReverbTypes},
    {kNl3ParamReverbSend,  "Send",  eTiControlDial,   127, 0.0, NULL        },
    {kNl3ParamReverbDecay, "Decay", eTiControlDial,   127, 0.0, NULL        },
    {kNl3ParamReverbDamp,  "Damp",  eTiControlDial,   127, 0.0, NULL        },
};
static const char *const kOrders[]          = {"D+R+C", "D>R>C", "D>C>R", "R>D>C", "R>C>D", "C>D>R", "C>R>D", "D+[R>C]", "D+[C>R]", "R+[D>C]", "R+[C>D]", "C+[D>R]", "C+[R>D]", "D>[R+C]", "R>[D+C]", "C>[D+R]", "[R+C]>D", "[D+C]>R", "[D+R]>C"};
static const tNl3Control kDrive[]           = {
    {kNl3ParamDistLevel,  "Level",  eTiControlDial, 127, 0.0, NULL},
    {kNl3ParamDistComp,   "Comp",   eTiControlDial, 127, 0.0, NULL},
    {kNl3ParamDistOutput, "Output", eTiControlDial, 127, 0.0, NULL},
};
static const tNl3Control kEq[]              = {
    {kNl3ParamEqBass,   "Bass",   eTiControlDial, 127, -64.0, NULL},
    {kNl3ParamEqTreble, "Treble", eTiControlDial, 127, -64.0, NULL},
};
static const tNl3Control kEffectsMain[]     = {
    {kNl3ParamEffectsOrder, "Order", eTiControlChoice,  18, 0.0, kOrders},
    {kNl3ParamEffectsDry,   "Dry",   eTiControlDial,   127, 0.0, NULL   },
};
static const tNl3Section kEffectsPage[]     = {
    {"CHORUS", {0.30, 0.70, 0.85}, kChorus,      5},
    {"DELAY",  {0.85, 0.65, 0.30}, kDelay,       7},
    {"REVERB", {0.55, 0.45, 0.85}, kReverb,      4},
    {"DRIVE",  {0.90, 0.40, 0.30}, kDrive,       3},
    {"EQ",     {0.50, 0.75, 0.40}, kEq,          2},
    {"OUTPUT", {0.60, 0.60, 0.65}, kEffectsMain, 2},
};
static const char *const kFxBypass[]        = {"On", "Bypass"};
static const tNl3Control kGlobalFx[]        = {
    {kNl3ParamEffectsBypass, "Effects", eTiControlChoice, 1, 0.0, kFxBypass},
    {kNl3ParamDelayOn,       "Delay",   eTiControlChoice, 1, 0.0, kOffOn   },
    {kNl3ParamReverbOn,      "Reverb",  eTiControlChoice, 1, 0.0, kOffOn   },
};
static const char *const kGlideStyles[]     = {"PORTA", "GLISS"};
static const char *const kGlideCurves[]     = {"LINEAR", "EXP"};
static const char *const kGlideTypes[]      = {"NORMAL", "AUTO", "2 DOWN", "2 UP", "5 DOWN", "5 UP", "7 DOWN", "7 UP", "12 DOWN", "12 UP"};
static const char *const kTriggers[]        = {"PERCUSS", "ENSEMBLE"};
static const tNl3Control kGlide[]           = {
    {kNl3ParamGlideTime,  "Time",    eTiControlDial,   127, 0.0, NULL        },
    {kNl3ParamGlideStyle, "Style",   eTiControlChoice,   1, 0.0, kGlideStyles},
    {kNl3ParamGlideCurve, "Curve",   eTiControlChoice,   1, 0.0, kGlideCurves},
    {kNl3ParamGlideType,  "Type",    eTiControlChoice,   9, 0.0, kGlideTypes },
    {kNl3ParamOscTrigger, "Trigger", eTiControlChoice,   1, 0.0, kTriggers   },
};
static const tNl3Section kOscillatorsPage[] = {
    {"OSC 1", {0.30, 0.55, 0.85}, kOsc1,  7},
    {"MIXER", {0.60, 0.60, 0.65}, kMixer, 4},
    {"OSC 2", {0.30, 0.70, 0.60}, kOsc2,  7},
    {"GLIDE", {0.60, 0.60, 0.65}, kGlide, 5},
    {"OSC 3", {0.85, 0.55, 0.30}, kOsc3,  7},
};
static const tNl3Section kFilterPage[]      = {
    {"FILTER", {0.90, 0.55, 0.20}, kFilter, 6},
};
static const tNl3Control kEnv2[]            = {
    {kNl3ParamEnv2Attack,   "Attack",  eTiControlDial, 127,   0.0, NULL},
    {kNl3ParamEnv2Decay,    "Decay",   eTiControlDial, 127,   0.0, NULL},
    {kNl3ParamEnv2Sustain,  "Sustain", eTiControlDial, 127,   0.0, NULL},
    {kNl3ParamEnv2Release,  "Release", eTiControlDial, 127,   0.0, NULL},
    {kNl3ParamEnv2Velocity, "Vel",     eTiControlDial, 127,   0.0, NULL},
    {kNl3ParamEnv2Cutoff,   "Cutoff",  eTiControlDial, 127, -64.0, NULL},
};
static const tNl3Control kEnv3[]            = {
    {kNl3ParamEnv3Attack,   "Attack",  eTiControlDial, 127,   0.0, NULL},
    {kNl3ParamEnv3Decay,    "Decay",   eTiControlDial, 127,   0.0, NULL},
    {kNl3ParamEnv3Sustain,  "Sustain", eTiControlDial, 127,   0.0, NULL},
    {kNl3ParamEnv3Release,  "Release", eTiControlDial, 127,   0.0, NULL},
    {kNl3ParamEnv3Velocity, "Vel",     eTiControlDial, 127,   0.0, NULL},
    {kNl3ParamEnv3Cutoff,   "Cutoff",  eTiControlDial, 127, -64.0, NULL},
};
static const tNl3Section kAmpPage[]         = {
    {"OUTPUT", {0.65, 0.65, 0.65}, kAmp, 2},
};
static const tNl3Section kEnvelopesPage[]   = {
    {"AMP ENV", {0.85, 0.75, 0.25}, kAmpEnv, 6},
    {"ENV 2",   {0.90, 0.55, 0.20}, kEnv2,   6},
    {"ENV 3",   {0.70, 0.45, 0.80}, kEnv3,   6},
};
static const char *const kLfoShapes[]       = {"Square", "Saw", "Triangle", "S+H"};
static const char *const kLfoRanges[]       = {"x1", "x10", "x100"};
static const char *const kLfoTriggers[]     = {"Free", "Key"};
static const char *const kLfoFades[]        = {"In", "Out"};
#define LFO_CONTROLS(n)    {                                                              \
       {kNl3ParamLfo ## n ## Shape, "Shape", eTiControlChoice, 3, 0.0, kLfoShapes},       \
       {kNl3ParamLfo ## n ## Speed, "Speed", eTiControlDial, 127, 0.0, NULL},             \
       {kNl3ParamLfo ## n ## Range, "Range", eTiControlChoice, 2, 0.0, kLfoRanges},       \
       {kNl3ParamLfo ## n ## Delay, "Delay", eTiControlDial, 127, 0.0, NULL},             \
       {kNl3ParamLfo ## n ## Fade, "Fade", eTiControlChoice, 1, 0.0, kLfoFades},          \
       {kNl3ParamLfo ## n ## Offset, "Offset", eTiControlDial, 127, 0.0, NULL},           \
       {kNl3ParamLfo ## n ## Trigger, "Trigger", eTiControlChoice, 1, 0.0, kLfoTriggers}, \
       {kNl3ParamLfo ## n ## Cutoff, "Cutoff", eTiControlDial, 127, -64.0, NULL},         \
}
static const tNl3Control kLfo1[]            = LFO_CONTROLS(1);
static const tNl3Control kLfo2[]            = LFO_CONTROLS(2);
static const tNl3Section kLfoPage[]         = {
    {"LFO 1", {0.80, 0.55, 0.85}, kLfo1, 8},
    {"LFO 2", {0.55, 0.80, 0.85}, kLfo2, 8},
};
static const tNl3Control kHardware[]        = {
    {kNl3ParamHwLink,    "Link",    eTiControlChoice,   1, 0.0, kOffOn},
    {kNl3ParamHwChannel, "Channel", eTiControlDial,    15, 1.0, NULL  },
    {kNl3ParamHwDevice,  "Device",  eTiControlDial,   127, 0.0, NULL  },
};
static const tNl3Control kPan[]             = {
    {kNl3ParamPan,        "Pan",     eTiControlDial,   127, 0.0, NULL  },
    {kNl3ParamPanSpeed,   "Speed",   eTiControlDial,   127, 0.0, NULL  },
    {kNl3ParamPanDepth,   "Depth",   eTiControlDial,   127, 0.0, NULL  },
    {kNl3ParamPanEffects, "Effects", eTiControlChoice,   1, 0.0, kOffOn},
};
static const tNl3Section kGlobalPage[]      = {
    {"PLUG-IN",  {0.65, 0.65, 0.65}, kVoices,   4},
    {"EFFECTS",  {0.65, 0.65, 0.65}, kGlobalFx, 3},
    {"HARDWARE", {0.65, 0.65, 0.65}, kHardware, 3},
    {"PAN",      {0.65, 0.65, 0.65}, kPan,      4},
};

const tNl3Page           gNl3Pages[]        = {
    {"OSC",     kOscillatorsPage, 5},
    {"FILTER",  kFilterPage,      1},
    {"AMP",     kAmpPage,         1},
    {"ENV",     kEnvelopesPage,   3},
    {"LFO",     kLfoPage,         2},
    {"EFFECTS", kEffectsPage,     6},
    {"GLOBAL",  kGlobalPage,      4},
};
const uint32_t           gNl3PageCount      = sizeof(gNl3Pages) / sizeof(gNl3Pages[0]);

tNl3EffectsSound nl3_effects_sound(const double * v, double tempo) {
    return (tNl3EffectsSound){
        .dry            = v[kNl3ParamEffectsDry] * 127.0,
        .chorusSend     = v[kNl3ParamChorusSend] * 127.0,
        .chorusSpeed    = v[kNl3ParamChorusSpeed] * 127.0,
        .chorusDepth    = v[kNl3ParamChorusDepth] * 127.0,
        .chorusFeedback = v[kNl3ParamChorusFeedback] * 127.0,
        .chorusType     = (int)lround(v[kNl3ParamChorusType] * 2.0),
        .delayOn        = v[kNl3ParamDelayOn] > 0.5,
        .delaySend      = v[kNl3ParamDelaySend] * 127.0,
        .delayTime      = v[kNl3ParamDelayTime] * 127.0,
        .delayFeedback  = v[kNl3ParamDelayFeedback] * 127.0,
        .delayDamp      = v[kNl3ParamDelayDamp] * 127.0,
        .delayWidth     = v[kNl3ParamDelayWidth] * 127.0,
        .delayRatio     = (int)lround(v[kNl3ParamDelayRatio] * 12.0),
        .delaySync      = (int)lround(v[kNl3ParamDelaySync] * 34.0),
        .tempo          = tempo,
        .reverbOn       = v[kNl3ParamReverbOn] > 0.5,
        .order          = (int)lround(v[kNl3ParamEffectsOrder] * 18.0),
        .reverbSend     = v[kNl3ParamReverbSend] * 127.0,
        .reverbDecay    = v[kNl3ParamReverbDecay] * 127.0,
        .reverbDamp     = v[kNl3ParamReverbDamp] * 127.0,
        .reverbType     = (int)lround(v[kNl3ParamReverbType] * 15.0),
        .distLevel      = v[kNl3ParamDistLevel] * 127.0,
        .distComp       = v[kNl3ParamDistComp] * 127.0,
        .distOutput     = v[kNl3ParamDistOutput] * 127.0,
        .eqBass         = v[kNl3ParamEqBass] * 127.0,
        .eqTreble       = v[kNl3ParamEqTreble] * 127.0,
        .pan            = v[kNl3ParamPan] * 127.0,
        .panSpeed       = v[kNl3ParamPanSpeed] * 127.0,
        .panDepth       = v[kNl3ParamPanDepth] * 127.0,
        .panEffects     = v[kNl3ParamPanEffects] > 0.5,
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

void nl3_controls_sound(const double * v, tNl3Sound * sound) {
    for (int l = 0; l < NL3_LFOS; l++) {
        int    at    = l * (kNl3ParamLfo2Shape - kNl3ParamLfo1Shape);
        double speed = v[kNl3ParamLfo1Speed + at] * 127.0;

        sound->lfo[l]       = (tNl3LfoSound){
            .shape   = (int)lround(v[kNl3ParamLfo1Shape + at] * 3.0),
            .hz      = (speed * speed / 1638.4) * pow(10.0, lround(v[kNl3ParamLfo1Range + at] * 2.0)),
            .delay   = lfo_delay(v[kNl3ParamLfo1Delay + at] * 127.0),
            .fadeOut = v[kNl3ParamLfo1Fade + l] > 0.5,
            .offset  = v[kNl3ParamLfo1Offset + at],
            .keyed   = v[kNl3ParamLfo1Trigger + at] > 0.5,
        };
        sound->lfoCutoff[l] = lfo_semis(v[kNl3ParamLfo1Cutoff + at]);

        for (int o = 0; o < NL3_OSCS; o++) {
            sound->osc[o].lfoPitch[l] = lfo_semis(v[kNl3ParamOsc1Lfo1Pitch + at + (2 * o)]);
            sound->osc[o].lfoWidth[l] = ((v[kNl3ParamOsc1Lfo1Width + at + (2 * o)] * 127.0) - 64.0) / 64.0;
        }
    }

    sound->bend           = (2.0 * v[kNl3ParamPitchBend]) - 1.0;
    sound->wheel          = v[kNl3ParamModWheel];
    sound->pedal          = v[kNl3ParamSustainPedal] >= 0.5;
    sound->mono           = v[kNl3ParamPolyMode] > 0.5;
    sound->legato         = v[kNl3ParamEnvTrigger] > 0.5;
    sound->wheelCutoff    = (v[kNl3ParamWheelCutoff] * 127.0) - 64.0;
    sound->wheelResonance = (v[kNl3ParamWheelResonance] * 127.0) - 64.0;

    for (int o = 0; o < NL3_OSCS; o++) {
        sound->osc[o].bendRange  = (v[kNl3ParamOsc1BendRange + o] * 24.0) - 12.0;
        sound->osc[o].wheelPitch = (v[kNl3ParamOsc1WheelPitch + o] * 127.0) - 64.0;
        sound->osc[o].wheelLevel = ((v[kNl3ParamOsc1WheelLevel + o] * 127.0) - 64.0) / 64.0;
    }
}
