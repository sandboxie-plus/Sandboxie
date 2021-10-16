/*
 * Copyright 2004-2020 Sandboxie Holdings, LLC 
 *
 * This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

//---------------------------------------------------------------------------
// GUI Services
//---------------------------------------------------------------------------


#ifndef _MY_GUI_H
#define _MY_GUI_H


#include "driver.h"


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------


BOOLEAN Gui_Init(void);

#ifdef XP_SUPPORT
void Gui_Unload(void);
#endif

BOOLEAN Gui_InitProcess(PROCESS *proc);

void Gui_Check_OpenWinClass(PROCESS *proc);


//---------------------------------------------------------------------------


#endif // _MY_GUI_H
