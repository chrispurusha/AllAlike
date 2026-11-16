/*
 * UltraAlike - a sampler (a placeholder on a sample player's engine), as a VST3 and Audio Unit instrument.
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
// Notes: Docs/code-notes/uaEmuInterp.md - "// notes §k" refers there.

#ifndef __UA_EMU_INTERP_H__
#define __UA_EMU_INTERP_H__

#include <stdint.h>

#define UA_EMU_SETTINGS    (7)

void ua_emu_init(void);                                    // builds the kernel tables; call before the first read (main thread)
int ua_emu_setting(double step, double hostRate);          // notes §2: which kernel a playback ratio uses
double ua_emu_tail(int setting);                           // source frames the kernel reaches either side
void ua_emu_read2(const float * left, const float * right, uint32_t frames, double position, int setting, float * outL, float * outR);

#endif
