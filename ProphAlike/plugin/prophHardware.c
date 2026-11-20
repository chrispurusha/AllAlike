/*
 * ProphAlike - Hardware link placeholder: all hardware sends are disabled until the Prophecy protocol is modelled.
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

// This placeholder keeps the hardware interface inert on every platform until its protocol is implemented.
// The claim always succeeds: with nothing sent, two instances cannot fight over the instrument.

#include <stdio.h>
#include <string.h>

#include "prophHardware.h"

bool proph_hardware_claim(int * lock) {
    *lock = 0;
    return true;
}

void proph_hardware_release(int * lock) {
    *lock = -1;
}

int proph_hardware_port_count(void) {
    return 0;
}

bool proph_hardware_port_name(int index, char * out, size_t len) {
    (void)index;

    if (len > 0) {
        out[0] = 0;
    }
    return false;
}

tProphHardwareResult proph_hardware_send_main(const uint8_t single[PROPH_SINGLE_LENGTH], const char * port, uint8_t device) {
    (void)single;
    (void)port;
    (void)device;
    return eProphHardwareNoMidi;
}

tProphHardwareResult proph_hardware_send_extension(const uint8_t single[PROPH_SINGLE_LENGTH], const char * port, uint8_t device) {
    (void)single;
    (void)port;
    (void)device;
    return eProphHardwareNoMidi;
}

tProphHardwareResult proph_hardware_read_start(const char * port, uint8_t device) {
    (void)port;
    (void)device;
    return eProphHardwareNoMidi;
}

tProphReadState proph_hardware_read_state(void) {
    return eProphReadIdle;
}

int proph_hardware_read_programs(void) {
    return 0;
}

bool proph_hardware_read_save(const char * path) {
    (void)path;
    return false;
}

void proph_hardware_read_stop(void) {
}

void proph_hardware_short(const char * port, uint8_t status, uint8_t data1, uint8_t data2) {
    (void)port;
    (void)status;
    (void)data1;
    (void)data2;
}

tProphHardwareResult proph_hardware_identify_start(const char * port) {
    (void)port;
    return eProphHardwareNoMidi;
}

int proph_hardware_identified(void) {
    return -1;
}
