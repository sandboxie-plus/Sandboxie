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
// Start Menu
//---------------------------------------------------------------------------

#include "stdafx.h"

#include "common/defines.h"
#include "common/list.h"
#include "core/dll/sbiedll.h"
#include "msgs/msgs.h"
#include "common/my_version.h"


//---------------------------------------------------------------------------
// Structures
//---------------------------------------------------------------------------


typedef struct _MENU_DIR MENU_DIR;
typedef struct _MENU_ITEM MENU_ITEM;

struct _MENU_DIR {
    LIST_ELEM list_elem;
    WCHAR *name;
    WCHAR *path;
    LIST subdirs;
    LIST items;
    MENU_DIR *parent;
    BOOLEAN desktop;
    BOOLEAN allfiles;
    BOOLEAN boxed;
    UCHAR source;
};

struct _MENU_ITEM {
    LIST_ELEM list_elem;
    WCHAR *name;
    HBITMAP bitmap;
    WCHAR *target;
    BOOLEAN boxed;
    UCHAR source;
};


//---------------------------------------------------------------------------
// Variables
//---------------------------------------------------------------------------


static HANDLE       StartMenu_Heap;
static ULONG_PTR   *StartMenu_IdMap;
static ULONG        StartMenu_LastId;
static ULONG        StartMenu_Count;
static WCHAR       *StartMenu_TempPath;

extern BOOL run_elevated_2;

extern BOOLEAN layout_rtl;


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------


static MENU_DIR *Insert_MENU_DIR(
                    MENU_DIR *menu, const WCHAR *name, const WCHAR *subdir,
                    MENU_DIR *insert_after, UCHAR source);

static MENU_ITEM *Insert_MENU_ITEM(
                    MENU_DIR *menu, WCHAR *name, WCHAR *target,
                    UCHAR source);

static ULONG InsertIntoIdMap(const WCHAR *str);

static HMENU BuildTopLevelMenu(MENU_DIR *root);

static void BuildMenu_InsertSeparator(HMENU hMenu);

static void BuildMenu_InsertMenus(
    HMENU hMenu, MENU_DIR *menu, BOOLEAN toplevel, BOOLEAN boxed,
    MENUINFO *menuinfo, BOOLEAN *separator);

static void BuildMenu_InsertItems(
    HMENU hMenu, MENU_DIR *menu, BOOLEAN boxed, BOOLEAN *separator);

static void BuildMenu(HMENU hMenu, MENU_DIR *menu, WCHAR *fullpath);

static void ScanFolder(MENU_DIR *menu, WCHAR *path, UCHAR source);

static void ScanAllFilesAndFolders(MENU_DIR *menu, WCHAR *path);

static LRESULT StartMenuWndProc(
                    HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);


//---------------------------------------------------------------------------
// Insert_MENU_DIR
//---------------------------------------------------------------------------


_FX MENU_DIR *Insert_MENU_DIR(
    MENU_DIR *menu, const WCHAR *name, const WCHAR *subdir,
    MENU_DIR *insert_after, UCHAR source)
{
    MENU_DIR *dir, *x;
    ULONG len;

    dir = (MENU_DIR*)HeapAlloc(StartMenu_Heap, 0, sizeof(MENU_DIR));
    if (! dir)
        return NULL;

    len = wcslen(name);
    dir->name = (WCHAR*)HeapAlloc(StartMenu_Heap, 0, (len + 8) * sizeof(WCHAR));
    if (! dir->name)
        return NULL;
    wcscpy(dir->name, L"  ");
    wcscat(dir->name, name);
    wcscat(dir->name, L"    ");

    len = subdir ? wcslen(subdir) : 0;
    if (menu->path)
        len += wcslen(menu->path);
    dir->path = (WCHAR*)HeapAlloc(StartMenu_Heap, 0, (len + 8) * sizeof(WCHAR));
    if (! dir->path)
        return NULL;
    if (menu->path)
        wcscpy(dir->path, menu->path);
    else
        dir->path[0] = L'\0';
    if (subdir) {
        BOOLEAN addbackslash = TRUE;
        if (menu->allfiles && (! menu->path[0]))
            addbackslash = FALSE;
        if (addbackslash)
            wcscat(dir->path, L"\\");
        wcscat(dir->path, subdir);
    }

    List_Init(&dir->subdirs);
    List_Init(&dir->items);

    dir->parent = menu;

    dir->desktop = menu->desktop;

    dir->allfiles = menu->allfiles;

    dir->boxed = FALSE;

    dir->source = source;

    x = (MENU_DIR*)List_Head(&menu->subdirs);
    while (x) {
        int cmp = _wcsicmp(x->name, dir->name);
        if (cmp == 0 && x->source == dir->source)
            return x;
        if ((! insert_after) && (cmp > 0))
            break;
        x = (MENU_DIR*)List_Next(x);
    }
    if (insert_after)
        List_Insert_After(&menu->subdirs, insert_after, dir);
    else if (x)
        List_Insert_Before(&menu->subdirs, x, dir);
    else
        List_Insert_After(&menu->subdirs, x, dir);

    return dir;
}


//---------------------------------------------------------------------------
// Insert_MENU_ITEM
//---------------------------------------------------------------------------


_FX MENU_ITEM *Insert_MENU_ITEM(
    MENU_DIR *menu, WCHAR *name, WCHAR *target, UCHAR source)
{
    static HDC ScreenDC = NULL;
    static HDC BitmapDC = NULL;
    static HBRUSH MenuBrush = NULL;
    HICON GetLinkIcon(WCHAR *path);
    MENU_ITEM *item, *x;
    HICON icon;

    item = (MENU_ITEM*)HeapAlloc(StartMenu_Heap, 0, sizeof(MENU_ITEM));
    if (! item)
        return NULL;

    item->name = (WCHAR*)HeapAlloc(
                    StartMenu_Heap, 0, (wcslen(name) + 8) * sizeof(WCHAR));
    if (! item->name)
        return NULL;

    wcscpy(item->name, L"  ");
    wcscat(item->name, name);
    wcscat(item->name, L"    ");

    item->target = (WCHAR*)HeapAlloc(
                    StartMenu_Heap, 0, (wcslen(target) + 1) * sizeof(WCHAR));
    if (! item->target)
        return NULL;
    wcscpy(item->target, target);

    icon = GetLinkIcon(target);
    if (icon) {
        HBITMAP old_bitmap;
        if (! ScreenDC) {
            ScreenDC = GetDC(NULL);
            BitmapDC = CreateCompatibleDC(NULL);
            MenuBrush = GetSysColorBrush(COLOR_MENU);
            SelectObject(BitmapDC, MenuBrush);
        }
        item->bitmap = CreateCompatibleBitmap(ScreenDC, 16, 16);
        old_bitmap = (HBITMAP)SelectObject(BitmapDC, item->bitmap);
        PatBlt(BitmapDC, 0, 0, 16, 16, PATCOPY);
        DrawIconEx(BitmapDC, 0, 0, icon, 16, 16, 0, NULL, DI_NORMAL);
        SelectObject(BitmapDC, old_bitmap);
        DestroyIcon(icon);
    } else
        item->bitmap = NULL;

    item->boxed = FALSE;

    item->source = source;

    x = (MENU_ITEM*)List_Head(&menu->items);
    while (x) {
        int cmp = _wcsicmp(x->name, item->name);
        if (cmp == 0 && x->source == item->source)
            return x;
        if (cmp > 0)
            break;
        x = (MENU_ITEM*)List_Next(x);
    }
    if (x)
        List_Insert_Before(&menu->items, x, item);
    else
        List_Insert_After(&menu->items, x, item);

    return item;
}


//---------------------------------------------------------------------------
// InsertIntoIdMap
//---------------------------------------------------------------------------


_FX ULONG InsertIntoIdMap(const WCHAR *str)
{
    if (StartMenu_LastId >= StartMenu_Count) {

        StartMenu_Count += 128;

        if (! StartMenu_IdMap) {

            StartMenu_IdMap = (ULONG_PTR*)HeapAlloc(
                StartMenu_Heap, 0,
                StartMenu_Count * sizeof(ULONG_PTR));

        } else {

            StartMenu_IdMap = (ULONG_PTR*)HeapReAlloc(
                StartMenu_Heap, 0, StartMenu_IdMap,
                StartMenu_Count * sizeof(ULONG_PTR));
        }
    }

    StartMenu_IdMap[StartMenu_LastId] = (ULONG_PTR)str;
    ++StartMenu_LastId;

    return StartMenu_LastId;
}


//---------------------------------------------------------------------------
// BuildTopLevelMenu
//---------------------------------------------------------------------------


_FX HMENU BuildTopLevelMenu(MENU_DIR *root)
{
    static WCHAR *_desktop = NULL;
    static WCHAR *_all_files_and_folders = NULL;
    WCHAR *path;
    MENU_DIR *subdir;
    HMENU hMenu;
    int i;

    if (! _desktop)
        _desktop = SbieDll_FormatMessage0(MSG_3112);

    subdir = NULL;

    path = (WCHAR*)HeapAlloc(StartMenu_Heap, 0, 2048 * sizeof(WCHAR));

#define GET_FOLDER_PATH(csidl) \
    (SHGetFolderPath(NULL, csidl, NULL, SHGFP_TYPE_CURRENT, path) == S_OK)

    //
    // add entry for desktop
    //

    if (GET_FOLDER_PATH(CSIDL_DESKTOPDIRECTORY)) {

        subdir = Insert_MENU_DIR(root, _desktop, NULL, NULL, 0);

        if (! subdir) {
            if (GET_FOLDER_PATH(CSIDL_COMMON_DESKTOPDIRECTORY)) {
                subdir = Insert_MENU_DIR(root, _desktop, NULL, NULL, 0);
            }
        }

        if (subdir)
            subdir->desktop = TRUE;

    } else
        subdir = NULL;

    //
    // add entry for start menu programs
    //

    for (i = 0; i < 2; ++i) {

        WCHAR *path_end;
        HANDLE hFind;
        WIN32_FIND_DATA data;

        int nFolder = (i == 0 ? CSIDL_STARTMENU : CSIDL_COMMON_STARTMENU);
        if (! GET_FOLDER_PATH(nFolder))
            continue;

        path_end = path + wcslen(path);
        wcscpy(path_end, L"\\*.*");

        hFind = FindFirstFile(path, &data);
        if ((! hFind) || hFind == INVALID_HANDLE_VALUE)
            continue;

        while (1) {

            if ((data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                    && wcscmp(data.cFileName, L".") != 0
                    && wcscmp(data.cFileName, L"..") != 0) {

                MENU_DIR *subdir2;
                WCHAR name[MAX_PATH + 8];

                wcscpy(name, data.cFileName);
                if (_wcsicmp(name, L"Programs") == 0)
                    wcscpy(name, SbieDll_FormatMessage0(MSG_3114));

                subdir2 = Insert_MENU_DIR(
                                root, name, data.cFileName, subdir, 0);

                if (subdir2)
                    subdir = subdir2;
            }

            if (! FindNextFile(hFind, &data))
                break;
        }

        FindClose(hFind);
    }

    //
    // add entry for All Files and Folders
    //

    if (! _all_files_and_folders)
        _all_files_and_folders = SbieDll_FormatMessage0(MSG_3521);

    subdir = Insert_MENU_DIR(root, _all_files_and_folders, NULL, subdir, 0);

    if (subdir)
        subdir->allfiles = TRUE;

    //
    // finish
    //

    HeapFree(StartMenu_Heap, 0, path);

    hMenu = CreatePopupMenu();
    BuildMenu(hMenu, root, NULL);
    return hMenu;
}


//---------------------------------------------------------------------------
// BuildMenu_InsertSeparator
//---------------------------------------------------------------------------


_FX void BuildMenu_InsertSeparator(HMENU hMenu)
{
    MENUITEMINFO mii;
    mii.cbSize = sizeof(mii);
    mii.fMask = MIIM_STATE | MIIM_FTYPE;
    mii.fType = MFT_SEPARATOR;
    mii.fState = MFS_DISABLED;
    InsertMenuItem(hMenu, GetMenuItemCount(hMenu), MF_BYPOSITION, &mii);
}


//---------------------------------------------------------------------------
// BuildMenu_InsertMenus
//---------------------------------------------------------------------------


_FX void BuildMenu_InsertMenus(
    HMENU hMenu, MENU_DIR *menu, BOOLEAN toplevel, BOOLEAN boxed,
    MENUINFO *menuinfo, BOOLEAN *separator)
{
    MENU_DIR *subdir;
    HMENU hSubMenu;
    MENUITEMINFO mii;

    subdir = (MENU_DIR *)List_Head(&menu->subdirs);
    while (subdir) {

        if (subdir->boxed != boxed) {
            subdir = (MENU_DIR *)List_Next(subdir);
            continue;
        }

        //
        // make sure that a separator appears between the group of boxed
        // items and non-boxed items, but only if we had both groups
        //

        if (boxed)
            *separator = TRUE;
        else if (*separator) {
            *separator = FALSE;
            BuildMenu_InsertSeparator(hMenu);
        }

        //
        // insert sub-menu
        //

        hSubMenu = CreateMenu();

        menuinfo->dwMenuData = (ULONG_PTR)subdir;
        SetMenuInfo(hSubMenu, menuinfo);

        mii.cbSize = sizeof(mii);
        mii.fMask = MIIM_STATE | MIIM_STRING | MIIM_SUBMENU;
        mii.fState = MFS_ENABLED;
        mii.hSubMenu = hSubMenu;
        mii.dwTypeData = subdir->name;
        mii.hbmpItem = NULL;
        mii.hbmpChecked = NULL;
        mii.hbmpUnchecked = NULL;

        InsertMenuItem(hMenu, GetMenuItemCount(hMenu), MF_BYPOSITION, &mii);

        subdir = (MENU_DIR *)List_Next(subdir);

        //
        // at the top level, insert a separator after each sub-menu
        //

        if (toplevel)
            BuildMenu_InsertSeparator(hMenu);
    }
}


//---------------------------------------------------------------------------
// BuildMenu_InsertItems
//---------------------------------------------------------------------------


_FX void BuildMenu_InsertItems(
    HMENU hMenu, MENU_DIR *menu, BOOLEAN boxed, BOOLEAN *separator)
{
    MENU_ITEM *item;
    MENUITEMINFO mii;

    item = (MENU_ITEM *)List_Head(&menu->items);
    while (item) {

        if (item->boxed != boxed) {
            item = (MENU_ITEM *)List_Next(item);
            continue;
        }

        //
        // make sure that a separator appears between the group of boxed
        // items and non-boxed items, but only if we had both groups
        //

        if (boxed)
            *separator = TRUE;
        else if (*separator) {
            *separator = FALSE;
            BuildMenu_InsertSeparator(hMenu);
        }

        //
        // insert item
        //

        mii.cbSize = sizeof(mii);
        mii.fMask = MIIM_ID | MIIM_STATE | MIIM_STRING | MIIM_BITMAP;
        mii.wID = InsertIntoIdMap(item->target);
        mii.fState = MFS_ENABLED;
        mii.dwTypeData = item->name;
        mii.hbmpItem = item->bitmap;
        mii.hbmpChecked = NULL;
        mii.hbmpUnchecked = NULL;

        InsertMenuItem(hMenu, GetMenuItemCount(hMenu), MF_BYPOSITION, &mii);

        item = (MENU_ITEM *)List_Next(item);
    }
}


//---------------------------------------------------------------------------
// BuildMenu
//---------------------------------------------------------------------------


_FX void BuildMenu(HMENU hMenu, MENU_DIR *menu, WCHAR *fullpath)
{
    static WCHAR title[128];
    static WCHAR *_explore = NULL;
    static WCHAR *_cancel = NULL;
    WCHAR boxname[BOXNAME_COUNT];
    MENUINFO menuinfo;
    MENUITEMINFO mii;
    BOOLEAN separator;

    if (! _explore)
        _explore = SbieDll_FormatMessage0(MSG_3113);
    if (! _cancel)
        _cancel  = SbieDll_FormatMessage0(MSG_3002);

    memzero(&menuinfo, sizeof(MENUINFO));
    menuinfo.cbSize = sizeof(MENUINFO);
    menuinfo.fMask = MIM_STYLE | MIM_MENUDATA;
    menuinfo.dwStyle = 0;
    menuinfo.dwMenuData = 0;
    SetMenuInfo(hMenu, &menuinfo);

    if (! fullpath) {

        //
        // top level menu, create the title label
        //

        SbieApi_QueryProcess(NULL, boxname, NULL, NULL, NULL);
        wsprintf(
            title, SbieDll_FormatMessage1(MSG_3111, boxname));

        mii.cbSize = sizeof(mii);
        mii.fMask = MIIM_STATE | MIIM_STRING;
        mii.fState = MFS_DISABLED;
        mii.dwTypeData = title;
        InsertMenuItem(hMenu, GetMenuItemCount(hMenu), MF_BYPOSITION, &mii);

        BuildMenu_InsertSeparator(hMenu);

    } else {

        //
        // sub menu, create a clickable "explore folder" item
        //

        ULONG fullpath_len = (wcslen(fullpath) + 1) * sizeof(WCHAR);
        WCHAR *fullpath2 = (WCHAR*)HeapAlloc(StartMenu_Heap, 0, fullpath_len);
        memcpy(fullpath2, fullpath, fullpath_len);

        wcscpy(title, L"  ");
        wcscat(title, _explore);

        mii.cbSize = sizeof(mii);
        mii.fMask = MIIM_ID | MIIM_STATE | MIIM_STRING;
        mii.wID = InsertIntoIdMap(fullpath2);
        mii.fState = MFS_ENABLED;
        mii.dwTypeData = title;
        InsertMenuItem(hMenu, GetMenuItemCount(hMenu), MF_BYPOSITION, &mii);

        BuildMenu_InsertSeparator(hMenu);
    }

    //
    // insert sub-menus and clickable items
    //

    separator = FALSE;

    BuildMenu_InsertMenus(
        hMenu, menu, (! fullpath), TRUE,  &menuinfo, &separator);

    BuildMenu_InsertItems(hMenu, menu, TRUE,  &separator);

    BuildMenu_InsertMenus(
        hMenu, menu, (! fullpath), FALSE, &menuinfo, &separator);

    BuildMenu_InsertItems(hMenu, menu, FALSE, &separator);

    //
    // insert "cancel" item at the bottom of the top level menu
    //

    if (! fullpath) {

        if (List_Count(&menu->items) != 0)
            BuildMenu_InsertSeparator(hMenu);

        wcscpy(title, L"  ");
        wcscat(title, _cancel);

        mii.cbSize = sizeof(mii);
        mii.fMask = MIIM_ID | MIIM_STATE | MIIM_STRING;
        mii.wID = 0;
        mii.fState = MFS_ENABLED;
        mii.dwTypeData = title;
        InsertMenuItem(hMenu, GetMenuItemCount(hMenu), MF_BYPOSITION, &mii);
    }
}


//---------------------------------------------------------------------------
// ScanFolder
//---------------------------------------------------------------------------


_FX void ScanFolder(MENU_DIR *menu, WCHAR *path, UCHAR source)
{
    WCHAR *path_end;
    HANDLE hFile;
    HANDLE hFind;
    WIN32_FIND_DATA data;

    path_end = path + wcslen(path);
    wcscpy(path_end, L"\\*.*");

    hFind = FindFirstFile(path, &data);
    if ((! hFind) || hFind == INVALID_HANDLE_VALUE)
        return;

    while (1) {

        BOOLEAN boxed = FALSE;

        if (wcscmp(data.cFileName, L".") != 0 &&
            wcscmp(data.cFileName, L"..") != 0 &&
            // OneDrive On-Demand feature set both FILE_ATTRIBUTE_RECALL_ON_DATA_ACCESS
            // and FILE_ATTRIBUTE_UNPINNED, to determine an expensive call, it should detect
            // flags: FILE_ATTRIBUTE_RECALL_ON_DATA_ACCESS | FILE_ATTRIBUTE_RECALL_ON_OPEN |
            // FILE_ATTRIBUTE_OFFLINE.
            //
            // This filter only considers the intersection
            (data.dwFileAttributes & FILE_ATTRIBUTE_RECALL_ON_DATA_ACCESS) == 0) {

            wcscpy(path_end + 1, data.cFileName);

            //
            // check if this is a sandboxed file
            //

            hFile = CreateFile(
                path, FILE_GENERIC_READ,
                FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                NULL, OPEN_EXISTING,
                FILE_ATTRIBUTE_NORMAL | FILE_FLAG_BACKUP_SEMANTICS, NULL);

            if (hFile != INVALID_HANDLE_VALUE) {

                BOOLEAN IsBoxedPath;
                LONG rc = SbieDll_GetHandlePath(
                        hFile, StartMenu_TempPath, &IsBoxedPath);

                if (rc == 0 && IsBoxedPath)
                    boxed = TRUE;

                CloseHandle(hFile);
            }

            //
            // add the folder or file
            //

            if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {

                MENU_DIR *menu_dir = Insert_MENU_DIR(
                                        menu, data.cFileName, data.cFileName,
                                        NULL, source);
                if (menu_dir)
                    menu_dir->boxed |= boxed;

            } else {

                WCHAR *ptr = NULL;
                if (_wcsicmp(data.cFileName, L"desktop.ini") != 0)
                    ptr = wcsrchr(data.cFileName, L'.');
                if (ptr && _wcsicmp(ptr, L".ico") == 0)
                    ptr = NULL;

                if (ptr) {

                    MENU_ITEM *menu_item;
                    wcscpy(path_end + 1, data.cFileName);
                    //*ptr = L'\0';
                    menu_item = Insert_MENU_ITEM(
                                    menu, data.cFileName, path, source);
                    if (menu_item)
                        menu_item->boxed |= boxed;
                }
            }
        }

        if (! FindNextFile(hFind, &data))
            break;
    }

    FindClose(hFind);
}


//---------------------------------------------------------------------------
// ScanAllFilesAndFolders
//---------------------------------------------------------------------------


_FX void ScanAllFilesAndFolders(MENU_DIR *menu, WCHAR *path)
{
    if (menu->path[0]) {

        wcscpy(path, menu->path);
        ScanFolder(menu, path, 0);
        wcscpy(path, menu->path);

    } else {

        ULONG DriveMask = GetLogicalDrives();
        WCHAR DrivePath[4] = L"?:\\";
        WCHAR DriveLetter;
        WCHAR *DriveTitle;

        for (DriveLetter = L'A'; DriveLetter <= 'Z'; ++DriveLetter) {
            if (! (DriveMask & (1 << (int)(DriveLetter - L'A'))))
                continue;

            DrivePath[0] = DriveLetter;
            DrivePath[1] = L'\0';
            DriveTitle = SbieDll_FormatMessage1(MSG_3676, DrivePath);
            DrivePath[1] = L':';
            DrivePath[2] = L'\\';
            DrivePath[3] = L'\0';

            Insert_MENU_DIR(menu, DriveTitle, DrivePath, NULL, 0);
        }

        path[0] = L'\0';
    }
}


//---------------------------------------------------------------------------
// StartMenuWndProc
//---------------------------------------------------------------------------


_FX LRESULT StartMenuWndProc(
    HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    HMENU hMenu;
    MENUINFO mi;
    MENU_DIR *menu;
    WCHAR *path = StartMenu_TempPath;
    ULONG count = 0;
    int i, nFolder;
    UCHAR source;

    //
    // proceed only if we get a WM_INITMENUPOPUP for an empty menu
    //

    if (msg != WM_INITMENUPOPUP)
        return DefWindowProc(hwnd, msg, wParam, lParam);

    hMenu = (HMENU)wParam;

    if (GetMenuItemCount(hMenu) != 0)
        return DefWindowProc(hwnd, msg, wParam, lParam);

    memzero(&mi, sizeof(MENUINFO));
    mi.cbSize = sizeof(MENUINFO);
    mi.fMask = MIM_MENUDATA;
    GetMenuInfo(hMenu, &mi);

    menu = (MENU_DIR *)mi.dwMenuData;
    if (! menu)
        return DefWindowProc(hwnd, msg, wParam, lParam);

    path = (WCHAR*)HeapAlloc(StartMenu_Heap, 0, 2048 * sizeof(WCHAR));

    //
    // build a menu for the all files and folders subtree
    //

    if (menu->allfiles) {

        ScanAllFilesAndFolders(menu, path);
        goto build_menu;
    }

    //
    // build new start menu entry
    //

    for (i = 0; i < 2; ++i) {

        if (menu->desktop) {
            nFolder = (i == 0 ? CSIDL_DESKTOPDIRECTORY
                              : CSIDL_COMMON_DESKTOPDIRECTORY);
            source = (i == 0 ? 'u' : 'a');  // this User or All users
        } else {
            nFolder = (i == 0 ? CSIDL_STARTMENU : CSIDL_COMMON_STARTMENU);
            source = 0;
        }

        if (! GET_FOLDER_PATH(nFolder))
            continue;

        wcscat(path, menu->path);

        ScanFolder(menu, path, source);
    }

    nFolder = menu->desktop ? CSIDL_DESKTOPDIRECTORY : CSIDL_STARTMENU;
    if (! GET_FOLDER_PATH(nFolder)) {
        nFolder = menu->desktop ? CSIDL_COMMON_DESKTOPDIRECTORY
                                : CSIDL_COMMON_STARTMENU;
        if (! GET_FOLDER_PATH(nFolder))
            path[0] = L'\0';
    }
    wcscat(path, menu->path);

    //
    //
    //

build_menu:

    BuildMenu(hMenu, menu, path);

    HeapFree(StartMenu_Heap, 0, path);

    return 0;
}


//---------------------------------------------------------------------------
// DoStartMenu
//---------------------------------------------------------------------------


_FX WCHAR *DoStartMenu()
{
    MENU_DIR *root;
    HMENU hMenu;
    WCHAR *title, *titleptr, *target;
    WNDCLASS wc;
    ATOM atom;
    HWND hWnd;
    ULONG flags, x;
    int id;

    //
    // initialize heap and id map
    //

    StartMenu_Heap = HeapCreate(HEAP_GENERATE_EXCEPTIONS, 1, 0);
    StartMenu_IdMap = NULL;
    StartMenu_LastId = 0;
    StartMenu_Count = 0;

    StartMenu_TempPath = (WCHAR*)HeapAlloc(StartMenu_Heap, 0, sizeof(WCHAR) * 8192);

    //
    //
    //

    root = (MENU_DIR*)HeapAlloc(StartMenu_Heap, 0, sizeof(MENU_DIR));
    memzero(root, sizeof(MENU_DIR));

    hMenu = BuildTopLevelMenu(root);

    //
    //
    //

    memzero(&wc, sizeof(WNDCLASS));
    wc.lpfnWndProc = StartMenuWndProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = SANDBOXIE L"StartMenuMessageOnlyWindowClass";
    atom = RegisterClass(&wc);

    title = SbieDll_FormatMessage0(MSG_3465);
    titleptr = wcschr(title, L'&');
    if (titleptr)
        memmove(titleptr, titleptr + 1, wcslen(titleptr));

    hWnd = CreateWindowEx(0x00000008, (LPCWSTR)atom, title, 0x94000000,
                          1, 1, 1, 1,
                          NULL, NULL, NULL, NULL);
    ShowWindow(hWnd, SW_SHOW);

    flags = TPM_RETURNCMD | TPM_TOPALIGN | TPM_LEFTBUTTON;
    if (layout_rtl) {
        RECT rect;
        GetWindowRect(GetDesktopWindow(), &rect);
        flags |= TPM_RIGHTALIGN | TPM_LAYOUTRTL;
        x = rect.right - 10;
    } else
        x = 10;

    SetForegroundWindow(hWnd);

    id = (UINT)TrackPopupMenu(hMenu, flags, x, 10, 0, hWnd, NULL);

    //
    //
    //

    if (id)
        target = (WCHAR *)StartMenu_IdMap[id - 1];
    else
        target = NULL;

    if (target) {

        ULONG target_attrs = GetFileAttributes(target);
        if (target_attrs == INVALID_FILE_ATTRIBUTES) {
            WCHAR *path = StartMenu_TempPath;
            if (GET_FOLDER_PATH(CSIDL_STARTMENU)) {
                ULONG path_len = wcslen(path);
                if (_wcsnicmp(path, target, path_len) == 0) {
                    GET_FOLDER_PATH(CSIDL_COMMON_STARTMENU);
                    wcscat(path, target + path_len);
                    target = path;
                }
            }
        }
    }

    if (target) {

        WCHAR *buf = (WCHAR *)HeapAlloc(
                        GetProcessHeap(), HEAP_GENERATE_EXCEPTIONS,
                        10240 * sizeof(WCHAR));
        wcscpy(buf, L"\"");
        wcscat(buf, target);
        wcscat(buf, L"\"");
        target = buf;
    }

    //
    //
    //

    DestroyWindow(hWnd);
    DestroyMenu(hMenu);

    HeapDestroy(StartMenu_Heap);
    StartMenu_Heap = NULL;

    return target;
}


//---------------------------------------------------------------------------
// WriteStartMenuResult
//---------------------------------------------------------------------------


_FX BOOL WriteStartMenuResult(const WCHAR *MapName, const WCHAR *Command)
{
    HANDLE hMapping;
    WCHAR *buf;
    WCHAR *IconPath;
    ULONG len, IconIndex;
    BOOLEAN GetLinkIconPathAndNumber(
        WCHAR *LinkPath, WCHAR **IconPath, ULONG *IconIndex);

    //
    // open shared memory area
    //

    hMapping = OpenFileMapping(FILE_MAP_WRITE, FALSE, MapName);
    if (! hMapping)
        return FALSE;

    buf = (WCHAR *)MapViewOfFile(hMapping, FILE_MAP_WRITE, 0, 0, 8192);
    if (! buf) {
        CloseHandle(hMapping);
        return FALSE;
    }

    //
    // write sandbox
    //

    SbieApi_QueryProcess(NULL, buf, NULL, NULL, NULL);

    if (run_elevated_2)
        buf[63] = L'$';

    //
    // write command without surrounding quote marks
    //

    len = wcslen(Command);
    if (len && Command[0] == L'\"' && Command[len - 1] == L'\"') {
        ++Command;
        len -= 2;
    }
    if (len > 952)
        len = 952;
    wmemcpy(buf + 64, Command, len);

    //
    // write icon information
    //

    if (GetLinkIconPathAndNumber(buf + 64, &IconPath, &IconIndex)) {

        BOOLEAN ok = FALSE;
        if (IconPath[0]) {

            HANDLE hFile = CreateFile(IconPath, FILE_GENERIC_READ,
                FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                NULL, OPEN_EXISTING,
                FILE_ATTRIBUTE_NORMAL | FILE_FLAG_BACKUP_SEMANTICS,
                NULL);
            if (hFile != INVALID_HANDLE_VALUE) {

                BOOLEAN IsBoxedPath;
                if (0 == SbieDll_GetHandlePath(
                                    hFile, buf + 1024, &IsBoxedPath)) {
                    if (SbieDll_TranslateNtToDosPath(buf + 1024)) {

                        *(ULONG *)(buf + 1020) = IconIndex;
                        ok = TRUE;
                    }
                }

                CloseHandle(hFile);
            }
        }

        if (! ok)
            buf[1024] = L'\0';

        if (IconPath[1024]) {
            len = wcslen(&IconPath[1024]);
            if (len > 1000)
                len = 1000;
            wmemcpy(buf + 2048, IconPath + 1024, len);
        }

        HeapFree(GetProcessHeap(), 0, IconPath);
    }

    //
    // finish
    //

    UnmapViewOfFile(buf);
    CloseHandle(hMapping);
    return TRUE;
}
