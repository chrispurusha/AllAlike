// Build: clang -O2 -Isrc tools/osAliasTest.c src/osVoice.c src/osEnvelope.c src/osResample.c -o /tmp/osAliasTest
// Run with no arguments; see Docs/code-notes/osResample.md for what the results should be.
// Plays a 44.1 kHz sine through the voice into 48 kHz at a given transposition and reports the level of the
// intended tone and of the strongest other component (a Goertzel scan, 1 Hz to 24 kHz in 10 Hz steps).
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "osVoice.h"

static double tone(const float * x, int n, double f, double rate) {
    double w = 2 * M_PI * f / rate, c = 2 * cos(w), s1 = 0, s2 = 0;

    for (int i = 0; i < n; i++) {
        double win = 0.5 - 0.5 * cos(2 * M_PI * i / (n - 1));
        double s0  = x[i] * win + c * s1 - s2;
        s2 = s1;
        s1 = s0;
    }

    return sqrt(s1 * s1 + s2 * s2 - c * s1 * s2) / (n / 4.0);
}

static void run(double freq, double semis) {
    int       n = 44100;
    float *   d = malloc(n * sizeof(float));

    for (int i = 0; i < n; i++) {
        d[i] = (float)sin(2 * M_PI * freq * i / 44100.0);
    }

    tOsSample s = {.frames = n, .channels = 1, .rate = 44100, .data = {d, d}};
    tOsVoices v;
    tOsPlay   p = {.gain = 1, .end = 1, .tune = semis, .env = {.hold = -1}};
    int       m = 16384;
    float *   l = calloc(m + 4096, sizeof(float)), * r = calloc(m + 4096, sizeof(float));
    os_voices_init(&v, 48000);
    os_voices_note_on(&v, &s, &p, 60, 1);
    os_voices_render(&v, &p, l, r, m + 4096);
    double    want = freq * pow(2, semis / 12.0), wantLevel = (want < 24000) ? tone(l + 4096, m, want, 48000) : 0, worst = 0, worstF = 0;

    for (double f = 20; f < 24000; f += 10) {
        if (fabs(f - want) < 60) {
            continue;
        }
        double a = tone(l + 4096, m, f, 48000);

        if (a > worst) {
            worst  = a;
            worstF = f;
        }
    }

    printf("%6.0f Hz at %+3.0f st -> %6.0f Hz: tone %6.1f dB, worst other %6.1f dB at %5.0f Hz\n", freq, semis, want,
           20 * log10(wantLevel + 1e-12), 20 * log10(worst + 1e-12), worstF);
}

int main(void) {
    run(1000, 0);
    run(10000, 0);
    run(18000, 0);
    run(5000, 12);
    run(10000, 12);
    run(15000, 12);
    run(8000, 24);
    run(1000, -12);
    run(18000, -12);
}
