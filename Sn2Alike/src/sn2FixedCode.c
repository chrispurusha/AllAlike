/*
 * Sn2Alike - the voice's per-sample arithmetic in the instrument's 24-bit words (generated).
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

// Notes: Docs/code-notes/sn2Fixed.md - "// notes §k" refers there. Generated: do not edit by hand.

#include "sn2Dsp.h"
#include "sn2FixedCode.h"

#ifndef SN2_XR
#define SN2_XR(d, i)       ((d)->xm[(i) & (SN2_DSP_WORDS - 1)])
#define SN2_YR(d, i)       ((d)->ym[(i) & (SN2_DSP_WORDS - 1)])
#define SN2_XW(d, i, v)    ((d)->xm[(i) & (SN2_DSP_WORDS - 1)] = (v))
#define SN2_YW(d, i, v)    ((d)->ym[(i) & (SN2_DSP_WORDS - 1)] = (v))
#endif

// notes §3: once a sample, ahead of the voices - the buses mixed out and cleared
void sn2_fixed_head(tSn2Dsp * d) {
    {
        d->r[4] = sn2_w24(sn2_s24(0xc02));
    }
    {
        d->r[1] = sn2_w24(sn2_s24(0x8));
    }
    {
        d->r[0] = sn2_w24(sn2_s24(0x1d));
    }
    {
        d->r[5] = sn2_w24(sn2_s24(0x43));
    }
    {
        int32_t v1 = sn2_s24(SN2_YR(d, d->r[4]));
        d->a    = sn2_ld(v1);
        d->r[4] = sn2_step(d, 4, 1);
    }
    {
        int32_t v2 = sn2_s24(SN2_YR(d, d->r[4]));
        d->b    = sn2_ld(v2);
        d->r[4] = sn2_step(d, 4, 1);
    }
    {
        int32_t v3 = sn2_lim(d->a);
        int32_t v4 = sn2_s24(SN2_YR(d, d->r[4]));
        SN2_XW(d, d->r[1], sn2_w24(v3));
        d->a    = sn2_ld(v4);
        d->r[1] = sn2_step(d, 1, 1);
        d->r[4] = sn2_step(d, 4, 1);
    }
    {
        int32_t v5 = sn2_lim(d->b);
        int32_t v6 = sn2_s24(SN2_YR(d, d->r[4]));
        SN2_XW(d, d->r[1], sn2_w24(v5));
        d->b    = sn2_ld(v6);
        d->r[1] = sn2_step(d, 1, 1);
        d->r[4] = sn2_step(d, 4, 1);
    }
    {
        int32_t v7 = sn2_lim(d->a);
        int32_t v8 = sn2_s24(SN2_YR(d, d->r[4]));
        SN2_XW(d, d->r[1], sn2_w24(v7));
        d->a    = sn2_ld(v8);
        d->r[1] = sn2_step(d, 1, 1);
        d->r[4] = sn2_step(d, 4, 1);
    }
    {
        int32_t v9 = sn2_lim(d->b);
        SN2_XW(d, d->r[1], sn2_w24(v9));
        d->r[1] = sn2_step(d, 1, 1);
    }
    {
        int32_t v10 = sn2_lim(d->a);
        SN2_XW(d, d->r[1], sn2_w24(v10));
        d->r[1] = sn2_step(d, 1, 1);
    }
    {
        d->n[5] = sn2_w24(sn2_s24(0xfffff1));
    }
    {
        d->r[4] = sn2_w24(sn2_s24(0x51));
    }
    {
        int32_t v11 = sn2_s24(SN2_XR(d, d->r[1]));
        int32_t v12 = sn2_s24(SN2_YR(d, d->r[5]));
        d->b    = 0;
        d->z    = true;
        d->neg  = false;
        d->e    = false;
        d->u    = true;
        d->v    = false;
        d->x0   = v11;
        d->y0   = v12;
        d->r[1] = sn2_step(d, 1, 1);
        d->r[5] = sn2_step(d, 5, 1);
    }
    {
        int32_t v13 = sn2_s24(SN2_XR(d, d->r[0]));
        d->a = sn2_ld(v13);
    }
    {
        int32_t v14 = sn2_lim(d->b);
        d->a    = sn2_mac(d, d->a, d->y0, d->x0, false, true);
        SN2_XW(d, d->r[0], sn2_w24(v14));
        d->r[0] = sn2_step(d, 0, 1);
    }
    {
        int32_t v15 = sn2_s24(SN2_XR(d, d->r[1]));
        int32_t v16 = sn2_s24(SN2_YR(d, d->r[5]));
        d->x0   = v15;
        d->y0   = v16;
        d->r[1] = sn2_step(d, 1, 1);
        d->r[5] = sn2_step(d, 5, 1);
    }
    {
        int32_t v17 = sn2_s24(SN2_XR(d, d->r[0]));
        int32_t v18 = sn2_lim(d->a);
        d->a    = sn2_ld(v17);
        SN2_YW(d, d->r[4], sn2_w24(v18));
        d->r[4] = sn2_step(d, 4, 1);
    }
    {
        int32_t v19 = sn2_lim(d->b);
        d->a    = sn2_mac(d, d->a, d->y0, d->x0, false, true);
        SN2_XW(d, d->r[0], sn2_w24(v19));
        d->r[0] = sn2_step(d, 0, 1);
    }
    {
        int32_t v20 = sn2_s24(SN2_XR(d, d->r[1]));
        int32_t v21 = sn2_s24(SN2_YR(d, d->r[5]));
        d->x0   = v20;
        d->y0   = v21;
        d->r[1] = sn2_step(d, 1, 1);
        d->r[5] = sn2_step(d, 5, 1);
    }
    {
        int32_t v22 = sn2_s24(SN2_XR(d, d->r[0]));
        int32_t v23 = sn2_lim(d->a);
        d->a    = sn2_ld(v22);
        SN2_YW(d, d->r[4], sn2_w24(v23));
        d->r[4] = sn2_step(d, 4, 1);
    }
    {
        int32_t v24 = sn2_lim(d->b);
        d->a    = sn2_mac(d, d->a, d->y0, d->x0, false, true);
        SN2_XW(d, d->r[0], sn2_w24(v24));
        d->r[0] = sn2_step(d, 0, 1);
    }
    {
        int32_t v25 = sn2_s24(SN2_XR(d, d->r[1]));
        int32_t v26 = sn2_s24(SN2_YR(d, d->r[5]));
        d->x0   = v25;
        d->y0   = v26;
        d->r[1] = sn2_step(d, 1, 1);
        d->r[5] = sn2_step(d, 5, 1);
    }
    {
        int32_t v27 = sn2_s24(SN2_XR(d, d->r[0]));
        int32_t v28 = sn2_lim(d->a);
        d->a    = sn2_ld(v27);
        SN2_YW(d, d->r[4], sn2_w24(v28));
        d->r[4] = sn2_step(d, 4, 1);
    }
    {
        int32_t v29 = sn2_lim(d->b);
        d->a    = sn2_mac(d, d->a, d->y0, d->x0, false, true);
        SN2_XW(d, d->r[0], sn2_w24(v29));
        d->r[0] = sn2_step(d, 0, 1);
    }
    {
        int32_t v30 = sn2_s24(SN2_XR(d, d->r[1]));
        int32_t v31 = sn2_s24(SN2_YR(d, d->r[5]));
        d->x0   = v30;
        d->y0   = v31;
        d->r[1] = sn2_step(d, 1, 1);
        d->r[5] = sn2_step(d, 5, 1);
    }
    {
        int32_t v32 = sn2_s24(SN2_XR(d, d->r[0]));
        int32_t v33 = sn2_lim(d->a);
        d->a    = sn2_ld(v32);
        SN2_YW(d, d->r[4], sn2_w24(v33));
        d->r[4] = sn2_step(d, 4, 1);
    }
    {
        int32_t v34 = sn2_lim(d->b);
        d->a    = sn2_mac(d, d->a, d->y0, d->x0, false, true);
        SN2_XW(d, d->r[0], sn2_w24(v34));
        d->r[0] = sn2_step(d, 0, 1);
    }
    {
        int32_t v35 = sn2_s24(SN2_XR(d, d->r[1]));
        int32_t v36 = sn2_s24(SN2_YR(d, d->r[5]));
        d->x0   = v35;
        d->y0   = v36;
        d->r[1] = sn2_step(d, 1, 1);
        d->r[5] = sn2_step(d, 5, 1);
    }
    {
        int32_t v37 = sn2_s24(SN2_XR(d, d->r[0]));
        int32_t v38 = sn2_lim(d->a);
        d->a    = sn2_ld(v37);
        SN2_YW(d, d->r[4], sn2_w24(v38));
        d->r[4] = sn2_step(d, 4, 1);
    }
    {
        int32_t v39 = sn2_lim(d->b);
        d->a    = sn2_mac(d, d->a, d->y0, d->x0, false, true);
        SN2_XW(d, d->r[0], sn2_w24(v39));
        d->r[0] = sn2_step(d, 0, 1);
    }
    {
        int32_t v40 = sn2_s24(SN2_XR(d, d->r[1]));
        int32_t v41 = sn2_s24(SN2_YR(d, d->r[5]));
        d->x0   = v40;
        d->y0   = v41;
        d->r[1] = sn2_step(d, 1, 1);
        d->r[5] = sn2_step(d, 5, 1);
    }
    {
        int32_t v42 = sn2_s24(SN2_XR(d, d->r[0]));
        int32_t v43 = sn2_lim(d->a);
        d->a    = sn2_ld(v42);
        SN2_YW(d, d->r[4], sn2_w24(v43));
        d->r[4] = sn2_step(d, 4, 1);
    }
    {
        int32_t v44 = sn2_lim(d->b);
        d->a    = sn2_mac(d, d->a, d->y0, d->x0, false, true);
        SN2_XW(d, d->r[0], sn2_w24(v44));
        d->r[0] = sn2_step(d, 0, 1);
    }
    {
        int32_t v45 = sn2_s24(SN2_XR(d, d->r[1]));
        int32_t v46 = sn2_s24(SN2_YR(d, d->r[5]));
        d->x0   = v45;
        d->y0   = v46;
        d->r[1] = sn2_step(d, 1, 1);
        d->r[5] = sn2_step(d, 5, 1);
    }
    {
        int32_t v47 = sn2_s24(SN2_XR(d, d->r[0]));
        int32_t v48 = sn2_lim(d->a);
        d->a    = sn2_ld(v47);
        SN2_YW(d, d->r[4], sn2_w24(v48));
        d->r[4] = sn2_step(d, 4, 1);
    }
    {
        int32_t v49 = sn2_lim(d->b);
        d->a    = sn2_mac(d, d->a, d->y0, d->x0, false, true);
        SN2_XW(d, d->r[0], sn2_w24(v49));
        d->r[0] = sn2_step(d, 0, 1);
    }
    {
        int32_t v50 = sn2_s24(SN2_XR(d, d->r[1]));
        int32_t v51 = sn2_s24(SN2_YR(d, d->r[5]));
        d->x0   = v50;
        d->y0   = v51;
        d->r[1] = sn2_step(d, 1, 1);
        d->r[5] = sn2_step(d, 5, 1);
    }
    {
        int32_t v52 = sn2_s24(SN2_XR(d, d->r[0]));
        int32_t v53 = sn2_lim(d->a);
        d->a    = sn2_ld(v52);
        SN2_YW(d, d->r[4], sn2_w24(v53));
        d->r[4] = sn2_step(d, 4, 1);
    }
    {
        int32_t v54 = sn2_lim(d->b);
        d->a    = sn2_mac(d, d->a, d->y0, d->x0, false, true);
        SN2_XW(d, d->r[0], sn2_w24(v54));
        d->r[0] = sn2_step(d, 0, 1);
    }
    {
        int32_t v55 = sn2_s24(SN2_XR(d, d->r[1]));
        int32_t v56 = sn2_s24(SN2_YR(d, d->r[5]));
        d->x0   = v55;
        d->y0   = v56;
        d->r[1] = sn2_step(d, 1, 1);
        d->r[5] = sn2_step(d, 5, 1);
    }
    {
        int32_t v57 = sn2_s24(SN2_XR(d, d->r[0]));
        int32_t v58 = sn2_lim(d->a);
        d->a    = sn2_ld(v57);
        SN2_YW(d, d->r[4], sn2_w24(v58));
        d->r[4] = sn2_step(d, 4, 1);
    }
    {
        int32_t v59 = sn2_lim(d->b);
        d->a    = sn2_mac(d, d->a, d->y0, d->x0, false, true);
        SN2_XW(d, d->r[0], sn2_w24(v59));
        d->r[0] = sn2_step(d, 0, 1);
    }
    {
        int32_t v60 = sn2_s24(SN2_XR(d, d->r[1]));
        int32_t v61 = sn2_s24(SN2_YR(d, d->r[5]));
        d->x0   = v60;
        d->y0   = v61;
        d->r[1] = sn2_step(d, 1, 1);
        d->r[5] = sn2_step(d, 5, 1);
    }
    {
        int32_t v62 = sn2_s24(SN2_XR(d, d->r[0]));
        int32_t v63 = sn2_lim(d->a);
        d->a    = sn2_ld(v62);
        SN2_YW(d, d->r[4], sn2_w24(v63));
        d->r[4] = sn2_step(d, 4, 1);
    }
    {
        int32_t v64 = sn2_lim(d->b);
        d->a    = sn2_mac(d, d->a, d->y0, d->x0, false, true);
        SN2_XW(d, d->r[0], sn2_w24(v64));
        d->r[0] = sn2_step(d, 0, 1);
    }
    {
        int32_t v65 = sn2_s24(SN2_XR(d, d->r[1]));
        int32_t v66 = sn2_s24(SN2_YR(d, d->r[5]));
        d->x0   = v65;
        d->y0   = v66;
        d->r[1] = sn2_step(d, 1, 1);
        d->r[5] = sn2_step(d, 5, 1);
    }
    {
        int32_t v67 = sn2_s24(SN2_XR(d, d->r[0]));
        int32_t v68 = sn2_lim(d->a);
        d->a    = sn2_ld(v67);
        SN2_YW(d, d->r[4], sn2_w24(v68));
        d->r[4] = sn2_step(d, 4, 1);
    }
    {
        int32_t v69 = sn2_lim(d->b);
        d->a    = sn2_mac(d, d->a, d->y0, d->x0, false, true);
        SN2_XW(d, d->r[0], sn2_w24(v69));
        d->r[0] = sn2_step(d, 0, 1);
    }
    {
        int32_t v70 = sn2_s24(SN2_XR(d, d->r[1]));
        int32_t v71 = sn2_s24(SN2_YR(d, d->r[5]));
        d->x0   = v70;
        d->y0   = v71;
        d->r[1] = sn2_step(d, 1, 1);
        d->r[5] = sn2_step(d, 5, 1);
    }
    {
        int32_t v72 = sn2_s24(SN2_XR(d, d->r[0]));
        int32_t v73 = sn2_lim(d->a);
        d->a    = sn2_ld(v72);
        SN2_YW(d, d->r[4], sn2_w24(v73));
        d->r[4] = sn2_step(d, 4, 1);
    }
    {
        int32_t v74 = sn2_lim(d->b);
        d->a    = sn2_mac(d, d->a, d->y0, d->x0, false, true);
        SN2_XW(d, d->r[0], sn2_w24(v74));
        d->r[0] = sn2_step(d, 0, 1);
    }
    {
        int32_t v75 = sn2_s24(SN2_XR(d, d->r[1]));
        int32_t v76 = sn2_s24(SN2_YR(d, d->r[5]));
        d->x0   = v75;
        d->y0   = v76;
        d->r[1] = sn2_step(d, 1, 1);
        d->r[5] = sn2_step(d, 5, (int32_t)d->n[5]);
    }
    {
        int32_t v77 = sn2_s24(SN2_XR(d, d->r[0]));
        int32_t v78 = sn2_lim(d->a);
        d->a    = sn2_ld(v77);
        SN2_YW(d, d->r[4], sn2_w24(v78));
        d->r[4] = sn2_step(d, 4, 1);
    }
    {
        int32_t v79 = sn2_lim(d->b);
        d->a    = sn2_mac(d, d->a, d->y0, d->x0, false, true);
        SN2_XW(d, d->r[0], sn2_w24(v79));
        d->r[0] = sn2_step(d, 0, 1);
    }
    {
        int32_t v80 = sn2_s24(SN2_XR(d, d->r[1]));
        int32_t v81 = sn2_s24(SN2_YR(d, d->r[5]));
        d->x0   = v80;
        d->y0   = v81;
        d->r[1] = sn2_step(d, 1, 1);
        d->r[5] = sn2_step(d, 5, 1);
    }
    {
        int32_t v82 = sn2_s24(SN2_XR(d, d->r[0]));
        int32_t v83 = sn2_lim(d->a);
        d->a    = sn2_ld(v82);
        SN2_YW(d, d->r[4], sn2_w24(v83));
        d->r[4] = sn2_step(d, 4, 1);
    }
    {
        int32_t v84 = sn2_lim(d->b);
        d->a    = sn2_mac(d, d->a, d->y0, d->x0, false, true);
        SN2_XW(d, d->r[0], sn2_w24(v84));
        d->r[0] = sn2_step(d, 0, 1);
    }
    {
        int32_t v85 = sn2_s24(SN2_XR(d, d->r[1]));
        int32_t v86 = sn2_s24(SN2_YR(d, d->r[5]));
        d->x0   = v85;
        d->y0   = v86;
        d->r[1] = sn2_step(d, 1, 1);
        d->r[5] = sn2_step(d, 5, 1);
    }
    {
        int32_t v87 = sn2_s24(SN2_XR(d, d->r[0]));
        int32_t v88 = sn2_lim(d->a);
        d->a    = sn2_ld(v87);
        SN2_YW(d, d->r[4], sn2_w24(v88));
        d->r[4] = sn2_step(d, 4, 1);
    }
    {
        int32_t v89 = sn2_lim(d->b);
        d->a    = sn2_mac(d, d->a, d->y0, d->x0, false, true);
        SN2_XW(d, d->r[0], sn2_w24(v89));
        d->r[0] = sn2_step(d, 0, 1);
    }
    {
        d->n[5] = sn2_w24(sn2_s24(0x5));
    }
    {
        int32_t v90 = sn2_s24(SN2_XR(d, d->r[0]));
        int32_t v91 = sn2_lim(d->a);
        d->x1   = v90;
        SN2_YW(d, d->r[4], sn2_w24(v91));
        d->r[0] = sn2_step(d, 0, 1);
        d->r[4] = sn2_step(d, 4, 1);
    }
    {
        int32_t v92 = sn2_s24(SN2_YR(d, 0x0));
        d->y1 = v92;
    }
    {
        int32_t v93 = sn2_s24(SN2_XR(d, d->r[0]));
        int32_t v94 = sn2_s24(SN2_YR(d, d->r[4]));
        d->a   = 0;
        d->z   = true;
        d->neg = false;
        d->e   = false;
        d->u   = true;
        d->v   = false;
        d->b   = sn2_ld(v93);
        d->y0  = v94;
    }
    {
        int32_t v95 = d->y0;
        d->a = (d->a & ~(int64_t)0xFFFFFF) | (int64_t)sn2_w24(v95);
    }
    {
        d->a = sn2_add(d, d->a, 1, true);
    }
    {
        int32_t v96 = sn2_lim(d->b);
        d->x0 = v96;
    }
    {
        int32_t v97 = sn2_s24((uint32_t)(d->a & 0xFFFFFF));
        d->a    = sn2_mac(d, d->a, d->y0, d->x0, false, true);
        SN2_YW(d, d->r[4], sn2_w24(v97));
        d->r[4] = sn2_step(d, 4, 1);
    }
    {
        int32_t v98 = sn2_s24(d->r[0]);
        d->b    = sn2_add(d, d->b, sn2_ld(d->y1), true);
        d->r[2] = sn2_w24(v98);
    }
    {
        int32_t v99 = sn2_s24((uint32_t)(d->a & 0xFFFFFF));
        {
            uint32_t w = (uint32_t)(d->b >> 24) & 0xFFFFFFu;
            uint32_t o = (w >> 1) & 0xFFFFFFu;
            d->b = sn2_set_hi(d->b, o);
            d->c = (w >> 0) & 1;
            sn2_flags_hi(d, o);
        }
        SN2_XW(d, d->r[0], sn2_w24(v99));
        d->r[0] = sn2_step(d, 0, 1);
    }
    {
        int32_t v100 = sn2_s24(SN2_XR(d, 0x2));
        d->y0 = v100;
    }
    {
        int32_t v101 = sn2_lim(d->b);
        SN2_XW(d, d->r[0], sn2_w24(v101));
        d->r[0] = sn2_step(d, 0, 1);
    }
    {
        int32_t v102 = sn2_s24(SN2_YR(d, 0x1b));
        d->b = sn2_ld(v102);
    }
    {
        int32_t v103 = sn2_s24(SN2_YR(d, 0x8));
        d->x0 = v103;
    }
    {
        int32_t v104 = sn2_s24(SN2_XR(d, d->r[0]));
        int32_t v105 = sn2_s24(SN2_YR(d, d->r[4]));
        d->b    = sn2_add(d, d->b, sn2_ld(d->x0), true);
        d->x1   = v104;
        d->a    = sn2_ld(v105);
        d->r[0] = sn2_step(d, 0, 1);
    }
    {
        {
            uint32_t o = ((uint32_t)(d->b >> 24) & (uint32_t)0x3) & 0xFFFFFFu;
            d->b = sn2_set_hi(d->b, o);
            sn2_flags_hi(d, o);
        }
    }
    {
        if (!d->z) {
            d->a = sn2_sub(d, d->a, sn2_ld(d->x1), false);
        }
    }
    {
        int32_t v106 = sn2_lim(d->b);
        d->b   = 0;
        d->z   = true;
        d->neg = false;
        d->e   = false;
        d->u   = true;
        d->v   = false;
        SN2_YW(d, 0x1b, sn2_w24(v106));
    }
    {
        d->a = sn2_ld(sn2_hi(d->a));
    }
    {
        int32_t v107 = sn2_s24(SN2_YR(d, 0x9));
        d->a  = sn2_add(d, d->a, sn2_ld(d->x1), true);
        d->x1 = v107;
    }
    {
        if (d->c) {
            d->b = sn2_ld(d->x0);
        }
    }
    {
        d->a = sn2_ld(sn2_hi(d->a));
    }
    {
        if (d->e) {
            d->b = sn2_ld(d->x1);
        }
    }
    {
        int32_t v108 = sn2_lim(d->a);
        d->a    = sn2_ext56((d->a < 0) ? -d->a : d->a);
        sn2_flags(d, d->a);
        d->v    = false;
        SN2_YW(d, d->r[4], sn2_w24(v108));
        d->r[4] = sn2_step(d, 4, 1);
    }
    {
        int32_t v109 = sn2_hi(d->b);
        SN2_YW(d, d->r[4], sn2_w24(v109));
        d->r[4] = sn2_step(d, 4, 1);
    }
    {
        int32_t v110 = sn2_lim(d->a);
        {
            uint32_t o = ((uint32_t)(d->a >> 24) ^ sn2_w24(d->y1)) & 0xFFFFFFu;
            d->a = sn2_set_hi(d->a, o);
            sn2_flags_hi(d, o);
        }
        SN2_XW(d, d->r[0], sn2_w24(v110));
        d->r[0] = sn2_step(d, 0, 1);
    }
    {
        if (d->e) {
            d->a = 0;
        }
    }
    {
        d->r[3] = sn2_w24(sn2_s24(0x31));
    }
    {
        d->n[4] = sn2_w24(sn2_s24(0x8));
    }
    {
        int32_t v111 = sn2_s24(SN2_YR(d, 0x12));
        d->m[6] = sn2_w24(v111);
    }
    {
        int32_t v112 = sn2_lim(d->a);
        SN2_XW(d, d->r[0], sn2_w24(v112));
        d->r[0] = sn2_step(d, 0, 1);
    }
}

// notes §3: one voice, once a sample - its slow words on one sample in four, then its sound
void sn2_fixed_pass(tSn2Dsp * d) {
    {
        uint32_t   wi = d->r[0];
        uint32_t   wv = SN2_XR(d, wi);
        uint32_t * w  = &wv;

        if (!((*w >> 0) & 1)) {
            goto L1_pass;
        }
    }
    {
        uint32_t   wi = d->r[0];
        uint32_t   wv = SN2_XR(d, wi);
        uint32_t * w  = &wv;

        if (!((*w >> 7) & 1)) {
            goto L2_pass;
        }
    }
    {
        d->x0 = sn2_s24(0x0);
    }
    d->r[1] = sn2_step(d, 0, 0x1f);
    {
        d->n[0] = sn2_w24(sn2_s24(0x43));
    }

    for (int lc631 = 0; lc631 < 6; lc631++) {
        {
            int32_t v1 = d->x0;
            SN2_XW(d, d->r[1], sn2_w24(v1));
            d->r[1] = sn2_step(d, 1, 1);
        }
    }

    d->r[1] = sn2_step(d, 0, (int32_t)d->n[0]);
    {
        d->n[0] = sn2_w24(sn2_s24(0x67));
    }

    for (int lc636 = 0; lc636 < 6; lc636++) {
        {
            int32_t v2 = d->x0;
            SN2_XW(d, d->r[1], sn2_w24(v2));
            d->r[1] = sn2_step(d, 1, 1);
        }
    }

    d->r[1] = sn2_step(d, 0, (int32_t)d->n[0]);
    {
        d->n[0] = sn2_w24(sn2_s24(0x74));
    }

    for (int lc641 = 0; lc641 < 6; lc641++) {
        {
            int32_t v3 = d->x0;
            SN2_XW(d, d->r[1], sn2_w24(v3));
            d->r[1] = sn2_step(d, 1, 1);
        }
    }

    d->r[1] = sn2_step(d, 0, (int32_t)d->n[0]);
    {
        d->n[0] = sn2_w24(sn2_s24(0x86));
    }

    for (int lc646 = 0; lc646 < 18; lc646++) {
        {
            int32_t v4 = d->x0;
            SN2_XW(d, d->r[1], sn2_w24(v4));
            d->r[1] = sn2_step(d, 1, 1);
        }
    }

    d->r[1] = sn2_step(d, 0, (int32_t)d->n[0]);
    {
        d->x1 = sn2_s24(0x400000);
    }
    {
        int32_t v5 = d->x1;
        SN2_XW(d, d->r[1], sn2_w24(v5));
        d->r[1] = sn2_step(d, 1, 1);
    }

    for (int lc654 = 0; lc654 < 7; lc654++) {
        {
            int32_t v6 = d->x0;
            SN2_XW(d, d->r[1], sn2_w24(v6));
            d->r[1] = sn2_step(d, 1, 1);
        }
    }

    {
        int32_t v7 = d->x1;
        SN2_XW(d, d->r[1], sn2_w24(v7));
        d->r[1] = sn2_step(d, 1, 1);
    }

    for (int lc658 = 0; lc658 < 8; lc658++) {
        {
            int32_t v8 = d->x0;
            SN2_XW(d, d->r[1], sn2_w24(v8));
            d->r[1] = sn2_step(d, 1, 1);
        }
    }

    {
        d->r[1] = sn2_w24(sn2_s24(0x0));
    }
    {
        int32_t v9 = sn2_s24(d->r[1]);
        SN2_XW(d, d->r[0], sn2_w24(v9));
    }
    goto L3_pass;
L2_pass:;
    {
        d->r[1] = sn2_w24(sn2_s24(0x3f));
    }
    {
        int32_t v10 = sn2_s24(d->r[1]);
        SN2_YW(d, sn2_step(d, 4, 0x15), sn2_w24(v10));
    }
    {
        d->r[1] = sn2_w24(sn2_s24(0x0));
    }
    {
        int32_t v11 = sn2_s24(d->r[1]);
        SN2_YW(d, sn2_step(d, 4, 0x17), sn2_w24(v11));
    }
    {
        int32_t v12 = sn2_s24(d->r[1]);
        SN2_YW(d, sn2_step(d, 4, 0x19), sn2_w24(v12));
    }
    {
        int32_t v13 = sn2_s24(d->r[1]);
        SN2_YW(d, sn2_step(d, 4, 0x1b), sn2_w24(v13));
    }
    {
        int32_t v14 = sn2_s24(d->r[1]);
        SN2_YW(d, sn2_step(d, 4, 0x1c), sn2_w24(v14));
    }
    {
        int32_t v15 = sn2_s24(d->r[1]);
        SN2_YW(d, sn2_step(d, 4, 0x1d), sn2_w24(v15));
    }
    {
        d->r[1] = sn2_w24(sn2_s24(0x0));
    }
    {
        int32_t v16 = sn2_s24(d->r[1]);
        d->x0 = v16;
    }
    {
        int32_t v17 = sn2_s24(SN2_XR(d, sn2_step(d, 0, 0xc0)));
        d->x1 = v17;
    }
    {
        uint32_t   wv = sn2_w24(d->x1);
        uint32_t * w  = &wv;

        if (!((*w >> 23) & 1)) {
            goto L4_pass;
        }
    }
    {
        d->x0 = sn2_s24(0x7fffff);
    }
L4_pass:;
    {
        int32_t v18 = d->x0;
        SN2_YW(d, sn2_step(d, 4, 0x1e), sn2_w24(v18));
    }
    {
        int32_t v19 = sn2_s24(d->r[1]);
        d->x0 = v19;
    }
    {
        int32_t v20 = sn2_s24(SN2_XR(d, sn2_step(d, 0, 0xc2)));
        d->x1 = v20;
    }
    {
        uint32_t   wv = sn2_w24(d->x1);
        uint32_t * w  = &wv;

        if (!((*w >> 23) & 1)) {
            goto L5_pass;
        }
    }
    {
        d->x0 = sn2_s24(0x7fffff);
    }
L5_pass:;
    {
        int32_t v21 = d->x0;
        SN2_YW(d, sn2_step(d, 4, 0x1f), sn2_w24(v21));
    }
    {
        int32_t v22 = sn2_s24(SN2_XR(d, 0x2f));
        d->x0 = v22;
    }
    {
        d->b = sn2_mac(d, 0, d->x0, d->x0, false, true);
    }
    {
        int32_t v23 = sn2_s24((uint32_t)(d->b & 0xFFFFFF));
        d->b = sn2_ld(v23);
    }
    {
        d->b = sn2_ext56((d->b < 0) ? -d->b : d->b);
        sn2_flags(d, d->b);
        d->v = false;
    }
    {
        uint32_t   wi = d->r[0];
        uint32_t   wv = SN2_XR(d, wi);
        uint32_t * w  = &wv;

        if (!((*w >> 1) & 1)) {
            goto L6_pass;
        }
    }
    {
        d->r[1] = sn2_w24(sn2_s24(0x800000));
    }
    {
        int32_t v24 = sn2_s24(SN2_XR(d, sn2_step(d, 0, 0xcc)));
        d->a = sn2_ld(v24);
    }
    {
        (void)sn2_sub(d, d->a, sn2_ld(sn2_s24(0x4)), true);
    }

    if (!d->z) {
        goto L7_pass;
    }
    {
        d->r[1] = sn2_w24(sn2_s24(0x400000));
    }
L7_pass:;
    {
        (void)sn2_sub(d, d->a, sn2_ld(sn2_s24(0x2)), true);
    }

    if (!d->z) {
        goto L8_pass;
    }
    {
        d->r[1] = sn2_w24(sn2_s24(0x2));
    }
L8_pass:;
    {
        int32_t v25 = sn2_s24(d->r[1]);
        SN2_YW(d, sn2_step(d, 4, 0x22), sn2_w24(v25));
    }
    {
        int32_t v26 = d->x0;
        SN2_YW(d, sn2_step(d, 4, 0x23), sn2_w24(v26));
    }
    {
        d->x1 = sn2_s24(0x0);
    }
    {
        int32_t v27 = d->x1;
        SN2_YW(d, sn2_step(d, 4, 0x36), sn2_w24(v27));
    }
L6_pass:;
    {
        uint32_t   wi = d->r[0];
        uint32_t   wv = SN2_XR(d, wi);
        uint32_t * w  = &wv;

        if (!((*w >> 2) & 1)) {
            goto L9_pass;
        }
    }
    {
        d->r[1] = sn2_w24(sn2_s24(0x800000));
    }
    {
        int32_t v28 = sn2_s24(SN2_XR(d, sn2_step(d, 0, 0xd4)));
        d->a = sn2_ld(v28);
    }
    {
        (void)sn2_sub(d, d->a, sn2_ld(sn2_s24(0x4)), true);
    }

    if (!d->z) {
        goto L10_pass;
    }
    {
        d->r[1] = sn2_w24(sn2_s24(0x400000));
    }
L10_pass:;
    {
        (void)sn2_sub(d, d->a, sn2_ld(sn2_s24(0x2)), true);
    }

    if (!d->z) {
        goto L11_pass;
    }
    {
        d->r[1] = sn2_w24(sn2_s24(0x2));
    }
L11_pass:;
    {
        int32_t v29 = sn2_s24(d->r[1]);
        SN2_YW(d, sn2_step(d, 4, 0x27), sn2_w24(v29));
    }
    {
        int32_t v30 = sn2_lim(d->b);
        SN2_YW(d, sn2_step(d, 4, 0x28), sn2_w24(v30));
    }
    {
        int32_t v31 = d->x1;
        SN2_YW(d, sn2_step(d, 4, 0x37), sn2_w24(v31));
    }
L9_pass:;
    {
        uint32_t   wi = d->r[0];
        uint32_t   wv = SN2_XR(d, wi);
        uint32_t * w  = &wv;

        if (!((*w >> 3) & 1)) {
            goto L12_pass;
        }
    }
    {
        int32_t v32 = sn2_s24(SN2_XR(d, d->r[0]));
        d->a = sn2_ld(v32);
    }
    {
        {
            uint32_t o = ((uint32_t)(d->a >> 24) & (uint32_t)0xffff00) & 0xFFFFFFu;
            d->a = sn2_set_hi(d->a, o);
            sn2_flags_hi(d, o);
        }
    }
    {
        int32_t v33 = sn2_hi(d->a);
        SN2_YW(d, sn2_step(d, 4, 0x0), sn2_w24(v33));
    }
    {
        int32_t v34 = sn2_hi(d->a);
        SN2_YW(d, sn2_step(d, 4, 0x4), sn2_w24(v34));
    }
    {
        int32_t v35 = sn2_hi(d->a);
        SN2_YW(d, sn2_step(d, 4, 0x8), sn2_w24(v35));
    }
L12_pass:;
    {
        uint32_t   wi = d->r[0];
        uint32_t   wv = SN2_XR(d, wi);
        uint32_t * w  = &wv;

        if (!((*w >> 4) & 1)) {
            goto L13_pass;
        }
    }
    {
        int32_t v36 = sn2_s24(SN2_XR(d, sn2_step(d, 0, 0xdd)));
        d->x0 = v36;
    }
    {
        int32_t v37 = d->x0;
        SN2_YW(d, sn2_step(d, 4, 0x32), sn2_w24(v37));
    }
L13_pass:;
    {
        int32_t v38 = sn2_s24(SN2_XR(d, sn2_step(d, 0, 0xd7)));
        d->r[1] = sn2_w24(v38);
    }
    {
        int32_t v39 = sn2_s24(d->r[1]);
        SN2_YW(d, sn2_step(d, 4, 0x2d), sn2_w24(v39));
    }
    {
        int32_t v40 = sn2_s24(d->r[1]);
        SN2_YW(d, sn2_step(d, 4, 0x2e), sn2_w24(v40));
    }
    {
        int32_t v41 = sn2_s24(SN2_XR(d, sn2_step(d, 0, 0xd8)));
        d->r[1] = sn2_w24(v41);
    }
    {
        int32_t v42 = sn2_s24(d->r[1]);
        SN2_YW(d, sn2_step(d, 4, 0x30), sn2_w24(v42));
    }
    {
        int32_t v43 = sn2_s24(d->r[1]);
        SN2_YW(d, sn2_step(d, 4, 0x31), sn2_w24(v43));
    }
    {
        int32_t v44 = sn2_s24(SN2_XR(d, sn2_step(d, 0, 0x86)));
        d->r[1] = sn2_w24(v44);
    }
    {
        int32_t v45 = sn2_s24(d->r[1]);
        SN2_YW(d, sn2_step(d, 4, 0xe), sn2_w24(v45));
    }
    {
        int32_t v46 = sn2_s24(d->r[1]);
        SN2_YW(d, sn2_step(d, 4, 0xf), sn2_w24(v46));
    }
L3_pass:;
    {
        int32_t v47 = sn2_s24(d->r[0]);
        d->a = sn2_ld(v47);
    }
    {
        d->x0 = sn2_s24(0x10e);
    }
    {
        d->a = sn2_add(d, d->a, sn2_ld(d->x0), true);
    }
    {
        int32_t v48 = sn2_lim(d->a);
        d->r[0] = sn2_w24(v48);
    }
    d->r[4] = sn2_step(d, 4, 0x3b);
    goto L_end;
L1_pass:;
    {
        int32_t v49 = sn2_s24(d->r[0]);
        d->a = sn2_ld(v49);
    }
    {
        d->a = sn2_add(d, d->a, sn2_ld(sn2_s24(0xde)), true);
    }
    {
        int32_t v50 = sn2_s24(d->r[4]);
        d->n[3] = sn2_w24(v50);
    }
    {
        int32_t v51 = sn2_lim(d->a);
        d->r[7] = sn2_w24(v51);
    }
    {
        int32_t v52 = sn2_s24(d->lc);
        d->a = sn2_ld(v52);
    }
    {
        {
            uint32_t o = ((uint32_t)(d->a >> 24) & (uint32_t)0x3) & 0xFFFFFFu;
            d->a = sn2_set_hi(d->a, o);
            sn2_flags_hi(d, o);
        }
    }
    {
        int32_t v53 = sn2_s24(SN2_YR(d, 0x1b));
        d->x0 = v53;
    }
    {
        (void)sn2_sub(d, d->a, sn2_ld(d->x0), true);
    }

    if (d->z) {
        goto L14_pass;
    }
    {
        uint32_t   wi = d->r[0];
        uint32_t   wv = SN2_XR(d, wi);
        uint32_t * w  = &wv;
        d->c = (*w >> 6) & 1;
    }

    if (!d->c) {
        goto L3_pass;
    }
    {
        int32_t v54 = sn2_s24(d->r[7]);
        d->r[0] = sn2_w24(v54);
    }
    d->r[5] = sn2_step(d, 4, 0x3b);
    goto L15_pass;
L14_pass:;
    {
        uint32_t   wi = d->r[0];
        uint32_t   wv = SN2_XR(d, wi);
        uint32_t * w  = &wv;
        d->c    = (*w >> 6) & 1;
        *w     |= (1u << 6);
        SN2_XW(d, wi, wv);
        d->r[0] = sn2_step(d, 0, 1);
    }
    d->r[5] = sn2_step(d, 4, 0x34);

    for (int lc797 = 0; lc797 < 2; lc797++) {
        {
            int32_t v55 = sn2_s24(SN2_XR(d, d->r[0]));
            d->a    = sn2_ld(v55);
            d->r[0] = sn2_step(d, 0, 1);
        }
        {
            int32_t v56 = sn2_s24(SN2_XR(d, d->r[0]));
            int32_t v57 = sn2_s24(SN2_YR(d, d->r[5]));
            d->x0   = v56;
            d->y0   = v57;
            d->r[0] = sn2_step(d, 0, 1);
            d->r[5] = sn2_step(d, 5, 1);
        }
        {
            int32_t v58 = sn2_s24(SN2_XR(d, d->r[0]));
            int32_t v59 = sn2_s24(SN2_YR(d, d->r[5]));
            d->a    = sn2_mac(d, d->a, d->y0, d->x0, false, true);
            d->x0   = v58;
            d->y0   = v59;
            d->r[0] = sn2_step(d, 0, 1);
            d->r[5] = sn2_step(d, 5, 1);
        }
        {
            int32_t v60 = sn2_s24(SN2_XR(d, d->r[0]));
            int32_t v61 = sn2_s24(SN2_YR(d, d->r[5]));
            d->a    = sn2_mac(d, d->a, d->y0, d->x0, false, true);
            d->x0   = v60;
            d->y0   = v61;
            d->r[0] = sn2_step(d, 0, 1);
            d->r[5] = sn2_step(d, 5, 1);
        }
        {
            int32_t v62 = sn2_s24(SN2_XR(d, d->r[0]));
            int32_t v63 = sn2_s24(SN2_YR(d, d->r[5]));
            d->a    = sn2_mac(d, d->a, d->y0, d->x0, false, true);
            d->x0   = v62;
            d->y0   = v63;
            d->r[0] = sn2_step(d, 0, 1);
            d->r[5] = sn2_step(d, 5, 1);
        }
        {
            int32_t v64 = sn2_s24(SN2_XR(d, d->r[0]));
            int32_t v65 = sn2_s24(SN2_YR(d, d->r[5]));
            d->a    = sn2_mac(d, d->a, d->y0, d->x0, false, true);
            d->x0   = v64;
            d->y0   = v65;
            d->r[0] = sn2_step(d, 0, 1);
            d->r[5] = sn2_step(d, 5, 1);
        }
        {
            int32_t v66 = sn2_s24(SN2_XR(d, d->r[0]));
            int32_t v67 = sn2_s24(SN2_YR(d, d->r[5]));
            d->a    = sn2_mac(d, d->a, d->y0, d->x0, false, true);
            d->x0   = v66;
            d->y0   = v67;
            d->r[5] = sn2_step(d, 5, 1);
        }
        {
            int32_t v68 = sn2_s24(SN2_XR(d, d->r[0]));
            int32_t v69 = sn2_s24(SN2_YR(d, d->r[5]));
            d->x0   = v68;
            d->y0   = v69;
            d->r[0] = sn2_step(d, 0, 1);
            d->r[5] = sn2_step(d, 5, -1);
        }
        {
            int32_t v70 = sn2_s24(SN2_YR(d, 0x11));
            d->a    = sn2_mac(d, d->a, d->y0, d->x0, false, true);
            d->r[6] = sn2_w24(v70);
        }
        {
            if (d->neg) {
                d->a = 0;
            }
        }
        {
            d->r[5] = sn2_step(d, 5, -(int32_t)d->n[5]);
        }
        {
            int32_t v71 = sn2_lim(d->a);
            d->x1 = v71;
        }
        {
            d->b = sn2_mac(d, 0, d->x1, sn2_s24(0x2000), false, true);
        }
        {
            d->a = sn2_mac(d, 0, d->x1, sn2_s24(0x10), false, true);
        }
        {
            int32_t v72 = sn2_lim(d->b);
            {
                uint32_t w = (uint32_t)(d->b >> 24) & 0xFFFFFFu;
                uint32_t o = (w << 1) & 0xFFFFFFu;
                d->b = sn2_set_hi(d->b, o);
                d->c = (w >> 23) & 1;
                sn2_flags_hi(d, o);
            }
            d->n[6] = sn2_w24(v72);
        }
        {
            int32_t v73 = sn2_s24(SN2_YR(d, 0xa));
            d->b  = sn2_asr(d, d->b, 1, true);
            d->x0 = v73;
        }
        {
            d->a = sn2_add(d, d->a, sn2_ld(sn2_s24(0x9)), true);
        }
        {
            int32_t v74 = sn2_s24((uint32_t)(d->b & 0xFFFFFF));
            d->b   = 0;
            d->z   = true;
            d->neg = false;
            d->e   = false;
            d->u   = true;
            d->v   = false;
            d->y1  = v74;
        }
        {
            int32_t v75 = sn2_s24(SN2_YR(d, sn2_step(d, 6, (int32_t)d->n[6])));
            d->b = (d->b & ~(int64_t)0xFFFFFF) | (int64_t)sn2_w24(v75);
        }
        {
            d->a = sn2_asl(d, d->b, (int)(sn2_w24(sn2_hi(d->a)) & 0x3F), true);
        }
        {
            int32_t v76 = sn2_s24(SN2_XR(d, d->r[0]));
            d->b    = sn2_mac(d, 0, d->x0, d->y1, false, true);
            d->y0   = v76;
            d->r[0] = sn2_step(d, 0, 1);
        }
        {
            int32_t v77 = sn2_lim(d->a);
            d->y1 = v77;
        }
        {
            int32_t v78 = sn2_lim(d->b);
            d->x0 = v78;
        }
        {
            int32_t v79 = sn2_s24(SN2_XR(d, d->r[0]));
            int32_t v80 = sn2_s24(SN2_YR(d, d->r[4]));
            d->a    = sn2_mac(d, d->a, d->x0, d->y1, false, true);
            d->x0   = v79;
            d->y1   = v80;
            d->r[0] = sn2_step(d, 0, 1);
            d->r[4] = sn2_step(d, 4, 1);
        }
        {
            int32_t v81 = sn2_s24(SN2_XR(d, d->r[0]));
            d->a    = sn2_mac(d, d->a, d->x1, d->y0, false, true);
            d->b    = sn2_ld(v81);
            d->r[0] = sn2_step(d, 0, 1);
        }
        {
            int32_t v82 = sn2_s24(SN2_XR(d, d->r[0]));
            int32_t v83 = sn2_s24(SN2_YR(d, d->r[5]));
            d->b    = sn2_mac(d, d->b, d->x0, d->y1, true, true);
            d->x0   = v82;
            d->y0   = v83;
            d->r[0] = sn2_step(d, 0, 1);
            d->r[5] = sn2_step(d, 5, 1);
        }
        {
            int32_t v84 = sn2_s24(SN2_XR(d, d->r[0]));
            int32_t v85 = sn2_s24(SN2_YR(d, d->r[5]));
            d->b    = sn2_mac(d, d->b, d->y0, d->x0, false, true);
            d->x0   = v84;
            d->y0   = v85;
            d->r[0] = sn2_step(d, 0, 1);
            d->r[5] = sn2_step(d, 5, 1);
        }
        {
            int32_t v86 = sn2_s24(SN2_XR(d, d->r[0]));
            int32_t v87 = sn2_s24(SN2_YR(d, d->r[5]));
            d->b    = sn2_mac(d, d->b, d->y0, d->x0, false, true);
            d->x0   = v86;
            d->y0   = v87;
            d->r[0] = sn2_step(d, 0, 1);
            d->r[5] = sn2_step(d, 5, 1);
        }
        {
            int32_t v88 = sn2_s24(SN2_XR(d, d->r[0]));
            int32_t v89 = sn2_s24(SN2_YR(d, d->r[5]));
            d->b    = sn2_mac(d, d->b, d->y0, d->x0, false, true);
            d->x0   = v88;
            d->y0   = v89;
            d->r[0] = sn2_step(d, 0, 1);
            d->r[5] = sn2_step(d, 5, 1);
        }
        {
            int32_t v90 = sn2_s24(SN2_XR(d, d->r[0]));
            int32_t v91 = sn2_s24(SN2_YR(d, d->r[5]));
            d->b    = sn2_mac(d, d->b, d->y0, d->x0, false, true);
            d->x0   = v90;
            d->y0   = v91;
            d->r[0] = sn2_step(d, 0, 1);
            d->r[5] = sn2_step(d, 5, 1);
        }
        {
            int32_t v92 = sn2_s24(SN2_XR(d, d->r[0]));
            int32_t v93 = sn2_s24(SN2_YR(d, d->r[5]));
            d->b    = sn2_mac(d, d->b, d->y0, d->x0, false, true);
            d->x0   = v92;
            d->y0   = v93;
            d->r[5] = sn2_step(d, 5, 1);
        }
        {
            int32_t v94 = sn2_s24(SN2_XR(d, d->r[0]));
            int32_t v95 = sn2_s24(SN2_YR(d, d->r[5]));
            d->b    = sn2_asr(d, d->b, 1, true);
            d->x0   = v94;
            d->y0   = v95;
            d->r[0] = sn2_step(d, 0, 1);
            d->r[5] = sn2_step(d, 5, -1);
        }
        {
            d->b    = sn2_mac(d, d->b, d->y0, d->x0, true, true);
            d->r[5] = sn2_step(d, 5, -(int32_t)d->n[5]);
        }
        {
            if (d->neg) {
                d->b = 0;
            }
        }
        {
            int32_t v96 = sn2_lim(d->a);
            SN2_XW(d, d->r[7], sn2_w24(v96));
            d->r[7] = sn2_step(d, 7, 1);
        }
        {
            int32_t v97 = sn2_lim(d->b);
            d->b = sn2_add(d, d->b, sn2_ld(d->x1), true);
            d->a = sn2_ld(v97);
        }
        {
            int32_t v98 = sn2_s24(SN2_YR(d, 0x0));
            d->y1 = v98;
        }
        {
            d->b = sn2_sub(d, d->b, sn2_ld(d->y1), true);
        }
        {
            if (!d->neg) {
                d->a = sn2_sub(d, d->a, d->b, false);
            }
        }
        {
            {
                bool c = d->c;
                (void)sn2_sub(d, d->a, 0, true);
                d->c = c;
                d->v = false;
            }
        }
        {
            if (d->neg) {
                d->a = 0;
            }
        }
        {
            d->b = sn2_add(d, d->b, sn2_ld(sn2_s24(0x80000)), true);
        }
        {
            d->a = sn2_asr(d, d->a, 10, true);
        }
        {
            d->b    = sn2_add(d, d->b, sn2_ld(d->y1), true);
            d->r[4] = sn2_step(d, 4, 1);
        }
        {
            int32_t v99 = sn2_hi(d->a);
            d->n[6] = sn2_w24(v99);
        }
        {
            d->a = sn2_asr(d, d->a, 9, true);
        }
        {
            d->r[4] = sn2_step(d, 4, 1);
        }
        {
            int32_t v100 = sn2_lim(d->a);
            d->y1 = v100;
        }
        {
            int32_t v101 = sn2_s24(SN2_YR(d, sn2_step(d, 6, (int32_t)d->n[6])));
            d->a = sn2_ld(v101);
        }
        {
            d->a = sn2_asr(d, d->a, 4, true);
        }
        {
            d->a = sn2_asl(d, d->a, (int)(sn2_w24(d->y1) & 0x3F), true);
        }
        {
            int32_t v102 = sn2_lim(d->b);
            d->y0 = v102;
        }
        {
            int32_t v103 = sn2_lim(d->a);
            d->b = sn2_ld(v103);
        }
        {
            int32_t v104 = sn2_lim(d->a);
            {
                uint32_t w = (uint32_t)(d->b >> 24) & 0xFFFFFFu;
                uint32_t o = (w >> 1) & 0xFFFFFFu;
                d->b = sn2_set_hi(d->b, o);
                d->c = (w >> 0) & 1;
                sn2_flags_hi(d, o);
            }
            d->a = sn2_ld(v104);
        }
        {
            d->x1 = sn2_s24(0xfc0000);
        }
        {
            int32_t v105 = sn2_lim(d->b);
            int32_t v106 = sn2_lim(d->b);
            {
                uint32_t o = ((uint32_t)(d->a >> 24) & sn2_w24(d->x1)) & 0xFFFFFFu;
                d->a = sn2_set_hi(d->a, o);
                sn2_flags_hi(d, o);
            }
            SN2_XW(d, d->r[7], sn2_w24(v105));
            d->y1   = v106;
            d->r[7] = sn2_step(d, 7, 1);
        }
        {
            d->x1 = sn2_s24(0x1ffff);
        }
        {
            int32_t v107 = sn2_hi(d->a);
            d->a = sn2_ld(d->y1);
            SN2_XW(d, d->r[7], sn2_w24(v107));
        }
        {
            int32_t v108 = sn2_s24(SN2_XR(d, d->r[7]));
            {
                uint32_t o = ((uint32_t)(d->a >> 24) & sn2_w24(d->x1)) & 0xFFFFFFu;
                d->a = sn2_set_hi(d->a, o);
                sn2_flags_hi(d, o);
            }
            d->b    = sn2_ld(v108);
            d->r[7] = sn2_step(d, 7, 1);
        }
        {
            d->b = sn2_add(d, d->b, d->a, true);
        }
        {
            {
                uint32_t o = ((uint32_t)(d->a >> 24) ^ sn2_w24(d->x1)) & 0xFFFFFFu;
                d->a = sn2_set_hi(d->a, o);
                sn2_flags_hi(d, o);
            }
        }
        {
            int32_t v109 = sn2_lim(d->b);
            d->b    = sn2_ld(d->y0);
            SN2_XW(d, d->r[7], sn2_w24(v109));
            d->r[7] = sn2_step(d, 7, 1);
        }
        {
            int32_t v110 = sn2_s24(SN2_XR(d, d->r[0]));
            int32_t v111 = sn2_s24(SN2_YR(d, d->r[5]));
            d->x1   = v110;
            d->y0   = v111;
            d->r[0] = sn2_step(d, 0, 1);
            d->r[5] = sn2_step(d, 5, 1);
        }
        {
            int32_t v112 = sn2_s24(SN2_XR(d, d->r[0]));
            int32_t v113 = sn2_lim(d->a);
            d->a    = sn2_ld(v112);
            d->y1   = v113;
            d->r[0] = sn2_step(d, 0, 1);
        }
        {
            int32_t v114 = sn2_s24(SN2_XR(d, d->r[0]));
            int32_t v115 = sn2_s24(SN2_YR(d, d->r[5]));
            d->a    = sn2_mac(d, d->a, d->x1, d->y0, true, true);
            d->x1   = v114;
            d->y0   = v115;
            d->r[0] = sn2_step(d, 0, 1);
            d->r[5] = sn2_step(d, 5, 1);
        }
        {
            int32_t v116 = sn2_s24(SN2_XR(d, d->r[0]));
            int32_t v117 = sn2_s24(SN2_YR(d, d->r[5]));
            d->a    = sn2_mac(d, d->a, d->x1, d->y0, true, true);
            d->x1   = v116;
            d->y0   = v117;
            d->r[0] = sn2_step(d, 0, 1);
            d->r[5] = sn2_step(d, 5, 1);
        }
        {
            int32_t v118 = sn2_s24(SN2_XR(d, d->r[0]));
            int32_t v119 = sn2_s24(SN2_YR(d, d->r[5]));
            d->a    = sn2_mac(d, d->a, d->x1, d->y0, true, true);
            d->x1   = v118;
            d->y0   = v119;
            d->r[0] = sn2_step(d, 0, 1);
            d->r[5] = sn2_step(d, 5, 1);
        }
        {
            int32_t v120 = sn2_s24(SN2_XR(d, d->r[0]));
            int32_t v121 = sn2_s24(SN2_YR(d, d->r[5]));
            d->a    = sn2_mac(d, d->a, d->x1, d->y0, true, true);
            d->x1   = v120;
            d->y0   = v121;
            d->r[0] = sn2_step(d, 0, 1);
            d->r[5] = sn2_step(d, 5, 1);
        }
        {
            int32_t v122 = sn2_s24(SN2_YR(d, 0x0));
            d->a  = sn2_mac(d, d->a, d->x1, d->y0, true, true);
            d->y0 = v122;
        }
        {
            if (d->neg) {
                d->a = 0;
            }
        }
        {
            d->a    = sn2_add(d, d->a, sn2_ld(d->y0), true);
            d->r[4] = sn2_step(d, 4, 1);
        }
        {
            d->a    = sn2_asr(d, d->a, 1, true);
            d->r[5] = sn2_step(d, 5, -(int32_t)d->n[5]);
        }
        {
            int32_t v123 = d->y1;

            if (d->a >= d->b) {
                d->b = d->a;
                d->c = false;
            } else {
                d->c = true;
            }
            SN2_XW(d, d->r[7], sn2_w24(v123));
            d->r[7] = sn2_step(d, 7, 1);
        }
        {
            int32_t v124 = sn2_s24(SN2_XR(d, d->r[0]));
            int32_t v125 = sn2_s24(SN2_YR(d, d->r[5]));
            d->x0   = v124;
            d->y1   = v125;
            d->r[0] = sn2_step(d, 0, 1);
            d->r[5] = sn2_step(d, 5, 1);
        }
        {
            int32_t v126 = sn2_s24(SN2_XR(d, d->r[0]));
            int32_t v127 = sn2_lim(d->b);
            d->b    = sn2_ld(v126);
            d->y0   = v127;
            d->r[0] = sn2_step(d, 0, 1);
        }
        {
            int32_t v128 = sn2_s24(SN2_XR(d, d->r[0]));
            int32_t v129 = sn2_s24(SN2_YR(d, d->r[5]));
            d->b    = sn2_mac(d, d->b, d->x0, d->y1, false, true);
            d->x0   = v128;
            d->y1   = v129;
            d->r[0] = sn2_step(d, 0, 1);
            d->r[5] = sn2_step(d, 5, 1);
        }
        {
            int32_t v130 = sn2_s24(SN2_XR(d, d->r[0]));
            int32_t v131 = sn2_s24(SN2_YR(d, d->r[5]));
            d->b    = sn2_mac(d, d->b, d->x0, d->y1, false, true);
            d->x0   = v130;
            d->y1   = v131;
            d->r[0] = sn2_step(d, 0, 1);
            d->r[5] = sn2_step(d, 5, 1);
        }
        {
            int32_t v132 = sn2_s24(SN2_XR(d, d->r[0]));
            int32_t v133 = sn2_s24(SN2_YR(d, d->r[5]));
            d->b    = sn2_mac(d, d->b, d->x0, d->y1, false, true);
            d->x0   = v132;
            d->y1   = v133;
            d->r[0] = sn2_step(d, 0, 1);
            d->r[5] = sn2_step(d, 5, 1);
        }
        {
            int32_t v134 = sn2_s24(SN2_XR(d, d->r[0]));
            int32_t v135 = sn2_s24(SN2_YR(d, d->r[5]));
            d->b    = sn2_mac(d, d->b, d->x0, d->y1, false, true);
            d->x0   = v134;
            d->y1   = v135;
            d->r[0] = sn2_step(d, 0, 1);
            d->r[5] = sn2_step(d, 5, 1);
        }
        {
            d->b    = sn2_mac(d, d->b, d->x0, d->y1, false, true);
            d->r[5] = sn2_step(d, 5, -(int32_t)d->n[5]);
        }
        {
            int32_t v136 = sn2_s24(SN2_XR(d, d->r[0]));
            d->b    = sn2_asl(d, d->b, 1, true);
            d->y1   = v136;
            d->r[0] = sn2_step(d, 0, 1);
        }
        {
            int32_t v137 = sn2_s24(SN2_YR(d, 0xb));
            d->a  = sn2_ld(d->y0);
            d->x0 = v137;
        }
        {
            int32_t v138 = sn2_s24(SN2_YR(d, 0x0));
            d->a  = sn2_sub(d, d->a, sn2_ld(d->x0), true);
            d->x1 = v138;
        }
        {
            d->a = sn2_asl(d, d->a, 5, true);
        }
        {
            int32_t v139 = sn2_s24(SN2_YR(d, 0xd));
            d->y0 = v139;
        }
        {
            int32_t v140 = (int32_t)(int8_t)(d->a >> 48);
            {
                uint32_t o = ~(uint32_t)(d->a >> 24) & 0xFFFFFFu;
                d->a = sn2_set_hi(d->a, o);
                sn2_flags_hi(d, o);
            }
            d->x0 = v140;
        }
        {
            d->a = (d->a & (((int64_t)1 << 48) - 1)) | ((int64_t)(int8_t)(sn2_s24(0x1)) * ((int64_t)1 << 48));
        }
        {
            d->a = sn2_asr(d, d->a, (int)(sn2_w24(d->x0) & 0x3F), true);
        }
        {
            d->a = sn2_asr(d, d->a, 3, true);
        }
        {
            int32_t v141 = sn2_s24(SN2_YR(d, 0xc));
            d->a  = sn2_sub(d, d->a, sn2_ld(d->y0), true);
            d->y0 = v141;
        }
        {
            int32_t v142 = sn2_lim(d->b);
            d->b  = sn2_ld(d->y1);
            d->x0 = v142;
        }
        {
            {
                uint32_t w = (uint32_t)(d->b >> 24) & 0xFFFFFFu;
                uint32_t o = (w >> 3) & 0xFFFFFFu;
                d->b = sn2_set_hi(d->b, o);
                d->c = (w >> 2) & 1;
                sn2_flags_hi(d, o);
            }
        }
        {
            if (d->c) {
                d->a = sn2_sub(d, d->a, sn2_ld(d->y0), false);
            }
        }
        {
            int32_t v143 = d->x0;
            d->b = sn2_ld(v143);
        }
        {
            if (!d->c) {
                d->b = sn2_add(d, d->b, sn2_ld(d->x1), false);
            }
        }
        {
            d->b = sn2_asr(d, d->b, 6, true);
        }
        {
            int32_t v144 = sn2_s24(SN2_XR(d, d->r[0]));
            d->x1   = v144;
            d->r[0] = sn2_step(d, 0, 1);
        }
        {
            int32_t v145 = sn2_lim(d->a);
            SN2_XW(d, d->r[7], sn2_w24(v145));
            d->r[7] = sn2_step(d, 7, 1);
        }
        {
            int32_t v146 = sn2_lim(d->b);
            SN2_XW(d, d->r[7], sn2_w24(v146));
            d->r[7] = sn2_step(d, 7, 1);
        }
        {
            int32_t v147 = d->y1;
            SN2_XW(d, d->r[7], sn2_w24(v147));
            d->r[7] = sn2_step(d, 7, 1);
        }
        {
            int32_t v148 = sn2_s24(SN2_XR(d, d->r[0]));
            d->b    = sn2_ld(v148);
            d->r[0] = sn2_step(d, 0, 1);
        }
        {
            int32_t v149 = sn2_s24(SN2_XR(d, d->r[0]));
            int32_t v150 = sn2_s24(SN2_YR(d, d->r[5]));
            d->x0   = v149;
            d->y0   = v150;
            d->r[0] = sn2_step(d, 0, 1);
            d->r[5] = sn2_step(d, 5, 1);
        }
        {
            int32_t v151 = sn2_s24(SN2_XR(d, d->r[0]));
            int32_t v152 = sn2_s24(SN2_YR(d, d->r[5]));
            d->b    = sn2_mac(d, d->b, d->y0, d->x0, false, true);
            d->x0   = v151;
            d->y0   = v152;
            d->r[0] = sn2_step(d, 0, 1);
            d->r[5] = sn2_step(d, 5, 1);
        }
        {
            int32_t v153 = sn2_s24(SN2_XR(d, d->r[0]));
            int32_t v154 = sn2_s24(SN2_YR(d, d->r[5]));
            d->b    = sn2_mac(d, d->b, d->y0, d->x0, false, true);
            d->x0   = v153;
            d->y0   = v154;
            d->r[0] = sn2_step(d, 0, 1);
            d->r[5] = sn2_step(d, 5, 1);
        }
        {
            int32_t v155 = sn2_s24(SN2_XR(d, d->r[0]));
            int32_t v156 = sn2_s24(SN2_YR(d, d->r[5]));
            d->b    = sn2_mac(d, d->b, d->y0, d->x0, false, true);
            d->x0   = v155;
            d->y0   = v156;
            d->r[0] = sn2_step(d, 0, 1);
            d->r[5] = sn2_step(d, 5, 1);
        }
        {
            int32_t v157 = sn2_s24(SN2_XR(d, d->r[0]));
            int32_t v158 = sn2_s24(SN2_YR(d, d->r[5]));
            d->b    = sn2_mac(d, d->b, d->y0, d->x0, false, true);
            d->x0   = v157;
            d->y0   = v158;
            d->r[0] = sn2_step(d, 0, 1);
            d->r[5] = sn2_step(d, 5, 1);
        }
        {
            d->b    = sn2_mac(d, d->b, d->y0, d->x0, false, true);
            d->r[5] = sn2_step(d, 5, -(int32_t)d->n[5]);
        }
        {
            if (d->neg) {
                d->b = 0;
            }
        }
        {
            int32_t v159 = d->x1;
            SN2_XW(d, d->r[7], sn2_w24(v159));
            d->r[7] = sn2_step(d, 7, 1);
        }
        {
            int32_t v160 = sn2_lim(d->b);
            SN2_XW(d, d->r[7], sn2_w24(v160));
            d->r[7] = sn2_step(d, 7, 1);
        }
    }

    {
        d->r[7] = sn2_step(d, 7, 1);
    }
    {
        d->r[7] = sn2_step(d, 7, 1);
    }
    {
        d->r[7] = sn2_step(d, 7, 1);
    }
    {
        int32_t v161 = sn2_s24(SN2_XR(d, d->r[0]));
        d->a    = sn2_ld(v161);
        d->r[0] = sn2_step(d, 0, 1);
    }
    {
        int32_t v162 = sn2_s24(SN2_XR(d, d->r[0]));
        int32_t v163 = sn2_s24(SN2_YR(d, d->r[5]));
        d->x0   = v162;
        d->y0   = v163;
        d->r[0] = sn2_step(d, 0, 1);
        d->r[5] = sn2_step(d, 5, 1);
    }
    {
        int32_t v164 = sn2_s24(SN2_XR(d, d->r[0]));
        int32_t v165 = sn2_s24(SN2_YR(d, d->r[5]));
        d->a    = sn2_mac(d, d->a, d->y0, d->x0, false, true);
        d->x0   = v164;
        d->y0   = v165;
        d->r[0] = sn2_step(d, 0, 1);
        d->r[5] = sn2_step(d, 5, 1);
    }
    {
        int32_t v166 = sn2_s24(SN2_XR(d, d->r[0]));
        int32_t v167 = sn2_s24(SN2_YR(d, d->r[5]));
        d->a    = sn2_mac(d, d->a, d->y0, d->x0, false, true);
        d->x0   = v166;
        d->y0   = v167;
        d->r[0] = sn2_step(d, 0, 1);
        d->r[5] = sn2_step(d, 5, 1);
    }
    {
        int32_t v168 = sn2_s24(SN2_XR(d, d->r[0]));
        int32_t v169 = sn2_s24(SN2_YR(d, d->r[5]));
        d->a    = sn2_mac(d, d->a, d->y0, d->x0, false, true);
        d->x0   = v168;
        d->y0   = v169;
        d->r[0] = sn2_step(d, 0, 1);
        d->r[5] = sn2_step(d, 5, 1);
    }
    {
        int32_t v170 = sn2_s24(SN2_XR(d, d->r[0]));
        int32_t v171 = sn2_s24(SN2_YR(d, d->r[5]));
        d->a    = sn2_mac(d, d->a, d->y0, d->x0, false, true);
        d->x0   = v170;
        d->y0   = v171;
        d->r[0] = sn2_step(d, 0, 1);
        d->r[5] = sn2_step(d, 5, 1);
    }
    {
        int32_t v172 = sn2_s24(SN2_XR(d, d->r[0]));
        int32_t v173 = sn2_s24(SN2_YR(d, d->r[5]));
        d->a    = sn2_mac(d, d->a, d->y0, d->x0, false, true);
        d->x0   = v172;
        d->y0   = v173;
        d->r[5] = sn2_step(d, 5, 1);
    }
    {
        int32_t v174 = sn2_s24(SN2_XR(d, d->r[0]));
        int32_t v175 = sn2_s24(SN2_YR(d, d->r[5]));
        d->x0   = v174;
        d->y0   = v175;
        d->r[0] = sn2_step(d, 0, 1);
        d->r[5] = sn2_step(d, 5, -1);
    }
    {
        int32_t v176 = sn2_s24(SN2_YR(d, 0x11));
        d->a    = sn2_mac(d, d->a, d->y0, d->x0, false, true);
        d->r[6] = sn2_w24(v176);
    }
    {
        if (d->neg) {
            d->a = 0;
        }
    }
    {
        d->r[5] = sn2_step(d, 5, -(int32_t)d->n[5]);
    }
    {
        int32_t v177 = sn2_lim(d->a);
        d->x1 = v177;
    }
    {
        d->b = sn2_mac(d, 0, d->x1, sn2_s24(0x2000), false, true);
    }
    {
        d->a = sn2_mac(d, 0, d->x1, sn2_s24(0x10), false, true);
    }
    {
        int32_t v178 = sn2_lim(d->b);
        {
            uint32_t w = (uint32_t)(d->b >> 24) & 0xFFFFFFu;
            uint32_t o = (w << 1) & 0xFFFFFFu;
            d->b = sn2_set_hi(d->b, o);
            d->c = (w >> 23) & 1;
            sn2_flags_hi(d, o);
        }
        d->n[6] = sn2_w24(v178);
    }
    {
        int32_t v179 = sn2_s24(SN2_YR(d, 0xa));
        d->b  = sn2_asr(d, d->b, 1, true);
        d->x0 = v179;
    }
    {
        d->a = sn2_add(d, d->a, sn2_ld(sn2_s24(0x9)), true);
    }
    {
        int32_t v180 = sn2_s24((uint32_t)(d->b & 0xFFFFFF));
        d->b   = 0;
        d->z   = true;
        d->neg = false;
        d->e   = false;
        d->u   = true;
        d->v   = false;
        d->y1  = v180;
    }
    {
        int32_t v181 = sn2_s24(SN2_YR(d, sn2_step(d, 6, (int32_t)d->n[6])));
        d->b = (d->b & ~(int64_t)0xFFFFFF) | (int64_t)sn2_w24(v181);
    }
    {
        d->a = sn2_asl(d, d->b, (int)(sn2_w24(sn2_hi(d->a)) & 0x3F), true);
    }
    {
        int32_t v182 = sn2_s24(SN2_XR(d, d->r[0]));
        d->b    = sn2_mac(d, 0, d->x0, d->y1, false, true);
        d->y0   = v182;
        d->r[0] = sn2_step(d, 0, 1);
    }
    {
        int32_t v183 = sn2_lim(d->a);
        d->y1 = v183;
    }
    {
        int32_t v184 = sn2_lim(d->b);
        d->x0 = v184;
    }
    {
        int32_t v185 = sn2_s24(SN2_XR(d, d->r[0]));
        int32_t v186 = sn2_s24(SN2_YR(d, d->r[4]));
        d->a    = sn2_mac(d, d->a, d->x0, d->y1, false, true);
        d->x0   = v185;
        d->y1   = v186;
        d->r[0] = sn2_step(d, 0, 1);
        d->r[4] = sn2_step(d, 4, 1);
    }
    {
        int32_t v187 = sn2_s24(SN2_XR(d, d->r[0]));
        d->a    = sn2_mac(d, d->a, d->x1, d->y0, false, true);
        d->b    = sn2_ld(v187);
        d->r[0] = sn2_step(d, 0, 1);
    }
    {
        int32_t v188 = sn2_s24(SN2_XR(d, d->r[0]));
        int32_t v189 = sn2_s24(SN2_YR(d, d->r[5]));
        d->b    = sn2_mac(d, d->b, d->x0, d->y1, true, true);
        d->x0   = v188;
        d->y0   = v189;
        d->r[0] = sn2_step(d, 0, 1);
        d->r[5] = sn2_step(d, 5, 1);
    }
    {
        int32_t v190 = sn2_s24(SN2_XR(d, d->r[0]));
        int32_t v191 = sn2_s24(SN2_YR(d, d->r[5]));
        d->b    = sn2_mac(d, d->b, d->y0, d->x0, false, true);
        d->x0   = v190;
        d->y0   = v191;
        d->r[0] = sn2_step(d, 0, 1);
        d->r[5] = sn2_step(d, 5, 1);
    }
    {
        int32_t v192 = sn2_s24(SN2_XR(d, d->r[0]));
        int32_t v193 = sn2_s24(SN2_YR(d, d->r[5]));
        d->b    = sn2_mac(d, d->b, d->y0, d->x0, false, true);
        d->x0   = v192;
        d->y0   = v193;
        d->r[0] = sn2_step(d, 0, 1);
        d->r[5] = sn2_step(d, 5, 1);
    }
    {
        int32_t v194 = sn2_s24(SN2_XR(d, d->r[0]));
        int32_t v195 = sn2_s24(SN2_YR(d, d->r[5]));
        d->b    = sn2_mac(d, d->b, d->y0, d->x0, false, true);
        d->x0   = v194;
        d->y0   = v195;
        d->r[0] = sn2_step(d, 0, 1);
        d->r[5] = sn2_step(d, 5, 1);
    }
    {
        int32_t v196 = sn2_s24(SN2_XR(d, d->r[0]));
        int32_t v197 = sn2_s24(SN2_YR(d, d->r[5]));
        d->b    = sn2_mac(d, d->b, d->y0, d->x0, false, true);
        d->x0   = v196;
        d->y0   = v197;
        d->r[0] = sn2_step(d, 0, 1);
        d->r[5] = sn2_step(d, 5, 1);
    }
    {
        int32_t v198 = sn2_s24(SN2_XR(d, d->r[0]));
        int32_t v199 = sn2_s24(SN2_YR(d, d->r[5]));
        d->b    = sn2_mac(d, d->b, d->y0, d->x0, false, true);
        d->x0   = v198;
        d->y0   = v199;
        d->r[5] = sn2_step(d, 5, 1);
    }
    {
        int32_t v200 = sn2_s24(SN2_XR(d, d->r[0]));
        int32_t v201 = sn2_s24(SN2_YR(d, d->r[5]));
        d->b    = sn2_asr(d, d->b, 1, true);
        d->x0   = v200;
        d->y0   = v201;
        d->r[0] = sn2_step(d, 0, 1);
        d->r[5] = sn2_step(d, 5, -1);
    }
    {
        d->b    = sn2_mac(d, d->b, d->y0, d->x0, true, true);
        d->r[5] = sn2_step(d, 5, -(int32_t)d->n[5]);
    }
    {
        if (d->neg) {
            d->b = 0;
        }
    }
    {
        int32_t v202 = sn2_lim(d->a);
        SN2_XW(d, d->r[7], sn2_w24(v202));
        d->r[7] = sn2_step(d, 7, 1);
    }
    {
        int32_t v203 = sn2_lim(d->b);
        d->b = sn2_add(d, d->b, sn2_ld(d->x1), true);
        d->a = sn2_ld(v203);
    }
    {
        int32_t v204 = sn2_s24(SN2_YR(d, 0x0));
        d->y1 = v204;
    }
    {
        d->b = sn2_sub(d, d->b, sn2_ld(d->y1), true);
    }
    {
        if (!d->neg) {
            d->a = sn2_sub(d, d->a, d->b, false);
        }
    }
    {
        {
            bool c = d->c;
            (void)sn2_sub(d, d->a, 0, true);
            d->c = c;
            d->v = false;
        }
    }
    {
        if (d->neg) {
            d->a = 0;
        }
    }
    {
        d->b = sn2_add(d, d->b, sn2_ld(sn2_s24(0x80000)), true);
    }
    {
        d->a = sn2_asr(d, d->a, 10, true);
    }
    {
        d->b    = sn2_add(d, d->b, sn2_ld(d->y1), true);
        d->r[4] = sn2_step(d, 4, 1);
    }
    {
        int32_t v205 = sn2_hi(d->a);
        d->n[6] = sn2_w24(v205);
    }
    {
        d->a = sn2_asr(d, d->a, 9, true);
    }
    {
        d->r[4] = sn2_step(d, 4, 1);
    }
    {
        int32_t v206 = sn2_lim(d->a);
        d->y1 = v206;
    }
    {
        int32_t v207 = sn2_s24(SN2_YR(d, sn2_step(d, 6, (int32_t)d->n[6])));
        d->a = sn2_ld(v207);
    }
    {
        d->a = sn2_asr(d, d->a, 4, true);
    }
    {
        d->a = sn2_asl(d, d->a, (int)(sn2_w24(d->y1) & 0x3F), true);
    }
    {
        int32_t v208 = sn2_lim(d->b);
        d->y0 = v208;
    }
    {
        int32_t v209 = sn2_lim(d->a);
        d->b = sn2_ld(v209);
    }
    {
        int32_t v210 = sn2_lim(d->a);
        {
            uint32_t w = (uint32_t)(d->b >> 24) & 0xFFFFFFu;
            uint32_t o = (w >> 1) & 0xFFFFFFu;
            d->b = sn2_set_hi(d->b, o);
            d->c = (w >> 0) & 1;
            sn2_flags_hi(d, o);
        }
        d->a = sn2_ld(v210);
    }
    {
        d->x1 = sn2_s24(0xfc0000);
    }
    {
        int32_t v211 = sn2_lim(d->b);
        int32_t v212 = sn2_lim(d->b);
        {
            uint32_t o = ((uint32_t)(d->a >> 24) & sn2_w24(d->x1)) & 0xFFFFFFu;
            d->a = sn2_set_hi(d->a, o);
            sn2_flags_hi(d, o);
        }
        SN2_XW(d, d->r[7], sn2_w24(v211));
        d->y1   = v212;
        d->r[7] = sn2_step(d, 7, 1);
    }
    {
        d->x1 = sn2_s24(0x1ffff);
    }
    {
        int32_t v213 = sn2_hi(d->a);
        d->a = sn2_ld(d->y1);
        SN2_XW(d, d->r[7], sn2_w24(v213));
    }
    {
        int32_t v214 = sn2_s24(SN2_XR(d, d->r[7]));
        {
            uint32_t o = ((uint32_t)(d->a >> 24) & sn2_w24(d->x1)) & 0xFFFFFFu;
            d->a = sn2_set_hi(d->a, o);
            sn2_flags_hi(d, o);
        }
        d->b    = sn2_ld(v214);
        d->r[7] = sn2_step(d, 7, 1);
    }
    {
        d->b = sn2_add(d, d->b, d->a, true);
    }
    {
        {
            uint32_t o = ((uint32_t)(d->a >> 24) ^ sn2_w24(d->x1)) & 0xFFFFFFu;
            d->a = sn2_set_hi(d->a, o);
            sn2_flags_hi(d, o);
        }
    }
    {
        int32_t v215 = sn2_lim(d->b);
        d->b    = sn2_ld(d->y0);
        SN2_XW(d, d->r[7], sn2_w24(v215));
        d->r[7] = sn2_step(d, 7, 1);
    }
    {
        int32_t v216 = sn2_s24(SN2_XR(d, d->r[0]));
        int32_t v217 = sn2_s24(SN2_YR(d, d->r[5]));
        d->x1   = v216;
        d->y0   = v217;
        d->r[0] = sn2_step(d, 0, 1);
        d->r[5] = sn2_step(d, 5, 1);
    }
    {
        int32_t v218 = sn2_s24(SN2_XR(d, d->r[0]));
        int32_t v219 = sn2_lim(d->a);
        d->a    = sn2_ld(v218);
        d->y1   = v219;
        d->r[0] = sn2_step(d, 0, 1);
    }
    {
        int32_t v220 = sn2_s24(SN2_XR(d, d->r[0]));
        int32_t v221 = sn2_s24(SN2_YR(d, d->r[5]));
        d->a    = sn2_mac(d, d->a, d->x1, d->y0, true, true);
        d->x1   = v220;
        d->y0   = v221;
        d->r[0] = sn2_step(d, 0, 1);
        d->r[5] = sn2_step(d, 5, 1);
    }
    {
        int32_t v222 = sn2_s24(SN2_XR(d, d->r[0]));
        int32_t v223 = sn2_s24(SN2_YR(d, d->r[5]));
        d->a    = sn2_mac(d, d->a, d->x1, d->y0, true, true);
        d->x1   = v222;
        d->y0   = v223;
        d->r[0] = sn2_step(d, 0, 1);
        d->r[5] = sn2_step(d, 5, 1);
    }
    {
        int32_t v224 = sn2_s24(SN2_XR(d, d->r[0]));
        int32_t v225 = sn2_s24(SN2_YR(d, d->r[5]));
        d->a    = sn2_mac(d, d->a, d->x1, d->y0, true, true);
        d->x1   = v224;
        d->y0   = v225;
        d->r[0] = sn2_step(d, 0, 1);
        d->r[5] = sn2_step(d, 5, 1);
    }
    {
        int32_t v226 = sn2_s24(SN2_XR(d, d->r[0]));
        int32_t v227 = sn2_s24(SN2_YR(d, d->r[5]));
        d->a    = sn2_mac(d, d->a, d->x1, d->y0, true, true);
        d->x1   = v226;
        d->y0   = v227;
        d->r[0] = sn2_step(d, 0, 1);
        d->r[5] = sn2_step(d, 5, 1);
    }
    {
        int32_t v228 = sn2_s24(SN2_YR(d, 0x0));
        d->a  = sn2_mac(d, d->a, d->x1, d->y0, true, true);
        d->y0 = v228;
    }
    {
        if (d->neg) {
            d->a = 0;
        }
    }
    {
        d->a    = sn2_add(d, d->a, sn2_ld(d->y0), true);
        d->r[4] = sn2_step(d, 4, 1);
    }
    {
        d->a    = sn2_asr(d, d->a, 1, true);
        d->r[5] = sn2_step(d, 5, -(int32_t)d->n[5]);
    }
    {
        int32_t v229 = d->y1;

        if (d->a >= d->b) {
            d->b = d->a;
            d->c = false;
        } else {
            d->c = true;
        }
        SN2_XW(d, d->r[7], sn2_w24(v229));
        d->r[7] = sn2_step(d, 7, 1);
    }
    {
        int32_t v230 = sn2_s24(SN2_XR(d, d->r[0]));
        int32_t v231 = sn2_s24(SN2_YR(d, d->r[5]));
        d->x0   = v230;
        d->y1   = v231;
        d->r[0] = sn2_step(d, 0, 1);
        d->r[5] = sn2_step(d, 5, 1);
    }
    {
        int32_t v232 = sn2_s24(SN2_XR(d, d->r[0]));
        int32_t v233 = sn2_lim(d->b);
        d->b    = sn2_ld(v232);
        d->y0   = v233;
        d->r[0] = sn2_step(d, 0, 1);
    }
    {
        int32_t v234 = sn2_s24(SN2_XR(d, d->r[0]));
        int32_t v235 = sn2_s24(SN2_YR(d, d->r[5]));
        d->b    = sn2_mac(d, d->b, d->x0, d->y1, false, true);
        d->x0   = v234;
        d->y1   = v235;
        d->r[0] = sn2_step(d, 0, 1);
        d->r[5] = sn2_step(d, 5, 1);
    }
    {
        int32_t v236 = sn2_s24(SN2_XR(d, d->r[0]));
        int32_t v237 = sn2_s24(SN2_YR(d, d->r[5]));
        d->b    = sn2_mac(d, d->b, d->x0, d->y1, false, true);
        d->x0   = v236;
        d->y1   = v237;
        d->r[0] = sn2_step(d, 0, 1);
        d->r[5] = sn2_step(d, 5, 1);
    }
    {
        int32_t v238 = sn2_s24(SN2_XR(d, d->r[0]));
        int32_t v239 = sn2_s24(SN2_YR(d, d->r[5]));
        d->b    = sn2_mac(d, d->b, d->x0, d->y1, false, true);
        d->x0   = v238;
        d->y1   = v239;
        d->r[0] = sn2_step(d, 0, 1);
        d->r[5] = sn2_step(d, 5, 1);
    }
    {
        int32_t v240 = sn2_s24(SN2_XR(d, d->r[0]));
        int32_t v241 = sn2_s24(SN2_YR(d, d->r[5]));
        d->b    = sn2_mac(d, d->b, d->x0, d->y1, false, true);
        d->x0   = v240;
        d->y1   = v241;
        d->r[0] = sn2_step(d, 0, 1);
        d->r[5] = sn2_step(d, 5, 1);
    }
    {
        d->b    = sn2_mac(d, d->b, d->x0, d->y1, false, true);
        d->r[5] = sn2_step(d, 5, -(int32_t)d->n[5]);
    }
    {
        int32_t v242 = sn2_s24(SN2_XR(d, d->r[0]));
        d->b    = sn2_asl(d, d->b, 1, true);
        d->y1   = v242;
        d->r[0] = sn2_step(d, 0, 1);
    }
    {
        int32_t v243 = sn2_s24(SN2_YR(d, 0xb));
        d->a  = sn2_ld(d->y0);
        d->x0 = v243;
    }
    {
        int32_t v244 = sn2_s24(SN2_YR(d, 0x0));
        d->a  = sn2_sub(d, d->a, sn2_ld(d->x0), true);
        d->x1 = v244;
    }
    {
        d->a = sn2_asl(d, d->a, 5, true);
    }
    {
        int32_t v245 = sn2_s24(SN2_YR(d, 0xd));
        d->y0 = v245;
    }
    {
        int32_t v246 = (int32_t)(int8_t)(d->a >> 48);
        {
            uint32_t o = ~(uint32_t)(d->a >> 24) & 0xFFFFFFu;
            d->a = sn2_set_hi(d->a, o);
            sn2_flags_hi(d, o);
        }
        d->x0 = v246;
    }
    {
        d->a = (d->a & (((int64_t)1 << 48) - 1)) | ((int64_t)(int8_t)(sn2_s24(0x1)) * ((int64_t)1 << 48));
    }
    {
        d->a = sn2_asr(d, d->a, (int)(sn2_w24(d->x0) & 0x3F), true);
    }
    {
        d->a = sn2_asr(d, d->a, 3, true);
    }
    {
        int32_t v247 = sn2_s24(SN2_YR(d, 0xc));
        d->a  = sn2_sub(d, d->a, sn2_ld(d->y0), true);
        d->y0 = v247;
    }
    {
        int32_t v248 = sn2_lim(d->b);
        d->b  = sn2_ld(d->y1);
        d->x0 = v248;
    }
    {
        {
            uint32_t w = (uint32_t)(d->b >> 24) & 0xFFFFFFu;
            uint32_t o = (w >> 3) & 0xFFFFFFu;
            d->b = sn2_set_hi(d->b, o);
            d->c = (w >> 2) & 1;
            sn2_flags_hi(d, o);
        }
    }
    {
        if (d->c) {
            d->a = sn2_sub(d, d->a, sn2_ld(d->y0), false);
        }
    }
    {
        int32_t v249 = d->x0;
        d->b = sn2_ld(v249);
    }
    {
        if (!d->c) {
            d->b = sn2_add(d, d->b, sn2_ld(d->x1), false);
        }
    }
    {
        d->b = sn2_asr(d, d->b, 6, true);
    }
    {
        int32_t v250 = sn2_s24(SN2_XR(d, d->r[0]));
        d->x1   = v250;
        d->r[0] = sn2_step(d, 0, 1);
    }
    {
        int32_t v251 = sn2_lim(d->a);
        SN2_XW(d, d->r[7], sn2_w24(v251));
        d->r[7] = sn2_step(d, 7, 1);
    }
    {
        int32_t v252 = sn2_lim(d->b);
        SN2_XW(d, d->r[7], sn2_w24(v252));
        d->r[7] = sn2_step(d, 7, 1);
    }
    {
        int32_t v253 = d->y1;
        SN2_XW(d, d->r[7], sn2_w24(v253));
        d->r[7] = sn2_step(d, 7, 1);
    }
    {
        int32_t v254 = sn2_s24(SN2_XR(d, d->r[0]));
        d->b    = sn2_ld(v254);
        d->r[0] = sn2_step(d, 0, 1);
    }
    {
        int32_t v255 = sn2_s24(SN2_XR(d, d->r[0]));
        int32_t v256 = sn2_s24(SN2_YR(d, d->r[5]));
        d->x0   = v255;
        d->y0   = v256;
        d->r[0] = sn2_step(d, 0, 1);
        d->r[5] = sn2_step(d, 5, 1);
    }
    {
        int32_t v257 = sn2_s24(SN2_XR(d, d->r[0]));
        int32_t v258 = sn2_s24(SN2_YR(d, d->r[5]));
        d->b    = sn2_mac(d, d->b, d->y0, d->x0, false, true);
        d->x0   = v257;
        d->y0   = v258;
        d->r[0] = sn2_step(d, 0, 1);
        d->r[5] = sn2_step(d, 5, 1);
    }
    {
        int32_t v259 = sn2_s24(SN2_XR(d, d->r[0]));
        int32_t v260 = sn2_s24(SN2_YR(d, d->r[5]));
        d->b    = sn2_mac(d, d->b, d->y0, d->x0, false, true);
        d->x0   = v259;
        d->y0   = v260;
        d->r[0] = sn2_step(d, 0, 1);
        d->r[5] = sn2_step(d, 5, 1);
    }
    {
        int32_t v261 = sn2_s24(SN2_XR(d, d->r[0]));
        int32_t v262 = sn2_s24(SN2_YR(d, d->r[5]));
        d->b    = sn2_mac(d, d->b, d->y0, d->x0, false, true);
        d->x0   = v261;
        d->y0   = v262;
        d->r[0] = sn2_step(d, 0, 1);
        d->r[5] = sn2_step(d, 5, 1);
    }
    {
        int32_t v263 = sn2_s24(SN2_XR(d, d->r[0]));
        int32_t v264 = sn2_s24(SN2_YR(d, d->r[5]));
        d->b    = sn2_mac(d, d->b, d->y0, d->x0, false, true);
        d->x0   = v263;
        d->y0   = v264;
        d->r[0] = sn2_step(d, 0, 1);
        d->r[5] = sn2_step(d, 5, 1);
    }
    {
        d->b    = sn2_mac(d, d->b, d->y0, d->x0, false, true);
        d->r[5] = sn2_step(d, 5, -(int32_t)d->n[5]);
    }
    {
        if (d->neg) {
            d->b = 0;
        }
    }
    {
        int32_t v265 = d->x1;
        SN2_XW(d, d->r[7], sn2_w24(v265));
        d->r[7] = sn2_step(d, 7, 1);
    }
    {
        int32_t v266 = sn2_lim(d->b);
        SN2_XW(d, d->r[7], sn2_w24(v266));
        d->r[7] = sn2_step(d, 7, 1);
    }
    {
        int32_t v267 = sn2_s24(SN2_XR(d, d->r[0]));
        d->n[1] = sn2_w24(v267);
        d->r[0] = sn2_step(d, 0, 1);
    }
    {
        int32_t v268 = sn2_s24(SN2_XR(d, d->r[0]));
        d->a    = sn2_ld(v268);
        d->r[0] = sn2_step(d, 0, 1);
    }
    {
        int32_t v269 = sn2_s24(SN2_XR(d, d->r[0]));
        int32_t v270 = sn2_s24(SN2_YR(d, d->r[5]));
        d->x0   = v269;
        d->y0   = v270;
        d->r[0] = sn2_step(d, 0, 1);
        d->r[5] = sn2_step(d, 5, 1);
    }
    {
        int32_t v271 = sn2_s24(SN2_XR(d, d->r[0]));
        int32_t v272 = sn2_s24(SN2_YR(d, d->r[5]));
        d->a    = sn2_mac(d, d->a, d->y0, d->x0, false, true);
        d->x0   = v271;
        d->y0   = v272;
        d->r[0] = sn2_step(d, 0, 1);
        d->r[5] = sn2_step(d, 5, 1);
    }
    {
        int32_t v273 = sn2_s24(SN2_XR(d, d->r[0]));
        int32_t v274 = sn2_s24(SN2_YR(d, d->r[5]));
        d->a    = sn2_mac(d, d->a, d->y0, d->x0, false, true);
        d->x0   = v273;
        d->y0   = v274;
        d->r[0] = sn2_step(d, 0, 1);
        d->r[5] = sn2_step(d, 5, 1);
    }
    {
        int32_t v275 = sn2_s24(SN2_XR(d, d->r[0]));
        int32_t v276 = sn2_s24(SN2_YR(d, d->r[5]));
        d->a    = sn2_mac(d, d->a, d->y0, d->x0, false, true);
        d->x0   = v275;
        d->y0   = v276;
        d->r[0] = sn2_step(d, 0, 1);
        d->r[5] = sn2_step(d, 5, 1);
    }
    {
        int32_t v277 = sn2_s24(SN2_XR(d, d->r[0]));
        int32_t v278 = sn2_s24(SN2_YR(d, d->r[5]));
        d->a    = sn2_mac(d, d->a, d->y0, d->x0, false, true);
        d->x0   = v277;
        d->y0   = v278;
        d->r[0] = sn2_step(d, 0, 1);
        d->r[5] = sn2_step(d, 5, 1);
    }
    {
        d->a    = sn2_mac(d, d->a, d->y0, d->x0, false, true);
        d->r[5] = sn2_step(d, 5, -(int32_t)d->n[5]);
    }
    {
        if (d->neg) {
            d->a = 0;
        }
    }
    {
        int32_t v279 = sn2_s24(SN2_XR(d, d->r[0]));
        int32_t v280 = sn2_s24(SN2_YR(d, d->r[4]));
        d->b    = sn2_ld(v279);
        d->y0   = v280;
        d->r[0] = sn2_step(d, 0, 1);
        d->r[4] = sn2_step(d, 4, 1);
    }
    {
        int32_t v281 = sn2_lim(d->a);
        d->a = sn2_ld(v281);
    }
    {
        d->a = sn2_asl(d, d->a, 4, true);
    }
    {
        int32_t v282 = sn2_s24(SN2_XR(d, d->r[0]));
        int32_t v283 = sn2_s24(SN2_YR(d, d->r[5]));
        d->x0   = v282;
        d->y0   = v283;
        d->r[0] = sn2_step(d, 0, 1);
        d->r[5] = sn2_step(d, 5, 1);
    }
    {
        int32_t v284 = (int32_t)(int8_t)(d->a >> 48);
        d->x1 = v284;
    }
    {
        d->a = (d->a & (((int64_t)1 << 48) - 1)) | ((int64_t)(int8_t)(sn2_s24(0x1)) * ((int64_t)1 << 48));
    }
    {
        d->a = sn2_asr(d, d->a, 9, true);
    }
    {
        d->a = sn2_asl(d, d->a, (int)(sn2_w24(d->x1) & 0x3F), true);
    }
    {
        int32_t v285 = sn2_s24(SN2_XR(d, d->r[0]));
        int32_t v286 = sn2_s24(SN2_YR(d, d->r[5]));
        d->b    = sn2_mac(d, d->b, d->y0, d->x0, false, true);
        d->x0   = v285;
        d->y0   = v286;
        d->r[0] = sn2_step(d, 0, 1);
        d->r[5] = sn2_step(d, 5, 1);
    }
    {
        int32_t v287 = sn2_s24(SN2_XR(d, d->r[0]));
        int32_t v288 = sn2_s24(SN2_YR(d, d->r[5]));
        d->b    = sn2_mac(d, d->b, d->y0, d->x0, false, true);
        d->x0   = v287;
        d->y0   = v288;
        d->r[0] = sn2_step(d, 0, 1);
        d->r[5] = sn2_step(d, 5, 1);
    }
    {
        int32_t v289 = sn2_s24(SN2_XR(d, d->r[0]));
        int32_t v290 = sn2_s24(SN2_YR(d, d->r[5]));
        d->b    = sn2_mac(d, d->b, d->y0, d->x0, false, true);
        d->x0   = v289;
        d->y0   = v290;
        d->r[0] = sn2_step(d, 0, 1);
        d->r[5] = sn2_step(d, 5, 1);
    }
    {
        int32_t v291 = sn2_s24(SN2_XR(d, d->r[0]));
        int32_t v292 = sn2_s24(SN2_YR(d, d->r[5]));
        d->b    = sn2_mac(d, d->b, d->y0, d->x0, false, true);
        d->x0   = v291;
        d->y0   = v292;
        d->r[0] = sn2_step(d, 0, 1);
        d->r[5] = sn2_step(d, 5, 1);
    }
    {
        d->b    = sn2_mac(d, d->b, d->y0, d->x0, false, true);
        d->r[5] = sn2_step(d, 5, -(int32_t)d->n[5]);
    }
    {
        if (d->neg) {
            d->b = 0;
        }
    }
    {
        int32_t v293 = sn2_lim(d->a);
        SN2_XW(d, d->r[7], sn2_w24(v293));
        d->r[7] = sn2_step(d, 7, 1);
    }
    {
        int32_t v294 = sn2_lim(d->b);
        d->a = sn2_ld(v294);
    }
    {
        d->c = (d->n[1] >> 2) & 1;
    }
    {
        if (d->c) {
            d->a = 0;
        }
    }
    {
        if (!d->c) {
            d->b = 0;
        }
    }
    {
        int32_t v295 = sn2_lim(d->a);
        SN2_XW(d, d->r[7], sn2_w24(v295));
        d->r[7] = sn2_step(d, 7, 1);
    }
    {
        int32_t v296 = sn2_lim(d->b);
        SN2_XW(d, sn2_step(d, 7, -0xf), sn2_w24(v296));
    }
    {
        int32_t v297 = sn2_s24(SN2_XR(d, d->r[0]));
        d->a    = sn2_ld(v297);
        d->r[0] = sn2_step(d, 0, 1);
    }
    {
        int32_t v298 = sn2_s24(SN2_XR(d, d->r[0]));
        int32_t v299 = sn2_s24(SN2_YR(d, d->r[5]));
        d->x0   = v298;
        d->y0   = v299;
        d->r[0] = sn2_step(d, 0, 1);
        d->r[5] = sn2_step(d, 5, 1);
    }
    {
        int32_t v300 = sn2_s24(SN2_XR(d, d->r[0]));
        int32_t v301 = sn2_s24(SN2_YR(d, d->r[5]));
        d->a    = sn2_mac(d, d->a, d->y0, d->x0, false, true);
        d->x0   = v300;
        d->y0   = v301;
        d->r[0] = sn2_step(d, 0, 1);
        d->r[5] = sn2_step(d, 5, 1);
    }
    {
        int32_t v302 = sn2_s24(SN2_XR(d, d->r[0]));
        int32_t v303 = sn2_s24(SN2_YR(d, d->r[5]));
        d->a    = sn2_mac(d, d->a, d->y0, d->x0, false, true);
        d->x0   = v302;
        d->y0   = v303;
        d->r[0] = sn2_step(d, 0, 1);
        d->r[5] = sn2_step(d, 5, 1);
    }
    {
        int32_t v304 = sn2_s24(SN2_XR(d, d->r[0]));
        int32_t v305 = sn2_s24(SN2_YR(d, d->r[5]));
        d->a    = sn2_mac(d, d->a, d->y0, d->x0, false, true);
        d->x0   = v304;
        d->y0   = v305;
        d->r[0] = sn2_step(d, 0, 1);
        d->r[5] = sn2_step(d, 5, 1);
    }
    {
        int32_t v306 = sn2_s24(SN2_XR(d, d->r[0]));
        int32_t v307 = sn2_s24(SN2_YR(d, d->r[5]));
        d->a    = sn2_mac(d, d->a, d->y0, d->x0, false, true);
        d->x0   = v306;
        d->y0   = v307;
        d->r[0] = sn2_step(d, 0, 1);
        d->r[5] = sn2_step(d, 5, 1);
    }
    {
        d->a    = sn2_mac(d, d->a, d->y0, d->x0, false, true);
        d->r[5] = sn2_step(d, 5, -(int32_t)d->n[5]);
    }
    {
        if (d->neg) {
            d->a = 0;
        }
    }
    {
        d->c = (d->n[1] >> 0) & 1;
    }
    {
        int32_t v308 = sn2_lim(d->a);
        d->b = sn2_ld(v308);
    }
    {
        if (d->c) {
            d->b = 0;
        }
    }
    {
        if (!d->c) {
            d->a = 0;
        }
    }
    {
        int32_t v309 = sn2_lim(d->b);
        SN2_XW(d, d->r[7], sn2_w24(v309));
        d->r[7] = sn2_step(d, 7, 1);
    }
    {
        int32_t v310 = sn2_lim(d->a);
        SN2_XW(d, sn2_step(d, 7, -0xf), sn2_w24(v310));
    }
    {
        int32_t v311 = sn2_s24(SN2_XR(d, d->r[0]));
        d->a    = sn2_ld(v311);
        d->r[0] = sn2_step(d, 0, 1);
    }
    {
        int32_t v312 = sn2_s24(SN2_XR(d, d->r[0]));
        int32_t v313 = sn2_s24(SN2_YR(d, d->r[5]));
        d->x0   = v312;
        d->y0   = v313;
        d->r[0] = sn2_step(d, 0, 1);
        d->r[5] = sn2_step(d, 5, 1);
    }
    {
        int32_t v314 = sn2_s24(SN2_XR(d, d->r[0]));
        int32_t v315 = sn2_s24(SN2_YR(d, d->r[5]));
        d->a    = sn2_mac(d, d->a, d->y0, d->x0, false, true);
        d->x0   = v314;
        d->y0   = v315;
        d->r[0] = sn2_step(d, 0, 1);
        d->r[5] = sn2_step(d, 5, 1);
    }
    {
        int32_t v316 = sn2_s24(SN2_XR(d, d->r[0]));
        int32_t v317 = sn2_s24(SN2_YR(d, d->r[5]));
        d->a    = sn2_mac(d, d->a, d->y0, d->x0, false, true);
        d->x0   = v316;
        d->y0   = v317;
        d->r[0] = sn2_step(d, 0, 1);
        d->r[5] = sn2_step(d, 5, 1);
    }
    {
        int32_t v318 = sn2_s24(SN2_XR(d, d->r[0]));
        int32_t v319 = sn2_s24(SN2_YR(d, d->r[5]));
        d->a    = sn2_mac(d, d->a, d->y0, d->x0, false, true);
        d->x0   = v318;
        d->y0   = v319;
        d->r[0] = sn2_step(d, 0, 1);
        d->r[5] = sn2_step(d, 5, 1);
    }
    {
        int32_t v320 = sn2_s24(SN2_XR(d, d->r[0]));
        int32_t v321 = sn2_s24(SN2_YR(d, d->r[5]));
        d->a    = sn2_mac(d, d->a, d->y0, d->x0, false, true);
        d->x0   = v320;
        d->y0   = v321;
        d->r[0] = sn2_step(d, 0, 1);
        d->r[5] = sn2_step(d, 5, 1);
    }
    {
        d->a    = sn2_mac(d, d->a, d->y0, d->x0, false, true);
        d->r[5] = sn2_step(d, 5, -(int32_t)d->n[5]);
    }
    {
        if (d->neg) {
            d->a = 0;
        }
    }
    {
        int32_t v322 = sn2_s24(SN2_YR(d, 0x7));
        d->y1 = v322;
    }
    {
        int32_t v323 = sn2_lim(d->a);
        d->b = sn2_ld(v323);
    }
    {
        d->c = (d->n[1] >> 1) & 1;
    }
    {
        if (d->c) {
            d->b = 0;
        }
    }
    {
        if (!d->c) {
            d->a = 0;
        }
    }
    {
        int32_t v324 = sn2_s24(SN2_YR(d, 0x8));
        d->x1 = v324;
    }
    {
        {
            uint32_t o = ((uint32_t)(d->b >> 24) | sn2_w24(d->x1)) & 0xFFFFFFu;
            d->b = sn2_set_hi(d->b, o);
            sn2_flags_hi(d, o);
        }
    }
    {
        d->c = (d->n[1] >> 5) & 1;
    }
    {
        if (!d->c) {{
                        uint32_t o = ((uint32_t)(d->b >> 24) ^ sn2_w24(d->x1)) & 0xFFFFFFu;
                        d->b = sn2_set_hi(d->b, o);
                    }
        }
    }
    {
        int32_t v325 = sn2_lim(d->a);
        SN2_XW(d, sn2_step(d, 7, -0xe), sn2_w24(v325));
    }
    {
        int32_t v326 = sn2_lim(d->b);
        SN2_XW(d, d->r[7], sn2_w24(v326));
        d->r[7] = sn2_step(d, 7, 1);
    }
    {
        int32_t v327 = sn2_s24(SN2_YR(d, 0x63));
        d->n[1] = sn2_w24(v327);
    }
    {
        int32_t v328 = sn2_s24(d->r[4]);
        d->r[1] = sn2_w24(v328);
    }
    {
        int32_t v329 = sn2_s24(SN2_XR(d, d->r[0]));
        int32_t v330 = sn2_s24(SN2_YR(d, d->r[4]));
        d->x1   = v329;
        d->y0   = v330;
        d->r[0] = sn2_step(d, 0, 1);
        d->r[4] = sn2_step(d, 4, 1);
    }
    {
        int32_t v331 = sn2_s24(SN2_XR(d, d->r[3]));
        int32_t v332 = sn2_s24(SN2_YR(d, d->r[4]));
        d->x0   = v331;
        d->y0   = v332;
        d->r[3] = sn2_step(d, 3, 1);
        d->r[4] = sn2_step(d, 4, 1);
    }
    {
        int32_t v333 = sn2_s24(SN2_XR(d, d->r[3]));
        int32_t v334 = sn2_s24(SN2_YR(d, d->r[4]));
        d->a    = sn2_mac(d, 0, d->y0, d->x0, false, true);
        d->x0   = v333;
        d->y0   = v334;
        d->r[3] = sn2_step(d, 3, -1);
        d->r[4] = sn2_step(d, 4, 1);
    }
    {
        int32_t v335 = d->x1;
        d->a = sn2_mac(d, d->a, d->y0, d->x0, false, true);
        SN2_YW(d, sn2_step(d, 1, (int32_t)d->n[1]), sn2_w24(v335));
    }
    {
        int32_t v336 = sn2_s24(SN2_XR(d, d->r[0]));
        int32_t v337 = sn2_s24(SN2_YR(d, d->r[5]));
        d->a    = sn2_add(d, d->a, sn2_ld(d->y1), true);
        d->x0   = v336;
        d->y0   = v337;
        d->r[0] = sn2_step(d, 0, 1);
        d->r[5] = sn2_step(d, 5, 1);
    }
    {
        int32_t v338 = sn2_s24(SN2_XR(d, d->r[0]));
        int32_t v339 = sn2_s24(SN2_YR(d, d->r[5]));
        d->a    = sn2_mac(d, d->a, d->y0, d->x0, false, true);
        d->x0   = v338;
        d->y0   = v339;
        d->r[0] = sn2_step(d, 0, 1);
        d->r[5] = sn2_step(d, 5, 1);
    }
    {
        int32_t v340 = sn2_s24(SN2_XR(d, d->r[0]));
        int32_t v341 = sn2_s24(SN2_YR(d, d->r[5]));
        d->a    = sn2_mac(d, d->a, d->y0, d->x0, false, true);
        d->x0   = v340;
        d->y0   = v341;
        d->r[0] = sn2_step(d, 0, 1);
        d->r[5] = sn2_step(d, 5, 1);
    }
    {
        int32_t v342 = sn2_s24(SN2_XR(d, d->r[0]));
        int32_t v343 = sn2_s24(SN2_YR(d, d->r[5]));
        d->a    = sn2_mac(d, d->a, d->y0, d->x0, false, true);
        d->x0   = v342;
        d->y0   = v343;
        d->r[0] = sn2_step(d, 0, 1);
        d->r[5] = sn2_step(d, 5, 1);
    }
    {
        int32_t v344 = sn2_s24(SN2_XR(d, d->r[0]));
        int32_t v345 = sn2_s24(SN2_YR(d, d->r[5]));
        d->a    = sn2_mac(d, d->a, d->y0, d->x0, false, true);
        d->x0   = v344;
        d->y0   = v345;
        d->r[0] = sn2_step(d, 0, 1);
        d->r[5] = sn2_step(d, 5, 1);
    }
    {
        int32_t v346 = sn2_s24(SN2_XR(d, d->r[0]));
        int32_t v347 = sn2_s24(SN2_YR(d, d->r[5]));
        d->a    = sn2_mac(d, d->a, d->y0, d->x0, false, true);
        d->x0   = v346;
        d->y0   = v347;
        d->r[0] = sn2_step(d, 0, 1);
        d->r[5] = sn2_step(d, 5, 1);
    }
    {
        int32_t v348 = sn2_s24(SN2_XR(d, d->r[0]));
        int32_t v349 = sn2_s24(SN2_YR(d, d->r[5]));
        d->a    = sn2_mac(d, d->a, d->y0, d->x0, false, true);
        d->x0   = v348;
        d->y0   = v349;
        d->r[0] = sn2_step(d, 0, 1);
        d->r[5] = sn2_step(d, 5, -1);
    }
    {
        d->a    = sn2_mac(d, d->a, d->y0, d->x0, false, true);
        d->r[5] = sn2_step(d, 5, -(int32_t)d->n[5]);
    }
    {
        int32_t v350 = sn2_s24(SN2_XR(d, d->r[0]));
        d->y0   = v350;
        d->r[0] = sn2_step(d, 0, 1);
    }
    {
        int32_t v351 = sn2_s24(SN2_XR(d, d->r[0]));
        int32_t v352 = sn2_s24(SN2_YR(d, d->r[5]));
        d->x1   = v351;
        d->y1   = v352;
        d->r[0] = sn2_step(d, 0, 1);
        d->r[5] = sn2_step(d, 5, 1);
    }
    {
        int32_t v353 = sn2_s24(SN2_XR(d, d->r[0]));
        int32_t v354 = sn2_s24(SN2_YR(d, d->r[5]));
        d->b    = sn2_mac(d, 0, d->y1, d->x1, true, true);
        d->x1   = v353;
        d->y1   = v354;
        d->r[0] = sn2_step(d, 0, 1);
        d->r[5] = sn2_step(d, 5, 1);
    }
    {
        int32_t v355 = sn2_s24(SN2_XR(d, d->r[0]));
        int32_t v356 = sn2_s24(SN2_YR(d, d->r[5]));
        d->b    = sn2_mac(d, d->b, d->y1, d->x1, true, true);
        d->x1   = v355;
        d->y1   = v356;
        d->r[0] = sn2_step(d, 0, 1);
        d->r[5] = sn2_step(d, 5, 1);
    }
    {
        int32_t v357 = sn2_s24(SN2_XR(d, d->r[0]));
        int32_t v358 = sn2_s24(SN2_YR(d, d->r[5]));
        d->b    = sn2_mac(d, d->b, d->y1, d->x1, true, true);
        d->x1   = v357;
        d->y1   = v358;
        d->r[0] = sn2_step(d, 0, 1);
        d->r[5] = sn2_step(d, 5, 1);
    }
    {
        int32_t v359 = sn2_s24(SN2_XR(d, d->r[0]));
        int32_t v360 = sn2_s24(SN2_YR(d, d->r[5]));
        d->b    = sn2_mac(d, d->b, d->y1, d->x1, true, true);
        d->x1   = v359;
        d->y1   = v360;
        d->r[0] = sn2_step(d, 0, 1);
        d->r[5] = sn2_step(d, 5, 1);
    }
    {
        int32_t v361 = sn2_s24(SN2_XR(d, d->r[0]));
        int32_t v362 = sn2_s24(SN2_YR(d, d->r[5]));
        d->b    = sn2_mac(d, d->b, d->y1, d->x1, true, true);
        d->x1   = v361;
        d->y1   = v362;
        d->r[0] = sn2_step(d, 0, 1);
        d->r[5] = sn2_step(d, 5, -1);
    }
    {
        d->b    = sn2_mac(d, d->b, d->y1, d->x1, true, true);
        d->r[5] = sn2_step(d, 5, -(int32_t)d->n[5]);
    }
    {
        int32_t v363 = sn2_s24(SN2_XR(d, sn2_step(d, 0, 0x5)));
        d->n[1] = sn2_w24(v363);
    }
    {
        int32_t v364 = sn2_s24(SN2_XR(d, d->r[0]));
        d->x1   = v364;
        d->r[0] = sn2_step(d, 0, 1);
    }
    {
        d->c = (d->n[1] >> 0) & 1;
    }

    if (!d->c) {
        goto L16_pass;
    }
    {
        d->b = sn2_sub(d, d->b, sn2_ld(d->x1), true);
    }
    {
        d->b = sn2_ext56(-d->b);
        sn2_flags(d, d->b);
        d->v = false;
    }
    {
        if (d->neg) {
            d->b = 0;
        }
    }
    {
        d->x0 = sn2_s24(0x2aaaab);
    }
    {
        int32_t v365 = sn2_lim(d->b);
        d->y1 = v365;
    }
    {
        d->b = sn2_mac(d, 0, d->x0, d->y1, true, true);
    }
    {
        int32_t v366 = d->y0;
        d->n[1] = sn2_w24(v366);
    }
    {
        int32_t v367 = sn2_lim(d->b);
        d->b  = sn2_add(d, d->b, d->a, true);
        d->x1 = v367;
    }
    {
        if (d->neg) {
            d->b = 0;
        }
    }
    {
        d->a = sn2_sub(d, d->a, sn2_ld(d->x1), true);
    }
    {
        if (d->neg) {
            d->a = 0;
        }
    }
    {
        int32_t v368 = sn2_lim(d->b);
        d->x0 = v368;
    }
    {
        int32_t v369 = sn2_lim(d->a);
        d->n[2] = sn2_w24(v369);
    }
    goto L17_pass;
L16_pass:;
    {
        {
            bool c = d->c;
            (void)sn2_sub(d, d->a, 0, true);
            d->c = c;
            d->v = false;
        }
    }
    {
        if (d->neg) {
            d->a = 0;
        }
    }
    {
        d->b = sn2_add(d, d->b, sn2_ld(d->y0), true);
    }
    {
        if (d->neg) {
            d->b = 0;
        }
    }
    {
        int32_t v370 = sn2_lim(d->a);
        d->x0 = v370;
    }
    {
        int32_t v371 = sn2_lim(d->a);
        d->n[2] = sn2_w24(v371);
    }
    {
        int32_t v372 = sn2_s24(SN2_YR(d, 0x2));
        d->a = sn2_ld(v372);
    }
    {
        int32_t v373 = sn2_lim(d->b);

        if (d->a >= d->b) {
            d->b = d->a;
            d->c = false;
        } else {
            d->c = true;
        }
        d->y0 = v373;
    }
    {
        int32_t v374 = sn2_lim(d->b);
        d->n[1] = sn2_w24(v374);
    }
L17_pass:;
    {
        d->b = sn2_mac(d, 0, d->x0, sn2_s24(0x2000), false, true);
    }
    {
        d->a = sn2_mac(d, 0, d->x0, sn2_s24(0x10), false, true);
    }
    {
        int32_t v375 = sn2_lim(d->b);
        d->n[6] = sn2_w24(v375);
    }
    {
        d->a = sn2_add(d, d->a, sn2_ld(sn2_s24(0x9)), true);
    }
    {
        d->b    = 0;
        d->z    = true;
        d->neg  = false;
        d->e    = false;
        d->u    = true;
        d->v    = false;
        d->n[0] = sn2_w24(sn2_s24(0xb));
    }
    {
        int32_t v376 = sn2_s24(SN2_YR(d, 0x0));
        d->y1 = v376;
    }
    {
        int32_t v377 = sn2_s24(SN2_YR(d, sn2_step(d, 6, (int32_t)d->n[6])));
        d->b = (d->b & ~(int64_t)0xFFFFFF) | (int64_t)sn2_w24(v377);
    }
    {
        d->a = sn2_asl(d, d->b, (int)(sn2_w24(sn2_hi(d->a)) & 0x3F), true);
    }
    {
        int32_t v378 = sn2_s24(SN2_XR(d, d->r[0]));
        d->x0   = v378;
        d->r[0] = sn2_step(d, 0, 1);
    }
    {
        int32_t v379 = sn2_lim(d->a);
        d->n[6] = sn2_w24(v379);
    }
    {
        int32_t v380 = d->x0;
        d->b = sn2_ld(v380);
    }
    {
        int32_t v381 = sn2_s24(SN2_XR(d, d->r[0]));
        d->b    = sn2_mac(d, d->b, d->y0, d->x0, true, true);
        d->a    = sn2_ld(v381);
        d->r[0] = sn2_step(d, 0, 1);
    }
    {
        int32_t v382 = sn2_s24(SN2_XR(d, d->r[0]));
        {
            uint32_t o = ((uint32_t)(d->b >> 24) ^ sn2_w24(d->y1)) & 0xFFFFFFu;
            d->b = sn2_set_hi(d->b, o);
            sn2_flags_hi(d, o);
        }
        d->x0   = v382;
        d->r[0] = sn2_step(d, 0, 1);
    }
    {
        int32_t v383 = sn2_s24(SN2_XR(d, d->r[0]));
        d->x1   = v383;
        d->r[0] = sn2_step(d, 0, 1);
    }
    {
        int32_t v384 = d->x0;
        int32_t v385 = sn2_lim(d->b);
        SN2_XW(d, d->r[7], sn2_w24(v384));
        d->y1   = v385;
        d->r[7] = sn2_step(d, 7, 1);
    }
    {
        int32_t v386 = sn2_s24(d->n[6]);
        d->b  = sn2_mac(d, 0, d->y1, d->x1, false, true);
        d->x0 = v386;
    }
    {
        int32_t v387 = sn2_s24(d->n[1]);
        d->x1 = v387;
    }
    {
        int32_t v388 = sn2_lim(d->a);
        d->n[1] = sn2_w24(v388);
    }
    {
        int32_t v389 = sn2_lim(d->b);
        int32_t v390 = sn2_lim(d->a);
        SN2_XW(d, d->r[7], sn2_w24(v389));
        d->y1   = v390;
        d->r[7] = sn2_step(d, 7, 1);
    }
    {
        int32_t v391 = sn2_s24(d->r[0]);
        SN2_XW(d, d->r[7], sn2_w24(v391));
        d->r[7] = sn2_step(d, 7, 1);
    }
    {
        int32_t v392 = sn2_s24(SN2_YR(d, 0x5));
        d->a  = sn2_mac(d, d->a, d->x0, d->y1, true, true);
        d->y1 = v392;
    }
    {
        int32_t v393 = sn2_s24(d->n[1]);
        d->b  = sn2_mac(d, 0, d->y1, d->x1, false, true);
        d->y1 = v393;
    }
    {
        int32_t v394 = sn2_lim(d->a);
        d->a    = sn2_mac(d, 0, d->x1, d->x0, false, true);
        SN2_XW(d, d->r[7], sn2_w24(v394));
        d->r[7] = sn2_step(d, 7, 1);
    }
    {
        int32_t v395 = sn2_lim(d->b);
        d->a  = sn2_asr(d, d->a, 1, true);
        d->x1 = v395;
    }
    {
        int32_t v396 = d->x0;
        SN2_XW(d, d->r[7], sn2_w24(v396));
        d->r[7] = sn2_step(d, 7, 1);
    }
    {
        int32_t v397 = sn2_lim(d->a);
        d->x0 = v397;
    }
    {
        int32_t v398 = sn2_s24(SN2_YR(d, 0x5));
        d->b  = sn2_mac(d, d->b, d->x1, d->x0, true, true);
        d->x1 = v398;
    }
    {
        int32_t v399 = sn2_s24(d->n[2]);
        d->x0 = v399;
    }
    {
        int32_t v400 = sn2_lim(d->b);
        SN2_XW(d, d->r[7], sn2_w24(v400));
        d->r[7] = sn2_step(d, 7, 1);
    }
    {
        d->b = sn2_mac(d, 0, d->x0, sn2_s24(0x2000), false, true);
    }
    {
        d->a = sn2_mac(d, 0, d->x0, sn2_s24(0x10), false, true);
    }
    {
        int32_t v401 = sn2_lim(d->b);
        d->n[6] = sn2_w24(v401);
    }
    {
        d->a = sn2_add(d, d->a, sn2_ld(sn2_s24(0x9)), true);
    }
    {
        d->b   = 0;
        d->z   = true;
        d->neg = false;
        d->e   = false;
        d->u   = true;
        d->v   = false;
    }
    {
        d->r[2] = sn2_step(d, 2, 1);
    }
    {
        int32_t v402 = sn2_s24(SN2_YR(d, sn2_step(d, 6, (int32_t)d->n[6])));
        d->b = (d->b & ~(int64_t)0xFFFFFF) | (int64_t)sn2_w24(v402);
    }
    {
        d->a = sn2_asl(d, d->b, (int)(sn2_w24(sn2_hi(d->a)) & 0x3F), true);
    }
    {
        int32_t v403 = d->y1;
        d->b = sn2_ld(v403);
    }
    {
        int32_t v404 = sn2_lim(d->a);
        d->a  = sn2_mac(d, 0, d->x1, d->y0, false, true);
        d->x0 = v404;
    }
    {
        d->b    = sn2_mac(d, d->b, d->x0, d->y1, true, true);
        d->r[0] = sn2_step(d, 0, (int32_t)d->n[0]);
    }
    {
        int32_t v405 = sn2_lim(d->b);
        d->b    = sn2_mac(d, 0, d->y0, d->x0, false, true);
        SN2_XW(d, d->r[7], sn2_w24(v405));
        d->r[7] = sn2_step(d, 7, 1);
    }
    {
        int32_t v406 = sn2_lim(d->a);
        d->b  = sn2_asr(d, d->b, 1, true);
        d->x1 = v406;
    }
    {
        int32_t v407 = d->x0;
        SN2_XW(d, d->r[7], sn2_w24(v407));
        d->r[7] = sn2_step(d, 7, 1);
    }
    {
        int32_t v408 = sn2_lim(d->b);
        d->x0 = v408;
    }
    {
        int32_t v409 = sn2_s24(SN2_YR(d, d->r[5]));
        d->a = sn2_mac(d, d->a, d->x1, d->x0, true, true);
        d->b = sn2_ld(v409);
    }
    d->r[4] = sn2_step(d, 4, 0x4);
    {
        int32_t v410 = sn2_lim(d->a);
        SN2_XW(d, d->r[7], sn2_w24(v410));
        d->r[7] = sn2_step(d, 7, 1);
    }
    {
        int32_t v411 = sn2_s24(SN2_XR(d, d->r[7]));
        d->x0   = v411;
        d->r[7] = sn2_step(d, 7, 1);
    }
    {
        d->b = sn2_sub(d, d->b, sn2_ld(d->x0), true);
    }
    {
        d->b = sn2_asr(d, d->b, 2, true);
    }
    {
        int32_t v412 = sn2_lim(d->b);
        SN2_XW(d, d->r[7], sn2_w24(v412));
        d->r[7] = sn2_step(d, 7, 1);
    }
    d->r[7] = sn2_step(d, 4, 0x7);
    {
        uint32_t   wi = d->r[4];
        uint32_t   wv = SN2_YR(d, wi);
        uint32_t * w  = &wv;
        d->r[4] = sn2_step(d, 4, 1);

        if (!((*w >> 0) & 1)) {
            goto L18_pass;
        }
    }
    d->r[0] = sn2_step(d, 0, 0x19);
    d->r[4] = sn2_step(d, 4, 0x6);
    d->r[7] = sn2_step(d, 7, 0x3);
    goto L19_pass;
L18_pass:;
    {
        int32_t v413 = sn2_s24(d->r[4]);
        d->r[1] = sn2_w24(v413);
    }
    {
        int32_t v414 = sn2_s24(SN2_XR(d, d->r[0]));
        d->y0   = v414;
        d->r[0] = sn2_step(d, 0, 1);
    }
    {
        int32_t v415 = sn2_s24(SN2_XR(d, d->r[0]));
        int32_t v416 = sn2_s24(SN2_YR(d, d->r[7]));
        d->x0   = v415;
        d->a    = sn2_ld(v416);
        d->r[0] = sn2_step(d, 0, 1);
    }
    {
        int32_t v417 = sn2_s24(SN2_XR(d, d->r[0]));
        d->y1   = v417;
        d->r[0] = sn2_step(d, 0, 1);
    }
    {
        int32_t v418 = sn2_s24(SN2_XR(d, d->r[0]));
        int32_t v419 = sn2_s24(SN2_YR(d, d->r[4]));
        d->x1   = v418;
        d->b    = sn2_ld(v419);
        d->r[0] = sn2_step(d, 0, 1);
        d->r[4] = sn2_step(d, 4, 1);
    }
    {
        uint32_t   wi = d->r[1];
        uint32_t   wv = SN2_YR(d, wi);
        uint32_t * w  = &wv;

        if (!((*w >> 5) & 1)) {
            goto L20_pass;
        }
    }
    {
        uint32_t   wi = d->r[1];
        uint32_t   wv = SN2_YR(d, wi);
        uint32_t * w  = &wv;

        if (!((*w >> 2) & 1)) {
            goto L21_pass;
        }
    }
    {
        d->a    = sn2_mac(d, d->a, d->y0, d->x0, false, true);
        d->r[0] = sn2_step(d, 0, 1);
    }
    {
        (void)sn2_sub(d, d->a, sn2_ld(d->y0), true);
        d->r[0] = sn2_step(d, 0, 1);
    }

    if (d->neg) {
        goto L22_pass;
    }
    {
        uint32_t   wi = d->r[1];
        uint32_t   wv = SN2_YR(d, wi);
        uint32_t * w  = &wv;
        d->c = (*w >> 2) & 1;
        *w  &= ~(1u << 2);
        SN2_YW(d, wi, wv);
    }
    {
        int32_t v420 = d->y0;
        d->a = sn2_ld(v420);
    }
L22_pass:;
    {
        d->r[4] = sn2_step(d, 4, 1);
    }
    {
        int32_t v421 = sn2_s24(SN2_XR(d, d->r[0]));
        int32_t v422 = sn2_lim(d->a);
        d->x0   = v421;
        SN2_YW(d, d->r[7], sn2_w24(v422));
        d->r[0] = sn2_step(d, 0, 1);
        d->r[7] = sn2_step(d, 7, 1);
    }
    goto L23_pass;
L20_pass:;
    {
        int32_t v423 = sn2_s24(SN2_XR(d, d->r[0]));
        d->n[1] = sn2_w24(v423);
        d->r[0] = sn2_step(d, 0, 1);
    }
    {
        int32_t v424 = sn2_s24(SN2_XR(d, d->r[0]));
        int32_t v425 = sn2_s24(SN2_YR(d, d->r[4]));
        d->x0   = v424;
        d->b    = sn2_ld(v425);
        d->r[0] = sn2_step(d, 0, 1);
    }
    {
        int32_t v426 = sn2_s24(SN2_XR(d, d->r[0]));
        d->b    = sn2_add(d, d->b, sn2_ld(d->x0), true);
        d->x0   = v426;
        d->r[0] = sn2_step(d, 0, 1);
    }
    {
        d->b = sn2_asl(d, d->b, 1, true);
    }
    {
        if (!d->z) {
            d->b = sn2_asr(d, d->b, 1, false);
        }
    }

    if (d->e) {
        goto L24_pass;
    }
    {
        int32_t v427 = sn2_lim(d->b);
        d->b    = sn2_ld(d->x0);
        SN2_YW(d, d->r[4], sn2_w24(v427));
        d->r[4] = sn2_step(d, 4, 1);
    }
    {
        int32_t v428 = sn2_lim(d->a);
        {
            bool c = d->c;
            (void)sn2_sub(d, d->b, 0, true);
            d->c = c;
            d->v = false;
        }
        d->x1 = v428;
    }

    if (d->z) {
        goto L25_pass;
    }
    {
        if (d->neg) {
            d->a = sn2_mac(d, d->a, d->x1, d->x0, false, false);
        }
    }
    {
        if (!d->neg) {
            d->a = sn2_mac(d, d->a, d->y0, d->x0, false, false);
        }
    }
    {
        (void)sn2_sub(d, d->a, sn2_ld(d->y0), true);
    }
    {
        if (!d->neg) {
            d->a = sn2_ld(d->y0);
        }
    }
    goto L26_pass;
L25_pass:;
    {
        d->x0 = sn2_s24(0x10000);
    }
    {
        int32_t v429 = sn2_s24(d->n[1]);
        d->b  = sn2_mac(d, 0, d->y0, d->x0, false, true);
        d->x0 = v429;
    }
    {
        d->a  = sn2_mac(d, d->a, d->y0, d->x0, true, true);
        d->x1 = sn2_s24(0x40000);
    }
    {
        int32_t v430 = d->y1;
        {
            int64_t m1 = (d->a < 0) ? -d->a : d->a;
            int64_t s0 = d->b;
            int64_t m2 = (s0 < 0) ? -s0 : s0;
            int64_t r  = sn2_ext56(m1 - m2);
            d->c = (uint64_t)m2 > (uint64_t)m1;
            d->v = false;
            sn2_flags(d, r);
        }
        d->b = sn2_ld(v430);
    }
    {
        if (!(d->z || (d->neg != d->v))) {
            d->b = sn2_ld(d->x1);
        }
    }
    {
        int32_t v431 = sn2_lim(d->a);
        d->x1 = v431;
    }
    {
        int32_t v432 = sn2_lim(d->b);
        d->y1 = v432;
    }
    {
        d->a = sn2_mac(d, d->a, d->y1, d->x1, true, true);
    }
    {
        d->a = sn2_mac(d, d->a, d->y0, d->x0, false, true);
    }
    goto L26_pass;
L24_pass:;
    {
        int32_t v433 = sn2_lim(d->a);
        d->y1 = v433;
    }
    {
        d->a = sn2_mac(d, d->a, d->y1, d->x1, true, true);
    }
    {
        d->r[4] = sn2_step(d, 4, 1);
    }
    {
        int32_t v434 = sn2_lim(d->a);
        SN2_YW(d, d->r[7], sn2_w24(v434));
        d->r[7] = sn2_step(d, 7, 1);
    }
    goto L23_pass;
L21_pass:;
    {
        int32_t v435 = sn2_s24(SN2_XR(d, d->r[0]));
        d->x0   = v435;
        d->r[0] = sn2_step(d, 0, 1);
    }
    {
        d->a = sn2_mac(d, d->a, d->y0, d->x0, true, true);
    }
    {
        int32_t v436 = sn2_lim(d->a);
        d->x1 = v436;
    }
    {
        d->a = sn2_mac(d, d->a, d->y1, d->x1, true, true);
    }
    {
        d->y1 = sn2_s24(0x10000);
    }
    {
        int32_t v437 = sn2_lim(d->a);
        d->a = sn2_mac(d, d->a, d->y0, d->x0, false, true);
        d->b = sn2_ld(v437);
    }
    {
        int32_t v438 = sn2_s24(SN2_XR(d, d->r[0]));
        d->b    = sn2_mac(d, d->b, d->y1, d->y0, true, true);
        d->x1   = v438;
        d->r[0] = sn2_step(d, 0, 1);
    }
    {
        int32_t v439 = sn2_s24(SN2_XR(d, d->r[0]));
        int32_t v440 = sn2_s24(SN2_YR(d, d->r[4]));
        d->x1   = v439;
        d->b    = sn2_ld(v440);
        d->r[0] = sn2_step(d, 0, 1);
    }

    if (!d->neg) {
        goto L27_pass;
    }
    {
        d->b = sn2_add(d, d->b, sn2_ld(sn2_s24(0x1)), true);
    }

    if (!d->neg) {
        goto L28_pass;
    }
    {
        uint32_t   wi = d->r[1];
        uint32_t   wv = SN2_YR(d, wi);
        uint32_t * w  = &wv;
        d->c = (*w >> 2) & 1;
        *w  |= (1u << 2);
        SN2_YW(d, wi, wv);
    }
    goto L27_pass;
L28_pass:;
    {
        uint32_t   wi = d->r[1];
        uint32_t   wv = SN2_YR(d, wi);
        uint32_t * w  = &wv;
        d->c = (*w >> 5) & 1;
        *w  &= ~(1u << 5);
        SN2_YW(d, wi, wv);
    }
L27_pass:;
    {
        int32_t v441 = sn2_lim(d->b);
        SN2_YW(d, d->r[4], sn2_w24(v441));
        d->r[4] = sn2_step(d, 4, 1);
    }
L26_pass:;
    {
        int32_t v442 = sn2_lim(d->a);
        SN2_YW(d, d->r[7], sn2_w24(v442));
        d->r[7] = sn2_step(d, 7, 1);
    }
L23_pass:;
    {
        int32_t v443 = sn2_s24(SN2_XR(d, d->r[0]));
        int32_t v444 = sn2_s24(SN2_YR(d, d->r[4]));
        d->a    = 0;
        d->z    = true;
        d->neg  = false;
        d->e    = false;
        d->u    = true;
        d->v    = false;
        d->x0   = v443;
        d->b    = sn2_ld(v444);
        d->r[0] = sn2_step(d, 0, 1);
    }
    {
        int32_t v445 = sn2_s24(SN2_XR(d, 0x5));
        d->y0 = v445;
    }
    {
        int32_t v446 = sn2_s24(SN2_XR(d, d->r[0]));
        d->b    = sn2_mac(d, d->b, d->y0, d->x0, false, true);
        d->x0   = v446;
        d->r[0] = sn2_step(d, 0, 1);
    }
    {
        int32_t v447 = sn2_s24(SN2_XR(d, d->r[0]));
        d->b    = sn2_add(d, d->b, sn2_ld(d->x0), true);
        d->y0   = v447;
        d->r[0] = sn2_step(d, 0, 1);
    }
    {
        if (d->e) {
            d->a = sn2_add(d, d->a, sn2_ld(d->y0), false);
        }
    }
    {
        int32_t v448 = sn2_s24(SN2_XR(d, d->r[0]));
        int32_t v449 = sn2_lim(d->b);
        d->x0   = v448;
        SN2_YW(d, d->r[4], sn2_w24(v449));
        d->r[0] = sn2_step(d, 0, 1);
        d->r[4] = sn2_step(d, 4, 1);
    }
    {
        int32_t v450 = sn2_lim(d->a);
        d->b  = sn2_ld(d->y0);
        d->y0 = v450;
    }
    {
        int32_t v451 = sn2_s24(SN2_XR(d, d->r[0]));
        d->y1   = v451;
        d->r[0] = sn2_step(d, 0, 1);
    }
    {
        int32_t v452 = sn2_s24(SN2_XR(d, d->r[0]));
        int32_t v453 = sn2_s24(SN2_YR(d, d->r[7]));
        d->x1   = v452;
        d->a    = sn2_ld(v453);
        d->r[0] = sn2_step(d, 0, 1);
    }
    {
        uint32_t   wi = d->r[1];
        uint32_t   wv = SN2_YR(d, wi);
        uint32_t * w  = &wv;

        if (!((*w >> 4) & 1)) {
            goto L29_pass;
        }
    }
    {
        uint32_t   wi = d->r[1];
        uint32_t   wv = SN2_YR(d, wi);
        uint32_t * w  = &wv;

        if (!((*w >> 1) & 1)) {
            goto L30_pass;
        }
    }
    {
        d->a    = sn2_mac(d, d->a, d->y0, d->x0, false, true);
        d->r[0] = sn2_step(d, 0, 1);
    }
    {
        (void)sn2_sub(d, d->a, d->b, true);
        d->r[0] = sn2_step(d, 0, 1);
    }

    if (d->neg) {
        goto L31_pass;
    }
    {
        uint32_t   wi = d->r[1];
        uint32_t   wv = SN2_YR(d, wi);
        uint32_t * w  = &wv;
        d->c = (*w >> 1) & 1;
        *w  &= ~(1u << 1);
        SN2_YW(d, wi, wv);
    }
    {
        int32_t v454 = d->y0;
        d->a = sn2_ld(v454);
    }
L31_pass:;
    {
        d->r[4] = sn2_step(d, 4, 1);
    }
    {
        int32_t v455 = sn2_s24(SN2_XR(d, d->r[0]));
        int32_t v456 = sn2_lim(d->a);
        d->x0   = v455;
        SN2_YW(d, d->r[7], sn2_w24(v456));
        d->r[0] = sn2_step(d, 0, 1);
        d->r[7] = sn2_step(d, 7, 1);
    }
    goto L32_pass;
L29_pass:;
    {
        int32_t v457 = sn2_s24(SN2_XR(d, d->r[0]));
        d->n[1] = sn2_w24(v457);
        d->r[0] = sn2_step(d, 0, 1);
    }
    {
        int32_t v458 = sn2_s24(SN2_XR(d, d->r[0]));
        int32_t v459 = sn2_s24(SN2_YR(d, d->r[4]));
        d->x0   = v458;
        d->b    = sn2_ld(v459);
        d->r[0] = sn2_step(d, 0, 1);
    }
    {
        int32_t v460 = sn2_s24(SN2_XR(d, d->r[0]));
        d->b    = sn2_add(d, d->b, sn2_ld(d->x0), true);
        d->x0   = v460;
        d->r[0] = sn2_step(d, 0, 1);
    }
    {
        d->b = sn2_asl(d, d->b, 1, true);
    }
    {
        if (!d->z) {
            d->b = sn2_asr(d, d->b, 1, false);
        }
    }

    if (d->e) {
        goto L33_pass;
    }
    {
        int32_t v461 = sn2_lim(d->b);
        d->b    = sn2_ld(d->x0);
        SN2_YW(d, d->r[4], sn2_w24(v461));
        d->r[4] = sn2_step(d, 4, 1);
    }
    {
        int32_t v462 = sn2_lim(d->a);
        {
            bool c = d->c;
            (void)sn2_sub(d, d->b, 0, true);
            d->c = c;
            d->v = false;
        }
        d->x1 = v462;
    }

    if (d->z) {
        goto L34_pass;
    }
    {
        if (d->neg) {
            d->a = sn2_mac(d, d->a, d->x1, d->x0, false, false);
        }
    }
    {
        if (!d->neg) {
            d->a = sn2_mac(d, d->a, d->y0, d->x0, false, false);
        }
    }
    {
        (void)sn2_sub(d, d->a, sn2_ld(d->y0), true);
    }
    {
        if (!d->neg) {
            d->a = sn2_ld(d->y0);
        }
    }
    goto L35_pass;
L34_pass:;
    {
        d->x0 = sn2_s24(0x10000);
    }
    {
        int32_t v463 = sn2_s24(d->n[1]);
        d->b  = sn2_mac(d, 0, d->y0, d->x0, false, true);
        d->x0 = v463;
    }
    {
        d->a  = sn2_mac(d, d->a, d->y0, d->x0, true, true);
        d->x1 = sn2_s24(0x40000);
    }
    {
        int32_t v464 = d->y1;
        {
            int64_t m1 = (d->a < 0) ? -d->a : d->a;
            int64_t s0 = d->b;
            int64_t m2 = (s0 < 0) ? -s0 : s0;
            int64_t r  = sn2_ext56(m1 - m2);
            d->c = (uint64_t)m2 > (uint64_t)m1;
            d->v = false;
            sn2_flags(d, r);
        }
        d->b = sn2_ld(v464);
    }
    {
        if (!(d->z || (d->neg != d->v))) {
            d->b = sn2_ld(d->x1);
        }
    }
    {
        int32_t v465 = sn2_lim(d->a);
        d->x1 = v465;
    }
    {
        int32_t v466 = sn2_lim(d->b);
        d->y1 = v466;
    }
    {
        d->a = sn2_mac(d, d->a, d->y1, d->x1, true, true);
    }
    {
        d->a = sn2_mac(d, d->a, d->y0, d->x0, false, true);
    }
    goto L35_pass;
L33_pass:;
    {
        int32_t v467 = sn2_lim(d->a);
        d->y1 = v467;
    }
    {
        d->a = sn2_mac(d, d->a, d->y1, d->x1, true, true);
    }
    {
        d->r[4] = sn2_step(d, 4, 1);
    }
    {
        int32_t v468 = sn2_lim(d->a);
        SN2_YW(d, d->r[7], sn2_w24(v468));
        d->r[7] = sn2_step(d, 7, 1);
    }
    goto L32_pass;
L30_pass:;
    {
        int32_t v469 = sn2_s24(SN2_XR(d, d->r[0]));
        d->x0   = v469;
        d->r[0] = sn2_step(d, 0, 1);
    }
    {
        d->a = sn2_mac(d, d->a, d->y0, d->x0, true, true);
    }
    {
        int32_t v470 = sn2_lim(d->a);
        d->x1 = v470;
    }
    {
        d->a = sn2_mac(d, d->a, d->y1, d->x1, true, true);
    }
    {
        d->y1 = sn2_s24(0x10000);
    }
    {
        int32_t v471 = sn2_lim(d->a);
        d->a = sn2_mac(d, d->a, d->y0, d->x0, false, true);
        d->b = sn2_ld(v471);
    }
    {
        int32_t v472 = sn2_s24(SN2_XR(d, d->r[0]));
        d->b    = sn2_mac(d, d->b, d->y1, d->y0, true, true);
        d->x1   = v472;
        d->r[0] = sn2_step(d, 0, 1);
    }
    {
        int32_t v473 = sn2_s24(SN2_XR(d, d->r[0]));
        int32_t v474 = sn2_s24(SN2_YR(d, d->r[4]));
        d->x1   = v473;
        d->b    = sn2_ld(v474);
        d->r[0] = sn2_step(d, 0, 1);
    }

    if (!d->neg) {
        goto L36_pass;
    }
    {
        d->b = sn2_add(d, d->b, sn2_ld(sn2_s24(0x1)), true);
    }

    if (!d->neg) {
        goto L37_pass;
    }
    {
        uint32_t   wi = d->r[1];
        uint32_t   wv = SN2_YR(d, wi);
        uint32_t * w  = &wv;
        d->c = (*w >> 1) & 1;
        *w  |= (1u << 1);
        SN2_YW(d, wi, wv);
    }
    goto L36_pass;
L37_pass:;
    {
        uint32_t   wi = d->r[1];
        uint32_t   wv = SN2_YR(d, wi);
        uint32_t * w  = &wv;
        d->c = (*w >> 4) & 1;
        *w  &= ~(1u << 4);
        SN2_YW(d, wi, wv);
    }
L36_pass:;
    {
        int32_t v475 = sn2_lim(d->b);
        SN2_YW(d, d->r[4], sn2_w24(v475));
        d->r[4] = sn2_step(d, 4, 1);
    }
L35_pass:;
    {
        int32_t v476 = sn2_lim(d->a);
        SN2_YW(d, d->r[7], sn2_w24(v476));
        d->r[7] = sn2_step(d, 7, 1);
    }
L32_pass:;
    {
        int32_t v477 = sn2_s24(SN2_XR(d, d->r[0]));
        int32_t v478 = sn2_s24(SN2_YR(d, d->r[4]));
        d->a    = 0;
        d->z    = true;
        d->neg  = false;
        d->e    = false;
        d->u    = true;
        d->v    = false;
        d->x0   = v477;
        d->b    = sn2_ld(v478);
        d->r[0] = sn2_step(d, 0, 1);
    }
    {
        int32_t v479 = sn2_s24(SN2_XR(d, 0x5));
        d->y0 = v479;
    }
    {
        int32_t v480 = sn2_s24(SN2_XR(d, d->r[0]));
        d->b    = sn2_mac(d, d->b, d->y0, d->x0, false, true);
        d->x0   = v480;
        d->r[0] = sn2_step(d, 0, 1);
    }
    {
        int32_t v481 = sn2_s24(SN2_XR(d, d->r[0]));
        d->b    = sn2_add(d, d->b, sn2_ld(d->x0), true);
        d->y0   = v481;
        d->r[0] = sn2_step(d, 0, 1);
    }
    {
        if (d->e) {
            d->a = sn2_add(d, d->a, sn2_ld(d->y0), false);
        }
    }
    {
        int32_t v482 = sn2_s24(SN2_XR(d, d->r[0]));
        int32_t v483 = sn2_lim(d->b);
        d->x0   = v482;
        SN2_YW(d, d->r[4], sn2_w24(v483));
        d->r[0] = sn2_step(d, 0, 1);
        d->r[4] = sn2_step(d, 4, 1);
    }
    {
        int32_t v484 = sn2_lim(d->a);
        d->b  = sn2_ld(d->y0);
        d->y0 = v484;
    }
    {
        int32_t v485 = sn2_s24(SN2_XR(d, d->r[0]));
        d->y1   = v485;
        d->r[0] = sn2_step(d, 0, 1);
    }
    {
        int32_t v486 = sn2_s24(SN2_XR(d, d->r[0]));
        int32_t v487 = sn2_s24(SN2_YR(d, d->r[7]));
        d->x1   = v486;
        d->a    = sn2_ld(v487);
        d->r[0] = sn2_step(d, 0, 1);
    }
    {
        uint32_t   wi = d->r[1];
        uint32_t   wv = SN2_YR(d, wi);
        uint32_t * w  = &wv;

        if (!((*w >> 3) & 1)) {
            goto L38_pass;
        }
    }
    {
        uint32_t   wi = d->r[1];
        uint32_t   wv = SN2_YR(d, wi);
        uint32_t * w  = &wv;

        if (!((*w >> 0) & 1)) {
            goto L39_pass;
        }
    }
    {
        d->a    = sn2_mac(d, d->a, d->y0, d->x0, false, true);
        d->r[0] = sn2_step(d, 0, 1);
    }
    {
        (void)sn2_sub(d, d->a, d->b, true);
        d->r[0] = sn2_step(d, 0, 1);
    }

    if (d->neg) {
        goto L40_pass;
    }
    {
        uint32_t   wi = d->r[1];
        uint32_t   wv = SN2_YR(d, wi);
        uint32_t * w  = &wv;
        d->c = (*w >> 0) & 1;
        *w  &= ~(1u << 0);
        SN2_YW(d, wi, wv);
    }
    {
        int32_t v488 = d->y0;
        d->a = sn2_ld(v488);
    }
L40_pass:;
    {
        d->r[4] = sn2_step(d, 4, 1);
    }
    {
        int32_t v489 = sn2_s24(SN2_XR(d, d->r[0]));
        int32_t v490 = sn2_lim(d->a);
        d->x0   = v489;
        SN2_YW(d, d->r[7], sn2_w24(v490));
        d->r[0] = sn2_step(d, 0, 1);
        d->r[7] = sn2_step(d, 7, 1);
    }
    goto L19_pass;
L38_pass:;
    {
        int32_t v491 = sn2_s24(SN2_XR(d, d->r[0]));
        d->n[1] = sn2_w24(v491);
        d->r[0] = sn2_step(d, 0, 1);
    }
    {
        int32_t v492 = sn2_s24(SN2_XR(d, d->r[0]));
        int32_t v493 = sn2_s24(SN2_YR(d, d->r[4]));
        d->x0   = v492;
        d->b    = sn2_ld(v493);
        d->r[0] = sn2_step(d, 0, 1);
    }
    {
        int32_t v494 = sn2_s24(SN2_XR(d, d->r[0]));
        d->b    = sn2_add(d, d->b, sn2_ld(d->x0), true);
        d->x0   = v494;
        d->r[0] = sn2_step(d, 0, 1);
    }
    {
        d->b = sn2_asl(d, d->b, 1, true);
    }
    {
        if (!d->z) {
            d->b = sn2_asr(d, d->b, 1, false);
        }
    }

    if (d->e) {
        goto L41_pass;
    }
    {
        int32_t v495 = sn2_lim(d->b);
        d->b    = sn2_ld(d->x0);
        SN2_YW(d, d->r[4], sn2_w24(v495));
        d->r[4] = sn2_step(d, 4, 1);
    }
    {
        int32_t v496 = sn2_lim(d->a);
        {
            bool c = d->c;
            (void)sn2_sub(d, d->b, 0, true);
            d->c = c;
            d->v = false;
        }
        d->x1 = v496;
    }

    if (d->z) {
        goto L42_pass;
    }
    {
        if (d->neg) {
            d->a = sn2_mac(d, d->a, d->x1, d->x0, false, false);
        }
    }
    {
        if (!d->neg) {
            d->a = sn2_mac(d, d->a, d->y0, d->x0, false, false);
        }
    }
    {
        (void)sn2_sub(d, d->a, sn2_ld(d->y0), true);
    }
    {
        if (!d->neg) {
            d->a = sn2_ld(d->y0);
        }
    }
    goto L43_pass;
L42_pass:;
    {
        d->x0 = sn2_s24(0x10000);
    }
    {
        int32_t v497 = sn2_s24(d->n[1]);
        d->b  = sn2_mac(d, 0, d->y0, d->x0, false, true);
        d->x0 = v497;
    }
    {
        d->a  = sn2_mac(d, d->a, d->y0, d->x0, true, true);
        d->x1 = sn2_s24(0x40000);
    }
    {
        int32_t v498 = d->y1;
        {
            int64_t m1 = (d->a < 0) ? -d->a : d->a;
            int64_t s0 = d->b;
            int64_t m2 = (s0 < 0) ? -s0 : s0;
            int64_t r  = sn2_ext56(m1 - m2);
            d->c = (uint64_t)m2 > (uint64_t)m1;
            d->v = false;
            sn2_flags(d, r);
        }
        d->b = sn2_ld(v498);
    }
    {
        if (!(d->z || (d->neg != d->v))) {
            d->b = sn2_ld(d->x1);
        }
    }
    {
        int32_t v499 = sn2_lim(d->a);
        d->x1 = v499;
    }
    {
        int32_t v500 = sn2_lim(d->b);
        d->y1 = v500;
    }
    {
        d->a = sn2_mac(d, d->a, d->y1, d->x1, true, true);
    }
    {
        d->a = sn2_mac(d, d->a, d->y0, d->x0, false, true);
    }
    goto L43_pass;
L41_pass:;
    {
        int32_t v501 = sn2_lim(d->a);
        d->y1 = v501;
    }
    {
        d->a = sn2_mac(d, d->a, d->y1, d->x1, true, true);
    }
    {
        d->r[4] = sn2_step(d, 4, 1);
    }
    {
        int32_t v502 = sn2_lim(d->a);
        SN2_YW(d, d->r[7], sn2_w24(v502));
        d->r[7] = sn2_step(d, 7, 1);
    }
    goto L19_pass;
L39_pass:;
    {
        int32_t v503 = sn2_s24(SN2_XR(d, d->r[0]));
        d->x0   = v503;
        d->r[0] = sn2_step(d, 0, 1);
    }
    {
        d->a = sn2_mac(d, d->a, d->y0, d->x0, true, true);
    }
    {
        int32_t v504 = sn2_lim(d->a);
        d->x1 = v504;
    }
    {
        d->a = sn2_mac(d, d->a, d->y1, d->x1, true, true);
    }
    {
        d->y1 = sn2_s24(0x10000);
    }
    {
        int32_t v505 = sn2_lim(d->a);
        d->a = sn2_mac(d, d->a, d->y0, d->x0, false, true);
        d->b = sn2_ld(v505);
    }
    {
        int32_t v506 = sn2_s24(SN2_XR(d, d->r[0]));
        d->b    = sn2_mac(d, d->b, d->y1, d->y0, true, true);
        d->x1   = v506;
        d->r[0] = sn2_step(d, 0, 1);
    }
    {
        int32_t v507 = sn2_s24(SN2_XR(d, d->r[0]));
        int32_t v508 = sn2_s24(SN2_YR(d, d->r[4]));
        d->x1   = v507;
        d->b    = sn2_ld(v508);
        d->r[0] = sn2_step(d, 0, 1);
    }

    if (!d->neg) {
        goto L44_pass;
    }
    {
        d->b = sn2_add(d, d->b, sn2_ld(sn2_s24(0x1)), true);
    }

    if (!d->neg) {
        goto L45_pass;
    }
    {
        uint32_t   wi = d->r[1];
        uint32_t   wv = SN2_YR(d, wi);
        uint32_t * w  = &wv;
        d->c = (*w >> 0) & 1;
        *w  |= (1u << 0);
        SN2_YW(d, wi, wv);
    }
    goto L44_pass;
L45_pass:;
    {
        uint32_t   wi = d->r[1];
        uint32_t   wv = SN2_YR(d, wi);
        uint32_t * w  = &wv;
        d->c = (*w >> 3) & 1;
        *w  &= ~(1u << 3);
        SN2_YW(d, wi, wv);
    }
L44_pass:;
    {
        int32_t v509 = sn2_lim(d->b);
        SN2_YW(d, d->r[4], sn2_w24(v509));
        d->r[4] = sn2_step(d, 4, 1);
    }
L43_pass:;
    {
        int32_t v510 = sn2_lim(d->a);
        SN2_YW(d, d->r[7], sn2_w24(v510));
        d->r[7] = sn2_step(d, 7, 1);
    }
L19_pass:;
    {
        int32_t v511 = sn2_s24(SN2_XR(d, d->r[0]));
        int32_t v512 = sn2_s24(SN2_YR(d, d->r[4]));
        d->x0   = v511;
        d->y0   = v512;
        d->r[0] = sn2_step(d, 0, 1);
        d->r[4] = sn2_step(d, 4, 1);
    }
    {
        int32_t v513 = sn2_s24(SN2_YR(d, d->r[4]));
        d->a    = sn2_mac(d, 0, d->y0, d->x0, false, true);
        d->x0   = v513;
        d->r[4] = sn2_step(d, 4, 1);
    }
    {
        int32_t v514 = sn2_s24(SN2_YR(d, d->r[4]));
        d->x1   = v514;
        d->r[4] = sn2_step(d, 4, 1);
    }
    {
        int32_t v515 = sn2_lim(d->a);
        SN2_YW(d, d->r[5], sn2_w24(v515));
        d->r[5] = sn2_step(d, 5, 1);
    }
    {
        int32_t v516 = d->x0;
        SN2_YW(d, d->r[5], sn2_w24(v516));
        d->r[5] = sn2_step(d, 5, 1);
    }
    {
        int32_t v517 = d->x1;
        SN2_YW(d, d->r[5], sn2_w24(v517));
        d->r[5] = sn2_step(d, 5, 1);
    }
    {
        int32_t v518 = sn2_s24(SN2_XR(d, 0x5));
        d->y0 = v518;
    }
    {
        int32_t v519 = sn2_s24(SN2_XR(d, d->r[0]));
        int32_t v520 = sn2_s24(SN2_YR(d, d->r[5]));
        d->x0   = v519;
        d->y1   = v520;
        d->r[0] = sn2_step(d, 0, 1);
        d->r[5] = sn2_step(d, 5, 1);
    }
    {
        int32_t v521 = sn2_s24(SN2_XR(d, d->r[0]));
        d->b    = sn2_mac(d, 0, d->y0, d->x0, false, true);
        d->x0   = v521;
        d->r[0] = sn2_step(d, 0, 1);
    }
    {
        int32_t v522 = sn2_s24(SN2_XR(d, d->r[0]));
        d->b    = sn2_add(d, d->b, sn2_ld(d->x0), true);
        d->x0   = v522;
        d->r[0] = sn2_step(d, 0, 1);
    }
    {
        int32_t v523 = sn2_s24(SN2_XR(d, d->r[0]));
        d->a    = sn2_mac(d, 0, d->y0, d->x0, false, true);
        d->x0   = v523;
        d->r[0] = sn2_step(d, 0, 1);
    }
    {
        int32_t v524 = sn2_s24(SN2_YR(d, d->r[4]));
        d->a    = sn2_add(d, d->a, sn2_ld(d->x0), true);
        d->y0   = v524;
        d->r[4] = sn2_step(d, 4, 1);
    }
    {
        int32_t v525 = sn2_lim(d->b);
        {
            bool c = d->c;
            (void)sn2_sub(d, d->b, 0, true);
            d->c = c;
            d->v = false;
        }
        d->x0 = v525;
    }
    {
        int32_t v526 = sn2_lim(d->a);
        d->b  = sn2_ld(d->y0);
        d->y1 = v526;
    }
    {
        if (d->neg) {
            d->b = sn2_mac(d, d->b, d->y0, d->x0, false, false);
        }
    }
    {
        if (!d->neg) {
            d->b = sn2_add(d, d->b, sn2_ld(d->x0), false);
        }
    }
    {
        int32_t v527 = sn2_s24(SN2_YR(d, d->r[4]));
        {
            bool c = d->c;
            (void)sn2_sub(d, d->a, 0, true);
            d->c = c;
            d->v = false;
        }
        d->y0   = v527;
        d->r[4] = sn2_step(d, 4, 1);
    }
    {
        int32_t v528 = sn2_lim(d->b);
        int32_t v529 = sn2_lim(d->b);
        d->b    = sn2_ld(d->y0);
        d->x0   = v528;
        SN2_YW(d, d->r[7], sn2_w24(v529));
        d->r[7] = sn2_step(d, 7, 1);
    }
    {
        if (d->neg) {
            d->b = sn2_mac(d, d->b, d->y1, d->y0, false, false);
        }
    }
    {
        if (!d->neg) {
            d->b = sn2_add(d, d->b, sn2_ld(d->y1), false);
        }
    }
    {
        int32_t v530 = sn2_s24(SN2_YR(d, 0x0));
        d->y1 = v530;
    }
    {
        int32_t v531 = sn2_s24(SN2_XR(d, d->r[0]));
        int32_t v532 = sn2_lim(d->b);
        d->a    = sn2_ld(d->x0);
        d->x0   = v531;
        SN2_YW(d, d->r[7], sn2_w24(v532));
        d->r[0] = sn2_step(d, 0, 1);
    }
    {
        int32_t v533 = sn2_s24(SN2_XR(d, d->r[0]));
        (void)sn2_sub(d, d->a, sn2_ld(d->x0), true);
        d->x0   = v533;
        d->r[0] = sn2_step(d, 0, 1);
    }
    {
        if (!(d->z || (d->neg != d->v))) {
            d->a = sn2_ld(d->y1);
        }
    }
    {
        int32_t v534 = sn2_s24(SN2_XR(d, d->r[0]));
        int32_t v535 = sn2_s24(SN2_YR(d, d->r[7]));
        (void)sn2_sub(d, d->a, sn2_ld(d->x0), true);
        d->x0   = v534;
        d->b    = sn2_ld(v535);
        d->r[0] = sn2_step(d, 0, 1);
        d->r[7] = sn2_step(d, 7, 1);
    }
    {
        if ((d->z || (d->neg != d->v))) {
            d->a = 0;
        }
    }
    {
        int32_t v536 = sn2_s24(SN2_XR(d, d->r[0]));
        (void)sn2_sub(d, d->b, sn2_ld(d->x0), true);
        d->x0   = v536;
        d->r[0] = sn2_step(d, 0, 1);
    }
    {
        if (!(d->z || (d->neg != d->v))) {
            d->b = sn2_ld(d->y1);
        }
    }
    {
        int32_t v537 = sn2_lim(d->a);
        (void)sn2_sub(d, d->b, sn2_ld(d->x0), true);
        SN2_YW(d, d->r[4], sn2_w24(v537));
        d->r[4] = sn2_step(d, 4, 1);
    }
    {
        if ((d->z || (d->neg != d->v))) {
            d->b = 0;
        }
    }
    {
        int32_t v538 = sn2_s24(SN2_XR(d, d->r[0]));
        int32_t v539 = sn2_s24(SN2_YR(d, d->r[5]));
        d->a    = sn2_ld(v538);
        d->y0   = v539;
        d->r[0] = sn2_step(d, 0, 1);
        d->r[5] = sn2_step(d, 5, 1);
    }
    {
        int32_t v540 = sn2_s24(SN2_XR(d, d->r[0]));
        int32_t v541 = sn2_lim(d->b);
        d->x0   = v540;
        SN2_YW(d, d->r[4], sn2_w24(v541));
        d->r[0] = sn2_step(d, 0, 1);
        d->r[4] = sn2_step(d, 4, 1);
    }
    {
        int32_t v542 = sn2_s24(SN2_XR(d, d->r[0]));
        int32_t v543 = sn2_s24(SN2_YR(d, d->r[5]));
        d->a    = sn2_mac(d, d->a, d->x1, d->x0, false, true);
        d->x0   = v542;
        d->y0   = v543;
        d->r[0] = sn2_step(d, 0, 1);
        d->r[5] = sn2_step(d, 5, 1);
    }
    {
        int32_t v544 = sn2_s24(SN2_XR(d, d->r[0]));
        int32_t v545 = sn2_s24(SN2_YR(d, d->r[5]));
        d->a    = sn2_mac(d, d->a, d->y0, d->x0, false, true);
        d->x0   = v544;
        d->y0   = v545;
        d->r[0] = sn2_step(d, 0, 1);
        d->r[5] = sn2_step(d, 5, -1);
    }
    {
        int32_t v546 = sn2_s24(SN2_XR(d, d->r[0]));
        int32_t v547 = sn2_s24(SN2_YR(d, d->r[5]));
        d->a    = sn2_mac(d, d->a, d->y0, d->x0, false, true);
        d->x0   = v546;
        d->y0   = v547;
        d->r[0] = sn2_step(d, 0, 1);
        d->r[5] = sn2_step(d, 5, -1);
    }
    {
        if (d->neg) {
            d->a = 0;
        }
    }
    {
        int32_t v548 = sn2_s24(SN2_XR(d, 0x5));
        d->y0 = v548;
    }
    {
        int32_t v549 = sn2_lim(d->a);
        int32_t v550 = sn2_s24(SN2_YR(d, d->r[4]));
        d->b    = sn2_mac(d, 0, d->y0, d->x0, false, true);
        d->x0   = v549;
        d->y0   = v550;
        d->r[4] = sn2_step(d, 4, 1);
    }
    {
        int32_t v551 = sn2_s24(SN2_YR(d, 0x0));
        d->a  = sn2_mac(d, 0, d->x0, d->x0, false, true);
        d->x1 = v551;
    }
    {
        int32_t v552 = sn2_s24(SN2_XR(d, d->r[2]));
        int32_t v553 = sn2_s24(SN2_YR(d, d->r[4]));
        d->a    = sn2_add(d, d->a, d->b, true);
        d->x0   = v552;
        d->b    = sn2_ld(v553);
        d->r[4] = sn2_step(d, 4, -1);
    }
    {
        int32_t v554 = sn2_s24(d->r[4]);
        d->r[1] = sn2_w24(v554);
    }
    {
        int32_t v555 = sn2_lim(d->a);
        d->a  = sn2_add(d, d->a, sn2_ld(d->y0), true);
        d->y1 = v555;
    }
    {
        if (d->e) {
            d->b = sn2_ld(d->x0);
        }
    }
    {
        int32_t v556 = sn2_hi(d->a);
        d->a    = sn2_mac(d, 0, d->x1, d->y0, false, true);
        SN2_YW(d, d->r[4], sn2_w24(v556));
        d->r[4] = sn2_step(d, 4, 1);
    }
    {
        int32_t v557 = sn2_lim(d->b);
        d->b    = 0;
        d->z    = true;
        d->neg  = false;
        d->e    = false;
        d->u    = true;
        d->v    = false;
        SN2_YW(d, d->r[4], sn2_w24(v557));
        d->r[4] = sn2_step(d, 4, 1);
    }
    {
        int32_t v558 = sn2_s24(SN2_XR(d, d->r[0]));
        d->a    = sn2_add(d, d->a, sn2_ld(d->x1), true);
        d->n[1] = sn2_w24(v558);
        d->r[0] = sn2_step(d, 0, 1);
    }
    {
        if (d->e) {
            d->b = sn2_ld(d->x1);
        }
    }
    {
        int32_t v559 = sn2_s24(SN2_XR(d, d->r[0]));
        {
            uint32_t w = (uint32_t)(d->a >> 24) & 0xFFFFFFu;
            uint32_t o = (w >> 1) & 0xFFFFFFu;
            d->a = sn2_set_hi(d->a, o);
            d->c = (w >> 0) & 1;
            sn2_flags_hi(d, o);
        }
        d->x0   = v559;
        d->r[0] = sn2_step(d, 0, 1);
    }
    {
        int32_t v560 = d->y0;
        int32_t v561 = sn2_lim(d->b);
        d->b    = sn2_ld(v560);
        SN2_YW(d, d->r[4], sn2_w24(v561));
        d->r[4] = sn2_step(d, 4, 1);
    }
    {
        int32_t v562 = sn2_lim(d->a);
        SN2_YW(d, d->r[4], sn2_w24(v562));
        d->r[4] = sn2_step(d, 4, 1);
    }
    {
        int32_t v563 = sn2_s24(SN2_YR(d, sn2_step(d, 1, (int32_t)d->n[1])));
        d->b = sn2_ext56((d->b < 0) ? -d->b : d->b);
        sn2_flags(d, d->b);
        d->v = false;
        d->a = sn2_ld(v563);
    }
    {
        int32_t v564 = sn2_s24(SN2_XR(d, d->r[0]));
        int32_t v565 = sn2_s24(SN2_YR(d, d->r[5]));
        d->a    = sn2_sub(d, d->a, sn2_ld(d->x0), true);
        d->x0   = v564;
        d->y0   = v565;
        d->r[0] = sn2_step(d, 0, 1);
        d->r[5] = sn2_step(d, 5, -1);
    }
    {
        int32_t v566 = sn2_lim(d->b);
        d->b    = sn2_mac(d, 0, d->x0, d->y1, false, true);
        SN2_YW(d, d->r[4], sn2_w24(v566));
        d->r[4] = sn2_step(d, 4, 1);
    }
    {
        if (d->z) {
            d->b = sn2_ld(d->x1);
        }
    }
    {
        if (d->neg) {
            d->b = sn2_ld(d->x1);
        }
    }
    {
        d->b = sn2_asl(d, d->b, 8, true);
    }
    {
        int32_t v567 = sn2_lim(d->a);
        int32_t v568 = sn2_s24(SN2_YR(d, d->r[7]));
        d->x0   = v567;
        d->y0   = v568;
        d->r[7] = sn2_step(d, 7, 1);
    }
    {
        int32_t v569 = sn2_lim(d->b);
        int32_t v570 = sn2_s24(SN2_YR(d, d->r[5]));
        d->a  = sn2_mac(d, 0, d->y0, d->x0, false, true);
        d->x0 = v569;
        d->y1 = v570;
    }
    {
        int32_t v571 = sn2_s24(SN2_XR(d, d->r[2]));
        d->a    = sn2_sub(d, d->a, sn2_ld(d->y1), true);
        d->y0   = v571;
        d->r[2] = sn2_step(d, 2, -1);
    }
    {
        d->b = sn2_mac(d, 0, d->y0, d->y0, false, true);
    }
    {
        int32_t v572 = sn2_lim(d->a);
        d->a  = sn2_ld(d->y1);
        d->y0 = v572;
    }
    {
        int32_t v573 = sn2_s24((uint32_t)(d->b & 0xFFFFFF));
        d->a = sn2_mac(d, d->a, d->y0, d->x0, false, true);
        d->b = sn2_ld(v573);
    }
    {
        int32_t v574 = sn2_s24(SN2_YR(d, sn2_step(d, 5, -0x1)));
        d->x1 = v574;
    }
    {
        int32_t v575 = sn2_lim(d->a);
        d->b    = sn2_ext56((d->b < 0) ? -d->b : d->b);
        sn2_flags(d, d->b);
        d->v    = false;
        SN2_YW(d, d->r[5], sn2_w24(v575));
        d->r[5] = sn2_step(d, 5, 1);
    }
    {
        int32_t v576 = sn2_s24(SN2_XR(d, d->r[0]));
        int32_t v577 = sn2_s24(SN2_YR(d, d->r[5]));
        d->a    = sn2_ld(v576);
        d->y0   = v577;
        d->r[0] = sn2_step(d, 0, 1);
        d->r[5] = sn2_step(d, 5, 1);
    }
    {
        int32_t v578 = sn2_s24(SN2_XR(d, d->r[0]));
        int32_t v579 = sn2_lim(d->b);
        d->x0   = v578;
        d->y1   = v579;
        d->r[0] = sn2_step(d, 0, 1);
    }
    {
        int32_t v580 = sn2_s24(SN2_XR(d, d->r[0]));
        int32_t v581 = sn2_s24(SN2_YR(d, d->r[5]));
        d->a    = sn2_mac(d, d->a, d->x1, d->x0, false, true);
        d->x0   = v580;
        d->y0   = v581;
        d->r[0] = sn2_step(d, 0, 1);
        d->r[5] = sn2_step(d, 5, 1);
    }
    {
        int32_t v582 = sn2_s24(SN2_XR(d, d->r[0]));
        int32_t v583 = sn2_s24(SN2_YR(d, d->r[5]));
        d->a    = sn2_mac(d, d->a, d->y0, d->x0, false, true);
        d->x0   = v582;
        d->y0   = v583;
        d->r[0] = sn2_step(d, 0, 1);
        d->r[5] = sn2_step(d, 5, -1);
    }
    {
        int32_t v584 = sn2_s24(SN2_XR(d, d->r[0]));
        int32_t v585 = sn2_s24(SN2_YR(d, d->r[5]));
        d->a    = sn2_mac(d, d->a, d->y0, d->x0, false, true);
        d->x0   = v584;
        d->y0   = v585;
        d->r[0] = sn2_step(d, 0, 1);
        d->r[5] = sn2_step(d, 5, -1);
    }
    {
        if (d->neg) {
            d->a = 0;
        }
    }
    {
        int32_t v586 = sn2_s24(SN2_XR(d, 0x5));
        d->y0 = v586;
    }
    {
        int32_t v587 = sn2_lim(d->a);
        int32_t v588 = sn2_s24(SN2_YR(d, d->r[4]));
        d->b    = sn2_mac(d, 0, d->y0, d->x0, false, true);
        d->x0   = v587;
        d->y0   = v588;
        d->r[4] = sn2_step(d, 4, 1);
    }
    {
        int32_t v589 = sn2_s24(SN2_YR(d, 0x0));
        d->a  = sn2_mac(d, 0, d->x0, d->x0, false, true);
        d->x1 = v589;
    }
    {
        int32_t v590 = sn2_s24(SN2_YR(d, d->r[4]));
        d->a    = sn2_add(d, d->a, d->b, true);
        d->b    = sn2_ld(v590);
        d->r[4] = sn2_step(d, 4, -1);
    }
    {
        int32_t v591 = sn2_s24(d->r[4]);
        d->r[1] = sn2_w24(v591);
    }
    {
        int32_t v592 = sn2_lim(d->a);
        d->a  = sn2_add(d, d->a, sn2_ld(d->y0), true);
        d->x0 = v592;
    }
    {
        if (d->e) {
            d->b = sn2_ld(d->y1);
        }
    }
    {
        int32_t v593 = sn2_hi(d->a);
        d->a    = sn2_mac(d, 0, d->x1, d->y0, false, true);
        SN2_YW(d, d->r[4], sn2_w24(v593));
        d->r[4] = sn2_step(d, 4, 1);
    }
    {
        int32_t v594 = sn2_lim(d->b);
        d->b    = 0;
        d->z    = true;
        d->neg  = false;
        d->e    = false;
        d->u    = true;
        d->v    = false;
        SN2_YW(d, d->r[4], sn2_w24(v594));
        d->r[4] = sn2_step(d, 4, 1);
    }
    {
        int32_t v595 = sn2_s24(SN2_XR(d, d->r[0]));
        d->a    = sn2_add(d, d->a, sn2_ld(d->x1), true);
        d->n[1] = sn2_w24(v595);
        d->r[0] = sn2_step(d, 0, 1);
    }
    {
        if (d->e) {
            d->b = sn2_ld(d->x1);
        }
    }
    {
        int32_t v596 = sn2_s24(SN2_XR(d, d->r[0]));
        {
            uint32_t w = (uint32_t)(d->a >> 24) & 0xFFFFFFu;
            uint32_t o = (w >> 1) & 0xFFFFFFu;
            d->a = sn2_set_hi(d->a, o);
            d->c = (w >> 0) & 1;
            sn2_flags_hi(d, o);
        }
        d->y1   = v596;
        d->r[0] = sn2_step(d, 0, 1);
    }
    {
        int32_t v597 = d->y0;
        int32_t v598 = sn2_lim(d->b);
        d->b    = sn2_ld(v597);
        SN2_YW(d, d->r[4], sn2_w24(v598));
        d->r[4] = sn2_step(d, 4, 1);
    }
    {
        int32_t v599 = sn2_lim(d->a);
        SN2_YW(d, d->r[4], sn2_w24(v599));
        d->r[4] = sn2_step(d, 4, 1);
    }
    {
        int32_t v600 = sn2_s24(SN2_YR(d, sn2_step(d, 1, (int32_t)d->n[1])));
        d->b = sn2_ext56((d->b < 0) ? -d->b : d->b);
        sn2_flags(d, d->b);
        d->v = false;
        d->a = sn2_ld(v600);
    }
    {
        int32_t v601 = sn2_s24(SN2_XR(d, d->r[0]));
        d->a    = sn2_sub(d, d->a, sn2_ld(d->y1), true);
        d->y1   = v601;
        d->r[0] = sn2_step(d, 0, 1);
    }
    {
        int32_t v602 = sn2_lim(d->b);
        d->b    = sn2_mac(d, 0, d->x0, d->y1, false, true);
        SN2_YW(d, d->r[4], sn2_w24(v602));
        d->r[4] = sn2_step(d, 4, 1);
    }
    {
        if (d->z) {
            d->b = sn2_ld(d->x1);
        }
    }
    {
        if (d->neg) {
            d->b = sn2_ld(d->x1);
        }
    }
    {
        d->b = sn2_asl(d, d->b, 8, true);
    }
    {
        int32_t v603 = sn2_lim(d->a);
        int32_t v604 = sn2_s24(SN2_YR(d, d->r[7]));
        d->x0   = v603;
        d->y0   = v604;
        d->r[7] = sn2_step(d, 7, 1);
    }
    {
        int32_t v605 = sn2_lim(d->b);
        int32_t v606 = sn2_s24(SN2_YR(d, d->r[5]));
        d->b  = sn2_mac(d, 0, d->y0, d->x0, false, true);
        d->x0 = v605;
        d->y0 = v606;
    }
    {
        d->b = sn2_sub(d, d->b, sn2_ld(d->y0), true);
    }
    {
        int32_t v607 = sn2_lim(d->b);
        d->b  = sn2_ld(d->y0);
        d->y0 = v607;
    }
    {
        d->b = sn2_mac(d, d->b, d->y0, d->x0, false, true);
    }
    {
        int32_t v608 = sn2_lim(d->b);
        SN2_YW(d, d->r[5], sn2_w24(v608));
        d->r[5] = sn2_step(d, 5, 1);
    }
    {
        int32_t v609 = sn2_s24(SN2_YR(d, 0x63));
        d->n[1] = sn2_w24(v609);
    }
    {
        int32_t v610 = sn2_s24(d->r[4]);
        d->r[1] = sn2_w24(v610);
    }
    {
        int32_t v611 = sn2_s24(SN2_XR(d, d->r[0]));
        int32_t v612 = sn2_s24(SN2_YR(d, d->r[4]));
        d->x1   = v611;
        d->y1   = v612;
        d->r[0] = sn2_step(d, 0, 1);
        d->r[4] = sn2_step(d, 4, 1);
    }
    {
        int32_t v613 = sn2_s24(SN2_XR(d, d->r[3]));
        int32_t v614 = sn2_s24(SN2_YR(d, d->r[4]));
        d->x0   = v613;
        d->y0   = v614;
        d->r[3] = sn2_step(d, 3, 1);
        d->r[4] = sn2_step(d, 4, 1);
    }
    {
        int32_t v615 = sn2_s24(SN2_XR(d, d->r[3]));
        int32_t v616 = sn2_s24(SN2_YR(d, d->r[4]));
        d->a    = sn2_mac(d, 0, d->y0, d->x0, false, true);
        d->x0   = v615;
        d->y0   = v616;
        d->r[3] = sn2_step(d, 3, -1);
        d->r[4] = sn2_step(d, 4, 1);
    }
    {
        int32_t v617 = d->x1;
        d->a = sn2_mac(d, d->a, d->y0, d->x0, false, true);
        SN2_YW(d, sn2_step(d, 1, (int32_t)d->n[1]), sn2_w24(v617));
    }
    {
        int32_t v618 = sn2_s24(d->r[4]);
        d->r[1] = sn2_w24(v618);
    }
    {
        d->r[4] = sn2_step(d, 4, 1);
    }
    {
        int32_t v619 = sn2_s24(SN2_XR(d, d->r[0]));
        int32_t v620 = sn2_lim(d->a);
        d->x1   = v619;
        SN2_YW(d, d->r[5], sn2_w24(v620));
        d->r[0] = sn2_step(d, 0, 1);
        d->r[5] = sn2_step(d, 5, 1);
    }
    {
        int32_t v621 = sn2_s24(SN2_XR(d, d->r[3]));
        int32_t v622 = sn2_s24(SN2_YR(d, d->r[4]));
        d->x0   = v621;
        d->y0   = v622;
        d->r[3] = sn2_step(d, 3, 1);
        d->r[4] = sn2_step(d, 4, 1);
    }
    {
        int32_t v623 = sn2_s24(SN2_XR(d, d->r[3]));
        int32_t v624 = sn2_s24(SN2_YR(d, d->r[4]));
        d->a    = sn2_mac(d, 0, d->y0, d->x0, false, true);
        d->x0   = v623;
        d->y0   = v624;
        d->r[3] = sn2_step(d, 3, -1);
        d->r[4] = sn2_step(d, 4, 1);
    }
    {
        int32_t v625 = d->x1;
        d->a = sn2_mac(d, d->a, d->y0, d->x0, false, true);
        SN2_YW(d, sn2_step(d, 1, (int32_t)d->n[1]), sn2_w24(v625));
    }
    {
        int32_t v626 = sn2_lim(d->a);
        SN2_YW(d, d->r[5], sn2_w24(v626));
        d->r[5] = sn2_step(d, 5, 1);
    }
    {
        int32_t v627 = sn2_s24(SN2_XR(d, d->r[0]));
        int32_t v628 = sn2_s24(SN2_YR(d, d->r[4]));
        d->x1   = v627;
        d->b    = sn2_ld(v628);
        d->r[0] = sn2_step(d, 0, 1);
    }
    {
        int32_t v629 = sn2_s24(SN2_XR(d, d->r[0]));
        int32_t v630 = sn2_s24(SN2_YR(d, d->r[4]));
        {
            uint32_t o = ((uint32_t)(d->b >> 24) & sn2_w24(d->x1)) & 0xFFFFFFu;
            d->b = sn2_set_hi(d->b, o);
            sn2_flags_hi(d, o);
        }
        d->x0   = v629;
        d->a    = sn2_ld(v630);
        d->r[0] = sn2_step(d, 0, 1);
    }
    {
        int32_t v631 = sn2_s24(SN2_XR(d, d->r[0]));
        d->a    = sn2_sub(d, d->a, sn2_ld(d->x0), true);
        d->x1   = v631;
        d->r[0] = sn2_step(d, 0, 1);
    }
    {
        if (d->neg) {
            d->a = sn2_add(d, d->a, sn2_ld(d->x1), false);
        }
    }
    {
        if (!d->neg) {
            d->a = sn2_sub(d, d->a, sn2_ld(d->x1), false);
        }
    }
    {
        int32_t v632 = sn2_s24(SN2_XR(d, d->r[0]));
        {
            int64_t m1 = (d->a < 0) ? -d->a : d->a;
            int64_t s0 = sn2_ld(d->x1);
            int64_t m2 = (s0 < 0) ? -s0 : s0;
            int64_t r  = sn2_ext56(m1 - m2);
            d->c = (uint64_t)m2 > (uint64_t)m1;
            d->v = false;
            sn2_flags(d, r);
        }
        d->y1   = v632;
        d->r[0] = sn2_step(d, 0, 1);
    }
    {
        if ((d->z || (d->neg != d->v))) {
            d->a = 0;
        }
    }
    {
        int32_t v633 = sn2_s24(SN2_YR(d, 0x1));
        d->y0 = v633;
    }
    {
        int32_t v634 = sn2_lim(d->a);
        d->a  = sn2_add(d, d->a, sn2_ld(d->x0), true);
        d->x1 = v634;
    }
    {
        int32_t v635 = sn2_lim(d->b);
        d->a  = sn2_mac(d, d->a, d->y1, d->x1, true, true);
        d->x0 = v635;
    }
    {
        d->b    = sn2_mac(d, 0, d->y0, d->x0, false, true);
        d->r[0] = sn2_step(d, 0, 1);
    }
    {
        int32_t v636 = sn2_lim(d->a);
        SN2_YW(d, d->r[4], sn2_w24(v636));
    }
    {
        int32_t v637 = sn2_lim(d->b);
        SN2_YW(d, d->r[5], sn2_w24(v637));
        d->r[5] = sn2_step(d, 5, 1);
    }
    {
        int32_t v638 = sn2_s24(d->n[3]);
        d->r[4] = sn2_w24(v638);
    }
L15_pass:;

    for (int lc1534 = 0; lc1534 < 2; lc1534++) {
        {
            int32_t v639 = sn2_s24(SN2_YR(d, 0x0));
            d->y0 = v639;
        }
        {
            int32_t v640 = sn2_s24(SN2_XR(d, d->r[0]));
            int32_t v641 = sn2_s24(SN2_YR(d, d->r[4]));
            d->a    = sn2_ld(v640);
            d->y1   = v641;
            d->r[0] = sn2_step(d, 0, 1);
            d->r[4] = sn2_step(d, 4, 1);
        }
        {
            int32_t v642 = sn2_s24(SN2_XR(d, d->r[0]));
            int32_t v643 = sn2_s24(SN2_YR(d, d->r[4]));
            d->a    = sn2_add(d, d->a, sn2_ld(d->y1), true);
            d->x1   = v642;
            d->b    = sn2_ld(v643);
            d->r[0] = sn2_step(d, 0, 1);
            d->r[4] = sn2_step(d, 4, -1);
        }
        {
            if (d->e) {
                d->b = sn2_ld(d->y0);
            }
        }
        {
            int32_t v644 = sn2_hi(d->a);
            d->y1 = v644;
        }
        {
            int32_t v645 = sn2_s24(SN2_XR(d, d->r[0]));
            int32_t v646 = d->y1;
            d->a    = sn2_mac(d, 0, d->y1, d->x1, false, true);
            d->x0   = v645;
            SN2_YW(d, d->r[4], sn2_w24(v646));
            d->r[0] = sn2_step(d, 0, 1);
            d->r[4] = sn2_step(d, 4, 1);
        }
        {
            int32_t v647 = sn2_s24(SN2_XR(d, d->r[0]));
            int32_t v648 = sn2_lim(d->b);
            d->a    = sn2_add(d, d->a, sn2_ld(d->x1), true);
            d->x1   = v647;
            SN2_YW(d, d->r[4], sn2_w24(v648));
            d->r[0] = sn2_step(d, 0, 1);
        }
        {
            d->b = d->a;
        }
        {
            int32_t v649 = sn2_s24(SN2_XR(d, d->r[0]));
            (void)sn2_sub(d, d->b, sn2_ld(d->x1), true);
            d->x1   = v649;
            d->r[0] = sn2_step(d, 0, 1);
        }
        {
            if (!(d->z || (d->neg != d->v))) {
                d->b = sn2_add(d, d->b, sn2_ld(d->x1), false);
            }
        }
        {
            if (!(d->z || (d->neg != d->v))) {
                d->b = sn2_add(d, d->b, sn2_ld(d->x1), false);
            }
        }
        {
            d->b = sn2_asl(d, d->b, 6, true);
        }
        {
            d->b = sn2_sub(d, d->b, sn2_ld(d->y0), true);
        }
        {
            int32_t v650 = d->y1;
            (void)sn2_sub(d, d->a, sn2_ld(d->x0), true);
            d->x0 = v650;
        }
        {
            if (!(d->z || (d->neg != d->v))) {
                d->a = sn2_add(d, d->a, sn2_ld(d->x1), false);
            }
        }
        {
            d->b = sn2_ld(sn2_hi(d->b));
        }
        {
            d->b = sn2_ext56((d->b < 0) ? -d->b : d->b);
            sn2_flags(d, d->b);
            d->v = false;
        }
        {
            if (d->e) {
                d->b = sn2_ld(d->y0);
            }
        }
        {
            int32_t v651 = sn2_s24(SN2_XR(d, d->r[0]));
            {
                uint32_t o = ((uint32_t)(d->b >> 24) ^ sn2_w24(d->y0)) & 0xFFFFFFu;
                d->b = sn2_set_hi(d->b, o);
                sn2_flags_hi(d, o);
            }
            d->y1   = v651;
            d->r[0] = sn2_step(d, 0, 1);
        }
        {
            d->a = sn2_asl(d, d->a, 6, true);
        }
        {
            int32_t v652 = sn2_hi(d->b);
            d->a  = sn2_sub(d, d->a, sn2_ld(d->y0), true);
            d->y0 = v652;
        }
        {
            int32_t v653 = sn2_s24(SN2_YR(d, 0x16));
            d->b  = sn2_mac(d, 0, d->y1, d->y0, false, true);
            d->y1 = v653;
        }
        {
            (void)sn2_sub(d, d->b, sn2_ld(d->y1), true);
            d->a = sn2_ld(sn2_hi(d->a));
        }
        {
            if (!(d->z || (d->neg != d->v))) {
                d->b = sn2_ld(d->y1);
            }
        }
        {
            int32_t v654 = sn2_s24(SN2_YR(d, 0x17));
            d->r[1] = sn2_w24(v654);
        }
        {
            int32_t v655 = sn2_hi(d->b);
            d->n[1] = sn2_w24(v655);
        }
        {
            int32_t v656 = sn2_s24((uint32_t)(d->b & 0xFFFFFF));
            d->b = sn2_ld(v656);
        }
        {
            {
                uint32_t w = (uint32_t)(d->b >> 24) & 0xFFFFFFu;
                uint32_t o = (w >> 1) & 0xFFFFFFu;
                d->b = sn2_set_hi(d->b, o);
                d->c = (w >> 0) & 1;
                sn2_flags_hi(d, o);
            }
            d->n[0] = sn2_w24(sn2_s24(0x5));
        }
        {
            d->r[1] = sn2_step(d, 1, (int32_t)d->n[1]);
        }
        {
            int32_t v657 = sn2_s24(SN2_YR(d, d->r[1]));
            d->y1   = v657;
            d->r[1] = sn2_step(d, 1, 1);
        }
        {
            int32_t v658 = sn2_hi(d->b);
            d->b  = sn2_ld(d->y1);
            d->y0 = v658;
        }
        {
            int32_t v659 = sn2_s24(SN2_YR(d, d->r[1]));
            d->b  = sn2_mac(d, d->b, d->y1, d->y0, true, true);
            d->y1 = v659;
        }
        {
            int32_t v660 = sn2_s24(SN2_XR(d, d->r[0]));
            d->b    = sn2_mac(d, d->b, d->y1, d->y0, false, true);
            d->y0   = v660;
            d->r[0] = sn2_step(d, 0, -(int32_t)d->n[0]);
        }
        {
            int32_t v661 = sn2_lim(d->a);
            d->y1 = v661;
        }
        {
            int32_t v662 = sn2_lim(d->b);
            d->x1 = v662;
        }
        {
            int32_t v663 = sn2_s24(SN2_XR(d, d->r[0]));
            d->b    = sn2_mac(d, 0, d->y1, d->x1, false, true);
            d->x1   = v663;
            d->r[0] = sn2_step(d, 0, 1);
        }
        {
            int32_t v664 = d->x0;
            d->y1 = v664;
        }
        {
            d->a = sn2_mac(d, 0, d->y1, d->x1, false, true);
        }
        {
            int32_t v665 = sn2_lim(d->b);
            d->a    = sn2_add(d, d->a, sn2_ld(d->y0), true);
            d->n[2] = sn2_w24(v665);
        }
        {
            int32_t v666 = sn2_s24(SN2_XR(d, d->r[0]));
            (void)sn2_sub(d, d->a, sn2_ld(d->x1), true);
            d->x0   = v666;
            d->r[0] = sn2_step(d, 0, 1);
        }
        {
            if (!(d->z || (d->neg != d->v))) {
                d->a = sn2_sub(d, d->a, sn2_ld(d->x1), false);
            }
        }
        {
            if (!(d->z || (d->neg != d->v))) {
                d->a = sn2_sub(d, d->a, sn2_ld(d->x1), false);
            }
        }
        {
            int32_t v667 = sn2_s24(SN2_XR(d, d->r[0]));
            d->a    = sn2_add(d, d->a, sn2_ld(d->x1), true);
            d->y1   = v667;
            d->r[0] = sn2_step(d, 0, 1);
        }
        {
            if (d->neg) {
                d->a = sn2_add(d, d->a, sn2_ld(d->x1), false);
            }
        }
        {
            if (d->neg) {
                d->a = sn2_add(d, d->a, sn2_ld(d->x1), false);
            }
        }
        {
            int32_t v668 = sn2_s24(SN2_YR(d, 0x0));
            d->b  = d->a;
            d->y0 = v668;
        }
        {
            int32_t v669 = sn2_s24(SN2_XR(d, d->r[0]));
            (void)sn2_sub(d, d->b, sn2_ld(d->y1), true);
            d->x1   = v669;
            d->r[0] = sn2_step(d, 0, 1);
        }
        {
            if (!(d->z || (d->neg != d->v))) {
                d->b = sn2_add(d, d->b, sn2_ld(d->x1), false);
            }
        }
        {
            if (!(d->z || (d->neg != d->v))) {
                d->b = sn2_add(d, d->b, sn2_ld(d->x1), false);
            }
        }
        {
            d->b = sn2_asl(d, d->b, 6, true);
        }
        {
            d->b = sn2_sub(d, d->b, sn2_ld(d->y0), true);
        }
        {
            (void)sn2_sub(d, d->a, sn2_ld(d->x0), true);
        }
        {
            if (!(d->z || (d->neg != d->v))) {
                d->a = sn2_add(d, d->a, sn2_ld(d->x1), false);
            }
        }
        {
            d->b = sn2_ld(sn2_hi(d->b));
        }
        {
            d->b = sn2_ext56((d->b < 0) ? -d->b : d->b);
            sn2_flags(d, d->b);
            d->v = false;
        }
        {
            if (d->e) {
                d->b = sn2_ld(d->y0);
            }
        }
        {
            int32_t v670 = sn2_s24(SN2_XR(d, d->r[0]));
            {
                uint32_t o = ((uint32_t)(d->b >> 24) ^ sn2_w24(d->y0)) & 0xFFFFFFu;
                d->b = sn2_set_hi(d->b, o);
                sn2_flags_hi(d, o);
            }
            d->y1   = v670;
            d->r[0] = sn2_step(d, 0, 1);
        }
        {
            d->a = sn2_asl(d, d->a, 6, true);
        }
        {
            int32_t v671 = sn2_hi(d->b);
            d->a  = sn2_sub(d, d->a, sn2_ld(d->y0), true);
            d->y0 = v671;
        }
        {
            int32_t v672 = sn2_s24(SN2_YR(d, 0x16));
            d->b  = sn2_mac(d, 0, d->y1, d->y0, false, true);
            d->y1 = v672;
        }
        {
            int32_t v673 = sn2_hi(d->a);
            (void)sn2_sub(d, d->b, sn2_ld(d->y1), true);
            d->x1 = v673;
        }
        {
            if (!(d->z || (d->neg != d->v))) {
                d->b = sn2_ld(d->y1);
            }
        }
        {
            int32_t v674 = sn2_s24(SN2_YR(d, 0x17));
            d->r[1] = sn2_w24(v674);
        }
        {
            int32_t v675 = sn2_hi(d->b);
            d->n[1] = sn2_w24(v675);
        }
        {
            int32_t v676 = sn2_s24((uint32_t)(d->b & 0xFFFFFF));
            d->b = sn2_ld(v676);
        }
        {
            int32_t v677 = sn2_s24(SN2_XR(d, d->r[0]));
            {
                uint32_t w = (uint32_t)(d->b >> 24) & 0xFFFFFFu;
                uint32_t o = (w >> 1) & 0xFFFFFFu;
                d->b = sn2_set_hi(d->b, o);
                d->c = (w >> 0) & 1;
                sn2_flags_hi(d, o);
            }
            d->x0   = v677;
            d->r[0] = sn2_step(d, 0, 1);
        }
        {
            int32_t v678 = sn2_s24(d->n[2]);
            d->x0 = v678;
        }
        {
            d->r[1] = sn2_step(d, 1, (int32_t)d->n[1]);
        }
        {
            int32_t v679 = sn2_s24(SN2_YR(d, d->r[1]));
            d->y1   = v679;
            d->r[1] = sn2_step(d, 1, 1);
        }
        {
            int32_t v680 = sn2_hi(d->b);
            d->a  = sn2_ld(d->y1);
            d->y0 = v680;
        }
        {
            int32_t v681 = sn2_s24(SN2_YR(d, d->r[1]));
            d->a  = sn2_mac(d, d->a, d->y1, d->y0, true, true);
            d->y1 = v681;
        }
        {
            int32_t v682 = sn2_s24(SN2_XR(d, d->r[0]));
            d->a = sn2_mac(d, d->a, d->y1, d->y0, false, true);
            d->b = sn2_ld(v682);
        }
        {
            {
                uint32_t w = (uint32_t)(d->b >> 24) & 0xFFFFFFu;
                uint32_t o = (w << 20) & 0xFFFFFFu;
                d->b = sn2_set_hi(d->b, o);
                d->c = (w >> 4) & 1;
                sn2_flags_hi(d, o);
            }
        }
        {
            if (d->c) {
                d->a = sn2_ext56(-d->a);
            }
        }
        {
            int32_t v683 = sn2_s24(SN2_XR(d, d->r[0]));
            {
                uint32_t w = (uint32_t)(d->b >> 24) & 0xFFFFFFu;
                uint32_t o = (w << 1) & 0xFFFFFFu;
                d->b = sn2_set_hi(d->b, o);
                d->c = (w >> 23) & 1;
                sn2_flags_hi(d, o);
            }
            d->n[2] = sn2_w24(v683);
            d->r[0] = sn2_step(d, 0, 1);
        }
        {
            int32_t v684 = sn2_lim(d->a);
            d->b  = sn2_ld(d->x0);
            d->y0 = v684;
        }
        {
            if (d->c) {
                d->b = sn2_mac(d, d->b, d->x1, d->y0, false, false);
            }
        }
        {
            int32_t v685 = sn2_s24(SN2_XR(d, d->r[0]));
            int32_t v686 = sn2_s24(SN2_YR(d, d->r[4]));
            d->b    = sn2_asr(d, d->b, 1, true);
            d->x1   = v685;
            d->y1   = v686;
            d->r[0] = sn2_step(d, 0, 1);
        }
        {
            d->a    = sn2_ld(d->y1);
            d->r[1] = sn2_w24(sn2_s24(0x8));
        }
        {
            int32_t v687 = sn2_lim(d->b);
            d->a  = sn2_mac(d, d->a, d->y1, d->x1, true, true);
            d->x0 = v687;
        }
        {
            int32_t v688 = sn2_s24(d->n[2]);
            d->b = sn2_ld(v688);
        }
        {
            int32_t v689 = sn2_s24(SN2_XR(d, d->r[0]));
            int32_t v690 = sn2_lim(d->a);
            d->a    = sn2_mac(d, 0, d->x0, d->y1, false, true);
            d->x1   = v689;
            SN2_YW(d, d->r[4], sn2_w24(v690));
            d->r[0] = sn2_step(d, 0, 1);
            d->r[4] = sn2_step(d, 4, 1);
        }
        {
            int32_t v691 = sn2_s24(SN2_XR(d, d->r[1]));
            {
                uint32_t w = (uint32_t)(d->b >> 24) & 0xFFFFFFu;
                uint32_t o = (w >> 1) & 0xFFFFFFu;
                d->b = sn2_set_hi(d->b, o);
                d->c = (w >> 0) & 1;
                sn2_flags_hi(d, o);
            }
            d->y1   = v691;
            d->r[1] = sn2_step(d, 1, 1);
        }
        {
            if (d->c) {
                d->a = sn2_ld(d->y1);
            }
        }
        {
            int32_t v692 = sn2_s24(SN2_XR(d, d->r[1]));
            {
                uint32_t w = (uint32_t)(d->b >> 24) & 0xFFFFFFu;
                uint32_t o = (w >> 1) & 0xFFFFFFu;
                d->b = sn2_set_hi(d->b, o);
                d->c = (w >> 0) & 1;
                sn2_flags_hi(d, o);
            }
            d->y1   = v692;
            d->r[1] = sn2_step(d, 1, 1);
        }
        {
            if (d->c) {
                d->a = sn2_ld(d->y1);
            }
        }
        {
            int32_t v693 = sn2_lim(d->a);
            int32_t v694 = sn2_lim(d->a);
            d->x0   = v693;
            SN2_YW(d, d->r[4], sn2_w24(v694));
            d->r[4] = sn2_step(d, 4, 1);
        }
        {
            d->a = sn2_mac(d, 0, d->x1, d->x0, false, true);
        }
        {
            int32_t v695 = sn2_lim(d->a);
            SN2_YW(d, d->r[4], sn2_w24(v695));
            d->r[4] = sn2_step(d, 4, 1);
        }
    }

    {
        int32_t v696 = sn2_s24(SN2_XR(d, d->r[0]));
        d->x0   = v696;
        d->r[0] = sn2_step(d, 0, 1);
    }
    {
        int32_t v697 = sn2_s24(SN2_YR(d, sn2_step(d, 4, 0x4)));
        d->y0 = v697;
    }
    {
        int32_t v698 = sn2_s24(SN2_XR(d, d->r[0]));
        d->b    = sn2_mac(d, 0, d->y0, d->x0, false, true);
        d->x0   = v698;
        d->r[0] = sn2_step(d, 0, 1);
    }
    {
        int32_t v699 = sn2_s24(SN2_YR(d, sn2_step(d, 4, -0x6)));
        d->y0 = v699;
    }
    {
        int32_t v700 = sn2_s24(SN2_XR(d, d->r[0]));
        d->b    = sn2_mac(d, d->b, d->y0, d->x0, false, true);
        d->x0   = v700;
        d->r[0] = sn2_step(d, 0, 1);
    }
    {
        int32_t v701 = sn2_s24(SN2_YR(d, sn2_step(d, 4, -0x2)));
        d->y0 = v701;
    }
    {
        d->b = sn2_mac(d, d->b, d->y0, d->x0, false, true);
    }
    {
        int32_t v702 = sn2_s24(SN2_XR(d, d->r[0]));
        int32_t v703 = sn2_s24(SN2_YR(d, d->r[4]));
        d->x0   = v702;
        d->y1   = v703;
        d->r[0] = sn2_step(d, 0, 1);
        d->r[4] = sn2_step(d, 4, 1);
    }
    {
        int32_t v704 = sn2_lim(d->b);
        d->x1 = v704;
    }
    {
        int32_t v705 = d->x0;
        d->b = sn2_mac(d, 0, d->x1, d->x0, false, true);
        d->a = sn2_ld(v705);
    }
    {
        d->b = sn2_asl(d, d->b, 7, true);
    }
    {
        int32_t v706 = sn2_lim(d->b);
        d->b = sn2_ld(v706);
    }
    {
        d->b = sn2_asr(d, d->b, 1, true);
    }
    {
        d->a = sn2_add(d, d->a, d->b, true);
    }
    {
        int32_t v707 = sn2_s24(SN2_YR(d, 0x0));
        d->y0 = v707;
    }
    {
        int32_t v708 = sn2_lim(d->a);
        d->a = sn2_ld(v708);
    }
    {
        int32_t v709 = sn2_s24(SN2_XR(d, d->r[0]));
        int32_t v710 = sn2_s24(SN2_YR(d, d->r[4]));
        d->a    = sn2_add(d, d->a, sn2_ld(d->y1), true);
        d->x1   = v709;
        d->b    = sn2_ld(v710);
        d->r[0] = sn2_step(d, 0, 1);
        d->r[4] = sn2_step(d, 4, -1);
    }
    {
        if (d->e) {
            d->b = sn2_ld(d->y0);
        }
    }
    {
        int32_t v711 = sn2_hi(d->a);
        d->y1 = v711;
    }
    {
        int32_t v712 = sn2_s24(SN2_XR(d, d->r[0]));
        int32_t v713 = d->y1;
        d->a    = sn2_mac(d, 0, d->y1, d->x1, false, true);
        d->x0   = v712;
        SN2_YW(d, d->r[4], sn2_w24(v713));
        d->r[0] = sn2_step(d, 0, 1);
        d->r[4] = sn2_step(d, 4, 1);
    }
    {
        int32_t v714 = sn2_s24(SN2_XR(d, d->r[0]));
        int32_t v715 = sn2_lim(d->b);
        d->a    = sn2_add(d, d->a, sn2_ld(d->x1), true);
        d->x1   = v714;
        SN2_YW(d, d->r[4], sn2_w24(v715));
        d->r[0] = sn2_step(d, 0, 1);
    }
    {
        d->b = d->a;
    }
    {
        int32_t v716 = sn2_s24(SN2_XR(d, d->r[0]));
        (void)sn2_sub(d, d->b, sn2_ld(d->x1), true);
        d->x1   = v716;
        d->r[0] = sn2_step(d, 0, 1);
    }
    {
        if (!(d->z || (d->neg != d->v))) {
            d->b = sn2_add(d, d->b, sn2_ld(d->x1), false);
        }
    }
    {
        if (!(d->z || (d->neg != d->v))) {
            d->b = sn2_add(d, d->b, sn2_ld(d->x1), false);
        }
    }
    {
        d->b = sn2_asl(d, d->b, 6, true);
    }
    {
        d->b = sn2_sub(d, d->b, sn2_ld(d->y0), true);
    }
    {
        int32_t v717 = d->y1;
        (void)sn2_sub(d, d->a, sn2_ld(d->x0), true);
        d->x0 = v717;
    }
    {
        if (!(d->z || (d->neg != d->v))) {
            d->a = sn2_add(d, d->a, sn2_ld(d->x1), false);
        }
    }
    {
        d->b = sn2_ld(sn2_hi(d->b));
    }
    {
        d->b = sn2_ext56((d->b < 0) ? -d->b : d->b);
        sn2_flags(d, d->b);
        d->v = false;
    }
    {
        if (d->e) {
            d->b = sn2_ld(d->y0);
        }
    }
    {
        int32_t v718 = sn2_s24(SN2_XR(d, d->r[0]));
        {
            uint32_t o = ((uint32_t)(d->b >> 24) ^ sn2_w24(d->y0)) & 0xFFFFFFu;
            d->b = sn2_set_hi(d->b, o);
            sn2_flags_hi(d, o);
        }
        d->y1   = v718;
        d->r[0] = sn2_step(d, 0, 1);
    }
    {
        d->a = sn2_asl(d, d->a, 6, true);
    }
    {
        int32_t v719 = sn2_hi(d->b);
        d->a  = sn2_sub(d, d->a, sn2_ld(d->y0), true);
        d->y0 = v719;
    }
    {
        int32_t v720 = sn2_s24(SN2_YR(d, 0x16));
        d->b  = sn2_mac(d, 0, d->y1, d->y0, false, true);
        d->y1 = v720;
    }
    {
        (void)sn2_sub(d, d->b, sn2_ld(d->y1), true);
        d->a = sn2_ld(sn2_hi(d->a));
    }
    {
        if (!(d->z || (d->neg != d->v))) {
            d->b = sn2_ld(d->y1);
        }
    }
    {
        int32_t v721 = sn2_s24(SN2_YR(d, 0x17));
        d->r[1] = sn2_w24(v721);
    }
    {
        int32_t v722 = sn2_hi(d->b);
        d->n[1] = sn2_w24(v722);
    }
    {
        int32_t v723 = sn2_s24((uint32_t)(d->b & 0xFFFFFF));
        d->b = sn2_ld(v723);
    }
    {
        {
            uint32_t w = (uint32_t)(d->b >> 24) & 0xFFFFFFu;
            uint32_t o = (w >> 1) & 0xFFFFFFu;
            d->b = sn2_set_hi(d->b, o);
            d->c = (w >> 0) & 1;
            sn2_flags_hi(d, o);
        }
        d->n[0] = sn2_w24(sn2_s24(0x5));
    }
    {
        d->r[1] = sn2_step(d, 1, (int32_t)d->n[1]);
    }
    {
        int32_t v724 = sn2_s24(SN2_YR(d, d->r[1]));
        d->y1   = v724;
        d->r[1] = sn2_step(d, 1, 1);
    }
    {
        int32_t v725 = sn2_hi(d->b);
        d->b  = sn2_ld(d->y1);
        d->y0 = v725;
    }
    {
        int32_t v726 = sn2_s24(SN2_YR(d, d->r[1]));
        d->b  = sn2_mac(d, d->b, d->y1, d->y0, true, true);
        d->y1 = v726;
    }
    {
        int32_t v727 = sn2_s24(SN2_XR(d, d->r[0]));
        d->b    = sn2_mac(d, d->b, d->y1, d->y0, false, true);
        d->y0   = v727;
        d->r[0] = sn2_step(d, 0, -(int32_t)d->n[0]);
    }
    {
        int32_t v728 = sn2_lim(d->a);
        d->y1 = v728;
    }
    {
        int32_t v729 = sn2_lim(d->b);
        d->x1 = v729;
    }
    {
        int32_t v730 = sn2_s24(SN2_XR(d, d->r[0]));
        d->b    = sn2_mac(d, 0, d->y1, d->x1, false, true);
        d->x1   = v730;
        d->r[0] = sn2_step(d, 0, 1);
    }
    {
        int32_t v731 = d->x0;
        d->y1 = v731;
    }
    {
        d->a = sn2_mac(d, 0, d->y1, d->x1, false, true);
    }
    {
        int32_t v732 = sn2_lim(d->b);
        d->a    = sn2_add(d, d->a, sn2_ld(d->y0), true);
        d->n[2] = sn2_w24(v732);
    }
    {
        int32_t v733 = sn2_s24(SN2_XR(d, d->r[0]));
        (void)sn2_sub(d, d->a, sn2_ld(d->x1), true);
        d->x0   = v733;
        d->r[0] = sn2_step(d, 0, 1);
    }
    {
        if (!(d->z || (d->neg != d->v))) {
            d->a = sn2_sub(d, d->a, sn2_ld(d->x1), false);
        }
    }
    {
        if (!(d->z || (d->neg != d->v))) {
            d->a = sn2_sub(d, d->a, sn2_ld(d->x1), false);
        }
    }
    {
        int32_t v734 = sn2_s24(SN2_XR(d, d->r[0]));
        d->a    = sn2_add(d, d->a, sn2_ld(d->x1), true);
        d->y1   = v734;
        d->r[0] = sn2_step(d, 0, 1);
    }
    {
        if (d->neg) {
            d->a = sn2_add(d, d->a, sn2_ld(d->x1), false);
        }
    }
    {
        if (d->neg) {
            d->a = sn2_add(d, d->a, sn2_ld(d->x1), false);
        }
    }
    {
        int32_t v735 = sn2_s24(SN2_YR(d, 0x0));
        d->b  = d->a;
        d->y0 = v735;
    }
    {
        int32_t v736 = sn2_s24(SN2_XR(d, d->r[0]));
        (void)sn2_sub(d, d->b, sn2_ld(d->y1), true);
        d->x1   = v736;
        d->r[0] = sn2_step(d, 0, 1);
    }
    {
        if (!(d->z || (d->neg != d->v))) {
            d->b = sn2_add(d, d->b, sn2_ld(d->x1), false);
        }
    }
    {
        if (!(d->z || (d->neg != d->v))) {
            d->b = sn2_add(d, d->b, sn2_ld(d->x1), false);
        }
    }
    {
        d->b = sn2_asl(d, d->b, 6, true);
    }
    {
        d->b = sn2_sub(d, d->b, sn2_ld(d->y0), true);
    }
    {
        (void)sn2_sub(d, d->a, sn2_ld(d->x0), true);
    }
    {
        if (!(d->z || (d->neg != d->v))) {
            d->a = sn2_add(d, d->a, sn2_ld(d->x1), false);
        }
    }
    {
        d->b = sn2_ld(sn2_hi(d->b));
    }
    {
        d->b = sn2_ext56((d->b < 0) ? -d->b : d->b);
        sn2_flags(d, d->b);
        d->v = false;
    }
    {
        if (d->e) {
            d->b = sn2_ld(d->y0);
        }
    }
    {
        int32_t v737 = sn2_s24(SN2_XR(d, d->r[0]));
        {
            uint32_t o = ((uint32_t)(d->b >> 24) ^ sn2_w24(d->y0)) & 0xFFFFFFu;
            d->b = sn2_set_hi(d->b, o);
            sn2_flags_hi(d, o);
        }
        d->y1   = v737;
        d->r[0] = sn2_step(d, 0, 1);
    }
    {
        d->a = sn2_asl(d, d->a, 6, true);
    }
    {
        int32_t v738 = sn2_hi(d->b);
        d->a  = sn2_sub(d, d->a, sn2_ld(d->y0), true);
        d->y0 = v738;
    }
    {
        int32_t v739 = sn2_s24(SN2_YR(d, 0x16));
        d->b  = sn2_mac(d, 0, d->y1, d->y0, false, true);
        d->y1 = v739;
    }
    {
        int32_t v740 = sn2_hi(d->a);
        (void)sn2_sub(d, d->b, sn2_ld(d->y1), true);
        d->x1 = v740;
    }
    {
        if (!(d->z || (d->neg != d->v))) {
            d->b = sn2_ld(d->y1);
        }
    }
    {
        int32_t v741 = sn2_s24(SN2_YR(d, 0x17));
        d->r[1] = sn2_w24(v741);
    }
    {
        int32_t v742 = sn2_hi(d->b);
        d->n[1] = sn2_w24(v742);
    }
    {
        int32_t v743 = sn2_s24((uint32_t)(d->b & 0xFFFFFF));
        d->b = sn2_ld(v743);
    }
    {
        int32_t v744 = sn2_s24(SN2_XR(d, d->r[0]));
        {
            uint32_t w = (uint32_t)(d->b >> 24) & 0xFFFFFFu;
            uint32_t o = (w >> 1) & 0xFFFFFFu;
            d->b = sn2_set_hi(d->b, o);
            d->c = (w >> 0) & 1;
            sn2_flags_hi(d, o);
        }
        d->x0   = v744;
        d->r[0] = sn2_step(d, 0, 1);
    }
    {
        int32_t v745 = sn2_s24(d->n[2]);
        d->x0 = v745;
    }
    {
        d->r[1] = sn2_step(d, 1, (int32_t)d->n[1]);
    }
    {
        int32_t v746 = sn2_s24(SN2_YR(d, d->r[1]));
        d->y1   = v746;
        d->r[1] = sn2_step(d, 1, 1);
    }
    {
        int32_t v747 = sn2_hi(d->b);
        d->a  = sn2_ld(d->y1);
        d->y0 = v747;
    }
    {
        int32_t v748 = sn2_s24(SN2_YR(d, d->r[1]));
        d->a  = sn2_mac(d, d->a, d->y1, d->y0, true, true);
        d->y1 = v748;
    }
    {
        int32_t v749 = sn2_s24(SN2_XR(d, d->r[0]));
        d->a = sn2_mac(d, d->a, d->y1, d->y0, false, true);
        d->b = sn2_ld(v749);
    }
    {
        {
            uint32_t w = (uint32_t)(d->b >> 24) & 0xFFFFFFu;
            uint32_t o = (w << 20) & 0xFFFFFFu;
            d->b = sn2_set_hi(d->b, o);
            d->c = (w >> 4) & 1;
            sn2_flags_hi(d, o);
        }
    }
    {
        if (d->c) {
            d->a = sn2_ext56(-d->a);
        }
    }
    {
        int32_t v750 = sn2_s24(SN2_XR(d, d->r[0]));
        {
            uint32_t w = (uint32_t)(d->b >> 24) & 0xFFFFFFu;
            uint32_t o = (w << 1) & 0xFFFFFFu;
            d->b = sn2_set_hi(d->b, o);
            d->c = (w >> 23) & 1;
            sn2_flags_hi(d, o);
        }
        d->n[2] = sn2_w24(v750);
        d->r[0] = sn2_step(d, 0, 1);
    }
    {
        int32_t v751 = sn2_lim(d->a);
        d->b  = sn2_ld(d->x0);
        d->y0 = v751;
    }
    {
        if (d->c) {
            d->b = sn2_mac(d, d->b, d->x1, d->y0, false, false);
        }
    }
    {
        int32_t v752 = sn2_s24(SN2_XR(d, d->r[0]));
        int32_t v753 = sn2_s24(SN2_YR(d, d->r[4]));
        d->b    = sn2_asr(d, d->b, 1, true);
        d->x1   = v752;
        d->y1   = v753;
        d->r[0] = sn2_step(d, 0, 1);
    }
    {
        d->a    = sn2_ld(d->y1);
        d->r[1] = sn2_w24(sn2_s24(0x8));
    }
    {
        int32_t v754 = sn2_lim(d->b);
        d->a  = sn2_mac(d, d->a, d->y1, d->x1, true, true);
        d->x0 = v754;
    }
    {
        int32_t v755 = sn2_s24(d->n[2]);
        d->b = sn2_ld(v755);
    }
    {
        int32_t v756 = sn2_s24(SN2_XR(d, d->r[0]));
        int32_t v757 = sn2_lim(d->a);
        d->a    = sn2_mac(d, 0, d->x0, d->y1, false, true);
        d->x1   = v756;
        SN2_YW(d, d->r[4], sn2_w24(v757));
        d->r[0] = sn2_step(d, 0, 1);
        d->r[4] = sn2_step(d, 4, 1);
    }
    {
        int32_t v758 = sn2_s24(SN2_XR(d, d->r[1]));
        {
            uint32_t w = (uint32_t)(d->b >> 24) & 0xFFFFFFu;
            uint32_t o = (w >> 1) & 0xFFFFFFu;
            d->b = sn2_set_hi(d->b, o);
            d->c = (w >> 0) & 1;
            sn2_flags_hi(d, o);
        }
        d->y1   = v758;
        d->r[1] = sn2_step(d, 1, 1);
    }
    {
        if (d->c) {
            d->a = sn2_ld(d->y1);
        }
    }
    {
        int32_t v759 = sn2_s24(SN2_XR(d, d->r[1]));
        {
            uint32_t w = (uint32_t)(d->b >> 24) & 0xFFFFFFu;
            uint32_t o = (w >> 1) & 0xFFFFFFu;
            d->b = sn2_set_hi(d->b, o);
            d->c = (w >> 0) & 1;
            sn2_flags_hi(d, o);
        }
        d->y1   = v759;
        d->r[1] = sn2_step(d, 1, 1);
    }
    {
        if (d->c) {
            d->a = sn2_ld(d->y1);
        }
    }
    {
        int32_t v760 = sn2_lim(d->a);
        int32_t v761 = sn2_lim(d->a);
        d->x0   = v760;
        SN2_YW(d, d->r[4], sn2_w24(v761));
        d->r[4] = sn2_step(d, 4, 1);
    }
    {
        d->a = sn2_mac(d, 0, d->x1, d->x0, false, true);
    }
    {
        int32_t v762 = sn2_lim(d->a);
        SN2_YW(d, d->r[4], sn2_w24(v762));
        d->r[4] = sn2_step(d, 4, 1);
    }
    {
        int32_t v763 = sn2_s24(SN2_XR(d, d->r[2]));
        int32_t v764 = sn2_s24(SN2_YR(d, d->r[4]));
        d->b  = sn2_ld(v763);
        d->y0 = v764;
    }
    {
        int32_t v765 = sn2_s24(SN2_YR(d, 0x1));
        d->y1 = v765;
    }
    {
        int32_t v766 = sn2_lim(d->b);
        d->b  = sn2_sub(d, d->b, sn2_ld(d->y0), true);
        d->x0 = v766;
    }
    {
        int32_t v767 = sn2_s24(SN2_XR(d, d->r[0]));
        int32_t v768 = sn2_s24(SN2_YR(d, d->r[4]));
        d->x1   = v767;
        d->a    = sn2_ld(v768);
        d->r[0] = sn2_step(d, 0, 1);
    }
    {
        int32_t v769 = sn2_lim(d->b);
        d->b  = sn2_mac(d, 0, d->x0, d->y1, false, true);
        d->y0 = v769;
    }
    {
        int32_t v770 = sn2_lim(d->a);
        d->a  = sn2_mac(d, d->a, d->x1, d->y0, false, true);
        d->y1 = v770;
    }
    {
        int32_t v771 = sn2_s24((uint32_t)(d->b & 0xFFFFFF));
        SN2_XW(d, d->r[2], sn2_w24(v771));
        d->r[2] = sn2_step(d, 2, -1);
    }
    {
        int32_t v772 = sn2_lim(d->a);
        d->a    = sn2_mac(d, d->a, d->y1, d->x1, true, true);
        SN2_YW(d, d->r[4], sn2_w24(v772));
        d->r[4] = sn2_step(d, 4, -1);
    }
    {
        int32_t v773 = sn2_s24(SN2_XR(d, d->r[0]));
        d->y0   = v773;
        d->r[0] = sn2_step(d, 0, 1);
    }
    {
        int32_t v774 = sn2_lim(d->a);
        d->y1 = v774;
    }
    {
        int32_t v775 = sn2_s24(SN2_YR(d, d->r[4]));
        d->a    = sn2_mac(d, 0, d->y1, d->y0, false, true);
        d->b    = sn2_ld(v775);
        d->r[4] = sn2_step(d, 4, -1);
    }
    {
        int32_t v776 = sn2_s24(SN2_YR(d, d->r[4]));
        d->y1   = v776;
        d->r[4] = sn2_step(d, 4, -(int32_t)d->n[4]);
    }
    {
        int32_t v777 = sn2_lim(d->a);
        int32_t v778 = sn2_s24(SN2_YR(d, d->r[4]));
        d->x1   = v777;
        d->y0   = v778;
        d->r[4] = sn2_step(d, 4, 1);
    }
    {
        int32_t v779 = sn2_s24(SN2_XR(d, d->r[0]));
        d->a    = sn2_mac(d, 0, d->y1, d->y0, false, true);
        d->y0   = v779;
        d->r[0] = sn2_step(d, 0, 1);
    }
    {
        int32_t v780 = sn2_lim(d->a);
        d->x0 = v780;
    }
    {
        d->b = sn2_mac(d, d->b, d->y0, d->x0, false, true);
    }
    {
        int32_t v781 = sn2_s24(SN2_YR(d, d->r[4]));
        d->b    = sn2_mac(d, d->b, d->y0, d->x0, false, true);
        d->x0   = v781;
        d->r[4] = sn2_step(d, 4, 1);
    }
    {
        d->b    = sn2_add(d, d->b, sn2_ld(d->x0), true);
        d->r[4] = sn2_step(d, 4, 1);
    }
    {
        d->r[4] = sn2_step(d, 4, 1);
    }
    {
        int32_t v782 = sn2_s24(SN2_YR(d, d->r[4]));
        d->x0   = v782;
        d->r[4] = sn2_step(d, 4, 1);
    }
    {
        int32_t v783 = sn2_s24(SN2_XR(d, d->r[0]));
        d->a    = sn2_mac(d, 0, d->x0, d->y1, false, true);
        d->y0   = v783;
        d->r[0] = sn2_step(d, 0, 1);
    }
    {
        int32_t v784 = sn2_lim(d->a);
        d->x0 = v784;
    }
    {
        int32_t v785 = d->y0;
        d->b = sn2_mac(d, d->b, d->y0, d->x0, false, true);
        d->a = sn2_ld(v785);
    }
    {
        int32_t v786 = sn2_s24(SN2_YR(d, d->r[4]));
        d->b    = sn2_mac(d, d->b, d->y0, d->x0, false, true);
        d->x0   = v786;
        d->r[4] = sn2_step(d, 4, (int32_t)d->n[4]);
    }
    {
        d->b    = sn2_add(d, d->b, sn2_ld(d->x0), true);
        d->r[4] = sn2_step(d, 4, 1);
    }
    {
        {
            uint32_t o = ((uint32_t)(d->a >> 24) & (uint32_t)0x1) & 0xFFFFFFu;
            d->a = sn2_set_hi(d->a, o);
            sn2_flags_hi(d, o);
        }
    }
    {
        if (!d->z) {
            d->a = d->b;
        }
    }
    {
        if (!d->z) {
            d->b = sn2_ld(d->x1);
        }
    }
    {
        if (d->z) {
            d->b = sn2_add(d, d->b, sn2_ld(d->x1), false);
        }
    }
    {
        if (d->z) {
            d->b = sn2_asr(d, d->b, 1, false);
        }
    }
    {
        d->a = sn2_asr(d, d->a, 2, true);
    }
    {
        int32_t v787 = sn2_lim(d->a);
        d->sp++;
        d->ss[d->sp] = sn2_w24(v787);
    }
    {
        int32_t v788 = sn2_s24(SN2_YR(d, 0x0));
        d->y1 = v788;
    }
    {
        int32_t v789 = sn2_lim(d->b);
        d->b  = sn2_ext56((d->b < 0) ? -d->b : d->b);
        sn2_flags(d, d->b);
        d->v  = false;
        d->y0 = v789;
    }
    {
        int32_t v790 = sn2_s24(SN2_XR(d, d->r[0]));
        d->b    = sn2_sub(d, d->b, sn2_ld(d->y1), true);
        d->x1   = v790;
        d->r[0] = sn2_step(d, 0, 1);
    }
    {
        int32_t v791 = sn2_s24(SN2_YR(d, 0x15));
        d->r[6] = sn2_w24(v791);
    }
    {
        int32_t v792 = sn2_lim(d->b);
        d->a  = sn2_ld(d->y1);
        d->x0 = v792;
    }
    {
        d->a = sn2_mac(d, d->a, d->x1, d->x0, false, true);
    }
    {
        d->b = sn2_clb(d, d->a);
    }
    {
        d->a = sn2_normf(d, d->a, sn2_hi(d->b));
    }
    {
        d->a = sn2_asr(d, d->a, 13, true);
    }
    {
        int32_t v793 = sn2_s24(SN2_XR(d, d->r[0]));
        d->x1   = v793;
        d->r[0] = sn2_step(d, 0, 1);
    }
    {
        int32_t v794 = sn2_hi(d->a);
        d->a    = sn2_ld(d->y0);
        d->n[6] = sn2_w24(v794);
    }
    {
        d->a = sn2_normf(d, d->a, sn2_hi(d->b));
    }
    {
        int32_t v795 = sn2_s24(SN2_XR(d, d->r[0]));
        d->r[1] = sn2_w24(v795);
        d->r[0] = sn2_step(d, 0, 1);
    }
    {
        int32_t v796 = sn2_lim(d->a);
        d->y0 = v796;
    }
    {
        int32_t v797 = sn2_s24(SN2_YR(d, sn2_step(d, 6, (int32_t)d->n[6])));
        d->b  = sn2_mac(d, 0, d->x1, d->y0, false, true);
        d->x0 = v797;
    }
    {
        int32_t v798 = sn2_s24(SN2_XR(d, d->r[0]));
        d->n[3] = sn2_w24(v798);
        d->r[0] = sn2_step(d, 0, 1);
    }
    {
        int32_t v799 = sn2_lim(d->b);
        d->y0 = v799;
    }
    {
        int32_t v800 = sn2_s24(SN2_XR(d, d->r[0]));
        d->b    = sn2_mac(d, 0, d->y0, d->x0, false, true);
        d->x0   = v800;
        d->r[0] = sn2_step(d, 0, 1);
    }
    {
        d->b = sn2_asl(d, d->b, 1, true);
    }
    {
        int32_t v801 = sn2_s24(SN2_YR(d, 0x11));
        d->r[6] = sn2_w24(v801);
    }
    {
        int32_t v802 = sn2_s24(SN2_XR(d, d->r[0]));
        int32_t v803 = sn2_lim(d->b);
        d->b    = sn2_ld(v802);
        d->y1   = v803;
        d->r[0] = sn2_step(d, 0, 1);
    }
    {
        int32_t v804 = sn2_s24(SN2_XR(d, d->r[1]));
        d->n[2] = sn2_w24(v804);
        d->r[1] = sn2_step(d, 1, 1);
    }
    {
        d->n[1] = sn2_w24(sn2_s24(0x3));
    }
    {
        int32_t v805 = sn2_s24(SN2_YR(d, sn2_step(d, 4, 0x1)));
        d->a = sn2_ld(v805);
    }
    {
        d->a  = sn2_ext56((d->a < 0) ? -d->a : d->a);
        sn2_flags(d, d->a);
        d->v  = false;
        d->x1 = sn2_s24(0x200000);
    }
    {
        d->a = sn2_sub(d, d->a, sn2_ld(d->x1), true);
    }
    {
        if (d->neg) {
            d->a = 0;
        }
    }
    {
        int32_t v806 = sn2_s24(d->n[3]);

        if (d->a >= d->b) {
            d->b = d->a;
            d->c = false;
        } else {
            d->c = true;
        }
        d->x1 = v806;
    }
    {
        int32_t v807 = d->y1;
        d->b    = sn2_sub(d, d->b, sn2_ld(d->x1), true);
        d->n[6] = sn2_w24(v807);
    }
    {
        int32_t v808 = sn2_s24(SN2_YR(d, d->r[4]));
        d->a    = sn2_ld(v808);
        d->r[4] = sn2_step(d, 4, 1);
    }
    {
        int32_t v809 = sn2_lim(d->b);
        int32_t v810 = sn2_s24(SN2_YR(d, d->r[4]));
        SN2_XW(d, d->r[2], sn2_w24(v809));
        d->y0   = v810;
        d->r[4] = sn2_step(d, 4, -1);
    }
    {
        int32_t v811 = sn2_s24(SN2_XR(d, d->r[1]));
        d->a    = sn2_mac(d, d->a, d->y0, d->x0, false, true);
        d->x1   = v811;
        d->r[1] = sn2_step(d, 1, 1);
    }
    {
        int32_t v812 = sn2_s24(SN2_XR(d, d->r[2]));
        d->b  = sn2_mac(d, 0, d->y1, d->x1, false, true);
        d->x1 = v812;
    }
    {
        int32_t v813 = sn2_lim(d->a);
        d->a = sn2_sub(d, d->a, sn2_ld(d->y1), true);
        SN2_YW(d, d->r[4], sn2_w24(v813));
    }
    {
        d->a = sn2_mac(d, d->a, d->x1, d->y0, false, true);
    }
    {
        int32_t v814 = sn2_s24(SN2_XR(d, d->r[1]));
        d->a    = sn2_mac(d, d->a, d->x1, d->y0, false, true);
        d->x1   = v814;
        d->r[1] = sn2_step(d, 1, 1);
    }
    {
        int32_t v815 = sn2_s24(SN2_XR(d, d->r[1]));
        int32_t v816 = sn2_s24(SN2_YR(d, d->r[4]));
        d->b    = sn2_mac(d, d->b, d->x1, d->y0, false, true);
        d->x1   = v815;
        d->y0   = v816;
        d->r[1] = sn2_step(d, 1, 1);
        d->r[4] = sn2_step(d, 4, 1);
    }
    {
        int32_t v817 = sn2_lim(d->a);
        int32_t v818 = sn2_s24(SN2_YR(d, d->r[4]));
        d->b  = sn2_mac(d, d->b, d->x1, d->y0, false, true);
        d->x1 = v817;
        d->a  = sn2_ld(v818);
    }
    {
        int32_t v819 = sn2_s24(SN2_XR(d, d->r[1]));
        d->a    = sn2_mac(d, d->a, d->x1, d->x0, true, true);
        d->y0   = v819;
        d->r[1] = sn2_step(d, 1, -(int32_t)d->n[1]);
    }
    {
        d->b = sn2_mac(d, d->b, d->x1, d->y0, true, true);
    }
    {
        int32_t v820 = sn2_lim(d->a);
        SN2_YW(d, d->r[4], sn2_w24(v820));
        d->r[4] = sn2_step(d, 4, -1);
    }
    {
        int32_t v821 = sn2_s24(SN2_YR(d, d->r[4]));
        d->a    = sn2_ld(v821);
        d->r[4] = sn2_step(d, 4, 1);
    }
    {
        int32_t v822 = sn2_s24(SN2_YR(d, d->r[4]));
        d->y0   = v822;
        d->r[4] = sn2_step(d, 4, -1);
    }
    {
        int32_t v823 = sn2_s24(SN2_XR(d, d->r[1]));
        d->a    = sn2_mac(d, d->a, d->y0, d->x0, false, true);
        d->x1   = v823;
        d->r[1] = sn2_step(d, 1, 1);
    }
    {
        int32_t v824 = sn2_s24(SN2_XR(d, d->r[2]));
        d->b  = sn2_mac(d, d->b, d->y1, d->x1, false, true);
        d->x1 = v824;
    }
    {
        int32_t v825 = sn2_lim(d->a);
        d->a = sn2_sub(d, d->a, sn2_ld(d->y1), true);
        SN2_YW(d, d->r[4], sn2_w24(v825));
    }
    {
        int32_t v826 = sn2_s24(SN2_XR(d, d->r[0]));
        d->a    = sn2_mac(d, d->a, d->x1, d->y0, false, true);
        d->n[3] = sn2_w24(v826);
        d->r[0] = sn2_step(d, 0, 1);
    }
    {
        int32_t v827 = sn2_s24(SN2_XR(d, d->r[1]));
        d->a    = sn2_mac(d, d->a, d->x1, d->y0, false, true);
        d->x1   = v827;
        d->r[1] = sn2_step(d, 1, 1);
    }
    {
        int32_t v828 = sn2_s24(SN2_XR(d, d->r[1]));
        int32_t v829 = sn2_s24(SN2_YR(d, d->r[4]));
        d->b    = sn2_mac(d, d->b, d->x1, d->y0, false, true);
        d->x1   = v828;
        d->y0   = v829;
        d->r[1] = sn2_step(d, 1, 1);
        d->r[4] = sn2_step(d, 4, 1);
    }
    {
        int32_t v830 = sn2_lim(d->a);
        int32_t v831 = sn2_s24(SN2_YR(d, d->r[4]));
        d->b  = sn2_mac(d, d->b, d->x1, d->y0, false, true);
        d->x1 = v830;
        d->a  = sn2_ld(v831);
    }
    {
        int32_t v832 = sn2_s24(SN2_XR(d, d->r[1]));
        d->a    = sn2_mac(d, d->a, d->x1, d->x0, true, true);
        d->y0   = v832;
        d->r[1] = sn2_step(d, 1, 1);
    }
    {
        int32_t v833 = sn2_s24(d->n[6]);
        d->b  = sn2_mac(d, d->b, d->x1, d->y0, true, true);
        d->y1 = v833;
    }
    {
        int32_t v834 = sn2_lim(d->a);
        SN2_YW(d, d->r[4], sn2_w24(v834));
        d->r[4] = sn2_step(d, 4, 1);
    }
    {
        int32_t v835 = sn2_lim(d->b);
        d->n[6] = sn2_w24(v835);
    }
    {
        d->c = (d->n[2] >> 1) & 1;
    }
    {
        if (d->c) {
            d->b = sn2_ld(d->y1);
        }
    }
    {
        int32_t v836 = sn2_s24(SN2_XR(d, d->r[0]));
        d->x0   = v836;
        d->r[0] = sn2_step(d, 0, 1);
    }
    {
        int32_t v837 = sn2_lim(d->b);
        d->y1 = v837;
    }
    {
        int32_t v838 = sn2_s24(SN2_YR(d, sn2_step(d, 4, 0x1)));
        d->a = sn2_ld(v838);
    }
    {
        d->a  = sn2_ext56((d->a < 0) ? -d->a : d->a);
        sn2_flags(d, d->a);
        d->v  = false;
        d->x1 = sn2_s24(0x200000);
    }
    {
        int32_t v839 = sn2_s24(SN2_XR(d, d->r[0]));
        d->a    = sn2_sub(d, d->a, sn2_ld(d->x1), true);
        d->b    = sn2_ld(v839);
        d->r[0] = sn2_step(d, 0, 1);
    }
    {
        if (d->neg) {
            d->a = 0;
        }
    }
    {
        int32_t v840 = sn2_s24(d->n[3]);

        if (d->a >= d->b) {
            d->b = d->a;
            d->c = false;
        } else {
            d->c = true;
        }
        d->x1 = v840;
    }
    {
        d->b = sn2_sub(d, d->b, sn2_ld(d->x1), true);
    }
    {
        int32_t v841 = sn2_s24(SN2_YR(d, d->r[4]));
        d->a    = sn2_ld(v841);
        d->r[4] = sn2_step(d, 4, 1);
    }
    {
        int32_t v842 = sn2_lim(d->b);
        int32_t v843 = sn2_s24(SN2_YR(d, d->r[4]));
        SN2_XW(d, d->r[2], sn2_w24(v842));
        d->y0   = v843;
        d->r[4] = sn2_step(d, 4, -1);
    }
    {
        int32_t v844 = sn2_s24(SN2_XR(d, d->r[1]));
        d->a    = sn2_mac(d, d->a, d->y0, d->x0, false, true);
        d->x1   = v844;
        d->r[1] = sn2_step(d, 1, 1);
    }
    {
        int32_t v845 = sn2_s24(SN2_XR(d, d->r[2]));
        d->b  = sn2_mac(d, 0, d->y1, d->x1, false, true);
        d->x1 = v845;
    }
    {
        int32_t v846 = sn2_lim(d->a);
        d->a = sn2_sub(d, d->a, sn2_ld(d->y1), true);
        SN2_YW(d, d->r[4], sn2_w24(v846));
    }
    {
        d->a = sn2_mac(d, d->a, d->x1, d->y0, false, true);
    }
    {
        int32_t v847 = sn2_s24(SN2_XR(d, d->r[1]));
        d->a    = sn2_mac(d, d->a, d->x1, d->y0, false, true);
        d->x1   = v847;
        d->r[1] = sn2_step(d, 1, 1);
    }
    {
        int32_t v848 = sn2_s24(SN2_XR(d, d->r[1]));
        int32_t v849 = sn2_s24(SN2_YR(d, d->r[4]));
        d->b    = sn2_mac(d, d->b, d->x1, d->y0, false, true);
        d->x1   = v848;
        d->y0   = v849;
        d->r[1] = sn2_step(d, 1, 1);
        d->r[4] = sn2_step(d, 4, 1);
    }
    {
        int32_t v850 = sn2_lim(d->a);
        int32_t v851 = sn2_s24(SN2_YR(d, d->r[4]));
        d->b  = sn2_mac(d, d->b, d->x1, d->y0, false, true);
        d->x1 = v850;
        d->a  = sn2_ld(v851);
    }
    {
        int32_t v852 = sn2_s24(SN2_XR(d, d->r[1]));
        d->a    = sn2_mac(d, d->a, d->x1, d->x0, true, true);
        d->y0   = v852;
        d->r[1] = sn2_step(d, 1, -(int32_t)d->n[1]);
    }
    {
        d->b = sn2_mac(d, d->b, d->x1, d->y0, true, true);
    }
    {
        int32_t v853 = sn2_lim(d->a);
        SN2_YW(d, d->r[4], sn2_w24(v853));
        d->r[4] = sn2_step(d, 4, -1);
    }
    {
        int32_t v854 = sn2_s24(SN2_YR(d, d->r[4]));
        d->a    = sn2_ld(v854);
        d->r[4] = sn2_step(d, 4, 1);
    }
    {
        int32_t v855 = sn2_s24(SN2_YR(d, d->r[4]));
        d->y0   = v855;
        d->r[4] = sn2_step(d, 4, -1);
    }
    {
        int32_t v856 = sn2_s24(SN2_XR(d, d->r[1]));
        d->a    = sn2_mac(d, d->a, d->y0, d->x0, false, true);
        d->x1   = v856;
        d->r[1] = sn2_step(d, 1, 1);
    }
    {
        int32_t v857 = sn2_s24(SN2_XR(d, d->r[2]));
        d->b    = sn2_mac(d, d->b, d->y1, d->x1, false, true);
        d->x1   = v857;
        d->r[2] = sn2_step(d, 2, 1);
    }
    {
        int32_t v858 = sn2_lim(d->a);
        d->a = sn2_sub(d, d->a, sn2_ld(d->y1), true);
        SN2_YW(d, d->r[4], sn2_w24(v858));
    }
    {
        d->a = sn2_mac(d, d->a, d->x1, d->y0, false, true);
    }
    {
        int32_t v859 = sn2_s24(SN2_XR(d, d->r[1]));
        d->a    = sn2_mac(d, d->a, d->x1, d->y0, false, true);
        d->x1   = v859;
        d->r[1] = sn2_step(d, 1, 1);
    }
    {
        int32_t v860 = sn2_s24(SN2_XR(d, d->r[1]));
        int32_t v861 = sn2_s24(SN2_YR(d, d->r[4]));
        d->b    = sn2_mac(d, d->b, d->x1, d->y0, false, true);
        d->x1   = v860;
        d->y0   = v861;
        d->r[1] = sn2_step(d, 1, 1);
        d->r[4] = sn2_step(d, 4, 1);
    }
    {
        int32_t v862 = sn2_lim(d->a);
        int32_t v863 = sn2_s24(SN2_YR(d, d->r[4]));
        d->b  = sn2_mac(d, d->b, d->x1, d->y0, false, true);
        d->x1 = v862;
        d->a  = sn2_ld(v863);
    }
    {
        int32_t v864 = sn2_s24(SN2_XR(d, d->r[1]));
        d->a    = sn2_mac(d, d->a, d->x1, d->x0, true, true);
        d->y0   = v864;
        d->r[1] = sn2_step(d, 1, 1);
    }
    {
        d->b = sn2_mac(d, d->b, d->x1, d->y0, true, true);
    }
    {
        int32_t v865 = sn2_s24(d->n[6]);
        d->x0 = v865;
    }
    {
        d->c = (d->n[2] >> 1) & 1;
    }
    {
        if (d->c) {
            d->b = sn2_add(d, d->b, sn2_ld(d->x0), false);
        }
    }
    {
        int32_t v866 = sn2_s24(SN2_XR(d, d->r[1]));
        int32_t v867 = sn2_lim(d->a);
        d->x0   = v866;
        SN2_YW(d, d->r[4], sn2_w24(v867));
        d->r[1] = sn2_step(d, 1, 1);
        d->r[4] = sn2_step(d, 4, 1);
    }
    {
        int32_t v868 = sn2_lim(d->b);
        d->y0 = v868;
    }
    {
        int32_t v869 = sn2_s24(SN2_XR(d, d->r[1]));
        d->b    = sn2_mac(d, 0, d->y0, d->x0, false, true);
        d->n[1] = sn2_w24(v869);
        d->r[1] = sn2_step(d, 1, 1);
    }
    {
        d->r[1] = sn2_w24(sn2_s24(0x1d));
    }
    {
        int32_t v870 = sn2_lim(d->b);
        d->y1 = v870;
    }
    {
        int32_t v871 = sn2_s24(SN2_YR(d, 0x0));
        d->b  = sn2_mac(d, 0, d->y1, d->y0, false, true);
        d->y1 = v871;
    }
    {
        {
            uint32_t o = ((uint32_t)(d->b >> 24) ^ sn2_w24(d->y1)) & 0xFFFFFFu;
            d->b = sn2_set_hi(d->b, o);
            sn2_flags_hi(d, o);
        }
    }
    {
        d->r[1] = sn2_step(d, 1, (int32_t)d->n[1]);
    }
    {
        int32_t v872 = sn2_lim(d->b);
        d->y1 = v872;
    }
    {
        d->b = sn2_mac(d, 0, d->y1, d->y0, false, true);
    }
    {
        int32_t v873 = sn2_s24(d->ss[d->sp]);
        d->sp--;
        d->x0 = v873;
    }
    {
        d->b = sn2_add(d, d->b, sn2_ld(d->x0), true);
    }
    {
        int32_t v874 = sn2_s24(SN2_XR(d, d->r[0]));
        d->a    = sn2_ld(v874);
        d->r[0] = sn2_step(d, 0, 1);
    }
    {
        int32_t v875 = sn2_s24(SN2_XR(d, d->r[0]));
        d->y1   = v875;
        d->r[0] = sn2_step(d, 0, -1);
    }
    {
        int32_t v876 = sn2_lim(d->b);
        d->a  = sn2_add(d, d->a, sn2_ld(d->y1), true);
        d->x0 = v876;
    }
    {
        int32_t v877 = sn2_s24(SN2_XR(d, d->r[1]));
        d->b = sn2_ld(v877);
    }
    {
        int32_t v878 = sn2_lim(d->a);
        int32_t v879 = sn2_lim(d->a);
        SN2_XW(d, d->r[0], sn2_w24(v878));
        d->y1   = v879;
        d->r[0] = sn2_step(d, 0, 1);
    }
    {
        d->b    = sn2_mac(d, d->b, d->x0, d->y1, false, true);
        d->r[0] = sn2_step(d, 0, 1);
    }
    {
        int32_t v880 = sn2_s24(d->r[5]);
        d->r[4] = sn2_w24(v880);
    }
    {
        int32_t v881 = sn2_lim(d->b);
        SN2_XW(d, d->r[1], sn2_w24(v881));
    }
L_end:;
}
