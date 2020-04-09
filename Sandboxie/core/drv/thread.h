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
// Thread Management
//---------------------------------------------------------------------------


#ifndef _MY_THREAD_H
#define _MY_THREAD_H


#include "process.h"


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------


BOOLEAN Thread_Init(void);

void Thread_Unload(void);

BOOLEAN Thread_InitProcess(PROCESS *proc);

void Thread_ReleaseProcess(PROCESS *proc);

BOOLEAN Thread_AdjustGrantedAccess(void);

void Thread_SetThreadToken(PROCESS *proc);

NTSTATUS Thread_StoreThreadToken(PROCESS *proc);

#define Thread_ClearThreadToken()                                   \
            PsImpersonateClient(PsGetCurrentThread(), NULL,         \
            FALSE, FALSE, SecurityAnonymous);

NTSTATUS Thread_CheckTokenObject(
    PROCESS *proc, void *Object, ACCESS_MASK GrantedAccess);


//---------------------------------------------------------------------------


#endif // _MY_THREAD_H
