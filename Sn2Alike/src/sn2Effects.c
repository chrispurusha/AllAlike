/*
 * Sn2Alike - the effects section.
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
// Notes: Docs/code-notes/sn2Effects.md - "// notes §k" refers there.

#include <math.h>
#include <string.h>

#include "sn2Effects.h"
#include "sn2Filter.h"

#define DRY_AT_FULL       (1.4227)  // notes §1
#define TAP_AT_FULL       (1.03)    // notes §2
#define TAP_LEAST_MS      (0.05)
#define QUAD_EXTRA_MS     (0.2)
#define MS_PER_DEPTH      (0.06)
#define SLOW_TAP_RATIO    (1.18)
#define FEEDBACK_SCALE    (1.0)     // notes §3: an estimate

#define REVERB_LEVEL      (0.12)    // notes §5
#define REVERB_HIGH_HZ    (8000.0)
#define REVERB_MEAN_S     (0.056)
#define DIST_POINTS       (17)
#define DIST_UNITY        (1.93)    // notes §7: effects on against bypass at Level 0, Output 64
#define OUTPUT_CEILING    (0.97675)
#define EQ_LOW_HZ         (500.0)   // notes §8
#define EQ_HIGH_HZ        (2200.0)
#define MS_PER_TIME       (6.592)   // notes §4
#define LONGEST_MS        (838.8)
#define DAMP_SLOPE        (0.00685) // notes §4

// notes §4: a side's share of the time, 0 silent, by Ratio
static const double kRatio[13][2]  = {
    {1.0,       1.0}, {      1.0, 0.75}, {0.75,  1.0}, { 1.0, 2.0 / 3.0}, {2.0 / 3.0, 1.0}, {1.0, 0.5}, {0.5, 1.0},
    {1.0, 1.0 / 3.0}, {1.0 / 3.0,  1.0}, { 1.0, 0.25}, {0.25,       1.0}, {      1.0, 0.0}, {0.0, 1.0},
};

// notes §4: beats, by Sync; 0 is Off
static const double kSyncBeats[35] = {
    0.0,  1.0 / 12.0, 1.0 / 8.0, 1.0 / 6.0,  1.0 / 4.0, 1.0 / 3.0, 3.0 / 8.0,  1.0 / 2.0, 2.0 / 3.0,  3.0 / 4.0,  1.0, 4.0 / 3.0,  3.0 / 2.0,
    2.0,   8.0 / 3.0,       3.0,       4.0, 16.0 / 3.0,       6.0,       8.0, 32.0 / 3.0,      12.0, 40.0 / 3.0, 16.0,      18.0, 56.0 / 3.0,20.0, 64.0 / 3.0, 24.0, 28.0, 30.0,
    32.0,       36.0,      42.0,      48.0,
};

// notes §2: L's two taps, then R's: the slow one first on L, the fast one first on R
static const double kStartPhase[4] = {0.60, 0.16, 0.69, 0.14};

void sn2_effects_bypass(tSn2Effects * e, const float * in, float * left, float * right, uint32_t frames) {
    for (uint32_t f = 0; f < frames; f++) {
        e->clipped = e->clipped || (fabs((double)in[f]) > OUTPUT_CEILING);
        left[f]    = (float)fmin(OUTPUT_CEILING, fmax(-OUTPUT_CEILING, (double)in[f]));
        right[f]   = left[f];
    }
}

void sn2_effects_reset(tSn2Effects * e) {
    memset(e, 0, sizeof(*e));

    for (int i = 0; i < 4; i++) {
        e->phase[i] = kStartPhase[i];
    }
}

// notes §2: seconds a cycle for the fast tap
static double chorus_period(double speed) {
    static const double kSpeed[]  = {15.0, 27.0, 40.0};
    static const double kPeriod[] = {6.33, 1.757, 0.802};
    int                 k         = (speed < kSpeed[1]) ? 1 : 2;
    double              t         = (speed - kSpeed[k - 1]) / (kSpeed[k] - kSpeed[k - 1]);

    return exp(log(kPeriod[k - 1]) + (t * (log(kPeriod[k]) - log(kPeriod[k - 1]))));
}

static double triangle(double phase) {
    return 1.0 - (4.0 * fabs(phase - 0.5));   // -1 at 0, +1 at a half
}

static double read_tap(const tSn2Effects * e, double samples) {
    double   back = fmin(fmax(samples, 1.0), (double)SN2_CHORUS_LINE - 2.0);
    uint32_t i    = (uint32_t)back;
    double   f    = back - i;
    float    a    = e->line[(e->write + SN2_CHORUS_LINE - i) % SN2_CHORUS_LINE];
    float    b    = e->line[(e->write + SN2_CHORUS_LINE - i - 1u) % SN2_CHORUS_LINE];

    return a + (f * (b - a));
}

// notes §5: seconds to fall 60 dB at Decay 20, 50, 86 and 127, by type
static const double kReverbT60[16][4]               = {
    {0.250, 0.313, 0.466,  0.561},
    {0.250, 0.313, 0.466,  0.561},
    {0.250, 0.313, 0.466,  0.561},
    {0.250, 0.313, 0.466,  0.561},
    {0.328, 0.371, 0.502,  0.832},
    {0.418, 0.524, 0.751,  1.297},
    {0.643, 0.811, 1.188,  2.114},
    {0.780, 1.012, 1.586,  3.128},
    {1.269, 1.853, 3.080,  6.991},
    {1.041, 1.538, 2.427,  6.336},
    {1.237, 1.943, 3.269,  9.126},
    {1.068, 1.583, 2.670,  8.386},
    {1.742, 2.842, 5.234, 17.952},
    {1.527, 2.297, 4.103, 14.376},
    {1.754, 2.814, 5.148, 18.000},
    {1.586, 2.278, 4.055, 14.354},
};
// notes §5: milliseconds to the fullest point, by type
static const double kReverbPeakMs[16]               = {270.0, 131.0, 131.0, 131.0, 108.0, 130.0, 162.0, 184.0, 120.0, 238.0, 100.0, 190.0, 149.0, 285.0, 94.0, 228.0};
static const double kReverbLevel[16]                = {3.262, 3.262, 3.262, 3.262, 2.336, 2.140, 1.814, 1.554, 1.546, 1.411, 1.558, 1.185, 1.457, 0.978, 1.362, 1.000};
static const int    kReverbLength[SN2_REVERB_LINES] = {1423, 1667, 1871, 2083, 2339, 2579, 2851, 3137};
static const double kReverbTap[SN2_REVERB_LINES]    = {0.20, 0.45, 0.70, 0.95, 0.30, 0.55, 0.80, 1.05};
static const double kReverbLeft[SN2_REVERB_LINES]   = {1.0, -1.0, 1.0, 1.0, -1.0, 1.0, 1.0, 1.0};
static const double kReverbRight[SN2_REVERB_LINES]  = {1.0, -1.0, -1.0, 1.0, 1.0, -1.0, 1.0, 1.0};

static double reverb_t60(int type, double decay) {
    static const double kDecay[4] = {20.0, 50.0, 86.0, 127.0};
    int                 k         = (decay <= kDecay[1]) ? 1 : ((decay <= kDecay[2]) ? 2 : 3);
    double              t         = (decay - kDecay[k - 1]) / (kDecay[k] - kDecay[k - 1]);

    return exp(log(kReverbT60[type][k - 1]) + (t * (log(kReverbT60[type][k]) - log(kReverbT60[type][k - 1]))));
}

// notes §5: the shortest rooms need a shorter loop
static double reverb_size(int type) {
    return (type < 6) ? 0.35 : 1.0;
}

// notes §5: the one-pole pass that takes `loss` dB off the top of the band in one trip
static double reverb_pass(double loss) {
    const double w      = 2.0 * M_PI * REVERB_HIGH_HZ / SN2_ENGINE_HZ;
    const double target = pow(10.0, -loss / 20.0);
    double       low = 0.01, high = 1.0;

    for (int i = 0; i < 24; i++) {
        double c    = (low + high) / 2.0;
        double gain = c / sqrt(1.0 + ((1.0 - c) * (1.0 - c)) - (2.0 * (1.0 - c) * cos(w)));

        if (gain > target) {
            high = c;
        } else {
            low = c;
        }
    }

    return high;
}

static void reverb_tick(tSn2Effects * e, const tSn2EffectsSound * s, double in, const double * gains, double pass, double peak, double * left, double * right) {
    double sum = 0.0, read[SN2_REVERB_LINES];
    double wet  = kReverbLevel[s->reverbType] * REVERB_LEVEL * sqrt(reverb_t60(s->reverbType, 86.0) / reverb_t60(s->reverbType, s->reverbDecay));
    double size = reverb_size(s->reverbType);

    e->reverbInput[e->reverbInputWrite] = (float)in;
    *left                               = 0.0;
    *right                              = 0.0;

    for (int i = 0; i < SN2_REVERB_LINES; i++) {
        uint32_t at = (e->reverbWrite + SN2_REVERB_LINE - (uint32_t)(kReverbLength[i] * size)) % SN2_REVERB_LINE;

        e->reverbLow[i] += pass * (e->reverbLine[i][at] - e->reverbLow[i]);
        read[i]          = e->reverbLow[i] * gains[i];
        sum             += read[i];
        *left           += read[i] * kReverbLeft[i];
        *right          += read[i] * kReverbRight[i];
    }

    sum                                *= 2.0 / SN2_REVERB_LINES;

    for (int i = 0; i < SN2_REVERB_LINES; i++) {
        uint32_t back = (uint32_t)(peak * kReverbTap[i]);
        uint32_t at   = (e->reverbInputWrite + SN2_REVERB_INPUT - ((back < SN2_REVERB_INPUT - 1u) ? back : (SN2_REVERB_INPUT - 2u))) % SN2_REVERB_INPUT;

        e->reverbLine[i][e->reverbWrite] = (float)(read[i] - sum + (e->reverbInput[at] * ((i & 1) ? -1.0 : 1.0)));
    }

    e->reverbWrite                      = (e->reverbWrite + 1u) % SN2_REVERB_LINE;
    e->reverbInputWrite                 = (e->reverbInputWrite + 1u) % SN2_REVERB_INPUT;
    *left                              *= wet;
    *right                             *= wet;
}

#define FX_DELAY     (0)
#define FX_REVERB    (1)
#define FX_CHORUS    (2)
#define D            (1 << FX_DELAY)
#define R            (1 << FX_REVERB)
#define C            (1 << FX_CHORUS)

// notes §6: the order to run them in, by Effects Order, then what feeds each of delay, reverb and chorus
static const int kOrder[19][3] = {
    {0, 1, 2}, {0, 1, 2}, {0, 2, 1}, {1, 0, 2}, {1, 2, 0}, {2, 0, 1}, {2, 1, 0}, {0, 1, 2}, {0, 2, 1}, {1, 0, 2}, {1, 2, 0},
    {2, 0, 1}, {2, 1, 0}, {0, 1, 2}, {1, 0, 2}, {2, 0, 1}, {1, 2, 0}, {0, 2, 1}, {0, 1, 2},
};
static const int kFeeds[19][3] = {
    {0, 0, 0}, {0, D, R}, {0, C, D}, {R, 0, D}, {C, 0, R}, {C,     D, 0}, {R, C,     0}, {0, 0, R    }, {0, C, 0}, {0, 0, D}, {C, 0, 0},
    {0, D, 0}, {R, 0, 0}, {0, D, D}, {R, 0, R}, {C, C, 0}, {R | C, 0, 0}, {0, D | C, 0}, {0, 0, D | R},
};

static double read_delay(const tSn2Effects * e, uint32_t samples) {
    uint32_t back = (samples < SN2_DELAY_LINE - 1u) ? samples : (SN2_DELAY_LINE - 2u);

    return e->delayLine[(e->delayWrite + SN2_DELAY_LINE - back) % SN2_DELAY_LINE];
}

static double delay_ms(const tSn2EffectsSound * s) {
    int sync = (s->delaySync < 0) ? 0 : ((s->delaySync > 34) ? 34 : s->delaySync);

    if (sync == 0) {
        return MS_PER_TIME * s->delayTime;
    }
    return fmin(kSyncBeats[sync] * 60000.0 / s->tempo, LONGEST_MS);
}

// notes §7: the drive and softness of the curve, measured at these Levels
static double dist_between(const double * values, double level) {
    static const double kLevel[7] = {0.0, 8.0, 30.0, 60.0, 90.0, 112.0, 127.0};

    for (int k = 1; k < 7; k++) {
        if (level <= kLevel[k]) {
            double t = (level - kLevel[k - 1]) / (kLevel[k] - kLevel[k - 1]);

            return exp(log(values[k - 1]) + (t * (log(values[k]) - log(values[k - 1]))));
        }
    }

    return values[6];
}

// notes §7: the input stage and the curve, as one soft-limited line at each measured Level
static double dist_table(const double * values, double level) {
    static const double kStep = 8.0;
    int                 k     = (int)fmin(floor(level / kStep), DIST_POINTS - 2.0);
    double              top   = (k == (DIST_POINTS - 2)) ? 127.0 : (double)((k + 1) * 8);
    double              t     = (level - (double)(k * 8)) / (top - (double)(k * 8));

    return exp(log(values[k]) + (t * (log(values[k + 1]) - log(values[k]))));
}

static double dist_curve(double x, double drive, double soft, double ceiling) {
    double driven = drive * x;

    return (ceiling * driven / pow(1.0 + pow(fabs(driven), soft), 1.0 / soft)) / DIST_UNITY;
}

// notes §8: a shelf's gain from its dial, between measured points
static double shelf_gain(const double * gains, double dial) {
    static const double kDial[5] = {0.0, 32.0, 64.0, 94.0, 127.0};

    for (int k = 1; k < 5; k++) {
        if (dial <= kDial[k]) {
            return gains[k - 1] + ((gains[k] - gains[k - 1]) * (dial - kDial[k - 1]) / (kDial[k] - kDial[k - 1]));
        }
    }

    return gains[4];
}

void sn2_effects_process(tSn2Effects * e, const tSn2EffectsSound * s, const float * in, float * left, float * right, uint32_t frames) {
    if (s->bypass) {
        memcpy(left, in, (size_t)frames * sizeof(float));
        memcpy(right, in, (size_t)frames * sizeof(float));
        return;
    }
    double              dry                  = DIST_UNITY * s->dry / 127.0;
    double              tap                  = TAP_AT_FULL * s->chorusSend / 127.0;
    double              period               = chorus_period(s->chorusSpeed) * SN2_ENGINE_HZ;
    double              step4[4]             = {1.0 / (period * SLOW_TAP_RATIO), 1.0 / period, 1.0 / period, 1.0 / (period * SLOW_TAP_RATIO)};
    double              reach                = s->chorusDepth * MS_PER_DEPTH * SN2_ENGINE_HZ / 1000.0;
    double              least                = (TAP_LEAST_MS + ((s->chorusType == 0) ? QUAD_EXTRA_MS : 0.0)) * SN2_ENGINE_HZ / 1000.0;
    double              feedback             = FEEDBACK_SCALE * (s->chorusFeedback - 64.0) / 64.0;

    double              send                 = s->delayOn ? (s->delaySend / 127.0) : 0.0;
    double              echo                 = DRY_AT_FULL;
    double              loop                 = s->delayFeedback / 128.0;
    double              pass                 = 1.0 - (DAMP_SLOPE * s->delayDamp);
    double              cross                = 1.0 - (s->delayWidth / 127.0);
    int                 ratio                = ((s->delayRatio >= 0) && (s->delayRatio < 13)) ? s->delayRatio : 0;
    uint32_t            full                 = (uint32_t)lround(delay_ms(s) * SN2_ENGINE_HZ / 1000.0);
    uint32_t            times[2]             = {(uint32_t)lround(full * kRatio[ratio][0]), (uint32_t)lround(full * kRatio[ratio][1])};
    int                 kind                 = (s->reverbType < 0) ? 0 : ((s->reverbType > 15) ? 15 : s->reverbType);
    double              t60                  = reverb_t60(kind, s->reverbDecay);
    double              rsend                = s->reverbOn ? (s->reverbSend / 127.0) : 0.0;
    double              peakAt               = kReverbPeakMs[kind] * SN2_ENGINE_HZ / 1000.0;
    double              highLoss             = (60.0 * REVERB_MEAN_S / t60) * ((1.0 / (1.0 - (0.67 * pow(s->reverbDamp / 127.0, 0.8)))) - 1.0);
    double              rpass                = reverb_pass(highLoss);
    double              gains[SN2_REVERB_LINES];
    tSn2EffectsSound    fixedType            = *s;

    fixedType.reverbType = kind;

    for (int i = 0; i < SN2_REVERB_LINES; i++) {
        gains[i] = pow(10.0, -3.0 * kReverbLength[i] * reverb_size(kind) / (SN2_ENGINE_HZ * t60));
    }

    int                 order                = ((s->order >= 0) && (s->order < 19)) ? s->order : 0;
    static const double kDrive[7]            = {1.0, 1.066, 1.35, 2.25, 6.5, 35.0, 60.0}; // Compensation's law only
    static const double kCurveK[DIST_POINTS] = {1.868, 1.75, 1.646, 1.278, 1.476, 1.667, 2.22, 2.724, 3.67, 4.85, 6.746, 9.37, 15.0, 24.396, 48.35, 109.324, 156.745};
    static const double kCurveS[DIST_POINTS] = {5.824, 4.0, 2.502, 1.601, 1.289, 1.055, 1.273, 1.027, 1.121, 1.153, 1.196, 1.231, 1.24, 1.256, 1.319, 1.642, 2.534};
    static const double kCurveC[DIST_POINTS] = {1.0708, 1.15, 1.2346, 1.6465, 1.5348, 1.5055, 1.1027, 1.0575, 0.8265, 0.6905, 0.5733, 0.4959, 0.44, 0.3915, 0.3619, 0.3465, 0.3435};
    static const double kBass[5]             = {0.0, 0.5, 1.0, 1.3, 1.6};
    static const double kTreb[5]             = {0.19, 0.42, 1.0, 2.1, 3.3};
    double              drive                = dist_between(kDrive, s->distLevel);
    double              curveK               = dist_table(kCurveK, s->distLevel);
    double              curveS               = dist_table(kCurveS, s->distLevel);
    double              curveC               = dist_table(kCurveC, s->distLevel);
    double              comp                 = exp(-(s->distComp - 64.0) * ((s->distComp > 64.0) ? 0.0105 : 0.0059) * log(drive));
    static const double kLiftAt[10]          = {0.0, 8.0, 16.0, 32.0, 48.0, 64.0, 80.0, 96.0, 112.0, 127.0};
    static const double kLiftDb[10]          = {0.0, 0.2, 0.8, 2.2, 3.0, 4.5, 6.4, 7.7, 8.8, 9.2};
    double              lift                 = 0.0;

    for (int k = 1; k < 10; k++) {
        if (s->distLevel <= kLiftAt[k]) {
            lift = kLiftDb[k - 1] + ((kLiftDb[k] - kLiftDb[k - 1]) * (s->distLevel - kLiftAt[k - 1]) / (kLiftAt[k] - kLiftAt[k - 1]));
            break;
        }
    }

    double              distOut              = comp * pow(10.0, lift / 20.0) * s->distOutput / 64.0;
    double              bassGain             = shelf_gain(kBass, s->eqBass);
    double              trebGain             = shelf_gain(kTreb, s->eqTreble);
    double              lowPass              = 1.0 - exp(-2.0 * M_PI * EQ_LOW_HZ / SN2_ENGINE_HZ);
    double              highPass             = 1.0 - exp(-2.0 * M_PI * EQ_HIGH_HZ / SN2_ENGINE_HZ);

    double              panSwing             = 0.5 * s->panDepth / 127.0;
    double              panStep              = s->panSpeed * s->panSpeed / (642.5 * SN2_ENGINE_HZ);

    for (uint32_t f = 0; f < frames; f++) {
        double wetL[3] = {0.0, 0.0, 0.0}, wetR[3] = {0.0, 0.0, 0.0};
        double voices = dist_curve(in[f], curveK, curveS, curveC) * distOut;

        // notes §6: each effect hears the voices at its send, and what feeds it at the same send
        for (int step = 0; step < 3; step++) {
            int    fx   = kOrder[order][step];
            double feed = voices;

            for (int from = 0; from < 3; from++) {
                if (kFeeds[order][fx] & (1 << from)) {
                    feed += (wetL[from] + wetR[from]) / 2.0;
                }
            }

            if (fx == FX_REVERB) {
                reverb_tick(e, &fixedType, feed * rsend, gains, rpass, peakAt, &wetL[fx], &wetR[fx]);
            } else if (fx == FX_DELAY) {
                double echoes[2];

                for (int side = 0; side < 2; side++) {
                    echoes[side] = (kRatio[ratio][side] > 0.0) ? read_delay(e, times[side]) : 0.0;
                }

                e->delayLow                += pass * (read_delay(e, full) - e->delayLow);
                e->delayLine[e->delayWrite] = (float)((feed * send) + (e->delayLow * loop));
                e->delayWrite               = (e->delayWrite + 1u) % SN2_DELAY_LINE;
                wetL[fx]                    = echo * (echoes[0] + (cross * echoes[1]));
                wetR[fx]                    = echo * (echoes[1] + (cross * echoes[0]));
            } else {
                double taps[4];

                e->line[e->write] = (float)(feed + (e->feedback * feedback));

                for (int i = 0; i < 4; i++) {
                    taps[i]     = read_tap(e, least + (reach * (triangle(e->phase[i]) + 1.0)));
                    e->phase[i] = fmod(e->phase[i] + step4[i], 1.0);
                }

                e->write          = (e->write + 1u) % SN2_CHORUS_LINE;
                e->feedback       = (taps[0] + taps[1] + taps[2] + taps[3]) / 4.0;
                wetL[fx]          = tap * (taps[0] + taps[1]);
                wetR[fx]          = tap * (taps[2] + taps[3]);
            }
        }

        // voices notes §16
        double place   = 0.5 + ((64.0 - s->pan) / 128.0) + (panSwing * sin(2.0 * M_PI * e->panPhase));

        e->panPhase = fmod(e->panPhase + panStep, 1.0);
        place       = fmin(1.0, fmax(0.0, place));
        double left2   = 2.0 * place;
        double right2  = 2.0 * (1.0 - place);
        double wet[2]  = {wetL[0] + wetL[1] + wetL[2], wetR[0] + wetR[1] + wetR[2]};
        double side[2] = {(voices * dry * left2) + (wet[0] * (s->panEffects ? left2 : 1.0)), (voices * dry * right2) + (wet[1] * (s->panEffects ? right2 : 1.0))};

        // notes §8
        for (int c = 0; c < 2; c++) {
            e->eqLow[c]  += lowPass * (side[c] - e->eqLow[c]);
            e->eqHigh[c] += highPass * (side[c] - e->eqHigh[c]);
            side[c]      += ((bassGain - 1.0) * e->eqLow[c]) + ((trebGain - 1.0) * (side[c] - e->eqHigh[c]));
        }

        // voices notes §14
        e->clipped  = e->clipped || (fabs(side[0]) > OUTPUT_CEILING) || (fabs(side[1]) > OUTPUT_CEILING);
        left[f]     = (float)fmin(OUTPUT_CEILING, fmax(-OUTPUT_CEILING, side[0]));
        right[f]    = (float)fmin(OUTPUT_CEILING, fmax(-OUTPUT_CEILING, side[1]));
    }
}
