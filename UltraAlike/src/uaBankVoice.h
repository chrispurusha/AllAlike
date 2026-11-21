/*
 * UltraAlike - a preset's voices: every zone a note reaches, from an E-mu bank.
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
// Notes: Docs/code-notes/uaBankVoice.md - "// notes §k" refers there.

#ifndef __UA_BANK_VOICE_H__
#define __UA_BANK_VOICE_H__

#include <stdbool.h>
#include <stdint.h>

#include "emuBank.h"
#include "uaEosEnv.h"
#include "uaEnvelope.h"
#include "uaVoice.h"

#define UA_BANK_VOICES    (64)

typedef struct {
    const tEmuPreset * preset;     // what started it: kept in memory while it sounds
    const tEmuSample * sample;
    double             position;   // in the sample's frames
    double             step;       // frames per output frame
    double             cutoff;     // the interpolator's bandwidth for that step
    int                emu;        // notes §4: the E-mu kernel (uaEmuInterp) read with Anti-alias Off, or -1
    bool               looping;    // notes §2: has passed its loop's end at least once
    double             level;      // velocity's part
    double             panL, panR; // notes §12: the zone's pan and level, 1 and 1 in the centre at 0 dB
    bool               oneShot;    // notes §12: a note off leaves it playing to its end
    double             fade;       // 1 while sounding; falls to 0 when stolen
    double             fadeStep;
    tUaEosEnvelope     amp;        // notes §6: its amplifier envelope, the dials' edits included
    tUaEosEnvState     env;
    uint8_t            note;
    bool               pedalHeld;  // notes §9: its key is up, the sustain pedal holds it
    bool               active;
    uint32_t           age;
} tUaBankVoice;

typedef struct {
    tUaBankVoice voice[UA_BANK_VOICES];
    double       hostRate;
    uint32_t     started;
    double       outL, outR;
    bool         outSet;
    float        keyVelocity[128];   // notes §13: what each key was struck with, for its release zones
    bool         keyDown[128];
} tUaBankVoices;

void ua_bank_voices_init(tUaBankVoices * v, double hostRate);
void ua_bank_voices_stop(tUaBankVoices * v);
void ua_bank_voices_fade_all(tUaBankVoices * v);
// notes §1: every zone of the preset whose keys and velocities hold the note
void ua_bank_voices_note_on(tUaBankVoices * v, const tEmuPreset * preset, const tUaEosEnvelope * envs, const tUaEosEnvelope * ref, const tUaPlay * play, uint8_t note, float velocity);
void ua_bank_voices_note_off(tUaBankVoices * v, uint8_t note, bool pedal);
// notes §13: after the note off, the zones that sound when this key is let go
void ua_bank_voices_release_zones(tUaBankVoices * v, const tEmuPreset * preset, const tUaEosEnvelope * envs, const tUaEosEnvelope * ref, const tUaPlay * play, uint8_t note);
void ua_bank_voices_pedal_up(tUaBankVoices * v);   // notes §9: releases what the pedal held
void ua_bank_voices_render(tUaBankVoices * v, const tUaPlay * play, float * left, float * right, uint32_t frames);
bool ua_bank_voices_use_other_than(const tUaBankVoices * v, const tEmuPreset * preset);

#endif // __UA_BANK_VOICE_H__
