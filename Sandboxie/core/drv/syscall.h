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
// Syscall Management
//---------------------------------------------------------------------------


#ifndef _MY_SYSCALL_H
#define _MY_SYSCALL_H


#include "driver.h"


//---------------------------------------------------------------------------
// Structures and Types
//---------------------------------------------------------------------------


struct _SYSCALL_ENTRY;
typedef struct _SYSCALL_ENTRY SYSCALL_ENTRY;


typedef NTSTATUS (*P_Syscall_Handler1)(
    PROCESS *proc, SYSCALL_ENTRY *syscall_entry, ULONG_PTR *user_args);


typedef NTSTATUS (*P_Syscall_Handler2)(
    PROCESS *proc, void *Object, UNICODE_STRING *Name,
    ACCESS_MASK GrantedAccess);

typedef BOOLEAN (*P_Syscall_Handler3_Support_Procmon_Stack)(
    PROCESS *proc, SYSCALL_ENTRY *syscall_entry, ULONG_PTR *user_args);


struct _SYSCALL_ENTRY {

    LIST_ELEM list_elem;
    USHORT syscall_index;
    USHORT param_count;
    ULONG ntdll_offset;
    void *ntos_func;
    P_Syscall_Handler1 handler1_func;
    P_Syscall_Handler2 handler2_func;
    P_Syscall_Handler3_Support_Procmon_Stack handler3_func_support_procmon;
    USHORT name_len;
    UCHAR name[1];

};


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------


void *Syscall_GetServiceTable(void);

BOOLEAN Syscall_Init(void);

SYSCALL_ENTRY *Syscall_GetByName(const UCHAR *name);

BOOLEAN Syscall_Set1(const UCHAR *name, P_Syscall_Handler1 handler_func);

BOOLEAN Syscall_Set2(const UCHAR *name, P_Syscall_Handler2 handler_func);

BOOLEAN Syscall_Set3(const UCHAR *name, P_Syscall_Handler3_Support_Procmon_Stack handler_func);

NTSTATUS Syscall_Invoke(SYSCALL_ENTRY *entry, ULONG_PTR *stack);


//---------------------------------------------------------------------------


#endif // _MY_SYSCALL_H
