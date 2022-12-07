/*
 * Copyright 2022 David Xanatos, xanasoft.com
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

#include "../../common/my_version.h"

//---------------------------------------------------------------------------
// Key (Delete)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Defines
//---------------------------------------------------------------------------

#define KEY_PATH_FILE_NAME      L"RegPaths.dat"

// Keep in sync with the FILE_..._FLAG's in file_del.c
// 
// path flags, saved to file
#define KEY_DELETED_FLAG       0x0001
#define KEY_RELOCATION_FLAG    0x0002

// internal volatile status flags
#define KEY_PATH_DELETED_FLAG      0x00010000
#define KEY_PATH_RELOCATED_FLAG    0x00020000
#define KEY_CHILDREN_DELETED_FLAG  0x00040000

#define KEY_DELETED_MASK    (KEY_DELETED_FLAG | KEY_PATH_DELETED_FLAG) 
#define KEY_RELOCATED_MASK  (KEY_RELOCATION_FLAG | KEY_PATH_RELOCATED_FLAG) 

#define KEY_IS_DELETED(x)       ((x & KEY_DELETED_FLAG) != 0)
#define KEY_PATH_DELETED(x)     ((x & KEY_DELETED_MASK) != 0)
#define KEY_PARENT_DELETED(x)   ((x & KEY_PATH_DELETED_FLAG) != 0)
#define KEY_PATH_RELOCATED(x)   ((x & KEY_RELOCATED_MASK) != 0)


//---------------------------------------------------------------------------
// Variables
//---------------------------------------------------------------------------


static LIST Key_PathRoot;
static CRITICAL_SECTION *Key_PathRoot_CritSec = NULL;

BOOLEAN Key_RegPaths_Loaded = FALSE;

static HANDLE Key_BoxRootWatcher = NULL;
static volatile ULONGLONG Key_PathsVersion = 0; // count reloads


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------

static ULONG Key_GetPathFlags(const WCHAR* Path, WCHAR** pRelocation);
static BOOLEAN Key_SavePathTree();
static BOOLEAN Key_LoadPathTree();
static VOID Key_RefreshPathTree();
BOOLEAN Key_InitDelete_v2();
static NTSTATUS Key_MarkDeletedEx_v2(const WCHAR* TruePath, const WCHAR* ValueName);
static ULONG Key_IsDeleted_v2(const WCHAR* TruePath);
static ULONG Key_IsDeletedEx_v2(const WCHAR* TruePath, const WCHAR* ValueName, BOOLEAN IsValue);

//
// we re use the _internal functions of the file implementation as they all are generic enough
//

VOID File_ClearPathBranche_internal(LIST* parent);
VOID File_SavePathTree_internal(LIST* Root, const WCHAR* name);
BOOLEAN File_LoadPathTree_internal(LIST* Root, const WCHAR* name);
VOID File_SetPathFlags_internal(LIST* Root, const WCHAR* Path, ULONG setFlags, ULONG clrFlags, const WCHAR* Relocation);
ULONG File_GetPathFlags_internal(LIST* Root, const WCHAR* Path, WCHAR** pRelocation, BOOLEAN CheckChildren);
VOID File_SavePathNode_internal(HANDLE hPathsFile, LIST* parent, WCHAR* Path, ULONG Length, ULONG SetFlags);
BOOLEAN File_MarkDeleted_internal(LIST* Root, const WCHAR* Path);
VOID File_SetRelocation_internal(LIST* Root, const WCHAR* OldTruePath, const WCHAR* NewTruePath);

HANDLE File_AcquireMutex(const WCHAR* MutexName);
void File_ReleaseMutex(HANDLE hMutex);
#define KEY_VCM_MUTEX SBIE L"_VCM_Mutex"


//---------------------------------------------------------------------------
// Key_GetPathFlags
//---------------------------------------------------------------------------


_FX ULONG Key_GetPathFlags(const WCHAR* Path, WCHAR** pRelocation)
{
    ULONG Flags;

    Key_RefreshPathTree();

    EnterCriticalSection(Key_PathRoot_CritSec);

    Flags = File_GetPathFlags_internal(&Key_PathRoot, Path, pRelocation, TRUE);

    LeaveCriticalSection(Key_PathRoot_CritSec);

    return Flags;
}


//---------------------------------------------------------------------------
// Key_SavePathTree
//---------------------------------------------------------------------------


_FX BOOLEAN Key_SavePathTree()
{
    EnterCriticalSection(Key_PathRoot_CritSec);

    File_SavePathTree_internal(&Key_PathRoot, KEY_PATH_FILE_NAME);

    LeaveCriticalSection(Key_PathRoot_CritSec);

    Key_PathsVersion++;

    return TRUE;
}


//---------------------------------------------------------------------------
// Key_LoadPathTree
//---------------------------------------------------------------------------


_FX BOOLEAN Key_LoadPathTree()
{
    HANDLE hMutex = File_AcquireMutex(KEY_VCM_MUTEX);

    EnterCriticalSection(Key_PathRoot_CritSec);

    Key_RegPaths_Loaded = File_LoadPathTree_internal(&Key_PathRoot, KEY_PATH_FILE_NAME);

    LeaveCriticalSection(Key_PathRoot_CritSec);
    
    File_ReleaseMutex(hMutex);

    Key_PathsVersion++;

    return TRUE;
}


//---------------------------------------------------------------------------
// Key_RefreshPathTree
//---------------------------------------------------------------------------


_FX VOID Key_RefreshPathTree()
{
    if (!Key_BoxRootWatcher)
        return;

    if (WaitForSingleObject(Key_BoxRootWatcher, 0) == WAIT_OBJECT_0) {

        //
        // something changed, reload the path tree
        //

        Key_LoadPathTree();

        FindNextChangeNotification(Key_BoxRootWatcher); // rearm the watcher
    }
}


//---------------------------------------------------------------------------
// Key_InitDelete_v2
//---------------------------------------------------------------------------


_FX BOOLEAN Key_InitDelete_v2()
{
    List_Init(&Key_PathRoot);

    Key_PathRoot_CritSec = Dll_Alloc(sizeof(CRITICAL_SECTION));
    InitializeCriticalSectionAndSpinCount(Key_PathRoot_CritSec, 1000);

    Key_LoadPathTree();

//#ifdef WITH_DEBUG
//    Key_SavePathTree();
//#endif

    WCHAR BoxFilePath[MAX_PATH] = { 0 };
    wcscpy(BoxFilePath, Dll_BoxFilePath);
    SbieDll_TranslateNtToDosPath(BoxFilePath);

    Key_BoxRootWatcher = FindFirstChangeNotification(BoxFilePath, FALSE, FILE_NOTIFY_CHANGE_SIZE | FILE_NOTIFY_CHANGE_LAST_WRITE);

    FindNextChangeNotification(Key_BoxRootWatcher); // arm the watcher

    return TRUE;
}


//---------------------------------------------------------------------------
// Key_MarkDeletedEx_v2
//---------------------------------------------------------------------------


_FX NTSTATUS Key_MarkDeletedEx_v2(const WCHAR* TruePath, const WCHAR* ValueName)
{
    //
    // add a key/value or directory to the deleted list
    //

    HANDLE hMutex = File_AcquireMutex(KEY_VCM_MUTEX);

    THREAD_DATA *TlsData = Dll_GetTlsData(NULL);

    WCHAR* FullPath = Dll_GetTlsNameBuffer(TlsData, TMPL_NAME_BUFFER, 
        (wcslen(TruePath) + (ValueName ? wcslen(ValueName) : 0) + 16) * sizeof(WCHAR)); // template buffer is not used for reg repurpose it here

    wcscpy(FullPath, TruePath);
    if (ValueName) {
        wcscat(FullPath, L"\\$");
        wcscat(FullPath, ValueName);
    }

    EnterCriticalSection(Key_PathRoot_CritSec);

    BOOLEAN bSet = File_MarkDeleted_internal(&Key_PathRoot, FullPath);

    LeaveCriticalSection(Key_PathRoot_CritSec);

    if (bSet) Key_SavePathTree();

    File_ReleaseMutex(hMutex);

    return STATUS_SUCCESS;
}


//---------------------------------------------------------------------------
// Key_IsDeleted_v2
//---------------------------------------------------------------------------


_FX ULONG Key_IsDeleted_v2(const WCHAR* TruePath)
{
    //
    // check if the key/value or one of its parent directories is listed as deleted
    //

    ULONG Flags = Key_GetPathFlags(TruePath, NULL);

    return (Flags & KEY_DELETED_MASK);
}


//---------------------------------------------------------------------------
// Key_IsDeletedEx_v2
//---------------------------------------------------------------------------


_FX ULONG Key_IsDeletedEx_v2(const WCHAR* TruePath, const WCHAR* ValueName, BOOLEAN IsValue)
{
    //
    // check if the key/value or one of its parent directories is listed as deleted
    //

    THREAD_DATA *TlsData = Dll_GetTlsData(NULL);

    WCHAR* FullPath = Dll_GetTlsNameBuffer(TlsData, TMPL_NAME_BUFFER, 
        (wcslen(TruePath) + (ValueName ? wcslen(ValueName) : 0) + 16) * sizeof(WCHAR)); // template buffer is not used for reg repurpose it here

    wcscpy(FullPath, TruePath);
    if (ValueName) {
        wcscat(FullPath, IsValue ? L"\\$" : L"\\");
        wcscat(FullPath, ValueName);
    }

    return Key_IsDeleted_v2(FullPath);
}


//---------------------------------------------------------------------------
// Key_HasDeleted_v2
//---------------------------------------------------------------------------


_FX BOOLEAN Key_HasDeleted_v2(const WCHAR* TruePath)
{
    //
    // Check if this folder has deleted children
    //

    ULONG Flags = Key_GetPathFlags(TruePath, NULL);

    return (Flags & KEY_CHILDREN_DELETED_FLAG) != 0;
}


//---------------------------------------------------------------------------
// Key_SetRelocation
//---------------------------------------------------------------------------


_FX NTSTATUS Key_SetRelocation(const WCHAR *OldTruePath, const WCHAR *NewTruePath)
{
    //
    // List a mapping for the new location
    //
    
    HANDLE hMutex = File_AcquireMutex(KEY_VCM_MUTEX);

    EnterCriticalSection(Key_PathRoot_CritSec);

    File_SetRelocation_internal(&Key_PathRoot, OldTruePath, NewTruePath);

    LeaveCriticalSection(Key_PathRoot_CritSec);

    Key_SavePathTree();

    File_ReleaseMutex(hMutex);

    return STATUS_SUCCESS;
}


//---------------------------------------------------------------------------
// Key_GetRelocation
//---------------------------------------------------------------------------


_FX WCHAR* Key_GetRelocation(const WCHAR *TruePath)
{
    //
    // Get redirection location, only if its the actual path and not a parent
    // 

    WCHAR* OldTruePath = NULL;
    ULONG Flags = Key_GetPathFlags(TruePath, &OldTruePath);
    if (KEY_PATH_RELOCATED(Flags))
        return OldTruePath;

    return NULL;
}


//---------------------------------------------------------------------------
// Key_ResolveTruePath
//---------------------------------------------------------------------------


_FX WCHAR* Key_ResolveTruePath(const WCHAR *TruePath, ULONG* PathFlags)
{
    //
    // Resolve the true path, taking into account redirection locations of parent folder
    // 

    WCHAR* OldTruePath = NULL;
    ULONG Flags = Key_GetPathFlags(TruePath, &OldTruePath);
    if (PathFlags) *PathFlags = Flags;

    return OldTruePath;
}

