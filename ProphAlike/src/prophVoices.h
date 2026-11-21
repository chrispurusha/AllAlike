/*
 * ProphAlike - the voices.
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
// Notes: Docs/code-notes/prophVoices.md - "// notes §k" refers there.

#ifndef __TI_VOICES_H__
#define __TI_VOICES_H__

#include <stdbool.h>
#include <stdint.h>

#include "prophEnvelope.h"
#include "prophFilter.h"
#include "prophOsc.h"

#define PROPH_OSCS            (3)
#define PROPH_CLEAN_TAPS      (48)   // notes §17: the low-pass back from four times the rate
#define PROPH_MOD_ENVS        (2)    // Env 2 and 3
#define PROPH_LEVEL_SLOTS     (6)    // Osc 1-3, ring 1x3 and 2x3, noise
#define PROPH_LEVEL_ROUTES    (5)
#define PROPH_LFOS            (2)
#define PROPH_MAX_VOICES      (32)
#define PROPH_VOLUME_UNITY    (83.2) // voices notes §14: the Volume that plays at unity

// notes §6: how the two filters are put together
typedef struct {
    bool            active;

    double          cleanHist[PROPH_OSCS][PROPH_CLEAN_TAPS]; // notes §17: each oscillator at four times the rate
    uint32_t        cleanAt;
    bool            gate;
    uint8_t         note;
    float           velocity;
    double          phase[PROPH_OSCS];
    double          increment[PROPH_OSCS];
    tProphEg        amp;       // envelope notes §10: the amp EG
    double          glide;     // notes §12: semitones from the note
    double          glideRate; // semitones a second, for a straight glide
    uint32_t        noiseSeed;
    double          noiseLow, noiseLast;
    tProphEnv       env[PROPH_MOD_ENVS];
    tProphFilter    filter;
    tProphFilterSet filterSet;            // filter notes §2: what the panel asks of it
    double          lfoPhase[PROPH_LFOS]; // 0..1
    double          lfoHeld[PROPH_LFOS];  // sample and hold
    double          lfoValue[PROPH_LFOS]; // this sub-block's, for the level routes
    double          lfoAge;               // seconds since the note began
    double          lfoSemis[PROPH_OSCS];
    uint64_t        started;              // for stealing the oldest
} tProphVoice;

#define PROPH_LFOS    (2)

// voices notes §15: one LFO, as per-sound values
typedef struct {
    int    shape;       // 0 square, 1 saw, 2 triangle, 3 sample and hold
    double hz;
    double delay;       // seconds of fade in
    bool   fadeOut;     // voices notes §15: from full, dying away, in place of a fade in
    double offset;      // added to the wave
    bool   keyed;       // restarts with each note
} tProphLfoSound;

// One oscillator, in the panel's units
typedef struct {
    tProphOscShape shape;
    double         octaves, semitones, cents;
    double         width;                // 0..2, 1 square
    double         soften;               // 0..127
    double         level;                // 0..1
    double         bendRange;            // semitones at full bend, negative inverts the wheel
    double         wheelPitch;           // semitones at full wheel
    double         lfoPitch[PROPH_LFOS]; // semitones at full swing
    double         lfoWidth[PROPH_LFOS]; // in width units
} tProphOscSound;

// Env 2 or 3, in the panel's units
typedef struct {
    double attack, decay, sustain, release, velocity, amount;
    double sustainTime, sustainRate;   // 0..127 (envelope notes §8)
} tProphModEnv;

// The sound as the voices play it, in the panel's units (notes §3)
typedef struct {
    tProphOscSound osc[PROPH_OSCS];
    double         cutoff;                                            // 0..127
    double         resonance;                                         // 0..127
    double         tracking;                                          // 0..1
    double         overdrive;                                         // 0..127
    double         normalise;                                         // 0..127, Q Normalise
    bool           oscsFree;                                          // Ensemble: phases run on between notes
    double         glideTime;                                         // 0..127
    int            glideStyle;                                        // 0 Portamento, 1 Glissando
    int            glideCurve;                                        // 0 Linear, 1 Exponential
    int            glideType;                                         // 0 Normal, 1 Auto, then the fixed intervals
    double         ampVelocity;                                       // 0..127, 64 none
    uint32_t       filterType, filterSlope;
    double         ampAttack, ampDecay, ampSustain, ampRelease;       // 0..127 (envelope notes)
    double         ampSustainTime, ampSustainRate;                    // 0..127 (envelope notes §8)
    double         ampStart, ampAttackLevel, ampBreak, ampSlope;      // 0..99, envelope notes §10 (with ampAttack, ampDecay, ampSustain, ampRelease)
    tProphModEnv   env[PROPH_MOD_ENVS];
    double         noiseLevel, noiseSoften;                           // 0..1, 0..127
    double         ring13, ring23;                                    // 0..1
    bool           fm13, fm23, fmNoise;                               // notes §18: each turns its level into FM depth on Osc 3
    double         bend;                                              // -1..1
    double         wheel;                                             // 0..1
    double         wheelCutoff, wheelResonance;                       // -64..63
    bool           mono;                                              // one voice, last note priority
    bool           pedal;                                             // the sustain pedal is down
    double         levelRoute[PROPH_LEVEL_SLOTS][PROPH_LEVEL_ROUTES]; // notes §19: full level per unit of Wheel, Env 2, Env 3, LFO 1, LFO 2
    bool           oscsAround;                                        // voices notes §20: the oscillators skip the filter
    bool           oneShot;                                           // envelope notes §9: the key's release is ignored
    tProphLfoSound lfo[PROPH_LFOS];
    double         lfoCutoff[PROPH_LFOS];                             // semitones at full swing
    bool           legato;                                            // mono: a change of note does not restart the envelopes
    bool           piano;                                             // notes §21: Same Note Piano
} tProphSound;

// One oscillator's sound from normalised parameters
#define OSC_SOUND(values, shape_, width_, soften_, level_, octave_, semitone_, fine_)                   { \
       .shape     = (tProphOscShape)lround((values)[shape_] * 2.0),                                       \
       .octaves   = ((values)[octave_] * 4.0) - 2.0,                                                      \
       .semitones = ((values)[semitone_] * 24.0) - 12.0,                                                  \
       .cents     = ((values)[fine_] * 127.0) - 64.0,                                                     \
       .width     = (values)[width_] * 127.0 / 64.0,                                                      \
       .soften    = (values)[soften_] * 127.0,                                                            \
       .level     = (values)[level_] * 127.0 / 128.0,                                                     \
}

// A modulation envelope from normalised parameters
#define MOD_ENV_SOUND(values, attack_, decay_, sustain_, release_, velocity_, amount_, time_, rate_)    { \
       .attack      = (values)[attack_] * 127.0,                                                          \
       .decay       = (values)[decay_] * 127.0,                                                           \
       .sustain     = (values)[sustain_] * 127.0,                                                         \
       .release     = (values)[release_] * 127.0,                                                         \
       .velocity    = (values)[velocity_] * 127.0,                                                        \
       .amount      = ((values)[amount_] * 127.0) - 64.0,                                                 \
       .sustainTime = (values)[time_] * 127.0,                                                            \
       .sustainRate = (values)[rate_] * 127.0,                                                            \
}

// notes §5: a control gliding to its new value as the instrument's Smooth On does
typedef struct {
    bool   started;
    double current, target, step;
    double left;          // samples to go
} tProphSmooth;

typedef struct {
    tProphVoice   voice[PROPH_MAX_VOICES];
    tProphSmooth  cutoff, resonance;
    uint32_t      limit;      // 1..PROPH_MAX_VOICES (design §1: the instance's voice limit)
    double        rate;
    uint64_t      clock;
    int           lastNote;      // notes §12: -1 before the first
    uint8_t       heldNote[128]; // mono: the keys down, oldest first
    float         heldVelocity[128];
    uint32_t      heldCount;
    bool          pedal;               // as the last block saw it
    double        lfoFree[PROPH_LFOS]; // the phase of a free running LFO
    bool          pedalled[128];       // keys up while the pedal held them
    double        edge[PROPH_EDGE_POINTS];
    bool          clean;               // notes §17: Anti-alias - the oscillators at four times the rate, filtered back
    bool          fixedPoint;          // filter notes §8: the instrument's arithmetic
    tProphEgShape ampShape;            // envelope notes §10: this block's, for the keys
} tProphVoices;

// voices notes §14: from the normalised Volume
static inline double proph_volume_gain(double normalized) {
    double volume = normalized * 127.0;

    return ((volume >= 126.5) ? 84.0 : volume) / PROPH_VOLUME_UNITY;
}

void proph_voices_init(tProphVoices * v);
void proph_voices_set_limit(tProphVoices * v, uint32_t limit);
void proph_voices_set_clean(tProphVoices * v, bool clean);   // notes §17: Anti-alias
void proph_voices_set_fixed(tProphVoices * v, bool fixedPoint);
void proph_voices_note_on(tProphVoices * v, const tProphSound * sound, uint8_t note, float velocity);
void proph_voices_note_off(tProphVoices * v, const tProphSound * sound, uint8_t note);
void proph_voices_all_off(tProphVoices * v);
// Adds frames of every voice into out (mono); gain applied
void proph_voices_render(tProphVoices * v, const tProphSound * sound, float * out, uint32_t frames, double gain);

#endif // __TI_VOICES_H__
