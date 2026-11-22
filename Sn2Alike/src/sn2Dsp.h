/*
 * Sn2Alike - the instrument's 24-bit arithmetic: registers, flags and memory as its voice processor keeps them.
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
// Notes: Docs/code-notes/sn2Filter.md - "// notes §k" refers there.


// Notes: Docs/code-notes/sn2Fixed.md - "// notes §k" refers there.

#ifndef __SN2_DSP_H__
#define __SN2_DSP_H__

#include <stdbool.h>
#include <stdint.h>

#define SN2_DSP_WORDS    (0x1000)  // notes §2: the memory a voice uses, X and Y

// notes §1: accumulators of 56 bits and words of 24, both kept sign-extended; flags as the arithmetic leaves them
typedef struct {
    int64_t    a, b;
    int32_t    x0, x1, y0, y1;
    uint32_t   r[8], n[8], m[8];
    uint32_t   ss[16];
    int        sp;
    uint32_t   lc;     // notes §2: the voice loop's count, which voice this is
    bool       c, v, z, neg, u, e;
    uint32_t * xm, * ym;
} tSn2Dsp;

#define SN2_M56    (((int64_t)1 << 56) - 1)

static inline int64_t sn2_ext56(int64_t v) {
    return (int64_t)((uint64_t)v << 8) >> 8;
}

static inline int32_t sn2_s24(uint32_t w) {
    return (int32_t)(w << 8) >> 8;
}

static inline uint32_t sn2_w24(int32_t v) {
    return (uint32_t)v & 0xFFFFFFu;
}

static inline int64_t sn2_ld(int32_t w) {   // a word into an accumulator: the low part cleared, sign above
    return (int64_t)w * 16777216;
}

static inline int32_t sn2_hi(int64_t acc) {   // the middle word as it stands
    return sn2_s24((uint32_t)(acc >> 24));
}

static inline int32_t sn2_lim(int64_t acc) {   // notes §1: a move out of an accumulator saturates past +-1
    return (acc >= ((int64_t)1 << 47)) ? 0x7FFFFF : ((acc < -((int64_t)1 << 47)) ? -0x800000 : sn2_hi(acc));
}

static inline void sn2_flags(tSn2Dsp * d, int64_t acc) {
    int64_t top = acc >> 47;

    d->z   = (acc == 0);
    d->neg = (acc < 0);
    d->e   = (top != 0) && (top != -1);
    d->u   = (((acc >> 47) ^ (acc >> 46)) & 1) == 0;
}

static inline int64_t sn2_add(tSn2Dsp * d, int64_t acc, int64_t val, bool flags) {
    uint64_t ua = (uint64_t)acc & SN2_M56, uv = (uint64_t)val & SN2_M56;
    int64_t  res = sn2_ext56((int64_t)(ua + uv));

    if (flags) {
        d->c = ((ua + uv) >> 56) != 0;
        d->v = (((res ^ acc) & (res ^ val)) >> 55) & 1;
        sn2_flags(d, res);
    }
    return res;
}

static inline int64_t sn2_sub(tSn2Dsp * d, int64_t acc, int64_t val, bool flags) {
    uint64_t ua = (uint64_t)acc & SN2_M56, uv = (uint64_t)val & SN2_M56;
    int64_t  res = sn2_ext56((int64_t)(ua - uv));

    if (flags) {
        d->c = ua < uv;
        d->v = (((acc ^ val) & (res ^ acc)) >> 55) & 1;
        sn2_flags(d, res);
    }
    return res;
}

static inline int64_t sn2_mac(tSn2Dsp * d, int64_t acc, int32_t s1, int32_t s2, bool negate, bool flags) {
    int64_t p   = (int64_t)s1 * s2 * 2;
    int64_t sum = acc + (negate ? -p : p);
    int64_t res = sn2_ext56(sum);

    if (flags) {
        d->v = (res != sum);
        sn2_flags(d, res);
    }
    return res;
}

static inline int64_t sn2_asr(tSn2Dsp * d, int64_t acc, int n, bool flags) {
    int64_t res = acc >> n;

    if (flags) {
        d->c = (n > 0) && ((acc >> (n - 1)) & 1);
        d->v = false;
        sn2_flags(d, res);
    }
    return res;
}

static inline int64_t sn2_asl(tSn2Dsp * d, int64_t acc, int n, bool flags) {
    int64_t res = sn2_ext56((int64_t)((uint64_t)acc << n));

    if (flags) {
        int64_t out = acc >> (55 - n);   // the bits shifted past the sign, and the sign

        d->c = (n > 0) && ((acc >> (56 - n)) & 1);
        d->v = (out != 0) && (out != -1);
        sn2_flags(d, res);
    }
    return res;
}

static inline int64_t sn2_set_hi(int64_t acc, uint32_t w) {   // the middle word replaced, the rest kept
    return (acc & ~((int64_t)0xFFFFFF << 24)) | ((int64_t)(w & 0xFFFFFFu) << 24);
}

static inline void sn2_flags_hi(tSn2Dsp * d, uint32_t w) {   // logical operations: the middle word decides
    d->neg = (w >> 23) & 1;
    d->z   = (w & 0xFFFFFFu) == 0;
    d->v   = false;
}

static inline int64_t sn2_clb(tSn2Dsp * d, int64_t s) {
    int     count = 0;

    if (s != 0) {
        uint64_t val = (uint64_t)((s < 0) ? ~(s << 8) : (s << 8)) | 0xFF;
        int      bsr = 63;

        while ((bsr > 0) && !((val >> bsr) & 1)) {
            bsr--;
        }
        count = bsr - 54;
    }
    int64_t res   = sn2_ld(count);

    d->neg = (res >> 47) & 1;
    d->z   = (count == 0);
    d->v   = false;
    return res;
}

static inline int64_t sn2_normf(tSn2Dsp * d, int64_t acc, int32_t s) {   // keeps C
    bool    c   = d->c;
    int64_t res = (s >= 0) ? sn2_asr(d, acc, s, true) : sn2_asl(d, acc, -s, true);

    d->c = c;
    return res;
}

// notes §2: address arithmetic - linear, or wrapping within a block of a power of two
static inline uint32_t sn2_step(const tSn2Dsp * d, int reg, int32_t delta) {
    uint32_t r = d->r[reg], m = d->m[reg];

    if ((m & 0xFFFF) == 0xFFFF) {
        return (r + (uint32_t)delta) & 0xFFFFFFu;
    }

    if (m & 0x8000) {   // multiple wrap: the low bits move, the block stays
        uint32_t size = (m & 0x7FFF) + 1;

        return (r & ~(size - 1)) | ((r + (uint32_t)delta) & (size - 1));
    }
    uint32_t size = m + 1, base = r - (r % size);

    return base + (uint32_t)(((int64_t)(r - base) + delta + size) % size);
}

#endif // __SN2_DSP_H__
