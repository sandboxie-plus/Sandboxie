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
// Shell
//---------------------------------------------------------------------------

#include "dll.h"

#include <windows.h>
#include <shellapi.h>
#include <shlobj.h>
#include <stdio.h>
#include "dll.h"
#include "taskbar.h"
#include "common/win32_ntddk.h"
#include "common/my_shlwapi.h"
#include "msgs/msgs.h"
#include "gui_p.h"

//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------


static BOOL SH_OpenFolder(const WCHAR *PathW, WCHAR verb, HANDLE *hProcess);

static WCHAR *SH32_AdjustPath(WCHAR *src, WCHAR **pArgs);

static HKEY SbieDll_AssocQueryKeyWow64(const WCHAR *subj);

static BOOL SH32_ShellExecuteExW(SHELLEXECUTEINFOW *lpExecInfo);

static BOOL SH32_Shell_NotifyIconW(
    DWORD dwMessage, PNOTIFYICONDATAW lpData);

static WCHAR *SbieDll_AssocQueryCommandInternal(
    const WCHAR *subj, const WCHAR *verb);

static ULONG SH32_SHChangeNotifyRegister(
    HWND hwnd, int fSources, LONG fEvents, UINT wMsg,
    int cEntries, SHChangeNotifyEntry *pfsne);

static ULONG SH32_SHOpenFolderAndSelectItems(
    LPCITEMIDLIST pidlFolder, UINT cidl, void *apidl, ULONG dwFlags);

static BOOLEAN SH32_SHOpenFolderAndSelectItems_2(
    LPCITEMIDLIST pidlParent, LPCITEMIDLIST pidlChild, WCHAR *path);

static ULONG SH32_GetPrivateProfileStringW(
    void *lpAppName, void *lpKeyName, void *lpDefault,
    void *lpReturnedString, ULONG nSize, void *lpFileName);

static HRESULT SH32_SHGetFolderLocation(
    HWND hwndOwner, int nFolder, HANDLE hToken,
    DWORD dwReserved, void **ppidl);

static NTSTATUS SH32_LdrGetDllHandleEx(
    ULONG_PTR One, ULONG_PTR Unknown1, ULONG_PTR Unknown2,
    UNICODE_STRING *DllName, ULONG_PTR *hModule);

static ULONG SH_WindowMonitorThread(void *lpParameter);


//---------------------------------------------------------------------------


typedef BOOL (*P_ShellExecuteEx)(
    void *lpExecInfo);

typedef BOOL (*P_Shell_NotifyIconW)(
    DWORD dwMessage, PNOTIFYICONDATAW lpData);

typedef ULONG (*P_SHChangeNotifyRegister)(
    HWND hwnd, int fSources, LONG fEvents, UINT wMsg,
    int cEntries, SHChangeNotifyEntry *pfsne);

typedef ULONG (*P_SHOpenFolderAndSelectItems)(
    LPCITEMIDLIST pidlFolder, UINT cidl, void *apidl, ULONG dwFlags);

typedef BOOL (*P_SHGetPathFromIDList)(
    LPCITEMIDLIST pidl, void *pszPath);

typedef ULONG (*P_GetPrivateProfileString)(
    void *lpAppName, void *lpKeyName, void *lpDefault,
    void *lpReturnedString, ULONG nSize, void *lpFileName);

typedef HRESULT (*P_SHGetFolderLocation)(
    HWND hwndOwner, int nFolder, HANDLE hToken,
    DWORD dwReserved, void **ppidl);


//---------------------------------------------------------------------------


static P_ShellExecuteEx         __sys_ShellExecuteExW               = NULL;

static P_Shell_NotifyIconW      __sys_Shell_NotifyIconW             = NULL;

static P_SHChangeNotifyRegister __sys_SHChangeNotifyRegister        = NULL;

static P_SHOpenFolderAndSelectItems
                                __sys_SHOpenFolderAndSelectItems    = NULL;

static P_SHGetPathFromIDList    __sys_SHGetPathFromIDListW          = NULL;

static P_GetPrivateProfileString __sys_GetPrivateProfileStringW     = NULL;

static P_SHGetFolderLocation    __sys_SHGetFolderLocation           = NULL;

extern P_LdrGetDllHandleEx      __sys_LdrGetDllHandleEx;


//---------------------------------------------------------------------------
// Variables
//---------------------------------------------------------------------------


extern const WCHAR *File_BQQB;


//---------------------------------------------------------------------------
// SbieDll_IsDirectory
//---------------------------------------------------------------------------


_FX BOOLEAN SbieDll_IsDirectory(const WCHAR *PathW)
{
    NTSTATUS status;
    WCHAR *ntpath, *dummy;
    ULONG len1, len2;
    UNICODE_STRING uni;
    OBJECT_ATTRIBUTES objattrs;
    FILE_NETWORK_OPEN_INFORMATION openinfo;
    BOOLEAN isdir;

    //
    // otherwise check if the path is a directory
    //

    len1 = 128;
    ntpath = Dll_Alloc((len1 + 16) * sizeof(WCHAR));
    len2 = GetFullPathName(PathW, len1, ntpath, &dummy);
    if (len2 >= len1) {
        Dll_Free(ntpath);
        len1 = len2;
        ntpath = Dll_Alloc((len1 + 16) * sizeof(WCHAR));
        len2 = GetFullPathName(PathW, len1, ntpath, &dummy);
    }
    if (len2 == 0)
        return FALSE;

    if (len2 == 2 || len2 == 3) {
        WCHAR ch0 = towlower(PathW[0]);
        WCHAR ch1 = PathW[1];
        WCHAR ch2 = PathW[2];
        if (ch0 >= L'a' && ch0 <= L'z' && ch1 == L':' &&
                (ch2 == L'\\' || ch2 == L'\0'))
            return TRUE;
    }

    wmemmove(ntpath + 4, ntpath, wcslen(ntpath) + 1);
    wmemcpy(ntpath, File_BQQB, 4);

    RtlInitUnicodeString(&uni, ntpath);
    InitializeObjectAttributes(
        &objattrs, &uni, OBJ_CASE_INSENSITIVE, NULL, NULL);

    status = NtQueryFullAttributesFile(&objattrs, &openinfo);

    isdir = FALSE;
    if (status == STATUS_NO_MEDIA_IN_DEVICE)
        isdir = TRUE;
    else if (status == STATUS_SUCCESS &&
                (openinfo.FileAttributes & FILE_ATTRIBUTE_DIRECTORY))
        isdir = TRUE;

    Dll_Free(ntpath);

    return isdir;
}


//---------------------------------------------------------------------------
// SH_OpenFolder
//---------------------------------------------------------------------------


_FX BOOL SH_OpenFolder(const WCHAR *PathW, WCHAR verb, HANDLE *hProcess)
{
    const WCHAR *ErrorText = NULL;
    WCHAR cmd[512];
    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    *hProcess = NULL;

    //
    // set up command line to invoke Windows Explorer
    //

    ZeroMemory(&si, sizeof(STARTUPINFO));
    si.cb = sizeof(STARTUPINFO);
    si.dwFlags = STARTF_FORCEOFFFEEDBACK;

    cmd[0] = L'\"';
    GetSystemWindowsDirectory(&cmd[1], MAX_PATH);
    wcscat(cmd, L"\\explorer.exe\" ");
    if (verb == L'e')
        wcscat(cmd, L"/e,");
    else if (verb == L's')
        wcscat(cmd, L"/n,/select,");
    wcscat(cmd, L"\"");
    wcscat(cmd, PathW);
    if (cmd[wcslen(cmd) - 1] == L'\\')
        cmd[wcslen(cmd) - 1] = L'\0';
    wcscat(cmd, L"\"");

    if (! CreateProcess(
                NULL, cmd, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {

        ULONG err = GetLastError();
        SbieApi_Log(2210, L"%S (%d)", PathW, err);
        return FALSE;
    }

    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);

    return TRUE;
}


//---------------------------------------------------------------------------
// SH32_AdjustPath
//---------------------------------------------------------------------------


_FX WCHAR *SH32_AdjustPath(WCHAR *src, WCHAR **pArgs)
{
    WCHAR *ptr, *dst;
    ULONG len;

    //
    // if src does not begin with a quote, we return it as-is,
    // but we also check if there are any parameters
    //

    if (! src)
        return src;

    if (*src != L'\"') {
        ptr = wcschr(src, L' ');
        if (ptr)
            *pArgs = ptr + 1;
        return src;
    }

    //
    // src is a quoted string so remove the quotes,
    // and also check if there are any parameters
    //

    ptr = wcschr(src + 1, L'\"');
    if (! ptr)
        return src;
    ++ptr;
    if (*ptr == L' ')
        *pArgs = ptr + 1;

    len = (ULONG)(ULONG_PTR)(ptr - src - 2);
    dst = HeapAlloc(GetProcessHeap(), 0, (len + 4) * sizeof(WCHAR));
    if (! dst)
        return src;
    wmemcpy(dst, src + 1, len);
    dst[len] = L'\0';

    return dst;
}


//---------------------------------------------------------------------------
// SH32_ShellExecuteExW
//---------------------------------------------------------------------------


_FX BOOL SH32_ShellExecuteExW(SHELLEXECUTEINFOW *lpExecInfo)
{
    THREAD_DATA *TlsData = Dll_GetTlsData(NULL);
    BOOL CallSystem = TRUE;
    const WCHAR *verb;
    WCHAR *path, *pArgs = NULL;
    BOOL FreePath = FALSE;
    BOOL b;
    ULONG err;
    BOOLEAN is_explore_verb;

    //
    // check if the request is to open a directory
    //

    ++TlsData->sh32_shell_execute;

    verb = lpExecInfo->lpVerb;

    is_explore_verb = (verb && _wcsicmp(verb, L"explore") == 0);
    if ((! verb) || (! verb[0]) || is_explore_verb ||
                                        _wcsicmp(verb, L"open") == 0) {

        if (lpExecInfo->fMask & SEE_MASK_IDLIST) {
            path = HeapAlloc(GetProcessHeap(), HEAP_GENERATE_EXCEPTIONS,
                             (MAX_PATH + 8) * sizeof(WCHAR));
            FreePath = TRUE;
            if (! __sys_SHGetPathFromIDListW(lpExecInfo->lpIDList, path))
                path[0] = L'\0';

        } else {

            path = SH32_AdjustPath((WCHAR *)lpExecInfo->lpFile, &pArgs);
            if (path != lpExecInfo->lpFile)
                FreePath = TRUE;
        }

        if (SbieDll_IsDirectory(path))
            CallSystem = FALSE;
    }

    //
    // call the system in case this is not a directory,
    // or call explorer.exe if this is a directory
    //

    err = 0;
    if (CallSystem) {

        b = __sys_ShellExecuteExW(lpExecInfo);
        if (! b) {
            err = GetLastError();

            //
            // Adobe Flash Installer calls ShellExecuteEx with a quoted
            // lpFile which also includes parameters:
            // "C:\ProgramData\NOS\...\flash_install_ax.exe" /s
            // for some reason, sandboxed ShellExecuteEx fails to locate
            // the file in this case, so we try again with an unquoted
            // lpFile, and split the parameters into lpParameters
            //

            if ((err == ERROR_FILE_NOT_FOUND) &&
                    FreePath && pArgs && (! lpExecInfo->lpParameters)) {

                SHELLEXECUTEINFOW MyExecInfo;

                memcpy(&MyExecInfo, lpExecInfo, sizeof(SHELLEXECUTEINFOW));
                MyExecInfo.lpFile = path;
                MyExecInfo.lpParameters = pArgs;

                b = __sys_ShellExecuteExW(&MyExecInfo);
                if (! b)
                    err = GetLastError();

                lpExecInfo->hInstApp = MyExecInfo.hInstApp;
                lpExecInfo->hProcess = MyExecInfo.hProcess;
            }
        }

    } else {

        b = SH_OpenFolder(
                path, (is_explore_verb ? L'e' : 0), &lpExecInfo->hProcess);

        //
        // set and return results like the real ShellExecuteExW
        //

        if (b)
            lpExecInfo->hInstApp = (HINSTANCE)33;
        else {
            lpExecInfo->hInstApp = (HINSTANCE)SE_ERR_FNF;
            err = ERROR_FILE_NOT_FOUND;
        }
    }

    //
    // finish
    //

    if (FreePath)
        HeapFree(GetProcessHeap(), 0, path);

    --TlsData->sh32_shell_execute;

    SetLastError(err);
    return b;

}


//---------------------------------------------------------------------------
// SH32_BornderToIcon
//---------------------------------------------------------------------------


HICON SH32_BorderToIcon(HICON hIcon, COLORREF color)
{
    typedef HDC(*P_GetDC)(HWND hWnd);
    typedef int(*P_ReleaseDC)(HWND hWnd, HDC hDC);
    typedef BOOL(*P_GetIconInfo)(HICON hIcon, PICONINFO piconinfo);
    typedef HICON(*P_CreateIconIndirect)(PICONINFO piconinfo);

    typedef HDC(*P_CreateCompatibleDC)(HDC hdc);
    typedef HGDIOBJ(*P_SelectObject)(HDC hdc, HGDIOBJ h);
    typedef COLORREF(*P_GetPixel)(HDC hdc, int x, int y);
    typedef COLORREF(*P_SetPixel)(HDC hdc, int x, int y, COLORREF color);
    typedef BOOL(*P_DeleteObject)(HGDIOBJ ho);
    typedef BOOL(*P_DeleteDC)(HDC hdc);

#define GET_WIN_API(name, lib) \
    P_##name name = Ldr_GetProcAddrNew(lib, #name, #name); \
    if(!name) return NULL;

    GET_WIN_API(GetDC, DllName_user32);
    GET_WIN_API(ReleaseDC, DllName_user32);
    GET_WIN_API(GetIconInfo, DllName_user32);
    GET_WIN_API(CreateIconIndirect, DllName_user32);

    GET_WIN_API(CreateCompatibleDC, DllName_gdi32);
    GET_WIN_API(SelectObject, DllName_gdi32);
    GET_WIN_API(GetPixel, DllName_gdi32);
    GET_WIN_API(SetPixel, DllName_gdi32);
    GET_WIN_API(DeleteObject, DllName_gdi32);
    GET_WIN_API(DeleteDC, DllName_gdi32);


    HICON       hNewIcon = NULL;
    HDC         hMainDC = NULL, hMemDC1 = NULL, hMemDC3 = NULL;
    HBITMAP     hOldBmp1 = NULL, hOldBmp3 = NULL;
    ICONINFO    csII;

    if (!GetIconInfo(hIcon, &csII)) return NULL;

    hMainDC = GetDC(NULL);
    hMemDC1 = CreateCompatibleDC(hMainDC);
    hMemDC3 = CreateCompatibleDC(hMainDC);
    if (hMainDC == NULL || hMemDC1 == NULL || hMemDC3 == NULL) return NULL;


    DWORD   dwWidth = csII.xHotspot * 2;
    DWORD   dwHeight = csII.yHotspot * 2;

    hOldBmp1 = (HBITMAP)SelectObject(hMemDC1, csII.hbmColor);
    hOldBmp3 = (HBITMAP)SelectObject(hMemDC3, csII.hbmMask);

    DWORD    dwLoopY = 0, dwLoopX = 0;
    COLORREF crPixel = 0;

    for (dwLoopY = 0; dwLoopY < dwHeight; dwLoopY++)
    {
        for (dwLoopX = 0; dwLoopX < dwWidth; dwLoopX++)
        {
            crPixel = GetPixel(hMemDC1, dwLoopX, dwLoopY);
            if (dwLoopY == 0 || dwLoopX == 0 || dwLoopY + 1 == dwHeight || dwLoopX + 1 == dwWidth)
                crPixel = color; // RGB(255, 255, 0);
            SetPixel(hMemDC1, dwLoopX, dwLoopY, crPixel);

            crPixel = GetPixel(hMemDC3, dwLoopX, dwLoopY);
            if (dwLoopY == 0 || dwLoopX == 0 || dwLoopY + 1 == dwHeight || dwLoopX + 1 == dwWidth)
                crPixel = 0;
            SetPixel(hMemDC3, dwLoopX, dwLoopY, crPixel);
        }
    }

    SelectObject(hMemDC1, hOldBmp1);
    SelectObject(hMemDC3, hOldBmp3);

    hNewIcon = CreateIconIndirect(&csII);


    DeleteObject(csII.hbmColor);
    DeleteObject(csII.hbmMask);
    DeleteDC(hMemDC1);
    DeleteDC(hMemDC3);
    ReleaseDC(NULL, hMainDC);

    return hNewIcon;
}


//---------------------------------------------------------------------------
// SH32_Shell_NotifyIconW
//---------------------------------------------------------------------------


_FX BOOL SH32_Shell_NotifyIconW(
    DWORD dwMessage, PNOTIFYICONDATAW lpData)
{
    BOOL ret;
    HICON icon = NULL;

    if (dwMessage == NIM_ADD || dwMessage == NIM_MODIFY)
    {
        if (!Gui_DisableTitle && lpData && lpData->cbSize >= sizeof(PNOTIFYICONDATAW))
        {
            ULONG len = wcslen(lpData->szTip);

            if (Gui_BoxNameTitleLen != 0 && (len + Gui_BoxNameTitleLen + 2) <= 127)
            {
                wmemmove(lpData->szTip + Gui_BoxNameTitleLen + 2, lpData->szTip, len + 1);
                wmemcpy(lpData->szTip, Gui_BoxNameTitleW, Gui_BoxNameTitleLen);
                wmemcpy(lpData->szTip + Gui_BoxNameTitleLen, L"\r\n", 2);
            }
            else
            {
                if (len + 8 > 127) {
                    lpData->szTip[127 - 8 - 3] = L'\0';
                    wcscat(lpData->szTip, L"...");
                    len = 127 - 8;
                }

                wmemmove(lpData->szTip + 4, lpData->szTip, len + 1);
                wmemcpy(lpData->szTip, L"[#] ", 4);
                wcscat(lpData->szTip, L" [#]");
            }
        }

        COLORREF color;
        if (SbieDll_GetBorderColor(NULL, &color, NULL, NULL))
        {
            HICON newIcon = SH32_BorderToIcon(lpData->hIcon, color);
            if (newIcon) {
                icon = lpData->hIcon;
                lpData->hIcon = newIcon;
            }
        }
    }

    ret = __sys_Shell_NotifyIconW(dwMessage, lpData);

    if (icon) 
    {
        typedef BOOL(*P_DestroyIcon)(HICON hIcon);
        P_DestroyIcon DestroyIcon = Ldr_GetProcAddrNew(DllName_user32, L"DestroyIcon", "DestroyIcon");

        DestroyIcon(lpData->hIcon);
        lpData->hIcon = icon;
    }

    return ret;
}


//---------------------------------------------------------------------------
// SH32_SHChangeNotifyRegister
//---------------------------------------------------------------------------


_FX ULONG SH32_SHChangeNotifyRegister(
    HWND hwnd, int fSources, LONG fEvents, UINT wMsg,
    int cEntries, SHChangeNotifyEntry *pfsne)
{
    ULONG rc;

    if (Dll_ImageType == DLL_IMAGE_INTERNET_EXPLORER && cEntries == 1) {

        ULONG LastError = GetLastError();

        WCHAR *path = Dll_AllocTemp((MAX_PATH + 16) * sizeof(WCHAR));
        if (__sys_SHGetPathFromIDListW(pfsne->pidl, path + 4)) {

            ULONG FileAttrs = GetFileAttributes(path + 4);
            if ((FileAttrs != INVALID_FILE_ATTRIBUTES) &&
                (FileAttrs & FILE_ATTRIBUTE_DIRECTORY)) {

                wmemcpy(path, File_BQQB, 4);
                File_CreateBoxedPath(path);
            }
        }

        Dll_Free(path);
        SetLastError(LastError);
    }

    rc = __sys_SHChangeNotifyRegister(
                    hwnd, fSources, fEvents, wMsg, cEntries, pfsne);

    if (! rc) {

        //
        // the File Open dialog box may get stuck if SHChangeNotifyRegister
        // returns zero (i.e., error) due to not being able to send a
        // message 0x401 (through SendCallbackMessageW) to a WorkerW window
        // in Windows Explorer
        //

        rc = tzuk;
    }

    return rc;
}


//---------------------------------------------------------------------------
// SH32_SHOpenFolderAndSelectItems
//---------------------------------------------------------------------------


_FX ULONG SH32_SHOpenFolderAndSelectItems(
    LPCITEMIDLIST pidlFolder, UINT cidl, void *apidl, ULONG dwFlags)
{
    HRESULT hr = __sys_SHOpenFolderAndSelectItems(
                                        pidlFolder, cidl, apidl, dwFlags);
    if (hr == REGDB_E_CLASSNOTREG || hr == E_UNEXPECTED) {

        //
        // SHOpenFolderAndSelectItems uses ShellWindows/IShellWindows
        // which is not available in the sandbox
        //

        ULONG LastError = GetLastError();

        WCHAR *path = Dll_AllocTemp((MAX_PATH + 64) * sizeof(WCHAR));

        WCHAR verb = 0;

        if (cidl == 0) {

            //
            // if cidl is zero, pidlFolder should specify the full pidl path
            // to an item to select in the parent folder window that opens
            //

            if (SH32_SHOpenFolderAndSelectItems_2(pidlFolder, NULL, path))
                verb = L's';

        } else if (cidl == 1) {

            //
            // if cidl is one, pidlFolder specifies the folder and the first
            // (and only) entry in apidl specifies the child to select.
            // MSDN does not say if the child pidl should be relative or
            // absolute, and firefox uses absolute pidl.  in any case, the
            // ILCombine in SH32_SHOpenFolderAndSelectItems_2 seems to work
            //

            LPCITEMIDLIST pidl2 = ((LPCITEMIDLIST *)apidl)[0];
            if (SH32_SHOpenFolderAndSelectItems_2(pidlFolder, pidl2, path))
                verb = L's';

        } else if (__sys_SHGetPathFromIDListW(pidlFolder, path)) {

            //
            // if we have more than one child entry, we just open the folder
            //

            ULONG FileAttrs = GetFileAttributes(path);
            if ((FileAttrs != INVALID_FILE_ATTRIBUTES) &&
                                    (FileAttrs & FILE_ATTRIBUTE_DIRECTORY))
                verb = L'e';
        }

        if (verb) {

            HANDLE hProcess;
            BOOL b = SH_OpenFolder(path, verb, &hProcess);
            if (hProcess)
                CloseHandle(hProcess);
            if (b) {
                hr = S_OK;
                LastError = 0;
            }
        }

        Dll_Free(path);
        SetLastError(LastError);
    }

    return hr;
}


//---------------------------------------------------------------------------
// SH32_SHOpenFolderAndSelectItems_2
//---------------------------------------------------------------------------


_FX BOOLEAN SH32_SHOpenFolderAndSelectItems_2(
    LPCITEMIDLIST pidlParent, LPCITEMIDLIST pidlChild, WCHAR *path)
{
    typedef BOOL (*P_ILIsParent)(
        LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2, BOOL fImmediate);
    typedef LPITEMIDLIST (*P_ILCombine)(
        LPCITEMIDLIST pidl1, LPCITEMIDLIST pid2);
    typedef void (*P_SHFree)(void *pv);

    BOOLEAN result = FALSE;

    P_ILIsParent pILIsParent = (P_ILIsParent)
        Ldr_GetProcAddrNew(DllName_shell32, L"ILIsParent","ILIsParent");

    P_ILCombine pILCombine = (P_ILCombine)
        Ldr_GetProcAddrNew(DllName_shell32, L"ILCombine","ILCombine");

    P_SHFree pSHFree = (P_SHFree)
        Ldr_GetProcAddrNew(DllName_shell32, L"SHFree","SHFree");

    if (pILIsParent && pILCombine && pSHFree) {

        LPCITEMIDLIST pidl;
        if (pidlParent && pidlChild &&
                            pILIsParent(pidlParent, pidlChild, FALSE)) {
            // if pidlChild is an absolute pidl with a full path
            // then we don't really need to combine it with parent
            pidlParent = NULL;
        }
        pidl = pILCombine(pidlParent, pidlChild);

        if (pidl) {

            if (__sys_SHGetPathFromIDListW(pidl, path)) {

                ULONG FileAttrs = GetFileAttributes(path);
                if (FileAttrs != INVALID_FILE_ATTRIBUTES) {

                    result = TRUE;
                }
            }

            pSHFree((void *)pidl);
        }
    }

    return result;
}


//---------------------------------------------------------------------------
// SH32_DoRunAs
//---------------------------------------------------------------------------


_FX BOOL SH32_DoRunAs(
    const WCHAR *CmdLine, const WCHAR *WorkDir,
    PROCESS_INFORMATION *pi, BOOL *cancelled)
{
    THREAD_DATA *TlsData = Dll_GetTlsData(NULL);
    WCHAR *arg;
    SHELLEXECUTEINFO shex;
    HMODULE shell32;

    //
    // implementation of the standard ElevateCreateProcess fix / shim
    // initially created for Internet Explorer, however this seems to be
    // needed in other programs as well.
    //
    // abort if running inside ShellExecuteEx which already does this
    //

    if (TlsData->sh32_shell_execute)
        return FALSE;

    //
    // separate the first component of the path, i.e. the program name,
    // and the rest of the path, i.e. the arguments to the program.
    // remove any quotes around the program name.
    //

    if (CmdLine == NULL)
        return FALSE;

    if (CmdLine[0] == L'\"') {
        ++CmdLine;
        arg = wcschr(CmdLine, L'\"');
        if (! arg)
            return FALSE;
    } else
        arg = wcschr(CmdLine, L' ');
    if (arg) {
        *arg = L'\0';
        do { ++arg; } while (*arg == L' ');
    }

    //
    // use ShellExecute to run the program with elevation
    //

    memzero(pi, sizeof(PROCESS_INFORMATION));

    memzero(&shex, sizeof(SHELLEXECUTEINFO));
    shex.cbSize = sizeof(SHELLEXECUTEINFO);
    shex.fMask = SEE_MASK_NOCLOSEPROCESS;
    shex.hwnd = NULL;
    shex.lpVerb = L"runas";
    shex.lpFile = CmdLine;
    shex.lpParameters = arg;
    shex.lpDirectory = WorkDir;
    shex.nShow = SW_SHOWNORMAL;
    shex.hInstApp = NULL;

    shell32 = LoadLibrary(DllName_shell32);
    if (shell32) {

        P_ShellExecuteEx pShellExecuteEx = (P_ShellExecuteEx)
            GetProcAddress(shell32, "ShellExecuteExW");

        if (pShellExecuteEx) {

            if (pShellExecuteEx(&shex) &&
                    ((int)(ULONG_PTR)shex.hInstApp > 32)) {

                pi->hProcess = shex.hProcess;
                return TRUE;
            }

            if (GetLastError() == ERROR_CANCELLED)
                *cancelled = TRUE;
        }

        FreeLibrary(shell32);
    }

    return FALSE;
}


//---------------------------------------------------------------------------
// SH32_GetPrivateProfileStringW
//---------------------------------------------------------------------------


_FX ULONG SH32_GetPrivateProfileStringW(
    void *lpAppName, void *lpKeyName, void *lpDefault,
    void *lpReturnedString, ULONG nSize, void *lpFileName)
{
    ULONG rv = __sys_GetPrivateProfileStringW(
        lpAppName, lpKeyName, lpDefault,
        lpReturnedString, nSize, lpFileName);

    if (rv == 12) {

        const WCHAR *AppNameW  = (WCHAR *)lpAppName;
        const WCHAR *KeyNameW  = (WCHAR *)lpKeyName;
        const WCHAR *FileNameW = (WCHAR* )lpFileName;
        const WCHAR *RetStrW   = (WCHAR *)lpReturnedString;
        if (AppNameW  && 0 == _wcsicmp(AppNameW, L"boot")           &&
            KeyNameW  && 0 == _wcsicmp(KeyNameW, L"shell")          &&
            FileNameW && 0 == _wcsicmp(FileNameW, L"system.ini")    &&
            RetStrW   && 0 == _wcsicmp(RetStrW, L"explorer.exe")) {

            *(WCHAR *)(RetStrW + 0) = L'x';
            *(WCHAR *)(RetStrW + 1) = L'\0';
            rv = 1;
        }
    }

    return rv;
}


//---------------------------------------------------------------------------
// SH32_SHGetFolderLocation
//---------------------------------------------------------------------------


_FX HRESULT SH32_SHGetFolderLocation(
    HWND hwndOwner, int nFolder, HANDLE hToken,
    DWORD dwReserved, void **ppidl)
{
    HRESULT hr = __sys_SHGetFolderLocation(
                        hwndOwner, nFolder, hToken, dwReserved, ppidl);

    if (hr == 0x80070002 && nFolder == CSIDL_PERSONAL) {

        //
        // if the Documents folder is missing, Windows Explorer hangs
        // on startup on Windows 7.  work around that by creating the
        // Documents folder in the sandbox
        //

        hr = __sys_SHGetFolderLocation(
                        hwndOwner, nFolder | CSIDL_FLAG_CREATE, hToken,
                        dwReserved, ppidl);
    }

    if (hr != 0 && nFolder == CSIDL_PERSONAL) {

        //
        // if the Documents folder is missing outside the sandbox,
        // and ClosedFilePath (or something else) prevents its creation
        // in the sandbox, then issue an error message
        //

        SbieApi_Log(2306, L"Documents");
    }

    return hr;
}


//---------------------------------------------------------------------------
// SH32_LdrGetDllHandleEx
//---------------------------------------------------------------------------


_FX NTSTATUS SH32_LdrGetDllHandleEx(
    ULONG_PTR One, ULONG_PTR Unknown1, ULONG_PTR Unknown2,
    UNICODE_STRING *DllName, ULONG_PTR *hModule)
{
    //
    // block GetModuleHandle calls for the explorer.exe module, which is
    // done primarily by SHELL32!IsProcessAnExplorer.  this is necessary
    // because SHELL32!CMountPoint::GetDrivesMask uses IsProcessAnExplorer
    // to decide if it will use GetLogicalDrives or rely on the
    // Shell HW Detection service, and we want to force GetLogicalDrives.
    //

    if (DllName && DllName->Buffer) {

        const WCHAR *_DllName = L"explorer.exe";
        const ULONG len = wcslen(_DllName);
        if (DllName->Length == len * sizeof(WCHAR) &&
            _wcsnicmp(DllName->Buffer, _DllName, len) == 0) {

            *hModule = 0;
            return STATUS_DLL_NOT_FOUND;
        }
    }

    return __sys_LdrGetDllHandleEx(
                    One, Unknown1, Unknown2, DllName, hModule);
}


//---------------------------------------------------------------------------
// SH32_Init
//---------------------------------------------------------------------------


_FX BOOLEAN SH32_Init(HMODULE module)
{
    P_ShellExecuteEx ShellExecuteExW;
    P_Shell_NotifyIconW Shell_NotifyIconW;
    P_SHChangeNotifyRegister SHChangeNotifyRegister;
    void *SHGetItemFromObject;
    P_SHOpenFolderAndSelectItems SHOpenFolderAndSelectItems;
    //
    // find utility APIs
    //

    __sys_SHGetPathFromIDListW = (P_SHGetPathFromIDList)
        GetProcAddress(module, "SHGetPathFromIDListW");

    //
    // intercept entry points
    //

    ShellExecuteExW = (P_ShellExecuteEx)
        GetProcAddress(module, "ShellExecuteExW");

    Shell_NotifyIconW = (P_Shell_NotifyIconW)
        GetProcAddress(module, "Shell_NotifyIconW");

    SHChangeNotifyRegister = (P_SHChangeNotifyRegister)
        GetProcAddress(module, "SHChangeNotifyRegister");

    SHGetItemFromObject = GetProcAddress(module, "SHGetItemFromObject");

    SHOpenFolderAndSelectItems = (P_SHOpenFolderAndSelectItems)
        GetProcAddress(module, "SHOpenFolderAndSelectItems");

    SBIEDLL_HOOK(SH32_,ShellExecuteExW);

    SBIEDLL_HOOK(SH32_,Shell_NotifyIconW);

    if (SHChangeNotifyRegister && SHGetItemFromObject) {

        //
        // the test for Windows 7-only SHGetItemFromObject is needed
        // to hook SHChangeNotifyRegister only on Windows 7
        //

        SBIEDLL_HOOK(SH32_,SHChangeNotifyRegister);
    }

    if (SHOpenFolderAndSelectItems) {

        SBIEDLL_HOOK(SH32_,SHOpenFolderAndSelectItems);
    }

    //
    // windows 7 taskbar management
    //

    if (! Taskbar_Init(module))
        return FALSE;

    //
    // install window monitor thread to auto-close Explorer windows
    //

    if (    Dll_ImageType == DLL_IMAGE_SHELL_EXPLORER
        ||  Dll_ImageType == DLL_IMAGE_INTERNET_EXPLORER) {

        WCHAR buf[8];
        SbieApi_QueryConfAsIs(
            NULL, L"NoAutoExitExplorer", 0, buf, sizeof(buf));
        if (! buf[0]) {

			HANDLE ThreadHandle = CreateThread(NULL, 0, SH_WindowMonitorThread, NULL, 0, NULL);
			if (ThreadHandle)
				CloseHandle(ThreadHandle);
        }
    }

    //
    // initialization for the Windows Shell Explorer process
    //

    if (Dll_ImageType == DLL_IMAGE_SHELL_EXPLORER) {

        //
        // hook ntdll!LdrGetDllHandleEx
        //

        if (! __sys_LdrGetDllHandleEx) {

            __sys_LdrGetDllHandleEx = (P_LdrGetDllHandleEx)
                GetProcAddress(Dll_Ntdll, "LdrGetDllHandleEx");
        }

        if (__sys_LdrGetDllHandleEx) {

            *(ULONG_PTR *)&__sys_LdrGetDllHandleEx = (ULONG_PTR)
                SbieDll_Hook("LdrGetDllHandleEx",
                    __sys_LdrGetDllHandleEx, SH32_LdrGetDllHandleEx, module);
        }

        //
        // hook GetPrivateProfileString to prevent Windows Explorer shell
        // process from taking over the desktop
        //

        if (1) {

            P_GetPrivateProfileString GetPrivateProfileStringW;

            GetPrivateProfileStringW = (P_GetPrivateProfileString)
                GetProcAddress(Dll_Kernel32, "GetPrivateProfileStringW");

            SBIEDLL_HOOK(SH32_,GetPrivateProfileStringW);
        }

        //
        // on Windows Vista, the registry key
        //         CLSID\{ceff45ee-c862-41de-aee2-a022c81eda92}
        // (for CLSID_SeparateExplorerFactory) is associated with an
        // AppId key, which causes failure with status
        //         CO_E_WRONG_SERVER_IDENTITY.
        // this association is undone to let Explorer start sandboxed.
        //

        if (Dll_OsBuild >= 6000) {

            Key_DeleteValueFromCLSID(
                L"clsid", L"ceff45ee-c862-41de-aee2-a022c81eda92", L"AppId");
        }

        //
        // on Windows 7, hook SHGetFolderLocation in Windows Explorer
        //

        if (Dll_OsBuild >= 7600) {

            P_SHGetFolderLocation SHGetFolderLocation;

            SHGetFolderLocation = (P_SHGetFolderLocation)
                GetProcAddress(module, "SHGetFolderLocation");

            SBIEDLL_HOOK(SH32_,SHGetFolderLocation);
        }
    }

    return TRUE;
}


//---------------------------------------------------------------------------
// SbieDll_AssocQueryKeyWow64
//---------------------------------------------------------------------------


_FX HKEY SbieDll_AssocQueryKeyWow64(const WCHAR *subj)
{
    NTSTATUS status;
    OBJECT_ATTRIBUTES objattrs;
    UNICODE_STRING objname;
    HKEY root;
    WCHAR subkey[128], *subkey2;
    HANDLE hkey;

    InitializeObjectAttributes(
        &objattrs, &objname, OBJ_CASE_INSENSITIVE, NULL, NULL);

    root = HKEY_CURRENT_USER;

retry:

    //
    // open HKEY_CLASSES_ROOT key for (subj)
    //

    wcscpy(subkey, L"\\REGISTRY\\");
    if (root == HKEY_CURRENT_USER)
        wcscat(subkey, L"USER\\CURRENT");
    else if (root == HKEY_LOCAL_MACHINE)
        wcscat(subkey, L"MACHINE");
    wcscat(subkey, L"\\SOFTWARE\\CLASSES\\");
    subkey2 = subkey + wcslen(subkey);
    wcscpy(subkey2, subj);
    RtlInitUnicodeString(&objname, subkey);

    status = NtOpenKey(&hkey, KEY_READ, &objattrs);
    if (NT_SUCCESS(status))
        return hkey;

    //
    // if we are a Wow64 process and couldn't find in HKEY_CLASSES_ROOT,
    // then look again in HKEY_CLASSES_ROOT\Wow64
    //

#ifndef _WIN64
    if (Dll_IsWow64) {

        wcscpy(subkey2, L"Wow6432Node\\");
        wcscat(subkey2, subj);
        RtlInitUnicodeString(&objname, subkey);

        status = NtOpenKey(&hkey, KEY_READ, &objattrs);
        if (NT_SUCCESS(status))
            return hkey;
    }
#endif

    //
    // if we looked at HKEY_CURRENT_USER, try again for HKEY_LOCAL_MACHINE
    //

    if (root == HKEY_CURRENT_USER) {

        root = HKEY_LOCAL_MACHINE;
        goto retry;
    }

    return NULL;
}


//---------------------------------------------------------------------------
// SbieDll_AssocQueryCommandInternal
//---------------------------------------------------------------------------


_FX WCHAR *SbieDll_AssocQueryCommandInternal(
    const WCHAR *subj, const WCHAR *verb)
{
    NTSTATUS status;
    OBJECT_ATTRIBUTES objattrs;
    UNICODE_STRING objname;
    HANDLE hkey, hkey2;
    WCHAR subkey[64];
    ULONG len;
    KEY_VALUE_PARTIAL_INFORMATION *kvpi;
    const ULONG kvpi_len = 1024;
    ULONG retry_count = 0;
    WCHAR *buf, *buf2;

    //
    // open the key for (subj), but if (subj) is really a dummy key pointing
    // somewhere else, then open the pointed-to key
    //

    kvpi = Dll_Alloc(kvpi_len);

    hkey = SbieDll_AssocQueryKeyWow64(subj);

    wcscpy(subkey, L"shell\\");
    wcscat(subkey, verb);
    wcscat(subkey, L"\\command");

    buf = NULL;

retry:

    ++retry_count;
    if ((! hkey) || retry_count == 8)
        goto finish;

    InitializeObjectAttributes(
        &objattrs, &objname, OBJ_CASE_INSENSITIVE, hkey, NULL);

    RtlInitUnicodeString(&objname, subkey);

    status = NtOpenKey(&hkey2, KEY_READ, &objattrs);
    if (! NT_SUCCESS(status)) {

        if (status != STATUS_OBJECT_NAME_NOT_FOUND &&
            status != STATUS_OBJECT_PATH_NOT_FOUND)
        {
            goto finish;
        }

        RtlInitUnicodeString(&objname, L"");
        ZeroMemory(kvpi, kvpi_len);
        status = NtQueryValueKey(
            hkey, &objname, KeyValuePartialInformation,
            kvpi, kvpi_len - 32, &len);

        if ((! NT_SUCCESS(status)) || kvpi->DataLength == 0)
            goto finish;

        NtClose(hkey);

        hkey = SbieDll_AssocQueryKeyWow64((WCHAR *)kvpi->Data);

        goto retry;
    }

    NtClose(hkey);
    hkey = hkey2;
    hkey2 = NULL;

    //
    // we have the key (subj)\shell\verb\command, get the default value
    //

    RtlInitUnicodeString(&objname, L"");
    ZeroMemory(kvpi, kvpi_len);
    status = NtQueryValueKey(
        hkey, &objname, KeyValuePartialInformation,
        kvpi, kvpi_len - 32, &len);

    if ((! NT_SUCCESS(status)) || kvpi->DataLength == 0 ||
            (kvpi->Type != REG_SZ && kvpi->Type != REG_EXPAND_SZ))
    {
        goto finish;
    }

    len = kvpi->DataLength;
    buf = Dll_Alloc(len + sizeof(WCHAR) * 2);
    ZeroMemory(buf, len + sizeof(WCHAR) * 2);
    CopyMemory(buf, kvpi->Data, len);

    //
    // finally, expand any environment strings within
    //

    if (kvpi->Type == REG_EXPAND_SZ) {

        len = ExpandEnvironmentStrings(buf, NULL, 0);
        if (len == 0) {
            Dll_Free(buf);
            buf = NULL;
            goto finish;
        }

        buf2 = Dll_Alloc((len + 8) * sizeof(WCHAR));
        ExpandEnvironmentStrings(buf, buf2, len);
        buf2[len] = L'\0';

        Dll_Free(buf);
        buf = buf2;
    }

    //
    // finish
    //

finish:

    if (hkey)
        NtClose(hkey);
    Dll_Free(kvpi);

    return buf;
}


//---------------------------------------------------------------------------
// SbieDll_AssocQueryCommand
//---------------------------------------------------------------------------


_FX WCHAR *SbieDll_AssocQueryCommand(const WCHAR *subj)
{
    WCHAR *cmd = SbieDll_AssocQueryCommandInternal(subj, L"open");
    if (! cmd)
        cmd = SbieDll_AssocQueryCommandInternal(subj, L"cplopen");
    return cmd;
}


//---------------------------------------------------------------------------
// SbieDll_AssocQueryProgram
//---------------------------------------------------------------------------


_FX WCHAR *SbieDll_AssocQueryProgram(const WCHAR *subj)
{
    WCHAR *cmd, *buf;
    WCHAR *ptr, *ptr2;
    WCHAR ch;
    ULONG len;

    cmd = SbieDll_AssocQueryCommand(subj);
    if (! cmd)
        return NULL;

    //
    // strip leading blanks and quotes, leave first word
    //

    ptr = cmd;
    while (*ptr && *ptr == L' ')
        ++ptr;
    if (*ptr == L'\"') {
        ch = L'\"';
        ++ptr;
    } else
        ch = L' ';
    ptr2 = wcschr(ptr, ch);
    if (ptr2)
        *ptr2 = L'\0';

    //
    // in case the path is in short form, get the long name
    //

    len = GetLongPathName(ptr, NULL, 0);
    if (len == 0) {
        Dll_Free(cmd);
        return NULL;
    }

    buf = Dll_Alloc((len + 8) * sizeof(WCHAR));
    GetLongPathName(ptr, buf, len);
    buf[len] = L'\0';

    Dll_Free(cmd);
    return buf;
}


//---------------------------------------------------------------------------
// SH_GetInternetExplorerVersion
//---------------------------------------------------------------------------


_FX ULONG SH_GetInternetExplorerVersion(void)
{
    NTSTATUS status;
    HANDLE handle;
    OBJECT_ATTRIBUTES objattrs;
    UNICODE_STRING objname;
    ULONG version;

    if (Dll_ImageType != DLL_IMAGE_INTERNET_EXPLORER)
        return 0;

    RtlInitUnicodeString(&objname,
        L"\\REGISTRY\\MACHINE\\SOFTWARE\\Microsoft\\Internet Explorer");
    InitializeObjectAttributes(
        &objattrs, &objname, OBJ_CASE_INSENSITIVE, NULL, NULL);

    status = NtOpenKey(&handle, KEY_READ, &objattrs);
    if (NT_SUCCESS(status)) {

        ULONG len;
        ULONG kvpi_space[64];
        KEY_VALUE_PARTIAL_INFORMATION *kvpi =
            (KEY_VALUE_PARTIAL_INFORMATION *)&kvpi_space;

        RtlInitUnicodeString(&objname, L"svcVersion");  // for IE 10
        memzero(kvpi_space, sizeof(kvpi_space));

        status = NtQueryValueKey(
            handle, &objname, KeyValuePartialInformation,
            kvpi, sizeof(kvpi_space) - 4, &len);

        if (! NT_SUCCESS(status)) {

            RtlInitUnicodeString(&objname, L"Version"); // IE 9 and earlier
            memzero(kvpi_space, sizeof(kvpi_space));

            status = NtQueryValueKey(
                handle, &objname, KeyValuePartialInformation,
                kvpi, sizeof(kvpi_space) - 4, &len);
        }

        if (NT_SUCCESS(status) && kvpi->Type == REG_SZ) {

            version = _wtoi((WCHAR *)kvpi->Data);
        }

        NtClose(handle);
    }

    return version;
}


//---------------------------------------------------------------------------
//
// SHSetInstanceExplorer support
//
// Code running in Explorer (both Windows and Internet) may AddRef() on the
// host process using SHGetInstanceExplorer and then forget to Release().
// This causes a sandboxed IE or Explorer to go on running indefinitely.
// To work around this, we have a thread that monitors the number of open
// windows, and forces Explorer to close when there are no more windows.
//
// Alternatively, in an IE 8 tab process, there are no open windows, so
// in this case we wait for the main IE process to terminate
//
//---------------------------------------------------------------------------


static ULONG SH_WindowMonitorThread(void *lpParameter);

static void SH_WindowMonitorThread_2(void);

static ULONG SH_WindowMonitorCount(void);

static BOOL SH_WindowMonitorEnum(HWND hWnd, LPARAM lParam);


//---------------------------------------------------------------------------
// SH_WindowMonitorThread
//---------------------------------------------------------------------------


_FX ULONG SH_WindowMonitorThread(void *lpParameter)
{
    ULONG win_count;
    BOOLEAN any_windows_ever_seen = FALSE;

    const ULONG timeout =
        (Dll_ImageType == DLL_IMAGE_SHELL_EXPLORER) ? 2500 : 5000;

    SH_WindowMonitorThread_2();

    while (1) {

        Sleep(any_windows_ever_seen ? timeout : 500);

        win_count = SH_WindowMonitorCount();

        if (win_count)
            any_windows_ever_seen = TRUE;

        else if (any_windows_ever_seen) {

            Sleep(timeout);

            win_count = SH_WindowMonitorCount();

            if (win_count == 0) {

                if (SH_GetInternetExplorerVersion() >= 8) {

                    //
                    // Internet Explorer version 8 needs a little more time
                    // to properly shut down after closing all windows.
                    // otherwise next run will try to recover last session.
                    //

                    Sleep(30000);
                }

                ExitProcess(0);
            }
        }
    }

    return 0;
}


//---------------------------------------------------------------------------
// SH_WindowMonitorThread_2
//---------------------------------------------------------------------------


_FX void SH_WindowMonitorThread_2(void)
{
    extern BOOLEAN Secure_IsInternetExplorerTabProcess;
    if (! Secure_IsInternetExplorerTabProcess)
        return;
    Proc_WaitForParentExit(0);
    Sleep(8000);
    ExitProcess(0);
}


//---------------------------------------------------------------------------
// SH_WindowMonitorCount
//---------------------------------------------------------------------------


_FX ULONG SH_WindowMonitorCount(void)
{
    SYSTEM_PROCESS_INFORMATION *info, *info_ptr;
    ULONG_PTR args[2];
    NTSTATUS status;
    ULONG i;

    //
    // get list of processes active in the system
    //

    info = SysInfo_QueryProcesses(NULL);
    if (! info)
        return 1234;

    //
    // for each process, set HandleCount = 1 if
    // - this is the current process
    // - this iexplore.exe process is a child of the current process
    // otherwise set HandleCount = 0
    //

    info_ptr = info;
    while (1) {

        info_ptr->HandleCount = 0;

        if (Dll_ProcessId == (ULONG_PTR)info_ptr->UniqueProcessId)
            info_ptr->HandleCount = 1;

        else if (Dll_ImageType == DLL_IMAGE_INTERNET_EXPLORER &&
                 Dll_ProcessId ==
                            (ULONG_PTR)info_ptr->InheritedFromProcessId) {

            WCHAR image[112];
            status = SbieApi_QueryProcess(info_ptr->UniqueProcessId,
                                          NULL, image, NULL, NULL);
            if (NT_SUCCESS(status) && _wcsicmp(image, L"iexplore.exe") == 0)
                info_ptr->HandleCount = 1;
        }

        i = info_ptr->NextEntryOffset;
        if (! i)
            break;
        info_ptr = (SYSTEM_PROCESS_INFORMATION *)((UCHAR *)info_ptr + i);
    }

    //
    // count active windows in the processes with HandleCount = 1
    //

    args[0] = (ULONG_PTR)info;
    args[1] = 0;

    if(!Gui_UseProxyService && __sys_EnumWindows)
		__sys_EnumWindows(SH_WindowMonitorEnum, (LPARAM)args);
    else
        Gui_EnumWindows(SH_WindowMonitorEnum, (LPARAM)args);

    Dll_Free(info);

    return (ULONG)args[1];
}


//---------------------------------------------------------------------------
// SH_WindowMonitorEnum
//---------------------------------------------------------------------------


_FX BOOL SH_WindowMonitorEnum(HWND hWnd, LPARAM lParam)
{
    extern DWORD (*__sys_GetWindowThreadProcessId)(HWND hWnd, ULONG *ppid);
    extern BOOL Gui_IsWindowVisible(HWND hWnd);

    SYSTEM_PROCESS_INFORMATION *info_ptr;
    ULONG pid, i;

    ULONG_PTR *args = (ULONG_PTR *)lParam;

    if (Gui_IsWindowVisible(hWnd)) {

        __sys_GetWindowThreadProcessId(hWnd, &pid);

        info_ptr = (SYSTEM_PROCESS_INFORMATION *)args[0];
        while (1) {

            if (info_ptr->HandleCount &&
                    (ULONG)(ULONG_PTR)info_ptr->UniqueProcessId == pid)
                ++args[1];

            i = info_ptr->NextEntryOffset;
            if (! i)
                break;
            info_ptr = (SYSTEM_PROCESS_INFORMATION *)((UCHAR *)info_ptr + i);
        }
    }

    return TRUE;
}


//---------------------------------------------------------------------------
//
// ZipFldr support
//
// The standard ZipFldr.dll pops up an extraction dialog box when run
// sandboxed.  We want to make sure that an Explorer window opens instead.
//
//---------------------------------------------------------------------------


typedef void (*P_RouteTheCall)(
    void *Unknown1, HINSTANCE hInstance, const UCHAR *Path, ULONG Unknown2);


static ULONG_PTR __sys_RouteTheCall = 0;


_FX void SH32_RouteTheCall(
    void *Unknown1, HINSTANCE hInstance, const UCHAR *Path, ULONG Unknown2)
{
    ANSI_STRING ansi;
    UNICODE_STRING uni;
    WCHAR *iptr, *optr;
    HANDLE hProcess;

    RtlInitString(&ansi, Path);
    uni.Buffer = NULL;
    RtlAnsiStringToUnicodeString(&uni, &ansi, TRUE);

    iptr = uni.Buffer;
    optr = uni.Buffer;
    while (*iptr) {
        if (*iptr != L'\"') {
            *optr = *iptr;
            ++optr;
        }
        ++iptr;
    }
    *optr = L'\0';

    SH_OpenFolder(uni.Buffer, 0, &hProcess);

    ExitProcess(0);
}


_FX BOOLEAN SH32_Init_ZipFldr(HMODULE module)
{
    P_RouteTheCall RouteTheCall;

    if (Dll_ImageType != DLL_IMAGE_RUNDLL32)
        return TRUE;

    RouteTheCall = (P_RouteTheCall) GetProcAddress(module, "RouteTheCall");
    if (RouteTheCall) {

        SBIEDLL_HOOK(SH32_,RouteTheCall);
    }

    return TRUE;
}


//---------------------------------------------------------------------------
//
// UxTheme support
//
// Hook the Windows Vista API SetWindowThemeAttribute so we can prevent
// the Shell Explorer from hiding its caption bar.
//
//---------------------------------------------------------------------------


typedef HRESULT (*P_SetWindowThemeAttribute)(
    HWND hwnd, ULONG eAttribute, void *pvAttribute, ULONG cbAttribute);


static ULONG_PTR __sys_SetWindowThemeAttribute = 0;


_FX HRESULT SH32_SetWindowThemeAttribute(
    HWND hwnd, ULONG eAttribute, void *pvAttribute, ULONG cbAttribute)
{
    return S_OK;
}


_FX BOOLEAN SH32_Init_UxTheme(HMODULE module)
{
    if (Dll_ImageType == DLL_IMAGE_SHELL_EXPLORER) {

        P_SetWindowThemeAttribute SetWindowThemeAttribute;

        SetWindowThemeAttribute = (P_SetWindowThemeAttribute)
            GetProcAddress(module, "SetWindowThemeAttribute");

        if (SetWindowThemeAttribute) {

            SBIEDLL_HOOK(SH32_,SetWindowThemeAttribute);
        }
    }

    return TRUE;
}


//---------------------------------------------------------------------------
//
// IContextMenu support
//
// Make sure that Windows Explorer passes a full path (including the
// sandbox prefix) for selected right-click shell extension handlers
// which are known to pass this path to a component outside the sandbox
//
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------


_FX void SH32_IContextMenu_HookVtbl(void *lpVtbl, int index, void *func)
{
    //
    // hook a specified member in the specified vtbl.  note that before
    // hooking, we check if we already hooked the specified vtbl
    //

    ULONG_PTR *vtbl = *(ULONG_PTR **)lpVtbl;
    void *xold = (void *)vtbl[index];
    void *xnew;
    ULONG_PTR *StubData = Dll_JumpStubDataForCode(xold);
    if (StubData && StubData[2] == tzuk)
        xnew = NULL;
    else
        xnew = Dll_JumpStub(xold, func, tzuk);
    if (xnew) {
        ULONG prot = PAGE_READWRITE;
        if (VirtualProtect(&vtbl[index], sizeof(ULONG_PTR), prot, &prot)) {
            vtbl[index] = (ULONG_PTR)xnew;
            VirtualProtect(&vtbl[index], sizeof(ULONG_PTR), prot, &prot);
        }
    }
}


_FX HRESULT SH32_IShellExtInit_Initialize(
    void *pShellExtInit,
    void *pidlFolder, IDataObject *pDataObject, HKEY hkeyProgID)
{
#if !defined(_M_ARM64) && !defined(_M_ARM64EC)
    ULONG_PTR *StubData = Dll_JumpStubData();
#else
    ULONG_PTR *StubData = (ULONG_PTR *)hkeyProgID;
    hkeyProgID = (HKEY)StubData[3];
#endif

    extern IDataObject *Ole_XDataObject_From_IDataObject(
        IDataObject *pDataObject);
    typedef HRESULT (*P_Initialize)(void *, void *, IDataObject *, HKEY);
    return ((P_Initialize)StubData[1])(
                        pShellExtInit, pidlFolder,
                        Ole_XDataObject_From_IDataObject(pDataObject),
                        hkeyProgID);
}


_FX HRESULT SH32_IContextMenuHook_QueryInterface(
    void *pContextMenu, REFIID riid, void **ppv
#if !defined(_M_ARM64) && !defined(_M_ARM64EC)
    ) {
    ULONG_PTR *StubData = Dll_JumpStubData();
#else 
    , ULONG_PTR *StubData) {
#endif

    EXTERN_C const IID IID_IShellExtInit;
    typedef HRESULT (*P_QueryInterface)(void *, REFIID, void **);
    HRESULT hr = ((P_QueryInterface)StubData[1])(pContextMenu, riid, ppv);
    if (SUCCEEDED(hr) &&
            memcmp(riid, &IID_IShellExtInit, sizeof(GUID)) == 0) {

        void *pShellExtInit = (*ppv);
        SH32_IContextMenu_HookVtbl(
            pShellExtInit, 3, SH32_IShellExtInit_Initialize);
    }

    return hr;
}


_FX void SH32_IContextMenu_Hook(REFCLSID rclsid, void *pContextMenu)
{
    //
    // this function is called by Com_CoCreateInstance when the
    // interface is IContextMenu.  check if the associated CLSID
    // is listed in the IContextMenuClsid setting for the box
    //

    typedef HRESULT (*P_IIDFromString)(LPCOLESTR lpsz, IID *lpiid);

    WCHAR buf[96];
    GUID guid;
    ULONG index = 0;
    BOOLEAN match = FALSE;

    P_IIDFromString pIIDFromString = (P_IIDFromString)
        Ldr_GetProcAddrNew(DllName_ole32_or_combase, L"IIDFromString","IIDFromString");
    if (! pIIDFromString)
        return;

    while (1) {
        NTSTATUS status = SbieApi_QueryConfAsIs(
                NULL, L"IContextMenuClsid", index, buf, 90 * sizeof(WCHAR));
        if (! NT_SUCCESS(status))
            break;
        ++index;
        if (pIIDFromString(buf, &guid) == 0 &&
                memcmp(rclsid, &guid, sizeof(GUID)) == 0) {
            match = TRUE;
            break;
        }
    }

    //
    // given an IContextMenu interface, hook its QueryInterface function
    // so we can intercept requests for the IShellExtInit interface
    //

    if (match) {

        SH32_IContextMenu_HookVtbl(
            pContextMenu, 0, SH32_IContextMenuHook_QueryInterface);
    }
}

