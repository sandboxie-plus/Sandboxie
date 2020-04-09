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
// Driver API
//---------------------------------------------------------------------------


#ifndef _MY_API_H
#define _MY_API_H


#include "driver.h"
#include "api_defs.h"

// There is a problem with a 32 bit app running in 64 bit Windows.  In a 32 bit app, the process handle passed via one of the API_ARGS structures
// is a HANDLE, which is a VOID*, which is 32 bits.  The 64 bit driver will compare this against NtCurrentProcess(), which is a 64 bit -1.
// So it never matches.  This macro solves that problem.
#define IS_ARG_CURRENT_PROCESS(h) ((ULONG)h == 0xffffffff)      // -1


//---------------------------------------------------------------------------
// Structures and Types
//---------------------------------------------------------------------------


typedef struct _API_WORK_ITEM {

    LIST_ELEM list_elem;
    ULONG length;           // length includes both header and data
    ULONG session_id;
    ULONG type;

    ULONG data[1];

} API_WORK_ITEM;

typedef struct _Sbie_SeFilterTokenArg
{
    PACCESS_TOKEN       ExistingToken;
    ULONG               Flags;
    PTOKEN_GROUPS       SidsToDisable;
    PTOKEN_PRIVILEGES   PrivilegesToDelete;
    PTOKEN_GROUPS       RestrictedSids;
    PACCESS_TOKEN       *NewToken;
    NTSTATUS            *status;
} Sbie_SeFilterTokenArg;

typedef struct _Sbie_SepFilterTokenArg
{
    void*           TokenObject;
    ULONG_PTR       SidCount;
    ULONG_PTR       SidPtr;
    ULONG_PTR       LengthIncrease;
    void            **NewToken;
    NTSTATUS        *status;
} Sbie_SepFilterTokenArg;

//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------


BOOLEAN Api_Init(void);

void Api_Unload(void);

//
// Disable API services in preparation of driver unload
//

BOOLEAN Api_Disable(void);

//
// Adds an API function
//

typedef NTSTATUS(*P_Api_Function)(PROCESS *, ULONG64 *parms);

void Api_SetFunction(ULONG func_code, P_Api_Function func_ptr);

//
// Resets the recorded information for SbieSvc process, when it terminates
//

void Api_ResetServiceProcess(void);

//
// Send a request kernel to the user mode service
//

BOOLEAN Api_SendServiceMessage(ULONG msgid, ULONG data_len, void *data);

//
// Publish WORK_ITEM to be consumed by SandboxieService.  Caller must
// allocate work_item from Driver_Pool, and initialize type, length and data
//

BOOLEAN Api_AddWork(API_WORK_ITEM *work_item);

//
// Copies boxname parameter from user
//

BOOLEAN Api_CopyBoxNameFromUser(
    WCHAR *boxname34, const WCHAR *user_boxname);

//
// Copies SID string parameter from user
//

BOOLEAN Api_CopySidStringFromUser(
    WCHAR *sidstring96, const WCHAR *user_sidstring);

//
// Copies the 'len' bytes from the kernel mode buffer at 'str',
// into the user mode buffer specified by 'uni', and updates uni->Length.
// May raise STATUS_BUFFER_TOO_SMALL or STATUS_ACCESS_VIOLATION
//

void Api_CopyStringToUser(
    UNICODE_STRING64 *uni, WCHAR *str, size_t len);

NTSTATUS Sbie_SepFilterTokenHandler(
    void*       TokenObject,
    ULONG_PTR   SidCount,
    ULONG_PTR   SidPtr,
    ULONG_PTR   LengthIncrease,
    void        **NewToken);

//---------------------------------------------------------------------------
// Variables
//---------------------------------------------------------------------------


extern volatile HANDLE Api_ServiceProcessId;


//---------------------------------------------------------------------------


#endif // _MY_API_H
