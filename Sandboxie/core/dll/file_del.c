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
// File (Delete)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Defines
//---------------------------------------------------------------------------

#define FILE_PATH_FILE_NAME     L"FilePaths.dat"

// path flags, saved to file
#define FILE_DELETED_FLAG       0x0001
#define FILE_RELOCATION_FLAG    0x0002

// internal volatile status flags
#define FILE_PATH_DELETED_FLAG      0x00010000
#define FILE_PATH_RELOCATED_FLAG    0x00020000
#define FILE_CHILDREN_DELETED_FLAG  0x00040000

#define FILE_DELETED_MASK   (FILE_DELETED_FLAG | FILE_PATH_DELETED_FLAG) 
#define FILE_RELOCATED_MASK (FILE_RELOCATION_FLAG | FILE_PATH_RELOCATED_FLAG) 

#define FILE_IS_DELETED(x)      ((x & FILE_DELETED_FLAG) != 0)
#define FILE_PATH_DELETED(x)    ((x & FILE_DELETED_MASK) != 0)
#define FILE_PARENT_DELETED(x)  ((x & FILE_PATH_DELETED_FLAG) != 0)
#define FILE_PATH_RELOCATED(x)  ((x & FILE_RELOCATED_MASK) != 0)

//---------------------------------------------------------------------------
// Structures and Types
//---------------------------------------------------------------------------

typedef struct _PATH_NODE {
    LIST_ELEM list_elem;
    LIST items;
    ULONG flags;
    WCHAR* relocation;
    ULONG name_len;
    WCHAR name[1];
} PATH_NODE;


//---------------------------------------------------------------------------
// Variables
//---------------------------------------------------------------------------


static LIST File_PathRoot;
static CRITICAL_SECTION *File_PathRoot_CritSec = NULL;

static HANDLE File_BoxRootWatcher = NULL;

//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------

static ULONG File_GetPathFlags(const WCHAR* Path, WCHAR** pRelocation);
static BOOLEAN File_SavePathTree();
static BOOLEAN File_LoadPathTree();
static VOID File_RefreshPathTree();
BOOLEAN File_InitDelete_v2();

static NTSTATUS File_MarkDeleted_v2(const WCHAR *TruePath);
static ULONG File_IsDeleted_v2(const WCHAR* TruePath);
static BOOLEAN File_HasDeleted_v2(const WCHAR* TruePath);
static WCHAR* File_GetRelocation(const WCHAR* TruePath);
static NTSTATUS File_SetRelocation(const WCHAR *OldTruePath, const WCHAR *NewTruePath);

HANDLE File_AcquireMutex(const WCHAR* MutexName);
void File_ReleaseMutex(HANDLE hMutex);
#define FILE_VFS_MUTEX SBIE L"_VFS_Mutex"

//---------------------------------------------------------------------------
// File_ClearPathBranche
//---------------------------------------------------------------------------


_FX VOID File_ClearPathBranche_internal(LIST* parent)
{
    PATH_NODE* child = List_Head(parent);
    while (child) {

        PATH_NODE* next_child = List_Next(child);

        File_ClearPathBranche_internal(&child->items);

        List_Remove(parent, child);
        if(child->relocation) Dll_Free(child->relocation);
        Dll_Free(child);

        child = next_child;
    }
}


//---------------------------------------------------------------------------
// File_GetPathNode_internal
//---------------------------------------------------------------------------


_FX PATH_NODE* File_GetPathNode_internal(LIST* parent, const WCHAR* name, ULONG name_len, BOOLEAN can_add) 
{
    PATH_NODE* child;
    
    child = List_Head(parent);
    while (child) {

        if (child->name_len == name_len && _wcsnicmp(child->name, name, name_len) == 0)
            break;

        child = List_Next(child);
    }

    if (!child && can_add) {

        child = Dll_Alloc(sizeof(PATH_NODE) + name_len*sizeof(WCHAR));
        memzero(child, sizeof(PATH_NODE));
        //List_Init(child->items); // done by memzero
        child->name_len = name_len;
        wmemcpy(child->name, name, name_len);
        child->name[name_len] = L'\0';

        List_Insert_After(parent, NULL, child);
    }

    return child;
}


//---------------------------------------------------------------------------
// File_FindPathBranche_internal
//---------------------------------------------------------------------------


_FX PATH_NODE* File_FindPathBranche_internal(LIST* Root, const WCHAR* Path, LIST** pParent, BOOLEAN can_add) 
{
    LIST* Parent = Root;
    PATH_NODE* Node;
    const WCHAR* next;
    for (const WCHAR* ptr = Path; *ptr; ptr = next + 1) {
        next = wcschr(ptr, L'\\');
        if (ptr == next) // handle initial \ as well as \\ or \\\ etc cases
            continue;
        if(!next) next = wcschr(ptr, L'\0'); // last
        
        Node = File_GetPathNode_internal(Parent, ptr, (ULONG)(next - ptr), can_add);
        if (!Node)
            return NULL;

        if (*next == L'\0') {
            if (pParent) *pParent = Parent;
            return Node;
        }
        Parent = &Node->items;
    }

    return NULL;
}


//---------------------------------------------------------------------------
// File_SetPathFlags_internal
//---------------------------------------------------------------------------


_FX VOID File_SetPathFlags_internal(LIST* Root, const WCHAR* Path, ULONG setFlags, ULONG clrFlags, const WCHAR* Relocation)
{
    PATH_NODE* Parent = Root;
    PATH_NODE* Node;
    const WCHAR* next;
    for (const WCHAR* ptr = Path; *ptr; ptr = next + 1) {
        next = wcschr(ptr, L'\\');
        if (ptr == next) // handle initial \ as well as \\ or \\\ etc cases
            continue;
        if(!next) next = wcschr(ptr, L'\0'); // last
        
        Node = File_GetPathNode_internal(Parent, ptr, (ULONG)(next - ptr), TRUE);

        if (*next == L'\0') { // set flag always on the last element only

            Node->flags |= setFlags;
            Node->flags &= ~clrFlags;

            if ((clrFlags & FILE_RELOCATION_FLAG) != 0 || (setFlags & FILE_RELOCATION_FLAG) != 0) {
                if (Node->relocation) {
                    Dll_Free(Node->relocation);
                    Node->relocation = NULL;
                }
            }
            if ((setFlags & FILE_RELOCATION_FLAG) != 0 && Relocation != NULL) {
                if (Relocation && wcslen(Relocation) > 0) {
                    Node->relocation = Dll_Alloc((wcslen(Relocation) + 1) * sizeof(WCHAR));
                    wcscpy(Node->relocation, Relocation);
                }
            }

            break;
        }
        Parent = &Node->items;
    }
}


//---------------------------------------------------------------------------
// File_GetPathFlags_internal
//---------------------------------------------------------------------------


_FX ULONG File_GetPathFlags_internal(LIST* Root, const WCHAR* Path, WCHAR** pRelocation, BOOLEAN CheckChildren)
{
    ULONG Flags = 0;
    const WCHAR* Relocation = NULL;
    const WCHAR* SubPath = NULL;

    LIST* Parent = Root;
    PATH_NODE* Node;
    PATH_NODE* child;
    const WCHAR* next;
    for (const WCHAR* ptr = Path; *ptr; ptr = next + 1) {
        next = wcschr(ptr, L'\\');
        if (ptr == next) // handle initial \ as well as \\ or \\\ etc cases
            continue;
        if(!next) next = wcschr(ptr, L'\0'); // last
        
        Node = File_GetPathNode_internal(Parent, ptr, (ULONG)(next - ptr), FALSE);
        if (!Node)
            break;

        //
        // we return the last relocation target
        //

        if ((Node->flags & FILE_RELOCATION_FLAG) != 0) {
            Relocation = Node->relocation;
            SubPath = next;
        }


        if (*next == L'\0') { // last path segment

            if ((Node->flags & FILE_RELOCATION_FLAG) != 0) 
                Flags |= FILE_RELOCATION_FLAG; 
            if ((Node->flags & FILE_DELETED_FLAG) != 0)
                Flags |= FILE_DELETED_FLAG; // flag set for the path

            if (CheckChildren) {
                child = List_Head(&Node->items);
                while (child) {
                    if ((child->flags & Flags) == Flags) {
                        Flags |= FILE_CHILDREN_DELETED_FLAG; // path set for children
                        break;
                    }
                    child = List_Next(child);
                }
            }

            break;
        }

        //
        // if we encounter a relocation previosue deletions on the path will be reset
        // relocations are only allowed to exist for not deleted paths
        //

        if ((Node->flags & FILE_RELOCATION_FLAG) != 0) {
            Flags = 0; // reset
            Flags |= FILE_PATH_RELOCATED_FLAG;
        }
        else if ((Node->flags & FILE_DELETED_FLAG) != 0)
            Flags |= FILE_PATH_DELETED_FLAG; // flag set for ancestor

        Parent = &Node->items;
    }

    if (Relocation && pRelocation) {

        THREAD_DATA *TlsData = Dll_GetTlsData(NULL);

        *pRelocation = Dll_GetTlsNameBuffer(TlsData, MISC_NAME_BUFFER, (wcslen(Relocation) + wcslen(SubPath) + 16) * sizeof(WCHAR)); // +16 some room for changes
        wcscpy(*pRelocation, Relocation);
        wcscat(*pRelocation, SubPath);
    }

    return Flags;
}


//---------------------------------------------------------------------------
// File_GetPathFlags
//---------------------------------------------------------------------------


_FX ULONG File_GetPathFlags(const WCHAR* Path, WCHAR** pRelocation)
{
    ULONG Flags;

    File_RefreshPathTree();

    EnterCriticalSection(File_PathRoot_CritSec);

    Flags = File_GetPathFlags_internal(&File_PathRoot, Path, pRelocation, TRUE);

    LeaveCriticalSection(File_PathRoot_CritSec);

    return Flags;
}


//---------------------------------------------------------------------------
// File_SavePathNode_internal
//---------------------------------------------------------------------------


_FX VOID File_SavePathNode_internal(HANDLE hPathsFile, LIST* parent, WCHAR* Path, ULONG Length, ULONG SetFlags) 
{
    IO_STATUS_BLOCK IoStatusBlock;

    const WCHAR CrLf[] = L"\r\n";
    WCHAR FlagStr[16] = L"|";

    // append  L"\\"
    Path[Length++] = L'\\'; //Path[Length] = L'0';
    WCHAR* PathBase = Path + Length;

    PATH_NODE* child;
    child = List_Head(parent);
    while (child) {

        wmemcpy(PathBase, child->name, child->name_len + 1);
        ULONG Path_Len = Length + child->name_len;

        //
        // don't write down flags that were already set for the parent, 
        // unless we have a relocation, that resets everything
        //

        if ((child->flags & FILE_RELOCATION_FLAG) != 0)
            SetFlags = 0;

        if ((child->flags & ~SetFlags) != 0 || child->relocation != NULL) { 

            // write the path
            NtWriteFile(hPathsFile, NULL, NULL, NULL, &IoStatusBlock, Path, Path_Len * sizeof(WCHAR), NULL, NULL);

            // write the flags
            _ultow(child->flags, FlagStr + 1, 16);
            NtWriteFile(hPathsFile, NULL, NULL, NULL, &IoStatusBlock, FlagStr, wcslen(FlagStr) * sizeof(WCHAR), NULL, NULL);

            // write the relocation
            if (child->relocation != NULL) {
                NtWriteFile(hPathsFile, NULL, NULL, NULL, &IoStatusBlock, FlagStr, sizeof(WCHAR), NULL, NULL); // write |
                NtWriteFile(hPathsFile, NULL, NULL, NULL, &IoStatusBlock, child->relocation, wcslen(child->relocation) * sizeof(WCHAR), NULL, NULL);
            }

            // write line ending
            NtWriteFile(hPathsFile, NULL, NULL, NULL, &IoStatusBlock, (void*)CrLf, sizeof(CrLf) - sizeof(WCHAR), NULL, NULL);
        }

        File_SavePathNode_internal(hPathsFile, &child->items, Path, Path_Len, SetFlags | child->flags);

        child = List_Next(child);
    }
}


//---------------------------------------------------------------------------
// File_SavePathTree_internal
//---------------------------------------------------------------------------


_FX VOID File_SavePathTree_internal(LIST* Root, const WCHAR* name)
{
    WCHAR PathsFile[MAX_PATH] = { 0 };
    wcscpy(PathsFile, Dll_BoxFilePath);
    wcscat(PathsFile, L"\\");
    wcscat(PathsFile, name);
    SbieDll_TranslateNtToDosPath(PathsFile);

    HANDLE hPathsFile;
    hPathsFile = CreateFile(PathsFile, FILE_APPEND_DATA, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hPathsFile == INVALID_HANDLE_VALUE)
        return;
    
    WCHAR* Path = (WCHAR *)Dll_Alloc((0x7FFF + 1)*sizeof(WCHAR)); // max nt path

    File_SavePathNode_internal(hPathsFile, Root, Path, 0, 0);

    Dll_Free(Path);

    CloseHandle(hPathsFile);
}


//---------------------------------------------------------------------------
// File_SavePathTree
//---------------------------------------------------------------------------


_FX BOOLEAN File_SavePathTree()
{
    EnterCriticalSection(File_PathRoot_CritSec);

    File_SavePathTree_internal(&File_PathRoot, FILE_PATH_FILE_NAME);

    LeaveCriticalSection(File_PathRoot_CritSec);

    return TRUE;
}


//---------------------------------------------------------------------------
// File_AcquireMutex
//---------------------------------------------------------------------------


_FX HANDLE File_AcquireMutex(const WCHAR *MutexName)
{
    HANDLE hMutex = OpenMutex(MUTEX_ALL_ACCESS, FALSE, MutexName);
    if (! hMutex)
        hMutex = CreateMutex(NULL, FALSE, MutexName);
    if (hMutex)
        WaitForSingleObject(hMutex, 5000);

    return hMutex;
}


//---------------------------------------------------------------------------
// Scm_ReleaseMutex
//---------------------------------------------------------------------------


_FX void File_ReleaseMutex(HANDLE hMutex)
{
    if (hMutex) {

        ReleaseMutex(hMutex);

        CloseHandle(hMutex);
    }
}


//---------------------------------------------------------------------------
// File_LoadPathTree_internal
//---------------------------------------------------------------------------


_FX BOOLEAN File_LoadPathTree_internal(LIST* Root, const WCHAR* name)
{
    WCHAR PathsFile[MAX_PATH] = { 0 };
    wcscpy(PathsFile, Dll_BoxFilePath);
    wcscat(PathsFile, L"\\");
    wcscat(PathsFile, name);
    SbieDll_TranslateNtToDosPath(PathsFile);

    HANDLE hPathsFile;
    hPathsFile = CreateFile(PathsFile, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hPathsFile == INVALID_HANDLE_VALUE) {
        hPathsFile = CreateFile(PathsFile, FILE_APPEND_DATA, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hPathsFile != INVALID_HANDLE_VALUE)
            CloseHandle(hPathsFile);
        return FALSE;
    }

    File_ClearPathBranche_internal(Root);

    LARGE_INTEGER fileSize;
    GetFileSizeEx(hPathsFile, &fileSize);

    WCHAR* Buffer = (WCHAR *)Dll_Alloc((ULONG)fileSize.QuadPart + 128);
    DWORD bytesRead;
    ReadFile(hPathsFile, Buffer, (DWORD)fileSize.QuadPart, &bytesRead, NULL);
    Buffer[bytesRead/sizeof(WCHAR)] = L'\0';

    WCHAR* Next = Buffer;
    while (*Next) {
        WCHAR* Line = Next;
        WCHAR* End = wcschr(Line, L'\n');
        if (End == NULL) {
            End = wcschr(Line, L'\0');
            Next = End;
        } else
            Next = End + 1;
        LONG LineLen = (LONG)(End - Line);
        if (LineLen > 1 && Line[LineLen - 1] == L'\r')
            LineLen -= 1;
        
        WCHAR savechar = Line[LineLen];
        Line[LineLen] = L'\0';

        WCHAR* Sep = wcschr(Line, L'|');
        if (!Sep || Sep > Next) continue; // invalid line, flags field missing
        *Sep = L'\0';

        WCHAR* endptr;
        ULONG Flags = wcstoul(Sep + 1, &endptr, 16);
        if (endptr && *endptr == L'|') endptr++;
        else endptr = NULL;

        File_SetPathFlags_internal(Root, Line, Flags, 0, endptr);

        *Sep = L'|';
        Line[LineLen] = savechar;
    }

    Dll_Free(Buffer);

    CloseHandle(hPathsFile);

    return TRUE;
}


//---------------------------------------------------------------------------
// File_LoadPathTree
//---------------------------------------------------------------------------


_FX BOOLEAN File_LoadPathTree()
{
    HANDLE hMutex = File_AcquireMutex(FILE_VFS_MUTEX);

    EnterCriticalSection(File_PathRoot_CritSec);

    File_LoadPathTree_internal(&File_PathRoot, FILE_PATH_FILE_NAME);

    LeaveCriticalSection(File_PathRoot_CritSec);

    File_ReleaseMutex(hMutex);

    return TRUE;
}


//---------------------------------------------------------------------------
// File_RefreshPathTree
//---------------------------------------------------------------------------


_FX VOID File_RefreshPathTree()
{
    if (!File_BoxRootWatcher)
        return;

    if (WaitForSingleObject(File_BoxRootWatcher, 0) == WAIT_OBJECT_0) {

        //
        // something changed, reload the path tree
        //

        File_LoadPathTree();

        FindNextChangeNotification(File_BoxRootWatcher); // rearm the watcher
    }
}


//---------------------------------------------------------------------------
// File_InitDelete_v2
//---------------------------------------------------------------------------


_FX BOOLEAN File_InitDelete_v2()
{
    List_Init(&File_PathRoot);

    File_PathRoot_CritSec = Dll_Alloc(sizeof(CRITICAL_SECTION));
    InitializeCriticalSectionAndSpinCount(File_PathRoot_CritSec, 1000);

    File_LoadPathTree();

//#ifdef WITH_DEBUG
//    File_SavePathTree();
//#endif

    WCHAR BoxFilePath[MAX_PATH] = { 0 };
    wcscpy(BoxFilePath, Dll_BoxFilePath);
    SbieDll_TranslateNtToDosPath(BoxFilePath);

    File_BoxRootWatcher = FindFirstChangeNotification(BoxFilePath, FALSE, FILE_NOTIFY_CHANGE_SIZE | FILE_NOTIFY_CHANGE_LAST_WRITE);

    FindNextChangeNotification(File_BoxRootWatcher); // arm the watcher

    return TRUE;
}


//---------------------------------------------------------------------------
// File_MarkDeleted_internal
//---------------------------------------------------------------------------


_FX BOOLEAN File_MarkDeleted_internal(LIST* Root, const WCHAR* Path)
{
    // 1. remove deleted branche

    LIST* Parent = NULL;
    PATH_NODE* Node = File_FindPathBranche_internal(Root, Path, &Parent, FALSE);
    if (Node) {
        if (Node->flags == FILE_DELETED_FLAG && Node->items.count == 0)
            return FALSE; // already marked deleted

        List_Remove(Parent, Node);

        File_ClearPathBranche_internal(&Node->items);
        if (Node->relocation) Dll_Free(Node->relocation);
        Dll_Free(Node);
    }

    // 2. set deleted flag

    File_SetPathFlags_internal(Root, Path, FILE_DELETED_FLAG, 0, NULL);

    // done

    return TRUE;
}


//---------------------------------------------------------------------------
// File_MarkDeleted_v2
//---------------------------------------------------------------------------


_FX NTSTATUS File_MarkDeleted_v2(const WCHAR* TruePath)
{
    //
    // add a file or directory to the deleted list
    //

    HANDLE hMutex = File_AcquireMutex(FILE_VFS_MUTEX);

    EnterCriticalSection(File_PathRoot_CritSec);

    BOOLEAN bSet = File_MarkDeleted_internal(&File_PathRoot, TruePath);

    LeaveCriticalSection(File_PathRoot_CritSec);

    if (bSet) File_SavePathTree();

    File_ReleaseMutex(hMutex);

    return STATUS_SUCCESS;
}


//---------------------------------------------------------------------------
// File_IsDeleted_v2
//---------------------------------------------------------------------------


_FX ULONG File_IsDeleted_v2(const WCHAR* TruePath)
{
    //
    // check if the file or one of its parent directories is listed as deleted
    // use the dedicated test method to properly take into account relocations
    //

    ULONG Flags = File_GetPathFlags(TruePath, NULL);

    return (Flags & FILE_DELETED_MASK);
}


//---------------------------------------------------------------------------
// File_HasDeleted_v2
//---------------------------------------------------------------------------


_FX BOOLEAN File_HasDeleted_v2(const WCHAR* TruePath)
{
    //
    // Check if this folder has deleted children
    //

    ULONG Flags = File_GetPathFlags(TruePath, NULL);

    return (Flags & FILE_CHILDREN_DELETED_FLAG) != 0;
}


//---------------------------------------------------------------------------
// File_SetRelocation_internal
//---------------------------------------------------------------------------


_FX VOID File_SetRelocation_internal(LIST* Root, const WCHAR *OldTruePath, const WCHAR *NewTruePath)
{
    // 1. separate branche from OldTruePath
    
    LIST* Parent = NULL;
    PATH_NODE* Node = File_FindPathBranche_internal(Root, OldTruePath, &Parent, FALSE);
    //if(Node) 
    //    List_Remove(Parent, Node); // leave node in it may have a delete flag

    // 2. check if old path has a relocation

    BOOLEAN HasRelocation = FALSE;
    if (Node && (Node->flags & FILE_RELOCATION_FLAG) != 0) {
        Node->flags &= ~FILE_RELOCATION_FLAG;
        if (Node->relocation) {
            HasRelocation = TRUE;
            OldTruePath = Node->relocation; // update relocation to oritinal true target
        }
    }


    // 3. add true delete entry OldTruePath

    File_SetPathFlags_internal(Root, OldTruePath, FILE_DELETED_FLAG, 0, NULL);


    // 4. set redirection NewTruePath -> OldTruePath

    PATH_NODE* NewNode = File_FindPathBranche_internal(Root, NewTruePath, NULL, TRUE);

    // OldTruePath may have a relocated parent, if so unwrap it
    if (!HasRelocation) {
        WCHAR* OldOldTruePath = NULL;
        File_GetPathFlags_internal(Root, OldTruePath, &OldOldTruePath, TRUE);
        if (OldOldTruePath) OldTruePath = OldOldTruePath;
    }
    
    NewNode->flags |= FILE_RELOCATION_FLAG;
    NewNode->relocation = Dll_Alloc((wcslen(OldTruePath) + 1) * sizeof(WCHAR));
    wcscpy(NewNode->relocation, OldTruePath);
    

    // 5. reatach branche to NewTruePath

    if (Node) {
        PATH_NODE* child = List_Head(&Node->items);
        while (child) {

            PATH_NODE* next_child = List_Next(child);

            List_Remove(&Node->items, child);
                
            List_Insert_After(&NewNode->items, NULL, child);

            child = next_child;
        }
    }
    

    // 6. clean up

    //if (Node) {
    if (HasRelocation) {
        //if (Node->relocation)
        Dll_Free(Node->relocation);
        Node->relocation = NULL;       
        //Dll_Free(Node);
    }
}


//---------------------------------------------------------------------------
// File_SetRelocation
//---------------------------------------------------------------------------


_FX NTSTATUS File_SetRelocation(const WCHAR* OldTruePath, const WCHAR* NewTruePath)
{
    //
    // List a mapping for the new location
    //

    HANDLE hMutex = File_AcquireMutex(FILE_VFS_MUTEX);

    EnterCriticalSection(File_PathRoot_CritSec);

    File_SetRelocation_internal(&File_PathRoot, OldTruePath, NewTruePath);

    LeaveCriticalSection(File_PathRoot_CritSec);

    File_SavePathTree();

    File_ReleaseMutex(hMutex);

    return STATUS_SUCCESS;
}


//---------------------------------------------------------------------------
// File_GetRelocation
//---------------------------------------------------------------------------


_FX WCHAR* File_GetRelocation(const WCHAR *TruePath)
{
    //
    // Get redirection location, only if its the actual path and not a parent
    // 

    WCHAR* OldTruePath = NULL;
    ULONG Flags = File_GetPathFlags(TruePath, &OldTruePath);
    if (FILE_PATH_RELOCATED(Flags))
        return OldTruePath;

    return NULL;
}

