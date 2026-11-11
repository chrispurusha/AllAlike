/*
 * OneShot - a one-sample player, as a VST3 and Audio Unit instrument.
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
// Notes: Docs/code-notes/osStretch.md - "// notes §k" refers there.

#ifndef __OS_STRETCH_H__
#define __OS_STRETCH_H__

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// notes §1: Rubber Band, offline. in[] holds inFrames at inRate (in[1] NULL for mono); out[] receives exactly
// outFrames (out[1] NULL for mono). pitchScale multiplies every frequency as heard once out is played at outRate.
// Worker thread only: it allocates, and takes far longer than a block.
// cancelled is asked between blocks; true abandons the render (and returns false).
bool os_stretch_render(const float * left, const float * right, uint32_t inFrames, double inRate, double outRate, double pitchScale, float * outLeft, float * outRight, uint32_t outFrames, bool (*cancelled)(void * user), void * user);

#ifdef __cplusplus
}
#endif

#endif // __OS_STRETCH_H__
