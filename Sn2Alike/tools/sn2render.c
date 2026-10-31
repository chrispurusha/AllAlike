// Renders a program through the engine: sn2render <file.syx> <index> <out.wav> <note> <seconds>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sn2Patch.h"
#include "sn2Voices.h"

#define RATE     (48000)
#define BLOCK    (256)

static uint8_t gPrograms[SN2_MAX_SINGLES][SN2_SINGLE_LENGTH];

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
    int               count  = sn2_patch_read_file(argv[1], gPrograms, SN2_MAX_SINGLES);
    int               index  = atoi(argv[2]);

    if (index >= count) {
        fprintf(stderr, "%d programs\n", count);
        return 1;
    }
    double            values[kSn2ParamCount];
    char              name[SN2_SINGLE_NAME_LENGTH + 1];

    for (uint32_t i = 0; i < kSn2ParamCount; i++) {
        values[i] = gSn2Params[i].defaultNormalized;
    }

    sn2_patch_from_single(gPrograms[index], values);
    sn2_patch_name(gPrograms[index], name);
    tSn2Sound         sound  = {
        .shape          = (tSn2OscShape)lround(values[kSn2ParamOscShape] * (eTiOscShapes - 1)),
        .pulseWidth     =                           0.05 + (0.9 * values[kSn2ParamPulseWidth]),
        .cutoff         = values[kSn2ParamCutoff] * 127.0,
        .resonance      = values[kSn2ParamResonance] * 127.0,
        .filterType     = (uint32_t)lround(values[kSn2ParamFilterType] * 11.0),
        .filterSlope    = (uint32_t)lround(values[kSn2ParamFilterSlope] * 2.0),
        .ampAttack      = values[kSn2ParamAmpAttack] * 127.0,
        .ampDecay       = values[kSn2ParamAmpDecay] * 127.0,
        .ampSustain     = values[kSn2ParamAmpSustain] * 127.0,
        .ampSustainTime = values[kSn2ParamAmpSustainTime] * 127.0,
        .ampRelease     = values[kSn2ParamAmpRelease] * 127.0,
    };
    uint32_t          frames = (uint32_t)(atof(argv[5]) * RATE);
    float *           out    = calloc(frames + BLOCK, sizeof(float));
    static tSn2Voices voices;

    sn2_voices_init(&voices, RATE);
    sn2_voices_set_limit(&voices, 16);
    sn2_voices_note_on(&voices, (uint8_t)atoi(argv[4]), 1.0f);

    for (uint32_t f = 0; f < frames; f += BLOCK) {
        sn2_voices_render(&voices, &sound, out + f, BLOCK, 1.0);
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
