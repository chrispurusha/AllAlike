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

// Notes: Docs/code-notes/sn2Fixed.md - "// notes §k" refers there.

#ifndef __SN2_FIXED_CODE_H__
#define __SN2_FIXED_CODE_H__

#include "sn2Dsp.h"

void sn2_fixed_head(tSn2Dsp * d);
void sn2_fixed_pass(tSn2Dsp * d);

#endif // __SN2_FIXED_CODE_H__
