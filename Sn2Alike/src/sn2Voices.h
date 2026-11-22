/*
 * Sn2Alike - the voices.
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
// Notes: Docs/code-notes/sn2Voices.md - "// notes §k" refers there.

#ifndef __TI_VOICES_H__
#define __TI_VOICES_H__

#include <stdbool.h>
#include <stdint.h>

#include "sn2Envelope.h"
#include "sn2Filter.h"
#include "sn2Fixed.h"
#include "sn2Osc.h"

#define SN2_OSCS            (3)
#define SN2_CLEAN_TAPS      (48)   // notes §17: the low-pass back from four times the rate
#define SN2_MOD_ENVS        (2)    // Env 2 and 3
#define SN2_LEVEL_SLOTS     (6)    // Osc 1-3, ring 1x3 and 2x3, noise
#define SN2_LEVEL_ROUTES    (5)
#define SN2_LFOS            (2)
#define SN2_MAX_VOICES      (32)
#define SN2_VOLUME_UNITY    (83.2) // voices notes §14: the Volume that plays at unity

// notes §6: how the two filters are put together
typedef struct {
    bool             active;

    double           cleanHist[SN2_OSCS][SN2_CLEAN_TAPS]; // notes §17: each oscillator at four times the rate
    uint32_t         cleanAt;
    bool             gate;
    uint8_t          note;
    int              which;     // notes §24: its number in a unison
    float            velocity;
    double           phase[SN2_OSCS];
    double           increment[SN2_OSCS];
    double           window[SN2_OSCS]; // sn2Osc notes §7: Formant Width's, 1 as each cycle starts
    tSn2Env          amp;              // the amplifier envelope
    double           glide;            // notes §12: semitones from the note
    double           glideRate;        // semitones a second, for a straight glide
    uint32_t         noiseSeed;
    uint32_t         noise, noiseCount;
    double           noiseLow;
    tSn2Env          env[SN2_MOD_ENVS];
    tSn2Filter       filter;
    double           lfoPhase[SN2_LFOS]; // 0..1
    double           lfoHeld[SN2_LFOS];  // sample and hold
    double           lfoValue[SN2_LFOS]; // this sub-block's, for the level routes
    double           lfoAge;             // seconds since the note began
    double           lfoSemis[SN2_OSCS];
    double           syncMod[SN2_OSCS];  // Sync steps the LFOs add this sub-block
    uint64_t         started;            // for stealing the oldest
    tSn2FixedVoice * fixed;              // sn2Fixed notes §9: its twin in the instrument's own words, with Maths FIXED
    uint32_t         quiet;              // samples silent since it was let go
} tSn2Voice;

#define SN2_LFOS    (2)

// voices notes §15: one LFO, as per-sound values
typedef struct {
    int    shape;       // 0 square, 1 saw, 2 triangle, 3 sample and hold
    double hz;
    double soften;      // voices notes §15: its smoothing, 0 none
    double fade;        // voices notes §15: a step of its delay, below 0 a part of what is left
    bool   fadeOut;     // voices notes §15: from full, dying away, in place of a fade in
    double offset;      // added to the wave
    bool   keyed;       // restarts with each note
} tSn2LfoSound;

// One oscillator, in the panel's units
typedef struct {
    tSn2OscShape shape;
    double       octaves, semitones, cents;
    double       width;              // 0..2, 1 square
    double       soften;             // 0..127
    double       sync;               // 0..127, 0 free
    double       syncEnv2;           // Sync steps at full Env 2
    double       syncEnv3;           // Sync steps at full Env 3 (mod knob route)
    double       syncWheel;          // Sync steps at full Mod Wheel (mod knob route)
    double       syncLfo[SN2_LFOS];  // Sync steps at full swing
    double       skew;               // sn2Osc notes §6: octaves the slave moves over a cycle, -1..1
    double       softenRoute[5];     // voices notes §23: Soften steps from the wheel, Env 2, Env 3, LFO 1 and LFO 2
    double       widthRoute[3];      // voices notes §23: in width units, from the wheel, Env 2 and Env 3
    double       keySync;            // sn2Osc notes §8: Sync steps off for each key up
    double       windowFall;         // sn2Osc notes §7: what the window loses a sample, 0 none
    double       envPitch[2];        // semitones at full Env 2, Env 3
    double       level;              // 0..1
    double       bendRange;          // semitones at full bend, negative inverts the wheel
    double       wheelPitch;         // semitones at full wheel
    double       lfoPitch[SN2_LFOS]; // semitones at full swing
    double       lfoWidth[SN2_LFOS]; // in width units
} tSn2OscSound;

// Env 2 or 3, in the panel's units
typedef struct {
    double delay, attack, decay, sustain, release, velocity, amount;
    double sustainTime, sustainRate;   // 0..127 (envelope notes §8)
} tSn2ModEnv;

// The sound as the voices play it, in the panel's units (notes §3)
typedef struct {
    tSn2OscSound osc[SN2_OSCS];
    double       cutoff;                                        // 0..127
    double       resonance;                                     // 0..127
    double       tracking;                                      // 0..1
    double       overdrive;                                     // 0..127
    double       normalise;                                     // 0..127, Q Normalise
    double       filterWidth;                                   // 0..127, Special Filter Width (sn2Filter notes §7)
    bool         oscsFree;                                      // Ensemble: phases run on between notes
    double       glideTime;                                     // 0..127
    int          glideStyle;                                    // 0 Portamento, 1 Glissando
    int          glideCurve;                                    // 0 Linear, 1 Exponential
    int          glideType;                                     // 0 Normal, 1 Auto, then the fixed intervals
    double       ampVelocity;                                   // 0..127, 64 none
    uint32_t     filterType, filterSlope;
    double       ampAttack, ampDecay, ampSustain, ampRelease;   // 0..127 (envelope notes)
    double       ampSustainTime, ampSustainRate;                // 0..127 (envelope notes §8)
    tSn2ModEnv   env[SN2_MOD_ENVS];
    double       noiseLevel, noiseSoften;                       // 0..1, 0..127
    double       ring13, ring23;                                // 0..1
    bool         fm13, fm23, fmNoise;                           // notes §18: each turns its level into FM depth on Osc 3
    double       bend;                                          // -1..1
    double       wheel;                                         // 0..1
    double       wheelCutoff, wheelResonance;                   // -64..63
    double       resRoute[4];                                   // -64..63, Env 2, Env 3, LFO 1, LFO 2 to resonance
    double       noiseRoute[5];                                 // -64..63 to Noise Soften: the wheel, Env 2, Env 3, LFO 1, LFO 2
    int          envTrack[3], envRepeats[3];                    // envelope notes §5: the amplifier, Env 2, Env 3
    int          unison, unisonDetune;                          // notes §24: voices to a key, 0 off; 0..127
    double       tempo;                                         // the host's, beats a minute
    bool         mono;                                          // one voice, last note priority
    bool         pedal;                                         // the sustain pedal is down
    double       levelRoute[SN2_LEVEL_SLOTS][SN2_LEVEL_ROUTES]; // notes §19: full level per unit of Wheel, Env 2, Env 3, LFO 1, LFO 2
    bool         oscsAround;                                    // voices notes §20: the oscillators skip the filter
    bool         oneShot;                                       // envelope notes §9: the key's release is ignored
    tSn2LfoSound lfo[SN2_LFOS];
    double       lfoCutoff[SN2_LFOS];                           // semitones at full swing
    bool         legato;                                        // mono: a change of note does not restart the envelopes
    bool         piano;                                         // notes §21: Same Note Piano
    bool         fixedPoint;                                    // sn2Fixed notes §9: Maths FIXED
    uint32_t     fixedWords[SN2_FIXED_WORDS];                   // the program's words for the host side to write
} tSn2Sound;

// One oscillator's sound from normalised parameters
#define OSC_SOUND(values, shape_, width_, soften_, level_, octave_, semitone_, fine_, sync_)            { \
       .shape     = (tSn2OscShape)lround((values)[shape_] * 2.0),                                         \
       .octaves   = ((values)[octave_] * 4.0) - 2.0,                                                      \
       .semitones = ((values)[semitone_] * 24.0) - 12.0,                                                  \
       .cents     = ((values)[fine_] * 127.0) - 64.0,                                                     \
       .width     = ((values)[width_] >= 1.0) ? 2.0 : ((values)[width_] * 127.0 / 64.0),                  \
       .soften    = (values)[soften_] * 127.0,                                                            \
       .sync      = (values)[sync_] * 127.0,                                                              \
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
} tSn2Smooth;

typedef struct {
    tSn2Voice        voice[SN2_MAX_VOICES];
    tSn2Smooth       cutoff, resonance;
    uint32_t         limit;   // 1..SN2_MAX_VOICES (design §1: the instance's voice limit)
    double           rate;
    uint64_t         clock;
    int              lastNote;      // notes §12: -1 before the first
    uint8_t          heldNote[128]; // mono: the keys down, oldest first
    float            heldVelocity[128];
    uint32_t         heldCount;
    bool             pedal;             // as the last block saw it
    double           lfoFree[SN2_LFOS]; // the phase of a free running LFO
    bool             pedalled[128];     // keys up while the pedal held them
    double           edge[SN2_EDGE_POINTS];
    bool             clean;             // notes §17: Anti-alias - the oscillators at four times the rate, filtered back
    tSn2FixedVoice * fixed;             // sn2Fixed notes §9: allocated the first time Maths FIXED is chosen
} tSn2Voices;

// voices notes §14: from the normalised Volume
static inline double sn2_volume_gain(double normalized) {
    return normalized * 127.0 / SN2_VOLUME_UNITY;
}

void sn2_voices_init(tSn2Voices * v);
void sn2_voices_set_limit(tSn2Voices * v, uint32_t limit);
void sn2_voices_set_clean(tSn2Voices * v, bool clean);   // notes §17: Anti-alias
void sn2_voices_set_fixed(tSn2Voices * v, bool fixed);   // sn2Fixed notes §9: Maths
void sn2_voices_free(tSn2Voices * v);
void sn2_voices_note_on(tSn2Voices * v, const tSn2Sound * sound, uint8_t note, float velocity);
void sn2_voices_note_off(tSn2Voices * v, const tSn2Sound * sound, uint8_t note);
void sn2_voices_all_off(tSn2Voices * v);
// Adds frames of every voice into out (mono); gain applied
void sn2_voices_render(tSn2Voices * v, const tSn2Sound * sound, float * out, uint32_t frames, double gain);

#endif // __TI_VOICES_H__
