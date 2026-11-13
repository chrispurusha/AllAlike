/*
 * ModAlike - offline test.
 *
 * Copyright (C) 2026 Chris Turner <chris_purusha@icloud.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

// Built and run by ./tools/do-matest. Drives the plug-in through its own callbacks, as a host would, and checks
// what it hears. Notes: Docs/code-notes/maTest.md

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "synthlibPlugin.h"

#define RATE     (48000.0)
#define BLOCK    (512)

enum {kEffect = 0, kPhaserType, kPhaserRate, kPhaserFeedback, kChorusDetune, kChorusAmount,
      kFlangerRate, kFlangerRange, kFlangerFeedback, kOutput};

static int sFailures;
static int sChecks;

#define CHECK(cond, ...)                                                       \
    do {                                                                       \
        sChecks++;                                                             \
        if (!(cond)) {                                                         \
            sFailures++;                                                       \
            printf("  FAIL line %d: ", __LINE__);                              \
            printf(__VA_ARGS__);                                               \
            printf("\n");                                                      \
        }                                                                      \
    } while (0)

static const tSynthLibPluginDesc * sDesc;

typedef struct {
    float * left;
    float * right;
    int     frames;
} tTake;

// Renders `seconds` of a stereo input through the plug-in. The input: a 440 Hz sine on both sides
// (`kind` 0), an impulse at 0.1 s on both (1), or white noise, the same on both (2).
static tTake run(void * inst, double seconds, int kind) {
    tTake take   = {NULL, NULL, (int)(seconds * RATE)};
    float inL[BLOCK], inR[BLOCK];

    take.left  = calloc((size_t)take.frames, sizeof(float));
    take.right = calloc((size_t)take.frames, sizeof(float));
    srand(1);

    for (int done = 0; done < take.frames; done += BLOCK) {
        int           n       = (take.frames - done < BLOCK) ? (take.frames - done) : BLOCK;
        const float * in[2]   = {inL, inR};
        float *       out[2]  = {take.left + done, take.right + done};

        for (int i = 0; i < n; i++) {
            long   k = done + i;
            double v = (kind == 0) ? 0.25 * sin(2.0 * M_PI * 440.0 * (double)k / RATE)
                       : (kind == 1) ? ((k == (long)(0.1 * RATE)) ? 1.0 : 0.0)
                       : 0.25 * ((2.0 * rand() / (double)RAND_MAX) - 1.0);

            inL[i] = inR[i] = (float)v;
        }
        sDesc->cb.process(inst, in, 2, out, 2, (uint32_t)n, NULL);
    }

    return take;
}

static void drop(tTake * t) {
    free(t->left);
    free(t->right);
}

static double rms(const float * x, int from, int count) {
    double s = 0.0;

    for (int i = from; i < from + count; i++) {
        s += (double)x[i] * x[i];
    }
    return sqrt(s / count);
}

// How far a level moves over the second half of a take: max/min of 20 ms windows, in dB
static double movement_db(const float * x, int frames) {
    int    w  = (int)(0.02 * RATE);
    double lo = 1e9, hi = 0.0;

    for (int at = frames / 2; at + w <= frames; at += w) {
        double r = rms(x, at, w);

        lo = (r < lo) ? r : lo;
        hi = (r > hi) ? r : hi;
    }
    return 20.0 * log10((hi + 1e-12) / (lo + 1e-12));
}

// Settles the apply thread: parameters reach the engine from there within a few milliseconds
static void set(void * inst, int id, double normalized) {
    sDesc->cb.setParam(inst, (uint32_t)id, normalized);
}

static void settle(void * inst) {
    tTake t = run(inst, 0.1, 2);   // blocks while the apply thread commits, as a host would keep calling

    drop(&t);
    struct timespec ts = {0, 30 * 1000 * 1000};

    nanosleep(&ts, NULL);
}

int main(void) {
    const tSynthLibPluginSet * set_ = synthlib_plugin_variants();

    sDesc = &set_->variants[0];
    printf("%s %s: %u parameters, %s\n", sDesc->name, sDesc->version, sDesc->numParams,
           sDesc->isInstrument ? "instrument" : "effect");

    void *          inst  = sDesc->cb.create(sDesc);
    tSynthLibSetup  setup = {RATE, BLOCK, true};

    CHECK(inst != NULL, "create failed");

    if (inst == NULL) {
        return 1;
    }
    sDesc->cb.prepare(inst, &setup);

    // ---- Latency: the impulse comes out where the plug-in says
    printf("latency\n");
    set(inst, kPhaserRate, 0.0);
    set(inst, kPhaserFeedback, 0.0);
    settle(inst);
    {
        tTake t    = run(inst, 0.5, 1);
        int   peak = 0;

        for (int i = 1; i < t.frames; i++) {
            peak = (fabsf(t.left[i]) > fabsf(t.left[peak])) ? i : peak;
        }
        int at = peak - (int)(0.1 * RATE);

        printf("  impulse peak %d samples late, reported %u\n", at, sDesc->cb.latencySamples(inst));
        CHECK(abs(at - (int)sDesc->cb.latencySamples(inst)) <= 4, "reported latency %u, measured %d",
              sDesc->cb.latencySamples(inst), at);
        drop(&t);
    }

    // ---- Each effect: it sounds, at about unity, and it moves
    static const char * names[3] = {"Phaser", "Chorus", "Flanger"};

    for (int effect = 0; effect < 3; effect++) {
        printf("%s\n", names[effect]);
        set(inst, kEffect, effect / 2.0);

        if (effect == 0) {
            set(inst, kPhaserRate, 100.0 / 127.0);
            set(inst, kPhaserFeedback, 100.0 / 127.0);
        }
        settle(inst);

        tTake  t     = run(inst, 2.0, 2);   // noise: a sine can sit between a phaser's notches
        double level = rms(t.left, t.frames / 2, t.frames / 2) / (0.25 / sqrt(3.0));
        double move  = movement_db(t.left, t.frames);
        double side  = 0.0;

        for (int i = t.frames / 2; i < t.frames; i++) {
            side += fabs((double)t.left[i] - t.right[i]);
        }
        side /= (t.frames / 2);
        printf("  level %.2f (%+.1f dB) of the input, moves %.1f dB, L-R mean difference %.4f\n",
               level, 20.0 * log10(level + 1e-12), move, side);
        CHECK((level > 0.3) && (level < 2.5), "%s level %.2f of the input", names[effect], level);

        if (effect == 1) {
            CHECK(side > 0.005, "the Chorus is not stereo: L-R %.4f", side);
        } else {
            CHECK(side < 1e-6, "%s sides differ on a mono input: %.6f", names[effect], side);
        }
        drop(&t);

        t = run(inst, 2.0, 0);                // a sine's level should swing as the notches sweep past
        move = movement_db(t.left, t.frames);
        printf("  on a sine it moves %.1f dB\n", move);

        if (effect != 1) {
            CHECK(move > 1.0, "%s does not move a sine: %.2f dB", names[effect], move);
        }
        drop(&t);
    }

    // ---- Output level
    printf("output level\n");
    set(inst, kEffect, 1.0);    // Flanger
    settle(inst);
    {
        tTake  a = run(inst, 1.0, 2);
        double before = rms(a.left, a.frames / 2, a.frames / 2);

        set(inst, kOutput, 18.0 / 36.0);  // -6 dB
        settle(inst);
        tTake  b = run(inst, 1.0, 2);
        double after = rms(b.left, b.frames / 2, b.frames / 2);
        double db = 20.0 * log10(after / before);

        printf("  Output -6 dB moves the level %.2f dB\n", db);
        CHECK(fabs(db + 6.0) < 1.0, "Output -6 dB gave %.2f dB", db);
        drop(&a);
        drop(&b);
    }

    char text[64];

    sDesc->cb.paramText(sDesc, inst, kEffect, 0.5, text, sizeof text);
    CHECK(strcmp(text, "Chorus") == 0, "Effect 0.5 reads \"%s\"", text);

    sDesc->cb.destroy(inst);
    printf("%d checks, %d failed\n", sChecks, sFailures);
    return (sFailures == 0) ? 0 : 1;
}
