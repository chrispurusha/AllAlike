/*
 * UltraAlike - a sampler (a placeholder on a sample player's engine), as a VST3 and Audio Unit instrument.
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
// Notes: Docs/code-notes/uaVoice.md - "// notes §k" refers there.

#ifndef __UA_VOICE_H__
#define __UA_VOICE_H__

#include <stdbool.h>
#include <stdint.h>

#include "uaEnvelope.h"
#include "uaCache.h"
#include "uaSample.h"

#define UA_VOICES    (8)

// What a note plays with, read from the parameters once a block
typedef struct {
    double      tune;     // semitones, fine included
    double      velocity; // 0..1: how much velocity moves the level
    bool        keyTrack;
    int         rootKey;
    bool        gate;        // a note off starts the release; otherwise the note off is ignored
    bool        mono;        // a new note fades out the ones still sounding
    double      pan;         // -1..1
    double      gain;        // linear
    tUaEnvShape env;
    double      start;       // seconds into the sample
    bool        snap;        // to the nearest zero crossing
    bool        reverse;     // notes §6: the sample backwards; Start counts from its end
    double      stretch;     // notes §8: the length's multiple, tempo included; 1 is off
    double      protect;     // seconds of the original kept as they are when stretched
    bool        protectSnap; // notes §11: its end to the nearest zero crossing (its own switch, not Snap)
    double      end;         // notes §9: where a note stops, 0..1 of the length as played; 1 the whole
    double      delay;       // notes §10: seconds a note waits before it sounds
} tUaPlay;

typedef struct {
    const tUaSample *     sample;
    double                position; // in the sample's frames as played (reversed when reverse), or the copy's
    double                step;     // frames per output frame; 1 through a copy
    double                cutoff;   // the interpolator's bandwidth for that step
    double                end;      // the position at which it has finished ringing
    double                level;    // velocity's part
    double                fade;     // 1 while sounding; falls to 0 when ended
    double                fadeStep; // per output frame, 0 while not fading
    double                rise;     // notes §5: a 1 ms fade-in for an unsnapped start, 1 once done
    double                riseStep;
    double                stop;     // notes §9: End, in position's units; a note ends here
    double                stopFrom; // where its 1 ms fade-out starts (Snap Off), or stop
    uint32_t              wait;     // notes §10: output frames still to wait before it sounds
    tUaEnv                env;
    const tUaCacheEntry * copy;     // notes §7: the resampled copy read instead, held while the voice plays
    bool                  reverse;
    uint8_t               note;
    bool                  active;
    uint32_t              age;     // for stealing the oldest
} tUaVoice;

typedef struct {
    tUaVoice   voice[UA_VOICES];
    double     hostRate;
    uint32_t   started;
    double     outL, outR;    // notes §4: the output gains (pan x volume), smoothed toward the parameters
    bool       outSet;
    tUaCache * cache;         // notes §7: NULL reads every note live
    bool       offline;       // notes §12: a bounce - a missing copy is built before the note plays
} tUaVoices;

void ua_voices_init(tUaVoices * v, double hostRate);
void ua_voices_set_cache(tUaVoices * v, tUaCache * cache);
void ua_voices_set_offline(tUaVoices * v, bool offline);

// notes §7: the step a note plays at - what a copy is resampled for
double ua_voices_step(const tUaVoices * v, const tUaSample * sample, const tUaPlay * play, uint8_t note);
bool ua_voices_stretched(const tUaPlay * play);
tUaCacheKey ua_voices_key(const tUaVoices * v, const tUaSample * sample, const tUaPlay * play, uint8_t note);   // the copy a note wants
void ua_voices_stop(tUaVoices * v);
void ua_voices_note_on(tUaVoices * v, const tUaSample * sample, const tUaPlay * play, uint8_t note, float velocity);
void ua_voices_note_off(tUaVoices * v, const tUaPlay * play, uint8_t note);
void ua_voices_fade_all(tUaVoices * v);

// notes §5: where a note starts in the sample, in its frames - Start, snapped as the settings ask
uint32_t ua_voices_start_frame(const tUaSample * sample, const tUaPlay * play);
uint32_t ua_voices_end_frame(const tUaSample * sample, const tUaPlay * play);
uint32_t ua_voices_protect_frame(const tUaSample * sample, const tUaPlay * play);   // notes §11: snapped, as played   // notes §9: frames as played; frames = the whole   // a change of sample: every voice ends, without a click

// Adds into left and right
void ua_voices_render(tUaVoices * v, const tUaPlay * play, float * left, float * right, uint32_t frames);

// true if any sounding voice still plays something other than sample - what must stay in memory
bool ua_voices_use_other_than(const tUaVoices * v, const tUaSample * sample);

#endif // __UA_VOICE_H__
