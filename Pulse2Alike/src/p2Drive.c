/*
 * Pulse2Alike - the drive: VCA saturation, after the filter.
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
// Notes: Docs/code-notes/p2Drive.md - "// notes §k" refers there.

#include <math.h>

#include "p2Drive.h"

// notes §2: the engine's level before the VCA, in the units the curves were measured in (the instrument's output
// at Volume 40, in full scale)
#define HARDWARE_UNITS_PER_ENGINE    (0.888)
#define DC_BLOCK_HZ                  (5.0)
#define DRIVE_POINTS                 (6)

typedef struct {
    double positive, negative;   // the clipper's ceilings
    double bias;                 // notes §10: a fixed offset ahead of the clipper, which the coupling no longer cancels
    double couplingHz;
    double shelfDirect, shelfHz; // how much passes straight, and the low-pass the rest goes through
    double hardPos, hardNeg;     // notes §6: each side's knee, 0 for tanh
    double expand;               // notes §10: a cubic ahead of the clipper, x (pos^2): an inflection (TUBE)
    double deadZone;             // notes §11: a one-sided dead zone's width (> 0 the negative side, < 0 the positive)
    double gainDb[DRIVE_POINTS]; // at kDriveAt
    double trimDb[DRIVE_POINTS]; // notes §5, §7: the ceilings' trim
    double topDb[DRIVE_POINTS];  // notes §13: the positive ceiling against the negative
} tCurveShape;

static const double      kDriveAt[DRIVE_POINTS] = {0.0, 16.0, 32.0, 64.0, 96.0, 127.0};

// notes §14: TUBE refitted 2026-11-07 for noise as well as tones (drive_fit_ratio.py with the noise bands, the shelf
// held open); §13's tone-first version is logged there. 96 between
static const tCurveShape kTube                  = {
    0.03157, 0.02130, -0.00270, 71.05, 0.9952, 228.7, 2.672, 4.274, 0.03116, 0.0,
    {-6.99, -0.18, 3.47, 7.38, 9.64, 11.83},
    {9.53,  7.71,  6.74, 6.34, 6.20, 6.07},
    {-4.71, 1.55,  2.65, 2.06, 1.92, 1.79}
};
// notes §11: FUZZ refitted 2026-11-06 with the dead zone (drive_fit2.py deadzone: the cycles, harmonics, two tones)
static const tCurveShape kFuzz                  = {
    0.05421, 0.07687, -0.02002, 894.9, 0.7199, 178.2, 0.5656, 0.7650, 0.0, -0.06256,
    {18.59, 30.08, 30.69, 32.98, 37.74, 42.35},
    {0.58, -0.31, 0.22, 0.55, 0.36, 0.18},
    {0.0,  0.0,   0.0,  0.0,  0.0,  0.0}
};

void p2_drive_init(tP2Drive * d, double rate) {
    d->rate    = rate;
    d->coupled = 0.0;
    d->lowPass = 0.0;
    d->dcBlock = 0.0;
    d->dcIn    = 0.0;
    d->rest      = 0.0;
    d->restDrive = -1.0;
    d->restCurve = -1;
}

// a per-Drive value, between the measured settings
static double at_drive(const double v[DRIVE_POINTS], double drive) {
    double d = fmin(127.0, fmax(0.0, drive));
    int    i = 0;

    while ((i < (DRIVE_POINTS - 2)) && (d > kDriveAt[i + 1])) {
        i++;
    }
    return v[i] + (((d - kDriveAt[i]) / (kDriveAt[i + 1] - kDriveAt[i])) * (v[i + 1] - v[i]));
}

// notes §6: a knee from round (hardness 1) to square (large); tanh where no hardness was fitted
static double clip(double u, double ceiling, double hardness) {
    double x = u / ceiling;

    if (hardness <= 0.0) {
        return ceiling * tanh(x);
    }
    return ceiling * x / pow(1.0 + pow(fabs(x), hardness), 1.0 / hardness);
}

static double one_pole(double hz, double rate) {
    return 1.0 - exp(-2.0 * M_PI * hz / rate);
}

// notes §10: the cubic ahead of the clipper; e is shape->expand / pos^2. notes §11: then the one-sided dead zone, w wide
static double expanded(double u, double e, double w) {
    u = u + (e * u * u * u);

    if (((w > 0.0) && (u < 0.0)) || ((w < 0.0) && (u > 0.0))) {
        u = u * (1.0 - exp(u / w));
    }
    return u;
}

double p2_drive_process(tP2Drive * d, tP2DriveCurve curve, double drive, double x) {
    const tCurveShape * c;
    double              u, clipped, shaped, out, hw, ceiling, pos, neg, bias, e, w;

    if (curve == eP2DriveNone) {
        return x;
    }

    if (curve == eP2DriveFx) {
        return 0.0;
    }
    c       = (curve == eP2DriveTube) ? &kTube : &kFuzz;
    hw      = x * HARDWARE_UNITS_PER_ENGINE;
    ceiling = pow(10.0, at_drive(c->trimDb, drive) / 20.0);
    pos     = c->positive * ceiling * pow(10.0, at_drive(c->topDb, drive) / 20.0);
    neg     = c->negative * ceiling;
    bias    = c->bias * ceiling;
    e       = c->expand / (pos * pos);
    w       = c->deadZone * ceiling;

    // notes §10: the coupling holds the clipper at what it gives the bias alone - worked out when Drive or curve change
    if ((drive != d->restDrive) || ((int)curve != d->restCurve)) {
        double ub = expanded(bias, e, w);

        d->rest      = (ub >= 0.0) ? clip(ub, pos, c->hardPos) : clip(ub, neg, c->hardNeg);
        d->restDrive = drive;
        d->restCurve = (int)curve;
    }
    u           = expanded((hw * pow(10.0, at_drive(c->gainDb, drive) / 20.0)) - d->coupled + bias, e, w);
    clipped     = (u >= 0.0) ? clip(u, pos, c->hardPos) : clip(u, neg, c->hardNeg);
    d->coupled += one_pole(c->couplingHz, d->rate) * (clipped - d->rest);
    d->lowPass += one_pole(c->shelfHz, d->rate) * (clipped - d->lowPass);
    shaped      = (c->shelfDirect * clipped) + ((1.0 - c->shelfDirect) * d->lowPass);
    // the rest value leaves a DC offset; the output stage is AC coupled
    out         = shaped - d->dcBlock;
    d->dcBlock += one_pole(DC_BLOCK_HZ, d->rate) * out;
    return out / HARDWARE_UNITS_PER_ENGINE;
}
