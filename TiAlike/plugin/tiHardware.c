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
#include <math.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <strings.h>
#include <sys/file.h>
#include <unistd.h>

#include <CoreMIDI/CoreMIDI.h>

#include "tiPrefs.h"
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

static bool endpoint_name(MIDIEndpointRef endpoint, char * out, size_t len) {
    CFStringRef name = NULL;

    if ((MIDIObjectGetStringProperty(endpoint, kMIDIPropertyDisplayName, &name) != noErr) || (name == NULL)) {
        return false;
    }
    bool        ok   = CFStringGetCString(name, out, (CFIndex)len, kCFStringEncodingUTF8);

    CFRelease(name);
    return ok;
}

// notes §1: the output chosen in the MIDI menu, or the instrument's own port when none is
static MIDIEndpointRef find_instrument(void) {
    const char * chosen = ti_hw_port();

    for (ItemCount i = 0; i < MIDIGetNumberOfDestinations(); i++) {
        MIDIEndpointRef endpoint = MIDIGetDestination(i);
        char            text[256];

        if (!endpoint_name(endpoint, text, sizeof(text))) {
            continue;
        }

        if ((chosen[0] != '\0') ? (strcmp(text, chosen) == 0) : (strcasestr(text, DEVICE_PORT) != NULL)) {
            return endpoint;
        }
    }

    return 0;
}

int ti_hardware_port_count(void) {
    ItemCount n = MIDIGetNumberOfDestinations();

    return (n > TI_HARDWARE_MAX_PORTS) ? TI_HARDWARE_MAX_PORTS : (int)n;
}

bool ti_hardware_port_name(int index, char * out, size_t len) {
    return endpoint_name(MIDIGetDestination((ItemCount)index), out, len);
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

static bool open_client(void) {
    if (gPort == 0) {
        if (  ((gClient == 0) && (MIDIClientCreate(CFSTR("TiAlike"), NULL, NULL, &gClient) != noErr))
           || (MIDIOutputPortCreate(gClient, CFSTR("to instrument"), &gPort) != noErr)) {
            gPort = 0;
            return false;
        }
    }
    return true;
}

static tTiHardwareResult send_bytes(const uint8_t * data, uint32_t length) {
    if (!open_client()) {
        return eTiHardwareNoMidi;
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

// ---- whole banks - notes §4, §5 ---------------------------------------------------------------------------

#define BANKS           (23)
#define BANK_SIZE       (128)
#define BANK_REQUEST    (0x32)
#define SINGLE_DUMP     (0x10)
#define QUIET_S         (6.0)       // notes §4: no Single for this long and the bank is asked for again
#define TRIES           (3)
#define WRITE_GAP_S     (1.0)       // notes §5: a Single takes 0.9 s over this port
#define SETTLE_S        (3.0)       // after the last write, before reading back

typedef enum {
    ePhaseIdle,
    ePhaseReading,
    ePhaseWriting,
    ePhaseSettling,
    ePhaseDone,
    ePhaseFailed,
} tPhase;

static MIDIPortRef     gInPort   = 0;
static MIDIEndpointRef gInSource = 0;
static int             gLock     = -1;
static uint8_t(*gGot)[BANK_SIZE][TI_SINGLE_LENGTH];      // [bank - 1][program]
static _Atomic bool    gHave[BANKS][BANK_SIZE];
static _Atomic int     gHaveCount[BANKS];
static _Atomic double  gLastIn;
static uint8_t         gIn[TI_SINGLE_LENGTH + 64];         // the message being received; MIDI thread only
static size_t          gInLength;
static bool            gInOverrun;

static tPhase          gPhase    = ePhaseIdle;
static int             gFirst, gLast, gBank, gTries;
static double          gAskedAt, gNextWriteAt, gSettleUntil;
static int             gWriteBank, gWritten, gMismatches;
static uint8_t(*gWrite)[TI_SINGLE_LENGTH];
static char            gStatus[160];

static double now_s(void) {
    return (double)clock_gettime_nsec_np(CLOCK_MONOTONIC) * 1e-9;
}

// MIDI thread: a Single dump to a bank, kept by its bank and program bytes
static void take_message(void) {
    int bank = gIn[7], program = gIn[8];

    if (  (gInLength != TI_SINGLE_LENGTH) || (gIn[6] != SINGLE_DUMP) || (gIn[1] != 0x00) || (gIn[2] != 0x20) || (gIn[3] != 0x33)
       || (bank < 1) || (bank > BANKS) || (program >= BANK_SIZE) || (gGot == NULL)) {
        return;
    }
    memcpy(gGot[bank - 1][program], gIn, TI_SINGLE_LENGTH);

    if (!atomic_exchange(&gHave[bank - 1][program], true)) {
        atomic_fetch_add(&gHaveCount[bank - 1], 1);
    }
}

static void read_input(const MIDIPacketList * list, void * port, void * source) {
    const MIDIPacket * packet = &list->packet[0];

    (void)port;
    (void)source;

    for (UInt32 i = 0; i < list->numPackets; i++) {
        for (UInt16 k = 0; k < packet->length; k++) {
            uint8_t b = packet->data[k];

            if (b >= 0xF8) {
                continue;
            }

            if (b == 0xF0) {
                gInLength  = 0;
                gInOverrun = false;
            }

            if (gInLength < sizeof(gIn)) {
                gIn[gInLength++] = b;
            } else {
                gInOverrun = true;
            }

            if ((b == 0xF7) && !gInOverrun) {
                take_message();
            }
        }

        packet = MIDIPacketNext(packet);
    }

    atomic_store(&gLastIn, now_s());
}

static MIDIEndpointRef find_source(void) {
    for (ItemCount i = 0; i < MIDIGetNumberOfSources(); i++) {
        MIDIEndpointRef endpoint = MIDIGetSource(i);
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

// the input, the store and the machine-wide lock a transfer needs
static tTiHardwareResult transfer_open(void) {
    if (gPhase != ePhaseIdle) {
        ti_hardware_transfer_stop();
    }

    if (!ti_hardware_claim(&gLock)) {
        return eTiHardwareBusy;
    }

    if (gGot == NULL) {
        gGot = calloc(BANKS, sizeof(*gGot));
    }

    if (gGot == NULL) {
        return eTiHardwareNoMidi;
    }

    if (!open_client()) {
        return eTiHardwareNoMidi;
    }

    if ((gInPort == 0) && (MIDIInputPortCreate(gClient, CFSTR("from instrument"), read_input, NULL, &gInPort) != noErr)) {
        gInPort = 0;
        return eTiHardwareNoMidi;
    }
    gInSource = find_source();

    if ((gInSource == 0) || (MIDIPortConnectSource(gInPort, gInSource, NULL) != noErr)) {
        gInSource = 0;
        return eTiHardwareNotFound;
    }
    return eTiHardwareSent;
}

static void forget_bank(int bank) {
    for (int p = 0; p < BANK_SIZE; p++) {
        atomic_store(&gHave[bank - 1][p], false);
    }

    atomic_store(&gHaveCount[bank - 1], 0);
}

static tTiHardwareResult ask_for_bank(int bank) {
    uint8_t m[9] = {0xF0, 0x00, 0x20, 0x33, 0x01, OMNI, BANK_REQUEST, (uint8_t)bank, 0xF7};

    gAskedAt = now_s();
    return send_bytes(m, sizeof(m));
}

static void fail(const char * why) {
    snprintf(gStatus, sizeof(gStatus), "%s", why);
    gPhase = ePhaseFailed;
}

// notes §4
tTiHardwareResult ti_hardware_read_start(int firstBank, int lastBank) {
    tTiHardwareResult r = transfer_open();

    if (r != eTiHardwareSent) {
        ti_hardware_transfer_stop();
        return r;
    }

    for (int b = 1; b <= BANKS; b++) {
        forget_bank(b);
    }

    gFirst = firstBank;
    gLast  = lastBank;
    gBank  = firstBank;
    gTries = 1;
    gPhase = ePhaseReading;
    atomic_store(&gLastIn, 0.0);
    snprintf(gStatus, sizeof(gStatus), "Asking for bank %c...", 'A' + gBank - 1);
    return ask_for_bank(gBank);
}

// notes §5
tTiHardwareResult ti_hardware_write_start(int bank, const uint8_t(*singles)[TI_SINGLE_LENGTH]) {
    if ((bank < 1) || (bank > TI_HW_RAM_BANKS)) {
        return eTiHardwareNotFound;   // a ROM bank: never
    }
    tTiHardwareResult r = transfer_open();

    if (r != eTiHardwareSent) {
        ti_hardware_transfer_stop();
        return r;
    }

    if ((gWrite == NULL) && ((gWrite = malloc(BANK_SIZE * TI_SINGLE_LENGTH)) == NULL)) {
        ti_hardware_transfer_stop();
        return eTiHardwareNoMidi;
    }

    for (int p = 0; p < BANK_SIZE; p++) {
        memcpy(gWrite[p], singles[p], TI_SINGLE_LENGTH);
        ti_patch_address(gWrite[p], ((bank - 1) * BANK_SIZE) + p + 1);
    }

    gWriteBank   = bank;
    gWritten     = 0;
    gMismatches  = 0;
    gNextWriteAt = now_s();
    gPhase       = ePhaseWriting;
    snprintf(gStatus, sizeof(gStatus), "Writing bank %c...", 'A' + bank - 1);
    return eTiHardwareSent;
}

// notes §5: what a read-back may differ in - the device byte, A1-A3 and the checksums
static bool same_single(const uint8_t * a, const uint8_t * b) {
    for (int i = 6; i < TI_SINGLE_LENGTH - 1; i++) {
        if ((i != 10) && (i != 11) && (i != 12) && (i != 265) && (i != TI_SINGLE_LENGTH - 2) && (a[i] != b[i])) {
            return false;
        }
    }

    return true;
}

static void reading_tick(void) {
    int    have = atomic_load(&gHaveCount[gBank - 1]);
    double last = fmax(atomic_load(&gLastIn), gAskedAt);

    snprintf(gStatus, sizeof(gStatus), "Bank %c: %d of %d Singles%s", 'A' + gBank - 1, have, BANK_SIZE, (gTries > 1) ? " (asked again)" : "");

    if (have == BANK_SIZE) {
        if (gBank == gLast) {
            if (gWriteBank != 0) {
                for (int p = 0; p < BANK_SIZE; p++) {
                    gMismatches += !same_single(gWrite[p], gGot[gWriteBank - 1][p]);
                }
            }
            gPhase = ePhaseDone;
            return;
        }
        gBank++;
        gTries = 1;
        (void)ask_for_bank(gBank);
        return;
    }

    if ((now_s() - last) < QUIET_S) {
        return;
    }

    if (gTries >= TRIES) {
        char why[160];

        snprintf(why, sizeof(why), (have == 0) ? "Nothing came back for bank %c. Is the Virus TI on, and connected over USB?"
                 : "Bank %c stopped short, %d of 128 Singles, after three tries.", 'A' + gBank - 1, have);
        fail(why);
        return;
    }
    gTries++;
    (void)ask_for_bank(gBank);   // notes §4: only once it has gone quiet - a new request aborts a dump
}

tTiTransferState ti_hardware_tick(void) {
    switch (gPhase) {
        case ePhaseIdle:
            return eTiTransferIdle;

        case ePhaseReading:
            reading_tick();
            break;

        case ePhaseWriting:

            if (now_s() >= gNextWriteAt) {
                if (send_bytes(gWrite[gWritten], TI_SINGLE_LENGTH) != eTiHardwareSent) {
                    fail("The Virus TI went away during the write.");
                    break;
                }
                gNextWriteAt = now_s() + WRITE_GAP_S;
                gWritten++;
                snprintf(gStatus, sizeof(gStatus), "Writing bank %c: %d of %d", 'A' + gWriteBank - 1, gWritten, BANK_SIZE);

                if (gWritten == BANK_SIZE) {
                    gSettleUntil = now_s() + SETTLE_S;
                    gPhase       = ePhaseSettling;
                }
            }
            break;

        case ePhaseSettling:

            if (now_s() >= gSettleUntil) {
                forget_bank(gWriteBank);
                gFirst = gLast = gBank = gWriteBank;
                gTries = 1;
                gPhase = ePhaseReading;
                (void)ask_for_bank(gBank);
            }
            break;

        case ePhaseDone:
            return eTiTransferDone;

        case ePhaseFailed:
            return eTiTransferFailed;
    }
    return eTiTransferRunning;
}

const char * ti_hardware_transfer_status(void) {
    return gStatus;
}

double ti_hardware_transfer_progress(void) {
    if (gPhase == ePhaseWriting) {
        return 0.5 * gWritten / BANK_SIZE;
    }
    int    done = 0;

    for (int b = gFirst; (b >= 1) && (b <= gLast); b++) {
        done += atomic_load(&gHaveCount[b - 1]);
    }

    double read = (gLast >= gFirst) ? ((double)done / ((gLast - gFirst + 1) * BANK_SIZE)) : 0.0;

    return (gWriteBank != 0) ? (0.5 + (0.5 * read)) : read;
}

const uint8_t * ti_hardware_read_single(int bank, int program) {
    return ((gGot != NULL) && (bank >= 1) && (bank <= BANKS) && (program >= 0) && (program < BANK_SIZE) && atomic_load(&gHave[bank - 1][program]))
           ? gGot[bank - 1][program] : NULL;
}

int ti_hardware_verify_mismatches(void) {
    return gMismatches;
}

void ti_hardware_transfer_stop(void) {
    if ((gInPort != 0) && (gInSource != 0)) {
        MIDIPortDisconnectSource(gInPort, gInSource);
    }
    gInSource  = 0;
    gPhase     = ePhaseIdle;
    gWriteBank = 0;
    ti_hardware_release(&gLock);
}
