// Renders a program through the engine: nl3render <file.syx> <index> <out.wav> <note> <seconds>
//   $NL3_FIXED: fixed-point arithmetic; $NL3_CLEAN: the Clean aliasing setting
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "nl3Patch.h"
#include "nl3Params.h"
#include "nl3Effects.h"
#include "nl3Voices.h"

#define RATE     (44100)    // NL3_ENGINE_HZ: the voices' own rate
#define BLOCK    (256)

static uint8_t gPrograms[NL3_MAX_SINGLES][NL3_SINGLE_LENGTH];

static void put32(FILE * f, uint32_t v) {
    fwrite(&v, 4, 1, f);
}

static void put16(FILE * f, uint16_t v) {
    fwrite(&v, 2, 1, f);
}

int main(int argc, char ** argv) {
    if (argc < 6) {
        return 1;
    }
    int               count  = nl3_patch_read_file(argv[1], gPrograms, NL3_MAX_SINGLES);
    int               index  = atoi(argv[2]);

    if (index >= count) {
        fprintf(stderr, "%d programs\n", count);
        return 1;
    }
    double            values[kNl3ParamCount];
    char              name[NL3_SINGLE_NAME_LENGTH + 1];

    for (uint32_t i = 0; i < kNl3ParamCount; i++) {
        values[i] = gNl3Params[i].defaultNormalized;
    }

    nl3_patch_from_single(gPrograms[index], values);
    nl3_patch_name(gPrograms[index], name);
    tNl3Sound         sound;

    nl3_sound_from_values(values, &sound);
    uint32_t          frames = (uint32_t)(atof(argv[5]) * RATE);
    float *           out    = calloc(frames + BLOCK, sizeof(float));
    static tNl3Voices voices;

    nl3_voices_init(&voices);
    nl3_voices_set_fixed(&voices, getenv("NL3_FIXED") != NULL);    // the Arithmetic and Aliasing settings
    nl3_voices_set_clean(&voices, getenv("NL3_CLEAN") != NULL);
    nl3_voices_set_limit(&voices, 16);
    nl3_voices_note_on(&voices, &sound, (uint8_t)atoi(argv[4]), (argc > 6) ? (float)(atof(argv[6]) / 127.0) : 1.0f);

    // NL3_CHORD="55,60,64": more notes with the first
    if (getenv("NL3_CHORD") != NULL) {
        char chord[256];

        snprintf(chord, sizeof(chord), "%s", getenv("NL3_CHORD"));

        for (char * n = strtok(chord, ","); n != NULL; n = strtok(NULL, ",")) {
            nl3_voices_note_on(&voices, &sound, (uint8_t)atoi(n), (argc > 6) ? (float)(atof(argv[6]) / 127.0) : 1.0f);
        }
    }
    uint32_t          gate   = (argc > 7) ? (uint32_t)(atof(argv[7]) * RATE) : frames;
    uint32_t          second = (argc > 10) ? (uint32_t)(atof(argv[10]) * RATE) : frames;

    for (uint32_t f = 0; f < frames; f += BLOCK) {
        // arguments 9 and 10: a second note and when it starts
        if ((argc > 10) && (f <= second) && (second < f + BLOCK)) {
            nl3_voices_note_on(&voices, &sound, (uint8_t)atoi(argv[9]), 1.0f);
        }

        if ((f <= gate) && (gate < f + BLOCK)) {
            nl3_voices_note_off(&voices, &sound, (uint8_t)atoi(argv[4]));
        }
        nl3_voices_render(&voices, &sound, out + f, BLOCK, nl3_volume_gain(values[kNl3ParamLevel]));
    }

    FILE *            w      = fopen(argv[3], "wb");

    fwrite("RIFF", 1, 4, w);
    put32(w, 36 + (frames * 4));
    fwrite("WAVEfmt ", 1, 8, w);
    put32(w, 16);
    put16(w, 3);
    put16(w, 1);
    put32(w, RATE);
    put32(w, RATE * 4);
    put16(w, 4);
    put16(w, 32);
    fwrite("data", 1, 4, w);
    put32(w, frames * 4);
    fwrite(out, 4, frames, w);
    fclose(w);
    printf("%s: %d of %d, type %u slope %u cutoff %.0f res %.0f\n", name, index, count, sound.filterType, sound.filterSlope, sound.cutoff, sound.resonance);
    return 0;
}
