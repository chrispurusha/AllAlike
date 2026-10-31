// Notes: Docs/code-notes/tiOsc.md - "// notes §k" refers there.

#ifndef __TI_WAVES_H__
#define __TI_WAVES_H__

#define TI_WAVE_PARTIALS    (64)

typedef struct {
    float amp, phase;
} tTiPartial;

extern const tTiPartial gTiWavePartials[62][TI_WAVE_PARTIALS];

#define TI_WAVETABLES            (100)
#define TI_WAVETABLE_STEPS       (16)
#define TI_WAVETABLE_PARTIALS    (64)

extern const float      gTiWavetables[TI_WAVETABLES][TI_WAVETABLE_STEPS][TI_WAVETABLE_PARTIALS][2];

#endif // __TI_WAVES_H__
