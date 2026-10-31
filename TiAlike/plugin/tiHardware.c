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

// Notes: Docs/code-notes/tiHardware.md - "// notes §k" refers there.

#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <sys/file.h>
#include <unistd.h>

#include <CoreMIDI/CoreMIDI.h>

#include "tiHardware.h"

#define DEVICE_PORT    "Virus TI USB Plugin"   // notes §1
#define PAGE_C_AT      (266)
#define PAGE_D_AT      (394)
#define PAGE_B_AT      (137)
#define PAGE_A_AT      (9)
#define SINGLE_PART    (0x40)
#define OMNI           (0x10)

static MIDIClientRef gClient = 0;
static MIDIPortRef   gPort   = 0;

static MIDIEndpointRef find_instrument(void) {
    for (ItemCount i = 0; i < MIDIGetNumberOfDestinations(); i++) {
        MIDIEndpointRef endpoint = MIDIGetDestination(i);
        CFStringRef     name     = NULL;
        char            text[256];

        if ((MIDIObjectGetStringProperty(endpoint, kMIDIPropertyDisplayName, &name) != noErr) || (name == NULL)) {
            continue;
        }
        bool            found    = CFStringGetCString(name, text, sizeof(text), kCFStringEncodingUTF8) && (strcasestr(text, DEVICE_PORT) != NULL);

        CFRelease(name);

        if (found) {
            return endpoint;
        }
    }

    return 0;
}

// notes §2
bool ti_hardware_claim(int * lock) {
    char path[1100];

    if (*lock >= 0) {
        return true;
    }
    snprintf(path, sizeof(path), "%s", ti_presets_path());
    snprintf(strrchr(path, '/') + 1, 32, "hardware.lock");
    *lock = open(path, O_RDWR | O_CREAT, 0644);

    if ((*lock >= 0) && (flock(*lock, LOCK_EX | LOCK_NB) == 0)) {
        return true;
    }

    if (*lock >= 0) {
        close(*lock);
    }
    *lock = -1;
    return false;
}

void ti_hardware_release(int * lock) {
    if (*lock >= 0) {
        close(*lock);
        *lock = -1;
    }
}

static tTiHardwareResult send_bytes(const uint8_t * data, uint32_t length) {
    if (gPort == 0) {
        if ((MIDIClientCreate(CFSTR("TiAlike"), NULL, NULL, &gClient) != noErr) || (MIDIOutputPortCreate(gClient, CFSTR("to instrument"), &gPort) != noErr)) {
            gPort = 0;
            return eTiHardwareNoMidi;
        }
    }
    MIDIEndpointRef  dest   = find_instrument();

    if (dest == 0) {
        return eTiHardwareNotFound;
    }
    Byte             buffer[1024];
    MIDIPacketList * list   = (MIDIPacketList *)buffer;
    MIDIPacket *     packet = MIDIPacketListInit(list);

    packet = MIDIPacketListAdd(list, sizeof(buffer), packet, 0, length, data);
    return ((packet != NULL) && (MIDISend(gPort, dest, list) == noErr)) ? eTiHardwareSent : eTiHardwareNoMidi;
}

// notes §3: ti_patch_to_single addressed it to the edit buffer
tTiHardwareResult ti_hardware_send_sound(uint8_t single[TI_SINGLE_LENGTH]) {
    return send_bytes(single, TI_SINGLE_LENGTH);
}

// notes §3: one byte of the edit buffer
tTiHardwareResult ti_hardware_send_byte(int at, uint8_t value) {
    int     page  = (at >= PAGE_D_AT) ? 0x6F : ((at >= PAGE_C_AT) ? 0x6E : ((at >= PAGE_B_AT) ? 0x71 : 0x70));
    int     from  = (at >= PAGE_D_AT) ? PAGE_D_AT : ((at >= PAGE_C_AT) ? PAGE_C_AT : ((at >= PAGE_B_AT) ? PAGE_B_AT : PAGE_A_AT));
    uint8_t m[11] = {0xF0, 0x00, 0x20, 0x33, 0x01, OMNI, (uint8_t)page, SINGLE_PART, (uint8_t)(at - from), value, 0xF7};

    return send_bytes(m, sizeof(m));
}

void ti_hardware_note(uint8_t note, uint8_t velocity) {
    uint8_t m[3] = {0x90, note, velocity};

    (void)send_bytes(m, sizeof(m));
}
