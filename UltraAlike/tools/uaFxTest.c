// Build: clang -O2 -Isrc tools/uaFxTest.c src/uaFx.c -o /tmp/uaFxTest
// Checks the per-layer effects (uaFx notes): Low Cut and High Cut are -3 dB at their cutoff and fall 12 or 24 dB an
// octave beyond it; each saturation mode bends a sine into harmonics, and the antiderivative keeps what folds back
// down - a 5 kHz tone driven hard at 48 kHz - at least 4 dB under the same curve applied sample by sample (uaFx notes §4 has the figures).
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "uaFx.h"

#define RATE    (48000.0)

// The level of a sine at f after the effects, in dB against its input
static double through(const tUaFx * fx, double f) {
    tUaFxState s;
    uint32_t   n = 48000;
    float *    x = malloc(n * sizeof(float));
    double     in = 0, out = 0;

    ua_fx_reset(&s);

    for (uint32_t i = 0; i < n; i++) {
        x[i] = (float)(0.25 * sin(2.0 * M_PI * f * i / RATE));
    }

    ua_fx_process(&s, fx, RATE, x, NULL, n);

    for (uint32_t i = n / 2; i < n; i++) {
        double ref = 0.25 * sin(2.0 * M_PI * f * i / RATE);

        in  += ref * ref;
        out += (double)x[i] * x[i];
    }

    free(x);
    return 10.0 * log10(out / in);
}

// The amplitude at f of x, by correlation (a Hann window over a whole number of its cycles is not needed: long enough)
static double level(const float * x, uint32_t n, double f) {
    double re = 0, im = 0;

    for (uint32_t i = 0; i < n; i++) {
        double w = 0.5 - (0.5 * cos(2.0 * M_PI * i / (n - 1)));

        re += x[i] * w * cos(2.0 * M_PI * f * i / RATE);
        im += x[i] * w * sin(2.0 * M_PI * f * i / RATE);
    }

    return 20.0 * log10((2.0 * sqrt((re * re) + (im * im)) / (n * 0.5)) + 1e-12);
}

// The worst alias of the curve applied sample by sample, with no antiderivative: what it is measured against
static double naive_alias(int mode, double drive) {
    uint32_t n     = 48000;
    float *  x     = malloc(n * sizeof(float));
    double   worst = -300;

    for (uint32_t i = 0; i < n; i++) {
        double v = drive * 0.5 * sin(2.0 * M_PI * 5000.0 * i / RATE);

        x[i] = (float)((mode == 1) ? fmax(-1.0, fmin(1.0, v)) : ((mode == 2) ? tanh(v + 0.15) - tanh(0.15) : tanh(v)));
    }

    for (int a = 3000; a < 24000; a += 5000) {
        worst = fmax(worst, level(x + 8000, 32000, a));
    }

    free(x);
    return worst;
}

int main(void) {
    int failures = 0;

    // 1. the filters: at the cutoff, an octave and two beyond
    for (int steep = 0; steep < 2; steep++) {
        tUaFx  hi  = {.filter = true, .lowHz = 10.0, .highHz = 1000.0, .slope = steep ? eUaSlope24 : eUaSlope12, .gainDb = 0, .hfHz = 20000};
        tUaFx  lo  = {.filter = true, .lowHz = 1000.0, .highHz = 23000.0, .slope = steep ? eUaSlope24 : eUaSlope12, .gainDb = 0, .hfHz = 20000};
        double h0 = through(&hi, 1000.0), h1 = through(&hi, 2000.0), h2 = through(&hi, 4000.0);
        double l0 = through(&lo, 1000.0), l1 = through(&lo, 500.0), l2 = through(&lo, 250.0);
        double per = steep ? 24.0 : 12.0;
        bool   ok  = (fabs(h0 + 3.0) < 0.3) && (fabs(l0 + 3.0) < 0.3) && (fabs((h1 - h2) - per) < 1.0) && (fabs((l1 - l2) - per) < 1.0);

        printf("%s  %d dB/oct  High Cut 1 kHz: %.1f / %.1f / %.1f dB at 1, 2, 4 kHz   Low Cut 1 kHz: %.1f / %.1f / %.1f dB at 1k, 500, 250 Hz\n",
               ok ? "ok  " : "FAIL", (int)per, h0, h1, h2, l0, l1, l2);
        failures += ok ? 0 : 1;
    }

    // 1b. Soft: Low Cut two one-poles (-6 dB at the cutoff, 12 dB/octave), High Cut one (-3 dB, 6 dB/octave)
    {
        tUaFx  hi = {.filter = true, .lowHz = 10.0, .highHz = 1000.0, .slope = eUaSlopeSoft, .hfHz = 20000};
        tUaFx  lo = {.filter = true, .lowHz = 1000.0, .highHz = 23000.0, .slope = eUaSlopeSoft, .hfHz = 20000};
        double h0 = through(&hi, 1000.0), h1 = through(&hi, 2000.0), h2 = through(&hi, 4000.0);
        double l0 = through(&lo, 1000.0), l1 = through(&lo, 250.0), l2 = through(&lo, 125.0);
        bool   ok = (fabs(h0 + 3.0) < 0.3) && (fabs((h1 - h2) - 6.0) < 1.0) && (fabs(l0 + 6.0) < 0.4) && (fabs((l1 - l2) - 12.0) < 1.0);

        printf("%s  Soft  High Cut 1 kHz: %.1f / %.1f / %.1f dB at 1, 2, 4 kHz   Low Cut 1 kHz: %.1f / %.1f / %.1f dB at 1k, 250, 125 Hz\n",
               ok ? "ok  " : "FAIL", h0, h1, h2, l0, l1, l2);
        failures += ok ? 0 : 1;
    }

    // 2. the saturators: a 5 kHz sine at +18 dB of drive - its harmonics, and the worst tone that is not one of them
    const char * names[] = {"Classic", "Drums", "Tape"};

    for (int mode = 0; mode < 3; mode++) {
        tUaFx      fx = {.saturate = true, .mode = (tUaSatMode)mode, .gainDb = 18.0, .outputDb = 0, .warmthDb = 0, .hfHz = 22000};
        tUaFxState s;
        uint32_t   n  = 48000;
        float *    x  = malloc(n * sizeof(float));
        double     f = 5000.0, h2, h3, worst = -300, at = 0;

        ua_fx_reset(&s);

        for (uint32_t i = 0; i < n; i++) {
            x[i] = (float)(0.5 * sin(2.0 * M_PI * f * i / RATE));
        }

        ua_fx_process(&s, &fx, RATE, x, NULL, n);
        h2        = level(x + 8000, 32000, 2 * f);
        h3        = level(x + 8000, 32000, 3 * f);

        // aliases of the harmonics above 24 kHz land on 48k - k*5k: 3k, 8k, 13k, 18k, 23k, ...
        for (int a = 3000; a < 24000; a += 5000) {
            double l = level(x + 8000, 32000, a);

            if (l > worst) {
                worst = l;
                at    = a;
            }
        }

        double plain = naive_alias(mode, pow(10.0, 18.0 / 20.0));
        bool   ok    = (fmax(h2, h3) > -40.0) && (worst < plain - 4.0);

        printf("%s  %-7s +18 dB on 5 kHz: 2nd %.1f dB, 3rd %.1f dB; worst alias %.1f dB at %.0f Hz (%.1f dB without the antiderivative)\n",
               ok ? "ok  " : "FAIL", names[mode], h2, h3, worst, at, plain);
        failures += ok ? 0 : 1;
        free(x);
    }

    // 3. 2x oversampling (notes §6): below the clip the hard clip is the input, so the output is the input exactly
    //    UA_FX_LATENCY late (through the two half-band filters); the plain delay, the same; and the aliases fall
    for (int sat = 0; sat < 2; sat++) {
        tUaFx      fx    = {.saturate = sat, .mode = eUaSatDrums, .gainDb = 0, .outputDb = 0, .hfHz = 22000, .oversample = true};
        tUaFxState s;
        uint32_t   n     = 4800;
        float *    x = malloc(n * sizeof(float)), * ref = malloc(n * sizeof(float));
        double     worst = 0;

        ua_fx_reset(&s);

        for (uint32_t i = 0; i < n; i++) {
            ref[i] = x[i] = (float)(0.3 * sin(2.0 * M_PI * 1000.0 * i / RATE) + 0.2 * sin(2.0 * M_PI * 7300.0 * i / RATE));
        }

        ua_fx_process(&s, &fx, RATE, x, NULL, n);

        // through the curve, its antiderivative adds half a sample at twice the rate: a quarter of one here
        for (uint32_t i = 200; i < n; i++) {
            // and averaging neighbours at twice the rate shades each tone by cos(pi f / 2 x the rate)
            double d    = (double)i - 23.25;
            double want = sat ? (0.3 * cos(M_PI * 1000.0 / (2.0 * RATE)) * sin(2.0 * M_PI * 1000.0 * d / RATE)) +
                          (0.2 * cos(M_PI * 7300.0 / (2.0 * RATE)) * sin(2.0 * M_PI * 7300.0 * d / RATE)) : ref[i - UA_FX_LATENCY];

            worst = fmax(worst, fabs(x[i] - want));
        }

        bool       ok    = worst < 0.003;

        printf("%s  oversampling, %s: output is the input %s frames late, largest difference %.1e\n", ok ? "ok  " : "FAIL",
               sat ? "through the curve" : "plain delay", sat ? "23.25" : "23", worst);
        failures += ok ? 0 : 1;
        free(x);
        free(ref);
    }

    for (int mode = 0; mode < 3; mode++) {
        tUaFx      fx = {.saturate = true, .mode = (tUaSatMode)mode, .gainDb = 18.0, .outputDb = 0, .warmthDb = 0, .hfHz = 22000, .oversample = true};
        tUaFxState s;
        uint32_t   n  = 48000;
        float *    x  = malloc(n * sizeof(float));
        double     worst = -300, plain = naive_alias(mode, pow(10.0, 18.0 / 20.0));

        ua_fx_reset(&s);

        for (uint32_t i = 0; i < n; i++) {
            x[i] = (float)(0.5 * sin(2.0 * M_PI * 5000.0 * i / RATE));
        }

        ua_fx_process(&s, &fx, RATE, x, NULL, n);

        // below 20 kHz: what the half-band leaves near its corner (23 kHz, from the 25 kHz harmonic) is above hearing
        for (int a = 3000; a < 20000; a += 5000) {
            worst = fmax(worst, level(x + 8000, 32000, a));
        }

        bool       ok = worst < -70.0;

        printf("%s  %-7s +18 dB on 5 kHz, oversampled: worst alias below 20 kHz %.1f dB (all bands %.1f plain); at 23 kHz %.1f\n",
               ok ? "ok  " : "FAIL", names[mode], worst, plain, level(x + 8000, 32000, 23000));
        failures += ok ? 0 : 1;
        free(x);
    }

    printf(failures ? "\n%d FAILED\n" : "\nall passed\n", failures);
    return failures ? 1 : 0;
}
