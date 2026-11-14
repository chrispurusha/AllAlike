// Offline check of the loader and the voice: loads each file given, plays it once at its root key at 48 kHz,
// and compares the output with the file (frames, peak, and the difference when no resampling is needed).
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "uaSample.h"
#include "uaVoice.h"

int main(int argc, char ** argv) {
    int failures = 0;

    for (int a = 1; a < argc; a++) {
        char        error[512];
        tUaSample * s        = ua_sample_load(argv[a], error, sizeof(error));

        if (s == NULL) {
            printf("FAIL  %s: %s\n", argv[a], error);
            failures++;
            continue;
        }
        tUaVoices   v;
        tUaPlay     p        = {.rootKey = 60, .gain = 1, .end = 1, .env = {.hold = -1.0, .sustain = 1.0}};
        double      hostRate = s->rate; // same rate: the output should equal the file
        uint32_t    n        = s->frames + 64;
        float *     l = calloc(n, sizeof(float)), * r = calloc(n, sizeof(float));
        double      peakIn = 0, peakOut = 0, diff = 0;
        uint32_t    last     = 0;

        ua_voices_init(&v, hostRate);
        ua_voices_note_on(&v, s, &p, 60, 1.0f);

        for (uint32_t at = 0; at < n; at += 256) {
            ua_voices_render(&v, &p, l + at, r + at, (n - at < 256) ? (n - at) : 256);
        }

        for (uint32_t i = 0; i < n; i++) {
            float in = (i < s->frames) ? s->data[0][i] : 0.0f;

            peakIn  = fmax(peakIn, fabs(in));
            peakOut = fmax(peakOut, fabs(l[i]));
            diff    = fmax(diff, fabs(l[i] - in));
            last    = (l[i] != 0.0f) ? i : last;
        }

        bool        ok       = (diff < 1e-5) && (last < s->frames);
        printf("%s  %u frames %u ch %.0f Hz  peak in %.4f out %.4f  max diff %.2e  %s\n", ok ? "OK  " : "FAIL",
               s->frames, s->channels, s->rate, peakIn, peakOut, diff, argv[a]);
        failures += ok ? 0 : 1;
        free(l);
        free(r);
        ua_sample_free(s);
    }

    return failures;
}
