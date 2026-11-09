/*
 * Sn2Alike - the link to the instrument on Windows: not yet. Every send reports no MIDI.
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

// The Windows build compiles this in place of sn2Hardware.c, whose CoreMIDI has no Windows counterpart yet.
// The claim always succeeds: with nothing sent, two instances cannot fight over the instrument.

#include <stdio.h>
#include <string.h>

#include "sn2Hardware.h"

bool sn2_hardware_claim(int * lock) {
    *lock = 0;
    return true;
}

void sn2_hardware_release(int * lock) {
    *lock = -1;
}

int sn2_hardware_port_count(void) {
    return 0;
}

bool sn2_hardware_port_name(int index, char * out, size_t len) {
    (void)index;

    if (len > 0) {
        out[0] = 0;
    }
    return false;
}

tSn2HardwareResult sn2_hardware_send_main(const uint8_t single[SN2_SINGLE_LENGTH], const char * port, uint8_t device) {
    (void)single;
    (void)port;
    (void)device;
    return eSn2HardwareNoMidi;
}

tSn2HardwareResult sn2_hardware_send_extension(const uint8_t single[SN2_SINGLE_LENGTH], const char * port, uint8_t device) {
    (void)single;
    (void)port;
    (void)device;
    return eSn2HardwareNoMidi;
}

tSn2HardwareResult sn2_hardware_read_start(const char * port, uint8_t device) {
    (void)port;
    (void)device;
    return eSn2HardwareNoMidi;
}

tSn2ReadState sn2_hardware_read_state(void) {
    return eSn2ReadIdle;
}

int sn2_hardware_read_programs(void) {
    return 0;
}

bool sn2_hardware_read_save(const char * path) {
    (void)path;
    return false;
}

void sn2_hardware_read_stop(void) {
}

void sn2_hardware_short(const char * port, uint8_t status, uint8_t data1, uint8_t data2) {
    (void)port;
    (void)status;
    (void)data1;
    (void)data2;
}

tSn2HardwareResult sn2_hardware_identify_start(const char * port) {
    (void)port;
    return eSn2HardwareNoMidi;
}

int sn2_hardware_identified(void) {
    return -1;
}
