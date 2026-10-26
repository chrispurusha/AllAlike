/*
 * TiAlike - the oscillator.
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
// Notes: Docs/code-notes/tiOsc.md - "// notes §k" refers there.

#ifndef __TI_OSC_H__
#define __TI_OSC_H__

typedef enum {
    eTiOscSaw = 0,
    eTiOscPulse,
    eTiOscTriangle,
    eTiOscShapes
} tTiOscShape;

// One sample at phase 0..1 (advanced by the caller); width is the pulse's, 0..1
double ti_osc_sample(tTiOscShape shape, double phase, double increment, double width);

#endif // __TI_OSC_H__
