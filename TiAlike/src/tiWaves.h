// Notes: Docs/code-notes/tiOsc.md - "// notes §k" refers there.

#ifndef __TI_WAVES_H__
#define __TI_WAVES_H__

#define TI_WAVE_PARTIALS    (64)

typedef struct {
    float amp, phase;
} tTiPartial;

extern const tTiPartial     gTiWavePartials[62][TI_WAVE_PARTIALS];

#define TI_WAVETABLES            (100)
#define TI_WAVETABLE_FRAMES      (1016)   // notes §5: every table's frames, one after another
#define TI_WAVETABLE_PARTIALS    (64)

extern const unsigned short gTiWavetableFirst[TI_WAVETABLES];   // a table's first frame
extern const unsigned char  gTiWavetableCount[TI_WAVETABLES];   // and how many it has, spread evenly over Index
extern const float          gTiWavetableFrames[TI_WAVETABLE_FRAMES][TI_WAVETABLE_PARTIALS][2];

#endif // __TI_WAVES_H__
