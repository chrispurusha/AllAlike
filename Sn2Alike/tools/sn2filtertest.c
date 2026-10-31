// Prints a filter's magnitude response: sn2filtertest type slope cutoff resonance
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "sn2Filter.h"

#define POINTS    (8192)

int main(int argc, char ** argv) {
    if (argc < 5) {
        return 1;
    }
    tSn2FilterCoeffs c;
    tSn2Filter       f;
    static double    h[POINTS];

    sn2_filter_coeffs((uint32_t)atoi(argv[1]), (uint32_t)atoi(argv[2]), atof(argv[3]), atof(argv[4]), 48000.0, &c);
    sn2_filter_reset(&f);

    for (int i = 0; i < POINTS; i++) {
        h[i] = sn2_filter_process(&f, &c, (i == 0) ? 1.0 : 0.0);
    }

    for (double hz = 40.0; hz < 20000.0; hz *= 1.1) {
        double re = 0.0, im = 0.0, w = 2.0 * M_PI * hz / 48000.0;

        for (int i = 0; i < POINTS; i++) {
            re += h[i] * cos(w * i);
            im -= h[i] * sin(w * i);
        }

        printf("%.2f %.2f\n", hz, 10.0 * log10((re * re) + (im * im) + 1e-14));
    }

    return 0;
}
