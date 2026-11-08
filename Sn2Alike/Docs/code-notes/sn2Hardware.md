# sn2Hardware.c

## 1. Who owns the instrument
One instance on the machine, by a file lock beside the presets, so two instances cannot fight over the edit buffer.

## 2. What is sent
Only the edit buffer: command 00, then 1E for the extension, sent apart. The bank dump command 02 writes flash and is never sent. No single-parameter message is used, so every edit sends the whole sound. Device ID and channel are the instrument's own settings, on the GLOBAL page.

## 3. Reading a backup
The All Programs request, the message the instrument itself transmits from its panel, asks only that it send. Done when the data stops for four seconds, or at the first performance if one comes. The answer needs MIDI from the instrument on the port of the same name. Nothing is stored until the import's confirm.

## 4. Finding the SysEx ID
The instrument does not show its SysEx ID, but it answers the universal identity request (F0 7E 7F 06 01 F7) with
F0 7E <id> 06 02 00 20 29 ..., its ID in the third byte - 06 on the instrument here. Choosing a port in the MIDI menu
sends the request and listens on the input of the same name for two seconds; a reply sets SysEx ID. With no reply
(no MIDI back, or another instrument on that port) SysEx ID stays as it was.
