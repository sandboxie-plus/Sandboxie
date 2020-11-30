/*
 * Copyright 2020 DavidXanatos, xanasoft.com
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



#include <ntstatus.h>
#define WIN32_NO_STATUS
typedef long NTSTATUS;

#include <windows.h>
#include "win32_ntddk.h"

//
// Do not include any external CRT into sboxdll project. 
// This DLL is injected early into the process start up sequence and adding dependencies may break Sandboxie. 
// Normally we link directly to the CRT build into ntdll.dll.
// As more recent versions of ntdll.lib are not offering many CRT functions we have to dynamically link them.
// If you are missing some expected functions we probably just not added them here yet, so just add what you need.
//

//int __cdecl __stdio_common_vswprintf(unsigned __int64 options, wchar_t *str, size_t len, const wchar_t *format, _locale_t locale, va_list valist);
//int __cdecl __stdio_common_vfwprintf(unsigned __int64 options, FILE *file, const wchar_t *format, _locale_t locale, va_list valist);
//int __cdecl __stdio_common_vswscanf(unsigned __int64 options, const wchar_t *input, size_t length, const wchar_t *format, _locale_t locale, va_list valist);
//int __cdecl __stdio_common_vfwscanf(unsigned __int64 options, FILE *file, const wchar_t *format, _locale_t locale, va_list valist);

int(*P_vsnwprintf)(wchar_t *_Buffer, size_t Count, const wchar_t * const, va_list Args) = NULL;
int(*P_vsnprintf)(char *_Buffer, size_t Count, const char * const, va_list Args) = NULL;

#ifndef OLD_DDK
void*(__cdecl *Pmemcpy)(void* _Dst, void const* _Src, size_t _Size) = NULL;
void* __cdecl memcpy(void* _Dst, void const* _Src, size_t _Size) { return Pmemcpy(_Dst, _Src, _Size); }

void*(__cdecl *Pmemset)(void* _Dst, int _Val, size_t _Size) = NULL;
void* __cdecl memset(void* _Dst, int _Val, size_t _Size) { return Pmemset(_Dst, _Val, _Size); }


wchar_t*(__cdecl *Pwcscpy)(wchar_t *dest, const wchar_t *src) = NULL;
wchar_t* __cdecl wcscpy(wchar_t *dest, const wchar_t *src) { return Pwcscpy(dest, src); }

size_t(__cdecl *Pwcslen)(wchar_t const* _String) = NULL;
size_t __cdecl wcslen(wchar_t const* _String) { return Pwcslen(_String); }

int(__cdecl *Pmemcmp)(void const* _Buf1, void const* _Buf2, size_t _Size) = NULL;
int __cdecl memcmp(void const* _Buf1, void const* _Buf2, size_t _Size) { return Pmemcmp(_Buf1, _Buf2, _Size); }

wchar_t*(__cdecl *Pwcscat)(wchar_t *dest, const wchar_t *src) = NULL;
wchar_t* __cdecl wcscat(wchar_t *dest, const wchar_t *src) { return Pwcscat(dest, src); }

int(__cdecl *Pwcscmp)(char const* _String1, char const* _String2) = NULL;
int __cdecl wcscmp(wchar_t const* _String1, wchar_t const* _String2) { return Pwcscmp(_String1, _String2); }

size_t(__cdecl *Pstrlen)(char const*_Str) = NULL;
size_t __cdecl strlen(char const* _Str) { return Pstrlen(_Str); }

char*(__cdecl *Pstrcpy)(char * destination, const char * source) = NULL;
char * __cdecl strcpy(char * destination, const char * source) { return Pstrcpy(destination, source); }

int(__cdecl *Pstrcmp)(char const* _Str1, char const* _Str2) = NULL;
int __cdecl strcmp(char const* _Str1, char const* _Str2) { return Pstrcmp(_Str1, _Str2); }


int(__cdecl *P_wcsnicmp)(wchar_t const* _String1, wchar_t const* _String2, size_t _MaxCount) = NULL;
int __cdecl _wcsnicmp(wchar_t const* _String1, wchar_t const* _String2, size_t _MaxCount) { return P_wcsnicmp(_String1, _String2, _MaxCount); }

wint_t(__cdecl *Ptowupper)(wint_t _C) = NULL;
wint_t __cdecl towupper(_In_ wint_t _C) { return Ptowupper(_C); }

wint_t(__cdecl *Ptowlower)(wint_t _C) = NULL;
wint_t __cdecl towlower(_In_ wint_t _C) { return Ptowlower(_C); }

int(__cdecl *Piswctype)(_In_ wint_t _C, _In_ wctype_t _Type) = NULL;
int __cdecl iswctype(_In_ wint_t _C, _In_ wctype_t _Type) { return Piswctype(_C, _Type); }

void*(__cdecl *Pmemmove)(void* _Dst, void const* _Src, size_t _Size) = NULL;
void* __cdecl memmove(void* _Dst, void const* _Src, size_t _Size) { return Pmemmove(_Dst, _Src, _Size); }

wchar_t *(__cdecl *Pwcschr)(wchar_t const* _Str, wchar_t _Ch) = NULL;
wchar_t * __cdecl wcschr(wchar_t const* _Str, wchar_t _Ch) { return Pwcschr(_Str, _Ch); }

wchar_t*(__cdecl *P_itow)(int value, wchar_t *buffer, int radix) = NULL;
wchar_t * __cdecl _itow(int value, wchar_t *buffer, int radix) { return P_itow(value, buffer, radix); }

int(__cdecl *Pwcsncmp)(wchar_t const* _String1, wchar_t const* _String2, size_t _MaxCount) = NULL;
int __cdecl wcsncmp(wchar_t const* _String1, wchar_t const* _String2, size_t _MaxCount) { return Pwcsncmp(_String1, _String2, _MaxCount); }

int(__cdecl *P_wcsicmp)(wchar_t const* _String1, wchar_t const* _String2) = NULL;
int __cdecl _wcsicmp(wchar_t const* _String1, wchar_t const* _String2) { return P_wcsicmp(_String1, _String2); }

wchar_t *(__cdecl *Pwcsrchr)(wchar_t const* _Str, wchar_t _Ch) = NULL;
wchar_t * __cdecl wcsrchr(wchar_t const* _Str, wchar_t _Ch) { return Pwcsrchr(_Str, _Ch); }

wchar_t *(__cdecl *Pwcsstr)(wchar_t const* _Str, wchar_t const* _SubStr) = NULL;
wchar_t * __cdecl wcsstr(wchar_t const* _Str, wchar_t const* _SubStr) { return Pwcsstr(_Str, _SubStr); }

wchar_t*(__cdecl *P_wcslwr)(wchar_t *string) = NULL;
wchar_t *__cdecl _wcslwr(wchar_t *string) { return P_wcslwr(string); }

int(__cdecl *P_stricmp)(char const* _String1, char const* _String2) = NULL;
int __cdecl _stricmp(char const* _String1, char const* _String2) { return P_stricmp(_String1, _String2); }

long(__cdecl *Pwcstol)(wchar_t const* _String, wchar_t ** _EndPtr, int _Radix) = NULL;
long __cdecl wcstol(wchar_t const* _String, wchar_t** _EndPtr, int _Radix) { return Pwcstol(_String, _EndPtr, _Radix); }

wchar_t*(__cdecl *Pwcsncpy)(wchar_t* dest, wchar_t const* src, size_t count) = NULL;
wchar_t* __cdecl wcsncpy(wchar_t* dest, const wchar_t* src, size_t count) { return Pwcsncpy(dest, src, count); }

__int64(__cdecl *P_wtoi64)(wchar_t const *_String) = NULL;
__int64 __cdecl _wtoi64(wchar_t const* _String) { return P_wtoi64(_String); }

char*(__cdecl *Pstrchr)(const char *s, int c) = NULL;
char* __cdecl strchr(const char *s, int c) { return Pstrchr(s, c); }

int(__cdecl *Pstrncmp)(const char * str1, const char * str2, size_t num);
int __cdecl strncmp(const char * str1, const char * str2, size_t num) { return Pstrncmp(str1, str2, num); }

unsigned long(__cdecl *Pwcstoul)(wchar_t const* _String, wchar_t ** _EndPtr, int _Radix) = NULL;
unsigned long __cdecl wcstoul(wchar_t const* _String, wchar_t** _EndPtr, int _Radix) { return Pwcstoul(_String, _EndPtr, _Radix); }

int(__cdecl *Ptolower)(int _C) = NULL;
int __cdecl tolower(int _C) { return Ptolower(_C); }

int(__cdecl *P_wtoi)(wchar_t const *_String) = NULL;
int __cdecl _wtoi(wchar_t const* _String) { return P_wtoi(_String); }

char *(__cdecl *Pstrstr)(char const* _Str, char const* _SubStr) = NULL;
char * __cdecl strstr(char const* _Str, char const* _SubStr) { return Pstrstr(_Str, _SubStr); }

char*(__cdecl *P_strlwr)(const* str) = NULL;
char* __cdecl _strlwr(char* str) { return P_strlwr(str); }

#ifndef _WIN64
int(__cdecl *P_except_handler3)(void* exception_record, void* registration, void* context, void* dispatcher);
int __cdecl _except_handler3(void* exception_record, void* registration, void* context, void* dispatcher) {
	return P_except_handler3(exception_record, registration, context, dispatcher);
}
#else
EXCEPTION_DISPOSITION(__cdecl *P__C_specific_handler)(struct _EXCEPTION_RECORD *ExceptionRecord, void* EstablisherFrame, struct _CONTEXT* ContextRecord, struct _DISPATCHER_CONTEXT *DispatcherContext) = NULL;
EXCEPTION_DISPOSITION __cdecl __C_specific_handler(struct _EXCEPTION_RECORD *ExceptionRecord, void* EstablisherFrame, struct _CONTEXT* ContextRecord, struct _DISPATCHER_CONTEXT *DispatcherContext) {
	return P__C_specific_handler(ExceptionRecord, EstablisherFrame, ContextRecord, DispatcherContext);
}

ULONG(__cdecl *P__chkstk)() = NULL;
ULONG __cdecl __chkstk() { return P__chkstk(); }
#endif
#endif

void InitMyNtDll(HMODULE Ntdll)
{
	*(FARPROC*)&P_vsnwprintf = GetProcAddress(Ntdll, "_vsnwprintf");
	*(FARPROC*)&P_vsnprintf = GetProcAddress(Ntdll, "_vsnprintf");

#ifndef OLD_DDK

	*(FARPROC*)&Pmemcpy = GetProcAddress(Ntdll, "memcpy");
	*(FARPROC*)&Pmemset = GetProcAddress(Ntdll, "memset");
	*(FARPROC*)&Pwcscpy = GetProcAddress(Ntdll, "wcscpy");
	*(FARPROC*)&Pwcslen = GetProcAddress(Ntdll, "wcslen");
	*(FARPROC*)&Pmemcmp = GetProcAddress(Ntdll, "memcmp");
	*(FARPROC*)&Pwcscat = GetProcAddress(Ntdll, "wcscat");
	*(FARPROC*)&Pwcscmp = GetProcAddress(Ntdll, "wcscmp");
	*(FARPROC*)&Pstrlen = GetProcAddress(Ntdll, "strlen");
	*(FARPROC*)&Pstrcpy = GetProcAddress(Ntdll, "strcpy");
	*(FARPROC*)&Pstrcmp = GetProcAddress(Ntdll, "strcmp");
	*(FARPROC*)&P_wcsnicmp = GetProcAddress(Ntdll, "_wcsnicmp");
	*(FARPROC*)&Ptowupper = GetProcAddress(Ntdll, "towupper");
	*(FARPROC*)&Ptowlower = GetProcAddress(Ntdll, "towlower");
	*(FARPROC*)&Piswctype = GetProcAddress(Ntdll, "iswctype");
	*(FARPROC*)&Pmemmove = GetProcAddress(Ntdll, "memmove");
	*(FARPROC*)&Pwcschr = GetProcAddress(Ntdll, "wcschr");
	*(FARPROC*)&P_itow = GetProcAddress(Ntdll, "_itow");
	*(FARPROC*)&Pwcsncmp = GetProcAddress(Ntdll, "wcsncmp");
	*(FARPROC*)&P_wcsicmp = GetProcAddress(Ntdll, "_wcsicmp");
	*(FARPROC*)&Pwcsrchr = GetProcAddress(Ntdll, "wcsrchr");
	*(FARPROC*)&Pwcsstr = GetProcAddress(Ntdll, "wcsstr");
	*(FARPROC*)&P_wcslwr = GetProcAddress(Ntdll, "_wcslwr");
	*(FARPROC*)&P_stricmp = GetProcAddress(Ntdll, "_stricmp");
	*(FARPROC*)&Pwcstol = GetProcAddress(Ntdll, "wcstol");
	*(FARPROC*)&Pwcsncpy = GetProcAddress(Ntdll, "wcsncpy");
	*(FARPROC*)&P_wtoi64 = GetProcAddress(Ntdll, "_wtoi64");
	*(FARPROC*)&Pstrchr = GetProcAddress(Ntdll, "strchr");
	*(FARPROC*)&Pstrncmp = GetProcAddress(Ntdll, "strncmp");
	*(FARPROC*)&Pwcstoul = GetProcAddress(Ntdll, "wcstoul");
	*(FARPROC*)&Ptolower = GetProcAddress(Ntdll, "tolower");
	*(FARPROC*)&P_wtoi = GetProcAddress(Ntdll, "_wtoi");
	*(FARPROC*)&Pstrstr = GetProcAddress(Ntdll, "strstr");
	*(FARPROC*)&P_strlwr = GetProcAddress(Ntdll, "_strlwr");

#ifndef _WIN64
	*(FARPROC*)&P_except_handler3 = GetProcAddress(Ntdll, "_except_handler3");
#else	
	*(FARPROC*)&P__C_specific_handler = GetProcAddress(Ntdll, "__C_specific_handler");
	*(FARPROC*)&P__chkstk = GetProcAddress(Ntdll, "__chkstk");
#endif

#endif
}





