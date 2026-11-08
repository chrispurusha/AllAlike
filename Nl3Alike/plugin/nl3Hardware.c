// Notes: Docs/code-notes/nl3Hardware.md - "// notes §k" refers there.

#include <fcntl.h>
#include <stdatomic.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <sys/file.h>
#include <unistd.h>

#include <CoreMIDI/CoreMIDI.h>

#include "nl3Hardware.h"

#define REQUEST_FROM_MEMORY    (0x31)
#define FROM_SLOT              (0x20)

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

static MIDIEndpointRef find_output(const char * port) {
    for (ItemCount i = 0; (port != NULL) && (port[0] != '\0') && (i < MIDIGetNumberOfDestinations()); i++) {
        char text[256];

        if (endpoint_name(MIDIGetDestination(i), text, sizeof(text)) && (strcmp(text, port) == 0)) {
            return MIDIGetDestination(i);
        }
    }

    return 0;
}

int nl3_hardware_port_count(void) {
    ItemCount n = MIDIGetNumberOfDestinations();

    return (n > NL3_HARDWARE_MAX_PORTS) ? NL3_HARDWARE_MAX_PORTS : (int)n;
}

bool nl3_hardware_port_name(int index, char * out, size_t len) {
    return endpoint_name(MIDIGetDestination((ItemCount)index), out, len);
}

bool nl3_hardware_claim(int * lock) {
    char path[1100];

    if (*lock >= 0) {
        return true;
    }
    snprintf(path, sizeof(path), "%s", nl3_presets_path());
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

void nl3_hardware_release(int * lock) {
    if (*lock >= 0) {
        close(*lock);
        *lock = -1;
    }
}

static tNl3HardwareResult send_bytes(const char * port, const uint8_t * data, uint32_t length) {
    if (gPort == 0) {
        if ((MIDIClientCreate(CFSTR("Nl3Alike"), NULL, NULL, &gClient) != noErr) || (MIDIOutputPortCreate(gClient, CFSTR("Out"), &gPort) != noErr)) {
            gPort = 0;
            return eNl3HardwareNoMidi;
        }
    }
    MIDIEndpointRef  dest   = find_output(port);

    if (dest == 0) {
        return eNl3HardwareNotFound;
    }
    Byte             buffer[1024];
    MIDIPacketList * list   = (MIDIPacketList *)buffer;
    MIDIPacket *     packet = MIDIPacketListInit(list);

    packet = MIDIPacketListAdd(list, sizeof(buffer), packet, 0, length, data);
    return ((packet != NULL) && (MIDISend(gPort, dest, list) == noErr)) ? eNl3HardwareSent : eNl3HardwareNoMidi;
}

// notes §2: the program into Slot A, as one message
tNl3HardwareResult nl3_hardware_send_main(const uint8_t single[NL3_SINGLE_LENGTH], const char * port, uint8_t device) {
    uint8_t m[NL3_SINGLE_LENGTH];

    memcpy(m, single, sizeof(m));
    m[2] = device;
    m[4] = FROM_SLOT;
    m[5] = 0;
    m[6] = 0;
    return send_bytes(port, m, sizeof(m));
}

// The Nord Lead 3 takes a program whole; there is nothing more to send
tNl3HardwareResult nl3_hardware_send_extension(const uint8_t single[NL3_SINGLE_LENGTH], const char * port, uint8_t device) {
    (void)single;
    (void)port;
    (void)device;
    return eNl3HardwareSent;
}

// ---- reading a backup - notes §3 ----------------------------------------------------------------------------

#define READ_LIMIT      (4 * 1024 * 1024)
#define READ_WAIT_S     (15.0)
#define READ_QUIET_S    (4.0)
#define READ_SKIP_S     (0.6)         // notes §3: an empty location does not answer
#define ALL_PROGRAMS    (NL3_PRESETS) // notes §3: asked for one at a time

static MIDIPortRef     gReadPort   = 0;
static MIDIEndpointRef gReadSource = 0;
static uint8_t *       gRead;
static _Atomic size_t  gReadLength;
static _Atomic int     gReadPrograms;
static _Atomic int     gReadNext;     // the next location to ask for
static _Atomic double  gReadAsked;    // when the last request went
static _Atomic bool    gReadPast, gReadActive;
static _Atomic double  gReadLast;
static double          gReadStarted;
static size_t          gReadMessage;  // where the message being received began
static char            gReadPortName[256];
static uint8_t         gReadDevice;

static tNl3HardwareResult send_bytes(const char * port, const uint8_t * data, uint32_t length);
static double now_s(void);

static tNl3HardwareResult request_program(int index) {
    atomic_store(&gReadAsked, now_s());
    uint8_t request[8] = {0xF0, 0x33, gReadDevice, 0x09, REQUEST_FROM_MEMORY, (uint8_t)(index / NL3_BANK_SIZE), (uint8_t)(index % NL3_BANK_SIZE), 0xF7};

    return send_bytes(gReadPortName, request, sizeof(request));
}

static double now_s(void) {
    return (double)clock_gettime_nsec_np(CLOCK_MONOTONIC) * 1e-9;
}

static void read_byte(uint8_t b) {
    size_t at = atomic_load(&gReadLength);

    if ((b >= 0xF8) || (at >= READ_LIMIT) || atomic_load(&gReadPast)) {
        return;     // real time bytes, or we have what we came for
    }

    if (b == 0xF0) {
        gReadMessage = at;
    }
    gRead[at] = b;
    atomic_store(&gReadLength, at + 1);

    // notes §3: each program asks for the next
    if ((b == 0xF7) && ((at + 1 - gReadMessage) == NL3_SINGLE_LENGTH)) {
        int next = atomic_fetch_add(&gReadNext, 1) + 1;

        atomic_fetch_add(&gReadPrograms, 1);

        if (next >= ALL_PROGRAMS) {
            atomic_store(&gReadPast, true);
        } else {
            (void)request_program(next);
        }
    }
}

static void read_input(const MIDIPacketList * list, void * port, void * source) {
    const MIDIPacket * packet = &list->packet[0];

    (void)port;
    (void)source;

    for (UInt32 i = 0; i < list->numPackets; i++) {
        for (UInt16 k = 0; k < packet->length; k++) {
            read_byte(packet->data[k]);
        }

        packet = MIDIPacketNext(packet);
    }

    atomic_store(&gReadLast, now_s());
}

static MIDIEndpointRef find_input(const char * port) {
    for (ItemCount i = 0; (port != NULL) && (i < MIDIGetNumberOfSources()); i++) {
        char text[256];

        if (endpoint_name(MIDIGetSource(i), text, sizeof(text)) && (strcmp(text, port) == 0)) {
            return MIDIGetSource(i);
        }
    }

    return 0;
}

tNl3HardwareResult nl3_hardware_read_start(const char * port, uint8_t device) {
    nl3_hardware_read_stop();
    snprintf(gReadPortName, sizeof(gReadPortName), "%s", (port != NULL) ? port : "");
    gReadDevice  = device;

    if (gRead == NULL) {
        gRead = (uint8_t *)malloc(READ_LIMIT);
    }

    if (gRead == NULL) {
        return eNl3HardwareNoMidi;
    }
    atomic_store(&gReadLength, 0);
    atomic_store(&gReadPrograms, 0);
    atomic_store(&gReadNext, 0);
    atomic_store(&gReadPast, false);
    atomic_store(&gReadLast, 0.0);
    gReadMessage = 0;

    if ((gClient == 0) && (MIDIClientCreate(CFSTR("Nl3Alike"), NULL, NULL, &gClient) != noErr)) {
        return eNl3HardwareNoMidi;
    }

    if ((gReadPort == 0) && (MIDIInputPortCreate(gClient, CFSTR("In"), read_input, NULL, &gReadPort) != noErr)) {
        gReadPort = 0;
        return eNl3HardwareNoMidi;
    }
    gReadSource  = find_input(port);

    if ((gReadSource == 0) || (MIDIPortConnectSource(gReadPort, gReadSource, NULL) != noErr)) {
        return eNl3HardwareNotFound;
    }
    gReadStarted = now_s();
    atomic_store(&gReadActive, true);
    return request_program(0);
}

tNl3ReadState nl3_hardware_read_state(void) {
    if (!atomic_load(&gReadActive)) {
        return eNl3ReadIdle;
    }
    double last = atomic_load(&gReadLast);

    if (atomic_load(&gReadPast)) {
        return eNl3ReadDone;
    }
    double now  = now_s();

    // notes §3: nothing back for a while, and nothing arriving - an empty location; ask for the one after
    if (((now - atomic_load(&gReadAsked)) > READ_SKIP_S) && ((last == 0.0) || ((now - last) > READ_SKIP_S))) {
        int next = atomic_fetch_add(&gReadNext, 1) + 1;

        if (next >= ALL_PROGRAMS) {
            atomic_store(&gReadPast, true);
            return eNl3ReadDone;
        }
        (void)request_program(next);
    }

    if (last == 0.0) {
        return ((now - gReadStarted) > READ_WAIT_S) ? eNl3ReadTimedOut : eNl3ReadWaiting;
    }
    return (((now - last) > READ_QUIET_S) && (atomic_load(&gReadNext) >= ALL_PROGRAMS)) ? eNl3ReadDone : eNl3ReadReceiving;
}

int nl3_hardware_read_programs(void) {
    return atomic_load(&gReadPrograms);
}

bool nl3_hardware_read_save(const char * path) {
    FILE * f = fopen(path, "wb");
    size_t n = atomic_load(&gReadLength);
    bool   ok;

    if (f == NULL) {
        return false;
    }
    ok = fwrite(gRead, 1, n, f) == n;
    return (fclose(f) == 0) && ok;
}

void nl3_hardware_read_stop(void) {
    if ((gReadPort != 0) && (gReadSource != 0)) {
        MIDIPortDisconnectSource(gReadPort, gReadSource);
    }
    gReadSource = 0;
    atomic_store(&gReadActive, false);
}

void nl3_hardware_short(const char * port, uint8_t status, uint8_t data1, uint8_t data2) {
    uint8_t m[3] = {status, data1, data2};

    (void)send_bytes(port, m, sizeof(m));
}
