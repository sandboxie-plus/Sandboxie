/*
 * Copyright 2021-2023 David Xanatos, xanasoft.com
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


typedef struct _HANDLE_HANDLER
{
	LIST_ELEM list_elem;

	P_HandlerFunc	Close;
	void*			Param;

	BOOL			bPropagate; // incompatible with Param, todo: add duplicate handler

} HANDLE_HANDLER;


typedef struct _HANDLE_STATE {

    BOOLEAN DeleteOnClose;
    LIST    CloseHandlers;
    WCHAR*  RelocationPath;

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


_FX VOID Handle_ExecuteCloseHandler(HANDLE FileHandle, BOOLEAN* DeleteOnClose)
{
    LIST CloseHandlers;

    EnterCriticalSection(&Handle_StatusData_CritSec);

    HANDLE_STATE* state = (HANDLE_STATE*)map_get(&Handle_StatusData, FileHandle);
    if (state) {

        *DeleteOnClose = state->DeleteOnClose;
        CloseHandlers = state->CloseHandlers;
        if (state->RelocationPath) Dll_Free(state->RelocationPath);
    }

    map_remove(&Handle_StatusData, FileHandle);

    LeaveCriticalSection(&Handle_StatusData_CritSec);

    //
    // execute all close handlers
    //

    if (state) {
        while (1) {
            HANDLE_HANDLER* handler = List_Head(&CloseHandlers);
            if (!handler)
                break;
            handler->Close(FileHandle, handler->Param);
            List_Remove(&CloseHandlers, handler);
            Pool_Free(handler, sizeof(HANDLE_HANDLER));
        }
    }
}


//---------------------------------------------------------------------------
// Handle_RegisterHandler
//---------------------------------------------------------------------------


_FX BOOLEAN Handle_RegisterHandler(HANDLE FileHandle, P_HandlerFunc CloseHandler, void* Params, BOOL bPropagate)
{
    if (!FileHandle || FileHandle == (HANDLE)-1)
        return FALSE;

    EnterCriticalSection(&Handle_StatusData_CritSec);

    HANDLE_STATE* state = map_get(&Handle_StatusData, FileHandle);
    if (!state) {
        state = map_insert(&Handle_StatusData, FileHandle, NULL, sizeof(HANDLE_STATE));
    }

    HANDLE_HANDLER* handler = List_Head(&state->CloseHandlers);
    while (handler) 
    {
        if (handler->Close == CloseHandler)
            break; // already registered
        handler = List_Next(handler);
    }

    if (handler == NULL) 
    {
        HANDLE_HANDLER* newNandler = Pool_Alloc(Dll_Pool, sizeof(HANDLE_HANDLER));
        memzero(&newNandler->list_elem, sizeof(LIST_ELEM));

        newNandler->Close = CloseHandler;
        newNandler->Param = Params;
        newNandler->bPropagate = bPropagate;

        List_Insert_After(&state->CloseHandlers, NULL, newNandler);
    }

    LeaveCriticalSection(&Handle_StatusData_CritSec);

    if (handler != NULL) {
        //SbieApi_Log(2301, L"CloseHandlers already registered"); // todo
        return FALSE;
    }

    return TRUE;
}


//---------------------------------------------------------------------------
// Handle_UnRegisterHandler
//---------------------------------------------------------------------------


_FX VOID Handle_UnRegisterHandler(HANDLE FileHandle, P_HandlerFunc CloseHandler, void** pParams)
{
    EnterCriticalSection(&Handle_StatusData_CritSec);

    HANDLE_STATE* state = map_get(&Handle_StatusData, FileHandle);
    if (state) {

        HANDLE_HANDLER* handler = List_Head(&state->CloseHandlers);
        while (handler) 
        {
            if (handler->Close == CloseHandler)
            {
                if (pParams) pParams = handler->Param;
                List_Remove(&state->CloseHandlers, handler);
                break;
            }
            handler = List_Next(handler);
        }
    }

    LeaveCriticalSection(&Handle_StatusData_CritSec);
}


//---------------------------------------------------------------------------
// Handle_SetupDuplicate
//---------------------------------------------------------------------------


_FX void Handle_SetupDuplicate(HANDLE OldFileHandle, HANDLE NewFileHandle)
{
    EnterCriticalSection(&Handle_StatusData_CritSec);

    HANDLE_STATE* state = map_get(&Handle_StatusData, OldFileHandle);
    if (state) {

        if(state->RelocationPath)
            Handle_SetRelocationPath(NewFileHandle, state->RelocationPath);

        HANDLE_HANDLER* handler = List_Head(&state->CloseHandlers);
        while (handler) 
        {
            if (handler->bPropagate) {
                Handle_RegisterHandler(NewFileHandle, handler->Close, NULL, TRUE);
                break;
            }
            handler = List_Next(handler);
        }
    }

    LeaveCriticalSection(&Handle_StatusData_CritSec);
}