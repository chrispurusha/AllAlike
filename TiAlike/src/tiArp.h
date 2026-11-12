/*
 * TiAlike - the arpeggiator.
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
// Notes: Docs/code-notes/tiArp.md - "// notes §k" refers there.

#ifndef __TI_ARP_H__
#define __TI_ARP_H__

#include <stdbool.h>
#include <stdint.h>

#define TI_ARP_KEYS      (32)
#define TI_ARP_EVENTS    (64)

// notes §2: one step of a pattern - note length and duration in 1/65536ths of a step, velocity 0 (a rest) to 127
typedef struct {
    int32_t length, duration;
    uint8_t velocity;
} tTiArpStep;

extern const tTiArpStep gTiArpSteps[];
extern const uint16_t   gTiArpPatternStart[64];   // pattern p (1-63) is steps [start[p-1], start[p])

// A note the arpeggiator starts or stops, at a frame of the block
typedef struct {
    uint32_t frame;
    uint8_t  note;
    float    velocity;
    bool     on;
} tTiArpEvent;

typedef struct {
    uint8_t  key[TI_ARP_KEYS];           // in the order played
    float    keyVelocity[TI_ARP_KEYS];
    bool     keyDown[TI_ARP_KEYS];       // false: held only by Hold
    uint32_t keys;
    bool     running;
    double   clock;                      // ticks since it was reset, 96 a quarter note, running always
    double   origin;                     // the tick the pattern started on
    double   tick;                       // ticks since the pattern started
    int64_t  nextTick;                   // where the next step starts
    int64_t  stepTicks;                  // where the current step started (unswung)
    uint32_t entry;                      // the next step of the pattern
    int      octave;
    int      last;                       // the last key chosen (without its octave)
    bool     down;                       // Up&Down's direction
    uint32_t played;                     // As Played's place in the keys
    uint8_t  sounding[TI_ARP_KEYS];      // notes started and not yet stopped
    uint32_t soundingCount;
    double   offTick;                    // when they stop
    uint32_t random;
    bool     pedal;                      // notes §1: the sustain pedal, which holds as Hold does
} tTiArp;

void ti_arp_reset(tTiArp * a);
// notes §1: the mode from a Single's bytes - 0 off, 1 Up ... 6 Chord, 7 Arp>Matrix
int ti_arp_mode(const uint8_t * pageA, const uint8_t * pageB);
void ti_arp_key(tTiArp * a, const uint8_t * pageB, uint8_t note, float velocity, bool down);
// Runs the arpeggiator over a block; fills events (at most TI_ARP_EVENTS) and returns how many
uint32_t ti_arp_run(tTiArp * a, const uint8_t * pageA, const uint8_t * pageB, const uint8_t * pageC, const uint8_t * pageD, uint32_t frames, double rate, double bpm, tTiArpEvent * events);
// The notes it has sounding, to stop them all at once
uint32_t ti_arp_stop(tTiArp * a, tTiArpEvent * events);

#endif // __TI_ARP_H__
