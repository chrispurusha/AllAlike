/*
 * TiAlike - the link to the instrument on Windows: not yet. Every send reports no MIDI.
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

// The Windows build compiles this in place of tiHardware.c, whose CoreMIDI has no Windows counterpart yet.
// The claim always succeeds: with nothing sent, two instances cannot fight over the instrument.

#include <stdio.h>
#include <string.h>

#include "tiHardware.h"

bool ti_hardware_claim(int * lock) {
    *lock = 0;
    return true;
}

void ti_hardware_release(int * lock) {
    *lock = -1;
}

tTiHardwareResult ti_hardware_send_sound(uint8_t single[TI_SINGLE_LENGTH]) {
    (void)single;
    return eTiHardwareNoMidi;
}

tTiHardwareResult ti_hardware_send_byte(int at, uint8_t value) {
    (void)at;
    (void)value;
    return eTiHardwareNoMidi;
}

void ti_hardware_note(uint8_t note, uint8_t velocity) {
    (void)note;
    (void)velocity;
}

tTiHardwareResult ti_hardware_read_start(int firstBank, int lastBank) {
    (void)firstBank;
    (void)lastBank;
    return eTiHardwareNoMidi;
}

tTiHardwareResult ti_hardware_write_start(int bank, const uint8_t(*singles)[TI_SINGLE_LENGTH]) {
    (void)bank;
    (void)singles;
    return eTiHardwareNoMidi;
}

tTiTransferState ti_hardware_tick(void) {
    return eTiTransferIdle;
}

const char * ti_hardware_transfer_status(void) {
    return "No MIDI on Windows yet.";
}

double ti_hardware_transfer_progress(void) {
    return 0.0;
}

const uint8_t * ti_hardware_read_single(int bank, int program) {
    (void)bank;
    (void)program;
    return NULL;
}

int ti_hardware_verify_mismatches(void) {
    return 0;
}

void ti_hardware_transfer_stop(void) {
}
