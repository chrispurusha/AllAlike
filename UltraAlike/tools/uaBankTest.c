// uaBankTest - plays a bank's preset through the bank voices: uaBankTest <bank> <preset from 0> <out.wav> [notes...]
// Checks each zone's pitch step against its root key and tuning, then writes a WAV of the notes one after another.
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "emuBank.h"
#include "uaBankVoice.h"

#define RATE    (48000.0)

// the bank voices never stretch; uaCache.c names this, uaStretch.cpp (Rubber Band) is left out of the test
bool ua_stretch_render(const float * left, const float * right, uint32_t inFrames, double inRate, double outRate, double pitchScale, float * outLeft,
                       float * outRight, uint32_t outFrames, bool (*cancelled)(void * user), void * user) {
    (void)left; (void)right; (void)inFrames; (void)inRate; (void)outRate; (void)pitchScale; (void)outLeft; (void)outRight; (void)outFrames;
    (void)cancelled; (void)user;
    return false;
}

static void put32(FILE * f, uint32_t v) { fwrite(&v, 4, 1, f); }
static void put16(FILE * f, uint16_t v) { fwrite(&v, 2, 1, f); }

int main(int argc, char ** argv) {
    char       error[256];
    tEmuBank * bank = (argc >= 4) ? emu_bank_open(argv[1], error, sizeof(error)) : NULL;

    if (bank == NULL) {
        printf("usage: uaBankTest <bank> <preset> <out.wav> [notes]%s%s\n", (argc >= 4) ? " - " : "", (argc >= 4) ? error : "");
        return 1;
    }
    static tEmuPreset     preset;
    static tUaEosEnvelope envs[EMU_BANK_MAX_ZONES];
    uint32_t              index = (uint32_t)atoi(argv[2]);

    if (!emu_bank_preset(bank, index, &preset)) {
        printf("FAIL  preset %u has no playable zones\n", index);
        return 1;
    }

    for (uint32_t z = 0; z < preset.zoneCount; z++) {
        ua_eos_from_bank(&preset.zone[z].amp, &envs[z]);
    }
    tUaPlay play;

    memset(&play, 0, sizeof(play));
    play.keyTrack = true; play.gate = true; play.gain = 1.0; play.end = 1.0; play.stretch = 1.0; play.antiAlias = true;
    play.bend = 1.0; play.voiceLimit = UA_BANK_VOICES; play.velocity = 0.0; play.depth = 16;
    play.eosEnv = envs[0];   // the dials where the preset sets them: no edit (uaBankVoice notes §6)

    int    notes[16], count = 0;

    for (int a = 4; (a < argc) && (count < 16); a++) {
        notes[count++] = atoi(argv[a]);
    }

    if (count == 0) {
        notes[count++] = preset.zone[0].rootNote;
    }
    static tUaBankVoices v;
    uint32_t             each = (uint32_t)(1.5 * RATE), total = each * (uint32_t)count;
    float *              L = calloc(total, sizeof(float)), * R = calloc(total, sizeof(float));
    int                  wrong = 0;

    ua_bank_voices_init(&v, RATE);

    for (int n = 0; n < count; n++) {
        ua_bank_voices_note_on(&v, &preset, envs, &envs[0], &play, (uint8_t)notes[n], 1.0f);

        for (int k = 0; k < UA_BANK_VOICES; k++) {
            if (!v.voice[k].active || (v.voice[k].note != notes[n])) {
                continue;
            }
            const tEmuSample * s = v.voice[k].sample;
            const tEmuZone *   zone = NULL;

            for (uint32_t z = 0; z < preset.zoneCount; z++) {
                if ((preset.zone[z].sample == s) && (notes[n] >= preset.zone[z].keyLow) && (notes[n] <= preset.zone[z].keyHigh)) {
                    zone = &preset.zone[z];
                }
            }
            double want = (zone != NULL) ? (s->rate / RATE) * pow(2.0, ((notes[n] - zone->rootNote) + zone->tune) / 12.0) : 0.0;

            wrong += (fabs(v.voice[k].step - want) > 1e-9);
            printf("note %d: sample %u (%u ch, %.1f Hz), root %d, tune %+.3f -> step %.6f, expected %.6f\n", notes[n], s->number, s->channels,
                   s->rate, (zone != NULL) ? zone->rootNote : -1, (zone != NULL) ? zone->tune : 0.0, v.voice[k].step, want);
        }
        ua_bank_voices_render(&v, &play, L + (n * each), R + (n * each), each * 2 / 3);
        ua_bank_voices_note_off(&v, (uint8_t)notes[n], false);
        ua_bank_voices_render(&v, &play, L + (n * each) + (each * 2 / 3), R + (n * each) + (each * 2 / 3), each - (each * 2 / 3));
    }
    FILE * f = fopen(argv[3], "wb");
    float  peak = 0.0f;

    fwrite("RIFF", 1, 4, f); put32(f, 36 + (total * 4)); fwrite("WAVEfmt ", 1, 8, f); put32(f, 16); put16(f, 1); put16(f, 2);
    put32(f, (uint32_t)RATE); put32(f, (uint32_t)RATE * 4); put16(f, 4); put16(f, 16); fwrite("data", 1, 4, f); put32(f, total * 4);

    for (uint32_t i = 0; i < total; i++) {
        int16_t l = (int16_t)lrintf(fmaxf(-1.0f, fminf(1.0f, L[i])) * 32767.0f), r = (int16_t)lrintf(fmaxf(-1.0f, fminf(1.0f, R[i])) * 32767.0f);

        put16(f, (uint16_t)l); put16(f, (uint16_t)r);
        peak = fmaxf(peak, fmaxf(fabsf(L[i]), fabsf(R[i])));
    }
    fclose(f);
    printf("%s  '%s': %u zones, %d note(s), peak %.3f, pitch steps %s\n", wrong ? "FAIL" : "PASS", emu_bank_preset_name(bank, index), preset.zoneCount, count, peak,
           wrong ? "WRONG" : "as the zones say");
    emu_bank_free(bank);
    return wrong ? 1 : 0;
}
