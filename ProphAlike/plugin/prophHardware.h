#ifndef __PROPH_HARDWARE_H__
#define __PROPH_HARDWARE_H__

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "prophPatch.h"

#define PROPH_HARDWARE_MAX_PORTS    (32)

typedef enum {
    eProphHardwareSent,
    eProphHardwareNotFound,   // the chosen output is not there
    eProphHardwareNoMidi,     // CoreMIDI refused the client, the port or the send
} tProphHardwareResult;

// notes §1: one instance on the machine owns the instrument, by flock(); *lock is -1 when it holds nothing
bool proph_hardware_claim(int * lock);
void proph_hardware_release(int * lock);

// The machine's MIDI outputs, for the menu; main thread
int proph_hardware_port_count(void);
bool proph_hardware_port_name(int index, char * out, size_t len);

// Placeholder interface only: these methods are inert until Prophecy hardware control is implemented.
tProphHardwareResult proph_hardware_send_main(const uint8_t single[PROPH_SINGLE_LENGTH], const char * port, uint8_t device);
tProphHardwareResult proph_hardware_send_extension(const uint8_t single[PROPH_SINGLE_LENGTH], const char * port, uint8_t device);

// Placeholder interface for future hardware backup support.
typedef enum {
    eProphReadIdle,
    eProphReadWaiting,      // asked, nothing yet
    eProphReadReceiving,
    eProphReadDone,
    eProphReadTimedOut,     // nothing came
} tProphReadState;

tProphHardwareResult proph_hardware_read_start(const char * port, uint8_t device);
tProphReadState proph_hardware_read_state(void);
int proph_hardware_read_programs(void);                   // complete so far
bool proph_hardware_read_save(const char * path);         // what came, as a .syx file
void proph_hardware_read_stop(void);

// Any thread. channel 0..15
void proph_hardware_short(const char * port, uint8_t status, uint8_t data1, uint8_t data2);

// Placeholder interface for future instrument identification.
tProphHardwareResult proph_hardware_identify_start(const char * port);
int proph_hardware_identified(void);   // the ID once it has come (then -1 again), -1 meanwhile

#endif // __PROPH_HARDWARE_H__
