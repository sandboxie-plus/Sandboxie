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
#include "core/svc/GuiWire.h"
#include "core/svc/UserWire.h"

//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------


static BOOL SH_OpenFolder(const WCHAR *PathW, WCHAR verb, HANDLE *hProcess);

static WCHAR *SH32_AdjustPath(WCHAR *src, WCHAR **pArgs);

static HKEY SbieDll_AssocQueryKeyWow64(const WCHAR *subj);

static BOOL SH32_ShellExecuteExW(SHELLEXECUTEINFOW *lpExecInfo);

static BOOL SH32_Shell_NotifyIconW(
    DWORD dwMessage, PNOTIFYICONDATAW lpData);

static ULONG SH32_wcsnlen(const WCHAR *src, ULONG maxChars);

static ULONG SH32_NotifyIconMaxChars(
    DWORD cbSize, ULONG fieldOffsetBytes, ULONG fieldCapacityChars);

static BOOLEAN SH32_Shell_NotifyIcon_ProxyCall(
    DWORD dwMessage, PNOTIFYICONDATAW lpData, BOOL *ret);

//static HRESULT SH32_SHGetFolderPathW(
//    HWND hwnd, int csidl, HANDLE hToken, DWORD dwFlags, LPWSTR pszPath);

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

static HRESULT WINAPI SH32_IShellWindows_FindWindowSW(
    void *pShellWindows,
    VARIANT *pvarLoc, VARIANT *pvarLocRoot,
    int swClass, long *phwnd, int swfwOptions,
    IDispatch **ppdispOut
#if defined(_M_ARM64) || defined(_M_ARM64EC)
    , ULONG_PTR *StubData
#endif
);

static void SH32_ComRelease(void *pUnknown);

static void SH32_ComAddRef(void *pUnknown);

static BOOLEAN SH32_IsUsableShellDispatch(IDispatch *pDispatch);

static HRESULT SH32_IShellWindows_ItemDispatch(
    void *pShellWindows, LONG index, IDispatch **ppDispatch);

static HRESULT SH32_IShellWindows_GetCount(
    void *pShellWindows, long *pCount);

static void *SH32_FakeDesktop_Create(void);
static void *SH32_FakeServiceProvider_Create(void *pDesktop);
static void *SH32_FakeBrowserView_Create(void);
static void *SH32_FakeShellView_Create(void);
static void *SH32_FakeFolderView_Create(void);
static void *SH32_FakeShellApp_Create(void);

static const GUID SH32_IID_IServiceProvider =
{ 0x6d5140c1, 0x7436, 0x11ce,{ 0x80, 0x34, 0x00, 0xaa, 0x00, 0x60, 0x09, 0xfa } };

static const GUID SH32_IID_IShellBrowser =
{ 0x000214e2, 0x0000, 0x0000,{ 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46 } };

static const GUID SH32_SID_STopLevelBrowser =
{ 0x4c96be40, 0x915c, 0x11cf,{ 0x99, 0xd3, 0x00, 0xaa, 0x00, 0x4a, 0xe8, 0x37 } };

static const GUID SH32_IID_IShellWindows =
{ 0x85cb6900, 0x4d95, 0x11cf,{ 0x96, 0x0c, 0x00, 0x80, 0xc7, 0xf4, 0xee, 0x85 } };

static const GUID SH32_IID_IShellFolderViewDual =
{ 0xE7A1AF80, 0x4D96, 0x11CF,{ 0x96, 0x0C, 0x00, 0x80, 0xC7, 0xF4, 0xEE, 0x85 } };

// {D8F015C0-C278-11CE-A49E-444553540000}  IShellDispatch
static const GUID SH32_IID_IShellDispatch =
{ 0xD8F015C0, 0xC278, 0x11CE,{ 0xA4, 0x9E, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00 } };

static const GUID SH32_IID_IShellDispatch2 =
{ 0xA4C6892C, 0x3BA9, 0x11D2,{ 0x9D, 0xEA, 0x00, 0xC0, 0x4F, 0xB1, 0x61, 0x62 } };


//---------------------------------------------------------------------------


typedef BOOL (*P_ShellExecuteEx)(
    void *lpExecInfo);

typedef BOOL (*P_Shell_NotifyIconW)(
    DWORD dwMessage, PNOTIFYICONDATAW lpData);

//typedef HRESULT (*P_SHGetFolderPathW)(
//    HWND hwnd, int csidl, HANDLE hToken, DWORD dwFlags, LPWSTR pszPath);

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

//static P_SHGetFolderPathW       __sys_SHGetFolderPathW              = NULL;

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

static const WCHAR *SH32_UseShellNotifyIconProxy = L"UseShellNotifyIconProxy";


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
// SH32_BreakoutDocument
//---------------------------------------------------------------------------


_FX BOOL SH32_BreakoutDocument(const WCHAR* path, ULONG len)
{
    if (SbieDll_CheckPatternInList(path, len, NULL, L"BreakoutDocument")) {

        NTSTATUS status;
        static WCHAR* _QueueName = NULL;

        if (!_QueueName) {
            _QueueName = Dll_Alloc(32 * sizeof(WCHAR));
            Sbie_snwprintf(_QueueName, 32, L"*USERPROXY_%08X", Dll_SessionId);
        }

        ULONG path_len = (len + 1) * sizeof(WCHAR);
        ULONG req_len = sizeof(USER_SHELL_EXEC_REQ) + path_len;
        ULONG path_pos = sizeof(USER_SHELL_EXEC_REQ);

        USER_SHELL_EXEC_REQ* req = (USER_SHELL_EXEC_REQ*)Dll_AllocTemp(req_len);

        WCHAR* path_buff = ((UCHAR*)req) + path_pos;
        memcpy(path_buff, path, path_len);

        req->msgid = USER_SHELL_EXEC;

        req->FileNameOffset = path_pos;

        ULONG* rpl = SbieDll_CallProxySvr(_QueueName, req, req_len, sizeof(*rpl), 100);
        if (!rpl)
            status = STATUS_INTERNAL_ERROR;
        else {
            status = rpl[0];

            Dll_Free(rpl);
        }

        Dll_Free(req);

        return TRUE;
    }

    return FALSE;
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
    // check if the request is to open a file located in a break out folder
    //

    if (lpExecInfo->lpFile) {

        if (SH32_BreakoutDocument(lpExecInfo->lpFile, wcslen(lpExecInfo->lpFile)))
            return TRUE;
    }

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

        if (path && SbieDll_IsDirectory(path))
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
// SH32_wcsnlen
//---------------------------------------------------------------------------


static ULONG SH32_wcsnlen(const WCHAR *src, ULONG maxChars)
{
    ULONG len = 0;

    if (!src)
        return 0;

    while (len < maxChars && src[len] != L'\0')
        ++len;

    return len;
}


//---------------------------------------------------------------------------
// SH32_NotifyIconMaxChars
//---------------------------------------------------------------------------


static ULONG SH32_NotifyIconMaxChars(
    DWORD cbSize, ULONG fieldOffsetBytes, ULONG fieldCapacityChars)
{
    ULONG chars;

    if (cbSize <= fieldOffsetBytes)
        return 0;

    chars = (cbSize - fieldOffsetBytes) / sizeof(WCHAR);
    if (chars > fieldCapacityChars)
        chars = fieldCapacityChars;

    return chars;
}


//---------------------------------------------------------------------------
// SH32_Shell_NotifyIcon_ProxyCall
//---------------------------------------------------------------------------


static BOOLEAN SH32_Shell_NotifyIcon_ProxyCall(
    DWORD dwMessage, PNOTIFYICONDATAW lpData, BOOL *ret)
{
    if (! lpData)
        return FALSE;

    GUI_SHELL_NOTIFY_ICON_REQ *req =
        (GUI_SHELL_NOTIFY_ICON_REQ *)Dll_AllocTemp(sizeof(GUI_SHELL_NOTIFY_ICON_REQ));
    if (! req)
        return FALSE;

    memzero(req, sizeof(*req));
    req->msgid     = GUI_SHELL_NOTIFY_ICON;
    req->dwMessage = dwMessage;

    req->cbSize           = lpData->cbSize;
    req->hWnd             = (ULONG)(ULONG_PTR)lpData->hWnd;
    req->uID              = lpData->uID;
    req->uFlags           = lpData->uFlags;
    req->uCallbackMessage = lpData->uCallbackMessage;
    req->hIcon            = (ULONG)(ULONG_PTR)lpData->hIcon;

    ULONG tipChars = SH32_NotifyIconMaxChars(
        lpData->cbSize,
        FIELD_OFFSET(NOTIFYICONDATAW, szTip),
        ARRAYSIZE(req->szTip));

    if (tipChars) {
        wmemcpy(req->szTip, lpData->szTip, tipChars);
        req->szTip[tipChars - 1] = L'\0';
    }

    if (lpData->cbSize >= (DWORD)NOTIFYICONDATAW_V2_SIZE) {

        req->dwState     = lpData->dwState;
        req->dwStateMask = lpData->dwStateMask;

        wmemcpy(req->szInfo, lpData->szInfo, ARRAYSIZE(req->szInfo) - 1);
        req->szInfo[ARRAYSIZE(req->szInfo) - 1] = L'\0';

        req->uVersion = lpData->uVersion;

        wmemcpy(req->szInfoTitle, lpData->szInfoTitle, ARRAYSIZE(req->szInfoTitle) - 1);
        req->szInfoTitle[ARRAYSIZE(req->szInfoTitle) - 1] = L'\0';

        req->dwInfoFlags = lpData->dwInfoFlags;
    }

#if (NTDDI_VERSION >= NTDDI_VISTA)
    if (lpData->cbSize >= (DWORD)NOTIFYICONDATAW_V3_SIZE) {
        req->guidItem = lpData->guidItem;
    }

    if (lpData->cbSize >= (DWORD)(NOTIFYICONDATAW_V3_SIZE + sizeof(HICON))) {
        req->hBalloonIcon = (ULONG)(ULONG_PTR)lpData->hBalloonIcon;
    }
#else
    if (lpData->cbSize >= (DWORD)(NOTIFYICONDATAW_V2_SIZE + sizeof(GUID))) {
        req->guidItem = lpData->guidItem;
    }
#endif

    GUI_SHELL_NOTIFY_ICON_RPL *rpl =
        Gui_CallProxy(req, sizeof(*req), sizeof(*rpl));

    Dll_Free(req);

    if (! rpl)
        return FALSE;

    if (ret)
        *ret = rpl->result ? TRUE : FALSE;

    SetLastError(rpl->error);
    Dll_Free(rpl);
    return TRUE;
}


//---------------------------------------------------------------------------
// SH32_Shell_NotifyIconW
//---------------------------------------------------------------------------


_FX BOOL SH32_Shell_NotifyIconW(
    DWORD dwMessage, PNOTIFYICONDATAW lpData)
{
    BOOL ret = FALSE;
    HICON icon = NULL;

    if (dwMessage == NIM_ADD || dwMessage == NIM_MODIFY)
    {
        if (!Gui_DisableTitle && lpData)
        {
            ULONG tipChars = SH32_NotifyIconMaxChars(
                lpData->cbSize,
                FIELD_OFFSET(NOTIFYICONDATAW, szTip),
                ARRAYSIZE(lpData->szTip));

            if (tipChars > 1)
            {
                ULONG tipMaxLen = tipChars - 1;
                ULONG len = SH32_wcsnlen(lpData->szTip, tipMaxLen);
                lpData->szTip[len] = L'\0';

                if (Gui_BoxNameTitleLen != 0 && (len + Gui_BoxNameTitleLen + 2) <= tipMaxLen)
                {
                    wmemmove(lpData->szTip + Gui_BoxNameTitleLen + 2, lpData->szTip, len + 1);
                    wmemcpy(lpData->szTip, Gui_BoxNameTitleW, Gui_BoxNameTitleLen);
                    wmemcpy(lpData->szTip + Gui_BoxNameTitleLen, L"\r\n", 2);
                }
                else
                {
                    if (tipMaxLen >= 8) {

                        if (len + 8 > tipMaxLen) {
                            if (tipMaxLen > 11) {
                                lpData->szTip[tipMaxLen - 8 - 3] = L'\0';
                                wcscat(lpData->szTip, L"...");
                                len = tipMaxLen - 8;
                            }
                            else {
                                lpData->szTip[0] = L'\0';
                                len = 0;
                            }
                        }

                        wmemmove(lpData->szTip + 4, lpData->szTip, len + 1);
                        wmemcpy(lpData->szTip, L"[#] ", 4);
                        wcscat(lpData->szTip, L" [#]");
                    }
                }
            }
        }

        COLORREF color;
        if (lpData && SbieDll_GetBorderColor(NULL, &color, NULL, NULL, NULL))
        {
            HICON newIcon = SH32_BorderToIcon(lpData->hIcon, color);
            if (newIcon) {
                icon = lpData->hIcon;
                lpData->hIcon = newIcon;
            }
        }
    }

    if (Gui_OpenAllWinClasses && Gui_UseProxyService
        && Config_GetSettingsForImageName_bool(SH32_UseShellNotifyIconProxy, TRUE)) {

        //
        // When OpenWinClass=* is set, FindWindowW/SendMessageW hooks are not
        // installed, so Shell_NotifyIconW cannot locate Shell_TrayWnd on the
        // Sandboxie desktop.  Route the call through the GUI proxy, which
        // runs on the real desktop and can invoke Shell_NotifyIconW there.
        //

        if (! SH32_Shell_NotifyIcon_ProxyCall(dwMessage, lpData, &ret))
            ret = __sys_Shell_NotifyIconW(dwMessage, lpData);

    } else {

        ret = __sys_Shell_NotifyIconW(dwMessage, lpData);
    }

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
// SH32_SHGetFolderPathW
//---------------------------------------------------------------------------


//_FX HRESULT SH32_SHGetFolderPathW(
//    HWND hwnd, int csidl, HANDLE hToken, DWORD dwFlags, LPWSTR pszPath)
//{
//    return __sys_SHGetFolderPathW(hwnd, csidl, hToken, dwFlags, pszPath);
//}


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
    //P_SHGetFolderPathW SHGetFolderPathW;
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

    //SHGetFolderPathW = (P_SHGetFolderPathW)
    //    GetProcAddress(module, "SHGetFolderPathW");
    //
    //SBIEDLL_HOOK(SH32_,SHGetFolderPathW);

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
    ULONG version = 0;

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
    if (!NT_SUCCESS(RtlAnsiStringToUnicodeString(&uni, &ansi, TRUE)) || !uni.Buffer)
        ExitProcess(0);

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

    ULONG_PTR *vtbl;
    void *xold;
    void *xnew;

    if (!lpVtbl || !func || index < 0)
        return;

    __try {
        vtbl = *(ULONG_PTR **)lpVtbl;
        if (!vtbl || !vtbl[index])
            return;
        xold = (void *)vtbl[index];
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        return;
    }

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


//---------------------------------------------------------------------------
// IShellWindows support
//---------------------------------------------------------------------------


#define SH32_SWC_DESKTOP            0x8
#define SH32_SWC_EXPLORER           0x0
#define SH32_SWFO_NEEDDISPATCH      0x1
#define SH32_SWFO_INCLUDEPENDING    0x2
#define SH32_E_POINTER              ((HRESULT)0x80004003L)


_FX BOOLEAN SH32_ComGetVtbl(void *pUnknown, ULONG requiredSlot, ULONG_PTR **ppVtbl)
{
    ULONG_PTR *vtbl;

    if (!ppVtbl)
        return FALSE;

    *ppVtbl = NULL;

    if (!pUnknown)
        return FALSE;

    __try {
        vtbl = *(ULONG_PTR **)pUnknown;
        if (!vtbl)
            return FALSE;
        if (!vtbl[requiredSlot])
            return FALSE;
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        return FALSE;
    }

    *ppVtbl = vtbl;
    return TRUE;
}


_FX void SH32_ComRelease(void *pUnknown)
{
    if (pUnknown) {
        ULONG_PTR *vtbl = NULL;
        typedef ULONG (STDMETHODCALLTYPE *P_Release)(void *);
        if (!SH32_ComGetVtbl(pUnknown, 2, &vtbl))
            return;
        ((P_Release)vtbl[2])(pUnknown);
    }
}


_FX void SH32_ComAddRef(void *pUnknown)
{
    if (pUnknown) {
        ULONG_PTR *vtbl = NULL;
        typedef ULONG (STDMETHODCALLTYPE *P_AddRef)(void *);
        if (!SH32_ComGetVtbl(pUnknown, 1, &vtbl))
            return;
        ((P_AddRef)vtbl[1])(pUnknown);
    }
}


//---------------------------------------------------------------------------
// Synthetic fake COM objects for the IShellWindows dispatch chain.
//
// The caller walks the chain:
//   FindWindowSW -> FakeDesktop (IDispatch)
//     ->QI-> FakeServiceProvider (IServiceProvider)
//       ->QueryService(SID_STopLevelBrowser, IShellBrowser) -> FakeBrowserView
//         ->QueryActiveShellView [vtbl 15] -> FakeShellView (IShellView)
//           ->GetItemObject(SVGIO_BACKGROUND, IID_IDispatch) [vtbl 15] -> FakeFolderView
//             ->QI-> IShellFolderViewDual (self)
//               ->get_Application [vtbl 7] -> FakeShellApp (IDispatch)
//                 ->QI-> IShellDispatch2 (self)
//                   ->ShellExecute [vtbl 31] -> SH32_FakeApp_ShellExecute -> SH32_ShellExecuteExW
//
// Identity guarantees inside the synthetic desktop loop:
//   - FakeDesktop.Parent and FakeDesktop.Windows return the same desktop identity.
//   - FakeDesktop.Application is cached per desktop.
//   - That desktop-owned FakeShellApp routes Windows back to the same desktop,
//     so desktop.Application.Windows round-trips to desktop.
//---------------------------------------------------------------------------


// Base object struct shared by the five fake objects that carry their own ref
// count: FakeDesktop, FakeBrowserView, FakeShellView, FakeFolderView, and
// FakeShellApp.  FakeServiceProvider borrows the desktop's ref count and uses
// a distinct layout (SH32_FakeServiceProviderData).
typedef struct _SH32_FakeCOM {
    const ULONG_PTR *vtbl;
    LONG             refCount;
} SH32_FakeCOM;

typedef HRESULT (*SH32_FakeDispPropGetter)(SH32_FakeCOM *pThis, VARIANT *pResult);
typedef HRESULT (*SH32_FakeDispMethod)(SH32_FakeCOM *pThis, DISPPARAMS *pDispParams, VARIANT *pResult);

typedef struct _SH32_FakeDispEntry {
    const WCHAR           *name;
    DISPID                 dispid;
    SH32_FakeDispPropGetter getProp;
    SH32_FakeDispMethod     callMethod;
} SH32_FakeDispEntry;

#define SH32_DISPID_APPLICATION   200
#define SH32_DISPID_PARENT        201
#define SH32_DISPID_WINDOWS       300
#define SH32_DISPID_FOLDER        301
#define SH32_DISPID_SCRIPT        302
#define SH32_DISPID_VIEWOPTIONS   303
#define SH32_DISPID_SHELLEXECUTE  601
#define SH32_DISPID_OPEN          602
#define SH32_DISPID_EXPLORE       603
#define SH32_DISPID_CONTROLPANELITEM 604

static HRESULT WINAPI SH32_FakeDisp_GetTypeInfoCount(void *pThis, UINT *pctinfo);
static HRESULT WINAPI SH32_FakeDisp_GetTypeInfo(void *pThis, UINT iTInfo, LCID lcid, ITypeInfo **ppTInfo);
static HRESULT SH32_FakeDisp_GetIDsOfNamesImpl(
    const SH32_FakeDispEntry *entries, REFIID riid, LPOLESTR *rgszNames, UINT cNames, DISPID *rgDispId);
static HRESULT SH32_FakeDisp_InvokeImpl(
    SH32_FakeCOM *pThis, const SH32_FakeDispEntry *entry,
    REFIID riid, WORD wFlags, DISPPARAMS *pDispParams, VARIANT *pVarResult, UINT *puArgErr);
static HRESULT SH32_FakeDisp_ReturnDispatch(VARIANT *pResult, void *pDispatch);
static HRESULT SH32_FakeDisp_GetApplication(SH32_FakeCOM *pThis, VARIANT *pResult);
static HRESULT SH32_FakeDisp_GetSelf(SH32_FakeCOM *pThis, VARIANT *pResult);
static HRESULT SH32_FakeDisp_ShellExecute(SH32_FakeCOM *pThis, DISPPARAMS *pDispParams, VARIANT *pResult);
static HRESULT SH32_FakeDisp_Open(SH32_FakeCOM *pThis, DISPPARAMS *pDispParams, VARIANT *pResult);
static HRESULT SH32_FakeDisp_Explore(SH32_FakeCOM *pThis, DISPPARAMS *pDispParams, VARIANT *pResult);
static HRESULT SH32_FakeDisp_ControlPanelItem(SH32_FakeCOM *pThis, DISPPARAMS *pDispParams, VARIANT *pResult);
static HRESULT SH32_FakeDisp_GetFolderNotImpl(SH32_FakeCOM *pThis, VARIANT *pResult);
static HRESULT SH32_FakeDisp_GetViewOptions(SH32_FakeCOM *pThis, VARIANT *pResult);
static const VARIANT *SH32_FakeDisp_GetArg(DISPPARAMS *pDispParams, UINT index);
static HRESULT WINAPI SH32_FakeApp_get_Application(SH32_FakeCOM *pThis, IDispatch **ppid);
static HRESULT WINAPI SH32_FakeApp_ShellExecute(SH32_FakeCOM *pThis, BSTR bstrFile,
    VARIANT vArgs, VARIANT vDir, VARIANT vOperation, VARIANT vShow);
static void SH32_FakeDisp_VariantInit(VARIANT *pVariant);
static void SH32_FakeDisp_VariantSetError(VARIANT *pVariant, HRESULT hr);
static void SH32_FakeDisp_VariantSetI4(VARIANT *pVariant, LONG value);
static BOOLEAN SH32_FakeDisp_CopyVariantShallow(const VARIANT *pSrc, VARIANT *pDst);
static BOOLEAN SH32_FakeDisp_CopyOptionalArg(DISPPARAMS *pDispParams, UINT index, VARIANT *pDst);
static const WCHAR *SH32_FakeDisp_GetString(const VARIANT *pVariant);
static void SH32_FakeDisp_TraceName(const WCHAR *objectName, const WCHAR *memberName, HRESULT hr);
static void SH32_FakeDisp_TraceInvoke(const WCHAR *objectName, const WCHAR *memberName, WORD wFlags, HRESULT hr);
static HRESULT SH32_FakeDisp_DoShellExecute(const WCHAR *verb, DISPPARAMS *pDispParams, VARIANT *pResult);
static HRESULT WINAPI SH32_FakeFolderView_get_Parent(SH32_FakeCOM *pThis, IDispatch **ppid);
static HRESULT WINAPI SH32_FakeFolderView_get_Script(SH32_FakeCOM *pThis, IDispatch **ppid);
static HRESULT WINAPI SH32_FakeFolderView_get_ViewOptions(SH32_FakeCOM *pThis, long *plViewOptions);
static HRESULT WINAPI SH32_FakeApp_get_Parent(SH32_FakeCOM *pThis, IDispatch **ppid);
static HRESULT WINAPI SH32_FakeApp_Windows(SH32_FakeCOM *pThis, IDispatch **ppid);
static HRESULT WINAPI SH32_FakeApp_Open(SH32_FakeCOM *pThis, VARIANT vDir);
static HRESULT WINAPI SH32_FakeApp_Explore(SH32_FakeCOM *pThis, VARIANT vDir);
static HRESULT WINAPI SH32_FakeApp_ControlPanelItem(SH32_FakeCOM *pThis, BSTR bstrDir);
static void *SH32_FakeShellApp_CreateForDesktop(void *pDesktop);
static void *SH32_FakeChildCache_Get(void **ppSlot, void *(*createFunc)(void));
static HRESULT SH32_FakeDesktop_GetApplication(SH32_FakeCOM *pThis, VARIANT *pResult);

// Arity-correct __stdcall stubs for unused vtable slots.
// On x86, WINAPI (__stdcall) is callee-cleanup: the compiler emits "ret N"
// where N = (argcount * sizeof(DWORD)), derived from the C function signature.
// A single-arity stub placed in a slot with a different arity would corrupt
// the stack on x86. We cover the required arities (0-10).
static HRESULT WINAPI SH32_FakeCOM_Stub0(void *p) { return E_NOTIMPL; }
static HRESULT WINAPI SH32_FakeCOM_Stub1(void *p, ULONG_PTR a1) { return E_NOTIMPL; }
static HRESULT WINAPI SH32_FakeCOM_Stub2(void *p, ULONG_PTR a1, ULONG_PTR a2) { return E_NOTIMPL; }
static HRESULT WINAPI SH32_FakeCOM_Stub3(void *p, ULONG_PTR a1, ULONG_PTR a2, ULONG_PTR a3) { return E_NOTIMPL; }
static HRESULT WINAPI SH32_FakeCOM_Stub4(void *p, ULONG_PTR a1, ULONG_PTR a2, ULONG_PTR a3, ULONG_PTR a4) { return E_NOTIMPL; }
static HRESULT WINAPI SH32_FakeCOM_Stub5(void *p, ULONG_PTR a1, ULONG_PTR a2, ULONG_PTR a3, ULONG_PTR a4, ULONG_PTR a5) { return E_NOTIMPL; }
static HRESULT WINAPI SH32_FakeCOM_Stub6(void *p, ULONG_PTR a1, ULONG_PTR a2, ULONG_PTR a3, ULONG_PTR a4, ULONG_PTR a5, ULONG_PTR a6) { return E_NOTIMPL; }
static HRESULT WINAPI SH32_FakeCOM_Stub7(void *p, ULONG_PTR a1, ULONG_PTR a2, ULONG_PTR a3, ULONG_PTR a4, ULONG_PTR a5, ULONG_PTR a6, ULONG_PTR a7) { return E_NOTIMPL; }
static HRESULT WINAPI SH32_FakeCOM_Stub8(void *p, ULONG_PTR a1, ULONG_PTR a2, ULONG_PTR a3, ULONG_PTR a4, ULONG_PTR a5, ULONG_PTR a6, ULONG_PTR a7, ULONG_PTR a8) { return E_NOTIMPL; }
static HRESULT WINAPI SH32_FakeCOM_Stub9(void *p, ULONG_PTR a1, ULONG_PTR a2, ULONG_PTR a3, ULONG_PTR a4, ULONG_PTR a5, ULONG_PTR a6, ULONG_PTR a7, ULONG_PTR a8, ULONG_PTR a9) { return E_NOTIMPL; }
static HRESULT WINAPI SH32_FakeCOM_Stub10(void *p, ULONG_PTR a1, ULONG_PTR a2, ULONG_PTR a3, ULONG_PTR a4, ULONG_PTR a5, ULONG_PTR a6, ULONG_PTR a7, ULONG_PTR a8, ULONG_PTR a9, ULONG_PTR a10) { return E_NOTIMPL; }

static HRESULT WINAPI SH32_FakeDisp_GetTypeInfoCount(void *pThis, UINT *pctinfo)
{
    UNREFERENCED_PARAMETER(pThis);

    if (!pctinfo)
        return E_POINTER;

    *pctinfo = 0;
    return S_OK;
}

static HRESULT WINAPI SH32_FakeDisp_GetTypeInfo(void *pThis, UINT iTInfo, LCID lcid, ITypeInfo **ppTInfo)
{
    UNREFERENCED_PARAMETER(pThis);
    UNREFERENCED_PARAMETER(iTInfo);
    UNREFERENCED_PARAMETER(lcid);

    if (!ppTInfo)
        return E_POINTER;

    *ppTInfo = NULL;
    return DISP_E_BADINDEX;
}

static void SH32_FakeDisp_VariantInit(VARIANT *pVariant)
{
    if (pVariant)
        memzero(pVariant, sizeof(VARIANT));
}

static void SH32_FakeDisp_VariantSetError(VARIANT *pVariant, HRESULT hr)
{
    SH32_FakeDisp_VariantInit(pVariant);
    if (pVariant) {
        pVariant->vt = VT_ERROR;
        pVariant->scode = hr;
    }
}

static void SH32_FakeDisp_VariantSetI4(VARIANT *pVariant, LONG value)
{
    SH32_FakeDisp_VariantInit(pVariant);
    if (pVariant) {
        pVariant->vt = VT_I4;
        pVariant->lVal = value;
    }
}

static BOOLEAN SH32_FakeDisp_CopyVariantShallow(const VARIANT *pSrc, VARIANT *pDst)
{
    if (!pSrc || !pDst)
        return FALSE;

    SH32_FakeDisp_VariantInit(pDst);

    if ((pSrc->vt & VT_BYREF) && (pSrc->vt & VT_TYPEMASK) == VT_VARIANT)
        return pSrc->pvarVal ? SH32_FakeDisp_CopyVariantShallow(pSrc->pvarVal, pDst) : FALSE;

    switch (pSrc->vt) {
    case VT_EMPTY:
    case VT_NULL:
    case VT_ERROR:
    case VT_BSTR:
    case VT_I2:
    case VT_I4:
    case VT_I8:
    case VT_UI4:
    case VT_BOOL:
    case VT_DISPATCH:
    case VT_UNKNOWN:
        *pDst = *pSrc;
        return TRUE;
    case (VT_BSTR | VT_BYREF):
        pDst->vt = VT_BSTR;
        pDst->bstrVal = pSrc->pbstrVal ? *pSrc->pbstrVal : NULL;
        return TRUE;
    case (VT_I2 | VT_BYREF):
        pDst->vt = VT_I2;
        pDst->iVal = pSrc->piVal ? *pSrc->piVal : 0;
        return TRUE;
    case (VT_I4 | VT_BYREF):
        pDst->vt = VT_I4;
        pDst->lVal = pSrc->plVal ? *pSrc->plVal : 0;
        return TRUE;
    case (VT_I8 | VT_BYREF):
        pDst->vt = VT_I8;
        pDst->llVal = pSrc->pllVal ? *pSrc->pllVal : 0;
        return TRUE;
    case (VT_UI4 | VT_BYREF):
        pDst->vt = VT_UI4;
        pDst->ulVal = pSrc->pulVal ? *pSrc->pulVal : 0;
        return TRUE;
    case (VT_BOOL | VT_BYREF):
        pDst->vt = VT_BOOL;
        pDst->boolVal = pSrc->pboolVal ? *pSrc->pboolVal : VARIANT_FALSE;
        return TRUE;
    case (VT_DISPATCH | VT_BYREF):
        pDst->vt = VT_DISPATCH;
        pDst->pdispVal = pSrc->ppdispVal ? *pSrc->ppdispVal : NULL;
        return TRUE;
    case (VT_UNKNOWN | VT_BYREF):
        pDst->vt = VT_UNKNOWN;
        pDst->punkVal = pSrc->ppunkVal ? *pSrc->ppunkVal : NULL;
        return TRUE;
    default:
        return FALSE;
    }
}

static BOOLEAN SH32_FakeDisp_CopyOptionalArg(DISPPARAMS *pDispParams, UINT index, VARIANT *pDst)
{
    const VARIANT *pArg = SH32_FakeDisp_GetArg(pDispParams, index);
    if (!pArg)
        return FALSE;
    return SH32_FakeDisp_CopyVariantShallow(pArg, pDst);
}

static const WCHAR *SH32_FakeDisp_GetString(const VARIANT *pVariant)
{
    if (!pVariant)
        return NULL;

    return (pVariant->vt == VT_BSTR && pVariant->bstrVal) ? pVariant->bstrVal : NULL;
}

static void SH32_FakeDisp_TraceName(const WCHAR *objectName, const WCHAR *memberName, HRESULT hr)
{
    WCHAR text[192];

    Sbie_snwprintf(text, 192, L"UseFakeShellDispatch[D0]: %s.GetIDsOfNames(%s) -> %08X",
        objectName ? objectName : L"?",
        memberName ? memberName : L"?",
        (ULONG)hr);
    SbieApi_MonitorPut2(MONITOR_COMCLASS | MONITOR_TRACE, text, FALSE);
}

static void SH32_FakeDisp_TraceInvoke(const WCHAR *objectName, const WCHAR *memberName, WORD wFlags, HRESULT hr)
{
    WCHAR text[192];

    Sbie_snwprintf(text, 192, L"UseFakeShellDispatch[D1]: %s.Invoke(%s,%04X) -> %08X",
        objectName ? objectName : L"?",
        memberName ? memberName : L"?",
        (ULONG)wFlags,
        (ULONG)hr);
    SbieApi_MonitorPut2(MONITOR_COMCLASS | MONITOR_TRACE, text, FALSE);
}

static const SH32_FakeDispEntry *SH32_FakeDisp_FindName(
    const SH32_FakeDispEntry *entries, const WCHAR *name)
{
    const SH32_FakeDispEntry *entry;

    if (!entries || !name)
        return NULL;

    for (entry = entries; entry->name; ++entry) {
        if (_wcsicmp(entry->name, name) == 0)
            return entry;
    }

    return NULL;
}

static const SH32_FakeDispEntry *SH32_FakeDisp_FindEntry(
    const SH32_FakeDispEntry *entries, DISPID dispidMember)
{
    const SH32_FakeDispEntry *entry;

    if (!entries)
        return NULL;

    for (entry = entries; entry->name; ++entry) {
        if (entry->dispid == dispidMember)
            return entry;
    }

    return NULL;
}

static HRESULT SH32_FakeDisp_GetIDsOfNamesImpl(
    const SH32_FakeDispEntry *entries, REFIID riid, LPOLESTR *rgszNames, UINT cNames, DISPID *rgDispId)
{
    UINT index;

    if (!rgDispId)
        return E_POINTER;

    if (!rgszNames || cNames == 0)
        return DISP_E_UNKNOWNNAME;

    if (riid && memcmp(riid, &GUID_NULL, sizeof(GUID)) != 0)
        return DISP_E_UNKNOWNINTERFACE;

    for (index = 0; index < cNames; ++index)
        rgDispId[index] = DISPID_UNKNOWN;

    for (index = 0; index < cNames; ++index) {
        const SH32_FakeDispEntry *entry = SH32_FakeDisp_FindName(entries, rgszNames[index]);
        if (!entry)
            return DISP_E_UNKNOWNNAME;
        rgDispId[index] = entry->dispid;
    }

    return S_OK;
}

static HRESULT SH32_FakeDisp_ReturnDispatch(VARIANT *pResult, void *pDispatch)
{
    if (!pResult) {
        SH32_ComRelease(pDispatch);
        return S_OK;
    }

    SH32_FakeDisp_VariantInit(pResult);
    pResult->vt = VT_DISPATCH;
    pResult->pdispVal = (IDispatch *)pDispatch;
    return S_OK;
}

static HRESULT SH32_FakeDisp_GetApplication(SH32_FakeCOM *pThis, VARIANT *pResult)
{
    IDispatch *pDispatch = NULL;
    HRESULT hr = SH32_FakeApp_get_Application(pThis, &pDispatch);
    if (FAILED(hr))
        return hr;
    return SH32_FakeDisp_ReturnDispatch(pResult, pDispatch);
}

static HRESULT SH32_FakeDisp_GetSelf(SH32_FakeCOM *pThis, VARIANT *pResult)
{
    SH32_ComAddRef(pThis);
    return SH32_FakeDisp_ReturnDispatch(pResult, pThis);
}

static HRESULT SH32_FakeDisp_GetFolderNotImpl(SH32_FakeCOM *pThis, VARIANT *pResult)
{
    UNREFERENCED_PARAMETER(pThis);
    SH32_FakeDisp_VariantSetError(pResult, DISP_E_MEMBERNOTFOUND);
    return DISP_E_MEMBERNOTFOUND;
}

static HRESULT SH32_FakeDisp_GetViewOptions(SH32_FakeCOM *pThis, VARIANT *pResult)
{
    UNREFERENCED_PARAMETER(pThis);
    SH32_FakeDisp_VariantSetI4(pResult, 0);
    return S_OK;
}

static const VARIANT *SH32_FakeDisp_GetArg(DISPPARAMS *pDispParams, UINT index)
{
    if (!pDispParams || index >= pDispParams->cArgs)
        return NULL;

    return &pDispParams->rgvarg[pDispParams->cArgs - 1 - index];
}

static HRESULT SH32_FakeDisp_DoShellExecute(const WCHAR *verb, DISPPARAMS *pDispParams, VARIANT *pResult)
{
    VARIANT vFile = { 0 };
    VARIANT vArgs = { 0 };
    VARIANT vDir = { 0 };
    VARIANT vOperation = { 0 };
    VARIANT vShow = { 0 };
    const WCHAR *fileName;

    if (!pDispParams || pDispParams->cNamedArgs != 0)
        return DISP_E_BADPARAMCOUNT;

    if (pDispParams->cArgs < 1 || pDispParams->cArgs > 5)
        return DISP_E_BADPARAMCOUNT;

    // vFile is already VT_EMPTY from = { 0 }; no VariantInit needed.
    SH32_FakeDisp_VariantSetError(&vArgs, DISP_E_PARAMNOTFOUND);
    SH32_FakeDisp_VariantSetError(&vDir, DISP_E_PARAMNOTFOUND);
    SH32_FakeDisp_VariantSetError(&vOperation, DISP_E_PARAMNOTFOUND);
    SH32_FakeDisp_VariantSetError(&vShow, DISP_E_PARAMNOTFOUND);

    if (!SH32_FakeDisp_CopyOptionalArg(pDispParams, 0, &vFile))
        return DISP_E_BADPARAMCOUNT;

    fileName = SH32_FakeDisp_GetString(&vFile);
    if (!fileName)
        return DISP_E_TYPEMISMATCH;

    SH32_FakeDisp_CopyOptionalArg(pDispParams, 1, &vArgs);
    SH32_FakeDisp_CopyOptionalArg(pDispParams, 2, &vDir);

    if (!verb)
        SH32_FakeDisp_CopyOptionalArg(pDispParams, 3, &vOperation);
    else {
        // verb is a WCHAR string literal, not a heap-allocated BSTR.  Setting
        // vt=VT_BSTR is intentional: SH32_FakeApp_ShellExecute only reads
        // bstrVal as LPCWSTR (sei.lpVerb) and never calls SysStringLen/Free.
        vOperation.vt = VT_BSTR;
        vOperation.bstrVal = (BSTR)verb;
    }

    SH32_FakeDisp_CopyOptionalArg(pDispParams, verb ? 3 : 4, &vShow);

    if (pResult)
        SH32_FakeDisp_VariantInit(pResult);

    return SH32_FakeApp_ShellExecute(NULL, (BSTR)fileName, vArgs, vDir, vOperation, vShow);
}

static HRESULT SH32_FakeDisp_ShellExecute(SH32_FakeCOM *pThis, DISPPARAMS *pDispParams, VARIANT *pResult)
{
    UNREFERENCED_PARAMETER(pThis);
    return SH32_FakeDisp_DoShellExecute(NULL, pDispParams, pResult);
}

static HRESULT SH32_FakeDisp_Open(SH32_FakeCOM *pThis, DISPPARAMS *pDispParams, VARIANT *pResult)
{
    UNREFERENCED_PARAMETER(pThis);
    return SH32_FakeDisp_DoShellExecute(L"open", pDispParams, pResult);
}

static HRESULT SH32_FakeDisp_Explore(SH32_FakeCOM *pThis, DISPPARAMS *pDispParams, VARIANT *pResult)
{
    UNREFERENCED_PARAMETER(pThis);
    return SH32_FakeDisp_DoShellExecute(L"explore", pDispParams, pResult);
}

static HRESULT SH32_FakeDisp_ControlPanelItem(SH32_FakeCOM *pThis, DISPPARAMS *pDispParams, VARIANT *pResult)
{
    UNREFERENCED_PARAMETER(pThis);
    return SH32_FakeDisp_DoShellExecute(L"open", pDispParams, pResult);
}

static HRESULT SH32_FakeDisp_InvokeImpl(
    SH32_FakeCOM *pThis, const SH32_FakeDispEntry *entry,
    REFIID riid, WORD wFlags, DISPPARAMS *pDispParams, VARIANT *pVarResult, UINT *puArgErr)
{
    if (puArgErr)
        *puArgErr = 0;

    if (riid && memcmp(riid, &GUID_NULL, sizeof(GUID)) != 0)
        return DISP_E_UNKNOWNINTERFACE;

    if (!entry)
        return DISP_E_MEMBERNOTFOUND;

    // When both DISPATCH_PROPERTYGET and DISPATCH_METHOD are set (e.g. VBScript
    // late binding), prefer the property getter if one exists, then fall through
    // to the method handler.  Previously this path returned DISP_E_MEMBERNOTFOUND
    // for method-only entries (ShellExecute, Open, Explore, ControlPanelItem)
    // whenever DISPATCH_PROPERTYGET happened to be set alongside DISPATCH_METHOD.
    if ((wFlags & DISPATCH_PROPERTYGET) != 0 && entry->getProp)
        return entry->getProp(pThis, pVarResult);

    if ((wFlags & DISPATCH_METHOD) != 0) {
        if (!entry->callMethod)
            return DISP_E_MEMBERNOTFOUND;
        return entry->callMethod(pThis, pDispParams, pVarResult);
    }

    // DISPATCH_PROPERTYGET with no getter registered, or an unsupported wFlags
    // combination (e.g. DISPATCH_PROPERTYPUT/PUTREF).  No properties are
    // settable on any fake object, so all such cases are MEMBERNOTFOUND.
    return DISP_E_MEMBERNOTFOUND;
}

static ULONG WINAPI SH32_FakeBase_AddRef(SH32_FakeCOM *pThis)
{
    return (ULONG)InterlockedIncrement(&pThis->refCount);
}

static ULONG WINAPI SH32_FakeBase_Release(SH32_FakeCOM *pThis)
{
    LONG r = InterlockedDecrement(&pThis->refCount);
    if (r == 0) Dll_Free(pThis);
    return (ULONG)(r < 0 ? 0 : r);
}

// ---- FakeDesktop (IDispatch face returned by FindWindowSW) ----

// {00000000-0000-0000-C000-000000000046}  IUnknown
static const GUID SH32_IID_IUnknown =
{ 0x00000000, 0x0000, 0x0000,{ 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46 } };
// {00020400-0000-0000-C000-000000000046}  IDispatch
static const GUID SH32_IID_IDispatch =
{ 0x00020400, 0x0000, 0x0000,{ 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46 } };

// Extended struct for FakeDesktop: caches the IServiceProvider tear-off and
// the desktop-owned Application dispatch object so repeated property reads
// preserve COM identity.  The Windows property returns self directly, so no
// separate pWindows slot is needed here.
// vtbl and refCount MUST remain the first two fields to be layout-compatible
// with SH32_FakeCOM and SH32_FakeBase_AddRef.
typedef struct _SH32_FakeDesktopData {
    const ULONG_PTR *vtbl;
    LONG             refCount;
    void            *pSP;   // cached FakeServiceProvider; NULL until first QI
    void            *pApplication;
} SH32_FakeDesktopData;

typedef struct _SH32_FakeServiceProviderData {
    const ULONG_PTR      *vtbl;
    SH32_FakeDesktopData *pDesktop;
} SH32_FakeServiceProviderData;

// FakeFolderView only needs to stabilize its Application child.  Parent is
// self-backed (returns pThis).  Script is NOT self-backed: it delegates to
// SH32_FakeFolderView_GetApplication and preserves identity via the same
// pApplication cache as Application.
typedef struct _SH32_FakeFolderViewData {
    const ULONG_PTR *vtbl;
    LONG             refCount;
    void            *pApplication;
} SH32_FakeFolderViewData;

// When created from FakeDesktop.Application, pDesktopOwner links the shell app
// back to that desktop so Windows returns the same desktop identity.
typedef struct _SH32_FakeShellAppData {
    const ULONG_PTR *vtbl;
    LONG             refCount;
    void            *pDesktopOwner;
    void            *pWindows;
} SH32_FakeShellAppData;

static void *SH32_FakeChildCache_Get(void **ppSlot, void *(*createFunc)(void))
{
    void *pChild = *ppSlot;

    if (!pChild) {
        void *pNew = createFunc();
        if (!pNew)
            return NULL;

        pChild = InterlockedCompareExchangePointer(ppSlot, pNew, NULL);
        if (!pChild)
            pChild = pNew;
        else
            SH32_ComRelease(pNew);
    }

    return pChild;
}

static HRESULT SH32_FakeDesktop_GetApplication(SH32_FakeCOM *pThis, VARIANT *pResult)
{
    SH32_FakeDesktopData *pDesktop = (SH32_FakeDesktopData *)pThis;
    void *pDispatch = pDesktop->pApplication;

    if (!pDispatch) {
        void *pNew = SH32_FakeShellApp_CreateForDesktop(pDesktop);
        if (!pNew)
            return E_OUTOFMEMORY;

        pDispatch = InterlockedCompareExchangePointer(&pDesktop->pApplication, pNew, NULL);
        if (!pDispatch)
            pDispatch = pNew;
        else
            SH32_ComRelease(pNew);
    }

    SH32_ComAddRef(pDispatch);
    return SH32_FakeDisp_ReturnDispatch(pResult, pDispatch);
}

static HRESULT SH32_FakeFolderView_GetApplication(SH32_FakeCOM *pThis, VARIANT *pResult)
{
    SH32_FakeFolderViewData *pFolderView = (SH32_FakeFolderViewData *)pThis;
    void *pDispatch = SH32_FakeChildCache_Get(&pFolderView->pApplication, SH32_FakeShellApp_Create);

    if (!pDispatch)
        return E_OUTOFMEMORY;

    SH32_ComAddRef(pDispatch);
    return SH32_FakeDisp_ReturnDispatch(pResult, pDispatch);
}

static HRESULT SH32_FakeShellApp_GetWindows(SH32_FakeCOM *pThis, VARIANT *pResult)
{
    SH32_FakeShellAppData *pApp = (SH32_FakeShellAppData *)pThis;
    void *pDispatch;

    // Snapshot pDesktopOwner once to avoid a TOCTOU race: SH32_FakeDesktop_Release
    // can null this field from another thread after we pass the null-check.
    pDispatch = pApp->pDesktopOwner;
    if (pDispatch) {
        SH32_ComAddRef(pDispatch);
        return SH32_FakeDisp_ReturnDispatch(pResult, pDispatch);
    }

    pDispatch = SH32_FakeChildCache_Get(&pApp->pWindows, SH32_FakeDesktop_Create);

    if (!pDispatch)
        return E_OUTOFMEMORY;

    SH32_ComAddRef(pDispatch);
    return SH32_FakeDisp_ReturnDispatch(pResult, pDispatch);
}

static HRESULT WINAPI SH32_FakeDesktop_QI(SH32_FakeDesktopData *pThis, REFIID riid, void **ppv)
{
    void *pSP;

    if (!ppv) return E_POINTER;
    if (memcmp(riid, &SH32_IID_IServiceProvider, sizeof(GUID)) == 0) {
        // Cache a tear-off IServiceProvider so repeated QI calls return the
        // same interface pointer while IUnknown identity remains the desktop.
        pSP = pThis->pSP;
        if (!pSP) {
            void *pNew = SH32_FakeServiceProvider_Create(pThis);
            if (!pNew) return E_OUTOFMEMORY;

            pSP = InterlockedCompareExchangePointer(&pThis->pSP, pNew, NULL);
            if (!pSP)
                pSP = pNew;
            else
                Dll_Free(pNew);
        }
        InterlockedIncrement(&pThis->refCount);
        *ppv = pSP;
        return S_OK;
    }
    // Accept IUnknown and IDispatch as self
    if (memcmp(riid, &SH32_IID_IUnknown,  sizeof(GUID)) == 0 ||
        memcmp(riid, &SH32_IID_IDispatch, sizeof(GUID)) == 0) {
        *ppv = pThis;
        InterlockedIncrement(&pThis->refCount);
        return S_OK;
    }
    *ppv = NULL;
    return E_NOINTERFACE;
}

static ULONG WINAPI SH32_FakeDesktop_Release(SH32_FakeDesktopData *pThis)
{
    LONG r = InterlockedDecrement(&pThis->refCount);
    if (r == 0) {
        // The cached service provider is a tear-off that shares the desktop
        // object's refcount, so it can be freed directly here.
        if (pThis->pSP) {
            Dll_Free(pThis->pSP);
            pThis->pSP = NULL;
        }
        if (pThis->pApplication) {
            ((SH32_FakeShellAppData *)pThis->pApplication)->pDesktopOwner = NULL;
            SH32_ComRelease(pThis->pApplication);
            pThis->pApplication = NULL;
        }
        Dll_Free(pThis);
    }
    return (ULONG)(r < 0 ? 0 : r);
}

static const SH32_FakeDispEntry SH32_FakeDesktop_DispEntries[] = {
    { L"Application", SH32_DISPID_APPLICATION, SH32_FakeDesktop_GetApplication, NULL },
    { L"Parent", SH32_DISPID_PARENT, SH32_FakeDisp_GetSelf, NULL },
    // Shell automation expects desktop.Windows to refer to the desktop window set,
    // not to a second synthetic desktop object.
    { L"Windows", SH32_DISPID_WINDOWS, SH32_FakeDisp_GetSelf, NULL },
    { L"ShellExecute", SH32_DISPID_SHELLEXECUTE, NULL, SH32_FakeDisp_ShellExecute },
    { L"Open", SH32_DISPID_OPEN, NULL, SH32_FakeDisp_Open },
    { L"Explore", SH32_DISPID_EXPLORE, NULL, SH32_FakeDisp_Explore },
    { L"ControlPanelItem", SH32_DISPID_CONTROLPANELITEM, NULL, SH32_FakeDisp_ControlPanelItem },
    { NULL, DISPID_UNKNOWN, NULL, NULL },
};

static HRESULT WINAPI SH32_FakeDesktop_GetIDsOfNames(
    SH32_FakeDesktopData *pThis, REFIID riid, LPOLESTR *rgszNames, UINT cNames, LCID lcid, DISPID *rgDispId)
{
    UNREFERENCED_PARAMETER(pThis);
    UNREFERENCED_PARAMETER(lcid);
    {
        HRESULT hr = SH32_FakeDisp_GetIDsOfNamesImpl(SH32_FakeDesktop_DispEntries, riid, rgszNames, cNames, rgDispId);
        SH32_FakeDisp_TraceName(L"FakeDesktop", (rgszNames && cNames) ? rgszNames[0] : NULL, hr);
        return hr;
    }
}

static HRESULT WINAPI SH32_FakeDesktop_Invoke(
    SH32_FakeDesktopData *pThis, DISPID dispidMember, REFIID riid, LCID lcid, WORD wFlags,
    DISPPARAMS *pDispParams, VARIANT *pVarResult, EXCEPINFO *pExcepInfo, UINT *puArgErr)
{
    UNREFERENCED_PARAMETER(lcid);
    UNREFERENCED_PARAMETER(pExcepInfo);
    {
        const SH32_FakeDispEntry *entry = SH32_FakeDisp_FindEntry(SH32_FakeDesktop_DispEntries, dispidMember);
        HRESULT hr = SH32_FakeDisp_InvokeImpl((SH32_FakeCOM *)pThis, entry,
            riid, wFlags, pDispParams, pVarResult, puArgErr);
        SH32_FakeDisp_TraceInvoke(L"FakeDesktop", entry ? entry->name : L"?", wFlags, hr);
        return hr;
    }
}

// IDispatch vtable: 7 entries [0..6]
static const ULONG_PTR SH32_FakeDesktop_Vtbl[] = {
    (ULONG_PTR)SH32_FakeDesktop_QI,      // [0] QueryInterface
    (ULONG_PTR)SH32_FakeBase_AddRef,     // [1] AddRef
    (ULONG_PTR)SH32_FakeDesktop_Release, // [2] Release  (specialized: frees cached pSP)
    (ULONG_PTR)SH32_FakeDisp_GetTypeInfoCount, // [3] GetTypeInfoCount
    (ULONG_PTR)SH32_FakeDisp_GetTypeInfo,      // [4] GetTypeInfo
    (ULONG_PTR)SH32_FakeDesktop_GetIDsOfNames, // [5] GetIDsOfNames
    (ULONG_PTR)SH32_FakeDesktop_Invoke,        // [6] Invoke
};

_FX void *SH32_FakeDesktop_Create(void)
{
    SH32_FakeDesktopData *p = Dll_Alloc(sizeof(SH32_FakeDesktopData));
    if (p) {
        p->vtbl = SH32_FakeDesktop_Vtbl;
        p->refCount = 1;
        p->pSP = NULL;
        p->pApplication = NULL;
    }
    return p;
}

// ---- FakeServiceProvider (IServiceProvider) ----

static ULONG WINAPI SH32_FakeSP_AddRef(SH32_FakeServiceProviderData *pThis)
{
    return (ULONG)InterlockedIncrement(&pThis->pDesktop->refCount);
}

static ULONG WINAPI SH32_FakeSP_Release(SH32_FakeServiceProviderData *pThis)
{
    return SH32_FakeDesktop_Release(pThis->pDesktop);
}

static HRESULT WINAPI SH32_FakeSP_QI(SH32_FakeServiceProviderData *pThis, REFIID riid, void **ppv)
{
    if (!ppv) return E_POINTER;
    if (memcmp(riid, &SH32_IID_IUnknown, sizeof(GUID)) == 0 ||
        memcmp(riid, &SH32_IID_IDispatch, sizeof(GUID)) == 0) {
        *ppv = pThis->pDesktop;
        InterlockedIncrement(&pThis->pDesktop->refCount);
        return S_OK;
    }
    if (memcmp(riid, &SH32_IID_IServiceProvider, sizeof(GUID)) == 0) {
        *ppv = pThis;
        InterlockedIncrement(&pThis->pDesktop->refCount);
        return S_OK;
    }
    *ppv = NULL;
    return E_NOINTERFACE;
}

static HRESULT WINAPI SH32_FakeSP_QueryService(SH32_FakeServiceProviderData *pThis, REFGUID guidSvc, REFIID riid, void **ppv)
{
    UNREFERENCED_PARAMETER(pThis);

    if (!ppv) return E_POINTER;
    *ppv = NULL;
    if (memcmp(guidSvc, &SH32_SID_STopLevelBrowser, sizeof(GUID)) == 0 &&
        memcmp(riid, &SH32_IID_IShellBrowser, sizeof(GUID)) == 0) {
        void *pBrowser = SH32_FakeBrowserView_Create();
        if (!pBrowser) return E_OUTOFMEMORY;
        *ppv = pBrowser;
        return S_OK;
    }
    return E_NOINTERFACE;
}

// IServiceProvider vtable: 4 entries [0..3]
static const ULONG_PTR SH32_FakeSP_Vtbl[] = {
    (ULONG_PTR)SH32_FakeSP_QI,           // [0] QueryInterface
    (ULONG_PTR)SH32_FakeSP_AddRef,        // [1] AddRef
    (ULONG_PTR)SH32_FakeSP_Release,       // [2] Release
    (ULONG_PTR)SH32_FakeSP_QueryService,  // [3] QueryService
};

_FX void *SH32_FakeServiceProvider_Create(void *pDesktop)
{
    SH32_FakeServiceProviderData *p = Dll_Alloc(sizeof(SH32_FakeServiceProviderData));
    if (p) {
        p->vtbl = SH32_FakeSP_Vtbl;
        p->pDesktop = (SH32_FakeDesktopData *)pDesktop;
    }
    return p;
}

// ---- FakeBrowserView (IShellBrowser, QueryActiveShellView at vtbl[15]) ----

static HRESULT WINAPI SH32_FakeBrowser_QI(SH32_FakeCOM *pThis, REFIID riid, void **ppv)
{
    if (!ppv) return E_POINTER;
    if (memcmp(riid, &SH32_IID_IUnknown,     sizeof(GUID)) == 0 ||
        memcmp(riid, &SH32_IID_IShellBrowser, sizeof(GUID)) == 0) {
        *ppv = pThis;
        InterlockedIncrement(&pThis->refCount);
        return S_OK;
    }
    *ppv = NULL;
    return E_NOINTERFACE;
}

static HRESULT WINAPI SH32_FakeBrowser_QueryActiveShellView(SH32_FakeCOM *pThis, void **ppshv)
{
    void *pView;
    if (!ppshv) return E_POINTER;
    *ppshv = NULL;
    pView = SH32_FakeShellView_Create();
    if (!pView) return E_OUTOFMEMORY;
    *ppshv = pView;
    return S_OK;
}

// IShellBrowser vtable: 18 entries [0..17]
static const ULONG_PTR SH32_FakeBrowser_Vtbl[] = {
    (ULONG_PTR)SH32_FakeBrowser_QI,                   // [0]  QueryInterface
    (ULONG_PTR)SH32_FakeBase_AddRef,                   // [1]  AddRef
    (ULONG_PTR)SH32_FakeBase_Release,                  // [2]  Release
    (ULONG_PTR)SH32_FakeCOM_Stub1,                     // [3]  GetWindow              (HWND*)
    (ULONG_PTR)SH32_FakeCOM_Stub1,                     // [4]  ContextSensitiveHelp   (BOOL)
    (ULONG_PTR)SH32_FakeCOM_Stub2,                     // [5]  InsertMenusSB          (HMENU,LPOLEMENUGROUPWIDTHS)
    (ULONG_PTR)SH32_FakeCOM_Stub3,                     // [6]  SetMenuSB              (HMENU,HOLEMENU,HWND)
    (ULONG_PTR)SH32_FakeCOM_Stub1,                     // [7]  RemoveMenusSB          (HMENU)
    (ULONG_PTR)SH32_FakeCOM_Stub1,                     // [8]  SetStatusTextSB        (LPCWSTR)
    (ULONG_PTR)SH32_FakeCOM_Stub1,                     // [9]  EnableModelessSB       (BOOL)
    (ULONG_PTR)SH32_FakeCOM_Stub2,                     // [10] TranslateAcceleratorSB (MSG*,WORD)
    (ULONG_PTR)SH32_FakeCOM_Stub2,                     // [11] BrowseObject           (PCUIDLIST_RELATIVE,UINT)
    (ULONG_PTR)SH32_FakeCOM_Stub2,                     // [12] GetViewStateStream     (DWORD,IStream**)
    (ULONG_PTR)SH32_FakeCOM_Stub2,                     // [13] GetControlWindow       (UINT,HWND*)
    (ULONG_PTR)SH32_FakeCOM_Stub5,                     // [14] SendControlMsg         (UINT,UINT,WPARAM,LPARAM,LRESULT*)
    (ULONG_PTR)SH32_FakeBrowser_QueryActiveShellView,  // [15] QueryActiveShellView
    (ULONG_PTR)SH32_FakeCOM_Stub1,                     // [16] OnViewWindowActive     (IShellView*)
    (ULONG_PTR)SH32_FakeCOM_Stub3,                     // [17] SetToolbarItems        (LPTBBUTTONSB,UINT,UINT)
};

_FX void *SH32_FakeBrowserView_Create(void)
{
    SH32_FakeCOM *p = Dll_Alloc(sizeof(SH32_FakeCOM));
    if (p) { p->vtbl = SH32_FakeBrowser_Vtbl; p->refCount = 1; }
    return p;
}

// ---- FakeShellView (IShellView, GetItemObject at vtbl[15]) ----

// {000214E3-0000-0000-C000-000000000046}  IShellView
static const GUID SH32_IID_IShellView =
{ 0x000214E3, 0x0000, 0x0000,{ 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46 } };

static HRESULT WINAPI SH32_FakeView_QI(SH32_FakeCOM *pThis, REFIID riid, void **ppv)
{
    if (!ppv) return E_POINTER;
    if (memcmp(riid, &SH32_IID_IUnknown,  sizeof(GUID)) == 0 ||
        memcmp(riid, &SH32_IID_IShellView, sizeof(GUID)) == 0) {
        *ppv = pThis;
        InterlockedIncrement(&pThis->refCount);
        return S_OK;
    }
    *ppv = NULL;
    return E_NOINTERFACE;
}

static HRESULT WINAPI SH32_FakeView_GetItemObject(SH32_FakeCOM *pThis, UINT uItem, REFIID riid, void **ppv)
{
    void *pApp;
    if (!ppv) return E_POINTER;
    *ppv = NULL;

    if (uItem != SVGIO_BACKGROUND)
        return E_NOINTERFACE;

    if (memcmp(riid, &SH32_IID_IDispatch, sizeof(GUID)) != 0 &&
        memcmp(riid, &SH32_IID_IUnknown,  sizeof(GUID)) != 0)
        return E_NOINTERFACE;

    pApp = SH32_FakeFolderView_Create();
    if (!pApp) return E_OUTOFMEMORY;
    *ppv = pApp;
    return S_OK;
}

// IShellView vtable: 16 entries [0..15]
static const ULONG_PTR SH32_FakeView_Vtbl[] = {
    (ULONG_PTR)SH32_FakeView_QI,              // [0]  QueryInterface
    (ULONG_PTR)SH32_FakeBase_AddRef,          // [1]  AddRef
    (ULONG_PTR)SH32_FakeBase_Release,         // [2]  Release
    (ULONG_PTR)SH32_FakeCOM_Stub1,            // [3]  GetWindow            (HWND*)
    (ULONG_PTR)SH32_FakeCOM_Stub1,            // [4]  ContextSensitiveHelp (BOOL)
    (ULONG_PTR)SH32_FakeCOM_Stub1,            // [5]  TranslateAccelerator (MSG*)
    (ULONG_PTR)SH32_FakeCOM_Stub1,            // [6]  EnableModeless       (BOOL)
    (ULONG_PTR)SH32_FakeCOM_Stub1,            // [7]  UIActivate           (UINT)
    (ULONG_PTR)SH32_FakeCOM_Stub0,            // [8]  Refresh              ()
    (ULONG_PTR)SH32_FakeCOM_Stub5,            // [9]  CreateViewWindow     (IShellView*,LPCFOLDERSETTINGS,IShellBrowser*,RECT*,HWND*)
    (ULONG_PTR)SH32_FakeCOM_Stub0,            // [10] DestroyViewWindow    ()
    (ULONG_PTR)SH32_FakeCOM_Stub1,            // [11] GetCurrentInfo       (LPFOLDERSETTINGS)
    (ULONG_PTR)SH32_FakeCOM_Stub3,            // [12] AddPropertySheetPages(DWORD,LPFNADDPROPSHEETPAGE,LPARAM)
    (ULONG_PTR)SH32_FakeCOM_Stub0,            // [13] SaveViewState        ()
    (ULONG_PTR)SH32_FakeCOM_Stub2,            // [14] SelectItem           (LPCITEMIDLIST,SVSIF)
    (ULONG_PTR)SH32_FakeView_GetItemObject,   // [15] GetItemObject
};

_FX void *SH32_FakeShellView_Create(void)
{
    SH32_FakeCOM *p = Dll_Alloc(sizeof(SH32_FakeCOM));
    if (p) { p->vtbl = SH32_FakeView_Vtbl; p->refCount = 1; }
    return p;
}

// ---- FakeFolderView (IShellFolderViewDual) ----
//
// vtbl[7]  = get_Application

static HRESULT WINAPI SH32_FakeFolderView_QI(SH32_FakeFolderViewData *pThis, REFIID riid, void **ppv)
{
    if (!ppv) return E_POINTER;
    if (memcmp(riid, &SH32_IID_IUnknown,           sizeof(GUID)) == 0 ||
        memcmp(riid, &SH32_IID_IDispatch,          sizeof(GUID)) == 0 ||
        memcmp(riid, &SH32_IID_IShellFolderViewDual, sizeof(GUID)) == 0) {
        *ppv = pThis;
        InterlockedIncrement(&pThis->refCount);
        return S_OK;
    }
    *ppv = NULL;
    return E_NOINTERFACE;
}

static HRESULT WINAPI SH32_FakeFolderView_get_Application(SH32_FakeCOM *pThis, IDispatch **ppid)
{
    SH32_FakeFolderViewData *pFolderView = (SH32_FakeFolderViewData *)pThis;
    void *pDispatch;

    if (!ppid) return E_POINTER;

    pDispatch = SH32_FakeChildCache_Get(&pFolderView->pApplication, SH32_FakeShellApp_Create);
    if (!pDispatch) {
        *ppid = NULL;
        return E_OUTOFMEMORY;
    }

    SH32_ComAddRef(pDispatch);
    *ppid = (IDispatch *)pDispatch;
    return S_OK;
}

static ULONG WINAPI SH32_FakeFolderView_Release(SH32_FakeFolderViewData *pThis)
{
    LONG r = InterlockedDecrement(&pThis->refCount);
    if (r == 0) {
        if (pThis->pApplication) {
            SH32_ComRelease(pThis->pApplication);
            pThis->pApplication = NULL;
        }
        Dll_Free(pThis);
    }
    return (ULONG)(r < 0 ? 0 : r);
}

static HRESULT WINAPI SH32_FakeFolderView_get_Parent(SH32_FakeCOM *pThis, IDispatch **ppid)
{
    // This synthetic implementation returns self for get_Parent (we have no
    // real parent shell object to surface).  SH32_FakeApp_get_Application is
    // reused because it simply returns pThis as IDispatch with an AddRef,
    // which is exactly the "return self" semantic needed here.
    return SH32_FakeApp_get_Application(pThis, ppid);
}

static HRESULT WINAPI SH32_FakeFolderView_get_Script(SH32_FakeCOM *pThis, IDispatch **ppid)
{
    // Script returns the same cached FakeShellApp as Application, matching real IShellFolderViewDual.
    return SH32_FakeFolderView_get_Application(pThis, ppid);
}

static HRESULT WINAPI SH32_FakeFolderView_get_ViewOptions(SH32_FakeCOM *pThis, long *plViewOptions)
{
    UNREFERENCED_PARAMETER(pThis);

    if (!plViewOptions)
        return E_POINTER;

    *plViewOptions = 0;
    return S_OK;
}

static const SH32_FakeDispEntry SH32_FakeFolderView_DispEntries[] = {
    { L"Application", SH32_DISPID_APPLICATION, SH32_FakeFolderView_GetApplication, NULL },
    { L"Parent", SH32_DISPID_PARENT, SH32_FakeDisp_GetSelf, NULL },
    { L"Folder", SH32_DISPID_FOLDER, SH32_FakeDisp_GetFolderNotImpl, NULL },
    // Script returns the same cached FakeShellApp as Application (matching real IShellFolderViewDual).
    { L"Script", SH32_DISPID_SCRIPT, SH32_FakeFolderView_GetApplication, NULL },
    { L"ViewOptions", SH32_DISPID_VIEWOPTIONS, SH32_FakeDisp_GetViewOptions, NULL },
    { NULL, DISPID_UNKNOWN, NULL, NULL },
};

static HRESULT WINAPI SH32_FakeFolderView_GetIDsOfNames(
    SH32_FakeCOM *pThis, REFIID riid, LPOLESTR *rgszNames, UINT cNames, LCID lcid, DISPID *rgDispId)
{
    UNREFERENCED_PARAMETER(pThis);
    UNREFERENCED_PARAMETER(lcid);
    {
        HRESULT hr = SH32_FakeDisp_GetIDsOfNamesImpl(SH32_FakeFolderView_DispEntries, riid, rgszNames, cNames, rgDispId);
        SH32_FakeDisp_TraceName(L"FakeFolderView", (rgszNames && cNames) ? rgszNames[0] : NULL, hr);
        return hr;
    }
}

static HRESULT WINAPI SH32_FakeFolderView_Invoke(
    SH32_FakeCOM *pThis, DISPID dispidMember, REFIID riid, LCID lcid, WORD wFlags,
    DISPPARAMS *pDispParams, VARIANT *pVarResult, EXCEPINFO *pExcepInfo, UINT *puArgErr)
{
    UNREFERENCED_PARAMETER(lcid);
    UNREFERENCED_PARAMETER(pExcepInfo);
    {
        const SH32_FakeDispEntry *entry = SH32_FakeDisp_FindEntry(SH32_FakeFolderView_DispEntries, dispidMember);
        HRESULT hr = SH32_FakeDisp_InvokeImpl(pThis, entry,
            riid, wFlags, pDispParams, pVarResult, puArgErr);
        SH32_FakeDisp_TraceInvoke(L"FakeFolderView", entry ? entry->name : L"?", wFlags, hr);
        return hr;
    }
}

// IShellFolderViewDual vtable: 16 entries [0..15]
// Stub arity note:
//   PopupItemMenu(FolderItem*,VARIANT,VARIANT,BSTR*) = 1+4+4+1 = 10 slots -> Stub10
static const ULONG_PTR SH32_FakeFolderView_Vtbl[] = {
    (ULONG_PTR)SH32_FakeFolderView_QI,              // [0]  QueryInterface
    (ULONG_PTR)SH32_FakeBase_AddRef,                // [1]  AddRef
    (ULONG_PTR)SH32_FakeFolderView_Release,         // [2]  Release
    (ULONG_PTR)SH32_FakeDisp_GetTypeInfoCount,      // [3]  GetTypeInfoCount
    (ULONG_PTR)SH32_FakeDisp_GetTypeInfo,           // [4]  GetTypeInfo
    (ULONG_PTR)SH32_FakeFolderView_GetIDsOfNames,   // [5]  GetIDsOfNames
    (ULONG_PTR)SH32_FakeFolderView_Invoke,          // [6]  Invoke
    (ULONG_PTR)SH32_FakeFolderView_get_Application, // [7]  get_Application
    (ULONG_PTR)SH32_FakeFolderView_get_Parent,      // [8]  get_Parent
    (ULONG_PTR)SH32_FakeCOM_Stub1,                  // [9]  get_Folder            (Folder**)
    (ULONG_PTR)SH32_FakeCOM_Stub1,                  // [10] SelectedItems         (FolderItems**)
    (ULONG_PTR)SH32_FakeCOM_Stub1,                  // [11] get_FocusedItem       (FolderItem**)
    (ULONG_PTR)SH32_FakeCOM_Stub2,                  // [12] SelectItem            (VARIANT*,int)
    (ULONG_PTR)SH32_FakeCOM_Stub10,                 // [13] PopupItemMenu         (FolderItem*,VARIANT,VARIANT,BSTR*)
    (ULONG_PTR)SH32_FakeFolderView_get_Script,      // [14] get_Script
    (ULONG_PTR)SH32_FakeFolderView_get_ViewOptions, // [15] get_ViewOptions
};

_FX void *SH32_FakeFolderView_Create(void)
{
    SH32_FakeFolderViewData *p = Dll_Alloc(sizeof(SH32_FakeFolderViewData));
    if (p) {
        p->vtbl = SH32_FakeFolderView_Vtbl;
        p->refCount = 1;
        p->pApplication = NULL;
    }
    return p;
}

// ---- FakeShellApp (IShellDispatch2) ----
//
// vtbl[7]  = get_Application
// vtbl[31] = ShellExecute

static HRESULT WINAPI SH32_FakeApp_QI(SH32_FakeShellAppData *pThis, REFIID riid, void **ppv)
{
    if (!ppv) return E_POINTER;
    // Only accept the interfaces whose vtable slots we actually implement.
    // Returning self for IShellDispatch3/4/5/6 would be wrong: those interfaces
    // add vtable slots beyond our 39-entry table, causing out-of-bounds access.
    // IShellDispatch (without suffix) is safe: its 30 slots are a strict subset
    // of the 39 we expose for IShellDispatch2.
    if (memcmp(riid, &SH32_IID_IUnknown,           sizeof(GUID)) == 0 ||
        memcmp(riid, &SH32_IID_IDispatch,          sizeof(GUID)) == 0 ||
        memcmp(riid, &SH32_IID_IShellDispatch,     sizeof(GUID)) == 0 ||
        memcmp(riid, &SH32_IID_IShellDispatch2,    sizeof(GUID)) == 0) {
        *ppv = pThis;
        InterlockedIncrement(&pThis->refCount);
        return S_OK;
    }
    *ppv = NULL;
    return E_NOINTERFACE;
}

static HRESULT WINAPI SH32_FakeApp_get_Application(SH32_FakeCOM *pThis, IDispatch **ppid)
{
    if (!ppid) return E_POINTER;
    *ppid = (IDispatch *)pThis;
    InterlockedIncrement(&pThis->refCount);
    return S_OK;
}

static HRESULT WINAPI SH32_FakeApp_get_Parent(SH32_FakeCOM *pThis, IDispatch **ppid)
{
    return SH32_FakeApp_get_Application(pThis, ppid);
}

static HRESULT WINAPI SH32_FakeApp_Windows(SH32_FakeCOM *pThis, IDispatch **ppid)
{
    SH32_FakeShellAppData *pApp = (SH32_FakeShellAppData *)pThis;
    void *pDispatch;

    if (!ppid)
        return E_POINTER;

    // Snapshot pDesktopOwner once to avoid a TOCTOU race: SH32_FakeDesktop_Release
    // can null this field from another thread after we pass the null-check.
    pDispatch = pApp->pDesktopOwner;
    if (pDispatch) {
        SH32_ComAddRef(pDispatch);
        *ppid = (IDispatch *)pDispatch;
        return S_OK;
    }

    pDispatch = SH32_FakeChildCache_Get(&pApp->pWindows, SH32_FakeDesktop_Create);
    if (!pDispatch) {
        *ppid = NULL;
        return E_OUTOFMEMORY;
    }

    SH32_ComAddRef(pDispatch);
    *ppid = (IDispatch *)pDispatch;
    return S_OK;
}

static ULONG WINAPI SH32_FakeApp_Release(SH32_FakeShellAppData *pThis)
{
    LONG r = InterlockedDecrement(&pThis->refCount);
    if (r == 0) {
        if (pThis->pWindows) {
            SH32_ComRelease(pThis->pWindows);
            pThis->pWindows = NULL;
        }
        Dll_Free(pThis);
    }
    return (ULONG)(r < 0 ? 0 : r);
}

static HRESULT WINAPI SH32_FakeApp_Open(SH32_FakeCOM *pThis, VARIANT vDir)
{
    DISPPARAMS dispParams = { 0 };

    UNREFERENCED_PARAMETER(pThis);

    dispParams.rgvarg = &vDir;
    dispParams.cArgs = 1;
    return SH32_FakeDisp_DoShellExecute(L"open", &dispParams, NULL);
}

static HRESULT WINAPI SH32_FakeApp_Explore(SH32_FakeCOM *pThis, VARIANT vDir)
{
    DISPPARAMS dispParams = { 0 };

    UNREFERENCED_PARAMETER(pThis);

    dispParams.rgvarg = &vDir;
    dispParams.cArgs = 1;
    return SH32_FakeDisp_DoShellExecute(L"explore", &dispParams, NULL);
}

static HRESULT WINAPI SH32_FakeApp_ControlPanelItem(SH32_FakeCOM *pThis, BSTR bstrDir)
{
    VARIANT vDir = { 0 };     // already VT_EMPTY; no VariantInit needed.
    DISPPARAMS dispParams = { 0 };

    UNREFERENCED_PARAMETER(pThis);

    vDir.vt = VT_BSTR;
    vDir.bstrVal = bstrDir;

    dispParams.rgvarg = &vDir;
    dispParams.cArgs = 1;
    return SH32_FakeDisp_DoShellExecute(L"open", &dispParams, NULL);
}

static HRESULT WINAPI SH32_FakeApp_ShellExecute(SH32_FakeCOM *pThis, BSTR bstrFile,
    VARIANT vArgs,
    VARIANT vDir,
    VARIANT vOperation,
    VARIANT vShow)
{
    SHELLEXECUTEINFOW sei;
    BOOL ok;
    DWORD err;
    UNREFERENCED_PARAMETER(pThis);
    memzero(&sei, sizeof(sei));
    sei.cbSize  = sizeof(sei);
    sei.fMask   = SEE_MASK_FLAG_NO_UI;
    sei.lpFile  = bstrFile;
    if (vArgs.vt      == VT_BSTR && vArgs.bstrVal)      sei.lpParameters = vArgs.bstrVal;
    if (vDir.vt       == VT_BSTR && vDir.bstrVal)       sei.lpDirectory  = vDir.bstrVal;
    if (vOperation.vt == VT_BSTR && vOperation.bstrVal) sei.lpVerb       = vOperation.bstrVal;
    sei.nShow = (vShow.vt == VT_I4)  ? vShow.lVal         :
                (vShow.vt == VT_I2)  ? (int)vShow.iVal    :
                (vShow.vt == VT_I8)  ? (int)vShow.llVal   :
                (vShow.vt == VT_UI4) ? (int)vShow.ulVal   : SW_SHOWNORMAL;
    ok = SH32_ShellExecuteExW(&sei);
    if (ok)
        return S_OK;

    err = GetLastError();
    if (err)
        return HRESULT_FROM_WIN32(err);

    return E_FAIL;
}

static const SH32_FakeDispEntry SH32_FakeApp_DispEntries[] = {
    { L"Application", SH32_DISPID_APPLICATION, SH32_FakeDisp_GetApplication, NULL },
    { L"Parent", SH32_DISPID_PARENT, SH32_FakeDisp_GetSelf, NULL },
    // For desktop-owned apps this returns the owning desktop; standalone fake
    // apps still synthesize and cache their own Windows object.
    { L"Windows", SH32_DISPID_WINDOWS, SH32_FakeShellApp_GetWindows, NULL },
    { L"ShellExecute", SH32_DISPID_SHELLEXECUTE, NULL, SH32_FakeDisp_ShellExecute },
    { L"Open", SH32_DISPID_OPEN, NULL, SH32_FakeDisp_Open },
    { L"Explore", SH32_DISPID_EXPLORE, NULL, SH32_FakeDisp_Explore },
    { L"ControlPanelItem", SH32_DISPID_CONTROLPANELITEM, NULL, SH32_FakeDisp_ControlPanelItem },
    { NULL, DISPID_UNKNOWN, NULL, NULL },
};

static HRESULT WINAPI SH32_FakeApp_GetIDsOfNames(
    SH32_FakeCOM *pThis, REFIID riid, LPOLESTR *rgszNames, UINT cNames, LCID lcid, DISPID *rgDispId)
{
    UNREFERENCED_PARAMETER(pThis);
    UNREFERENCED_PARAMETER(lcid);
    {
        HRESULT hr = SH32_FakeDisp_GetIDsOfNamesImpl(SH32_FakeApp_DispEntries, riid, rgszNames, cNames, rgDispId);
        SH32_FakeDisp_TraceName(L"FakeShellApp", (rgszNames && cNames) ? rgszNames[0] : NULL, hr);
        return hr;
    }
}

static HRESULT WINAPI SH32_FakeApp_Invoke(
    SH32_FakeCOM *pThis, DISPID dispidMember, REFIID riid, LCID lcid, WORD wFlags,
    DISPPARAMS *pDispParams, VARIANT *pVarResult, EXCEPINFO *pExcepInfo, UINT *puArgErr)
{
    UNREFERENCED_PARAMETER(lcid);
    UNREFERENCED_PARAMETER(pExcepInfo);
    {
        const SH32_FakeDispEntry *entry = SH32_FakeDisp_FindEntry(SH32_FakeApp_DispEntries, dispidMember);
        HRESULT hr = SH32_FakeDisp_InvokeImpl(pThis, entry,
            riid, wFlags, pDispParams, pVarResult, puArgErr);
        SH32_FakeDisp_TraceInvoke(L"FakeShellApp", entry ? entry->name : L"?", wFlags, hr);
        return hr;
    }
}

// IShellDispatch2 vtable: 39 entries [0..38]
// Stub arity notes:
//   VARIANT passed by value = 16 bytes on x86 = 4 DWORD slots.
//   NameSpace(VARIANT,ptr)            = 4+1     = 5 slots -> Stub5
//   BrowseForFolder(long,BSTR,long,VARIANT,ptr) = 1+1+1+4+1 = 8 slots -> Stub8
//   ServiceStart/Stop(BSTR,VARIANT,VARIANT*)    = 1+4+1   = 6 slots -> Stub6
//   ShowBrowserBar(BSTR,VARIANT,VARIANT*)       = 1+4+1   = 6 slots -> Stub6
//   ShellExecute(BSTR,VARIANT,VARIANT,VARIANT,VARIANT) = 1+4+4+4+4 = 17 slots (live impl, no stub needed)
//   Open/Explore(VARIANT)             = 4       = 4 slots (live impls, no stub needed)
static const ULONG_PTR SH32_FakeApp_Vtbl[] = {
    (ULONG_PTR)SH32_FakeApp_QI,               // [0]  QueryInterface
    (ULONG_PTR)SH32_FakeBase_AddRef,          // [1]  AddRef
    (ULONG_PTR)SH32_FakeApp_Release,          // [2]  Release
    (ULONG_PTR)SH32_FakeDisp_GetTypeInfoCount, // [3]  GetTypeInfoCount
    (ULONG_PTR)SH32_FakeDisp_GetTypeInfo,      // [4]  GetTypeInfo
    (ULONG_PTR)SH32_FakeApp_GetIDsOfNames,     // [5]  GetIDsOfNames
    (ULONG_PTR)SH32_FakeApp_Invoke,            // [6]  Invoke
    (ULONG_PTR)SH32_FakeApp_get_Application,  // [7]  get_Application
    (ULONG_PTR)SH32_FakeApp_get_Parent,       // [8]  get_Parent
    (ULONG_PTR)SH32_FakeCOM_Stub5,            // [9]  NameSpace             (VARIANT,Folder**)
    (ULONG_PTR)SH32_FakeCOM_Stub8,            // [10] BrowseForFolder       (long,BSTR,long,VARIANT,Folder**)
    (ULONG_PTR)SH32_FakeApp_Windows,          // [11] Windows
    (ULONG_PTR)SH32_FakeApp_Open,             // [12] Open
    (ULONG_PTR)SH32_FakeApp_Explore,          // [13] Explore
    (ULONG_PTR)SH32_FakeCOM_Stub0,            // [14] MinimizeAll           ()
    (ULONG_PTR)SH32_FakeCOM_Stub0,            // [15] UndoMinimizeALL       ()
    (ULONG_PTR)SH32_FakeCOM_Stub0,            // [16] FileRun               ()
    (ULONG_PTR)SH32_FakeCOM_Stub0,            // [17] CascadeWindows        ()
    (ULONG_PTR)SH32_FakeCOM_Stub0,            // [18] TileVertically        ()
    (ULONG_PTR)SH32_FakeCOM_Stub0,            // [19] TileHorizontally      ()
    (ULONG_PTR)SH32_FakeCOM_Stub0,            // [20] ShutdownWindows       ()
    (ULONG_PTR)SH32_FakeCOM_Stub0,            // [21] Suspend               ()
    (ULONG_PTR)SH32_FakeCOM_Stub0,            // [22] EjectPC               ()
    (ULONG_PTR)SH32_FakeCOM_Stub0,            // [23] SetTime               ()
    (ULONG_PTR)SH32_FakeCOM_Stub0,            // [24] TrayProperties        ()
    (ULONG_PTR)SH32_FakeCOM_Stub0,            // [25] Help                  ()
    (ULONG_PTR)SH32_FakeCOM_Stub0,            // [26] FindFiles             ()
    (ULONG_PTR)SH32_FakeCOM_Stub0,            // [27] FindComputer          ()
    (ULONG_PTR)SH32_FakeCOM_Stub0,            // [28] RefreshMenu           ()
    (ULONG_PTR)SH32_FakeApp_ControlPanelItem, // [29] ControlPanelItem
    (ULONG_PTR)SH32_FakeCOM_Stub3,            // [30] IsRestricted          (BSTR,BSTR,long*)
    (ULONG_PTR)SH32_FakeApp_ShellExecute,     // [31] ShellExecute
    (ULONG_PTR)SH32_FakeCOM_Stub3,            // [32] FindPrinter           (BSTR,BSTR,BSTR)
    (ULONG_PTR)SH32_FakeCOM_Stub2,            // [33] GetSystemInformation  (BSTR,VARIANT*)
    (ULONG_PTR)SH32_FakeCOM_Stub6,            // [34] ServiceStart          (BSTR,VARIANT,VARIANT*)
    (ULONG_PTR)SH32_FakeCOM_Stub6,            // [35] ServiceStop           (BSTR,VARIANT,VARIANT*)
    (ULONG_PTR)SH32_FakeCOM_Stub2,            // [36] IsServiceRunning      (BSTR,VARIANT*)
    (ULONG_PTR)SH32_FakeCOM_Stub2,            // [37] CanStartStopService   (BSTR,VARIANT*)
    (ULONG_PTR)SH32_FakeCOM_Stub6,            // [38] ShowBrowserBar        (BSTR,VARIANT,VARIANT*)
};

_FX void *SH32_FakeShellApp_Create(void)
{
    return SH32_FakeShellApp_CreateForDesktop(NULL);
}

static void *SH32_FakeShellApp_CreateForDesktop(void *pDesktop)
{
    SH32_FakeShellAppData *p = Dll_Alloc(sizeof(SH32_FakeShellAppData));
    if (p) {
        p->vtbl = SH32_FakeApp_Vtbl;
        p->refCount = 1;
        p->pDesktopOwner = pDesktop;
        p->pWindows = NULL;
    }
    return p;
}


_FX BOOLEAN SH32_IsUsableShellDispatch(IDispatch *pDispatch)
{
    typedef HRESULT (STDMETHODCALLTYPE *P_QueryInterface)(void *, REFIID, void **);
    typedef HRESULT (STDMETHODCALLTYPE *P_QueryService)(void *, REFGUID, REFIID, void **);
    typedef HRESULT (STDMETHODCALLTYPE *P_QueryActiveShellView)(void *, void **);
    typedef HRESULT (STDMETHODCALLTYPE *P_GetItemObject)(void *, UINT, REFIID, void **);
    typedef HRESULT (STDMETHODCALLTYPE *P_get_Application)(void *, IDispatch **);

    IServiceProvider *pServiceProvider = NULL;
    void *pShellBrowser = NULL;
    void *pShellView = NULL;
    IDispatch *pViewDispatch = NULL;
    void *pFolderViewDual = NULL;
    void *pShellDispatch2 = NULL;

    if (! pDispatch)
        return FALSE;

    {
        ULONG_PTR *vtbl = NULL;
        if (!SH32_ComGetVtbl(pDispatch, 0, &vtbl))
            return FALSE;
        HRESULT hr = ((P_QueryInterface)vtbl[0])(
            pDispatch, &SH32_IID_IServiceProvider, (void **)&pServiceProvider);
        if (FAILED(hr) || ! pServiceProvider)
            return FALSE;
    }

    {
        ULONG_PTR *vtbl = NULL;
        if (!SH32_ComGetVtbl(pServiceProvider, 3, &vtbl)) {
            SH32_ComRelease(pServiceProvider);
            pServiceProvider = NULL;
            return FALSE;
        }
        HRESULT hr = ((P_QueryService)vtbl[3])(
            pServiceProvider,
            &SH32_SID_STopLevelBrowser,
            &SH32_IID_IShellBrowser,
            &pShellBrowser);

        SH32_ComRelease(pServiceProvider);
        pServiceProvider = NULL;

        if (FAILED(hr) || ! pShellBrowser)
            return FALSE;
    }

    {
        ULONG_PTR *vtbl = NULL;
        if (!SH32_ComGetVtbl(pShellBrowser, 15, &vtbl)) {
            SH32_ComRelease(pShellBrowser);
            pShellBrowser = NULL;
            return FALSE;
        }
        HRESULT hr = ((P_QueryActiveShellView)vtbl[15])(
            pShellBrowser, &pShellView);

        SH32_ComRelease(pShellBrowser);
        pShellBrowser = NULL;

        if (FAILED(hr) || ! pShellView)
            return FALSE;
    }

    {
        ULONG_PTR *vtbl = NULL;
        if (!SH32_ComGetVtbl(pShellView, 15, &vtbl)) {
            SH32_ComRelease(pShellView);
            pShellView = NULL;
            return FALSE;
        }
        HRESULT hr = ((P_GetItemObject)vtbl[15])(
            pShellView, SVGIO_BACKGROUND, &SH32_IID_IDispatch, (void **)&pViewDispatch);

        SH32_ComRelease(pShellView);
        pShellView = NULL;

        if (FAILED(hr) || ! pViewDispatch)
            return FALSE;
    }

    {
        ULONG_PTR *vtbl = NULL;
        if (!SH32_ComGetVtbl(pViewDispatch, 0, &vtbl)) {
            SH32_ComRelease(pViewDispatch);
            pViewDispatch = NULL;
            return FALSE;
        }
        HRESULT hr = ((P_QueryInterface)vtbl[0])(
            pViewDispatch, &SH32_IID_IShellFolderViewDual, &pFolderViewDual);

        SH32_ComRelease(pViewDispatch);
        pViewDispatch = NULL;

        if (FAILED(hr) || ! pFolderViewDual)
            return FALSE;
    }

    {
        ULONG_PTR *vtbl = NULL;
        IDispatch *pApp = NULL;

        if (!SH32_ComGetVtbl(pFolderViewDual, 7, &vtbl)) {
            SH32_ComRelease(pFolderViewDual);
            pFolderViewDual = NULL;
            return FALSE;
        }

        HRESULT hr = ((P_get_Application)vtbl[7])(pFolderViewDual, &pApp);

        SH32_ComRelease(pFolderViewDual);
        pFolderViewDual = NULL;

        if (FAILED(hr) || ! pApp)
            return FALSE;

        {
            ULONG_PTR *vtbl2 = NULL;
            if (!SH32_ComGetVtbl(pApp, 0, &vtbl2)) {
                SH32_ComRelease(pApp);
                return FALSE;
            }
            hr = ((P_QueryInterface)vtbl2[0])(
                pApp, &SH32_IID_IShellDispatch2, &pShellDispatch2);
            SH32_ComRelease(pApp);
            if (FAILED(hr) || ! pShellDispatch2)
                return FALSE;
        }
    }

    SH32_ComRelease(pShellDispatch2);
    return TRUE;
}


_FX HRESULT SH32_IShellWindows_ItemDispatch(
    void *pShellWindows, LONG index, IDispatch **ppDispatch)
{
    VARIANT vIndex;
    ULONG_PTR *vtbl;

    typedef HRESULT (STDMETHODCALLTYPE *P_Item)(void *, VARIANT, IDispatch **);

    if (! ppDispatch)
        return SH32_E_POINTER;

    *ppDispatch = NULL;

    if (! pShellWindows)
        return E_NOINTERFACE;

    memzero(&vIndex, sizeof(vIndex));
    vIndex.vt = VT_I4;
    vIndex.lVal = index;

    if (!SH32_ComGetVtbl(pShellWindows, 8, &vtbl))
        return E_NOINTERFACE;

    return ((P_Item)vtbl[8])(pShellWindows, vIndex, ppDispatch);
}


_FX HRESULT SH32_IShellWindows_GetCount(
    void *pShellWindows, long *pCount)
{
    ULONG_PTR *vtbl;
    typedef HRESULT (STDMETHODCALLTYPE *P_get_Count)(void *, long *);

    if (! pCount)
        return SH32_E_POINTER;

    *pCount = 0;

    if (! pShellWindows)
        return E_NOINTERFACE;

    if (!SH32_ComGetVtbl(pShellWindows, 7, &vtbl))
        return E_NOINTERFACE;

    return ((P_get_Count)vtbl[7])(pShellWindows, pCount);
}


_FX HRESULT WINAPI SH32_IShellWindows_FindWindowSW(
    void *pShellWindows,
    VARIANT *pvarLoc, VARIANT *pvarLocRoot,
    int swClass, long *phwnd, int swfwOptions,
    IDispatch **ppdispOut
#if !defined(_M_ARM64) && !defined(_M_ARM64EC)
    )
{
    ULONG_PTR *StubData = Dll_JumpStubData();
#else
    , ULONG_PTR *StubData)
{
#endif
    typedef HRESULT (STDMETHODCALLTYPE *P_FindWindowSW)(
        void *, VARIANT *, VARIANT *, int, long *, int, IDispatch **);

    if (!StubData || !StubData[1])
        return E_NOINTERFACE;

    if (ppdispOut)
        *ppdispOut = NULL;

    HRESULT hr = ((P_FindWindowSW)StubData[1])(
        pShellWindows, pvarLoc, pvarLocRoot,
        swClass, phwnd, swfwOptions, ppdispOut);

    if (swClass != SH32_SWC_DESKTOP)
        return hr;

    // UseFakeShellDispatch=n leaves desktop shell dispatch untouched.
    // When enabled, Sandboxie keeps the forced synthetic path in compartment
    // mode, but otherwise prefers a usable real desktop dispatch first and
    // falls back to the synthetic chain only when the real shell path is not
    // available or not usable.
    if (! Config_GetSettingsForImageName_bool(L"UseFakeShellDispatch", TRUE))
        return hr;

    if ((swfwOptions & SH32_SWFO_NEEDDISPATCH) && ppdispOut) {

        // In compartment mode we still force the synthetic dispatch chain so
        // ShellExecute remains routed through Sandboxie-controlled code.
        if (Dll_CompartmentMode) {
            if (*ppdispOut) {
                SH32_ComRelease(*ppdispOut);
                *ppdispOut = NULL;
            }
            *ppdispOut = (IDispatch *)SH32_FakeDesktop_Create();
            if (*ppdispOut)
                SbieApi_MonitorPut2(MONITOR_COMCLASS | MONITOR_TRACE,
                    L"UseFakeShellDispatch[S0]: enabled in compartment mode -> synthetic dispatch", FALSE);
            return *ppdispOut ? S_OK : E_OUTOFMEMORY;
        }

        BOOLEAN valid_dispatch = SH32_IsUsableShellDispatch(*ppdispOut);

        if (valid_dispatch) {
            // Accept this only if the shell window is owned by a process that
            // runs inside this sandbox.  When OpenClsid allows the real
            // IShellWindows, FindWindowSW returns the HOST desktop dispatch
            // (which also satisfies SH32_IsUsableShellDispatch), but it must
            // not be used as the in-sandbox dispatch - fall through to S5.
            BOOLEAN ownerInSandbox = FALSE;
            if (phwnd && *phwnd) {
                ULONG ownerPid = 0;
                extern DWORD (*__sys_GetWindowThreadProcessId)(HWND hWnd, ULONG *ppid);
                if (__sys_GetWindowThreadProcessId)
                    __sys_GetWindowThreadProcessId((HWND)(LONG_PTR)*phwnd, &ownerPid);
                if (ownerPid) {
                    WCHAR sbxname[BOXNAME_COUNT];
                    LONG st = SbieApi_QueryProcess(
                        (HANDLE)(ULONG_PTR)ownerPid, sbxname, NULL, NULL, NULL);
                    ownerInSandbox = NT_SUCCESS(st) && sbxname[0] != L'\0';
                }
            }
            if (ownerInSandbox) {
                SbieApi_MonitorPut2(MONITOR_COMCLASS | MONITOR_TRACE,
                    L"UseFakeShellDispatch[S1A]: desktop dispatch already usable", FALSE);
            } else {
                valid_dispatch = FALSE;
                SbieApi_MonitorPut2(MONITOR_COMCLASS | MONITOR_TRACE,
                    L"UseFakeShellDispatch[S1A]: hwnd not in sandbox -> fallback", FALSE);
            }
        }

        if (! valid_dispatch) {
            SbieApi_MonitorPut2(MONITOR_COMCLASS | MONITOR_TRACE,
                L"UseFakeShellDispatch[S1]: desktop dispatch unusable, trying fallbacks", FALSE);

            IDispatch *pDispatch2 = NULL;
            HRESULT hr2;
            long hwnd2 = 0;

            if (*ppdispOut) {
                SH32_ComRelease(*ppdispOut);
                *ppdispOut = NULL;
            }

            hr2 = ((P_FindWindowSW)StubData[1])(
                pShellWindows, pvarLoc, pvarLocRoot,
                SH32_SWC_EXPLORER,
                &hwnd2,
                swfwOptions | SH32_SWFO_INCLUDEPENDING,
                &pDispatch2);

            if (SUCCEEDED(hr2) && SH32_IsUsableShellDispatch(pDispatch2)) {
                *ppdispOut = pDispatch2;
                if (phwnd)
                    *phwnd = hwnd2;
                hr = hr2;
                SbieApi_MonitorPut2(MONITOR_COMCLASS | MONITOR_TRACE,
                    L"UseFakeShellDispatch[S2]: explorer fallback yielded usable dispatch", FALSE);
            }
            else {
                SH32_ComRelease(pDispatch2);
                pDispatch2 = NULL;

                hr2 = SH32_IShellWindows_ItemDispatch(
                    pShellWindows, 0, &pDispatch2);

                if (SUCCEEDED(hr2) && SH32_IsUsableShellDispatch(pDispatch2)) {
                    *ppdispOut = pDispatch2;
                    hr = S_OK;
                    // *phwnd is not updated here: IShellWindows::Item() returns no window
                    // handle.  The tail GetShellWindow() call below fills it in.
                    SbieApi_MonitorPut2(MONITOR_COMCLASS | MONITOR_TRACE,
                        L"UseFakeShellDispatch[S3]: Item(0) fallback yielded usable dispatch", FALSE);
                }
                else {
                    SH32_ComRelease(pDispatch2);
                    pDispatch2 = NULL;

                    {
                        long count = 0;
                        long index;

                        hr2 = SH32_IShellWindows_GetCount(pShellWindows, &count);
                        if (SUCCEEDED(hr2) && count > 0) {

                            for (index = 0; index < count; ++index) {

                                hr2 = SH32_IShellWindows_ItemDispatch(
                                    pShellWindows, index, &pDispatch2);

                                if (FAILED(hr2) || ! pDispatch2)
                                    continue;

                                if (SH32_IsUsableShellDispatch(pDispatch2)) {
                                    *ppdispOut = pDispatch2;
                                    hr = S_OK;
                                    // *phwnd is not updated here for the same reason as S3.
                                    SbieApi_MonitorPut2(MONITOR_COMCLASS | MONITOR_TRACE,
                                        L"UseFakeShellDispatch[S4]: Item(n) fallback yielded usable dispatch", FALSE);
                                    break;
                                }

                                SH32_ComRelease(pDispatch2);
                                pDispatch2 = NULL;
                            }
                        }

                        // No real shell window found in the sandbox
                        // synthesise the full IShellDispatch2 chain ourselves.
                        if (! *ppdispOut) {
                            *ppdispOut = (IDispatch *)SH32_FakeDesktop_Create();
                            if (*ppdispOut)
                                SbieApi_MonitorPut2(MONITOR_COMCLASS | MONITOR_TRACE,
                                    L"UseFakeShellDispatch[S5]: no usable real dispatch -> synthetic dispatch", FALSE);
                            hr = *ppdispOut ? S_OK : E_OUTOFMEMORY;
                        }
                    }
                }
            }
        }
    }

    if (SUCCEEDED(hr) && phwnd && *phwnd)
        return hr;

    typedef HWND (*P_GetShellWindow)(void);
    P_GetShellWindow pGetShellWindow = (P_GetShellWindow)
        Ldr_GetProcAddrNew(DllName_user32, L"GetShellWindow", "GetShellWindow");

    HWND hDesktop = pGetShellWindow ? pGetShellWindow() : NULL;

    if (! hDesktop)
        return hr;

    if (phwnd)
        *phwnd = (long)(LONG_PTR)hDesktop;

    if (swfwOptions & SH32_SWFO_NEEDDISPATCH) {

        if (! ppdispOut)
            return SH32_E_POINTER;

        if (! *ppdispOut)
            return FAILED(hr) ? hr : SH32_E_POINTER;
    }

    return S_OK;
}


_FX void SH32_IShellWindows_Hook(REFCLSID rclsid, REFIID riid, void *pUnknown)
{
    static const WCHAR *TraceMsg =
        L"UseFakeShellDispatch[H0]: IShellWindows hook sanity check failed";
    ULONG_PTR *vtbl;
    IUnknown *pIsw;
    HRESULT hr;
    typedef HRESULT (STDMETHODCALLTYPE *P_QueryInterface)(void *, REFIID, void **);

    UNREFERENCED_PARAMETER(rclsid);

    if (! pUnknown)
        return;

    if (memcmp(riid, &SH32_IID_IShellWindows, sizeof(GUID)) != 0)
        return;

    if (!SH32_ComGetVtbl(pUnknown, 15, &vtbl)) {
        SbieApi_MonitorPut2(MONITOR_COMCLASS | MONITOR_TRACE, TraceMsg, FALSE);
        return;
    }

    pIsw = NULL;
    hr = ((P_QueryInterface)vtbl[0])(
        pUnknown, &SH32_IID_IShellWindows, (void **)&pIsw);
    if (FAILED(hr) || !pIsw) {
        SbieApi_MonitorPut2(MONITOR_COMCLASS | MONITOR_TRACE, TraceMsg, FALSE);
        return;
    }

    if (!SH32_ComGetVtbl(pIsw, 15, &vtbl)) {
        SbieApi_MonitorPut2(MONITOR_COMCLASS | MONITOR_TRACE, TraceMsg, FALSE);
        SH32_ComRelease(pIsw);
        return;
    }

    SH32_IContextMenu_HookVtbl(
        pIsw, 15, SH32_IShellWindows_FindWindowSW);

    SH32_ComRelease(pIsw);
}


