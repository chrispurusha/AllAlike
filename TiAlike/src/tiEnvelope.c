/*
 * TiAlike - the envelopes.
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
// Notes: Docs/code-notes/tiEnvelope.md - "// notes §k" refers there.

#include <math.h>

#include "tiEnvelope.h"

#define TICKS_PER_SECOND    (46875.0 / 4.0)   // notes §1
#define TOP                 (1.0)
#define RISE_TOP            (127.0 / 128.0)   // notes §4
#define FLOOR               (1.0 / 32768.0)
#define SILENT              (1.0 / 65536.0)   // notes §5
#define GLIDE_RATE          (1.0 / 4096.0)    // notes §6, per tick

// notes §7: Fixed - the curves' points as words, the update every 16 samples, the level's marks
#define WORD                (8388608)          // 2^23, a word's full scale
#define UPDATE_SAMPLES      (16u)
#define START_WORD          (0x1000)
#define ATTACK_TOP_WORD     (0x7f0000)
#define SLOPE_FLOOR_WORD    (0x100)
#define SLOPE_TOP_WORD      (0x7f0000)

static const int32_t kAttackPoints[33]  = {
    8388607, 6291456, 4194304, 2097152, 1048576, 524288, 262144, 185362, 131072, 92681, 65536, 46516, 32768, 23170, 16384, 11585, 8192,
    5792,       4096,    2896,    2048,    1440,   1024,    720,    512,    352,   256,   181,   128,    90,    64,    32, 16
};
static const int32_t kFallPoints[37]    = {
    524288, 262144, 131072, 65536, 32768, 23170, 16384, 11585, 9410, 7643, 6208, 5043, 4096, 3444, 2896, 2435, 2048, 1722, 1448, 1218,
    1024,      861,    724,   609,   512,   416,   338,   274,  223,  169,  128,   97,   74,   56,   39,   16, 0
};
static const int32_t kSustainPoints[33] = {
    0,         13113,   22829,   39746,   69197,  120473,  158961,  209744,  276751,  365166,  481826,  635755,  838861,  941217, 1056063, 1184922, 1329505,
    1491729, 1673747, 1877975, 2107123, 2364231, 2652711, 2976390, 3339565, 3747054, 4204263, 4717261, 5292854, 5938680, 6663308,
    7476355, 8388607
};

// notes §2: log2 of the attack rate at each fourth step of the dial
static double attack_log2(int i) {
    if (i <= 0) {
        return 0.0;
    }

    if (i == 1) {
        return log2(0.75);
    }

    if (i <= 6) {
        return -(double)(i - 1);
    }

    if (i <= 30) {
        return -5.0 - ((i - 6) / 2.0);
    }
    return -17.0 - (i - 30);
}

// notes §3: the same for decay and release, as breakpoints in log2; past the last, no movement
static double fall_rate(int i) {
    static const double kAt[]   = {0, 4, 7, 12, 24, 28, 33, 34, 35};
    static const double kLog2[] = {-4.0, -8.0, -9.5, -11.0, -14.0, -15.2, -17.2, -17.7, -19.0};
    const int           n       = (int)(sizeof(kAt) / sizeof(kAt[0]));

    if (i > kAt[n - 1]) {
        return 0.0;
    }

    for (int k = 1; k < n; k++) {
        if (i <= kAt[k]) {
            return exp2(kLog2[k - 1] + ((kLog2[k] - kLog2[k - 1]) * (i - kAt[k - 1]) / (kAt[k] - kAt[k - 1])));
        }
    }

    return exp2(kLog2[0]);
}

static double attack_rate(int i) {
    return exp2(attack_log2(i));
}

// notes §4: the sustain level's curve, at each fourth step of the dial
static double sustain_curve(int i) {
    if (i <= 0) {
        return 0.0;
    }

    if (i < 5) {
        return 0.1 * exp2(-2.8 - (0.8 * (5 - i)));
    }

    if (i < 12) {
        return 0.1 * exp2(-0.4 * (12 - i));
    }
    return pow(10.0, (i - 32) / 20.0);
}

// Between the points, a straight line - as the instrument reads its curves
static double between(double ( *curve )(int), double index) {
    int    i = (int)floor(index);
    double f = index - i;

    return (curve(i) * (1.0 - f)) + (curve(i + 1) * f);
}

double ti_env_sustain_level(double dial) {
    return (between(sustain_curve, dial / 4.0) / 2.0) + (dial / 256.0) + FLOOR;
}

// notes §4a: Sub, Noise and Ring's level - a quarter of the dial and three quarters of the sustain table, the dial over 127
double ti_env_mix_level(double dial) {
    double x = fmin(fmax(dial, 0.0), 127.0) / 127.0;

    return (x / 4.0) + (0.75 * between(sustain_curve, 32.0 * x));
}

void ti_env_shape(double attack, double decay, double sustain, double sustainTime, double release, double sampleRate,
                  tTiEnvShape * shape) {
    double ticks = TICKS_PER_SECOND / sampleRate;   // per sample

    shape->fixed      = false;

    shape->attackStep = between(attack_rate, attack / 4.0) * ticks;
    shape->decayMul   = pow(1.0 - between(fall_rate, decay / 4.0), ticks);
    shape->releaseMul = pow(1.0 - between(fall_rate, release / 4.0), ticks);
    shape->sustain    = ti_env_sustain_level(sustain);
    shape->glideStep  = GLIDE_RATE * ticks;
    shape->slopeStep  = 0.0;
    shape->slopeMul   = 1.0;

    // notes §4: below 64 a fall, above it a rise, both off the decay curve
    if (sustainTime < 64.0) {
        shape->slopeMul = pow(1.0 - (between(fall_rate, (9.0 / 16.0) * sustainTime) / 2.0), ticks);
    } else if (sustainTime > 64.0) {
        shape->slopeStep = (between(fall_rate, (9.0 / 16.0) * (128.0 - sustainTime)) / 4.0) * ticks;
    }
}

// notes §7: a word times a word, as fractions - the product doubled, the bits below the word dropped (towards minus)
static int64_t word_mul(int64_t a, int64_t b, int shift) {
    int64_t p = a * b * 2;

    return (p >= 0) ? (p >> shift) : -((-p + ((INT64_C(1) << shift) - 1)) >> shift);
}

// A curve's points at a position: the whole part picks a point, the rest (as a word) runs a line to the next
static int64_t points_at(const int32_t * points, int count, int64_t position) {
    int64_t i = position >> 18;

    if (i >= count - 1) {
        return points[count - 1];
    }
    return points[i] + word_mul(points[i + 1] - points[i], (position << 5) & (WORD - 1), 24);
}

// tiLfo notes §1: the fall curve read at a position (the index in 1/65536ths of a point, times 4)
int64_t ti_env_fall_curve(int64_t position) {
    return points_at(kFallPoints, 37, position);
}

void ti_env_shape_fixed(double attack, double decay, double sustain, double sustainTime, double release, double sampleRate, bool amplifier,
                        tTiEnvShape * shape) {
    int64_t k = llround(0.5 * 46875.0 / sampleRate * WORD);   // notes §7: the rate's scale for this sample rate

    ti_env_shape(attack, decay, sustain, sustainTime, release, sampleRate, shape);
    shape->fixed       = true;
    shape->startWord   = amplifier ? START_WORD : 0;
    shape->topWord     = amplifier ? ATTACK_TOP_WORD : (WORD - 1);

    if (amplifier) {
        shape->attackRate = 4 * word_mul(points_at(kAttackPoints, 33, llround(attack * 65536.0)), k, 23);
    } else if (llround(attack) == 0) {
        shape->attackRate = 4 * (WORD - 1);
    } else {
        // notes §7: the others' Attack reads the fall curve - half its rate, and a sliver more
        int64_t t = points_at(kFallPoints, 37, llround(attack * 65536.0));
        int64_t p = 2 * t * k;

        shape->attackRate = 4 * ((p + (t * (p >> 24))) >> 24);
    }
    shape->attackRate  = (shape->attackRate > 4 * (WORD - 1)) ? 4 * (WORD - 1) : shape->attackRate;
    shape->decayRate   = 4 * word_mul(-points_at(kFallPoints, 37, llround(decay * 65536.0)), k, 23);
    shape->releaseRate = 4 * word_mul(-points_at(kFallPoints, 37, llround(release * 65536.0)), k, 23);
    shape->slopeRate   = 0;

    if (sustainTime < 63.5) {
        int64_t v = llround(2.0 * sustainTime * 65536.0);

        shape->slopeRate = 4 * word_mul(-points_at(kFallPoints, 37, v + (v >> 3)), k, 24);
    } else if (sustainTime > 64.5) {
        int64_t v = llround(2.0 * (128.0 - sustainTime) * 65536.0);

        shape->slopeRate = 4 * word_mul(points_at(kFallPoints, 37, v + (v >> 3)), k, 25);
    }
    // the sustain level: the dial over 127 as a word; the amplifier's half the curve there and half the word; and a floor
    int64_t x = llround(sustain * 66052.0);
    int64_t t = ((x >> 18) >= 32) ? kSustainPoints[32] : points_at(kSustainPoints, 33, x);
    int64_t s = (amplifier ? ((t >> 1) + (x >> 1)) : x) + SLOPE_FLOOR_WORD;

    shape->sustainWord = (s > WORD - 1) ? WORD - 1 : s;
}

// notes §7: one update of the level - add, or take the level's high word times the rate off the whole level
static void update_fixed(tTiEnv * env, const tTiEnvShape * shape) {
    int64_t hi = env->acc >> 24;
    int64_t b;

    switch (env->stage) {
        case eTiEnvAttack:
            b = env->acc + (shape->attackRate << 24);

            if (b < (shape->topWord << 24)) {
                env->acc = b;
            } else {
                env->acc   = shape->topWord << 24;
                env->stage = eTiEnvDecay;
            }
            break;

        case eTiEnvDecay:
            b = env->acc + (2 * hi * shape->decayRate);

            if (b > (shape->sustainWord << 24)) {
                env->acc = b;
            } else {
                env->acc   = shape->sustainWord << 24;
                env->stage = eTiEnvSustain;
            }
            break;

        case eTiEnvSustain:
        case eTiEnvGlide:

            if (shape->slopeRate < 0) {
                b        = env->acc + (2 * hi * shape->slopeRate);
                env->acc = (b > ((int64_t)SLOPE_FLOOR_WORD << 24)) ? b : ((int64_t)SLOPE_FLOOR_WORD << 24);
            } else if (shape->slopeRate > 0) {
                b        = env->acc + (shape->slopeRate << 24);
                env->acc = (b < ((int64_t)SLOPE_TOP_WORD << 24)) ? b : ((int64_t)SLOPE_TOP_WORD << 24);
            }
            break;

        case eTiEnvRelease:
            b        = env->acc + (2 * hi * shape->releaseRate);
            env->acc = (b > 0) ? b : 0;

            if ((env->acc >> 24) == 0) {
                env->acc   = 0;
                env->stage = eTiEnvIdle;
            }
            break;

        default:
            break;
    }
}

static double next_fixed(tTiEnv * env, const tTiEnvShape * shape) {
    if ((env->stage == eTiEnvAttack) && (env->acc == 0)) {
        env->acc     = shape->startWord << 24;   // a note from silence starts here, with an update at once
        env->count   = UPDATE_SAMPLES - 1u;
        env->started = false;
    }

    if (++env->count >= UPDATE_SAMPLES) {
        env->count   = 0;
        update_fixed(env, shape);
        env->from    = env->started ? env->to : ((double)(env->acc >> 24) / WORD);   // from silence: at once
        env->to      = (double)(env->acc >> 24) / WORD;
        env->started = true;
    }
    // notes §7: between updates the level moves in a straight line, sample by sample, from one update's to the next's
    env->level = env->from + ((env->to - env->from) * (double)(env->count + 1) / (double)UPDATE_SAMPLES);
    return env->level;
}

void ti_env_gate(tTiEnv * env, bool on) {
    if (on) {
        env->stage = eTiEnvAttack;
    } else if (env->stage != eTiEnvIdle) {
        env->stage = eTiEnvRelease;
    }
}

static void hold(tTiEnv * env, double level) {
    env->level = level;
    env->held  = level;
    env->stage = eTiEnvSustain;
}

double ti_env_next(tTiEnv * env, const tTiEnvShape * shape) {
    if (shape->fixed) {
        return next_fixed(env, shape);
    }

    switch (env->stage) {
        case eTiEnvAttack:
            env->level += shape->attackStep;

            if (env->level >= TOP) {
                env->level = TOP;
                env->stage = eTiEnvDecay;
            }
            break;

        case eTiEnvDecay:
            env->level *= shape->decayMul;

            if (env->level <= shape->sustain) {
                hold(env, shape->sustain);
            }
            break;

        case eTiEnvSustain:

            // notes §6
            if (shape->sustain != env->held) {
                env->held  = shape->sustain;
                env->stage = eTiEnvGlide;
            } else if (shape->slopeStep > 0.0) {
                env->level = (env->level < RISE_TOP) ? fmin(RISE_TOP, env->level + shape->slopeStep) : env->level;
            } else {
                env->level = fmax(FLOOR, env->level * shape->slopeMul);
            }
            break;

        case eTiEnvGlide:

            if (env->level < shape->sustain) {
                env->level = fmin(shape->sustain, env->level + shape->glideStep);
            } else {
                env->level = fmax(shape->sustain, env->level - shape->glideStep);
            }

            if (env->level == shape->sustain) {
                hold(env, shape->sustain);
            }
            break;

        case eTiEnvRelease:
            env->level *= shape->releaseMul;

            // notes §5
            if (env->level < SILENT) {
                env->level = 0.0;
                env->stage = eTiEnvIdle;
            }
            break;

        case eTiEnvIdle:
        default:
            break;
    }
    return env->level;
}
