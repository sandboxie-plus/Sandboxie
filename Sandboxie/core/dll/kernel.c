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

static BOOLEAN Kernel_InitIdentity(HMODULE module);

static BOOLEAN Kernel_InitTimeZone(HMODULE module);

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

	if (Config_GetSettingsForImageName_bool(L"HideDiskSerialNumber", FALSE)) {

		InitializeCriticalSection(&Kernel_DiskSN_CritSec);
		map_init(&Kernel_DiskSN, Dll_Pool);

		void* GetVolumeInformationByHandleW = GetProcAddress(Dll_KernelBase ? Dll_KernelBase : Dll_Kernel32, "GetVolumeInformationByHandleW");
		if (GetVolumeInformationByHandleW) {
			SBIEDLL_HOOK(Kernel_, GetVolumeInformationByHandleW);
		}
	}

	if (!Kernel_InitIdentity(module))
		return FALSE;

	if (!Kernel_InitTimeZone(module))
		return FALSE;

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



//---------------------------------------------------------------------------
//
// Custom computer name, user name and time zone
//
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
// Defines
//---------------------------------------------------------------------------


#define KERNEL_HOST_NAME_LEN            63      // max length of a DNS host label
#define KERNEL_USER_NAME_LEN            20      // max length of a SAM account name

#define KERNEL_NAME_SAM_COMPATIBLE      2       // EXTENDED_NAME_FORMAT values
#define KERNEL_NAME_DISPLAY             3
#define KERNEL_NAME_USER_PRINCIPAL      8

#define KERNEL_WSAEFAULT                10014
#define KERNEL_SOCKET_ERROR             (-1)

#define KERNEL_FILETIME_PER_MINUTE      600000000LL


//---------------------------------------------------------------------------
// Structures and Types
//---------------------------------------------------------------------------


//
// same layout as DYNAMIC_TIME_ZONE_INFORMATION, which the SDK headers
// declare only for _WIN32_WINNT >= 0x0600
//

typedef struct _KERNEL_DYNAMIC_TIME_ZONE_INFORMATION {
	LONG Bias;
	WCHAR StandardName[32];
	SYSTEMTIME StandardDate;
	LONG StandardBias;
	WCHAR DaylightName[32];
	SYSTEMTIME DaylightDate;
	LONG DaylightBias;
	WCHAR TimeZoneKeyName[128];
	BOOLEAN DynamicDaylightTimeDisabled;
} KERNEL_DYNAMIC_TIME_ZONE_INFORMATION;


//---------------------------------------------------------------------------
// Functions Prototypes
//---------------------------------------------------------------------------


typedef BOOL (*P_GetComputerNameW)(LPWSTR lpBuffer, LPDWORD nSize);

typedef BOOL (*P_GetComputerNameA)(LPSTR lpBuffer, LPDWORD nSize);

typedef BOOL (*P_GetComputerNameExW)(COMPUTER_NAME_FORMAT NameType, LPWSTR lpBuffer, LPDWORD nSize);

typedef BOOL (*P_GetComputerNameExA)(COMPUTER_NAME_FORMAT NameType, LPSTR lpBuffer, LPDWORD nSize);

typedef int (*P_gethostname)(char *name, int namelen);

typedef int (*P_GetHostNameW)(WCHAR *name, int namelen);

typedef BOOL (*P_GetUserNameW)(LPWSTR lpBuffer, LPDWORD pcbBuffer);

typedef BOOL (*P_GetUserNameA)(LPSTR lpBuffer, LPDWORD pcbBuffer);

typedef BOOLEAN (*P_GetUserNameExW)(ULONG NameFormat, LPWSTR lpNameBuffer, PULONG nSize);

typedef BOOLEAN (*P_GetUserNameExA)(ULONG NameFormat, LPSTR lpNameBuffer, PULONG nSize);

typedef DWORD (*P_GetTimeZoneInformation)(LPTIME_ZONE_INFORMATION lpTimeZoneInformation);

typedef DWORD (*P_GetDynamicTimeZoneInformation)(KERNEL_DYNAMIC_TIME_ZONE_INFORMATION *pTimeZoneInformation);

typedef BOOL (*P_GetTimeZoneInformationForYear)(USHORT wYear, KERNEL_DYNAMIC_TIME_ZONE_INFORMATION *pdtzi, LPTIME_ZONE_INFORMATION ptzi);

typedef BOOL (*P_SystemTimeToTzSpecificLocalTime)(const TIME_ZONE_INFORMATION *lpTimeZoneInformation, const SYSTEMTIME *lpUniversalTime, LPSYSTEMTIME lpLocalTime);

typedef BOOL (*P_SystemTimeToTzSpecificLocalTimeEx)(const KERNEL_DYNAMIC_TIME_ZONE_INFORMATION *lpTimeZoneInformation, const SYSTEMTIME *lpUniversalTime, LPSYSTEMTIME lpLocalTime);

typedef BOOL (*P_TzSpecificLocalTimeToSystemTime)(const TIME_ZONE_INFORMATION *lpTimeZoneInformation, const SYSTEMTIME *lpLocalTime, LPSYSTEMTIME lpUniversalTime);

typedef BOOL (*P_TzSpecificLocalTimeToSystemTimeEx)(const KERNEL_DYNAMIC_TIME_ZONE_INFORMATION *lpTimeZoneInformation, const SYSTEMTIME *lpLocalTime, LPSYSTEMTIME lpUniversalTime);

typedef void (*P_GetLocalTime)(LPSYSTEMTIME lpSystemTime);

typedef BOOL (*P_FileTimeToLocalFileTime)(const FILETIME *lpFileTime, LPFILETIME lpLocalFileTime);

typedef BOOL (*P_LocalFileTimeToFileTime)(const FILETIME *lpLocalFileTime, LPFILETIME lpFileTime);


//---------------------------------------------------------------------------
// Variables
//---------------------------------------------------------------------------


static P_GetComputerNameW                   __sys_GetComputerNameW                  = NULL;
static P_GetComputerNameA                   __sys_GetComputerNameA                  = NULL;
static P_GetComputerNameExW                 __sys_GetComputerNameExW                = NULL;
static P_GetComputerNameExA                 __sys_GetComputerNameExA                = NULL;
static P_gethostname                        __sys_gethostname                       = NULL;
static P_GetHostNameW                       __sys_GetHostNameW                      = NULL;
static P_GetUserNameW                       __sys_GetUserNameW                      = NULL;
static P_GetUserNameA                       __sys_GetUserNameA                      = NULL;
static P_GetUserNameExW                     __sys_GetUserNameExW                    = NULL;
static P_GetUserNameExA                     __sys_GetUserNameExA                    = NULL;

static P_GetTimeZoneInformation             __sys_GetTimeZoneInformation            = NULL;
static P_GetDynamicTimeZoneInformation      __sys_GetDynamicTimeZoneInformation     = NULL;
static P_GetTimeZoneInformationForYear      __sys_GetTimeZoneInformationForYear     = NULL;
static P_SystemTimeToTzSpecificLocalTime    __sys_SystemTimeToTzSpecificLocalTime   = NULL;
static P_SystemTimeToTzSpecificLocalTimeEx  __sys_SystemTimeToTzSpecificLocalTimeEx = NULL;
static P_TzSpecificLocalTimeToSystemTime    __sys_TzSpecificLocalTimeToSystemTime   = NULL;
static P_TzSpecificLocalTimeToSystemTimeEx  __sys_TzSpecificLocalTimeToSystemTimeEx = NULL;
static P_GetLocalTime                       __sys_GetLocalTime                      = NULL;
static P_FileTimeToLocalFileTime            __sys_FileTimeToLocalFileTime           = NULL;
static P_LocalFileTimeToFileTime            __sys_LocalFileTimeToFileTime           = NULL;

static WCHAR Kernel_CustomHostName[KERNEL_HOST_NAME_LEN + 1] = { 0 };
static WCHAR Kernel_CustomNetBiosName[MAX_COMPUTERNAME_LENGTH + 1] = { 0 };
static WCHAR Kernel_RealNetBiosName[MAX_COMPUTERNAME_LENGTH + 1] = { 0 };
static WCHAR Kernel_CustomUserName[KERNEL_USER_NAME_LEN + 1] = { 0 };

static KERNEL_DYNAMIC_TIME_ZONE_INFORMATION Kernel_CustomTimeZone;
static TIME_ZONE_INFORMATION Kernel_CustomTimeZoneForYear;
static USHORT Kernel_CustomTimeZoneYear = 0;
static CRITICAL_SECTION Kernel_TimeZone_CritSec;


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------


static BOOL Kernel_GetComputerNameW(LPWSTR lpBuffer, LPDWORD nSize);

static BOOL Kernel_GetComputerNameA(LPSTR lpBuffer, LPDWORD nSize);

static BOOL Kernel_GetComputerNameExW(COMPUTER_NAME_FORMAT NameType, LPWSTR lpBuffer, LPDWORD nSize);

static BOOL Kernel_GetComputerNameExA(COMPUTER_NAME_FORMAT NameType, LPSTR lpBuffer, LPDWORD nSize);

static int Kernel_gethostname(char *name, int namelen);

static int Kernel_GetHostNameW(WCHAR *name, int namelen);

static BOOL Kernel_GetUserNameW(LPWSTR lpBuffer, LPDWORD pcbBuffer);

static BOOL Kernel_GetUserNameA(LPSTR lpBuffer, LPDWORD pcbBuffer);

static BOOLEAN Kernel_GetUserNameExW(ULONG NameFormat, LPWSTR lpNameBuffer, PULONG nSize);

static BOOLEAN Kernel_GetUserNameExA(ULONG NameFormat, LPSTR lpNameBuffer, PULONG nSize);

static DWORD Kernel_GetTimeZoneInformation(LPTIME_ZONE_INFORMATION lpTimeZoneInformation);

static DWORD Kernel_GetDynamicTimeZoneInformation(KERNEL_DYNAMIC_TIME_ZONE_INFORMATION *pTimeZoneInformation);

static BOOL Kernel_GetTimeZoneInformationForYear(USHORT wYear, KERNEL_DYNAMIC_TIME_ZONE_INFORMATION *pdtzi, LPTIME_ZONE_INFORMATION ptzi);

static BOOL Kernel_SystemTimeToTzSpecificLocalTime(const TIME_ZONE_INFORMATION *lpTimeZoneInformation, const SYSTEMTIME *lpUniversalTime, LPSYSTEMTIME lpLocalTime);

static BOOL Kernel_SystemTimeToTzSpecificLocalTimeEx(const KERNEL_DYNAMIC_TIME_ZONE_INFORMATION *lpTimeZoneInformation, const SYSTEMTIME *lpUniversalTime, LPSYSTEMTIME lpLocalTime);

static BOOL Kernel_TzSpecificLocalTimeToSystemTime(const TIME_ZONE_INFORMATION *lpTimeZoneInformation, const SYSTEMTIME *lpLocalTime, LPSYSTEMTIME lpUniversalTime);

static BOOL Kernel_TzSpecificLocalTimeToSystemTimeEx(const KERNEL_DYNAMIC_TIME_ZONE_INFORMATION *lpTimeZoneInformation, const SYSTEMTIME *lpLocalTime, LPSYSTEMTIME lpUniversalTime);

static void Kernel_GetLocalTime(LPSYSTEMTIME lpSystemTime);

static BOOL Kernel_FileTimeToLocalFileTime(const FILETIME *lpFileTime, LPFILETIME lpLocalFileTime);

static BOOL Kernel_LocalFileTimeToFileTime(const FILETIME *lpLocalFileTime, LPFILETIME lpFileTime);


//---------------------------------------------------------------------------
// Kernel_GetProcAddress
//---------------------------------------------------------------------------


_FX void *Kernel_GetProcAddress(const char *ProcName)
{
	void *proc = NULL;
	if (Dll_KernelBase)
		proc = GetProcAddress(Dll_KernelBase, ProcName);
	if (! proc)
		proc = GetProcAddress(Dll_Kernel32, ProcName);
	return proc;
}


//---------------------------------------------------------------------------
// Kernel_IsValidName
//---------------------------------------------------------------------------


_FX BOOLEAN Kernel_IsValidName(const WCHAR *Name, ULONG MaxLen, BOOLEAN IsHostName)
{
	BOOLEAN DigitsOnly = TRUE;
	ULONG len;

	for (len = 0; Name[len]; len++) {

		WCHAR c = Name[len];
		if (len >= MaxLen)
			return FALSE;
		if (c >= L'0' && c <= L'9')
			continue;
		DigitsOnly = FALSE;
		if ((c >= L'a' && c <= L'z') || (c >= L'A' && c <= L'Z') || c == L'-')
			continue;
		if (! IsHostName && (c == L'_' || c == L'.'))
			continue;
		return FALSE;
	}

	if (len == 0)
		return FALSE;

	//
	// Windows does not accept host names which consist of digits only
	// or which start or end with a hyphen
	//

	if (IsHostName && (DigitsOnly || Name[0] == L'-' || Name[len - 1] == L'-'))
		return FALSE;

	return TRUE;
}


//---------------------------------------------------------------------------
// Kernel_CopyNameW
//---------------------------------------------------------------------------


_FX BOOL Kernel_CopyNameW(
	const WCHAR *Name, WCHAR *lpBuffer, DWORD *nSize, ULONG ErrorCode, BOOLEAN CountNull)
{
	DWORD len = (DWORD)wcslen(Name);

	if (! nSize) {
		SetLastError(ERROR_INVALID_PARAMETER);
		return FALSE;
	}

	if (! lpBuffer || *nSize < len + 1) {
		*nSize = len + 1;
		SetLastError(ErrorCode);
		return FALSE;
	}

	wmemcpy(lpBuffer, Name, len + 1);
	*nSize = CountNull ? len + 1 : len;
	return TRUE;
}


//---------------------------------------------------------------------------
// Kernel_CopyNameA
//---------------------------------------------------------------------------


_FX BOOL Kernel_CopyNameA(
	const WCHAR *Name, char *lpBuffer, DWORD *nSize, ULONG ErrorCode, BOOLEAN CountNull)
{
	int len;

	if (! nSize) {
		SetLastError(ERROR_INVALID_PARAMETER);
		return FALSE;
	}

	len = WideCharToMultiByte(CP_ACP, 0, Name, -1, NULL, 0, NULL, NULL); // including null
	if (len <= 0)
		return FALSE;

	if (! lpBuffer || *nSize < (DWORD)len) {
		*nSize = len;
		SetLastError(ErrorCode);
		return FALSE;
	}

	WideCharToMultiByte(CP_ACP, 0, Name, -1, lpBuffer, len, NULL, NULL);
	*nSize = CountNull ? len : len - 1;
	return TRUE;
}


//---------------------------------------------------------------------------
// Kernel_ReplaceEnvVar
//---------------------------------------------------------------------------


_FX void Kernel_ReplaceEnvVar(const WCHAR *Name, const WCHAR *OldValue, const WCHAR *NewValue)
{
	WCHAR Value[MAX_PATH];
	DWORD len = GetEnvironmentVariableW(Name, Value, ARRAYSIZE(Value));
	if (len == 0 || len >= ARRAYSIZE(Value))
		return;
	if (OldValue && _wcsicmp(Value, OldValue) != 0)
		return;
	SetEnvironmentVariableW(Name, NewValue);
}


//---------------------------------------------------------------------------
// Kernel_InitIdentity
//---------------------------------------------------------------------------


_FX BOOLEAN Kernel_InitIdentity(HMODULE module)
{
	WCHAR Value[128];
	DWORD len;
	ULONG i;

	//
	// remember the real NetBIOS name, it is needed to recognize
	// the computer name in environment variables and account names
	//

	len = ARRAYSIZE(Kernel_RealNetBiosName);
	if (! GetComputerNameW(Kernel_RealNetBiosName, &len))
		Kernel_RealNetBiosName[0] = L'\0';

	SbieDll_GetSettingsForName(NULL, Dll_ImageName, L"CustomComputerName", Value, sizeof(Value), NULL);
	if (*Value && Kernel_IsValidName(Value, KERNEL_HOST_NAME_LEN, TRUE)) {

		wcscpy(Kernel_CustomHostName, Value);

		//
		// the NetBIOS name is upper case and at most 15 characters long
		//

		for (i = 0; i < MAX_COMPUTERNAME_LENGTH && Value[i]; i++) {
			WCHAR c = Value[i];
			Kernel_CustomNetBiosName[i] = (c >= L'a' && c <= L'z') ? (WCHAR)(c - L'a' + L'A') : c;
		}
		Kernel_CustomNetBiosName[i] = L'\0';
	}

	SbieDll_GetSettingsForName(NULL, Dll_ImageName, L"CustomUserName", Value, sizeof(Value), NULL);
	if (*Value && Kernel_IsValidName(Value, KERNEL_USER_NAME_LEN, FALSE))
		wcscpy(Kernel_CustomUserName, Value);

	if (*Kernel_CustomHostName) {

		void *GetComputerNameW = Kernel_GetProcAddress("GetComputerNameW");
		void *GetComputerNameA = Kernel_GetProcAddress("GetComputerNameA");
		void *GetComputerNameExW = Kernel_GetProcAddress("GetComputerNameExW");
		void *GetComputerNameExA = Kernel_GetProcAddress("GetComputerNameExA");

		if (GetComputerNameW) {
			SBIEDLL_HOOK(Kernel_, GetComputerNameW);
		}
		if (GetComputerNameA) {
			SBIEDLL_HOOK(Kernel_, GetComputerNameA);
		}
		if (GetComputerNameExW) {
			SBIEDLL_HOOK(Kernel_, GetComputerNameExW);
		}
		if (GetComputerNameExA) {
			SBIEDLL_HOOK(Kernel_, GetComputerNameExA);
		}

		Kernel_ReplaceEnvVar(L"COMPUTERNAME", NULL, Kernel_CustomNetBiosName);

		if (*Kernel_RealNetBiosName) {

			WCHAR OldServer[MAX_COMPUTERNAME_LENGTH + 3];
			WCHAR NewServer[MAX_COMPUTERNAME_LENGTH + 3];

			//
			// for local accounts the user domain is the computer name
			//

			Kernel_ReplaceEnvVar(L"USERDOMAIN", Kernel_RealNetBiosName, Kernel_CustomNetBiosName);
			Kernel_ReplaceEnvVar(L"USERDOMAIN_ROAMINGPROFILE", Kernel_RealNetBiosName, Kernel_CustomNetBiosName);

			wcscpy(OldServer, L"\\\\");
			wcscat(OldServer, Kernel_RealNetBiosName);
			wcscpy(NewServer, L"\\\\");
			wcscat(NewServer, Kernel_CustomNetBiosName);
			Kernel_ReplaceEnvVar(L"LOGONSERVER", OldServer, NewServer);
		}
	}

	if (*Kernel_CustomUserName) {

		Kernel_ReplaceEnvVar(L"USERNAME", NULL, Kernel_CustomUserName);
	}

	return TRUE;
}


//---------------------------------------------------------------------------
// Kernel_Init_Ws2_32
//---------------------------------------------------------------------------


_FX BOOLEAN Kernel_Init_Ws2_32(HMODULE module)
{
	if (*Kernel_CustomHostName) {

		void *gethostname = GetProcAddress(module, "gethostname");
		void *GetHostNameW = GetProcAddress(module, "GetHostNameW");

		if (gethostname) {
			SBIEDLL_HOOK(Kernel_, gethostname);
		}
		if (GetHostNameW) {
			SBIEDLL_HOOK(Kernel_, GetHostNameW);
		}
	}

	return TRUE;
}


//---------------------------------------------------------------------------
// Kernel_Init_AdvApi
//---------------------------------------------------------------------------


_FX BOOLEAN Kernel_Init_AdvApi(HMODULE module)
{
	if (*Kernel_CustomUserName) {

		void *GetUserNameW = GetProcAddress(module, "GetUserNameW");
		void *GetUserNameA = GetProcAddress(module, "GetUserNameA");

		if (GetUserNameW) {
			SBIEDLL_HOOK(Kernel_, GetUserNameW);
		}
		if (GetUserNameA) {
			SBIEDLL_HOOK(Kernel_, GetUserNameA);
		}
	}

	return TRUE;
}


//---------------------------------------------------------------------------
// Kernel_Init_SspiCli
//---------------------------------------------------------------------------


_FX BOOLEAN Kernel_Init_SspiCli(HMODULE module)
{
	//
	// NameSamCompatible contains the computer name for local accounts,
	// so hook GetUserNameEx also when only the computer name is customized
	//

	if (*Kernel_CustomUserName || *Kernel_CustomHostName) {

		void *GetUserNameExW = GetProcAddress(module, "GetUserNameExW");
		void *GetUserNameExA = GetProcAddress(module, "GetUserNameExA");

		if (GetUserNameExW) {
			SBIEDLL_HOOK(Kernel_, GetUserNameExW);

			if (GetUserNameExA) {
				SBIEDLL_HOOK(Kernel_, GetUserNameExA);
			}
		}
	}

	return TRUE;
}


//---------------------------------------------------------------------------
// Kernel_GetCustomComputerNameEx
//---------------------------------------------------------------------------


_FX BOOLEAN Kernel_GetCustomComputerNameEx(COMPUTER_NAME_FORMAT NameType, WCHAR *Name, ULONG NameSize)
{
	WCHAR Domain[256];
	DWORD DomainLen;

	switch (NameType) {

	case ComputerNameNetBIOS:
	case ComputerNamePhysicalNetBIOS:
		wcscpy_s(Name, NameSize, Kernel_CustomNetBiosName);
		return TRUE;

	case ComputerNameDnsHostname:
	case ComputerNamePhysicalDnsHostname:
		wcscpy_s(Name, NameSize, Kernel_CustomHostName);
		return TRUE;

	case ComputerNameDnsFullyQualified:
	case ComputerNamePhysicalDnsFullyQualified:
		wcscpy_s(Name, NameSize, Kernel_CustomHostName);
		DomainLen = ARRAYSIZE(Domain);
		if (__sys_GetComputerNameExW && __sys_GetComputerNameExW(
				(NameType == ComputerNameDnsFullyQualified) ? ComputerNameDnsDomain : ComputerNamePhysicalDnsDomain,
				Domain, &DomainLen) && DomainLen > 0) {
			wcscat_s(Name, NameSize, L".");
			wcscat_s(Name, NameSize, Domain);
		}
		return TRUE;
	}

	return FALSE;
}


//---------------------------------------------------------------------------
// Kernel_GetComputerNameW
//---------------------------------------------------------------------------


_FX BOOL Kernel_GetComputerNameW(LPWSTR lpBuffer, LPDWORD nSize)
{
	return Kernel_CopyNameW(Kernel_CustomNetBiosName, lpBuffer, nSize, ERROR_BUFFER_OVERFLOW, FALSE);
}


//---------------------------------------------------------------------------
// Kernel_GetComputerNameA
//---------------------------------------------------------------------------


_FX BOOL Kernel_GetComputerNameA(LPSTR lpBuffer, LPDWORD nSize)
{
	return Kernel_CopyNameA(Kernel_CustomNetBiosName, lpBuffer, nSize, ERROR_BUFFER_OVERFLOW, FALSE);
}


//---------------------------------------------------------------------------
// Kernel_GetComputerNameExW
//---------------------------------------------------------------------------


_FX BOOL Kernel_GetComputerNameExW(COMPUTER_NAME_FORMAT NameType, LPWSTR lpBuffer, LPDWORD nSize)
{
	WCHAR Name[KERNEL_HOST_NAME_LEN + 1 + 256];

	if (! Kernel_GetCustomComputerNameEx(NameType, Name, ARRAYSIZE(Name)))
		return __sys_GetComputerNameExW(NameType, lpBuffer, nSize);

	return Kernel_CopyNameW(Name, lpBuffer, nSize, ERROR_MORE_DATA, FALSE);
}


//---------------------------------------------------------------------------
// Kernel_GetComputerNameExA
//---------------------------------------------------------------------------


_FX BOOL Kernel_GetComputerNameExA(COMPUTER_NAME_FORMAT NameType, LPSTR lpBuffer, LPDWORD nSize)
{
	WCHAR Name[KERNEL_HOST_NAME_LEN + 1 + 256];

	if (! Kernel_GetCustomComputerNameEx(NameType, Name, ARRAYSIZE(Name)))
		return __sys_GetComputerNameExA(NameType, lpBuffer, nSize);

	return Kernel_CopyNameA(Name, lpBuffer, nSize, ERROR_MORE_DATA, FALSE);
}


//---------------------------------------------------------------------------
// Kernel_gethostname
//---------------------------------------------------------------------------


_FX int Kernel_gethostname(char *name, int namelen)
{
	char RealName[256];
	int len, i;

	//
	// call the original function first to preserve its error handling,
	// e.g. WSANOTINITIALISED when WSAStartup was not called
	//

	if (__sys_gethostname(RealName, sizeof(RealName)) != 0)
		return KERNEL_SOCKET_ERROR;

	len = (int)wcslen(Kernel_CustomHostName);
	if (! name || namelen <= len) {
		SetLastError(KERNEL_WSAEFAULT);
		return KERNEL_SOCKET_ERROR;
	}

	for (i = 0; i <= len; i++) // the host name contains only ASCII characters
		name[i] = (char)Kernel_CustomHostName[i];

	return 0;
}


//---------------------------------------------------------------------------
// Kernel_GetHostNameW
//---------------------------------------------------------------------------


_FX int Kernel_GetHostNameW(WCHAR *name, int namelen)
{
	WCHAR RealName[256];
	int len;

	if (__sys_GetHostNameW(RealName, ARRAYSIZE(RealName)) != 0)
		return KERNEL_SOCKET_ERROR;

	len = (int)wcslen(Kernel_CustomHostName);
	if (! name || namelen <= len) {
		SetLastError(KERNEL_WSAEFAULT);
		return KERNEL_SOCKET_ERROR;
	}

	wmemcpy(name, Kernel_CustomHostName, len + 1);
	return 0;
}


//---------------------------------------------------------------------------
// Kernel_GetUserNameW
//---------------------------------------------------------------------------


_FX BOOL Kernel_GetUserNameW(LPWSTR lpBuffer, LPDWORD pcbBuffer)
{
	return Kernel_CopyNameW(Kernel_CustomUserName, lpBuffer, pcbBuffer, ERROR_INSUFFICIENT_BUFFER, TRUE);
}


//---------------------------------------------------------------------------
// Kernel_GetUserNameA
//---------------------------------------------------------------------------


_FX BOOL Kernel_GetUserNameA(LPSTR lpBuffer, LPDWORD pcbBuffer)
{
	return Kernel_CopyNameA(Kernel_CustomUserName, lpBuffer, pcbBuffer, ERROR_INSUFFICIENT_BUFFER, TRUE);
}


//---------------------------------------------------------------------------
// Kernel_GetCustomUserNameEx
//---------------------------------------------------------------------------


_FX ULONG Kernel_GetCustomUserNameEx(ULONG NameFormat, WCHAR *Name, ULONG NameSize)
{
	WCHAR RealName[MAX_PATH];
	ULONG RealLen;
	WCHAR *Sep;

	switch (NameFormat) {

	case KERNEL_NAME_SAM_COMPATIBLE:

		//
		// DOMAIN\User, for local accounts DOMAIN is the computer name
		//

		RealLen = ARRAYSIZE(RealName);
		if (! __sys_GetUserNameExW(NameFormat, RealName, &RealLen))
			return ERROR_NOT_SUPPORTED;
		Sep = wcschr(RealName, L'\\');
		if (! Sep)
			return ERROR_NOT_SUPPORTED;
		*Sep = L'\0';

		if (*Kernel_CustomNetBiosName && _wcsicmp(RealName, Kernel_RealNetBiosName) == 0)
			wcscpy_s(Name, NameSize, Kernel_CustomNetBiosName);
		else
			wcscpy_s(Name, NameSize, RealName);
		wcscat_s(Name, NameSize, L"\\");
		wcscat_s(Name, NameSize, *Kernel_CustomUserName ? Kernel_CustomUserName : Sep + 1);
		return ERROR_SUCCESS;

	case KERNEL_NAME_DISPLAY:

		if (! *Kernel_CustomUserName)
			break;
		wcscpy_s(Name, NameSize, Kernel_CustomUserName);
		return ERROR_SUCCESS;

	case KERNEL_NAME_USER_PRINCIPAL:

		//
		// a local account has no UPN, so do not reveal the e-mail
		// address of a Microsoft account
		//

		if (! *Kernel_CustomUserName)
			break;
		return ERROR_NONE_MAPPED;
	}

	return ERROR_NOT_SUPPORTED;
}


//---------------------------------------------------------------------------
// Kernel_GetUserNameExW
//---------------------------------------------------------------------------


_FX BOOLEAN Kernel_GetUserNameExW(ULONG NameFormat, LPWSTR lpNameBuffer, PULONG nSize)
{
	WCHAR Name[MAX_PATH * 2];

	ULONG Error = Kernel_GetCustomUserNameEx(NameFormat, Name, ARRAYSIZE(Name));
	if (Error == ERROR_NOT_SUPPORTED)
		return __sys_GetUserNameExW(NameFormat, lpNameBuffer, nSize);
	if (Error != ERROR_SUCCESS) {
		SetLastError(Error);
		return FALSE;
	}

	return Kernel_CopyNameW(Name, lpNameBuffer, nSize, ERROR_MORE_DATA, FALSE) ? TRUE : FALSE;
}


//---------------------------------------------------------------------------
// Kernel_GetUserNameExA
//---------------------------------------------------------------------------


_FX BOOLEAN Kernel_GetUserNameExA(ULONG NameFormat, LPSTR lpNameBuffer, PULONG nSize)
{
	WCHAR Name[MAX_PATH * 2];

	ULONG Error = Kernel_GetCustomUserNameEx(NameFormat, Name, ARRAYSIZE(Name));
	if (Error == ERROR_NOT_SUPPORTED)
		return __sys_GetUserNameExA(NameFormat, lpNameBuffer, nSize);
	if (Error != ERROR_SUCCESS) {
		SetLastError(Error);
		return FALSE;
	}

	return Kernel_CopyNameA(Name, lpNameBuffer, nSize, ERROR_MORE_DATA, FALSE) ? TRUE : FALSE;
}


//---------------------------------------------------------------------------
// Kernel_LoadTimeZone
//---------------------------------------------------------------------------


_FX BOOLEAN Kernel_LoadTimeZone(const WCHAR *KeyName)
{
	static const WCHAR *TimeZonesKey =
		L"\\registry\\machine\\software\\microsoft\\windows nt\\currentversion\\time zones\\";

	struct {
		LONG Bias;
		LONG StandardBias;
		LONG DaylightBias;
		SYSTEMTIME StandardDate;
		SYSTEMTIME DaylightDate;
	} RegTzi;

	union {
		KEY_VALUE_PARTIAL_INFORMATION info;
		UCHAR space[sizeof(KEY_VALUE_PARTIAL_INFORMATION) + 64];
	} value;

	NTSTATUS status;
	OBJECT_ATTRIBUTES objattrs;
	UNICODE_STRING uni;
	HANDLE handle;
	WCHAR path[256];
	WCHAR *ptr;
	ULONG len;
	BOOLEAN ok = FALSE;

	ULONG Wow64 = Dll_IsWin64 ? KEY_WOW64_64KEY : 0;

	if (wcschr(KeyName, L'\\') ||
			wcslen(KeyName) >= ARRAYSIZE(Kernel_CustomTimeZone.TimeZoneKeyName) ||
			wcslen(TimeZonesKey) + wcslen(KeyName) >= ARRAYSIZE(path))
		return FALSE;

	wcscpy(path, TimeZonesKey);
	wcscat(path, KeyName);

	RtlInitUnicodeString(&uni, path);
	InitializeObjectAttributes(&objattrs, &uni, OBJ_CASE_INSENSITIVE, NULL, NULL);

	status = NtOpenKey(&handle, KEY_READ | Wow64, &objattrs);
	if (! NT_SUCCESS(status))
		return FALSE;

	RtlInitUnicodeString(&uni, L"TZI");
	status = NtQueryValueKey(
		handle, &uni, KeyValuePartialInformation, &value, sizeof(value), &len);
	if (NT_SUCCESS(status) && value.info.Type == REG_BINARY &&
			value.info.DataLength >= sizeof(RegTzi)) {

		memcpy(&RegTzi, value.info.Data, sizeof(RegTzi));
		ok = TRUE;
	}

	NtClose(handle);

	if (! ok)
		return FALSE;

	memset(&Kernel_CustomTimeZone, 0, sizeof(Kernel_CustomTimeZone));
	Kernel_CustomTimeZone.Bias = RegTzi.Bias;
	Kernel_CustomTimeZone.StandardBias = RegTzi.StandardBias;
	Kernel_CustomTimeZone.DaylightBias = RegTzi.DaylightBias;
	Kernel_CustomTimeZone.StandardDate = RegTzi.StandardDate;
	Kernel_CustomTimeZone.DaylightDate = RegTzi.DaylightDate;
	wcscpy(Kernel_CustomTimeZone.TimeZoneKeyName, KeyName);
	Kernel_CustomTimeZone.DynamicDaylightTimeDisabled = FALSE;

	//
	// the display names in the registry are localized to the host's
	// language, use the language neutral key name instead, e.g.
	// "Tokyo Standard Time", and "... Daylight Time" for daylight time
	//

	wcsncpy(Kernel_CustomTimeZone.StandardName, KeyName, ARRAYSIZE(Kernel_CustomTimeZone.StandardName) - 1);
	wcsncpy(Kernel_CustomTimeZone.DaylightName, KeyName, ARRAYSIZE(Kernel_CustomTimeZone.DaylightName) - 1);
	ptr = wcsstr(Kernel_CustomTimeZone.DaylightName, L"Standard");
	if (ptr)
		wmemcpy(ptr, L"Daylight", 8);

	return TRUE;
}


//---------------------------------------------------------------------------
// Kernel_InitTimeZone
//---------------------------------------------------------------------------


_FX BOOLEAN Kernel_InitTimeZone(HMODULE module)
{
	WCHAR KeyName[ARRAYSIZE(Kernel_CustomTimeZone.TimeZoneKeyName)];

	SbieDll_GetSettingsForName(NULL, Dll_ImageName, L"CustomTimeZone", KeyName, sizeof(KeyName), NULL);
	if (! *KeyName || ! Kernel_LoadTimeZone(KeyName))
		return TRUE;

	InitializeCriticalSection(&Kernel_TimeZone_CritSec);

	{
		void *GetTimeZoneInformation = Kernel_GetProcAddress("GetTimeZoneInformation");
		void *GetDynamicTimeZoneInformation = Kernel_GetProcAddress("GetDynamicTimeZoneInformation");
		void *GetTimeZoneInformationForYear = Kernel_GetProcAddress("GetTimeZoneInformationForYear");
		void *SystemTimeToTzSpecificLocalTime = Kernel_GetProcAddress("SystemTimeToTzSpecificLocalTime");
		void *SystemTimeToTzSpecificLocalTimeEx = Kernel_GetProcAddress("SystemTimeToTzSpecificLocalTimeEx");
		void *TzSpecificLocalTimeToSystemTime = Kernel_GetProcAddress("TzSpecificLocalTimeToSystemTime");
		void *TzSpecificLocalTimeToSystemTimeEx = Kernel_GetProcAddress("TzSpecificLocalTimeToSystemTimeEx");
		void *GetLocalTime = Kernel_GetProcAddress("GetLocalTime");
		void *FileTimeToLocalFileTime = Kernel_GetProcAddress("FileTimeToLocalFileTime");
		void *LocalFileTimeToFileTime = Kernel_GetProcAddress("LocalFileTimeToFileTime");

		//
		// all other hooks depend on the time zone conversion functions
		//

		if (! SystemTimeToTzSpecificLocalTime || ! TzSpecificLocalTimeToSystemTime)
			return TRUE;

		SBIEDLL_HOOK(Kernel_, SystemTimeToTzSpecificLocalTime);
		SBIEDLL_HOOK(Kernel_, TzSpecificLocalTimeToSystemTime);

		if (GetTimeZoneInformationForYear) {
			SBIEDLL_HOOK(Kernel_, GetTimeZoneInformationForYear);
		}
		if (SystemTimeToTzSpecificLocalTimeEx) {
			SBIEDLL_HOOK(Kernel_, SystemTimeToTzSpecificLocalTimeEx);
		}
		if (TzSpecificLocalTimeToSystemTimeEx) {
			SBIEDLL_HOOK(Kernel_, TzSpecificLocalTimeToSystemTimeEx);
		}
		if (GetTimeZoneInformation) {
			SBIEDLL_HOOK(Kernel_, GetTimeZoneInformation);
		}
		if (GetDynamicTimeZoneInformation) {
			SBIEDLL_HOOK(Kernel_, GetDynamicTimeZoneInformation);
		}
		if (GetLocalTime) {
			SBIEDLL_HOOK(Kernel_, GetLocalTime);
		}
		if (FileTimeToLocalFileTime) {
			SBIEDLL_HOOK(Kernel_, FileTimeToLocalFileTime);
		}
		if (LocalFileTimeToFileTime) {
			SBIEDLL_HOOK(Kernel_, LocalFileTimeToFileTime);
		}
	}

	return TRUE;
}


//---------------------------------------------------------------------------
// Kernel_GetCustomTimeZoneForYear
//---------------------------------------------------------------------------


_FX void Kernel_GetCustomTimeZoneForYear(USHORT wYear, TIME_ZONE_INFORMATION *tzi)
{
	//
	// GetTimeZoneInformationForYear reads the dynamic DST rules from the
	// registry, cache the result as this is called for every conversion
	//

	EnterCriticalSection(&Kernel_TimeZone_CritSec);

	if (Kernel_CustomTimeZoneYear != wYear) {

		KERNEL_DYNAMIC_TIME_ZONE_INFORMATION dtzi = Kernel_CustomTimeZone;

		if (! __sys_GetTimeZoneInformationForYear ||
				! __sys_GetTimeZoneInformationForYear(wYear, &dtzi, &Kernel_CustomTimeZoneForYear)) {

			memcpy(&Kernel_CustomTimeZoneForYear, &Kernel_CustomTimeZone, sizeof(TIME_ZONE_INFORMATION));
		}

		wcscpy(Kernel_CustomTimeZoneForYear.StandardName, Kernel_CustomTimeZone.StandardName);
		wcscpy(Kernel_CustomTimeZoneForYear.DaylightName, Kernel_CustomTimeZone.DaylightName);

		Kernel_CustomTimeZoneYear = wYear;
	}

	*tzi = Kernel_CustomTimeZoneForYear;

	LeaveCriticalSection(&Kernel_TimeZone_CritSec);
}


//---------------------------------------------------------------------------
// Kernel_GetCustomTimeZoneNow
//---------------------------------------------------------------------------


_FX DWORD Kernel_GetCustomTimeZoneNow(TIME_ZONE_INFORMATION *tzi, LONG *CurrentBias)
{
	SYSTEMTIME UtcTime, LocalTime;
	FILETIME UtcFileTime, LocalFileTime;
	LARGE_INTEGER Utc, Local;
	LONG Bias;

	GetSystemTime(&UtcTime);
	Kernel_GetCustomTimeZoneForYear(UtcTime.wYear, tzi);

	//
	// let Windows evaluate the DST transition dates and derive
	// the bias which is currently in effect from the result
	//

	Bias = tzi->Bias + tzi->StandardBias;
	if (__sys_SystemTimeToTzSpecificLocalTime(tzi, &UtcTime, &LocalTime) &&
			SystemTimeToFileTime(&UtcTime, &UtcFileTime) &&
			SystemTimeToFileTime(&LocalTime, &LocalFileTime)) {

		Utc.LowPart = UtcFileTime.dwLowDateTime;
		Utc.HighPart = UtcFileTime.dwHighDateTime;
		Local.LowPart = LocalFileTime.dwLowDateTime;
		Local.HighPart = LocalFileTime.dwHighDateTime;
		Bias = (LONG)((Utc.QuadPart - Local.QuadPart) / KERNEL_FILETIME_PER_MINUTE);
	}

	if (CurrentBias)
		*CurrentBias = Bias;

	if (tzi->DaylightDate.wMonth == 0 || tzi->StandardDate.wMonth == 0)
		return TIME_ZONE_ID_UNKNOWN;

	if (tzi->DaylightBias != tzi->StandardBias && Bias == tzi->Bias + tzi->DaylightBias)
		return TIME_ZONE_ID_DAYLIGHT;

	return TIME_ZONE_ID_STANDARD;
}


//---------------------------------------------------------------------------
// Kernel_GetTimeZoneInformation
//---------------------------------------------------------------------------


_FX DWORD Kernel_GetTimeZoneInformation(LPTIME_ZONE_INFORMATION lpTimeZoneInformation)
{
	TIME_ZONE_INFORMATION tzi;
	DWORD id;

	if (! lpTimeZoneInformation) {
		SetLastError(ERROR_INVALID_PARAMETER);
		return TIME_ZONE_ID_INVALID;
	}

	id = Kernel_GetCustomTimeZoneNow(&tzi, NULL);
	*lpTimeZoneInformation = tzi;
	return id;
}


//---------------------------------------------------------------------------
// Kernel_GetDynamicTimeZoneInformation
//---------------------------------------------------------------------------


_FX DWORD Kernel_GetDynamicTimeZoneInformation(KERNEL_DYNAMIC_TIME_ZONE_INFORMATION *pTimeZoneInformation)
{
	TIME_ZONE_INFORMATION tzi;
	DWORD id;

	if (! pTimeZoneInformation) {
		SetLastError(ERROR_INVALID_PARAMETER);
		return TIME_ZONE_ID_INVALID;
	}

	id = Kernel_GetCustomTimeZoneNow(&tzi, NULL);

	//
	// TIME_ZONE_INFORMATION is the leading part of DYNAMIC_TIME_ZONE_INFORMATION
	//

	*pTimeZoneInformation = Kernel_CustomTimeZone;
	memcpy(pTimeZoneInformation, &tzi, sizeof(TIME_ZONE_INFORMATION));
	return id;
}


//---------------------------------------------------------------------------
// Kernel_GetTimeZoneInformationForYear
//---------------------------------------------------------------------------


_FX BOOL Kernel_GetTimeZoneInformationForYear(USHORT wYear, KERNEL_DYNAMIC_TIME_ZONE_INFORMATION *pdtzi, LPTIME_ZONE_INFORMATION ptzi)
{
	if (pdtzi)
		return __sys_GetTimeZoneInformationForYear(wYear, pdtzi, ptzi);

	if (! ptzi) {
		SetLastError(ERROR_INVALID_PARAMETER);
		return FALSE;
	}

	Kernel_GetCustomTimeZoneForYear(wYear, ptzi);
	return TRUE;
}


//---------------------------------------------------------------------------
// Kernel_SystemTimeToTzSpecificLocalTime
//---------------------------------------------------------------------------


_FX BOOL Kernel_SystemTimeToTzSpecificLocalTime(const TIME_ZONE_INFORMATION *lpTimeZoneInformation, const SYSTEMTIME *lpUniversalTime, LPSYSTEMTIME lpLocalTime)
{
	TIME_ZONE_INFORMATION tzi;

	if (lpTimeZoneInformation || ! lpUniversalTime)
		return __sys_SystemTimeToTzSpecificLocalTime(lpTimeZoneInformation, lpUniversalTime, lpLocalTime);

	Kernel_GetCustomTimeZoneForYear(lpUniversalTime->wYear, &tzi);
	return __sys_SystemTimeToTzSpecificLocalTime(&tzi, lpUniversalTime, lpLocalTime);
}


//---------------------------------------------------------------------------
// Kernel_SystemTimeToTzSpecificLocalTimeEx
//---------------------------------------------------------------------------


_FX BOOL Kernel_SystemTimeToTzSpecificLocalTimeEx(const KERNEL_DYNAMIC_TIME_ZONE_INFORMATION *lpTimeZoneInformation, const SYSTEMTIME *lpUniversalTime, LPSYSTEMTIME lpLocalTime)
{
	KERNEL_DYNAMIC_TIME_ZONE_INFORMATION dtzi;

	if (lpTimeZoneInformation)
		return __sys_SystemTimeToTzSpecificLocalTimeEx(lpTimeZoneInformation, lpUniversalTime, lpLocalTime);

	dtzi = Kernel_CustomTimeZone;
	return __sys_SystemTimeToTzSpecificLocalTimeEx(&dtzi, lpUniversalTime, lpLocalTime);
}


//---------------------------------------------------------------------------
// Kernel_TzSpecificLocalTimeToSystemTime
//---------------------------------------------------------------------------


_FX BOOL Kernel_TzSpecificLocalTimeToSystemTime(const TIME_ZONE_INFORMATION *lpTimeZoneInformation, const SYSTEMTIME *lpLocalTime, LPSYSTEMTIME lpUniversalTime)
{
	TIME_ZONE_INFORMATION tzi;

	if (lpTimeZoneInformation || ! lpLocalTime)
		return __sys_TzSpecificLocalTimeToSystemTime(lpTimeZoneInformation, lpLocalTime, lpUniversalTime);

	Kernel_GetCustomTimeZoneForYear(lpLocalTime->wYear, &tzi);
	return __sys_TzSpecificLocalTimeToSystemTime(&tzi, lpLocalTime, lpUniversalTime);
}


//---------------------------------------------------------------------------
// Kernel_TzSpecificLocalTimeToSystemTimeEx
//---------------------------------------------------------------------------


_FX BOOL Kernel_TzSpecificLocalTimeToSystemTimeEx(const KERNEL_DYNAMIC_TIME_ZONE_INFORMATION *lpTimeZoneInformation, const SYSTEMTIME *lpLocalTime, LPSYSTEMTIME lpUniversalTime)
{
	KERNEL_DYNAMIC_TIME_ZONE_INFORMATION dtzi;

	if (lpTimeZoneInformation)
		return __sys_TzSpecificLocalTimeToSystemTimeEx(lpTimeZoneInformation, lpLocalTime, lpUniversalTime);

	dtzi = Kernel_CustomTimeZone;
	return __sys_TzSpecificLocalTimeToSystemTimeEx(&dtzi, lpLocalTime, lpUniversalTime);
}


//---------------------------------------------------------------------------
// Kernel_GetLocalTime
//---------------------------------------------------------------------------


_FX void Kernel_GetLocalTime(LPSYSTEMTIME lpSystemTime)
{
	TIME_ZONE_INFORMATION tzi;
	SYSTEMTIME UtcTime;

	GetSystemTime(&UtcTime);
	Kernel_GetCustomTimeZoneForYear(UtcTime.wYear, &tzi);
	if (! __sys_SystemTimeToTzSpecificLocalTime(&tzi, &UtcTime, lpSystemTime))
		__sys_GetLocalTime(lpSystemTime);
}


//---------------------------------------------------------------------------
// Kernel_FileTimeToLocalFileTime
//---------------------------------------------------------------------------


_FX BOOL Kernel_FileTimeToLocalFileTime(const FILETIME *lpFileTime, LPFILETIME lpLocalFileTime)
{
	TIME_ZONE_INFORMATION tzi;
	LARGE_INTEGER Time;
	LONG Bias;

	if (! lpFileTime || ! lpLocalFileTime)
		return __sys_FileTimeToLocalFileTime(lpFileTime, lpLocalFileTime);

	//
	// like the original, use the bias which is currently in effect,
	// regardless of the DST state at the given time
	//

	Kernel_GetCustomTimeZoneNow(&tzi, &Bias);

	Time.LowPart = lpFileTime->dwLowDateTime;
	Time.HighPart = lpFileTime->dwHighDateTime;
	Time.QuadPart -= (LONGLONG)Bias * KERNEL_FILETIME_PER_MINUTE;
	lpLocalFileTime->dwLowDateTime = Time.LowPart;
	lpLocalFileTime->dwHighDateTime = Time.HighPart;
	return TRUE;
}


//---------------------------------------------------------------------------
// Kernel_LocalFileTimeToFileTime
//---------------------------------------------------------------------------


_FX BOOL Kernel_LocalFileTimeToFileTime(const FILETIME *lpLocalFileTime, LPFILETIME lpFileTime)
{
	TIME_ZONE_INFORMATION tzi;
	LARGE_INTEGER Time;
	LONG Bias;

	if (! lpLocalFileTime || ! lpFileTime)
		return __sys_LocalFileTimeToFileTime(lpLocalFileTime, lpFileTime);

	Kernel_GetCustomTimeZoneNow(&tzi, &Bias);

	Time.LowPart = lpLocalFileTime->dwLowDateTime;
	Time.HighPart = lpLocalFileTime->dwHighDateTime;
	Time.QuadPart += (LONGLONG)Bias * KERNEL_FILETIME_PER_MINUTE;
	lpFileTime->dwLowDateTime = Time.LowPart;
	lpFileTime->dwHighDateTime = Time.HighPart;
	return TRUE;
}
