/*
 * Copyright 2004-2020 Sandboxie Holdings, LLC 
 * Copyright 2020-2023 David Xanatos, xanasoft.com
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
// Utilities
//---------------------------------------------------------------------------


#ifndef _MY_UTIL_H
#define _MY_UTIL_H


#include "my_winnt.h"


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------


//
// Returns TRUE if current process is running as LocalSystem account
//

BOOLEAN MyIsCurrentProcessRunningAsLocalSystem(void);


//
// return TRUE is specified process is running with one of the system
// account S-1-5-18, S-1-5-19, S-1-5-20
//


BOOLEAN MyIsProcessRunningAsSystemAccount(HANDLE ProcessId);


//
// return TRUE if current process is our helper service process
//


BOOLEAN MyIsCallerMyServiceProcess(void);


//
// Returns the Terminal Services session id for the current process
//

NTSTATUS MyGetSessionId(ULONG *SessionId);


//
// Returns the parent process id for the current process
//

NTSTATUS MyGetParentId(HANDLE *ParentId);


//
// Enable/disable write protection in kernel mode (util.asm)
//

void DisableWriteProtect(void);
void EnableWriteProtect(void);


//
// Issue CPUID instruction
//

void InvokeCPUID(ULONG Type, ULONG Info[4]);

WCHAR *SearchUnicodeString(PCUNICODE_STRING pString1, PWCHAR pString2, BOOLEAN boolCaseInSensitive);
BOOLEAN UnicodeStringStartsWith(PCUNICODE_STRING pString1, PWCHAR pString2, BOOLEAN boolCaseInSensitive);
BOOLEAN UnicodeStringEndsWith(PCUNICODE_STRING pString1, PWCHAR pString2, BOOLEAN boolCaseInSensitive);
BOOLEAN DoesRegValueExist(ULONG RelativeTo, WCHAR *Path, WCHAR *ValueName);
NTSTATUS GetRegString(ULONG RelativeTo, const WCHAR *Path, const WCHAR *ValueName, UNICODE_STRING* pData);
void *memmem(const void *pSearchBuf, size_t nBufSize, const void *pPattern, size_t nPatternSize);



//
// return TRUE if current process has a valid custom signature
//

BOOLEAN MyIsCallerSigned(void);


NTSTATUS MyValidateCertificate(void);

//
// misc helpers
//

HANDLE Util_GetProcessPidByName(const WCHAR* name);

BOOLEAN Util_IsCsrssProcess(HANDLE pid);

BOOLEAN Util_IsProtectedProcess(HANDLE pid);

LARGE_INTEGER Util_GetTimestamp(void);


// Sensible limit that may or may not correspond to the actual Windows value.
#define MAX_STACK_DEPTH 256

#define RTL_WALK_USER_MODE_STACK 0x00000001
#define RTL_WALK_VALID_FLAGS 0x00000001

ULONG Util_CaptureStack(_Out_ PVOID* Frames, _In_ ULONG Count);

//---------------------------------------------------------------------------


#endif // _MY_UTIL_H
