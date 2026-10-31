// Renders a Single through the engine, offline: a 524-byte .syx, a script of notes, a 32-bit float stereo WAV.
// usage: tirender <single.syx> <script> <out.wav> <seconds> <rate>
// A script line is "<seconds> on 1 <note> <velocity>" or "<seconds> off 1 <note>" or "<seconds> ctl <id> <value>".
// Build: tools/do-tirender
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tiVoices.h"

#define SINGLE_LENGTH    (524)
#define BLOCK            (64u)
#define PAGE_A           (9)
#define PAGE_C           (266)

typedef struct {
    double t;
    int    kind, a;
    double b;
} tEvent;

static tTiVoices gVoices;

static void put32(FILE * f, uint32_t v) {
    fwrite(&v, 4, 1, f);
}

static void put16(FILE * f, uint16_t v) {
    fwrite(&v, 2, 1, f);
}

int main(int argc, char ** argv) {
    if (argc < 6) {
        fprintf(stderr, "usage: tirender <single.syx> <script> <out.wav> <seconds> <rate>\n");
        return 2;
    }
    uint8_t       single[SINGLE_LENGTH];
    FILE *        f     = fopen(argv[1], "rb");

    if ((f == NULL) || (fread(single, 1, SINGLE_LENGTH, f) != SINGLE_LENGTH)) {
        fprintf(stderr, "no Single in %s\n", argv[1]);
        return 1;
    }
    fclose(f);
    tTiSound      sound;

    memcpy(sound.a, single + PAGE_A, 128);
    memcpy(sound.b, single + PAGE_A + 128, 128);
    memcpy(sound.c, single + PAGE_C, 128);
    memcpy(sound.d, single + PAGE_C + 128, 128);

    static tEvent events[4096];
    int           count = 0;
    char          line[256];

    f = fopen(argv[2], "r");

    while ((f != NULL) && (fgets(line, sizeof(line), f) != NULL) && (count < 4096)) {
        double t, v = 0.0;
        char   kind[16];
        int    ch, n;

        if (sscanf(line, "%lf %15s %d %d %lf", &t, kind, &ch, &n, &v) >= 4) {
            events[count++] = (tEvent){
                t, (strcmp(kind, "on") == 0) ? 1 : 0, n, v
            };
        } else if (sscanf(line, "%lf ctl %d %lf", &t, &n, &v) == 3) {
            events[count++] = (tEvent){
                t, 2, n, v
            };
        }
    }

    if (f != NULL) {
        fclose(f);
    }
    double   seconds = atof(argv[4]), rate = atof(argv[5]);
    uint32_t frames = (uint32_t)(seconds * rate);
    float *  l      = (float *)calloc(frames + BLOCK, sizeof(float));
    float *  r      = (float *)calloc(frames + BLOCK, sizeof(float));

    ti_voices_init(&gVoices, rate);
    ti_voices_set_tempo(&gVoices, 120.0);
    ti_voices_set_effects(&gVoices, true, true);
    ti_voices_set_fixed(&gVoices, getenv("TI_FLOAT") == NULL);
    gVoices.sound = sound;
    int      next   = 0;

    for (uint32_t done = 0; done < frames;) {
        uint32_t at = frames;

        while ((next < count) && ((uint32_t)(events[next].t * rate) <= done)) {
            tEvent * e = &events[next++];

            if (e->kind == 1) {
                ti_voices_note_on(&gVoices, (uint8_t)e->a, (float)(e->b / 127.0));
            } else if (e->kind == 0) {
                ti_voices_note_off(&gVoices, (uint8_t)e->a, 0.0f);
            } else {
                ti_voices_control(&gVoices, (tTiControlId)e->a, e->b);
            }
        }

        if (next < count) {
            at = (uint32_t)(events[next].t * rate);
        }
        at   = (at > (done + BLOCK)) ? (done + BLOCK) : at;
        at   = (at > frames) ? frames : at;
        ti_voices_render(&gVoices, &sound, l + done, r + done, at - done);
        done = at;
    }

    f = fopen(argv[3], "wb");

    if (f == NULL) {
        return 1;
    }
    fwrite("RIFF", 1, 4, f);
    put32(f, 36 + (frames * 8));
    fwrite("WAVEfmt ", 1, 8, f);
    put32(f, 16);
    put16(f, 3);
    put16(f, 2);
    put32(f, (uint32_t)rate);
    put32(f, (uint32_t)rate * 8);
    put16(f, 8);
    put16(f, 32);
    fwrite("data", 1, 4, f);
    put32(f, frames * 8);

    for (uint32_t i = 0; i < frames; i++) {
        fwrite(&l[i], 4, 1, f);
        fwrite(&r[i], 4, 1, f);
    }

    fclose(f);
    return 0;
}
