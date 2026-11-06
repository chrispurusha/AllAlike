# p2Hardware.c

## 1. Finding the instrument
The first CoreMIDI destination whose name contains "pulse", in any case - the Pulse 2's USB port is "Pulse2". One
client and output port per process, made on first use, handed to SynthLib's `synthlib_midi_send_to()`.

## 2. The edit buffer only
A dump addressed to a slot (bytes 5-6 = bank, program) is stored to that slot's flash; one addressed 7F 00 only
replaces the edit buffer. Library files carry their slot number, so every send readdresses the dump to 7F 00 and
recomputes the checksum (the sum of bytes 1-134, low seven bits) whatever it came with. Checked on the instrument
2026-10-19: a dump carrying slot 05 0B arrived in the edit buffer byte for byte.

The one exception is Store Sound to Pulse 2 Slot (p2_hardware_store_to_slot): the dump addressed to P*n* as bank
(*n* - 1) / 128, program (*n* - 1) % 128, one flash write. Checked 2026-11-06 on P450, empty until then: it took the
store and read back byte for byte, but for an out-of-range Osc 1 Shape (119) stored as 0 (findings 2026-11-06).
P500 is the instrument's random sound generator (CT): it is never written, so the function refuses anything outside
P001-P499 (P2_HARDWARE_LAST_STORE_SLOT) with eP2HardwareBadSlot and sends nothing; the panel never offers P500.

## 3. One owner
An exclusive, non-blocking `flock()` on `hardware.lock` beside the presets file. Each instance opens the file itself,
so the lock separates instances in one process as well as instances in different hosts, and the system drops it
when the descriptor is closed - by the instance, or by its process ending however it ends - so it can never be
left held. Checked: a second instance in the process is refused, release frees it, another process's hold refuses
us and its death releases it (2026-10-23).

## 4. Backup
One sound request at a time (`F0 3E 16 7F 00 <bank> <prog> F7`, the slot counted from 0 as two 7-bit bytes), each
answered or timed out, on a thread of its own; nothing but requests is sent. A reply comes within about 20 ms
(measured: ten in 0.49 s with 30 ms pauses), so the wait is 0.25 s; P407-P499 never answer, which puts a whole
backup just under 47 s. A reply counts only if it is a sound dump addressed to the slot asked for.
The input is a CoreMIDI port of its own on the plug-in's client, connected to the first source with "pulse" in
its name for the run only. It is a MIDI 1.0 protocol port (`MIDIInputPortCreateWithProtocol`, as G2Alike's input
is - the older call is deprecated): SysEx arrives as SysEx7 packets of up to six bytes, F0 and F7 left out, and is
put back together here.
