/*
 * TiAlike - fixed-point arithmetic, as the chip does it.
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
// Notes: Docs/code-notes/tiFixed.md - "// notes §k" refers there.

#ifndef __TI_FIXED_H__
#define __TI_FIXED_H__

#include <math.h>
#include <stdint.h>

#define TI_Q23           (8388608.0)
#define TI_WORD_MAX      (8388607)
#define TI_WORD_MIN      (-8388608)

// notes §1: a voice's level on the instrument's full scale
#define TI_FIXED_UNIT    (0.1767767)        // 1 / (4 x root 2)

static inline int32_t ti_word_limit(int64_t x) {
    return (x > TI_WORD_MAX) ? TI_WORD_MAX : ((x < TI_WORD_MIN) ? TI_WORD_MIN : (int32_t)x);
}

static inline int32_t ti_word(double x) {
    return ti_word_limit((int64_t)llround(x * TI_Q23));
}

// notes §2: multiply two words into the accumulator, as a fraction
static inline int64_t ti_mac(int32_t a, int32_t b) {
    return ((int64_t)a * (int64_t)b) * 2;
}

// notes §3: convergent rounding to a word, limited on the move out
static inline int32_t ti_round(int64_t acc) {
    int64_t word = acc >> 24;
    int64_t rest = acc & 0xFFFFFF;

    if ((rest > 0x800000) || ((rest == 0x800000) && ((word & 1) != 0))) {
        word++;
    }
    return ti_word_limit(word);
}

#endif // __TI_FIXED_H__
