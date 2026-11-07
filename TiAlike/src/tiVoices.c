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

#include <math.h>
#include <string.h>

#include "tiFixed.h"
#include "tiVoices.h"

#define SMOOTH_S             (2048.0 / 46875.0) // notes §5
#define SUB_BLOCK            (32u)              // notes §4
#define OUTPUT_GAIN          (-0.4155)          // notes §7: a sine at Patch Volume 127 and Filter 1 alone
#define RING_GAIN            (3.0)              // notes §7
#define NOISE_LEVEL          (0.719)            // notes §7
#define SUB_SQUARE           (2.0 / 3.0)        // notes §7
#define SUB_TRIANGLE         (2.62)             // notes §7
#define SPLIT_GAIN           (1.28)             // notes §6
#define OSC3_GAIN            (0.01332)          // notes §7: of a sine, per step: Volume is a straight line
#define PAN_HARD             (0.585)            // notes §7
#define DC_BLOCK_HZ          (15.0)             // notes §7
#define KEY_CENTRE           (60.0)             // notes §7
#define TUNE_CENTS           (-0.951)           // notes §7
#define TUNE_SLOPE           (1.576)
#define DETUNE_SLOPE         (1.393)
#define MOD_DEPTH            (128.0)            // notes §9: full amount, in dial units
#define KEY_DIAL_PER_SEMI    (1.08)             // notes §11
#define PITCH_MOD_DEPTH      (128.0)            // notes §9: full amount, in semitones
#define PITCH_MOD_LIMIT      (48.0)             // notes §24
#define PUNCH_SECONDS        (0.03)
#define PUNCH_TAU            (0.0004)
#define PUNCH_LEVEL          (3.376)
#define PUNCH_OCTAVE         (9.6)

// Matrix destinations (the instrument's numbering)
enum {
    kDstVolume        = 1,
    kDstPan           = 3,
    kDstTranspose     = 4,
    kDstPortamento    = 5,
    kDstShape1        = 6,
    kDstWidth1        = 7,
    kDstWave1         = 8,
    kDstPitch1        = 9,
    kDstShape2        = 11,
    kDstWidth2        = 12,
    kDstWave2         = 13,
    kDstPitch2        = 14,
    kDstDetune        = 15,
    kDstFm            = 16,
    kDstEnvPitch      = 17,
    kDstEnvFm         = 18,
    kDstOscBalance    = 20,
    kDstSub           = 21,
    kDstOscVolume     = 22,
    kDstNoise         = 23,
    kDstCutoff1       = 24,
    kDstCutoff2       = 25,
    kDstRes1          = 26,
    kDstRes2          = 27,
    kDstEnvAmt1       = 28,
    kDstEnvAmt2       = 29,
    kDstFilterBalance = 32,
    kDstFAttack       = 33,
    kDstFRelease      = 37,
    kDstAAttack       = 38,
    kDstARelease      = 42,
    kDstLfo1Rate      = 43,
    kDstLfo1Contour   = 44,
    kDstLfo1Pitch1    = 45,
    kDstLfo1Pitch2    = 46,
    kDstLfo1Width     = 47,
    kDstLfo1Res       = 48,
    kDstLfo1Gain      = 49,
    kDstLfo2Rate      = 50,
    kDstLfo2Contour   = 51,
    kDstLfo2Shape     = 52,
    kDstLfo2Fm        = 53,
    kDstLfo2Cut1      = 54,
    kDstLfo2Cut2      = 55,
    kDstLfo2Pan       = 56,
    kDstLfo3Rate      = 57,
    kDstLfo3Amount    = 58,
    kDstNoiseColor    = 91,
    kDstRing          = 90,
    kDstPunch         = 89,
    kDstOsc3Volume    = 97,
    kDstOsc3Pitch     = 98,
    kDstOsc3Detune    = 99,
    kDstLfo1Assign    = 100,
    kDstIndex1        = 73,
    kDstIndex2        = 74,
    kDstLfo2Assign    = 101,
    kDstCount         = 128
};

// Sources (the instrument's numbering) past the live controls
enum {
    kSrcAmpEnv = 19,
    kSrcFilterEnv,
    kSrcLfo1,
    kSrcLfo2,
    kSrcLfo3,
    kSrcVelOn,
    kSrcVelOff,
    kSrcKey,
    kSrcRandom,
    kSrcArp,
    kSrcLfo1Uni,
    kSrcLfo2Uni,
    kSrcLfo3Uni,
    kSrc1Percent,
    kSrc10Percent,
    kSrcEnv3 = 38,
    kSrcEnv4
};

// notes §9: where each matrix slot lives: source, then up to three destination and amount pairs (page, index)
typedef struct {
    uint8_t page, index;
} tTiAt;
static const struct {
    tTiAt source, slot[3][2];
} kSlots[6] = {
    {{1,  64}, {{{1,  65}, {1,  66}}, {{2,  90}, {2,  91}}, {{2,  92}, {2,  93}}}},
    {{1,  67}, {{{1,  68}, {1,  69}}, {{1,  70}, {1,  71}}, {{2,  94}, {2,  95}}}},
    {{1,  72}, {{{1,  73}, {1,  74}}, {{1,  75}, {1,  76}}, {{1,  77}, {1,  78}}}},
    {{1, 103}, {{{1, 104}, {1, 105}}, {{2,  96}, {2,  97}}, {{2,  98}, {2,  99}}}},
    {{1, 106}, {{{1, 107}, {1, 108}}, {{2, 100}, {2, 101}}, {{2, 102}, {2, 103}}}},
    {{1, 109}, {{{1, 110}, {1, 111}}, {{2, 104}, {2, 105}}, {{2, 106}, {2, 107}}}},
};

static uint8_t page_byte(const tTiSound * s, tTiAt at) {
    return (at.page == 0) ? s->a[at.index] : ((at.page == 1) ? s->b[at.index] : s->c[at.index]);
}

static double bipolar(uint8_t byte) {
    return ((double)byte - 64.0) / 64.0;
}

static double clamp(double x, double lo, double hi) {
    return (x < lo) ? lo : ((x > hi) ? hi : x);
}

void ti_voices_init(tTiVoices * v, double rate) {
    memset(v, 0, sizeof(*v));
    v->limit      = 16;
    v->tempo      = 0.0;
    v->lastNote   = -1;
    v->fixedPoint = false;
    ti_osc_init();
    ti_voices_set_rate(v, rate);

    for (uint32_t i = 0; i < TI_MAX_VOICES; i++) {
        v->voice[i].noise = (i * 2654435761u) + 1u;
    }

    for (int l = 0; l < 3; l++) {
        ti_lfo_trigger(&v->monoLfo[l], 0.0, 12345u + l);
    }
}

void ti_voices_set_rate(tTiVoices * v, double rate) {
    v->rate = rate;
    ti_filter_tables(&v->filterTables, rate);
    ti_effects_init(&v->effects, rate);
}

void ti_voices_set_fixed(tTiVoices * v, bool fixed) {
    v->fixedPoint = fixed;
}

void ti_voices_set_effects(tTiVoices * v, bool delay, bool reverb) {
    v->effects.delayAllowed  = delay;
    v->effects.reverbAllowed = reverb;
}

// notes §5: the host's position at the start of this block; starting the transport lands held controls
void ti_voices_set_transport(tTiVoices * v, bool playing, double quarterNotes) {
    if (playing && !v->hostPlaying) {
        v->releaseHeld = true;
    }
    v->hostPlaying = playing;
    v->hostBeats   = quarterNotes;
}

// notes §5: a whole new Single takes its values at once, and a change held from the old one is dropped
void ti_voices_new_sound(tTiVoices * v, const tTiSound * s) {
    v->sound = *s;
    v->heard = true;
}

void ti_voices_set_tempo(tTiVoices * v, double bpm) {
    v->tempo       = (bpm > 1.0) ? bpm : 0.0;
    v->effects.bpm = v->tempo;
}

static void gate_voice(tTiVoice * vc, bool on) {
    ti_env_gate(&vc->amp, on);
    ti_env_gate(&vc->filterEnv, on);
    ti_env_gate(&vc->env3, on);
    ti_env_gate(&vc->env4, on);
}

void ti_voices_set_limit(tTiVoices * v, uint32_t limit) {
    limit    = (limit < 1u) ? 1u : ((limit > TI_MAX_VOICES) ? TI_MAX_VOICES : limit);
    v->limit = limit;

    // notes §2: voices above a lowered limit are released, not cut
    for (uint32_t i = limit; i < TI_MAX_VOICES; i++) {
        v->voice[i].gate = false;
        gate_voice(&v->voice[i], false);
    }
}

void ti_voices_control(tTiVoices * v, tTiControlId id, double value) {
    bool lifted = (id == eTiCtlHold) && (v->control[id] >= 0.5) && (value < 0.5);

    v->control[id] = value;

    // notes §10: the pedal up releases every voice whose key is no longer down
    for (uint32_t i = 0; lifted && (i < TI_MAX_VOICES); i++) {
        tTiVoice * vc   = &v->voice[i];
        bool       down = false;

        for (uint32_t k = 0; k < v->heldCount; k++) {
            down = down || (v->held[k] == vc->note);
        }

        if (vc->gate && !down) {
            vc->gate = false;
            gate_voice(vc, false);
        }
    }
}

static tTiVoice * pick_voice(tTiVoices * v) {
    tTiVoice * pick = NULL;

    // a free voice, else the oldest: a key struck again takes a new voice like any other note
    for (uint32_t i = 0; (i < v->limit) && (pick == NULL); i++) {
        if (!v->voice[i].active) {
            pick = &v->voice[i];
        }
    }

    if (pick == NULL) {
        pick = &v->voice[0];

        for (uint32_t i = 1; i < v->limit; i++) {
            if (v->voice[i].started < pick->started) {
                pick = &v->voice[i];
            }
        }
    }
    return pick;
}

// notes §9: a note's voice from silence or stolen
#define FREE_LFO_PHASE    (0.0)   // notes §20

static void lfo_shape(const tTiVoices * v, const tTiSound * s, int l, const double * mod, tTiLfoShape * out);

static void start_voice(tTiVoices * v, tTiVoice * vc, uint8_t note, float velocity, double detune, double pan, double place, double lfoPhase, bool oddVoice, bool retrigger) {
    const tTiSound * s      = &v->sound;
    bool             silent = !vc->active || (vc->amp.level <= 0.0);

    if (silent) {
        vc->pitch = ((s->a[5] > 0) && (v->lastNote >= 0)) ? (double)v->lastNote : (double)note;
    }
    vc->active          = true;
    vc->gate            = true;
    vc->note            = note;
    vc->velocity        = velocity;
    vc->releaseVelocity = 1.0f;
    vc->detune          = detune;
    vc->place           = fabs(place);
    vc->pan             = pan;
    vc->started         = ++v->clock;
    vc->noise           = (vc->noise * 1664525u) + 1013904223u;
    vc->random          = ((double)(int32_t)vc->noise) / 2147483648.0;

    if (retrigger || silent) {
        gate_voice(vc, true);
        vc->punchLeft = PUNCH_SECONDS * v->rate;

        if (s->a[54] == 0) {             // notes §26: Attack 0 is there on the first sample
            vc->filterEnv.level = 1.0;
        }

        for (int l = 0; l < 3; l++) {
            uint8_t trig = (l == 0) ? s->a[73] : ((l == 1) ? s->a[85] : 0);
            bool    mono = (l == 0) ? (s->a[70] != 0) : ((l == 1) ? (s->a[82] != 0) : (s->b[9] != 0));

            if (!mono) {
                double ph      = fmod(((trig > 0) ? ((trig - 1.0) / 126.0) : 0.0) + lfoPhase + 1.0, 1.0); // notes §12

                bool   envMode = (l == 0) ? (s->a[69] != 0) : ((l == 1) ? (s->a[81] != 0) : false);       // notes §12: always from the top
                bool   clocked = (l == 0) ? (s->b[18] != 0) : ((l == 1) ? (s->b[19] != 0) : false);

                vc->lfo[l].pinned = envMode && clocked && oddVoice;     // notes §13

                if ((trig > 0) || (l == 2) || envMode) {
                    ti_lfo_trigger(&vc->lfo[l], ph, 0u);
                } else {
                    vc->lfo[l].spent = false;
                    vc->lfo[l].seen  = 0.0;

                    if (silent) {
                        double      none[kDstCount] = {0.0};
                        tTiLfoShape shape;

                        lfo_shape(v, s, l, none, &shape);
                        ti_lfo_trigger(&vc->lfo[l], fmod(FREE_LFO_PHASE + (shape.rateHz * v->seconds) + lfoPhase + 1.0, 1.0), 0u);
                    }
                }
            }
        }
    }
    double init = s->b[35];

    // notes §7: a fixed phase restarts on every note, sounding or not
    if (silent || (retrigger && (init > 0.0))) {
        vc->highPass[0] = 0.0;
        vc->highPass[1] = 0.0;
        // notes §7: Init Phase places Osc 2 against Osc 1; at 0 the oscillators run free of each other
        vc->phase[0]    = (init > 0.0) ? 0.0 : ((vc->random * 0.5) + 0.5);
        vc->phase[1]    = (init > 0.0) ? (init / 128.0) : fmod((vc->random * 0.381966) + 0.7, 1.0);
        vc->phase[2]    = (init > 0.0) ? 0.0 : fmod((vc->random * 0.236068) + 0.2, 1.0);
        vc->subPhase    = 0.0;
        vc->twin[0][0]  = vc->twin[0][1] = vc->phase[0];
        vc->twin[1][0]  = vc->twin[1][1] = vc->phase[1];
        vc->grain[0][0] = 0.5 * vc->phase[0];
        vc->grain[1][0] = 0.5 * vc->phase[1];
        vc->grain[0][1] = vc->grain[0][2] = vc->grain[1][1] = vc->grain[1][2] = 0.0;

        for (int k = 0; k < TI_HYPER_SAWS; k++) {
            vc->hyper[0][k] = fmod(0.618034 * (k + 1) * (vc->random + 2.0), 1.0);
            vc->hyper[1][k] = fmod(0.414214 * (k + 1) * (vc->random + 2.0), 1.0);
        }

        ti_filter_reset(&vc->filter);    // a voice starting from silence starts with a quiet filter
        ti_filter_reset(&vc->filter2);
        ti_filter_reset(&vc->filterB);
    }
}

// notes §13: a Single older than the TI's format keeps Unison on page A
#define TI_FORMAT    (12)

static const uint8_t * unison_bytes(const tTiSound * s) {
    return (s->a[0] < TI_FORMAT) ? &s->a[97] : &s->d[120];
}

// notes §13: where each voice of a stack stands
static const double kStack[6][6] = {
    { 0.0,  0.0,  0.0, 0.0, 0.0, 0.0},
    {-1.0,  1.0,  0.0, 0.0, 0.0, 0.0},
    {-1.0,  0.0,  1.0, 0.0, 0.0, 0.0},
    {-1.0, -0.5,  0.5, 1.0, 0.0, 0.0},
    {-1.0, -0.5,  0.0, 0.5, 1.0, 0.0},
    {-1.0, -0.6, -0.3, 0.3, 0.6, 1.0},
};

static double unison_cents(double dial) {
    static const double kAt[5] = {0, 32, 64, 96, 127};
    static const double kCt[5] = {0, 1.5, 4.6, 10.0, 24.1};

    for (int i = 1; i < 5; i++) {
        if (dial <= kAt[i]) {
            return kCt[i - 1] + ((kCt[i] - kCt[i - 1]) * (dial - kAt[i - 1]) / (kAt[i] - kAt[i - 1]));
        }
    }

    return kCt[4];
}

static uint32_t unison_count(const tTiSound * s) {
    uint8_t u = unison_bytes(s)[0];

    return (u >= 5) ? 6u : (u + 1u);   // notes §13
}

static void trigger_note(tTiVoices * v, uint8_t note, float velocity, bool retrigger) {
    const tTiSound * s      = &v->sound;
    uint32_t         n      = unison_count(s);
    bool             mono   = (s->a[94] >= 1) && (s->a[94] <= 4);
    const uint8_t *  bytes  = unison_bytes(s);
    double           spread = bytes[2] / 127.0;
    double           cents  = unison_cents(bytes[1]);
    double           phases = ((double)bytes[3] - 64.0) / 128.0;   // notes §13: of a cycle, every second voice

    for (uint32_t u = 0; u < n; u++) {
        double     place = kStack[n - 1u][u];     // notes §13: -1..1 across the stack
        tTiVoice * vc;

        if (mono) {
            vc = &v->voice[u % v->limit];
        } else {
            vc = pick_voice(v);
        }
        start_voice(v, vc, note, velocity, place * cents / 100.0, place * spread, place, (u & 1u) ? phases : 0.0, (u & 1u) != 0u, retrigger);
    }
}

void ti_voices_note_on(tTiVoices * v, uint8_t note, float velocity) {
    bool mono   = (v->sound.a[94] >= 1) && (v->sound.a[94] <= 4);
    bool legato = mono && (v->heldCount > 0u);

    if (v->heldCount < 128u) {
        v->held[v->heldCount++] = note;
    }
    trigger_note(v, note, velocity, !legato || (v->sound.a[94] == 1) || (v->sound.a[94] == 3));
    v->lastNote = note;
}

void ti_voices_note_off(tTiVoices * v, uint8_t note, float velocity) {
    bool mono = (v->sound.a[94] >= 1) && (v->sound.a[94] <= 4);

    for (uint32_t i = 0; i < v->heldCount; i++) {
        if (v->held[i] == note) {
            memmove(&v->held[i], &v->held[i + 1], v->heldCount - i - 1);
            v->heldCount--;
            break;
        }
    }

    // notes §10: a mono voice falls back to the last key still down
    if (mono && (v->heldCount > 0u)) {
        trigger_note(v, v->held[v->heldCount - 1u], (float)v->voice[0].velocity, false);
        return;
    }

    if ((v->sound.a[94] == 5) || (v->control[eTiCtlHold] >= 0.5)) {   // notes §10: Hold, and the pedal
        return;
    }

    for (uint32_t i = 0; i < TI_MAX_VOICES; i++) {
        if (v->voice[i].active && v->voice[i].gate && (mono || (v->voice[i].note == note))) {
            v->voice[i].gate            = false;
            v->voice[i].releaseVelocity = velocity;
            gate_voice(&v->voice[i], false);
        }
    }
}

void ti_voices_all_off(tTiVoices * v) {
    v->heldCount = 0u;

    for (uint32_t i = 0; i < TI_MAX_VOICES; i++) {
        tTiVoice * vc = &v->voice[i];

        vc->active    = false;
        vc->gate      = false;
        vc->amp       = (tTiEnv){
            0
        };
        vc->filterEnv = vc->env3 = vc->env4 = vc->amp;
        ti_filter_reset(&vc->filter);
        ti_filter_reset(&vc->filter2);
        ti_filter_reset(&vc->filterB);
    }
}

// notes §5: Smooth Mode's Quantise values 4..20 hold a change to the next division, in whole notes
static const double  kQuantiseWhole[17] = {
    1.0 / 64.0, 1.0 / 32.0, 1.0 / 16.0, 1.0 / 8.0, 1.0 / 4.0, 1.0 / 2.0, 3.0 / 64.0, 3.0 / 32.0, 3.0 / 16.0, 3.0 / 8.0,
    1.0 / 24.0, 1.0 / 12.0,  1.0 / 6.0, 1.0 / 3.0, 2.0 / 3.0, 3.0 / 4.0, 1.0
};

// notes §5: the page A bytes Quantise holds - the continuous controls; switches, modes, shapes, wave selects and the
// amp envelope's Attack, Decay and Release land at once
static const uint8_t kQuantiseHeld[]    = {
    1,   2,  3,  4,  5,  6,  7,  8,   9,  10,  11,  12,  13,  14,  15,  16,  17,  18,  20,  21,  22, 23, 25, 26, 27, 29, 30, 31, 33, 34, 36,
    37, 39, 40, 41, 42, 43, 44, 45,  46,  47,  48,  50,  56,  57,  58,  61,  62,  67,  71,  72,  73, 74, 75, 76, 77, 78, 79, 83,
    84, 85, 86, 87, 88, 89, 90, 91, 104, 105, 106, 107, 108, 109, 111, 113, 114, 115, 116, 117, 119,
};

static bool quantise_mode(const tTiSound * s) {
    return (s->b[25] >= 4) && (s->b[25] <= 20);
}

// notes §5: the sound the voices play - a held byte keeps its old value until the division (release_held)
static void hear(tTiVoices * v, const tTiSound * s) {
    tTiSound now = *s;

    if (v->heard && quantise_mode(s)) {
        for (size_t i = 0; i < sizeof(kQuantiseHeld); i++) {
            now.a[kQuantiseHeld[i]] = v->sound.a[kQuantiseHeld[i]];
        }
    }
    v->sound = now;
    v->heard = true;
}

void ti_voices_hear(tTiVoices * v, const tTiSound * s) {
    hear(v, s);
}

static void smooth_toward(tTiSmooth * s, double target, double rate, bool instant) {
    if (!s->started || instant) {
        s->started = true;
        s->current = s->target = target;
        s->left    = 0.0;
    } else if (target != s->target) {
        s->target = target;
        s->left   = SMOOTH_S * rate;
        s->step   = (target - s->current) / s->left;
    }
}

static double wrap1(double p);

// notes §29: k for each stream, in semitones from the note: Formant from 64; the Formant models (6, 7) hold it at a
// fixed pitch, middle C at 64; the Complex ones (5, 7) spread the streams by Sync, 3/8 of a semitone a step
static void grain_ratios(int model, double formant, double sync, double noteSemitones, double k[2]) {
    double semis = formant - 64.0 + ((model >= 6) ? (60.0 - noteSemitones) : 0.0);
    double apart = (model & 1) ? (0.375 * sync) : 0.0;

    k[0] = pow(2.0, (semis + apart) / 12.0);
    k[1] = pow(2.0, (semis - apart) / 12.0);
}

// notes §29: the half-rate phase, and each stream's drift away from it by Detune
static void advance_grain(double g[3], double inc, const double k[2], double down, double up, bool pair) {
    if (!pair) {
        return;
    }
    g[0] = wrap1(g[0] + (0.5 * inc));
    g[1] = wrap1(g[1] + (inc * k[0] * (up - 1.0)));
    g[2] = wrap1(g[2] + (inc * k[1] * (down - 1.0)));
}

static double smooth_advance(tTiSmooth * s, uint32_t frames) {
    if (s->left > (double)frames) {
        s->current += s->step * frames;
        s->left    -= frames;
    } else {
        s->current = s->target;
        s->left    = 0.0;
    }
    return s->current;
}

// notes §6: the two filters' coefficients and how their outputs mix, for one sub-block
typedef struct {
    tTiFilterCoeffs f1, f2;
    tTiRouting      routing;
    double          mix;       // serial: Filter 1's share in or around Filter 2; parallel: Filter 1's level
    tTiFilterCoeffs f1b;       // notes §6: Filter 1's second section in Serial 6
    uint8_t         curve;     // notes §17: Saturation Curve
    double          drive;     // 0..1
} tTiFilterSet;

#define RING_DC             (0.0006)

#ifndef SAT_UNIT
#define SAT_UNIT            TI_FIXED_UNIT
#define SAT_RECTIFIER_DC    (14.7)
#endif

// notes §17: nine points across the drive, in between a straight line
static double drive_table(const double * t, double drive) {
    double p = fmin(fmax(drive, 0.0), 1.0) * 8.0;
    int    i = (int)p;

    i = (i > 7) ? 7 : i;
    return t[i] + ((p - i) * (t[i + 1] - t[i]));
}

#define SAT_SHAPER    (206.0)

// notes §17: 18 dB across the drive
static double sat_input(double drive) {
    return 0.125 * pow(10.0, 0.9 * drive);
}

static const double kSatMakeUp[5][9] = {
    {   1.0, 0.8516, 0.6953, 0.5781, 0.4844, 0.4062, 0.3438, 0.2969, 0.2656},
    {   1.0, 0.8516, 0.6953, 0.5781, 0.4844, 0.4141, 0.3516, 0.3125, 0.2812},
    {   1.0, 0.8672, 0.7422, 0.6562, 0.5859, 0.5312, 0.4844, 0.4609, 0.4453},
    {   1.0, 0.9219, 0.8516, 0.7969,   0.75, 0.7031, 0.6641, 0.6406,  0.625},
    {0.0234, 0.0234,   0.02, 0.0171, 0.0151, 0.0137, 0.0142, 0.0146, 0.0151}
};

// notes §17: each side of zero is an arctangent of its own, peak and steepness per curve
static double shape(int curve, double u) {
    static const double kPeak[4][2]  = {{0.3125, 0.625}, {0.13125, 1.05}, {0.09375, 0.375}, {0.08125, 0.1625}};
    static const double kSteep[4][2] = {{43.52, 21.76}, {102.4, 12.8}, {204.8, 51.2}, {409.6, 204.8}};
    int                 side         = (u < 0.0) ? 1 : 0;

    return (2.0 / M_PI) * kPeak[curve][side] * atan(kSteep[curve][side] * u);
}

// notes §17: drive into a curve, make-up gain out of it
double ti_saturate_curve(double x, uint8_t curve, double drive, double unit) {
    if (curve <= 4) {
        double u = fmin(fmax(x * unit * sat_input(drive), -0.99999), 0.99999);

        return drive_table(kSatMakeUp[curve - 1], drive) * shape(curve - 1, u) / unit;
    }
    double g = sat_input(drive);
    double u = fmin(fmax(x * unit * g * g * 1024.0, -1.0), 1.0);

    return 4.0 * drive_table(kSatMakeUp[4], drive) * u / unit;
}

#define HOLD_LONGEST      (131.0)         // notes §25
#define SAT_KEY_FOLLOW    (0.535 / 128.0) // notes §17: a key-followed filter curve's control, per semitone

// notes §17: a filter curve's corner point i, on the digital filters' third-octave points, as 1 - its bilinear pole, halved
static double sat_corner_q(int i, double rate) {
    double hz = (i >= 31) ? 22968.75 : (1000.0 * pow(2.0, (i - 17) / 3.0));
    double t  = tan(M_PI * fmin(hz, 0.49 * rate) / rate);

    return t / (1.0 + t);
}

static double saturate(double x, uint8_t curve, double drive, double * state, double rate, double pitch) {
    if ((curve >= 1) && (curve <= 5)) {   // Digital, 5: a hard limit far below full scale
        return ti_saturate_curve(x, curve, drive, SAT_UNIT);
    }

    if (curve == 6) {                       // Shaper: a sine of the driven signal
        return 0.358 * (1.0 - (0.25 * drive)) * sin(x * SAT_UNIT * sat_input(drive) * SAT_SHAPER);
    }

    if (curve == 7) {                       // Rectifier: from the signal to its folded form
        double u = fmin(fmax(x * SAT_UNIT, -1.0), 1.0);
        double y = ((0.975 * (1.0 - (drive * drive)) * u) + (drive * (1.0 + (0.96 * drive)) * (0.638 - fabs(u)))) / SAT_UNIT;

        state[1] += (1.0 - exp(-2.0 * M_PI * SAT_RECTIFIER_DC / rate)) * (y - state[1]);
        return y - state[1];
    }

    switch (curve) {
        case 8:              // Bit Reducer: notes §17, the step over nine points of drive
        {
            static const double kBits[9] = {16.0, 32.0, 64.0, 128.0, 192.0, 256.0, 384.0, 512.0, 1024.0};
            double              at       = 8.0 * fmin(fmax(drive, 0.0), 1.0);
            int                 i        = (at >= 8.0) ? 7 : (int)at;
            double              inverse  = 1.0 / (kBits[i] + ((at - i) * (kBits[i + 1] - kBits[i])));   // interpolated as 1/step
            double              step     = 1.0 / (16384.0 * inverse * SAT_UNIT);

            return floor((x / step) + 0.5) * step;
        }
        case 9:              // Rate Reducer, and with key follow
        case 10:
        {
            // notes §25
            double hold = (rate / 48000.0) * ((curve == 10) ? fmin(ti_dist_hold(127.0 * drive) * pow(2.0, (KEY_CENTRE - pitch) / 12.0), HOLD_LONGEST) : ti_dist_hold(127.0 * drive));

            state[1] += 1.0;

            if (state[1] >= hold) {
                state[1] -= hold;
                state[0]  = x;
            }
            return state[0];
        }

        case 11:             // notes §17: Low Pass and High Pass, and each with key follow
        case 12:
        case 13:
        case 14:
        {
            bool   low     = (curve <= 12);
            double control = (low ? (1.0 - drive) : drive) + (((curve & 1) == 0) ? (SAT_KEY_FOLLOW * (pitch - KEY_CENTRE)) : 0.0);
            double point   = 31.0 * fmin(fmax(control, 0.0), 1.0);
            int    i       = (point >= 31.0) ? 30 : (int)point;
            double q       = sat_corner_q(i, rate) + ((point - i) * (sat_corner_q(i + 1, rate) - sat_corner_q(i, rate)));
            double c       = 1.0 - (2.0 * q);
            double y       = (low ? (q * (x + state[1])) : ((1.0 - q) * (x - state[1]))) + (c * state[0]);

            state[1] = x;
            state[0] = y;
            return y;
        }

        default:
            return x;
    }
}

#define MIX_F1_ONLY     (0.99951171875)     // notes §6: at Balance 0 in series Filter 2 is left out
#define MIX_DRY_GAIN    (0.70703125)

// notes §6: the centre-detented 9-point curves Balance and the linked Cutoff 2 run through
static const double kBalanceCurve[9] = {-1.0, -0.6875, -0.375, -0.125, 0.0, 0.125, 0.375, 0.6875, 1.0};
static const double kOffsetCurve[9]  = {-1.0, -0.5, -0.25, -0.0625, 0.0, 0.0625, 0.25, 0.5, 1.0};

static double curve9(const double * curve, double value) {
    double v = fmin(fmax(value, 0.0), 127.0) / 128.0;

    if (v >= (126.0 / 128.0)) {
        v = (2.0 * v) - (126.0 / 128.0);    // the top two steps stretched to reach the curve's end
    }
    double p = fmin(8.0 * v, 7.9999);
    int    i = (int)p;

    return curve[i] + ((p - i) * (curve[i + 1] - curve[i]));
}

double ti_voices_cutoff2(double cutoff, double cutoff2, bool link) {
    return fmax(link ? (cutoff + (128.0 * curve9(kOffsetCurve, cutoff2))) : cutoff2, 0.0);
}

static void scale_numerator(tTiFilterCoeffs * c, double g) {
    c->b0 *= g;
    c->b1 *= g;
    c->b2 *= g;
}

// notes §6: Serial 6's band pass gain, by resonance row (of 64)
static const double kSerial6BpGain[9] = {5.0, 6.0, 8.0, 12.0, 16.0, 24.0, 32.0, 48.0, 64.0};

static double serial6_bp_gain(double resonance) {
    double row = 8.0 * fmin(fmax(resonance, 0.0), 127.999) / 128.0;
    int    r   = (row >= 8.0) ? 7 : (int)row;

    return kSerial6BpGain[r] + ((row - r) * (kSerial6BpGain[r + 1] - kSerial6BpGain[r]));
}

static tTiFilterMode filter_mode(uint8_t byte, uint8_t last) {
    return (tTiFilterMode)((byte > last) ? last : byte);
}

static void filter_set(const tTiVoices * v, const tTiSound * s, double c1, double res, double c2, double res2, double balance, double drive,
                       tTiFilterSet * set) {
    tTiRouting    routing = (tTiRouting)(s->a[53] & 3);
    tTiFilterMode mode1   = filter_mode(s->a[51], eTiFilterAnalog4);
    tTiFilterMode mode2   = filter_mode(s->a[52], eTiFilterBandStop);
    double        bal     = curve9(kBalanceCurve, balance);
    bool          serial  = (routing == eTiRoutingSerial4) || (routing == eTiRoutingSerial6);

    // notes §6: after an analog Filter 1, Filter 2's resonance stops at 100
    if ((mode1 >= eTiFilterAnalog1) && (mode2 != eTiFilterBandPass)) {
        res2 = fmin(res2, 100.0);
    }
    double        reduced = res2;
    bool          six     = (routing == eTiRoutingSerial6) && (mode1 < eTiFilterAnalog1);
    double        res1    = six ? (0.625 * fmax(res, 0.0)) : res;   // notes §6: Serial 6's two sections, at 5/8 resonance

    if (serial) {
        // notes §6: two cutoffs close together take Filter 2's resonance down
        double window = (routing == eTiRoutingSerial6) ? 256.0 : 128.0;
        double apart  = 60.0 * fabs(c2 - c1);

        if (apart < window) {
            reduced *= 1.0 + ((3.0 * (apart - window) / 1024.0) * (res1 / 128.0));
        }
    }
    ti_filter_coeffs(&v->filterTables, mode1, c1, res1, drive, &set->f1);
    ti_filter_coeffs(&v->filterTables, mode1, c1, (routing == eTiRoutingSerial6) ? res1 : res, drive, &set->f1b);

    // notes §6: in Serial 6 the poles take 5/8 of the resonance but the gain the whole of it, on the first section only
    if (six && (mode1 == eTiFilterBandPass)) {
        double unit = 1.0 / ti_filter_row_gain(mode1, res1);

        scale_numerator(&set->f1, unit * serial6_bp_gain(res) / 16.0);
        scale_numerator(&set->f1b, unit * 4.0);
    } else if (six) {
        scale_numerator(&set->f1, ti_filter_row_gain(mode1, res) / ti_filter_row_gain(mode1, res1));
        scale_numerator(&set->f1b, 1.0 / ti_filter_row_gain(mode1, res1));
    } else if (routing == eTiRoutingSerial6) {
        ti_filter_coeffs(&v->filterTables, mode1, c1, 0.0, drive, &set->f1b);   // notes §6: the analog modes as before
    }
    set->curve   = s->a[49];
    set->drive   = drive;
    ti_filter_coeffs(&v->filterTables, mode2, c2, reduced, 0.0, &set->f2);

    // notes §6: the reduced resonance shapes the peak, but the output gain stays the unreduced one's (not in band pass)
    if ((reduced != res2) && (mode2 != eTiFilterBandPass)) {
        scale_numerator(&set->f2, ti_filter_row_gain(mode2, res2) / ti_filter_row_gain(mode2, reduced));
    }

    // notes §6: after an analog Filter 1, Filter 2 keeps its level as resonance rises
    if ((mode1 >= eTiFilterAnalog1) && (mode2 != eTiFilterBandPass)) {
        scale_numerator(&set->f2, 1.0 / ti_filter_row_gain(mode2, res2));
    }
    set->routing = routing;

    // notes §6: Filter 1's share (mix) and Filter 2's gain, each a straight line in bal on either side of the centre
    if (serial) {
        set->mix = -bal;
        scale_numerator(&set->f2, (bal < 0.0) ? (0.70703 * (1.0 + bal)) : 0.70703);
    } else if (routing == eTiRoutingParallel4) {
        set->mix = 2.0 * ((bal < 0.0) ? (0.17675 - (0.07324 * bal)) : (0.17675 * (1.0 - bal)));
        scale_numerator(&set->f2, (bal < 0.0) ? (0.35352 * (1.0 + bal)) : (0.35352 + (0.14648 * bal)));
    } else {
        set->mix = (bal < 0.0) ? (0.70703 - (0.29297 * bal)) : (0.70703 * (1.0 - bal));
        scale_numerator(&set->f2, (bal < 0.0) ? (0.70703 * (1.0 + bal)) : (0.70703 + (0.29297 * bal)));
    }
    ti_filter_set_fixed(&set->f1, v->fixedPoint);
    ti_filter_set_fixed(&set->f1b, v->fixedPoint);
    ti_filter_set_fixed(&set->f2, v->fixedPoint);
}

// notes §6
static double filters_process(tTiVoice * vc, const tTiFilterSet * set, double raw, double raw2, double * right, double rate) {
    double m  = set->mix;

    if ((m == 0.0) && (set->routing == eTiRoutingParallel4)) {   // notes §6: Filter 1 has no share
        *right = 0.0;
        return ti_filter_process(&vc->filter2, &set->f2, raw);
    }
    double y1 = ti_filter_process(&vc->filter, &set->f1, raw);

    if (set->routing == eTiRoutingSerial6) {
        y1 = ti_filter_process(&vc->filterB, &set->f1b, y1);
    }

    // notes §17: the analog modes have their own drive, and no saturation stage
    if (set->f1.poles == 0u) {
        y1 = saturate(y1, set->curve, set->drive, vc->satState, rate, vc->pitch);
    }

    // notes §6: Split sends each oscillator its own way, Filter 1 left and Filter 2 right
    if (set->routing == eTiRoutingSplit) {
        *right = SPLIT_GAIN * ti_filter_process(&vc->filter2, &set->f2, raw2);
        return SPLIT_GAIN * m * y1;
    }
    *right = 0.0;

    if ((set->routing == eTiRoutingSerial4) || (set->routing == eTiRoutingSerial6)) {
        if (m >= MIX_F1_ONLY) {
            return MIX_DRY_GAIN * m * y1;
        }

        if (m > 0.0) {
            return ti_filter_process(&vc->filter2, &set->f2, y1) + (MIX_DRY_GAIN * m * y1);
        }
        return ti_filter_process(&vc->filter2, &set->f2, ((1.0 + m) * y1) - (m * raw));
    }
    return ti_filter_process(&vc->filter2, &set->f2, raw) + (m * y1);
}

// notes §7: Patch Volume, in dB at each eighth step of the dial below 48, then a straight line
static double patch_volume(double dial) {
    static const double kDb[7] = {-90.0, -51.2, -41.5, -34.3, -29.5, -24.6, -19.9};

    dial = clamp(dial, 0.0, 127.0);

    if (dial >= 48.0) {
        return pow(10.0, -(127.0 - dial) * 0.2513 / 20.0);
    }
    int                 i      = (int)(dial / 8.0);

    return (dial <= 0.0) ? 0.0 : pow(10.0, (kDb[i] + (((dial / 8.0) - i) * (kDb[(i < 6) ? (i + 1) : 6] - kDb[i]))) / 20.0);
}

// notes §7
static double mix_level(double dial) {
    return (dial <= 0.0) ? 0.0 : pow(ti_env_sustain_level(clamp(dial, 0.0, 127.0)), 1.25);
}

static double osc_volume(double dial) {
    return (dial <= 0.0) ? 0.0 : ti_env_sustain_level(2.0 * clamp(dial, 0.0, 64.0));
}

// notes §7: two straight lines, meeting at the centre
static double balance_1(double b) {
    b = clamp(b, 0.0, 127.0);
    return (b <= 64.0) ? (1.0 - (0.2057 * b / 64.0)) : ((b >= 127.0) ? 0.0 : (0.7943 * (128.0 - b) / 64.0));
}

static double balance_2(double b) {
    b = clamp(b, 0.0, 127.0);
    return (b <= 64.0) ? (0.798 * b / 64.0) : (0.798 + (0.2043 * (b - 64.0) / 63.0));
}

// notes §7: Osc 2 Detune in cents at note 60
static double detune_cents(double dial) {
    static const double kAt[10] = {0, 8, 16, 32, 48, 64, 80, 96, 112, 127};
    static const double kCt[10] = {0, 0.272, 0.568, 2.339, 4.699, 7.055, 9.409, 17.76, 27.28, 43.29};

    dial = clamp(dial, 0.0, 127.0);

    for (int i = 1; i < 10; i++) {
        if (dial <= kAt[i]) {
            return kCt[i - 1] + ((kCt[i] - kCt[i - 1]) * (dial - kAt[i - 1]) / (kAt[i] - kAt[i - 1]));
        }
    }

    return kCt[9];
}

// notes §14: FM's depth, of Osc 2's frequency, from an amount 0..1
static double fm_depth(double amount) {
    static const double kAt[5] = {0, 32, 64, 96, 127};
    static const double kTo[5] = {0, 1.0, 4.0, 8.0, 15.75};
    double              d      = fmin(fabs(amount) * 127.0, 127.0);

    for (int i = 1; i < 5; i++) {
        if (d <= kAt[i]) {
            double v = kTo[i - 1] + ((kTo[i] - kTo[i - 1]) * (d - kAt[i - 1]) / (kAt[i] - kAt[i - 1]));

            return (amount < 0.0) ? -v : v;
        }
    }

    return (amount < 0.0) ? -kTo[4] : kTo[4];
}

// notes §11: Env Amount, in dial units of cutoff at a full envelope
static double env_amount(double dial) {
    static const double kAt[4] = {0, 32, 64, 127};
    static const double kTo[4] = {0, 16, 48, 126.75};

    dial = clamp(dial, 0.0, 127.0);

    for (int i = 1; i < 4; i++) {
        if (dial <= kAt[i]) {
            return kTo[i - 1] + ((kTo[i] - kTo[i - 1]) * (dial - kAt[i - 1]) / (kAt[i] - kAt[i - 1]));
        }
    }

    return kTo[3];
}

// notes §11: a filter envelope's attack, of full travel per second at each fourth step of the dial
static double attack_rate(double dial) {
    static const double kFrame    = 0.0058057;
    static const double kRate[33] = {
        1.0,        1.0,     1.0, 0.26676, 0.13312, 0.09407, 0.06649,   0.047, 0.03817,   0.031, 0.02518, 0.02045, 0.01661, 0.01396, 0.01174, 0.00987,
        0.0083, 0.00698, 0.00587, 0.00494, 0.00415, 0.00349, 0.00293, 0.00246, 0.00208, 0.00169, 0.00137, 0.00111,  0.0009, 0.00068, 0.00052, 0.00039, 0.00029
    };
    double              p         = clamp(dial, 0.0, 127.0) / 4.0;
    int                 i         = (int)p;

    i = (i > 31) ? 31 : i;
    return (kRate[i] + ((p - i) * (kRate[i + 1] - kRate[i]))) / kFrame;
}

// notes §11: velocity scales the envelope's offset, from full at the loudest (or softest) downward
static double velocity_scale(uint8_t byte, double velocity) {
    double a = curve9(kBalanceCurve, byte);

    return (a >= 0.0) ? (1.0 - (a * (1.0 - velocity))) : (1.0 + (a * velocity));
}

// notes §24
static double pitch_offset(uint8_t semi, double amount) {
    return clamp(((double)semi - 64.0) + (PITCH_MOD_DEPTH * amount), -PITCH_MOD_LIMIT, PITCH_MOD_LIMIT);
}

static double key_pitch(double pitch, uint8_t keyfollow) {
    return KEY_CENTRE + ((pitch - KEY_CENTRE) * ((double)keyfollow - 64.0) / 32.0);
}

// notes §7: free-running oscillators sit a shade flat of a fixed starting phase's
static double semitones_to_increment(const tTiVoices * v, double semitones, double note) {
    double cents = (v->sound.b[35] > 0) ? 0.29 : (TUNE_CENTS * pow(TUNE_SLOPE, -(note - KEY_CENTRE) / 24.0));

    return 440.0 * pow(2.0, (semitones - 69.0 + (cents / 100.0)) / 12.0) / v->rate;
}

static double source_value(const tTiVoices * v, const tTiVoice * vc, const double * lfo, uint8_t source) {
    if (source == 0) {
        return 0.0;
    }

    if (source <= eTiCtlCount) {
        return v->control[source - 1];
    }

    switch (source) {
        case kSrcAmpEnv:     return vc->amp.level;

        case kSrcFilterEnv:  return vc->filterEnv.level;

        case kSrcLfo1:       return 0.5 * lfo[0];   // notes §9

        case kSrcLfo2:       return 0.5 * lfo[1];

        case kSrcLfo3:       return 0.5 * lfo[2];

        case kSrcVelOn:      return vc->velocity;

        case kSrcVelOff:     return vc->releaseVelocity;

        case kSrcArp:        return -0.5;              // notes §9: with the arpeggiator off

        case kSrcKey:        return ((double)vc->note - 60.0) / 63.0;

        case kSrcRandom:     return vc->random;

        case kSrcLfo1Uni:    return 0.5 + (0.5 * lfo[0]);

        case kSrcLfo2Uni:    return 0.5 + (0.5 * lfo[1]);

        case kSrcLfo3Uni:    return 0.5 + (0.5 * lfo[2]);

        case kSrc1Percent:   return 0.012;

        case kSrc10Percent:  return 0.1024;

        case kSrcEnv3:       return vc->env3.level;

        case kSrcEnv4:       return vc->env4.level;

        default:             return 0.0;
    }
}

static void lfo_shape(const tTiVoices * v, const tTiSound * s, int l, const double * mod, tTiLfoShape * out) {
    static const double kDivide[22] = {
        0,        1.0 / 64, 1.0 / 32, 1.0 / 16, 1.0 / 8, 1.0 / 4, 1.0 / 2, 3.0 / 64, 3.0 / 32, 3.0 / 16, 3.0 / 8,
        1.0 / 24, 1.0 / 12,  1.0 / 6,  1.0 / 3, 2.0 / 3, 3.0 / 4,     1.0,      2.0,      4.0,      8.0, 16.0
    };
    uint8_t             rate        = (l == 0) ? s->a[67] : ((l == 1) ? s->a[79] : s->b[7]);
    uint8_t             shape       = (l == 0) ? s->a[68] : ((l == 1) ? s->a[80] : s->b[8]);
    uint8_t             cont        = (l == 0) ? s->a[71] : ((l == 1) ? s->a[83] : 64);
    uint8_t             env         = (l == 0) ? s->a[69] : ((l == 1) ? s->a[81] : 0);
    uint8_t             clock       = (l == 0) ? s->b[18] : ((l == 1) ? s->b[19] : s->b[21]);
    int                 dstR        = (l == 0) ? kDstLfo1Rate : ((l == 1) ? kDstLfo2Rate : kDstLfo3Rate);

    out->rateHz   = ti_lfo_rate_hz(clamp(rate + (MOD_DEPTH * mod[dstR]), 0.0, 127.0));

    if ((clock > 0) && (clock < 22)) {
        out->rateHz = (((v->tempo > 0.0) ? v->tempo : (63.0 + s->b[16])) / 60.0) / (4.0 * kDivide[clock]);    // notes §12: the patch's own tempo with no host
    }
    out->shape    = (shape > 67) ? 67 : shape;
    out->contour  = clamp(bipolar(cont) + ((l < 2) ? mod[(l == 0) ? kDstLfo1Contour : kDstLfo2Contour] : 0.0), -1.0, 1.0);
    out->envMode  = (env != 0);
    out->fadeStep = 0.0;

    if ((l == 2) && (s->b[13] > 0)) {
        out->fadeStep = (32.0 / 46875.0) / fmax(ti_lfo_rate_hz(127.0 - s->b[13]) > 0.0 ? 1.0 / ti_lfo_rate_hz(127.0 - s->b[13]) : 1.0, 0.001);
    }
}

// notes §22: which destination moves each slot's amount
static const uint8_t kSlotAmount[6][3] = {{82, 93, 94}, {83, 84, 80}, {85, 86, 87}, {120, 76, 77}, {121, 30, 31}, {122, 75, 10}};

static bool slot_amount_dst(uint8_t dst) {
    for (int p = 0; p < 6; p++) {
        for (int d = 0; d < 3; d++) {
            if (kSlotAmount[p][d] == dst) {
                return true;
            }
        }
    }

    return false;
}

static uint8_t modded(uint8_t byte, double amount) {
    return (uint8_t)lround(clamp(byte + (MOD_DEPTH * amount), 0.0, 127.0));
}

// The matrix and the fixed LFO routes; mod[] holds each destination's sum, -1..1 of full travel
static void voice_mods(const tTiVoices * v, const tTiSound * s, const tTiVoice * vc, const double * lfo, double * mod) {
    memset(mod, 0, sizeof(double) * kDstCount);

    // notes §22: a slot may move another slot's amount, so those routes are summed first
    double extra[kDstCount] = {0.0};

    for (int pass = 0; pass < 2; pass++) {
        for (int p = 0; p < 6; p++) {
            uint8_t source = page_byte(s, kSlots[p].source);

            if (source == 0) {
                continue;
            }
            double  value  = source_value(v, vc, lfo, source);

            for (int d = 0; d < 3; d++) {
                uint8_t dst    = page_byte(s, kSlots[p].slot[d][0]);

                if ((dst == 0) || (dst >= kDstCount)) {
                    continue;
                }
                double  amount = bipolar(page_byte(s, kSlots[p].slot[d][1]));

                if (pass == 0) {
                    extra[dst] += slot_amount_dst(dst) ? (value * amount) : 0.0;
                } else {
                    mod[dst] += value * (amount + extra[kSlotAmount[p][d]]);
                }
            }
        }
    }

    if ((s->b[79] != 0) && (s->b[79] < kDstCount)) {
        mod[s->b[79]] += 0.5 * lfo[0] * (bipolar(s->b[80]) + mod[kDstLfo1Assign]);
    }

    if ((s->b[81] != 0) && (s->b[81] < kDstCount)) {
        mod[s->b[81]] += 0.5 * lfo[1] * (bipolar(s->b[82]) + mod[kDstLfo2Assign]);
    }
}

// notes §12: semitones of pitch at an amount, the same curve either side of zero
static double lfo_pitch_amount(double amount) {
    static const double kAt[6] = {0, 0.125, 0.25, 0.5, 0.75, 0.984};
    static const double kTo[6] = {0, 0.092, 0.307, 1.987, 7.886, 31.27};
    double              a      = fabs(amount);

    for (int i = 1; i < 6; i++) {
        if (a <= kAt[i]) {
            double v = kTo[i - 1] + ((kTo[i] - kTo[i - 1]) * (a - kAt[i - 1]) / (kAt[i] - kAt[i - 1]));

            return (amount < 0.0) ? -v : v;
        }
    }

    return (amount < 0.0) ? -kTo[5] : kTo[5];
}

static double wrap1(double p) {
    return p - floor(p);
}

// notes §7: the noise, coloured below the centre by a low pass and above it by a shelf away from the lows
static double noise_sample(tTiVoice * vc, double color, double rate) {
    vc->noise = (vc->noise * 1664525u) + 1013904223u;
    double x = ((double)(int32_t)vc->noise) / 2147483648.0;

    if (color < 64.0) {
        double hz = 350.0 * pow(6.28, color / 32.0);
        double a  = 1.0 - exp(-2.0 * M_PI * hz / rate);
        double f  = color / 64.0;

        vc->noiseState += a * (x - vc->noiseState);
        return vc->noiseState * pow(10.0, 7.5 * (1.0 - (f * f)) / 20.0);
    }

    if (color > 64.0) {
        double hz    = 350.0 * pow(8.6, (color - 96.0) / 31.0);
        double a     = 1.0 - exp(-2.0 * M_PI * hz / rate);
        double floor = pow(10.0, -23.0 * ((color - 64.0) / 63.0) / 20.0);

        vc->noiseState += a * (x - vc->noiseState);
        return x - ((1.0 - floor) * vc->noiseState);
    }
    return x;
}

static void render_voice(tTiVoices * v, const tTiSound * s, tTiVoice * vc, const tTiFilterSet * set, const tTiEnvShape * amp, const tTiEnvShape * fenv,
                         const tTiEnvShape * e3, const tTiEnvShape * e4, const double * mod, float * left, float * right, uint32_t frames, double levelL, double levelR) {
    double   note1     = key_pitch(vc->pitch, s->a[21]) + ((double)s->a[93] - 64.0) + (mod[kDstTranspose] * 48.0);
    double   note2     = key_pitch(vc->pitch, s->a[31]) + ((double)s->a[93] - 64.0) + (mod[kDstTranspose] * 48.0);
    double   scale     = pow(DETUNE_SLOPE, -(vc->pitch - KEY_CENTRE) / 24.0);
    double   unison    = vc->detune;
    double   bend      = fabs(v->control[eTiCtlBend]) * ((v->control[eTiCtlBend] >= 0.0) ? bipolar(s->b[26]) : bipolar(s->b[27])) * 64.0;
    double   env       = vc->filterEnv.level;
    double   semi1     = note1 + bend + unison + pitch_offset(s->a[20], mod[kDstPitch1]);
    double   semi2     = note2 + bend + unison + pitch_offset(s->a[25], mod[kDstPitch2]) + ((detune_cents(s->a[26] + (127.0 * mod[kDstDetune])) * scale) / 100.0)
                         + (2.0 * lfo_pitch_amount(bipolar(s->a[29]) + mod[kDstEnvPitch]) * env);
    double   semi3     = key_pitch(vc->pitch, s->a[31]) + ((double)s->a[93] - 64.0) + (mod[kDstTranspose] * 48.0) + bend + unison + ((double)s->b[43] - 64.0) + ((detune_cents(s->b[44]) * scale) / 100.0) + (PITCH_MOD_DEPTH * mod[kDstOsc3Pitch]);
    double   inc1      = semitones_to_increment(v, semi1, semi1);
    double   inc2      = semitones_to_increment(v, semi2, semi2);
    double   inc3      = semitones_to_increment(v, semi3, semi3);
    double   shape1    = clamp(s->a[17] + (MOD_DEPTH * (mod[kDstShape1] + mod[kDstIndex1] + mod[kDstLfo2Shape] + (bipolar(s->b[47]) * vc->velocity))), 0.0, 127.0);
    double   shape2    = clamp(s->a[22] + (MOD_DEPTH * (mod[kDstShape2] + mod[kDstIndex2] + mod[kDstLfo2Shape] + (bipolar(s->b[48]) * vc->velocity))), 0.0, 127.0);
    double   width1    = clamp(s->a[18] + (MOD_DEPTH * (mod[kDstWidth1] + mod[kDstLfo1Width] + (bipolar(s->b[49]) * vc->velocity))), -127.0, 127.0);   // notes §23
    double   width2    = clamp(s->a[23] + (MOD_DEPTH * (mod[kDstWidth2] + mod[kDstLfo1Width] + (bipolar(s->b[49]) * vc->velocity))), -127.0, 127.0);   // notes §23
    int      wave1     = (int)clamp(s->a[19] + (63.0 * mod[kDstWave1]), 0.0, 63.0);
    int      wave2     = (int)clamp(s->a[24] + (63.0 * mod[kDstWave2]), 0.0, 63.0);
    double   bal       = s->a[33] + (MOD_DEPTH * mod[kDstOscBalance]);
    double   g1        = balance_1(bal);
    double   g2        = balance_2(bal);
    double   oscLvl    = osc_volume(s->a[36] + (MOD_DEPTH * mod[kDstOscVolume]));
    double   subLvl    = mix_level(s->a[34] + (MOD_DEPTH * mod[kDstSub]));
    double   noise     = mix_level(s->a[37] + (MOD_DEPTH * mod[kDstNoise]));
    double   color     = clamp(s->a[39] + (MOD_DEPTH * mod[kDstNoiseColor]), 0.0, 127.0);
    double   ring      = RING_GAIN * mix_level((double)((s->a[0] < TI_FORMAT) ? s->a[38] : s->a[50]) + (MOD_DEPTH * mod[kDstRing]));
    double   fmAmt     = fmax((double)s->a[27] / 127.0 + mod[kDstFm] + (bipolar(s->b[50]) * vc->velocity) + mod[kDstLfo2Fm] + ((bipolar(s->a[30]) + mod[kDstEnvFm]) * env), 0.0);
    double   fm        = (s->c[35] == 1) ? 0.0 : fm_depth(fmAmt); // notes §21: a HyperSaw Osc 2 reads it as sync
    double   syncUp    = ((s->c[35] == 1) && (fmAmt > 0.0)) ? pow(2.0, fmin(fmAmt * 127.0, 128.0) / 32.0) : 0.0;
    uint8_t  mode3     = s->b[41];
    double   lvl3      = (mode3 == 1) ? 0.0 : (OSC3_GAIN * clamp(s->b[42] + (MOD_DEPTH * mod[kDstOsc3Volume]), 0.0, 127.0));
    bool     sync      = (s->a[28] != 0);
    bool     sqSub     = (s->a[35] == 0);
    uint8_t  fmode     = (s->b[34] > 3) ? 0 : s->b[34];
    double   hp        = 1.0 - exp(-2.0 * M_PI * DC_BLOCK_HZ / v->rate);
    double   pAmt      = clamp((s->b[36] + (MOD_DEPTH * mod[kDstPunch])) / 127.0, 0.0, 1.0);
    double   punch     = PUNCH_LEVEL * pow(pAmt, 1.6);
    double   punchTau  = PUNCH_TAU * pow(2.0, (36.0 - (double)vc->note) / PUNCH_OCTAVE);

    bool     table1    = (s->c[30] == 2);
    bool     table2    = (s->c[35] == 2);
    bool     pwm1      = (s->c[30] == 3);   // notes §28: Wave PWM
    bool     pwm2      = (s->c[35] == 3);
    double   twinDown1 = 0.0, twinUp1 = 0.0, twinDown2 = 0.0, twinUp2 = 0.0;

    if (pwm1) {
        ti_osc_pwm_detune(s->c[43], &twinDown1, &twinUp1);
    }

    if (pwm2) {
        ti_osc_pwm_detune(s->c[63], &twinDown2, &twinUp2);
    }
    // notes §29: Grain (4, 5) and Formant (6, 7); the Complex ones (5, 7) run two streams
    int      grain1    = ((s->c[30] >= 4) && (s->c[30] <= 7)) ? s->c[30] : 0;
    int      grain2    = ((s->c[35] >= 4) && (s->c[35] <= 7)) ? s->c[35] : 0;
    double   gk1[2], gk2[2], gDown1 = 1.0, gUp1 = 1.0, gDown2 = 1.0, gUp2 = 1.0;

    if (grain1) {
        grain_ratios(grain1, s->c[42], s->c[37], semi1, gk1);
        ti_osc_pwm_detune(s->c[43], &gDown1, &gUp1);
        gDown1 = 1.0 / gUp1;    // notes §29: the streams part evenly, unlike Wave PWM's copies
    }

    if (grain2) {
        grain_ratios(grain2, s->c[62], s->c[57], semi2, gk2);
        ti_osc_pwm_detune(s->c[63], &gDown2, &gUp2);
        gDown2 = 1.0 / gUp2;
    }
    bool     hyper1    = (s->c[30] == 1);
    bool     hyper2    = (s->c[35] == 1);
    tTiHyper stack1, stack2;

    // notes §4: Shape and Pulse Width are Density and Spread here
    if (hyper1) {
        ti_osc_hyper(shape1, width1, &stack1);
    }

    if (hyper2) {
        ti_osc_hyper(shape2, width2, &stack2);
    }
    double   syncLevel = 1.0;

    // notes §21: restarting together the saws add in step; the stack keeps the level of one
    if (hyper2 && (syncUp > 0.0)) {
        double sum = 0.0;

        for (int k = 0; k < stack2.count; k++) {
            sum += stack2.gain[k];
        }

        syncLevel = 1.0 / sum;
    }
    // notes §7: what cannot be heard is not computed; the phases run on regardless
    bool     split     = (set->routing == eTiRoutingSplit);
    bool     useFm     = (fm != 0.0);
    bool     useO2     = (g2 != 0.0) || (ring > 0.0);
    bool     useO1     = (g1 != 0.0) || (ring > 0.0) || (useFm && (fmode == 2));
    bool     useO3     = (mode3 != 0) && (lvl3 != 0.0);

    for (uint32_t f = 0; f < frames; f++) {
        double o1        = 0.0;

        if (!useO1) {
            // silent
        } else if (hyper1) {
            o1 = ti_osc_hyper_sample(&stack1, vc->hyper[0], inc1);
        } else if (grain1 & 1) {
            o1 = ti_osc_grain_pair(s->a[19], shape1, s->c[44], gk1, vc->grain[0], inc1);
        } else if (grain1) {
            o1 = ti_osc_grain(s->a[19], shape1, s->c[44], gk1[0], vc->phase[0], inc1);
        } else if (pwm1) {
            o1 = ti_osc_wave_pwm(s->a[19], shape1, s->c[44], vc->twin[0], width1, inc1);
        } else if (table1) {
            o1 = ti_osc_wavetable(s->a[19], shape1, s->c[44], vc->phase[0], inc1);
        } else {
            o1 = ti_osc_classic(wave1, shape1, width1, vc->phase[0], inc1);
        }
        double modulator = 0.0;

        switch (useFm ? fmode : 4) {
            case 1:  modulator = ti_osc_triangle(vc->phase[0]);
                break;
            case 2:  modulator = o1;
                break;
            case 3:  modulator = noise_sample(vc, 64.0, v->rate);
                break;
            case 4:  break;
            default: modulator = 0.5 + (0.5 * ti_osc_triangle(vc->phase[0]));
                break;
        }
        double i2        = inc2 * (1.0 + (fm * modulator));
        double o2        = 0.0;

        if (!useO2) {
            // silent
        } else if (hyper2) {
            o2 = syncLevel * ti_osc_hyper_sample(&stack2, vc->hyper[1], (syncUp > 0.0) ? (i2 * syncUp) : i2);
        } else if (grain2 & 1) {
            o2 = ti_osc_grain_pair(s->a[24], shape2, s->c[64], gk2, vc->grain[1], i2);
        } else if (grain2) {
            o2 = ti_osc_grain(s->a[24], shape2, s->c[64], gk2[0], vc->phase[1], i2);
        } else if (pwm2) {
            o2 = ti_osc_wave_pwm(s->a[24], shape2, s->c[64], vc->twin[1], width2, i2);
        } else if (table2) {
            o2 = ti_osc_wavetable(s->a[24], shape2, s->c[64], vc->phase[1], i2);
        } else {
            o2 = ti_osc_classic(wave2, shape2, width2, vc->phase[1], i2);
        }
        double o3        = 0.0;

        if (useO3) {
            switch (mode3) {
                case 1:  o3 = ti_osc_classic(wave2, shape2, width2, vc->phase[2], inc3);
                    break;
                case 2:  o3 = ti_osc_saw(vc->phase[2], inc3);
                    break;
                case 3:  o3 = ti_osc_square(vc->phase[2], inc3);
                    break;
                case 4:  o3 = ti_osc_sine(vc->phase[2]);
                    break;
                case 5:  o3 = ti_osc_triangle(vc->phase[2]);
                    break;
                default: o3 = ti_osc_wave(mode3 - 4, vc->phase[2], inc3);
                    break;
            }
        }
        double sub       = 0.0;

        if (subLvl > 0.0) {
            sub = sqSub ? (SUB_SQUARE * ti_osc_square(vc->subPhase, 0.5 * inc1) * 2.0) : (SUB_TRIANGLE * ti_osc_triangle(vc->subPhase));
        }
        double side      = split ? 0.0 : 1.0;
        double raw       = oscLvl * ((g1 * o1) + (side * g2 * o2) + (subLvl * sub) + (lvl3 * o3));
        double raw2      = oscLvl * g2 * o2;

        // notes §15: Punch, a click at the start of a note, ahead of the high pass and the filters
        if (vc->punchLeft > 0.0) {
            double age   = (PUNCH_SECONDS * v->rate) - vc->punchLeft;

            // notes §15: the sign of the first sample, without the correction at a wave's edge
            if (age < 1.0) {
                double n1 = (hyper1 || table1 || pwm1 || grain1) ? o1 : ti_osc_classic(wave1, shape1, width1, vc->phase[0] + 1e-7, 1e-9);
                double n2 = (hyper2 || table2 || pwm2 || grain2) ? o2 : ti_osc_classic(wave2, shape2, width2, vc->phase[1] + 1e-7, 1e-9);

                vc->punchSign = (((g1 * n1) + (g2 * n2)) > -1e-5) ? 1.0 : -1.0;
            }
            double click = vc->punchSign * punch * oscLvl * exp(-age / (punchTau * v->rate));

            raw           += click;
            raw2          += click;
            vc->punchLeft -= 1.0;
        }
        // notes §7: the high pass sits on the oscillators, so what ring and noise add keeps its lows
        vc->highPass[0] += hp * (raw - vc->highPass[0]);

        if (split) {
            vc->highPass[1] += hp * (raw2 - vc->highPass[1]);
            raw2            -= vc->highPass[1];
        }
        raw             -= vc->highPass[0];

        if (noise > 0.0) {
            raw += noise * NOISE_LEVEL * noise_sample(vc, color, v->rate);
        }

        if (ring > 0.0) {
            double product = o1 * o2;

            vc->ringDc += RING_DC * (product - vc->ringDc);   // notes §7: the product carries no DC
            raw        += ring * (product - vc->ringDc);
        }
        double r2;
        double y1   = filters_process(vc, set, raw, raw2, &r2, v->rate);
        double y    = ti_env_next(&vc->amp, amp);

        (void)ti_env_next(&vc->filterEnv, fenv);
        (void)ti_env_next(&vc->env3, e3);
        (void)ti_env_next(&vc->env4, e4);
        double gain = y * OUTPUT_GAIN;
        double l = y1, r = (set->routing == eTiRoutingSplit) ? r2 : y1;

        left[f]       += (float)(l * gain * levelL);
        right[f]      += (float)(r * gain * levelR);

        vc->phase[0]  += inc1;
        vc->phase[1]  += i2;
        vc->phase[2]  += inc3;
        vc->subPhase  += 0.5 * inc1;
        vc->twin[0][0] = wrap1(vc->twin[0][0] + (inc1 * twinDown1));
        vc->twin[0][1] = wrap1(vc->twin[0][1] + (inc1 * twinUp1));
        vc->twin[1][0] = wrap1(vc->twin[1][0] + (i2 * twinDown2));
        vc->twin[1][1] = wrap1(vc->twin[1][1] + (i2 * twinUp2));
        advance_grain(vc->grain[0], inc1, gk1, gDown1, gUp1, grain1 & 1);
        advance_grain(vc->grain[1], i2, gk2, gDown2, gUp2, grain2 & 1);

        if (vc->phase[0] >= 1.0) {
            vc->phase[0] -= 1.0;

            if (sync) {
                vc->phase[1]   = vc->phase[0] * (i2 / inc1);  // notes §7: hard, at the sample's own fraction
                vc->twin[1][0] = vc->twin[1][1] = vc->phase[1];
            }

            if (syncUp > 0.0) {
                for (int k = 0; k < stack2.count; k++) {
                    vc->hyper[1][k] = wrap1(vc->phase[0] * syncUp * stack2.ratio[k] * (i2 / inc1));
                }
            }
        }
        vc->phase[1]   = wrap1(vc->phase[1]);
        vc->phase[2]   = wrap1(vc->phase[2]);
        vc->subPhase   = wrap1(vc->subPhase);
    }

    if (ti_env_idle(&vc->amp)) {
        vc->active = false;
        vc->gate   = false;
    }
}

// notes §10: Portamento, an exponential approach whose time constant triples every 32 steps
static double glide_share(double dial, double frames, double rate) {
    return (dial <= 0.0) ? 1.0 : (1.0 - exp(-(frames / rate) / (0.00565 * pow(3.0, dial / 32.0))));
}

static void env_shape_mod(const uint8_t * base, const double * mod, int first, double rate, tTiEnvShape * shape) {
    double m[5] = {0, 0, 0, 0, 0};

    if (first != 0) {
        for (int i = 0; i < 5; i++) {
            m[i] = MOD_DEPTH * mod[first + i];
        }
    }
    ti_env_shape(clamp(base[0] + m[0], 0.0, 127.0), clamp(base[1] + m[1], 0.0, 127.0), clamp(base[2] + m[2], 0.0, 127.0),
                 clamp(base[3] + m[3], 0.0, 127.0), clamp(base[4] + m[4], 0.0, 127.0), rate, shape);
}

// notes §16: Analog Boost, a low shelf at the output; its corner follows Tune and its lift falls as the corner rises
static void analog_boost(tTiVoices * v, const tTiSound * s, float * left, float * right, uint32_t frames) {
    double intensity = s->b[97] / 127.0;
    double tune      = s->b[98] / 127.0;

    if ((intensity <= 0.0) || (s->c[26] != 0)) {    // notes §16: only the Analog Boost type
        return;
    }
    double dB        = 5.96 * ((intensity < 0.504) ? (1.55 * intensity) : (0.78 + (0.44 * (intensity - 0.504))));
    double side      = (tune < 0.252) ? 1.0 : ((tune < 0.504) ? (1.0 - (1.11 * (tune - 0.252))) : (0.72 - (1.03 * (tune - 0.504))));
    double gain      = pow(10.0, dB * side / 20.0) - 1.0;
    double a         = 1.0 - exp(-2.0 * M_PI * (130.0 * pow(23.0, tune)) / v->rate);

    for (uint32_t f = 0; f < frames; f++) {
        v->boostState[0] += a * (left[f] - v->boostState[0]);
        v->boostState[1] += a * (right[f] - v->boostState[1]);
        left[f]          += (float)(gain * v->boostState[0]);
        right[f]         += (float)(gain * v->boostState[1]);
    }
}

// notes §22: destinations that are effect settings, as the byte each one moves
static const struct {
    uint8_t dst, page, at;
} kFxDst[] = {
    { 62, 0, 105}, { 63, 0, 106}, { 64, 0, 107}, {65, 0, 108}, { 66, 0, 109}, { 67, 0, 113}, { 68, 0, 114}, { 69, 0, 115},
    { 70, 0, 116}, { 71, 0, 117}, { 92, 0, 119}, {95, 1, 101}, {102, 1,  85}, {103, 1,  86}, {104, 1,  87}, {105, 1,  88},
    {106, 1,  89}, {117, 1,  92}, {118, 1,  93}, {72, 2,   2}, {108, 2,   4}, {109, 2,   5}, {110, 2,   6}, {111, 2,   7},
    { 96, 2,  21}, { 78, 2,  25}, { 79, 2,  23}, {81, 2,  24},
};

void ti_voices_render(tTiVoices * v, const tTiSound * given, float * left, float * right, uint32_t frames) {
    // notes §5: no Smooth Mode glides; the Quantise values hold a change to the next division of the tempo, then step
    const tTiSound * s         = &v->sound;
    bool             quantise  = quantise_mode(given);
    double           bpm       = (v->tempo > 0.0) ? v->tempo : (63.0 + given->b[16]);
    double           wholes    = quantise ? kQuantiseWhole[given->b[25] - 4] : 1.0;
    double           was       = v->seconds;
    double           beatsWere = v->hostBeats;
    bool             passed    = false;

    v->seconds    += (double)frames / v->rate;

    if (v->hostPlaying) {
        v->hostBeats += ((double)frames / v->rate) * (bpm / 60.0);
    }

    // notes §5: the division passed in this block (a block early at most) - the host's beats while it plays, else the
    // instance's own clock at the tempo; or the transport started
    if (quantise && v->hostPlaying) {
        passed = (floor(v->hostBeats / (4.0 * wholes)) != floor(beatsWere / (4.0 * wholes)));
    } else if (quantise) {
        passed = (floor(v->seconds / ((60.0 / bpm) * 4.0 * wholes)) != floor(was / ((60.0 / bpm) * 4.0 * wholes)));
    }

    if (passed || v->releaseHeld) {
        v->sound = *given;
        v->heard = true;
    } else {
        hear(v, given);
    }
    v->releaseHeld = false;

    smooth_toward(&v->cutoff, s->a[40], v->rate, true);
    smooth_toward(&v->resonance, s->a[42], v->rate, true);
    smooth_toward(&v->cutoff2, s->a[41], v->rate, true);
    smooth_toward(&v->resonance2, s->a[43], v->rate, true);
    smooth_toward(&v->balance, s->a[48], v->rate, true);

    // notes §4: per voice every sub-block, because every voice's keyfollow, envelopes and modulation differ
    for (uint32_t done = 0; done < frames; done += SUB_BLOCK) {
        uint32_t n         = ((frames - done) < SUB_BLOCK) ? (frames - done) : SUB_BLOCK;
        double   cutoff    = smooth_advance(&v->cutoff, n);
        double   res       = smooth_advance(&v->resonance, n);
        double   cutoff2   = smooth_advance(&v->cutoff2, n);
        double   res2      = smooth_advance(&v->resonance2, n);
        double   balance   = smooth_advance(&v->balance, n);
        double   seconds   = n / v->rate;
        double   none[kDstCount];
        double   mono[3];
        bool     isMono[3] = {s->a[70] != 0, s->a[82] != 0, s->b[9] != 0};

        memset(none, 0, sizeof(none));

        for (int l = 0; l < 3; l++) {
            tTiLfoShape shape;

            lfo_shape(v, s, l, none, &shape);
            shape.envMode = false;
            mono[l]       = ti_lfo_tick(&v->monoLfo[l], &shape, seconds);
        }

        uint64_t newest    = 0u;

        for (uint32_t i = 0; i < TI_MAX_VOICES; i++) {
            tTiVoice * vc     = &v->voice[i];

            if (!vc->active) {
                continue;
            }
            double     lfo[3] = {vc->lfo[0].value, vc->lfo[1].value, vc->lfo[2].value};
            double     mod[kDstCount];

            for (int l = 0; l < 3; l++) {
                lfo[l] = isMono[l] ? mono[l] : lfo[l];
            }

            voice_mods(v, s, vc, lfo, mod);

            for (int l = 0; l < 3; l++) {
                if (!isMono[l]) {
                    tTiLfoShape shape;

                    lfo_shape(v, s, l, mod, &shape);
                    lfo[l] = ti_lfo_tick(&vc->lfo[l], &shape, seconds);
                }
            }

            // the fixed routes
            mod[kDstPitch1]    += lfo[0] * lfo_pitch_amount(bipolar(s->a[74]) + mod[kDstLfo1Pitch1]) / PITCH_MOD_DEPTH;
            mod[kDstPitch2]    += lfo[0] * lfo_pitch_amount(bipolar(s->a[75]) + mod[kDstLfo1Pitch2]) / PITCH_MOD_DEPTH;
            mod[kDstLfo1Width] += lfo[0] * bipolar(s->a[76]);
            mod[kDstRes1]      += 0.5 * lfo[0] * curve9(kOffsetCurve, modded(s->a[77], mod[kDstLfo1Res]));
            mod[kDstRes2]      += 0.5 * lfo[0] * curve9(kOffsetCurve, modded(s->a[77], mod[kDstLfo1Res]));
            mod[kDstLfo2Shape] += 0.5 * lfo[1] * bipolar(s->a[86]);
            mod[kDstLfo2Fm]    += lfo[1] * bipolar(s->a[87]);
            mod[kDstCutoff1]   += 0.5 * lfo[1] * curve9(kOffsetCurve, modded(s->a[88], mod[kDstLfo2Cut1])); // notes §12
            mod[kDstCutoff2]   += 0.5 * lfo[1] * curve9(kOffsetCurve, modded(s->a[89], mod[kDstLfo2Cut2]));
            mod[kDstPan]       += 0.5 * lfo[1] * curve9(kOffsetCurve, modded(s->a[90], mod[kDstLfo2Pan]));

            if (s->b[11] < 7) {
                static const int kPitch[7][3] = {{kDstPitch1,          0, 0}, {kDstPitch1, kDstPitch2, 0}, {kDstPitch2, 0, 0}, {kDstWidth1, 0, 0},
                                                 {kDstWidth1, kDstWidth2, 0}, {kDstWidth2,          0, 0}, {         0, 0, 0}};
                double           raw          = fmin(fmax((s->b[12] / 127.0) + mod[kDstLfo3Amount], 0.0), 1.0);
                double           amount       = lfo[2] * ((s->b[11] < 3) ? ((8.0 * raw * raw * raw * raw) / PITCH_MOD_DEPTH) : raw);

                for (int k = 0; k < 2; k++) {
                    if (kPitch[s->b[11]][k] != 0) {
                        mod[kPitch[s->b[11]][k]] += amount;
                    }
                }
            }

            if (vc->started >= newest) {
                newest = vc->started;
                memcpy(v->fxMod, mod, sizeof(v->fxMod));
            }
            // the pitch glide
            vc->pitch          += ((double)vc->note - vc->pitch) * glide_share(s->a[5] + (MOD_DEPTH * mod[kDstPortamento]), n, v->rate);

            // notes §6: the filters, each from its own controls for this voice
            // notes §26: the filters take the envelope as it will stand at the end of the block
            tTiEnvShape  fenv;

            env_shape_mod(&s->a[54], mod, kDstFAttack, v->rate, &fenv);
            fenv.sustain        = clamp(s->a[56] + (MOD_DEPTH * mod[kDstFAttack + 2]), 0.0, 127.0) / 127.0;
            fenv.attackStep     = attack_rate(s->a[54] + (MOD_DEPTH * mod[kDstFAttack])) / v->rate;
            tTiEnv       ahead   = vc->filterEnv;

            for (uint32_t k = 0; k < n; k++) {
                (void)ti_env_next(&ahead, &fenv);
            }

            double       env     = ahead.level;
            double       keyAmt  = KEY_DIAL_PER_SEMI * ((double)vc->pitch + fmin((double)s->a[93] - 64.0, 0.0) - s->b[33]);   // notes §10
            double       sign1   = (s->b[30] == 0) ? -1.0 : 1.0;
            double       sign2   = (s->b[31] == 0) ? -1.0 : 1.0;
            double       vel     = vc->velocity;
            double       amt1    = sign1 * env_amount(s->a[44] + (MOD_DEPTH * mod[kDstEnvAmt1])) * velocity_scale(s->b[54], vel) * env;
            double       amt2    = sign2 * env_amount(s->a[45] + (MOD_DEPTH * mod[kDstEnvAmt2])) * velocity_scale(s->b[55], vel) * env;
            double       key1    = keyAmt * ((double)s->a[46] - 64.0) / 63.0;
            double       key2    = keyAmt * ((double)s->a[47] - 64.0) / 63.0;
            double       c1      = cutoff + (MOD_DEPTH * mod[kDstCutoff1]) + amt1 + key1;
            double       c2      = (s->b[32] != 0) ? (ti_voices_cutoff2(cutoff + (MOD_DEPTH * mod[kDstCutoff1]), cutoff2, true) + (MOD_DEPTH * mod[kDstCutoff2]) + amt2 + key2)
                                            : (cutoff2 + (MOD_DEPTH * mod[kDstCutoff2]) + amt2 + key2);
            double       r1      = res + (MOD_DEPTH * mod[kDstRes1]) + (128.0 * curve9(kOffsetCurve, s->b[56]) * vel);
            double       r2      = res2 + (MOD_DEPTH * mod[kDstRes2]) + (128.0 * curve9(kOffsetCurve, s->b[57]) * vel);
            double       drive   = fmax(0.0, ((double)s->a[36] - 64.0) / 64.0);
            tTiFilterSet set;

            filter_set(v, s, fmax(c1, 0.0), fmax(r1, 0.0), fmax(c2, 0.0), fmax(r2, 0.0), balance + (MOD_DEPTH * mod[kDstFilterBalance]), drive, &set);

            tTiEnvShape  amp, e3, e4;

            env_shape_mod(&s->a[59], mod, kDstAAttack, v->rate, &amp);
            env_shape_mod(&s->c[80], mod, 0, v->rate, &e3);
            env_shape_mod(&s->c[85], mod, 0, v->rate, &e4);
            // notes §11: only the amplifier's sustain is curved; the others' are straight lines
            e3.sustain    = s->c[82] / 127.0;
            e4.sustain    = s->c[87] / 127.0;
            e3.attackStep = attack_rate(s->c[80]) / v->rate;
            e4.attackStep = attack_rate(s->c[85]) / v->rate;

            double       ampVel  = bipolar(s->b[60]);
            double       velDial = (ampVel >= 0.0) ? (127.0 - (ampVel * 127.0 * (1.0 - vel))) : (127.0 + (ampVel * 127.0 * vel)); // notes §11
            double       level   = (patch_volume(s->a[91] + (MOD_DEPTH * mod[kDstVolume])) / patch_volume(fmax(s->a[91], 1.0))) * ((ampVel == 0.0) ? 1.0 : mix_level(velDial));
            double       pan     = clamp(bipolar(s->a[10]) + (2.0 * mod[kDstPan]) + vc->pan + (bipolar(s->b[61]) * vel), -1.0, 1.0);
            double       pl      = (pan <= 0.0) ? (1.0 - (PAN_HARD * pan)) : (1.0 - pan); // notes §7: two lines, meeting at the centre
            double       pr      = (pan >= 0.0) ? (1.0 + (PAN_HARD * pan)) : (1.0 + pan);
            level        *= 1.0 - (0.25 * vc->place);                                     // notes §13: 1 at the centre of a stack, 0.75 at its edges

            render_voice(v, s, vc, &set, &amp, &fenv, &e3, &e4, mod, left + done, right + done, n, level * pl, level * pr);
        }
    }

    analog_boost(v, s, left, right, frames);
    // notes §22
    tTiSound fx = *s;

    for (size_t k = 0; k < sizeof(kFxDst) / sizeof(kFxDst[0]); k++) {
        uint8_t * page = (kFxDst[k].page == 0) ? fx.a : ((kFxDst[k].page == 1) ? fx.b : fx.c);

        page[kFxDst[k].at] = modded(page[kFxDst[k].at], v->fxMod[kFxDst[k].dst]);
    }

    ti_effects_render(&v->effects, fx.a, fx.b, fx.c, patch_volume(fmax(s->a[91], 1.0)), left, right, frames);

    // notes §18: the end of the chain clips hard at full scale, with no limiter before it
    for (uint32_t f = 0; f < frames; f++) {
        v->clipped = v->clipped || (fabsf(left[f]) > 1.0f) || (fabsf(right[f]) > 1.0f);
        left[f]    = fminf(fmaxf(left[f], -1.0f), 1.0f);
        right[f]   = fminf(fmaxf(right[f], -1.0f), 1.0f);
    }
}
