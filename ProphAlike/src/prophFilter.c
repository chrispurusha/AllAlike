/*
 * ProphAlike - the filter: one state-variable filter, as the instrument's, in its fixed point and in floating point.
 *
 * Copyright (C) 2026 Chris Turner <chris_purusha@icloud.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
// Notes: Docs/code-notes/prophFilter.md - "// notes §k" refers there.

#include <math.h>
#include <string.h>

#include "prophFilter.h"

// notes §5: the cutoff word's target at each Cutoff step, resonance 0
static const uint16_t kCutoffBase[100] = {
    0x0000, 0x0021, 0x0027, 0x0029, 0x002A, 0x002C, 0x002E, 0x0030, 0x0031, 0x0033, 0x0035, 0x0037,
    0x003A, 0x003D, 0x0041, 0x0045, 0x0049, 0x004D, 0x0052, 0x0057, 0x005C, 0x0061, 0x0067, 0x006D,
    0x0074, 0x007A, 0x0082, 0x008A, 0x0092, 0x009B, 0x00A4, 0x00AE, 0x00B8, 0x00C3, 0x00CF, 0x00DB,
    0x00E8, 0x00F6, 0x0104, 0x0114, 0x0124, 0x0136, 0x0148, 0x015C, 0x0171, 0x0187, 0x019E, 0x01B6,
    0x01D1, 0x01EC, 0x020A, 0x0229, 0x0249, 0x026C, 0x0291, 0x02B8, 0x02E2, 0x030E, 0x033C, 0x036D,
    0x03A2, 0x03D9, 0x0414, 0x0452, 0x0494, 0x04D9, 0x0523, 0x0571, 0x05C4, 0x061C, 0x0679, 0x06DB,
    0x0744, 0x07B3, 0x0828, 0x08A4, 0x0928, 0x09B3, 0x0A47, 0x0AE3, 0x0B89, 0x0C39, 0x0CF3, 0x0DB7,
    0x0E89, 0x0F66, 0x1050, 0x1149, 0x1250, 0x1366, 0x148F, 0x15C7, 0x1712, 0x1873, 0x19E6, 0x1B6F,
    0x1D13, 0x1ECD, 0x20A0, 0x2293,
};

// notes §6: Resonance 0, 10 .. 90, 99 - the cutoff's lift, the input gain's and the damping's targets
static const double   kLift[11]        = {1.0000, 1.1530, 1.3249, 1.5113, 1.7047, 1.8851, 2.0686, 2.2161, 2.3271, 2.3946, 2.4133};
static const uint32_t kGainTarget[11]  = {0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF, 0x7C7EF8FC, 0x39BA7374, 0x07D60FAC};
static const uint32_t kDampTarget[11]  = {0x80010004, 0x988D311C, 0xAE7E5CFE, 0xC1D283A6, 0xD28AA516, 0xE000C002, 0xEC22D846, 0xF504EA0A, 0xFB48F692, 0xFEF1FDE4, 0xFFF0FFE2};

#define NOTCH_LOW       ((int32_t)0x80000001)   // notes §4: the low-pass taken from the notch at -1
#define GAIN_K1         ((int32_t)0x008FFFFF)   // notes §7: the input gain's glide
#define GAIN_K2         ((int32_t)0x7F700000)
#define DAMP_K1         ((int32_t)0x00FFFFFF)   // the damping's
#define DAMP_K2         ((int32_t)0x7F000000)
#define STEP_SAMPLES    (128)                   // notes §2: a cutoff change glides over this many samples
#define Q31             (2147483648.0)

// ---- the instrument's arithmetic - notes §8 ---------------------------------------------------------------------

#define ACC_TOP    (((int64_t)1 << 47) - 1)

static int64_t clip_acc(int64_t m) {   // the accumulator read out saturates past 48 bits
    return (m > ACC_TOP) ? ACC_TOP : ((m < -ACC_TOP - 1) ? (-ACC_TOP - 1) : m);
}

static int32_t word(int64_t m) {        // a store of the accumulator's high part: >> 16, to 32 bits
    return (int32_t)(uint32_t)((uint64_t)(clip_acc(m) >> 16));
}

static int64_t times(int32_t c, int32_t a) {   // a coefficient times a word through the multiplier's A port
    return ((int64_t)c * (int64_t)(int32_t)((uint32_t)a & 0xFFFFFF00u)) >> 15;
}

static int32_t glide_word(int32_t x, int32_t target, int32_t k1, int32_t k2) {
    return word(times(k1, target) + times(k2, x));
}

// ---- control ------------------------------------------------------------------------------------------------------

static double table11(const double * t, double r) {   // the points sit at Resonance 0, 10 .. 90, 99
    static const double at[11] = {0, 10, 20, 30, 40, 50, 60, 70, 80, 90, 99};

    for (int i = 0; i < 10; i++) {
        if (r <= at[i + 1]) {
            return t[i] + ((t[i + 1] - t[i]) * (r - at[i]) / (at[i + 1] - at[i]));
        }
    }

    return t[10];
}

static double word11(const uint32_t * t, double r) {
    double v[11];

    for (int i = 0; i < 11; i++) {
        v[i] = (double)(int32_t)t[i];
    }

    return table11(v, r);
}

void proph_filter_set(tProphFilter * f, tProphFilterSet * s, uint32_t type, double cutoff, double resonance) {
    double  c    = fmin(99.0, fmax(0.0, cutoff));
    int     i    = (int)floor(c);
    double  base = (i >= 99) ? kCutoffBase[99] : (kCutoffBase[i] + ((kCutoffBase[i + 1] - kCutoffBase[i]) * (c - i)));  // notes §5
    double  r    = fmin(99.0, fmax(0.0, resonance));
    int32_t t    = (int32_t)fmin(32767.0, lround(base * table11(kLift, r)));

    s->type       = (type < PROPH_FILTER_TYPES) ? type : 1;
    s->target     = t << 16;
    s->gainTarget = (int32_t)lround(word11(kGainTarget, r));
    s->dampTarget = (int32_t)lround(word11(kDampTarget, r));

    if (t != f->lastTarget) {   // notes §2: the step is set once, from the last target to the new one
        int32_t delta = t - f->lastTarget;
        int32_t step  = (delta >= 0) ? (delta / STEP_SAMPLES) : -((-delta) / STEP_SAMPLES);

        f->step       = (step != 0) ? step : ((delta > 0) ? 1 : -1);
        f->lastTarget = t;
    }

    if (!f->primed) {   // a fresh filter starts where it is asked to be
        f->f      = s->target;
        f->gain   = s->gainTarget;
        f->damp   = s->dampTarget;
        f->ff     = (double)s->target / Q31;
        f->fgain  = (double)s->gainTarget / Q31;
        f->fdamp  = (double)s->dampTarget / Q31;
        f->primed = true;
    }
}

void proph_filter_reset(tProphFilter * f) {
    int32_t      target = f->lastTarget;
    bool         primed = f->primed;
    tProphFilter keep   = *f;

    memset(f, 0, sizeof(*f));
    f->lastTarget = target;
    f->primed     = primed;
    f->f          = keep.f;
    f->gain       = keep.gain;
    f->damp       = keep.damp;
    f->step       = keep.step;
    f->ff         = keep.ff;
    f->fgain      = keep.fgain;
    f->fdamp      = keep.fdamp;
}

// notes §2: the working cutoff moves by the step, and stops at the target rather than pass it
static void slew(tProphFilter * f, const tProphFilterSet * s) {
    int32_t moved = (int32_t)((uint32_t)f->f + ((uint32_t)f->step << 16));
    int32_t ahead = (int32_t)((uint32_t)s->target - (uint32_t)moved) ^ (int32_t)((uint32_t)f->step << 16);

    f->f = (ahead > 0) ? moved : s->target;
}

static double process_fixed(tProphFilter * f, const tProphFilterSet * s, double x) {
    double  scaled = fmin(8388607.0, fmax(-8388608.0, round(x * 8388608.0)));
    int32_t in     = (int32_t)scaled;   // a 24-bit sample
    int64_t m;

    slew(f, s);
    m        = ((int64_t)f->gain * in) >> 7;  // notes §4
    m       += times(f->damp, f->band);
    m       += times(f->damp, f->band);
    f->notch = word(m);
    m       += times(NOTCH_LOW, f->low);
    f->high  = word(m);
    m        = (int64_t)f->band << 16;
    m       += times(f->f, f->high);
    m       += times(f->f, f->high);
    f->band  = word(m);
    f->thru  = (int32_t)((uint32_t)in << 8);
    m        = (int64_t)f->low << 16;
    m       += times(f->f, f->band);
    m       += times(f->f, f->band);
    f->low   = word(m);

    // notes §7: after the filter, the gain and damping glide on toward their targets
    f->gain  = glide_word(f->gain, s->gainTarget, GAIN_K1, GAIN_K2);
    f->damp  = glide_word(f->damp, s->dampTarget, DAMP_K1, DAMP_K2);

    int32_t out;

    switch (s->type) {   // notes §3: THRU, LPF, HPF, BPF, BRF
        case 0:  out = f->thru;
            break;
        case 1:  out = f->low;
            break;
        case 2:  out = f->high;
            break;
        case 3:  out = f->band;
            break;
        default: out = f->notch;
            break;
    }
    return (double)(out >> 8) / 8388608.0;   // the top 24 bits
}

static double process_float(tProphFilter * f, const tProphFilterSet * s, double x) {
    double target = (double)s->target / Q31;
    double step   = (double)f->step * 65536.0 / Q31;

    f->ff    += step;
    f->ff     = ((step >= 0.0) ? (f->ff >= target) : (f->ff <= target)) ? target : f->ff;

    double notch  = (f->fgain * x) + (2.0 * f->fdamp * f->fband);
    double high   = notch - f->flow;

    f->fband += 2.0 * f->ff * high;
    f->flow  += 2.0 * f->ff * f->fband;
    f->fgain += ((double)s->gainTarget / Q31 - f->fgain) * ((double)GAIN_K1 / Q31);
    f->fdamp += ((double)s->dampTarget / Q31 - f->fdamp) * ((double)DAMP_K1 / Q31);

    switch (s->type) {
        case 0:  return x;

        case 1:  return f->flow;

        case 2:  return high;

        case 3:  return f->fband;

        default: return notch;
    }
}

double proph_filter_process(tProphFilter * f, const tProphFilterSet * s, double x, bool fixedPoint) {
    return fixedPoint ? process_fixed(f, s, x) : process_float(f, s, x);
}
