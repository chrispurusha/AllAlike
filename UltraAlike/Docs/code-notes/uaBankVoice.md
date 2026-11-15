# uaBankVoice.c

## 1. Notes and zones
A note starts a voice for every zone of the preset whose key and velocity windows hold it. Its step is the sample's
own rate over the host's, times 2^((note - root + Tune)/12): the sample's tuning is in its rate (SynthLib emuBank
§3). Every note off releases, whatever the layer's Mode - a sampler's notes are held. Up to 64 voices a layer; past
that the oldest is faded out in 3 ms. Not yet read from the bank, so not played: a voice's tuning, level, pan,
envelopes, filter, LFOs and cords - each voice plays at unity through the layer's own envelope.

## 2. Loops
A looped sample plays to its loop end and then round the loop for as long as the voice sounds, release included.
The interpolator's taps either side of the seam are gathered in the order they sound - past the loop end, the
loop's start again; once looping, before the loop start, the loop's end - so the seam reads as one continuous
signal. Away from the seam it reads the sample directly.

## 3. The bank's amplifier envelope
A zone whose voice holds an envelope (SynthLib emuBank §5) plays through it rather than the layer's: Attack 1, Attack 2,
Decay 1 and Decay 2 in turn, each a straight line from where the level is to the stage's level in the stage's time (a
time of 0 jumps), Decay 2's level held while the key is down; at the note off Release 1 and Release 2 the same way
from wherever it is, and the voice ends after Release 2. A voice taken over while sounding starts from its level. The
segments' shape is not measured: a straight line in level until Emulator X3 or the E5000 says otherwise. The layer's
envelope dials do nothing to such a zone.
