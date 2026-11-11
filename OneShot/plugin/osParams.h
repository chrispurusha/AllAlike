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
    kOsParamCount
};

extern const tSynthLibParam gOsParams[kOsParamCount];
extern const tOsPage        gOsPages[];
extern const uint32_t       gOsPageCount;

// The text a host and the panel show for a parameter's value
void os_param_text(uint32_t id, double normalized, char * out, size_t len);
double os_volume_gain(double normalized);
double os_start_time(double normalized);                               // notes §5: seconds into the sample
bool os_hold_is_infinite(double normalized);                           // the Hold dial's top position
bool os_is_gate(const double values[kOsParamCount]);                   // Mode is Gate: a note off releases
bool os_param_active(uint32_t id, const double values[kOsParamCount]); // notes §4: false is drawn greyed
tOsEnvShape os_env_shape(const double values[kOsParamCount]);          // normalized values in

#endif // __OS_PARAMS_H__
