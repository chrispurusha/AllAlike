// Build: clang -O2 -Isrc tools/osCacheTest.c src/osVoice.c src/osEnvelope.c src/osResample.c src/osCache.c \
//        src/osSample.c -framework AudioToolbox -framework CoreFoundation -o /tmp/osCacheTest
// (add -fsanitize=thread for the threading half; it links src/osStretch.cpp and SynthLib's build/librubberband.a,
// -lc++ -framework Accelerate). Checks the resampled copies (osCache notes): a note through a copy
// plays what the live interpolator plays, forwards and reversed; Reverse at the file's own rate is the file backwards;
// what 8 voices cost each way; and the audio thread taking and dropping copies while the worker builds and evicts.
#include <math.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "osCache.h"
#include "osVoice.h"

static tOsSample gSample;
static _Atomic(const tOsSample *) gCurrent;   // what the cache's thread may read, published whole as the plug-in does

static const tOsSample * borrow(void * user, uint32_t serial) {
    const tOsSample * s = atomic_load(&gCurrent);

    (void)user;
    return ((s != NULL) && (serial == s->serial)) ? s : NULL;
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

        if ((i % 20) == 19) {
            os_cache_request(c, key, false);   // a full queue drops a request; the plug-in asks again at the next note
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

    atomic_store(&gCurrent, &gSample);
    os_cache_init(&cache, (tOsCacheSource){borrow, give_back}, NULL);

    // 1. through a copy against live, at the pitches a 44.1 kHz file takes in a 48 kHz project
    double   tunes[]  = {0, 7, 12, -5, 24};
    uint32_t m        = 60000;
    float *  l1 = malloc(m * 4), * r1 = malloc(m * 4), * l2 = malloc(m * 4), * r2 = malloc(m * 4);

    for (int rev = 0; rev < 2; rev++) {
        for (int k = 0; k < 5; k++) {
            tOsPlay     p       = {.rootKey = 60, .gain = 1, .end = 1, .tune = tunes[k], .reverse = rev, .env = {.hold = -1.0, .sustain = 1.0}};
            tOsVoices   live, cached;

            os_voices_init(&live, 48000);
            os_voices_init(&cached, 48000);
            os_voices_set_cache(&cached, &cache);
            tOsCacheKey key     = os_voices_key(&cached, &gSample, &p, 60);

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
        tOsPlay   p     = {.rootKey = 60, .gain = 1, .end = 1, .reverse = true, .env = {.hold = -1.0, .sustain = 1.0}};
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
        tOsPlay   p = {.rootKey = 60, .gain = 1, .end = 1, .env = {.hold = -1.0, .sustain = 1.0}};
        tOsVoices v;
        float     L[512], R[512];
        double    seconds = 5.0, t0;

        os_voices_init(&v, 48000);
        os_voices_set_cache(&v, useCache ? &cache : NULL);
        (void)wait_for(&cache, os_voices_key(&v, &gSample, &p, 60));
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
        tOsPlay   p = {.rootKey = 60, .gain = 1, .end = 1, .keyTrack = true, .env = {.hold = -1.0, .sustain = 1.0}};
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

    // 5. stretched copies (osCache notes §7): a 440 Hz tone, 1 s, 44.1 kHz, its first 50 ms protected
    {
        static tOsSample tone;
        uint32_t         frames  = 44100;

        tone         = (tOsSample){
            .serial = 9, .frames = frames, .channels = 1, .rate = 44100.0
        };
        tone.data[0] = tone.data[1] = malloc(frames * sizeof(float));

        for (uint32_t i = 0; i < frames; i++) {
            tone.data[0][i] = 0.5f * sinf(2.0f * (float)M_PI * 440.0f * (float)i / 44100.0f);
        }

        atomic_store(&gCurrent, &tone);
        cache.budget = (size_t)256 * 1024 * 1024;

        struct {
            double stretch, tune;
            bool   reverse;
        }                cases[] = {{2.0, 0, false}, {0.5, 0, false}, {4.0, 0, false}, {2.0, 12, false}, {0.5, -12, false}, {2.0, 0, true}};

        for (int k = 0; k < 6; k++) {
            tOsPlay               p      = {
                .rootKey =   60, .gain = 1, .tune = cases[k].tune, .reverse = cases[k].reverse, .stretch = cases[k].stretch,
                .protect = 0.05, .end  = 1, .env  = {.hold = -1.0, .sustain = 1.0}};
            tOsVoices             v;

            os_voices_init(&v, 48000);
            os_voices_set_cache(&v, &cache);
            tOsCacheKey           key    = os_voices_key(&v, &tone, &p, 60);

            os_cache_request(&cache, key, false);

            if (!wait_for(&cache, key)) {
                printf("FAIL  stretched copy never built: x%g %+g st\n", cases[k].stretch, cases[k].tune);
                failures++;
                continue;
            }
            const tOsCacheEntry * e      = os_cache_acquire(&cache, key);
            double                want   = 1.0 * cases[k].stretch;
            double                got    = e->frames / 48000.0;
            double                pitch  = pow(2.0, cases[k].tune / 12.0);
            uint32_t              from = e->plan.protectOut + 2400, to = e->frames - 2400;
            int                   cross  = 0;

            for (uint32_t i = from + 1; i < to; i++) {
                cross += (e->data[0][i - 1] < 0.0f) && (e->data[0][i] >= 0.0f);
            }

            double                hz     = cross / ((to - from) / 48000.0);
            double                worst  = 0.0;   // the protected part is the resampled original, untouched
            tOsCopyPlan           plain  = {.step = key.step};
            float *               ref    = malloc((e->plan.protectOut + 1) * sizeof(float));

            (void)plain;
            os_cache_render(&tone, cases[k].reverse, key.step, ref, NULL, e->plan.protectOut);

            for (uint32_t i = 0; i < e->plan.protectOut; i++) {
                worst = fmax(worst, fabs(ref[i] - e->data[0][i]));
            }

            free(ref);

            // the seam: no step from one frame to the next bigger than the tone's own largest, and the level held
            double                seamStep = 0.0, toneStep = 0.0, before = 0.0, after = 0.0;
            uint32_t              at     = e->plan.protectOut;

            for (uint32_t i = at - 400; i < at + 600; i++) {
                seamStep = fmax(seamStep, fabs(e->data[0][i] - e->data[0][i - 1]));
            }

            for (uint32_t i = from + 1; i < to; i++) {
                toneStep = fmax(toneStep, fabs(e->data[0][i] - e->data[0][i - 1]));
            }

            for (uint32_t i = at - 1000; i < at; i++) {
                before = fmax(before, fabs(e->data[0][i]));
            }

            for (uint32_t i = at + 200; i < at + 1200; i++) {
                after = fmax(after, fabs(e->data[0][i]));
            }

            bool                  seamOk = (seamStep < toneStep * 1.05) && (fabs(20.0 * log10(after / before)) < 1.0);

            printf("      seam: largest step %.4f (tone %.4f), level %+.2f dB across it\n", seamStep, toneStep, 20.0 * log10(after / before));

            bool                  ok     = seamOk && (fabs(got - want) < 0.002) && (fabs(hz - (440.0 * pitch)) < (440.0 * pitch * 0.01)) && (worst < 1e-6);

            printf("%s  x%.2f %+3g st%-9s length %.4f s (want %.4f), tone %.1f Hz (want %.1f), protected part %s\n",
                   ok ? "ok  " : "FAIL", cases[k].stretch, cases[k].tune, cases[k].reverse ? " reversed" : "", got, want, hz,
                   440.0 * pitch, (worst < 1e-6) ? "untouched" : "CHANGED");
            failures += ok ? 0 : 1;
            os_cache_release(e);
        }
    }

    // 6. End (osVoice notes §9): where the note stops, live and through a stretched copy, snapped and not
    {
        const tOsSample * tone    = atomic_load(&gCurrent); // the 440 Hz tone of section 5
        struct {
            double stretch;
            bool   snap;
        }                 cases[] = {{1.0, false}, {1.0, true}, {2.0, false}, {2.0, true}};

        for (int k = 0; k < 4; k++) {
            tOsPlay   p     = {
                .rootKey =   60, .gain    = 1, .end = 0.5, .snap = cases[k].snap, .stretch = cases[k].stretch, .protect = 0.05,
                .env     = {.hold = -1.0, .sustain = 1.0}};
            tOsVoices v;
            uint32_t  m     = 120000;
            float *   l = calloc(m, sizeof(float)), * r = calloc(m, sizeof(float));

            os_voices_init(&v, 48000);
            os_voices_set_cache(&v, &cache);

            if (cases[k].stretch != 1.0) {
                (void)wait_for(&cache, os_voices_key(&v, tone, &p, 60));
            }
            os_voices_note_on(&v, tone, &p, 60, 1.0f);
            render(&v, &p, l, r, m);
            uint32_t  last  = 0;

            for (uint32_t i = 0; i < m; i++) {
                last = (fabsf(l[i]) > 1e-6f) ? i : last;
            }

            // half the tone, where the copy puts it (its protected 50 ms is not stretched)
            double    want  = 0.5 * 44100.0 * 48000.0 / 44100.0;
            double    slack = cases[k].snap ? (0.005 * 48000.0) : 3.0;

            if (cases[k].stretch != 1.0) {
                const tOsCacheEntry * e = os_cache_acquire(&cache, os_voices_key(&v, tone, &p, 60));

                want = os_cache_position(e, 22050);
                os_cache_release(e);
            }
            // Snap Off: the last frames faded; On: the last frame at a crossing (under one frame's swing of the tone)
            double    tail  = 0.0;

            for (uint32_t i = (cases[k].snap ? last : last - 4); i <= last; i++) {
                tail = fmax(tail, fabs(l[i]));
            }

            bool      ok    = (fabs((double)last - want) <= slack) && (tail < 0.06);

            printf("%s  End 50%% x%.0f Snap %-3s: stops at %u (want %.0f +-%.0f), last frames peak %.3f\n", ok ? "ok  " : "FAIL",
                   cases[k].stretch, cases[k].snap ? "On" : "Off", last, want, slack, tail);
            failures += ok ? 0 : 1;
            os_voices_stop(&v);
            free(l);
            free(r);
        }
    }

    // 7. Delay (osVoice notes §10): the first sound comes exactly that many output frames late
    {
        const tOsSample * tone     = atomic_load(&gCurrent);
        double            delays[] = {0.0, 0.0001, 0.010, 0.25};

        for (int k = 0; k < 4; k++) {
            tOsPlay   p     = {.rootKey = 60, .gain = 1, .end = 1, .delay = delays[k], .env = {.hold = -1.0, .sustain = 1.0}};
            tOsVoices v;
            uint32_t  m = 48000, first = m;
            float *   l = calloc(m, sizeof(float)), * r = calloc(m, sizeof(float)), * l0 = calloc(m, sizeof(float));

            os_voices_init(&v, 48000);
            os_voices_note_on(&v, tone, &p, 60, 1.0f);
            render(&v, &p, l, r, m);
            p.delay = 0.0;
            os_voices_init(&v, 48000);
            os_voices_note_on(&v, tone, &p, 60, 1.0f);
            render(&v, &p, l0, r, m);

            uint32_t  wait  = (uint32_t)lround(delays[k] * 48000.0);
            double    worst = 0.0;   // the delayed note is the undelayed one, moved

            for (uint32_t i = 0; i < m; i++) {
                first = ((first == m) && (fabsf(l[i]) > 1e-6f)) ? i : first;
            }

            for (uint32_t i = wait; i < m; i++) {
                worst = fmax(worst, fabs(l[i] - l0[i - wait]));
            }

            for (uint32_t i = 0; i < wait; i++) {
                worst = fmax(worst, fabs(l[i]));
            }

            bool      ok    = (worst < 1e-6);

            printf("%s  Delay %.4f s: silent for %u frames, then the undelayed note moved (largest difference %.1e)\n",
                   ok ? "ok  " : "FAIL", delays[k], first, worst);
            failures += ok ? 0 : 1;
            free(l);
            free(r);
            free(l0);
        }
    }

    // 8. Protect snapped (osVoice notes §11): the boundary the copy is keyed and built on is a zero crossing
    {
        const tOsSample * tone = atomic_load(&gCurrent);

        for (int snap = 0; snap < 2; snap++) {
            tOsPlay     p     = {.rootKey = 60, .gain = 1, .end = 1, .protectSnap = snap, .stretch = 2.0, .protect = 0.051, .env = {.hold = -1.0}};
            tOsVoices   v;

            os_voices_init(&v, 48000);
            tOsCacheKey key   = os_voices_key(&v, tone, &p, 60);
            tOsCopyPlan plan  = os_cache_plan(tone, key);
            uint32_t    f     = plan.protectIn;
            bool        cross = (f > 0) && (f + 1 < tone->frames)
                                && (((tone->data[0][f - 1] <= 0.0f) != (tone->data[0][f + 1] <= 0.0f)) || (tone->data[0][f] == 0.0f));
            bool        ok    = snap ? (cross && (fabsf(tone->data[0][f]) < 0.02f)) : (f == (uint32_t)lround(0.051 * 44100.0));

            printf("%s  Protect 51 ms Snap %-3s: boundary at frame %u (%s), sample there %.4f\n", ok ? "ok  " : "FAIL",
                   snap ? "On" : "Off", f, snap ? "a crossing" : "as set", tone->data[0][f]);
            failures += ok ? 0 : 1;
        }
    }

    os_cache_destroy(&cache);
    printf(failures ? "\n%d FAILED\n" : "\nall passed\n", failures);
    return failures ? 1 : 0;
}
