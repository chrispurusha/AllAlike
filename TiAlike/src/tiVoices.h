/*
 * TiAlike - the voices.
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
// Notes: Docs/code-notes/tiVoices.md - "// notes §k" refers there.

#ifndef __TI_VOICES_H__
#define __TI_VOICES_H__

#include <stdbool.h>
#include <stdint.h>

#include "tiEffects.h"
#include "tiEnvelope.h"
#include "tiFilter.h"
#include "tiLfo.h"
#include "tiOsc.h"

#define TI_MAX_VOICES    (32)
#define TI_PAGE          (128)

// notes §6: how the two filters are put together
typedef enum {
    eTiRoutingSerial4 = 0,
    eTiRoutingSerial6,
    eTiRoutingParallel4,
    eTiRoutingSplit,
    eTiRoutings
} tTiRouting;

// notes §8: the live controls a Single's matrix can read, 0..1 (bend -1..1)
typedef enum {
    eTiCtlBend = 0,
    eTiCtlPressure,
    eTiCtlWheel,
    eTiCtlBreath,
    eTiCtl3,
    eTiCtlFoot,
    eTiCtlData,
    eTiCtlBalance,
    eTiCtl9,
    eTiCtlExpression,
    eTiCtl12,
    eTiCtl13,
    eTiCtl14,
    eTiCtl15,
    eTiCtl16,
    eTiCtlHold,
    eTiCtlPortamento,
    eTiCtlSostenuto,
    eTiCtlCount
} tTiControlId;

typedef struct {
    bool      active;
    bool      gate;
    uint8_t   note;
    float     velocity, releaseVelocity;
    double    pitch;          // semitones, gliding to note
    double    phase[3], subPhase;
    double    index[2];       // notes §31: the wavetable Index each oscillator last played, ramped from
    bool      indexSet;
    double    hyper[2][TI_HYPER_SAWS];
    double    twin[2][2];                   // notes §28: Wave PWM's two detuned copies of Osc 1 and Osc 2
    double    grain[2][3];                  // notes §29: the Complex models' half-rate phase and two drifts
    double    detune, pan;                  // unison's offsets
    double    place;                        // 0 at the centre of a stack, 1 at its edge
    double    noiseState;
    uint32_t  noise;
    double    highPass[2];
    double    random;
    double    drift, driftStep;             // notes §7a: Osc 2 Detune's triangle, -1..1, and its step per second
    uint32_t  driftSeed;
    double    ringDc;
    double    satState[2];
    double    punchLeft;                    // samples
    double    punchSign;
    tTiEnv    amp, filterEnv, env3, env4;
    tTiLfo    lfo[3];
    tTiFilter filter, filter2, filterB;     // filterB: Filter 1's second section in Serial 6
    uint64_t  started;                      // for stealing the oldest
} tTiVoice;

// A Single as the voices read it: pages A, B and C (notes §3)
typedef struct {
    uint8_t a[TI_PAGE], b[TI_PAGE], c[TI_PAGE], d[TI_PAGE];
} tTiSound;

// notes §5: a control gliding to its new value as the instrument's Smooth On does
typedef struct {
    bool   started;
    double current, target, step;
    double left;          // samples to go
} tTiSmooth;

typedef struct {
    tTiVoice        voice[TI_MAX_VOICES];
    tTiSmooth       cutoff, resonance, cutoff2, resonance2, balance;
    tTiLfo          monoLfo[3];
    uint32_t        limit;       // 1..TI_MAX_VOICES (design §1: the instance's voice limit)
    double          rate, tempo;
    double          seconds;     // since the instance started, for the free LFOs
    bool            hostPlaying; // notes §5: Quantise counts the host's beats while it plays
    double          hostBeats;   // quarter notes, as of the next sample to render
    bool            releaseHeld; // notes §5: the transport started - held controls land now
    bool            heard;       // sound holds a Single
    bool            glideNow;    // notes §10: the note being started glides
    double          fxMod[128];  // the newest voice's modulation, for the effects
    double          control[eTiCtlCount];
    tTiFilterTables filterTables;
    uint64_t        clock;
    bool            clipped;     // set when the output hits full scale; the caller clears it
    uint8_t         held[128];   // notes §10: keys down, oldest first, for the mono modes
    uint32_t        heldCount;
    int             lastNote;
    bool            fixedPoint;  // notes §19: the arithmetic the instrument uses
    double          boostState[2];
    tTiEffects      effects;
    tTiSound        sound;       // as the voices play it: a Quantise-held byte keeps its old value (notes §5)
} tTiVoices;

// notes §30: the rate the voices run at for a host rate - the host's own if the instrument has it
double ti_voices_engine_rate(double hostRate);
void ti_voices_init(tTiVoices * v, double rate);
void ti_voices_set_rate(tTiVoices * v, double rate);
void ti_voices_set_limit(tTiVoices * v, uint32_t limit);
void ti_voices_set_effects(tTiVoices * v, bool delay, bool reverb);             // notes tiEffects §8
void ti_voices_set_tempo(tTiVoices * v, double bpm);
void ti_voices_set_transport(tTiVoices * v, bool playing, double quarterNotes); // notes §5
void ti_voices_new_sound(tTiVoices * v, const tTiSound * s);                    // notes §5
void ti_voices_hear(tTiVoices * v, const tTiSound * s);                         // notes §5
void ti_voices_set_fixed(tTiVoices * v, bool fixed);
void ti_voices_note_on(tTiVoices * v, uint8_t note, float velocity);
void ti_voices_note_off(tTiVoices * v, uint8_t note, float velocity);
void ti_voices_control(tTiVoices * v, tTiControlId id, double value);
void ti_voices_all_off(tTiVoices * v);
// notes §6: Filter 2's cutoff, 0..127 and beyond, from Cutoff, Cutoff 2 and Link
double ti_voices_cutoff2(double cutoff, double cutoff2, bool link);
// Adds frames of every voice into left and right
// notes §17: curves 1-5, also what Distortion Light to Digital are
double ti_saturate_curve(double x, uint8_t curve, double drive, double unit);
void ti_voices_render(tTiVoices * v, const tTiSound * sound, float * left, float * right, uint32_t frames);

#endif // __TI_VOICES_H__
