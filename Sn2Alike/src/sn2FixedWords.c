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
#include <stdint.h>

#include "sn2Fixed.h"

// notes §8: where each field the words read sits in a single: start of its dump, offset, mask, shift
typedef struct {
    uint16_t dump, offset;
    uint8_t  mask, shift;
} tField;

static const tField kFields[] = {
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
    {  0, 212, 0xff, 0},   // FilterModKnobWhFrequency
    {  0, 222, 0xff, 0},   // FilterModKnobWhResonance
    {  0, 230, 0xff, 0},   // FilterQNormalise
    {  0, 224, 0xff, 0},   // FilterResEnv2
    {  0, 225, 0xff, 0},   // FilterResEnv3
    {  0, 226, 0xff, 0},   // FilterResLFO1
    {  0, 227, 0xff, 0},   // FilterResLFO2
    {  0, 221, 0xff, 0},   // FilterResonance
    {  0, 220, 0xff, 0},   // FilterTracking
    {  0, 162, 0xff, 0},   // LFO1Offset
    {297,   0, 0xff, 0},   // LFO1SoftenAmount
    {  0, 163, 0xff, 0},   // LFO1Speed
    {  0, 166, 0xff, 0},   // LFO1SpeedAftertouchDepth
    {  0, 164, 0xff, 0},   // LFO1SpeedEnv3
    {  0, 165, 0xff, 0},   // LFO1SpeedWhDepth
    {  0, 167, 0xff, 0},   // LFO1Sync
    {  0, 161, 0xff, 0},   // LFO1Type
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
    {  0,  52, 0xff, 0},   // Osc1ModKnobLFO2Sync
    {  0,  36, 0xff, 0},   // Osc1ModKnobWhHardness
    {  0, 125, 0xff, 0},   // Osc1ModKnobWhMix
    {  0,  28, 0xff, 0},   // Osc1ModKnobWhPitch
    {  0,  48, 0xff, 0},   // Osc1ModKnobWhSync
    {  0,  42, 0xff, 0},   // Osc1ModKnobWhWidth
    {  0,  24, 0xff, 0},   // Osc1Semitone
    {  0,  35, 0xff, 0},   // Osc1Soften
    {  0,  37, 0xff, 0},   // Osc1SoftenEnv2
    {  0,  47, 0xff, 0},   // Osc1Sync
    {  0,  49, 0xff, 0},   // Osc1SyncEnv2
    {  0,  55, 0xff, 0},   // Osc1SyncFormantWidth
    {  0,  51, 0xff, 0},   // Osc1SyncLFO1
    {  0,  54, 0xff, 0},   // Osc1SyncSkew
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
    {  0,  86, 0xff, 0},   // Osc2ModKnobLFO2Sync
    {  0,  70, 0xff, 0},   // Osc2ModKnobWhHardness
    {  0, 131, 0xff, 0},   // Osc2ModKnobWhMix
    {  0,  62, 0xff, 0},   // Osc2ModKnobWhPitch
    {  0,  82, 0xff, 0},   // Osc2ModKnobWhSync
    {  0,  58, 0xff, 0},   // Osc2Semitone
    {  0,  69, 0xff, 0},   // Osc2Soften
    {  0,  71, 0xff, 0},   // Osc2SoftenEnv2
    {  0,  81, 0xff, 0},   // Osc2Sync
    {  0,  83, 0xff, 0},   // Osc2SyncEnv2
    {  0,  89, 0xff, 0},   // Osc2SyncFormantWidth
    {  0,  85, 0xff, 0},   // Osc2SyncLFO1
    {  0,  88, 0xff, 0},   // Osc2SyncSkew
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
    {  0, 120, 0xff, 0},   // Osc3ModKnobLFO2Sync
    {  0, 104, 0xff, 0},   // Osc3ModKnobWhHardness
    {  0, 137, 0xff, 0},   // Osc3ModKnobWhMix
    {  0,  96, 0xff, 0},   // Osc3ModKnobWhPitch
    {  0, 116, 0xff, 0},   // Osc3ModKnobWhSync
    {  0,  92, 0xff, 0},   // Osc3Semitone
    {  0, 103, 0xff, 0},   // Osc3Soften
    {  0, 105, 0xff, 0},   // Osc3SoftenEnv2
    {  0, 115, 0xff, 0},   // Osc3Sync
    {  0, 117, 0xff, 0},   // Osc3SyncEnv2
    {  0, 123, 0xff, 0},   // Osc3SyncFormantWidth
    {  0, 119, 0xff, 0},   // Osc3SyncLFO1
    {  0, 122, 0xff, 0},   // Osc3SyncSkew
    {  0, 236, 0xff, 0},   // PartProgramVolume
    {  0, 142, 0xff, 0},   // RingMod1x3MixLevel
    {  0, 148, 0xff, 0},   // RingMod2x3MixLevel
    {297,   7, 0xff, 0},   // SpecialFilterWidth
};

#define FIELD_WIDTH    (152)
#define FIELD_GATE     (0)
static const int kVelocityField[3] = {7, 15, 23};   // each envelope's velocity depth

// notes §8: every word as it stands when nothing moves it
static const uint32_t kBase[SN2_FIXED_WORDS] = {
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

static const tLinear kLinear[] = {
    {0x01, 93,   2636137,   43691},
    {0x06, 90,  -2796160,   43690},
    {0x07, 77,         0,   66052},
    {0x09, 100,  -1048576,   16384},
    {0x0a, 96,         0,   43691},
    {0x0b, 97,  -5592320,   87380},
    {0x0c, 82,  -5592320,   87380},
    {0x0d, 99,  -5592320,   87380},
    {0x0e, 87,  -5592320,   87380},
    {0x0f, 91,  -5592320,   87380},
    {0x10, 76,         0,   65536},
    {0x11, 95,  -8388608,  131072},
    {0x12, 94,   8388096,  -66048},
    {0x13, 80,  -8388608,  131072},
    {0x14, 83,  -8388608,  131072},
    {0x15, 85,  -8388608,  131072},
    {0x16, 88,  -8388608,  131072},
    {0x1c, 92,  -1048576,   16384},
    {0x1e, 98,    260096,   -2048},
    {0x1f, 79,         0,   65536},
    {0x20, 78,  -8388608,  131072},
    {0x21, 81,  -8388608,  131072},
    {0x22, 84,  -8388608,  131072},
    {0x23, 86,  -8388608,  131072},
    {0x24, 89,  -8388608,  131072},
    {0x25, 117,   1543862,   43691},
    {0x2a, 115,  -2796160,   43690},
    {0x2b, 102,         0,   66052},
    {0x2d, 124,  -1048576,   16384},
    {0x2e, 120,         0,   43691},
    {0x2f, 121,  -5592320,   87380},
    {0x30, 107,  -5592320,   87380},
    {0x31, 123,  -5592320,   87380},
    {0x32, 112,  -5592320,   87380},
    {0x33, 116,  -5592320,   87380},
    {0x34, 101,         0,   65536},
    {0x35, 119,  -8388608,  131072},
    {0x36, 118,   8388096,  -66048},
    {0x37, 105,  -8388608,  131072},
    {0x38, 108,  -8388608,  131072},
    {0x39, 110,  -8388608,  131072},
    {0x3a, 113,  -8388608,  131072},
    {0x42, 122,    260096,   -2048},
    {0x43, 104,         0,   65536},
    {0x44, 103,  -8388608,  131072},
    {0x45, 106,  -8388608,  131072},
    {0x46, 109,  -8388608,  131072},
    {0x47, 111,  -8388608,  131072},
    {0x48, 114,  -8388608,  131072},
    {0x49, 141,    451587,   43691},
    {0x4e, 139,  -2796160,   43690},
    {0x4f, 126,         0,   66052},
    {0x51, 148,  -1048576,   16384},
    {0x52, 144,         0,   43691},
    {0x53, 145,  -5592320,   87380},
    {0x54, 131,  -5592320,   87380},
    {0x55, 147,  -5592320,   87380},
    {0x56, 136,  -5592320,   87380},
    {0x57, 140,  -5592320,   87380},
    {0x58, 125,         0,   65536},
    {0x59, 143,  -8388608,  131072},
    {0x5a, 142,   8388096,  -66048},
    {0x5b, 129,  -8388608,  131072},
    {0x5c, 132,  -8388608,  131072},
    {0x5d, 134,  -8388608,  131072},
    {0x5e, 137,  -8388608,  131072},
    {0x66, 146,    260096,   -2048},
    {0x67, 128,         0,   65536},
    {0x68, 127,  -8388608,  131072},
    {0x69, 130,  -8388608,  131072},
    {0x6a, 133,  -8388608,  131072},
    {0x6b, 135,  -8388608,  131072},
    {0x6c, 138,  -8388608,  131072},
    {0x6e, 75,         0,   65536},
    {0x6f, 66,  -8388608,  131072},
    {0x70, 67,  -8388608,  131072},
    {0x71, 69,  -8388608,  131072},
    {0x72, 71,  -8388608,  131072},
    {0x73, 73,  -8388608,  131072},
    {0x74, 65,         0,   65536},
    {0x75, 64,  -8388608,  131072},
    {0x76, 68,  -8388608,  131072},
    {0x77, 70,  -8388608,  131072},
    {0x78, 72,  -8388608,  131072},
    {0x79, 74,  -8388608,  131072},
    {0x7a, 150,         0,   65536},
    {0x7b, 54,  -8388608,  131072},
    {0x7c, 56,  -8388608,  131072},
    {0x7d, 58,  -8388608,  131072},
    {0x7e, 60,  -8388608,  131072},
    {0x7f, 62,  -8388608,  131072},
    {0x80, 151,         0,   65536},
    {0x81, 55,  -8388608,  131072},
    {0x82, 57,  -8388608,  131072},
    {0x83, 59,  -8388608,  131072},
    {0x84, 61,  -8388608,  131072},
    {0x85, 63,  -8388608,  131072},
    {0x86, 26,   1011434,   43691},
    {0x87, 27,  -6291456,   98304},
    {0x88, 28,  -6291456,   98304},
    {0x8b, 29,  -6291456,   98304},
    {0x8c, 24,  -6291456,   98304},
    {0x8d, 37,         0,   66052},
    {0x8e, 36,   8388096,  -66048},
    {0x8f, 32,  -8388608,  131072},
    {0x90, 33,  -8388608,  131072},
    {0x91, 34,  -8388608,  131072},
    {0x92, 35,  -8388608,  131072},
    {0x93, 30,  -8388608,  131072},
    {0x94, 25,  -8388608,  131072},
    {0x96, 31,         0,   65536},
    {0xb2, 12,         0,   65536},
    {0xbb, 20,         0,   65536},
    {0xbe, 149,         0,   52416},
    {0xc7, 40,         0,    2966},
    {0xc8, 42,   -379648,    5932},
    {0xc9, 43,   -379648,    5932},
    {0xca, 41,   -379648,    5932},
    {0xcd, 38,   4194048,  -33024},
    {0xcf, 48,         0,    2966},
    {0xd0, 50,   -379648,    5932},
    {0xd1, 51,   -379648,    5932},
    {0xd2, 49,   -379648,    5932},
    {0xd5, 46,   4194048,  -33024},
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
    {  0, 127,  1,  -1736, 2, {{12, 363}, {127, 107}}},
    {  0, 127,  1,   -302, 1, {{127, 107}}},
    {  0, 127,  1,   -302, 1, {{127, 107}}},
    {  0,  63, -1,   -195, 8, {{1, 0}, {49, 214}, {51, 213}, {55, 214}, {58, 213}, {59, 215}, {61, 210}, {63, 216}}},
    { 64,  64,  0,      0, 1, {{0, 0}}},
    { 65, 127,  1,  14689, 2, {{1, -192}, {62, -214}}},
    {  0, 126,  1,   -355, 2, {{20, 336}, {126, 80}}},
    {127, 127,  0,      0, 1, {{0, 0}}},
    {  0, 127,  1,  -1379, 2, {{20, 336}, {127, 80}}},
    {  0,  13,  1,  12393, 1, {{13, -8}}},
    { 14,  14,  0,      0, 1, {{0, 0}}},
    { 15, 127,  1,  19457, 7, {{3, -426}, {4, 1278}, {5, -1792}, {15, -134}, {31, -62}, {62, -32}, {112, -16}}},
    {  0,  17,  1,  12425, 1, {{17, -8}}},
    { 18,  18,  0,      0, 1, {{0, 0}}},
    { 19, 127,  1,  19457, 7, {{3, -426}, {4, 1278}, {5, -1792}, {15, -134}, {31, -62}, {62, -32}, {108, -16}}},
};

typedef struct {
    uint8_t first, count;
    int8_t  overrideAt;      // -1 none
    int32_t overrideWord;
} tLaw;

static const tLaw kLaws[] = {
    {0, 1, -1, 0},   // Env1Attack
    {1, 1, -1, 0},   // Env1Decay
    {2, 1, -1, 0},   // Env1Release
    {3, 3, -1, 0},   // Env1SustainRate
    {6, 2, -1, 0},   // Env1SustainTime
    {8, 1, -1, 0},   // Env2Delay
    {9, 3, -1, 0},   // LFO1Type
    {12, 3, -1, 0},   // LFO2Type
};

static const uint32_t kTables[][128] = {
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

static const tLookup kLookups[] = {
    {0xa6,  1,  0, -1},   // Env1Attack
    {0xa7,  2,  1, -1},   // Env1Decay
    {0xa8,  3,  2, -1},   // Env1Release
    {0xab,  5,  3, -1},   // Env1SustainRate
    {0xaa,  6,  4, -1},   // Env1SustainTime
    {0xad, 10,  5, -1},   // Env2Delay
    {0xce, 39, -1,  0},   // LFO1SoftenAmount
    {0xcb, 44, -1,  1},   // LFO1Sync
    {0xcc, 45,  6, -1},   // LFO1Type
    {0xd3, 52, -1,  2},   // LFO2Sync
    {0xd4, 53,  7, -1},   // LFO2Type
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
    {0xd6, 47, -1,  0},   // LFO2SoftenAmount
};

typedef struct {
    uint8_t word, field;
    int32_t scale;
} tSquare;

static const tSquare kSquares[] = {
    {0xa9,  4, 512},   // Env1Sustain
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
        int32_t e = s->e;

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

void sn2_fixed_program_words(const uint8_t * single, uint32_t words[SN2_FIXED_WORDS]) {
    for (int i = 0; i < SN2_FIXED_WORDS; i++) {
        words[i] = kBase[i];
    }

    for (unsigned i = 0; i < sizeof(kLinear) / sizeof(kLinear[0]); i++) {
        words[kLinear[i].word] = (uint32_t)(kLinear[i].base + (kLinear[i].step * field(single, kLinear[i].field))) & 0xFFFFFFu;
    }

    for (unsigned i = 0; i < sizeof(kLookups) / sizeof(kLookups[0]); i++) {
        int v = field(single, kLookups[i].field) & 0x7F;

        words[kLookups[i].word] = (kLookups[i].law >= 0) ? law_word(&kLaws[kLookups[i].law], v) : kTables[kLookups[i].table][v];
    }

    // notes §8: until the key arrives, each envelope's velocity word holds its depth
    for (int e = 0; e < 3; e++) {
        words[0xA5 + (9 * e)] = (uint32_t)field(single, kVelocityField[e]);
    }

    // notes §8: the Special Width, none with Constant Gate
    words[0x95] = field(single, FIELD_GATE) ? 0u : ((uint32_t)field(single, FIELD_WIDTH) << 16);

    for (unsigned i = 0; i < sizeof(kSquares) / sizeof(kSquares[0]); i++) {
        int v = field(single, kSquares[i].field);

        words[kSquares[i].word] = (uint32_t)(kSquares[i].scale * v * v) & 0xFFFFFFu;
    }
}

// notes §8: the key, as semitones from middle C in the word's middle
void sn2_fixed_note_words(uint32_t words[SN2_FIXED_WORDS], uint8_t note, float velocity) {
    int     v     = (int)lroundf(velocity * 127.0f);

    v = (v < 1) ? 1 : ((v > 127) ? 127 : v);
    int32_t curve = (((v * v) >> 7) * v) >> 7;   // notes §8: the velocity's curve, 0 to 125
    words[0xDA] = (uint32_t)(((int32_t)note - 60) * 65536) & 0xFFFFFFu;

    for (int e = 0; e < 3; e++) {   // notes §8: the depth (64 none) tilts the envelope's level by the curve
        int32_t depth = (int32_t)words[0xA5 + (9 * e)] & 0x7F;
        int32_t floor = 127 * 1024 * ((128 - depth < 64) ? (128 - depth) : 64);

        words[0xA5 + (9 * e)] = (uint32_t)(floor + ((depth - 64) * 1024 * curve)) & 0xFFFFFFu;
    }
}
