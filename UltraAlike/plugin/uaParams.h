#ifndef __UA_PARAMS_H__
#define __UA_PARAMS_H__

#include "synthlibPlugin.h"
#include "uaDraw.h"
#include "uaEnvelope.h"

// notes §9: four layers, each with every parameter below; layer n's ids are n x UA_LAYER_STRIDE + the id here, so
// layer 1's are the ids a one-layer project saved
#define UA_LAYERS          (4)
#define UA_LAYER_STRIDE    (256)

// APPENDED TO, NEVER INSERTED INTO: the ids are what host projects save
enum {
    kUaParamVolume = 0,
    kUaParamPan,
    kUaParamTune,
    kUaParamFine,
    kUaParamVelocity,
    kUaParamKeyTrack,
    kUaParamRootKey,
    kUaParamMode,
    kUaParamVoices,
    kUaParamEnvMode,
    kUaParamAttack,
    kUaParamHold,
    kUaParamDecay,
    kUaParamSustain,
    kUaParamRelease,
    kUaParamAttackCurve,
    kUaParamDecayCurve,
    kUaParamReleaseCurve,
    kUaParamStart,
    kUaParamSnap,
    kUaParamReverse,
    kUaParamStretch,
    kUaParamSync,
    kUaParamBaseTempo,
    kUaParamProtect,
    kUaParamEnd,
    kUaParamDelay,
    kUaParamProtectSnap,
    kUaParamLowKey,
    kUaParamHighKey,
    kUaParamMute,
    kUaParamFilter,
    kUaParamLowCut,
    kUaParamHighCut,
    kUaParamSlope,
    kUaParamSaturate,
    kUaParamSatMode,
    kUaParamSatGain,
    kUaParamSatOutput,
    kUaParamSatWarmth,
    kUaParamSatHf,
    kUaParamCount
};

extern const tSynthLibParam gUaParams[kUaParamCount];
extern const tUaPage        gUaPages[];
extern const uint32_t       gUaPageCount;

// The text a host and the panel show for a parameter's value
void ua_param_text(uint32_t id, double normalized, char * out, size_t len);
double ua_volume_gain(double normalized);
double ua_start_time(double normalized);                               // notes §5: seconds into the sample
double ua_stretch_multiple(double normalized);                         // notes §6: x0.25 to x4
double ua_base_tempo(double normalized);                               // BPM
double ua_protect_time(double normalized);                             // seconds of the original; Start's law
double ua_end_fraction(double normalized);                             // notes §7: of the length as played, 0..1
double ua_delay_time(double normalized);
double ua_cut_hz(double normalized);                                   // notes §11: 20 Hz to 20 kHz
double ua_hf_hz(double normalized);                                    // 1 to 20 kHz
double ua_param_db(uint32_t id, double normalized);                    // Gain, Output, Warmth                               // notes §8: seconds a note waits; Start's law
void ua_time_text(double seconds, char * out, size_t len);
bool ua_hold_is_infinite(double normalized);                           // the Hold dial's top position
bool ua_is_gate(const double values[kUaParamCount]);                   // Mode is Gate: a note off releases
bool ua_param_active(uint32_t id, const double values[kUaParamCount]); // notes §4: false is drawn greyed
tUaEnvShape ua_env_shape(const double values[kUaParamCount]);          // normalized values in

#endif // __UA_PARAMS_H__
