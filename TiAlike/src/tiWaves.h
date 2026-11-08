// Notes: Docs/code-notes/tiOsc.md - "// notes §k" refers there.

#ifndef __TI_WAVES_H__
#define __TI_WAVES_H__

#define TI_WAVE_PARTIALS    (64)

typedef struct {
    float amp, phase;
} tTiPartial;

extern const tTiPartial          gTiWavePartials[62][TI_WAVE_PARTIALS];

#define TI_WAVETABLES            (100)
#define TI_WAVETABLE_FRAMES      (1016)   // notes §5: every table's frames, one after another
#define TI_WAVETABLE_PARTIALS    (256)    // notes §5: a frame is a 512-sample cycle

typedef struct {
    unsigned short partial;
    float          amp, phase;
} tTiWavetablePartial;

extern const unsigned short      gTiWavetableFirst[TI_WAVETABLES];           // a table's first frame
extern const unsigned char       gTiWavetableCount[TI_WAVETABLES];           // and how many it has, spread evenly over Index
extern const unsigned short      gTiWavetableSize[TI_WAVETABLES];            // notes §5: its cycle, 64 to 512 samples
extern const unsigned char       gTiWavetableSharpTop[TI_WAVETABLES];        // notes §5: its longest cycle read with a steepened step
extern const unsigned int        gTiWavetableStart[TI_WAVETABLE_FRAMES + 1]; // a frame's first partial below
extern const tTiWavetablePartial gTiWavetablePartials[];                     // only the partials it has

#endif // __TI_WAVES_H__
