/*
 * ProphAlike - a placeholder inspired by the Korg Prophecy, as a VST3 and Audio Unit
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
// Notes: Docs/code-notes/prophPlugin.md - "// notes §k" refers there.

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
#include "prophDraw.h"
#include "prophParams.h"
#include "prophPatch.h"
#include "prophPrefs.h"
#include "prophHardware.h"
#include "synthlibCtlPoints.h"
#include "prophEffects.h"
#include "prophVoices.h"

// touch /tmp/prophalike-log, read /tmp/prophalike.log - see SynthLib's plugin/synthlibLog.h.

#ifndef PROPH_VERSION_STRING
#define PROPH_VERSION_STRING    "0.1.0"
#endif

#ifndef PROPH_AU_VERSION
#define PROPH_AU_VERSION    (0x00000100)
#endif

// THESE MAY NEVER CHANGE: a saved project finds the plug-in by them.
static const uint8_t gProcessorUid[16]  = {0x88, 0x5B, 0xB1, 0xF5, 0x0A, 0x59, 0x44, 0x6F, 0x9B, 0xFB, 0xA4, 0x2B, 0x9D, 0x02, 0xBB, 0xB4};
static const uint8_t gControllerUid[16] = {0x1B, 0xA4, 0x4D, 0x94, 0xB8, 0x7D, 0x49, 0x39, 0xB0, 0xBB, 0x6F, 0x69, 0x51, 0x7B, 0xE9, 0x7E};

// THESE MUST MATCH do-plugin - the bundle identifier especially
#define FOUR_CC(a, b, c, d)    (((uint32_t)(a) << 24) | ((uint32_t)(b) << 16) | ((uint32_t)(c) << 8) | (uint32_t)(d))
#define PROPH_AU_TYPE            FOUR_CC('a', 'u', 'm', 'u')
#define PROPH_AU_SUBTYPE         FOUR_CC('P', 'r', 'p', 'h')
#define PROPH_AU_MANUFACTURER    FOUR_CC('C', 'P', 'u', 'r')
#define PROPH_AU_BUNDLE_ID       "com.chrispurusha.prophalike.au"

// notes §2: a block's notes, played at their own sample when the block renders
#define MAX_EVENTS               (256)

#define DEFAULT_TEMPO            (120.0) // notes §13: until the host gives one
#define CLIP_HOLD_S              (1.5)
#define HOST_CEILING             (0.97675f)
#define OUTPUT_HEADROOM          (0.5f)  // notes §16: 6 dB under the instrument's level, after its ceiling
#define HOST_CHUNK               (256u)
#define ENGINE_FRAMES            (4096u) // notes §12: HOST_CHUNK at a host rate down to 2.5 kHz

typedef struct {
    uint32_t offset;
    uint8_t  note;
    bool     on;
    float    velocity;
} tProphEvent;

typedef struct {
    tProphVoices       voices;
    tSynthLibResampler resampler;  // notes §12
    tSynthLibResampler resamplerR;
    tProphEffects      effects;
    double             hostRate;
    double             tempo;
    float              engine[ENGINE_FRAMES];
    float              engineL[ENGINE_FRAMES], engineR[ENGINE_FRAMES];
    tProphEvent        events[MAX_EVENTS];
    uint32_t           eventCount;
    double             values[kProphParamCount]; // normalized
    // notes §5: the sound, beyond its parameters - main thread, and the host's state calls
    char               soundName[PROPH_SINGLE_NAME_LENGTH + 1];
    bool               haveBase;
    uint8_t            base[PROPH_SINGLE_LENGTH]; // the Single it came from, whose bytes a save keeps
    char               patchPath[1024];
    int                patchIndex;                // its place in that file
    int                preset;                    // 1..PROPH_PRESETS, 0 none
    double             presetsChecked;
    tSynthLibSwapFade  fade;
    _Atomic bool       clipped;     // the audio thread saw the ceiling; the panel holds the lamp
    double             clipUntil;
    // notes §14: the link to the instrument
    int                hwLock;
    bool               hwSent, hwExtensionDue;
    double             hwSentAt;
    uint8_t            hwLast[PROPH_SINGLE_LENGTH];
    _Atomic bool       startPreset;  // notes: a new instance plays A000
    _Atomic int        wantPreset;   // a program change waiting for the fade
    _Atomic bool       clearEffects; // notes §15: a new sound starts with empty effects, the old one's echoes gone
    tSynthLibCtlPoints ctl;          // notes §17: this block's wheel and bend, each at its frame
} tProphPlugin;

static uint32_t voice_limit(const tProphPlugin * t) {
    return 1u + (uint32_t)lround(t->values[kProphParamVoiceLimit] * (PROPH_MAX_VOICES - 1));
}

static void * proph_create(const tSynthLibPluginDesc * desc) {
    tProphPlugin * t = (tProphPlugin *)calloc(1, sizeof(tProphPlugin));

    (void)desc;

    if (t == NULL) {
        return NULL;
    }

    for (uint32_t i = 0; i < kProphParamCount; i++) {
        t->values[i] = gProphParams[i].defaultNormalized;
    }

    t->hwLock      = -1;
    synthlib_swap_fade_init(&t->fade);
    proph_presets_refresh();
    t->startPreset = true;

    snprintf(t->soundName, sizeof(t->soundName), "Init");
    proph_voices_init(&t->voices);
    proph_voices_set_clean(&t->voices, t->values[kProphParamAntiAlias] >= 0.5);   // voices notes §17: On unless the host says otherwise
    proph_voices_set_fixed(&t->voices, t->values[kProphParamArithmetic] >= 0.5);
    proph_effects_reset(&t->effects);
    t->hostRate    = 48000.0;
    t->tempo       = DEFAULT_TEMPO;
    synthlib_resample_init(&t->resampler, PROPH_ENGINE_HZ, t->hostRate);
    synthlib_resample_init(&t->resamplerR, PROPH_ENGINE_HZ, t->hostRate);
    proph_voices_set_limit(&t->voices, voice_limit(t));
#ifndef SYNTHLIB_NO_EDITOR
    proph_prefs_init();
#endif
    synthlib_log_line("created - ProphAlike %s", PROPH_VERSION_STRING);
    return t;
}

static void proph_destroy(void * inst) {
    proph_hardware_release(&((tProphPlugin *)inst)->hwLock);
    synthlib_resample_free(&((tProphPlugin *)inst)->resampler);
    synthlib_resample_free(&((tProphPlugin *)inst)->resamplerR);
    free(inst);
}

// notes §5: the plug-in's own state, after the parameters
#define STATE_MAGIC     "TIS1"
#define STATE_LENGTH    (4 + (PROPH_SINGLE_NAME_LENGTH + 1) + 1 + PROPH_SINGLE_LENGTH + 1024 + 2 + 2)

static size_t proph_get_state(void * inst, void * out, size_t len) {
    tProphPlugin * t = (tProphPlugin *)inst;
    uint8_t *      o = (uint8_t *)out;

    if ((out == NULL) || (len < STATE_LENGTH)) {
        return STATE_LENGTH;
    }
    memcpy(o, STATE_MAGIC, 4);
    o   += 4;
    memcpy(o, t->soundName, PROPH_SINGLE_NAME_LENGTH + 1);
    o   += PROPH_SINGLE_NAME_LENGTH + 1;
    *o++ = t->haveBase ? 1 : 0;
    memcpy(o, t->base, PROPH_SINGLE_LENGTH);
    o   += PROPH_SINGLE_LENGTH;
    memcpy(o, t->patchPath, 1024);
    o   += 1024;
    *o++ = (uint8_t)(t->patchIndex >> 8);
    *o++ = (uint8_t)(t->patchIndex & 0xFF);
    *o++ = (uint8_t)(t->preset >> 8);
    *o   = (uint8_t)(t->preset & 0xFF);
    return STATE_LENGTH;
}

static void proph_set_state(void * inst, const void * data, size_t len) {
    tProphPlugin *  t = (tProphPlugin *)inst;
    const uint8_t * d = (const uint8_t *)data;

    if ((data == NULL) || (len < STATE_LENGTH) || (memcmp(d, STATE_MAGIC, 4) != 0)) {
        return;
    }
    atomic_store(&t->startPreset, false);   // a project's sound, not A001
    d                                     += 4;
    memcpy(t->soundName, d, PROPH_SINGLE_NAME_LENGTH + 1);
    t->soundName[PROPH_SINGLE_NAME_LENGTH] = '\0';
    d                                     += PROPH_SINGLE_NAME_LENGTH + 1;
    t->haveBase                            = (*d++ != 0);
    memcpy(t->base, d, PROPH_SINGLE_LENGTH);
    d                                     += PROPH_SINGLE_LENGTH;
    memcpy(t->patchPath, d, 1024);
    t->patchPath[1023]                     = '\0';
    d                                     += 1024;
    t->patchIndex                          = (d[0] << 8) | d[1];
    t->preset                              = (d[2] << 8) | d[3];
    t->preset                              = ((t->preset >= 1) && (t->preset <= PROPH_PRESETS)) ? t->preset : 0;
}

// notes §12: voices at their own rate, converted to the host's
static void render_host(tProphPlugin * t, const tProphSound * sound, float * outL, float * outR, uint32_t frames, double gain) {
    tProphEffectsSound effects = proph_effects_sound(t->values, t->tempo);

    if (atomic_exchange(&t->clearEffects, false)) {
        proph_effects_reset(&t->effects);   // notes §15
        proph_voices_all_off(&t->voices);   // the old sound's notes stop too, rather than playing on in the new one
    }

    while (frames > 0) {
        uint32_t chunk = (frames < HOST_CHUNK) ? frames : HOST_CHUNK;
        uint32_t need  = synthlib_resample_needed(&t->resampler, chunk);

        memset(t->engine, 0, (size_t)need * sizeof(float));
        proph_voices_render(&t->voices, sound, t->engine, need, gain);

        if (t->values[kProphParamEffectsBypass] > 0.5) {
            proph_effects_bypass(&t->effects, t->engine, t->engineL, t->engineR, need);
        } else {
            proph_effects_process(&t->effects, &effects, t->engine, t->engineL, t->engineR, need);
        }

        if (t->effects.clipped) {
            t->effects.clipped = false;

            if (!atomic_load(&t->fade.holding) && (t->fade.gain >= 1.0f)) {   // notes §15: what leaves the plug-in, not a block the swap fade holds down
                atomic_store(&t->clipped, true);
            }
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

static uint32_t proph_latency_samples(void * inst) {
    return ((tProphPlugin *)inst)->resampler.latency;
}

static void proph_prepare(void * inst, const tSynthLibSetup * setup) {
    tProphPlugin * t = (tProphPlugin *)inst;

    t->hostRate = setup->sampleRate;
    synthlib_resample_init(&t->resampler, PROPH_ENGINE_HZ, t->hostRate);
    synthlib_resample_init(&t->resamplerR, PROPH_ENGINE_HZ, t->hostRate);

    if (setup->offline) {
        atomic_store(&t->startPreset, false);   // an offline render plays what it is given
    }
}

static void proph_set_active(void * inst, bool active) {
    (void)active;
    ((tProphPlugin *)inst)->eventCount = 0;
    proph_voices_all_off(&((tProphPlugin *)inst)->voices);
}

static void proph_set_param(void * inst, uint32_t id, double normalized);

static tProphSound make_sound(const tProphPlugin * t) {
    tProphSound sound = (tProphSound){
        .osc            = {
            OSC_SOUND(t->values, kProphParamOscShape,  kProphParamPulseWidth, kProphParamOscSoften,  kProphParamOscLevel,  kProphParamOsc1Octave, kProphParamOsc1Semitone, kProphParamOsc1Fine),
            OSC_SOUND(t->values, kProphParamOsc2Shape, kProphParamOsc2Width,  kProphParamOsc2Soften, kProphParamOsc2Level, kProphParamOsc2Octave, kProphParamOsc2Semitone, kProphParamOsc2Fine),
            OSC_SOUND(t->values, kProphParamOsc3Shape, kProphParamOsc3Width,  kProphParamOsc3Soften, kProphParamOsc3Level, kProphParamOsc3Octave, kProphParamOsc3Semitone, kProphParamOsc3Fine),
        },
        .cutoff         = t->values[kProphParamCutoff] * 99.0,
        .resonance      = t->values[kProphParamResonance] * 99.0,
        .tracking       = t->values[kProphParamFilterTrack],
        .overdrive      = t->values[kProphParamFilterDrive] * 127.0,
        .normalise      = t->values[kProphParamFilterQ] * 127.0,
        .oscsFree       = t->values[kProphParamOscTrigger] > 0.5,
        .glideTime      = t->values[kProphParamGlideTime] * 127.0,
        .glideStyle     = (int)lround(t->values[kProphParamGlideStyle]),
        .glideCurve     = (int)lround(t->values[kProphParamGlideCurve]),
        .glideType      = (int)lround(t->values[kProphParamGlideType] * 9.0),
        .ampVelocity    = t->values[kProphParamAmpVelocity] * 127.0,
        .filterType     = (uint32_t)lround(t->values[kProphParamFilterType] * 4.0),
        .filterSlope    = (uint32_t)lround(t->values[kProphParamFilterSlope] * 2.0),
        .ampAttack      = t->values[kProphParamAmpAttack] * 99.0,
        .ampDecay       = t->values[kProphParamAmpDecay] * 99.0,
        .ampSustain     = t->values[kProphParamAmpSustain] * 99.0,
        .ampRelease     = t->values[kProphParamAmpRelease] * 99.0,
        .ampStart       = t->values[kProphParamAmpStart] * 99.0,
        .ampAttackLevel = t->values[kProphParamAmpAttackLevel] * 99.0,
        .ampBreak       = t->values[kProphParamAmpBreak] * 99.0,
        .ampSlope       = t->values[kProphParamAmpSlope] * 99.0,
        .ampSustainTime = t->values[kProphParamAmpSustainTime] * 127.0,
        .ampSustainRate = t->values[kProphParamAmpSustainRate] * 127.0,
        .noiseLevel     = t->values[kProphParamNoiseLevel] * 127.0 / 128.0,
        .noiseSoften    = t->values[kProphParamNoiseSoften] * 127.0,
        .ring13         = t->values[kProphParamRing13] * 127.0 / 128.0,
        .ring23         = t->values[kProphParamRing23] * 127.0 / 128.0,
        .env            = {MOD_ENV_SOUND(t->values, kProphParamEnv2Attack, kProphParamEnv2Decay, kProphParamEnv2Sustain, kProphParamEnv2Release, kProphParamEnv2Velocity, kProphParamEnv2Cutoff, kProphParamEnv2SustainTime, kProphParamEnv2SustainRate), MOD_ENV_SOUND(t->values, kProphParamEnv3Attack, kProphParamEnv3Decay, kProphParamEnv3Sustain, kProphParamEnv3Release, kProphParamEnv3Velocity, kProphParamEnv3Cutoff, kProphParamEnv3SustainTime, kProphParamEnv3SustainRate)},
    };

    proph_controls_sound(t->values, &sound);
    return sound;
}

static void play(tProphPlugin * t, const tProphSound * sound, const tProphEvent * e) {
    if (e->on) {
        proph_voices_note_on(&t->voices, sound, e->note, e->velocity);
    } else {
        proph_voices_note_off(&t->voices, sound, e->note);
    }
}

// notes §2: a full queue plays the note at once rather than lose it
static void queue(tProphPlugin * t, tProphEvent e) {
    if (t->eventCount < MAX_EVENTS) {
        t->events[t->eventCount++] = e;
    } else {
        tProphSound sound = make_sound(t);

        play(t, &sound, &e);
    }
}

static void proph_note_on(void * inst, uint8_t channel, uint8_t note, float velocity, uint32_t sampleOffset) {
    tProphPlugin * t = (tProphPlugin *)inst;

    (void)t;   // notes §14: the instrument takes its notes from the host, not from Link
    (void)channel;
    queue((tProphPlugin *)inst, (tProphEvent){sampleOffset, note, true, velocity});
}

static void proph_note_off(void * inst, uint8_t channel, uint8_t note, float velocity, uint32_t sampleOffset) {
    tProphPlugin * t = (tProphPlugin *)inst;

    (void)t;
    (void)channel;
    (void)velocity;
    queue((tProphPlugin *)inst, (tProphEvent){sampleOffset, note, false, 0.0f});
}

static void proph_load_single(void * user, const uint8_t * single, const char * path, int index, int preset);

// A new instance plays A000, first audio block or first editor frame; with no presets it stays on Init
static void take_start_preset(tProphPlugin * t) {
    if (atomic_exchange(&t->startPreset, false) && (proph_presets_single(1)[0] == 0xF0)) {
        proph_load_single(t, proph_presets_single(1), "", 0, 1);
    }
}

static void take_program_change(tProphPlugin * t) {
    int number = atomic_load(&t->wantPreset);

    if ((number != 0) && synthlib_swap_fade_silent(&t->fade)) {
        atomic_store(&t->wantPreset, 0);
        proph_load_single(t, proph_presets_single(number), "", 0, number);
        synthlib_swap_fade_end(&t->fade);
    }
}

static void proph_process(void * inst, const float *const * in, uint32_t numIn, float ** out, uint32_t numOut,
                          uint32_t frames, const tSynthLibTransport * transport) {
    tProphPlugin * t      = (tProphPlugin *)inst;

    take_start_preset(t);
    take_program_change(t);
    double         gain   = proph_volume_gain(t->values[kProphParamVolume]);
    double         master = t->values[kProphParamMaster];
    tProphSound    sound  = make_sound(t);

    (void)in;
    (void)numIn;

    if ((transport != NULL) && transport->tempoValid && (transport->tempo > 0.0)) {
        t->tempo = transport->tempo;    // notes §13: otherwise the last the host gave
    }

    if ((out == NULL) || (numOut == 0) || (out[0] == NULL)) {
        for (uint32_t k = 0; k < t->ctl.count; k++) {
            proph_set_param(t, t->ctl.point[k].id, t->ctl.point[k].value);
        }

        synthlib_ctl_points_clear(&t->ctl);

        for (uint32_t i = 0; i < t->eventCount; i++) {
            play(t, &sound, &t->events[i]);
        }

        t->eventCount = 0;
        return;
    }
    float *  left  = out[0];
    float *  right = (numOut > 1) ? out[1] : NULL;

    // notes §2: up to each note, then the note; the host sends them in time order
    uint32_t done  = 0;

    for (uint32_t i = 0; i <= t->eventCount; i++) {
        uint32_t          at = (i < t->eventCount) ? ((t->events[i].offset < frames) ? t->events[i].offset : frames) : frames;
        tSynthLibCtlPoint c;

        // notes §17: each wheel and bend value at its own frame
        while (synthlib_ctl_points_due(&t->ctl, at, &c)) {
            uint32_t w = (c.offset < frames) ? c.offset : frames;

            if (w > done) {
                render_host(t, &sound, left + done, (right != NULL) ? (right + done) : NULL, w - done, gain);
                done = w;
            }
            proph_set_param(t, c.id, c.value);
            sound = make_sound(t);
        }

        if (i == t->eventCount) {
            break;
        }

        if (at > done) {
            render_host(t, &sound, left + done, (right != NULL) ? (right + done) : NULL, at - done, gain);
            done = at;
        }
        play(t, &sound, &t->events[i]);
    }

    t->eventCount = 0;
    synthlib_ctl_points_clear(&t->ctl);

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

static void proph_set_param(void * inst, uint32_t id, double normalized) {
    tProphPlugin * t = (tProphPlugin *)inst;

    if (id >= kProphParamCount) {
        return;
    }
    t->values[id] = normalized;

    if (id == kProphParamProgram) {
        int bank   = (int)lround(t->values[kProphParamBank] * 127.0) % (PROPH_PRESETS / PROPH_BANK_SIZE);
        int number = (bank * PROPH_BANK_SIZE) + (int)lround(normalized * 127.0) + 1;

        // already on it, nothing happens, edits and all
        if ((number != t->preset) && (proph_presets_single(number)[0] == 0xF0)) {
            atomic_store(&t->wantPreset, number);
            synthlib_swap_fade_start(&t->fade);
        }
    } else if (id == kProphParamAntiAlias) {
        proph_voices_set_clean(&t->voices, normalized >= 0.5);   // voices notes §17
    } else if (id == kProphParamArithmetic) {
        proph_voices_set_fixed(&t->voices, normalized >= 0.5);   // filter notes §8
    } else if (id == kProphParamVoiceLimit) {
        proph_voices_set_limit(&t->voices, voice_limit(t));
    }
}

// notes §17: every value a block brings, at its frame - the performance controllers' kept for the block, the rest at once
static void proph_param_points(void * inst, uint32_t id, const tSynthLibParamPoint * points, uint32_t count) {
    tProphPlugin * t = (tProphPlugin *)inst;

    if ((count == 0u) || (id >= kProphParamCount)) {
        return;
    }

    if ((id == kProphParamModWheel) || (id == kProphParamPitchBend)) {
        synthlib_ctl_points_add(&t->ctl, id, points, count);
    } else {
        proph_set_param(inst, id, points[count - 1].value);
    }
}

static double proph_get_param(void * inst, uint32_t id) {
    return (id < kProphParamCount) ? ((tProphPlugin *)inst)->values[id] : 0.0;
}

static bool proph_param_text(const tSynthLibPluginDesc * desc, void * inst, uint32_t id, double value, char * out, size_t len) {
    static const char *const shapes[]  = {"Saw", "Square", "Double Saw"};
    static const char *const filters[] = {"LPF", "BPF", "HPF", "Res LPF", "Res BPF", "Res HPF", "Notch", "LPF+LPF", "BPF+BPF", "HPF+HPF", "LPF+BPF", "BPF+HPF"};
    static const char *const slopes[]  = {"12 dB", "18 dB", "24 dB"};

    (void)desc;
    (void)inst;

    if (id >= kProphParamCount) {
        return false;
    }
    long                     step      = lround(value * gProphParams[id].stepCount);

    if ((id == kProphParamOscShape) || (id == kProphParamOsc2Shape) || (id == kProphParamOsc3Shape)) {
        snprintf(out, len, "%s", shapes[(step < 0) ? 0 : ((step > 2) ? 2 : step)]);
    } else if (id == kProphParamFilterType) {
        snprintf(out, len, "%s", filters[(step < 0) ? 0 : ((step > 11) ? 11 : step)]);
    } else if (id == kProphParamChorusType) {
        static const char *const types[] = {"Quad Chorus", "Chorus/Flanger", "Phaser"};

        snprintf(out, len, "%s", types[(step < 0) ? 0 : ((step > 2) ? 2 : step)]);
    } else if (id == kProphParamGlideType) {
        static const char *const types[] = {"Normal", "Auto", "2 Semi Down", "2 Semi Up", "5 Semi Down", "5 Semi Up", "7 Semi Down", "7 Semi Up", "12 Semi Down", "12 Semi Up"};

        snprintf(out, len, "%s", types[(step < 0) ? 0 : ((step > 9) ? 9 : step)]);
    } else if (id == kProphParamGlideStyle) {
        snprintf(out, len, "%s", (step > 0) ? "Glissando" : "Portamento");
    } else if (id == kProphParamGlideCurve) {
        snprintf(out, len, "%s", (step > 0) ? "Exponential" : "Linear");
    } else if (id == kProphParamReverbType) {
        static const char *const types[] = {"Gated Reverse", "Gated Rising", "Gated Gentle", "Gated Falling", "Dry Chamber", "Echo Chamber", "Small Room", "Big Room", "Medium 1", "Medium 2", "Medium Plate 1", "Medium Plate 2", "Large 1", "Large 2", "Large Plate 1", "Large Plate 2"};

        snprintf(out, len, "%s", types[(step < 0) ? 0 : ((step > 15) ? 15 : step)]);
    } else if (id == kProphParamEffectsOrder) {
        static const char *const orders[] = {"D+R+C", "D>R>C", "D>C>R", "R>D>C", "R>C>D", "C>D>R", "C>R>D", "D+[R>C]", "D+[C>R]", "R+[D>C]", "R+[C>D]", "C+[D>R]", "C+[R>D]", "D>[R+C]", "R>[D+C]", "C>[D+R]", "[R+C]>D", "[D+C]>R", "[D+R]>C"};

        snprintf(out, len, "%s", orders[(step < 0) ? 0 : ((step > 18) ? 18 : step)]);
    } else if (id == kProphParamOscTrigger) {
        snprintf(out, len, "%s", (step > 0) ? "Ensemble" : "Percussive");
    } else if (id == kProphParamDelaySync) {
        static const char *const syncs[] = {"Off", "32nd T", "32nd", "16th T", "16th", "8th T", "16th .", "8th", "4th T", "8th .", "4th", "2nd T", "4th .", "2nd", "1 Bar T", "2nd .", "1 Bar", "2 Bar T", "1 Bar .", "2 Bars", "4 Bar T", "3 Bars", "5 Bar T", "4 Bars", "3 Bar .", "7 Bar T", "5 Bars", "8 Bar T", "6 Bars", "7 Bars", "5 Bar .", "8 Bars", "6 Bar .", "7 Bar .", "8 Bar ."};

        snprintf(out, len, "%s", syncs[(step < 0) ? 0 : ((step > 34) ? 34 : step)]);
    } else if (id == kProphParamDelayRatio) {
        static const char *const ratios[] = {"1:1", "1:0.75", "0.75:1", "1:0.66", "0.66:1", "1:0.5", "0.5:1", "1:0.33", "0.33:1", "1:0.25", "0.25:1", "1:Off", "Off:1"};

        snprintf(out, len, "%s", ratios[(step < 0) ? 0 : ((step > 12) ? 12 : step)]);
    } else if ((id == kProphParamDelayOn) || (id == kProphParamReverbOn)) {
        snprintf(out, len, "%s", (step > 0) ? "On" : "Off");
    } else if ((id == kProphParamLfo1Shape) || (id == kProphParamLfo2Shape)) {
        static const char *const shapes[] = {"Square", "Saw", "Triangle", "S+H"};

        snprintf(out, len, "%s", shapes[(step < 0) ? 0 : ((step > 3) ? 3 : step)]);
    } else if ((id == kProphParamLfo1Range) || (id == kProphParamLfo2Range)) {
        static const char *const ranges[] = {"x1", "x10", "x100"};

        snprintf(out, len, "%s", ranges[(step < 0) ? 0 : ((step > 2) ? 2 : step)]);
    } else if ((id == kProphParamLfo1Trigger) || (id == kProphParamLfo2Trigger)) {
        snprintf(out, len, "%s", (step > 0) ? "Key" : "Free");
    } else if ((id == kProphParamLfo1Fade) || (id == kProphParamLfo2Fade)) {
        snprintf(out, len, "%s", (step > 0) ? "Out" : "In");
    } else if ((id == kProphParamHwLink) || (id == kProphParamPanEffects)) {
        snprintf(out, len, "%s", (step > 0) ? "On" : "Off");
    } else if (id == kProphParamHwChannel) {
        snprintf(out, len, "%ld", step + 1);
    } else if (id == kProphParamEffectsBypass) {
        snprintf(out, len, "%s", (step > 0) ? "Bypass" : "On");
    } else if (id == kProphParamPolyMode) {
        snprintf(out, len, "%s", (step > 0) ? "Mono" : "Poly");
    } else if (id == kProphParamEnvTrigger) {
        snprintf(out, len, "%s", (step > 0) ? "Legato" : "Retrigger");
    } else if (id == kProphParamFilterSlope) {
        snprintf(out, len, "%s", slopes[(step < 0) ? 0 : ((step > 2) ? 2 : step)]);
    } else {
        snprintf(out, len, "%ld", lround(gProphParams[id].plainMin) + step);
    }
    return true;
}

// ---- the editor - notes §3: Pulse2Alike's panel view, with this plug-in's pages --------------------------------

// ---- Singles and presets - notes §5 ------------------------------------------------------------------------

// What a sound carries: everything but the plug-in's own settings
static bool sound_param(uint32_t id) {
    return (id != kProphParamVoiceLimit) && (id != kProphParamAntiAlias) && (id != kProphParamArithmetic) && (id != kProphParamSameNote) && (id != kProphParamMaster) && (id != kProphParamEffectsBypass) && (id != kProphParamSustainPedal)
           && (id != kProphParamHwLink) && (id != kProphParamHwChannel) && (id != kProphParamHwDevice) && (id != kProphParamProgram) && (id != kProphParamBank)
           && (id != kProphParamPitchBend) && (id != kProphParamModWheel);
}

static void current_values(void * user, double values[kProphParamCount]) {
    for (uint32_t id = 0; id < kProphParamCount; id++) {
        values[id] = synthlib_plugin_param_value(user, id);
    }
}

// The sound as it is now, as a Single: the host's values over the Single it came from
static void current_single(void * user, uint8_t single[PROPH_SINGLE_LENGTH]) {
    tProphPlugin * t = (tProphPlugin *)user;
    double         values[kProphParamCount];

    current_values(user, values);
    proph_patch_to_single(values, t->haveBase ? t->base : NULL, t->soundName, single);
}

// notes §6: every parameter the sound carries, through the host, so the change is one the host sees
static void proph_load_single(void * user, const uint8_t * single, const char * path, int index, int preset) {
    tProphPlugin * t = (tProphPlugin *)user;
    double         values[kProphParamCount];

    // the instance's own values, not the host's: asking the host takes a lock on VST3, and a program
    // change runs this on the audio thread
    memcpy(values, t->values, sizeof(values));
    proph_patch_from_single(single, values);
    atomic_store(&t->clearEffects, true);   // notes §15: before the new values, so the audio thread sees the change of sound first

    // The whole sound into the engine now, while the fade holds it silent. The host's copies come back
    // later - a block or more on VST3, and on an Audio Unit only once the main thread is free - so
    // waiting for them let the fade open on half the old sound and half the new.
    for (uint32_t id = 0; id < kProphParamCount; id++) {
        if (sound_param(id)) {
            t->values[id] = values[id];
            synthlib_plugin_param_edited(user, id, values[id]);
        }
    }

    memcpy(t->base, single, PROPH_SINGLE_LENGTH);
    t->haveBase   = true;
    proph_patch_name(single, t->soundName);
    snprintf(t->patchPath, sizeof(t->patchPath), "%s", path);
    t->patchIndex = index;
    t->preset     = preset;
}

static bool proph_save_patch(void * user, const char * path) {
    tProphPlugin * t = (tProphPlugin *)user;
    uint8_t        single[PROPH_SINGLE_LENGTH];

    current_single(user, single);

    if (!proph_patch_save_file(path, single)) {
        synthlib_log_line("could not write %s", path);
        return false;
    }
    memcpy(t->base, single, PROPH_SINGLE_LENGTH);
    t->haveBase   = true;
    snprintf(t->patchPath, sizeof(t->patchPath), "%s", path);
    t->patchIndex = 0;
    t->preset     = 0;
    return true;
}

static void proph_load_faded(void * user, const uint8_t * single, const char * path, int index, int preset) {
    tProphPlugin * t = (tProphPlugin *)user;

    synthlib_swap_fade_begin(&t->fade);
    proph_load_single(user, single, path, index, preset);
    synthlib_swap_fade_end(&t->fade);
    synthlib_log_line("loaded \"%s\" (%s #%d, preset %d)", t->soundName, path, index, preset);
}

static bool proph_store_preset(void * user, int number) {
    tProphPlugin * t = (tProphPlugin *)user;
    uint8_t        single[PROPH_SINGLE_LENGTH];

    current_single(user, single);

    if (!proph_presets_store(number, single)) {
        return false;
    }
    memcpy(t->base, proph_presets_single(number), PROPH_SINGLE_LENGTH);
    t->haveBase     = true;
    t->patchPath[0] = '\0';
    t->patchIndex   = 0;
    t->preset       = number;
    return true;
}

// every sound parameter to its default; the next save has no Single to keep bytes from
static void proph_init_patch(void * user) {
    tProphPlugin * t = (tProphPlugin *)user;

    for (uint32_t id = 0; id < kProphParamCount; id++) {
        if (sound_param(id)) {
            synthlib_plugin_param_edited(user, id, gProphParams[id].defaultNormalized);
        }
    }

    t->haveBase     = false;
    t->patchPath[0] = '\0';
    t->patchIndex   = 0;
    t->preset       = 0;
    snprintf(t->soundName, sizeof(t->soundName), "Init");
}

static const char * proph_sound_name(void * user) {
    return ((tProphPlugin *)user)->soundName;
}

// notes §5: the name only - written to a Single at the next save or store
static void proph_rename(void * user, const char * name) {
    snprintf(((tProphPlugin *)user)->soundName, PROPH_SINGLE_NAME_LENGTH + 1, "%s", name);
}

// notes §7: edited, if a parameter it carries or its name differs from the stored preset
static bool preset_edited(tProphPlugin * t, void * user) {
    double values[kProphParamCount], stored[kProphParamCount];
    char   name[PROPH_SINGLE_NAME_LENGTH + 1];

    if (t->preset == 0) {
        return false;
    }
    current_values(user, values);
    memcpy(stored, values, sizeof(stored));
    proph_patch_from_single(proph_presets_single(t->preset), stored);
    proph_patch_name(proph_presets_single(t->preset), name);

    for (uint32_t id = 0; id < kProphParamCount; id++) {
        if (sound_param(id) && (fabs(stored[id] - values[id]) > 1e-6)) {
            return true;
        }
    }

    return strcmp(name, t->soundName) != 0;
}

#define LINK_MIN_SECONDS    (0.5)    // notes §14: a whole sound, at most this often
#define LINK_EXT_SECONDS    (0.2)    // the extension after the main message

#ifndef SYNTHLIB_NO_EDITOR   // an editor-less build (the MPC) leaves out the panel and the instrument link

static double now_seconds(void) {
    struct timespec ts;

    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec + (ts.tv_nsec * 1e-9);
}

// notes §14: the instrument follows the sound, whole; notes and controllers come from the host
static void hardware_link(tProphPlugin * t, void * user) {
    if (synthlib_plugin_param_value(user, kProphParamHwLink) < 0.5) {
        proph_hardware_release(&t->hwLock);
        t->hwSent         = false;
        t->hwExtensionDue = false;
        proph_draw_set_link_busy(false);
        return;
    }
    bool    busy   = !proph_hardware_claim(&t->hwLock);

    proph_draw_set_link_busy(busy);

    if (busy) {
        t->hwSent = false;      // all of the sound once the instrument is free
        return;
    }
    uint8_t single[PROPH_SINGLE_LENGTH];
    uint8_t device = (uint8_t)lround(synthlib_plugin_param_value(user, kProphParamHwDevice) * 127.0);

    if (t->hwExtensionDue) {
        if ((now_seconds() - t->hwSentAt) >= LINK_EXT_SECONDS) {
            (void)proph_hardware_send_extension(t->hwLast, proph_hw_port(), device);
            t->hwExtensionDue = false;
        }
        return;
    }
    current_single(user, single);

    if ((!t->hwSent || (memcmp(single, t->hwLast, PROPH_SINGLE_LENGTH) != 0)) && ((now_seconds() - t->hwSentAt) >= LINK_MIN_SECONDS)) {
        tProphHardwareResult r = proph_hardware_send_main(single, proph_hw_port(), device);

        memcpy(t->hwLast, single, PROPH_SINGLE_LENGTH);
        t->hwSentAt       = now_seconds();
        t->hwSent         = (r == eProphHardwareSent);
        t->hwExtensionDue = t->hwSent;
        synthlib_log_line("Link: sound sent to \"%s\" (result %d)", proph_hw_port(), (int)r);
    }
    // notes §14: bend, wheel and pedal reach the instrument from the host with the notes, not from Link
}

static const tProphPatchHost gPatchHost = {
    .load = proph_load_faded, .save   = proph_save_patch, .storePreset = proph_store_preset, .init = proph_init_patch,
    .name = proph_sound_name, .rename = proph_rename,
};

// In mono the voice limit means nothing: the dial says so
static bool limit_display(uint32_t paramId, const double * values, int32_t * step, char * text, size_t len) {
    if ((paramId != kProphParamVoiceLimit) || (values[kProphParamPolyMode] <= 0.5)) {
        return false;
    }
    *step = 0;
    snprintf(text, len, "Mono");
    return true;
}

static void proph_panel_init(void) {
    proph_draw_init();

    for (uint32_t i = 0; i < kProphParamCount; i++) {
        proph_draw_set_default(gProphParams[i].id, gProphParams[i].defaultNormalized);
    }

    proph_draw_set_unity(kProphParamFilterTrack, 127, 127);   // draw notes §15

    proph_draw_set_pages(gProphPages, gProphPageCount);
    proph_draw_set_host(&gPatchHost);
    proph_draw_set_display(limit_display);
}

// notes: a plain-text channel, off until /tmp/prophalike-backdoor exists
static double bd_get(void * user, uint32_t id) {
    return proph_get_param(user, id);
}

static void bd_set(void * user, uint32_t id, double normalized) {
    proph_set_param(user, id, normalized);
    synthlib_plugin_param_edited(user, id, normalized);
}

static bool bd_text(void * user, uint32_t id, double normalized, char * out, size_t len) {
    return proph_param_text(NULL, user, id, normalized, out, len);
}

static bool bd_preset(void * user, int number) {
    if ((number < 1) || (number > PROPH_PRESETS)) {
        return false;
    }
    (void)proph_presets_refresh();
    proph_load_faded(user, proph_presets_single(number), "", 0, number);
    return true;
}

static const tSynthLibBackdoor gBackdoor = {
    .params = gProphParams,     .count  = kProphParamCount,
    .get    = bd_get,           .set    = bd_set,          .text  = bd_text,
    .name   = proph_sound_name, .rename = proph_rename,
    .preset = bd_preset,        .save   = proph_save_patch,.init  = proph_init_patch,
};

static void proph_panel_sync(void * user) {
    synthlib_backdoor_poll("prophalike", &gBackdoor, user);
    tProphPlugin * t  = (tProphPlugin *)user;

    proph_draw_set_user(user);

    if (atomic_exchange(&t->clipped, false)) {
        t->clipUntil = now_seconds() + CLIP_HOLD_S;
    }
    proph_draw_set_clip(now_seconds() < t->clipUntil);
    hardware_link(t, user);
    proph_draw_read_tick();
    int            id = proph_hardware_identified(); // hardware notes §4

    if (id >= 0) {
        proph_set_param(user, kProphParamHwDevice, (double)id / 127.0);
        synthlib_plugin_param_edited(user, kProphParamHwDevice, (double)id / 127.0);
    }

    // another instance's store shows within a second
    if ((now_seconds() - t->presetsChecked) >= 1.0) {
        t->presetsChecked = now_seconds();
        (void)proph_presets_refresh();
    }
    proph_draw_set_patch(t->patchPath, t->patchIndex);   // this instance's, every sync
    proph_draw_set_preset(t->preset, preset_edited(t, user));

    take_start_preset((tProphPlugin *)user);

    for (uint32_t id = 0; id < kProphParamCount; id++) {
        proph_draw_set_value(id, synthlib_plugin_param_value(user, id));
    }
}

static void proph_panel_frame(void * user, int pixelWidth, int pixelHeight) {
    (void)user;
    proph_draw_frame(pixelWidth, pixelHeight);
}

static bool proph_panel_press(void * user, double x, double y) {
    tProphEdit edit;
    bool       grabbed = false;

    (void)proph_draw_press(x, y, &edit, &grabbed);

    if (edit.paramId != UINT32_MAX) {
        synthlib_plugin_param_edited(user, edit.paramId, edit.normalized);
    }
    return grabbed;
}

static bool proph_panel_keeps_pointer(void * user) {
    (void)user;
    return proph_dial_mode() == eDialModeRotary;
}

static void proph_panel_drag_to(void * user, double x, double y) {
    tProphEdit edit;

    if (proph_draw_drag_to(x, y, &edit)) {
        synthlib_plugin_param_edited(user, edit.paramId, edit.normalized);
    }
}

static void proph_panel_drag_by(void * user, double dx, double dy) {
    tProphEdit edit;

    if (proph_draw_drag_by(dx, dy, &edit)) {
        synthlib_plugin_param_edited(user, edit.paramId, edit.normalized);
    }
}

static void proph_panel_release(void * user, double x, double y) {
    tProphEdit edit;

    proph_draw_release(x, y, &edit);

    if (edit.paramId != UINT32_MAX) {
        synthlib_plugin_param_edited(user, edit.paramId, edit.normalized);
    }
}

static bool proph_panel_key(void * user, int glfwKey, unsigned int codepoint, bool isRepeat) {
    (void)user;
    return proph_draw_key(glfwKey, codepoint, isRepeat);
}

static void proph_panel_scroll(void * user, double x, double y, double dy) {
    (void)user;
    (void)x;
    (void)y;
    proph_draw_scroll(dy);
}

static const tSynthLibPanel      gPanel = {
    .canvasWidth  = PROPH_CANVAS_W,
    .init         = proph_panel_init,
    .sync         = proph_panel_sync,
    .frame        = proph_panel_frame,
    .press        = proph_panel_press,
    .dragBy       = proph_panel_drag_by,
    .keepsPointer = proph_panel_keeps_pointer,
    .dragTo       = proph_panel_drag_to,
    .release      = proph_panel_release,
    .scroll       = proph_panel_scroll,
    .key          = proph_panel_key,
    .pointer      = proph_draw_set_mouse,
    .menuActive   = proph_draw_menu_active,
};

static void * proph_create_view(const tSynthLibPluginDesc * desc, void * inst, double width, double height) {
    (void)desc;
    return synthlib_panel_view_create(&gPanel, inst, width, height);
}
#endif // SYNTHLIB_NO_EDITOR

static const tSynthLibBus        gOutputs[1] = {{"Out", 2, false, true}};

static const tSynthLibPluginDesc gDescriptor = {
    .name               = "ProphAlike",
    .vendor             = "Chris Purusha",
    .url                = "https://github.com/chrispurusha/ProphAlike",
    .email              = "",
    .version            = PROPH_VERSION_STRING,

    .isInstrument       = true,
    .vst3SubCategory    = "Instrument|Synth",
    .outputs            = gOutputs,
    .numOutputs         =                                            1,
    .wantsMidiIn        = true,

    .vst3ProcessorUid   = gProcessorUid,
    .vst3ControllerUid  = gControllerUid,
    .auType             = PROPH_AU_TYPE,
    .auSubType          = PROPH_AU_SUBTYPE,
    .auManufacturer     = PROPH_AU_MANUFACTURER,
    .auVersion          = PROPH_AU_VERSION,
    .auBundleId         = PROPH_AU_BUNDLE_ID,

    .params             = gProphParams,
    .numParams          = kProphParamCount,

    .editorDefaultWidth = PROPH_CANVAS_W,
    .editorMinWidth     = PROPH_CANVAS_W * 0.75,
    .editorMaxWidth     = PROPH_CANVAS_W * 2.0,
    .editorAspect       = PROPH_CANVAS_W / PROPH_CANVAS_H,

    .cb                 = {
        .create         = proph_create,
        .destroy        = proph_destroy,
        .prepare        = proph_prepare,
        .latencySamples = proph_latency_samples,
        .setActive      = proph_set_active,
        .process        = proph_process,
        .noteOn         = proph_note_on,
        .noteOff        = proph_note_off,
        .setParam       = proph_set_param,
        .paramPoints    = proph_param_points,
        .getParam       = proph_get_param,
        .paramText      = proph_param_text,
        .getState       = proph_get_state,
        .setState       = proph_set_state,
#ifndef SYNTHLIB_NO_EDITOR
        .createView     = proph_create_view,
#endif
    }
};

static const tSynthLibPluginSet  gSet = {
    .variants = &gDescriptor,
    .count    = 1
};

static const tSynthLibPluginSet * plugin_variants(void) {
    return &gSet;
}

SYNTHLIB_PLUGIN_REGISTER(plugin_variants, "prophalike")
