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
// Defines
//---------------------------------------------------------------------------


#define PROCESS_DENIED_ACCESS_MASK                              \
        ~(  STANDARD_RIGHTS_READ | SYNCHRONIZE |                \
            PROCESS_VM_READ | PROCESS_QUERY_INFORMATION |       \
            PROCESS_QUERY_LIMITED_INFORMATION )

#define THREAD_DENIED_ACCESS_MASK                               \
        ~(  STANDARD_RIGHTS_READ | SYNCHRONIZE |                \
            THREAD_GET_CONTEXT | THREAD_QUERY_INFORMATION |     \
            THREAD_QUERY_LIMITED_INFORMATION )


//---------------------------------------------------------------------------
// Structures and Types
//---------------------------------------------------------------------------


struct _THREAD {

#ifndef USE_PROCESS_MAP
    LIST_ELEM list_elem;
#endif 

    HANDLE tid;

    void *token_object;
    BOOLEAN token_CopyOnOpen;
    BOOLEAN token_EffectiveOnly;
    SECURITY_IMPERSONATION_LEVEL token_ImpersonationLevel;

};

//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------


BOOLEAN Thread_Init(void);

void Thread_Unload(void);

BOOLEAN Thread_InitProcess(PROCESS *proc);

void Thread_ReleaseProcess(PROCESS *proc);

#ifdef XP_SUPPORT
BOOLEAN Thread_AdjustGrantedAccess(void);
#endif

void Thread_SetThreadToken(PROCESS *proc);

NTSTATUS Thread_StoreThreadToken(PROCESS *proc);

#define Thread_ClearThreadToken()                                   \
            PsImpersonateClient(PsGetCurrentThread(), NULL,         \
            FALSE, FALSE, SecurityAnonymous);

NTSTATUS Thread_CheckTokenObject(
    PROCESS *proc, void *Object, ACCESS_MASK GrantedAccess);

THREAD *Thread_GetByThreadId(PROCESS *proc, HANDLE tid);

NTSTATUS Thread_CheckObject_Common(
    PROCESS *proc, PEPROCESS ProcessObject,
    ACCESS_MASK GrantedAccess, BOOLEAN EntireProcess,
    BOOLEAN ExplicitAccess);

//---------------------------------------------------------------------------


#endif // _MY_THREAD_H
