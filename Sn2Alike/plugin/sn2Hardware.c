// Notes: Docs/code-notes/sn2Hardware.md - "// notes §k" refers there.

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

#include "sn2Hardware.h"

#define MAIN_LENGTH       (297)
#define MAIN_DATA         (286)
#define EXT_DATA          (88)
#define SOURCE_DATA_AT    (10)
#define EDIT_MAIN         (0x00)
#define EDIT_EXT          (0x1E)
#define FORMAT            (0x09)

static const uint8_t kHeader[6] = {0xF0, 0x00, 0x20, 0x29, 0x01, 0x24};

static MIDIClientRef gClient    = 0;
static MIDIPortRef   gPort      = 0;

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

int sn2_hardware_port_count(void) {
    ItemCount n = MIDIGetNumberOfDestinations();

    return (n > SN2_HARDWARE_MAX_PORTS) ? SN2_HARDWARE_MAX_PORTS : (int)n;
}

bool sn2_hardware_port_name(int index, char * out, size_t len) {
    return endpoint_name(MIDIGetDestination((ItemCount)index), out, len);
}

bool sn2_hardware_claim(int * lock) {
    char path[1100];

    if (*lock >= 0) {
        return true;
    }
    snprintf(path, sizeof(path), "%s", sn2_presets_path());
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

void sn2_hardware_release(int * lock) {
    if (*lock >= 0) {
        close(*lock);
        *lock = -1;
    }
}

static tSn2HardwareResult send_bytes(const char * port, const uint8_t * data, uint32_t length) {
    if (gPort == 0) {
        if ((MIDIClientCreate(CFSTR("Sn2Alike"), NULL, NULL, &gClient) != noErr) || (MIDIOutputPortCreate(gClient, CFSTR("Out"), &gPort) != noErr)) {
            gPort = 0;
            return eSn2HardwareNoMidi;
        }
    }
    MIDIEndpointRef  dest   = find_output(port);

    if (dest == 0) {
        return eSn2HardwareNotFound;
    }
    Byte             buffer[1024];
    MIDIPacketList * list   = (MIDIPacketList *)buffer;
    MIDIPacket *     packet = MIDIPacketListInit(list);

    packet = MIDIPacketListAdd(list, sizeof(buffer), packet, 0, length, data);
    return ((packet != NULL) && (MIDISend(gPort, dest, list) == noErr)) ? eSn2HardwareSent : eSn2HardwareNoMidi;
}

// notes §2: header, device, command, format, the program's data, end
static tSn2HardwareResult send_message(const char * port, uint8_t device, uint8_t command, const uint8_t * data, uint32_t length) {
    uint8_t m[400];

    memcpy(m, kHeader, sizeof(kHeader));
    m[6]          = device;
    m[7]          = command;
    m[8]          = FORMAT;
    memcpy(&m[9], data, length);
    m[9 + length] = 0xF7;
    return send_bytes(port, m, 10 + length);
}

tSn2HardwareResult sn2_hardware_send_main(const uint8_t single[SN2_SINGLE_LENGTH], const char * port, uint8_t device) {
    return send_message(port, device, EDIT_MAIN, &single[SOURCE_DATA_AT], MAIN_DATA);
}

tSn2HardwareResult sn2_hardware_send_extension(const uint8_t single[SN2_SINGLE_LENGTH], const char * port, uint8_t device) {
    return send_message(port, device, EDIT_EXT, &single[MAIN_LENGTH + SOURCE_DATA_AT], EXT_DATA);
}

// ---- reading a backup - notes §3 ----------------------------------------------------------------------------

#define READ_LIMIT      (4 * 1024 * 1024)
#define READ_WAIT_S     (15.0)
#define READ_QUIET_S    (4.0)
#define ALL_PROGRAMS    (0x04)    // notes §3
#define PROGRAM_DUMP    (0x02)
#define PROGRAM_EXT     (0x1F)
#define PERFORMANCE     (0x07)

static MIDIPortRef     gReadPort   = 0;
static MIDIEndpointRef gReadSource = 0;
static uint8_t *       gRead;
static _Atomic size_t  gReadLength;
static _Atomic int     gReadPrograms;
static _Atomic bool    gReadPast, gReadActive;
static _Atomic double  gReadLast;
static double          gReadStarted;
static size_t          gReadMessage;  // where the message being received began

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

    // the programs come first; the first performance means they have all passed
    if ((at == (gReadMessage + 7)) && (b == PERFORMANCE)) {
        atomic_store(&gReadPast, true);
        atomic_store(&gReadLength, gReadMessage);
        return;
    }
    atomic_store(&gReadLength, at + 1);

    if ((b == 0xF7) && ((at - gReadMessage) > 7) && (gRead[gReadMessage + 7] == PROGRAM_EXT)) {
        atomic_fetch_add(&gReadPrograms, 1);
    }
}

#define IDENTIFY_WAIT_S    (2.0)

static _Atomic bool gIdentifying;
static _Atomic int  gIdentified = -1;
static double       gIdentifyStarted;

// notes §4: the universal identity reply, F0 7E <id> 06 02 then Novation's 00 20 29
static void identify_byte(uint8_t b) {
    static uint8_t m[8];
    static int     n = 8;

    if (b == 0xF0) {
        n = 0;
    }

    if (n >= 8) {
        return;
    }
    m[n++] = b;

    if ((n == 8) && (m[1] == 0x7E) && (m[3] == 0x06) && (m[4] == 0x02) && (m[5] == 0x00) && (m[6] == 0x20) && (m[7] == 0x29)) {
        atomic_store(&gIdentified, (int)m[2]);
        atomic_store(&gIdentifying, false);
    }
}

static void read_input(const MIDIPacketList * list, void * port, void * source) {
    const MIDIPacket * packet = &list->packet[0];

    (void)port;
    (void)source;

    for (UInt32 i = 0; i < list->numPackets; i++) {
        for (UInt16 k = 0; k < packet->length; k++) {
            if (atomic_load(&gIdentifying)) {
                identify_byte(packet->data[k]);
            } else {
                read_byte(packet->data[k]);
            }
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

tSn2HardwareResult sn2_hardware_read_start(const char * port, uint8_t device) {
    uint8_t request[9] = {0xF0, 0x00, 0x20, 0x29, 0x01, 0x24, device, ALL_PROGRAMS, 0xF7};

    sn2_hardware_read_stop();

    if (gRead == NULL) {
        gRead = (uint8_t *)malloc(READ_LIMIT);
    }

    if (gRead == NULL) {
        return eSn2HardwareNoMidi;
    }
    atomic_store(&gReadLength, 0);
    atomic_store(&gReadPrograms, 0);
    atomic_store(&gReadPast, false);
    atomic_store(&gReadLast, 0.0);
    gReadMessage = 0;

    if ((gClient == 0) && (MIDIClientCreate(CFSTR("Sn2Alike"), NULL, NULL, &gClient) != noErr)) {
        return eSn2HardwareNoMidi;
    }

    if ((gReadPort == 0) && (MIDIInputPortCreate(gClient, CFSTR("In"), read_input, NULL, &gReadPort) != noErr)) {
        gReadPort = 0;
        return eSn2HardwareNoMidi;
    }
    gReadSource  = find_input(port);

    if ((gReadSource == 0) || (MIDIPortConnectSource(gReadPort, gReadSource, NULL) != noErr)) {
        return eSn2HardwareNotFound;
    }
    gReadStarted = now_s();
    atomic_store(&gReadActive, true);
    return send_bytes(port, request, sizeof(request));
}

tSn2HardwareResult sn2_hardware_identify_start(const char * port) {
    static const uint8_t request[6] = {0xF0, 0x7E, 0x7F, 0x06, 0x01, 0xF7};   // identity request, to every device

    if (atomic_load(&gReadActive) || (port == NULL) || (port[0] == '\0')) {
        return eSn2HardwareNotFound;   // a read is listening already
    }

    if ((gClient == 0) && (MIDIClientCreate(CFSTR("Sn2Alike"), NULL, NULL, &gClient) != noErr)) {
        return eSn2HardwareNoMidi;
    }

    if ((gReadPort == 0) && (MIDIInputPortCreate(gClient, CFSTR("In"), read_input, NULL, &gReadPort) != noErr)) {
        gReadPort = 0;
        return eSn2HardwareNoMidi;
    }
    gReadSource      = find_input(port);

    if ((gReadSource == 0) || (MIDIPortConnectSource(gReadPort, gReadSource, NULL) != noErr)) {
        gReadSource = 0;
        return eSn2HardwareNotFound;
    }
    atomic_store(&gIdentified, -1);
    gIdentifyStarted = now_s();
    atomic_store(&gIdentifying, true);
    return send_bytes(port, request, sizeof(request));
}

int sn2_hardware_identified(void) {
    int id = atomic_exchange(&gIdentified, -1);

    if ((id >= 0) || (atomic_load(&gIdentifying) && ((now_s() - gIdentifyStarted) > IDENTIFY_WAIT_S))) {
        atomic_store(&gIdentifying, false);

        if (!atomic_load(&gReadActive) && (gReadPort != 0) && (gReadSource != 0)) {
            MIDIPortDisconnectSource(gReadPort, gReadSource);
            gReadSource = 0;
        }
    }
    return id;
}

tSn2ReadState sn2_hardware_read_state(void) {
    if (!atomic_load(&gReadActive)) {
        return eSn2ReadIdle;
    }
    double last = atomic_load(&gReadLast);

    if (atomic_load(&gReadPast)) {
        return eSn2ReadDone;
    }

    if (last == 0.0) {
        return ((now_s() - gReadStarted) > READ_WAIT_S) ? eSn2ReadTimedOut : eSn2ReadWaiting;
    }
    return ((now_s() - last) > READ_QUIET_S) ? eSn2ReadDone : eSn2ReadReceiving;
}

int sn2_hardware_read_programs(void) {
    return atomic_load(&gReadPrograms);
}

bool sn2_hardware_read_save(const char * path) {
    FILE * f = fopen(path, "wb");
    size_t n = atomic_load(&gReadLength);
    bool   ok;

    if (f == NULL) {
        return false;
    }
    ok = fwrite(gRead, 1, n, f) == n;
    return (fclose(f) == 0) && ok;
}

void sn2_hardware_read_stop(void) {
    if ((gReadPort != 0) && (gReadSource != 0)) {
        MIDIPortDisconnectSource(gReadPort, gReadSource);
    }
    gReadSource = 0;
    atomic_store(&gReadActive, false);
}

void sn2_hardware_short(const char * port, uint8_t status, uint8_t data1, uint8_t data2) {
    uint8_t m[3] = {status, data1, data2};

    (void)send_bytes(port, m, sizeof(m));
}
