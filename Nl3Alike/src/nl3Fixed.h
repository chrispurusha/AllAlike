/*
 * Nl3Alike - fixed-point arithmetic, as the chip does it.
 *
 * Copyright (C) 2026 Chris Turner <chris_purusha@icloud.com>
 *
 * This program is free software: you can redistribute it and/or modify it under the terms of the
 * GNU General Public License as published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without
 * even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with this program. If
 * not, see <https://www.gnu.org/licenses/>.
 */
// Notes: Docs/code-notes/nl3Fixed.md - "// notes §k" refers there.

#ifndef __NL3_FIXED_H__
#define __NL3_FIXED_H__

#include <math.h>
#include <stdint.h>

#define NL3_Q23         (8388608.0)
#define NL3_WORD_MAX    (8388607)
#define NL3_WORD_MIN    (-8388608)
#define NL3_ACC_ONE     (INT64_C(1) << 47)

typedef int32_t   tNl3Word; // notes §1: 24 bits, 1.0 = 2^23
typedef int64_t   tNl3Acc;  // notes §1: 56 bits, 1.0 = 2^47

static inline tNl3Word nl3_word_limit(int64_t x) {
    return (x > NL3_WORD_MAX) ? NL3_WORD_MAX : ((x < NL3_WORD_MIN) ? NL3_WORD_MIN : (tNl3Word)x);
}

static inline tNl3Word nl3_word(double x) {
    return nl3_word_limit((int64_t)llround(x * NL3_Q23));
}

static inline double nl3_word_value(tNl3Word w) {
    return (double)w / NL3_Q23;
}

// notes §2: a word into an accumulator
static inline tNl3Acc nl3_acc(tNl3Word w) {
    return (tNl3Acc)w * (INT64_C(1) << 24);
}

// notes §2: the accumulator keeps 56 bits; a left shift drops what passes the top
static inline tNl3Acc nl3_acc_wrap(tNl3Acc a) {
    return (tNl3Acc)((uint64_t)a << 8) >> 8;
}

static inline tNl3Acc nl3_asl(tNl3Acc a, int n) {
    return nl3_acc_wrap((tNl3Acc)((uint64_t)a << n));
}

// notes §3: two words multiply as fractions, so the product is doubled
static inline tNl3Acc nl3_mul(tNl3Word a, tNl3Word b) {
    return ((tNl3Acc)a * (tNl3Acc)b) * 2;
}

// notes §4: moving an accumulator out limits it, then keeps the top word
static inline tNl3Word nl3_move(tNl3Acc a) {
    if (a >= NL3_ACC_ONE) {
        return NL3_WORD_MAX;
    }

    if (a < -NL3_ACC_ONE) {
        return NL3_WORD_MIN;
    }
    return (tNl3Word)(a >> 24);
}

// notes §5: convergent rounding to a word, for mpyr; the low word is cleared
static inline tNl3Acc nl3_round(tNl3Acc acc) {
    int64_t word = acc >> 24;
    int64_t rest = acc & 0xFFFFFF;

    if ((rest > 0x800000) || ((rest == 0x800000) && ((word & 1) != 0))) {
        word++;
    }
    return (tNl3Acc)word * (INT64_C(1) << 24);
}

#endif // __NL3_FIXED_H__
