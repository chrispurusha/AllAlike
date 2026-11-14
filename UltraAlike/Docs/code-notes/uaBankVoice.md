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
