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
// Event Services
//---------------------------------------------------------------------------


#include "dll.h"


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------


static BOOLEAN Event_EvtIntAssertConfig(
    ULONG_PTR Unknown1, ULONG_PTR Unknown2, ULONG_PTR Unknown3);


//---------------------------------------------------------------------------


typedef BOOL (*P_EvtIntAssertConfig)(
    ULONG_PTR Unknown1, ULONG_PTR Unknown2, ULONG_PTR Unknown3);



//---------------------------------------------------------------------------
// Event_EvtIntAssertConfig
//---------------------------------------------------------------------------


ALIGNED BOOLEAN Event_EvtIntAssertConfig(
    ULONG_PTR Unknown1, ULONG_PTR Unknown2, ULONG_PTR Unknown3)
{
    SetLastError(0);
    return TRUE;
}


//---------------------------------------------------------------------------
// EvtApi_Init
//---------------------------------------------------------------------------


ALIGNED BOOLEAN EvtApi_Init(HMODULE module)
{
    P_EvtIntAssertConfig EvtIntAssertConfig;
    ULONG_PTR __sys_EvtIntAssertConfig = 0;

    EvtIntAssertConfig = (P_EvtIntAssertConfig)
        GetProcAddress(module, "EvtIntAssertConfig");

    SBIEDLL_HOOK(Event_,EvtIntAssertConfig);

    return TRUE;
}
