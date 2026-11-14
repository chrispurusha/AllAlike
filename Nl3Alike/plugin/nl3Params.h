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

#ifndef __NL3_PARAMS_H__
#define __NL3_PARAMS_H__

#include "synthlibPlugin.h"
#include "nl3Draw.h"
#include "nl3Voices.h"

// notes §1: the plug-in's own settings, then the Program's fields in the order the instrument dumps them, then the
// four morph groups. The ids are what host projects save.
enum {
    kNl3ParamVoiceLimit = 0,
    kNl3ParamMaster,
    kNl3ParamArithmetic,
    kNl3ParamAliasing,
    kNl3ParamProgram,
    kNl3ParamBank,
    kNl3ParamPitchBend,
    kNl3ParamModWheel,
    kNl3ParamSustainPedal,
    kNl3ParamHwLink,
    kNl3ParamHwChannel,
    kNl3ParamHwDevice,
    kNl3ParamOsc1Waveform,
    kNl3ParamOsc1Shape,
    kNl3ParamOsc1Sync,
    kNl3ParamOsc1SyncTune,
    kNl3ParamOsc1NoiseSeed,
    kNl3ParamOsc1FmAmount,
    kNl3ParamOsc2Waveform,
    kNl3ParamOsc2Shape,
    kNl3ParamOsc2Sync,
    kNl3ParamOsc2Coarse,
    kNl3ParamOsc2Fine,
    kNl3ParamOsc2Kbt,
    kNl3ParamOsc2Partial,
    kNl3ParamOsc2SyncTune,
    kNl3ParamOsc2NoiseSeed,
    kNl3ParamOsc2FmAmount,
    kNl3ParamOsc2Carrier,
    kNl3ParamOsc2Modulator,
    kNl3ParamOsc2NoiseType,
    kNl3ParamOsc2NoiseFreq,
    kNl3ParamOscModAmount,
    kNl3ParamOscModType,
    kNl3ParamOscMix,
    kNl3ParamOscMixNoise,
    kNl3ParamFilterMode,
    kNl3ParamFilterType,
    kNl3ParamFilterSlope,
    kNl3ParamFilter2Type,
    kNl3ParamCutoff,
    kNl3ParamCutoff2,
    kNl3ParamResonance,
    kNl3ParamFilterDist,
    kNl3ParamFilterTrack,
    kNl3ParamFilterBypass,
    kNl3ParamFilter2Env,
    kNl3ParamFEnvAttack,
    kNl3ParamFEnvDecay,
    kNl3ParamFEnvSustain,
    kNl3ParamFEnvRelease,
    kNl3ParamFEnvAmount,
    kNl3ParamFEnvVelocity,
    kNl3ParamFEnvInvert,
    kNl3ParamFEnvExpAttack,
    kNl3ParamAmpAttack,
    kNl3ParamAmpDecay,
    kNl3ParamAmpSustain,
    kNl3ParamAmpRelease,
    kNl3ParamAmpExpAttack,
    kNl3ParamLevel,
    kNl3ParamLfo1Rate,
    kNl3ParamLfo1Amount,
    kNl3ParamLfo1Waveform,
    kNl3ParamLfo1Dest,
    kNl3ParamLfo1EnvKbs,
    kNl3ParamLfo1Mono,
    kNl3ParamLfo1Invert,
    kNl3ParamLfo1ClockSync,
    kNl3ParamLfo1Divisor,
    kNl3ParamLfo2Rate,
    kNl3ParamLfo2Amount,
    kNl3ParamLfo2Waveform,
    kNl3ParamLfo2Dest,
    kNl3ParamLfo2EnvKbs,
    kNl3ParamLfo2Mono,
    kNl3ParamLfo2Invert,
    kNl3ParamLfo2ClockSync,
    kNl3ParamLfo2Divisor,
    kNl3ParamMEnvAttack,
    kNl3ParamMEnvDecay,
    kNl3ParamMEnvAmount,
    kNl3ParamMEnvDest,
    kNl3ParamMEnvInvert,
    kNl3ParamMEnvRelease,
    kNl3ParamMEnvRepeat,
    kNl3ParamMEnvExpAttack,
    kNl3ParamGlideRate,
    kNl3ParamGlideMode,
    kNl3ParamVibratoRate,
    kNl3ParamVibratoAmount,
    kNl3ParamVibratoSource,
    kNl3ParamMono,
    kNl3ParamLegato,
    kNl3ParamMonoPriority,
    kNl3ParamUnison,
    kNl3ParamUnisonDetune,
    kNl3ParamOctaveShift,
    kNl3ParamTranspose,
    kNl3ParamChordMemory,
    kNl3ParamArpRun,
    kNl3ParamArpRate,
    kNl3ParamArpMode,
    kNl3ParamArpRange,
    kNl3ParamArpKbdSync,
    kNl3ParamArpClockSync,
    kNl3ParamArpDivisor,
    kNl3ParamMorphWheel_Lfo1Rate,
    kNl3ParamMorphWheel_Lfo1Amount,
    kNl3ParamMorphWheel_Lfo2Rate,
    kNl3ParamMorphWheel_Lfo2Amount,
    kNl3ParamMorphWheel_ModEnvAttack,
    kNl3ParamMorphWheel_ModEnvDecayRelease,
    kNl3ParamMorphWheel_ModEnvAmount,
    kNl3ParamMorphWheel_Osc2FinePitch,
    kNl3ParamMorphWheel_Osc2CoarsePitch,
    kNl3ParamMorphWheel_Oscmod,
    kNl3ParamMorphWheel_Oscmix,
    kNl3ParamMorphWheel_Osc1Shape,
    kNl3ParamMorphWheel_Osc2Shape,
    kNl3ParamMorphWheel_AmpEnvAttack,
    kNl3ParamMorphWheel_AmpEnvDecay,
    kNl3ParamMorphWheel_AmpEnvSustain,
    kNl3ParamMorphWheel_AmpEnvRelease,
    kNl3ParamMorphWheel_FiltEnvAttack,
    kNl3ParamMorphWheel_FiltEnvDecay,
    kNl3ParamMorphWheel_FiltEnvSustain,
    kNl3ParamMorphWheel_FiltEnvRelease,
    kNl3ParamMorphWheel_FiltEnvAmount,
    kNl3ParamMorphWheel_FiltFrequency1,
    kNl3ParamMorphWheel_FiltFrequency2,
    kNl3ParamMorphWheel_FiltResonance,
    kNl3ParamMorphWheel_OutputLevel,
    kNl3ParamMorphTouch_Lfo1Rate,
    kNl3ParamMorphTouch_Lfo1Amount,
    kNl3ParamMorphTouch_Lfo2Rate,
    kNl3ParamMorphTouch_Lfo2Amount,
    kNl3ParamMorphTouch_ModEnvAttack,
    kNl3ParamMorphTouch_ModEnvDecayRelease,
    kNl3ParamMorphTouch_ModEnvAmount,
    kNl3ParamMorphTouch_Osc2FinePitch,
    kNl3ParamMorphTouch_Osc2CoarsePitch,
    kNl3ParamMorphTouch_Oscmod,
    kNl3ParamMorphTouch_Oscmix,
    kNl3ParamMorphTouch_Osc1Shape,
    kNl3ParamMorphTouch_Osc2Shape,
    kNl3ParamMorphTouch_AmpEnvAttack,
    kNl3ParamMorphTouch_AmpEnvDecay,
    kNl3ParamMorphTouch_AmpEnvSustain,
    kNl3ParamMorphTouch_AmpEnvRelease,
    kNl3ParamMorphTouch_FiltEnvAttack,
    kNl3ParamMorphTouch_FiltEnvDecay,
    kNl3ParamMorphTouch_FiltEnvSustain,
    kNl3ParamMorphTouch_FiltEnvRelease,
    kNl3ParamMorphTouch_FiltEnvAmount,
    kNl3ParamMorphTouch_FiltFrequency1,
    kNl3ParamMorphTouch_FiltFrequency2,
    kNl3ParamMorphTouch_FiltResonance,
    kNl3ParamMorphTouch_OutputLevel,
    kNl3ParamMorphVelocity_Lfo1Rate,
    kNl3ParamMorphVelocity_Lfo1Amount,
    kNl3ParamMorphVelocity_Lfo2Rate,
    kNl3ParamMorphVelocity_Lfo2Amount,
    kNl3ParamMorphVelocity_ModEnvAttack,
    kNl3ParamMorphVelocity_ModEnvDecayRelease,
    kNl3ParamMorphVelocity_ModEnvAmount,
    kNl3ParamMorphVelocity_Osc2FinePitch,
    kNl3ParamMorphVelocity_Osc2CoarsePitch,
    kNl3ParamMorphVelocity_Oscmod,
    kNl3ParamMorphVelocity_Oscmix,
    kNl3ParamMorphVelocity_Osc1Shape,
    kNl3ParamMorphVelocity_Osc2Shape,
    kNl3ParamMorphVelocity_AmpEnvAttack,
    kNl3ParamMorphVelocity_AmpEnvDecay,
    kNl3ParamMorphVelocity_AmpEnvSustain,
    kNl3ParamMorphVelocity_AmpEnvRelease,
    kNl3ParamMorphVelocity_FiltEnvAttack,
    kNl3ParamMorphVelocity_FiltEnvDecay,
    kNl3ParamMorphVelocity_FiltEnvSustain,
    kNl3ParamMorphVelocity_FiltEnvRelease,
    kNl3ParamMorphVelocity_FiltEnvAmount,
    kNl3ParamMorphVelocity_FiltFrequency1,
    kNl3ParamMorphVelocity_FiltFrequency2,
    kNl3ParamMorphVelocity_FiltResonance,
    kNl3ParamMorphVelocity_OutputLevel,
    kNl3ParamMorphKbd_Lfo1Rate,
    kNl3ParamMorphKbd_Lfo1Amount,
    kNl3ParamMorphKbd_Lfo2Rate,
    kNl3ParamMorphKbd_Lfo2Amount,
    kNl3ParamMorphKbd_ModEnvAttack,
    kNl3ParamMorphKbd_ModEnvDecayRelease,
    kNl3ParamMorphKbd_ModEnvAmount,
    kNl3ParamMorphKbd_Osc2FinePitch,
    kNl3ParamMorphKbd_Osc2CoarsePitch,
    kNl3ParamMorphKbd_Oscmod,
    kNl3ParamMorphKbd_Oscmix,
    kNl3ParamMorphKbd_Osc1Shape,
    kNl3ParamMorphKbd_Osc2Shape,
    kNl3ParamMorphKbd_AmpEnvAttack,
    kNl3ParamMorphKbd_AmpEnvDecay,
    kNl3ParamMorphKbd_AmpEnvSustain,
    kNl3ParamMorphKbd_AmpEnvRelease,
    kNl3ParamMorphKbd_FiltEnvAttack,
    kNl3ParamMorphKbd_FiltEnvDecay,
    kNl3ParamMorphKbd_FiltEnvSustain,
    kNl3ParamMorphKbd_FiltEnvRelease,
    kNl3ParamMorphKbd_FiltEnvAmount,
    kNl3ParamMorphKbd_FiltFrequency1,
    kNl3ParamMorphKbd_FiltFrequency2,
    kNl3ParamMorphKbd_FiltResonance,
    kNl3ParamMorphKbd_OutputLevel,
    kNl3ParamCount
};

#define NL3_FIRST_PROGRAM_PARAM    (kNl3ParamOsc1Waveform)

extern const tSynthLibParam gNl3Params[kNl3ParamCount];
extern const tNl3Page       gNl3Pages[];
extern const uint32_t       gNl3PageCount;

// notes §3: where a parameter sits in the Program, in bits from the start of its unpacked body
typedef struct {
    uint32_t paramId;
    uint16_t at;
    uint8_t  bits;
    bool     isSigned;
} tNl3Field;

extern const tNl3Field      gNl3Fields[];
extern const uint32_t       gNl3FieldCount;

// The names of a switch's positions, NULL for a dial
const char * const * nl3_param_choices(uint32_t id);

// The voices' sound from normalised parameters
void nl3_sound_from_values(const double * values, tNl3Sound * sound);
// voices notes §25: the values with the four morph groups added for one voice; false when no morph is set
bool nl3_morph_values(const double * values, float velocity, int note, double * out);

#endif // __NL3_PARAMS_H__
