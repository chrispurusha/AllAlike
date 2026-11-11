// Notes: Docs/code-notes/osParams.md - "// notes §k" refers there.

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "osEnvelope.h"
#include "osParams.h"

#define LIST            SYNTHLIB_PARAM_LIST
#define NONE            SYNTHLIB_MIDI_NONE
#define GEN             eSynthLibUnitGeneric
#define START_KNEE_S    (0.001)
#define START_MAX_S     (2.0)
#define IDX             eSynthLibUnitIndexed

// id, title, short title, unit, plain min, plain max, default (normalized), steps, MIDI, flags
const tSynthLibParam gOsParams[kOsParamCount] = {
    {kOsParamVolume,       "Volume",        "Volume",  GEN,                       0.0,  127.0, 103.0 / 127.0,  127, NONE,    0},
    {kOsParamPan,          "Pan",           "Pan",     GEN,                       0.0,  127.0,  64.0 / 127.0,  127, NONE,    0},
    {kOsParamTune,         "Tune",          "Tune",    eSynthLibUnitSemitones,  -24.0,   24.0,           0.5,   48, NONE,    0},
    {kOsParamFine,         "Fine Tune",     "Fine",    GEN,                     -50.0,   50.0,           0.5,  100, NONE,    0},
    {kOsParamVelocity,     "Velocity",      "Vel",     GEN,                       0.0,  127.0, 100.0 / 127.0,  127, NONE,    0},
    {kOsParamKeyTrack,     "Key Track",     "Keys",    IDX,                       0.0,    1.0,           0.0,    1, NONE, LIST},
    {kOsParamRootKey,      "Root Key",      "Root",    GEN,                       0.0,  127.0,  60.0 / 127.0,  127, NONE,    0},
    {kOsParamMode,         "Mode",          "Mode",    IDX,                       0.0,    1.0,           0.0,    1, NONE, LIST},
    {kOsParamVoices,       "Voices",        "Voices",  IDX,                       0.0,    1.0,           0.0,    1, NONE, LIST},
    // notes §3: the envelope; its defaults leave a sample as it is
    {kOsParamEnvMode,      "Envelope",      "Env",     IDX,                       0.0,    1.0,           0.0,    1, NONE, LIST},
    {kOsParamAttack,       "Attack",        "Attack",  GEN,                       0.0, 1000.0,           0.0, 1000, NONE,    0},
    {kOsParamHold,         "Hold",          "Hold",    GEN,                       0.0,  127.0,           1.0,  127, NONE,    0},
    {kOsParamDecay,        "Decay",         "Decay",   GEN,                       0.0,  127.0,  64.0 / 127.0,  127, NONE,    0},
    {kOsParamSustain,      "Sustain",       "Sustain", GEN,                       0.0,  127.0,           1.0,  127, NONE,    0},
    {kOsParamRelease,      "Release",       "Release", GEN,                       0.0,  127.0,  35.0 / 127.0,  127, NONE,    0},
    {kOsParamAttackCurve,  "Attack Curve",  "A Curve", GEN,                    -100.0,  100.0, 100.0 / 200.0,  200, NONE,    0},
    {kOsParamDecayCurve,   "Decay Curve",   "D Curve", GEN,                    -100.0,  100.0, 150.0 / 200.0,  200, NONE,    0},
    {kOsParamReleaseCurve, "Release Curve", "R Curve", GEN,                    -100.0,  100.0, 150.0 / 200.0,  200, NONE,    0},
    {kOsParamStart,        "Start",         "Start",   GEN,                       0.0, 1000.0,           0.0, 1000, NONE,    0},
    {kOsParamSnap,         "Snap to Zero",  "Snap",    IDX,                       0.0,    1.0,           1.0,    1, NONE, LIST},
    {kOsParamReverse,      "Reverse",       "Rev",     IDX,                       0.0,    1.0,           0.0,    1, NONE, LIST},
};

static const char *const kOffOn[]    = {"Off", "On"};
static const char *const kModes[]    = {"One Shot", "Gate"};
static const char *const kVoices[]   = {"Poly", "Mono"};
static const char *const kEnvModes[] = {"AHR", "ADSR"};

static long step_of(uint32_t id, double normalized) {
    return lround(normalized * gOsParams[id].stepCount);
}

bool os_hold_is_infinite(double normalized) {
    return step_of(kOsParamHold, normalized) >= 127;
}

bool os_is_gate(const double values[kOsParamCount]) {
    return step_of(kOsParamMode, values[kOsParamMode]) > 0;
}

static double stepped(const double values[kOsParamCount], uint32_t id) {
    return (double)step_of(id, values[id]) / gOsParams[id].stepCount;
}

// notes §4: Decay and Sustain are ADSR's; in ADSR the Release needs a note off, which only Gate acts on, while
// AHR's Release follows its Hold and always acts
bool os_param_active(uint32_t id, const double values[kOsParamCount]) {
    bool adsr = step_of(kOsParamEnvMode, values[kOsParamEnvMode]) > 0;

    switch (id) {
        case kOsParamDecay:
        case kOsParamDecayCurve:
        case kOsParamSustain:
            return adsr;

        case kOsParamRelease:
        case kOsParamReleaseCurve:
            return !adsr || os_is_gate(values);

        case kOsParamRootKey:
            return step_of(kOsParamKeyTrack, values[kOsParamKeyTrack]) > 0;

        default:
            return true;
    }
}

tOsEnvShape os_env_shape(const double values[kOsParamCount]) {
    return (tOsEnvShape){
        .adsr         = step_of(kOsParamEnvMode, values[kOsParamEnvMode]) > 0,
        .attack       = os_env_attack_time(stepped(values, kOsParamAttack)),
        .hold         = os_hold_is_infinite(values[kOsParamHold]) ? -1.0 : os_env_time(stepped(values, kOsParamHold)),
        .decay        = os_env_time(stepped(values, kOsParamDecay)),
        .sustain      = stepped(values, kOsParamSustain),
        .release      = os_env_time(stepped(values, kOsParamRelease)),
        .attackCurve  = os_env_curve(stepped(values, kOsParamAttackCurve)),
        .decayCurve   = os_env_curve(stepped(values, kOsParamDecayCurve)),
        .releaseCurve = os_env_curve(stepped(values, kOsParamReleaseCurve)),
    };
}

static void time_text(double seconds, char * out, size_t len) {
    if (seconds < 0.000005) {
        snprintf(out, len, "0 ms");
    } else if (seconds < 0.001) {
        snprintf(out, len, "%.2f ms", seconds * 1000.0);
    } else if (seconds < 0.01) {
        snprintf(out, len, "%.1f ms", seconds * 1000.0);
    } else if (seconds < 1.0) {
        snprintf(out, len, "%.0f ms", seconds * 1000.0);
    } else {
        snprintf(out, len, "%.2f s", seconds);
    }
}

// notes §5: as Attack's law - exponential above 1 ms, to 2 s; the first 5 ms about a quarter of the dial
double os_start_time(double normalized) {
    double v = fmin(1.0, fmax(0.0, normalized));

    return START_KNEE_S * (pow((START_MAX_S / START_KNEE_S) + 1.0, v) - 1.0);
}

// notes §1: 0 is silence, then -57 to +6 dB in 0.5 dB steps - 0 dB at 115, the default -6 dB at 103
double os_volume_gain(double normalized) {
    long step = step_of(kOsParamVolume, normalized);

    return (step <= 0) ? 0.0 : pow(10.0, (-57.0 + ((double)(step - 1) * 0.5)) / 20.0);
}

void os_param_text(uint32_t id, double normalized, char * out, size_t len) {
    static const char *const kNotes[] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
    long                     step     = step_of(id, normalized);

    switch (id) {
        case kOsParamVolume:
        {
            double gain = os_volume_gain(normalized);

            if (gain <= 0.0) {
                snprintf(out, len, "-inf");
            } else {
                snprintf(out, len, "%.1f dB", 20.0 * log10(gain));
            }
            break;
        }

        case kOsParamPan:
            snprintf(out, len, (step == 64) ? "C" : ((step < 64) ? "L%ld" : "R%ld"), labs(step - 64));
            break;

        case kOsParamTune:
            snprintf(out, len, "%+ld st", step - 24);
            break;

        case kOsParamFine:
            snprintf(out, len, "%+ld ct", step - 50);
            break;

        case kOsParamRootKey:   // notes §2: middle C (60) is C3, as Ableton names it
            snprintf(out, len, "%s%ld", kNotes[step % 12], (step / 12) - 2);
            break;

        case kOsParamKeyTrack:
            snprintf(out, len, "%s", kOffOn[step > 0]);
            break;

        case kOsParamMode:
            snprintf(out, len, "%s", kModes[step > 0]);
            break;

        case kOsParamVoices:
            snprintf(out, len, "%s", kVoices[step > 0]);
            break;

        case kOsParamEnvMode:
            snprintf(out, len, "%s", kEnvModes[step > 0]);
            break;

        case kOsParamHold:

            if (os_hold_is_infinite(normalized)) {
                snprintf(out, len, "Inf");
                break;
            }
        // fall through

        case kOsParamDecay:
        case kOsParamRelease:
            time_text(os_env_time((double)step / 127.0), out, len);
            break;

        case kOsParamAttack:
            time_text(os_env_attack_time((double)step / 1000.0), out, len);
            break;

        case kOsParamStart:
            time_text(os_start_time((double)step / 1000.0), out, len);
            break;

        case kOsParamSnap:
        case kOsParamReverse:
            snprintf(out, len, "%s", kOffOn[step > 0]);
            break;

        case kOsParamSustain:
            snprintf(out, len, "%.0f%%", (double)step * 100.0 / 127.0);
            break;

        case kOsParamAttackCurve:
        case kOsParamDecayCurve:
        case kOsParamReleaseCurve:
            snprintf(out, len, "%+ld", step - 100);
            break;

        default:
            snprintf(out, len, "%ld", step);
            break;
    }
}

#define DIAL(id, label)             {id, label, eOsControlDial, NULL}
#define CHOICE(id, label, names)    {id, label, eOsControlChoice, names}
#define COUNT(a)                    (sizeof(a) / sizeof((a)[0]))

static const tOsControl kPitch[]     = {
    DIAL(kOsParamTune, "Tune"), DIAL(kOsParamFine, "Fine"), CHOICE(kOsParamKeyTrack, "Keys", kOffOn), DIAL(kOsParamRootKey, "Root"),
};
static const tOsControl kPlay[]      = {
    CHOICE(kOsParamMode, "Mode",   kModes),             CHOICE(kOsParamVoices, "Voices", kVoices), CHOICE(kOsParamReverse, "Reverse", kOffOn),
    DIAL(kOsParamStart,  "Start"), CHOICE(kOsParamSnap, "Snap",                kOffOn),
};
static const tOsControl kOutput[]    = {
    DIAL(kOsParamVelocity, "Velocity"), DIAL(kOsParamPan, "Pan"), DIAL(kOsParamVolume, "Volume"),
};

static const tOsControl kEnvelope[]  = {
    CHOICE(kOsParamEnvMode, "Type",   kEnvModes),           DIAL(kOsParamAttack, "Attack"),            DIAL(kOsParamHold, "Hold"),
    DIAL(kOsParamDecay,     "Decay"), DIAL(kOsParamSustain, "Sustain"),          DIAL(kOsParamRelease, "Release"),
};
static const tOsControl kCurves[]    = {
    DIAL(kOsParamAttackCurve, "Attack"), DIAL(kOsParamDecayCurve, "Decay"), DIAL(kOsParamReleaseCurve, "Release"),
};

static const tOsSection kMainPage[]  = {
    {"PITCH",    {0.90, 0.70, 0.30}, kPitch,    COUNT(kPitch)   },
    {"PLAY",     {0.40, 0.70, 0.85}, kPlay,     COUNT(kPlay)    },
    {"OUTPUT",   {0.75, 0.75, 0.75}, kOutput,   COUNT(kOutput)  },
    {"ENVELOPE", {0.55, 0.80, 0.40}, kEnvelope, COUNT(kEnvelope)},
    {"CURVE",    {0.55, 0.80, 0.40}, kCurves,   COUNT(kCurves)  },
};

const tOsPage           gOsPages[]   = {
    {"SAMPLE", kMainPage, COUNT(kMainPage)},
};
const uint32_t          gOsPageCount = COUNT(gOsPages);
