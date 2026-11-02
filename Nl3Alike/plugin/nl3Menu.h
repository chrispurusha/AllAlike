/*
 * Nl3Alike - the menu bar.
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
// Notes: Docs/code-notes/nl3Menu.md - "// notes §k" refers there.

#ifndef __TI_MENU_H__
#define __TI_MENU_H__

#include "menuBar.h"

#define NL3_MENU_BAR_H    (20.0)

const tMenuBarItem * nl3_menu_bar(void);
tRectangle nl3_menu_bar_rect(void);

#endif // __TI_MENU_H__
