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

#ifndef __TI_PARAMS_H__
#define __TI_PARAMS_H__

#include "synthlibPlugin.h"
#include "nl3Draw.h"
#include "nl3Effects.h"
#include "nl3Voices.h"

// APPENDED TO, NEVER INSERTED INTO: the ids are what host projects save
enum {
    kNl3ParamVoiceLimit = 0,
    kNl3ParamVolume,
    kNl3ParamOscShape,
    kNl3ParamPulseWidth,
    kNl3ParamCutoff,
    kNl3ParamResonance,
    kNl3ParamFilterType,
    kNl3ParamFilterSlope,
    kNl3ParamAmpAttack,
    kNl3ParamAmpDecay,
    kNl3ParamAmpSustain,
    kNl3ParamAmpSustainTime,
    kNl3ParamAmpRelease,
    kNl3ParamProgram,
    kNl3ParamBank,
    kNl3ParamOscSoften,
    kNl3ParamOscLevel,
    kNl3ParamOsc2Shape,
    kNl3ParamOsc2Width,
    kNl3ParamOsc2Soften,
    kNl3ParamOsc2Level,
    kNl3ParamOsc3Shape,
    kNl3ParamOsc3Width,
    kNl3ParamOsc3Soften,
    kNl3ParamOsc3Level,
    kNl3ParamOsc1Octave,
    kNl3ParamOsc1Semitone,
    kNl3ParamOsc1Fine,
    kNl3ParamOsc2Octave,
    kNl3ParamOsc2Semitone,
    kNl3ParamOsc2Fine,
    kNl3ParamOsc3Octave,
    kNl3ParamOsc3Semitone,
    kNl3ParamOsc3Fine,
    kNl3ParamFilterTrack,
    kNl3ParamFilterDrive,
    kNl3ParamAmpVelocity,
    kNl3ParamEnv2Attack,
    kNl3ParamEnv2Decay,
    kNl3ParamEnv2Sustain,
    kNl3ParamEnv2Release,
    kNl3ParamEnv2Velocity,
    kNl3ParamEnv3Attack,
    kNl3ParamEnv3Decay,
    kNl3ParamEnv3Sustain,
    kNl3ParamEnv3Release,
    kNl3ParamEnv3Velocity,
    kNl3ParamEnv2Cutoff,
    kNl3ParamEnv3Cutoff,
    kNl3ParamNoiseLevel,
    kNl3ParamNoiseSoften,
    kNl3ParamRing13,
    kNl3ParamRing23,
    kNl3ParamFilterQ,
    kNl3ParamEffectsDry,
    kNl3ParamChorusSend,
    kNl3ParamChorusSpeed,
    kNl3ParamChorusDepth,
    kNl3ParamChorusFeedback,
    kNl3ParamChorusType,
    kNl3ParamDelayOn,
    kNl3ParamReverbOn,
    kNl3ParamDelaySend,
    kNl3ParamDelayTime,
    kNl3ParamDelayFeedback,
    kNl3ParamDelayDamp,
    kNl3ParamDelayWidth,
    kNl3ParamDelayRatio,
    kNl3ParamDelaySync,
    kNl3ParamGlideTime,
    kNl3ParamGlideStyle,
    kNl3ParamGlideCurve,
    kNl3ParamGlideType,
    kNl3ParamReverbSend,
    kNl3ParamReverbDecay,
    kNl3ParamReverbDamp,
    kNl3ParamReverbType,
    kNl3ParamEffectsOrder,
    kNl3ParamOscTrigger,
    kNl3ParamDistLevel,
    kNl3ParamDistComp,
    kNl3ParamDistOutput,
    kNl3ParamEqBass,
    kNl3ParamEqTreble,
    kNl3ParamPitchBend,
    kNl3ParamModWheel,
    kNl3ParamOsc1BendRange,
    kNl3ParamOsc2BendRange,
    kNl3ParamOsc3BendRange,
    kNl3ParamOsc1WheelPitch,
    kNl3ParamOsc2WheelPitch,
    kNl3ParamOsc3WheelPitch,
    kNl3ParamOsc1WheelLevel,
    kNl3ParamOsc2WheelLevel,
    kNl3ParamOsc3WheelLevel,
    kNl3ParamWheelCutoff,
    kNl3ParamWheelResonance,
    kNl3ParamPolyMode,
    kNl3ParamEnvTrigger,
    kNl3ParamMaster,
    kNl3ParamEffectsBypass,
    kNl3ParamSustainPedal,
    kNl3ParamLfo1Shape,
    kNl3ParamLfo1Speed,
    kNl3ParamLfo1Range,
    kNl3ParamLfo1Delay,
    kNl3ParamLfo1Offset,
    kNl3ParamLfo1Trigger,
    kNl3ParamLfo1Cutoff,
    kNl3ParamOsc1Lfo1Pitch,
    kNl3ParamOsc1Lfo1Width,
    kNl3ParamOsc2Lfo1Pitch,
    kNl3ParamOsc2Lfo1Width,
    kNl3ParamOsc3Lfo1Pitch,
    kNl3ParamOsc3Lfo1Width,
    kNl3ParamLfo2Shape,
    kNl3ParamLfo2Speed,
    kNl3ParamLfo2Range,
    kNl3ParamLfo2Delay,
    kNl3ParamLfo2Offset,
    kNl3ParamLfo2Trigger,
    kNl3ParamLfo2Cutoff,
    kNl3ParamOsc1Lfo2Pitch,
    kNl3ParamOsc1Lfo2Width,
    kNl3ParamOsc2Lfo2Pitch,
    kNl3ParamOsc2Lfo2Width,
    kNl3ParamOsc3Lfo2Pitch,
    kNl3ParamOsc3Lfo2Width,
    kNl3ParamLfo1Fade,
    kNl3ParamLfo2Fade,
    kNl3ParamHwLink,
    kNl3ParamHwChannel,
    kNl3ParamHwDevice,
    kNl3ParamPan,
    kNl3ParamPanSpeed,
    kNl3ParamPanDepth,
    kNl3ParamPanEffects,
    kNl3ParamCount
};

extern const tSynthLibParam gNl3Params[kNl3ParamCount];
extern const tNl3Page       gNl3Pages[];
extern const uint32_t       gNl3PageCount;

// The effects as the engine takes them, from normalised parameters
tNl3EffectsSound nl3_effects_sound(const double * values, double tempo);

// Bend, the wheel and their routes, from normalised parameters
void nl3_controls_sound(const double * values, tNl3Sound * sound);

#endif // __TI_PARAMS_H__
