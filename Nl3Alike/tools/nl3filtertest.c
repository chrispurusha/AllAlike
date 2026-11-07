// Prints a filter's magnitude response: nl3filtertest type slope cutoff resonance [fixed]
//   type 0 LP, 1 HP; slope 0 12 dB, 1 24 dB; fixed 1 runs the instrument's arithmetic
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "nl3Filter.h"

#define POINTS     (16384)

#define IMPULSE    (0.25)

int main(int argc, char ** argv) {
    if (argc < 5) {
        return 1;
    }
    tNl3FilterCoeffs c;
    tNl3Filter       f;
    static double    h[POINTS];
    bool             fixed = (argc > 5) && (atoi(argv[5]) != 0);

    nl3_filter_coeffs(nl3_filter_mode((uint32_t)atoi(argv[1]), (uint32_t)atoi(argv[2])), atof(argv[3]), atof(argv[4]), &c);
    nl3_filter_reset(&f);

    for (int i = 0; i < POINTS; i++) {
        double x = (i == 0) ? IMPULSE : 0.0;

        // the filter leaves a quarter of its input at DC
        h[i] = 4.0 * (fixed ? nl3_word_value(nl3_filter_process_fixed(&f, &c, nl3_word(x))) : nl3_filter_process(&f, &c, x)) / IMPULSE;
    }

    for (double hz = 40.0; hz < 20000.0; hz *= 1.1) {
        double re = 0.0, im = 0.0, w = 2.0 * M_PI * hz / NL3_ENGINE_HZ;

        for (int i = 0; i < POINTS; i++) {
            re += h[i] * cos(w * i);
            im -= h[i] * sin(w * i);
        }

        printf("%.2f %.2f\n", hz, 10.0 * log10((re * re) + (im * im) + 1e-14));
    }

    return 0;
}
