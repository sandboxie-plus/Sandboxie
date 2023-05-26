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
// Tokens
//---------------------------------------------------------------------------


#ifndef _MY_TOKEN_H
#define _MY_TOKEN_H


#include "driver.h"


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------


BOOLEAN Token_Init(void);

void *Token_Query(
    void *TokenObject, TOKEN_INFORMATION_CLASS InfoClass, ULONG SessionId);

NTSTATUS Token_QuerySidString(void *TokenObject, UNICODE_STRING *SidString);

BOOLEAN Token_CheckPrivilege(
    void *TokenObject, ULONG PrivilegeLowPart, ULONG SessionId);

void *Token_QueryPrimary(TOKEN_INFORMATION_CLASS InfoClass, ULONG SessionId);

void *Token_Filter(void *TokenObject, ULONG DropRights, ULONG SessionId);

void *Token_Restrict(
    void *TokenObject, ULONG FilterFlags, PROCESS *proc);

NTSTATUS Token_AssignPrimaryHandle(
    void *ProcessObject, HANDLE TokenKernelHandle, ULONG SessionId);

BOOLEAN Token_ReplacePrimary(PROCESS *proc);

void Token_ReleaseProcess(PROCESS *proc);

BOOLEAN Token_ResetPrimary(PROCESS *proc);
ULONG GetThreadTokenOwnerPid();
//---------------------------------------------------------------------------

NTSTATUS Token_Api_Filter(PROCESS *proc, ULONG64 *parms);

//---------------------------------------------------------------------------

#endif // _MY_TOKEN_H
