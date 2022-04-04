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
// Shell Links and Internet Shortcuts
//---------------------------------------------------------------------------

#include "stdafx.h"

#include "core/dll/sbiedll.h"
#include "common/defines.h"


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------


extern void Show_Error(WCHAR *Descr);
extern void MyCoInitialize(void);


//---------------------------------------------------------------------------
// GetLinkInstance
//---------------------------------------------------------------------------


void GetLinkInstance(
    const WCHAR *path,
    IShellLink **ppShellLink,
    IPersistFile **ppPersistFile,
    bool msi)
{
    static const WCHAR *_format = L"%s [%08X] - %s";
    WCHAR errmsg[512];

    *ppShellLink = NULL;
    *ppPersistFile = NULL;

    const WCHAR *dot = wcsrchr(path, L'.');
    if (! dot)
        return;
    if (_wcsicmp(dot, L".lnk") != 0 && _wcsicmp(dot, L".url") != 0)
        return;
    if (_wcsicmp(dot, L".url") == 0)
        return;

    MyCoInitialize();

    IShellLink *pShellLink;
    HRESULT hr = CoCreateInstance(
        CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink,
        (void **)&pShellLink);
    if (FAILED(hr)) {
        wsprintf(
            errmsg, _format, L"Cannot instantiate IShellLink", hr, path);
        Show_Error(errmsg);
        return;
    }

    IPersistFile *pPersistFile;
    hr = pShellLink->QueryInterface(
        IID_IPersistFile, (void **)&pPersistFile);
    if (FAILED(hr)) {
        wsprintf(
            errmsg, _format, L"Cannot instantiate IPersistFile", hr, path);
        Show_Error(errmsg);
        pShellLink->Release();
        return;
    }

    hr = pPersistFile->Load(path, STGM_READ);
    if (SUCCEEDED(hr) && msi) {
        hr = pShellLink->Resolve(
            NULL, SLR_NOUPDATE | SLR_NO_UI | SLR_INVOKE_MSI);
    }
    if (FAILED(hr)) {
        // Show_Error(L"Cannot resolve shortcut object");
        pPersistFile->Release();
        pShellLink->Release();
        return;
    }

    *ppShellLink = pShellLink;
    *ppPersistFile = pPersistFile;
}


//---------------------------------------------------------------------------
// ResolveLink
//---------------------------------------------------------------------------


/*
extern "C" BOOL ResolveLink(WCHAR *path)
{
    WCHAR *ptr = wcsrchr(path, L'.');
    if (! ptr)
        return TRUE;
    if (_wcsicmp(ptr, L".lnk") != 0 && _wcsicmp(ptr, L".url") != 0)
        return TRUE;

    IShellLink *pShellLink;
    IPersistFile *pPersistFile;
    GetLinkInstance(path, &pShellLink, &pPersistFile, true);
    if ((! pShellLink) || (! pPersistFile))
        return FALSE;

    // using IShellLink::GetIDList and then SHGetPathFromIDList,
    // rather than simply IShellLink::GetPath, because the former
    // resolves MSI advertisements, and the latter does not

    ITEMIDLIST *pidl = NULL;
    HRESULT hr = pShellLink->GetIDList(&pidl);
    if (SUCCEEDED(hr)) {

        SHGetPathFromIDList(pidl, path);
        if (path[0] != L'\"') {
            wcscpy(path, L"\"");
            SHGetPathFromIDList(pidl, &path[1]);
            wcscat(path, L"\" ");
        } else
            wcscat(path, L" ");
        WCHAR *ptr = path + wcslen(path);
        hr = pShellLink->GetArguments(ptr, 1024);
    }

    // change to the working directory specified in the link

    DWORD buflen = 10240;
    WCHAR *buf = (WCHAR *)HeapAlloc(
        GetProcessHeap(), HEAP_GENERATE_EXCEPTIONS, buflen);

    hr = pShellLink->GetWorkingDirectory(
        buf, buflen / sizeof(WCHAR) - 1);
    if (SUCCEEDED(hr))
        SetCurrentDirectory(buf);

    HeapFree(GetProcessHeap(), HEAP_GENERATE_EXCEPTIONS, buf);

    pPersistFile->Release();
    pShellLink->Release();

    if (FAILED(hr)) {
        Show_Error(L"There is a problem with the shortcut object.");
        return FALSE;
    }

    return TRUE;
}
*/


//---------------------------------------------------------------------------
// GetLinkIconPathAndIndex
//---------------------------------------------------------------------------


BOOLEAN GetLinkIconPathAndNumber(
    WCHAR *LinkPath, WCHAR **IconPath, ULONG *IconIndex)
{
    ULONG buflen = 4096;
    WCHAR *buf = (WCHAR *)HeapAlloc(
                                GetProcessHeap(), HEAP_ZERO_MEMORY, buflen);
    if (! buf)
        return FALSE;

    IShellLink *pShellLink;
    IPersistFile *pPersistFile;
    GetLinkInstance(LinkPath, &pShellLink, &pPersistFile, false);
    if ((! pShellLink) || (! pPersistFile)) {

        ULONG attrs = GetFileAttributes(LinkPath);
        if (attrs != INVALID_FILE_ATTRIBUTES) {

            const WCHAR *dot;
            if (attrs & FILE_ATTRIBUTE_DIRECTORY)
                dot = L"Folder";
            else
                dot = wcsrchr(LinkPath, L'.');

            if (dot && (_wcsicmp(dot, L".exe") == 0 ||
                        _wcsicmp(dot, L".dll") == 0)) {

                HICON hIcon =
                    ExtractIcon(GetModuleHandle(NULL), LinkPath, 0);
                if (hIcon && hIcon != (HICON)1) {

                    DestroyIcon(hIcon);

                    *IconIndex = 0;
                    wcscpy(buf, LinkPath);
                    *IconPath = buf;
                    return TRUE;
                }
            }

            if (dot) {

                ULONG len = 1000;
                HRESULT hr = AssocQueryString(ASSOCF_INIT_DEFAULTTOSTAR,
                                              ASSOCSTR_DEFAULTICON,
                                              dot, NULL, buf, &len);
                if (SUCCEEDED(hr) && buf[0]) {

                    WCHAR *comma = wcsrchr(buf, L',');
                    if (comma) {
                        *comma = L'\0';
                        *IconIndex = _wtoi(comma + 1);
                    } else
                        *IconIndex = 0;

                    if (buf[0] == L'\"') {
                        wmemmove(buf, buf + 1, wcslen(buf));
                        comma = wcschr(buf, L'\"');
                        if (comma)
                            *comma = L'\0';
                    }

                    *IconPath = buf;
                    return TRUE;
                }
            }
        }

        HeapFree(GetProcessHeap(), 0, buf);
        return FALSE;
    }

    int index;
    HRESULT hr = pShellLink->GetIconLocation(&buf[0], 1000, &index);
    if (FAILED(hr))
        buf[0] = L'\0';
    if (! buf[0]) {
        LPITEMIDLIST pidl;
        LPCITEMIDLIST pidl_last;
        IShellFolder *pShellFolder;
        IExtractIcon *pExtractIcon;
        hr = pShellLink->GetIDList(&pidl);
        if (SUCCEEDED(hr)) {
            hr = SHBindToParent(
                            pidl, IID_IShellFolder,
                            (void **)&pShellFolder, &pidl_last);
            if (SUCCEEDED(hr)) {
                hr = pShellFolder->GetUIObjectOf(
                            NULL, 1, &pidl_last, IID_IExtractIcon, NULL,
                            (void **)&pExtractIcon);
                if (SUCCEEDED(hr)) {
                    UINT flags = 0;
                    hr = pExtractIcon->GetIconLocation(
                            0, &buf[0], 1000, &index, &flags);
                    if (FAILED(hr) || (flags & GIL_NOTFILENAME))
                        buf[0] = L'\0';
                    pExtractIcon->Release();
                }
                pShellFolder->Release();
            }
            CoTaskMemFree(pidl);
        }
    }
    if (! buf[0]) {
        hr = pShellLink->GetPath(&buf[0], 1000, NULL, 0);
        if (FAILED(hr))
            buf[0] = L'\0';
        index = 0;
    }

    if (buf[0]) {
        ULONG exp_len = ExpandEnvironmentStrings(&buf[0], &buf[1024], 1000);
        if (exp_len < 1000)
            wmemcpy(&buf[0], &buf[1024], exp_len);
    }
    if (index == -1)
        index = 0;

    buf[1022] = L'\0';
    buf[1024] = L'\0';
    hr = pShellLink->GetWorkingDirectory(&buf[1024], 1000);
    if (FAILED(hr))
        buf[1024] = L'\0';

    pPersistFile->Release();
    pShellLink->Release();

    if ((! buf[0]) && (! buf[1024])) {
        HeapFree(GetProcessHeap(), 0, buf);
        return FALSE;
    }

    *IconPath = buf;
    *IconIndex = (ULONG)index;
    return TRUE;
}


//---------------------------------------------------------------------------
// GetLinkIcon
//---------------------------------------------------------------------------


HICON GetLinkIcon(WCHAR *path)
{
    HICON hIcon = NULL;
    WCHAR *IconPath;
    ULONG IconIndex;

    if (GetLinkIconPathAndNumber(path, &IconPath, &IconIndex)) {
        if (ExtractIconEx(IconPath, IconIndex, NULL, &hIcon, 1) != 1)
            hIcon = NULL;
        if (! hIcon) {
            USHORT wIconIndex = (USHORT)IconIndex;
            hIcon = ExtractAssociatedIcon(
                GetModuleHandle(NULL), IconPath, &wIconIndex);
        }
        HeapFree(GetProcessHeap(), 0, IconPath);
    }

    return hIcon;
}


//---------------------------------------------------------------------------
// ResolveExtension
//---------------------------------------------------------------------------


extern "C" BOOL ResolveExtension(WCHAR *path)
{
    ULONG len = (wcslen(path) + 1) * sizeof(WCHAR);
    WCHAR *src = (WCHAR *)HeapAlloc(
        GetProcessHeap(), HEAP_GENERATE_EXCEPTIONS, len);
    if (path[0] == L'\"') {
        wcscpy(src, &path[1]);
        if (wcschr(src, L'\"'))
            *wcschr(src, L'\"') = L'\0';
    } else
        wcscpy(src, path);

    // test if path is a simple path to a document
    DWORD attrs = GetFileAttributes(src);
    if (attrs == INVALID_FILE_ATTRIBUTES)
        return TRUE;

    // now figure out if the extension is for a document
    WCHAR *dot = wcsrchr(src, L'.');
    if (! dot)
        return TRUE;

    if (_wcsnicmp(dot, L".exe", 4) == 0)
        return TRUE;

    WCHAR *cmd = SbieDll_AssocQueryCommand(dot);
    if (! cmd)
        return TRUE;

    WCHAR *iptr = cmd;
    WCHAR *optr = path;

    while (*iptr) {

        if (iptr[0] == L'%' && (iptr[1] == L'1' || iptr[1] == L'L')) {
            wcscpy(optr, src);
            optr += wcslen(src);
            iptr += 2;

        } else if (iptr[0] == L'%' && iptr[1] == L'*') {
            iptr += 2;

        } else {
            *optr = *iptr;
            ++optr;
            ++iptr;
        }
    }

    *optr = L'\0';

    HeapFree(GetProcessHeap(), HEAP_GENERATE_EXCEPTIONS, src);

    return TRUE;
}


//---------------------------------------------------------------------------
// ResolveDirectory
//---------------------------------------------------------------------------


BOOL ResolveDirectory(WCHAR *PathW)
{
    WCHAR *PathUnquoted = (WCHAR *)HeapAlloc(
        GetProcessHeap(), HEAP_GENERATE_EXCEPTIONS, 10240 * sizeof(WCHAR));
    if (*PathW == L'\"') {
        wcscpy(PathUnquoted, PathW + 1);
        if (PathUnquoted[wcslen(PathUnquoted) - 1] == L'\"')
        {
            PathUnquoted[wcslen(PathUnquoted) - 1] = L'\0';
        }
    } else
        wcscpy(PathUnquoted, PathW);

    WCHAR *Folder = (WCHAR *)HeapAlloc(
        GetProcessHeap(), HEAP_GENERATE_EXCEPTIONS, 10240 * sizeof(WCHAR));

    if (PathW[0] == L'.' && PathW[1] == L'\0') {
        HRESULT hr = SHGetFolderPath(
            NULL, CSIDL_PERSONAL, NULL, SHGFP_TYPE_CURRENT, Folder);
    } else {
        WCHAR *FileNamePtr;
        GetFullPathName(PathUnquoted, 10236, Folder, &FileNamePtr);
    }

    if (SbieDll_IsDirectory(Folder)) {

        PathW[0] = L'\"';
        GetSystemWindowsDirectory(&PathW[1], MAX_PATH);
        wcscat(PathW, L"\\explorer.exe\" /e,\"");
        wcscat(PathW, Folder);
        wcscat(PathW, L"\"");
    }

    HeapFree(GetProcessHeap(), HEAP_GENERATE_EXCEPTIONS, Folder);
    HeapFree(GetProcessHeap(), HEAP_GENERATE_EXCEPTIONS, PathUnquoted);

    return TRUE;
}
