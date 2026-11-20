/*
 * ProphAlike - the effects section.
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
// Notes: Docs/code-notes/prophEffects.md - "// notes §k" refers there.

#ifndef __PROPH_EFFECTS_H__
#define __PROPH_EFFECTS_H__

#include <stdbool.h>
#include <stdint.h>

#define PROPH_CHORUS_LINE     (2048u)
#define PROPH_REVERB_LINES    (8)
#define PROPH_REVERB_LINE     (4096u)
#define PROPH_REVERB_INPUT    (16384u)
#define PROPH_DELAY_LINE      (33554u)  // the longest time, in samples, and a little

// The panel's units
typedef struct {
    bool   bypass;
    double dry;                                  // 0..127
    double chorusSend, chorusSpeed, chorusDepth; // 0..127
    double chorusFeedback;                       // 0..127, 64 none
    int    chorusType;                           // 0 Quad, 1 Chorus/Flanger, 2 Phaser
    bool   delayOn;                              // notes §4: the Global Delay switch
    double delaySend, delayTime, delayFeedback;  // 0..127
    double delayDamp, delayWidth;                // 0..127
    int    delayRatio, delaySync;                // indices
    double tempo;                                // BPM
    bool   reverbOn;                             // notes §5: the Global Reverb switch
    double reverbSend, reverbDecay, reverbDamp;  // 0..127
    int    reverbType;                           // 0..15
    int    order;                                // notes §6: 0..18
    double distLevel, distComp, distOutput;      // 0..127, notes §7
    double eqBass, eqTreble;                     // 0..127, 64 flat, notes §8
    double pan, panSpeed, panDepth;              // 0..127
    bool   panEffects;                           // the wet signal pans with the dry
} tProphEffectsSound;

typedef struct {
    float    line[PROPH_CHORUS_LINE];
    uint32_t write;
    double   phase[4];
    double   feedback;
    float    delayLine[PROPH_DELAY_LINE];
    uint32_t delayWrite;
    double   delayLow;
    float    reverbLine[PROPH_REVERB_LINES][PROPH_REVERB_LINE];
    float    reverbInput[PROPH_REVERB_INPUT];
    double   reverbLow[PROPH_REVERB_LINES];
    uint32_t reverbWrite, reverbInputWrite;
    double   eqLow[2], eqHigh[2];
    double   panPhase;  // 0..1
    bool     clipped;   // set when the output hits the ceiling; the caller clears it
} tProphEffects;

void proph_effects_reset(tProphEffects * e);
// The voices with every effect out; only the output ceiling is left
void proph_effects_bypass(tProphEffects * e, const float * in, float * left, float * right, uint32_t frames);
// A block of the voices' mono mix to stereo, all at the engine's rate
void proph_effects_process(tProphEffects * e, const tProphEffectsSound * s, const float * in, float * left, float * right, uint32_t frames);

#endif // __PROPH_EFFECTS_H__
