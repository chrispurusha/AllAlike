// ModAlike - its preferences: how a drag turns a dial. GPLv3, see LICENSE.
#ifndef __MA_PREFS_H__
#define __MA_PREFS_H__

#include "synthlibTypes.h"

#define MA_PREF_DIAL_MODE    "dialMode"

void ma_prefs_init(void);    // once per process; restores the drag mode

// the plug-in's own, since SynthLib's plug-in stubs keep no dial mode
tDialMode ma_dial_mode(void);
void ma_set_dial_mode(tDialMode mode);    // and saves it

#endif // __MA_PREFS_H__
