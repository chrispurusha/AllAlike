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

#ifndef __TI_PARAMS_H__
#define __TI_PARAMS_H__

#include "synthlibPlugin.h"
#include "prophDraw.h"
#include "prophEffects.h"
#include "prophVoices.h"

// The ids are what host projects save; few users yet, so a new one goes where it belongs (owner 2026-11-17)
enum {
    kProphParamVoiceLimit = 0,
    kProphParamVolume,
    kProphParamOscShape,
    kProphParamPulseWidth,
    kProphParamCutoff,
    kProphParamResonance,
    kProphParamFilterType,
    kProphParamFilterSlope,
    kProphParamAmpAttack,
    kProphParamAmpDecay,
    kProphParamAmpSustain,
    kProphParamAmpSustainTime,
    kProphParamAmpRelease,
    kProphParamProgram,
    kProphParamBank,
    kProphParamOscSoften,
    kProphParamOscLevel,
    kProphParamOsc2Shape,
    kProphParamOsc2Width,
    kProphParamOsc2Soften,
    kProphParamOsc2Level,
    kProphParamOsc3Shape,
    kProphParamOsc3Width,
    kProphParamOsc3Soften,
    kProphParamOsc3Level,
    kProphParamOsc1Octave,
    kProphParamOsc1Semitone,
    kProphParamOsc1Fine,
    kProphParamOsc2Octave,
    kProphParamOsc2Semitone,
    kProphParamOsc2Fine,
    kProphParamOsc3Octave,
    kProphParamOsc3Semitone,
    kProphParamOsc3Fine,
    kProphParamFilterTrack,
    kProphParamFilterDrive,
    kProphParamAmpVelocity,
    kProphParamEnv2Attack,
    kProphParamEnv2Decay,
    kProphParamEnv2Sustain,
    kProphParamEnv2Release,
    kProphParamEnv2Velocity,
    kProphParamEnv3Attack,
    kProphParamEnv3Decay,
    kProphParamEnv3Sustain,
    kProphParamEnv3Release,
    kProphParamEnv3Velocity,
    kProphParamEnv2Cutoff,
    kProphParamEnv3Cutoff,
    kProphParamNoiseLevel,
    kProphParamNoiseSoften,
    kProphParamRing13,
    kProphParamRing23,
    kProphParamFilterQ,
    kProphParamEffectsDry,
    kProphParamChorusSend,
    kProphParamChorusSpeed,
    kProphParamChorusDepth,
    kProphParamChorusFeedback,
    kProphParamChorusType,
    kProphParamDelayOn,
    kProphParamReverbOn,
    kProphParamDelaySend,
    kProphParamDelayTime,
    kProphParamDelayFeedback,
    kProphParamDelayDamp,
    kProphParamDelayWidth,
    kProphParamDelayRatio,
    kProphParamDelaySync,
    kProphParamGlideTime,
    kProphParamGlideStyle,
    kProphParamGlideCurve,
    kProphParamGlideType,
    kProphParamReverbSend,
    kProphParamReverbDecay,
    kProphParamReverbDamp,
    kProphParamReverbType,
    kProphParamEffectsOrder,
    kProphParamOscTrigger,
    kProphParamDistLevel,
    kProphParamDistComp,
    kProphParamDistOutput,
    kProphParamEqBass,
    kProphParamEqTreble,
    kProphParamPitchBend,
    kProphParamModWheel,
    kProphParamOsc1BendRange,
    kProphParamOsc2BendRange,
    kProphParamOsc3BendRange,
    kProphParamOsc1WheelPitch,
    kProphParamOsc2WheelPitch,
    kProphParamOsc3WheelPitch,
    kProphParamOsc1WheelLevel,
    kProphParamOsc2WheelLevel,
    kProphParamOsc3WheelLevel,
    kProphParamWheelCutoff,
    kProphParamWheelResonance,
    kProphParamPolyMode,
    kProphParamEnvTrigger,
    kProphParamMaster,
    kProphParamEffectsBypass,
    kProphParamSustainPedal,
    kProphParamLfo1Shape,
    kProphParamLfo1Speed,
    kProphParamLfo1Range,
    kProphParamLfo1Delay,
    kProphParamLfo1Offset,
    kProphParamLfo1Trigger,
    kProphParamLfo1Cutoff,
    kProphParamOsc1Lfo1Pitch,
    kProphParamOsc1Lfo1Width,
    kProphParamOsc2Lfo1Pitch,
    kProphParamOsc2Lfo1Width,
    kProphParamOsc3Lfo1Pitch,
    kProphParamOsc3Lfo1Width,
    kProphParamLfo2Shape,
    kProphParamLfo2Speed,
    kProphParamLfo2Range,
    kProphParamLfo2Delay,
    kProphParamLfo2Offset,
    kProphParamLfo2Trigger,
    kProphParamLfo2Cutoff,
    kProphParamOsc1Lfo2Pitch,
    kProphParamOsc1Lfo2Width,
    kProphParamOsc2Lfo2Pitch,
    kProphParamOsc2Lfo2Width,
    kProphParamOsc3Lfo2Pitch,
    kProphParamOsc3Lfo2Width,
    kProphParamLfo1Fade,
    kProphParamLfo2Fade,
    kProphParamHwLink,
    kProphParamHwChannel,
    kProphParamHwDevice,
    kProphParamPan,
    kProphParamPanSpeed,
    kProphParamPanDepth,
    kProphParamPanEffects,
    kProphParamAntiAlias,      // prophVoices notes §17
    kProphParamSameNote,       // prophVoices notes §21: Stack or Piano
    kProphParamFm13,           // prophVoices notes §18: Osc 1 frequency-modulates Osc 3, at the 1x3 level
    kProphParamFm23,           // Osc 2 does
    kProphParamFmNoise,        // the noise does, at the noise level
    kProphParamAmpSustainRate, // prophEnvelope notes §8
    kProphParamEnv2SustainTime,
    kProphParamEnv2SustainRate,
    kProphParamEnv3SustainTime,
    kProphParamEnv3SustainRate,
    kProphParamOneShot,          // prophEnvelope notes §9
    kProphParamOscsBypass,       // prophVoices notes §20
    kProphParamRing13WheelLevel, // prophVoices notes §19: the mixer levels' own routes
    kProphParamRing23WheelLevel,
    kProphParamNoiseWheelLevel,
    kProphParamOsc1Env2Level,
    kProphParamOsc2Env2Level,
    kProphParamOsc3Env2Level,
    kProphParamRing13Env2Level,
    kProphParamRing23Env2Level,
    kProphParamNoiseEnv2Level,
    kProphParamOsc1Env3Level,
    kProphParamOsc2Env3Level,
    kProphParamOsc3Env3Level,
    kProphParamRing13Env3Level,
    kProphParamRing23Env3Level,
    kProphParamNoiseEnv3Level,
    kProphParamOsc1Lfo1Level,
    kProphParamOsc2Lfo1Level,
    kProphParamOsc3Lfo1Level,
    kProphParamRing13Lfo1Level,
    kProphParamRing23Lfo1Level,
    kProphParamNoiseLfo1Level,
    kProphParamOsc1Lfo2Level,
    kProphParamOsc2Lfo2Level,
    kProphParamOsc3Lfo2Level,
    kProphParamRing13Lfo2Level,
    kProphParamRing23Lfo2Level,
    kProphParamNoiseLfo2Level,
    kProphParamCount
};

extern const tSynthLibParam gProphParams[kProphParamCount];
extern const tProphPage       gProphPages[];
extern const uint32_t       gProphPageCount;

// The effects as the engine takes them, from normalised parameters
tProphEffectsSound proph_effects_sound(const double * values, double tempo);

// Bend, the wheel and their routes, from normalised parameters
void proph_controls_sound(const double * values, tProphSound * sound);

#endif // __TI_PARAMS_H__
