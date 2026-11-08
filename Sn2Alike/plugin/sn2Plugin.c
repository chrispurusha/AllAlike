/*
 * Sn2Alike - a reproduction inspired by the Novation Supernova II, as a VST3 and Audio Unit
 * instrument.
 *
 * Copyright (C) 2026 Chris Turner <chris_purusha@icloud.com>
 *
 * This program is free software: you can redistribute it and/or modify it under the terms of the
 * GNU General Public License as published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without
 * even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with this program. If
 * not, see <https://www.gnu.org/licenses/>.
 */
// Notes: Docs/code-notes/sn2Plugin.md - "// notes §k" refers there.

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "synthlibPlugin.h"
#include "synthlibBackdoor.h"
#include "synthlibResample.h"
#include "synthlibLog.h"
#include "synthlibSwapFade.h"
#include "synthlibPanelView.h"
#include "synthlibTypes.h"
#include "inputState.h"
#include "sn2Draw.h"
#include "sn2Params.h"
#include "sn2Patch.h"
#include "sn2Prefs.h"
#include "sn2Hardware.h"
#include "sn2Effects.h"
#include "sn2Voices.h"

// touch /tmp/sn2alike-log, read /tmp/sn2alike.log - see SynthLib's plugin/synthlibLog.h.

#ifndef SN2_VERSION_STRING
#define SN2_VERSION_STRING    "0.1.0"
#endif

#ifndef SN2_AU_VERSION
#define SN2_AU_VERSION    (0x00000100)
#endif

// THESE MAY NEVER CHANGE: a saved project finds the plug-in by them.
static const uint8_t gProcessorUid[16]  = {0xA3, 0xAF, 0x37, 0xA1, 0x94, 0xDF, 0x43, 0xD3, 0xB9, 0x53, 0xDF, 0x1E, 0x87, 0x62, 0xF5, 0xA2};
static const uint8_t gControllerUid[16] = {0x2B, 0xE4, 0x1E, 0xF9, 0xA5, 0x0F, 0x4C, 0xF3, 0xAF, 0x46, 0xF5, 0x6D, 0x8F, 0x77, 0x8D, 0x53};

// THESE MUST MATCH do-plugin - the bundle identifier especially
#define FOUR_CC(a, b, c, d)    (((uint32_t)(a) << 24) | ((uint32_t)(b) << 16) | ((uint32_t)(c) << 8) | (uint32_t)(d))
#define SN2_AU_TYPE            FOUR_CC('a', 'u', 'm', 'u')
#define SN2_AU_SUBTYPE         FOUR_CC('S', 'n', '2', 'a')
#define SN2_AU_MANUFACTURER    FOUR_CC('C', 'P', 'u', 'r')
#define SN2_AU_BUNDLE_ID       "com.chrispurusha.sn2alike.au"

// notes §2: a block's notes, played at their own sample when the block renders
#define MAX_EVENTS             (256)

#define DEFAULT_TEMPO          (120.0) // notes §13: until the host gives one
#define CLIP_HOLD_S            (1.5)
#define HOST_CEILING           (0.97675f)
#define OUTPUT_HEADROOM        (0.5f)  // notes §16: 6 dB under the instrument's level, after its ceiling
#define HOST_CHUNK             (256u)
#define ENGINE_FRAMES          (4096u) // notes §12: HOST_CHUNK at a host rate down to 2.5 kHz

typedef struct {
    uint32_t offset;
    uint8_t  note;
    bool     on;
    float    velocity;
} tSn2Event;

typedef struct {
    tSn2Voices         voices;
    tSynthLibResampler resampler;  // notes §12
    tSynthLibResampler resamplerR;
    tSn2Effects        effects;
    double             hostRate;
    double             tempo;
    float              engine[ENGINE_FRAMES];
    float              engineL[ENGINE_FRAMES], engineR[ENGINE_FRAMES];
    tSn2Event          events[MAX_EVENTS];
    uint32_t           eventCount;
    double             values[kSn2ParamCount]; // normalized
    // notes §5: the sound, beyond its parameters - main thread, and the host's state calls
    char               soundName[SN2_SINGLE_NAME_LENGTH + 1];
    bool               haveBase;
    uint8_t            base[SN2_SINGLE_LENGTH]; // the Single it came from, whose bytes a save keeps
    char               patchPath[1024];
    int                patchIndex;              // its place in that file
    int                preset;                  // 1..SN2_PRESETS, 0 none
    double             presetsChecked;
    tSynthLibSwapFade  fade;
    _Atomic bool       clipped;     // the audio thread saw the ceiling; the panel holds the lamp
    double             clipUntil;
    // notes §14: the link to the instrument
    int                hwLock;
    bool               hwSent, hwExtensionDue;
    double             hwSentAt;
    uint8_t            hwLast[SN2_SINGLE_LENGTH];
    _Atomic bool       startPreset;  // notes: a new instance plays A000
    _Atomic int        wantPreset;   // a program change waiting for the fade
    _Atomic bool       clearEffects; // notes §15: a new sound starts with empty effects, the old one's echoes gone
} tSn2Plugin;

static uint32_t voice_limit(const tSn2Plugin * t) {
    return 1u + (uint32_t)lround(t->values[kSn2ParamVoiceLimit] * (SN2_MAX_VOICES - 1));
}

static void * sn2_create(const tSynthLibPluginDesc * desc) {
    tSn2Plugin * t = (tSn2Plugin *)calloc(1, sizeof(tSn2Plugin));

    (void)desc;

    if (t == NULL) {
        return NULL;
    }

    for (uint32_t i = 0; i < kSn2ParamCount; i++) {
        t->values[i] = gSn2Params[i].defaultNormalized;
    }

    t->hwLock      = -1;
    synthlib_swap_fade_init(&t->fade);
    sn2_presets_refresh();
    t->startPreset = true;

    snprintf(t->soundName, sizeof(t->soundName), "Init");
    sn2_voices_init(&t->voices);
    sn2_effects_reset(&t->effects);
    t->hostRate    = 48000.0;
    t->tempo       = DEFAULT_TEMPO;
    synthlib_resample_init(&t->resampler, SN2_ENGINE_HZ, t->hostRate);
    synthlib_resample_init(&t->resamplerR, SN2_ENGINE_HZ, t->hostRate);
    sn2_voices_set_limit(&t->voices, voice_limit(t));
    sn2_prefs_init();
    synthlib_log_line("created - Sn2Alike %s", SN2_VERSION_STRING);
    return t;
}

static void sn2_destroy(void * inst) {
    sn2_hardware_release(&((tSn2Plugin *)inst)->hwLock);
    synthlib_resample_free(&((tSn2Plugin *)inst)->resampler);
    synthlib_resample_free(&((tSn2Plugin *)inst)->resamplerR);
    free(inst);
}

// notes §5: the plug-in's own state, after the parameters
#define STATE_MAGIC     "TIS1"
#define STATE_LENGTH    (4 + (SN2_SINGLE_NAME_LENGTH + 1) + 1 + SN2_SINGLE_LENGTH + 1024 + 2 + 2)

static size_t sn2_get_state(void * inst, void * out, size_t len) {
    tSn2Plugin * t = (tSn2Plugin *)inst;
    uint8_t *    o = (uint8_t *)out;

    if ((out == NULL) || (len < STATE_LENGTH)) {
        return STATE_LENGTH;
    }
    memcpy(o, STATE_MAGIC, 4);
    o   += 4;
    memcpy(o, t->soundName, SN2_SINGLE_NAME_LENGTH + 1);
    o   += SN2_SINGLE_NAME_LENGTH + 1;
    *o++ = t->haveBase ? 1 : 0;
    memcpy(o, t->base, SN2_SINGLE_LENGTH);
    o   += SN2_SINGLE_LENGTH;
    memcpy(o, t->patchPath, 1024);
    o   += 1024;
    *o++ = (uint8_t)(t->patchIndex >> 8);
    *o++ = (uint8_t)(t->patchIndex & 0xFF);
    *o++ = (uint8_t)(t->preset >> 8);
    *o   = (uint8_t)(t->preset & 0xFF);
    return STATE_LENGTH;
}

static void sn2_set_state(void * inst, const void * data, size_t len) {
    tSn2Plugin *    t = (tSn2Plugin *)inst;
    const uint8_t * d = (const uint8_t *)data;

    if ((data == NULL) || (len < STATE_LENGTH) || (memcmp(d, STATE_MAGIC, 4) != 0)) {
        return;
    }
    atomic_store(&t->startPreset, false);   // a project's sound, not A001
    d                                   += 4;
    memcpy(t->soundName, d, SN2_SINGLE_NAME_LENGTH + 1);
    t->soundName[SN2_SINGLE_NAME_LENGTH] = '\0';
    d                                   += SN2_SINGLE_NAME_LENGTH + 1;
    t->haveBase                          = (*d++ != 0);
    memcpy(t->base, d, SN2_SINGLE_LENGTH);
    d                                   += SN2_SINGLE_LENGTH;
    memcpy(t->patchPath, d, 1024);
    t->patchPath[1023]                   = '\0';
    d                                   += 1024;
    t->patchIndex                        = (d[0] << 8) | d[1];
    t->preset                            = (d[2] << 8) | d[3];
    t->preset                            = ((t->preset >= 1) && (t->preset <= SN2_PRESETS)) ? t->preset : 0;
}

// notes §12: voices at their own rate, converted to the host's
static void render_host(tSn2Plugin * t, const tSn2Sound * sound, float * outL, float * outR, uint32_t frames, double gain) {
    tSn2EffectsSound effects = sn2_effects_sound(t->values, t->tempo);

    if (atomic_exchange(&t->clearEffects, false)) {
        sn2_effects_reset(&t->effects);   // notes §15
    }

    while (frames > 0) {
        uint32_t chunk = (frames < HOST_CHUNK) ? frames : HOST_CHUNK;
        uint32_t need  = synthlib_resample_needed(&t->resampler, chunk);

        memset(t->engine, 0, (size_t)need * sizeof(float));
        sn2_voices_render(&t->voices, sound, t->engine, need, gain);

        if (t->values[kSn2ParamEffectsBypass] > 0.5) {
            sn2_effects_bypass(&t->effects, t->engine, t->engineL, t->engineR, need);
        } else {
            sn2_effects_process(&t->effects, &effects, t->engine, t->engineL, t->engineR, need);
        }

        if (t->effects.clipped) {
            t->effects.clipped = false;
            atomic_store(&t->clipped, true);
        }
        synthlib_resample_process(&t->resampler, t->engineL, outL, chunk);
        float    discard[HOST_CHUNK];

        synthlib_resample_process(&t->resamplerR, t->engineR, (outR != NULL) ? outR : discard, chunk);

        // the resampler rounds the ceiling's corners a little past it
        for (uint32_t i = 0; i < chunk; i++) {
            outL[i] = OUTPUT_HEADROOM * fminf(HOST_CEILING, fmaxf(-HOST_CEILING, outL[i]));

            if (outR != NULL) {
                outR[i] = OUTPUT_HEADROOM * fminf(HOST_CEILING, fmaxf(-HOST_CEILING, outR[i]));
            }
        }

        outL   += chunk;
        outR    = (outR != NULL) ? (outR + chunk) : NULL;
        frames -= chunk;
    }
}

static uint32_t sn2_latency_samples(void * inst) {
    return ((tSn2Plugin *)inst)->resampler.latency;
}

static void sn2_prepare(void * inst, const tSynthLibSetup * setup) {
    tSn2Plugin * t = (tSn2Plugin *)inst;

    t->hostRate = setup->sampleRate;
    synthlib_resample_init(&t->resampler, SN2_ENGINE_HZ, t->hostRate);
    synthlib_resample_init(&t->resamplerR, SN2_ENGINE_HZ, t->hostRate);

    if (setup->offline) {
        atomic_store(&t->startPreset, false);   // an offline render plays what it is given
    }
}

static void sn2_set_active(void * inst, bool active) {
    (void)active;
    ((tSn2Plugin *)inst)->eventCount = 0;
    sn2_voices_all_off(&((tSn2Plugin *)inst)->voices);
}

static tSn2Sound make_sound(const tSn2Plugin * t) {
    tSn2Sound sound = (tSn2Sound){
        .osc         = {
            OSC_SOUND(t->values, kSn2ParamOscShape,  kSn2ParamPulseWidth, kSn2ParamOscSoften,  kSn2ParamOscLevel,  kSn2ParamOsc1Octave, kSn2ParamOsc1Semitone, kSn2ParamOsc1Fine),
            OSC_SOUND(t->values, kSn2ParamOsc2Shape, kSn2ParamOsc2Width,  kSn2ParamOsc2Soften, kSn2ParamOsc2Level, kSn2ParamOsc2Octave, kSn2ParamOsc2Semitone, kSn2ParamOsc2Fine),
            OSC_SOUND(t->values, kSn2ParamOsc3Shape, kSn2ParamOsc3Width,  kSn2ParamOsc3Soften, kSn2ParamOsc3Level, kSn2ParamOsc3Octave, kSn2ParamOsc3Semitone, kSn2ParamOsc3Fine),
        },
        .cutoff      = t->values[kSn2ParamCutoff] * 127.0,
        .resonance   = t->values[kSn2ParamResonance] * 127.0,
        .tracking    = t->values[kSn2ParamFilterTrack],
        .overdrive   = t->values[kSn2ParamFilterDrive] * 127.0,
        .normalise   = t->values[kSn2ParamFilterQ] * 127.0,
        .oscsFree    = t->values[kSn2ParamOscTrigger] > 0.5,
        .glideTime   = t->values[kSn2ParamGlideTime] * 127.0,
        .glideStyle  = (int)lround(t->values[kSn2ParamGlideStyle]),
        .glideCurve  = (int)lround(t->values[kSn2ParamGlideCurve]),
        .glideType   = (int)lround(t->values[kSn2ParamGlideType] * 9.0),
        .ampVelocity = t->values[kSn2ParamAmpVelocity] * 127.0,
        .filterType  = (uint32_t)lround(t->values[kSn2ParamFilterType] * 11.0),
        .filterSlope = (uint32_t)lround(t->values[kSn2ParamFilterSlope] * 2.0),
        .ampAttack   = t->values[kSn2ParamAmpAttack] * 127.0,
        .ampDecay    = t->values[kSn2ParamAmpDecay] * 127.0,
        .ampSustain  = t->values[kSn2ParamAmpSustain] * 127.0,
        .ampRelease  = t->values[kSn2ParamAmpRelease] * 127.0,
        .noiseLevel  = t->values[kSn2ParamNoiseLevel] * 127.0 / 128.0,
        .noiseSoften = t->values[kSn2ParamNoiseSoften] * 127.0,
        .ring13      = t->values[kSn2ParamRing13] * 127.0 / 128.0,
        .ring23      = t->values[kSn2ParamRing23] * 127.0 / 128.0,
        .env         = {MOD_ENV_SOUND(t->values, kSn2ParamEnv2Attack, kSn2ParamEnv2Decay, kSn2ParamEnv2Sustain, kSn2ParamEnv2Release, kSn2ParamEnv2Velocity, kSn2ParamEnv2Cutoff), MOD_ENV_SOUND(t->values, kSn2ParamEnv3Attack, kSn2ParamEnv3Decay, kSn2ParamEnv3Sustain, kSn2ParamEnv3Release, kSn2ParamEnv3Velocity, kSn2ParamEnv3Cutoff)},
    };

    sn2_controls_sound(t->values, &sound);
    return sound;
}

static void play(tSn2Plugin * t, const tSn2Sound * sound, const tSn2Event * e) {
    if (e->on) {
        sn2_voices_note_on(&t->voices, sound, e->note, e->velocity);
    } else {
        sn2_voices_note_off(&t->voices, sound, e->note);
    }
}

// notes §2: a full queue plays the note at once rather than lose it
static void queue(tSn2Plugin * t, tSn2Event e) {
    if (t->eventCount < MAX_EVENTS) {
        t->events[t->eventCount++] = e;
    } else {
        tSn2Sound sound = make_sound(t);

        play(t, &sound, &e);
    }
}

static void sn2_note_on(void * inst, uint8_t channel, uint8_t note, float velocity, uint32_t sampleOffset) {
    tSn2Plugin * t = (tSn2Plugin *)inst;

    (void)t;   // notes §14: the instrument takes its notes from the host, not from Link
    (void)channel;
    queue((tSn2Plugin *)inst, (tSn2Event){sampleOffset, note, true, velocity});
}

static void sn2_note_off(void * inst, uint8_t channel, uint8_t note, float velocity, uint32_t sampleOffset) {
    tSn2Plugin * t = (tSn2Plugin *)inst;

    (void)t;
    (void)channel;
    (void)velocity;
    queue((tSn2Plugin *)inst, (tSn2Event){sampleOffset, note, false, 0.0f});
}

static void sn2_load_single(void * user, const uint8_t * single, const char * path, int index, int preset);

// A new instance plays A000, first audio block or first editor frame; with no presets it stays on Init
static void take_start_preset(tSn2Plugin * t) {
    if (atomic_exchange(&t->startPreset, false) && (sn2_presets_single(1)[0] == 0xF0)) {
        sn2_load_single(t, sn2_presets_single(1), "", 0, 1);
    }
}

static void take_program_change(tSn2Plugin * t) {
    int number = atomic_load(&t->wantPreset);

    if ((number != 0) && synthlib_swap_fade_silent(&t->fade)) {
        atomic_store(&t->wantPreset, 0);
        sn2_load_single(t, sn2_presets_single(number), "", 0, number);
        synthlib_swap_fade_end(&t->fade);
    }
}

static void sn2_process(void * inst, const float *const * in, uint32_t numIn, float ** out, uint32_t numOut,
                        uint32_t frames, const tSynthLibTransport * transport) {
    tSn2Plugin * t      = (tSn2Plugin *)inst;

    take_start_preset(t);
    take_program_change(t);
    double       gain   = sn2_volume_gain(t->values[kSn2ParamVolume]);
    double       master = t->values[kSn2ParamMaster];
    tSn2Sound    sound  = make_sound(t);

    (void)in;
    (void)numIn;

    if ((transport != NULL) && transport->tempoValid && (transport->tempo > 0.0)) {
        t->tempo = transport->tempo;    // notes §13: otherwise the last the host gave
    }

    if ((out == NULL) || (numOut == 0) || (out[0] == NULL)) {
        for (uint32_t i = 0; i < t->eventCount; i++) {
            play(t, &sound, &t->events[i]);
        }

        t->eventCount = 0;
        return;
    }
    float *      left   = out[0];
    float *      right  = (numOut > 1) ? out[1] : NULL;

    // notes §2: up to each note, then the note; the host sends them in time order
    uint32_t     done   = 0;

    for (uint32_t i = 0; i < t->eventCount; i++) {
        uint32_t at = (t->events[i].offset < frames) ? t->events[i].offset : frames;

        if (at > done) {
            render_host(t, &sound, left + done, (right != NULL) ? (right + done) : NULL, at - done, gain);
            done = at;
        }
        play(t, &sound, &t->events[i]);
    }

    t->eventCount = 0;

    if (done < frames) {
        render_host(t, &sound, left + done, (right != NULL) ? (right + done) : NULL, frames - done, gain);
    }

    // after the ceiling: it turns the output down and never saves the sound from it
    for (uint32_t f = 0; f < frames; f++) {
        left[f] *= (float)master;

        if (right != NULL) {
            right[f] *= (float)master;
        }
    }

    synthlib_swap_fade_apply(&t->fade, left, right, frames, t->hostRate);

    for (uint32_t ch = 2; ch < numOut; ch++) {
        if (out[ch] != NULL) {
            memcpy(out[ch], left, (size_t)frames * sizeof(float));
        }
    }
}

static void sn2_set_param(void * inst, uint32_t id, double normalized) {
    tSn2Plugin * t = (tSn2Plugin *)inst;

    if (id >= kSn2ParamCount) {
        return;
    }
    t->values[id] = normalized;

    if (id == kSn2ParamProgram) {
        int bank   = (int)lround(t->values[kSn2ParamBank] * 127.0) % (SN2_PRESETS / SN2_BANK_SIZE);
        int number = (bank * SN2_BANK_SIZE) + (int)lround(normalized * 127.0) + 1;

        // already on it, nothing happens, edits and all
        if ((number != t->preset) && (sn2_presets_single(number)[0] == 0xF0)) {
            atomic_store(&t->wantPreset, number);
            synthlib_swap_fade_start(&t->fade);
        }
    } else if (id == kSn2ParamVoiceLimit) {
        sn2_voices_set_limit(&t->voices, voice_limit(t));
    }
}

static double sn2_get_param(void * inst, uint32_t id) {
    return (id < kSn2ParamCount) ? ((tSn2Plugin *)inst)->values[id] : 0.0;
}

static bool sn2_param_text(const tSynthLibPluginDesc * desc, void * inst, uint32_t id, double value, char * out, size_t len) {
    static const char *const shapes[]  = {"Saw", "Square", "Double Saw"};
    static const char *const filters[] = {"LPF", "BPF", "HPF", "Res LPF", "Res BPF", "Res HPF", "Notch", "LPF+LPF", "BPF+BPF", "HPF+HPF", "LPF+BPF", "BPF+HPF"};
    static const char *const slopes[]  = {"12 dB", "18 dB", "24 dB"};

    (void)desc;
    (void)inst;

    if (id >= kSn2ParamCount) {
        return false;
    }
    long                     step      = lround(value * gSn2Params[id].stepCount);

    if ((id == kSn2ParamOscShape) || (id == kSn2ParamOsc2Shape) || (id == kSn2ParamOsc3Shape)) {
        snprintf(out, len, "%s", shapes[(step < 0) ? 0 : ((step > 2) ? 2 : step)]);
    } else if (id == kSn2ParamFilterType) {
        snprintf(out, len, "%s", filters[(step < 0) ? 0 : ((step > 11) ? 11 : step)]);
    } else if (id == kSn2ParamChorusType) {
        static const char *const types[] = {"Quad Chorus", "Chorus/Flanger", "Phaser"};

        snprintf(out, len, "%s", types[(step < 0) ? 0 : ((step > 2) ? 2 : step)]);
    } else if (id == kSn2ParamGlideType) {
        static const char *const types[] = {"Normal", "Auto", "2 Semi Down", "2 Semi Up", "5 Semi Down", "5 Semi Up", "7 Semi Down", "7 Semi Up", "12 Semi Down", "12 Semi Up"};

        snprintf(out, len, "%s", types[(step < 0) ? 0 : ((step > 9) ? 9 : step)]);
    } else if (id == kSn2ParamGlideStyle) {
        snprintf(out, len, "%s", (step > 0) ? "Glissando" : "Portamento");
    } else if (id == kSn2ParamGlideCurve) {
        snprintf(out, len, "%s", (step > 0) ? "Exponential" : "Linear");
    } else if (id == kSn2ParamReverbType) {
        static const char *const types[] = {"Gated Reverse", "Gated Rising", "Gated Gentle", "Gated Falling", "Dry Chamber", "Echo Chamber", "Small Room", "Big Room", "Medium 1", "Medium 2", "Medium Plate 1", "Medium Plate 2", "Large 1", "Large 2", "Large Plate 1", "Large Plate 2"};

        snprintf(out, len, "%s", types[(step < 0) ? 0 : ((step > 15) ? 15 : step)]);
    } else if (id == kSn2ParamEffectsOrder) {
        static const char *const orders[] = {"D+R+C", "D>R>C", "D>C>R", "R>D>C", "R>C>D", "C>D>R", "C>R>D", "D+[R>C]", "D+[C>R]", "R+[D>C]", "R+[C>D]", "C+[D>R]", "C+[R>D]", "D>[R+C]", "R>[D+C]", "C>[D+R]", "[R+C]>D", "[D+C]>R", "[D+R]>C"};

        snprintf(out, len, "%s", orders[(step < 0) ? 0 : ((step > 18) ? 18 : step)]);
    } else if (id == kSn2ParamOscTrigger) {
        snprintf(out, len, "%s", (step > 0) ? "Ensemble" : "Percussive");
    } else if (id == kSn2ParamDelaySync) {
        static const char *const syncs[] = {"Off", "32nd T", "32nd", "16th T", "16th", "8th T", "16th .", "8th", "4th T", "8th .", "4th", "2nd T", "4th .", "2nd", "1 Bar T", "2nd .", "1 Bar", "2 Bar T", "1 Bar .", "2 Bars", "4 Bar T", "3 Bars", "5 Bar T", "4 Bars", "3 Bar .", "7 Bar T", "5 Bars", "8 Bar T", "6 Bars", "7 Bars", "5 Bar .", "8 Bars", "6 Bar .", "7 Bar .", "8 Bar ."};

        snprintf(out, len, "%s", syncs[(step < 0) ? 0 : ((step > 34) ? 34 : step)]);
    } else if (id == kSn2ParamDelayRatio) {
        static const char *const ratios[] = {"1:1", "1:0.75", "0.75:1", "1:0.66", "0.66:1", "1:0.5", "0.5:1", "1:0.33", "0.33:1", "1:0.25", "0.25:1", "1:Off", "Off:1"};

        snprintf(out, len, "%s", ratios[(step < 0) ? 0 : ((step > 12) ? 12 : step)]);
    } else if ((id == kSn2ParamDelayOn) || (id == kSn2ParamReverbOn)) {
        snprintf(out, len, "%s", (step > 0) ? "On" : "Off");
    } else if ((id == kSn2ParamLfo1Shape) || (id == kSn2ParamLfo2Shape)) {
        static const char *const shapes[] = {"Square", "Saw", "Triangle", "S+H"};

        snprintf(out, len, "%s", shapes[(step < 0) ? 0 : ((step > 3) ? 3 : step)]);
    } else if ((id == kSn2ParamLfo1Range) || (id == kSn2ParamLfo2Range)) {
        static const char *const ranges[] = {"x1", "x10", "x100"};

        snprintf(out, len, "%s", ranges[(step < 0) ? 0 : ((step > 2) ? 2 : step)]);
    } else if ((id == kSn2ParamLfo1Trigger) || (id == kSn2ParamLfo2Trigger)) {
        snprintf(out, len, "%s", (step > 0) ? "Key" : "Free");
    } else if ((id == kSn2ParamLfo1Fade) || (id == kSn2ParamLfo2Fade)) {
        snprintf(out, len, "%s", (step > 0) ? "Out" : "In");
    } else if ((id == kSn2ParamHwLink) || (id == kSn2ParamPanEffects)) {
        snprintf(out, len, "%s", (step > 0) ? "On" : "Off");
    } else if (id == kSn2ParamHwChannel) {
        snprintf(out, len, "%ld", step + 1);
    } else if (id == kSn2ParamEffectsBypass) {
        snprintf(out, len, "%s", (step > 0) ? "Bypass" : "On");
    } else if (id == kSn2ParamPolyMode) {
        snprintf(out, len, "%s", (step > 0) ? "Mono" : "Poly");
    } else if (id == kSn2ParamEnvTrigger) {
        snprintf(out, len, "%s", (step > 0) ? "Legato" : "Retrigger");
    } else if (id == kSn2ParamFilterSlope) {
        snprintf(out, len, "%s", slopes[(step < 0) ? 0 : ((step > 2) ? 2 : step)]);
    } else {
        snprintf(out, len, "%ld", lround(gSn2Params[id].plainMin) + step);
    }
    return true;
}

// ---- the editor - notes §3: Pulse2Alike's panel view, with this plug-in's pages --------------------------------

// ---- Singles and presets - notes §5 ------------------------------------------------------------------------

// What a sound carries: everything but the plug-in's own settings
static bool sound_param(uint32_t id) {
    return (id != kSn2ParamVoiceLimit) && (id != kSn2ParamMaster) && (id != kSn2ParamEffectsBypass) && (id != kSn2ParamSustainPedal)
           && (id != kSn2ParamHwLink) && (id != kSn2ParamHwChannel) && (id != kSn2ParamHwDevice) && (id != kSn2ParamProgram) && (id != kSn2ParamBank)
           && (id != kSn2ParamPitchBend) && (id != kSn2ParamModWheel);
}

static void current_values(void * user, double values[kSn2ParamCount]) {
    for (uint32_t id = 0; id < kSn2ParamCount; id++) {
        values[id] = synthlib_plugin_param_value(user, id);
    }
}

// The sound as it is now, as a Single: the host's values over the Single it came from
static void current_single(void * user, uint8_t single[SN2_SINGLE_LENGTH]) {
    tSn2Plugin * t = (tSn2Plugin *)user;
    double       values[kSn2ParamCount];

    current_values(user, values);
    sn2_patch_to_single(values, t->haveBase ? t->base : NULL, t->soundName, single);
}

// notes §6: every parameter the sound carries, through the host, so the change is one the host sees
static void sn2_load_single(void * user, const uint8_t * single, const char * path, int index, int preset) {
    tSn2Plugin * t = (tSn2Plugin *)user;
    double       values[kSn2ParamCount];

    // the instance's own values, not the host's: asking the host takes a lock on VST3, and a program
    // change runs this on the audio thread
    memcpy(values, t->values, sizeof(values));
    sn2_patch_from_single(single, values);

    // The whole sound into the engine now, while the fade holds it silent. The host's copies come back
    // later - a block or more on VST3, and on an Audio Unit only once the main thread is free - so
    // waiting for them let the fade open on half the old sound and half the new.
    for (uint32_t id = 0; id < kSn2ParamCount; id++) {
        if (sound_param(id)) {
            t->values[id] = values[id];
            synthlib_plugin_param_edited(user, id, values[id]);
        }
    }

    atomic_store(&t->clearEffects, true);   // notes §15: the audio thread does it, at its next block
    memcpy(t->base, single, SN2_SINGLE_LENGTH);
    t->haveBase   = true;
    sn2_patch_name(single, t->soundName);
    snprintf(t->patchPath, sizeof(t->patchPath), "%s", path);
    t->patchIndex = index;
    t->preset     = preset;
}

static bool sn2_save_patch(void * user, const char * path) {
    tSn2Plugin * t = (tSn2Plugin *)user;
    uint8_t      single[SN2_SINGLE_LENGTH];

    current_single(user, single);

    if (!sn2_patch_save_file(path, single)) {
        synthlib_log_line("could not write %s", path);
        return false;
    }
    memcpy(t->base, single, SN2_SINGLE_LENGTH);
    t->haveBase   = true;
    snprintf(t->patchPath, sizeof(t->patchPath), "%s", path);
    t->patchIndex = 0;
    t->preset     = 0;
    return true;
}

static void sn2_load_faded(void * user, const uint8_t * single, const char * path, int index, int preset) {
    tSn2Plugin * t = (tSn2Plugin *)user;

    synthlib_swap_fade_begin(&t->fade);
    sn2_load_single(user, single, path, index, preset);
    synthlib_swap_fade_end(&t->fade);
    synthlib_log_line("loaded \"%s\" (%s #%d, preset %d)", t->soundName, path, index, preset);
}

static bool sn2_store_preset(void * user, int number) {
    tSn2Plugin * t = (tSn2Plugin *)user;
    uint8_t      single[SN2_SINGLE_LENGTH];

    current_single(user, single);

    if (!sn2_presets_store(number, single)) {
        return false;
    }
    memcpy(t->base, sn2_presets_single(number), SN2_SINGLE_LENGTH);
    t->haveBase     = true;
    t->patchPath[0] = '\0';
    t->patchIndex   = 0;
    t->preset       = number;
    return true;
}

// every sound parameter to its default; the next save has no Single to keep bytes from
static void sn2_init_patch(void * user) {
    tSn2Plugin * t = (tSn2Plugin *)user;

    for (uint32_t id = 0; id < kSn2ParamCount; id++) {
        if (sound_param(id)) {
            synthlib_plugin_param_edited(user, id, gSn2Params[id].defaultNormalized);
        }
    }

    t->haveBase     = false;
    t->patchPath[0] = '\0';
    t->patchIndex   = 0;
    t->preset       = 0;
    snprintf(t->soundName, sizeof(t->soundName), "Init");
}

static const char * sn2_sound_name(void * user) {
    return ((tSn2Plugin *)user)->soundName;
}

// notes §5: the name only - written to a Single at the next save or store
static void sn2_rename(void * user, const char * name) {
    snprintf(((tSn2Plugin *)user)->soundName, SN2_SINGLE_NAME_LENGTH + 1, "%s", name);
}

// notes §7: edited, if a parameter it carries or its name differs from the stored preset
static bool preset_edited(tSn2Plugin * t, void * user) {
    double values[kSn2ParamCount], stored[kSn2ParamCount];
    char   name[SN2_SINGLE_NAME_LENGTH + 1];

    if (t->preset == 0) {
        return false;
    }
    current_values(user, values);
    memcpy(stored, values, sizeof(stored));
    sn2_patch_from_single(sn2_presets_single(t->preset), stored);
    sn2_patch_name(sn2_presets_single(t->preset), name);

    for (uint32_t id = 0; id < kSn2ParamCount; id++) {
        if (sound_param(id) && (fabs(stored[id] - values[id]) > 1e-6)) {
            return true;
        }
    }

    return strcmp(name, t->soundName) != 0;
}

#define LINK_MIN_SECONDS    (0.5)    // notes §14: a whole sound, at most this often
#define LINK_EXT_SECONDS    (0.2)    // the extension after the main message

static double now_seconds(void) {
    struct timespec ts;

    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec + (ts.tv_nsec * 1e-9);
}

// notes §14: the instrument follows the sound, whole; notes and controllers come from the host
static void hardware_link(tSn2Plugin * t, void * user) {
    if (synthlib_plugin_param_value(user, kSn2ParamHwLink) < 0.5) {
        sn2_hardware_release(&t->hwLock);
        t->hwSent         = false;
        t->hwExtensionDue = false;
        sn2_draw_set_link_busy(false);
        return;
    }
    bool    busy   = !sn2_hardware_claim(&t->hwLock);

    sn2_draw_set_link_busy(busy);

    if (busy) {
        t->hwSent = false;      // all of the sound once the instrument is free
        return;
    }
    uint8_t single[SN2_SINGLE_LENGTH];
    uint8_t device = (uint8_t)lround(synthlib_plugin_param_value(user, kSn2ParamHwDevice) * 127.0);

    if (t->hwExtensionDue) {
        if ((now_seconds() - t->hwSentAt) >= LINK_EXT_SECONDS) {
            (void)sn2_hardware_send_extension(t->hwLast, sn2_hw_port(), device);
            t->hwExtensionDue = false;
        }
        return;
    }
    current_single(user, single);

    if ((!t->hwSent || (memcmp(single, t->hwLast, SN2_SINGLE_LENGTH) != 0)) && ((now_seconds() - t->hwSentAt) >= LINK_MIN_SECONDS)) {
        tSn2HardwareResult r = sn2_hardware_send_main(single, sn2_hw_port(), device);

        memcpy(t->hwLast, single, SN2_SINGLE_LENGTH);
        t->hwSentAt       = now_seconds();
        t->hwSent         = (r == eSn2HardwareSent);
        t->hwExtensionDue = t->hwSent;
        synthlib_log_line("Link: sound sent to \"%s\" (result %d)", sn2_hw_port(), (int)r);
    }
    // notes §14: bend, wheel and pedal reach the instrument from the host with the notes, not from Link
}

static const tSn2PatchHost gPatchHost = {
    .load = sn2_load_faded, .save   = sn2_save_patch, .storePreset = sn2_store_preset, .init = sn2_init_patch,
    .name = sn2_sound_name, .rename = sn2_rename,
};

// In mono the voice limit means nothing: the dial says so
static bool limit_display(uint32_t paramId, const double * values, int32_t * step, char * text, size_t len) {
    if ((paramId != kSn2ParamVoiceLimit) || (values[kSn2ParamPolyMode] <= 0.5)) {
        return false;
    }
    *step = 0;
    snprintf(text, len, "Mono");
    return true;
}

static void sn2_panel_init(void) {
    sn2_draw_init();
    sn2_draw_set_pages(gSn2Pages, gSn2PageCount);
    sn2_draw_set_host(&gPatchHost);
    sn2_draw_set_display(limit_display);
}

// notes: a plain-text channel, off until /tmp/sn2alike-backdoor exists
static double bd_get(void * user, uint32_t id) {
    return sn2_get_param(user, id);
}

static void bd_set(void * user, uint32_t id, double normalized) {
    sn2_set_param(user, id, normalized);
    synthlib_plugin_param_edited(user, id, normalized);
}

static bool bd_text(void * user, uint32_t id, double normalized, char * out, size_t len) {
    return sn2_param_text(NULL, user, id, normalized, out, len);
}

static bool bd_preset(void * user, int number) {
    if ((number < 1) || (number > SN2_PRESETS)) {
        return false;
    }
    (void)sn2_presets_refresh();
    sn2_load_faded(user, sn2_presets_single(number), "", 0, number);
    return true;
}

static const tSynthLibBackdoor gBackdoor = {
    .params = gSn2Params,     .count  = kSn2ParamCount,
    .get    = bd_get,         .set    = bd_set,        .text  = bd_text,
    .name   = sn2_sound_name, .rename = sn2_rename,
    .preset = bd_preset,      .save   = sn2_save_patch,.init  = sn2_init_patch,
};

static void sn2_panel_sync(void * user) {
    synthlib_backdoor_poll("sn2alike", &gBackdoor, user);
    tSn2Plugin * t  = (tSn2Plugin *)user;

    sn2_draw_set_user(user);

    if (atomic_exchange(&t->clipped, false)) {
        t->clipUntil = now_seconds() + CLIP_HOLD_S;
    }
    sn2_draw_set_clip(now_seconds() < t->clipUntil);
    hardware_link(t, user);
    sn2_draw_read_tick();
    int          id = sn2_hardware_identified(); // hardware notes §4

    if (id >= 0) {
        sn2_set_param(user, kSn2ParamHwDevice, (double)id / 127.0);
        synthlib_plugin_param_edited(user, kSn2ParamHwDevice, (double)id / 127.0);
    }

    // another instance's store shows within a second
    if ((now_seconds() - t->presetsChecked) >= 1.0) {
        t->presetsChecked = now_seconds();
        (void)sn2_presets_refresh();
    }
    sn2_draw_set_patch(t->patchPath, t->patchIndex);   // this instance's, every sync
    sn2_draw_set_preset(t->preset, preset_edited(t, user));

    take_start_preset((tSn2Plugin *)user);

    for (uint32_t id = 0; id < kSn2ParamCount; id++) {
        sn2_draw_set_value(id, synthlib_plugin_param_value(user, id));
    }
}

static void sn2_panel_frame(void * user, int pixelWidth, int pixelHeight) {
    (void)user;
    sn2_draw_frame(pixelWidth, pixelHeight);
}

static bool sn2_panel_press(void * user, double x, double y) {
    tSn2Edit edit;
    bool     grabbed = false;

    (void)sn2_draw_press(x, y, &edit, &grabbed);

    if (edit.paramId != UINT32_MAX) {
        synthlib_plugin_param_edited(user, edit.paramId, edit.normalized);
    }
    return grabbed;
}

static bool sn2_panel_keeps_pointer(void * user) {
    (void)user;
    return sn2_dial_mode() == eDialModeRotary;
}

static void sn2_panel_drag_to(void * user, double x, double y) {
    tSn2Edit edit;

    if (sn2_draw_drag_to(x, y, &edit)) {
        synthlib_plugin_param_edited(user, edit.paramId, edit.normalized);
    }
}

static void sn2_panel_drag_by(void * user, double dx, double dy) {
    tSn2Edit edit;

    if (sn2_draw_drag_by(dx, dy, &edit)) {
        synthlib_plugin_param_edited(user, edit.paramId, edit.normalized);
    }
}

static void sn2_panel_release(void * user, double x, double y) {
    tSn2Edit edit;

    sn2_draw_release(x, y, &edit);

    if (edit.paramId != UINT32_MAX) {
        synthlib_plugin_param_edited(user, edit.paramId, edit.normalized);
    }
}

static bool sn2_panel_key(void * user, int glfwKey, unsigned int codepoint, bool isRepeat) {
    (void)user;
    return sn2_draw_key(glfwKey, codepoint, isRepeat);
}

static void sn2_panel_scroll(void * user, double x, double y, double dy) {
    (void)user;
    (void)x;
    (void)y;
    sn2_draw_scroll(dy);
}

static const tSynthLibPanel      gPanel = {
    .canvasWidth  = SN2_CANVAS_W,
    .init         = sn2_panel_init,
    .sync         = sn2_panel_sync,
    .frame        = sn2_panel_frame,
    .press        = sn2_panel_press,
    .dragBy       = sn2_panel_drag_by,
    .keepsPointer = sn2_panel_keeps_pointer,
    .dragTo       = sn2_panel_drag_to,
    .release      = sn2_panel_release,
    .scroll       = sn2_panel_scroll,
    .key          = sn2_panel_key,
    .pointer      = sn2_draw_set_mouse,
    .menuActive   = sn2_draw_menu_active,
};

static void * sn2_create_view(const tSynthLibPluginDesc * desc, void * inst, double width, double height) {
    (void)desc;
    return synthlib_panel_view_create(&gPanel, inst, width, height);
}

static const tSynthLibBus        gOutputs[1] = {{"Out", 2, false, true}};

static const tSynthLibPluginDesc gDescriptor = {
    .name               = "Sn2Alike",
    .vendor             = "Chris Purusha",
    .url                = "https://github.com/chrispurusha/Sn2Alike",
    .email              = "",
    .version            = SN2_VERSION_STRING,

    .isInstrument       = true,
    .vst3SubCategory    = "Instrument|Synth",
    .outputs            = gOutputs,
    .numOutputs         =                                          1,
    .wantsMidiIn        = true,

    .vst3ProcessorUid   = gProcessorUid,
    .vst3ControllerUid  = gControllerUid,
    .auType             = SN2_AU_TYPE,
    .auSubType          = SN2_AU_SUBTYPE,
    .auManufacturer     = SN2_AU_MANUFACTURER,
    .auVersion          = SN2_AU_VERSION,
    .auBundleId         = SN2_AU_BUNDLE_ID,

    .params             = gSn2Params,
    .numParams          = kSn2ParamCount,

    .editorDefaultWidth = SN2_CANVAS_W,
    .editorMinWidth     = SN2_CANVAS_W * 0.75,
    .editorMaxWidth     = SN2_CANVAS_W * 2.0,
    .editorAspect       = SN2_CANVAS_W / SN2_CANVAS_H,

    .cb                 = {
        .create         = sn2_create,
        .destroy        = sn2_destroy,
        .prepare        = sn2_prepare,
        .latencySamples = sn2_latency_samples,
        .setActive      = sn2_set_active,
        .process        = sn2_process,
        .noteOn         = sn2_note_on,
        .noteOff        = sn2_note_off,
        .setParam       = sn2_set_param,
        .getParam       = sn2_get_param,
        .paramText      = sn2_param_text,
        .getState       = sn2_get_state,
        .setState       = sn2_set_state,
        .createView     = sn2_create_view,
    }
};

static const tSynthLibPluginSet  gSet = {
    .variants = &gDescriptor,
    .count    = 1
};

static const tSynthLibPluginSet * plugin_variants(void) {
    return &gSet;
}

SYNTHLIB_PLUGIN_REGISTER(plugin_variants, "sn2alike")
