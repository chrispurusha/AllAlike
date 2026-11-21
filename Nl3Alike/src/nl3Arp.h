/*
 * Nl3Alike - the arpeggiator: the held keys stepped through as the instrument does, Sub Arpeggio and Mask included.
 *
 * Copyright (C) 2026 Chris Turner <chris_purusha@icloud.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
// Notes: Docs/code-notes/nl3Arp.md - "// notes §k" refers there.

#ifndef __NL3_ARP_H__
#define __NL3_ARP_H__

#include <stdbool.h>
#include <stdint.h>

#define NL3_ARP_KEYS          (20)   // notes §3: the instrument's list holds 20
#define NL3_ARP_MASK_STEPS    (16)

// the directions; Still is the Sub Arpeggio's only
enum {
    eNl3ArpUp,
    eNl3ArpDown,
    eNl3ArpUpDown,
    eNl3ArpRandom,
    eNl3ArpStill
};

// notes §1: the program's arpeggiator settings, as plain values
typedef struct {
    bool     run;
    int      direction;      // Up, Down, Up/Down, Random
    int      octaves;        // 1-4
    bool     sub, mask;      // the Mode: Normal, Sub, Mask, Sub+Mask
    int      subDirection;   // Up, Down, Up/Down, Random, Still
    int      subRange;       // 1-8 notes
    int      maskLength;     // 1-16 steps
    uint16_t maskOn;         // bit k-1: step k plays
    bool     clockSync;
    double   rate;           // the Rate knob, 0-127
    int      divisor;        // the clock divisor, 0-127
    int      voiceMode;      // notes §3: 0 poly, 1 mono, 2 mono legato, 3 poly legato
    int      priority;       // notes §4: Mono's note priority - 0 off, 1 low, 2 high
} tNl3ArpSettings;

// notes §5: where the host's beat is, for clock sync
typedef struct {
    double hostRate;
    double tempo;            // BPM
    bool   locked;           // beats is known: the host's position, or a count run on at its tempo while it is stopped
    double beats;            // quarter notes, at this moment
} tNl3ArpClock;

typedef void (*tNl3ArpNote)(void * user, uint8_t note, float velocity, bool on);

typedef struct {
    uint8_t  key[NL3_ARP_KEYS];      // held, lowest first
    float    velocity[NL3_ARP_KEYS];
    int      keys;
    bool     running;
    double   untilStep;              // host frames to the next step
    int32_t  count;                  // notes §3: the step count, from 0 at the first key; pattern, Sub and Mask all follow it
    int      randomAt;               // notes §4: Random draws a new place only when the position moves
    int      randomIndex;
    int      sounding;               // the note playing, -1 none
    uint32_t random;                 // notes §4: the instrument's generator
} tNl3Arp;

void nl3_arp_init(tNl3Arp * a);
double nl3_arp_step_seconds(const tNl3ArpSettings * s, double tempo);   // notes §2
// a key pressed or let go - the first key starts the arpeggio at once
void nl3_arp_key(tNl3Arp * a, const tNl3ArpSettings * s, const tNl3ArpClock * c, uint8_t note, float velocity, bool on, tNl3ArpNote play, void * user);
// host frames until the next step, as a whole number to render first; UINT32_MAX when nothing runs
uint32_t nl3_arp_frames_to_step(const tNl3Arp * a);
void nl3_arp_elapse(tNl3Arp * a, uint32_t frames);
void nl3_arp_step(tNl3Arp * a, const tNl3ArpSettings * s, const tNl3ArpClock * c, tNl3ArpNote play, void * user);
// Run switched off: the note playing ends and the keys are forgotten
void nl3_arp_stop(tNl3Arp * a, tNl3ArpNote play, void * user);

#endif // __NL3_ARP_H__
