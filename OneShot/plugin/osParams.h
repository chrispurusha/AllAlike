#ifndef __OS_PARAMS_H__
#define __OS_PARAMS_H__

#include "synthlibPlugin.h"
#include "osDraw.h"
#include "osEnvelope.h"

// APPENDED TO, NEVER INSERTED INTO: the ids are what host projects save
enum {
    kOsParamVolume = 0,
    kOsParamPan,
    kOsParamTune,
    kOsParamFine,
    kOsParamVelocity,
    kOsParamKeyTrack,
    kOsParamRootKey,
    kOsParamMode,
    kOsParamVoices,
    kOsParamEnvMode,
    kOsParamAttack,
    kOsParamHold,
    kOsParamDecay,
    kOsParamSustain,
    kOsParamRelease,
    kOsParamAttackCurve,
    kOsParamDecayCurve,
    kOsParamReleaseCurve,
    kOsParamStart,
    kOsParamSnap,
    kOsParamReverse,
    kOsParamStretch,
    kOsParamSync,
    kOsParamBaseTempo,
    kOsParamProtect,
    kOsParamEnd,
    kOsParamDelay,
    kOsParamProtectSnap,
    kOsParamCount
};

extern const tSynthLibParam gOsParams[kOsParamCount];
extern const tOsPage        gOsPages[];
extern const uint32_t       gOsPageCount;

// The text a host and the panel show for a parameter's value
void os_param_text(uint32_t id, double normalized, char * out, size_t len);
double os_volume_gain(double normalized);
double os_start_time(double normalized);                               // notes §5: seconds into the sample
double os_stretch_multiple(double normalized);                         // notes §6: x0.25 to x4
double os_base_tempo(double normalized);                               // BPM
double os_protect_time(double normalized);                             // seconds of the original; Start's law
double os_end_fraction(double normalized);                             // notes §7: of the length as played, 0..1
double os_delay_time(double normalized);                               // notes §8: seconds a note waits; Start's law
void os_time_text(double seconds, char * out, size_t len);
bool os_hold_is_infinite(double normalized);                           // the Hold dial's top position
bool os_is_gate(const double values[kOsParamCount]);                   // Mode is Gate: a note off releases
bool os_param_active(uint32_t id, const double values[kOsParamCount]); // notes §4: false is drawn greyed
tOsEnvShape os_env_shape(const double values[kOsParamCount]);          // normalized values in

#endif // __OS_PARAMS_H__
