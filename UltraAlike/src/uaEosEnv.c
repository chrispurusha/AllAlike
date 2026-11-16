/*
 * UltraAlike - a sampler (a placeholder on a sample player's engine), as a VST3 and Audio Unit instrument.
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
// Notes: Docs/code-notes/uaEosEnv.md - "// notes §k" refers there.

#include <math.h>
#include <stddef.h>

#include "uaEosEnv.h"

#define SPAN_DB(depth)    (96.0 - (3.0 * (double)(depth)))   // notes §2
#define LEVEL_FLOOR_G     (0.958)
#define LEVEL_SLOPE_G     (0.00984)
#define LEVEL_TOP_G       (0.0072)
#define ATTACK_DB         (34.0)                             // notes §4
#define ATTACK_DEPTH_K    (0.2)
#define EXB_RATE_SCALE    (254.0)                            // notes §5

const tUaEosEnvelope gUaEosDefaultEnvelope = {
    .rate  = {  0,   0,   0,   0, 30, 0},
    .level = {100, 100, 100, 100,  0, 0},
};

// notes §3: dB a second at depth 16 at these rates, straight between them in log
static const double  kKnotRate[]           = {0, 10, 30, 60, 90, 127};
static const double  kKnotDbS[]            = {3670.0, 680.0, 135.0, 23.5, 4.37, 0.39};

double ua_eos_level_gain(double percent, int depth) {
    if (percent <= 0.0) {
        return 0.0;
    }
    double g = fmax(LEVEL_FLOOR_G - (LEVEL_SLOPE_G * percent), LEVEL_TOP_G * (100.0 - percent));   // notes §2: the line below 90%, the gentler top above

    return pow(10.0, -SPAN_DB(depth) * fmax(0.0, g) / 20.0);
}

static double speed16(double rate) {
    int    k = 0;

    rate = fmin(127.0, fmax(0.0, rate));

    while ((k < 4) && (rate > kKnotRate[k + 1])) {
        k++;
    }
    double f = (rate - kKnotRate[k]) / (kKnotRate[k + 1] - kKnotRate[k]);

    return kKnotDbS[k] * pow(kKnotDbS[k + 1] / kKnotDbS[k], f);
}

double ua_eos_speed_db(int rate, int depth) {
    return speed16(rate) * SPAN_DB(depth) / SPAN_DB(16);
}

double ua_eos_attack_time(int rate, int depth) {
    return (ATTACK_DB / speed16(rate)) * (1.0 + (ATTACK_DEPTH_K * (16.0 - (double)depth) / 16.0));
}

// notes §5: an Emulator X bank stores each stage's rate as a fraction, 0.5 for rate 127
void ua_eos_from_bank(const tEmuEnvelope * stored, tUaEosEnvelope * out) {
    if ((stored == NULL) || !stored->present) {
        *out = gUaEosDefaultEnvelope;
        return;
    }

    for (int s = 0; s < UA_EOS_STAGES; s++) {
        out->rate[s]  = (uint8_t)lround(fmin(127.0, fmax(0.0, stored->time[s] * EXB_RATE_SCALE)));
        out->level[s] = (uint8_t)lround(fmin(100.0, fmax(0.0, stored->level[s])));
    }
}

void ua_eos_env_start(tUaEosEnvState * s, double fromGain) {
    s->stage = 0;
    s->gain  = fromGain;
}

void ua_eos_env_release(tUaEosEnvState * s) {
    if (s->stage < 5) {
        s->stage = 5;
    }
}

bool ua_eos_env_done(const tUaEosEnvState * s) {
    return s->stage >= 7;
}

static double to_db(double gain, int depth) {
    return (gain > 0.0) ? 20.0 * log10(gain) : -SPAN_DB(depth);
}

// notes §6: each stage moves toward its level at its rate - attacks straight in gain, the rest straight in dB - and
// the next starts when it arrives; Decay 2's level is held until the note off; done after Release 2
double ua_eos_env_step(tUaEosEnvState * s, const tUaEosEnvelope * e, int depth, double dt) {
    while ((s->stage < 7) && (s->stage != 4)) {
        int    k      = (s->stage < 4) ? s->stage : (s->stage - 1);
        double target = ua_eos_level_gain(e->level[k], depth);
        int    rate   = e->rate[k];

        if (rate > 0) {
            if (k < 2) {
                double step = dt / ua_eos_attack_time(rate, depth);

                s->gain = (s->gain < target) ? fmin(target, s->gain + step) : fmax(target, s->gain - step);
            } else {
                double now = to_db(s->gain, depth), want = to_db(target, depth), step = ua_eos_speed_db(rate, depth) * dt;
                double next = (now < want) ? fmin(want, now + step) : fmax(want, now - step);

                s->gain = ((target <= 0.0) && (next <= -SPAN_DB(depth))) ? 0.0 : pow(10.0, next / 20.0);
            }

            if (s->gain != target) {
                break;
            }
        }
        s->gain = target;
        s->stage++;
    }
    return s->gain;
}
