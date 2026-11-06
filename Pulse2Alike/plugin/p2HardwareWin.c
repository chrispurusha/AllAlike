/*
 * Pulse2Alike - the link to the instrument on Windows: not yet. Every send reports no MIDI.
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

// The Windows build compiles this in place of p2Hardware.c, whose CoreMIDI has no Windows counterpart yet.
// The claim always succeeds: with nothing sent, two instances cannot fight over the instrument.

#include <stdio.h>
#include <string.h>

#include "p2Hardware.h"

bool p2_hardware_claim(int * lock) {
    *lock = 0;
    return true;
}

void p2_hardware_release(int * lock) {
    *lock = -1;
}

tP2HardwareResult p2_hardware_send(const uint8_t * data, uint32_t length) {
    (void)data;
    (void)length;
    return eP2HardwareNoMidi;
}

tP2HardwareResult p2_hardware_send_to_edit_buffer(uint8_t dump[P2_DUMP_LENGTH]) {
    (void)dump;
    return eP2HardwareNoMidi;
}

tP2HardwareResult p2_hardware_store_to_slot(uint8_t dump[P2_DUMP_LENGTH], int number) {
    (void)dump;
    (void)number;
    return eP2HardwareNoMidi;
}

tP2HardwareResult p2_hardware_backup_start(void) {
    return eP2HardwareNoMidi;
}

bool p2_hardware_backup_poll(int * asked, int * found) {
    *asked = 0;
    *found = 0;
    return true;
}

void p2_hardware_backup_cancel(void) {
}

int p2_hardware_backup_take(uint8_t dumps[P2_PATCH_SLOTS][P2_DUMP_LENGTH]) {
    (void)dumps;
    return 0;
}
