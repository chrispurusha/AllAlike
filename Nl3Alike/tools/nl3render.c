// Renders a program through the engine: nl3render <file.syx> <index> <out.wav> <note> <seconds>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "nl3Patch.h"
#include "nl3Params.h"
#include "nl3Effects.h"
#include "nl3Voices.h"

#define RATE     (39062)
#define BLOCK    (256)

static uint8_t gPrograms[NL3_MAX_SINGLES][NL3_SINGLE_LENGTH];

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
    int               count = nl3_patch_read_file(argv[1], gPrograms, NL3_MAX_SINGLES);
    int               index = atoi(argv[2]);

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
    tNl3Sound         sound = {
        .osc         = {
            OSC_SOUND(values, kNl3ParamOscShape,  kNl3ParamPulseWidth, kNl3ParamOscSoften,  kNl3ParamOscLevel,  kNl3ParamOsc1Octave, kNl3ParamOsc1Semitone, kNl3ParamOsc1Fine),
            OSC_SOUND(values, kNl3ParamOsc2Shape, kNl3ParamOsc2Width,  kNl3ParamOsc2Soften, kNl3ParamOsc2Level, kNl3ParamOsc2Octave, kNl3ParamOsc2Semitone, kNl3ParamOsc2Fine),
            OSC_SOUND(values, kNl3ParamOsc3Shape, kNl3ParamOsc3Width,  kNl3ParamOsc3Soften, kNl3ParamOsc3Level, kNl3ParamOsc3Octave, kNl3ParamOsc3Semitone, kNl3ParamOsc3Fine),
        },
        .cutoff      = values[kNl3ParamCutoff] * 127.0,
        .resonance   = values[kNl3ParamResonance] * 127.0,
        .tracking    = values[kNl3ParamFilterTrack],
        .overdrive   = values[kNl3ParamFilterDrive] * 127.0,
        .normalise   = values[kNl3ParamFilterQ] * 127.0,
        .oscsFree    = values[kNl3ParamOscTrigger] > 0.5,
        .glideTime   = values[kNl3ParamGlideTime] * 127.0,
        .glideStyle  = (int)lround(values[kNl3ParamGlideStyle]),
        .glideCurve  = (int)lround(values[kNl3ParamGlideCurve]),
        .glideType   = (int)lround(values[kNl3ParamGlideType] * 9.0),
        .ampVelocity = values[kNl3ParamAmpVelocity] * 127.0,
        .filterType  = (uint32_t)lround(values[kNl3ParamFilterType] * 11.0),
        .filterSlope = (uint32_t)lround(values[kNl3ParamFilterSlope] * 2.0),
        .ampAttack   = values[kNl3ParamAmpAttack] * 127.0,
        .ampDecay    = values[kNl3ParamAmpDecay] * 127.0,
        .ampSustain  = values[kNl3ParamAmpSustain] * 127.0,
        .ampRelease  = values[kNl3ParamAmpRelease] * 127.0,
        .noiseLevel  = values[kNl3ParamNoiseLevel] * 127.0 / 128.0,
        .noiseSoften = values[kNl3ParamNoiseSoften] * 127.0,
        .ring13      = values[kNl3ParamRing13] * 127.0 / 128.0,
        .ring23      = values[kNl3ParamRing23] * 127.0 / 128.0,
        .env         = {MOD_ENV_SOUND(values, kNl3ParamEnv2Attack, kNl3ParamEnv2Decay, kNl3ParamEnv2Sustain, kNl3ParamEnv2Release, kNl3ParamEnv2Velocity, kNl3ParamEnv2Cutoff), MOD_ENV_SOUND(values, kNl3ParamEnv3Attack, kNl3ParamEnv3Decay, kNl3ParamEnv3Sustain, kNl3ParamEnv3Release, kNl3ParamEnv3Velocity, kNl3ParamEnv3Cutoff)},
    };
    uint32_t          frames = (uint32_t)(atof(argv[5]) * RATE);
    float *           out    = calloc(frames + BLOCK, sizeof(float));
    nl3_controls_sound(values, &sound);
    static tNl3Voices voices;

    nl3_voices_init(&voices);
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
        nl3_voices_render(&voices, &sound, out + f, BLOCK, nl3_volume_gain(values[kNl3ParamVolume]));
    }

    // "fx" as argument 8: through the effects, as left then right interleaved
    if ((argc > 8) && (strcmp(argv[8], "fx") == 0)) {
        static tNl3Effects e;
        tNl3EffectsSound   es = nl3_effects_sound(values, 122.0);
        float *            l = calloc(frames, sizeof(float)), * r = calloc(frames, sizeof(float));

        nl3_effects_reset(&e);
        nl3_effects_process(&e, &es, out, l, r, frames);
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
