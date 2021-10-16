/*
 * Copyright 2021 David Xanatos, xanasoft.com 
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
// Windows Filtering Platform
//---------------------------------------------------------------------------


#ifndef _MY_WFP_H
#define _MY_WFP_H


#include "driver.h"


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------


BOOLEAN WFP_Init(void);
BOOLEAN WFP_Load(void);
void WFP_Unload(void);

BOOLEAN WFP_InitProcess(PROCESS *proc);
BOOLEAN WFP_UpdateProcess(PROCESS *proc);
void WFP_DeleteProcess(PROCESS *proc);


//---------------------------------------------------------------------------


#endif // _MY_WFP_H
