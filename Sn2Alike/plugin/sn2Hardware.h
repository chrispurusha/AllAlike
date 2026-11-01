#ifndef __SN2_HARDWARE_H__
#define __SN2_HARDWARE_H__

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "sn2Patch.h"

#define SN2_HARDWARE_MAX_PORTS    (32)

typedef enum {
    eSn2HardwareSent,
    eSn2HardwareNotFound,   // the chosen output is not there
    eSn2HardwareNoMidi,     // CoreMIDI refused the client, the port or the send
} tSn2HardwareResult;

// notes §1: one instance on the machine owns the instrument, by flock(); *lock is -1 when it holds nothing
bool sn2_hardware_claim(int * lock);
void sn2_hardware_release(int * lock);

// The machine's MIDI outputs, for the menu; main thread
int sn2_hardware_port_count(void);
bool sn2_hardware_port_name(int index, char * out, size_t len);

// notes §2: only the edit buffer is ever addressed, so no stored program can change. A sound is two messages,
// sent apart: the main one, then the extension.
tSn2HardwareResult sn2_hardware_send_main(const uint8_t single[SN2_SINGLE_LENGTH], const char * port, uint8_t device);
tSn2HardwareResult sn2_hardware_send_extension(const uint8_t single[SN2_SINGLE_LENGTH], const char * port, uint8_t device);

// notes §3: a whole backup. Asks for it, keeps what arrives, and is done when the programs have passed.
typedef enum {
    eSn2ReadIdle,
    eSn2ReadWaiting,      // asked, nothing yet
    eSn2ReadReceiving,
    eSn2ReadDone,
    eSn2ReadTimedOut,     // nothing came
} tSn2ReadState;

tSn2HardwareResult sn2_hardware_read_start(const char * port, uint8_t device);
tSn2ReadState sn2_hardware_read_state(void);
int sn2_hardware_read_programs(void);                   // complete so far
bool sn2_hardware_read_save(const char * path);         // what came, as a .syx file
void sn2_hardware_read_stop(void);

// Any thread. channel 0..15
void sn2_hardware_short(const char * port, uint8_t status, uint8_t data1, uint8_t data2);

#endif // __SN2_HARDWARE_H__
