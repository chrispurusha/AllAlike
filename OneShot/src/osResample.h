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
// Notes: Docs/code-notes/osResample.md - "// notes §k" refers there.

#ifndef __OS_RESAMPLE_H__
#define __OS_RESAMPLE_H__

#include <stdint.h>

#define OS_RESAMPLE_HALF    (32)    // taps each side at full bandwidth

void os_resample_init(void);    // builds the kernel table; call before the first read (main thread)

// notes §1: the sample d at a fractional position, band-limited to cutoff (1 = the file's own Nyquist; below 1 when
// the sample plays faster than its rate allows). Past either end reads as silence.
float os_resample_read(const float * d, uint32_t frames, double position, double cutoff);

// notes §5: both channels at once, from one set of weights; right may be NULL (mono: *outR = *outL)
void os_resample_read2(const float * left, const float * right, uint32_t frames, double position, double cutoff, float * outL, float * outR);

// What a playback step needs: 1 up to the file's own speed, then 1/step, never below the narrowest the table serves
double os_resample_cutoff(double step);

// How far past the last frame the filter still rings, in frames, at that cutoff
double os_resample_tail(double cutoff);

#endif // __OS_RESAMPLE_H__
