// Build: clang -O2 -Isrc tools/osCacheTest.c src/osVoice.c src/osEnvelope.c src/osResample.c src/osCache.c \
//        src/osSample.c -framework AudioToolbox -framework CoreFoundation -o /tmp/osCacheTest
// (add -fsanitize=thread for the threading half). Checks the resampled copies (osCache notes): a note through a copy
// plays what the live interpolator plays, forwards and reversed; Reverse at the file's own rate is the file backwards;
// what 8 voices cost each way; and the audio thread taking and dropping copies while the worker builds and evicts.
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "osCache.h"
#include "osVoice.h"

static tOsSample gSample;

static const tOsSample * borrow(void * user, uint32_t serial) {
    (void)user;
    return (serial == gSample.serial) ? &gSample : NULL;
}

static void give_back(void * user, const tOsSample * s) {
    (void)user;
    (void)s;
}

static double now(void) {
    struct timespec t;

    clock_gettime(CLOCK_MONOTONIC, &t);
    return t.tv_sec + (t.tv_nsec * 1e-9);
}

static void render(tOsVoices * v, const tOsPlay * p, float * l, float * r, uint32_t n) {
    memset(l, 0, n * sizeof(float));
    memset(r, 0, n * sizeof(float));

    for (uint32_t at = 0; at < n; at += 512) {
        os_voices_render(v, p, l + at, r + at, (n - at < 512) ? (n - at) : 512);
    }
}

static bool wait_for(tOsCache * c, tOsCacheKey key) {
    for (int i = 0; i < 400; i++) {
        const tOsCacheEntry * e = os_cache_acquire(c, key);

        if (e != NULL) {
            os_cache_release(e);
            return true;
        }
        usleep(5000);
    }

    return false;
}

int main(void) {
    int      failures = 0;
    uint32_t n        = 44100;
    tOsCache cache;

    gSample = (tOsSample){
        .serial = 7, .frames = n, .channels = 2, .rate = 44100.0
    };

    for (int ch = 0; ch < 2; ch++) {
        gSample.data[ch] = malloc(n * sizeof(float));

        for (uint32_t i = 0; i < n; i++) {
            gSample.data[ch][i] = (float)(((rand() % 2001) - 1000) / 1000.0) * expf(-(float)i / 8000.0f);
        }
    }

    os_cache_init(&cache, (tOsCacheSource){borrow, give_back}, NULL);

    // 1. through a copy against live, at the pitches a 44.1 kHz file takes in a 48 kHz project
    double   tunes[]  = {0, 7, 12, -5, 24};
    uint32_t m        = 60000;
    float *  l1 = malloc(m * 4), * r1 = malloc(m * 4), * l2 = malloc(m * 4), * r2 = malloc(m * 4);

    for (int rev = 0; rev < 2; rev++) {
        for (int k = 0; k < 5; k++) {
            tOsPlay     p       = {.rootKey = 60, .gain = 1, .tune = tunes[k], .reverse = rev, .env = {.hold = -1.0, .sustain = 1.0}};
            tOsVoices   live, cached;

            os_voices_init(&live, 48000);
            os_voices_init(&cached, 48000);
            os_voices_set_cache(&cached, &cache);
            tOsCacheKey key     = {gSample.serial, rev, os_voices_step(&cached, &gSample, &p, 60)};

            os_cache_request(&cache, key, true);

            if (!wait_for(&cache, key)) {
                printf("FAIL  copy never built: %+g st%s\n", tunes[k], rev ? " reversed" : "");
                failures++;
                continue;
            }
            os_voices_note_on(&live, &gSample, &p, 60, 1.0f);
            os_voices_note_on(&cached, &gSample, &p, 60, 1.0f);
            bool        viaCopy = cached.voice[0].copy != NULL;

            render(&live, &p, l1, r1, m);
            render(&cached, &p, l2, r2, m);
            double      worst   = 0;

            for (uint32_t i = 0; i < m; i++) {
                worst = fmax(worst, fmax(fabs(l1[i] - l2[i]), fabs(r1[i] - r2[i])));
            }

            bool        ok      = viaCopy && (worst < 1e-5);

            printf("%s  %+3g st%-9s via copy %s, largest difference from live %.1e\n", ok ? "ok  " : "FAIL", tunes[k],
                   rev ? " reversed" : "", viaCopy ? "yes" : "NO", worst);
            failures += ok ? 0 : 1;
            os_voices_stop(&cached);
        }
    }

    // 2. Reverse at the file's own rate: the file backwards, sample for sample
    {
        tOsPlay   p     = {.rootKey = 60, .gain = 1, .reverse = true, .env = {.hold = -1.0, .sustain = 1.0}};
        tOsVoices v;
        double    worst = 0;

        os_voices_init(&v, 44100);
        os_voices_note_on(&v, &gSample, &p, 60, 1.0f);
        render(&v, &p, l1, r1, n);

        for (uint32_t i = 0; i < n; i++) {
            worst = fmax(worst, fabs(l1[i] - gSample.data[0][n - 1 - i]));
        }

        printf("%s  reversed at the file's rate: largest difference from the file backwards %.1e\n", (worst < 1e-6) ? "ok  " : "FAIL", worst);
        failures += (worst < 1e-6) ? 0 : 1;
    }

    // 3. what 8 voices cost, live and through a copy (a note every 85 ms, so all eight stay busy)
    for (int useCache = 0; useCache < 2; useCache++) {
        tOsPlay   p = {.rootKey = 60, .gain = 1, .env = {.hold = -1.0, .sustain = 1.0}};
        tOsVoices v;
        float     L[512], R[512];
        double    seconds = 5.0, t0;

        os_voices_init(&v, 48000);
        os_voices_set_cache(&v, useCache ? &cache : NULL);
        (void)wait_for(&cache, (tOsCacheKey){gSample.serial, false, os_voices_step(&v, &gSample, &p, 60)});
        t0 = now();

        for (int b = 0; b < (int)(seconds * 48000 / 512); b++) {
            if ((b % 8) == 0) {
                os_voices_note_on(&v, &gSample, &p, 60, 1.0f);
            }
            memset(L, 0, sizeof(L));
            memset(R, 0, sizeof(R));
            os_voices_render(&v, &p, L, R, 512);
        }

        printf("      8 voices, 44.1 kHz file at 48 kHz, %-13s %.2f%% of a core\n", useCache ? "through copy:" : "live:", (now() - t0) / seconds * 100.0);
        os_voices_stop(&v);
    }

    // 4. key-tracked notes asked for and played while the worker builds and evicts (run under -fsanitize=thread)
    {
        tOsPlay   p = {.rootKey = 60, .gain = 1, .keyTrack = true, .env = {.hold = -1.0, .sustain = 1.0}};
        tOsVoices v;
        float     L[512], R[512];

        cache.budget = 3 * os_cache_length(&gSample, 0.5) * 2 * sizeof(float);   // room for a few: eviction runs
        os_voices_init(&v, 48000);
        os_voices_set_cache(&v, &cache);

        for (int b = 0; b < 4000; b++) {
            if ((b % 3) == 0) {
                os_voices_note_on(&v, &gSample, &p, (uint8_t)(48 + (rand() % 25)), 1.0f);
            }
            os_voices_render(&v, &p, L, R, 512);

            if ((b % 500) == 0) {
                usleep(2000);
            }
        }

        os_voices_stop(&v);
        printf("ok    stress: 1333 key-tracked notes with eviction, no crash\n");
    }

    os_cache_destroy(&cache);
    printf(failures ? "\n%d FAILED\n" : "\nall passed\n", failures);
    return failures ? 1 : 0;
}
