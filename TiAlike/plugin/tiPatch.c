/*
 * TiAlike - Singles: reading them out of .syx and .mid files, and writing one back.
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
// Notes: Docs/code-notes/tiPatch.md - "// notes §k" refers there.

#include <dirent.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "tiPatch.h"

#include <sys/stat.h>

#define SOUNDS_FOLDER          "Library/Mobile Documents/com~apple~CloudDocs/Patches/VirusPatches" // under $HOME
#define PRESETS_FOLDER         "Library/Application Support/TiAlike"                               // beside prefs.txt
#define PRESETS_FILE           "Presets.syx"

// notes §1: where things sit in the message
#define DEVICE_AT              (5)
#define COMMAND_AT             (6)
#define BANK_AT                (7)
#define PROGRAM_AT             (8)
#define DATA_AT                (9)
#define CHECKSUM1_AT           (DATA_AT + 256)
#define CHECKSUM2_AT           (CHECKSUM1_AT + 1 + 256)
#define OLDER_SINGLE_LENGTH    (CHECKSUM1_AT + 2)
#define PAGE_A                 (DATA_AT)
#define PAGE_B                 (DATA_AT + 128)
#define NAME_AT                (PAGE_B + 112)
#define PAGE_C                 (CHECKSUM1_AT + 1)
#define PAGE_D                 (PAGE_C + 128)

#define SINGLE_DUMP            (0x10)
#define OMNI_DEVICE            (0x10)
#define EDIT_BUFFER_BANK       (0x00)
#define EDIT_BUFFER_PROG       (0x40)

#define MAX_FILE_BYTES         (16 * 1024 * 1024)
#define MAX_FOLDER_FILES       (4096)

static const uint8_t kHeader[5] = {0xF0, 0x00, 0x20, 0x33, 0x01};

// notes §3: the bits of a byte a parameter takes; the rest of the byte is kept when it is written back
static const struct {
    uint32_t paramId;
    uint16_t at;
    uint8_t  mask;
}                    kCarried[] = {
    {kTiParamVolume,                          PAGE_A + 91,  0x7F},
    {kTiParamOsc1Wave,                        PAGE_A + 19,  0x7F},
    {kTiParamOsc1Semi,                        PAGE_A + 20,  0x7F},
    {kTiParamOsc2Shape,                       PAGE_A + 22,  0x7F},
    {kTiParamOsc2Width,                       PAGE_A + 23,  0x7F},
    {kTiParamOsc2Wave,                        PAGE_A + 24,  0x7F},
    {kTiParamOsc2Semi,                        PAGE_A + 25,  0x7F},
    {kTiParamOsc2Detune,                      PAGE_A + 26,  0x7F},
    {kTiParamFm,                              PAGE_A + 27,  0x7F},
    {kTiParamSync,                            PAGE_A + 28,  0x7F},
    {kTiParamOscBalance,                      PAGE_A + 33,  0x7F},
    {kTiParamSub,                             PAGE_A + 34,  0x7F},
    {kTiParamSubShape,                        PAGE_A + 35,  0x7F},
    {kTiParamOscVolume,                       PAGE_A + 36,  0x7F},
    {kTiParamNoise,                           PAGE_A + 37,  0x7F},
    {kTiParamNoiseColor,                      PAGE_A + 39,  0x7F},
    {kTiParamRing,                            PAGE_A + 50,  0x7F},
    {kTiParamEnvAmount1,                      PAGE_A + 44,  0x7F},
    {kTiParamEnvAmount2,                      PAGE_A + 45,  0x7F},
    {kTiParamKey1,                            PAGE_A + 46,  0x7F},
    {kTiParamKey2,                            PAGE_A + 47,  0x7F},
    {kTiParamSaturation,                      PAGE_A + 49,  0x7F},
    {kTiParamFilterAttack,                    PAGE_A + 54,  0x7F},
    {kTiParamFilterDecay,                     PAGE_A + 55,  0x7F},
    {kTiParamFilterSustain,                   PAGE_A + 56,  0x7F},
    {kTiParamFilterSustainTime,               PAGE_A + 57,  0x7F},
    {kTiParamFilterRelease,                   PAGE_A + 58,  0x7F},
    {kTiParamLfo1Rate,                        PAGE_A + 67,  0x7F},
    {kTiParamLfo1Shape,                       PAGE_A + 68,  0x7F},
    {kTiParamLfo1Pitch,                       PAGE_A + 74,  0x7F},
    {kTiParamLfo2Rate,                        PAGE_A + 79,  0x7F},
    {kTiParamLfo2Shape,                       PAGE_A + 80,  0x7F},
    {kTiParamLfo2Cutoff,                      PAGE_A + 88,  0x7F},
    {kTiParamKeyMode,                         PAGE_A + 94,  0x7F},
    {kTiParamPortamento,                      PAGE_A + 5,   0x7F},
    {kTiParamPan,                             PAGE_A + 10,  0x7F},
    {kTiParamTranspose,                       PAGE_A + 93,  0x7F},
    {kTiParamPunch,                           PAGE_B + 36,  0x7F},
    {kTiParamOsc3Mode,                        PAGE_B + 41,  0x7F},
    {kTiParamOsc3Volume,                      PAGE_B + 42,  0x7F},
    {kTiParamOsc3Semi,                        PAGE_B + 43,  0x7F},
    {kTiParamOsc3Detune,                      PAGE_B + 44,  0x7F},
    {kTiParamLfo3Rate,                        PAGE_B + 7,   0x7F},
    {kTiParamLfo3Shape,                       PAGE_B + 8,   0x7F},
    {kTiParamLfo3Amount,                      PAGE_B + 12,  0x7F},
    {kTiParamLfo3Fade,                        PAGE_B + 13,  0x7F},
    {kTiParamEnv3Attack,                      PAGE_C + 80,  0x7F},
    {kTiParamEnv3Decay,                       PAGE_C + 81,  0x7F},
    {kTiParamEnv3Sustain,                     PAGE_C + 82,  0x7F},
    {kTiParamEnv3SustainTime,                 PAGE_C + 83,  0x7F},
    {kTiParamEnv3Release,                     PAGE_C + 84,  0x7F},
    {kTiParamEnv4Attack,                      PAGE_C + 85,  0x7F},
    {kTiParamEnv4Decay,                       PAGE_C + 86,  0x7F},
    {kTiParamEnv4Sustain,                     PAGE_C + 87,  0x7F},
    {kTiParamEnv4SustainTime,                 PAGE_C + 88,  0x7F},
    {kTiParamEnv4Release,                     PAGE_C + 89,  0x7F},
    {kTiParamUnisonVoices,                    PAGE_D + 120, 0x7F},
    {kTiParamUnisonDetune,                    PAGE_D + 121, 0x7F},
    {kTiParamUnisonSpread,                    PAGE_D + 122, 0x7F},
    {kTiParamUnisonPhase,                     PAGE_D + 123, 0x7F},
    {kTiParamOsc1Model,                       PAGE_C + 30,  0x7F},
    {kTiParamOsc2Model,                       PAGE_C + 35,  0x7F},
    {kTiParamMod1Source,                      PAGE_B + 64,  0x7F},
    {kTiParamMod1Dest,                        PAGE_B + 65,  0x7F},
    {kTiParamMod1Amount,                      PAGE_B + 66,  0x7F},
    {kTiParamMod2Source,                      PAGE_B + 67,  0x7F},
    {kTiParamMod2Dest,                        PAGE_B + 68,  0x7F},
    {kTiParamMod2Amount,                      PAGE_B + 69,  0x7F},
    {kTiParamMod3Source,                      PAGE_B + 72,  0x7F},
    {kTiParamMod3Dest,                        PAGE_B + 73,  0x7F},
    {kTiParamMod3Amount,                      PAGE_B + 74,  0x7F},
    {kTiParamMod4Source,                      PAGE_B + 103, 0x7F},
    {kTiParamMod4Dest,                        PAGE_B + 104, 0x7F},
    {kTiParamMod4Amount,                      PAGE_B + 105, 0x7F},
    {kTiParamMod5Source,                      PAGE_B + 106, 0x7F},
    {kTiParamMod5Dest,                        PAGE_B + 107, 0x7F},
    {kTiParamMod5Amount,                      PAGE_B + 108, 0x7F},
    {kTiParamMod6Source,                      PAGE_B + 109, 0x7F},
    {kTiParamMod6Dest,                        PAGE_B + 110, 0x7F},
    {kTiParamMod6Amount,                      PAGE_B + 111, 0x7F},
    {kTiParamOscShape,                        PAGE_A + 17,  0x7F},
    {kTiParamPulseWidth,                      PAGE_A + 18,  0x7F},
    {kTiParamCutoff,                          PAGE_A + 40,  0x7F},
    {kTiParamCutoff2,                         PAGE_A + 41,  0x7F},
    {kTiParamResonance,                       PAGE_A + 42,  0x7F},
    {kTiParamResonance2,                      PAGE_A + 43,  0x7F},
    {kTiParamBalance,                         PAGE_A + 48,  0x7F},
    {kTiParamFilterMode,                      PAGE_A + 51,  0x7F},
    {kTiParamFilter2Mode,                     PAGE_A + 52,  0x7F},
    {kTiParamRouting,                         PAGE_A + 53,  0x03},
    {kTiParamFilterLink,                      PAGE_B + 32,  0x7F},
    {kTiParamAmpAttack,                       PAGE_A + 59,  0x7F},
    {kTiParamAmpDecay,                        PAGE_A + 60,  0x7F},
    {kTiParamAmpSustain,                      PAGE_A + 61,  0x7F},
    {kTiParamAmpSustainTime,                  PAGE_A + 62,  0x7F},
    {kTiParamAmpRelease,                      PAGE_A + 63,  0x7F},
    {kTiParamA21Osc1Keyfollow,                PAGE_A + 21,  0x7F},
    {kTiParamA31Osc2Keyfollow,                PAGE_A + 31,  0x7F},
    {kTiParamA29Osc2FiltEnvAmt,               PAGE_A + 29,  0x7F},
    {kTiParamA30FmFiltEnvAmt,                 PAGE_A + 30,  0x7F},
    {kTiParamB34OscFmMode,                    PAGE_B + 34,  0x7F},
    {kTiParamB35OscInitPhase,                 PAGE_B + 35,  0x7F},
    {kTiParamB15ArpMode,                      PAGE_B + 15,  0x07},
    {kTiParamB2ArpPattern,                    PAGE_B + 2,   0x3F},
    {kTiParamB3ArpOctaves,                    PAGE_B + 3,   0x03},
    {kTiParamB4ArpHold,                       PAGE_B + 4,   0x01},
    {kTiParamB5ArpNoteLength,                 PAGE_B + 5,   0x7F},
    {kTiParamB6ArpSwing,                      PAGE_B + 6,   0x7F},
    {kTiParamB17ArpClock,                     PAGE_B + 17,  0x1F},
    {kTiParamB30Filter1EnvPolarity,           PAGE_B + 30,  0x7F},
    {kTiParamB31Filter2EnvPolarity,           PAGE_B + 31,  0x7F},
    {kTiParamB33FilterKeytrackBase,           PAGE_B + 33,  0x7F},
    {kTiParamB47Osc1ShapeVelocity,            PAGE_B + 47,  0x7F},
    {kTiParamB48Osc2ShapeVelocity,            PAGE_B + 48,  0x7F},
    {kTiParamB49PulsewidthVelocity,           PAGE_B + 49,  0x7F},
    {kTiParamB50FmAmountVelocity,             PAGE_B + 50,  0x7F},
    {kTiParamB54Flt1EnvamtVelocity,           PAGE_B + 54,  0x7F},
    {kTiParamB55Flt2EnvamtVelocity,           PAGE_B + 55,  0x7F},
    {kTiParamB56Resonance1Velocity,           PAGE_B + 56,  0x7F},
    {kTiParamB57Resonance2Velocity,           PAGE_B + 57,  0x7F},
    {kTiParamB60AmpVelocity,                  PAGE_B + 60,  0x7F},
    {kTiParamB61PanoramaVelocity,             PAGE_B + 61,  0x7F},
    {kTiParamA69Lfo1EnvMode,                  PAGE_A + 69,  0x7F},
    {kTiParamA70Lfo1Mode,                     PAGE_A + 70,  0x7F},
    {kTiParamA71Lfo1Symmetry,                 PAGE_A + 71,  0x7F},
    {kTiParamA72Lfo1Keyfollow,                PAGE_A + 72,  0x7F},
    {kTiParamA73Lfo1Keytrigger,               PAGE_A + 73,  0x7F},
    {kTiParamB18Lfo1Clock,                    PAGE_B + 18,  0x7F},
    {kTiParamA75Osc2Lfo1Amount,               PAGE_A + 75,  0x7F},
    {kTiParamA76PwLfo1Amount,                 PAGE_A + 76,  0x7F},
    {kTiParamA77ResoLfo1Amount,               PAGE_A + 77,  0x7F},
    {kTiParamA78FiltgainLfo1Amount,           PAGE_A + 78,  0x7F},
    {kTiParamA81Lfo2EnvMode,                  PAGE_A + 81,  0x7F},
    {kTiParamA82Lfo2Mode,                     PAGE_A + 82,  0x7F},
    {kTiParamA83Lfo2Symmetry,                 PAGE_A + 83,  0x7F},
    {kTiParamA84Lfo2Keyfollow,                PAGE_A + 84,  0x7F},
    {kTiParamA85Lfo2Keytrigger,               PAGE_A + 85,  0x7F},
    {kTiParamB19Lfo2Clock,                    PAGE_B + 19,  0x7F},
    {kTiParamA86ShapeLfo2Amount,              PAGE_A + 86,  0x7F},
    {kTiParamA87FmLfo2Amount,                 PAGE_A + 87,  0x7F},
    {kTiParamA89Cutoff2Lfo2Amount,            PAGE_A + 89,  0x7F},
    {kTiParamA90PanLfo2Amount,                PAGE_A + 90,  0x7F},
    {kTiParamB9Lfo3Mode,                      PAGE_B + 9,   0x7F},
    {kTiParamB10Lfo3Keyfollow,                PAGE_B + 10,  0x7F},
    {kTiParamB11Lfo3Destination,              PAGE_B + 11,  0x7F},
    {kTiParamB21Lfo3Clock,                    PAGE_B + 21,  0x7F},
    {kTiParamB79Lfo1AssignDest,               PAGE_B + 79,  0x7F},
    {kTiParamB80Lfo1AssignAmount,             PAGE_B + 80,  0x7F},
    {kTiParamB81Lfo2AssignDest,               PAGE_B + 81,  0x7F},
    {kTiParamB82Lfo2AssignAmount,             PAGE_B + 82,  0x7F},
    {kTiParamB16ClockTempo,                   PAGE_B + 16,  0x7F},
    {kTiParamB25ControlSmoothMode,            PAGE_B + 25,  0x7F},
    {kTiParamB26BenderRangeUp,                PAGE_B + 26,  0x7F},
    {kTiParamB27BenderRangeDown,              PAGE_B + 27,  0x7F},
    {kTiParamC37Osc1WavetableSync,            PAGE_C + 37,  0x7F},
    {kTiParamC42Osc1WavetableFormantshift,    PAGE_C + 42,  0x7F},
    {kTiParamC43Osc1WavetableInternalDetune,  PAGE_C + 43,  0x7F},
    {kTiParamC44Osc1WavetableInterpolation,   PAGE_C + 44,  0x7F},
    {kTiParamC57Osc2WavetableSync,            PAGE_C + 57,  0x7F},
    {kTiParamC62Osc2WavetableFormantshift,    PAGE_C + 62,  0x7F},
    {kTiParamC63Osc2WavetableInternalDetune,  PAGE_C + 63,  0x7F},
    {kTiParamC64Osc2WavetableInterpolation,   PAGE_C + 64,  0x7F},
    {kTiParamB100DistortionCurve,             PAGE_B + 100, 0x7F},
    {kTiParamB101DistortionIntensity,         PAGE_B + 101, 0x7F},
    {kTiParamC70PatchDistortionTrebleBooster, PAGE_C + 70,  0x7F},
    {kTiParamC71PatchDistortionHighCut,       PAGE_C + 71,  0x7F},
    {kTiParamC72PatchDistortionMix,           PAGE_C + 72,  0x7F},
    {kTiParamC73PatchDistortionQuality,       PAGE_C + 73,  0x7F},
    {kTiParamC74PatchDistortionTone127,       PAGE_C + 74,  0x7F},
    {kTiParamC26CharacterType,                PAGE_C + 26,  0x7F},
    {kTiParamB97BassIntensity,                PAGE_B + 97,  0x7F},
    {kTiParamB98BassTune,                     PAGE_B + 98,  0x7F},
    {kTiParamC19FilterBankType,               PAGE_C + 19,  0x7F},
    {kTiParamC20FilterBankMix,                PAGE_C + 20,  0x7F},
    {kTiParamC21FilterBankCombFrequency,      PAGE_C + 21,  0x7F},
    {kTiParamC22FilterBankStereoPhase,        PAGE_C + 22,  0x7F},
    {kTiParamC23FilterBankFilterType,         PAGE_C + 23,  0x7F},
    {kTiParamC24FilterBankShapeR,             PAGE_C + 24,  0x7F},
    {kTiParamC25FilterBankResonance,          PAGE_C + 25,  0x7F},
    {kTiParamA103ChorusType,                  PAGE_A + 103, 0x7F},
    {kTiParamA104ChorusMix2,                  PAGE_A + 104, 0x7F},
    {kTiParamA105ChorusMix,                   PAGE_A + 105, 0x7F},
    {kTiParamA106ChorusRate,                  PAGE_A + 106, 0x7F},
    {kTiParamA107ChorusDistance,              PAGE_A + 107, 0x7F},
    {kTiParamA108ChorusAmount,                PAGE_A + 108, 0x7F},
    {kTiParamA109ChorusFeedback,              PAGE_A + 109, 0x7F},
    {kTiParamA110ChorusLfoShape,              PAGE_A + 110, 0x7F},
    {kTiParamA111ChorusXOver,                 PAGE_A + 111, 0x7F},
    {kTiParamB84PhaserMode,                   PAGE_B + 84,  0x7F},
    {kTiParamB85PhaserMix,                    PAGE_B + 85,  0x7F},
    {kTiParamB86PhaserRate,                   PAGE_B + 86,  0x7F},
    {kTiParamB87PhaserDepth,                  PAGE_B + 87,  0x7F},
    {kTiParamB88PhaserFrequency,              PAGE_B + 88,  0x7F},
    {kTiParamB89PhaserFeedback,               PAGE_B + 89,  0x7F},
    {kTiParamB90PhaserSpread,                 PAGE_B + 90,  0x7F},
    {kTiParamB95LoweqGain,                    PAGE_B + 95,  0x7F},
    {kTiParamB45LoweqFrequency,               PAGE_B + 45,  0x7F},
    {kTiParamB92MideqGain,                    PAGE_B + 92,  0x7F},
    {kTiParamB93MideqFrequency,               PAGE_B + 93,  0x7F},
    {kTiParamB94MideqQFactor,                 PAGE_B + 94,  0x7F},
    {kTiParamB96HigheqGain,                   PAGE_B + 96,  0x7F},
    {kTiParamB46HigheqFrequency,              PAGE_B + 46,  0x7F},
    {kTiParamA112DelayMode,                   PAGE_A + 112, 0x7F},
    {kTiParamA113DelaySend,                   PAGE_A + 113, 0x7F},
    {kTiParamA114DelayTapeDelayTime,          PAGE_A + 114, 0x7F},
    {kTiParamA115DelayFeedback,               PAGE_A + 115, 0x7F},
    {kTiParamA116DlyRateRevDecay,             PAGE_A + 116, 0x7F},
    {kTiParamA117DlyDepth,                    PAGE_A + 117, 0x7F},
    {kTiParamA118DelayLfoShape,               PAGE_A + 118, 0x7F},
    {kTiParamA119DelayColor,                  PAGE_A + 119, 0x7F},
    {kTiParamB20DelayClock,                   PAGE_B + 20,  0x7F},
    {kTiParamC10DelayType,                    PAGE_C + 10,  0x7F},
    {kTiParamC12DelayTapeDelayRatio,          PAGE_C + 12,  0x7F},
    {kTiParamC13DelayTapeDelayLeftClock,      PAGE_C + 13,  0x7F},
    {kTiParamC14DelayTapeDelayRightClock,     PAGE_C + 14,  0x7F},
    {kTiParamC17DelayTapeDelayBandwidth,      PAGE_C + 17,  0x7F},
    {kTiParamC1ReverbMode,                    PAGE_C + 1,   0x7F},
    {kTiParamC2ReverbSend,                    PAGE_C + 2,   0x7F},
    {kTiParamC3ReverbType,                    PAGE_C + 3,   0x7F},
    {kTiParamC4ReverbTime,                    PAGE_C + 4,   0x7F},
    {kTiParamC5ReverbDamping,                 PAGE_C + 5,   0x7F},
    {kTiParamC6ReverbColor,                   PAGE_C + 6,   0x7F},
    {kTiParamC7ReverbPredelay,                PAGE_C + 7,   0x7F},
    {kTiParamC8ReverbClock,                   PAGE_C + 8,   0x7F},
    {kTiParamC9ReverbFeedback,                PAGE_C + 9,   0x7F},
    {kTiParamC90Assign1Destination2,          PAGE_C + 90,  0x7F},
    {kTiParamC91Assign1Amount2,               PAGE_C + 91,  0x7F},
    {kTiParamC92Assign1Destination3,          PAGE_C + 92,  0x7F},
    {kTiParamC93Assign1Amount3,               PAGE_C + 93,  0x7F},
    {kTiParamB70Assign2Destination2,          PAGE_B + 70,  0x7F},
    {kTiParamB71Assign2Amount2,               PAGE_B + 71,  0x7F},
    {kTiParamC94Assign2Destination3,          PAGE_C + 94,  0x7F},
    {kTiParamC95Assign2Amount3,               PAGE_C + 95,  0x7F},
    {kTiParamB75Assign3Destination2,          PAGE_B + 75,  0x7F},
    {kTiParamB76Assign3Amount2,               PAGE_B + 76,  0x7F},
    {kTiParamB77Assign3Destination3,          PAGE_B + 77,  0x7F},
    {kTiParamB78Assign3Amount3,               PAGE_B + 78,  0x7F},
    {kTiParamC96Assign4Destination2,          PAGE_C + 96,  0x7F},
    {kTiParamC97Assign4Amount2,               PAGE_C + 97,  0x7F},
    {kTiParamC98Assign4Destination3,          PAGE_C + 98,  0x7F},
    {kTiParamC99Assign4Amount3,               PAGE_C + 99,  0x7F},
    {kTiParamC100Assign5Destination2,         PAGE_C + 100, 0x7F},
    {kTiParamC101Assign5Amount2,              PAGE_C + 101, 0x7F},
    {kTiParamC102Assign5Destination3,         PAGE_C + 102, 0x7F},
    {kTiParamC103Assign5Amount3,              PAGE_C + 103, 0x7F},
    {kTiParamC104Assign6Destination2,         PAGE_C + 104, 0x7F},
    {kTiParamC105Assign6Amount2,              PAGE_C + 105, 0x7F},
    {kTiParamC106Assign6Destination3,         PAGE_C + 106, 0x7F},
    {kTiParamC107Assign6Amount3,              PAGE_C + 107, 0x7F},
};

// notes §7: the bytes of a new sound, the instrument's own starting point
static const uint8_t kInitBlock1[256] = {
    12,  0,   0,   0,   0,   0,  0,  0,   0,  0,  64,  0,   0,   0,  0,  0,   0, 64,  0,   0, 64, 96, 64,   0,
    0,  64,  32,   0,   0,  64, 64, 96,   0, 64,   0,  0,  64,   0,  0, 64, 127, 64,  0,   0,  0,  0, 64,  64,
    64,  0,   0,   0,   0,   0,  0, 46,   0, 64, 127,  0, 127, 127, 64,  4,   0,  0,  0,  48,  1,  0,  0,  64,
    0,   0,  64,  64,  64,  64, 64, 48,   1,  0,   0, 64,   0,   0, 64, 64,  64, 64, 64, 100,  0, 64,  0,   0,
    0,   0,   0,   0,   0,   0,  0,  1, 127,  0,  69, 16, 127,  64,  1,  0,   1,  0,  0,   0, 16, 12,  1,  64,
    0,   0,   0,   0,   0,   0,  0,  0,   0,  0,   0,  0,   0,  64,  0, 94,   1,  0,  0,   1,  0,  0,  0,   0,
    57,  4,   0,   0,   0,   0,  0,  0,   0,  1,  66, 62,   1,   0,  1,  1,   1, 36,  0,   0, 64,  0,  0,   0,
    0,   0,  64,  64,  40,  43, 85, 64,  64, 64,  64,  0,   0,   0, 64, 64,  64, 64,  0,   0, 64, 64,  0,   0,
    3,  58, 100,   0,   0,  64,  0, 64,   0,  0,  64,  0,  64,   0, 64,  0,  64,  0, 64,   0,  3,  0, 36, 112,
    48, 64, 127,   0,  64,  71, 51, 64,  64, 40,  32,  0,   0,   0,  0,  3,   0, 64,  3,   0, 64,  3,  0,  64,
    32, 45,  73, 110, 105, 116, 45, 32,  32, 32,   2,  0,   0,   0,  0,  0,
};

static const uint8_t kInitBlock2[256] = {
    0,     1,  0,   2, 68, 20,  64,  1,  0,   0,   0,  0,   3,  4,  2,   0, 0,  0,   0,  0, 127,   0, 64,  0,
    0,    64,  0,   0,  0,  0,   0,  0,  0,   0,   0,  0,   0,  0,  0,   0, 0,  0,  64,  0,   0,   0,  0,  0,
    0,     0,  0,   0,  0,  0,   0,  0,  0,   0,   0,  0,   0,  0, 64,   0, 0,  0,   0,  0,   0,   0,  0,  0,
    127, 127,  0,   0,  0,  0,   0,  0, 20,  70,   0,  0,  70, 20, 70,   0, 0, 70,   0, 64,   0,  64,  0, 64,
    0,    64,  0,  64,  0, 64,   0, 64,  0,  64,   0, 64,   0,  0,  0,   0, 0,  0,   0,  0,   0,   0,  0,  0,
    0,     0,  1,   1,  1,  0,   0, 31, 64, 100,   1, 64, 100,  0, 64, 100, 1, 64, 100,  0,  64, 100,  1, 64,
    100,   0, 64, 100,  1, 64, 100,  0, 64, 100,   1, 64, 100,  0, 64, 100, 1, 64, 100,  0,  64, 100,  1, 64,
    100,   0, 64, 100,  1, 64, 100,  0, 64, 100,   1, 64, 100,  0, 64, 100, 1, 64, 100,  0,  64, 100,  1, 64,
    100,   0, 64, 100,  1, 64, 100,  0, 64, 100,   1, 64, 100,  0, 64, 100, 1, 64, 100,  0,  64, 100,  1, 64,
    100,   0, 64, 100,  1, 64, 100,  0,  0,   0,   0,  0,   0,  0,  0,   0, 0,  0,   0,  0,   0,   0,  0,  0,
    0,     0,  0,   0,  0,  0,   0,  0,  0,  48, 127, 64,   0,  1,  0,   0,
};

static bool is_single(const uint8_t * m, size_t len) {
    return (len == TI_SINGLE_LENGTH) && (memcmp(m, kHeader, sizeof(kHeader)) == 0) && (m[COMMAND_AT] == SINGLE_DUMP)
           && (m[TI_SINGLE_LENGTH - 1] == 0xF7);
}

// notes §1: an older Single - one 256-byte block and its checksum
static bool is_older_single(const uint8_t * m, size_t len) {
    return (len == OLDER_SINGLE_LENGTH) && (memcmp(m, kHeader, sizeof(kHeader)) == 0) && (m[COMMAND_AT] == SINGLE_DUMP)
           && (m[OLDER_SINGLE_LENGTH - 1] == 0xF7);
}

static uint8_t checksum(const uint8_t * m, int from, int to);

static int add_if_single(const uint8_t * m, size_t len, uint8_t(*singles)[TI_SINGLE_LENGTH], int count, int capacity) {
    if ((count < capacity) && is_single(m, len)) {
        memcpy(singles[count++], m, TI_SINGLE_LENGTH);
    } else if ((count < capacity) && is_older_single(m, len)) {
        uint8_t * out = singles[count++];

        memset(out, 0, TI_SINGLE_LENGTH);
        memcpy(out, m, CHECKSUM1_AT + 1);
        out[CHECKSUM2_AT]         = checksum(out, DEVICE_AT, CHECKSUM2_AT);
        out[TI_SINGLE_LENGTH - 1] = 0xF7;
    }
    return count;
}

static int from_syx(const uint8_t * d, size_t n, uint8_t(*singles)[TI_SINGLE_LENGTH], int capacity) {
    int count = 0;

    for (size_t i = 0; i < n; i++) {
        if (d[i] != 0xF0) {
            continue;
        }
        size_t end = i + 1;

        while ((end < n) && (d[end] != 0xF7)) {
            end++;
        }

        if (end >= n) {
            break;
        }
        count = add_if_single(&d[i], end + 1 - i, singles, count, capacity);
        i     = end;
    }

    return count;
}

static bool vlq(const uint8_t * d, size_t n, size_t * i, uint32_t * value) {
    *value = 0;

    for (int k = 0; (k < 4) && (*i < n); k++) {
        uint8_t b = d[(*i)++];

        *value = (*value << 7) | (b & 0x7Fu);

        if ((b & 0x80u) == 0) {
            return true;
        }
    }

    return false;
}

// notes §2: every SysEx event of every track; an F7 event continues the message before it
static int from_mid(const uint8_t * d, size_t n, uint8_t(*singles)[TI_SINGLE_LENGTH], int capacity) {
    static uint8_t pending[TI_SINGLE_LENGTH + 1];
    int            count = 0;
    size_t         i     = 14;

    while ((i + 8 <= n) && (count < capacity)) {
        uint32_t length = ((uint32_t)d[i + 4] << 24) | ((uint32_t)d[i + 5] << 16) | ((uint32_t)d[i + 6] << 8) | d[i + 7];
        size_t   j      = i + 8;
        size_t   end    = (j + length <= n) ? (j + length) : n;
        size_t   have   = 0;
        bool     open   = false;
        uint8_t  status = 0;

        if (memcmp(&d[i], "MTrk", 4) != 0) {
            i = end;
            continue;
        }

        while (j < end) {
            uint32_t delta, size;

            if (!vlq(d, end, &j, &delta) || (j >= end)) {
                break;
            }
            uint8_t  b = d[j];

            if (b == 0xFF) {
                j += 2;

                if (!vlq(d, end, &j, &size)) {
                    break;
                }
                j += size;
            } else if ((b == 0xF0) || (b == 0xF7)) {
                j++;

                if (!vlq(d, end, &j, &size) || (j + size > end)) {
                    break;
                }

                if (b == 0xF0) {
                    pending[0] = 0xF0;
                    have       = 1;
                    open       = true;
                }

                for (uint32_t k = 0; open && (k < size); k++) {
                    if (have >= sizeof(pending)) {
                        open = false;   // too long for a Single
                    } else {
                        pending[have++] = d[j + k];
                    }
                }

                j += size;

                if (open && (have > 0) && (pending[have - 1] == 0xF7)) {
                    count = add_if_single(pending, have, singles, count, capacity);
                    open  = false;
                }
            } else {
                if ((b & 0x80u) != 0) {
                    status = b;
                    j++;
                }
                j += (((status & 0xF0u) == 0xC0u) || ((status & 0xF0u) == 0xD0u)) ? 1 : 2;
            }
        }
        i = end;
    }
    return count;
}

int ti_patch_read_file(const char * path, uint8_t(*singles)[TI_SINGLE_LENGTH], int capacity) {
    FILE *    f = fopen(path, "rb");
    uint8_t * d;
    long      n;
    int       count;

    if (f == NULL) {
        return 0;
    }

    if ((fseek(f, 0, SEEK_END) != 0) || ((n = ftell(f)) <= 0) || (n > MAX_FILE_BYTES) || (fseek(f, 0, SEEK_SET) != 0)) {
        fclose(f);
        return 0;
    }
    d     = (uint8_t *)malloc((size_t)n);

    if ((d == NULL) || (fread(d, 1, (size_t)n, f) != (size_t)n)) {
        free(d);
        fclose(f);
        return 0;
    }
    fclose(f);
    count = ((n >= 14) && (memcmp(d, "MThd", 4) == 0)) ? from_mid(d, (size_t)n, singles, capacity) : from_syx(d, (size_t)n, singles, capacity);
    free(d);
    return count;
}

// notes §3a: a Single older than the TI format keeps a few settings where its format had them
#define TI_FORMAT    (12)

static uint16_t carried_at(size_t k, const uint8_t single[TI_SINGLE_LENGTH]) {
    uint16_t at = kCarried[k].at;

    if (single[PAGE_A] >= TI_FORMAT) {
        return at;
    }

    if (at == (PAGE_B + 15)) {
        return PAGE_B + 1;                      // Arp Mode
    }

    if ((at >= (PAGE_D + 120)) && (at <= (PAGE_D + 123))) {
        return (uint16_t)(PAGE_A + 97 + (at - (PAGE_D + 120)));   // Unison voices, detune, spread, phase
    }
    return (at == (PAGE_A + 50)) ? (PAGE_A + 38) : at;   // Ring
}

void ti_patch_from_single(const uint8_t single[TI_SINGLE_LENGTH], double normalized[kTiParamCount]) {
    for (size_t k = 0; k < sizeof(kCarried) / sizeof(kCarried[0]); k++) {
        int32_t steps = gTiParams[kCarried[k].paramId].stepCount;
        int32_t value = single[carried_at(k, single)] & kCarried[k].mask;

        value                           = (value > steps) ? steps : value;
        normalized[kCarried[k].paramId] = (double)value / steps;
    }
}

void ti_patch_name(const uint8_t single[TI_SINGLE_LENGTH], char name[TI_SINGLE_NAME_LENGTH + 1]) {
    int n = TI_SINGLE_NAME_LENGTH;

    for (int i = 0; i < TI_SINGLE_NAME_LENGTH; i++) {
        uint8_t c = single[NAME_AT + i];

        name[i] = ((c >= 32) && (c < 127)) ? (char)c : ' ';
    }

    while ((n > 0) && (name[n - 1] == ' ')) {
        n--;
    }
    name[n] = '\0';
}

static uint8_t checksum(const uint8_t * m, int from, int to) {
    unsigned sum = 0;

    for (int i = from; i < to; i++) {
        sum += m[i];
    }

    return (uint8_t)(sum & 0x7Fu);
}

void ti_patch_to_single(const double normalized[kTiParamCount], const uint8_t * base, const char * name, uint8_t out[TI_SINGLE_LENGTH]) {
    size_t len                                = (name != NULL) ? strlen(name) : 0;
    double was[kTiParamCount];
    char   wasName[TI_SINGLE_NAME_LENGTH + 1] = "";

    if (base != NULL) {
        ti_patch_from_single(base, was);
        ti_patch_name(base, wasName);
    }

    if (base != NULL) {
        memcpy(out, base, TI_SINGLE_LENGTH);
    } else {
        memset(out, 0, TI_SINGLE_LENGTH);
        memcpy(out + DATA_AT, kInitBlock1, sizeof(kInitBlock1));
        memcpy(out + CHECKSUM1_AT + 1, kInitBlock2, sizeof(kInitBlock2));
    }
    memcpy(out, kHeader, sizeof(kHeader));
    out[DEVICE_AT]            = OMNI_DEVICE;
    out[COMMAND_AT]           = SINGLE_DUMP;
    out[BANK_AT]              = EDIT_BUFFER_BANK;
    out[PROGRAM_AT]           = EDIT_BUFFER_PROG;
    out[TI_SINGLE_LENGTH - 1] = 0xF7;

    for (size_t k = 0; k < sizeof(kCarried) / sizeof(kCarried[0]); k++) {
        int32_t  steps = gTiParams[kCarried[k].paramId].stepCount;
        long     value = lround(fmin(1.0, fmax(0.0, normalized[kCarried[k].paramId])) * steps);

        // notes §4: a byte is kept as it came unless its parameter changed - it may hold more than the panel shows
        if ((base != NULL) && (value == lround(was[kCarried[k].paramId] * steps))) {
            continue;
        }
        uint16_t at    = carried_at(k, out);

        out[at] = (uint8_t)((out[at] & ~kCarried[k].mask) | ((uint8_t)value & kCarried[k].mask));
    }

    for (size_t i = 0; (base == NULL || strcmp(wasName, (name != NULL) ? name : "") != 0) && (i < TI_SINGLE_NAME_LENGTH); i++) {
        char c = (i < len) ? name[i] : ' ';

        out[NAME_AT + i] = ((c >= 32) && (c < 127)) ? (uint8_t)c : ' ';
    }

    ti_patch_address(out, 0);
}

void ti_patch_address(uint8_t single[TI_SINGLE_LENGTH], int number) {
    single[BANK_AT]      = (number > 0) ? (uint8_t)(1 + ((number - 1) / TI_BANK_SIZE)) : EDIT_BUFFER_BANK;
    single[PROGRAM_AT]   = (number > 0) ? (uint8_t)((number - 1) % TI_BANK_SIZE) : EDIT_BUFFER_PROG;
    single[CHECKSUM1_AT] = checksum(single, DEVICE_AT, CHECKSUM1_AT);
    single[CHECKSUM2_AT] = checksum(single, DEVICE_AT, CHECKSUM2_AT);
}

void ti_patch_init_single(uint8_t out[TI_SINGLE_LENGTH]) {
    double values[kTiParamCount];

    for (uint32_t id = 0; id < kTiParamCount; id++) {
        values[id] = gTiParams[id].defaultNormalized;
    }

    ti_patch_to_single(values, NULL, "Init", out);
}

void ti_patch_sound(const uint8_t single[TI_SINGLE_LENGTH], tTiSound * sound) {
    memcpy(sound->a, single + PAGE_A, TI_PAGE);
    memcpy(sound->b, single + PAGE_B, TI_PAGE);
    memcpy(sound->c, single + CHECKSUM1_AT + 1, TI_PAGE);
    memcpy(sound->d, single + CHECKSUM1_AT + 1 + TI_PAGE, TI_PAGE);
}

bool ti_patch_save_file(const char * path, const uint8_t single[TI_SINGLE_LENGTH]) {
    FILE * f = fopen(path, "wb");
    bool   ok;

    if (f == NULL) {
        return false;
    }
    ok = fwrite(single, 1, TI_SINGLE_LENGTH, f) == TI_SINGLE_LENGTH;
    return (fclose(f) == 0) && ok;
}

const char * ti_patch_sounds_folder(void) {
    static char folder[1024];

    snprintf(folder, sizeof(folder), "%s/%s", getenv("HOME") ? getenv("HOME") : "", SOUNDS_FOLDER);
    return folder;
}

static bool patch_file_name(const char * name) {
    size_t n = strlen(name);

    return (name[0] != '.') && (n > 4) && ((strcasecmp(name + n - 4, ".syx") == 0) || (strcasecmp(name + n - 4, ".mid") == 0));
}

static int by_name(const void * a, const void * b) {
    return strcasecmp(*(const char *const *)a, *(const char *const *)b);
}

// ---- the presets - notes §5 ---------------------------------------------------------------------------------

static uint8_t gPresets[TI_PRESETS][TI_SINGLE_LENGTH];
static char    gPresetNames[TI_PRESETS][TI_SINGLE_NAME_LENGTH + 1];
static bool    gPresetsRead;
static time_t  gPresetsModified;

void ti_preset_label(int number, char * out, size_t len) {
    snprintf(out, len, "%c%03d", 'A' + ((number - 1) / TI_BANK_SIZE), ((number - 1) % TI_BANK_SIZE) + 1);
}

const char * ti_presets_path(void) {
    static char path[1024];

    snprintf(path, sizeof(path), "%s/%s/%s", getenv("HOME") ? getenv("HOME") : "", PRESETS_FOLDER, PRESETS_FILE);
    return path;
}

// slots past what was found are Init, and each is addressed to its own place
static void fill_and_address(uint8_t singles[TI_PRESETS][TI_SINGLE_LENGTH], int found) {
    for (int i = 0; i < TI_PRESETS; i++) {
        if (i >= found) {
            ti_patch_init_single(singles[i]);
        }
        ti_patch_address(singles[i], i + 1);
    }
}

static void read_presets(void) {
    static uint8_t found[TI_PRESETS][TI_SINGLE_LENGTH];
    int            count = ti_patch_read_file(ti_presets_path(), found, TI_PRESETS);

    fill_and_address(found, count);
    memcpy(gPresets, found, sizeof(gPresets));

    for (int i = 0; i < TI_PRESETS; i++) {
        ti_patch_name(gPresets[i], gPresetNames[i]);
    }
}

static time_t presets_modified(void) {
    struct stat st;

    return (stat(ti_presets_path(), &st) == 0) ? st.st_mtime : 0;
}

bool ti_presets_refresh(void) {
    time_t modified = presets_modified();

    if (gPresetsRead && (modified == gPresetsModified)) {
        return false;
    }
    gPresetsRead     = true;
    gPresetsModified = modified;
    read_presets();
    return true;
}

const uint8_t * ti_presets_single(int number) {
    return gPresets[((number >= 1) && (number <= TI_PRESETS)) ? (number - 1) : 0];
}

const char * ti_presets_name(int number) {
    return ((number >= 1) && (number <= TI_PRESETS)) ? gPresetNames[number - 1] : "";
}

static bool write_all(const char * path, const uint8_t singles[TI_PRESETS][TI_SINGLE_LENGTH]) {
    FILE * f = fopen(path, "wb");
    bool   ok;

    if (f == NULL) {
        return false;
    }
    ok = fwrite(singles, TI_SINGLE_LENGTH, TI_PRESETS, f) == TI_PRESETS;
    return (fclose(f) == 0) && ok;
}

static bool write_presets(void) {
    char folder[1024];
    bool ok;

    snprintf(folder, sizeof(folder), "%s/%s", getenv("HOME") ? getenv("HOME") : "", PRESETS_FOLDER);

    for (char * p = folder + 1; *p != '\0'; p++) {
        if (*p == '/') {
            *p = '\0';
            mkdir(folder, 0755);
            *p = '/';
        }
    }

    mkdir(folder, 0755);
    ok               = write_all(ti_presets_path(), (const uint8_t(*)[TI_SINGLE_LENGTH]) gPresets);
    gPresetsModified = presets_modified();

    for (int i = 0; i < TI_PRESETS; i++) {
        ti_patch_name(gPresets[i], gPresetNames[i]);
    }

    return ok;
}

bool ti_presets_store(int number, const uint8_t single[TI_SINGLE_LENGTH]) {
    if ((number < 1) || (number > TI_PRESETS)) {
        return false;
    }
    (void)ti_presets_refresh();   // another instance's store is kept
    memcpy(gPresets[number - 1], single, TI_SINGLE_LENGTH);
    ti_patch_address(gPresets[number - 1], number);
    return write_presets();
}

bool ti_presets_store_all(const uint8_t singles[TI_PRESETS][TI_SINGLE_LENGTH]) {
    memcpy(gPresets, singles, sizeof(gPresets));
    fill_and_address(gPresets, TI_PRESETS);
    gPresetsRead = true;
    return write_presets();
}

// notes §6: in file order from A001; what is left over is Init
int ti_presets_read_file(const char * path, uint8_t singles[TI_PRESETS][TI_SINGLE_LENGTH]) {
    int count = ti_patch_read_file(path, singles, TI_PRESETS);

    fill_and_address(singles, count);
    return count;
}

// notes §6: every .syx and .mid in the folder, in name order, one after the other
int ti_presets_read_folder(const char * folder, uint8_t singles[TI_PRESETS][TI_SINGLE_LENGTH]) {
    const char *    names[MAX_FOLDER_FILES];
    int             files = 0, count = 0;
    DIR *           dir = opendir(folder);
    struct dirent * e;
    char            path[2048];

    if (dir == NULL) {
        return 0;
    }

    while (((e = readdir(dir)) != NULL) && (files < MAX_FOLDER_FILES)) {
        if (patch_file_name(e->d_name)) {
            names[files++] = strdup(e->d_name);
        }
    }
    closedir(dir);
    qsort(names, (size_t)files, sizeof(names[0]), by_name);

    for (int i = 0; i < files; i++) {
        if ((names[i] != NULL) && (count < TI_PRESETS)) {
            snprintf(path, sizeof(path), "%s/%s", folder, names[i]);
            count += ti_patch_read_file(path, &singles[count], TI_PRESETS - count);
        }
        free((void *)names[i]);
    }

    fill_and_address(singles, count);
    return count;
}

bool ti_presets_write_file(const char * path) {
    (void)ti_presets_refresh();
    return write_all(path, (const uint8_t(*)[TI_SINGLE_LENGTH]) gPresets);
}
