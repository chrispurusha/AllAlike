// Notes: Docs/code-notes/uaParams.md - "// notes §k" refers there.

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "uaEnvelope.h"
#include "uaParams.h"

#define LIST            SYNTHLIB_PARAM_LIST
#define NONE            SYNTHLIB_MIDI_NONE
#define GEN             eSynthLibUnitGeneric
#define START_KNEE_S    (0.001)
#define START_MAX_S     (2.0)
#define IDX             eSynthLibUnitIndexed

// id, title, short title, unit, plain min, plain max, default (normalized), steps, MIDI, flags
const tSynthLibParam gUaParams[kUaParamCount] = {
    {kUaParamVolume,       "Volume",          "Volume",  GEN,                      -96.0,   10.0,   96.0 / 106.0,   106, NONE,                                                                  0},
    {kUaParamPan,          "Pan",             "Pan",     GEN,                        0.0,  127.0,   64.0 / 127.0,   127, NONE,                                                                  0},
    {kUaParamTune,         "Coarse",          "Coarse",  eSynthLibUnitSemitones,   -36.0,   36.0,            0.5,    72, NONE,                                                                  0},
    {kUaParamFine,         "Fine",            "Fine",    GEN,                      -64.0,   64.0,            0.5,   128, NONE,                                                                  0},
    {kUaParamVelocity,     "Velocity",        "Vel",     GEN,                        0.0,  127.0,  100.0 / 127.0,   127, NONE,                                                                  0},
    {kUaParamKeyTrack,     "Non-Transpose",   "NonTr",   IDX,                        0.0,    1.0,            0.0,     1, NONE,                     LIST                                          },
    {kUaParamRootKey,      "Root Key",        "Root",    GEN,                        0.0,  127.0,   60.0 / 127.0,   127, NONE,                                                                  0},
    {kUaParamMode,         "Mode",            "Mode",    IDX,                        0.0,    1.0,            0.0,     1, NONE,                     LIST | SYNTHLIB_PARAM_HIDDEN                  },
    {kUaParamVoices,       "Voices",          "Voices",  IDX,                        0.0,    1.0,            1.0,     1, NONE,                     LIST | SYNTHLIB_PARAM_HIDDEN                  },
    // notes §3: the envelope; its defaults leave a sample as it is
    {kUaParamEnvMode,      "Envelope",        "Env",     IDX,                        0.0,    1.0,            0.0,     1, NONE,                     LIST | SYNTHLIB_PARAM_HIDDEN                  },
    {kUaParamAttack,       "Attack",          "Attack",  GEN,                        0.0, 1000.0,            0.0,  1000, NONE,                     SYNTHLIB_PARAM_HIDDEN                         },
    {kUaParamHold,         "Hold",            "Hold",    GEN,                        0.0,  127.0,            1.0,   127, NONE,                     SYNTHLIB_PARAM_HIDDEN                         },
    {kUaParamDecay,        "Decay",           "Decay",   GEN,                        0.0,  127.0,   64.0 / 127.0,   127, NONE,                     SYNTHLIB_PARAM_HIDDEN                         },
    {kUaParamSustain,      "Sustain",         "Sustain", GEN,                        0.0,  127.0,            1.0,   127, NONE,                     SYNTHLIB_PARAM_HIDDEN                         },
    {kUaParamRelease,      "Release",         "Release", GEN,                        0.0,  127.0,   35.0 / 127.0,   127, NONE,                     SYNTHLIB_PARAM_HIDDEN                         },
    {kUaParamAttackCurve,  "Attack Curve",    "A Curve", GEN,                     -100.0,  100.0,  100.0 / 200.0,   200, NONE,                     SYNTHLIB_PARAM_HIDDEN                         },
    {kUaParamDecayCurve,   "Decay Curve",     "D Curve", GEN,                     -100.0,  100.0,  150.0 / 200.0,   200, NONE,                     SYNTHLIB_PARAM_HIDDEN                         },
    {kUaParamReleaseCurve, "Release Curve",   "R Curve", GEN,                     -100.0,  100.0,  150.0 / 200.0,   200, NONE,                     SYNTHLIB_PARAM_HIDDEN                         },
    {kUaParamStart,        "Start",           "Start",   GEN,                        0.0, 1000.0,            0.0,  1000, NONE,                     SYNTHLIB_PARAM_HIDDEN                         },
    {kUaParamSnap,         "Snap to Zero",    "Snap",    IDX,                        0.0,    1.0,            1.0,     1, NONE,                     LIST | SYNTHLIB_PARAM_HIDDEN                  },
    {kUaParamReverse,      "Reverse",         "Rev",     IDX,                        0.0,    1.0,            0.0,     1, NONE,                     LIST | SYNTHLIB_PARAM_HIDDEN                  },
    // notes §6: time-stretching
    {kUaParamStretch,      "Stretch",         "Stretch", GEN,                     -200.0,  200.0,            0.5,   400, NONE,                     SYNTHLIB_PARAM_HIDDEN                         },
    {kUaParamSync,         "Tempo Sync",      "Sync",    IDX,                        0.0,    1.0,            0.0,     1, NONE,                     LIST | SYNTHLIB_PARAM_HIDDEN                  },
    {kUaParamBaseTempo,    "Base Tempo",      "Base",    GEN,                       40.0,  240.0,            0.4,   400, NONE,                     SYNTHLIB_PARAM_HIDDEN                         },
    {kUaParamProtect,      "Protect",         "Protect", GEN,                        0.0, 1000.0, 236.0 / 1000.0,  1000, NONE,                     SYNTHLIB_PARAM_HIDDEN                         },
    {kUaParamEnd,          "End",             "End",     GEN,                        0.0,  100.0,            1.0,  1000, NONE,                     SYNTHLIB_PARAM_HIDDEN                         },
    {kUaParamDelay,        "Delay",           "Delay",   GEN,                        0.0, 1000.0,            0.0,  1000, NONE,                     SYNTHLIB_PARAM_HIDDEN                         },
    {kUaParamProtectSnap,  "Protect Snap",    "PSnap",   IDX,                        0.0,    1.0,            1.0,     1, NONE,                     LIST | SYNTHLIB_PARAM_HIDDEN                  },
    {kUaParamLowKey,       "Low Key",         "Low",     GEN,                        0.0,  127.0,            0.0,   127, NONE,                                                                  0},
    {kUaParamHighKey,      "High Key",        "High",    GEN,                        0.0,  127.0,            1.0,   127, NONE,                                                                  0},
    {kUaParamMute,         "Mute",            "Mute",    IDX,                        0.0,    1.0,            0.0,     1, NONE,                     LIST | SYNTHLIB_PARAM_HIDDEN                  },
    // notes §11: the layer's filter and saturation
    {kUaParamFilter,       "Filter",          "Filter",  IDX,                        0.0,    1.0,            0.0,     1, NONE,                     LIST | SYNTHLIB_PARAM_HIDDEN                  },
    {kUaParamLowCut,       "Low Cut",         "LoCut",   GEN,                        0.0, 1000.0,            0.0,  1000, NONE,                     SYNTHLIB_PARAM_HIDDEN                         },
    {kUaParamHighCut,      "High Cut",        "HiCut",   GEN,                        0.0, 1000.0,            1.0,  1000, NONE,                     SYNTHLIB_PARAM_HIDDEN                         },
    {kUaParamSlope,        "Filter Slope",    "Slope",   IDX,                        0.0,    2.0,            0.5,     2, NONE,                     LIST | SYNTHLIB_PARAM_HIDDEN                  },
    {kUaParamSaturate,     "Saturation",      "Sat",     IDX,                        0.0,    1.0,            0.0,     1, NONE,                     LIST | SYNTHLIB_PARAM_HIDDEN                  },
    {kUaParamSatMode,      "Sat Mode",        "Mode",    IDX,                        0.0,    2.0,            0.0,     2, NONE,                     LIST | SYNTHLIB_PARAM_HIDDEN                  },
    {kUaParamSatGain,      "Sat Gain",        "Gain",    GEN,                        0.0,   30.0,            0.0,    60, NONE,                     SYNTHLIB_PARAM_HIDDEN                         },
    {kUaParamSatOutput,    "Sat Output",      "Output",  GEN,                      -30.0,    6.0,    60.0 / 72.0,    72, NONE,                     SYNTHLIB_PARAM_HIDDEN                         },
    {kUaParamSatWarmth,    "Sat Warmth",      "Warmth",  GEN,                      -12.0,   12.0,            0.5,    48, NONE,                     SYNTHLIB_PARAM_HIDDEN                         },
    {kUaParamSatHf,        "Sat HF",          "HF",      GEN,                        0.0, 1000.0,            1.0,  1000, NONE,                     SYNTHLIB_PARAM_HIDDEN                         },
    // notes §13: global
    {kUaParamAntiAlias,    "Anti-alias",      "AA",      IDX,                        0.0,    1.0,            1.0,     1, NONE,                     LIST                                          },
    {kUaParamPolarity,     "Polarity",        "Pol",     IDX,                        0.0,    1.0,            0.0,     1, NONE,                     LIST                                          },
    // notes §14: the E5000's own controls
    {kUaParamTranspose,    "Transpose",       "Transp",  eSynthLibUnitSemitones,   -24.0,   24.0,            0.5,    48, NONE,                                                                  0},
    {kUaParamSolo,         "Solo",            "Solo",    IDX,                        0.0,    8.0,            0.0,     8, NONE,                     LIST                                          },
    {kUaParamEnvDepth,     "Env Depth",       "Depth",   IDX,                        0.0,   16.0,            0.0,    16, NONE,                                                                  0},
    {kUaParamAtk1Rate,     "Attack 1 Rate",   "Atk1",    IDX,                        0.0,  127.0,            0.0,   127, NONE,                                                                  0},
    {kUaParamAtk2Rate,     "Attack 2 Rate",   "Atk2",    IDX,                        0.0,  127.0,            0.0,   127, NONE,                                                                  0},
    {kUaParamDcy1Rate,     "Decay 1 Rate",    "Dcy1",    IDX,                        0.0,  127.0,            0.0,   127, NONE,                                                                  0},
    {kUaParamDcy2Rate,     "Decay 2 Rate",    "Dcy2",    IDX,                        0.0,  127.0,            0.0,   127, NONE,                                                                  0},
    {kUaParamRls1Rate,     "Release 1 Rate",  "Rls1",    IDX,                        0.0,  127.0,   30.0 / 127.0,   127, NONE,                                                                  0},
    {kUaParamRls2Rate,     "Release 2 Rate",  "Rls2",    IDX,                        0.0,  127.0,            0.0,   127, NONE,                                                                  0},
    {kUaParamAtk1Level,    "Attack 1 Level",  "A1 Lvl",  IDX,                        0.0,  100.0,            1.0,   100, NONE,                                                                  0},
    {kUaParamAtk2Level,    "Attack 2 Level",  "A2 Lvl",  IDX,                        0.0,  100.0,            1.0,   100, NONE,                                                                  0},
    {kUaParamDcy1Level,    "Decay 1 Level",   "D1 Lvl",  IDX,                        0.0,  100.0,            1.0,   100, NONE,                                                                  0},
    {kUaParamDcy2Level,    "Decay 2 Level",   "D2 Lvl",  IDX,                        0.0,  100.0,            1.0,   100, NONE,                                                                  0},
    {kUaParamRls1Level,    "Release 1 Level", "R1 Lvl",  IDX,                        0.0,  100.0,            0.0,   100, NONE,                                                                  0},
    {kUaParamRls2Level,    "Release 2 Level", "R2 Lvl",  IDX,                        0.0,  100.0,            0.0,   100, NONE,                                                                  0},
    {kUaParamSustainPedal, "Sustain Pedal",   "Pedal",   GEN,                        0.0,  127.0,            0.0,   127, SYNTHLIB_MIDI_SUSTAIN,    SYNTHLIB_PARAM_HIDDEN | SYNTHLIB_PARAM_NO_SAVE},
    {kUaParamPitchBend,    "Pitch Bend",      "Bend",    GEN,                    -8192.0, 8191.0,            0.5, 16383, SYNTHLIB_MIDI_PITCH_BEND, SYNTHLIB_PARAM_HIDDEN | SYNTHLIB_PARAM_NO_SAVE},
    {kUaParamModWheel,     "Mod Wheel",       "Mod",     GEN,                        0.0,  127.0,            0.0,   127, SYNTHLIB_MIDI_MOD_WHEEL,  SYNTHLIB_PARAM_HIDDEN | SYNTHLIB_PARAM_NO_SAVE},
    {kUaParamBendRange,    "Bend Range",      "Bend",    eSynthLibUnitSemitones,     0.0,   12.0,     2.0 / 12.0,    12, NONE,                                                                  0},
    {kUaParamVoiceLimit,   "Voice Limit",     "Voices",  IDX,                        1.0,   64.0,            1.0,    63, NONE,                                                                  0},
    {kUaParamSteal,        "Steal",           "Steal",   IDX,                        0.0,    1.0,            0.0,     1, NONE,                     LIST                                          },
    {kUaParamSameNote,     "Same Note",       "Same",    IDX,                        0.0,    1.0,            0.0,     1, NONE,                     LIST                                          },
};

static const char *const kOffOn[]    = {"Off", "On"};
static const char *const kModes[]    = {"One Shot", "Gate"};
static const char *const kVoices[]   = {"Poly", "Mono"};
static const char *const kEnvModes[] = {"AHR", "AHDSR"};
static const char *const kSlopes[]   = {"Soft", "12 dB", "24 dB"};
static const char *const kSatModes[] = {"Classic", "Drums", "Tape"};
static const char *const kPolarity[] = {"Normal", "Inverted"};
static const char *const kSteal[]    = {"Released", "Oldest"};
static const char *const kSameNote[] = {"Stack", "Piano"};
static const char *const kSolo[]     = {"Off", "Multi Trig", "Melody Last", "Melody Low", "Melody High", "Synth Last", "Synth Low", "Synth High", "Fingered"};

static long step_of(uint32_t id, double normalized) {
    return lround(normalized * gUaParams[id].stepCount);
}

bool ua_hold_is_infinite(double normalized) {
    return step_of(kUaParamHold, normalized) >= 127;
}

bool ua_is_gate(const double values[kUaParamCount]) {
    return step_of(kUaParamMode, values[kUaParamMode]) > 0;
}

static double stepped(const double values[kUaParamCount], uint32_t id) {
    return (double)step_of(id, values[id]) / gUaParams[id].stepCount;
}

// notes §4: Decay and Sustain are AHDSR's; in AHDSR the Release needs a note off, which only Gate acts on, while
// AHR's Release follows its Hold and always acts
bool ua_param_active(uint32_t id, const double values[kUaParamCount]) {
    bool adsr = step_of(kUaParamEnvMode, values[kUaParamEnvMode]) > 0;

    switch (id) {
        case kUaParamDecay:
        case kUaParamDecayCurve:
        case kUaParamSustain:
            return adsr;

        case kUaParamRelease:
        case kUaParamReleaseCurve:
            return !adsr || ua_is_gate(values);

        case kUaParamRootKey:   // notes §14: the root matters while keys transpose
            return step_of(kUaParamKeyTrack, values[kUaParamKeyTrack]) == 0;

        case kUaParamBaseTempo:
            return step_of(kUaParamSync, values[kUaParamSync]) > 0;

        case kUaParamLowCut:
        case kUaParamHighCut:
        case kUaParamSlope:
            return step_of(kUaParamFilter, values[kUaParamFilter]) > 0;

        case kUaParamSatMode:
        case kUaParamSatGain:
        case kUaParamSatOutput:
            return step_of(kUaParamSaturate, values[kUaParamSaturate]) > 0;

        case kUaParamSatWarmth:
        case kUaParamSatHf:
            return (step_of(kUaParamSaturate, values[kUaParamSaturate]) > 0) && (step_of(kUaParamSatMode, values[kUaParamSatMode]) == 2);

        case kUaParamProtect:
        case kUaParamProtectSnap:
            return (step_of(kUaParamStretch, values[kUaParamStretch]) != 200) || (step_of(kUaParamSync, values[kUaParamSync]) > 0);

        default:
            return true;
    }
}

tUaEnvShape ua_env_shape(const double values[kUaParamCount]) {
    return (tUaEnvShape){
        .adsr         = step_of(kUaParamEnvMode, values[kUaParamEnvMode]) > 0,
        .attack       = ua_env_attack_time(stepped(values, kUaParamAttack)),
        .hold         = ua_hold_is_infinite(values[kUaParamHold]) ? -1.0 : ua_env_time(stepped(values, kUaParamHold)),
        .decay        = ua_env_time(stepped(values, kUaParamDecay)),
        .sustain      = stepped(values, kUaParamSustain),
        .release      = ua_env_time(stepped(values, kUaParamRelease)),
        .attackCurve  = ua_env_curve(stepped(values, kUaParamAttackCurve)),
        .decayCurve   = ua_env_curve(stepped(values, kUaParamDecayCurve)),
        .releaseCurve = ua_env_curve(stepped(values, kUaParamReleaseCurve)),
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
double ua_start_time(double normalized) {
    double v = fmin(1.0, fmax(0.0, normalized));

    return START_KNEE_S * (pow((START_MAX_S / START_KNEE_S) + 1.0, v) - 1.0);
}

// notes §6: 2^(k/100) - a hundred steps an octave, x1 in the middle
double ua_stretch_multiple(double normalized) {
    return pow(2.0, (double)(step_of(kUaParamStretch, normalized) - 200) / 100.0);
}

double ua_base_tempo(double normalized) {
    return 40.0 + ((double)step_of(kUaParamBaseTempo, normalized) * 0.5);
}

// notes §7: a tenth of a percent a step; the panel shows it as a time, which needs the sample
double ua_end_fraction(double normalized) {
    return (double)step_of(kUaParamEnd, normalized) / 1000.0;
}

// notes §8: as Start's - 0 to 2 s, exponential above 1 ms, so the first steps are microseconds for lining up layers
double ua_delay_time(double normalized) {
    return ua_start_time((double)step_of(kUaParamDelay, normalized) / 1000.0);
}

// notes §11: three decades, a thousand steps - about 0.7% each
double ua_cut_hz(double normalized) {
    return 20.0 * pow(1000.0, (double)step_of(kUaParamLowCut, normalized) / 1000.0);
}

double ua_hf_hz(double normalized) {
    return 1000.0 * pow(20.0, (double)step_of(kUaParamSatHf, normalized) / 1000.0);
}

double ua_param_db(uint32_t id, double normalized) {
    return gUaParams[id].plainMin + ((gUaParams[id].plainMax - gUaParams[id].plainMin) * (double)step_of(id, normalized) / gUaParams[id].stepCount);
}

static void hz_text(double hz, char * out, size_t len) {
    if (hz >= 10000.0) {
        snprintf(out, len, "%.1f kHz", hz / 1000.0);
    } else if (hz >= 1000.0) {
        snprintf(out, len, "%.2f kHz", hz / 1000.0);
    } else {
        snprintf(out, len, "%.0f Hz", hz);
    }
}

void ua_time_text(double seconds, char * out, size_t len) {
    time_text(seconds, out, len);
}

// notes §6: as Start's - 0 to 2 s of the original, the default 5 ms
double ua_protect_time(double normalized) {
    return ua_start_time((double)step_of(kUaParamProtect, normalized) / 1000.0);
}

// notes §1: -96 to +10 dB in 1 dB steps, as the E5000's Volume; -96 is silence
double ua_volume_gain(double normalized) {
    long db = step_of(kUaParamVolume, normalized) - 96;

    return (db <= -96) ? 0.0 : pow(10.0, (double)db / 20.0);
}

double ua_fine_semitones(double normalized) {
    return (double)(step_of(kUaParamFine, normalized) - 64) / 64.0;
}

void ua_param_text(uint32_t id, double normalized, char * out, size_t len) {
    static const char *const kNotes[] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
    long                     step     = step_of(id, normalized);

    switch (id) {
        case kUaParamVolume:
        {
            double gain = ua_volume_gain(normalized);

            if (gain <= 0.0) {
                snprintf(out, len, "-inf");
            } else {
                snprintf(out, len, "%+.0f dB", 20.0 * log10(gain));
            }
            break;
        }

        case kUaParamPan:
            snprintf(out, len, (step == 64) ? "C" : ((step < 64) ? "L%ld" : "R%ld"), labs(step - 64));
            break;

        case kUaParamTune:
            snprintf(out, len, "%+ld st", step - 36);
            break;

        case kUaParamTranspose:
            snprintf(out, len, "%+ld st", step - 24);
            break;

        case kUaParamBendRange:
            snprintf(out, len, "%ld st", step);
            break;

        case kUaParamFine:
            snprintf(out, len, "%+ld", step - 64);
            break;

        case kUaParamSolo:
            snprintf(out, len, "%s", kSolo[(step < 0) ? 0 : ((step > 8) ? 8 : step)]);
            break;

        case kUaParamEnvDepth:
            snprintf(out, len, "-%ld dB", 96 - (3 * step));
            break;

        case kUaParamAtk1Level:
        case kUaParamAtk2Level:
        case kUaParamDcy1Level:
        case kUaParamDcy2Level:
        case kUaParamRls1Level:
        case kUaParamRls2Level:
            snprintf(out, len, "%ld%%", step);
            break;

        case kUaParamRootKey:   // notes §2: middle C (60) is C3, as Ableton names it
        case kUaParamLowKey:
        case kUaParamHighKey:
            snprintf(out, len, "%s%ld", kNotes[step % 12], (step / 12) - 2);
            break;

        case kUaParamKeyTrack:
            snprintf(out, len, "%s", kOffOn[step > 0]);
            break;

        case kUaParamMode:
            snprintf(out, len, "%s", kModes[step > 0]);
            break;

        case kUaParamVoices:
            snprintf(out, len, "%s", kVoices[step > 0]);
            break;

        case kUaParamEnvMode:
            snprintf(out, len, "%s", kEnvModes[step > 0]);
            break;

        case kUaParamHold:

            if (ua_hold_is_infinite(normalized)) {
                snprintf(out, len, "Inf");
                break;
            }
        // fall through

        case kUaParamDecay:
        case kUaParamRelease:
            time_text(ua_env_time((double)step / 127.0), out, len);
            break;

        case kUaParamAttack:
            time_text(ua_env_attack_time((double)step / 1000.0), out, len);
            break;

        case kUaParamStart:
            time_text(ua_start_time((double)step / 1000.0), out, len);
            break;

        case kUaParamSnap:
        case kUaParamReverse:
        case kUaParamSync:
        case kUaParamProtectSnap:
        case kUaParamMute:
        case kUaParamFilter:
        case kUaParamSaturate:
        case kUaParamAntiAlias:
            snprintf(out, len, "%s", kOffOn[step > 0]);
            break;

        case kUaParamPolarity:
            snprintf(out, len, "%s", kPolarity[step > 0]);
            break;

        case kUaParamSteal:
            snprintf(out, len, "%s", kSteal[step > 0]);
            break;

        case kUaParamSameNote:
            snprintf(out, len, "%s", kSameNote[step > 0]);
            break;

        case kUaParamVoiceLimit:
            snprintf(out, len, "%ld", 1 + step);
            break;

        case kUaParamStretch:
            snprintf(out, len, "x%.2f", ua_stretch_multiple(normalized));
            break;

        case kUaParamBaseTempo:
            snprintf(out, len, "%.1f", ua_base_tempo(normalized));
            break;

        case kUaParamEnd:
            snprintf(out, len, "%.1f%%", ua_end_fraction(normalized) * 100.0);
            break;

        case kUaParamDelay:
            time_text(ua_delay_time(normalized), out, len);
            break;

        case kUaParamLowCut:
        case kUaParamHighCut:
            hz_text(ua_cut_hz(normalized), out, len);
            break;

        case kUaParamSatHf:
            hz_text(ua_hf_hz(normalized), out, len);
            break;

        case kUaParamSlope:
            snprintf(out, len, "%s", kSlopes[(step < 0) ? 0 : ((step > 2) ? 2 : step)]);
            break;

        case kUaParamSatMode:
            snprintf(out, len, "%s", kSatModes[(step < 0) ? 0 : ((step > 2) ? 2 : step)]);
            break;

        case kUaParamSatGain:
        case kUaParamSatOutput:
        case kUaParamSatWarmth:
            snprintf(out, len, "%+.1f dB", ua_param_db(id, normalized));
            break;

        case kUaParamProtect:
            time_text(ua_protect_time(normalized), out, len);
            break;

        case kUaParamSustain:
            snprintf(out, len, "%.0f%%", (double)step * 100.0 / 127.0);
            break;

        case kUaParamAttackCurve:
        case kUaParamDecayCurve:
        case kUaParamReleaseCurve:
            snprintf(out, len, "%+ld", step - 100);
            break;

        default:
            snprintf(out, len, "%ld", step);
            break;
    }
}

#define DIAL(id, label)             {id, label, eUaControlDial, NULL}
#define CHOICE(id, label, names)    {id, label, eUaControlChoice, names}
#define TOGGLE(id, label)           {id, label, eUaControlToggle, kOffOn}
#define COUNT(a)                    (sizeof(a) / sizeof((a)[0]))

static const tUaControl kPitch[]      = {  // notes §14
    DIAL(kUaParamTranspose, "Transp"), DIAL(kUaParamTune,   "Coarse"), DIAL(kUaParamFine,    "Fine"), TOGGLE(kUaParamKeyTrack, "NonTr"),
    DIAL(kUaParamRootKey,   "Root"),   DIAL(kUaParamLowKey, "Low"),    DIAL(kUaParamHighKey, "High"),
};
static const tUaControl kPlay[]       = {
    CHOICE(kUaParamSolo, "Solo", kSolo), DIAL(kUaParamBendRange, "Bend"),
};
static const tUaControl kOutput[]     = {
    DIAL(kUaParamVolume, "Volume"), DIAL(kUaParamPan, "Pan"), DIAL(kUaParamVelocity, "Velocity"),
};
static const tUaControl kEnvRate[]    = {
    DIAL(kUaParamAtk1Rate, "Atk1"), DIAL(kUaParamAtk2Rate, "Atk2"), DIAL(kUaParamDcy1Rate, "Dcy1"),
    DIAL(kUaParamDcy2Rate, "Dcy2"), DIAL(kUaParamRls1Rate, "Rls1"), DIAL(kUaParamRls2Rate, "Rls2"),
};
static const tUaControl kEnvLevel[]   = {
    DIAL(kUaParamAtk1Level, "Atk1"), DIAL(kUaParamAtk2Level, "Atk2"), DIAL(kUaParamDcy1Level, "Dcy1"),
    DIAL(kUaParamDcy2Level, "Dcy2"), DIAL(kUaParamRls1Level, "Rls1"), DIAL(kUaParamRls2Level, "Rls2"),DIAL(kUaParamEnvDepth,  "Depth"),
};
static const tUaControl kGlobal[]     = {  // notes §13
    TOGGLE(kUaParamAntiAlias, "Anti-alias"), CHOICE(kUaParamPolarity, "Polarity", kPolarity),
    DIAL(kUaParamVoiceLimit,  "Voices"),     CHOICE(kUaParamSteal,    "Steal",    kSteal),   CHOICE(kUaParamSameNote,"Same Note", kSameNote),
};
static const tUaSection kOscPage[]    = {   // notes §12: the pages in the Alikes' common order (uaDraw notes §22)
    {"PITCH", {0.90, 0.70, 0.30}, kPitch, COUNT(kPitch)},
    {"PLAY",  {0.40, 0.70, 0.85}, kPlay,  COUNT(kPlay) },
};
static const tUaSection kAmpPage[]    = {
    {"OUTPUT",        {0.75, 0.75, 0.75}, kOutput,   COUNT(kOutput)  },
    {"AMP ENV RATE",  {0.55, 0.80, 0.40}, kEnvRate,  COUNT(kEnvRate) },
    {"AMP ENV LEVEL", {0.55, 0.80, 0.40}, kEnvLevel, COUNT(kEnvLevel)},
};
static const tUaSection kGlobalPage[] = {
    {"GLOBAL", {0.85, 0.55, 0.55}, kGlobal, COUNT(kGlobal)},
};

const tUaPage           gUaPages[]    = {
    {"OSC",    kOscPage,    COUNT(kOscPage)   },
    {"AMP",    kAmpPage,    COUNT(kAmpPage)   },
    {"GLOBAL", kGlobalPage, COUNT(kGlobalPage)},
};
const uint32_t          gUaPageCount  = COUNT(gUaPages);

