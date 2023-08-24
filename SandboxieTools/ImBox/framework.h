// header.h : include file for standard system include files,
// or project specific include files
//

#pragma once

#include "targetver.h"

#include <ntstatus.h>

#define WIN32_NO_STATUS
typedef long NTSTATUS;

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files
#include <windows.h>
#include <winternl.h>
#include <winioctl.h>

// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

extern "C" {
	NTSYSCALLAPI NTSTATUS NTAPI NtSetEvent(HANDLE EventHandle, PLONG PreviousState);
	NTSYSCALLAPI NTSTATUS NTAPI NtSignalAndWaitForSingleObject(IN HANDLE SignalHandle, IN HANDLE WaitHandle, IN BOOLEAN Alertable, IN PLARGE_INTEGER Timeout OPTIONAL);
#define NtCurrentProcess() ( (HANDLE)(LONG_PTR) -1 )
	NTSYSCALLAPI NTSTATUS NTAPI NtAllocateVirtualMemory(HANDLE ProcessHandle, PVOID* BaseAddress, ULONG_PTR ZeroBits, PSIZE_T RegionSize, ULONG AllocationType, ULONG Protect);
	NTSYSCALLAPI NTSTATUS NTAPI NtFreeVirtualMemory(HANDLE ProcessHandle, PVOID* BaseAddress, PSIZE_T RegionSize, ULONG FreeType);
}


// std includes
#include <string>
#include <sstream>
#include <deque>
#include <list>
#include <vector>
#include <map>
#include <set>
#include <memory>
#include <locale>
#include <codecvt>
#include <algorithm>