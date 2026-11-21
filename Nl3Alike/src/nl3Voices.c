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

#include <math.h>
#include <string.h>

#include "nl3Voices.h"

#define SMOOTH_S          (2048.0 / 46875.0)        // notes §5
#define VELOCITY_FLOOR    (0.0157)
#define VELOCITY_RANGE    (0.98)
#define RING_GAIN         (2.0)
#define MIX_SCALE         (2.0 / 3.0)          // osc notes §18: Osc Mix's gains on the instrument carry 2/3
#define OSC_MOD_SOURCE    (0.5)                // osc notes §17: Osc 2 hands Osc Mod half its output
#define NOISE3_GAIN       (0.355)              // voices notes §44: 6.1 dB under a saw, as the instrument's
#define NOISE_LCG_MUL     (0x0B2D9Du)          // osc notes §15
#define NOISE_LCG_ADD     (0x361963u)
#define ENV_TO_CUTOFF     (2.2146)
#define DRIVE_IN          (0.5)         // voices notes §8
#define FILTER_IN         (4.0 / 3.0)   // voices notes §19: with DRIVE_IN, the mix reaches the filter at 2/3, as the instrument's
#define TRACK_PIVOT       (36.0)        // voices notes §9: Keyboard Track pivots on C2
#define FILTER_OUT        (1.5)         // voices notes §19: the filter leaves a quarter of what it is given; 4 x (1/4) / (2/3)
#define LFO_FADE_HOLD     (0.0125)
#define LFO_FADE_FALL     (5.7)
#define SUB_BLOCK         (32u)          // notes §4
#define UNISON_SPAN       (244.0)        // voices notes §26: delay samples across the word
#define UNISON_SECOND     (0.7)          // voices notes §26: the second read's step against the first's

static void rank_voices(tNl3Voices * v);

void nl3_voices_init(tNl3Voices * v) {
    memset(v, 0, sizeof(*v));
    v->limit    = 24;
    v->rate     = NL3_ENGINE_HZ;
    v->lastNote = -1;
    nl3_edge_build(v->edge);
    nl3_steps_build(&v->steps);

    // voices notes §8: the oscillators run free and are never set back - each starts somewhere of its own
    for (uint32_t i = 0; i < NL3_MAX_VOICES; i++) {
        for (int o = 0; o < NL3_OSCS; o++) {
            uint32_t h = ((i * NL3_OSCS) + (uint32_t)o + 1u) * 2654435761u;

            v->voice[i].phase[o] = ((double)(h >> 8) / 8388608.0) - 1.0;
        }
    }

    rank_voices(v);
}

// voices notes §19
void nl3_voices_set_fixed(tNl3Voices * v, bool fixed) {
    v->fixedPoint = fixed;
}

void nl3_voices_set_clean(tNl3Voices * v, bool clean) {
    v->clean = clean;
}

void nl3_voices_set_limit(tNl3Voices * v, uint32_t limit) {
    limit    = (limit < 1u) ? 1u : ((limit > NL3_MAX_VOICES) ? NL3_MAX_VOICES : limit);
    v->limit = limit;
    rank_voices(v);

    // notes §2: voices above a lowered limit are released, not cut
    for (uint32_t i = limit; i < NL3_MAX_VOICES; i++) {
        v->voice[i].gate = false;
        nl3_env_gate(&v->voice[i].amp, false);
    }
}

// osc notes §8: the oscillator's pitch in notes
static double osc_pitch(const tNl3OscSound * o, double note, double bend, double wheel, double moved) {
    note = o->fixedPitch ? 64.0 : note;   // osc notes §21: no key, no Octave Shift, no glide
    double semitones = note - 60.0 + (12.0 * o->octaves) + o->semitones + (o->cents / 100.0) + (bend * o->bendRange) + (wheel * o->wheelPitch) + moved;

    return 60.0 + semitones;
}

// voices notes §12: the glide's speed in semitones a second, Rate 1-127, measured on the instrument's arithmetic (no
// closed form found yet - see the notes)
static const double kGlideRate[127] = {
    578.9, 578.9, 571.5, 559.2, 549.5, 534.9, 522.5, 503.3, 485.1, 470.6, 466.5, 464.6, 458.5, 442.2, 424.7, 404.1,
    387.3, 375.7, 368.4, 358.7, 352.4, 343.4, 331.6, 322.6, 314.4, 305.5, 301.3, 293.0, 283.7, 275.5, 264.8, 256.3,
    251.0, 243.7, 236.2, 228.6, 220.2, 214.7, 209.9, 204.5, 197.8, 192.0, 184.9, 178.9, 174.2, 169.6, 164.5, 160.1,
    155.0, 149.8, 145.4, 140.9, 136.5, 132.6, 128.7, 125.0, 121.2, 117.5, 114.3, 111.1, 107.9, 104.6, 101.5,  98.2,
    95.4,   92.6,  89.9,  87.3,  84.8,  82.2,  79.7,  77.3,  75.1,  72.8,  70.7,  68.6,  66.6,  64.5,  63.0,  61.5, 59.9, 58.1,
    56.1,   53.9,  52.2,  50.4,  49.0,  47.6,  46.2,  44.8,  43.2,  41.6,  40.4,  39.1,  37.8,  36.6,  35.3,  34.0, 32.8, 31.6,
    30.3,   29.1,  27.9,  26.8,  25.7,  24.6,  23.4,  22.3,  21.2,  20.2,  19.0,  17.9,  16.8,  15.7,  14.7,  13.7, 12.6, 11.5,
    10.5,    9.4,   8.3,   7.3,   6.2,   5.2,   4.0,   2.9, 1.9
};

static double glide_rate(double rate) {
    long r = lround(fmin(127.0, fmax(1.0, rate)));

    return kGlideRate[r - 1];
}

static double glide_heard(const tNl3Voice * vc, const tNl3Sound * sound) {
    return (sound->glideStyle == 1) ? round(vc->glide) : vc->glide;
}

static void set_pitch(tNl3Voice * vc, const tNl3Sound * sound) {
    double note = (double)vc->note + sound->keyShift + glide_heard(vc, sound);

    for (int o = 0; o < NL3_OSCS; o++) {
        vc->pitch[o]     = osc_pitch(&sound->osc[o], note, sound->bend, sound->wheel, vc->lfoSemis[o]);
        vc->increment[o] = nl3_pitch_increment(vc->pitch[o]);
    }
}

// voices notes §15
static double next_random(uint32_t * seed) {
    uint32_t x = *seed;

    x    ^= x << 13;
    x    ^= x >> 17;
    x    ^= x << 5;
    *seed = x;
    return ((double)x / 2147483648.0) - 1.0;
}

static void lfo_restart(tNl3Voice * vc, const tNl3Sound * sound) {
    vc->lfoAge = 0.0;

    for (int l = 0; l < NL3_LFOS; l++) {
        vc->lfoHeld[l] = next_random(&vc->noiseSeed);

        if (sound->lfo[l].keyed) {
            vc->lfoPhase[l] = 0.0;
        }
    }
}

// voices notes §21: the instrument's waves, on its phase p = -1..1 (phase 0..1 here)
static double lfo_wave(int shape, double phase, double held) {
    double p = (2.0 * phase) - 1.0;

    switch (shape) {
        case 0:
            return (p > 0.0) ? 1.0 : -1.0;

        case 1:
            return -p;

        case 2:
            return (2.0 * fabs(p)) - 1.0;

        default:
            return held;
    }
}

static double lfo_fade(const tNl3LfoSound * lfo, double age) {
    if (lfo->fadeOut) {
        return (lfo->delay > 0.0) ? exp(-fmax(age - LFO_FADE_HOLD, 0.0) * LFO_FADE_FALL / lfo->delay) : 0.0;
    }
    return (lfo->delay > 0.0) ? fmin(1.0, age / lfo->delay) : 1.0;
}

// osc notes §19: a sine at half size, from a phase of 2.0 a cycle in -1..1, as the instrument's Dual Sine reads it
static double dual_half_sine(double p) {
    return 0.5 * sin(0.5 * M_PI * ((2.0 * fabs(p)) - 1.0));
}

static double dual_wrap(double p) {
    return p - (2.0 * floor((p + 1.0) / 2.0));
}

// osc notes §19: where a Dual Sine starts on a new note - Osc 1's plain Dual Sine runs on
static void dual_restart(tNl3Voice * vc, const tNl3Sound * sound) {
    for (int o = 0; o < 2; o++) {
        const tNl3OscSound * os = &sound->osc[o];

        if (!os->dualSine || ((o == 0) && !os->dualFm)) {
            continue;
        }
        vc->phase[o]       = (o == 0) ? 0.5 : (69.0 / 128.0);
        vc->dualPhase[o]   = (o == 0) ? (-59.0 / 128.0) : (os->dualFm ? (-54.0 / 128.0) : (69.0 / 128.0));
        vc->dualHist[o][0] = ((o == 0) && os->dualFm) ? dual_half_sine(vc->dualPhase[o]) : 0.0;
        vc->dualHist[o][1] = vc->dualHist[o][0];
    }
}

// osc notes §19: one sample of a Dual Sine; s is the Shape (or FM Amount) word, applied squared
static double dual_sine(tNl3Voice * vc, int o, const tNl3OscSound * os, double incA, double incBase, double s) {
    double level = s * s;
    double incB  = fmin(0.999, os->dualRatio * incBase);   // Osc Mod's FM reaches sine A alone
    double a, b;

    vc->dualPhase[o]   = dual_wrap(vc->dualPhase[o] + incB);

    if (!os->dualFm) {
        double hA, hB;

        vc->phase[o]   = dual_wrap(vc->phase[o] + incA);
        hA             = dual_half_sine(vc->phase[o]);
        hB             = dual_half_sine(vc->dualPhase[o]);
        vc->dualMod[o] = -(hA + (3.0 * level * hB));   // the instrument hands Osc Mod B three times as strong, inverted
        return hA + (level * hB);
    }

    if (o == 0) {   // Osc 1: by B's last change, 32 s^2 (B[n-2] - B[n-1])
        vc->phase[o] = dual_wrap(vc->phase[o] + incA + (32.0 * level * (vc->dualHist[o][1] - vc->dualHist[o][0])));
    } else {        // Osc 2: by B's last value
        vc->phase[o] = dual_wrap(vc->phase[o] + incA + (level * vc->dualHist[o][0]));
    }
    a                  = dual_half_sine(vc->phase[o]);
    b                  = dual_half_sine(vc->dualPhase[o]);
    vc->dualHist[o][1] = vc->dualHist[o][0];
    vc->dualHist[o][0] = b;
    vc->dualMod[o]     = -a;   // inverted, as the instrument hands it to Osc Mod
    return a;
}

// osc notes §18: Osc Mod's DIST, RM and DM, set up once a sub-block from the (modulated) Amount
typedef struct {
    int    type;
    double d;                     // Amount / 128, 127 as 1
    double drive, cubic;          // DIST
    double dmIncrement;           // DM: the hidden sine's increment
} tNl3OscMod;

static void osc_mod_setup(int type, double amount, double osc1Increment, tNl3OscMod * m) {
    double a = fmin(127.0, fmax(0.0, amount));

    m->type        = type;
    m->d           = (a >= 127.0) ? 1.0 : (a / 128.0);
    m->drive       = fmax(1.0 / 64.0, m->d * m->d * m->d);
    m->cubic       = (a < 32.0) ? (pow(a / 32.0, 3.0) / 3.0) : (1.0 / 3.0);
    m->dmIncrement = osc1Increment * exp2((round(a) - 64.0) / 12.0);   // a semitone a step, unison at 64
}

static double osc_mod_clip(double x) {
    return fmin(1.0, fmax(-1.0, x));
}

// osc notes §18: what the oscillators give the filter: Osc 1's and Osc 2's parts after the mix, and Osc 2 raw
static double osc_mod_mix(tNl3Voice * vc, const tNl3OscMod * m, double part1, double part2, double osc2) {
    switch (m->type) {
        case 0:     // DM: the mix times a hidden sine raised to 0..1, clipped
        {
            double p = vc->dmPhase + m->dmIncrement;
            double t;

            p          -= 2.0 * floor((p + 1.0) / 2.0);    // a cycle is 2.0, kept in -1..1
            vc->dmPhase = p;
            t           = (2.0 * fabs(p)) - 1.0;
            return 0.5 * osc_mod_clip(8.0 * (part1 + part2) * 0.5 * (1.0 + sin(0.5 * M_PI * t)));
        }
        case 1:     // DIST: a hard clip into a cubic soft clip
        {
            double y = osc_mod_clip(64.0 * m->drive * (part1 + part2));

            return 0.75 * (y - (m->cubic * y * y * y));
        }
        case 5:     // RM: from Osc 1 towards Osc 1 times Osc 2, with Osc Mix's two gains
            return part2 + ((2.0 / 3.0) * (1.0 - (m->d * m->d)) * part1) + ((4.0 / 3.0) * ((2.0 * m->d) - (m->d * m->d)) * part1 * osc2);

        default:
            return part1 + part2;
    }
}

// voices notes §41: the Mod Env as the both-ways destinations (pitch, Osc Mix) take it - in AR mode measured from the
// top, so the destination is as set while the key is held and bent below it on the way up and down
static double mod_env_out(const tNl3Voice * vc, const tNl3Sound * sound) {
    return vc->env[1].level - ((sound->env[1].sustain >= 127.0) ? 1.0 : 0.0);
}

// voices notes §43: an envelope at a one-way destination (filter, Shape, Osc Mod) - Invert flips its shape, still one
// way: Amount x (1 - level), in AD and AR alike
static double env_one_way(const tNl3Voice * vc, const tNl3Sound * sound, int e) {
    if (sound->env[e].invert) {
        return 1.0 - vc->env[e].level;
    }
    return vc->env[e].level;   // voices notes §41: AR holds at the top on the one-way destinations
}

// voices notes §43: at a both-ways destination (pitch, Osc Mix) Invert is a sign
static double env_both_ways(const tNl3Voice * vc, const tNl3Sound * sound) {
    return (sound->env[1].invert ? -1.0 : 1.0) * mod_env_out(vc, sound);
}

// filter notes §15-16, §20: Filter 2's type in multi mode
static const tNl3FilterMode kMultiModes[6] = {eNl3FilterLpBpPar, eNl3FilterLpHpPar, eNl3FilterHpLpSer, eNl3FilterBpBpPar, eNl3FilterPeak, eNl3FilterNotch};

// voices notes §15: every LFO moved on by one sub-block, and what it does to this voice
static void lfo_advance(tNl3Voice * vc, const tNl3Sound * sound, double seconds, double * cutoff, double width[NL3_OSCS],
                        double * mix, double * ampGain, double * cutoff2, double * oscMod, double * res, double * pan) {
    *res     = 0.0;
    *pan     = 0.0;
    *cutoff  = 0.0;
    *cutoff2 = 0.0;
    *oscMod  = 0.0;
    *mix     = 0.0;
    *ampGain = 1.0;

    for (int o = 0; o < NL3_OSCS; o++) {
        vc->lfoSemis[o] = 0.0;
        width[o]        = sound->osc[o].width;
    }

    for (int l = 0; l < NL3_LFOS; l++) {
        const tNl3LfoSound * lfo   = &sound->lfo[l];
        double               phase = vc->lfoPhase[l];   // voices notes §29: every voice its own LFO, at its own rate
        double               next  = phase + (lfo->hz * seconds);
        double               fade  = lfo_fade(lfo, vc->lfoAge);

        if ((next >= 1.0) && lfo->single) {
            next = nextafter(1.0, 0.0);   // voices notes §37: Env mode holds the end of its one cycle
        } else if (next >= 1.0) {
            next          -= floor(next);
            vc->lfoHeld[l] = next_random(&vc->noiseSeed);
        }
        vc->lfoPhase[l] = next;
        double               value = (lfo_wave(lfo->shape, next, vc->lfoHeld[l]) * fade) + lfo->offset;
        double               up    = 0.5 * (value + fade);     // voices notes §22: 0 at the wave's bottom, the fade at its top
        double               both  = lfo->single ? up : value; // voices notes §42: in Env mode the both-ways destinations go one way too, ending at 0

        up              = lfo->invert ? (fade - up) : up;      // voices notes §40: Invert flips the one-way destinations' wave, still one way
        double               down  = fade - up;

        tNl3LfoTargets       t     = {
            sound->lfoCutoff[l], sound->lfoMix[l], sound->lfoAmp[l],     {sound->osc[0].lfoPitch[l], sound->osc[1].lfoPitch[l]},
            {sound->osc[0].lfoWidth[l],sound->osc[1].lfoWidth[l]}, sound->lfoCutoff2[l], sound->lfoOscMod[l], sound->lfoRes[l], sound->lfoPan[l]
        };

        if ((l == 0) && (sound->modEnvLfo1 != 0.0)) {   // voices notes §24: the Mod Env adds to LFO 1's Amount
            double amount = fmin(127.0, fmax(0.0, lfo->amount + (env_one_way(vc, sound, 1) * sound->modEnvLfo1)));

            nl3_lfo_targets(l, lfo->to, amount, lfo->invert, &t);
        }
        *cutoff        += up * t.cutoff;
        *oscMod        += up * t.oscMod;  // voices notes §39: up from the Amount, as Filter and Shape
        *res           += up * t.res;     // voices notes §46: one way, as Filter
        *pan           += both * t.pan;   // both ways

        if (!((lfo->to == 5) && sound->filter2Env)) {   // voices notes §39: with Filter 2 Env on, Filter 2 already follows
            *cutoff2 += up * t.cutoff2;
        }
        *mix           += both * t.mix;
        *ampGain       *= 1.0 - (down * t.amp);

        for (int o = 0; o < NL3_OSCS; o++) {
            vc->lfoSemis[o] += both * t.pitch[o];
            width[o]        += up * t.width[o];
        }
    }

    vc->lfoAge += seconds;
}

// voices notes §12
static void start_glide(tNl3Voices * v, const tNl3Sound * sound, tNl3Voice * pick, uint8_t note, bool over) {
    static const double kFixed[8] = {-2.0, 2.0, -5.0, 5.0, -7.0, 7.0, -12.0, 12.0};
    bool                held      = over;

    for (uint32_t i = 0; i < NL3_MAX_VOICES; i++) {
        held = held || (v->voice[i].gate && (&v->voice[i] != pick));
    }

    // voices notes §12a: from the pitch this voice had reached, mid-glide or not; a voice never played, from the last note
    double              from      = (pick->note > 0) ? ((double)pick->note + pick->glide) : (double)v->lastNote;

    pick->glide     = 0.0;

    if (sound->glideTime > 0.0) {
        if (sound->glideType >= 2) {
            pick->glide = kFixed[(sound->glideType - 2) & 7];
        } else if ((sound->mono || (sound->legato && held)) && (v->lastNote >= 0) && ((sound->glideType == 0) || held)) {   // voices notes §12a
            pick->glide = from - (double)note;
        }
    }
    pick->glideRate = glide_rate(sound->glideTime);   // voices notes §12: a constant rate, whatever the interval
    v->lastNote     = note;
}

static void advance_glide(tNl3Voice * vc, const tNl3Sound * sound, double seconds) {
    (void)sound;
    double size = fabs(vc->glide);
    double move = vc->glideRate * seconds;   // voices notes §12: a straight line in semitones
    vc->glide = (move >= size) ? 0.0 : ((vc->glide > 0.0) ? (vc->glide - move) : (vc->glide + move));
}

static void held_remove(tNl3Voices * v, uint8_t note) {
    for (uint32_t i = 0; i < v->heldCount; i++) {
        if (v->heldNote[i] == note) {
            memmove(&v->heldNote[i], &v->heldNote[i + 1], (v->heldCount - i - 1) * sizeof(v->heldNote[0]));
            memmove(&v->heldVelocity[i], &v->heldVelocity[i + 1], (v->heldCount - i - 1) * sizeof(v->heldVelocity[0]));
            v->heldCount--;
            return;
        }
    }
}

// voices notes §13: the one voice moves to a note; the envelopes restart unless it is legato over a held note
static void mono_play(tNl3Voices * v, const tNl3Sound * sound, uint8_t note, float velocity) {
    tNl3Voice * vc      = &v->voice[0];
    bool        over    = vc->active && vc->gate;
    bool        restart = !over || !sound->legato;

    vc->active = true;
    vc->gate   = true;
    start_glide(v, sound, vc, note, over);   // voices notes §12a: from the note it was on, so before taking the new one
    vc->note   = note;
    vc->key    = note;

    if (restart) {
        vc->velocity  = velocity;

        vc->started   = ++v->clock;
        vc->noiseSeed = vc->noiseSeed ? vc->noiseSeed : (uint32_t)((v->clock * 2654435761u) | 1u);

        if ((vc->delayWord[0] | vc->delayWord[1]) == 0u) {   // voices notes §26: each voice's reads start somewhere of their own
            vc->delayWord[0] = (vc->noiseSeed * 2246822519u) & 0xFFFFFFu;
            vc->delayWord[1] = (vc->noiseSeed * 3266489917u) & 0xFFFFFFu;
        }
        lfo_restart(vc, sound);
        dual_restart(vc, sound);   // osc notes §19
        nl3_env_gate(&vc->amp, true);

        for (int e = 0; e < NL3_MOD_ENVS; e++) {
            nl3_env_gate(&vc->env[e], true);
        }

        if (vc->amp.level <= 0.0) {
            nl3_filter_reset(&vc->filter);
        }
    }
    set_pitch(vc, sound);
}

// voices notes §28: the level a voice is ranked by - silent is 0, 16 bits as the instrument reads it
static uint32_t rank_level(const tNl3Voice * vc) {
    return vc->active ? (uint32_t)fmin(65535.0, floor(fmax(0.0, vc->amp.level) * 65535.0)) : 0u;
}

// voices notes §28: after every note event, the order the next notes will take the voices in - the keys up quietest
// first, then the held ones oldest first sparing the lowest held key, then whatever is left
static void rank_voices(tNl3Voices * v) {
    bool ranked[NL3_MAX_VOICES] = {false};
    int  lowestHeld             = 0x7FFF;

    for (uint32_t i = 0; i < v->limit; i++) {
        if (v->voice[i].active && v->voice[i].gate && (v->voice[i].note < lowestHeld)) {
            lowestHeld = v->voice[i].note;
        }
    }

    for (uint32_t k = 0; k < v->limit; k++) {
        int  pick  = -1;

        for (uint32_t i = 0; i < v->limit; i++) {
            const tNl3Voice * vc = &v->voice[i];

            if (  !ranked[i] && !(vc->active && vc->gate) && (rank_level(vc) < 0xFFFFu)
               && ((pick < 0) || (rank_level(vc) < rank_level(&v->voice[pick])))) {
                pick = (int)i;
            }
        }

        bool spare = pick < 0;   // every voice left is held: the oldest, sparing the lowest key

        for (uint32_t i = 0; (i < v->limit) && spare; i++) {
            if (!ranked[i] && (v->voice[i].note != lowestHeld) && ((pick < 0) || (v->voice[i].started < v->voice[pick].started))) {
                pick = (int)i;
            }
        }

        for (uint32_t i = 0; (i < v->limit) && (pick < 0); i++) {
            if (!ranked[i]) {
                pick = (int)i;
            }
        }

        ranked[pick] = true;
        v->order[k]  = (uint8_t)pick;
    }

    v->orderNext = 0;
}

static void start_voice(tNl3Voices * v, const tNl3Sound * sound, uint8_t key, uint8_t note, float velocity);

void nl3_voices_note_on(tNl3Voices * v, const tNl3Sound * sound, uint8_t note, float velocity) {
    v->pedalled[note & 0x7F] = false;

    if (sound->mono) {
        held_remove(v, note);
        v->heldNote[v->heldCount]     = note;
        v->heldVelocity[v->heldCount] = velocity;
        v->heldCount++;
        mono_play(v, sound, note, velocity);
        return;
    }
    v->heldCount             = 0;

    if (sound->chordNotes == 0) {
        start_voice(v, sound, note, note, velocity);
        return;
    }

    for (uint32_t k = 0; k < sound->chordNotes; k++) {   // voices notes §47: one voice per chord note
        int n = note + sound->chord[k];

        if (n <= 127) {
            start_voice(v, sound, note, (uint8_t)n, velocity);
        }
    }
}

static void start_voice(tNl3Voices * v, const tNl3Sound * sound, uint8_t key, uint8_t note, float velocity) {
    tNl3Voice * pick = NULL;

    // voices notes §34: Piano takes over the key's newest sounding voice; Stack lets it go on releasing and takes another
    for (uint32_t i = 0; (i < v->limit) && sound->piano; i++) {
        if (v->voice[i].active && (v->voice[i].key == key) && (v->voice[i].note == note) && ((pick == NULL) || (v->voice[i].started > pick->started))) {
            pick = &v->voice[i];
        }
    }

    for (uint32_t i = 0; i < v->limit; i++) {
        if (v->voice[i].active && v->voice[i].gate && (v->voice[i].key == key) && (v->voice[i].note == note) && (&v->voice[i] != pick)) {
            v->voice[i].gate = false;
            nl3_env_gate(&v->voice[i].amp, false);

            for (int e = 0; e < NL3_MOD_ENVS; e++) {
                nl3_env_gate(&v->voice[i].env[e], false);
            }
        }
    }

    if (pick == NULL) {
        pick         = &v->voice[v->order[v->orderNext % v->limit]];   // voices notes §28
        v->orderNext = (v->orderNext + 1) % v->limit;
    }
    bool wasSounding = pick->active;

    pick->active    = true;
    pick->gate      = true;
    pick->velocity  = velocity;

    start_glide(v, sound, pick, note, false);
    pick->note      = note;            // voices notes §12a: after the glide has read the note it was on
    pick->key       = key;
    set_pitch(pick, sound);

    dual_restart(pick, sound);   // osc notes §19
    pick->started   = ++v->clock;
    pick->noiseSeed = pick->noiseSeed ? pick->noiseSeed : (uint32_t)((v->clock * 2654435761u) | 1u);
    lfo_restart(pick, sound);

    for (int l = 0; (l < NL3_LFOS) && !wasSounding; l++) {
        if (!sound->lfo[l].keyed) {
            pick->lfoPhase[l] = v->lfoFree[l];   // voices notes §29: a free LFO starting from silence joins the others' phase
        }
    }

    nl3_env_gate(&pick->amp, true);

    for (int e = 0; e < NL3_MOD_ENVS; e++) {
        nl3_env_gate(&pick->env[e], true);
    }

    if (pick->amp.level <= 0.0) {
        nl3_filter_reset(&pick->filter);    // a voice starting from silence starts with a quiet filter
    }
    rank_voices(v);
}

static void release_note(tNl3Voices * v, const tNl3Sound * sound, uint8_t note);

// A key let go with the pedal down waits for the pedal
void nl3_voices_note_off(tNl3Voices * v, const tNl3Sound * sound, uint8_t note) {
    if (sound->pedal) {
        v->pedalled[note & 0x7F] = true;
        rank_voices(v);   // voices notes §28: every note event ranks the voices again
        return;
    }
    release_note(v, sound, note);
}

static void release_note(tNl3Voices * v, const tNl3Sound * sound, uint8_t note) {
    if (sound->mono) {
        held_remove(v, note);

        if (v->heldCount > 0) {
            uint32_t last = v->heldCount - 1;

            if (v->voice[0].note == note) {
                mono_play(v, sound, v->heldNote[last], v->heldVelocity[last]);
            }
            return;
        }
    }

    for (uint32_t i = 0; i < NL3_MAX_VOICES; i++) {
        if (v->voice[i].active && (v->voice[i].key == note)) {   // voices notes §47: every note of the key's chord
            v->voice[i].gate = false;
            nl3_env_gate(&v->voice[i].amp, false);

            for (int e = 0; e < NL3_MOD_ENVS; e++) {
                nl3_env_gate(&v->voice[i].env[e], false);
            }
        }
    }

    rank_voices(v);
}

void nl3_voices_all_off(tNl3Voices * v) {
    v->heldCount = 0;
    memset(v->pedalled, 0, sizeof(v->pedalled));

    for (uint32_t i = 0; i < NL3_MAX_VOICES; i++) {
        v->voice[i].active = false;
        v->voice[i].gate   = false;
        v->voice[i].amp    = (tNl3Env){
            0
        };
        nl3_filter_reset(&v->voice[i].filter);
        memset(v->voice[i].env, 0, sizeof(v->voice[i].env));
        memset(v->voice[i].ring, 0, sizeof(v->voice[i].ring));   // voices notes §26: Unison's delayed copies
    }

    rank_voices(v);
}

static void smooth_toward(tNl3Smooth * s, double target, double rate) {
    if (!s->started) {
        s->started = true;
        s->current = s->target = target;
        s->left    = 0.0;
    } else if (target != s->target) {
        s->target = target;
        s->left   = SMOOTH_S * rate;
        s->step   = (target - s->current) / s->left;
    }
}

static double smooth_advance(tNl3Smooth * s, uint32_t frames) {
    if (s->left > (double)frames) {
        s->current += s->step * frames;
        s->left    -= frames;
    } else {
        s->current = s->target;
        s->left    = 0.0;
    }
    return s->current;
}

// voices notes §11
// voices notes §44: Noise3 - the noise generator's words, each averaged with the one before, at the instrument's level
static double noise3(tNl3Voice * vc) {
    uint32_t s = vc->noiseWord[2] ? vc->noiseWord[2] : (((vc->noiseSeed * 2246822519u) | 1u) & 0xFFFFFFu);
    double   x;
    double   out;

    s                = ((s * NOISE_LCG_MUL) + NOISE_LCG_ADD) & 0xFFFFFFu;
    vc->noiseWord[2] = s;
    x                = nl3_word_value((tNl3Word)((int32_t)(s << 8) >> 8));
    out              = 0.5 * (x + vc->noiseLast);
    vc->noiseLast    = x;
    return NOISE3_GAIN * out;
}

// voices notes §8: Overdrive 0..127 as the drive word, and the saturator on the filter's input
#define DRIVE_LEAST    (1.0)      // voices notes §8: linear at 0, as the instrument's path is
#define DRIVE_TRIM     (0.012)

static double drive_word(double overdrive) {
    double v = overdrive / 128.0;

    return 1.0 - (DRIVE_LEAST * (1.0 - v) * (1.0 - v)) - (DRIVE_TRIM * v * (1.0 - v));
}

static double drive(double x, double word) {
    return x / (1.0 + (word * (fabs(x) - 1.0)));
}

// voices notes §10
static double velocity_level(double velocity, double amount) {
    double curve = VELOCITY_FLOOR + (VELOCITY_RANGE * velocity * velocity * velocity);

    if (amount >= 64.0) {
        double a = (amount - 64.0) / 63.0;

        return 1.0 - a + (a * curve);
    }
    return 1.0 - (((64.0 - amount) / 64.0) * curve);
}

// voices notes §26: the 6-tap fractional read, sinc(t) x hann(t/3) about the point `delay` samples back
// osc notes §13: an oscillator's phase a sample on - a synced one restarted at +0.5, moved on by the share of a sample
// since its hidden partner passed zero going up
static tNl3Word advance_word(tNl3Voice * vc, int o, const tNl3OscSound * os, tNl3Word hiddenInc, tNl3Word inc) {
    tNl3Word phase  = (tNl3Word)llround(vc->phase[o] * NL3_Q23);

    if (!os->sync) {
        return nl3_phase_advance(phase, inc);
    }
    tNl3Word hidden = nl3_phase_advance((tNl3Word)llround(vc->syncPhase[o] * NL3_Q23), hiddenInc);

    vc->syncPhase[o] = nl3_word_value(hidden);

    if ((hidden >= 0) && (hidden < hiddenInc)) {
        tNl3Word past = (tNl3Word)(((int64_t)hidden << 23) / hiddenInc);

        return nl3_phase_advance(0x400000, (tNl3Word)(((int64_t)inc * past) >> 23));
    }
    return nl3_phase_advance(phase, inc);
}

static double advance(tNl3Voice * vc, int o, const tNl3OscSound * os, double hiddenInc, double inc) {
    double phase  = vc->phase[o] + inc;

    phase           -= (phase >= 1.0) ? 2.0 : 0.0;

    if (!os->sync) {
        return phase;
    }
    double hidden = vc->syncPhase[o] + hiddenInc;

    hidden          -= (hidden >= 1.0) ? 2.0 : 0.0;
    vc->syncPhase[o] = hidden;
    return ((hidden >= 0.0) && (hidden < hiddenInc)) ? (0.5 + (inc * hidden / hiddenInc)) : phase;
}

static double ring_read(const tNl3Voice * vc, double delay) {
    double place = (double)vc->ringAt - delay;
    double base  = floor(place);
    double p     = place - base;
    double sum   = 0.0;

    for (int j = -2; j <= 3; j++) {
        double t = (double)j - p;
        double s = (fabs(t) < 1e-9) ? 1.0 : (sin(M_PI * t) / (M_PI * t));
        double h = (fabs(t) >= 3.0) ? 0.0 : (0.5 + (0.5 * cos(M_PI * t / 3.0)));

        sum += vc->ring[((int)base + j) & 255] * s * h;
    }

    return sum;
}

// osc notes §15: the Noise waveform - a 24-bit generator, averaged with its last average, then the two-pole lowpass
// at q 1 on a word from Shape; out the mean of its last two lowpass values
static double osc_noise(tNl3Voice * vc, int o, double shape) {
    double   f, k;
    uint32_t s  = vc->noiseWord[o] ? vc->noiseWord[o] : (((vc->noiseSeed * 2654435761u) ^ (uint32_t)(o + 1)) & 0xFFFFFFu);

    nl3_noise_coeffs(shape, &f, &k);
    s                = ((s * NOISE_LCG_MUL) + NOISE_LCG_ADD) & 0xFFFFFFu;
    vc->noiseWord[o] = s;
    double   x  = nl3_word_value((tNl3Word)((int32_t)(s << 8) >> 8));
    double   b  = 0.5 * (vc->noiseAvg[o] = 0.5 * (x + vc->noiseAvg[o]));
    double   bn = vc->noiseB[o] + (2.0 * f * vc->noiseA[o]);
    double   h  = b - bn - (2.0 * k * vc->noiseA[o]);
    double   y  = 0.5 * (bn + vc->noiseB[o]);

    vc->noiseA[o]   += 2.0 * f * h;
    vc->noiseB[o]    = bn;
    return y;
}

// osc notes §20: Osc 2 Noise - the generator through a resonant state-variable filter at the oscillator's pitch
static double osc2_noise(tNl3Voice * vc, const tNl3OscSound * os, double increment, double shape) {
    double   f, k, n;
    uint32_t s    = vc->noiseWord[1] ? vc->noiseWord[1] : (((vc->noiseSeed * 2654435761u) ^ 2u) & 0xFFFFFFu);

    nl3_noise2_coeffs(increment, shape, &f, &k, &n);
    s                = ((s * NOISE_LCG_MUL) + NOISE_LCG_ADD) & 0xFFFFFFu;
    vc->noiseWord[1] = s;
    double   x    = nl3_word_value((tNl3Word)((int32_t)(s << 8) >> 8)) * (1.0 - (0.85 * shape)) / 512.0;
    double   u    = (os->noise2 == 3) ? (n * n * x) : (((os->noise2 == 2) ? n : 1.0) * (x + vc->noise2Last));
    double   low  = vc->noise2Low + (f * vc->noise2Band);
    double   high = u - low - (k * vc->noise2Band);
    double   out  = (os->noise2 == 1) ? (64.0 * (low + vc->noise2Low)) : ((os->noise2 == 2) ? 0.0 : (256.0 * high));

    vc->noise2Last   = x;
    vc->noise2Low    = low;
    vc->noise2Band  += 4.0 * f * high;

    if (os->noise2 == 2) {
        out = 64.0 * vc->noise2Band;
    }
    return fmin(1.0, fmax(-1.0, out));
}

// osc notes §22: synched noise - four sub-steps a sample, each moving a phase on a quarter of the increment; the
// generator steps every eight sub-steps and restarts at the seed whenever the phase wraps (once a cycle); the four
// sub-samples averaged, then halved with the last output
static double sync_noise(tNl3Voice * vc, int o, const tNl3OscSound * os, double increment) {
    double sum = 0.0;

    for (int k = 0; k < 4; k++) {
        bool wrapped;

        vc->snPhase[o] += 0.25 * increment;
        wrapped         = vc->snPhase[o] >= 1.0;
        vc->snPhase[o] -= wrapped ? 2.0 : 0.0;
        vc->snCount[o]  = wrapped ? 0u : ((vc->snCount[o] + 1u) & 7u);

        if (vc->snCount[o] == 0u) {
            vc->snState[o] = wrapped ? os->syncSeed : (((vc->snState[o] * NOISE_LCG_MUL) + NOISE_LCG_ADD) & 0xFFFFFFu);
        }
        sum            += nl3_word_value((tNl3Word)((int32_t)(vc->snState[o] << 8) >> 8));
    }

    double out = 0.5 * (vc->snOut[o] + vc->snAvg[o]);

    vc->snAvg[o] = 0.25 * sum;
    vc->snOut[o] = out;
    return out;
}

// voices notes §26-27: how far a read's delay moves from 128 samples, from its word - a ramp, wrapping
static double unison_offset(uint32_t word) {
    int32_t signedWord = (int32_t)(word << 8) >> 8;

    return ((double)signedWord * UNISON_SPAN) / 16777216.0;
}

static void render_sub_block(tNl3Voices * v, const tNl3Sound * sound, double cutoff, double res, const tNl3EnvShape * amp, const tNl3EnvShape * mods,
                             float * out, float * outRight, uint32_t frames, double gain) {
    double driveW = drive_word(sound->overdrive);

    for (uint32_t i = 0; i < NL3_MAX_VOICES; i++) {
        tNl3Voice *          vc         = &v->voice[i];

        if (!vc->active) {
            continue;
        }
        // voices notes §25: the voice's own sound when a morph moves it, with its own envelope laws
        const tNl3Sound *    vs         = v->voiceMorphed[i] ? &v->voiceSound[i] : sound;
        tNl3EnvShape         voiceAmp, voiceMods[NL3_MOD_ENVS];

        if (vs != sound) {
            nl3_env_shape(vs->ampAttack, vs->ampDecay, vs->ampSustain, vs->ampRelease, true, vs->ampExponential, v->rate, &voiceAmp);

            for (int e = 0; e < NL3_MOD_ENVS; e++) {
                nl3_env_shape(vs->env[e].attack, vs->env[e].decay, vs->env[e].sustain, vs->env[e].release, false, vs->env[e].exponential, v->rate, &voiceMods[e]);
                voiceMods[e].repeat = vs->env[e].repeat;
            }
        }
        const tNl3EnvShape * vAmp       = (vs != sound) ? &voiceAmp : amp;
        const tNl3EnvShape * vMods      = (vs != sound) ? voiceMods : mods;
        double               lfoCut, lfoMix, lfoAmp, lfoCut2, lfoOscMod, lfoRes, lfoPan;
        double               width[NL3_OSCS];

        lfo_advance(vc, vs, (double)frames / v->rate, &lfoCut, width, &lfoMix, &lfoAmp, &lfoCut2, &lfoOscMod, &lfoRes, &lfoPan);

        for (int o = 0; (o < 2) && (vs->vibratoSemis != 0.0); o++) {
            vc->lfoSemis[o] += sin(2.0 * M_PI * v->vibratoPhase) * vs->vibratoSemis;   // voices notes §31
        }

        // voices notes §24: the Mod envelope (Env 3) where it is at the start of the sub-block
        double               modEnv     = env_one_way(vc, vs, 1);  // voices notes §41, §43: at the one-way destinations
        double               modEnvBoth = env_both_ways(vc, vs);   // and at pitch and Osc Mix

        lfoMix += modEnvBoth * vs->modEnvMix;
        // osc notes §17: Osc Mod's depth, the square of its Amount (127 as 128), the Mod Env adding steps first
        double               fmAmount   = fmin(127.0, fmax(0.0, vs->oscModAmount + (modEnv * vs->modEnvOscMod) + lfoOscMod));  // voices notes §39: the LFOs too
        double               fmDepth    = ((vs->oscModType == 3) || (vs->oscModType == 4)) ? ((fmAmount >= 127.0) ? 1.0 : ((fmAmount / 128.0) * (fmAmount / 128.0))) : 0.0;
        tNl3OscMod           oscMod;

        osc_mod_setup(vs->oscModType, fmAmount, vc->increment[0], &oscMod);   // osc notes §18

        for (int o = 0; o < NL3_OSCS; o++) {
            vc->lfoSemis[o] += modEnvBoth * vs->modEnvPitch[o];
            width[o]        += modEnv * vs->modEnvWidth[o];
        }

        if (vc->glide != 0.0) {
            advance_glide(vc, vs, (double)frames / v->rate);
        }
        set_pitch(vc, vs);
        double               stretch[NL3_OSCS];
        double               level      = velocity_level(vc->velocity, vs->ampVelocity);
        tNl3FilterCoeffs     coeffs;

        // voices notes §9
        double               moved      = (((double)vc->note + vs->keyShift - TRACK_PIVOT) * vs->tracking) + (vs->wheel * vs->wheelCutoff * ENV_TO_CUTOFF) + lfoCut;

        for (int e = 0; e < NL3_MOD_ENVS; e++) {
            moved += env_one_way(vc, vs, e) * vs->env[e].amount * ((vs->env[e].velocity > 64.0) ? (vc->velocity * 127.0 / 128.0) : 1.0);   // voices notes §23, §41
        }

        // voices notes §32: a morph's offsets on the panel's values; the wheel itself steps (plugin notes §18)
        double               cutTo      = vs->cutoff - sound->cutoff;
        double               resTo      = vs->resonance - sound->resonance;
        double               levelTo    = (sound->level > 0.0) ? (nl3_volume_gain(vs->level) / nl3_volume_gain(sound->level)) : 1.0;

        vc->morphCut   = cutTo;
        vc->morphRes   = resTo;
        vc->morphLevel = levelTo;
        level         *= vc->morphLevel;
        double               voiceRes   = fmin(127.0, fmax(0.0, res + vc->morphRes + (vs->wheel * vs->wheelResonance) + lfoRes)); // voices notes §46

        bool                 multiOn    = vs->filterMulti && (vs->filter2Type <= 5);                                              // filter notes §15-16, §20
        tNl3FilterMode       fmode      = multiOn ? kMultiModes[vs->filter2Type]
                                                 : nl3_filter_mode(vs->filterType, vs->filterSlope);
        double               cut1       = nl3_filter_knob(cutoff) + vc->morphCut + moved;  // filter notes §4
        double               cut2       = nl3_filter_knob(vs->cutoff2) + (vs->filter2Env ? moved : (((double)vc->note + vs->keyShift - TRACK_PIVOT) * vs->tracking));

        // voices notes §39: what the LFOs and Mod Env send to Filter 2 alone (with Filter 2 Env on, "both" is in moved already)
        cut2          += lfoCut2 + ((vs->filter2Env && (vs->env[1].amount != 0.0)) ? 0.0 : (modEnv * vs->modEnvCutoff2));
        // filter notes §18: Osc Mod FILT, Osc 2 times d^3 / 2 onto the cutoff word, every sample
        double               filtD      = (vs->oscModType == 2) ? ((fmAmount >= 127.0) ? 1.0 : (fmAmount / 128.0)) : 0.0;
        double               filtDepth  = 0.5 * filtD * filtD * filtD;

        if (multiOn) {
            nl3_filter_coeffs_multi(fmode, cut1, cut2, voiceRes, 0.0, &coeffs);
        } else {
            nl3_filter_coeffs(fmode, cut1, voiceRes, 0.0, &coeffs);
            coeffs.dist = vs->filterDist;   // filter notes §17
        }
        double               oscLevel[NL3_OSCS];
        tNl3Word             incrementWord[NL3_OSCS];

        tNl3Word             syncWord[NL3_OSCS];
        double               syncInc[NL3_OSCS];

        for (int o = 0; o < NL3_OSCS; o++) {
            incrementWord[o] = nl3_pitch_increment_fixed(vc->pitch[o]);
            // osc notes §13a: in sync mode Shape is the sync pitch, so what modulates Shape moves it, in knob steps
            double tune = fmin(127.0, fmax(0.0, vs->osc[o].syncTune + ((width[o] - vs->osc[o].width) * 128.0)));

            syncWord[o]      = vs->osc[o].sync ? nl3_pitch_increment_fixed(vc->pitch[o] + (tune / 2.0)) : incrementWord[o]; // osc notes §13
            syncInc[o]       = vs->osc[o].sync ? (vc->increment[o] * pow(2.0, tune / 24.0)) : vc->increment[o];
            stretch[o]       = nl3_osc_stretch(vc->increment[o], vs->osc[o].soften);
            oscLevel[o]      = fmin(1.0, fmax(0.0, vs->osc[o].level + (vs->wheel * vs->osc[o].wheelLevel)));
        }

        // voices notes §36: Osc Mix's two gains, 1 - x^2 and 2x - x^2, its modulation added to x first
        double               mixX     = fmin(1.0, fmax(0.0, vs->mixX + lfoMix));
        double               noiseMix = (vs->noiseLevel > 0.0) ? ((2.0 * mixX) - (mixX * mixX)) : 0.0;

        oscLevel[0] = 1.0 - (mixX * mixX);
        oscLevel[1] = (vs->osc[1].level > 0.0) || (vs->noiseLevel <= 0.0) ? ((2.0 * mixX) - (mixX * mixX)) : 0.0;

        for (uint32_t f = 0; f < frames; f++) {
            double mix = 0.0;

            for (int e = 0; e < NL3_MOD_ENVS; e++) {
                (void)nl3_env_next(&vc->env[e], &vMods[e]);
            }

            double wave[NL3_OSCS];

            for (int o = 0; o < NL3_OSCS; o++) {
                const tNl3OscSound * os   = &vs->osc[o];
                tNl3Word             incW = syncWord[o];   // the increment it sounds at (osc notes §13)
                double               inc  = syncInc[o];

                // osc notes §17: Osc Mod FM adds Osc 2's last output to Osc 1's increment, DFM its last change
                if ((o == 0) && (fmDepth > 0.0)) {
                    double add = OSC_MOD_SOURCE * ((vs->oscModType == 3) ? (4.0 * fmDepth * vc->fmLast[0]) : (16.0 * fmDepth * (vc->fmLast[1] - vc->fmLast[0])));

                    inc += add;
                    incW = nl3_word(fmax(-0.999, fmin(0.999, nl3_word_value(incW) + add)));
                }

                if (os->syncNoise) {                   // osc notes §22
                    wave[o] = sync_noise(vc, o, os, vc->increment[o]);
                } else if (os->noise2) {               // osc notes §20
                    wave[o] = osc2_noise(vc, os, inc, fmin(nl3_word_value(NL3_WORD_MAX), fmax(0.0, width[o])));
                } else if (os->dualSine) {             // osc notes §19
                    wave[o] = dual_sine(vc, o, os, inc, syncInc[o], fmin(nl3_word_value(NL3_WORD_MAX), fmax(0.0, width[o])));
                } else if (os->noise) {                // osc notes §15
                    wave[o] = osc_noise(vc, o, fmin(127.0, fmax(0.0, width[o] * 128.0)));
                } else if (os->shape == eNl3OscSine) { // osc notes §10, §12
                    double shape = fmin(nl3_word_value(NL3_WORD_MAX), fmax(0.0, width[o]));

                    if (v->fixedPoint) {
                        tNl3Word p   = advance_word(vc, o, os, incrementWord[o], incW);
                        tNl3Word out = nl3_sine_fixed(p);

                        p            = nl3_sine_feedback_fixed(p, nl3_word(shape), nl3_word(vc->sineLast[o]), out);
                        vc->phase[o] = nl3_word_value(p);
                        wave[o]      = nl3_word_value(out);
                    } else {
                        vc->phase[o] = advance(vc, o, os, vc->increment[o], inc);
                        wave[o]      = nl3_sine(vc->phase[o]);
                        vc->phase[o] = nl3_sine_feedback(vc->phase[o], shape, vc->sineLast[o], wave[o]);
                    }
                    vc->sineLast[o] = wave[o];
                } else if (os->shape == eNl3OscSquare) {   // osc notes §11
                    double w = fmin(nl3_word_value(NL3_WORD_MAX), fmax(0.0, width[o]));

                    if (v->fixedPoint && !v->clean) {
                        tNl3Word p = advance_word(vc, o, os, incrementWord[o], incW);

                        vc->phase[o] = nl3_word_value(p);
                        wave[o]      = nl3_word_value(nl3_pulse_fixed(&v->steps, p, incW, nl3_word(w)));
                    } else {
                        vc->phase[o] = advance(vc, o, os, vc->increment[o], inc);
                        wave[o]      = nl3_pulse(&v->steps, vc->phase[o], inc, w, v->clean);
                    }
                } else if ((os->shape == eNl3OscSaw) || (os->shape == eNl3OscTriangle)) {
                    // osc notes §5-7, §9: the saw and the triangle take their phase advanced
                    bool tri = os->shape == eNl3OscTriangle;

                    if (v->fixedPoint && !v->clean) {
                        tNl3Word p = advance_word(vc, o, os, incrementWord[o], incW);

                        vc->phase[o] = nl3_word_value(p);
                        wave[o]      = nl3_word_value(tri ? nl3_triangle_fixed(&v->steps, p, incW) : nl3_saw_fixed(&v->steps, p, incW));
                    } else {
                        vc->phase[o] = advance(vc, o, os, vc->increment[o], inc);
                        wave[o]      = tri ? nl3_triangle(&v->steps, vc->phase[o], inc) : nl3_saw(&v->steps, vc->phase[o], inc, v->clean);
                    }
                } else {
                    wave[o]      = nl3_osc_sample(v->edge, os->shape, vc->phase[o], stretch[o], fmin(2.0, fmax(0.0, width[o])));
                    vc->phase[o] = advance(vc, o, os, vc->increment[o], inc);
                }

                // osc notes §13: a synced oscillator sounds a sample late, faded out and back in over the four
                // samples either side of its restart
                if (os->sync) {
                    double hiddenInc = v->fixedPoint ? nl3_word_value(incrementWord[o]) : vc->increment[o];
                    double fade      = (hiddenInc > 0.0) ? fmin(1.0, fabs(vc->syncPhase[o]) / (4.0 * hiddenInc)) : 1.0;
                    double now       = wave[o];

                    wave[o]         = fade * vc->syncLast[o];
                    vc->syncLast[o] = now;
                }

                if (o >= 2) {
                    mix += wave[o] * oscLevel[o];
                }
            }

            // voices notes §38: with Osc Bypass, Osc 2 goes round the filter - or with Noise3 both oscillators, the
            // noise alone going through
            bool   aside = vs->oscBypass && (vs->noiseLevel <= 0.0);
            double oscs  = osc_mod_mix(vc, &oscMod, MIX_SCALE * wave[0] * oscLevel[0], aside ? 0.0 : (MIX_SCALE * wave[1] * oscLevel[1]), wave[1]) / MIX_SCALE; // osc notes §18: at the instrument's mix level
            double noise = noise3(vc) * noiseMix;                                                                                                               // voices notes §44
            double round = 0.0;

            if (aside) {
                round = wave[1] * oscLevel[1];
            } else if (vs->oscBypass) {
                round = mix + oscs;
                mix   = 0.0;
                oscs  = 0.0;
            }
            mix                 += oscs + noise;
            mix                 += RING_GAIN * wave[2] * ((wave[0] * vs->ring13) + (wave[1] * vs->ring23));
            vc->fmLast[1]        = vc->fmLast[0];                                  // osc notes §17
            vc->fmLast[0]        = vs->osc[1].dualSine ? vc->dualMod[1] : wave[1]; // osc notes §19: a Dual Sine hands Osc Mod its own signal

            if (filtDepth > 0.0) {   // filter notes §18: from Osc 2's last sample, as the instrument's next coefficients take it
                double offset = filtDepth * vc->fmLast[1];

                if (multiOn) {
                    nl3_filter_coeffs_multi(fmode, cut1, cut2, voiceRes, offset, &coeffs);
                } else {
                    nl3_filter_coeffs(fmode, cut1, voiceRes, offset, &coeffs);
                    coeffs.dist = vs->filterDist;
                }
            }
            double x   = drive(mix * DRIVE_IN, driveW) * FILTER_IN;
            double y   = FILTER_OUT * (v->fixedPoint ? nl3_word_value(nl3_filter_process_fixed(&vc->filter, &coeffs, nl3_word(x))) : nl3_filter_process(&vc->filter, &coeffs, x));

            y                   += FILTER_OUT * (round * DRIVE_IN * FILTER_IN) / 4.0; // voices notes §38: added to the filter's output at a quarter

            double dry = y * nl3_env_next(&vc->amp, vAmp) * level * gain * lfoAmp;

            // voices notes §26: Unison - the voice beside two copies read at ramping delays, one each side
            vc->ring[vc->ringAt] = (float)dry;
            double left = dry, right = dry;

            if (vs->unison) {
                double first  = unison_offset(vc->delayWord[0]);
                double second = unison_offset(vc->delayWord[1]);

                // voices notes §27: each side the voice beside two copies, one read ahead of 128 samples and one as far
                // behind - one sharp, one as flat; the full step's pair on the right
                left             = 0.5 * (dry + ring_read(vc, 128.0 - second) + ring_read(vc, 128.0 + second));
                right            = 0.5 * (dry + ring_read(vc, 128.0 - first) + ring_read(vc, 128.0 + first));
                vc->delayWord[0] = (vc->delayWord[0] + (uint32_t)lround(vs->unisonStep)) & 0xFFFFFFu;
                vc->delayWord[1] = (vc->delayWord[1] + (uint32_t)lround(vs->unisonStep * UNISON_SECOND)) & 0xFFFFFFu;
            }
            vc->ringAt           = (vc->ringAt + 1u) & 255u;

            left                *= 1.0 + lfoPan; // voices notes §46: LFO 2's pan, linear
            right               *= 1.0 - lfoPan;

            if (outRight != NULL) {
                out[f]      += (float)left;
                outRight[f] += (float)right;
            } else {
                out[f] += (float)(0.5 * (left + right));
            }
        }

        if (nl3_env_idle(&vc->amp)) {
            vc->active = false;
            vc->gate   = false;
        }
    }
}

void nl3_voices_render(tNl3Voices * v, const tNl3Sound * sound, float * out, float * outRight, uint32_t frames, double gain) {
    if (v->pedal && !sound->pedal) {
        for (uint32_t n = 0; n < 128; n++) {
            if (v->pedalled[n]) {
                v->pedalled[n] = false;
                release_note(v, sound, (uint8_t)n);
            }
        }
    }
    v->pedal = sound->pedal;

    smooth_toward(&v->cutoff, sound->cutoff, v->rate);
    smooth_toward(&v->resonance, sound->resonance, v->rate);
    tNl3EnvShape amp;

    nl3_env_shape(sound->ampAttack, sound->ampDecay, sound->ampSustain, sound->ampRelease, true, sound->ampExponential, v->rate, &amp);
    tNl3EnvShape mods[NL3_MOD_ENVS];

    for (int e = 0; e < NL3_MOD_ENVS; e++) {
        const tNl3ModEnv * m = &sound->env[e];

        nl3_env_shape(m->attack, m->decay, m->sustain, m->release, false, m->exponential, v->rate, &mods[e]);
        mods[e].repeat = m->repeat;
    }

    // notes §4: the coefficients every sub-block, the same for every voice until keyfollow and envelopes come
    for (uint32_t done = 0; done < frames; done += SUB_BLOCK) {
        uint32_t n      = ((frames - done) < SUB_BLOCK) ? (frames - done) : SUB_BLOCK;
        double   cutoff = smooth_advance(&v->cutoff, n);
        double   res    = smooth_advance(&v->resonance, n);

        render_sub_block(v, sound, cutoff, res, &amp, mods, out + done, (outRight != NULL) ? (outRight + done) : NULL, n, gain);

        for (int l = 0; l < NL3_LFOS; l++) {
            v->lfoFree[l] += sound->lfo[l].hz * ((double)n / v->rate);
            v->lfoFree[l] -= floor(v->lfoFree[l]);
        }

        v->vibratoPhase += sound->vibratoHz * ((double)n / v->rate);
        v->vibratoPhase -= floor(v->vibratoPhase);
    }
}
