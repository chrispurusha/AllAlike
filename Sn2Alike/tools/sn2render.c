// Renders a program through the engine: sn2render <file.syx> <index> <out.wav> <note> <seconds>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sn2Patch.h"
#include "sn2Params.h"
#include "sn2Effects.h"
#include "sn2Voices.h"

#define RATE     (39062)
#define BLOCK    (256)

static uint8_t gPrograms[SN2_MAX_SINGLES][SN2_SINGLE_LENGTH];

static void put32(FILE * f, uint32_t v) {
    fwrite(&v, 4, 1, f);
}

static void put16(FILE * f, uint16_t v) {
    fwrite(&v, 2, 1, f);
}

static void fxWrite(const char * path, const float * l, const float * r, uint32_t frames) {
    FILE * w = fopen(path, "wb");

    fwrite("RIFF", 1, 4, w);
    put32(w, 36 + (frames * 8));
    fwrite("WAVEfmt ", 1, 8, w);
    put32(w, 16);
    put16(w, 3);
    put16(w, 2);
    put32(w, RATE);
    put32(w, RATE * 8);
    put16(w, 8);
    put16(w, 32);
    fwrite("data", 1, 4, w);
    put32(w, frames * 8);

    for (uint32_t i = 0; i < frames; i++) {
        fwrite(&l[i], 4, 1, w);
        fwrite(&r[i], 4, 1, w);
    }

    fclose(w);
}

int main(int argc, char ** argv) {
    if (argc < 6) {
        return 1;
    }
    int               count = sn2_patch_read_file(argv[1], gPrograms, SN2_MAX_SINGLES);
    int               index = atoi(argv[2]);

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
    tSn2Sound         sound = {
        .osc            = {
            OSC_SOUND(values, kSn2ParamOscShape,  kSn2ParamPulseWidth, kSn2ParamOscSoften,  kSn2ParamOscLevel,  kSn2ParamOsc1Octave, kSn2ParamOsc1Semitone, kSn2ParamOsc1Fine),
            OSC_SOUND(values, kSn2ParamOsc2Shape, kSn2ParamOsc2Width,  kSn2ParamOsc2Soften, kSn2ParamOsc2Level, kSn2ParamOsc2Octave, kSn2ParamOsc2Semitone, kSn2ParamOsc2Fine),
            OSC_SOUND(values, kSn2ParamOsc3Shape, kSn2ParamOsc3Width,  kSn2ParamOsc3Soften, kSn2ParamOsc3Level, kSn2ParamOsc3Octave, kSn2ParamOsc3Semitone, kSn2ParamOsc3Fine),
        },
        .cutoff         = values[kSn2ParamCutoff] * 127.0,
        .resonance      = values[kSn2ParamResonance] * 127.0,
        .tracking       = values[kSn2ParamFilterTrack],
        .overdrive      = values[kSn2ParamFilterDrive] * 127.0,
        .normalise      = values[kSn2ParamFilterQ] * 127.0,
        .oscsFree       = values[kSn2ParamOscTrigger] > 0.5,
        .glideTime      = values[kSn2ParamGlideTime] * 127.0,
        .glideStyle     = (int)lround(values[kSn2ParamGlideStyle]),
        .glideCurve     = (int)lround(values[kSn2ParamGlideCurve]),
        .glideType      = (int)lround(values[kSn2ParamGlideType] * 9.0),
        .ampVelocity    = values[kSn2ParamAmpVelocity] * 127.0,
        .filterType     = (uint32_t)lround(values[kSn2ParamFilterType] * 11.0),
        .filterSlope    = (uint32_t)lround(values[kSn2ParamFilterSlope] * 2.0),
        .ampAttack      = values[kSn2ParamAmpAttack] * 127.0,
        .ampDecay       = values[kSn2ParamAmpDecay] * 127.0,
        .ampSustain     = values[kSn2ParamAmpSustain] * 127.0,
        .ampRelease     = values[kSn2ParamAmpRelease] * 127.0,
        .ampSustainTime = values[kSn2ParamAmpSustainTime] * 127.0,
        .ampSustainRate = values[kSn2ParamAmpSustainRate] * 127.0,
        .noiseLevel     = values[kSn2ParamNoiseLevel] * 127.0 / 128.0,
        .noiseSoften    = values[kSn2ParamNoiseSoften] * 127.0,
        .ring13         = values[kSn2ParamRing13] * 127.0 / 128.0,
        .ring23         = values[kSn2ParamRing23] * 127.0 / 128.0,
        .env            = {MOD_ENV_SOUND(values, kSn2ParamEnv2Attack, kSn2ParamEnv2Decay, kSn2ParamEnv2Sustain, kSn2ParamEnv2Release, kSn2ParamEnv2Velocity, kSn2ParamEnv2Cutoff, kSn2ParamEnv2SustainTime, kSn2ParamEnv2SustainRate), MOD_ENV_SOUND(values, kSn2ParamEnv3Attack, kSn2ParamEnv3Decay, kSn2ParamEnv3Sustain, kSn2ParamEnv3Release, kSn2ParamEnv3Velocity, kSn2ParamEnv3Cutoff, kSn2ParamEnv3SustainTime, kSn2ParamEnv3SustainRate)},
    };
    uint32_t          frames = (uint32_t)(atof(argv[5]) * RATE);
    float *           out    = calloc(frames + BLOCK, sizeof(float));
    sn2_controls_sound(values, &sound);
    static tSn2Voices voices;

    sn2_voices_init(&voices);
    sn2_voices_set_limit(&voices, 16);
    sn2_voices_note_on(&voices, &sound, (uint8_t)atoi(argv[4]), (argc > 6) ? (float)(atof(argv[6]) / 127.0) : 1.0f);

    // SN2_CHORD="55,60,64": more notes with the first
    if (getenv("SN2_CHORD") != NULL) {
        char chord[256];

        snprintf(chord, sizeof(chord), "%s", getenv("SN2_CHORD"));

        for (char * n = strtok(chord, ","); n != NULL; n = strtok(NULL, ",")) {
            sn2_voices_note_on(&voices, &sound, (uint8_t)atoi(n), (argc > 6) ? (float)(atof(argv[6]) / 127.0) : 1.0f);
        }
    }
    uint32_t          gate   = (argc > 7) ? (uint32_t)(atof(argv[7]) * RATE) : frames;
    uint32_t          second = (argc > 10) ? (uint32_t)(atof(argv[10]) * RATE) : frames;

    for (uint32_t f = 0; f < frames; f += BLOCK) {
        // arguments 9 and 10: a second note and when it starts
        if ((argc > 10) && (f <= second) && (second < f + BLOCK)) {
            sn2_voices_note_on(&voices, &sound, (uint8_t)atoi(argv[9]), 1.0f);
        }

        if ((f <= gate) && (gate < f + BLOCK)) {
            sn2_voices_note_off(&voices, &sound, (uint8_t)atoi(argv[4]));
        }
        sn2_voices_render(&voices, &sound, out + f, BLOCK, sn2_volume_gain(values[kSn2ParamVolume]));
    }

    // "fx" as argument 8: through the effects, as left then right interleaved
    if ((argc > 8) && (strcmp(argv[8], "fx") == 0)) {
        static tSn2Effects e;
        tSn2EffectsSound   es = sn2_effects_sound(values, 122.0);
        float *            l = calloc(frames, sizeof(float)), * r = calloc(frames, sizeof(float));

        sn2_effects_reset(&e);
        sn2_effects_process(&e, &es, out, l, r, frames);
        fxWrite(argv[3], l, r, frames);
        return 0;
    }
    FILE * w = fopen(argv[3], "wb");

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
