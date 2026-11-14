#ifndef __NL3_ENVELOPE_H__
#define __NL3_ENVELOPE_H__

#include <stdbool.h>

typedef enum {
    eNl3EnvIdle = 0,
    eNl3EnvAttack,
    eNl3EnvDecay,     // notes §1: towards Sustain, for as long as the key is held
    eNl3EnvRelease
} tNl3EnvStage;

// notes §1: each stage a step towards its target by a fraction of the distance, per sample at one rate
typedef struct {
    double attackK, decayK, releaseK;
    double sustain;        // notes §3: the target, in the envelope's own domain
    bool   cubed;          // notes §4: Exp Attack
    bool   repeat;         // notes §6: the decay aims below zero and starts the attack again on reaching it
} tNl3EnvShape;

typedef struct {
    tNl3EnvStage stage;
    double       state;    // notes §4: the envelope's own value; the cube root of the level with Exp Attack
    double       level;    // 0..1, what it puts out
} tNl3Env;

// The panel's 0..127 for Attack, Decay, Sustain and Release; squared: the amplifier's Sustain (notes §3); Exp Attack
void nl3_env_shape(double attack, double decay, double sustain, double release, bool squared, bool exponential, double sampleRate, tNl3EnvShape * shape);
void nl3_env_gate(tNl3Env * env, bool on);                      // on: attack from where it is; off: release
double nl3_env_next(tNl3Env * env, const tNl3EnvShape * shape); // one sample on

static inline bool nl3_env_idle(const tNl3Env * env) {
    return env->stage == eNl3EnvIdle;
}

#endif // __NL3_ENVELOPE_H__
