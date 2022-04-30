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
// GDI Services
//---------------------------------------------------------------------------


#include "dll.h"
#include "gui_p.h"
#include "gdi.h"
#include "core/svc/GuiWire.h"
#include "common/my_version.h"
#include <stdio.h>


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------


static ULONG_PTR Gdi_GdiDllInitialize_XP(
    HMODULE User32, BOOLEAN Init, ULONG_PTR Unknown3);

static ULONG_PTR Gdi_GdiDllInitialize_Vista(HMODULE User32, BOOLEAN Init);

static ULONG_PTR Gdi_GdiDllInitialize_Common(
    ULONG NumArgs, HMODULE User32, BOOLEAN Init, ULONG_PTR Unknown3);

#ifndef _WIN64

static HDC Gdi_CreateDCW(
    void *lpszDriver, void *lpszDevice, void *lpszOutput, void *lpInitData);

#endif ! _WIN64

static WCHAR *Gdi_GetFontPath(const WCHAR *path);

static ULONG Gdi_GdiAddFontResourceW(
    const WCHAR *path, ULONG flags, void *reserved);

static ULONG Gdi_RemoveFontResourceExW(
    const WCHAR *path, ULONG flags, void *reserved);

static ULONG Gdi_GetFontResourceInfoW(
    const WCHAR *path, ULONG *buflen, void *bufptr, ULONG querytype);

static ULONG Gdi_CreateScalableFontResourceW(
    ULONG hidden, const WCHAR *res, const WCHAR *file, const WCHAR *path);

static void Gdi_AddFontsInBox(void);

static void Gdi_AddFontsInBox_2(
    HANDLE hFontsDir, WCHAR *WinFonts);

static int Gdi_EnumFontFamiliesExA(
    HDC hdc, void *lpLogfont, void *lpEnumFontFamExProc,
    LPARAM lParam, DWORD dwFlags);

static int Gdi_EnumFontFamiliesExW(
    HDC hdc, void *lpLogfont, void *lpEnumFontFamExProc,
    LPARAM lParam, DWORD dwFlags);

static HGDIOBJ Gdi_GetStockObject(int fnObject);

//---------------------------------------------------------------------------

#ifndef _WIN64
static BOOL Gdi_OpenPrinter2W(
    void *pPrinterName, HANDLE *phPrinter, void *pDefault, void *pOptions);

static BOOL Gdi_ClosePrinter(HANDLE hPrinter);

#endif _WIN64


//---------------------------------------------------------------------------


typedef HDC (*P_CreateDCW)(
    void *lpszDriver, void *lpszDevice, void *lpszOutput, void *lpInitData);

typedef ULONG (*P_GdiAddFontResourceW)(
    const WCHAR *path, ULONG flags, void *reserved);

typedef ULONG (*P_RemoveFontResourceExW)(
    const WCHAR *path, ULONG flags, void *reserved);

typedef ULONG (*P_GetFontResourceInfoW)(
    const WCHAR *path, ULONG *buflen, void *bufptr, ULONG querytype);

typedef ULONG (*P_CreateScalableFontResourceW)(
    ULONG hidden, const WCHAR *res, const WCHAR *file, const WCHAR *path);

typedef int (*P_EnumFontFamiliesEx)(
    HDC hdc, void *lpLogfont, void *lpEnumFontFamExProc,
    LPARAM lParam, DWORD dwFlags);

typedef HGDIOBJ (*P_GetStockObject)(int fnObject);

//---------------------------------------------------------------------------


typedef BOOL (*P_OpenPrinter2W)(
    void *pPrinterName, HANDLE *phPrinter, void *pDefault, void *pOptions);

typedef BOOL (*P_ClosePrinter)(HANDLE hPrinter);

typedef LONG (*P_DocumentProperties)(
    HWND hWnd, HANDLE hPrinter, void *pDeviceName,
    void *pDevModeOutput, void *pDevModeInput, DWORD fMode);


//---------------------------------------------------------------------------


void                           *__sys_GdiDllInitialize              = NULL;
P_CreateDCW                     __sys_CreateDCW                     = NULL;
P_GdiAddFontResourceW           __sys_GdiAddFontResourceW           = NULL;
P_RemoveFontResourceExW         __sys_RemoveFontResourceExW         = NULL;
P_GetFontResourceInfoW          __sys_GetFontResourceInfoW          = NULL;
P_CreateScalableFontResourceW   __sys_CreateScalableFontResourceW   = NULL;
P_EnumFontFamiliesEx            __sys_EnumFontFamiliesExA           = NULL;
P_EnumFontFamiliesEx            __sys_EnumFontFamiliesExW           = NULL;
P_OpenPrinter2W                 __sys_OpenPrinter2W                 = NULL;
P_ClosePrinter                  __sys_ClosePrinter                  = NULL;
P_GetEnhMetaFileBits            __sys_GetEnhMetaFileBits            = NULL;
P_GetBitmapBits                 __sys_GetBitmapBits                 = NULL;
P_DeleteObject                  __sys_DeleteObject                  = NULL;
P_DeleteEnhMetaFile             __sys_DeleteEnhMetaFile             = NULL;
P_GetStockObject                __sys_GetStockObject                = NULL;

//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
// Variables
//---------------------------------------------------------------------------


static CRITICAL_SECTION Gdi_CritSec;

static WCHAR *Gdi_LastFontPath = NULL;

static HDESK Gdi_ProcessDesktop = NULL;

static const WCHAR *Gdi_SplWow64InUseEventName =
    SBIE_BOXED_ L"SplWow64InUse";

static volatile HANDLE Gdi_SplWow64InUseEvent = NULL;

static volatile ULONG Gdi_SplWow64InUseCount = 0;


//---------------------------------------------------------------------------
// Gdi_GdiDllInitialize_XP
//---------------------------------------------------------------------------


_FX ULONG_PTR Gdi_GdiDllInitialize_XP(
    HMODULE User32, BOOLEAN Init, ULONG_PTR Unknown3)
{
    return Gdi_GdiDllInitialize_Common(3, User32, Init, Unknown3);
}


//---------------------------------------------------------------------------
// Gdi_GdiDllInitialize_Vista
//---------------------------------------------------------------------------


_FX ULONG_PTR Gdi_GdiDllInitialize_Vista(HMODULE User32, BOOLEAN Init)
{
    return Gdi_GdiDllInitialize_Common(2, User32, Init, 0);
}


//---------------------------------------------------------------------------
// Gdi_GdiDllInitialize_Common
//---------------------------------------------------------------------------


_FX ULONG_PTR Gdi_GdiDllInitialize_Common(
    ULONG NumArgs, HMODULE User32, BOOLEAN Init, ULONG_PTR Unknown3)
{
    static BOOLEAN _Initialized = FALSE;

    ULONG_PTR rc;

    //
    // GdiDllInitialize is called from User32 dll initialization, and it
    // calls gdi32!NtGdiInit to make the first win32k in the process.
    // we have to explicitly connect to a window station before the
    // process can make any win32k calls, see also:
    // Gui_ConnectToWindowStationAndDesktop
    //

	// NoSbieDesk BEGIN
	if(!Dll_CompartmentMode && !SbieApi_QueryConfBool(NULL, L"NoSandboxieDesktop", FALSE))
	// NoSbieDesk END
    if (! _Initialized) {

        if (! Gui_ConnectToWindowStationAndDesktop(User32))
            return 1;

        _Initialized = TRUE;
    }

    if (NumArgs == 3) {

        typedef ULONG_PTR (*P_GdiDllInitialize_XP)(
            HMODULE User32, BOOLEAN Init, ULONG_PTR Unknown3);

        rc = ((P_GdiDllInitialize_XP)__sys_GdiDllInitialize)(
                                            User32, Init, Unknown3);

    } else if (NumArgs == 2) {

        typedef ULONG_PTR (*P_GdiDllInitialize_Vista)(
            HMODULE User32, BOOLEAN Init);

        rc = ((P_GdiDllInitialize_Vista)__sys_GdiDllInitialize)(
                                            User32, Init);

    } else
        rc = 99;

    if (rc == 0) {

        //
        // GdiDllInitialize is called shortly after USER32 updates the
        // KernelCallbackTable in the PEB to specify its table of callback
        // functions that are called by win32k.  so take this opportunity
        // to hook the CREATESTRUCT callback for WM_CREATE messages
        //

        Gui_Hook_CREATESTRUCT_Handler();
    }

    if (rc != 0) {

        WCHAR errtxt[48];
        Sbie_snwprintf(errtxt, 48, L"GdiInit.%08p", (void*)rc);
        SbieApi_Log(2205, errtxt);
    }

    return rc;
}


//---------------------------------------------------------------------------
// Gdi_SplWow64
//---------------------------------------------------------------------------


_FX void Gdi_SplWow64(BOOLEAN Register)
{
    //
    // see GuiServer::SplWow64Slave
    //

    // NoSbieDesk BEGIN
    if (Dll_CompartmentMode || SbieApi_QueryConfBool(NULL, L"NoSandboxieDesktop", FALSE))
        return;
	// NoSbieDesk END

    GUI_SPLWOW64_REQ req;
    void *rpl;

    if (Register) {

        if (Ldr_BoxedImage || _wcsicmp(Dll_ImageName, L"splwow64.exe") != 0)
            return;
    }

    req.msgid = GUI_SPLWOW64;
    req.set = Register;
    req.win8 = (Dll_OsBuild >= 8400) ? TRUE : FALSE;
    rpl = Gui_CallProxy(&req, sizeof(req), sizeof(ULONG));
    if (rpl)
        Dll_Free(rpl);

}


//---------------------------------------------------------------------------
// Gdi_CreateDCW
//---------------------------------------------------------------------------


#ifndef _WIN64

_FX HDC Gdi_CreateDCW(
    void *lpszDriver, void *lpszDevice, void *lpszOutput, void *lpInitData)
{
    //
    // on 64-bit Windows 8, some 32-bit programs (Notepad, Chrome) cannot
    // create a printer DC (via WINSPOOL) if an instance of SplWow64.exe
    // has been terminated, since the last time that 32-bit process has
    // connected to SplWow64.exe.  the reason for this is not clear, but
    // it seems a possible workaround is to try recreating the DC several
    // times, until the CreateDC call finally works.
    //

    HDC hdc = __sys_CreateDCW(
                        lpszDriver, lpszDevice, lpszOutput, lpInitData);

    if ((! hdc) && lpszDriver && _wcsicmp(lpszDriver, L"WINSPOOL") == 0) {

        P_DocumentProperties __sys_DocumentProperties =
            Ldr_GetProcAddrNew(L"winspool.drv", L"DocumentPropertiesW","DocumentPropertiesW");

        ULONG retry = 0;

        while (__sys_DocumentProperties && (! hdc) && (retry < 20)) {

            HANDLE hPrinter;

            Sleep(retry * 25);

            if (! __sys_OpenPrinter2W(lpInitData, &hPrinter, NULL, NULL))
                break;

            __sys_DocumentProperties(
                NULL, hPrinter, lpInitData, NULL, NULL, 0);

            hdc = __sys_CreateDCW(
                        lpszDriver, lpszDevice, lpszOutput, lpInitData);

            __sys_ClosePrinter(hPrinter);

            retry++;
        }
    }

    return hdc;
}

#endif ! _WIN64


//---------------------------------------------------------------------------
// Gdi_GetFontPath
//---------------------------------------------------------------------------


_FX WCHAR *Gdi_GetFontPath(const WCHAR *path)
{
    WCHAR *path1;
    WCHAR *path2;
    ULONG path_len;
    HANDLE hFile;

    path1 = Dll_AllocTemp(8192);
    path_len = wcslen(path);
    if (path_len > 4095)
        path_len = 4095;
    wmemcpy(path1, path, path_len);
    path1[path_len] = L'\0';

    hFile = CreateFile(
        path1, GENERIC_READ, FILE_SHARE_VALID_FLAGS, NULL,
        OPEN_EXISTING, 0, NULL);

    if (hFile == INVALID_HANDLE_VALUE) {

        WCHAR *backslash = wcschr(path, L'\\');
        if (! backslash) {
            wmemzero(path1, MAX_PATH + 1);
            GetSystemWindowsDirectory(path1, MAX_PATH);
            if (! path1[0])
                wcscpy(path1, L"C:\\WINDOWS");
            else if (path1[wcslen(path1) - 1] == L'\\')
                path1[wcslen(path1) - 1] = L'\0';
            wcscat(path1, L"\\Fonts\\");
            wcscat(path1, path);

            hFile = CreateFile(
                path1, GENERIC_READ, FILE_SHARE_VALID_FLAGS, NULL,
                OPEN_EXISTING, 0, NULL);
        }
    }

    if (hFile != INVALID_HANDLE_VALUE) {

        BOOLEAN is_copy;
        NTSTATUS status = SbieDll_GetHandlePath(hFile, path1, &is_copy);
        if ((! NT_SUCCESS(status)) ||
            (! SbieDll_TranslateNtToDosPath(path1))) {

            wmemcpy(path1, path, path_len);
            path1[path_len] = L'\0';
        }

        CloseHandle(hFile);
    }

    //
    // if the requested path is not inside the sandbox (i.e. an open path)
    // then File_ReplaceTokenIfFontRequest in core/drv/file.c will not
    // elevate the thread token, so we issue a warning about that
    //

    path2 = File_TranslateDosToNtPath(path1);
    if (path2) {
        ULONG mp_flags = SbieDll_MatchPath(L'f', path2);
        if (PATH_IS_OPEN(mp_flags))
            SbieApi_Log(2205, L"Font Path %S", path1);
        Dll_Free(path2);
    }

    return path1;
}


//---------------------------------------------------------------------------
// Gdi_GdiAddFontResourceW
//---------------------------------------------------------------------------


_FX ULONG Gdi_GdiAddFontResourceW(
    const WCHAR *path, ULONG flags, void *reserved)
{
    ULONG rv, err;

    WCHAR *path1 = Gdi_GetFontPath(path);
    flags |= FR_PRIVATE;

    rv = __sys_GdiAddFontResourceW(path1, flags, reserved);
    err = GetLastError();

    if (rv) {

        EnterCriticalSection(&Gdi_CritSec);

        if (Gdi_LastFontPath)
            Dll_Free(Gdi_LastFontPath);

        Gdi_LastFontPath = path1;

        LeaveCriticalSection(&Gdi_CritSec);

    } else {

        Dll_Free(path1);
    }

    SetLastError(err);
    return rv;
}


//---------------------------------------------------------------------------
// Gdi_RemoveFontResourceExW
//---------------------------------------------------------------------------


_FX ULONG Gdi_RemoveFontResourceExW(
    const WCHAR *path, ULONG flags, void *reserved)
{
    ULONG rv, err;

    WCHAR *path1 = Gdi_GetFontPath(path);
    flags |= FR_PRIVATE;

    rv = __sys_RemoveFontResourceExW(path1, flags, reserved);
    err = GetLastError();

    EnterCriticalSection(&Gdi_CritSec);

    if (Gdi_LastFontPath) {
        Dll_Free(Gdi_LastFontPath);
        Gdi_LastFontPath = NULL;
    }

    LeaveCriticalSection(&Gdi_CritSec);

    Dll_Free(path1);

    SetLastError(err);
    return rv;
}


//---------------------------------------------------------------------------
// Gdi_GetFontResourceInfoW
//---------------------------------------------------------------------------


_FX ULONG Gdi_GetFontResourceInfoW(
    const WCHAR *path, ULONG *buflen, void *bufptr, ULONG querytype)
{
    ULONG rv, err;

    WCHAR *path1 = Gdi_GetFontPath(path);

    rv = __sys_GetFontResourceInfoW(path1, buflen, bufptr, querytype);
    err = GetLastError();

    if (! rv) {

        EnterCriticalSection(&Gdi_CritSec);

        if (Gdi_LastFontPath && _wcsicmp(path1, Gdi_LastFontPath) == 0) {

            //
            // __sys_GetFontResourceInfoW will fail for a font added
            // using FR_PRIVATE, so temporarily add the font normally
            //

            rv = __sys_RemoveFontResourceExW(path1, FR_PRIVATE, NULL);
            if (rv) {

                rv = __sys_GdiAddFontResourceW(path1, 0, NULL);

                if (rv) {

                    rv = __sys_GetFontResourceInfoW(
                                        path1, buflen, bufptr, querytype);
                    err = GetLastError();

                    __sys_RemoveFontResourceExW(path1, 0, NULL);
                }

                __sys_GdiAddFontResourceW(path1, FR_PRIVATE, NULL);
            }
        }

        LeaveCriticalSection(&Gdi_CritSec);
    }

    Dll_Free(path1);

    SetLastError(err);
    return rv;
}


//---------------------------------------------------------------------------
// Gdi_CreateScalableFontResourceW
//---------------------------------------------------------------------------


_FX ULONG Gdi_CreateScalableFontResourceW(
    ULONG hidden, const WCHAR *res, const WCHAR *file, const WCHAR *path)
{
    const WCHAR *res1;
    const WCHAR *file1;
    ULONG rv, err;

    if (path) {
        res1 = res;
        file1 = file;
    } else {
        res1 = Gdi_GetFontPath(res);
        file1 = Gdi_GetFontPath(file);
    }

    rv = __sys_CreateScalableFontResourceW(TRUE, res1, file1, path);
    err = GetLastError();

    if (res1 != res)
        Dll_Free((void *)res1);
    if (file1 != file)
        Dll_Free((void *)file1);

    SetLastError(err);
    return rv;
}


//---------------------------------------------------------------------------
// Gdi_AddFontsInBox
//---------------------------------------------------------------------------


_FX void Gdi_AddFontsInBox(void)
{
    static volatile BOOLEAN AlreadyAdded = FALSE;

    if (AlreadyAdded)
        return;

    EnterCriticalSection(&Gdi_CritSec);

    if (! AlreadyAdded) {

        WCHAR WinFonts[MAX_PATH * 2];
        HANDLE hFile = INVALID_HANDLE_VALUE;

        GetSystemWindowsDirectory(WinFonts, MAX_PATH);
        if (WinFonts[0]) {
            if (WinFonts[wcslen(WinFonts) - 1] == L'\\')
                WinFonts[wcslen(WinFonts) - 1] = L'\0';
            wcscat(WinFonts, L"\\Fonts");

            hFile = CreateFile(WinFonts, GENERIC_READ,
                               FILE_SHARE_VALID_FLAGS, NULL,
                               OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS,
                               NULL);
        }

        if (hFile != INVALID_HANDLE_VALUE) {

            BOOLEAN is_copy = FALSE;
            NTSTATUS status = SbieDll_GetHandlePath(hFile, NULL, &is_copy);
            if (NT_SUCCESS(status) && is_copy) {

                Gdi_AddFontsInBox_2(hFile, WinFonts);
            }
            
            CloseHandle(hFile);
        }
    }

    AlreadyAdded = TRUE;

    LeaveCriticalSection(&Gdi_CritSec);
}


//---------------------------------------------------------------------------
// Gdi_AddFontsInBox_2
//---------------------------------------------------------------------------


_FX void Gdi_AddFontsInBox_2(HANDLE hFontsDir, WCHAR *WinFonts)
{
    NTSTATUS status;
    FILE_DIRECTORY_INFORMATION *info;
    UCHAR *next_entry;

    ULONG WinFonts_len = wcslen(WinFonts);
    WinFonts[WinFonts_len] = L'\\';

    WCHAR *buf8k = Dll_AllocTemp(8192);

    while (1) {

        info = (FILE_DIRECTORY_INFORMATION *)buf8k;

        status = File_MyQueryDirectoryFile(
            hFontsDir, info, 8000, FileDirectoryInformation,
            FALSE, NULL, FALSE);

        if (status != STATUS_SUCCESS && status != STATUS_BUFFER_OVERFLOW)
            break;

        while (1) {

            if ((info->FileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0) {

                memcpy(WinFonts + WinFonts_len + 1,
                       info->FileName, info->FileNameLength);
                WinFonts[WinFonts_len + 1 +
                         (info->FileNameLength / sizeof(WCHAR))] = L'\0';

                Gdi_GdiAddFontResourceW(WinFonts, 0, 0);
            }

            if (info->NextEntryOffset == 0)
                break;
            next_entry = ((UCHAR *)info) + info->NextEntryOffset;
            info = (FILE_DIRECTORY_INFORMATION *)next_entry;
        }
    }

    Dll_Free(buf8k);
}


//---------------------------------------------------------------------------
// Gdi_EnumFontFamiliesExA
//---------------------------------------------------------------------------


_FX int Gdi_EnumFontFamiliesExA(
    HDC hdc, void *lpLogfont, void *lpEnumFontFamExProc,
    LPARAM lParam, DWORD dwFlags)
{
    Gdi_AddFontsInBox();
    return __sys_EnumFontFamiliesExA(
        hdc, lpLogfont, lpEnumFontFamExProc, lParam, dwFlags);
}


//---------------------------------------------------------------------------
// Gdi_EnumFontFamiliesExW
//---------------------------------------------------------------------------


_FX int Gdi_EnumFontFamiliesExW(
    HDC hdc, void *lpLogfont, void *lpEnumFontFamExProc,
    LPARAM lParam, DWORD dwFlags)
{
    Gdi_AddFontsInBox();
    return __sys_EnumFontFamiliesExW(
        hdc, lpLogfont, lpEnumFontFamExProc, lParam, dwFlags);
}


//---------------------------------------------------------------------------
// Gdi_InitZero
//---------------------------------------------------------------------------


_FX BOOLEAN Gdi_InitZero(void)
{
    static void *Saved_GdiDllInitialize = NULL;
    void *GdiDllInitialize;
    void *Gdi_GdiDllInitialize;

    //
    // GDI/WIN32K initialize
    //
    // because there is a circular dependency between USER32 and GDI32,
    // we split the hook on GdiDllInitialize out of the main Gdi_Init
    // into this smaller init function, so it can be invoked directly
    // from Gui_Init, in case USER32 is loaded before GDI32 (and thus
    // Gui_Init is called before Gdi_Init).
    //
    // we also use Ldr_GetProcAddr rather than GetProcAddress to avoid
    // circular triggering of DLL initialization calls.
    //
    // note that in most cases GDI32 is loaded before USER32 by the
    // ntdll loader, but there are cases where this is not so.
    //

    GdiDllInitialize = Ldr_GetProcAddrNew(DllName_gdi32, L"GdiDllInitialize","GdiDllInitialize");

    if (GdiDllInitialize == Saved_GdiDllInitialize)
        return TRUE;

    Saved_GdiDllInitialize = GdiDllInitialize;

    if (Dll_OsBuild >= 6000)
        Gdi_GdiDllInitialize = Gdi_GdiDllInitialize_Vista;

    else if (Ldr_GetProcAddrNew(L"ntdll.dll", L"NtApphelpCacheControl","NtApphelpCacheControl")) {
        // Windows 2003 has Vista-style GdiDllInitialize
        Gdi_GdiDllInitialize = Gdi_GdiDllInitialize_Vista;

    } else
        Gdi_GdiDllInitialize = Gdi_GdiDllInitialize_XP;

    SBIEDLL_HOOK(Gdi_,GdiDllInitialize);

    return TRUE;
}


//---------------------------------------------------------------------------
// Gdi_Full_Init_impl
//---------------------------------------------------------------------------


_FX BOOLEAN Gdi_Full_Init_impl(HMODULE module, BOOLEAN full)
{
	P_CreateDCW CreateDCW;
	P_GdiAddFontResourceW GdiAddFontResourceW;
	P_RemoveFontResourceExW RemoveFontResourceExW;
	P_GetFontResourceInfoW GetFontResourceInfoW;
	P_CreateScalableFontResourceW CreateScalableFontResourceW;

	P_EnumFontFamiliesEx EnumFontFamiliesExA;
	P_EnumFontFamiliesEx EnumFontFamiliesExW;
	P_GetStockObject GetStockObject;

	InitializeCriticalSection(&Gdi_CritSec);

	if (!Gdi_InitZero())
		return FALSE;

	//
	// add, remove, get, create fonts
	//
	CreateDCW = (P_CreateDCW)
		GetProcAddress(module, "CreateDCW");

	GdiAddFontResourceW = (P_GdiAddFontResourceW)
		GetProcAddress(module, "GdiAddFontResourceW");

	RemoveFontResourceExW = (P_RemoveFontResourceExW)
		GetProcAddress(module, "RemoveFontResourceExW");

	GetFontResourceInfoW = (P_GetFontResourceInfoW)
		GetProcAddress(module, "GetFontResourceInfoW");

	if (full) {
		CreateScalableFontResourceW = (P_CreateScalableFontResourceW)
			GetProcAddress(module, "CreateScalableFontResourceWImpl");
	}
	else {
		CreateScalableFontResourceW = (P_CreateScalableFontResourceW)
			GetProcAddress(module, "CreateScalableFontResourceW");
	}

#ifndef _WIN64

	if (Dll_OsBuild >= 8400) {
		SBIEDLL_HOOK(Gdi_, CreateDCW);
	}

#endif ! _WIN64

	SBIEDLL_HOOK(Gdi_, GdiAddFontResourceW);

	SBIEDLL_HOOK(Gdi_, RemoveFontResourceExW);

	if (GetFontResourceInfoW) {
		SBIEDLL_HOOK(Gdi_, GetFontResourceInfoW);
	}

	SBIEDLL_HOOK(Gdi_, CreateScalableFontResourceW);

	//
	// enumerate
	//

	EnumFontFamiliesExA = (P_EnumFontFamiliesEx)
		GetProcAddress(module, "EnumFontFamiliesExA");

	EnumFontFamiliesExW = (P_EnumFontFamiliesEx)
		GetProcAddress(module, "EnumFontFamiliesExW");

	if (full) {
		GetStockObject = (P_GetStockObject)
			GetProcAddress(module, "GetStockObject");
	}

	SBIEDLL_HOOK(Gdi_, EnumFontFamiliesExA);
	SBIEDLL_HOOK(Gdi_, EnumFontFamiliesExW);
	if (full) {
		SBIEDLL_HOOK(Gdi_, GetStockObject);
	}

	__sys_GetEnhMetaFileBits = (P_GetEnhMetaFileBits)
		GetProcAddress(module, "GetEnhMetaFileBits");

	__sys_GetBitmapBits = (P_GetBitmapBits)
		GetProcAddress(module, "GetBitmapBits");

	__sys_DeleteObject = (P_DeleteObject)
		GetProcAddress(module, "DeleteObject");

	__sys_DeleteEnhMetaFile = (P_DeleteEnhMetaFile)
		GetProcAddress(module, "DeleteEnhMetaFile");

	return TRUE;
}


//---------------------------------------------------------------------------
// Gdi_Full_Init
//---------------------------------------------------------------------------

_FX BOOLEAN Gdi_Full_Init(HMODULE module) 
{
	if (Dll_OsBuild < 14291) {
		return TRUE;
	}

    return Gdi_Full_Init_impl(module, TRUE);
}


//---------------------------------------------------------------------------
// Gdi_Init_Spool
//---------------------------------------------------------------------------


_FX BOOLEAN Gdi_Init_Spool(HMODULE module)
{
#ifndef _WIN64

    if (Dll_OsBuild >= 8400) {

        __sys_OpenPrinter2W = (P_OpenPrinter2W)
            GetProcAddress(module, "OpenPrinter2W");
        __sys_ClosePrinter = (P_ClosePrinter)
            GetProcAddress(module, "ClosePrinter");
    }

#endif ! _WIN64

    return TRUE;
}


//---------------------------------------------------------------------------


_FX BOOLEAN Gdi_Init(HMODULE module)
{
	if (Dll_OsBuild >= 14291) {
		return TRUE;
	}

	return Gdi_Full_Init_impl(module, FALSE);
}

//Workaround for a rare chrome crash in a non-vm environment.  There is a chance for gdi32full!GetStockObject to cause a crash 
//(memory access violation) when accessing the SYSTEM_FONT (13) object from the chrome sandbox.  GetStockObject accesses a dll shared memory
//section that is not initialized at the time GetStockObject is called. And there is no error handling in GetStockObject 
//to check this case.  The following function handles the unhandled error and returns 0 (Object not found) to the caller.
//Chrome continues to function normally with the return of 0.  However, there is a deeper problem that is likely caused by
//high entropy ASLR in windows 10 when the GDI environment initializes during chrome_child.dll load in a sandboxie and 
//chrome sandbox environment.

_FX HGDIOBJ Gdi_GetStockObject(int fnObject) {
    HGDIOBJ rc = 0;

    __try {
        rc = __sys_GetStockObject(fnObject);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        rc = 0;
    }

    return rc;
}
