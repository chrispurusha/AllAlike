/*
 * OneShot - a one-sample player, as a VST3 and Audio Unit instrument.
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
// Notes: Docs/code-notes/osVoice.md - "// notes §k" refers there.

#ifndef __OS_VOICE_H__
#define __OS_VOICE_H__

#include <stdbool.h>
#include <stdint.h>

#include "osEnvelope.h"
#include "osCache.h"
#include "osSample.h"

#define OS_VOICES    (8)

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
    tOsEnvShape env;
    double      start;       // seconds into the sample
    bool        snap;        // to the nearest zero crossing
    bool        reverse;     // notes §6: the sample backwards; Start counts from its end
    double      stretch;     // notes §8: the length's multiple, tempo included; 1 is off
    double      protect;     // seconds of the original kept as they are when stretched
    bool        protectSnap; // notes §11: its end to the nearest zero crossing (its own switch, not Snap)
    double      end;         // notes §9: where a note stops, 0..1 of the length as played; 1 the whole
    double      delay;       // notes §10: seconds a note waits before it sounds
} tOsPlay;

typedef struct {
    const tOsSample *     sample;
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
    tOsEnv                env;
    const tOsCacheEntry * copy;     // notes §7: the resampled copy read instead, held while the voice plays
    bool                  reverse;
    uint8_t               note;
    bool                  active;
    uint32_t              age;     // for stealing the oldest
} tOsVoice;

typedef struct {
    tOsVoice   voice[OS_VOICES];
    double     hostRate;
    uint32_t   started;
    double     outL, outR;    // notes §4: the output gains (pan x volume), smoothed toward the parameters
    bool       outSet;
    tOsCache * cache;         // notes §7: NULL reads every note live
} tOsVoices;

void os_voices_init(tOsVoices * v, double hostRate);
void os_voices_set_cache(tOsVoices * v, tOsCache * cache);

// notes §7: the step a note plays at - what a copy is resampled for
double os_voices_step(const tOsVoices * v, const tOsSample * sample, const tOsPlay * play, uint8_t note);
bool os_voices_stretched(const tOsPlay * play);
tOsCacheKey os_voices_key(const tOsVoices * v, const tOsSample * sample, const tOsPlay * play, uint8_t note);   // the copy a note wants
void os_voices_stop(tOsVoices * v);
void os_voices_note_on(tOsVoices * v, const tOsSample * sample, const tOsPlay * play, uint8_t note, float velocity);
void os_voices_note_off(tOsVoices * v, const tOsPlay * play, uint8_t note);
void os_voices_fade_all(tOsVoices * v);

// notes §5: where a note starts in the sample, in its frames - Start, snapped as the settings ask
uint32_t os_voices_start_frame(const tOsSample * sample, const tOsPlay * play);
uint32_t os_voices_end_frame(const tOsSample * sample, const tOsPlay * play);
uint32_t os_voices_protect_frame(const tOsSample * sample, const tOsPlay * play);   // notes §11: snapped, as played   // notes §9: frames as played; frames = the whole   // a change of sample: every voice ends, without a click

// Adds into left and right
void os_voices_render(tOsVoices * v, const tOsPlay * play, float * left, float * right, uint32_t frames);

// true if any sounding voice still plays something other than sample - what must stay in memory
bool os_voices_use_other_than(const tOsVoices * v, const tOsSample * sample);

#endif // __OS_VOICE_H__
