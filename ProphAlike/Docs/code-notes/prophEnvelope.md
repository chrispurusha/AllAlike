# prophEnvelope.h notes

This source is temporary scaffold inherited from an AllAlike instrument template. The sections below preserve its code-comment references only; its behaviour has not been measured against or validated on the Korg Prophecy. Replace each section with Prophecy-specific findings as the corresponding code is implemented.

## §1

Placeholder implementation note: this code path is inherited scaffolding, not a measured Prophecy behaviour.

## §3

Placeholder implementation note: this code path is inherited scaffolding, not a measured Prophecy behaviour.

## §6

Placeholder implementation note: this code path is inherited scaffolding, not a measured Prophecy behaviour.

## §8

Placeholder implementation note: this code path is inherited scaffolding, not a measured Prophecy behaviour.

## §10. The amp EG
The Prophecy's amp envelope: from Start Level, Attack to Attack Level, Decay to Break Point, Slope to Sustain Level,
held, then Release to nothing - straight segments, each over its time. Times are approximate (2 ms at 0, a decade
every 25 steps, 18 s at 99) and the segments' curves are not yet measured. A key restarts from Start Level.
