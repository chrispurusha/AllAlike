/*
 * TiAlike - the parameters and the panel pages.
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
// Notes: Docs/code-notes/tiParams.md - "// notes §k" refers there.

#ifndef __TI_PARAMS_H__
#define __TI_PARAMS_H__

#include "synthlibPlugin.h"
#include "tiDraw.h"

// APPENDED TO, NEVER INSERTED INTO: the ids are what host projects save
enum {
    kTiParamVoiceLimit = 0,
    kTiParamVolume,
    kTiParamOscShape,
    kTiParamPulseWidth,
    kTiParamCutoff,
    kTiParamResonance,
    kTiParamFilterMode,
    kTiParamCutoff2,
    kTiParamResonance2,
    kTiParamFilter2Mode,
    kTiParamRouting,
    kTiParamBalance,
    kTiParamFilterLink,
    kTiParamCount
};

extern const tSynthLibParam gTiParams[kTiParamCount];
extern const tTiPage        gTiPages[];
extern const uint32_t       gTiPageCount;

#endif // __TI_PARAMS_H__
