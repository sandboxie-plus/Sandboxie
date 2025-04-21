/*
 * Copyright 2021-2024 David Xanatos, xanasoft.com
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
// Kernel
//---------------------------------------------------------------------------

//#define NOGDI
//#include <windows.h>
//#include "common/win32_ntddk.h"
#include "dll.h"
#include "obj.h"
#include <wchar.h>

#include "common/pool.h"
#include "common/map.h"

#define CONF_LINE_LEN               2000    // keep in sync with drv/conf.c

//---------------------------------------------------------------------------
// Functions Prototypes
//---------------------------------------------------------------------------

typedef LPWSTR (*P_GetCommandLineW)(VOID);

typedef LPSTR (*P_GetCommandLineA)(VOID);

typedef EXECUTION_STATE (*P_SetThreadExecutionState)(EXECUTION_STATE esFlags);

typedef DWORD(*P_GetTickCount)();

typedef ULONGLONG (*P_GetTickCount64)();

typedef BOOL(*P_QueryUnbiasedInterruptTime)(PULONGLONG UnbiasedTime);

//typedef void(*P_Sleep)(DWORD dwMiSecond);

typedef DWORD(*P_SleepEx)(DWORD dwMiSecond, BOOL bAlert);

typedef BOOL (*P_QueryPerformanceCounter)(LARGE_INTEGER* lpPerformanceCount);


typedef LANGID (*P_GetUserDefaultUILanguage)();

typedef int (*P_GetUserDefaultLocaleName)(LPWSTR lpLocaleName, int cchLocaleName);

typedef int (*LCIDToLocaleName)(LCID Locale, LPWSTR lpName, int cchName, DWORD dwFlags);

typedef LCID (*P_GetUserDefaultLCID)();

typedef LANGID (*P_GetUserDefaultLangID)();

typedef int (*P_GetUserDefaultGeoName)(LPWSTR geoName, int geoNameCount);

typedef LANGID (*P_GetSystemDefaultUILanguage)();

typedef int (*P_GetSystemDefaultLocaleName)(LPWSTR lpLocaleName, int cchLocaleName);

typedef LCID (*P_GetSystemDefaultLCID)();

typedef LANGID (*P_GetSystemDefaultLangID)();

typedef BOOL (*P_GetVolumeInformationByHandleW)(HANDLE hFile, LPWSTR lpVolumeNameBuffer, DWORD nVolumeNameSize, LPDWORD lpVolumeSerialNumber,LPDWORD lpMaximumComponentLength, LPDWORD lpFileSystemFlags, LPWSTR  lpFileSystemNameBuffer, DWORD nFileSystemNameSize);

//typedef int (*P_GetLocaleInfoEx)(LPCWSTR lpLocaleName, LCTYPE LCType, LPWSTR lpLCData, int cchData);

//typedef int (*P_GetLocaleInfoA)(LCID Locale, LCTYPE LCType, LPSTR lpLCData, int cchData);

//typedef int (*P_GetLocaleInfoW)(LCID Locale, LCTYPE LCType, LPWSTR lpLCData, int cchData);


//---------------------------------------------------------------------------
// Variables
//---------------------------------------------------------------------------


P_GetCommandLineW				__sys_GetCommandLineW				= NULL;
P_GetCommandLineA				__sys_GetCommandLineA				= NULL;

UNICODE_STRING	Kernel_CommandLineW = { 0 };
ANSI_STRING		Kernel_CommandLineA = { 0 };

P_SetThreadExecutionState		__sys_SetThreadExecutionState		= NULL;
//P_Sleep						__sys_Sleep							= NULL;
P_SleepEx						__sys_SleepEx						= NULL;
P_GetTickCount					__sys_GetTickCount					= NULL;
P_GetTickCount64				__sys_GetTickCount64				= NULL;
P_QueryUnbiasedInterruptTime	__sys_QueryUnbiasedInterruptTime	= NULL;
P_QueryPerformanceCounter		__sys_QueryPerformanceCounter		= NULL;

P_GetUserDefaultUILanguage 		__sys_GetUserDefaultUILanguage 		= NULL;
P_GetUserDefaultLocaleName 		__sys_GetUserDefaultLocaleName 		= NULL;
P_GetUserDefaultLCID 			__sys_GetUserDefaultLCID 			= NULL;
P_GetUserDefaultLangID 			__sys_GetUserDefaultLangID 			= NULL;
P_GetUserDefaultGeoName 		__sys_GetUserDefaultGeoName 		= NULL;
P_GetSystemDefaultUILanguage 	__sys_GetSystemDefaultUILanguage 	= NULL;
P_GetSystemDefaultLocaleName 	__sys_GetSystemDefaultLocaleName 	= NULL;
P_GetSystemDefaultLCID 			__sys_GetSystemDefaultLCID 			= NULL;
P_GetSystemDefaultLangID 		__sys_GetSystemDefaultLangID 		= NULL;
P_GetVolumeInformationByHandleW __sys_GetVolumeInformationByHandleW = NULL;

LCID			Kernel_CustomLCID = 0;

extern POOL* Dll_Pool;

static HASH_MAP Kernel_DiskSN;
static CRITICAL_SECTION Kernel_DiskSN_CritSec;
static ULONG64 Dll_FirstGetTickCountValue = 0;
//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------

static LPWSTR Kernel_GetCommandLineW(VOID);

static LPSTR Kernel_GetCommandLineA(VOID);

static EXECUTION_STATE Kernel_SetThreadExecutionState(EXECUTION_STATE esFlags);

static DWORD Kernel_GetTickCount();

static ULONGLONG Kernel_GetTickCount64();

static BOOL Kernel_QueryUnbiasedInterruptTime(PULONGLONG UnbiasedTime);

//static void Kernel_Sleep(DWORD dwMiSecond); // no need hooking sleep as it internally just calls SleepEx

static DWORD Kernel_SleepEx(DWORD dwMiSecond, BOOL bAlert);

static BOOL Kernel_QueryPerformanceCounter(LARGE_INTEGER* lpPerformanceCount);


static LANGID Kernel_GetUserDefaultUILanguage();

static int Kernel_GetUserDefaultLocaleName(LPWSTR lpLocaleName, int cchLocaleName);

static LCID Kernel_GetUserDefaultLCID();

static LANGID Kernel_GetUserDefaultLangID();

static int Kernel_GetUserDefaultGeoName(LPWSTR geoName, int geoNameCount);

static LANGID Kernel_GetSystemDefaultUILanguage();

static int Kernel_GetSystemDefaultLocaleName(LPWSTR lpLocaleName, int cchLocaleName);

static LCID Kernel_GetSystemDefaultLCID();

static LANGID Kernel_GetSystemDefaultLangID();

static BOOL Kernel_GetVolumeInformationByHandleW(HANDLE hFile, LPWSTR lpVolumeNameBuffer, DWORD nVolumeNameSize, LPDWORD lpVolumeSerialNumber, LPDWORD lpMaximumComponentLength, LPDWORD lpFileSystemFlags, LPWSTR  lpFileSystemNameBuffer, DWORD nFileSystemNameSize);

extern NTSTATUS File_GetName(
    HANDLE RootDirectory, UNICODE_STRING *ObjectName,
    WCHAR **OutTruePath, WCHAR **OutCopyPath, ULONG *OutFlags);

//---------------------------------------------------------------------------
// Kernel_Init
//---------------------------------------------------------------------------


_FX BOOLEAN Kernel_Init()
{
	HMODULE module = Dll_Kernel32;

	if (Dll_ImageType == DLL_IMAGE_GOOGLE_CHROME) {

		RTL_USER_PROCESS_PARAMETERS* ProcessParms = Proc_GetRtlUserProcessParameters();

		if (!wcsstr(ProcessParms->CommandLine.Buffer, L" --type=")) { // don't add flags to child processes

			NTSTATUS status;
			WCHAR CustomChromiumFlags[CONF_LINE_LEN];
			status = SbieApi_QueryConfAsIs(NULL, L"CustomChromiumFlags", 0, CustomChromiumFlags, ARRAYSIZE(CustomChromiumFlags));
			if (NT_SUCCESS(status)) {

				const WCHAR* lpCommandLine = ProcessParms->CommandLine.Buffer;
				const WCHAR* lpArguments = SbieDll_FindArgumentEnd(lpCommandLine);
				if (lpArguments == NULL)
					lpArguments = wcsrchr(lpCommandLine, L'\0');

				Kernel_CommandLineW.MaximumLength = ProcessParms->CommandLine.MaximumLength + (CONF_LINE_LEN + 8) * sizeof(WCHAR);
				Kernel_CommandLineW.Buffer = LocalAlloc(LMEM_FIXED,Kernel_CommandLineW.MaximumLength);

				// copy argument 0
				wmemcpy(Kernel_CommandLineW.Buffer, lpCommandLine, lpArguments - lpCommandLine);
				Kernel_CommandLineW.Buffer[lpArguments - lpCommandLine] = 0;
				
				// add custom arguments
				if(Kernel_CommandLineW.Buffer[lpArguments - lpCommandLine - 1] != L' ')
					wcscat(Kernel_CommandLineW.Buffer, L" ");
				wcscat(Kernel_CommandLineW.Buffer, CustomChromiumFlags);

				// add remaining arguments
				wcscat(Kernel_CommandLineW.Buffer, lpArguments);


				Kernel_CommandLineW.Length = wcslen(Kernel_CommandLineW.Buffer) * sizeof(WCHAR);

				RtlUnicodeStringToAnsiString(&Kernel_CommandLineA, &Kernel_CommandLineW, TRUE);

				void* GetCommandLineW = GetProcAddress(Dll_KernelBase ? Dll_KernelBase : Dll_Kernel32, "GetCommandLineW");
				SBIEDLL_HOOK(Kernel_, GetCommandLineW);

				void* GetCommandLineA = GetProcAddress(Dll_KernelBase ? Dll_KernelBase : Dll_Kernel32, "GetCommandLineA");
				SBIEDLL_HOOK(Kernel_, GetCommandLineA);
			}
		}
	}

	if (SbieApi_QueryConfBool(NULL, L"BlockInterferePower", FALSE)) {

        SBIEDLL_HOOK(Kernel_, SetThreadExecutionState);
    }

	if (SbieApi_QueryConfBool(NULL, L"UseChangeSpeed", FALSE)) {

		SBIEDLL_HOOK(Kernel_, GetTickCount);
		Dll_FirstGetTickCountValue = __sys_GetTickCount();

		void* GetTickCount64 = GetProcAddress(Dll_KernelBase ? Dll_KernelBase : Dll_Kernel32, "GetTickCount64");
		if (GetTickCount64) {
			SBIEDLL_HOOK(Kernel_, GetTickCount64) 
		}
		void* QueryUnbiasedInterruptTime = GetProcAddress(Dll_KernelBase ? Dll_KernelBase : Dll_Kernel32, "QueryUnbiasedInterruptTime");
		if (QueryUnbiasedInterruptTime) {
			SBIEDLL_HOOK(Kernel_, QueryUnbiasedInterruptTime);
		}
		SBIEDLL_HOOK(Kernel_, QueryPerformanceCounter);
		//SBIEDLL_HOOK(Kernel_, Sleep);
		SBIEDLL_HOOK(Kernel_, SleepEx);	
	}

	Kernel_CustomLCID = (LCID)SbieApi_QueryConfNumber(NULL, L"CustomLCID", 0); // use 1033 for en-US
	if (Kernel_CustomLCID) {
	
		SBIEDLL_HOOK(Kernel_, GetUserDefaultUILanguage);
		void* GetUserDefaultLocaleName = GetProcAddress(Dll_KernelBase ? Dll_KernelBase : Dll_Kernel32, "GetUserDefaultLocaleName");
		if (GetUserDefaultLocaleName) {
			SBIEDLL_HOOK(Kernel_, GetUserDefaultLocaleName);
		}
		SBIEDLL_HOOK(Kernel_, GetUserDefaultLCID);
		SBIEDLL_HOOK(Kernel_, GetUserDefaultLangID);
		void* GetUserDefaultGeoName = GetProcAddress(Dll_KernelBase ? Dll_KernelBase : Dll_Kernel32, "GetUserDefaultGeoName");
		if (GetUserDefaultGeoName) {
			SBIEDLL_HOOK(Kernel_, GetUserDefaultGeoName);
		}
		SBIEDLL_HOOK(Kernel_, GetSystemDefaultUILanguage);
		void* GetSystemDefaultLocaleName = GetProcAddress(Dll_KernelBase ? Dll_KernelBase : Dll_Kernel32, "GetSystemDefaultLocaleName");
		if (GetSystemDefaultLocaleName) {
			SBIEDLL_HOOK(Kernel_, GetSystemDefaultLocaleName);
		}
		SBIEDLL_HOOK(Kernel_, GetSystemDefaultLCID);
		SBIEDLL_HOOK(Kernel_, GetSystemDefaultLangID);
	}

	if (SbieApi_QueryConfBool(NULL, L"HideDiskSerialNumber", FALSE)) {

		InitializeCriticalSection(&Kernel_DiskSN_CritSec);
		map_init(&Kernel_DiskSN, Dll_Pool);

		void* GetVolumeInformationByHandleW = GetProcAddress(Dll_KernelBase ? Dll_KernelBase : Dll_Kernel32, "GetVolumeInformationByHandleW");
		if (GetVolumeInformationByHandleW) {
			SBIEDLL_HOOK(Kernel_, GetVolumeInformationByHandleW);
		}
	}
	return TRUE;
}


//---------------------------------------------------------------------------
// Kernel_GetCommandLineW
//---------------------------------------------------------------------------


_FX LPWSTR Kernel_GetCommandLineW(VOID)
{
	return Kernel_CommandLineW.Buffer;
	//return __sys_GetCommandLineW();
}


//---------------------------------------------------------------------------
// Kernel_GetCommandLineA
//---------------------------------------------------------------------------


_FX LPSTR Kernel_GetCommandLineA(VOID)
{
	return Kernel_CommandLineA.Buffer;
	//return __sys_GetCommandLineA();
}


//---------------------------------------------------------------------------
// Kernel_SetThreadExecutionState
//---------------------------------------------------------------------------


_FX EXECUTION_STATE Kernel_SetThreadExecutionState(EXECUTION_STATE esFlags) 
{
	SetLastError(ERROR_ACCESS_DENIED);
	return 0;
	//return __sys_SetThreadExecutionState(esFlags);
}


//---------------------------------------------------------------------------
// Kernel_GetTickCount
//---------------------------------------------------------------------------


_FX DWORD Kernel_GetTickCount() 
{
	ULONG add = SbieApi_QueryConfNumber(NULL, L"AddTickSpeed", 1);
	ULONG low = SbieApi_QueryConfNumber(NULL, L"LowTickSpeed", 1);
	ULONG64 count = __sys_GetTickCount();
	
	if(add != 0 && low != 0) {
		count = Dll_FirstGetTickCountValue + (count - Dll_FirstGetTickCountValue) * add / low; // multi
	}

	return (DWORD)count;
}


//---------------------------------------------------------------------------
// Kernel_GetTickCount64
//---------------------------------------------------------------------------


_FX ULONGLONG Kernel_GetTickCount64() 
{
	ULONG add = SbieApi_QueryConfNumber(NULL, L"AddTickSpeed", 1);
	ULONG low = SbieApi_QueryConfNumber(NULL, L"LowTickSpeed", 1);
	if (add != 0 && low != 0)
		return __sys_GetTickCount64() * add / low;
	return __sys_GetTickCount64() * add;
}


//---------------------------------------------------------------------------
// Kernel_QueryUnbiasedInterruptTime
//---------------------------------------------------------------------------


_FX BOOL Kernel_QueryUnbiasedInterruptTime(PULONGLONG UnbiasedTime)
{
	BOOL rtn = __sys_QueryUnbiasedInterruptTime(UnbiasedTime);
	ULONG add = SbieApi_QueryConfNumber(NULL, L"AddTickSpeed", 1);
	ULONG low = SbieApi_QueryConfNumber(NULL, L"LowTickSpeed", 1);
	if (add != 0 && low != 0)
		*UnbiasedTime *= add / low;
	else
		*UnbiasedTime *= add;
	return rtn;
}


//---------------------------------------------------------------------------
// Kernel_SleepEx
//---------------------------------------------------------------------------


_FX DWORD Kernel_SleepEx(DWORD dwMiSecond, BOOL bAlert) 
{
	ULONG add = SbieApi_QueryConfNumber(NULL, L"AddSleepSpeed", 1);
	ULONG low = SbieApi_QueryConfNumber(NULL, L"LowSleepSpeed", 1);
	if (add != 0 && low != 0)
		return __sys_SleepEx(dwMiSecond * low / add, bAlert);
	return __sys_SleepEx(dwMiSecond, bAlert);
}


//---------------------------------------------------------------------------
// Kernel_QueryPerformanceCounter
//---------------------------------------------------------------------------


_FX BOOL Kernel_QueryPerformanceCounter(LARGE_INTEGER* lpPerformanceCount)
{
	BOOL rtn = __sys_QueryPerformanceCounter(lpPerformanceCount);
	ULONG add = SbieApi_QueryConfNumber(NULL, L"AddTickSpeed", 1);
	ULONG low = SbieApi_QueryConfNumber(NULL, L"LowTickSpeed", 1);
	if (add != 0 && low != 0)
		lpPerformanceCount->QuadPart = lpPerformanceCount->QuadPart * add / low;
	return rtn;
}


//---------------------------------------------------------------------------
// Kernel_GetUserDefaultUILanguage
//---------------------------------------------------------------------------


_FX LANGID Kernel_GetUserDefaultUILanguage() 
{
	return (LANGID)Kernel_CustomLCID;
}


//---------------------------------------------------------------------------
// Kernel_GetUserDefaultLocaleName
//---------------------------------------------------------------------------


_FX int Kernel_GetUserDefaultLocaleName(LPWSTR lpLocaleName, int cchLocaleName) 
{
	return Kernel_GetSystemDefaultLocaleName(lpLocaleName, cchLocaleName);
}


//---------------------------------------------------------------------------
// Kernel_GetUserDefaultLCID
//---------------------------------------------------------------------------


_FX LCID Kernel_GetUserDefaultLCID() 
{
	return Kernel_CustomLCID;
}


//---------------------------------------------------------------------------
// Kernel_GetUserDefaultLangID
//---------------------------------------------------------------------------


_FX LANGID Kernel_GetUserDefaultLangID() 
{
	return (LANGID)Kernel_CustomLCID;
}


//---------------------------------------------------------------------------
// Kernel_GetUserDefaultGeoName
//---------------------------------------------------------------------------


_FX int Kernel_GetUserDefaultGeoName(LPWSTR geoName, int geoNameCount) 
{
	WCHAR LocaleName[32];
	int cchLocaleName = Kernel_GetSystemDefaultLocaleName(LocaleName, ARRAYSIZE(LocaleName));
	if (cchLocaleName > 0) {
		WCHAR* Name = wcsrchr(LocaleName, L'-');
		if (Name) {
			int len = (int)wcslen(Name++);
			if (geoNameCount >= len) {
				wcscpy(geoName, Name);
			}
			return len;
		}
	}
	return 0;
}


//---------------------------------------------------------------------------
// Kernel_GetSystemDefaultUILanguage
//---------------------------------------------------------------------------


_FX LANGID Kernel_GetSystemDefaultUILanguage() 
{
	return (LANGID)Kernel_CustomLCID;
}


//---------------------------------------------------------------------------
// Kernel_GetSystemDefaultLocaleName
//---------------------------------------------------------------------------


_FX int Kernel_GetSystemDefaultLocaleName(LPWSTR lpLocaleName, int cchLocaleName) 
{
	LCIDToLocaleName ltln = (LCIDToLocaleName)GetProcAddress(Dll_KernelBase ? Dll_KernelBase : Dll_Kernel32, "LCIDToLocaleName");
	if (ltln) {
		int ret = ltln(Kernel_CustomLCID, lpLocaleName, cchLocaleName, 0);
		if (ret) 
			return ret;
	}
	
	// on failure fallback to en_US
	if (cchLocaleName >= 6) {
		wcscpy(lpLocaleName, L"en_US");
		return 6;
	}
	return 0;
}


//---------------------------------------------------------------------------
// Kernel_GetSystemDefaultLCID
//---------------------------------------------------------------------------


_FX LCID Kernel_GetSystemDefaultLCID() 
{
	return Kernel_CustomLCID;
}


//---------------------------------------------------------------------------
// Kernel_GetSystemDefaultLangID
//---------------------------------------------------------------------------


_FX LANGID Kernel_GetSystemDefaultLangID() 
{
	return (LANGID)Kernel_CustomLCID;
}


//----------------------------------------------------------------------------
//Kernel_GetVolumeInformationByHandleW
//----------------------------------------------------------------------------

BOOL hex_string_to_uint8_array(const wchar_t* str, unsigned char* output_array, size_t* output_length, BOOL swap_bytes);

_FX BOOL Kernel_GetVolumeInformationByHandleW(HANDLE hFile, LPWSTR lpVolumeNameBuffer, DWORD nVolumeNameSize, LPDWORD lpVolumeSerialNumber,LPDWORD lpMaximumComponentLength, LPDWORD lpFileSystemFlags, LPWSTR  lpFileSystemNameBuffer, DWORD nFileSystemNameSize) 
{
	DWORD ourSerialNumber = 0;

	BOOL rtn = __sys_GetVolumeInformationByHandleW(hFile, lpVolumeNameBuffer, nVolumeNameSize, &ourSerialNumber, lpMaximumComponentLength, lpFileSystemFlags, lpFileSystemNameBuffer, nFileSystemNameSize);
	if (lpVolumeSerialNumber != NULL) {

        EnterCriticalSection(&Kernel_DiskSN_CritSec);

		void* key = (void*)ourSerialNumber;

		DWORD* lpCachedSerialNumber = map_get(&Kernel_DiskSN, key);
		if (lpCachedSerialNumber)
			*lpVolumeSerialNumber = *lpCachedSerialNumber;
		else
		{
			WCHAR DeviceName[MAX_PATH] = { 0 };

			ULONG LastError;
			THREAD_DATA* TlsData;

			TlsData = Dll_GetTlsData(&LastError);
			Dll_PushTlsNameBuffer(TlsData);

			WCHAR* TruePath, * CopyPath;
			File_GetName(hFile, NULL, &TruePath, &CopyPath, NULL);

			if (_wcsnicmp(TruePath, L"\\Device\\", 8) == 0)
			{
				WCHAR* End = wcschr(TruePath + 8, L'\\');
				if(!End) End = wcschr(TruePath + 8, L'\0');
				wcsncpy(DeviceName, TruePath + 8, End - (TruePath + 8));
			}

			Dll_PopTlsNameBuffer(TlsData);
			SetLastError(LastError);

			if(*DeviceName == 0)
				*lpVolumeSerialNumber = Dll_rand();
			else
			{
				WCHAR Value[30] = { 0 };
				SbieDll_GetSettingsForName(NULL, DeviceName, L"DiskSerialNumber", Value, sizeof(Value), L"");
				DWORD value_buf = 0;;
				size_t value_len = sizeof(value_buf);
				if (hex_string_to_uint8_array(Value, &value_buf, &value_len, TRUE))
					*lpVolumeSerialNumber = value_buf;
				else 
					*lpVolumeSerialNumber = Dll_rand();
			}
			
			map_insert(&Kernel_DiskSN, key, lpVolumeSerialNumber, sizeof(DWORD));
		}

		LeaveCriticalSection(&Kernel_DiskSN_CritSec);
	}
	return rtn;
}
