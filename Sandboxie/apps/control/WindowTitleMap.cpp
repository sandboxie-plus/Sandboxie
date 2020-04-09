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
// Window Title Map
//---------------------------------------------------------------------------


#include "stdafx.h"
#include "WindowTitleMap.h"


//---------------------------------------------------------------------------
// Imports from PSAPI
//---------------------------------------------------------------------------


extern "C" DWORD GetModuleFileNameExW(
    HANDLE hProcess, HMODULE hModule, LPTSTR lpFilename, DWORD nSize);

typedef int (*P_InternalGetWindowText)(
    HWND hWnd, LPWSTR lpString, int nMaxCount);


//---------------------------------------------------------------------------
// Structures
//---------------------------------------------------------------------------


struct MapEntry {

   WCHAR name[100];
   HICON icon;
   FILETIME time;
   BOOL valid;

};


//---------------------------------------------------------------------------
// Variables
//---------------------------------------------------------------------------


CWindowTitleMap *CWindowTitleMap::m_instance = new CWindowTitleMap();


//---------------------------------------------------------------------------
// Constructor
//---------------------------------------------------------------------------


CWindowTitleMap::CWindowTitleMap()
{
    m_counter = 0;

    m_nullIcon = LoadIcon(NULL, IDI_APPLICATION);

    m_pGetWindowText = GetProcAddress(
        GetModuleHandle(L"user32.dll"), "InternalGetWindowText");

    InitHashTable(100, TRUE);
}


//---------------------------------------------------------------------------
// Destructor
//---------------------------------------------------------------------------


CWindowTitleMap::~CWindowTitleMap()
{
    Clear();
}


//---------------------------------------------------------------------------
// GetInstance
//---------------------------------------------------------------------------


CWindowTitleMap &CWindowTitleMap::GetInstance()
{
    return *m_instance;
}


//---------------------------------------------------------------------------
// Clear
//---------------------------------------------------------------------------


void CWindowTitleMap::Clear()
{
    void *key, *ptr;
    POSITION pos = GetStartPosition();
    while (pos) {
        GetNextAssoc(pos, key, ptr);
        MapEntry *entry = (MapEntry *)ptr;
        delete entry;
    }
    RemoveAll();
}


//---------------------------------------------------------------------------
// Refresh
//---------------------------------------------------------------------------


void CWindowTitleMap::Refresh()
{
    ++m_counter;
    SbieApi_EnumProcess(NULL, m_pids);
    EnumWindows(CWindowTitleMap::EnumProc, (LPARAM)this);
    RefreshIcons();
}


//---------------------------------------------------------------------------
// ShouldIgnoreProcess
//---------------------------------------------------------------------------


BOOL CWindowTitleMap::ShouldIgnoreProcess(ULONG pid)
{
    ULONG i, num;

    num = m_pids[0];
    if (num > 500)
        return FALSE;

    for (i = 1; i <= num; ++i)
        if (m_pids[i] == pid)
            return FALSE;

    return TRUE;
}


//---------------------------------------------------------------------------
// EnumProc
//---------------------------------------------------------------------------


BOOL CWindowTitleMap::EnumProc(HWND hwnd, LPARAM lParam)
{
    if (GetParent(hwnd) || GetWindow(hwnd, GW_OWNER))
        return TRUE;
    ULONG style = GetWindowLong(hwnd, GWL_STYLE);
    if ((style & (WS_CAPTION | WS_SYSMENU)) != (WS_CAPTION | WS_SYSMENU))
        return TRUE;
    if (! IsWindowVisible(hwnd))
        return TRUE;
    /*
    if ((style & WS_OVERLAPPEDWINDOW) != WS_OVERLAPPEDWINDOW &&
        (style & WS_POPUPWINDOW)      != WS_POPUPWINDOW)
        return TRUE;
    */

    CWindowTitleMap &map = *(CWindowTitleMap *)lParam;

#ifdef _WIN64
    ULONG64 pid;
    ULONG pid32;
    GetWindowThreadProcessId(hwnd, &pid32);
    pid = pid32;
#else
    ULONG pid;
    GetWindowThreadProcessId(hwnd, &pid);
#endif

    if (map.ShouldIgnoreProcess((ULONG)(ULONG_PTR)pid))
        return TRUE;

    MapEntry *entry;
    void *ptr;
    BOOL ok = map.Lookup((void *)pid, ptr);
    if (ok && ptr) {
        entry = (MapEntry *)ptr;
        if ((map.m_counter % 5) == 0)
            entry->name[0] = L'\0';
    } else {
        entry = new MapEntry();
        entry->name[0] = L'\0';
        entry->icon = NULL;
        entry->time.dwLowDateTime = 0;
        entry->time.dwHighDateTime = 0;
    }
    entry->valid = TRUE;

    if (! entry->name[0]) {

        ((P_InternalGetWindowText)map.m_pGetWindowText)(hwnd, entry->name, 99);
        entry->name[99] = L'\0';

        map.SetAt((void *)pid, entry);
    }

    return TRUE;
}


//---------------------------------------------------------------------------
// RefreshIcons
//---------------------------------------------------------------------------


void CWindowTitleMap::RefreshIcons()
{
    WCHAR path[300];
    //SHFILEINFO shfi;

    void *key, *ptr;
    POSITION pos = GetStartPosition();
    while (pos) {
        GetNextAssoc(pos, key, ptr);
        MapEntry *entry = (MapEntry *)ptr;
        if (! entry->valid)
            continue;

        //
        // open the process object to extract its icon
        //

        HANDLE hProcess = NULL;

        if (! ShouldIgnoreProcess((ULONG)(ULONG_PTR)key)) {

            hProcess = OpenProcess(
                PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,
                FALSE, (ULONG)(ULONG_PTR)key);
        }

        if (! hProcess) {

            if (entry->icon) {
                DestroyIcon(entry->icon);
                entry->icon = NULL;
            }
            entry->name[0] = L'\0';
            entry->valid = FALSE;

            continue;
        }

        //
        // check the process creation time, if it's the same
        // as the recorded time, then skip
        //

        FILETIME time, time1, time2, time3;
        BOOL ok = GetProcessTimes(hProcess, &time, &time1, &time2, &time3);
        if (! ok) {

            if (entry->icon) {
                DestroyIcon(entry->icon);
                entry->icon = NULL;
            }
            entry->name[0] = L'\0';
            entry->valid = FALSE;

        } else if (time.dwLowDateTime == entry->time.dwLowDateTime
                && time.dwHighDateTime == entry->time.dwHighDateTime)
            ok = FALSE;

        if (! ok) {
            CloseHandle(hProcess);
            continue;
        }

        entry->time.dwLowDateTime  = time.dwLowDateTime;
        entry->time.dwHighDateTime = time.dwHighDateTime;

        //
        // extract the icon
        //

        if (entry->icon) {
            DestroyIcon(entry->icon);
            entry->icon = NULL;
        }

        path[299] = L'\0';
        int sz = GetModuleFileNameExW(hProcess, NULL, path, 299);

        CloseHandle(hProcess);

        if (sz == 0)
            continue;

        HICON hIcon;
        ExtractIconEx(path, 0, &hIcon, NULL, 1);
        entry->icon = hIcon;
    }
}


//---------------------------------------------------------------------------
// Get
//---------------------------------------------------------------------------


const WCHAR *CWindowTitleMap::Get(ULONG pid, HICON &icon)
{
    void *ptr;
    BOOL ok = CMapPtrToPtr::Lookup((void *)(ULONG_PTR)pid, ptr);
    if (ok) {
        MapEntry *entry = (MapEntry *)ptr;
        icon = entry->icon;
        if (! icon)
            icon = m_nullIcon;
        return (const WCHAR *)entry->name;
    }
    icon = m_nullIcon;
    return NULL;
}
