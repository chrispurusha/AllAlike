/*
 * TiAlike - the effects.
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
// Notes: Docs/code-notes/tiEffects.md - "// notes §k" refers there.

#ifndef __TI_EFFECTS_H__
#define __TI_EFFECTS_H__

#include <stdbool.h>
#include <stdint.h>

#define TI_CHORUS_LINE    (16384)

#define TI_DIST_DELAY     (26)
#define TI_DELAY_LINE     (262144)
#define TI_REVERB_LINE    (16382)
#define TI_REVERB_PRE     (65536)
#define TI_BANK_LINE      (16384)

typedef struct {
    double b0, b1, b2, a1, a2;
    double x1[2], x2[2], y1[2], y2[2];
} tTiBiquad;

typedef struct {
    double    rate;
    tTiBiquad low, mid, high;
    uint8_t   lowKey[2], midKey[3], highKey[2];   // what the coefficients were made from
    uint8_t   ready;
    bool      wasOn[6];
    // notes §9: the reverb
    float     verbX[TI_REVERB_LINE];
    int32_t   verbR[4], verbIn;
    int       verbN[4], verbIndex, verbTable[38];
    uint8_t   verbRoom;
    float     verbPre[TI_REVERB_PRE];
    uint32_t  verbPreAt, verbHalf;
    double    verbTone[4], verbSum, verbLast[2], verbMid[2];
    bool      delayAllowed, reverbAllowed;   // notes §8: the plug-in's global switches
    double    bpm;
    float     delayLine[2][TI_DELAY_LINE];
    uint32_t  delayAt;
    double    distState[2];
    double    distLast[2];
    double    distHigh[2];
    double    distHeld[2];
    double    treble[2];
    double    highCut[2][2];
    double    driveState[2][4];
    float     distLine[2][TI_DIST_DELAY];
    uint32_t  distAt[2];
    // notes §3: the phaser
    double    phaseLfo;
    double    apX[2][12], apY[2][12];
    double    phaserLast[2];
    double    apCoeff[2][12];
    uint32_t  phaserLeft;
    // notes §5: the character curves
    tTiBiquad character[8];
    double    cabinet[2][4];
    double    widener[6];
    // notes §6: the filter bank
    double    bankPhase;
    double    hilbert[2][8][4];
    double    hilbertDelay[2];
    double    bankState[2][8][2];
    float     bankLine[2][TI_BANK_LINE];
    uint32_t  bankAt[2];
    uint8_t   characterType;
    // notes §4: the chorus
    double    chorusLfo;
    float     chorusLine[2][TI_CHORUS_LINE];
    uint32_t  chorusAt;
    double    chorusDelay, chorusSwing;   // notes §13: glided toward the bytes' values, so a modulated step does not zipper
    bool      chorusGliding;
} tTiEffects;

#define TI_DIST_CURVES    (25)


// notes §2: laws the saturation curves of the same names share
double ti_dist_corner(bool low, double intensity);
double ti_dist_high_pass_pole(double amount, double rate);
double ti_dist_step(double intensity);
double ti_dist_hold(double intensity);
void ti_effects_init(tTiEffects * e, double rate);
void ti_effects_reset(tTiEffects * e);   // notes §15
// The Single's pages A, B and C
void ti_effects_render(tTiEffects * e, const uint8_t * pageA, const uint8_t * pageB, const uint8_t * pageC, double volume, float * left, float * right, uint32_t frames);

#endif // __TI_EFFECTS_H__
