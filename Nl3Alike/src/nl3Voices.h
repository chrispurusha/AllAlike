/*
 * Nl3Alike - the voices.
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
// Notes: Docs/code-notes/nl3Voices.md - "// notes §k" refers there.

#ifndef __TI_VOICES_H__
#define __TI_VOICES_H__

#include <stdbool.h>
#include <stdint.h>

#include "nl3Envelope.h"
#include "nl3Filter.h"
#include "nl3Osc.h"

#define NL3_OSCS            (3)
#define NL3_MOD_ENVS        (2)    // Env 2 and 3
#define NL3_LFOS            (2)
#define NL3_MAX_VOICES      (32)
#define NL3_VOLUME_UNITY    (83.2) // voices notes §14: the Volume that plays at unity

// notes §6: how the two filters are put together
typedef struct {
    bool       active;
    bool       gate;
    uint8_t    note;
    float      velocity;
    double     phase[NL3_OSCS];
    double     increment[NL3_OSCS];
    tNl3Env    amp;       // the amplifier envelope
    double     glide;     // notes §12: semitones from the note
    double     glideRate; // semitones a second, for a straight glide
    uint32_t   noiseSeed;
    double     noiseLow, noiseLast;
    tNl3Env    env[NL3_MOD_ENVS];
    tNl3Filter filter;
    double     lfoPhase[NL3_LFOS]; // 0..1
    double     lfoHeld[NL3_LFOS];  // sample and hold
    double     lfoAge;             // seconds since the note began
    double     lfoSemis[NL3_OSCS];
    uint64_t   started;            // for stealing the oldest
} tNl3Voice;

#define NL3_LFOS    (2)

// voices notes §15: one LFO, as per-sound values
typedef struct {
    int    shape;       // 0 square, 1 saw, 2 triangle, 3 sample and hold
    double hz;
    double delay;       // seconds of fade in
    bool   fadeOut;     // voices notes §15: from full, dying away, in place of a fade in
    double offset;      // added to the wave
    bool   keyed;       // restarts with each note
} tNl3LfoSound;

// One oscillator, in the panel's units
typedef struct {
    tNl3OscShape shape;
    double       octaves, semitones, cents;
    double       width;              // 0..2, 1 square
    double       soften;             // 0..127
    double       level;              // 0..1
    double       bendRange;          // semitones at full bend, negative inverts the wheel
    double       wheelPitch;         // semitones at full wheel
    double       wheelLevel;         // added to level at full wheel
    double       lfoPitch[NL3_LFOS]; // semitones at full swing
    double       lfoWidth[NL3_LFOS]; // in width units
} tNl3OscSound;

// Env 2 or 3, in the panel's units
typedef struct {
    double attack, decay, sustain, release, velocity, amount;
} tNl3ModEnv;

// The sound as the voices play it, in the panel's units (notes §3)
typedef struct {
    tNl3OscSound osc[NL3_OSCS];
    double       cutoff;                                      // 0..127
    double       resonance;                                   // 0..127
    double       tracking;                                    // 0 or 1: Keyboard Track off or on
    double       overdrive;                                   // 0..127
    double       normalise;                                   // 0..127, Q Normalise
    bool         oscsFree;                                    // Ensemble: phases run on between notes
    double       glideTime;                                   // 0..127
    int          glideStyle;                                  // 0 Portamento, 1 Glissando
    int          glideCurve;                                  // 0 Linear, 1 Exponential
    int          glideType;                                   // 0 Normal, 1 Auto, then the fixed intervals
    double       ampVelocity;                                 // 0..127, 64 none
    uint32_t     filterType, filterSlope;
    double       ampAttack, ampDecay, ampSustain, ampRelease; // 0..127 (envelope notes)
    tNl3ModEnv   env[NL3_MOD_ENVS];
    double       noiseLevel, noiseSoften;                     // 0..1, 0..127
    double       ring13, ring23;                              // 0..1
    double       bend;                                        // -1..1
    double       wheel;                                       // 0..1
    double       wheelCutoff, wheelResonance;                 // -64..63
    bool         mono;                                        // one voice, last note priority
    bool         pedal;                                       // the sustain pedal is down
    tNl3LfoSound lfo[NL3_LFOS];
    double       lfoCutoff[NL3_LFOS];                         // semitones at full swing
    bool         legato;                                      // mono: a change of note does not restart the envelopes
} tNl3Sound;

// One oscillator's sound from normalised parameters
#define OSC_SOUND(values, shape_, width_, soften_, level_, octave_, semitone_, fine_)     { \
       .shape     = (tNl3OscShape)lround((values)[shape_] * 2.0),                           \
       .octaves   = ((values)[octave_] * 4.0) - 2.0,                                        \
       .semitones = ((values)[semitone_] * 24.0) - 12.0,                                    \
       .cents     = ((values)[fine_] * 127.0) - 64.0,                                       \
       .width     = (values)[width_] * 127.0 / 64.0,                                        \
       .soften    = (values)[soften_] * 127.0,                                              \
       .level     = (values)[level_] * 127.0 / 128.0,                                       \
}

// A modulation envelope from normalised parameters
#define MOD_ENV_SOUND(values, attack_, decay_, sustain_, release_, velocity_, amount_)    { \
       .attack   = (values)[attack_] * 127.0,                                               \
       .decay    = (values)[decay_] * 127.0,                                                \
       .sustain  = (values)[sustain_] * 127.0,                                              \
       .release  = (values)[release_] * 127.0,                                              \
       .velocity = (values)[velocity_] * 127.0,                                             \
       .amount   = ((values)[amount_] * 127.0) - 64.0,                                      \
}

// notes §5: a control gliding to its new value as the instrument's Smooth On does
typedef struct {
    bool   started;
    double current, target, step;
    double left;          // samples to go
} tNl3Smooth;

typedef struct {
    tNl3Voice  voice[NL3_MAX_VOICES];
    tNl3Smooth cutoff, resonance;
    uint32_t   limit;         // 1..NL3_MAX_VOICES (design §1: the instance's voice limit)
    double     rate;
    uint64_t   clock;
    int        lastNote;      // notes §12: -1 before the first
    uint8_t    heldNote[128]; // mono: the keys down, oldest first
    float      heldVelocity[128];
    uint32_t   heldCount;
    bool       pedal;             // as the last block saw it
    double     lfoFree[NL3_LFOS]; // the phase of a free running LFO
    bool       pedalled[128];     // keys up while the pedal held them
    double     edge[NL3_EDGE_POINTS];
    bool       fixedPoint;        // notes §19: the arithmetic the instrument uses
} tNl3Voices;

// voices notes §14: from the normalised Volume
static inline double nl3_volume_gain(double normalized) {
    double volume = normalized * 127.0;

    return ((volume >= 126.5) ? 84.0 : volume) / NL3_VOLUME_UNITY;
}

void nl3_voices_init(tNl3Voices * v);
void nl3_voices_set_limit(tNl3Voices * v, uint32_t limit);
void nl3_voices_set_fixed(tNl3Voices * v, bool fixed);
void nl3_voices_note_on(tNl3Voices * v, const tNl3Sound * sound, uint8_t note, float velocity);
void nl3_voices_note_off(tNl3Voices * v, const tNl3Sound * sound, uint8_t note);
void nl3_voices_all_off(tNl3Voices * v);
// Adds frames of every voice into out (mono); gain applied
void nl3_voices_render(tNl3Voices * v, const tNl3Sound * sound, float * out, uint32_t frames, double gain);

#endif // __TI_VOICES_H__
