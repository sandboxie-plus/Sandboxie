/*
 * Copyright 2021-2022 David Xanatos, xanasoft.com
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
// Handle
//---------------------------------------------------------------------------

#define NOGDI
#include "dll.h"
#include "handle.h"
#include <stdio.h>
#include "debug.h"

#include "common/pool.h"
#include "common/map.h"


//---------------------------------------------------------------------------
// Structures and Types
//---------------------------------------------------------------------------

typedef struct _HANDLE_STATE {

    BOOLEAN DeleteOnClose;
    P_CloseHandler CloseHandlers[MAX_CLOSE_HANDLERS];
    WCHAR* RelocationPath;

} HANDLE_STATE;


//---------------------------------------------------------------------------
// Variables
//---------------------------------------------------------------------------

extern POOL* Dll_Pool;

static HASH_MAP Handle_StatusData;
static CRITICAL_SECTION Handle_StatusData_CritSec;


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
// Handle_Init
//---------------------------------------------------------------------------


_FX BOOLEAN Handle_Init(void)
{
    InitializeCriticalSection(&Handle_StatusData_CritSec);
    map_init(&Handle_StatusData, Dll_Pool);

    return TRUE;
}


//---------------------------------------------------------------------------
// Handle_SetDeleteOnClose
//---------------------------------------------------------------------------


_FX VOID Handle_SetDeleteOnClose(HANDLE FileHandle, BOOLEAN DeleteOnClose)
{
    EnterCriticalSection(&Handle_StatusData_CritSec);

    HANDLE_STATE* state = map_get(&Handle_StatusData, FileHandle);
    if (!state) {
        state = map_insert(&Handle_StatusData, FileHandle, NULL, sizeof(HANDLE_STATE));
    }

    state->DeleteOnClose = DeleteOnClose;

    LeaveCriticalSection(&Handle_StatusData_CritSec);
}


//---------------------------------------------------------------------------
// Handle_SetRelocationPath
//---------------------------------------------------------------------------


_FX VOID Handle_SetRelocationPath(HANDLE FileHandle, WCHAR* RelocationPath)
{
    EnterCriticalSection(&Handle_StatusData_CritSec);

    HANDLE_STATE* state = map_get(&Handle_StatusData, FileHandle);
    if (!state) { // this should always be the case, as we only use Handle_SetRelocationPath when the handle is first created
        state = map_insert(&Handle_StatusData, FileHandle, NULL, sizeof(HANDLE_STATE));
    } else  if (state->RelocationPath) Dll_Free(state->RelocationPath); // should not happen but in case

    state->RelocationPath = Dll_Alloc((wcslen(RelocationPath) + 1) * sizeof(WCHAR));
    wcscpy(state->RelocationPath, RelocationPath);

    LeaveCriticalSection(&Handle_StatusData_CritSec);
}


//---------------------------------------------------------------------------
// Handle_GetRelocationPath
//---------------------------------------------------------------------------


_FX WCHAR* Handle_GetRelocationPath(HANDLE FileHandle, ULONG ExtraLength)
{
    WCHAR* name = NULL;

    EnterCriticalSection(&Handle_StatusData_CritSec);

    HANDLE_STATE* state = map_get(&Handle_StatusData, FileHandle);
    if (state && state->RelocationPath) {
           
        THREAD_DATA *TlsData = Dll_GetTlsData(NULL);

        ULONG length = (wcslen(state->RelocationPath) + 1) * sizeof(WCHAR);
        name = Dll_GetTlsNameBuffer(TlsData, TRUE_NAME_BUFFER, length + ExtraLength);
        wcscpy(name, state->RelocationPath);
    }

    LeaveCriticalSection(&Handle_StatusData_CritSec);

    return name;
}


//---------------------------------------------------------------------------
// Handle_FreeCloseHandler
//---------------------------------------------------------------------------


_FX BOOLEAN Handle_FreeCloseHandler(HANDLE FileHandle, P_CloseHandler* CloseHandlers, BOOLEAN* DeleteOnClose)
{
    BOOLEAN HasCloseHandlers = FALSE;

    EnterCriticalSection(&Handle_StatusData_CritSec);

    HANDLE_STATE* state = (HANDLE_STATE*)map_get(&Handle_StatusData, FileHandle);
    if (state) {

        HasCloseHandlers = TRUE;

        if(CloseHandlers) 
            memcpy(CloseHandlers, state->CloseHandlers, MAX_CLOSE_HANDLERS * sizeof(P_CloseHandler));
        if(DeleteOnClose) *DeleteOnClose = state->DeleteOnClose;

        if (state->RelocationPath) Dll_Free(state->RelocationPath);
    }

    map_remove(&Handle_StatusData, FileHandle);

    LeaveCriticalSection(&Handle_StatusData_CritSec);

    return HasCloseHandlers;
}


//---------------------------------------------------------------------------
// Handle_RegisterCloseHandler
//---------------------------------------------------------------------------


_FX BOOLEAN Handle_RegisterCloseHandler(HANDLE FileHandle, P_CloseHandler CloseHandler)
{
    if (!FileHandle || FileHandle == (HANDLE)-1)
        return FALSE;

    ULONG i;

    EnterCriticalSection(&Handle_StatusData_CritSec);

    HANDLE_STATE* state = map_get(&Handle_StatusData, FileHandle);
    if (!state) {
        state = map_insert(&Handle_StatusData, FileHandle, NULL, sizeof(HANDLE_STATE));
    }

    for (i = 0; i < MAX_CLOSE_HANDLERS; i++) {
        if (state->CloseHandlers[i] == CloseHandler)
            break; // already registered
        if (state->CloseHandlers[i] == NULL) {
            state->CloseHandlers[i] = CloseHandler; // set to empty slot
            break;
        }
    }

    LeaveCriticalSection(&Handle_StatusData_CritSec);

    if (i == MAX_CLOSE_HANDLERS) {
        SbieApi_Log(2301, L"No free CloseHandlers slot available");
        return FALSE;
    }

    return TRUE;
}


//---------------------------------------------------------------------------
// Handle_UnRegisterCloseHandler
//---------------------------------------------------------------------------


_FX BOOLEAN Handle_UnRegisterCloseHandler(HANDLE FileHandle, P_CloseHandler CloseHandler)
{
    ULONG i = MAX_CLOSE_HANDLERS;

    EnterCriticalSection(&Handle_StatusData_CritSec);

    HANDLE_STATE* state = map_get(&Handle_StatusData, FileHandle);
    if (state) {

        for (i = 0; i < MAX_CLOSE_HANDLERS; i++) {
            if (state->CloseHandlers[i] == CloseHandler) {
                state->CloseHandlers[i] = NULL; // clear slot
                break;
            }
        }
    }

    LeaveCriticalSection(&Handle_StatusData_CritSec);

    return i != MAX_CLOSE_HANDLERS;
}


//---------------------------------------------------------------------------
// Handle_SetupDuplicate
//---------------------------------------------------------------------------


_FX void Handle_SetupDuplicate(HANDLE OldFileHandle, HANDLE NewFileHandle)
{
    ULONG i;

    EnterCriticalSection(&Handle_StatusData_CritSec);

    HANDLE_STATE* state = map_get(&Handle_StatusData, OldFileHandle);
    if (state) {

        if(state->RelocationPath)
            Handle_SetRelocationPath(NewFileHandle, state->RelocationPath);

        // todo: add a flag to each CloseHandlers entry to indicate if it should be propagated or not
        BOOLEAN found = FALSE;
        for (i = 0; i < MAX_CLOSE_HANDLERS; i++) {
            if (state->CloseHandlers[i] == File_NotifyRecover) {
                found = TRUE;
                break;
            }
        }
        if(found)
            Handle_RegisterCloseHandler(NewFileHandle, File_NotifyRecover);
    }

    LeaveCriticalSection(&Handle_StatusData_CritSec);
}
