/*
 * Sn2Alike - the words the host side writes for a voice, from the program, the key and the velocity (generated).
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
// Notes: Docs/code-notes/sn2Filter.md - "// notes §k" refers there.


// Notes: Docs/code-notes/sn2Fixed.md - "// notes §k" refers there. Generated from measurements: do not edit by hand.

#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

#include "sn2Filter.h"
#include "sn2Fixed.h"
#include "sn2Osc.h"

// notes §8: where each field the words read sits in a single: start of its dump, offset, mask, shift
typedef struct {
    uint16_t dump, offset;
    uint8_t  mask, shift;
} tField;

static const tField   kFields[] = {
    {  0, 237, 0x01, 3},   // ConstantGate
    {  0, 194, 0xff, 0},   // Env1Attack
    {  0, 195, 0xff, 0},   // Env1Decay
    {  0, 197, 0xff, 0},   // Env1Release
    {  0, 196, 0xff, 0},   // Env1Sustain
    {297,  19, 0xff, 0},   // Env1SustainRate
    {297,  16, 0xff, 0},   // Env1SustainTime
    {  0, 198, 0xff, 0},   // Env1Velocity
    {  0, 188, 0xff, 0},   // Env2Attack
    {  0, 189, 0xff, 0},   // Env2Decay
    {  0, 193, 0xff, 0},   // Env2Delay
    {  0, 191, 0xff, 0},   // Env2Release
    {  0, 190, 0xff, 0},   // Env2Sustain
    {297,  20, 0xff, 0},   // Env2SustainRate
    {297,  17, 0xff, 0},   // Env2SustainTime
    {  0, 192, 0xff, 0},   // Env2Velocity
    {  0, 182, 0xff, 0},   // Env3Attack
    {  0, 183, 0xff, 0},   // Env3Decay
    {  0, 187, 0xff, 0},   // Env3Delay
    {  0, 185, 0xff, 0},   // Env3Release
    {  0, 184, 0xff, 0},   // Env3Sustain
    {297,  21, 0xff, 0},   // Env3SustainRate
    {297,  18, 0xff, 0},   // Env3SustainTime
    {  0, 186, 0xff, 0},   // Env3Velocity
    {  0, 213, 0xff, 0},   // FilterAftertouchFrequency
    {  0, 223, 0xff, 0},   // FilterAftertouchResonance
    {  0, 211, 0xff, 0},   // FilterCutoffFreq
    {  0, 214, 0xff, 0},   // FilterFreqEnv2
    {  0, 215, 0xff, 0},   // FilterFreqEnv3
    {  0, 216, 0xff, 0},   // FilterFreqLFO1
    {  0, 217, 0xff, 0},   // FilterFreqLFO2
    {  0, 212, 0xff, 0},   // FilterModKnobWhFrequency
    {  0, 222, 0xff, 0},   // FilterModKnobWhResonance
    {  0, 210, 0xff, 0},   // FilterOverdrive
    {  0, 230, 0xff, 0},   // FilterQNormalise
    {  0, 224, 0xff, 0},   // FilterResEnv2
    {  0, 225, 0xff, 0},   // FilterResEnv3
    {  0, 226, 0xff, 0},   // FilterResLFO1
    {  0, 227, 0xff, 0},   // FilterResLFO2
    {  0, 221, 0xff, 0},   // FilterResonance
    {  0, 208, 0xff, 0},   // FilterSlope
    {  0, 220, 0xff, 0},   // FilterTracking
    {  0, 207, 0xff, 0},   // FilterType
    {  0, 160, 0xff, 0},   // LFO1Delay
    {297,   3, 0x01, 3},   // LFO1FadeMode
    {  0, 162, 0xff, 0},   // LFO1Offset
    {297,   0, 0xff, 0},   // LFO1SoftenAmount
    {  0, 163, 0xff, 0},   // LFO1Speed
    {  0, 166, 0xff, 0},   // LFO1SpeedAftertouchDepth
    {  0, 164, 0xff, 0},   // LFO1SpeedEnv3
    {  0, 165, 0xff, 0},   // LFO1SpeedWhDepth
    {  0, 167, 0xff, 0},   // LFO1Sync
    {  0, 161, 0xff, 0},   // LFO1Type
    {  0, 170, 0xff, 0},   // LFO2Delay
    {297,   3, 0x01, 4},   // LFO2FadeMode
    {  0, 172, 0xff, 0},   // LFO2Offset
    {297,   1, 0xff, 0},   // LFO2SoftenAmount
    {  0, 173, 0xff, 0},   // LFO2Speed
    {  0, 176, 0xff, 0},   // LFO2SpeedAftertouchDepth
    {  0, 174, 0xff, 0},   // LFO2SpeedEnv3
    {  0, 175, 0xff, 0},   // LFO2SpeedWhDepth
    {  0, 177, 0xff, 0},   // LFO2Sync
    {  0, 171, 0xff, 0},   // LFO2Type
    {  0, 144, 0xff, 0},   // ModKnobEnv2Mix1x3
    {  0, 150, 0xff, 0},   // ModKnobEnv2Mix2x3
    {  0, 145, 0xff, 0},   // ModKnobEnv3Mix1x3
    {  0, 151, 0xff, 0},   // ModKnobEnv3Mix2x3
    {  0, 146, 0xff, 0},   // ModKnobLFO1Mix1x3
    {  0, 152, 0xff, 0},   // ModKnobLFO1Mix2x3
    {  0, 147, 0xff, 0},   // ModKnobLFO2Mix1x3
    {  0, 153, 0xff, 0},   // ModKnobLFO2Mix2x3
    {  0, 143, 0xff, 0},   // ModKnobWhMix1x3
    {  0, 149, 0xff, 0},   // ModKnobWhMix2x3
    {  0, 156, 0xff, 0},   // NoiseMixEnv2
    {  0, 154, 0xff, 0},   // NoiseMixLevel
    {  0, 203, 0xff, 0},   // NoiseModKnobEnv2Hardness
    {  0, 204, 0xff, 0},   // NoiseModKnobEnv3Hardness
    {  0, 157, 0xff, 0},   // NoiseModKnobEnv3Mix
    {  0, 205, 0xff, 0},   // NoiseModKnobLFO1Hardness
    {  0, 158, 0xff, 0},   // NoiseModKnobLFO1Mix
    {  0, 206, 0xff, 0},   // NoiseModKnobLFO2Hardness
    {  0, 159, 0xff, 0},   // NoiseModKnobLFO2Mix
    {  0, 202, 0xff, 0},   // NoiseModKnobWhHardness
    {  0, 155, 0xff, 0},   // NoiseModKnobWhMix
    {  0, 201, 0xff, 0},   // NoiseSoften
    {  0,  25, 0xff, 0},   // Osc1FineTune
    {  0,  53, 0xff, 0},   // Osc1KeySync
    {  0,  27, 0xff, 0},   // Osc1ManKnobPitch
    {  0, 126, 0xff, 0},   // Osc1MixEnv2
    {  0, 124, 0xff, 0},   // Osc1MixLevel
    {  0,  38, 0xff, 0},   // Osc1ModKnobEnv3Hardness
    {  0, 127, 0xff, 0},   // Osc1ModKnobEnv3Mix
    {  0,  50, 0xff, 0},   // Osc1ModKnobEnv3Sync
    {  0,  39, 0xff, 0},   // Osc1ModKnobLFO1Hardness
    {  0, 128, 0xff, 0},   // Osc1ModKnobLFO1Mix
    {  0,  40, 0xff, 0},   // Osc1ModKnobLFO2Hardness
    {  0, 129, 0xff, 0},   // Osc1ModKnobLFO2Mix
    {  0,  32, 0xff, 0},   // Osc1ModKnobLFO2Pitch
    {  0,  52, 0xff, 0},   // Osc1ModKnobLFO2Sync
    {  0,  36, 0xff, 0},   // Osc1ModKnobWhHardness
    {  0, 125, 0xff, 0},   // Osc1ModKnobWhMix
    {  0,  28, 0xff, 0},   // Osc1ModKnobWhPitch
    {  0,  48, 0xff, 0},   // Osc1ModKnobWhSync
    {  0,  42, 0xff, 0},   // Osc1ModKnobWhWidth
    {  0,  23, 0xff, 0},   // Osc1Octave
    {  0,  29, 0xff, 0},   // Osc1PitchEnv2
    {  0,  30, 0xff, 0},   // Osc1PitchEnv3
    {  0,  31, 0xff, 0},   // Osc1PitchLFO1
    {  0,  41, 0xff, 0},   // Osc1PulseWidth
    {  0,  24, 0xff, 0},   // Osc1Semitone
    {  0,  35, 0xff, 0},   // Osc1Soften
    {  0,  37, 0xff, 0},   // Osc1SoftenEnv2
    {  0,  47, 0xff, 0},   // Osc1Sync
    {  0,  49, 0xff, 0},   // Osc1SyncEnv2
    {  0,  55, 0xff, 0},   // Osc1SyncFormantWidth
    {  0,  51, 0xff, 0},   // Osc1SyncLFO1
    {  0,  54, 0xff, 0},   // Osc1SyncSkew
    {  0,  22, 0xff, 0},   // Osc1Type
    {  0,  59, 0xff, 0},   // Osc2FineTune
    {  0,  87, 0xff, 0},   // Osc2KeySync
    {  0,  61, 0xff, 0},   // Osc2ManKnobPitch
    {  0, 132, 0xff, 0},   // Osc2MixEnv2
    {  0, 130, 0xff, 0},   // Osc2MixLevel
    {  0,  72, 0xff, 0},   // Osc2ModKnobEnv3Hardness
    {  0, 133, 0xff, 0},   // Osc2ModKnobEnv3Mix
    {  0,  84, 0xff, 0},   // Osc2ModKnobEnv3Sync
    {  0,  73, 0xff, 0},   // Osc2ModKnobLFO1Hardness
    {  0, 134, 0xff, 0},   // Osc2ModKnobLFO1Mix
    {  0,  74, 0xff, 0},   // Osc2ModKnobLFO2Hardness
    {  0, 135, 0xff, 0},   // Osc2ModKnobLFO2Mix
    {  0,  66, 0xff, 0},   // Osc2ModKnobLFO2Pitch
    {  0,  86, 0xff, 0},   // Osc2ModKnobLFO2Sync
    {  0,  70, 0xff, 0},   // Osc2ModKnobWhHardness
    {  0, 131, 0xff, 0},   // Osc2ModKnobWhMix
    {  0,  62, 0xff, 0},   // Osc2ModKnobWhPitch
    {  0,  82, 0xff, 0},   // Osc2ModKnobWhSync
    {  0,  57, 0xff, 0},   // Osc2Octave
    {  0,  63, 0xff, 0},   // Osc2PitchEnv2
    {  0,  64, 0xff, 0},   // Osc2PitchEnv3
    {  0,  65, 0xff, 0},   // Osc2PitchLFO1
    {  0,  75, 0xff, 0},   // Osc2PulseWidth
    {  0,  58, 0xff, 0},   // Osc2Semitone
    {  0,  69, 0xff, 0},   // Osc2Soften
    {  0,  71, 0xff, 0},   // Osc2SoftenEnv2
    {  0,  81, 0xff, 0},   // Osc2Sync
    {  0,  83, 0xff, 0},   // Osc2SyncEnv2
    {  0,  89, 0xff, 0},   // Osc2SyncFormantWidth
    {  0,  85, 0xff, 0},   // Osc2SyncLFO1
    {  0,  88, 0xff, 0},   // Osc2SyncSkew
    {  0,  56, 0xff, 0},   // Osc2Type
    {  0,  93, 0xff, 0},   // Osc3FineTune
    {  0, 121, 0xff, 0},   // Osc3KeySync
    {  0,  95, 0xff, 0},   // Osc3ManKnobPitch
    {  0, 138, 0xff, 0},   // Osc3MixEnv2
    {  0, 136, 0xff, 0},   // Osc3MixLevel
    {  0, 106, 0xff, 0},   // Osc3ModKnobEnv3Hardness
    {  0, 139, 0xff, 0},   // Osc3ModKnobEnv3Mix
    {  0, 118, 0xff, 0},   // Osc3ModKnobEnv3Sync
    {  0, 107, 0xff, 0},   // Osc3ModKnobLFO1Hardness
    {  0, 140, 0xff, 0},   // Osc3ModKnobLFO1Mix
    {  0, 108, 0xff, 0},   // Osc3ModKnobLFO2Hardness
    {  0, 141, 0xff, 0},   // Osc3ModKnobLFO2Mix
    {  0, 100, 0xff, 0},   // Osc3ModKnobLFO2Pitch
    {  0, 120, 0xff, 0},   // Osc3ModKnobLFO2Sync
    {  0, 104, 0xff, 0},   // Osc3ModKnobWhHardness
    {  0, 137, 0xff, 0},   // Osc3ModKnobWhMix
    {  0,  96, 0xff, 0},   // Osc3ModKnobWhPitch
    {  0, 116, 0xff, 0},   // Osc3ModKnobWhSync
    {  0,  91, 0xff, 0},   // Osc3Octave
    {  0,  97, 0xff, 0},   // Osc3PitchEnv2
    {  0,  98, 0xff, 0},   // Osc3PitchEnv3
    {  0,  99, 0xff, 0},   // Osc3PitchLFO1
    {  0, 109, 0xff, 0},   // Osc3PulseWidth
    {  0,  92, 0xff, 0},   // Osc3Semitone
    {  0, 103, 0xff, 0},   // Osc3Soften
    {  0, 105, 0xff, 0},   // Osc3SoftenEnv2
    {  0, 115, 0xff, 0},   // Osc3Sync
    {  0, 117, 0xff, 0},   // Osc3SyncEnv2
    {  0, 123, 0xff, 0},   // Osc3SyncFormantWidth
    {  0, 119, 0xff, 0},   // Osc3SyncLFO1
    {  0, 122, 0xff, 0},   // Osc3SyncSkew
    {  0,  90, 0xff, 0},   // Osc3Type
    {  0, 236, 0xff, 0},   // PartProgramVolume
    {  0, 142, 0xff, 0},   // RingMod1x3MixLevel
    {  0, 148, 0xff, 0},   // RingMod2x3MixLevel
    {297,   7, 0xff, 0},   // SpecialFilterWidth
};

#define FIELD_WIDTH    (185)
#define FIELD_GATE     (0)
static const int      kVelocityField[3]      = {7, 15, 23}; // each envelope's velocity depth
#define FIELD_TYPE     (42)
#define FIELD_SLOPE    (40)
static const int      kOscField[3][5]        = { // octave, semitone, fine, pulse width, type
    {104, 109,  85, 108, 117},
    {136, 141, 118, 140, 149},
    {168, 173, 150, 172, 181},
};
static const uint32_t kOscType[12]           = {0x18, 4, 5, 6, 0xC, 4, 4, 4, 4, 4, 4, 4}; // notes §8: the oscillator's mode, by type

// notes §8: every word as it stands when nothing moves it
static const uint32_t kBase[SN2_FIXED_BLOCK] = {
    0x000000, 0x4e397c, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x7ffffc,
    0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000,
    0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000,
    0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000004, 0x000000, 0x7f0000,
    0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x4e397c, 0x000000, 0x000000,
    0x000000, 0x000000, 0x000000, 0x7ffffc, 0x000000, 0x000000, 0x000000, 0x000000,
    0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000,
    0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000,
    0x000000, 0x000004, 0x000000, 0x640000, 0x000000, 0x000000, 0x000000, 0x000000,
    0x000000, 0x4e397c, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x7ffffc,
    0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000,
    0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000,
    0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000004, 0x000000, 0x000000,
    0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x7f0000, 0x000000,
    0x000000, 0x000000, 0x000000, 0x800000, 0x000000, 0x000000, 0x000000, 0x000000,
    0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000,
    0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x3e19ac, 0x000000,
    0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x438600, 0x000000,
    0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x400000, 0x000000, 0x000000,
    0x1fbd00, 0x7fff00, 0x000001, 0x000000, 0x000000, 0x400000, 0x000000, 0x000000,
    0x000000, 0x400000, 0x000000, 0x100000, 0x000000, 0x7f0000, 0x077000, 0x14b800,
    0x0bac00, 0x7e0200, 0x000000, 0x000000, 0x000000, 0x2b1800, 0x7f0000, 0x364000,
    0x14b800, 0x14b800, 0x7f0000, 0x000000, 0x000000, 0x000000, 0x2b1800, 0x7f0000,
    0x364000, 0x14b800, 0x14b800, 0x7f0000, 0x000000, 0x000000, 0x326340, 0x000000,
    0x158c00, 0x000000, 0x158c00, 0x7fffff, 0xffffff, 0x7fffff, 0xffffff, 0x000000,
    0x000000, 0x000000, 0x000000, 0x000000, 0x000004, 0x3fff00, 0x800000, 0x000000,
    0x000000, 0x000000, 0x000000, 0x000000, 0x000004, 0x3fff00, 0x800000, 0x000000,
    0x000000, 0xffffff, 0x000000, 0x036b00, 0x6d6000, 0xc40000,
};

// notes §8: a word that follows its field in a straight line: base + step x value
typedef struct {
    uint8_t word, field;
    int32_t base, step;
} tLinear;

static const tLinear  kLinear[] = {
    {0x01, 109,  2636137,  43691},
    {0x06, 101, -2796160,  43690},
    {0x07,  87,        0,  66052},
    {0x09, 116, -1048576,  16384},
    {0x0a, 112,        0,  43691},
    {0x0b, 113, -5592320,  87380},
    {0x0c,  92, -5592320,  87380},
    {0x0d, 115, -5592320,  87380},
    {0x0e,  98, -5592320,  87380},
    {0x0f, 102, -5592320,  87380},
    {0x10,  86,        0,  65536},
    {0x11, 111, -8388608, 131072},
    {0x12, 110,  8388096, -66048},
    {0x13,  90, -8388608, 131072},
    {0x14,  93, -8388608, 131072},
    {0x15,  95, -8388608, 131072},
    {0x16,  99, -8388608, 131072},
    {0x1c, 103, -1048576,  16384},
    {0x1e, 114,   260096,  -2048},
    {0x1f,  89,        0,  65536},
    {0x20,  88, -8388608, 131072},
    {0x21,  91, -8388608, 131072},
    {0x22,  94, -8388608, 131072},
    {0x23,  96, -8388608, 131072},
    {0x24, 100, -8388608, 131072},
    {0x25, 141,  1543862,  43691},
    {0x2a, 134, -2796160,  43690},
    {0x2b, 120,        0,  66052},
    {0x2d, 148, -1048576,  16384},
    {0x2e, 144,        0,  43691},
    {0x2f, 145, -5592320,  87380},
    {0x30, 125, -5592320,  87380},
    {0x31, 147, -5592320,  87380},
    {0x32, 131, -5592320,  87380},
    {0x33, 135, -5592320,  87380},
    {0x34, 119,        0,  65536},
    {0x35, 143, -8388608, 131072},
    {0x36, 142,  8388096, -66048},
    {0x37, 123, -8388608, 131072},
    {0x38, 126, -8388608, 131072},
    {0x39, 128, -8388608, 131072},
    {0x3a, 132, -8388608, 131072},
    {0x42, 146,   260096,  -2048},
    {0x43, 122,        0,  65536},
    {0x44, 121, -8388608, 131072},
    {0x45, 124, -8388608, 131072},
    {0x46, 127, -8388608, 131072},
    {0x47, 129, -8388608, 131072},
    {0x48, 133, -8388608, 131072},
    {0x49, 173,   451587,  43691},
    {0x4e, 166, -2796160,  43690},
    {0x4f, 152,        0,  66052},
    {0x51, 180, -1048576,  16384},
    {0x52, 176,        0,  43691},
    {0x53, 177, -5592320,  87380},
    {0x54, 157, -5592320,  87380},
    {0x55, 179, -5592320,  87380},
    {0x56, 163, -5592320,  87380},
    {0x57, 167, -5592320,  87380},
    {0x58, 151,        0,  65536},
    {0x59, 175, -8388608, 131072},
    {0x5a, 174,  8388096, -66048},
    {0x5b, 155, -8388608, 131072},
    {0x5c, 158, -8388608, 131072},
    {0x5d, 160, -8388608, 131072},
    {0x5e, 164, -8388608, 131072},
    {0x66, 178,   260096,  -2048},
    {0x67, 154,        0,  65536},
    {0x68, 153, -8388608, 131072},
    {0x69, 156, -8388608, 131072},
    {0x6a, 159, -8388608, 131072},
    {0x6b, 161, -8388608, 131072},
    {0x6c, 165, -8388608, 131072},
    {0x6e,  84,        0,  65536},
    {0x6f,  75, -8388608, 131072},
    {0x70,  76, -8388608, 131072},
    {0x71,  78, -8388608, 131072},
    {0x72,  80, -8388608, 131072},
    {0x73,  82, -8388608, 131072},
    {0x74,  74,        0,  65536},
    {0x75,  73, -8388608, 131072},
    {0x76,  77, -8388608, 131072},
    {0x77,  79, -8388608, 131072},
    {0x78,  81, -8388608, 131072},
    {0x79,  83, -8388608, 131072},
    {0x7a, 183,        0,  65536},
    {0x7b,  63, -8388608, 131072},
    {0x7c,  65, -8388608, 131072},
    {0x7d,  67, -8388608, 131072},
    {0x7e,  69, -8388608, 131072},
    {0x7f,  71, -8388608, 131072},
    {0x80, 184,        0,  65536},
    {0x81,  64, -8388608, 131072},
    {0x82,  66, -8388608, 131072},
    {0x83,  68, -8388608, 131072},
    {0x84,  70, -8388608, 131072},
    {0x85,  72, -8388608, 131072},
    {0x86,  26,  1011434,  43691},
    {0x87,  27, -6291456,  98304},
    {0x88,  28, -6291456,  98304},
    {0x8b,  31, -6291456,  98304},
    {0x8c,  24, -6291456,  98304},
    {0x8d,  41,        0,  66052},
    {0x8e,  39,  8388096, -66048},
    {0x8f,  35, -8388608, 131072},
    {0x90,  36, -8388608, 131072},
    {0x91,  37, -8388608, 131072},
    {0x92,  38, -8388608, 131072},
    {0x93,  32, -8388608, 131072},
    {0x94,  25, -8388608, 131072},
    {0x96,  34,        0,  65536},
    {0xb2,  12,        0,  65536},
    {0xbb,  20,        0,  65536},
    {0xbe, 182,        0,  52416},
    {0xc7,  47,        0,   2966},
    {0xc8,  49,  -379648,   5932},
    {0xc9,  50,  -379648,   5932},
    {0xca,  48,  -379648,   5932},
    {0xcd,  45,  4194048, -33024},
    {0xcf,  57,        0,   2966},
    {0xd0,  59,  -379648,   5932},
    {0xd1,  60,  -379648,   5932},
    {0xd2,  58,  -379648,   5932},
    {0xd5,  55,  4194048, -33024},
};

// notes §8: a word the host side looks up in a table of its own, 128 steps, measured
// notes §8: the host side's time laws: an octave cut into 1024 steps, its value read straight between octaves
typedef struct {
    uint8_t from, to;
    int8_t  sign;
    int32_t e;        // the exponent at from
    uint8_t nruns;
    struct {
        uint8_t until;    // values from the segment start
        int16_t step;
    } run[8];
} tSegment;

static const tSegment kSegments[] = {
    {  0, 127,  1, -1736, 2, {{ 12,   363}, {127,  107}      }     },
    {  0, 127,  1,  -302, 1, {{127,   107}}},
    {  0, 127,  1,  -302, 1, {{127,   107}}},
    {  0,  63, -1,  -195, 8, {{  1,     0}, { 49,  214}, {51, 213}, {55, 214}, {58, 213}, {59, 215}, { 61, 210}, {63, 216}}},
    { 64,  64,  0,     0, 1, {{  0,     0}}},
    { 65, 127,  1, 14689, 2, {{  1,  -192}, { 62, -214}      }     },
    {  0, 126,  1,  -355, 2, {{ 20,   336}, {126,   80}      }     },
    {127, 127,  0,     0, 1, {{  0,     0}}},
    {  0, 127,  1, -1379, 2, {{ 20,   336}, {127,   80}      }     },
    {  0,   0,  0,     0, 1, {{  0,     0}}},
    {  1, 127,  1,  4096, 7, {{  1, -1024}, {  3, -512}, {7, -256}, {15, -128}, {31, -64}, {63, -32}, {126,  0}}},
    {  0, 120,  0,     0, 1, {{120,     0}}},
    {121, 127,  1,  4096, 3, {{  1, -1024}, {  3, -512}, {6, -256}     }},
    {  0,  13,  1, 12393, 1, {{ 13,    -8}}},
    { 14,  14,  0,     0, 1, {{  0,     0}}},
    { 15, 127,  1, 19457, 7, {{  3,  -426}, {  4, 1278}, {5, -1792}, {15, -134}, {31, -62}, {62, -32}, {112, -16}}},
    {  0,  17,  1, 12425, 1, {{ 17,    -8}}},
    { 18,  18,  0,     0, 1, {{  0,     0}}},
    { 19, 127,  1, 19457, 7, {{  3,  -426}, {  4, 1278}, {5, -1792}, {15, -134}, {31, -62}, {62, -32}, {108, -16}}},
};

typedef struct {
    uint8_t first, count;
    int8_t  overrideAt;      // -1 none
    int32_t overrideWord;
} tLaw;

static const tLaw     kLaws[]        = {
    { 0, 1, -1, 0},  // Env1Attack
    { 1, 1, -1, 0},  // Env1Decay
    { 2, 1, -1, 0},  // Env1Release
    { 3, 3, -1, 0},  // Env1SustainRate
    { 6, 2, -1, 0},  // Env1SustainTime
    { 8, 1, -1, 0},  // Env2Delay
    { 9, 2, -1, 0},  // FilterOverdrive
    {11, 2, -1, 0},  // FilterQNormalise
    {13, 3, -1, 0},  // LFO1Type
    {16, 3, -1, 0},  // LFO2Type
};

static const uint32_t kTables[][128] = {
    {   // FilterOverdrive
        0x01fd00, 0x03f700, 0x05ed00, 0x07df00, 0x09cd00, 0x0bb700, 0x0d9d00, 0x0f7f00,
        0x115d00, 0x133700, 0x150d00, 0x16df00, 0x18ad00, 0x1a7700, 0x1c3d00, 0x1dff00,
        0x1fbd00, 0x217700, 0x232d00, 0x24df00, 0x268d00, 0x283700, 0x29dd00, 0x2b7f00,
        0x2d1d00, 0x2eb700, 0x304d00, 0x31df00, 0x336d00, 0x34f700, 0x367d00, 0x37ff00,
        0x397d00, 0x3af700, 0x3c6d00, 0x3ddf00, 0x3f4d00, 0x40b700, 0x421d00, 0x437f00,
        0x44dd00, 0x463700, 0x478d00, 0x48df00, 0x4a2d00, 0x4b7700, 0x4cbd00, 0x4dff00,
        0x4f3d00, 0x507700, 0x51ad00, 0x52df00, 0x540d00, 0x553700, 0x565d00, 0x577f00,
        0x589d00, 0x59b700, 0x5acd00, 0x5bdf00, 0x5ced00, 0x5df700, 0x5efd00, 0x5fff00,
        0x60fd00, 0x61f700, 0x62ed00, 0x63df00, 0x64cd00, 0x65b700, 0x669d00, 0x677f00,
        0x685d00, 0x693700, 0x6a0d00, 0x6adf00, 0x6bad00, 0x6c7700, 0x6d3d00, 0x6dff00,
        0x6ebd00, 0x6f7700, 0x702d00, 0x70df00, 0x718d00, 0x723700, 0x72dd00, 0x737f00,
        0x741d00, 0x74b700, 0x754d00, 0x75df00, 0x766d00, 0x76f700, 0x777d00, 0x77ff00,
        0x787d00, 0x78f700, 0x796d00, 0x79df00, 0x7a4d00, 0x7ab700, 0x7b1d00, 0x7b7f00,
        0x7bdd00, 0x7c3700, 0x7c8d00, 0x7cdf00, 0x7d2d00, 0x7d7700, 0x7dbd00, 0x7dff00,
        0x7e3d00, 0x7e7700, 0x7ead00, 0x7edf00, 0x7f0d00, 0x7f3700, 0x7f5d00, 0x7f7f00,
        0x7f9d00, 0x7fb700, 0x7fcd00, 0x7fdf00, 0x7fed00, 0x7ff700, 0x7ffd00, 0x7fff00,
    },
    {   // LFO1SoftenAmount
        0x800000, 0x7ff000, 0x781000, 0x705000, 0x689000, 0x60f000, 0x595000, 0x51d000,
        0x4a5000, 0x42f000, 0x3dc800, 0x3a2800, 0x368800, 0x32f800, 0x2f6800, 0x2be800,
        0x286800, 0x24f800, 0x218800, 0x1f1400, 0x1d6400, 0x1bbc00, 0x1a1400, 0x187400,
        0x16d400, 0x153c00, 0x13a400, 0x121400, 0x108400, 0x0f7e00, 0x0eba00, 0x0dfa00,
        0x0d3a00, 0x0c7e00, 0x0bc200, 0x0b0a00, 0x0a5200, 0x099e00, 0x08ea00, 0x083a00,
        0x07c500, 0x076f00, 0x071900, 0x06c500, 0x067100, 0x061f00, 0x05cd00, 0x057d00,
        0x052d00, 0x04df00, 0x049100, 0x044500, 0x03fc80, 0x03d780, 0x03b280, 0x038e80,
        0x036a80, 0x034780, 0x032480, 0x030280, 0x02e080, 0x02bf80, 0x029e80, 0x027e80,
        0x025e80, 0x023f80, 0x022080, 0x020280, 0x01f240, 0x01e3c0, 0x01d540, 0x01c740,
        0x01b940, 0x01abc0, 0x019e40, 0x019140, 0x018440, 0x0177c0, 0x016b40, 0x015f40,
        0x015340, 0x0147c0, 0x013c40, 0x013140, 0x012640, 0x011bc0, 0x011140, 0x010740,
        0x00fea0, 0x00f9e0, 0x00f520, 0x00f0a0, 0x00ec20, 0x00e7e0, 0x00e3a0, 0x00dfa0,
        0x00dba0, 0x00d7e0, 0x00d420, 0x00d0a0, 0x00cd20, 0x00c9e0, 0x00c6a0, 0x00c3a0,
        0x00c0a0, 0x00bde0, 0x00bb20, 0x00b8a0, 0x00b620, 0x00b3e0, 0x00b1a0, 0x00afa0,
        0x00ada0, 0x00abe0, 0x00aa20, 0x00a8a0, 0x00a720, 0x00a5e0, 0x00a4a0, 0x00a3a0,
        0x00a2a0, 0x00a1e0, 0x00a120, 0x00a0a0, 0x00a020, 0x009fe0, 0x009fa0, 0x009fa0,
    },
    {   // LFO1Sync
        0x000000, 0x01a36c, 0x01179d, 0x00d1b6, 0x008bce, 0x0068db, 0x005d34, 0x0045e7,
        0x00346d, 0x002e9a, 0x0022f3, 0x001a36, 0x00174d, 0x001179, 0x000d1b, 0x000ba6,
        0x0008bc, 0x00068d, 0x0005d3, 0x00045e, 0x000346, 0x0002e9, 0x00029f, 0x00022f,
        0x0001f1, 0x0001df, 0x0001bf, 0x0001a3, 0x000174, 0x00013f, 0x00012a, 0x000117,
        0x0000f8, 0x0000d5, 0x0000ba, 0x00270f, 0x003a97, 0x004e1f, 0x00752e, 0x009c3e,
        0x000000, 0x01a36c, 0x01179d, 0x00d1b6, 0x008bce, 0x0068db, 0x005d34, 0x0045e7,
        0x00346d, 0x002e9a, 0x0022f3, 0x001a36, 0x00174d, 0x001179, 0x000d1b, 0x000ba6,
        0x0008bc, 0x00068d, 0x0005d3, 0x00045e, 0x000346, 0x0002e9, 0x00029f, 0x00022f,
        0x0001f1, 0x0001df, 0x0001bf, 0x0001a3, 0x000174, 0x00013f, 0x00012a, 0x000117,
        0x0000f8, 0x0000d5, 0x0000ba, 0x001a36, 0x00174d, 0x001179, 0x000d1b, 0x000ba6,
        0x0008bc, 0x00068d, 0x0005d3, 0x00045e, 0x000346, 0x0002e9, 0x00029f, 0x00022f,
        0x0001f1, 0x0001df, 0x0001bf, 0x0001a3, 0x000174, 0x00013f, 0x00012a, 0x000117,
        0x0000f8, 0x0000d5, 0x0000ba, 0x00270f, 0x003a97, 0x004e1f, 0x00752e, 0x009c3e,
        0x00afc5, 0x00ea5d, 0x01387c, 0x015f8b, 0x01d4ba, 0x0270f8, 0x02bf17, 0x03a974,
        0x04e1f0, 0x057e2e, 0x0752e8, 0x09c3e0, 0x0afc5c, 0x0ea5d0, 0x1387c0, 0x15f8b8,
        0x1869b0, 0x1d4ba0, 0x20f514, 0x222d90, 0x249e88, 0x270f80, 0x2bf170, 0x334458,
    },
    {   // LFO2Sync
        0x000000, 0x01a36c, 0x01179d, 0x00d1b6, 0x008bce, 0x0068db, 0x005d34, 0x0045e7,
        0x00346d, 0x002e9a, 0x0022f3, 0x001a36, 0x00174d, 0x001179, 0x000d1b, 0x000ba6,
        0x0008bc, 0x00068d, 0x0005d3, 0x00045e, 0x000346, 0x0002e9, 0x00029f, 0x00022f,
        0x0001f1, 0x0001df, 0x0001bf, 0x0001a3, 0x000174, 0x00013f, 0x00012a, 0x000117,
        0x0000f8, 0x0000d5, 0x0000ba, 0x00270f, 0x003a97, 0x004e1f, 0x00752e, 0x009c3e,
        0x00afc5, 0x00ea5d, 0x01387c, 0x015f8b, 0x01d4ba, 0x0270f8, 0x02bf17, 0x03a974,
        0x04e1f0, 0x057e2e, 0x0752e8, 0x09c3e0, 0x0afc5c, 0x0ea5d0, 0x1387c0, 0x15f8b8,
        0x1869b0, 0x1d4ba0, 0x20f514, 0x222d90, 0x249e88, 0x270f80, 0x2bf170, 0x334458,
        0x36edcc, 0x01a36c, 0x01179d, 0x00d1b6, 0x008bce, 0x0068db, 0x005d34, 0x0045e7,
        0x00346d, 0x002e9a, 0x0022f3, 0x001a36, 0x00174d, 0x001179, 0x000d1b, 0x000ba6,
        0x000000, 0x01a36c, 0x01179d, 0x00d1b6, 0x008bce, 0x0068db, 0x005d34, 0x0045e7,
        0x00346d, 0x002e9a, 0x0022f3, 0x001a36, 0x00174d, 0x001179, 0x000d1b, 0x000ba6,
        0x0008bc, 0x00068d, 0x0005d3, 0x00045e, 0x000346, 0x0002e9, 0x00029f, 0x00022f,
        0x0001f1, 0x0001df, 0x0001bf, 0x0001a3, 0x000174, 0x00013f, 0x00012a, 0x000117,
        0x0000f8, 0x0000d5, 0x0000ba, 0x09c3e0, 0x0afc5c, 0x0ea5d0, 0x1387c0, 0x15f8b8,
        0x1869b0, 0x1d4ba0, 0x20f514, 0x222d90, 0x249e88, 0x270f80, 0x2bf170, 0x334458,
    },
};

typedef struct {
    uint8_t word, field;
    int8_t  law, table;   // one of them, the other -1
} tLookup;

static const tLookup  kLookups[] = {
    {0xa6,  1,  0, -1},   // Env1Attack
    {0xa7,  2,  1, -1},   // Env1Decay
    {0xa8,  3,  2, -1},   // Env1Release
    {0xab,  5,  3, -1},   // Env1SustainRate
    {0xaa,  6,  4, -1},   // Env1SustainTime
    {0xad, 10,  5, -1},   // Env2Delay
    {0x98, 33, -1,  0},   // FilterOverdrive
    {0xa3, 33,  6, -1},   // FilterOverdrive
    {0x97, 34,  7, -1},   // FilterQNormalise
    {0xce, 46, -1,  1},   // LFO1SoftenAmount
    {0xcb, 51, -1,  2},   // LFO1Sync
    {0xcc, 52,  8, -1},   // LFO1Type
    {0xd3, 61, -1,  3},   // LFO2Sync
    {0xd4, 62,  9, -1},   // LFO2Type
    {0xaf,  8,  0, -1},   // Env2Attack
    {0xb0,  9,  1, -1},   // Env2Decay
    {0xb1, 11,  1, -1},   // Env2Release
    {0xb4, 13,  3, -1},   // Env2SustainRate
    {0xb3, 14,  4, -1},   // Env2SustainTime
    {0xb8, 16,  0, -1},   // Env3Attack
    {0xb9, 17,  1, -1},   // Env3Decay
    {0xb6, 18,  5, -1},   // Env3Delay
    {0xba, 19,  1, -1},   // Env3Release
    {0xbd, 21,  3, -1},   // Env3SustainRate
    {0xbc, 22,  4, -1},   // Env3SustainTime
    {0xd6, 56, -1,  1},   // LFO2SoftenAmount
};

typedef struct {
    uint8_t word, field;
    int32_t scale;
} tSquare;

static const tSquare  kSigned[]  = { // notes §8: a depth, the amount from the middle squared, its sign kept
    {0x89,  29, 1536},               // FilterFreqLFO1
    {0x8a,  30, 1536},               // FilterFreqLFO2
    {0x05,  97, 1536},               // Osc1ModKnobLFO2Pitch
    {0x02, 105, 1536},               // Osc1PitchEnv2
    {0x03, 106, 1536},               // Osc1PitchEnv3
    {0x04, 107, 1536},               // Osc1PitchLFO1
    {0x29, 130, 1536},               // Osc2ModKnobLFO2Pitch
    {0x26, 137, 1536},               // Osc2PitchEnv2
    {0x27, 138, 1536},               // Osc2PitchEnv3
    {0x28, 139, 1536},               // Osc2PitchLFO1
    {0x4d, 162, 1536},               // Osc3ModKnobLFO2Pitch
    {0x4a, 169, 1536},               // Osc3PitchEnv2
    {0x4b, 170, 1536},               // Osc3PitchEnv3
    {0x4c, 171, 1536},               // Osc3PitchLFO1
};

static const tSquare  kSquares[] = {
    {0xa9, 4, 512},    // Env1Sustain
};

// notes §8: 2^20 halved for each octave of the exponent, read straight across the octave
static int32_t octave_word(int32_t e) {
    int32_t n = e >> 10;
    int32_t f = e & 1023;
    int64_t m = (n >= 0) ? ((int64_t)1 << 20) >> n : ((int64_t)1 << 20) << -n;

    return (int32_t)((m * (2048 - f)) >> 11);
}

static uint32_t law_word(const tLaw * l, int v) {
    if (v == l->overrideAt) {
        return (uint32_t)l->overrideWord & 0xFFFFFFu;
    }

    for (int k = 0; k < l->count; k++) {
        const tSegment * s = &kSegments[l->first + k];

        if ((v < s->from) || (v > s->to)) {
            continue;
        }
        int32_t          e = s->e;

        for (int i = s->from; i < v; i++) {   // the exponent walks the runs to v
            int at = i - s->from, r = 0;

            while ((r < s->nruns - 1) && (at >= s->run[r].until)) {
                r++;
            }
            e += s->run[r].step;
        }

        return (uint32_t)(s->sign * octave_word(e)) & 0xFFFFFFu;
    }

    return 0;
}

static int field(const uint8_t * single, int f) {
    const tField * d = &kFields[f];

    return (single[d->dump + 10 + d->offset] >> d->shift) & d->mask;
}

// notes §8: an oscillator's pitch from its semitones and cents
static uint32_t pitch_word(int semis, int fine) {
    int pitch = (0x8156 + (semis * 256) + ((fine * 8 * 0x51EB) >> 16)) & 0xFFFF;

    return (uint32_t)(((int64_t)pitch * 0xAAAB) >> 8) & 0xFFFFFFu;
}

// notes §13
int sn2_unison_cents(int voices, int which, int detune) {
    int part = (8 * voices) + 16;

    return ((((((8 * which) + 5 - (4 * voices)) * 256) + part) / part) * detune) >> 8;
}

#define EXTENSION    (297 + 10)

// notes §8: a pulse width depth, its byte twice over
static uint32_t width_word(uint8_t byte) {
    int d = (byte & 0x7F) - 64;

    return (d == 63) ? 0x7FFE81u : ((uint32_t)((d * 2) & 0xFF) * 0x010001u);
}

// notes §8: 1024 to 2047 in an exponent's low ten bits, shifted by its octave
static int32_t exponent(int32_t e) {
    int32_t m     = (e & 1023) | 1024;
    int32_t shift = (e >> 10) - 11;

    return (shift >= 0) ? (m << shift) : (m >> -shift);
}

void sn2_fixed_program_words(const uint8_t * single, uint32_t words[SN2_FIXED_WORDS]) {
    memset(words, 0, SN2_FIXED_WORDS * sizeof(words[0]));
    memcpy(words, kBase, sizeof(kBase));

    for (unsigned i = 0; i < sizeof(kLinear) / sizeof(kLinear[0]); i++) {
        words[kLinear[i].word] = (uint32_t)(kLinear[i].base + (kLinear[i].step * field(single, kLinear[i].field))) & 0xFFFFFFu;
    }

    for (unsigned i = 0; i < sizeof(kLookups) / sizeof(kLookups[0]); i++) {
        int v = field(single, kLookups[i].field) & 0x7F;

        words[kLookups[i].word] = (kLookups[i].law >= 0) ? law_word(&kLaws[kLookups[i].law], v) : kTables[kLookups[i].table][v];
    }

    // notes §8: the filter's type - its mode (plain 0, resonant 1, parallel 3) and its taps, a half for each one
    int    type = field(single, FIELD_TYPE) - 3;
    bool   parallel;
    double taps[2][4];

    type        = (type < 0) ? 0 : ((type > 11) ? 11 : type);
    sn2_filter_taps((uint32_t)type, (uint32_t)field(single, FIELD_SLOPE), &parallel, taps);
    words[0x9A] = parallel ? 3u : ((type >= 3) ? 1u : 0u);

    for (int k = 0; k < 8; k++) {
        words[0x9B + k] = (uint32_t)lround(taps[k / 4][k % 4] * 4194304.0) & 0xFFFFFFu;
    }

    // notes §8: each oscillator's tuning - its fields count from 32 + 5o, 57 + 25o and 64 - and its pulse width
    for (int o = 0; o < 3; o++) {
        int octave = field(single, kOscField[o][0]) - (33 + (5 * o));
        int semis  = field(single, kOscField[o][1]) - (57 + (25 * o));
        int fine   = field(single, kOscField[o][2]) - 64;
        int width  = field(single, kOscField[o][3]) - 64;

        words[SN2_FIXED_TUNING + o]     = (uint32_t)((octave * 12) + semis);
        words[SN2_FIXED_TUNING + 3 + o] = (uint32_t)fine;
        words[0x01 + (0x24 * o)]        = pitch_word((octave * 12) + semis, fine);
        words[0x18 + (0x24 * o)]        = (uint32_t)((width >= 63) ? 0x3FFFFF : (width * 65536)) & 0xFFFFFFu;

        int type   = field(single, kOscField[o][4]) - (10 * o); // each oscillator's types count from 10o

        words[0x1D + (0x24 * o)]        = kOscType[(type < 0) ? 0 : ((type > 11) ? 11 : type)];
    }

    const uint8_t * extra = single + EXTENSION;
    uint8_t         modes = extra[3];   // notes §8: FM, Filter OSCs Bypass, the LFOs' fade modes

    words[0x6D] = modes;

    for (int o = 0; o < 3; o++) {   // notes §8: what moves each pulse width - Env 2, Env 3, LFO 1, LFO 2, and the wheel
        static const int kWidthWord[4] = {0x17, 0x19, 0x1A, 0x1B};
        static const int kWidthByte[4] = {43, 44, 45, 46};

        for (int k = 0; k < 4; k++) {
            words[kWidthWord[k] + (0x24 * o)] = width_word(single[10 + kWidthByte[k] + (34 * o)]);
        }

        if (o > 0) {
            words[0x1C + (0x24 * o)] = width_word(single[10 + 42 + (34 * o)]);
        }
    }

    for (int g = 0; g < 3; g++) {   // notes §8: noise and the two rings, squared where they are FM depths instead
        static const int kFmBit[3] = {2, 0, 1};

        if (((modes >> kFmBit[g]) & 1) != 0) {
            int at = 0x74 + (6 * g);

            words[at] = (uint32_t)(int32_t)(((words[at] >> 16) * (words[at] >> 16)) << 9) & 0xFFFFFFu;

            for (int i = 1; i < 6; i++) {
                int32_t d = sn2_s24(words[at + i]) / 131072;

                words[at + i] = (uint32_t)(d * abs(d) * 2048) & 0xFFFFFFu;
            }
        }
    }

    for (int l = 0; l < 2; l++) {   // notes §8: each LFO's delay and fade, and its speed and what moves it
        static const int32_t kRange[3] = {0x0B96, 0x24A3, 0x73DA};
        const uint8_t *      b         = single + 10 + 160 + (10 * l);
        int                  range     = single[10 + 180 + l] - (28 + (3 * l));
        int32_t              k         = kRange[(range < 0) ? 2 : ((range > 2) ? 2 : range)];
        int                  d         = 127 - b[0];
        int32_t              delay     = exponent((d * 0x50) + ((d > 0x5F) ? ((d - 0x5F) * 128) : 0) + 0x41B3);

        words[0xC0 + (2 * l)] = (uint32_t)((((modes >> (3 + l)) & 1) != 0) ? (-(delay * 2) >> 8) : (delay >> 9)) & 0xFFFFFFu;

        for (int i = 0; i < 4; i++) {   // locked to the tempo, only the tempo moves it
            words[0xC7 + (8 * l) + i] = (b[7] != 0) ? 0u : (uint32_t)((i == 0) ? (b[3] * k) : (((b[3 + i] - 64) * 2) * k)) & 0xFFFFFFu;
        }
    }

    if ((single[10 + 237] & 0x04) != 0) {   // notes §8: One Shot - no hold
        words[0xAA] = words[0xB3] = words[0xBC] = 0x7FFFFF;
    }
    words[0xD9]                = (single[10 + 233] == 120) ? 0xFFFFFF : 0xFF0000;

    for (int e = 0; e < 3; e++) {   // notes §8: what the key needs - repeats, time tracking and the dials it moves
        static const uint8_t kDial[3][4] = {{194, 195, 197, 16}, {188, 189, 191, 17}, {182, 183, 185, 18}};

        words[SN2_FIXED_REPEATS + e] = (uint32_t)((extra[13 + e] >= 127) ? -0x7FFF80 : -((int32_t)extra[13 + e] + 1)) & 0xFFFFFFu;   // 127 for ever
        words[SN2_FIXED_RATES + e]   = extra[19 + e];
        words[SN2_FIXED_TRACKS + e]  = extra[4 + e];

        for (int k = 0; k < 4; k++) {
            words[SN2_FIXED_DIALS + (4 * e) + k] = (k < 3) ? single[10 + kDial[e][k]] : extra[kDial[e][k]];
        }
    }

    if ((single[10 + 238] & 0x20) != 0) {   // notes §13
        words[SN2_FIXED_UNISON]     = (uint32_t)(((single[10 + 238] >> 2) & 7) + 2);
        words[SN2_FIXED_UNISON + 1] = extra[2] & 0x7Fu;
        words[0xBE]                 = (uint32_t)((words[0xBE] * (16 - words[SN2_FIXED_UNISON])) / 16);
    }
    words[SN2_FIXED_GLIDE]     = single[10 + 231];
    words[SN2_FIXED_GLIDE + 1] = (single[10 + 234] == 122) ? 1 : 0;

    // notes §8: until the key arrives, each envelope's velocity word holds its depth
    for (int e = 0; e < 3; e++) {
        words[0xA5 + (9 * e)] = (uint32_t)field(single, kVelocityField[e]);
    }

    // notes §8: the Special Width, none with Constant Gate
    words[0x95]                = field(single, FIELD_GATE) ? 0u : ((uint32_t)field(single, FIELD_WIDTH) << 16);

    for (unsigned i = 0; i < sizeof(kSigned) / sizeof(kSigned[0]); i++) {
        int a = (field(single, kSigned[i].field) & 0x7F) - 64;

        words[kSigned[i].word] = (uint32_t)(kSigned[i].scale * a * ((a < 0) ? -a : a)) & 0xFFFFFFu;
    }

    for (unsigned i = 0; i < sizeof(kSquares) / sizeof(kSquares[0]); i++) {
        int v = field(single, kSquares[i].field);

        words[kSquares[i].word] = (uint32_t)(kSquares[i].scale * v * v) & 0xFFFFFFu;
    }
}

static int clamp_dial(int dial) {
    return (dial < 0) ? 0 : ((dial > 127) ? 127 : dial);
}

// notes §8: how many steps a key moves an envelope's times, Attack by two thirds as much
int sn2_fixed_key_moves(int note, int track, bool attack) {
    int high = ((note - 60) * (track - 64) * 2) >> 8;

    return attack ? high : (high + (high >> 1));
}

// notes §8: Sustain Rate, its distance from the centre as a time the key moves like the others
static int32_t rate_word(int dial, int moved) {
    int s = dial - 64;

    if (s == 0) {
        return 0;
    }
    int d = 127 - clamp_dial(127 - (2 * ((abs(s) > 63) ? 63 : abs(s))) - moved);

    return (s > 0) ? (exponent((d * 0x6B) + 0x39B3) >> 8) : (-exponent((d * 0x6B) + 0x4019) >> 8);
}

// notes §8: the key, as semitones from middle C in the word's middle
void sn2_fixed_note_words(uint32_t words[SN2_FIXED_WORDS], uint8_t note, float velocity, double glide, int which) {
    if (words[SN2_FIXED_UNISON] > 0) {   // notes §13
        for (int o = 0; o < 3; o++) {
            words[0x01 + (0x24 * o)] = pitch_word((int)words[SN2_FIXED_TUNING + o], (int)words[SN2_FIXED_TUNING + 3 + o] + sn2_unison_cents((int)words[SN2_FIXED_UNISON], which, (int)words[SN2_FIXED_UNISON + 1]));
        }
    }
    int     v     = (int)lroundf(velocity * 127.0f);

    v           = (v < 1) ? 1 : ((v > 127) ? 127 : v);
    int32_t curve = (((v * v) >> 7) * v) >> 7;   // notes §8: the velocity's curve, 0 to 125
    words[0xDA] = (uint32_t)(((int32_t)note - 60) * 65536) & 0xFFFFFFu;
    words[0xDD] = (uint32_t)(int32_t)lround(((double)note - 60.0 + glide) * 65536.0) & 0xFFFFFFu;

    for (int e = 0; e < 3; e++) {   // notes §8: the depth (64 none) tilts the envelope's level by the curve
        int32_t depth = (int32_t)words[0xA5 + (9 * e)] & 0x7F;
        int32_t floor = 127 * 1024 * ((128 - depth < 64) ? (128 - depth) : 64);

        words[0xA5 + (9 * e)] = (uint32_t)(floor + ((depth - 64) * 1024 * curve)) & 0xFFFFFFu;

        int32_t track = (int32_t)words[SN2_FIXED_TRACKS + e] - 64;

        if (track != 0) {   // notes §8: the key's distance from middle C moves the time dials
            static const uint8_t kWord[3][4] = {{0xA6, 0xA7, 0xA8, 0xAA}, {0xAF, 0xB0, 0xB1, 0xB3}, {0xB8, 0xB9, 0xBA, 0xBC}};
            int                  high        = sn2_fixed_key_moves(note, track + 64, true);
            int                  moved       = sn2_fixed_key_moves(note, track + 64, false);

            words[0xAB + (9 * e)] = (uint32_t)rate_word((int)words[SN2_FIXED_RATES + e], moved) & 0xFFFFFFu;

            for (int k = 0; k < 4; k++) {
                int dial = (int)words[SN2_FIXED_DIALS + (4 * e) + k];
                int by   = (k == 0) ? high : moved;   // Attack by two thirds as much

                if ((k == 3) && ((dial == 127) || (words[kWord[e][k]] == 0x7FFFFF))) {
                    continue;
                }
                dial = clamp_dial(dial - by);

                for (unsigned i = 0; i < sizeof(kLookups) / sizeof(kLookups[0]); i++) {
                    if (kLookups[i].word == kWord[e][k]) {
                        words[kWord[e][k]] = law_word(&kLaws[kLookups[i].law], dial);
                    }
                }
            }
        }
    }

    // notes §8: the glide - towards the key by a part of what is left, or in a straight line over the interval
    int32_t d    = 127 - (int32_t)words[SN2_FIXED_GLIDE];
    int32_t rate = exponent((d * 0x5D) + ((d > 0x70) ? ((d - 0x70) * 256) : 0) + 0x41B3);

    if (words[SN2_FIXED_GLIDE + 1] != 0) {
        int32_t line = (int32_t)(((int64_t)lround(fabs(glide) * 256.0) * rate) >> 16);

        words[0xDB] = (uint32_t)(((line < 0x200) ? 0x200 : line) >> 8);
        words[0xDC] = 0;
    } else {
        words[0xDB] = (uint32_t)(rate >> 13) & 0xFFFFFFu;
        words[0xDC] = (uint32_t)(rate >> 8) & 0xFFFFFFu;
    }
}

double sn2_fixed_env_step(tSn2EnvLaw which, double dial, int moved) {
    static const uint8_t kWord[6] = {0xAD, 0xA6, 0xA7, 0xA8, 0xAA, 0xAB};
    int                  v        = clamp_dial((int)lround(dial));

    if (which == eSn2EnvRate) {
        return rate_word(v, moved) / 8388608.0;
    }

    if ((which != eSn2EnvTime) || (v < 127)) {   // Sustain Time 127 holds for ever, whatever the key
        v = clamp_dial(v - moved);
    }

    for (unsigned i = 0; i < sizeof(kLookups) / sizeof(kLookups[0]); i++) {
        if (kLookups[i].word == kWord[which]) {
            return sn2_s24(law_word(&kLaws[kLookups[i].law], v)) / 8388608.0;
        }
    }

    return 0.0;
}

double sn2_fixed_lfo_soften(double dial) {
    int32_t w = sn2_s24(kTables[1][clamp_dial((int)lround(dial))]);

    return (w < 0) ? 0.0 : (w / 8388608.0);
}

static int32_t velocity_word(int v, int32_t depth) {
    int32_t curve = (((v * v) >> 7) * v) >> 7;   // notes §8: the velocity's curve, 0 to 125
    int32_t floor = 127 * 1024 * ((128 - depth < 64) ? (128 - depth) : 64);

    return floor + ((depth - 64) * 1024 * curve);
}

double sn2_fixed_velocity(double velocity, double depth) {
    int v = (int)lround(velocity * 127.0);

    return velocity_word((v < 1) ? 1 : ((v > 127) ? 127 : v), (int32_t)lround(depth)) / 8388608.0;
}

double sn2_fixed_lfo_delay(double dial, bool fadeOut) {
    int32_t d     = 127 - (int32_t)lround(dial);
    int32_t delay = exponent((d * 0x50) + ((d > 0x5F) ? ((d - 0x5F) * 128) : 0) + 0x41B3);

    return (fadeOut ? (-(delay * 2) >> 8) : (delay >> 9)) / 8388608.0;
}

double sn2_fixed_lfo_sync_hz(int sync) {
    return (double)kTables[2][(sync < 0) ? 0 : ((sync > 34) ? 34 : sync)] * 3221248.0 / (8388608.0 * 8388608.0) * (SN2_ENGINE_HZ / 8.0);
}
