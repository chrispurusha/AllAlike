/*
 * Nl3Alike - a reproduction inspired by the Clavia Nord Lead 3, as a VST3 and Audio Unit
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
// Notes: Docs/code-notes/nl3Plugin.md - "// notes §k" refers there.

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
#include "nl3Draw.h"
#include "nl3Params.h"
#include "nl3Patch.h"
#include "nl3Prefs.h"
#include "nl3Hardware.h"
#include "nl3Effects.h"
#include "nl3Voices.h"

// touch /tmp/nl3alike-log, read /tmp/nl3alike.log - see SynthLib's plugin/synthlibLog.h.

#ifndef NL3_VERSION_STRING
#define NL3_VERSION_STRING    "0.1.0"
#endif

#ifndef NL3_AU_VERSION
#define NL3_AU_VERSION    (0x00000100)
#endif

// THESE MAY NEVER CHANGE: a saved project finds the plug-in by them.
static const uint8_t gProcessorUid[16]  = {0x17, 0xE5, 0xB8, 0x3A, 0x85, 0xFE, 0x42, 0x7F, 0x94, 0xA6, 0x6C, 0x41, 0x8F, 0x65, 0x56, 0xBD};
static const uint8_t gControllerUid[16] = {0xDB, 0xDA, 0x8B, 0x35, 0x1F, 0xAB, 0x42, 0xB0, 0xB4, 0xFC, 0x91, 0xDC, 0xD8, 0xC4, 0x54, 0x30};

// THESE MUST MATCH do-plugin - the bundle identifier especially
#define FOUR_CC(a, b, c, d)    (((uint32_t)(a) << 24) | ((uint32_t)(b) << 16) | ((uint32_t)(c) << 8) | (uint32_t)(d))
#define NL3_AU_TYPE            FOUR_CC('a', 'u', 'm', 'u')
#define NL3_AU_SUBTYPE         FOUR_CC('N', 'l', '3', 'a')
#define NL3_AU_MANUFACTURER    FOUR_CC('C', 'P', 'u', 'r')
#define NL3_AU_BUNDLE_ID       "com.chrispurusha.nl3alike.au"

// notes §2: a block's notes, played at their own sample when the block renders
#define MAX_EVENTS             (256)

#define DEFAULT_TEMPO          (122.0) // notes §13
#define CLIP_HOLD_S            (1.5)
#define HOST_CEILING           (0.97675f)
#define HOST_CHUNK             (256u)
#define ENGINE_FRAMES          (4096u) // notes §12: HOST_CHUNK at a host rate down to 2.5 kHz

typedef struct {
    uint32_t offset;
    uint8_t  note;
    bool     on;
    float    velocity;
} tNl3Event;

typedef struct {
    tNl3Voices         voices;
    tSynthLibResampler resampler;  // notes §12
    tSynthLibResampler resamplerR;
    tNl3Effects        effects;
    double             hostRate;
    double             tempo;
    float              engine[ENGINE_FRAMES];
    float              engineL[ENGINE_FRAMES], engineR[ENGINE_FRAMES];
    tNl3Event          events[MAX_EVENTS];
    uint32_t           eventCount;
    double             values[kNl3ParamCount]; // normalized
    // notes §5: the sound, beyond its parameters - main thread, and the host's state calls
    char               soundName[NL3_SINGLE_NAME_LENGTH + 1];
    bool               haveBase;
    uint8_t            base[NL3_SINGLE_LENGTH]; // the Single it came from, whose bytes a save keeps
    char               patchPath[1024];
    int                patchIndex;              // its place in that file
    int                preset;                  // 1..NL3_PRESETS, 0 none
    double             presetsChecked;
    tSynthLibSwapFade  fade;
    _Atomic bool       clipped;     // the audio thread saw the ceiling; the panel holds the lamp
    double             clipUntil;
    // notes §14: the link to the instrument
    int                hwLock;
    bool               hwSent, hwExtensionDue;
    double             hwSentAt;
    uint8_t            hwLast[NL3_SINGLE_LENGTH];
    int                hwBend, hwWheel, hwPedal;
    _Atomic bool       startPreset;  // notes: a new instance plays A000
    _Atomic int        wantPreset;   // a program change waiting for the fade
    _Atomic bool       clearEffects; // notes §15: a new sound starts with empty effects, the old one's echoes gone
} tNl3Plugin;

static uint32_t voice_limit(const tNl3Plugin * t) {
    return 1u + (uint32_t)lround(t->values[kNl3ParamVoiceLimit] * (NL3_MAX_VOICES - 1));
}

static void * nl3_create(const tSynthLibPluginDesc * desc) {
    tNl3Plugin * t = (tNl3Plugin *)calloc(1, sizeof(tNl3Plugin));

    (void)desc;

    if (t == NULL) {
        return NULL;
    }

    for (uint32_t i = 0; i < kNl3ParamCount; i++) {
        t->values[i] = gNl3Params[i].defaultNormalized;
    }

    t->hwLock      = -1;
    t->hwBend      = t->hwWheel = t->hwPedal = -1;
    synthlib_swap_fade_init(&t->fade);
    nl3_presets_refresh();
    t->startPreset = true;

    snprintf(t->soundName, sizeof(t->soundName), "Init");
    nl3_voices_init(&t->voices);
    nl3_effects_reset(&t->effects);
    t->hostRate    = 48000.0;
    t->tempo       = DEFAULT_TEMPO;
    synthlib_resample_init(&t->resampler, NL3_ENGINE_HZ, t->hostRate);
    synthlib_resample_init(&t->resamplerR, NL3_ENGINE_HZ, t->hostRate);
    nl3_voices_set_limit(&t->voices, voice_limit(t));
    nl3_prefs_init();
    synthlib_log_line("created - Nl3Alike %s", NL3_VERSION_STRING);
    return t;
}

static void nl3_destroy(void * inst) {
    nl3_hardware_release(&((tNl3Plugin *)inst)->hwLock);
    synthlib_resample_free(&((tNl3Plugin *)inst)->resampler);
    synthlib_resample_free(&((tNl3Plugin *)inst)->resamplerR);
    free(inst);
}

// notes §5: the plug-in's own state, after the parameters
#define STATE_MAGIC     "TIS1"
#define STATE_LENGTH    (4 + (NL3_SINGLE_NAME_LENGTH + 1) + 1 + NL3_SINGLE_LENGTH + 1024 + 2 + 2)

static size_t nl3_get_state(void * inst, void * out, size_t len) {
    tNl3Plugin * t = (tNl3Plugin *)inst;
    uint8_t *    o = (uint8_t *)out;

    if ((out == NULL) || (len < STATE_LENGTH)) {
        return STATE_LENGTH;
    }
    memcpy(o, STATE_MAGIC, 4);
    o   += 4;
    memcpy(o, t->soundName, NL3_SINGLE_NAME_LENGTH + 1);
    o   += NL3_SINGLE_NAME_LENGTH + 1;
    *o++ = t->haveBase ? 1 : 0;
    memcpy(o, t->base, NL3_SINGLE_LENGTH);
    o   += NL3_SINGLE_LENGTH;
    memcpy(o, t->patchPath, 1024);
    o   += 1024;
    *o++ = (uint8_t)(t->patchIndex >> 8);
    *o++ = (uint8_t)(t->patchIndex & 0xFF);
    *o++ = (uint8_t)(t->preset >> 8);
    *o   = (uint8_t)(t->preset & 0xFF);
    return STATE_LENGTH;
}

static void nl3_set_state(void * inst, const void * data, size_t len) {
    tNl3Plugin *    t = (tNl3Plugin *)inst;
    const uint8_t * d = (const uint8_t *)data;

    if ((data == NULL) || (len < STATE_LENGTH) || (memcmp(d, STATE_MAGIC, 4) != 0)) {
        return;
    }
    atomic_store(&t->startPreset, false);   // a project's sound, not A001
    d                                   += 4;
    memcpy(t->soundName, d, NL3_SINGLE_NAME_LENGTH + 1);
    t->soundName[NL3_SINGLE_NAME_LENGTH] = '\0';
    d                                   += NL3_SINGLE_NAME_LENGTH + 1;
    t->haveBase                          = (*d++ != 0);
    memcpy(t->base, d, NL3_SINGLE_LENGTH);
    d                                   += NL3_SINGLE_LENGTH;
    memcpy(t->patchPath, d, 1024);
    t->patchPath[1023]                   = '\0';
    d                                   += 1024;
    t->patchIndex                        = (d[0] << 8) | d[1];
    t->preset                            = (d[2] << 8) | d[3];
    t->preset                            = ((t->preset >= 1) && (t->preset <= NL3_PRESETS)) ? t->preset : 0;
}

// notes §12: voices at their own rate, converted to the host's
static void render_host(tNl3Plugin * t, const tNl3Sound * sound, float * outL, float * outR, uint32_t frames, double gain) {
    tNl3EffectsSound effects = nl3_effects_sound(t->values, t->tempo);

    if (atomic_exchange(&t->clearEffects, false)) {
        nl3_effects_reset(&t->effects);   // notes §15
    }

    while (frames > 0) {
        uint32_t chunk = (frames < HOST_CHUNK) ? frames : HOST_CHUNK;
        uint32_t need  = synthlib_resample_needed(&t->resampler, chunk);

        memset(t->engine, 0, (size_t)need * sizeof(float));
        nl3_voices_render(&t->voices, sound, t->engine, need, gain);

        if (t->values[kNl3ParamEffectsBypass] > 0.5) {
            nl3_effects_bypass(&t->effects, t->engine, t->engineL, t->engineR, need);
        } else {
            nl3_effects_process(&t->effects, &effects, t->engine, t->engineL, t->engineR, need);
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
            outL[i] = fminf(HOST_CEILING, fmaxf(-HOST_CEILING, outL[i]));

            if (outR != NULL) {
                outR[i] = fminf(HOST_CEILING, fmaxf(-HOST_CEILING, outR[i]));
            }
        }

        outL   += chunk;
        outR    = (outR != NULL) ? (outR + chunk) : NULL;
        frames -= chunk;
    }
}

static uint32_t nl3_latency_samples(void * inst) {
    return ((tNl3Plugin *)inst)->resampler.latency;
}

static void nl3_prepare(void * inst, const tSynthLibSetup * setup) {
    tNl3Plugin * t = (tNl3Plugin *)inst;

    t->hostRate = setup->sampleRate;
    synthlib_resample_init(&t->resampler, NL3_ENGINE_HZ, t->hostRate);
    synthlib_resample_init(&t->resamplerR, NL3_ENGINE_HZ, t->hostRate);

    if (setup->offline) {
        atomic_store(&t->startPreset, false);   // an offline render plays what it is given
    }
}

static void nl3_set_active(void * inst, bool active) {
    (void)active;
    ((tNl3Plugin *)inst)->eventCount = 0;
    nl3_voices_all_off(&((tNl3Plugin *)inst)->voices);
}

static tNl3Sound make_sound(const tNl3Plugin * t) {
    tNl3Sound sound = (tNl3Sound){
        .osc         = {
            OSC_SOUND(t->values, kNl3ParamOscShape,  kNl3ParamPulseWidth, kNl3ParamOscSoften,  kNl3ParamOscLevel,  kNl3ParamOsc1Octave, kNl3ParamOsc1Semitone, kNl3ParamOsc1Fine),
            OSC_SOUND(t->values, kNl3ParamOsc2Shape, kNl3ParamOsc2Width,  kNl3ParamOsc2Soften, kNl3ParamOsc2Level, kNl3ParamOsc2Octave, kNl3ParamOsc2Semitone, kNl3ParamOsc2Fine),
            OSC_SOUND(t->values, kNl3ParamOsc3Shape, kNl3ParamOsc3Width,  kNl3ParamOsc3Soften, kNl3ParamOsc3Level, kNl3ParamOsc3Octave, kNl3ParamOsc3Semitone, kNl3ParamOsc3Fine),
        },
        .cutoff      = t->values[kNl3ParamCutoff] * 127.0,
        .resonance   = t->values[kNl3ParamResonance] * 127.0,
        .tracking    = t->values[kNl3ParamFilterTrack],
        .overdrive   = t->values[kNl3ParamFilterDrive] * 127.0,
        .normalise   = t->values[kNl3ParamFilterQ] * 127.0,
        .oscsFree    = t->values[kNl3ParamOscTrigger] > 0.5,
        .glideTime   = t->values[kNl3ParamGlideTime] * 127.0,
        .glideStyle  = (int)lround(t->values[kNl3ParamGlideStyle]),
        .glideCurve  = (int)lround(t->values[kNl3ParamGlideCurve]),
        .glideType   = (int)lround(t->values[kNl3ParamGlideType] * 9.0),
        .ampVelocity = t->values[kNl3ParamAmpVelocity] * 127.0,
        .filterType  = (uint32_t)lround(t->values[kNl3ParamFilterType] * 11.0),
        .filterSlope = (uint32_t)lround(t->values[kNl3ParamFilterSlope] * 2.0),
        .ampAttack   = t->values[kNl3ParamAmpAttack] * 127.0,
        .ampDecay    = t->values[kNl3ParamAmpDecay] * 127.0,
        .ampSustain  = t->values[kNl3ParamAmpSustain] * 127.0,
        .ampRelease  = t->values[kNl3ParamAmpRelease] * 127.0,
        .noiseLevel  = t->values[kNl3ParamNoiseLevel] * 127.0 / 128.0,
        .noiseSoften = t->values[kNl3ParamNoiseSoften] * 127.0,
        .ring13      = t->values[kNl3ParamRing13] * 127.0 / 128.0,
        .ring23      = t->values[kNl3ParamRing23] * 127.0 / 128.0,
        .env         = {MOD_ENV_SOUND(t->values, kNl3ParamEnv2Attack, kNl3ParamEnv2Decay, kNl3ParamEnv2Sustain, kNl3ParamEnv2Release, kNl3ParamEnv2Velocity, kNl3ParamEnv2Cutoff), MOD_ENV_SOUND(t->values, kNl3ParamEnv3Attack, kNl3ParamEnv3Decay, kNl3ParamEnv3Sustain, kNl3ParamEnv3Release, kNl3ParamEnv3Velocity, kNl3ParamEnv3Cutoff)},
    };

    nl3_controls_sound(t->values, &sound);
    return sound;
}

static void play(tNl3Plugin * t, const tNl3Sound * sound, const tNl3Event * e) {
    if (e->on) {
        nl3_voices_note_on(&t->voices, sound, e->note, e->velocity);
    } else {
        nl3_voices_note_off(&t->voices, sound, e->note);
    }
}

// notes §2: a full queue plays the note at once rather than lose it
static void queue(tNl3Plugin * t, tNl3Event e) {
    if (t->eventCount < MAX_EVENTS) {
        t->events[t->eventCount++] = e;
    } else {
        tNl3Sound sound = make_sound(t);

        play(t, &sound, &e);
    }
}

static uint8_t hw_channel(const tNl3Plugin * t) {
    return (uint8_t)lround(t->values[kNl3ParamHwChannel] * 15.0);
}

static void nl3_note_on(void * inst, uint8_t channel, uint8_t note, float velocity, uint32_t sampleOffset) {
    tNl3Plugin * t = (tNl3Plugin *)inst;

    if (t->values[kNl3ParamHwLink] >= 0.5) {
        nl3_hardware_short(nl3_hw_port(), (uint8_t)(0x90 | hw_channel(t)), note, (uint8_t)lroundf(velocity * 127.0f));
    }
    (void)channel;
    queue((tNl3Plugin *)inst, (tNl3Event){sampleOffset, note, true, velocity});
}

static void nl3_note_off(void * inst, uint8_t channel, uint8_t note, float velocity, uint32_t sampleOffset) {
    tNl3Plugin * t = (tNl3Plugin *)inst;

    if (t->values[kNl3ParamHwLink] >= 0.5) {
        nl3_hardware_short(nl3_hw_port(), (uint8_t)(0x90 | hw_channel(t)), note, 0);
    }
    (void)channel;
    (void)velocity;
    queue((tNl3Plugin *)inst, (tNl3Event){sampleOffset, note, false, 0.0f});
}

static void nl3_load_single(void * user, const uint8_t * single, const char * path, int index, int preset);

// A new instance plays A000, first audio block or first editor frame; with no presets it stays on Init
static void take_start_preset(tNl3Plugin * t) {
    if (atomic_exchange(&t->startPreset, false) && (nl3_presets_single(1)[0] == 0xF0)) {
        nl3_load_single(t, nl3_presets_single(1), "", 0, 1);
    }
}

static void take_program_change(tNl3Plugin * t) {
    int number = atomic_load(&t->wantPreset);

    if ((number != 0) && synthlib_swap_fade_silent(&t->fade)) {
        atomic_store(&t->wantPreset, 0);
        nl3_load_single(t, nl3_presets_single(number), "", 0, number);
        synthlib_swap_fade_end(&t->fade);
    }
}

static void nl3_process(void * inst, const float *const * in, uint32_t numIn, float ** out, uint32_t numOut,
                        uint32_t frames, const tSynthLibTransport * transport) {
    tNl3Plugin * t      = (tNl3Plugin *)inst;

    take_start_preset(t);
    take_program_change(t);
    double       gain   = nl3_volume_gain(t->values[kNl3ParamVolume]);
    double       master = t->values[kNl3ParamMaster];
    tNl3Sound    sound  = make_sound(t);

    (void)in;
    (void)numIn;
    t->tempo = ((transport != NULL) && transport->tempoValid && (transport->tempo > 0.0)) ? transport->tempo : DEFAULT_TEMPO;

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

static void nl3_set_param(void * inst, uint32_t id, double normalized) {
    tNl3Plugin * t = (tNl3Plugin *)inst;

    if (id >= kNl3ParamCount) {
        return;
    }
    t->values[id] = normalized;

    if (id == kNl3ParamProgram) {
        int bank   = (int)lround(t->values[kNl3ParamBank] * 127.0) % (NL3_PRESETS / NL3_BANK_SIZE);
        int number = (bank * NL3_BANK_SIZE) + (int)lround(normalized * 127.0) + 1;

        // already on it, nothing happens, edits and all
        if ((number != t->preset) && (nl3_presets_single(number)[0] == 0xF0)) {
            atomic_store(&t->wantPreset, number);
            synthlib_swap_fade_start(&t->fade);
        }
    } else if (id == kNl3ParamVoiceLimit) {
        nl3_voices_set_limit(&t->voices, voice_limit(t));
    }
}

static double nl3_get_param(void * inst, uint32_t id) {
    return (id < kNl3ParamCount) ? ((tNl3Plugin *)inst)->values[id] : 0.0;
}

static bool nl3_param_text(const tSynthLibPluginDesc * desc, void * inst, uint32_t id, double value, char * out, size_t len) {
    static const char *const shapes[]  = {"Saw", "Square", "Double Saw"};
    static const char *const filters[] = {"LPF", "BPF", "HPF", "Res LPF", "Res BPF", "Res HPF", "Notch", "LPF+LPF", "BPF+BPF", "HPF+HPF", "LPF+BPF", "BPF+HPF"};
    static const char *const slopes[]  = {"12 dB", "18 dB", "24 dB"};

    (void)desc;
    (void)inst;

    if (id >= kNl3ParamCount) {
        return false;
    }
    long                     step      = lround(value * gNl3Params[id].stepCount);

    if ((id == kNl3ParamOscShape) || (id == kNl3ParamOsc2Shape) || (id == kNl3ParamOsc3Shape)) {
        snprintf(out, len, "%s", shapes[(step < 0) ? 0 : ((step > 2) ? 2 : step)]);
    } else if (id == kNl3ParamFilterType) {
        snprintf(out, len, "%s", filters[(step < 0) ? 0 : ((step > 11) ? 11 : step)]);
    } else if (id == kNl3ParamChorusType) {
        static const char *const types[] = {"Quad Chorus", "Chorus/Flanger", "Phaser"};

        snprintf(out, len, "%s", types[(step < 0) ? 0 : ((step > 2) ? 2 : step)]);
    } else if (id == kNl3ParamGlideType) {
        static const char *const types[] = {"Normal", "Auto", "2 Semi Down", "2 Semi Up", "5 Semi Down", "5 Semi Up", "7 Semi Down", "7 Semi Up", "12 Semi Down", "12 Semi Up"};

        snprintf(out, len, "%s", types[(step < 0) ? 0 : ((step > 9) ? 9 : step)]);
    } else if (id == kNl3ParamGlideStyle) {
        snprintf(out, len, "%s", (step > 0) ? "Glissando" : "Portamento");
    } else if (id == kNl3ParamGlideCurve) {
        snprintf(out, len, "%s", (step > 0) ? "Exponential" : "Linear");
    } else if (id == kNl3ParamReverbType) {
        static const char *const types[] = {"Gated Reverse", "Gated Rising", "Gated Gentle", "Gated Falling", "Dry Chamber", "Echo Chamber", "Small Room", "Big Room", "Medium 1", "Medium 2", "Medium Plate 1", "Medium Plate 2", "Large 1", "Large 2", "Large Plate 1", "Large Plate 2"};

        snprintf(out, len, "%s", types[(step < 0) ? 0 : ((step > 15) ? 15 : step)]);
    } else if (id == kNl3ParamEffectsOrder) {
        static const char *const orders[] = {"D+R+C", "D>R>C", "D>C>R", "R>D>C", "R>C>D", "C>D>R", "C>R>D", "D+[R>C]", "D+[C>R]", "R+[D>C]", "R+[C>D]", "C+[D>R]", "C+[R>D]", "D>[R+C]", "R>[D+C]", "C>[D+R]", "[R+C]>D", "[D+C]>R", "[D+R]>C"};

        snprintf(out, len, "%s", orders[(step < 0) ? 0 : ((step > 18) ? 18 : step)]);
    } else if (id == kNl3ParamOscTrigger) {
        snprintf(out, len, "%s", (step > 0) ? "Ensemble" : "Percussive");
    } else if (id == kNl3ParamDelaySync) {
        static const char *const syncs[] = {"Off", "32nd T", "32nd", "16th T", "16th", "8th T", "16th .", "8th", "4th T", "8th .", "4th", "2nd T", "4th .", "2nd", "1 Bar T", "2nd .", "1 Bar", "2 Bar T", "1 Bar .", "2 Bars", "4 Bar T", "3 Bars", "5 Bar T", "4 Bars", "3 Bar .", "7 Bar T", "5 Bars", "8 Bar T", "6 Bars", "7 Bars", "5 Bar .", "8 Bars", "6 Bar .", "7 Bar .", "8 Bar ."};

        snprintf(out, len, "%s", syncs[(step < 0) ? 0 : ((step > 34) ? 34 : step)]);
    } else if (id == kNl3ParamDelayRatio) {
        static const char *const ratios[] = {"1:1", "1:0.75", "0.75:1", "1:0.66", "0.66:1", "1:0.5", "0.5:1", "1:0.33", "0.33:1", "1:0.25", "0.25:1", "1:Off", "Off:1"};

        snprintf(out, len, "%s", ratios[(step < 0) ? 0 : ((step > 12) ? 12 : step)]);
    } else if ((id == kNl3ParamDelayOn) || (id == kNl3ParamReverbOn)) {
        snprintf(out, len, "%s", (step > 0) ? "On" : "Off");
    } else if ((id == kNl3ParamLfo1Shape) || (id == kNl3ParamLfo2Shape)) {
        static const char *const shapes[] = {"Square", "Saw", "Triangle", "S+H"};

        snprintf(out, len, "%s", shapes[(step < 0) ? 0 : ((step > 3) ? 3 : step)]);
    } else if ((id == kNl3ParamLfo1Range) || (id == kNl3ParamLfo2Range)) {
        static const char *const ranges[] = {"x1", "x10", "x100"};

        snprintf(out, len, "%s", ranges[(step < 0) ? 0 : ((step > 2) ? 2 : step)]);
    } else if ((id == kNl3ParamLfo1Trigger) || (id == kNl3ParamLfo2Trigger)) {
        snprintf(out, len, "%s", (step > 0) ? "Key" : "Free");
    } else if ((id == kNl3ParamLfo1Fade) || (id == kNl3ParamLfo2Fade)) {
        snprintf(out, len, "%s", (step > 0) ? "Out" : "In");
    } else if ((id == kNl3ParamHwLink) || (id == kNl3ParamPanEffects)) {
        snprintf(out, len, "%s", (step > 0) ? "On" : "Off");
    } else if (id == kNl3ParamHwChannel) {
        snprintf(out, len, "%ld", step + 1);
    } else if (id == kNl3ParamEffectsBypass) {
        snprintf(out, len, "%s", (step > 0) ? "Bypass" : "On");
    } else if (id == kNl3ParamPolyMode) {
        snprintf(out, len, "%s", (step > 0) ? "Mono" : "Poly");
    } else if (id == kNl3ParamEnvTrigger) {
        snprintf(out, len, "%s", (step > 0) ? "Legato" : "Retrigger");
    } else if (id == kNl3ParamFilterSlope) {
        snprintf(out, len, "%s", slopes[(step < 0) ? 0 : ((step > 2) ? 2 : step)]);
    } else {
        snprintf(out, len, "%ld", lround(gNl3Params[id].plainMin) + step);
    }
    return true;
}

// ---- the editor - notes §3: Pulse2Alike's panel view, with this plug-in's pages --------------------------------

// ---- Singles and presets - notes §5 ------------------------------------------------------------------------

// What a sound carries: everything but the plug-in's own settings
static bool sound_param(uint32_t id) {
    return (id != kNl3ParamVoiceLimit) && (id != kNl3ParamMaster) && (id != kNl3ParamEffectsBypass) && (id != kNl3ParamSustainPedal)
           && (id != kNl3ParamHwLink) && (id != kNl3ParamHwChannel) && (id != kNl3ParamHwDevice) && (id != kNl3ParamProgram) && (id != kNl3ParamBank)
           && (id != kNl3ParamPitchBend) && (id != kNl3ParamModWheel);
}

static void current_values(void * user, double values[kNl3ParamCount]) {
    for (uint32_t id = 0; id < kNl3ParamCount; id++) {
        values[id] = synthlib_plugin_param_value(user, id);
    }
}

// The sound as it is now, as a Single: the host's values over the Single it came from
static void current_single(void * user, uint8_t single[NL3_SINGLE_LENGTH]) {
    tNl3Plugin * t = (tNl3Plugin *)user;
    double       values[kNl3ParamCount];

    current_values(user, values);
    nl3_patch_to_single(values, t->haveBase ? t->base : NULL, t->soundName, single);
}

// notes §6: every parameter the sound carries, through the host, so the change is one the host sees
static void nl3_load_single(void * user, const uint8_t * single, const char * path, int index, int preset) {
    tNl3Plugin * t = (tNl3Plugin *)user;
    double       values[kNl3ParamCount];

    current_values(user, values);
    nl3_patch_from_single(single, values);

    for (uint32_t id = 0; id < kNl3ParamCount; id++) {
        if (sound_param(id)) {
            synthlib_plugin_param_edited(user, id, values[id]);
        }
    }

    atomic_store(&t->clearEffects, true);   // notes §15: the audio thread does it, at its next block
    memcpy(t->base, single, NL3_SINGLE_LENGTH);
    t->haveBase   = true;
    nl3_patch_name(single, t->soundName);
    snprintf(t->patchPath, sizeof(t->patchPath), "%s", path);
    t->patchIndex = index;
    t->preset     = preset;
    synthlib_log_line("loaded \"%s\" (%s #%d, preset %d)", t->soundName, path, index, preset);
}

static bool nl3_save_patch(void * user, const char * path) {
    tNl3Plugin * t = (tNl3Plugin *)user;
    uint8_t      single[NL3_SINGLE_LENGTH];

    current_single(user, single);

    if (!nl3_patch_save_file(path, single)) {
        synthlib_log_line("could not write %s", path);
        return false;
    }
    memcpy(t->base, single, NL3_SINGLE_LENGTH);
    t->haveBase   = true;
    snprintf(t->patchPath, sizeof(t->patchPath), "%s", path);
    t->patchIndex = 0;
    t->preset     = 0;
    return true;
}

static void nl3_load_faded(void * user, const uint8_t * single, const char * path, int index, int preset) {
    tNl3Plugin * t = (tNl3Plugin *)user;

    synthlib_swap_fade_begin(&t->fade);
    nl3_load_single(user, single, path, index, preset);
    synthlib_swap_fade_end(&t->fade);
}

static bool nl3_store_preset(void * user, int number) {
    tNl3Plugin * t = (tNl3Plugin *)user;
    uint8_t      single[NL3_SINGLE_LENGTH];

    current_single(user, single);

    if (!nl3_presets_store(number, single)) {
        return false;
    }
    memcpy(t->base, nl3_presets_single(number), NL3_SINGLE_LENGTH);
    t->haveBase     = true;
    t->patchPath[0] = '\0';
    t->patchIndex   = 0;
    t->preset       = number;
    return true;
}

// every sound parameter to its default; the next save has no Single to keep bytes from
static void nl3_init_patch(void * user) {
    tNl3Plugin * t = (tNl3Plugin *)user;

    for (uint32_t id = 0; id < kNl3ParamCount; id++) {
        if (sound_param(id)) {
            synthlib_plugin_param_edited(user, id, gNl3Params[id].defaultNormalized);
        }
    }

    t->haveBase     = false;
    t->patchPath[0] = '\0';
    t->patchIndex   = 0;
    t->preset       = 0;
    snprintf(t->soundName, sizeof(t->soundName), "Init");
}

static const char * nl3_sound_name(void * user) {
    return ((tNl3Plugin *)user)->soundName;
}

// notes §5: the name only - written to a Single at the next save or store
static void nl3_rename(void * user, const char * name) {
    snprintf(((tNl3Plugin *)user)->soundName, NL3_SINGLE_NAME_LENGTH + 1, "%s", name);
}

// notes §7: edited, if a parameter it carries or its name differs from the stored preset
static bool preset_edited(tNl3Plugin * t, void * user) {
    double values[kNl3ParamCount], stored[kNl3ParamCount];
    char   name[NL3_SINGLE_NAME_LENGTH + 1];

    if (t->preset == 0) {
        return false;
    }
    current_values(user, values);
    memcpy(stored, values, sizeof(stored));
    nl3_patch_from_single(nl3_presets_single(t->preset), stored);
    nl3_patch_name(nl3_presets_single(t->preset), name);

    for (uint32_t id = 0; id < kNl3ParamCount; id++) {
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

// notes §14: the instrument follows the sound, whole, and the controllers
static void hardware_link(tNl3Plugin * t, void * user) {
    if (synthlib_plugin_param_value(user, kNl3ParamHwLink) < 0.5) {
        if (t->hwLock >= 0) {
            nl3_hardware_short(nl3_hw_port(), (uint8_t)(0xB0 | hw_channel(t)), 123, 0);
        }
        nl3_hardware_release(&t->hwLock);
        t->hwSent         = false;
        t->hwExtensionDue = false;
        return;
    }

    if (!nl3_hardware_claim(&t->hwLock)) {
        return;
    }
    uint8_t single[NL3_SINGLE_LENGTH];
    uint8_t device = (uint8_t)lround(synthlib_plugin_param_value(user, kNl3ParamHwDevice) * 127.0);

    if (t->hwExtensionDue) {
        if ((now_seconds() - t->hwSentAt) >= LINK_EXT_SECONDS) {
            (void)nl3_hardware_send_extension(t->hwLast, nl3_hw_port(), device);
            t->hwExtensionDue = false;
        }
        return;
    }
    current_single(user, single);

    if ((!t->hwSent || (memcmp(single, t->hwLast, NL3_SINGLE_LENGTH) != 0)) && ((now_seconds() - t->hwSentAt) >= LINK_MIN_SECONDS)) {
        tNl3HardwareResult r = nl3_hardware_send_main(single, nl3_hw_port(), device);

        memcpy(t->hwLast, single, NL3_SINGLE_LENGTH);
        t->hwSentAt       = now_seconds();
        t->hwSent         = (r == eNl3HardwareSent);
        t->hwExtensionDue = t->hwSent;
        synthlib_log_line("Link: sound sent to \"%s\" (result %d)", nl3_hw_port(), (int)r);
    }
    uint8_t status = (uint8_t)(0xB0 | hw_channel(t));
    int     bend   = (int)lround(synthlib_plugin_param_value(user, kNl3ParamPitchBend) * 16383.0);
    int     wheel  = (int)lround(synthlib_plugin_param_value(user, kNl3ParamModWheel) * 127.0);
    int     pedal  = (synthlib_plugin_param_value(user, kNl3ParamSustainPedal) >= 0.5) ? 127 : 0;

    if (bend != t->hwBend) {
        nl3_hardware_short(nl3_hw_port(), (uint8_t)(0xE0 | hw_channel(t)), (uint8_t)(bend & 0x7F), (uint8_t)(bend >> 7));
        t->hwBend = bend;
    }

    if (wheel != t->hwWheel) {
        nl3_hardware_short(nl3_hw_port(), status, 1, (uint8_t)wheel);
        t->hwWheel = wheel;
    }

    if (pedal != t->hwPedal) {
        nl3_hardware_short(nl3_hw_port(), status, 64, (uint8_t)pedal);
        t->hwPedal = pedal;
    }
}

static const tNl3PatchHost gPatchHost = {
    .load = nl3_load_faded, .save   = nl3_save_patch, .storePreset = nl3_store_preset, .init = nl3_init_patch,
    .name = nl3_sound_name, .rename = nl3_rename,
};

// In mono the voice limit means nothing: the dial says so
static bool limit_display(uint32_t paramId, const double * values, int32_t * step, char * text, size_t len) {
    if ((paramId != kNl3ParamVoiceLimit) || (values[kNl3ParamPolyMode] <= 0.5)) {
        return false;
    }
    *step = 0;
    snprintf(text, len, "Mono");
    return true;
}

static void nl3_panel_init(void) {
    nl3_draw_init();
    nl3_draw_set_pages(gNl3Pages, gNl3PageCount);
    nl3_draw_set_host(&gPatchHost);
    nl3_draw_set_display(limit_display);
}

// notes: a plain-text channel, off until /tmp/nl3alike-backdoor exists
static double bd_get(void * user, uint32_t id) {
    return nl3_get_param(user, id);
}

static void bd_set(void * user, uint32_t id, double normalized) {
    nl3_set_param(user, id, normalized);
    synthlib_plugin_param_edited(user, id, normalized);
}

static bool bd_text(void * user, uint32_t id, double normalized, char * out, size_t len) {
    return nl3_param_text(NULL, user, id, normalized, out, len);
}

static bool bd_preset(void * user, int number) {
    if ((number < 1) || (number > NL3_PRESETS)) {
        return false;
    }
    (void)nl3_presets_refresh();
    nl3_load_faded(user, nl3_presets_single(number), "", 0, number);
    return true;
}

static const tSynthLibBackdoor gBackdoor = {
    .params = gNl3Params,     .count  = kNl3ParamCount,
    .get    = bd_get,         .set    = bd_set,        .text  = bd_text,
    .name   = nl3_sound_name, .rename = nl3_rename,
    .preset = bd_preset,      .save   = nl3_save_patch,.init  = nl3_init_patch,
};

static void nl3_panel_sync(void * user) {
    synthlib_backdoor_poll("nl3alike", &gBackdoor, user);
    tNl3Plugin * t = (tNl3Plugin *)user;

    nl3_draw_set_user(user);

    if (atomic_exchange(&t->clipped, false)) {
        t->clipUntil = now_seconds() + CLIP_HOLD_S;
    }
    nl3_draw_set_clip(now_seconds() < t->clipUntil);
    hardware_link(t, user);
    nl3_draw_read_tick();

    // another instance's store shows within a second
    if ((now_seconds() - t->presetsChecked) >= 1.0) {
        t->presetsChecked = now_seconds();
        (void)nl3_presets_refresh();
    }
    nl3_draw_set_patch(t->patchPath, t->patchIndex);   // this instance's, every sync
    nl3_draw_set_preset(t->preset, preset_edited(t, user));

    take_start_preset((tNl3Plugin *)user);

    for (uint32_t id = 0; id < kNl3ParamCount; id++) {
        nl3_draw_set_value(id, synthlib_plugin_param_value(user, id));
    }
}

static void nl3_panel_frame(void * user, int pixelWidth, int pixelHeight) {
    (void)user;
    nl3_draw_frame(pixelWidth, pixelHeight);
}

static bool nl3_panel_press(void * user, double x, double y) {
    tNl3Edit edit;
    bool     grabbed = false;

    (void)nl3_draw_press(x, y, &edit, &grabbed);

    if (edit.paramId != UINT32_MAX) {
        synthlib_plugin_param_edited(user, edit.paramId, edit.normalized);
    }
    return grabbed;
}

static bool nl3_panel_keeps_pointer(void * user) {
    (void)user;
    return nl3_dial_mode() == eDialModeRotary;
}

static void nl3_panel_drag_to(void * user, double x, double y) {
    tNl3Edit edit;

    if (nl3_draw_drag_to(x, y, &edit)) {
        synthlib_plugin_param_edited(user, edit.paramId, edit.normalized);
    }
}

static void nl3_panel_drag_by(void * user, double dx, double dy) {
    tNl3Edit edit;

    if (nl3_draw_drag_by(dx, dy, &edit)) {
        synthlib_plugin_param_edited(user, edit.paramId, edit.normalized);
    }
}

static void nl3_panel_release(void * user, double x, double y) {
    tNl3Edit edit;

    nl3_draw_release(x, y, &edit);

    if (edit.paramId != UINT32_MAX) {
        synthlib_plugin_param_edited(user, edit.paramId, edit.normalized);
    }
}

static bool nl3_panel_key(void * user, int glfwKey, unsigned int codepoint, bool isRepeat) {
    (void)user;
    return nl3_draw_key(glfwKey, codepoint, isRepeat);
}

static void nl3_panel_scroll(void * user, double x, double y, double dy) {
    (void)user;
    (void)x;
    (void)y;
    nl3_draw_scroll(dy);
}

static const tSynthLibPanel      gPanel = {
    .canvasWidth  = NL3_CANVAS_W,
    .init         = nl3_panel_init,
    .sync         = nl3_panel_sync,
    .frame        = nl3_panel_frame,
    .press        = nl3_panel_press,
    .dragBy       = nl3_panel_drag_by,
    .keepsPointer = nl3_panel_keeps_pointer,
    .dragTo       = nl3_panel_drag_to,
    .release      = nl3_panel_release,
    .scroll       = nl3_panel_scroll,
    .key          = nl3_panel_key,
    .pointer      = nl3_draw_set_mouse,
    .menuActive   = nl3_draw_menu_active,
};

static void * nl3_create_view(const tSynthLibPluginDesc * desc, void * inst, double width, double height) {
    (void)desc;
    return synthlib_panel_view_create(&gPanel, inst, width, height);
}

static const tSynthLibBus        gOutputs[1] = {{"Out", 2, false, true}};

static const tSynthLibPluginDesc gDescriptor = {
    .name               = "Nl3Alike",
    .vendor             = "Chris Purusha",
    .url                = "https://github.com/chrispurusha/Nl3Alike",
    .email              = "",
    .version            = NL3_VERSION_STRING,

    .isInstrument       = true,
    .vst3SubCategory    = "Instrument|Synth",
    .outputs            = gOutputs,
    .numOutputs         =                                          1,
    .wantsMidiIn        = true,

    .vst3ProcessorUid   = gProcessorUid,
    .vst3ControllerUid  = gControllerUid,
    .auType             = NL3_AU_TYPE,
    .auSubType          = NL3_AU_SUBTYPE,
    .auManufacturer     = NL3_AU_MANUFACTURER,
    .auVersion          = NL3_AU_VERSION,
    .auBundleId         = NL3_AU_BUNDLE_ID,

    .params             = gNl3Params,
    .numParams          = kNl3ParamCount,

    .editorDefaultWidth = NL3_CANVAS_W,
    .editorMinWidth     = NL3_CANVAS_W * 0.75,
    .editorMaxWidth     = NL3_CANVAS_W * 2.0,
    .editorAspect       = NL3_CANVAS_W / NL3_CANVAS_H,

    .cb                 = {
        .create         = nl3_create,
        .destroy        = nl3_destroy,
        .prepare        = nl3_prepare,
        .latencySamples = nl3_latency_samples,
        .setActive      = nl3_set_active,
        .process        = nl3_process,
        .noteOn         = nl3_note_on,
        .noteOff        = nl3_note_off,
        .setParam       = nl3_set_param,
        .getParam       = nl3_get_param,
        .paramText      = nl3_param_text,
        .getState       = nl3_get_state,
        .setState       = nl3_set_state,
        .createView     = nl3_create_view,
    }
};

static const tSynthLibPluginSet  gSet = {
    .variants = &gDescriptor,
    .count    = 1
};

static const tSynthLibPluginSet * plugin_variants(void) {
    return &gSet;
}

SYNTHLIB_PLUGIN_REGISTER(plugin_variants, "nl3alike")
