/*
 * Pulse2Alike - the sound, sent to the instrument's edit buffer over USB MIDI.
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

#ifndef __TI_HARDWARE_H__
#define __TI_HARDWARE_H__

#include <stdbool.h>
#include <stdint.h>

#include "tiPatch.h"

typedef enum {
    eTiHardwareSent,
    eTiHardwareNotFound,   // notes §1: no port of the instrument's
    eTiHardwareNoMidi,     // CoreMIDI refused the client, the port or the send
    eTiHardwareBusy,       // notes §2: another instance owns the instrument
} tTiHardwareResult;

// notes §2: one instance on the machine owns the instrument, by flock(); *lock is -1 when it holds nothing
bool ti_hardware_claim(int * lock);
void ti_hardware_release(int * lock);

// notes §3: only the edit buffer is ever addressed, so no bank on the instrument can change
tTiHardwareResult ti_hardware_send_sound(uint8_t single[TI_SINGLE_LENGTH]);
tTiHardwareResult ti_hardware_send_byte(int at, uint8_t value);   // at: its place in the Single message
void ti_hardware_note(uint8_t note, uint8_t velocity);            // velocity 0 is note off; from any thread

// notes §4-§5: a transfer of whole banks, one at a time on the machine; main thread, driven by ti_hardware_tick()
#define TI_HW_RAM_BANKS    (4)     // A-D; the rest are ROM and are never written

typedef enum {
    eTiTransferIdle,
    eTiTransferRunning,
    eTiTransferDone,
    eTiTransferFailed,     // ti_hardware_transfer_status() says why
} tTiTransferState;

// notes §4: banks first..last (1 = A .. 23 = W) into the reader's store
tTiHardwareResult ti_hardware_read_start(int firstBank, int lastBank);
// notes §5: 128 Singles into RAM bank 1-4, then read back to check them
tTiHardwareResult ti_hardware_write_start(int bank, const uint8_t(*singles)[TI_SINGLE_LENGTH]);
tTiTransferState ti_hardware_tick(void);
const char * ti_hardware_transfer_status(void);                   // a line for the progress box, or why it failed
double ti_hardware_transfer_progress(void);                       // 0..1
const uint8_t * ti_hardware_read_single(int bank, int program);   // what arrived, NULL if nothing did
int ti_hardware_verify_mismatches(void);                          // notes §5: after a write, Singles read back different
void ti_hardware_transfer_stop(void);

#endif // __TI_HARDWARE_H__
