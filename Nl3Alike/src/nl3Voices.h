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
    double     pitch[NL3_OSCS]; // osc notes §8: in notes
    tNl3Env    amp;             // the amplifier envelope
    double     glide;           // notes §12: semitones from the note
    double     glideRate;       // semitones a second, for a straight glide
    uint32_t   noiseSeed;
    double     noiseLow, noiseLast;
    tNl3Env    env[NL3_MOD_ENVS];
    tNl3Filter filter;
    double     lfoPhase[NL3_LFOS]; // 0..1
    double     lfoHeld[NL3_LFOS];  // sample and hold
    double     lfoAge;             // seconds since the note began
    double     lfoSemis[NL3_OSCS];
    uint64_t   started;            // for stealing the oldest
    float      ring[256];          // voices notes §26: the voice's output, for the Unison reads
    uint32_t   ringAt;
    uint32_t   delayWord[2];       // voices notes §26: 24 bits each, stepped every sample with Unison on
    double     sineLast[NL3_OSCS]; // osc notes §12: the Sine's last sample, for its Shape
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
    double       width;              // osc notes §11: the pulse's width word, 0 (square) to 1.0
    double       soften;             // 0..127
    double       level;              // 0..1
    double       bendRange;          // semitones at full bend, negative inverts the wheel
    double       wheelPitch;         // semitones at full wheel
    double       wheelLevel;         // added to level at full wheel
    double       lfoPitch[NL3_LFOS]; // semitones at full swing
    double       lfoWidth[NL3_LFOS]; // voices notes §22: width word added at the LFO's top
} tNl3OscSound;

// Env 2 or 3, in the panel's units
typedef struct {
    double attack, decay, sustain, release, velocity, amount;
    bool   exponential;    // Exp Attack (envelope notes §4)
    bool   repeat;         // envelope notes §6
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
    bool         filterMulti;                                 // filter notes §15: the multi modes
    uint32_t     filter2Type;
    double       cutoff2;                                     // 0..127
    bool         filter2Env;                                  // the envelope and its moves reach the second cutoff too
    double       ampAttack, ampDecay, ampSustain, ampRelease; // 0..127 (envelope notes)
    bool         ampExponential;                              // Exp Attack (envelope notes §4)
    tNl3ModEnv   env[NL3_MOD_ENVS];
    double       noiseLevel, noiseSoften;                     // 0..1, 0..127
    double       ring13, ring23;                              // 0..1
    double       bend;                                        // -1..1
    double       wheel;                                       // 0..1
    double       wheelCutoff, wheelResonance;                 // -64..63
    bool         mono;                                        // one voice, last note priority
    bool         pedal;                                       // the sustain pedal is down
    tNl3LfoSound lfo[NL3_LFOS];
    double       lfoCutoff[NL3_LFOS];                         // voices notes §22: semitones at the LFO's top, none at its bottom
    double       lfoMix[NL3_LFOS];                            // voices notes §22: Osc Mix moved either way, as a fraction of the dial
    double       lfoAmp[NL3_LFOS];                            // voices notes §22: the level's fall at the LFO's bottom, 1 = silence
    double       modEnvPitch[NL3_OSCS];                       // voices notes §24: semitones at the Mod envelope's top
    double       modEnvWidth[NL3_OSCS];                       // voices notes §24: width word at its top
    double       modEnvMix;                                   // voices notes §24: Osc Mix at its top, a fraction of the dial
    bool         unison;                                      // voices notes §26
    double       unisonStep;                                  // voices notes §26: the first read's delay word step a sample
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
    bool       pedal;                      // as the last block saw it
    double     lfoFree[NL3_LFOS];          // the phase of a free running LFO
    bool       pedalled[128];              // keys up while the pedal held them
    double     edge[NL3_EDGE_POINTS];
    bool       fixedPoint;                 // notes §19: the arithmetic the instrument uses
    bool       clean;                      // notes §20: the Aliasing setting
    tNl3Steps  steps;
    tNl3Sound  voiceSound[NL3_MAX_VOICES]; // voices notes §25: a voice's sound with its morphs, when it has any
    bool       voiceMorphed[NL3_MAX_VOICES];
} tNl3Voices;

// voices notes §14: from the normalised Volume
static inline double nl3_volume_gain(double normalized) {
    double volume = normalized * 127.0;

    return ((volume >= 126.5) ? 84.0 : volume) / NL3_VOLUME_UNITY;
}

void nl3_voices_init(tNl3Voices * v);
void nl3_voices_set_limit(tNl3Voices * v, uint32_t limit);
void nl3_voices_set_fixed(tNl3Voices * v, bool fixed);
void nl3_voices_set_clean(tNl3Voices * v, bool clean);
void nl3_voices_note_on(tNl3Voices * v, const tNl3Sound * sound, uint8_t note, float velocity);
void nl3_voices_note_off(tNl3Voices * v, const tNl3Sound * sound, uint8_t note);
void nl3_voices_all_off(tNl3Voices * v);
// Adds frames of every voice into out (mono); gain applied
// out is the left channel, outRight the right (NULL: a mono sum into out)
void nl3_voices_render(tNl3Voices * v, const tNl3Sound * sound, float * out, float * outRight, uint32_t frames, double gain);

#endif // __TI_VOICES_H__
