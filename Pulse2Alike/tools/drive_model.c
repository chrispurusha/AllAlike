// The engine's drive curve (src/p2Drive.c) with every constant passed in, for tools/drive_shape.py to fit - built
// by that script into a shared library. p2Drive notes §6.
#include <math.h>

// shape: positive, negative, bias, couplingHz, shelfDirect, shelfHz, hardPos, hardNeg; loss: knee, slope, trim
// (slope 0 = no quiet-input loss)
static double knee_clip(double u, double ceiling, double hard) {
    double x = u / ceiling;

    return ceiling * x / pow(1.0 + pow(fabs(x), hard), 1.0 / hard);
}

// negScale: the negative ceiling's scale at this Drive (p2Drive notes §9; 1 = as the shape)
void drive_run2(const double * x, int n, double rate, const double * shape, double gain, const double * loss, double negScale,
                double * out) {
    double kc = 1.0 - exp(-2.0 * M_PI * shape[3] / rate), ks = 1.0 - exp(-2.0 * M_PI * shape[5] / rate);
    double kd = 1.0 - exp(-2.0 * M_PI * 5.0 / rate);
    double ka = 1.0 - exp(-1.0 / (0.003 * rate)), kr = 1.0 - exp(-1.0 / (0.060 * rate));
    double coupled = 0.0, low = 0.0, dc = 0.0, power = 0.0;

    for (int i = 0; i < n; i++) {
        double v = x[i], p2 = v * v, cf = 1.0;

        power   += ((p2 > power) ? ka : kr) * (p2 - power);

        if (loss[1] > 0.0) {
            double level = (10.0 * log10(fmax(power, 1e-20))) - 3.15;

            cf = pow(10.0, (loss[2] - (loss[1] * fmax(0.0, loss[0] - level))) / 20.0);
        }
        double u = (v * gain) - coupled + (shape[2] * cf);
        double c = (u >= 0.0) ? knee_clip(u, shape[0] * cf, shape[6]) : knee_clip(u, shape[1] * cf * negScale, shape[7]);

        coupled += kc * c;
        low     += ks * (c - low);
        double o = (shape[4] * c) + ((1.0 - shape[4]) * low) - dc;

        dc      += kd * o;
        out[i]   = o;
    }
}

void drive_run(const double * x, int n, double rate, const double * shape, double gain, const double * loss, double * out) {
    drive_run2(x, n, rate, shape, gain, loss, 1.0, out);
}

// p2Drive notes §9's model: an OTA-style gain stage, then a fixed clipper inside the coupling loop, then the shelf.
// p: I0, a, b (the stage's current, I0 + exp(a + b * drive / 127): its gain and its ceiling together), s (input
// scale into the stage's tanh), e (the stage's offset: lopsided only as it saturates), pos, neg, hardPos, hardNeg,
// couplingHz, shelfDirect, shelfHz
void drive_run3(const double * x, int n, double rate, const double * p, double drive, double * out) {
    double current = p[0] + exp(p[1] + (p[2] * drive / 127.0));
    double kc      = 1.0 - exp(-2.0 * M_PI * p[9] / rate), ks = 1.0 - exp(-2.0 * M_PI * p[11] / rate);
    double kd      = 1.0 - exp(-2.0 * M_PI * 5.0 / rate);
    double rest    = tanh(p[4]);
    double coupled = 0.0, low = 0.0, dc = 0.0;
    int    i       = 0;

    for (i = 0; i < n; i++) {
        double y = current * (tanh((x[i] * p[3]) + p[4]) - rest);
        double u = y - coupled;
        double c = (u >= 0.0) ? knee_clip(u, p[5], p[7]) : knee_clip(u, p[6], p[8]);
        double o = 0.0;

        coupled += kc * c;
        low     += ks * (c - low);
        o        = (p[10] * c) + ((1.0 - p[10]) * low) - dc;
        dc      += kd * o;
        out[i]   = o;
    }
}

// p2Drive notes §10: as drive_run2, but the coupling loop holds the clipper's output at its resting value - what it
// gives the bias alone - instead of at zero, so a fixed offset ahead of the clipper is not cancelled: small signals
// sit on the lopsided part of the knee, large ones swamp it
void drive_run4(const double * x, int n, double rate, const double * shape, double gain, double trimDb, double * out) {
    double cf      = pow(10.0, trimDb / 20.0);
    double pos     = shape[0] * cf, neg = shape[1] * cf, bias = shape[2] * cf;
    double kc      = 1.0 - exp(-2.0 * M_PI * shape[3] / rate), ks = 1.0 - exp(-2.0 * M_PI * shape[5] / rate);
    double kd      = 1.0 - exp(-2.0 * M_PI * 5.0 / rate);
    double rest    = (bias >= 0.0) ? knee_clip(bias, pos, shape[6]) : knee_clip(bias, neg, shape[7]);
    double coupled = 0.0, low = rest, dc = rest;
    int    i       = 0;

    for (i = 0; i < n; i++) {
        double u = (x[i] * gain) - coupled + bias;
        double c = (u >= 0.0) ? knee_clip(u, pos, shape[6]) : knee_clip(u, neg, shape[7]);
        double o = 0.0;

        coupled += kc * (c - rest);
        low     += ks * (c - low);
        o        = (shape[4] * c) + ((1.0 - shape[4]) * low) - dc;
        dc      += kd * o;
        out[i]   = o;
    }
}

// p2Drive notes §10: drive_run4 with a cubic ahead of the clipper, u + e u^3 / pos^2 (shape[8] = e): e > 0 steepens the
// curve before it saturates - an inflection, for TUBE's odd products that dip and rise with level
void drive_run5(const double * x, int n, double rate, const double * shape, double gain, double trimDb, double * out) {
    double cf      = pow(10.0, trimDb / 20.0);
    double pos     = shape[0] * cf, neg = shape[1] * cf, bias = shape[2] * cf, e = shape[8] / (pos * pos);
    double kc      = 1.0 - exp(-2.0 * M_PI * shape[3] / rate), ks = 1.0 - exp(-2.0 * M_PI * shape[5] / rate);
    double kd      = 1.0 - exp(-2.0 * M_PI * 5.0 / rate);
    double ub      = bias + (e * bias * bias * bias);
    double rest    = (ub >= 0.0) ? knee_clip(ub, pos, shape[6]) : knee_clip(ub, neg, shape[7]);
    double coupled = 0.0, low = rest, dc = rest;
    int    i       = 0;

    for (i = 0; i < n; i++) {
        double u = (x[i] * gain) - coupled + bias;
        double c = 0.0;
        double o = 0.0;

        u        = u + (e * u * u * u);
        c        = (u >= 0.0) ? knee_clip(u, pos, shape[6]) : knee_clip(u, neg, shape[7]);
        coupled += kc * (c - rest);
        low     += ks * (c - low);
        o        = (shape[4] * c) + ((1.0 - shape[4]) * low) - dc;
        dc      += kd * o;
        out[i]   = o;
    }
}

// p2Drive notes §11: drive_run5 with a one-sided dead zone ahead of the clipper (shape[9] = w, in the knee's units):
// on the negative side (w > 0) or the positive one (w < 0) the curve starts flat and reaches its full slope beyond
// about |w| - a diode's threshold: quiet signals lose most of one half, loud ones hardly notice
static double dead_zone(double u, double w) {
    if ((w > 0.0) && (u < 0.0)) {
        return u * (1.0 - exp(u / w));
    }

    if ((w < 0.0) && (u > 0.0)) {
        return u * (1.0 - exp(u / w));
    }
    return u;
}

void drive_run6(const double * x, int n, double rate, const double * shape, double gain, double trimDb, double * out) {
    double cf      = pow(10.0, trimDb / 20.0);
    double pos     = shape[0] * cf, neg = shape[1] * cf, bias = shape[2] * cf, e = shape[8] / (pos * pos), w = shape[9] * cf;
    double kc      = 1.0 - exp(-2.0 * M_PI * shape[3] / rate), ks = 1.0 - exp(-2.0 * M_PI * shape[5] / rate);
    double kd      = 1.0 - exp(-2.0 * M_PI * 5.0 / rate);
    double ub      = dead_zone(bias + (e * bias * bias * bias), w);
    double rest    = (ub >= 0.0) ? knee_clip(ub, pos, shape[6]) : knee_clip(ub, neg, shape[7]);
    double coupled = 0.0, low = rest, dc = rest;
    int    i       = 0;

    for (i = 0; i < n; i++) {
        double u = (x[i] * gain) - coupled + bias;
        double c = 0.0;
        double o = 0.0;

        u        = dead_zone(u + (e * u * u * u), w);
        c        = (u >= 0.0) ? knee_clip(u, pos, shape[6]) : knee_clip(u, neg, shape[7]);
        coupled += kc * (c - rest);
        low     += ks * (c - low);
        o        = (shape[4] * c) + ((1.0 - shape[4]) * low) - dc;
        dc      += kd * o;
        out[i]   = o;
    }
}

// p2Drive notes §12: drive_run6 with positive feedback around the clipper - its output, back to its input, times h
// (shape[10]): with enough of it the stage snaps between its sides once its input crosses a threshold (hysteresis),
// as TUBE does at high Drive. The fed-back output is taken from the last sample, against its rest
void drive_run7(const double * x, int n, double rate, const double * shape, double gain, double trimDb, double * out) {
    double cf      = pow(10.0, trimDb / 20.0);
    double pos     = shape[0] * cf, neg = shape[1] * cf, bias = shape[2] * cf, e = shape[8] / (pos * pos), w = shape[9] * cf;
    double h       = shape[10];
    double kc      = 1.0 - exp(-2.0 * M_PI * shape[3] / rate), ks = 1.0 - exp(-2.0 * M_PI * shape[5] / rate);
    double kd      = 1.0 - exp(-2.0 * M_PI * 5.0 / rate);
    double ub      = dead_zone(bias + (e * bias * bias * bias), w);
    double rest    = (ub >= 0.0) ? knee_clip(ub, pos, shape[6]) : knee_clip(ub, neg, shape[7]);
    double coupled = 0.0, low = rest, dc = rest, last = rest;
    int    i       = 0;

    for (i = 0; i < n; i++) {
        double u = (x[i] * gain) - coupled + bias + (h * (last - rest));
        double c = 0.0;
        double o = 0.0;

        u        = dead_zone(u + (e * u * u * u), w);
        c        = (u >= 0.0) ? knee_clip(u, pos, shape[6]) : knee_clip(u, neg, shape[7]);
        last     = c;
        coupled += kc * (c - rest);
        low     += ks * (c - low);
        o        = (shape[4] * c) + ((1.0 - shape[4]) * low) - dc;
        dc      += kd * o;
        out[i]   = o;
    }
}

/* drive_run7 with the feedback ahead of the gain: the loop gain h x gain, so it latches only where the gain is high (p2Drive notes §12). */
void drive_run8(const double * x, int n, double rate, const double * shape, double gain, double trimDb, double * out) {
    double cf      = pow(10.0, trimDb / 20.0);
    double pos     = shape[0] * cf, neg = shape[1] * cf, bias = shape[2] * cf, e = shape[8] / (pos * pos), w = shape[9] * cf;
    double h       = shape[10];
    double kc      = 1.0 - exp(-2.0 * M_PI * shape[3] / rate), ks = 1.0 - exp(-2.0 * M_PI * shape[5] / rate);
    double kd      = 1.0 - exp(-2.0 * M_PI * 5.0 / rate);
    double ub      = dead_zone(bias + (e * bias * bias * bias), w);
    double rest    = (ub >= 0.0) ? knee_clip(ub, pos, shape[6]) : knee_clip(ub, neg, shape[7]);
    double coupled = 0.0, low = rest, dc = rest, last = rest;
    int    i       = 0;

    for (i = 0; i < n; i++) {
        double u = (x[i] * gain) - coupled + bias + (h * gain * (last - rest));
        double c = 0.0;
        double o = 0.0;

        u        = dead_zone(u + (e * u * u * u), w);
        c        = (u >= 0.0) ? knee_clip(u, pos, shape[6]) : knee_clip(u, neg, shape[7]);
        last     = c;
        coupled += kc * (c - rest);
        low     += ks * (c - low);
        o        = (shape[4] * c) + ((1.0 - shape[4]) * low) - dc;
        dc      += kd * o;
        out[i]   = o;
    }
}
