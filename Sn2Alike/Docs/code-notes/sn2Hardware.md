# sn2Hardware.c

## 1. Who owns the instrument
One instance on the machine, by a file lock beside the presets, so two instances cannot fight over the edit buffer.

## 2. What is sent
Only the edit buffer: command 00, then 1E for the extension, sent apart. The bank dump command 02 writes flash and is never sent. No single-parameter message is used, so every edit sends the whole sound. Channel on the GLOBAL page is the instrument's Global MIDI channel, and gives the messages their device byte (§4).

## 3. Reading a backup
The All Programs request, the message the instrument itself transmits from its panel, asks only that it send. Done when the data stops for four seconds, or at the first performance if one comes. The answer needs MIDI from the instrument on the port of the same name. Nothing is stored until the import's confirm.

## 4. The device byte
It is the instrument's Global MIDI channel less 1 (channel 7 is 06). With any other, the instrument answers "incoming
SysEx message, not Global MIDI ch" and takes nothing (2026-11-22). So Link has one setting, Channel, and no Device; its
old parameter is kept, hidden and unused, so that later ids do not move. Changing Channel sends the sound again. The
instrument answers the universal identity request (F0 7E 7F 06 01 F7) with F0 7E <byte> 06 02 00 20 29 ...: choosing a
port in the MIDI menu sends it, listens on the input of the same name for two seconds, and sets Channel from the reply.
With no reply (no MIDI back, or another instrument on that port) Channel stays as it was.
