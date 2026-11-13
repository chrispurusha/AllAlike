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

enum {
    kEffect = 0,
    kPhaserType,
    kPhaserRate,
    kPhaserFeedback,
    kChorusDetune,
    kChorusAmount,
    kFlangerRate,
    kFlangerRange,
    kFlangerFeedback,
    kOutput,
    kStereo,
    kOffset,
    kChorusInput,
    kMix,
    kEconomy,
    kFixedPoint,
    kOutputStage,
    kAntiAlias
};

static int                         sFailures;
static int                         sChecks;

#define CHECK(cond, ...)                         \
   do {                                          \
       sChecks++;                                \
       if (!(cond)) {                            \
           sFailures++;                          \
           printf("  FAIL line %d: ", __LINE__); \
           printf(__VA_ARGS__);                  \
           printf("\n");                         \
       }                                         \
   } while (0)

static const tSynthLibPluginDesc * sDesc;

typedef struct {
    float * left;
    float * right;
    int     frames;
} tTake;

// Renders `seconds` of a stereo input through the plug-in. The input: a 440 Hz sine on both sides
// (`kind` 0), an impulse at 0.1 s on both (1), white noise the same on both (2), noise on the left only (3), or
// noise on the left and its inverse on the right - all side, no mid (4). The input is kept in sInL/sInR.
static float *                     sInL;
static float *                     sInR;

static tTake run(void * inst, double seconds, int kind) {
    tTake take = {NULL, NULL, (int)(seconds * RATE)};
    float inL[BLOCK], inR[BLOCK];

    take.left  = calloc((size_t)take.frames, sizeof(float));
    take.right = calloc((size_t)take.frames, sizeof(float));
    free(sInL);
    free(sInR);
    sInL       = calloc((size_t)take.frames, sizeof(float));
    sInR       = calloc((size_t)take.frames, sizeof(float));
    srand(1);

    for (int done = 0; done < take.frames; done += BLOCK) {
        int           n      = (take.frames - done < BLOCK) ? (take.frames - done) : BLOCK;
        const float * in[2]  = {inL, inR};
        float *       out[2] = {take.left + done, take.right + done};

        for (int i = 0; i < n; i++) {
            long   k = done + i;
            double v = (kind == 0) ? 0.25 * sin(2.0 * M_PI * 440.0 * (double)k / RATE)
                       : (kind == 1) ? ((k == (long)(0.1 * RATE)) ? 1.0 : 0.0)
                       : 0.25 * ((2.0 * rand() / (double)RAND_MAX) - 1.0);

            inL[i]  = (float)v;
            inR[i]  = (kind == 3) ? 0.0f : ((kind == 4) ? (float)-v : (float)v);
            sInL[k] = inL[i];
            sInR[k] = inR[i];
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
    int    w = (int)(0.02 * RATE);
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

// The apply thread commits a change within 5 ms; keep rendering, with pauses, as a host keeps calling
static void settle(void * inst) {
    struct timespec ts = {0, 15 * 1000 * 1000};

    for (int i = 0; i < 4; i++) {
        tTake t = run(inst, 0.05, 2);

        drop(&t);
        nanosleep(&ts, NULL);
    }
}

// Mean |L - R| over the second half of a take
static double side_of(const tTake * t) {
    double sum = 0.0;

    for (int i = t->frames / 2; i < t->frames; i++) {
        sum += fabs((double)t->left[i] - t->right[i]);
    }

    return sum / (t->frames / 2);
}

// The largest difference between an output and the input `delay` samples earlier, over the second half
static double error_against_input(const float * out, const float * in, int frames, int delay) {
    double worst = 0.0;

    for (int i = frames / 2; i < frames; i++) {
        double e = fabs((double)out[i] - in[i - delay]);

        worst = (e > worst) ? e : worst;
    }

    return worst;
}

// The level of a sine at `hz` through the plug-in, left channel, dB against its input
static double tone_db(void * inst, double hz) {
    int    frames = (int)(0.5 * RATE);
    float  inL[BLOCK], inR[BLOCK], * l = calloc((size_t)frames, sizeof(float)), * r = calloc((size_t)frames, sizeof(float));
    double so = 0.0, si = 0.0;

    for (int done = 0; done < frames; done += BLOCK) {
        const float * in[2]  = {inL, inR};
        float *       out[2] = {l + done, r + done};

        for (int i = 0; i < BLOCK; i++) {
            inL[i] = inR[i] = 0.25f * (float)sin(2.0 * M_PI * hz * (double)(done + i) / RATE);
        }

        sDesc->cb.process(inst, in, 2, out, 2, BLOCK, NULL);

        for (int i = 0; (done >= frames / 2) && (i < BLOCK); i++) {
            so += (double)l[done + i] * l[done + i];
            si += (double)inL[i] * inL[i];
        }
    }

    free(l);
    free(r);
    return 10.0 * log10(so / si);
}

// A host's half of the wrapper, enough for the plug-in to call back into: counts latency changes
static int sLatencyChanges;

static void fmt_param_edited(void * inst, uint32_t id, double normalized) {
    (void)inst;
    (void)id;
    (void)normalized;
}

static double fmt_param_value(void * inst, uint32_t id) {
    (void)inst;
    (void)id;
    return 0.0;
}

static void fmt_latency_changed(void * inst) {
    (void)inst;
    sLatencyChanges++;
}

static bool fmt_request_resize(void * inst, double w, double h) {
    (void)inst;
    (void)w;
    (void)h;
    return false;
}

static bool fmt_send_message(void * inst, const char * id, int64_t v) {
    (void)inst;
    (void)id;
    (void)v;
    return false;
}

static bool fmt_midi_out(void * inst, uint8_t status, uint8_t d1, uint8_t d2, uint32_t offset) {
    (void)inst;
    (void)status;
    (void)d1;
    (void)d2;
    (void)offset;
    return false;
}

static const tSynthLibPluginFormat kFormat = {
    fmt_param_edited, fmt_param_value, fmt_latency_changed, fmt_request_resize, fmt_send_message, fmt_midi_out
};

int main(void) {
    const tSynthLibPluginSet * set_  = synthlib_plugin_variants();

    sDesc = &set_->variants[0];
    synthlib_plugin_set_format(&kFormat);
    printf("%s %s: %u parameters, %s\n", sDesc->name, sDesc->version, sDesc->numParams,
           sDesc->isInstrument ? "instrument" : "effect");

    void *                     inst  = sDesc->cb.create(sDesc);
    tSynthLibSetup             setup = {RATE, BLOCK, true};

    CHECK(inst != NULL, "create failed");

    if (inst == NULL) {
        return 1;
    }
    sDesc->cb.prepare(inst, &setup);

    // ---- Latency: the impulse comes out where the plug-in says
    char                       first[64];

    sDesc->cb.paramText(sDesc, inst, kEffect, sDesc->cb.getParam(inst, kEffect), first, sizeof first);
    printf("a new instance plays: %s\n", first);
    CHECK(strcmp(first, "Chorus") == 0, "a new instance starts on %s, not the Chorus", first);

    printf("latency\n");
    set(inst, kEffect, 0.0);    // the Phaser at rest: the impulse comes through undelayed by the effect
    set(inst, kPhaserRate, 0.0);
    set(inst, kPhaserFeedback, 0.0);
    settle(inst);
    {
        tTake t    = run(inst, 0.5, 1);
        int   peak = 0;

        for (int i = 1; i < t.frames; i++) {
            peak = (fabsf(t.left[i]) > fabsf(t.left[peak])) ? i : peak;
        }

        int   at   = peak - (int)(0.1 * RATE);

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

        t     = run(inst, 2.0, 0);            // a sine's level should swing as the notches sweep past
        move  = movement_db(t.left, t.frames);
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
        tTake  a      = run(inst, 1.0, 2);
        double before = rms(a.left, a.frames / 2, a.frames / 2);

        set(inst, kOutput, 18.0 / 36.0);  // -6 dB
        settle(inst);
        tTake  b      = run(inst, 1.0, 2);
        double after  = rms(b.left, b.frames / 2, b.frames / 2);
        double db     = 20.0 * log10(after / before);

        printf("  Output -6 dB moves the level %.2f dB\n", db);
        CHECK(fabs(db + 6.0) < 1.0, "Output -6 dB gave %.2f dB", db);
        drop(&a);
        drop(&b);
    }

    set(inst, kOutput, 24.0 / 36.0);

    // ---- Wide: the right side's LFO behind the left, for both LFO effects
    for (int effect = 0; effect < 3; effect += 2) {
        const char * name = (effect == 0) ? "Phaser" : "Flanger";

        printf("stereo: Wide, %s\n", name);
        set(inst, kEffect, effect / 2.0);
        set(inst, kStereo, 0.5);
        set(inst, kOffset, 1.0);    // 180 degrees
        settle(inst);
        tTake        t    = run(inst, 1.0, 2);
        double       d    = side_of(&t);

        printf("  180 degrees: L-R %.4f on a mono input\n", d);
        CHECK(d > 0.01, "%s Wide moved nothing: L-R %.4f", name, d);
        drop(&t);

        set(inst, kStereo, 0.0);    // back to Linked
        settle(inst);
        t = run(inst, 1.0, 2);
        d = side_of(&t);
        printf("  back to Linked: L-R %.6f\n", d);
        CHECK(d < 1e-6, "%s Linked after Wide left the sides apart: %.6f", name, d);
        drop(&t);
    }

    // ---- The Chorus keeps a stereo source's image; mono in sums it, as the module does
    printf("stereo: the Chorus's input\n");
    set(inst, kEffect, 0.5);
    settle(inst);
    {
        tTake  t = run(inst, 1.0, 3);   // the left only
        double l = rms(t.left, t.frames / 2, t.frames / 2), r = rms(t.right, t.frames / 2, t.frames / 2);

        printf("  Stereo in, a left-only source: left %.4f, right %.4f\n", l, r);
        CHECK(r < (l / 100.0), "the left-only source reached the right: %.4f against %.4f", r, l);
        drop(&t);
    }
    set(inst, kChorusInput, 1.0);
    settle(inst);
    {
        tTake  t = run(inst, 1.0, 3);
        double l = rms(t.left, t.frames / 2, t.frames / 2), r = rms(t.right, t.frames / 2, t.frames / 2);

        printf("  Mono in: left %.4f, right %.4f\n", l, r);
        CHECK(r > (l / 4.0), "mono in did not spread the left to the right: %.4f against %.4f", r, l);
        drop(&t);
    }
    set(inst, kChorusInput, 0.0);

    // ---- Mid/Side: the side passes untouched, only the mid is processed
    printf("stereo: Mid/Side\n");
    set(inst, kEffect, 1.0);
    set(inst, kStereo, 1.0);
    settle(inst);
    {
        uint32_t lat = sDesc->cb.latencySamples(inst);
        tTake    t   = run(inst, 1.0, 4);   // all side, no mid
        double   eL  = error_against_input(t.left, sInL, t.frames, (int)lat);
        double   eR  = error_against_input(t.right, sInR, t.frames, (int)lat);

        printf("  a side-only input comes out as it went in, %u samples later: error %.2g, %.2g\n", lat, eL, eR);
        CHECK((eL < 1e-5) && (eR < 1e-5), "the side was changed: %g, %g", eL, eR);
        drop(&t);
    }
    set(inst, kStereo, 0.0);

    // ---- Mix: 0 is the input, delayed to line up
    printf("mix\n");
    set(inst, kMix, 0.0);
    settle(inst);
    {
        uint32_t lat = sDesc->cb.latencySamples(inst);
        tTake    t   = run(inst, 1.0, 3);
        double   eL  = error_against_input(t.left, sInL, t.frames, (int)lat);
        double   eR  = error_against_input(t.right, sInR, t.frames, (int)lat);

        printf("  Mix 0: the input, %u samples later: error %.2g, %.2g\n", lat, eL, eR);
        CHECK((eL < 1e-6) && (eR < 1e-6), "Mix 0 is not the dry input: %g, %g", eL, eR);
        drop(&t);
    }
    set(inst, kMix, 1.0);

    // ---- At 96 kHz the engine needs no resampling, and the plug-in says so
    printf("latency at 96 kHz\n");
    {
        tSynthLibSetup fast   = {96000.0, BLOCK, true};
        int            before = sLatencyChanges;

        sDesc->cb.prepare(inst, &fast);
        printf("  reported %u, host told %d time(s)\n", sDesc->cb.latencySamples(inst), sLatencyChanges - before);
        CHECK(sDesc->cb.latencySamples(inst) == 0, "reported %u at 96 kHz", sDesc->cb.latencySamples(inst));
        CHECK(sLatencyChanges > before, "the host was not told the latency changed");
        sDesc->cb.prepare(inst, &setup);
    }

    // ---- Engine Rate: the host's rate - no latency; each effect's level against the 96 kHz one
    printf("engine rate: host (economy)\n");
    {
        static const char * names[3] = {"Phaser", "Chorus", "Flanger"};
        double              level96[3];

        for (int effect = 0; effect < 3; effect++) {
            set(inst, kEffect, effect / 2.0);
            settle(inst);
            tTake t = run(inst, 2.0, 2);

            level96[effect] = rms(t.left, t.frames / 2, t.frames / 2);
            drop(&t);
        }

        int                 before   = sLatencyChanges;

        set(inst, kEconomy, 1.0);
        settle(inst);
        settle(inst);
        printf("  latency %u, host told %d time(s)\n", sDesc->cb.latencySamples(inst), sLatencyChanges - before);
        CHECK(sDesc->cb.latencySamples(inst) == 0, "economy reports %u", sDesc->cb.latencySamples(inst));
        CHECK(sLatencyChanges > before, "the host was not told the latency changed");

        for (int effect = 0; effect < 3; effect++) {
            set(inst, kEffect, effect / 2.0);
            settle(inst);
            tTake  t  = run(inst, 2.0, 2);
            double db = 20.0 * log10(rms(t.left, t.frames / 2, t.frames / 2) / level96[effect]);

            printf("  %s level against 96 kHz: %+.2f dB\n", names[effect], db);
            CHECK(fabs(db) < 1.5, "%s moved %+.2f dB at the host's rate", names[effect], db);
            drop(&t);
        }

        set(inst, kEconomy, 0.0);
        settle(inst);
        settle(inst);
        CHECK(sDesc->cb.latencySamples(inst) == 31, "back at 96 kHz the latency is %u", sDesc->cb.latencySamples(inst));
    }

    // ---- The output stage, Anti-alias and fixed point, on a tone through the Chorus at Amount 0 (the dry path
    // through the engine)
    printf("output stage, anti-alias, fixed point\n");
    {
        set(inst, kEffect, 0.5);
        set(inst, kChorusAmount, 0.0);
        settle(inst);
        double flat15  = tone_db(inst, 15000.0) - tone_db(inst, 440.0);

        set(inst, kOutputStage, 1.0);
        settle(inst);
        double stage15 = tone_db(inst, 15000.0) - tone_db(inst, 440.0);

        printf("  96 kHz, 15 kHz against 440 Hz: output stage off %+.2f dB, on %+.2f dB\n", flat15, stage15);
        CHECK(fabs(flat15) < 0.15, "output stage off is not flat: %+.2f dB", flat15);
        CHECK((stage15 < -0.3) && (stage15 > -0.7), "output stage on: %+.2f dB at 15 kHz", stage15);

        double aa96    = tone_db(inst, 20000.0) - tone_db(inst, 440.0);

        set(inst, kEconomy, 1.0);   // the output stage is forced off here; Anti-alias (on) filters the input
        settle(inst);
        settle(inst);
        double host15  = tone_db(inst, 15000.0) - tone_db(inst, 440.0);
        double aaOn    = tone_db(inst, 20000.0) - tone_db(inst, 440.0);

        set(inst, kAntiAlias, 0.0);
        settle(inst);
        double aaOff   = tone_db(inst, 20000.0) - tone_db(inst, 440.0);

        printf("  host rate: 15 kHz %+.2f dB (output stage forced off, anti-alias on); 20 kHz anti-alias on %+.2f, off %+.2f, at 96 kHz %+.2f\n",
               host15, aaOn, aaOff, aa96);
        CHECK(aaOn < (aaOff - 2.0), "Anti-alias did not cut 20 kHz at the host rate: on %+.2f, off %+.2f", aaOn, aaOff);
        CHECK(fabs(aaOff) < 0.5, "host rate with Anti-alias off is not flat at 20 kHz: %+.2f", aaOff);
        // at 96 kHz the engine's own resampling rolls off just below the host's Nyquist: 20 kHz measured -3.4 dB at
        // 48 kHz, with Anti-alias doing nothing there (notes §18)
        CHECK((aa96 > -4.5) && (aa96 < -2.0), "the 96 kHz roll-off at 20 kHz moved: %+.2f", aa96);

        set(inst, kAntiAlias, 1.0);
        set(inst, kEconomy, 0.0);
        set(inst, kOutputStage, 0.0);
        set(inst, kFixedPoint, 1.0);
        settle(inst);
        settle(inst);
        double fixed   = tone_db(inst, 440.0);

        set(inst, kFixedPoint, 0.0);
        settle(inst);
        double flt     = tone_db(inst, 440.0);

        printf("  440 Hz through the engine: fixed %+.3f dB, floating %+.3f dB\n", fixed, flt);
        CHECK(fabs(fixed - flt) < 0.01, "fixed point moved a plain tone %+.3f dB", fixed - flt);
        set(inst, kChorusAmount, 1.0);
        settle(inst);
    }

    char text[64];

    sDesc->cb.paramText(sDesc, inst, kEffect, 0.5, text, sizeof text);
    CHECK(strcmp(text, "Chorus") == 0, "Effect 0.5 reads \"%s\"", text);

    sDesc->cb.destroy(inst);
    printf("%d checks, %d failed\n", sChecks, sFailures);
    return (sFailures == 0) ? 0 : 1;
}
