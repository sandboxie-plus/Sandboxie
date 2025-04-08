/*
 * Copyright 2004-2020 Sandboxie Holdings, LLC 
 * Copyright 2020 David Xanatos, xanasoft.com
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
// GUI Services
//---------------------------------------------------------------------------

#define NOGDI
#include "dll.h"

#include "gui_p.h"
#include "core/svc/GuiWire.h"
#include <stdio.h>
#include "common\pattern.h"


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------


static ATOM Gui_RegisterClassA(WNDCLASSA *wcin);

static ATOM Gui_RegisterClassW(WNDCLASSW *wcin);

static ATOM Gui_RegisterClassExA(WNDCLASSEXA *wcin);

static ATOM Gui_RegisterClassExW(WNDCLASSEXW *wcin);

static BOOL Gui_UnregisterClassA(
    const UCHAR *lpClassName, HINSTANCE hInstance);

static BOOL Gui_UnregisterClassW(
    const WCHAR *lpClassName, HINSTANCE hInstance);

static ATOM Gui_GetClassInfoA(
    HINSTANCE hInstance, const UCHAR *lpClassName, WNDCLASSA *wcout);

static ATOM Gui_GetClassInfoW(
    HINSTANCE hInstance, const WCHAR *lpClassName, WNDCLASSW *wcout);

static ATOM Gui_GetClassInfoExA(
    HINSTANCE hInstance, const UCHAR *lpClassName, WNDCLASSEXA *wcout);

static ATOM Gui_GetClassInfoExW(
    HINSTANCE hInstance, const WCHAR *lpClassName, WNDCLASSEXW *wcout);

static int Gui_GetClassNameA(
    HWND hWnd, UCHAR *lpClassName, int nMaxCount);

static int Gui_GetClassNameW(
    HWND hWnd, WCHAR *lpClassName, int nMaxCount);

static ULONG Gui_GetClassName2(
    HWND hWnd, void *clsnm, ULONG maxlen, BOOLEAN unicode);

static BOOLEAN Gui_IsWellKnownClass(const WCHAR *iptr);

static BOOLEAN Gui_NoRenameClass(const WCHAR* iptr);

static ULONG_PTR Gui_CREATESTRUCT_Handler(ULONG_PTR *args);


//---------------------------------------------------------------------------


typedef ULONG_PTR (*P_CREATESTRUCT_Handler)(ULONG_PTR *args);


//---------------------------------------------------------------------------
// Variables
//---------------------------------------------------------------------------


static const WCHAR *Gui_comctl32 = L"comctl32.dll";
static const WCHAR *Gui_riched20 = L"riched20.dll";

static WCHAR *Gui_BoxPrefixW = NULL;
static char *Gui_BoxPrefixA = NULL;

static ULONG Gui_BoxPrefix_Len = 0;

static CRITICAL_SECTION Gui_IsOpenClass_CritSec;

static LIST Gui_NoRenameWinClasses;

static P_CREATESTRUCT_Handler __sys_CREATESTRUCT_Handler = NULL;

static ULONG_PTR Gui_HighestAddress = 0;

BOOLEAN Gui_RenameClasses = TRUE;
BOOLEAN Gui_OpenAllWinClasses = FALSE;
BOOLEAN Gui_UseProtectScreen = FALSE;
BOOLEAN Gui_UseBlockCapture = FALSE;


//---------------------------------------------------------------------------
// Gui_InitClass
//---------------------------------------------------------------------------


_FX BOOLEAN Gui_InitClass(HMODULE module)
{
    static const WCHAR *Sandbox = L"Sandbox";
    ULONG len;

    InitializeCriticalSection(&Gui_IsOpenClass_CritSec);

    List_Init(&Gui_NoRenameWinClasses);

    //
    // initialize BoxPrefix variables
    //

    len = (wcslen(Sandbox) + 1 + wcslen(Dll_BoxName) + 1 + 1)
        * sizeof(WCHAR);
    Gui_BoxPrefixW = Dll_Alloc(len);
    Sbie_snwprintf(Gui_BoxPrefixW, len / sizeof(WCHAR), L"%s:%s:", Sandbox, Dll_BoxName);
    Gui_BoxPrefix_Len = wcslen(Gui_BoxPrefixW);

    len = Gui_BoxPrefix_Len + 1;
    Gui_BoxPrefixA = Dll_Alloc(len);
    WideCharToMultiByte(
        CP_ACP, 0, Gui_BoxPrefixW, wcslen(Gui_BoxPrefixW),
        Gui_BoxPrefixA, len, NULL, NULL);
    Gui_BoxPrefixA[len - 1] = '\0';

    //
    // if OpenWinClass specifies *, we will not do any window class
    // renaming.  note that we can't use Gui_IsOpenClass yet, because
    // we can't issue PostMessage yet, and so the path list will not
    // be initialized yet (see Gui_IsOpenClass)
    //

    if (Dll_ProcessFlags & SBIE_FLAG_OPEN_ALL_WIN_CLASS) {

        Gui_OpenAllWinClasses = TRUE;
        Gui_RenameClasses = FALSE;

    } else {

        len = 0;

        while (1) {

            WCHAR buf[10];
            LONG rc = SbieApi_QueryConfAsIs(
                NULL, L"OpenWinClass", len, buf, 8 * sizeof(WCHAR));
            ++len;
            if (rc == 0) {
                if ((buf[0] == L'*' || buf[0] == L'#') && buf[1] == L'\0') {
                    Gui_RenameClasses = FALSE;
                    break;
                }
            } else if (rc != STATUS_BUFFER_TOO_SMALL)
                break;
        }
    }

    //
    // if OpenWinClass does not specify * or #, we might still need to
    // keep some window classes from getting the sandbox name prefix
    //

    if (Gui_RenameClasses) {

        Config_InitPatternList(NULL, L"NoRenameWinClass", &Gui_NoRenameWinClasses, FALSE);

        PATTERN* pat = List_Head(&Gui_NoRenameWinClasses);
        while (pat)
        {
            const WCHAR* patsrc = Pattern_Source(pat);
            if (patsrc[0] == L'*' && patsrc[1] == L'\0') {
                Gui_RenameClasses = FALSE;
                break;
            }
            pat = List_Next(pat);
        }
    }

    Gui_UseProtectScreen = SbieApi_QueryConfBool(NULL, L"CoverBoxedWindows", FALSE);

    //
    // hook functions
    //

    if (Gui_OpenAllWinClasses)
        return TRUE;

    SBIEDLL_HOOK_GUI(GetClassNameA);
    SBIEDLL_HOOK_GUI(GetClassNameW);

    if (! Gui_RenameClasses)
        return TRUE;

    SBIEDLL_HOOK_GUI(GetClassInfoA);
    SBIEDLL_HOOK_GUI(GetClassInfoW);
    SBIEDLL_HOOK_GUI(GetClassInfoExA);
    SBIEDLL_HOOK_GUI(GetClassInfoExW);

    SBIEDLL_HOOK_GUI(RegisterClassA);
    SBIEDLL_HOOK_GUI(RegisterClassW);
    SBIEDLL_HOOK_GUI(RegisterClassExA);
    SBIEDLL_HOOK_GUI(RegisterClassExW);

    SBIEDLL_HOOK_GUI(UnregisterClassA);
    SBIEDLL_HOOK_GUI(UnregisterClassW);

    return TRUE;
}


//---------------------------------------------------------------------------
// Gui_Hook_CREATESTRUCT_Handler
//---------------------------------------------------------------------------


_FX void Gui_Hook_CREATESTRUCT_Handler(void)
{
    SYSTEM_INFO sysinfo;
    ULONG_PTR peb, KernelCallbackTable;
    ULONG prot;

    //
    // the PEB includes a short table of callback functions that are
    // used by kernel side win32k to invoke user mode code.  index 10
    // is used to send the initial window creation message WM_CREATE.
    //
    // some window procedures look at the class name passed in
    // WM_CREATE and fail if it's not what they expect, which would
    // probably be the case, since we added the Sandbox:BoxName: prefix.
    // we need to hook index 10 and strip the sandbox prefix.
    // this is done by Gui_CREATESTRUCT_Handler, see there
    //

    if (! Gui_RenameClasses)
        return;

    GetSystemInfo(&sysinfo);
    Gui_HighestAddress = (ULONG_PTR)sysinfo.lpMaximumApplicationAddress;

#ifdef _WIN64
    peb = NtCurrentPeb();
    KernelCallbackTable = *(ULONG_PTR *)(peb + 0x58);
#else ! _WIN64
    peb = __readfsdword(0x30);    // PEB
    KernelCallbackTable = *(ULONG *)(peb + 0x2C);
#endif _WIN64

    if (KernelCallbackTable) {

        // index 10 is for __fnINLPCREATESTRUCT
        ULONG_PTR *funcptr = &((ULONG_PTR *)KernelCallbackTable)[10];

        if (*funcptr && VirtualProtect(
                funcptr, sizeof(ULONG_PTR), PAGE_READWRITE, &prot)) {

            __sys_CREATESTRUCT_Handler = (P_CREATESTRUCT_Handler)
                InterlockedExchangePointer((void **)funcptr,
                                           (void *)Gui_CREATESTRUCT_Handler);

            VirtualProtect(funcptr, sizeof(ULONG_PTR), prot, &prot);
        }
    }
}


//---------------------------------------------------------------------------
// Gui_CreateClassNameW
//---------------------------------------------------------------------------


_FX WCHAR *Gui_CreateClassNameW(const WCHAR *istr)
{
    int len;
    WCHAR *ostr;
    WCHAR clsnm[256];
    const WCHAR *iptr;

    if (((ULONG_PTR)istr & ((ULONG_PTR)LongToPtr(0xFFFF0000))) == 0) {
        ULONG_PTR atom = (ULONG_PTR)istr;
        if (atom <= 0xBFFF)
            return (WCHAR *)istr;
        if (__sys_GetClipboardFormatNameW((ATOM)atom, clsnm, 255) == 0)
            return (WCHAR *)istr;
        iptr = clsnm;
    } else
        iptr = istr;

#ifdef DEBUG_CREATECLASSNAME
    OutputDebugStringW(L"CreateClassNameW - Class Name Is\n");
    OutputDebugStringW(iptr);OutputDebugString(L"\n");
#endif

    if (wcsncmp(iptr, Gui_BoxPrefixW, Gui_BoxPrefix_Len) == 0)
        return (WCHAR *)istr;

    if (Gui_NoRenameClass(iptr) || Gui_IsOpenClass(iptr))
        return (WCHAR *)istr;

    len = (Gui_BoxPrefix_Len + wcslen(iptr) + 1) * sizeof(WCHAR);
    ostr = Dll_Alloc(len);
    if (ostr) {
        wmemcpy(ostr, Gui_BoxPrefixW, Gui_BoxPrefix_Len);
        wcscpy(ostr + Gui_BoxPrefix_Len, iptr);
    }

    return ostr;
}


//---------------------------------------------------------------------------
// Gui_CreateClassNameA
//---------------------------------------------------------------------------


_FX UCHAR *Gui_CreateClassNameA(const UCHAR *istr)
{
    int len;
    UCHAR *ostr;
    UCHAR clsnm[256];
    const UCHAR *iptr;

    if (((ULONG_PTR)istr & ((ULONG_PTR)LongToPtr(0xFFFF0000))) == 0) {
        ULONG_PTR atom = (ULONG_PTR)istr;
        if (atom <= 0xBFFF)
            return (UCHAR *)istr;
        if (__sys_GetClipboardFormatNameA((ATOM)atom, clsnm, 255) == 0)
            return (UCHAR *)istr;
        iptr = clsnm;
    } else
        iptr = istr;

#ifdef DEBUG_CREATECLASSNAME
    OutputDebugStringW(L"CreateClassNameA - Class Name Is\n");
    OutputDebugStringA(iptr);OutputDebugString(L"\n");
#endif

    if (strncmp(iptr, Gui_BoxPrefixA, Gui_BoxPrefix_Len) == 0)
        return (UCHAR *)istr;

    if (1) {

        NTSTATUS status;
        UNICODE_STRING uni;
        ANSI_STRING ansi;
        BOOLEAN UseOldName;

        RtlInitString(&ansi, iptr);
        status = RtlAnsiStringToUnicodeString(&uni, &ansi, TRUE);

        UseOldName = Gui_NoRenameClass(uni.Buffer);
        if (! UseOldName)
            UseOldName = Gui_IsOpenClass(uni.Buffer);

        RtlFreeUnicodeString(&uni);

        if (UseOldName)
            return (UCHAR *)istr;
    }

    len = (Gui_BoxPrefix_Len + strlen(iptr) + 1) * sizeof(UCHAR);
    ostr = Dll_Alloc(len);
    if (ostr) {
        memcpy(ostr, Gui_BoxPrefixA, Gui_BoxPrefix_Len);
        strcpy(ostr + Gui_BoxPrefix_Len, iptr);
    }

    return ostr;
}


//---------------------------------------------------------------------------
// Gui_RegisterClassExW
//---------------------------------------------------------------------------


_FX ATOM Gui_RegisterClassExW(WNDCLASSEXW *wcin)
{
    ATOM atom;
    WNDCLASSEXW wc;
    const void *saveClassName;
    WCHAR instName[128];

    memcpy(&wc, wcin, sizeof(wc));
    if (wc.hInstance) {
        memzero(instName, sizeof(instName));
        GetModuleFileName(wc.hInstance, instName, 126);
    } else
        instName[0] = L'\0';
    saveClassName = wc.lpszClassName;
    if (_wcsicmp(instName, Gui_comctl32) != 0 &&
        _wcsicmp(instName, Gui_riched20) != 0) {
        wc.lpszClassName = Gui_CreateClassNameW(wc.lpszClassName);
        if (! wc.lpszClassName)
            return 0;
    }

#ifdef DEBUG_REGISTERCLASS
    OutputDebugStringW(L"RegisterClassExW - Register Class Name Is \n");
    if ((ULONG_PTR)wc.lpszClassName >= 0x10000)
        OutputDebugStringW(wc.lpszClassName);
    OutputDebugStringW(L"\n");
#endif

    atom = __sys_RegisterClassExW(&wc);
    if (saveClassName != wc.lpszClassName)
        Gui_Free((void *)wc.lpszClassName);
    return atom;
}


//---------------------------------------------------------------------------
// Gui_RegisterClassExA
//---------------------------------------------------------------------------


_FX ATOM Gui_RegisterClassExA(WNDCLASSEXA *wcin)
{
    ATOM atom;
    WNDCLASSEXA wc;
    const void *saveClassName;
    WCHAR instName[128];

    memcpy(&wc, wcin, sizeof(wc));
    if (wc.hInstance) {
        memzero(instName, sizeof(instName));
        GetModuleFileName(wc.hInstance, instName, 126);
    } else
        instName[0] = L'\0';
    saveClassName = wc.lpszClassName;
    if (_wcsicmp(instName, Gui_comctl32) != 0 &&
        _wcsicmp(instName, Gui_riched20) != 0) {
        wc.lpszClassName = Gui_CreateClassNameA(wc.lpszClassName);
        if (! wc.lpszClassName)
            return 0;
    }

#ifdef DEBUG_REGISTERCLASS
    OutputDebugStringW(L"RegisterClassExA - Register Class Name Is \n");
    if ((ULONG_PTR)wc.lpszClassName >= 0x10000)
        OutputDebugStringA(wc.lpszClassName);
    OutputDebugStringW(L"\n");
#endif

    atom = __sys_RegisterClassExA(&wc);
    if (saveClassName != wc.lpszClassName)
        Gui_Free((void *)wc.lpszClassName);
    return atom;
}


//---------------------------------------------------------------------------
// Gui_RegisterClassW
//---------------------------------------------------------------------------


_FX ATOM Gui_RegisterClassW(WNDCLASSW *wcin)
{
    ATOM atom;
    WNDCLASSW wc;
    const void *saveClassName;
    WCHAR instName[128];

    memcpy(&wc, wcin, sizeof(wc));
    if (wc.hInstance) {
        memzero(instName, sizeof(instName));
        GetModuleFileName(wc.hInstance, instName, 126);
    } else
        instName[0] = L'\0';
    saveClassName = wc.lpszClassName;
    if (_wcsicmp(instName, Gui_comctl32) != 0 &&
        _wcsicmp(instName, Gui_riched20) != 0) {
        wc.lpszClassName = Gui_CreateClassNameW(wc.lpszClassName);
        if (! wc.lpszClassName)
            return 0;
    }

#ifdef DEBUG_REGISTERCLASS
    OutputDebugStringW(L"RegisterClassW - Register Class Name Is \n");
    if ((ULONG_PTR)wc.lpszClassName >= 0x10000)
        OutputDebugStringW(wc.lpszClassName);
    OutputDebugStringW(L"\n");
#endif

    atom = __sys_RegisterClassW(&wc);
    if (saveClassName != wc.lpszClassName)
        Gui_Free((void *)wc.lpszClassName);
    return atom;
}


//---------------------------------------------------------------------------
// Gui_RegisterClassA
//---------------------------------------------------------------------------


_FX ATOM Gui_RegisterClassA(WNDCLASSA *wcin)
{
    ATOM atom;
    WNDCLASSA wc;
    const void *saveClassName;
    WCHAR instName[128];

    memcpy(&wc, wcin, sizeof(wc));
    if (wc.hInstance) {
        memzero(instName, sizeof(instName));
        GetModuleFileName(wc.hInstance, instName, 126);
    } else
        instName[0] = L'\0';
    saveClassName = wc.lpszClassName;
    if (_wcsicmp(instName, Gui_comctl32) != 0 &&
        _wcsicmp(instName, Gui_riched20) != 0) {
        wc.lpszClassName = Gui_CreateClassNameA(wc.lpszClassName);
        if (! wc.lpszClassName)
            return 0;
    }

#ifdef DEBUG_REGISTERCLASS
    OutputDebugStringW(L"RegisterClassA - Register Class Name Is \n");
    if ((ULONG_PTR)wc.lpszClassName >= 0x10000)
        OutputDebugStringA(wc.lpszClassName);
    OutputDebugStringW(L"\n");
#endif

    atom = __sys_RegisterClassA(&wc);
    if (saveClassName != wc.lpszClassName)
        Gui_Free((void *)wc.lpszClassName);
    return atom;
}


//---------------------------------------------------------------------------
// Gui_UnregisterClassW
//---------------------------------------------------------------------------


_FX BOOL Gui_UnregisterClassW(
    const WCHAR *lpClassName, HINSTANCE hInstance)
{
    WCHAR *clsnm;
    BOOL ok;

    clsnm = Gui_CreateClassNameW(lpClassName);
    ok = __sys_UnregisterClassW(clsnm, hInstance);
    if ((! ok) && GetLastError() == ERROR_CLASS_DOES_NOT_EXIST)
        ok = __sys_UnregisterClassW(lpClassName, hInstance);
    if (clsnm != lpClassName)
        Gui_Free(clsnm);
    return ok;
}


//---------------------------------------------------------------------------
// Gui_UnregisterClassW
//---------------------------------------------------------------------------


_FX BOOL Gui_UnregisterClassA(
    const UCHAR *lpClassName, HINSTANCE hInstance)
{
    UCHAR *clsnm;
    BOOL ok;

    clsnm = Gui_CreateClassNameA(lpClassName);
    ok = __sys_UnregisterClassA(clsnm, hInstance);
    if ((! ok) && GetLastError() == ERROR_CLASS_DOES_NOT_EXIST)
        ok = __sys_UnregisterClassA(lpClassName, hInstance);
    if (clsnm != lpClassName)
        Gui_Free(clsnm);
    return ok;
}


//---------------------------------------------------------------------------
// Gui_GetClassInfoExW
//---------------------------------------------------------------------------


_FX ATOM Gui_GetClassInfoExW(
    HINSTANCE hInstance, const WCHAR *lpClassName, WNDCLASSEXW *wcout)
{
    WCHAR *clsnm;
    ATOM atom;

    clsnm = Gui_CreateClassNameW(lpClassName);
    atom = (ATOM)__sys_GetClassInfoExW(hInstance, clsnm, wcout);
    if (clsnm != lpClassName)
        Gui_Free(clsnm);
    if (atom)
        wcout->lpszClassName = lpClassName;
    else if (GetLastError() == ERROR_CLASS_DOES_NOT_EXIST)
        atom = (ATOM)__sys_GetClassInfoExW(hInstance, lpClassName, wcout);
    return atom;
}


//---------------------------------------------------------------------------
// Gui_GetClassInfoExA
//---------------------------------------------------------------------------


_FX ATOM Gui_GetClassInfoExA(
    HINSTANCE hInstance, const UCHAR *lpClassName, WNDCLASSEXA *wcout)
{
    UCHAR *clsnm;
    ATOM atom;

    clsnm = Gui_CreateClassNameA(lpClassName);
    atom = (ATOM)__sys_GetClassInfoExA(hInstance, clsnm, wcout);
    if (clsnm != lpClassName)
        Gui_Free(clsnm);
    if (atom)
        wcout->lpszClassName = lpClassName;
    else if (GetLastError() == ERROR_CLASS_DOES_NOT_EXIST)
        atom = (ATOM)__sys_GetClassInfoExA(hInstance, lpClassName, wcout);
    return atom;
}


//---------------------------------------------------------------------------
// Gui_GetClassInfoW
//---------------------------------------------------------------------------


_FX ATOM Gui_GetClassInfoW(
    HINSTANCE hInstance, const WCHAR *lpClassName, WNDCLASSW *wcout)
{
    WCHAR *clsnm;
    ATOM atom;

    clsnm = Gui_CreateClassNameW(lpClassName);
    atom = (ATOM)__sys_GetClassInfoW(hInstance, clsnm, wcout);
    if (clsnm != lpClassName)
        Gui_Free(clsnm);
    if (atom)
        wcout->lpszClassName = lpClassName;
    else if (GetLastError() == ERROR_CLASS_DOES_NOT_EXIST)
        atom = (ATOM)__sys_GetClassInfoW(hInstance, lpClassName, wcout);
    return atom;
}


//---------------------------------------------------------------------------
// Gui_GetClassInfoA
//---------------------------------------------------------------------------


_FX ATOM Gui_GetClassInfoA(
    HINSTANCE hInstance, const UCHAR *lpClassName, WNDCLASSA *wcout)
{
    UCHAR *clsnm;
    ATOM atom;

    clsnm = Gui_CreateClassNameA(lpClassName);
    atom = (ATOM)__sys_GetClassInfoA(hInstance, clsnm, wcout);
    if (clsnm != lpClassName)
        Gui_Free(clsnm);
    if (atom)
        wcout->lpszClassName = lpClassName;
    else if (GetLastError() == ERROR_CLASS_DOES_NOT_EXIST)
        atom = (ATOM)__sys_GetClassInfoA(hInstance, lpClassName, wcout);
    return atom;
}


//---------------------------------------------------------------------------
// Gui_GetClassNameW
//---------------------------------------------------------------------------


_FX int Gui_GetClassNameW(
    HWND hWnd, WCHAR *lpClassName, int nMaxCount)
{
    WCHAR *clsnm, *clsnm_ptr;
    ULONG err;
    int n, n0;

    err = GetLastError();
    clsnm = Dll_Alloc(sizeof(WCHAR) * 1024);
    n = __sys_GetClassNameW(hWnd, clsnm, 1023);
    if (Gui_UseProxyService && ! n) {
        SetLastError(err);
        n = Gui_GetClassName2(hWnd, clsnm, 1023, TRUE);
    }
    n0 = n;
    err = GetLastError();

    if (n > 8 && wcsncmp(clsnm, L"Sandbox:", 8) == 0) {
        clsnm_ptr = wcschr(clsnm + 8, L':');
        if (! clsnm_ptr)
            n = -1;
        else if (nMaxCount) {
            ++clsnm_ptr;
            n -= (ULONG)(clsnm_ptr - clsnm);
            if (n > nMaxCount - 1)
                n = nMaxCount - 1;
            if (lpClassName) {
                wmemcpy(lpClassName, clsnm_ptr, n);
                lpClassName[n] = L'\0';
            }
        } else
            n = 0;
    } else
        n = -1;

    if (n == -1) {
        if (n0 > nMaxCount - 1)
            n = nMaxCount - 1;
        else
            n = n0;
        if (lpClassName) {
            wmemcpy(lpClassName, clsnm, n);
            lpClassName[n] = L'\0';
        }
    }

    Dll_Free(clsnm);
    SetLastError(err);

    return n;
}


//---------------------------------------------------------------------------
// Gui_GetClassNameA
//---------------------------------------------------------------------------


_FX int Gui_GetClassNameA(
    HWND hWnd, UCHAR *lpClassName, int nMaxCount)
{
    UCHAR *clsnm, *clsnm_ptr;
    ULONG err;
    int n, n0;

    err = GetLastError();
    clsnm = Dll_Alloc(sizeof(UCHAR) * 1024);
    n = __sys_GetClassNameA(hWnd, clsnm, 1023);
    if (Gui_UseProxyService && ! n) {
        SetLastError(err);
        n = Gui_GetClassName2(hWnd, clsnm, 1023, FALSE);
    }
    n0 = n;
    err = GetLastError();

    if (n > 8 && strncmp(clsnm, "Sandbox:", 8) == 0) {
        clsnm_ptr = strchr(clsnm + 8, ':');
        if (! clsnm_ptr)
            n = -1;
        else if (nMaxCount) {
            ++clsnm_ptr;
            n -= (ULONG)(clsnm_ptr - clsnm);
            if (n > nMaxCount - 1)
                n = nMaxCount - 1;
            if (lpClassName) {
                memcpy(lpClassName, clsnm_ptr, n);
                lpClassName[n] = '\0';
            }
        } else
            n = 0;
    } else
        n = -1;

    if (n == -1) {
        if (n0 > nMaxCount - 1)
            n = nMaxCount - 1;
        else
            n = n0;
        if (lpClassName) {
            memcpy(lpClassName, clsnm, n);
            lpClassName[n] = L'\0';
        }
    }

    Dll_Free(clsnm);
    SetLastError(err);

    return n;
}


//---------------------------------------------------------------------------
// Gui_GetClassName2
//---------------------------------------------------------------------------


_FX ULONG Gui_GetClassName2(
    HWND hWnd, void *clsnm, ULONG maxlen, BOOLEAN unicode)
{
    GUI_GET_CLASS_NAME_REQ req;
    GUI_GET_CLASS_NAME_RPL *rpl;
    ULONG result;
    ULONG error;

    req.msgid = GUI_GET_CLASS_NAME;
    req.error = GetLastError();
    req.maxlen = maxlen;
    req.unicode = unicode;
    req.hwnd = (ULONG)(ULONG_PTR)hWnd;

    rpl = Gui_CallProxy(&req, sizeof(GUI_GET_CLASS_NAME_REQ),
                        sizeof(GUI_GET_CLASS_NAME_RPL));
    if (! rpl)
        result = 0;
    else {

        result = rpl->result;
        error = rpl->error;

        if (result) {
            ULONG copy_len = result + 1;
            if (unicode)
                copy_len *= sizeof(WCHAR);
            memcpy(clsnm, rpl->name, copy_len);
        }

        Dll_Free(rpl);
    }

    SetLastError(error);
    return result;
}


//---------------------------------------------------------------------------
// Gui_IsWellKnownClass
//---------------------------------------------------------------------------


_FX BOOLEAN Gui_IsWellKnownClass(const WCHAR *iptr)
{
    static const WCHAR *NamesC[] = {
        L"CHECKLIST_ACLUI",         L"CLIPBRDWNDCLASS",
        L"CicLoaderWndClass",       L"CicMarshalWndClass",
        L"CiceroUIWndFrame",        L"ComboLBox",
        L"ComboBox",                L"ComboBoxEx32",
        //L"com.adobe.ape.stage",     // FIXME
        NULL
    };

    static const WCHAR *NamesD[] = {
        L"DDEMLMom",                L"DDEMLEvent",
        L"DDEMLAnsiClient",         L"DDEMLAnsiServer",
        L"DDEMLUnicodeClient",      L"DDEMLUnicodeServer",
        NULL
    };

    static const WCHAR *NamesM[] = {
        L"mdiclient",
        L"msctls_hotkey",           L"msctls_hotkey32",
        L"msctls_progress",         L"msctls_progress32",
        L"msctls_statusbar",        L"msctls_statusbar32",
        L"msctls_trackbar",         L"msctls_trackbar32",
        L"msctls_updown",           L"msctls_updown32",
        L"MSCTFIME Composition",    L"MSCTFIME UI",
        NULL
    };

    static const WCHAR *NamesO[] = {
        L"OleDdeWndClass",          L"OleMainThreadWndClass",
        L"OLE2UIresimage",          L"OLE2UIiconbox",
        NULL
    };

    static const WCHAR *NamesR[] = {
        L"ReBarWindow32",           L"REComboBox20W",
        L"REListBox20W",            L"RICHEDIT"
        L"RichEdit20A",             L"RichEdit20W",
        L"RichEdit20WPT",
        NULL
    };

    static const WCHAR *NamesS[] = {
        L"Static",                  L"ScrollBar",
        L"SysAnimate32",            L"SysCredential",
        L"SysDateTimePick32",       L"SysHeader32",
        L"SysIPAddress32",          L"SysLink",
        L"SysListView32",           L"SysMonthCal32",
        L"SysPager",                L"SysTabControl32",
        L"SysTreeView32",
        NULL
    };

    static const WCHAR *NamesT[] = {
        L"ToolbarWindow32",
        L"tooltips_class",          L"tooltips_class32",
        NULL
    };

    static const WCHAR *NamesW[] = {
        L"WorkerA",                 L"WorkerW",
        NULL
    };

    static const WCHAR *Names_[] = {
        L"#32768",                  L"#32770",
        L"AtlAxWin",                L"Button",
        L"Edit",                    L"ListBox",
        L"IME",                     L"NativeFontCtl",
        L"PrintTray_Notify_WndClass",
        NULL
    };

    const WCHAR **NamesPtr;

    UCHAR ch = (UCHAR)towlower(*iptr);
    if (ch == 'c')
        NamesPtr = NamesC;
    else if (ch == 'd')
        NamesPtr = NamesD;
    else if (ch == 'm')
        NamesPtr = NamesM;
    else if (ch == 'o')
        NamesPtr = NamesO;
    else if (ch == 'r')
        NamesPtr = NamesR;
    else if (ch == 's')
        NamesPtr = NamesS;
    else if (ch == 't')
        NamesPtr = NamesT;
    else if (ch == 'w')
        NamesPtr = NamesW;
    else
        NamesPtr = Names_;

    while (*NamesPtr) {
        if (_wcsicmp(*NamesPtr, iptr) == 0)
            return TRUE;
        ++NamesPtr;
    }

    //
    // FIXME Adobe window classes having to do with the WM_CREATE problem
    //

    //if (ch == 'o' && _wcsnicmp(iptr, L"OWL.", 4) == 0)
    //    return TRUE;

    //
    // Windows 8 immersive popup window
    //

    if (Dll_OsBuild >= 8400 && _wcsicmp(iptr, L"Shell_Flyout") == 0)
        return TRUE;

    //
    // all Windows.UI.*
    // 

    if (_wcsnicmp(iptr, L"Windows.UI.", 11) == 0)
        return TRUE;

    //
    // finish
    //

    return FALSE;
}


//---------------------------------------------------------------------------
// Gui_NoRenameClass
//---------------------------------------------------------------------------


_FX BOOLEAN Gui_NoRenameClass(const WCHAR* iptr)
{
    //
    // treat all well known classes as NoRenameWinClass
    //

    if (Gui_IsWellKnownClass(iptr))
        return TRUE;

    //
    // Check NoRenameWinClass
    //

    BOOL bNoRename = FALSE;

    ULONG path_len = (wcslen(iptr) + 1) * sizeof(WCHAR);
    WCHAR* path_lwr = Dll_AllocTemp(path_len);
    if (!path_lwr) {
        SbieApi_Log(2305, NULL);
        return FALSE;
    }
    memcpy(path_lwr, iptr, path_len);
    _wcslwr(path_lwr);
    path_len = wcslen(path_lwr);

    PATTERN* pat = List_Head(&Gui_NoRenameWinClasses);
    while (pat)
    {
        if (Pattern_Match(pat, path_lwr, path_len))
        {
            bNoRename = TRUE;
            break;
        }
        pat = List_Next(pat);
    }

    Dll_Free(path_lwr);

    return bNoRename;
}


//---------------------------------------------------------------------------
// Gui_IsOpenClass
//---------------------------------------------------------------------------


_FX BOOLEAN Gui_IsOpenClass(const WCHAR *ClassName)
{
    static BOOLEAN Gui_MatchPath_Initialized = FALSE;
    ULONG mp_flags;

    if (! Gui_MatchPath_Initialized) {

        EnterCriticalSection(&Gui_IsOpenClass_CritSec);

        if (! Gui_MatchPath_Initialized) {

            SbieDll_MatchPath(L'w', (const WCHAR *)-1);

            Gui_MatchPath_Initialized = TRUE;
        }

        LeaveCriticalSection(&Gui_IsOpenClass_CritSec);
    }

    //
    // check the openness for the specified class name
    //

    mp_flags = SbieDll_MatchPath(L'w', ClassName);
    if (PATH_IS_OPEN(mp_flags))
        return TRUE;

    return FALSE;
}


//---------------------------------------------------------------------------
// Gui_UnCreateClassName
//---------------------------------------------------------------------------


const WCHAR *Gui_UnCreateClassName(const WCHAR *clsnm)
{
    if (wcsncmp(clsnm, Gui_BoxPrefixW, Gui_BoxPrefix_Len) == 0)
        clsnm += Gui_BoxPrefix_Len;
    return clsnm;
}


//---------------------------------------------------------------------------
// Gui_IsWindowAccessible
//---------------------------------------------------------------------------


_FX BOOLEAN Gui_IsWindowAccessible(HWND hWnd)
{
    BOOLEAN allow = FALSE;
    ULONG_PTR idProcess;

    //
    // allow if target special pseudo handle
    //

    if (hWnd == HWND_MESSAGE || hWnd == HWND_DESKTOP 
     || hWnd == HWND_BOTTOM || hWnd == HWND_NOTOPMOST || hWnd == HWND_TOPMOST) {

        return TRUE;
    }

    //
    // allow if target window is part of a process in the same sandbox
    //

    if (Gui_IsSameBox(hWnd, &idProcess, NULL)) {

        allow = TRUE;

    } else if (hWnd) {

        //
        // allow if class name of target window matches OpenWinClass
        //

        WCHAR clsnm[256];
        ULONG have_clsnm = __sys_GetClassNameW(hWnd, clsnm, 255);
        if (! have_clsnm && Gui_UseProxyService)
            have_clsnm = Gui_GetClassName2(hWnd, clsnm, 255, TRUE);
        if (have_clsnm) {

            clsnm[255] = L'\0';

            if (Gui_IsOpenClass(clsnm)) {

                allow = TRUE;
            }

#ifndef _WIN64

            else if (((ULONG_PTR)hWnd & 0xFFFF0000) == 0) {

                //
                // hWnd could be an index into some internal table
                // (possibly only under WOW64), in which case the
                // GetWindowThreadProcessId call in Gui_IsSameBox
                // fails, even though GetClassNameW here does work.
                // work around this by checking the class name prefix
                //

                const WCHAR *clsnm2 = Gui_UnCreateClassName(clsnm);
                if (clsnm2 != clsnm) {

                    allow = TRUE;
                }
            }

#endif ! _WIN64

        }

        //
        // allow if process name of target window matches $:OpenWinClass
        //

        if ((! allow) && idProcess) {

            NTSTATUS status;
            HANDLE hProcess = SbieDll_OpenProcess(PROCESS_QUERY_INFORMATION, (HANDLE)idProcess);
            if (hProcess) {

                ULONG len;
                status = NtQueryInformationProcess(
                    hProcess, ProcessImageFileName,
                    clsnm, sizeof(clsnm) - 8, &len);

                if (NT_SUCCESS(status)) {

                    WCHAR *name;
                    UNICODE_STRING *uni = (UNICODE_STRING *)clsnm;

                    if (uni->Buffer) {      // Curt 26-Aug-2013

                        uni->Buffer[uni->Length / sizeof(WCHAR)] = L'\0';
                        name = wcsrchr(uni->Buffer, L'\\');
                        if (name)
                            ++name;
                        else
                            name = uni->Buffer;

                        --name;
                        *name = L':';
                        --name;
                        *name = L'$';

                        if (Gui_IsOpenClass(name)) {

                            allow = TRUE;
                        }
                    }
                }

                CloseHandle(hProcess);
            }
        }
    }

    return allow;
}


//---------------------------------------------------------------------------
// Gui_CREATESTRUCT_Handler
//---------------------------------------------------------------------------


_FX ULONG_PTR Gui_CREATESTRUCT_Handler(ULONG_PTR *args)
{
    ULONG LastError;
    THREAD_DATA *TlsData = Dll_GetTlsData(&LastError);

    ULONG_PTR *pclsnm;
    ULONG_PTR clsnm;

    //
    // adjust offsets into pointers
    //

    extern void Gui_FixupCallbackPointers(ULONG_PTR *args);
    Gui_FixupCallbackPointers(args);

    //
    // get a pointer to the window class name in the CREATESTRUCT
    //

#ifdef _WIN64

    pclsnm = &args[0x88 / sizeof(ULONG_PTR)];
    clsnm = *pclsnm;

#else ! _WIN64

    if (Dll_IsWow64)
        pclsnm = &args[0x54 / sizeof(ULONG_PTR)];
    else
        pclsnm = &args[0x50 / sizeof(ULONG_PTR)];
    clsnm = *pclsnm;

#endif _WIN64

    if (clsnm > Gui_HighestAddress)
        clsnm = 0;
    else if ((clsnm & ((ULONG_PTR)LongToPtr(0xFFFF0000))) == 0)
        clsnm = 0;

    //
    // if the class name is sandboxed, advance the pointer
    //

    if (clsnm) {

        if (wmemcmp((WCHAR *)clsnm, Gui_BoxPrefixW,
                                    Gui_BoxPrefix_Len) == 0) {

            clsnm += Gui_BoxPrefix_Len * sizeof(WCHAR);
            *pclsnm = clsnm;

        } else if (memcmp((UCHAR *)clsnm, Gui_BoxPrefixA,
                                          Gui_BoxPrefix_Len) == 0) {

            clsnm += Gui_BoxPrefix_Len * sizeof(UCHAR);
            *pclsnm = clsnm;
        }
    }

    //
    // call the original CREATESTRUCT handler
    //

    return __sys_CREATESTRUCT_Handler(args);
}


//---------------------------------------------------------------------------
// Gui_CREATESTRUCT_Restore
//---------------------------------------------------------------------------


_FX void Gui_CREATESTRUCT_Restore(LPARAM lParam)
{
    //
    // if the application calls DefWindowProc while processing WM_CREATE
    // or WM_NCCREATE then we want to restore the real window class name
    // that was adjusted by Gui_CREATESTRUCT_Handler
    //

    if (! Gui_RenameClasses)
        return;

    __try {

        LPCREATESTRUCT lpcs = (LPCREATESTRUCT)lParam;
        ULONG_PTR *pclsnm = (ULONG_PTR *)&lpcs->lpszClass;
        ULONG_PTR clsnm = *pclsnm;

        if (clsnm > Gui_HighestAddress)
            clsnm = 0;
        else if ((clsnm & ((ULONG_PTR)LongToPtr(0xFFFF0000))) == 0)
            clsnm = 0;

        if (clsnm) {

            ULONG_PTR clsnmW = clsnm - Gui_BoxPrefix_Len * sizeof(WCHAR);
            ULONG_PTR clsnmA = clsnm - Gui_BoxPrefix_Len * sizeof(UCHAR);

            if (wmemcmp((WCHAR *)clsnmW, Gui_BoxPrefixW,
                                         Gui_BoxPrefix_Len) == 0) {

                *pclsnm = clsnmW;

            } else if (memcmp((UCHAR *)clsnmA, Gui_BoxPrefixA,
                                               Gui_BoxPrefix_Len) == 0) {

                *pclsnm = clsnmA;
            }
        }

    } __except (EXCEPTION_EXECUTE_HANDLER) {
    }
}
