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
#define NL3_CHORD_NOTES     12
#define NL3_VOLUME_UNITY    (91.2) // voices notes §14: the Volume that plays at unity (Volume 100 as loud as the old law)

// notes §6: how the two filters are put together
typedef struct {
    bool       active;
    bool       gate;
    uint8_t    note;
    uint8_t    key;    // voices notes §47: the key that started it (a chord's notes share one)
    float      velocity;
    double     phase[NL3_OSCS];
    double     increment[NL3_OSCS];
    double     pitch[NL3_OSCS]; // osc notes §8: in notes
    tNl3Env    amp;             // the amplifier envelope
    double     glide;           // notes §12: semitones from the note
    double     glideRate;       // semitones a second, for a straight glide
    uint32_t   noiseSeed;
    double     noiseLast;       // voices notes §44: Noise3's last word
    tNl3Env    env[NL3_MOD_ENVS];
    tNl3Filter filter;
    double     lfoPhase[NL3_LFOS];                // 0..1
    double     lfoHeld[NL3_LFOS];                 // sample and hold
    double     lfoAge;                            // seconds since the note began
    double     lfoSemis[NL3_OSCS];
    uint64_t   started;                           // for stealing the oldest
    float      ring[256];                         // voices notes §26: the voice's output, for the Unison reads
    uint32_t   ringAt;
    uint32_t   delayWord[2];                      // voices notes §26: 24 bits each, stepped every sample with Unison on
    double     sineLast[NL3_OSCS];                // osc notes §12: the Sine's last sample, for its Shape
    double     syncPhase[NL3_OSCS];               // osc notes §13: the hidden oscillator's phase
    double     syncLast[NL3_OSCS];                // osc notes §13: the synced oscillator's last sample, faded at a restart
    uint32_t   noiseWord[NL3_OSCS];               // osc notes §15: the Noise waveform's generator, its average and its filter
    double     noiseAvg[NL3_OSCS], noiseA[NL3_OSCS], noiseB[NL3_OSCS];
    double     noise2Low, noise2Band, noise2Last; // osc notes §20: Osc 2 Noise's filter and its last input
    double     morphCut, morphRes, morphLevel;
    double     fmLast[2];                         // osc notes §17: Osc 2's last two outputs, Osc Mod FM/DFM's modulator
    double     dmPhase;                           // osc notes §18: DM's hidden sine
    double     dualPhase[2];                      // osc notes §19: Dual Sine's sine B, per oscillator
    double     dualHist[2][2];                    // osc notes §19: sine B's last two outputs (halved)
    double     dualMod[2];                        // osc notes §19: what a Dual Sine hands Osc Mod FM/DFM
    uint32_t   snState[2], snCount[2];            // osc notes §22: synched noise's generator and sub-step count
    double     snPhase[2], snAvg[2], snOut[2];
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
    bool   single;      // voices notes §37: Env mode - one cycle per key, then held where it ends
    int    to;          // its destination, 0..11
    double amount;      // Amount, 0..127
    bool   invert;
} tNl3LfoSound;

// voices notes §21-22: what an LFO's Amount does at each destination - pitch cubic, the rest a step a step
typedef struct {
    double cutoff, mix, amp, pitch[2], width[2];
    double cutoff2, oscMod;   // voices notes §39: Filter 2's cutoff steps; Osc Mod Amount steps
    double res, pan;          // voices notes §46: LFO 2's Resonance steps (one way) and pan (both ways)
} tNl3LfoTargets;

static inline void nl3_lfo_targets(int l, int to, double amount, bool invert, tNl3LfoTargets * t) {
    double sign = invert ? -1.0 : 1.0;   // voices notes §22, §40: only the both-ways destinations take Invert as a sign
    double a    = amount / 127.0;        // the one-way ones (filter, shape, Osc Mod, amp) flip the wave instead
    double cube = (amount >= 127.0) ? 128.0 : amount;
    double semi = sign * cube * cube * cube / 16384.0;

    t->cutoff   = ((to == 3) || (to == 5)) ? (a * 127.0) : 0.0;
    t->mix      = ((l == 0) && (to == 10)) ? (sign * a) : 0.0;
    t->amp      = ((l == 0) && (to == 11)) ? (a * 127.0 / 64.0) : 0.0;
    t->pitch[0] = ((to == 0) || (to == 2)) ? semi : 0.0;
    t->pitch[1] = ((to == 1) || (to == 2)) ? semi : 0.0;
    t->width[0] = ((to == 6) || (to == 8)) ? (a * 127.0 / 128.0) : 0.0;
    t->width[1] = ((to == 7) || (to == 8)) ? (a * 127.0 / 128.0) : 0.0;
    t->cutoff2  = ((to == 4) || (to == 5)) ? (a * 127.0) : 0.0;   // voices notes §39
    t->oscMod   = (to == 9) ? (a * 127.0) : 0.0;
    t->res      = ((l == 1) && (to == 10)) ? (a * 127.0) : 0.0;   // voices notes §46
    t->pan      = ((l == 1) && (to == 11)) ? (sign * amount / 128.0) : 0.0;
}

// One oscillator, in the panel's units
typedef struct {
    tNl3OscShape shape;
    double       octaves, semitones, cents;
    double       width;              // osc notes §11: the pulse's width word, 0 (square) to 1.0
    bool         noise;              // osc notes §15: the Noise waveform, its colour from the width
    double       soften;             // 0..127
    double       level;              // 0..1
    double       bendRange;          // semitones at full bend, negative inverts the wheel
    double       wheelPitch;         // semitones at full wheel
    double       wheelLevel;         // added to level at full wheel
    double       lfoPitch[NL3_LFOS]; // semitones at full swing
    double       lfoWidth[NL3_LFOS]; // voices notes §22: width word added at the LFO's top
    bool         sync;               // osc notes §13: restarted by a hidden oscillator at the note's pitch
    double       syncTune;           // osc notes §13: 0..127, half a semitone a step above the hidden one
    bool         dualSine;           // osc notes §19: Dual Sine, two sines
    bool         dualFm;             // osc notes §19: Dual Sine FM (the Sync switch, in Dual Sine)
    double       dualRatio;          // osc notes §19: sine B's increment over sine A's
    int          noise2;             // osc notes §20: Osc 2 Noise, 1 low pass, 2 band pass, 3 high pass (0 none)
    bool         fixedPitch;         // osc notes §21: Keyboard Track off - from MIDI note 64, not the key played
    bool         syncNoise;          // osc notes §22: Noise with Sync - the same noise restarted every cycle
    uint32_t     syncSeed;           // osc notes §22: where it restarts, Sync Noise Timbre in 16 steps
} tNl3OscSound;

// Env 2 or 3, in the panel's units
typedef struct {
    double attack, decay, sustain, release, velocity, amount;
    bool   exponential;    // Exp Attack (envelope notes §4)
    bool   repeat;         // envelope notes §6
    bool   invert;         // voices notes §43: flips the shape on the one-way destinations, a sign on the both-ways ones
} tNl3ModEnv;

// The sound as the voices play it, in the panel's units (notes §3)
typedef struct {
    tNl3OscSound osc[NL3_OSCS];
    double       cutoff;                                      // 0..127
    double       resonance;                                   // 0..127
    double       level;                                       // the normalised Output Level (voices notes §32: a morph moves it)
    double       keyShift;                                    // voices notes §33: Octave Shift, in semitones
    double       tracking;                                    // 0 or 1: Keyboard Track off or on
    double       overdrive;                                   // 0..127
    double       filterDist;                                  // filter notes §17: Dist LP's Distortion, 0..1
    bool         oscBypass;                                   // voices notes §38: Osc 2 (with Noise3, both oscillators) around the filter
    double       normalise;                                   // 0..127, Q Normalise
    double       glideTime;                                   // 0..127
    int          glideStyle;                                  // 0 Portamento, 1 Glissando
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
    double       noiseLevel;                                  // 0..1
    double       ring13, ring23;                              // 0..1
    double       bend;                                        // -1..1
    double       wheel;                                       // 0..1
    double       wheelCutoff, wheelResonance;                 // -64..63
    bool         mono;                                        // one voice, last note priority
    uint32_t     chordNotes;                                  // voices notes §47: 0 = Chord Memory off
    uint8_t      chord[NL3_CHORD_NOTES];                      // each note's interval above the key
    bool         pedal;                                       // the sustain pedal is down
    tNl3LfoSound lfo[NL3_LFOS];
    double       lfoCutoff[NL3_LFOS];                         // voices notes §22: semitones at the LFO's top, none at its bottom
    double       lfoMix[NL3_LFOS];                            // voices notes §22: Osc Mix moved either way, as a fraction of the dial
    double       lfoAmp[NL3_LFOS];                            // voices notes §22: the level's fall at the LFO's bottom, 1 = silence
    double       modEnvPitch[NL3_OSCS];                       // voices notes §24: semitones at the Mod envelope's top
    double       modEnvWidth[NL3_OSCS];                       // voices notes §24: width word at its top
    double       modEnvMix;                                   // voices notes §24: Osc Mix at its top, a fraction of the dial
    int          oscModType;                                  // osc notes §17: 0 DM 1 DIST 2 FILT 3 FM 4 DFM 5 RM
    double       oscModAmount;                                // 0..127
    double       modEnvOscMod;                                // Amount steps the Mod Env adds at its top
    double       modEnvCutoff2;                               // voices notes §39: Filter 2's cutoff steps at the Mod Env's top
    double       lfoCutoff2[NL3_LFOS], lfoOscMod[NL3_LFOS];   // voices notes §39
    double       lfoRes[NL3_LFOS], lfoPan[NL3_LFOS];          // voices notes §46
    double       mixX;                                        // voices notes §36: Osc Mix as x, 0..1
    double       modEnvLfo1;                                  // voices notes §24: LFO 1 Amount steps added at its top
    double       vibratoSemis;                                // voices notes §31: the vibrato's depth now, its source applied
    double       vibratoHz;
    bool         unison;                                      // voices notes §26
    double       unisonStep;                                  // voices notes §26: the first read's delay word step a sample
    bool         legato;                                      // mono: a change of note does not restart the envelopes
    bool         piano;                                       // voices notes §34: Same Note Piano
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
    uint8_t    order[NL3_MAX_VOICES];      // voices notes §28: the voices in the order the next notes take them, ranked at the last note event
    uint32_t   orderNext;
    bool       pedal;                      // as the last block saw it
    double     lfoFree[NL3_LFOS];          // the phase of a free running LFO
    bool       pedalled[128];              // keys up while the pedal held them
    double     edge[NL3_EDGE_POINTS];
    bool       fixedPoint;                 // notes §19: the arithmetic the instrument uses
    bool       clean;                      // notes §20: the Aliasing setting
    double     vibratoPhase;               // voices notes §31: one vibrato for every voice, 0..1
    tNl3Steps  steps;
    tNl3Sound  voiceSound[NL3_MAX_VOICES]; // voices notes §25: a voice's sound with its morphs, when it has any
    bool       voiceMorphed[NL3_MAX_VOICES];
} tNl3Voices;

// voices notes §14: from the normalised Volume
static inline double nl3_volume_gain(double normalized) {
    double v = normalized * 127.0 / NL3_VOLUME_UNITY;

    return v * v;
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
