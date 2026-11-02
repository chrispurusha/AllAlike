#ifndef __NL3_HARDWARE_H__
#define __NL3_HARDWARE_H__

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "nl3Patch.h"

#define NL3_HARDWARE_MAX_PORTS    (32)

typedef enum {
    eNl3HardwareSent,
    eNl3HardwareNotFound,   // the chosen output is not there
    eNl3HardwareNoMidi,     // CoreMIDI refused the client, the port or the send
} tNl3HardwareResult;

// notes §1: one instance on the machine owns the instrument, by flock(); *lock is -1 when it holds nothing
bool nl3_hardware_claim(int * lock);
void nl3_hardware_release(int * lock);

// The machine's MIDI outputs, for the menu; main thread
int nl3_hardware_port_count(void);
bool nl3_hardware_port_name(int index, char * out, size_t len);

// notes §2: only the edit buffer is ever addressed, so no stored program can change. A sound is two messages,
// sent apart: the main one, then the extension.
tNl3HardwareResult nl3_hardware_send_main(const uint8_t single[NL3_SINGLE_LENGTH], const char * port, uint8_t device);
tNl3HardwareResult nl3_hardware_send_extension(const uint8_t single[NL3_SINGLE_LENGTH], const char * port, uint8_t device);

// notes §3: a whole backup. Asks for it, keeps what arrives, and is done when the programs have passed.
typedef enum {
    eNl3ReadIdle,
    eNl3ReadWaiting,      // asked, nothing yet
    eNl3ReadReceiving,
    eNl3ReadDone,
    eNl3ReadTimedOut,     // nothing came
} tNl3ReadState;

tNl3HardwareResult nl3_hardware_read_start(const char * port, uint8_t device);
tNl3ReadState nl3_hardware_read_state(void);
int nl3_hardware_read_programs(void);                   // complete so far
bool nl3_hardware_read_save(const char * path);         // what came, as a .syx file
void nl3_hardware_read_stop(void);

// Any thread. channel 0..15
void nl3_hardware_short(const char * port, uint8_t status, uint8_t data1, uint8_t data2);

#endif // __NL3_HARDWARE_H__
