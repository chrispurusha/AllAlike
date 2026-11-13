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

#ifndef __TI_PARAMS_H__
#define __TI_PARAMS_H__

#include "synthlibPlugin.h"
#include "sn2Draw.h"
#include "sn2Effects.h"
#include "sn2Voices.h"

// APPENDED TO, NEVER INSERTED INTO: the ids are what host projects save
enum {
    kSn2ParamVoiceLimit = 0,
    kSn2ParamVolume,
    kSn2ParamOscShape,
    kSn2ParamPulseWidth,
    kSn2ParamCutoff,
    kSn2ParamResonance,
    kSn2ParamFilterType,
    kSn2ParamFilterSlope,
    kSn2ParamAmpAttack,
    kSn2ParamAmpDecay,
    kSn2ParamAmpSustain,
    kSn2ParamAmpSustainTime,
    kSn2ParamAmpRelease,
    kSn2ParamProgram,
    kSn2ParamBank,
    kSn2ParamOscSoften,
    kSn2ParamOscLevel,
    kSn2ParamOsc2Shape,
    kSn2ParamOsc2Width,
    kSn2ParamOsc2Soften,
    kSn2ParamOsc2Level,
    kSn2ParamOsc3Shape,
    kSn2ParamOsc3Width,
    kSn2ParamOsc3Soften,
    kSn2ParamOsc3Level,
    kSn2ParamOsc1Octave,
    kSn2ParamOsc1Semitone,
    kSn2ParamOsc1Fine,
    kSn2ParamOsc2Octave,
    kSn2ParamOsc2Semitone,
    kSn2ParamOsc2Fine,
    kSn2ParamOsc3Octave,
    kSn2ParamOsc3Semitone,
    kSn2ParamOsc3Fine,
    kSn2ParamFilterTrack,
    kSn2ParamFilterDrive,
    kSn2ParamAmpVelocity,
    kSn2ParamEnv2Attack,
    kSn2ParamEnv2Decay,
    kSn2ParamEnv2Sustain,
    kSn2ParamEnv2Release,
    kSn2ParamEnv2Velocity,
    kSn2ParamEnv3Attack,
    kSn2ParamEnv3Decay,
    kSn2ParamEnv3Sustain,
    kSn2ParamEnv3Release,
    kSn2ParamEnv3Velocity,
    kSn2ParamEnv2Cutoff,
    kSn2ParamEnv3Cutoff,
    kSn2ParamNoiseLevel,
    kSn2ParamNoiseSoften,
    kSn2ParamRing13,
    kSn2ParamRing23,
    kSn2ParamFilterQ,
    kSn2ParamEffectsDry,
    kSn2ParamChorusSend,
    kSn2ParamChorusSpeed,
    kSn2ParamChorusDepth,
    kSn2ParamChorusFeedback,
    kSn2ParamChorusType,
    kSn2ParamDelayOn,
    kSn2ParamReverbOn,
    kSn2ParamDelaySend,
    kSn2ParamDelayTime,
    kSn2ParamDelayFeedback,
    kSn2ParamDelayDamp,
    kSn2ParamDelayWidth,
    kSn2ParamDelayRatio,
    kSn2ParamDelaySync,
    kSn2ParamGlideTime,
    kSn2ParamGlideStyle,
    kSn2ParamGlideCurve,
    kSn2ParamGlideType,
    kSn2ParamReverbSend,
    kSn2ParamReverbDecay,
    kSn2ParamReverbDamp,
    kSn2ParamReverbType,
    kSn2ParamEffectsOrder,
    kSn2ParamOscTrigger,
    kSn2ParamDistLevel,
    kSn2ParamDistComp,
    kSn2ParamDistOutput,
    kSn2ParamEqBass,
    kSn2ParamEqTreble,
    kSn2ParamPitchBend,
    kSn2ParamModWheel,
    kSn2ParamOsc1BendRange,
    kSn2ParamOsc2BendRange,
    kSn2ParamOsc3BendRange,
    kSn2ParamOsc1WheelPitch,
    kSn2ParamOsc2WheelPitch,
    kSn2ParamOsc3WheelPitch,
    kSn2ParamOsc1WheelLevel,
    kSn2ParamOsc2WheelLevel,
    kSn2ParamOsc3WheelLevel,
    kSn2ParamWheelCutoff,
    kSn2ParamWheelResonance,
    kSn2ParamPolyMode,
    kSn2ParamEnvTrigger,
    kSn2ParamMaster,
    kSn2ParamEffectsBypass,
    kSn2ParamSustainPedal,
    kSn2ParamLfo1Shape,
    kSn2ParamLfo1Speed,
    kSn2ParamLfo1Range,
    kSn2ParamLfo1Delay,
    kSn2ParamLfo1Offset,
    kSn2ParamLfo1Trigger,
    kSn2ParamLfo1Cutoff,
    kSn2ParamOsc1Lfo1Pitch,
    kSn2ParamOsc1Lfo1Width,
    kSn2ParamOsc2Lfo1Pitch,
    kSn2ParamOsc2Lfo1Width,
    kSn2ParamOsc3Lfo1Pitch,
    kSn2ParamOsc3Lfo1Width,
    kSn2ParamLfo2Shape,
    kSn2ParamLfo2Speed,
    kSn2ParamLfo2Range,
    kSn2ParamLfo2Delay,
    kSn2ParamLfo2Offset,
    kSn2ParamLfo2Trigger,
    kSn2ParamLfo2Cutoff,
    kSn2ParamOsc1Lfo2Pitch,
    kSn2ParamOsc1Lfo2Width,
    kSn2ParamOsc2Lfo2Pitch,
    kSn2ParamOsc2Lfo2Width,
    kSn2ParamOsc3Lfo2Pitch,
    kSn2ParamOsc3Lfo2Width,
    kSn2ParamLfo1Fade,
    kSn2ParamLfo2Fade,
    kSn2ParamHwLink,
    kSn2ParamHwChannel,
    kSn2ParamHwDevice,
    kSn2ParamPan,
    kSn2ParamPanSpeed,
    kSn2ParamPanDepth,
    kSn2ParamPanEffects,
    kSn2ParamAntiAlias,      // sn2Voices notes §17
    kSn2ParamFm13,           // sn2Voices notes §18: Osc 1 frequency-modulates Osc 3, at the 1x3 level
    kSn2ParamFm23,           // Osc 2 does
    kSn2ParamFmNoise,        // the noise does, at the noise level
    kSn2ParamAmpSustainRate, // sn2Envelope notes §8
    kSn2ParamEnv2SustainTime,
    kSn2ParamEnv2SustainRate,
    kSn2ParamEnv3SustainTime,
    kSn2ParamEnv3SustainRate,
    kSn2ParamOneShot,          // sn2Envelope notes §9
    kSn2ParamOscsBypass,       // sn2Voices notes §20
    kSn2ParamRing13WheelLevel, // sn2Voices notes §19: the mixer levels' own routes
    kSn2ParamRing23WheelLevel,
    kSn2ParamNoiseWheelLevel,
    kSn2ParamOsc1Env2Level,
    kSn2ParamOsc2Env2Level,
    kSn2ParamOsc3Env2Level,
    kSn2ParamRing13Env2Level,
    kSn2ParamRing23Env2Level,
    kSn2ParamNoiseEnv2Level,
    kSn2ParamOsc1Env3Level,
    kSn2ParamOsc2Env3Level,
    kSn2ParamOsc3Env3Level,
    kSn2ParamRing13Env3Level,
    kSn2ParamRing23Env3Level,
    kSn2ParamNoiseEnv3Level,
    kSn2ParamOsc1Lfo1Level,
    kSn2ParamOsc2Lfo1Level,
    kSn2ParamOsc3Lfo1Level,
    kSn2ParamRing13Lfo1Level,
    kSn2ParamRing23Lfo1Level,
    kSn2ParamNoiseLfo1Level,
    kSn2ParamOsc1Lfo2Level,
    kSn2ParamOsc2Lfo2Level,
    kSn2ParamOsc3Lfo2Level,
    kSn2ParamRing13Lfo2Level,
    kSn2ParamRing23Lfo2Level,
    kSn2ParamNoiseLfo2Level,
    kSn2ParamCount
};

extern const tSynthLibParam gSn2Params[kSn2ParamCount];
extern const tSn2Page       gSn2Pages[];
extern const uint32_t       gSn2PageCount;

// The effects as the engine takes them, from normalised parameters
tSn2EffectsSound sn2_effects_sound(const double * values, double tempo);

// Bend, the wheel and their routes, from normalised parameters
void sn2_controls_sound(const double * values, tSn2Sound * sound);

#endif // __TI_PARAMS_H__
