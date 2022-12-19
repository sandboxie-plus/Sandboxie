/*
 * Copyright 2004-2020 Sandboxie Holdings, LLC 
 * Copyright 2021-2022 David Xanatos, xanasoft.com
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
// Key
//---------------------------------------------------------------------------


#define NOGDI
#include "dll.h"
#include "obj.h"
#include "handle.h"
#include "core/svc/FileWire.h"
#include "core/drv/api_defs.h"
#include <stdio.h>
#include "debug.h"

//---------------------------------------------------------------------------
// Defines
//---------------------------------------------------------------------------


#define KEY_DENIED_ACCESS                                       \
    ~(STANDARD_RIGHTS_READ | GENERIC_READ | SYNCHRONIZE |       \
      KEY_QUERY_VALUE | KEY_ENUMERATE_SUB_KEYS | KEY_NOTIFY |   \
      KEY_WOW64_32KEY | KEY_WOW64_64KEY)

#define KEY_WOW64_FLAGS

#define DELETE_MARK_LOW   0xDEAD44A0
#define DELETE_MARK_HIGH  0x01B01234

#define IS_DELETE_MARK(p_large_integer)                 \
    ((p_large_integer)->HighPart == DELETE_MARK_HIGH && \
     (p_large_integer)->LowPart  == DELETE_MARK_LOW)


#define OBJECT_ATTRIBUTES_ATTRIBUTES                            \
    (ObjectAttributes                                           \
        ? ObjectAttributes->Attributes | OBJ_CASE_INSENSITIVE   \
        : 0)


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------


SBIEDLL_EXPORT NTSTATUS Key_GetName(
    HANDLE RootDirectory, UNICODE_STRING *ObjectName,
    WCHAR **OutTruePath, WCHAR **OutCopyPath, BOOLEAN *OutIsBoxedPath);

static NTSTATUS Key_FixNameWow64(
    ACCESS_MASK DesiredAccess, WCHAR **OutTruePath, WCHAR **OutCopyPath);

static NTSTATUS Key_FixNameWow64_2(
    WCHAR **OutTruePath, WCHAR **OutCopyPath);

static BOOLEAN Key_FixNameWow64_3(WCHAR **OutPath);

static ACCESS_MASK Key_GetWow64Flag(
    const WCHAR *TruePath, ACCESS_MASK DesiredAccess);

static NTSTATUS Key_NtOpenKey(
    HANDLE *KeyHandle,
    ACCESS_MASK DesiredAccess,
    OBJECT_ATTRIBUTES *ObjectAttributes);

static NTSTATUS Key_NtOpenKeyImpl(
    HANDLE *KeyHandle,
    ACCESS_MASK DesiredAccess,
    OBJECT_ATTRIBUTES *ObjectAttributes);

static NTSTATUS Key_NtOpenKeyEx(
    HANDLE *KeyHandle,
    ACCESS_MASK DesiredAccess,
    OBJECT_ATTRIBUTES *ObjectAttributes,
    ULONG OpenOptions);

static NTSTATUS Key_NtCreateKey(
    HANDLE *KeyHandle,
    ACCESS_MASK DesiredAccess,
    OBJECT_ATTRIBUTES *ObjectAttributes,
    ULONG TitleIndex,
    UNICODE_STRING *Class,
    ULONG CreateOptions,
    ULONG *Disposition);

static NTSTATUS Key_NtCreateKeyImpl(
    HANDLE *KeyHandle,
    ACCESS_MASK DesiredAccess,
    OBJECT_ATTRIBUTES *ObjectAttributes,
    ULONG TitleIndex,
    UNICODE_STRING *Class,
    ULONG CreateOptions,
    ULONG *Disposition);

static NTSTATUS Key_CreatePath(
    OBJECT_ATTRIBUTES *objattrs, PSECURITY_DESCRIPTOR *sd);

static NTSTATUS Key_CreatePath_Key(
    HANDLE *out_handle, OBJECT_ATTRIBUTES *objattrs, ULONG *out_disp);

static BOOLEAN Key_CheckDeletedKey(HANDLE KeyHandle);

static BOOLEAN Key_CheckDeletedParent(WCHAR *CopyPath);

static ULONG Key_CheckDepthForIsWritePath(const WCHAR *TruePath);

static NTSTATUS Key_NtDeleteKey(HANDLE KeyHandle);

static NTSTATUS Key_NtDeleteValueKey(
    HANDLE KeyHandle, UNICODE_STRING *ValueName);

static NTSTATUS Key_NtSetValueKey(
    HANDLE KeyHandle,
    UNICODE_STRING *ValueName,
    ULONG TitleIndex,
    ULONG Type,
    void *Data,
    ULONG DataSize);

static NTSTATUS Key_NtQueryKey(
    HANDLE KeyHandle,
    KEY_INFORMATION_CLASS KeyInformationClass,
    void *KeyInformation,
    ULONG Length,
    ULONG *ResultLength);

static NTSTATUS Key_NtQueryKeyImpl(
    HANDLE KeyHandle,
    KEY_INFORMATION_CLASS KeyInformationClass,
    void *KeyInformation,
    ULONG Length,
    ULONG *ResultLength);

static NTSTATUS Key_NtEnumerateKey(
    HANDLE KeyHandle,
    ULONG Index,
    KEY_INFORMATION_CLASS KeyInformationClass,
    void *KeyInformation,
    ULONG Length,
    ULONG *ResultLength);

static NTSTATUS Key_NtEnumerateKeyFake(
    const WCHAR *KeyPath,
    ULONG KeyPathLen,
    KEY_INFORMATION_CLASS KeyInformationClass,
    void *KeyInformation,
    ULONG Length,
    ULONG *ResultLength,
    LARGE_INTEGER *LastWriteTime);

static NTSTATUS Key_NtQueryValueKey(
    HANDLE KeyHandle,
    UNICODE_STRING *ValueName,
    KEY_VALUE_INFORMATION_CLASS KeyValueInformationClass,
    void *KeyValueInformation,
    ULONG Length,
    ULONG *ResultLength);

static NTSTATUS Key_NtQueryValueKeyFakeForInternetExplorer(
    const WCHAR *TruePath,
    const WCHAR *ValueNameBuf,
    ULONG ValueNameLen,
    void *OutputBuf,
    ULONG OutputLen,
    ULONG *ResultLength);

static NTSTATUS Key_NtQueryValueKeyFakeForAcrobatReader(
    const WCHAR *TruePath,
    const WCHAR *ValueNameBuf,
    ULONG ValueNameLen,
    void *OutputBuf,
    ULONG OutputLen,
    ULONG *ResultLength);

static NTSTATUS Key_NtQueryValueKeyFakeForCreateProcess(
    const WCHAR *TruePath,
    const WCHAR *ValueNameBuf,
    ULONG ValueNameLen,
    void *OutputBuf,
    ULONG OutputLen,
    ULONG *ResultLength);

static NTSTATUS Key_NtEnumerateValueKey(
    HANDLE KeyHandle,
    ULONG Index,
    KEY_VALUE_INFORMATION_CLASS KeyValueInformationClass,
    void *KeyValueInformation,
    ULONG Length,
    ULONG *ResultLength);

static BOOLEAN Key_CheckDeletedValue(
    KEY_VALUE_INFORMATION_CLASS KeyValueInformationClass, void *info);

static NTSTATUS Key_NtQueryMultipleValueKey(
    HANDLE KeyHandle,
    PKEY_VALUE_ENTRY ValueEntries,
    ULONG EntryCount,
    void *ValueBuffer,
    ULONG *Length,
    ULONG *ResultLength);

static NTSTATUS Key_MarkUnDeleted(HANDLE KeyHandle);

static NTSTATUS Key_NtNotifyChangeKey(
    HANDLE KeyHandle,
    HANDLE Event OPTIONAL,
    PIO_APC_ROUTINE ApcRoutine OPTIONAL,
    PVOID ApcContext OPTIONAL,
    PIO_STATUS_BLOCK IoStatusBlock,
    ULONG CompletionFilter,
    BOOLEAN WatchTree,
    PVOID Buffer,
    ULONG BufferSize,
    BOOLEAN Asynchronous);

static NTSTATUS Key_NtNotifyChangeMultipleKeys(
    HANDLE MasterKeyHandle,
    ULONG Count,
    OBJECT_ATTRIBUTES *SlaveObjects,
    HANDLE Event OPTIONAL,
    PIO_APC_ROUTINE ApcRoutine OPTIONAL,
    PVOID ApcContext OPTIONAL,
    PIO_STATUS_BLOCK IoStatusBlock,
    ULONG CompletionFilter,
    BOOLEAN WatchTree,
    PVOID Buffer,
    ULONG BufferSize,
    BOOLEAN Asynchronous);

static NTSTATUS Key_NtRenameKey(
    HANDLE KeyHandle, UNICODE_STRING *ReplacementName);


static NTSTATUS Key_NtSaveKey(
    HANDLE KeyHandle, HANDLE FileHandle);

static NTSTATUS Key_NtSaveKeyEx(
    HANDLE KeyHandle, HANDLE FileHandle, ULONG Flags);

static NTSTATUS Key_NtLoadKey(
    OBJECT_ATTRIBUTES *TargetObjectAttributes,
    OBJECT_ATTRIBUTES *SourceObjectAttributes);

static NTSTATUS Key_NtLoadKey2(
    OBJECT_ATTRIBUTES *TargetObjectAttributes,
    OBJECT_ATTRIBUTES *SourceObjectAttributes, ULONG Flags);

static NTSTATUS Key_NtLoadKey3(
    OBJECT_ATTRIBUTES *TargetObjectAttributes,
    OBJECT_ATTRIBUTES *SourceObjectAttributes, ULONG Flags,
    PVOID LoadArguments, ULONG LoadArgumentCount, ACCESS_MASK DesiredAccess, HANDLE KeyHandle, ULONG Unknown);

static NTSTATUS Key_NtLoadKeyEx(
    OBJECT_ATTRIBUTES *TargetObjectAttributes,
    OBJECT_ATTRIBUTES *SourceObjectAttributes, ULONG Flags,
    HANDLE TrustClassKey, PVOID Reserved, PVOID ObjectContext, PVOID CallbackReserved, PIO_STATUS_BLOCK IoStatusBlock);


NTSTATUS File_NtCloseImpl(HANDLE FileHandle);

//---------------------------------------------------------------------------


       P_NtOpenKey                  __sys_NtOpenKey                 = NULL;
static P_NtOpenKeyEx                __sys_NtOpenKeyEx               = NULL;
static P_NtCreateKey                __sys_NtCreateKey               = NULL;
static P_NtDeleteKey                __sys_NtDeleteKey               = NULL;
static P_NtDeleteValueKey           __sys_NtDeleteValueKey          = NULL;
static P_NtSetValueKey              __sys_NtSetValueKey             = NULL;
static P_NtQueryKey                 __sys_NtQueryKey                = NULL;
static P_NtEnumerateKey             __sys_NtEnumerateKey            = NULL;
static P_NtQueryValueKey            __sys_NtQueryValueKey           = NULL;
       P_NtEnumerateValueKey        __sys_NtEnumerateValueKey       = NULL;
static P_NtQueryMultipleValueKey    __sys_NtQueryMultipleValueKey   = NULL;
static P_NtNotifyChangeKey          __sys_NtNotifyChangeKey         = NULL;
static P_NtNotifyChangeMultipleKeys __sys_NtNotifyChangeMultipleKeys= NULL;
static P_NtRenameKey                __sys_NtRenameKey               = NULL;

static P_NtSaveKey                  __sys_NtSaveKey                 = NULL;
static P_NtSaveKeyEx                __sys_NtSaveKeyEx               = NULL;
static P_NtLoadKey                  __sys_NtLoadKey                 = NULL;
static P_NtLoadKey2                 __sys_NtLoadKey2                = NULL;
static P_NtLoadKey3                 __sys_NtLoadKey3                = NULL;
static P_NtLoadKeyEx                __sys_NtLoadKeyEx               = NULL;


//---------------------------------------------------------------------------
// Variables
//---------------------------------------------------------------------------


static WCHAR *Key_CurrentUser = NULL;
static ULONG Key_CurrentUserLen = 0;

       const WCHAR *Key_Registry = L"\\registry";
       const ULONG Key_RegistryLen = 9;

       const WCHAR *Key_UserCurrent = L"\\user\\current";
       const ULONG Key_UserCurrentLen = 13;

static const WCHAR *Key_System = L"\\registry\\machine\\system";
static const ULONG Key_System_Len = 24;

static const WCHAR *Key_ClickToRun = L"\\registry\\machine\\SOFTWARE\\Microsoft\\Office\\15.0\\ClickToRun\\REGISTRY";
static const ULONG Key_ClickToRunLen = 68;

static const WCHAR *Key_ClickToRun_v16 = L"\\registry\\machine\\SOFTWARE\\Microsoft\\Office\\ClickToRun\\REGISTRY";
static const ULONG Key_ClickToRunLen_v16 = 63;


static const WCHAR *Key_Wow6432Node = L"\\Wow6432Node\\";


static BOOLEAN Key_UseObjectNames = FALSE;

BOOLEAN Key_Delete_v2 = FALSE;

//---------------------------------------------------------------------------
// Debug Prints
//---------------------------------------------------------------------------


#if 0
#define PRINT_STATUS(x) if (Key_Trace) { \
    WCHAR status_txt[64];                                   \
    Sbie_snwprintf(status_txt, 64, L#x L" - %08X\n", status);         \
    OutputDebugString(status_txt);                          \
    }
#else
#define PRINT_STATUS(x)
#endif


#if 0
#define PRINT_PATH(title,ptrstr) if (Key_Trace) { \
    WCHAR path_txt[256];                                    \
    Sbie_snwprintf(path_txt, 256, title L"%-200.200s\n", *ptrstr);     \
    OutputDebugString(path_txt);                            \
    }
#else
#define PRINT_PATH(title,ptrstr)
#endif


//---------------------------------------------------------------------------
// Key (other modules)
//---------------------------------------------------------------------------


#include "key_del.c"
#include "key_merge.c"
#include "key_util.c"



//---------------------------------------------------------------------------
// Key_Init
//---------------------------------------------------------------------------

_FX BOOLEAN Key_Init(void)
{
    HMODULE module = Dll_Ntdll;

    InitializeCriticalSection(&Key_Handles_CritSec);

    SbieDll_MatchPath(L'k', (const WCHAR *)-1);

    Key_UseObjectNames = SbieApi_QueryConfBool(NULL, L"UseObjectNameForKeys", FALSE);

    List_Init(&Key_Handles);
    List_Init(&Key_MergeCacheList);

    //
    // initialize the registry prefix for the current user key:
    // \REGISTRY\USER\S-x-y
    //

    Key_CurrentUserLen = Key_RegistryLen
                       + 6 /* \user\ */
                       + wcslen(Dll_SidString);
    Key_CurrentUser = Dll_Alloc((Key_CurrentUserLen + 1) * sizeof(WCHAR));
    wcscpy(Key_CurrentUser, Key_Registry);
    wcscat(Key_CurrentUser, L"\\user\\");
    wcscat(Key_CurrentUser, Dll_SidString);

    //
    // intercept NTDLL entry points
    //

    SBIEDLL_HOOK(Key_,NtCreateKey);
    SBIEDLL_HOOK(Key_,NtOpenKey);
    SBIEDLL_HOOK(Key_,NtDeleteKey);
    SBIEDLL_HOOK(Key_,NtDeleteValueKey);
    SBIEDLL_HOOK(Key_,NtSetValueKey);
    SBIEDLL_HOOK(Key_,NtQueryKey);
    SBIEDLL_HOOK(Key_,NtEnumerateKey);
    SBIEDLL_HOOK(Key_,NtQueryValueKey);
    SBIEDLL_HOOK(Key_,NtEnumerateValueKey);
    SBIEDLL_HOOK(Key_,NtQueryMultipleValueKey);
    SBIEDLL_HOOK(Key_,NtNotifyChangeKey);
    SBIEDLL_HOOK(Key_,NtNotifyChangeMultipleKeys);

    void* NtRenameKey = GetProcAddress(Dll_Ntdll, "NtRenameKey");
    if (NtRenameKey) { // Windows XP
        SBIEDLL_HOOK(Key_,NtRenameKey);
    }

    void* NtOpenKeyEx = GetProcAddress(Dll_Ntdll, "NtOpenKeyEx");
    if (NtOpenKeyEx) { // windows server 2008 R2
        SBIEDLL_HOOK(Key_, NtOpenKeyEx);
    }

    
    SBIEDLL_HOOK(Key_, NtSaveKey);
    
    void* NtSaveKeyEx = GetProcAddress(Dll_Ntdll, "NtSaveKeyEx");
    if (NtSaveKeyEx) { // Windows XP
        SBIEDLL_HOOK(Key_,NtSaveKeyEx);
    }

    SBIEDLL_HOOK(Key_, NtLoadKey);
    SBIEDLL_HOOK(Key_, NtLoadKey2);

    void* NtLoadKey3 = GetProcAddress(Dll_Ntdll, "NtLoadKey3");
    if (NtLoadKey3) { // Windows 10 2004
        SBIEDLL_HOOK(Key_,NtLoadKey3);
    }

    void* NtLoadKeyEx = GetProcAddress(Dll_Ntdll, "NtLoadKeyEx");
    if (NtLoadKeyEx) { // Windows Server 2003 
        SBIEDLL_HOOK(Key_,NtLoadKeyEx);
    }
    
    return TRUE;
}


//---------------------------------------------------------------------------
// Key_GetName
//---------------------------------------------------------------------------


_FX NTSTATUS Key_GetName(
    HANDLE RootDirectory, UNICODE_STRING *ObjectName,
    WCHAR **OutTruePath, WCHAR **OutCopyPath, BOOLEAN *OutIsBoxedPath)
{
    static const WCHAR *_bfe =
        L"\\registry\\machine\\system\\CurrentControlSet\\services\\bfe";
    static const WCHAR *_ControlSet =
        L"\\registry\\machine\\system\\ControlSet";     // 35
    static const WCHAR *_SideBySide =
        L"\\registry\\machine\\software"                // 70
            L"\\Microsoft\\Windows\\CurrentVersion\\SideBySide";

    THREAD_DATA *TlsData = Dll_GetTlsData(NULL);

    NTSTATUS status;
    ULONG length;
    WCHAR *name;
    ULONG objname_len;
    BOOLEAN is_boxed_path;

    *OutTruePath = NULL;
    *OutCopyPath = NULL;
    if (OutIsBoxedPath)
        *OutIsBoxedPath = FALSE;

    if (ObjectName)
        objname_len = ObjectName->Length & ~1;
    else
        objname_len = 0;

    //
    // if a root handle is specified, we query the full name of the
    // root key, and append the ObjectName
    //

    if (RootDirectory) {

        length = 256;
        name = Dll_GetTlsNameBuffer(
                        TlsData, TRUE_NAME_BUFFER, length + objname_len);

        if (Key_UseObjectNames)
        {
            //
            // Note: some keys like L"\\REGISTRY\\MACHINE\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Perflib\\007"
            // do not return valid results when querying using NtQueryKey and returns a STATUS_INVALID_HANDLE
            // using Obj_GetObjectName seems to break SBIE on Windows 10 1803, strangely on W7 it is fine
            // hence this alternative access mode is made optional
            //

            status = Obj_GetObjectName(RootDirectory, name, &length);

            if (status == STATUS_BUFFER_OVERFLOW || status == STATUS_BUFFER_TOO_SMALL || status == STATUS_INFO_LENGTH_MISMATCH) {

                name = Dll_GetTlsNameBuffer(
                    TlsData, TRUE_NAME_BUFFER, length + objname_len);

                status = Obj_GetObjectName(RootDirectory, name, &length);
            }

            if (!NT_SUCCESS(status))
                return status;

            *OutTruePath = ((OBJECT_NAME_INFORMATION*)name)->Name.Buffer;

            name = (*OutTruePath)
                + ((OBJECT_NAME_INFORMATION*)name)->Name.Length / sizeof(WCHAR);
        }
        else
        {
            status = __sys_NtQueryKey(
                RootDirectory, KeyNameInformation, name, length, &length);

            if (status == STATUS_BUFFER_OVERFLOW || status == STATUS_BUFFER_TOO_SMALL || status == STATUS_INFO_LENGTH_MISMATCH) {

                name = Dll_GetTlsNameBuffer(
                            TlsData, TRUE_NAME_BUFFER, length + objname_len);

                status = __sys_NtQueryKey(
                    RootDirectory, KeyNameInformation, name, length, &length);
            }

            if (! NT_SUCCESS(status))
                return status;

            *OutTruePath = ((KEY_NAME_INFORMATION *)name)->Name;

            name = (*OutTruePath)
                 + ((KEY_NAME_INFORMATION *)name)->NameLength / sizeof(WCHAR);
        }

        if (objname_len) {

            *name = L'\\';
            ++name;
            memcpy(name, ObjectName->Buffer, objname_len);

            name += objname_len / sizeof(WCHAR);
        }

        *name = L'\0';

    //
    // if no root handle, then we simply copy the object name,
    // if one was provided
    //

    } else if (objname_len) {

        name = Dll_GetTlsNameBuffer(TlsData, TRUE_NAME_BUFFER, objname_len);

        *OutTruePath = name;

        memcpy(name, ObjectName->Buffer, objname_len);

        name += objname_len / sizeof(WCHAR);
        *name = L'\0';

    //
    // if no root handle, and no object name, then abort
    //

    } else
        return STATUS_OBJECT_PATH_SYNTAX_BAD;

    //
    // make sure the true path begins with the "\registry" prefix.
    // note that Windows returns more informative status codes here,
    // like STATUS_OBJECT_NAME_NOT_FOUND, STATUS_OBJECT_PATH_NOT_FOUND
    // and STATUS_OBJECT_TYPE_MISMATCH.  but we take the easy way out
    //

    length = wcslen(*OutTruePath);

    if (length < Key_RegistryLen ||
            _wcsnicmp(*OutTruePath, Key_Registry, Key_RegistryLen) != 0)
    {
        return STATUS_OBJECT_PATH_SYNTAX_BAD;
    }

    if ((*OutTruePath)[Key_RegistryLen] != L'\\' &&
        (*OutTruePath)[Key_RegistryLen] != L'\0')
    {
        return STATUS_OBJECT_PATH_SYNTAX_BAD;
    }

    //
    // remove trailing and duplicate backslashes
    //

    name = *OutTruePath;

    if (name[length - 1] == L'\\') {
        --length;
        name[length] = L'\0';
    }

    while (name[0]) {
        if (name[0] == L'\\' && name[1] == L'\\') {

            ULONG move_len = length - (ULONG)(name - *OutTruePath) + 1;
            wmemmove(name, name + 1, move_len);
            --length;

        } else
            ++name;
    }

    //
    // if the true path leads inside the box, we advance the pointer
    // and restore the "\REGISTRY" prefix that would have been lost.
    //

    is_boxed_path = FALSE;

check_sandbox_prefix:

    if (length >= Dll_BoxKeyPathLen &&
            0 == Dll_NlsStrCmp(
                *OutTruePath, Dll_BoxKeyPath, Dll_BoxKeyPathLen))
    {
        *OutTruePath += Dll_BoxKeyPathLen - Key_RegistryLen;
        wmemcpy(*OutTruePath, Key_Registry, Key_RegistryLen);
        length -= Dll_BoxKeyPathLen - Key_RegistryLen;
        if (OutIsBoxedPath)
            *OutIsBoxedPath = TRUE;
        is_boxed_path = TRUE;

        goto check_sandbox_prefix;
    }

    //
    // the true path may now begin with \REGISTRY\USER\CURRENT, for
    // instance, because the process specified a RootDirectory handle
    // for a key inside the box.  we have to change that to
    // \REGISTRY\USER\S-x-y.  a later section of code will change
    // it back to \REGISTRY\USER\CURRENT for the copy path.
    //
    // note that we temporarily use the COPY_NAME_BUFFER here, but
    // that's ok because it hasn't been initialized yet
    //

    if (length >= Key_RegistryLen + Key_UserCurrentLen &&
            _wcsnicmp(*OutTruePath + Key_RegistryLen,
                      Key_UserCurrent, Key_UserCurrentLen) == 0)
    {
        ULONG prefix_len, suffix_len;
        WCHAR *name2;

        prefix_len = Key_RegistryLen + Key_UserCurrentLen;
        suffix_len = length - prefix_len + /* NULL */ 1;

        length = Key_CurrentUserLen + suffix_len;
        name2 = Dll_GetTlsNameBuffer(
                    TlsData, COPY_NAME_BUFFER, length * sizeof(WCHAR));
        wmemcpy(name2, Key_CurrentUser, Key_CurrentUserLen);
        wmemcpy(name2 + Key_CurrentUserLen,
                *OutTruePath + prefix_len, suffix_len);

        *OutTruePath = Dll_GetTlsNameBuffer(
                   TlsData, TRUE_NAME_BUFFER, length * sizeof(WCHAR));
        wmemcpy(*OutTruePath, name2, length);

        length -= /* NULL */ 1;

        goto check_sandbox_prefix;
    }

    //
    // on Windows Vista, SandboxieRpcSs.exe must not be allowed to see
    // the key for the Windows Firewall service, otherwise it tries to
    // manage it, and fails
    //

    if (Dll_ImageType == DLL_IMAGE_SANDBOXIE_RPCSS) {

        if (_wcsnicmp(*OutTruePath, _bfe, wcslen(_bfe)) == 0)
            return STATUS_OBJECT_NAME_NOT_FOUND;
    }

    //
    // if this is a unboxed path, and we opened it by object,
    // check path relocation and update true path accordingly.
    //

    if (!is_boxed_path && RootDirectory) {
      
        name = Handle_GetRelocationPath(RootDirectory, objname_len);
        if (name) {

            *OutTruePath = name;

            name = (*OutTruePath) + wcslen(*OutTruePath);

            if (objname_len) {

                *name = L'\\';
                ++name;
                memcpy(name, ObjectName->Buffer, objname_len);

                name += objname_len / sizeof(WCHAR);
            }

            *name = L'\0';
        }
    }

    //
    // now create the copy path, which is the box prefix prepended
    // to the true path that we have.  note that the copy path will
    // still be missing its null terminator.
    //

    name = Dll_GetTlsNameBuffer(
                TlsData, COPY_NAME_BUFFER, Dll_BoxKeyPathLen + length);

    *OutCopyPath = name;

    wmemcpy(name, Dll_BoxKeyPath, Dll_BoxKeyPathLen);
    name += Dll_BoxKeyPathLen;

    //
    // if true path begins with "\REGISTRY\USER\S-sid" with the sid for
    // the current user, then this entire prefix is replaced by
    // "USER\CURRENT" in the copy path.  otherwise only the "\REGISTRY"
    // prefix is dropped in the copy path.
    //

    if (length >= Key_CurrentUserLen &&
           _wcsnicmp(*OutTruePath, Key_CurrentUser, Key_CurrentUserLen) == 0)
    {
        wmemcpy(name, Key_UserCurrent, Key_UserCurrentLen);
        name += Key_UserCurrentLen;

        length -= Key_CurrentUserLen;
        wmemcpy(name, *OutTruePath + Key_CurrentUserLen, length);

    } else if (length >= 35 &&
            _wcsnicmp(*OutTruePath, _ControlSet, 35) == 0) {

        //
        // if true path begins with \REGISTRY\MACHINE\ControlSetXxx\Yyy
        // then set copy path to \MACHINE\CurrentControlSet\Yyy
        //

        WCHAR *backslash = wcschr(*OutTruePath + 35, L'\\');
        wmemcpy(name, _bfe + 9, 33);        // copy up to CurrentControlSet
        if (backslash) {
            length = wcslen(backslash);
            wmemcpy(name + 33, backslash, length);
            length += 33;
        } else
            length = 33;

    } else if (Dll_OsBuild >= 6000 && Dll_OsBuild < 7600 && length >= 70 &&
            _wcsnicmp(*OutTruePath, _SideBySide, 70) == 0) {

        //
        // on Windows Vista, but not Windows 7, all the registry keys below
        // HKLM\Software\Microsoft\Windows\CurrentVersion\SideBySide
        // are reparsed to a corresponding subkey below \HKLM\COMPONENTS
        //

        WCHAR *ptr = *OutTruePath + 70;
        wmemcpy(name, L"\\MACHINE\\COMPONENTS", 19);
        length = wcslen(ptr);
        wmemcpy(name + 19, ptr, length);
        length += 19;

    } else {

        //
        // otherwise drop the \REGISTRY prefix and copy everything
        // else to the copy path
        //

        length -= Key_RegistryLen;
        wmemcpy(name, *OutTruePath + Key_RegistryLen, length);
    }

    //
    // null-terminate the copy path and finish
    //

    name += length;
    *name = L'\0';

    /*if (0) {
        ULONG len = wcslen(*OutTruePath) + 1;
        WCHAR *path = Dll_AllocTemp(len * sizeof(WCHAR));
        wmemcpy(path, *OutTruePath, len);
        _wcslwr(path);
        if (wcsstr(path, L"mmdevices\\audio")) {
            while (! IsDebuggerPresent()) { OutputDebugString(L"BREAK\n"); Sleep(500); }
            __debugbreak();
        }
    }*/

    return STATUS_SUCCESS;
}


//---------------------------------------------------------------------------
// Key_FixNameWow64
//---------------------------------------------------------------------------


_FX NTSTATUS Key_FixNameWow64(
    ACCESS_MASK DesiredAccess, WCHAR **OutTruePath, WCHAR **OutCopyPath)
{
#ifndef _WIN64
    NTSTATUS status;
    OBJECT_ATTRIBUTES objattrs;
    UNICODE_STRING objname;
    HANDLE handle;
    WCHAR *TruePath, *BackslashPtr, *NewPtr;
#endif 

    //
    // WOW64 Registry Redirector.  in case that either of these is true:
    //
    // - caller is 32-bit process, that did not specify KEY_WOW64_64KEY
    // - caller is 64-bit process, and did specify KEY_WOW64_32KEY
    //
    // we open the TruePath, so the registry redirector can do its magic,
    // and then reconstruct the output TruePath and CopyPath
    //

#ifndef _WIN64
    if (! Dll_IsWow64) {        // caller is 64-bit process
#endif
        if (! (DesiredAccess & KEY_WOW64_32KEY))
            return STATUS_SUCCESS;

        // ToDo: ???
        // NoSysCallHooks BEGIN
        //if(Dll_CompartmentMode || SbieApi_QueryConfBool(NULL, L"NoSysCallHooks", FALSE))
        //    return STATUS_SUCCESS;
        // NoSysCallHooks END

        return Key_FixNameWow64_2(OutTruePath, OutCopyPath);
#ifndef _WIN64
    }

    if (DesiredAccess & KEY_WOW64_64KEY)
        return STATUS_SUCCESS;

    DesiredAccess &= (KEY_WOW64_32KEY | KEY_WOW64_64KEY);
    DesiredAccess |= KEY_READ;

    //
    // try to open TruePath, chopping off the last path component
    //
    // note that a 32-bit program gets registry redirection through
    // a call to the NtOpenKey in the Wow64 thunking layer.  that is
    // not the case for a 64-bit program, see Key_FixNameWow64_2
    //

    InitializeObjectAttributes(
        &objattrs, &objname, OBJ_CASE_INSENSITIVE, NULL, NULL);

    handle = NULL;
    TruePath = *OutTruePath;
    BackslashPtr = NULL;

    while (1) {

        RtlInitUnicodeString(&objname, TruePath);

        status = __sys_NtOpenKey(&handle, DesiredAccess, &objattrs);

        if (status != STATUS_OBJECT_NAME_NOT_FOUND &&
            status != STATUS_OBJECT_PATH_NOT_FOUND) {

            break;
        }

        NewPtr = wcsrchr(TruePath, L'\\');
        if (BackslashPtr)
            *BackslashPtr = L'\\';
        BackslashPtr = NewPtr;

        if (! BackslashPtr) {
            status = STATUS_OBJECT_NAME_NOT_FOUND;
            break;
        }
        *BackslashPtr = L'\0';
    }

    //
    // if we could not open any part of TruePath, or if there was
    // some unexpected error, then just do nothing
    //

    if (! NT_SUCCESS(status)) {

        if (BackslashPtr)
            *BackslashPtr = L'\\';

        return STATUS_SUCCESS;
    }

    //
    // otherwise we can now call Key_GetName again
    //

    if (BackslashPtr) {

        ULONG len = (wcslen(BackslashPtr + 1) + 1) * sizeof(WCHAR);
        NewPtr = Dll_AllocTemp(len);
        memcpy(NewPtr, BackslashPtr + 1, len);
        RtlInitUnicodeString(&objname, NewPtr);

    } else {

        NewPtr = NULL;
        memzero(&objname, sizeof(UNICODE_STRING));
    }

    status = Key_GetName(handle, &objname, OutTruePath, OutCopyPath, NULL);

    if (NewPtr)
        Dll_Free(NewPtr);
    File_NtCloseImpl(handle);

    if (NT_SUCCESS(status)) {

        //
        // the result might contain \Wow6432Node\Wow6432Node,
        // so strip one of them away
        //

        if (Key_FixNameWow64_3(OutTruePath))
            Key_FixNameWow64_3(OutCopyPath);
    }

    return status;
#endif
}


//---------------------------------------------------------------------------
// Key_FixNameWow64_2
//---------------------------------------------------------------------------


_FX NTSTATUS Key_FixNameWow64_2(WCHAR **OutTruePath, WCHAR **OutCopyPath)
{
    NTSTATUS status;
    WCHAR *TruePath;
    ULONG TruePath_len;
    FILE_OPEN_WOW64_KEY_REQ *req;
    FILE_OPEN_WOW64_KEY_RPL *rpl;
    ULONG req_len;
    UNICODE_STRING objname;

    //
    // for a 64-bit program trying to open a 32-bit registry key, the
    // call to NtOpenKey does not do registry redirection, as there is
    // no Wow64 thunking layer, so call SbieSvc to open the key using
    // RegOpenKeyEx
    //

    TruePath = *OutTruePath;
    TruePath_len = (wcslen(TruePath) + 1) * sizeof(WCHAR);
    req_len = sizeof(FILE_OPEN_WOW64_KEY_REQ) + TruePath_len;
    req = (FILE_OPEN_WOW64_KEY_REQ *)Dll_AllocTemp(req_len);

    req->h.length = req_len;
    req->h.msgid = MSGID_FILE_OPEN_WOW64_KEY;
    req->Wow64DesiredAccess = KEY_WOW64_32KEY;
    req->KeyPath_len = TruePath_len;
    memcpy(req->KeyPath, TruePath, TruePath_len);

    rpl = (FILE_OPEN_WOW64_KEY_RPL *)SbieDll_CallServer((MSG_HEADER *)req);
    if (! rpl)
        status = STATUS_INSUFFICIENT_RESOURCES;
    else {

        status = rpl->h.status;
        if (NT_SUCCESS(status)) {

            RtlInitUnicodeString(&objname, rpl->KeyPath);

            status = Key_GetName(NULL, &objname,
                                 OutTruePath, OutCopyPath,
                                 NULL);

            if (NT_SUCCESS(status)) {

                //
                // the result might contain \Wow6432Node\Wow6432Node,
                // so strip one of them away
                //

                if (Key_FixNameWow64_3(OutTruePath))
                    Key_FixNameWow64_3(OutCopyPath);
            }
        }

        Dll_Free(rpl);
    }

    return status;
}


//---------------------------------------------------------------------------
// Key_FixNameWow64_3
//---------------------------------------------------------------------------


_FX BOOLEAN Key_FixNameWow64_3(WCHAR **OutPath)
{
    WCHAR *ptr1, *ptr2;
    ptr1 = wcsstr(*OutPath, Key_Wow6432Node);
    if (! ptr1)
        return FALSE;
    ptr2 = ptr1 + 12;
    if (wcsncmp(ptr2, Key_Wow6432Node, 12) != 0)
        return FALSE;
    ptr1 = ptr2 + 12;
    wmemmove(ptr2, ptr1, wcslen(ptr1) + 1);
    return TRUE;
}


//---------------------------------------------------------------------------
// Key_GetWow64Flag
//---------------------------------------------------------------------------


_FX ACCESS_MASK Key_GetWow64Flag(
    const WCHAR *TruePath, ACCESS_MASK DesiredAccess)
{
    if (Dll_IsWin64) {

        //
        // if this is Windows 7, we can always return KEY_WOW64_64KEY
        //
        // if this is Windows Vista, we have to return KEY_WOW64_32KEY
        // if the TruePath references Wow6432Node
        //

        if (Dll_OsBuild >= 7600)
            DesiredAccess |= KEY_WOW64_64KEY;

        else if (wcsstr(TruePath, Key_Wow6432Node))
            DesiredAccess |= KEY_WOW64_32KEY;

        else
            DesiredAccess |= KEY_WOW64_64KEY;
    }

    return DesiredAccess;
}


//---------------------------------------------------------------------------
// Key_NtOpenKey
//---------------------------------------------------------------------------


_FX NTSTATUS Key_NtOpenKey(
    HANDLE *KeyHandle,
    ACCESS_MASK DesiredAccess,
    OBJECT_ATTRIBUTES *ObjectAttributes)
{
    NTSTATUS status =  Key_NtOpenKeyImpl(KeyHandle, DesiredAccess, ObjectAttributes);

    status = StopTailCallOptimization(status);

    return status;
}

//---------------------------------------------------------------------------
// Key_NtOpenKeyImpl
//---------------------------------------------------------------------------


_FX NTSTATUS Key_NtOpenKeyImpl(
    HANDLE *KeyHandle,
    ACCESS_MASK DesiredAccess,
    OBJECT_ATTRIBUTES *ObjectAttributes)
{
    //
    // use Key_NtCreateKey here so if anybody else intercepts
    // NtCreateKey, we don't let them see the special CreateOptions
    //

    NTSTATUS status = Key_NtCreateKeyImpl(
        KeyHandle, DesiredAccess, ObjectAttributes, 0, NULL, tzuk, NULL);
    return status;
}


//---------------------------------------------------------------------------
// Key_NtOpenKeyEx
//---------------------------------------------------------------------------


_FX NTSTATUS Key_NtOpenKeyEx(
    HANDLE *KeyHandle,
    ACCESS_MASK DesiredAccess,
    OBJECT_ATTRIBUTES *ObjectAttributes,
    ULONG OpenOptions)
{
    NTSTATUS status;

    OpenOptions &= ~REG_OPTION_BACKUP_RESTORE;
    /*if (OpenOptions) {
        // probably REG_OPTION_OPEN_LINK
        SbieApi_Log(2205, L"NtOpenKeyEx (%08X)", OpenOptions);
    }*/

    status = Key_NtCreateKeyImpl(
        KeyHandle, DesiredAccess, ObjectAttributes, 0, NULL, tzuk, NULL);

    status = StopTailCallOptimization(status);

    return status;
}


//---------------------------------------------------------------------------
// Key_NtCreateKey
//---------------------------------------------------------------------------


_FX NTSTATUS Key_NtCreateKey(
    HANDLE *KeyHandle,
    ACCESS_MASK DesiredAccess,
    OBJECT_ATTRIBUTES *ObjectAttributes,
    ULONG TitleIndex,
    UNICODE_STRING *Class,
    ULONG CreateOptions,
    ULONG *Disposition)
{
    NTSTATUS status =  Key_NtCreateKeyImpl( KeyHandle,
                                            DesiredAccess,
                                            ObjectAttributes,
                                            TitleIndex,
                                            Class,
                                            CreateOptions,
                                            Disposition);

    status = StopTailCallOptimization(status);

    return status;
}


//---------------------------------------------------------------------------
// Key_NtCreateKeyImpl
//---------------------------------------------------------------------------


#ifdef WITH_DEBUG_
static P_NtCreateKey               __sys_NtCreateKey_ = NULL;

_FX NTSTATUS Key_MyCreateKey(
    HANDLE *KeyHandle,
    ACCESS_MASK DesiredAccess,
    OBJECT_ATTRIBUTES *ObjectAttributes,
    ULONG TitleIndex,
    UNICODE_STRING *Class,
    ULONG CreateOptions,
    ULONG *Disposition)
{
    ULONG Disposition_ = 0;

    if (!Disposition)
        Disposition = &Disposition_;

    NTSTATUS status = __sys_NtCreateKey_(
        KeyHandle, DesiredAccess, ObjectAttributes, TitleIndex,
        Class, CreateOptions, Disposition);

    if (*Disposition == REG_CREATED_NEW_KEY) {
       while (! IsDebuggerPresent()) { OutputDebugString(L"BREAK\n"); Sleep(500); }
          __debugbreak();
    }

    //if (NT_SUCCESS(status)) DbgPrint("%p: %p\r\n", _ReturnAddress(), *KeyHandle);

    status = StopTailCallOptimization(status);

    return status;
}
#endif


_FX NTSTATUS Key_NtCreateKeyImpl(
    HANDLE *KeyHandle,
    ACCESS_MASK DesiredAccess,
    OBJECT_ATTRIBUTES *ObjectAttributes,
    ULONG TitleIndex,
    UNICODE_STRING *Class,
    ULONG CreateOptions,
    ULONG *Disposition)
{
    ULONG LastError;
    THREAD_DATA *TlsData = Dll_GetTlsData(&LastError);

    NTSTATUS status;
    WCHAR *TruePath;
    WCHAR *CopyPath;
    UNICODE_STRING objname;
    OBJECT_ATTRIBUTES objattrs;
    WCHAR *BackslashPtr;
    ULONG OriginalDesiredAccess;
    ULONG Wow64KeyReadAccess;
    ULONG mp_flags;
    BOOLEAN CopyPathCreated;
    BOOLEAN TruePathExists;
    PSECURITY_DESCRIPTOR *OverrideSecurityDescriptor;
    ULONG TruePathFlags;
    WCHAR* OriginalPath;
    BOOLEAN TrueOpened;

#ifdef WITH_DEBUG_
    if (__sys_NtCreateKey_ == NULL)
    {
        __sys_NtCreateKey_ = __sys_NtCreateKey;
        __sys_NtCreateKey = Key_MyCreateKey;
    }
#endif

    //
    // if this is a recursive invocation of NtCreateKey,
    // then pass it as-is down the chain
    //

    if (TlsData->key_NtCreateKey_lock) {

        if (CreateOptions == tzuk) {

            return __sys_NtOpenKey(
                KeyHandle, DesiredAccess, ObjectAttributes);

        } else {

            return __sys_NtCreateKey(
                KeyHandle, DesiredAccess, ObjectAttributes,
                TitleIndex, Class, CreateOptions, Disposition);
        }
    }

    //
    // not a recursive invocation, handle the call here
    //

    CopyPathCreated = FALSE;
    TruePathExists = FALSE;
    OriginalPath = NULL;
    TrueOpened = FALSE;

    TlsData->key_NtCreateKey_lock = TRUE;

    Dll_PushTlsNameBuffer(TlsData);

    if (Dll_OsBuild >= 8400 && Dll_ImageType == DLL_IMAGE_TRUSTED_INSTALLER)
        DesiredAccess &= ~ACCESS_SYSTEM_SECURITY;   // for TiWorker.exe (W8)

    if ((CreateOptions != tzuk) && (CreateOptions & REG_OPTION_VOLATILE))
        CreateOptions &= ~REG_OPTION_VOLATILE;

    __try {

    //
    // get the full paths for the true and copy keys
    //

    status = Key_GetName(
        ObjectAttributes->RootDirectory, ObjectAttributes->ObjectName,
        &TruePath, &CopyPath, NULL);
    if (! NT_SUCCESS(status))
        __leave;

    InitializeObjectAttributes(&objattrs,
        &objname, OBJECT_ATTRIBUTES_ATTRIBUTES, NULL, Secure_NormalSD);

    //
    // allow SXS module to intercept open requests during CreateProcess
    //

    if (TlsData->proc_create_process &&
                                Sxs_KeyCallback(TruePath, KeyHandle))
        __leave;

    //
    // check if this is an open or closed path
    //

    mp_flags = SbieDll_MatchPath(L'k', TruePath);

    if (PATH_IS_CLOSED(mp_flags)) {
        status = STATUS_ACCESS_DENIED;
        __leave;
    }

    if (PATH_IS_OPEN(mp_flags)) {

#define KEY_READ_WOW64  (KEY_READ |     \
            (DesiredAccess & (KEY_WOW64_32KEY | KEY_WOW64_64KEY)))

        //
        // Application specific hives are loaded under \REGISTRY\A\ and can not be enumerated, or opened by name.
        // MSDN: All operations on hives loaded by RegLoadAppKey have to be performed relative to the handle returned.
        // So it is not possible to use TruePath but we can use the original handle relative ObjectAttributes here instead.
        //

        BOOLEAN IsAKey = _wcsnicmp(TruePath, L"\\REGISTRY\\A\\", 12) == 0;

        RtlInitUnicodeString(&objname, TruePath);
        objattrs.SecurityDescriptor = ObjectAttributes->SecurityDescriptor;

        if (CreateOptions == tzuk) {

            if(IsAKey)
                status = __sys_NtOpenKey(KeyHandle, DesiredAccess, ObjectAttributes);
            else
                status = __sys_NtOpenKey(KeyHandle, DesiredAccess, &objattrs);

            if (status == STATUS_ACCESS_DENIED &&
                    DesiredAccess == MAXIMUM_ALLOWED) {

                status = __sys_NtOpenKey(
                                    KeyHandle, KEY_READ_WOW64, &objattrs);
            }

        } else {

            if(IsAKey)
                status = __sys_NtCreateKey(
                    KeyHandle, DesiredAccess, ObjectAttributes,
                    TitleIndex, Class, CreateOptions, Disposition);
            else
                status = __sys_NtCreateKey(
                    KeyHandle, DesiredAccess, &objattrs,
                    TitleIndex, Class, CreateOptions, Disposition);

            if (status == STATUS_ACCESS_DENIED &&
                    DesiredAccess == MAXIMUM_ALLOWED) {

                status = __sys_NtCreateKey(
                    KeyHandle, KEY_READ_WOW64, &objattrs,
                    TitleIndex, Class, CreateOptions, Disposition);
            }
        }

        if (NT_SUCCESS(status)) TrueOpened = TRUE;

        __leave;

#undef KEY_READ_WOW64

    }

    //
    // adjust key paths for 32-bit programs on 64-bit Windows
    //

    OriginalDesiredAccess = 0;

    if (Dll_IsWin64) {

        if (_wcsnicmp(TruePath, Key_ClickToRun, Key_ClickToRunLen) != 0
            && _wcsnicmp(TruePath, Key_ClickToRun_v16, Key_ClickToRunLen_v16) != 0
            )
        {
            status = Key_FixNameWow64(DesiredAccess, &TruePath, &CopyPath);
            if (!NT_SUCCESS(status))
                __leave;

#ifndef _WIN64
            if (Dll_IsWow64)
                OriginalDesiredAccess = DesiredAccess;
#endif

            DesiredAccess &= ~(KEY_WOW64_32KEY | KEY_WOW64_64KEY);
        }
    }

    Wow64KeyReadAccess = Key_GetWow64Flag(TruePath, KEY_READ);

    RtlInitUnicodeString(&objname, CopyPath);

    //
    // first we try to create or open CopyPath with whatever DesiredAccess
    // the caller specified.  if this succeeds, then CopyPath must exist
    // (or was successfully created) in the box, so we're almost done.
    // we also check that the parent of the key is not marked deleted.
    //

    if (!Key_Delete_v2)
    if (Key_CheckDeletedParent(CopyPath)) {

        //
        // registry functions never return STATUS_OBJECT_PATH_NOT_FOUND,
        // only STATUS_OBJECT_NAME_NOT_FOUND
        //

        status = STATUS_OBJECT_NAME_NOT_FOUND;
        __leave;
    }

    //
    // note that we create or open the CopyPath specifically without
    // the KEY_WOW64_64KEY flag, because we want WOW64 hooks to inspect
    // or change any values written using NtSetValueKey.  On Windows 7,
    // the KEY_WOW64_64KEY flag inhibits the NtSetValueKey behavior.
    //

    if (CreateOptions == tzuk) {

        status = __sys_NtOpenKey(
            KeyHandle, DesiredAccess | KEY_READ, &objattrs);

    } else {

        CreateOptions &= ~REG_OPTION_BACKUP_RESTORE;

        status = __sys_NtCreateKey(
            KeyHandle, DesiredAccess | KEY_READ, &objattrs,
            TitleIndex, Class, CreateOptions, Disposition);

        if (NT_SUCCESS(status)) {

            if (! Disposition)
                CopyPathCreated = TRUE;
            else if (*Disposition == REG_CREATED_NEW_KEY)
                CopyPathCreated = TRUE;
        }
    }

    //
    // if this is a low integrity process, adjust the label on the
    // requested copy key so that we can open it
    //

    if (status == STATUS_ACCESS_DENIED && Secure_IsRestrictedToken(TRUE)) {

        NTSTATUS status2 = SbieApi_SetLowLabelKey(CopyPath);
        if (NT_SUCCESS(status2)) {

            if (CreateOptions == tzuk) {

                status2 = __sys_NtOpenKey(
                    KeyHandle, DesiredAccess | KEY_READ, &objattrs);

            } else {

                status2 = __sys_NtCreateKey(
                    KeyHandle, DesiredAccess | KEY_READ, &objattrs,
                    TitleIndex, Class, CreateOptions, Disposition);

                if (NT_SUCCESS(status2)) {

                    if (! Disposition)
                        CopyPathCreated = TRUE;
                    else if (*Disposition == REG_CREATED_NEW_KEY)
                        CopyPathCreated = TRUE;
                }
            }

            if (NT_SUCCESS(status2))
                status = status2;
        }
    }

    //
    // if we're successful, we may have to make sure the key is not
    // deleted.  this is important so that
    // - NtOpenKey requests can be failed for a deleted key
    // - NtCreateKey can adjust the LastWriteTime, and return
    //   correct Disposition, if one was specified
    //

    if (NT_SUCCESS(status)) {

        BOOLEAN KeyDeleted = FALSE;

        if (!Key_Delete_v2)
        if (Key_CheckDeletedKey(*KeyHandle)) {

            KeyDeleted = TRUE;

            if (CreateOptions == tzuk) {

                File_NtCloseImpl(*KeyHandle);
                *KeyHandle = 0;
                status = STATUS_OBJECT_NAME_NOT_FOUND;
                __leave;
            }

            //
            // if the key was "created" over an existing key that was
            // marked deleted, we have to adjust the LastWriteTime
            //

            Key_MarkUnDeleted(*KeyHandle);
        }

        //
        // if caller specified disposition, we have to make sure that
        // NtCreateKey really did create the new key.  we don't want to
        // return REG_CREATED_NEW_KEY if there is an existing TruePath.
        //

        if (Disposition) {

            if (KeyDeleted) {

                *Disposition = REG_CREATED_NEW_KEY;
            }
            // If the path is write only, __sys_NtOpenKey is going to return STATUS_ACCESS_DENIED,
            // so don't bother to check.  Disposition will already be set from above call to __sys_NtCreateKey
            // that created key in sandbox.
            else if ( (*Disposition == REG_CREATED_NEW_KEY) && PATH_NOT_WRITE(mp_flags) ) {

                HANDLE handle;

                RtlInitUnicodeString(&objname, TruePath);
                status = __sys_NtOpenKey(
                                    &handle, Wow64KeyReadAccess, &objattrs);

                if (NT_SUCCESS(status)) {

                    File_NtCloseImpl(handle);
                    *Disposition = REG_OPENED_EXISTING_KEY;

                }
                else if (status == STATUS_OBJECT_NAME_NOT_FOUND ||
                    status == STATUS_OBJECT_PATH_NOT_FOUND) {

                    // *Disposition = REG_CREATED_NEW_KEY;
                    status = STATUS_SUCCESS;

                }
                else {

                    File_NtCloseImpl(*KeyHandle);
                    *KeyHandle = 0;
                }
            }
        }

        __leave;
    }

    //
    // Check true path relocation
    //

    TruePathFlags = 0;

    if (Key_Delete_v2) {

        WCHAR* OldTruePath = Key_ResolveTruePath(TruePath, &TruePathFlags);
        if (OldTruePath) {
            OriginalPath = TruePath;
            TruePath = OldTruePath;
        }

        // if key marked as deleted don't even try opening true path
        if (KEY_PATH_DELETED(TruePathFlags) && CreateOptions == tzuk) { 
            status = STATUS_OBJECT_NAME_NOT_FOUND;
            __leave;
        }
    }


    //
    // if we're successful, or we got any of the three status codes
    // that we don't handle, then stop here
    //

    if (NT_SUCCESS(status) || (
            status != STATUS_OBJECT_NAME_NOT_FOUND &&
            status != STATUS_OBJECT_PATH_NOT_FOUND))
    {
        __leave;
    }

    //
    // at this point CopyPath does not exist, or else it would have
    // already been opened or created above.  see if the corresponding
    // TruePath key exists outside the sandbox
    //

    RtlInitUnicodeString(&objname, TruePath);

    if (PATH_IS_WRITE(mp_flags)) {

        //
        // if the path matches a write-only path, the key must be the
        // highest level key in the write-only hierarchy.  we use
        // SbieApi_OpenKey which should open the key even if blocked
        //

        int depth = Key_CheckDepthForIsWritePath(TruePath);
        if (depth == 0) {
            status = SbieApi_OpenKey(KeyHandle, TruePath);
            if (NT_SUCCESS(status))
                goto SkipReadOnlyCheck;
        } else
            status = STATUS_OBJECT_NAME_NOT_FOUND;

    } else {

        //
        // otherwise not write-only, so do normal NtOpenKey
        //

        status = __sys_NtOpenKey(KeyHandle, Wow64KeyReadAccess, &objattrs);
    }

    if (NT_SUCCESS(status)) {

        TrueOpened = TRUE;

        //
        // if the TruePath key exists, and caller is asking for
        // read-only (*) access to this key, then return the handle
        // we just opened.
        // otherwise, we proceed ahead to create the sandboxed key
        //
        // (*) we consider MAXIMUM_ALLOWED as read-only access, unless
        // this is a WOW64 process specifying the KEY_WOW64_xxKEY flag.
        // the reason is that if NtSetValueKey has to re-open this key
        // for write access, it would not be able to pass the WOW64 flag
        //
        // this special case makes IMHO no sense, wow registry redirection
        // acts only on specific paths, and our sandboxed paths do not fall 
        // into that category, hence they do not need KEY_WOW64_xxKEY flags!
        //

        //if (OriginalDesiredAccess & (KEY_WOW64_32KEY | KEY_WOW64_64KEY))
        //    goto SkipReadOnlyCheck;

        if (((DesiredAccess & ~MAXIMUM_ALLOWED) & KEY_DENIED_ACCESS) == 0) {

            if (Disposition)
                *Disposition = REG_OPENED_EXISTING_KEY;
            __leave;
        }

SkipReadOnlyCheck:

        TruePathExists = TRUE;

        File_NtCloseImpl(*KeyHandle);
        *KeyHandle = 0;

    } else if (status == STATUS_OBJECT_NAME_NOT_FOUND ||
               status == STATUS_OBJECT_PATH_NOT_FOUND) {

        //
        // if the TruePath key does not exist, and the caller is asking
        // to open the key (not create it), then fail the request
        //

        if (CreateOptions == tzuk)
            __leave;

    } else {

        //
        // also fail the request for any other error
        //

        __leave;
    }

    //
    // if we get here, the caller has requested write access to a key
    // that does not yet exist inside the box.  try to open the
    // parent of the TruePath, to see if the key hierarchy exists
    // outside the sandbox
    //

    BackslashPtr = wcsrchr(TruePath, L'\\');
    *BackslashPtr = L'\0';

    RtlInitUnicodeString(&objname, TruePath);

    status = SbieApi_OpenKey(KeyHandle, TruePath);

    *BackslashPtr = L'\\';

    if (! NT_SUCCESS(status))
        __leave;

    File_NtCloseImpl(*KeyHandle);
    *KeyHandle = 0;

    //
    // the parent hierarchy exists outside the sandbox, so create the
    // matching parent hierarchy inside the sandbox -- note that this
    // creates the entire path, including the last component.
    //
    // note that if caller requests to use Secure_EveryoneSD then
    // we use this to create the last component in the path.  used
    // by the GetSetCustomLevel function from custom module
    //

    CopyPathCreated = TRUE;

    RtlInitUnicodeString(&objname, CopyPath);

    OverrideSecurityDescriptor = ObjectAttributes->SecurityDescriptor;
    if (OverrideSecurityDescriptor != Secure_EveryoneSD)
        OverrideSecurityDescriptor = NULL;

    status = Key_CreatePath(&objattrs, OverrideSecurityDescriptor);

    //
    // finally, open the last path component on behalf of the caller
    //

    if (NT_SUCCESS(status)) {

        //
        // make sure not to pass KEY_WOW64_64KEY flag, in order
        // to not inhibit NtSetValueKey from changing values,
        // as described above in more detail
        //

        status = __sys_NtOpenKey(KeyHandle, DesiredAccess, &objattrs);

        if (NT_SUCCESS(status) && Disposition) {

            if (TruePathExists)
                *Disposition = REG_OPENED_EXISTING_KEY;
            else
                *Disposition = REG_CREATED_NEW_KEY;
        }
    }

    //
    // finish
    //

    } __except (EXCEPTION_EXECUTE_HANDLER) {
        status = GetExceptionCode();
    }

    //
    // we can get access denied in a restricted Chrome sandbox process:
    // perhaps we tried to access a key in the box which isn't
    // accessible to a restricted token, but the real key might be
    // accessible, so try to access the real key
    //

    if (Dll_RestrictedToken && status == STATUS_ACCESS_DENIED) {

        if (DesiredAccess == MAXIMUM_ALLOWED)
            DesiredAccess = KEY_READ;
        DesiredAccess |=
            OriginalDesiredAccess & (KEY_WOW64_32KEY | KEY_WOW64_64KEY);

        status = Key_NtCreateKeyImpl(
                    KeyHandle, DesiredAccess, ObjectAttributes,
                    TitleIndex, Class, CreateOptions, Disposition);

        if (status == STATUS_ACCESS_DENIED && CreateOptions != tzuk) {

            //
            // SbieDrv might block NtCreateKey
            // so try one more time using NtOpenKey
            //

            status = __sys_NtOpenKey(
                KeyHandle, DesiredAccess, ObjectAttributes);

            if (NT_SUCCESS(status)) TrueOpened = TRUE; // is that right?

            if (NT_SUCCESS(status) && Disposition)
                *Disposition = REG_OPENED_EXISTING_KEY;
        }
    }

    //
    // refresh all merges, if we invoked NtCreateKey at any time
    // (but don't refresh if we only used NtOpenKey)
    //

    if (CopyPathCreated)
        Key_DiscardMergeByPath(TruePath, TRUE);

    //
    // Relocation, if we opened a relocated location we need to 
    // store the original true path for the Key_GetName function
    //

    if (TrueOpened && OriginalPath) {

        Handle_SetRelocationPath(*KeyHandle, OriginalPath);
    }

    //
    // finish
    //

    Dll_PopTlsNameBuffer(TlsData);

    TlsData->key_NtCreateKey_lock = FALSE;

    SetLastError(LastError);
    return status;
}


//---------------------------------------------------------------------------
// Key_CreatePath
//---------------------------------------------------------------------------


_FX NTSTATUS Key_CreatePath(
    OBJECT_ATTRIBUTES *objattrs, PSECURITY_DESCRIPTOR *sd)
{
    NTSTATUS status;
    HANDLE handle;
    WCHAR *path;
    WCHAR *sep;
    WCHAR savechar;
    UNICODE_STRING *objname;
    USHORT savelength;
    USHORT savemaximumlength;
    ULONG disp;

    if (Key_Delete_v2) {

        THREAD_DATA *TlsData = Dll_GetTlsData(NULL);

        BOOLEAN ParentDeleted = FALSE;
        WCHAR *TruePath;
        WCHAR *CopyPath;

        Dll_PushTlsNameBuffer(TlsData);

        __try {

        status = Key_GetName(NULL, objattrs->ObjectName, &TruePath, &CopyPath, NULL);

        } __except (EXCEPTION_EXECUTE_HANDLER) {
            status = GetExceptionCode();
        }

        if (NT_SUCCESS(status)) {
            ParentDeleted = KEY_PARENT_DELETED(Key_IsDeleted_v2(TruePath));
        }

        Dll_PopTlsNameBuffer(TlsData);

        if(ParentDeleted)
            return STATUS_OBJECT_NAME_NOT_FOUND;
    }

    //
    // first we traverse backward along the path, removing the last
    // path component each time, and trying to create the path that
    // we have left.  we stop when we succeed, ie, when we reach
    // the end of the existing key hierarchy.
    //

    objname = objattrs->ObjectName;
    path = objname->Buffer;
    sep = path + objname->Length / sizeof(WCHAR);

    while (1) {

        --sep;
        while ((sep > path) && (*sep != L'\\'))
            --sep;

        if (sep <= path) {
            //
            // we went back all the way to the first character.  this
            // shouldn't happen in practice, because certainly we
            // should've stopped at existing keys before getting here.
            //
            return STATUS_OBJECT_PATH_INVALID;
        }

        //
        // chop off the last component of the path, and try to open
        // or create it.  if we succeed, break out of this loop.
        // (unless the key was already there, and marked deleted.)
        //

        savechar = *sep;
        *sep = L'\0';

        savelength = objname->Length;
        savemaximumlength = objname->MaximumLength;
        objname->Length = (sep - path) * sizeof(WCHAR);
        objname->MaximumLength = objname->Length + sizeof(WCHAR);

        status = Key_CreatePath_Key(&handle, objattrs, &disp);

        objname->Length = savelength;
        objname->MaximumLength = savemaximumlength;
        *sep = savechar;

        if (NT_SUCCESS(status)) {

            if (!Key_Delete_v2)
            if (disp == REG_OPENED_EXISTING_KEY) {
                if (Key_CheckDeletedKey(handle)) {

                    File_NtCloseImpl(handle);
                    return STATUS_OBJECT_NAME_NOT_FOUND;
                }
            }

            File_NtCloseImpl(handle);

            break;
        }

        if (status != STATUS_OBJECT_NAME_NOT_FOUND &&
            status != STATUS_OBJECT_PATH_NOT_FOUND &&
            status != STATUS_ACCESS_DENIED)
        {
            return status;
        }
    }

    //
    // now traverse forward, creating all the missing keys in the hierarchy
    //

    while (1) {

        ++sep;
        while (*sep && *sep != L'\\')
            ++sep;

        savechar = *sep;
        *sep = L'\0';

        savelength = objname->Length;
        savemaximumlength = objname->MaximumLength;
        objname->Length = (sep - path) * sizeof(WCHAR);
        objname->MaximumLength = objname->Length + sizeof(WCHAR);

        if ((! savechar) && sd)             // override security descriptor
            objattrs->SecurityDescriptor = sd;  // for last path component

        status = Key_CreatePath_Key(&handle, objattrs, &disp);

        if (status == STATUS_ACCESS_DENIED && Dll_RestrictedToken) {

            //
            // if a low integrity process can't create a new key, then we
            // probably need to adjust the integrity label on the parent key
            //

            WCHAR *backslash = wcsrchr(path, L'\\');
            if (backslash) {

                *backslash = L'\0';
                status = SbieApi_SetLowLabelKey(path);
                *backslash = L'\\';

                if (NT_SUCCESS(status)) {

                    status = Key_CreatePath_Key(&handle, objattrs, &disp);

                } else
                    status = STATUS_ACCESS_DENIED;
            }
        }

        objname->Length = savelength;
        objname->MaximumLength = savemaximumlength;
        *sep = savechar;

        if (NT_SUCCESS(status))
            File_NtCloseImpl(handle);

        if ((! NT_SUCCESS(status)) || (! savechar))
            break;
    }

    return status;
}


//---------------------------------------------------------------------------
// Key_CreatePath_Key
//---------------------------------------------------------------------------


_FX NTSTATUS Key_CreatePath_Key(
    HANDLE *out_handle, OBJECT_ATTRIBUTES *objattrs, ULONG *out_disp)
{
    NTSTATUS status = __sys_NtCreateKey(out_handle, GENERIC_READ, objattrs,
                                        0, NULL, 0, out_disp);

    if (status == STATUS_CHILD_MUST_BE_VOLATILE) {

        status = __sys_NtCreateKey(out_handle, GENERIC_READ, objattrs, 0,
                                   NULL, REG_OPTION_VOLATILE, out_disp);
    }

    return status;
}


//---------------------------------------------------------------------------
// Key_CheckDeletedKey
//---------------------------------------------------------------------------


_FX BOOLEAN Key_CheckDeletedKey(HANDLE KeyHandle)
{
    NTSTATUS status;
    KEY_BASIC_INFORMATION info;
    ULONG len;

    status = __sys_NtQueryKey(
        KeyHandle, KeyBasicInformation, &info, sizeof(info), &len);

    if (status != STATUS_SUCCESS && status != STATUS_BUFFER_OVERFLOW)
        return FALSE;

    return (IS_DELETE_MARK(&info.LastWriteTime));
}


//---------------------------------------------------------------------------
// Key_CheckDeletedParent
//---------------------------------------------------------------------------


_FX BOOLEAN Key_CheckDeletedParent(WCHAR *CopyPath)
{
    NTSTATUS status;
    OBJECT_ATTRIBUTES objattrs;
    UNICODE_STRING objname;
    HANDLE handle;
    WCHAR *ptr = NULL;
    const WCHAR *ptr2 = CopyPath + Dll_BoxKeyPathLen;
    ULONG len2;

    //
    // remove the last path component so we can open the parent key
    //

    while (1) {

        WCHAR *ptr_old = ptr;
        ptr = wcsrchr(CopyPath, L'\\');
        if (ptr_old)
            *ptr_old = L'\\';

        if ((! ptr) || ptr == CopyPath)
            return FALSE;

        *ptr = L'\0';

        //
        // special cases
        //

#define COND(ll,ss) (len2 == ll && _wcsicmp(ptr2, ss) == 0)

        len2 = wcslen(ptr2);
        if (COND(8,  L"\\machine")                                      ||
            COND(17, L"\\machine\\software")                            ||
            COND(25, L"\\machine\\software\\classes")                   ||
            COND(31, L"\\machine\\software\\classes\\clsid")            ||
            COND(33, L"\\machine\\software\\classes\\typelib")          ||
            COND(35, L"\\machine\\software\\classes\\interface")        ||
            COND(5,  L"\\user")                                         ||
            COND(13, L"\\user\\current")                                ||
            COND(22, L"\\user\\current\\software")                      ||
            COND(30, L"\\user\\current\\software\\classes")             ||
            COND(36, L"\\user\\current\\software\\classes\\clsid")      ||
            COND(38, L"\\user\\current\\software\\classes\\typelib")    ||
            COND(40, L"\\user\\current\\software\\classes\\interface")  ||
            0                                                           ) {

            *ptr = L'\\';
            return FALSE;
        }

        if (Dll_IsWin64 &&
            COND(29, L"\\machine\\software\\wow6432node")               ||
            COND(36, L"\\machine\\software\\classes\\wow6432node")      ||
            0                                                           ) {

            *ptr = L'\\';
            return FALSE;
        }

#undef COND

        //
        // check if the parent CopyPath key exists and is marked deleted
        //

        RtlInitUnicodeString(&objname, CopyPath);
        InitializeObjectAttributes(
            &objattrs, &objname, OBJ_CASE_INSENSITIVE, NULL, NULL);

        status = __sys_NtOpenKey(&handle, KEY_READ, &objattrs);

        if (NT_SUCCESS(status)) {

            BOOLEAN deleted = Key_CheckDeletedKey(handle);
            File_NtCloseImpl(handle);

            if (deleted) {
                *ptr = L'\\';
                return TRUE;
            }
        }

        //
        // a deleted copy key does not have any children (see
        // Key_NtDeleteKey and Key_NtDeleteKeyTree) so we don't
        // have to scan more than one level
        //

        *ptr = L'\\';
        return FALSE;
    }
}


//---------------------------------------------------------------------------
// Key_CheckDepthForIsWritePath
//---------------------------------------------------------------------------


_FX ULONG Key_CheckDepthForIsWritePath(const WCHAR *TruePath)
{
    ULONG mp_flags, len;
    WCHAR *copy, *ptr;

    //
    // given a path that matches a write-only setting, this function
    // removes the last path component in each iteration to find out
    // how deep the input path is relative to the write-only setting
    //

    len = wcslen(TruePath);
    copy = Dll_AllocTemp((len + 1) * sizeof(WCHAR));
    wmemcpy(copy, TruePath, len + 1);

    len = 0;

    while (1) {

        ptr = wcsrchr(copy, L'\\');
        if (! ptr)
            break;
        *ptr = L'\0';

        mp_flags = SbieDll_MatchPath(L'k', copy);
        if (PATH_NOT_WRITE(mp_flags))
            break;

        ++len;
    }

    Dll_Free(copy);

    return len;
}


//---------------------------------------------------------------------------
// Key_NtDeleteKey
//---------------------------------------------------------------------------


_FX NTSTATUS Key_NtDeleteKey(HANDLE KeyHandle)
{
    NTSTATUS status = Key_NtDeleteKeyTreeImpl(KeyHandle, FALSE);

    status = StopTailCallOptimization(status);

    return status;
}


//---------------------------------------------------------------------------
// Key_NtDeleteKeyTree
//---------------------------------------------------------------------------


_FX NTSTATUS Key_NtDeleteKeyTree(HANDLE KeyHandle, BOOLEAN DeleteTree)
{
    NTSTATUS status = Key_NtDeleteKeyTreeImpl(KeyHandle, DeleteTree);

    status = StopTailCallOptimization(status);

    return status;
}


//---------------------------------------------------------------------------
// Key_NtDeleteKeyTreeImpl
//---------------------------------------------------------------------------


_FX NTSTATUS Key_NtDeleteKeyTreeImpl(HANDLE KeyHandle, BOOLEAN DeleteTree)
{
    THREAD_DATA *TlsData = Dll_GetTlsData(NULL);

    NTSTATUS status;
    OBJECT_ATTRIBUTES objattrs;
    UNICODE_STRING objname;
    HANDLE handle, handle2;
    WCHAR *TruePath;
    WCHAR *CopyPath;
    BOOLEAN IsBoxedPath;
    KEY_BASIC_INFORMATION *info;
    ULONG info_len;
    ULONG len;
    ACCESS_MASK Wow64DeleteAccess;

    Dll_PushTlsNameBuffer(TlsData);
    info = NULL;

    __try {

    //
    // get the full CopyPath to the key we want to delete
    //

    RtlInitUnicodeString(&objname, L"");
    InitializeObjectAttributes(
        &objattrs, &objname, OBJ_CASE_INSENSITIVE, NULL, NULL);

    status = Key_GetName(KeyHandle, &objname, &TruePath, &CopyPath, NULL);
    if (! NT_SUCCESS(status))
        __leave;

    RtlInitUnicodeString(&objname, CopyPath);

    //
    // open the key.  this will create a copy key, if necessary
    //

    status = Key_NtOpenKeyImpl(&handle, GENERIC_WRITE | KEY_READ | DELETE, &objattrs);
    if (! NT_SUCCESS(status))
        __leave;

    //
    // see if we got a true path even though we asked for write access.
    // if so, this must be an open key, so we should let the system do it
    //

    status = Key_GetName(
        handle, &objname, &TruePath, &CopyPath, &IsBoxedPath);
    if (! NT_SUCCESS(status))
        __leave;

    if (! IsBoxedPath) {
        File_NtCloseImpl(handle);
        status = __sys_NtDeleteKey(KeyHandle);
        __leave;
    }

    //
    // allocate a work buffer
    //

    info_len = 256;             // at least sizeof(KEY_FULL_INFORMATION)
    info = Dll_Alloc(info_len);

    //
    // use our NtQueryKey to get the (merged) number of subkeys this
    // key has.  our NtQueryKey does not count keys marked deleted
    //

    if (! DeleteTree) {

        status = Key_NtQueryKeyImpl(
                    handle, KeyFullInformation, info, info_len, &len);

        if (status == STATUS_SUCCESS || status == STATUS_BUFFER_OVERFLOW) {

            if (((KEY_FULL_INFORMATION *)info)->SubKeys) {
                File_NtCloseImpl(handle);
                status = STATUS_CANNOT_DELETE;
                __leave;
            }

        } else              // status != STATUS_SUCCESS
            __leave;
    }

    //
    // now we know all subkeys are deleted (or marked deleted), but
    // we must physically delete them before marking this key deleted.
    //
    // note that this time we want to work explicitly inside the box,
    // and we want to see the keys marked deleted, so we use the system
    // __sys_NtEnumerateKey directly
    //

    while (1) {

        status = __sys_NtEnumerateKey(
            handle, 0, KeyBasicInformation, info, info_len, &len);

        if (status == STATUS_BUFFER_OVERFLOW) {

            Dll_Free(info);
            info_len += 256;
            info = Dll_Alloc(info_len);
            continue;
        }

        if (status != STATUS_SUCCESS) {

            if (status == STATUS_NO_MORE_ENTRIES)
                break;
            File_NtCloseImpl(handle);
            __leave;
        }

        objname.Length = (USHORT)info->NameLength;
        objname.MaximumLength = objname.Length;
        objname.Buffer = info->Name;

        objattrs.RootDirectory = handle;

        Wow64DeleteAccess =
            Key_GetWow64Flag(info->Name, GENERIC_WRITE | DELETE);

        status = __sys_NtOpenKey(&handle2, Wow64DeleteAccess, &objattrs);

        if (status != STATUS_SUCCESS) {
            File_NtCloseImpl(handle);
            __leave;
        }

        if (DeleteTree)
            Key_NtDeleteKeyTreeImpl(handle2, TRUE);

        __sys_NtDeleteKey(handle2);
        File_NtCloseImpl(handle2);
    }

    //
    // finally, mark the key deleted
    //

    status = Key_MarkDeletedAndClose(handle);

    //
    // finish
    //

    } __except (EXCEPTION_EXECUTE_HANDLER) {
        status = GetExceptionCode();
    }

    if (info)
        Dll_Free(info);

    Dll_PopTlsNameBuffer(TlsData);
    return status;
}


//---------------------------------------------------------------------------
// Key_MarkDeletedAndClose
//---------------------------------------------------------------------------


_FX NTSTATUS Key_MarkDeletedAndClose(HANDLE KeyHandle)
{
    THREAD_DATA *TlsData = Dll_GetTlsData(NULL);
    NTSTATUS status;
    KEY_WRITE_TIME_INFORMATION kwti;

    //
    // mark key deleted by setting its last write time information
    //

    if (Key_Delete_v2) {

        THREAD_DATA *TlsData = Dll_GetTlsData(NULL);

        UNICODE_STRING objname;
        WCHAR *TruePath;
        WCHAR *CopyPath;

        Dll_PushTlsNameBuffer(TlsData);

        RtlInitUnicodeString(&objname, L"");

        __try {

        status = Key_GetName(KeyHandle, &objname, &TruePath, &CopyPath, NULL);

        } __except (EXCEPTION_EXECUTE_HANDLER) {
            status = GetExceptionCode();
        }

        if (NT_SUCCESS(status)) {
            Key_MarkDeletedEx_v2(TruePath, NULL);

            Key_DiscardMergeByPath(TruePath, TRUE);
        }

        Dll_PopTlsNameBuffer(TlsData);

        __sys_NtDeleteKey(KeyHandle);

    }
    else {

        kwti.LastWriteTime.HighPart = DELETE_MARK_HIGH;
        kwti.LastWriteTime.LowPart = DELETE_MARK_LOW;
        status = NtSetInformationKey(
            KeyHandle, KeyWriteTimeInformation,
            &kwti, sizeof(KEY_WRITE_TIME_INFORMATION));

        //
        // refresh all merges
        //

        Key_DiscardMergeByHandle(TlsData, KeyHandle, TRUE);
    }

    //
    // close key handle
    //

    NtClose(KeyHandle);

    return status;
}


//---------------------------------------------------------------------------
// Key_MarkUnDeleted
//---------------------------------------------------------------------------


_FX NTSTATUS Key_MarkUnDeleted(HANDLE KeyHandle)
{
    NTSTATUS status;
    KEY_WRITE_TIME_INFORMATION kwti;
    FILETIME ft;

    //
    // mark key un-deleted by setting its last write time information
    //

    GetSystemTimeAsFileTime(&ft);
    kwti.LastWriteTime.HighPart = ft.dwHighDateTime;
    kwti.LastWriteTime.LowPart  = ft.dwLowDateTime;

    status = NtSetInformationKey(
        KeyHandle, KeyWriteTimeInformation,
        &kwti, sizeof(KEY_WRITE_TIME_INFORMATION));

    if (status == STATUS_ACCESS_DENIED) {

        //
        // if we get STATUS_ACCESS_DENIED, the caller may be using a
        // key handle that was not opened with KEY_SET_VALUE access
        //

        OBJECT_ATTRIBUTES objattrs;
        UNICODE_STRING objname;
        HANDLE handle;

        RtlInitUnicodeString(&objname, L"");
        InitializeObjectAttributes(
            &objattrs, &objname, OBJ_CASE_INSENSITIVE, KeyHandle, NULL);

        status = NtOpenKey(&handle, KEY_SET_VALUE, &objattrs);

        if (NT_SUCCESS(status)) {

            status = NtSetInformationKey(
                handle, KeyWriteTimeInformation,
                &kwti, sizeof(KEY_WRITE_TIME_INFORMATION));

            NtClose(handle);
        }
    }

    return status;
}


//---------------------------------------------------------------------------
// Key_NtDeleteValueKey
//---------------------------------------------------------------------------


_FX NTSTATUS Key_NtDeleteValueKey(
    HANDLE KeyHandle, UNICODE_STRING *ValueName)
{
    ULONG LastError;
    THREAD_DATA *TlsData = Dll_GetTlsData(&LastError);
    NTSTATUS status;
    UNICODE_STRING objname;
    WCHAR *TruePath;
    WCHAR *CopyPath;
    ULONG mp_flags;

    //
    // check if open or closed path
    //

    Dll_PushTlsNameBuffer(TlsData);

    __try {

    //
    // get the full paths for the true and copy keys
    //

    RtlInitUnicodeString(&objname, L"");

    status = Key_GetName(KeyHandle, &objname, &TruePath, &CopyPath, NULL);
    if (! NT_SUCCESS(status))
        __leave;

    mp_flags = SbieDll_MatchPath(L'k', TruePath);

    if (PATH_IS_CLOSED(mp_flags)) {
        status = STATUS_ACCESS_DENIED;
        __leave;
    }

    //
    // finish
    //

    } __except (EXCEPTION_EXECUTE_HANDLER) {
        status = GetExceptionCode();
    }

    if (NT_SUCCESS(status)) {

        if (PATH_IS_OPEN(mp_flags)) {

            status = __sys_NtDeleteValueKey(KeyHandle, ValueName);

        } else if (Key_Delete_v2) { 

            RtlInitUnicodeString(&objname, CopyPath);

            OBJECT_ATTRIBUTES objattrs;
            InitializeObjectAttributes(&objattrs, &objname, OBJ_CASE_INSENSITIVE, NULL, NULL);
            Key_CreatePath(&objattrs, NULL);

            Key_MarkDeletedEx_v2(TruePath, ValueName->Buffer);

            __sys_NtDeleteValueKey(KeyHandle, ValueName);

        } else {

            //
            // mark the value deleted by setting its type to the
            // special value, but only if that value exists
            //

            union {
                KEY_VALUE_BASIC_INFORMATION info;
                UCHAR info_space[64];
            } u;
            ULONG len;

            status = Key_NtQueryValueKey(
                KeyHandle, ValueName,
                KeyValueBasicInformation, &u.info, sizeof(u), &len);

            if (status == STATUS_SUCCESS ||
                status == STATUS_BUFFER_OVERFLOW) {

                status = NtSetValueKey(
                                KeyHandle, ValueName, 0, tzuk, NULL, 0);
            }
        }
    }

    Dll_PopTlsNameBuffer(TlsData);

    SetLastError(LastError);
    return status;
}


//---------------------------------------------------------------------------
// Key_NtSetValueKey
//---------------------------------------------------------------------------


_FX NTSTATUS Key_NtSetValueKey(
    HANDLE KeyHandle,
    UNICODE_STRING *ValueName,
    ULONG TitleIndex,
    ULONG Type,
    void *Data,
    ULONG DataSize)
{
    ULONG LastError;
    THREAD_DATA *TlsData = Dll_GetTlsData(&LastError);

    static const WCHAR *_empty = L"";
    UNICODE_STRING uni;
    NTSTATUS status;

    //
    // otherwise normal processing
    //

    if (ValueName && ValueName->Length && ValueName->Buffer) {

        uni.Length        = ValueName->Length & ~1;
        uni.MaximumLength = uni.Length + sizeof(WCHAR);
        uni.Buffer        = ValueName->Buffer;

        //
        // when installing WinSxS assemblies, the TrustedInstaller
        // component alternately creates and deletes a StoreDirty
        // value in the \REGISTRY\MACHINE\COMPONENTS keys, but
        // occasionally (at least in a sandboxed process) forgets
        // to delete this value, and then complains that it still
        // exists, and aborts.  A workaround is to just not create
        // this value, which causes TrustedInstaller to complain
        // about it, but it still completes the installation.
        //

        if (Dll_ImageType == DLL_IMAGE_TRUSTED_INSTALLER &&
                uni.Length == 20 &&
                _wcsnicmp(uni.Buffer, L"StoreDirty", 10) == 0) {

            return STATUS_SUCCESS;
        }

    } else
        RtlInitUnicodeString(&uni, _empty);

    status = __sys_NtSetValueKey(
        KeyHandle, &uni, 0, Type, Data, DataSize);

    if (status == STATUS_ACCESS_DENIED) {

        //
        // if we get STATUS_ACCESS_DENIED, the caller may be using a
        // TruePath handle that was opened with MAXIMUM_ALLOWED, but
        // reduced to read-only access in our NtCreateKey
        //

        OBJECT_ATTRIBUTES objattrs;
        UNICODE_STRING objname;
        HANDLE handle;

        RtlInitUnicodeString(&objname, L"");
        InitializeObjectAttributes(
            &objattrs, &objname, OBJ_CASE_INSENSITIVE, KeyHandle, NULL);

        status = NtOpenKey(&handle, KEY_WRITE, &objattrs);

        if (NT_SUCCESS(status)) {

            status = __sys_NtSetValueKey(
                handle, &uni, 0, Type, Data, DataSize);

            NtClose(handle);
        }
    }

    //
    // refresh all merges
    //

    Key_DiscardMergeByHandle(TlsData, KeyHandle, FALSE);

    SetLastError(LastError);

    return status;
}


//---------------------------------------------------------------------------
// Key_NtQueryKey
//---------------------------------------------------------------------------


_FX NTSTATUS Key_NtQueryKey(
    HANDLE KeyHandle,
    KEY_INFORMATION_CLASS KeyInformationClass,
    void *KeyInformation,
    ULONG Length,
    ULONG *ResultLength)
{
    NTSTATUS status = Key_NtQueryKeyImpl(   KeyHandle,
                                            KeyInformationClass,
                                            KeyInformation,
                                            Length,
                                            ResultLength );

    status = StopTailCallOptimization(status);

    return status;
}

//---------------------------------------------------------------------------
// Key_NtQueryKeyImpl
//---------------------------------------------------------------------------


_FX NTSTATUS Key_NtQueryKeyImpl(
    HANDLE KeyHandle,
    KEY_INFORMATION_CLASS KeyInformationClass,
    void *KeyInformation,
    ULONG Length,
    ULONG *ResultLength)
{
    ULONG LastError;
    THREAD_DATA *TlsData = Dll_GetTlsData(&LastError);

    NTSTATUS status;
    UNICODE_STRING objname;
    WCHAR *TruePath;
    WCHAR *CopyPath;
    KEY_MERGE *merge;
    ULONG KeyNameLen;
    ULONG MaxNameLen;
    ULONG NumSubkeys;
    ULONG MaxValueNameLen;
    ULONG MaxValueDataLen;
    ULONG NumValues;
    KEY_MERGE_SUBKEY *subkey;
    KEY_MERGE_VALUE *value;

    Dll_PushTlsNameBuffer(TlsData);

    __try {

    //
    // get the full paths for the true and copy keys
    //

    RtlInitUnicodeString(&objname, L"");

    status = Key_GetName(KeyHandle, &objname, &TruePath, &CopyPath, NULL);
    if (! NT_SUCCESS(status))
        __leave;

    //
    // for KeyBasicInformation, KeyNodeInformation, KeyFlagsInformation,
    // we let the system handle the call, then check for the delete mark.
    // They both begin with a LARGE_INTEGER LastWriteTime.
    //

    if (KeyInformationClass == KeyBasicInformation ||
        KeyInformationClass == KeyNodeInformation  ||
        KeyInformationClass == KeyFlagsInformation ||
        
        KeyInformationClass == KeyTrustInformation ||
        KeyInformationClass == KeyLayerInformation)
    {
        status = __sys_NtQueryKey(
            KeyHandle, KeyInformationClass, KeyInformation,
            Length, ResultLength);

        if (NT_SUCCESS(status) || status == STATUS_BUFFER_OVERFLOW)
        {
            if (Key_Delete_v2) {
                if (Key_IsDeleted_v2(TruePath))
                    status = STATUS_KEY_DELETED;
            }
            else if(KeyInformationClass != KeyFlagsInformation) {
                if (IS_DELETE_MARK((LARGE_INTEGER*)KeyInformation))
                    status = STATUS_KEY_DELETED;
            }
        }

        __leave;
    }

    //
    // for KeyNameInformation, we want to place TruePath in the
    // output buffer, even if KeyHandle is a boxed key.  this is
    // for good measure.
    //

    if (KeyInformationClass == KeyNameInformation) {

        KEY_NAME_INFORMATION *info = (KEY_NAME_INFORMATION *)KeyInformation;

        ULONG len = wcslen(TruePath) * sizeof(WCHAR);
        *ResultLength = sizeof(ULONG) + len;
        if (Length < sizeof(ULONG))
            status = STATUS_BUFFER_TOO_SMALL;
        else {
            info->NameLength = len;
            Length -= sizeof(ULONG);
            if (Length >= len) {
                Length = len;
                status = STATUS_SUCCESS;
            } else
                status = STATUS_BUFFER_OVERFLOW;
            memcpy(info->Name, TruePath, Length);
        }

        __leave;
    }

    //
    // handle KeyHandleTagsInformation on Windows 7 (64-bit only?)
    //

    if (KeyInformationClass == KeyHandleTagsInformation &&
            __sys_NtOpenKeyEx) {

        if (Length != sizeof(ULONG))
            status = STATUS_INFO_LENGTH_MISMATCH;
        else {

            *(ULONG *)KeyInformation = 0;
            *ResultLength = sizeof(ULONG);
            status = STATUS_SUCCESS;
        }

        __leave;
    }

    //
    // otherwise we expect to handle at this point, either
    // KeyFullInformation or KeyCachedInformation.  for these we need
    // to build a merged key.  if we get STATUS_BAD_INITIAL_PC, it
    // means there exists only one of the two counterpart keys, so we
    // can let the system do the work, and just check the delete mark
    //

    if (KeyInformationClass != KeyFullInformation &&
        KeyInformationClass != KeyCachedInformation)
    {
        status = STATUS_INVALID_PARAMETER;
        SbieApi_Log(2205, L"NtQueryKeyImpl KeyInfo: %d", KeyInformationClass);
        __leave;
    }

    status = Key_Merge(KeyHandle, TruePath, CopyPath, TRUE, TRUE, &merge);

    if (! NT_SUCCESS(status)) {

        if (status == STATUS_BAD_INITIAL_PC) {

            status = __sys_NtQueryKey(
                KeyHandle, KeyInformationClass, KeyInformation,
                Length, ResultLength);

            if (NT_SUCCESS(status) || status == STATUS_BUFFER_OVERFLOW)
            {
                if (Key_Delete_v2) {
                    if (Key_IsDeleted_v2(TruePath))
                        status = STATUS_KEY_DELETED;
                }
                else {
                    if (IS_DELETE_MARK((LARGE_INTEGER*)KeyInformation))
                        status = STATUS_KEY_DELETED;
                }
            }
        }

        __leave;
    }

    //
    // scan the merge key to find longest subkey name,
    // longest value name and longest value data
    //

    if (KeyInformationClass == KeyCachedInformation) {

        WCHAR *ptr = wcsrchr(merge->name, L'\\') + 1;
        KeyNameLen = wcslen(ptr) * sizeof(WCHAR);
    }

    MaxNameLen = 0;
    NumSubkeys = List_Count(&merge->subkeys);

    subkey = List_Head(&merge->subkeys);
    while (subkey) {
        if (subkey->name_len > MaxNameLen)
            MaxNameLen = subkey->name_len;
        subkey = List_Next(subkey);
    }

    MaxValueNameLen = 0;
    MaxValueDataLen = 0;
    NumValues = List_Count(&merge->values);

    value = List_Head(&merge->values);
    while (value) {
        if (value->name_len > MaxValueNameLen)
            MaxValueNameLen = value->name_len;
        if (value->data_len > MaxValueDataLen)
            MaxValueDataLen = value->data_len;
        value = List_Next(value);
    }

    LeaveCriticalSection(&Key_Handles_CritSec);

    //
    // here we use the merge entry to fill the caller's output buffer
    //

    status = STATUS_SUCCESS;

    if (KeyInformationClass == KeyFullInformation) {

        KEY_FULL_INFORMATION *info =
            (KEY_FULL_INFORMATION *)KeyInformation;

        *ResultLength = FIELD_OFFSET(KEY_FULL_INFORMATION, Class);
        if (Length < *ResultLength)
            status = STATUS_BUFFER_TOO_SMALL;
        else {

            info->LastWriteTime.QuadPart = merge->last_write_time.QuadPart;
            info->TitleIndex = 0;
            info->ClassOffset = -1;
            info->ClassLength = 0;
            info->SubKeys = NumSubkeys;
            info->MaxNameLen = MaxNameLen;
            info->MaxClassLen = 0;
            info->Values = NumValues;
            info->MaxValueNameLen = MaxValueNameLen;
            info->MaxValueDataLen = MaxValueDataLen;
        }

    } else if (KeyInformationClass == KeyCachedInformation) {

        // KeyCachedInformation needs exactly 40 bytes, which is
        // sizeof(KEY_CACHED_INFORMATION) including the Name[1] field.
        // But it doesn't actually fill that field...

        KEY_CACHED_INFORMATION *info =
            (KEY_CACHED_INFORMATION *)KeyInformation;

        *ResultLength = sizeof(KEY_CACHED_INFORMATION);
        if (Length < *ResultLength)
            status = STATUS_BUFFER_TOO_SMALL;
        else {

            info->LastWriteTime.QuadPart = merge->last_write_time.QuadPart;
            info->TitleIndex = 0;
            info->SubKeys = NumSubkeys;
            info->MaxNameLen = MaxNameLen;
            info->Values = NumValues;
            info->MaxValueNameLen = MaxValueNameLen;
            info->MaxValueDataLen = MaxValueDataLen;
            info->NameLength = KeyNameLen;
        }
    }

    //
    // finish
    //

    } __except (EXCEPTION_EXECUTE_HANDLER) {
        status = GetExceptionCode();
    }

    Dll_PopTlsNameBuffer(TlsData);
    SetLastError(LastError);
    return status;
}


//---------------------------------------------------------------------------
// Key_NtEnumerateKey
//---------------------------------------------------------------------------


_FX NTSTATUS Key_NtEnumerateKey(
    HANDLE KeyHandle,
    ULONG Index,
    KEY_INFORMATION_CLASS KeyInformationClass,
    void *KeyInformation,
    ULONG Length,
    ULONG *ResultLength)
{
    static const WCHAR *_Registry_User = L"\\registry\\user";
    static const ULONG _Registry_User_Len = 14;

    ULONG LastError;
    THREAD_DATA *TlsData = Dll_GetTlsData(&LastError);

    NTSTATUS status;
    UNICODE_STRING objname;
    OBJECT_ATTRIBUTES objattrs;
    WCHAR *TruePath;
    WCHAR *CopyPath;
    KEY_MERGE *merge;
    KEY_MERGE_SUBKEY *subkey;
    ULONG len, SaveIndex;
    WCHAR *SubkeyPath;
    ULONG SubkeyPathLen;
    HANDLE SubkeyHandle;
    LARGE_INTEGER SubkeyLastWriteTime;

    Dll_PushTlsNameBuffer(TlsData);

    __try {

    //
    // validate KeyInformationClass.  the system NtEnumerateKey only
    // handles the following three classes
    //

    if (KeyInformationClass != KeyBasicInformation &&
        KeyInformationClass != KeyNodeInformation &&
        KeyInformationClass != KeyFullInformation)
    {
        SbieApi_Log(2205, L"NtEnumerateKey KeyInfo: %d", KeyInformationClass);
        status = STATUS_INVALID_PARAMETER;
        __leave;
    }

    //
    // get the full paths for the true and copy keys
    //

    RtlInitUnicodeString(&objname, L"");

    status = Key_GetName(KeyHandle, &objname, &TruePath, &CopyPath, NULL);
    if (! NT_SUCCESS(status))
        __leave;

    //
    // we need to build a merged key.  if we get STATUS_BAD_INITIAL_PC,
    // it means there exists only one of the two counterpart keys, so we
    // can let the system do the work, and just check the delete mark
    //

    status = Key_Merge(KeyHandle, TruePath, CopyPath, TRUE, FALSE, &merge);

    if (! NT_SUCCESS(status)) {

        if (status == STATUS_BAD_INITIAL_PC) {

            status = __sys_NtEnumerateKey(
                KeyHandle, Index, KeyInformationClass, KeyInformation,
                Length, ResultLength);

            if (NT_SUCCESS(status) || status == STATUS_BUFFER_OVERFLOW) 
            {
                if (Key_Delete_v2) {
                    if (Key_IsDeleted_v2(TruePath))
                        status = STATUS_KEY_DELETED;
                }
                else {
                    if (IS_DELETE_MARK((LARGE_INTEGER*)KeyInformation))
                        status = STATUS_KEY_DELETED;
                }
            }
        }

        __leave;
    }

    //
    // find the key with the requested index
    //

    SaveIndex = Index;

    if (merge->last_index && Index >= merge->last_index) {
        Index -= merge->last_index;
        subkey = (KEY_MERGE_SUBKEY *)merge->last_subkey;
    } else
        subkey = List_Head(&merge->subkeys);

    while (Index > 0 && subkey) {
        subkey = List_Next(subkey);
        --Index;
    }

    if (subkey) {

        len = merge->name_len
            + sizeof(WCHAR)     // backspace
            + subkey->name_len
            + sizeof(WCHAR);    // null

        SubkeyPath = Dll_Alloc(len);

        objname.MaximumLength = (USHORT)len;
        objname.Length = objname.MaximumLength - sizeof(WCHAR);
        objname.Buffer = SubkeyPath;

        memcpy(SubkeyPath, merge->name, merge->name_len);
        SubkeyPath[merge->name_len / sizeof(WCHAR)] = L'\\';
        memcpy(SubkeyPath + merge->name_len / sizeof(WCHAR) + 1,
               subkey->name, subkey->name_len + sizeof(WCHAR));

        SubkeyPathLen = (merge->name_len + sizeof(WCHAR) + subkey->name_len)
                      / sizeof(WCHAR);

        SubkeyLastWriteTime = subkey->LastWriteTime;
    }

    //
    // unlock the Key_Handles list before we query the subkey.
    //
    // if access to the subkey is denied, we fake a result that
    // only contains the name of the subkey, in Key_NtEnumerateKeyFake.
    //
    // we also do this if the parent key is \REGISTRY\USER, because
    // calling NtQueryKey on its subkeys sometimes yields incorrect
    // results in the Name field ("$$$PROTO.HIV" is returned as
    // the name of some of the hives below \REGISTRY\USER)
    //

    LeaveCriticalSection(&Key_Handles_CritSec);

    if (! subkey) {

        merge->last_index = 0;
        merge->last_subkey = NULL;

        status = STATUS_NO_MORE_ENTRIES;
        __leave;
    }

    merge->last_index = SaveIndex;
    merge->last_subkey = subkey;

    //
    // for some keys that are a hive root, the registry returns correct
    // names when the keys are enumerated from the parent key, but returns
    // $$$PROTO.HIV as the key name, when the child key itself is queried.
    //
    // since we don't do NtEnumerateKey, but actually use NtQueryKey on
    // the child key of the merge, we run into this problem.  so for the
    // known cases where the child key is a hive key, we set here
    // STATUS_ACCESS_DENIED to indicate a need for NtEnumerateKeyFake
    //

    if (merge->name_len == _Registry_User_Len * sizeof(WCHAR) &&
        _wcsnicmp(merge->name, _Registry_User, _Registry_User_Len) == 0) {

        //
        // caller is enumerating the keys below \REGISTRY\USER, which
        // are all hive root keys
        //

        status = STATUS_ACCESS_DENIED;

    } else if (SubkeyPathLen == Key_System_Len &&
            _wcsnicmp(SubkeyPath, Key_System, Key_System_Len) == 0) {

        //
        // caller is enumerating the SYSTEM child of \REGISTRY\MACHINE
        //

        status = STATUS_ACCESS_DENIED;

    } else {

        if ((KeyInformationClass == KeyBasicInformation ||
             KeyInformationClass == KeyNodeInformation)) {

            //
            // if we can serve the request from our cached information
            // (which is only when the key has no title or class), then
            // do so without invoking NtOpenKey/NtQueryKey
            //

            if (! subkey->TitleOrClass) {

                status = STATUS_ACCESS_DENIED;

            } else {

                //
                // if the subkey to query is HKU\S-x-y-z\SOFTWARE\CLASSES,
                // then actually the HKU\S-x-y-z_CLASSES key will be opened.
                // Thus for KeyBasicInformation and KeyNodeInformation, which
                // are really used to only get the name of the subkey, the
                // returned name will be wrong ("current_classes" instead
                // of "classes").  We fake a result for this case too
                //

                if (SubkeyPathLen > _Registry_User_Len + 2 &&
                    SubkeyPath[_Registry_User_Len] == L'\\' &&
                (   SubkeyPath[_Registry_User_Len + 1] == L'S' ||
                    SubkeyPath[_Registry_User_Len + 1] == L's') &&
                    SubkeyPath[_Registry_User_Len + 2] == L'-' &&
                        _wcsnicmp(SubkeyPath, _Registry_User,
                                              _Registry_User_Len) == 0) {

                    WCHAR *backslash =
                    wcschr(SubkeyPath + _Registry_User_Len + 1, L'\\');
                    if (backslash &&
                        _wcsicmp(backslash, L"\\Software\\Classes") == 0) {

                        status = STATUS_ACCESS_DENIED;
                    }
                }
            }
        }

        if (NT_SUCCESS(status)) {

            InitializeObjectAttributes(
                &objattrs, &objname, OBJ_CASE_INSENSITIVE, NULL, NULL);

            status = Key_NtOpenKeyImpl(&SubkeyHandle,
                               Key_GetWow64Flag(SubkeyPath, KEY_READ),
                               &objattrs);
        }
    }

    if (NT_SUCCESS(status)) {

        status = Key_NtQueryKeyImpl(
            SubkeyHandle, KeyInformationClass, KeyInformation,
            Length, ResultLength);

        File_NtCloseImpl(SubkeyHandle);

    } else if (status == STATUS_ACCESS_DENIED) {

        //
        // if we can't open the subkey due to access denied, then we
        // fake the result.  (this happens for instance when looking
        // at the SECURITY subkey of the \REGISTRY\MACHINE key)
        //

        status = Key_NtEnumerateKeyFake(
            SubkeyPath, SubkeyPathLen, KeyInformationClass, KeyInformation,
            Length, ResultLength, &SubkeyLastWriteTime);
    }

    Dll_Free(SubkeyPath);

    //
    // finish
    //

    } __except (EXCEPTION_EXECUTE_HANDLER) {
        status = GetExceptionCode();
    }

    Dll_PopTlsNameBuffer(TlsData);
    SetLastError(LastError);
    return status;
}


//---------------------------------------------------------------------------
// Key_NtEnumerateKeyFake
//---------------------------------------------------------------------------


_FX NTSTATUS Key_NtEnumerateKeyFake(
    const WCHAR *KeyPath,
    ULONG KeyPathLen,
    KEY_INFORMATION_CLASS KeyInformationClass,
    void *KeyInformation,
    ULONG Length,
    ULONG *ResultLength,
    LARGE_INTEGER *LastWriteTime)
{
    NTSTATUS status;
    ULONG MinLength;
    ULONG *NameLength;
    WCHAR *Name;
    WCHAR *subkey_name;
    ULONG subkey_name_len;

    __try {

    if (KeyInformationClass == KeyFullInformation) {
        subkey_name = NULL;
        subkey_name_len = 0;
    } else {
        subkey_name = wcsrchr(KeyPath, L'\\') + 1;
        subkey_name_len = (KeyPathLen - (subkey_name - KeyPath))
                        * sizeof(WCHAR);
    }

    if (KeyInformationClass == KeyBasicInformation) {
        MinLength = sizeof(KEY_BASIC_INFORMATION);
        NameLength = &((KEY_BASIC_INFORMATION *)KeyInformation)->NameLength;
        Name = &((KEY_BASIC_INFORMATION *)KeyInformation)->Name[0];

    } else if (KeyInformationClass == KeyNodeInformation) {
        MinLength = sizeof(KEY_NODE_INFORMATION);
        NameLength = &((KEY_NODE_INFORMATION *)KeyInformation)->NameLength;
        Name = &((KEY_NODE_INFORMATION *)KeyInformation)->Name[0];

    } else if (KeyInformationClass == KeyFullInformation) {
        MinLength = sizeof(KEY_FULL_INFORMATION);
        NameLength = NULL;
        Name = NULL;
    }

    *ResultLength = MinLength + subkey_name_len;
    if (Length < MinLength) {
        status = STATUS_BUFFER_TOO_SMALL;
        __leave;
    }

    memzero(KeyInformation, MinLength);
    status = STATUS_SUCCESS;

    *(LARGE_INTEGER *)KeyInformation = *LastWriteTime;

    if (KeyInformationClass == KeyNodeInformation)
        ((KEY_NODE_INFORMATION *)KeyInformation)->ClassOffset = -1;

    if (KeyInformationClass != KeyFullInformation) {
        Length -= MinLength;
        if (Length >= subkey_name_len)
            Length = subkey_name_len;
        else
            status = STATUS_BUFFER_OVERFLOW;
        memcpy(Name, subkey_name, Length);
        *NameLength = subkey_name_len;
    }

    } __except (EXCEPTION_EXECUTE_HANDLER) {
        status = GetExceptionCode();
    }

    return status;
}


//---------------------------------------------------------------------------
// Key_NtQueryValueKey
//---------------------------------------------------------------------------


_FX NTSTATUS Key_NtQueryValueKey(
    HANDLE KeyHandle,
    UNICODE_STRING *ValueName,
    KEY_VALUE_INFORMATION_CLASS KeyValueInformationClass,
    void *KeyValueInformation,
    ULONG Length,
    ULONG *ResultLength)
{
    ULONG LastError;
    THREAD_DATA *TlsData = Dll_GetTlsData(&LastError);

    NTSTATUS status;
    UNICODE_STRING objname;
    WCHAR *TruePath;
    WCHAR *CopyPath;
    KEY_MERGE *merge;
    KEY_MERGE_VALUE *value;
    WCHAR *ValueNameBuf;
    ULONG ValueNameLen1;
    ULONG ValueNameLen2;
    BOOLEAN key_handles_locked;

    Dll_PushTlsNameBuffer(TlsData);

    key_handles_locked = FALSE;

    __try {

    //
    // prepare ValueName parameter for easy access
    //

    if (ValueName && ValueName->Buffer) {

        ValueNameBuf  = ValueName->Buffer;
        ValueNameLen1 = ValueName->Length / sizeof(WCHAR);
        ValueNameLen2 = ValueNameLen1 * sizeof(WCHAR);

    } else {

        ValueNameBuf  = L"";
        ValueNameLen1 = 0;
        ValueNameLen2 = 0;
    }

    //
    // get the full paths for the true and copy keys
    //

    RtlInitUnicodeString(&objname, L"");

    status = Key_GetName(KeyHandle, &objname, &TruePath, &CopyPath, NULL);
    if (! NT_SUCCESS(status))
        __leave;

    //
    // special hacks on NtQueryValueKey
    //

    if (KeyValueInformationClass == KeyValuePartialInformation &&
        KeyValueInformation && ResultLength) {

        if (Dll_ImageType == DLL_IMAGE_INTERNET_EXPLORER) {

            status = Key_NtQueryValueKeyFakeForInternetExplorer(
                TruePath, ValueNameBuf, ValueNameLen1,
                KeyValueInformation, Length, ResultLength);

            if (status != STATUS_BAD_INITIAL_PC)
                __leave;
        }

        if (Dll_ImageType == DLL_IMAGE_ACROBAT_READER ||
            Dll_ImageType == DLL_IMAGE_PLUGIN_CONTAINER ||
            Dll_ImageType == DLL_IMAGE_GOOGLE_CHROME ||
            Dll_ImageType == DLL_IMAGE_INTERNET_EXPLORER) { // AcroPDF.dll

            status = Key_NtQueryValueKeyFakeForAcrobatReader(
                TruePath, ValueNameBuf, ValueNameLen1,
                KeyValueInformation, Length, ResultLength);

            if (status != STATUS_BAD_INITIAL_PC)
                __leave;
        }

        if (TlsData->proc_create_process) {

            status = Key_NtQueryValueKeyFakeForCreateProcess(
                TruePath, ValueNameBuf, ValueNameLen1,
                KeyValueInformation, Length, ResultLength);

            if (status != STATUS_BAD_INITIAL_PC)
                __leave;
        }
    }

    //
    // get or create the merge entry for this key handle.  if we get
    // STATUS_BAD_INITIAL_PC, it means there exists only one of the
    // counterpart keys, so we can let the system do the work, and
    // just check the delete mark
    //

    status = Key_Merge(KeyHandle, TruePath, CopyPath, FALSE, TRUE, &merge);

    if (! NT_SUCCESS(status)) {

        if (status == STATUS_BAD_INITIAL_PC) {

            status = __sys_NtQueryValueKey(
                KeyHandle, ValueName,
                KeyValueInformationClass, KeyValueInformation,
                Length, ResultLength);

            if (NT_SUCCESS(status) || status == STATUS_BUFFER_OVERFLOW)
            {
                if (Key_Delete_v2) {
                    if(Key_IsDeletedEx_v2(TruePath, ValueNameBuf, TRUE))
                        status = STATUS_OBJECT_NAME_NOT_FOUND;
                }
                else {
                    if (Key_CheckDeletedValue(KeyValueInformationClass, KeyValueInformation))
                        status = STATUS_OBJECT_NAME_NOT_FOUND;
                }
            }
        }

        __leave;
    }

    //
    // scan the merge to find the requested value name.  note that
    // ValueName comes from the caller so we have to use case-insensitive
    // comparison for it.
    //

    key_handles_locked = TRUE;

    value = List_Head(&merge->values);
    while (value) {
        if (value->name_len == ValueNameLen2 &&
                _wcsnicmp(value->name, ValueNameBuf, ValueNameLen1) == 0)
            break;
        value = List_Next(value);
    }

    if (value) {
        status = Key_GetMergedValue(
            value, KeyValueInformationClass, KeyValueInformation,
            Length, ResultLength);
    } else
        status = STATUS_OBJECT_NAME_NOT_FOUND;

    //
    // finish
    //

    } __except (EXCEPTION_EXECUTE_HANDLER) {
        status = GetExceptionCode();
    }

    if (key_handles_locked)
        LeaveCriticalSection(&Key_Handles_CritSec);

    Dll_PopTlsNameBuffer(TlsData);
    SetLastError(LastError);
    return status;
}


//---------------------------------------------------------------------------
// Key_NtQueryValueKeyFakeForInternetExplorer
//---------------------------------------------------------------------------


_FX NTSTATUS Key_NtQueryValueKeyFakeForInternetExplorer(
    const WCHAR *TruePath,
    const WCHAR *ValueNameBuf,
    ULONG ValueNameLen,
    void *KeyValueInformation,
    ULONG Length,
    ULONG *ResultLength)
{
    ULONG ValueType = 0;
    ULONG ValueData;

    if (Length < sizeof(ULONG) * 4) {

        ;

    //
    // for Internet Explorer 9 we need to disable its detours on USER32
    // functions as it cannot handle our own hooks
    //

    } else if (ValueNameLen == 13 &&
                    _wcsicmp(ValueNameBuf, L"DetourDialogs") == 0) {

        ValueType = REG_DWORD;
        ValueData = 0;                  // detour dialogs OFF

    //
    // we fake registry values related to protected mode only if we
    // did not actually drop rights from an admin account process.
    // this makes the non-admin sandboxed IE to still use the "low"
    // IE folders, while running with protected mode off.
    //
    // alternatively, if rights were actually dropped, then we install
    // our hook on NtQueryInformationToken to fake admin privileges,
    // and this will turn protected mode off automatically.
    // see Secure_Init
    //

    } else if (Dll_ProcessFlags & SBIE_FLAG_RIGHTS_DROPPED) {

        ;

    //
    // hack:  if the Internet Explorer process is checking for
    // value 2500, the Protected Mode setting, in the registry key
    // ...\Microsoft\Windows\CurrentVersion\Internet Settings\Zones
    // then always return 3 - protected mode OFF
    //

    } else if (ValueNameLen == 4 && _wcsicmp(ValueNameBuf, L"2500") == 0) {

        static const WCHAR *_Zones =
            L"\\Microsoft\\Windows\\CurrentVersion\\"
                L"Internet Settings\\Zones";

        BOOLEAN found = FALSE;
        const WCHAR *backslash = TruePath;
        while (1) {
            backslash = wcschr(backslash + 1, L'\\');
            if (! backslash)
                break;
            if (_wcsnicmp(backslash, _Zones, wcslen(_Zones)) == 0) {
                found = TRUE;
                break;
            }
        }

        if (found) {

            ValueType = REG_DWORD;
            ValueData = 3;              // protected mode OFF
        }

    //
    // hack:  if the Internet Explorer process is checking for value
    // ProtectedModeOffForAllZones, return value 1.  this alternate
    // approach is sometimes used instead of the approach above
    //

    } else if (ValueNameLen == 27 &&
            _wcsicmp(ValueNameBuf, L"ProtectedModeOffForAllZones") == 0) {

        ValueType = REG_DWORD;
        ValueData = 1;                  // protected mode OFF

    //
    // hack:  if the Internet Explorer process is checking for value
    // NoProtectedModeBanner, return value 1, to prevent the gold bar
    // warning that protected mode is turned off
    //

    } else if (ValueNameLen == 21 &&
                    _wcsicmp(ValueNameBuf, L"NoProtectedModeBanner") == 0) {

        ValueType = REG_DWORD;
        ValueData = 1;                  // don't show gold bar
    }

    //
    // finish
    //

    if (ValueType == REG_DWORD) {

        KEY_VALUE_PARTIAL_INFORMATION *kvpi =
            (KEY_VALUE_PARTIAL_INFORMATION *)KeyValueInformation;

        kvpi->TitleIndex     = 0;
        kvpi->Type           = ValueType;
        kvpi->DataLength     = sizeof(ULONG);
        *(ULONG *)kvpi->Data = ValueData;

        *ResultLength = sizeof(ULONG) * 4;

        return STATUS_SUCCESS;
    }

    return STATUS_BAD_INITIAL_PC;
}


//---------------------------------------------------------------------------
// Key_NtQueryValueKeyFakeForAcrobatReader
//---------------------------------------------------------------------------


_FX NTSTATUS Key_NtQueryValueKeyFakeForAcrobatReader(
    const WCHAR *TruePath,
    const WCHAR *ValueNameBuf,
    ULONG ValueNameLen,
    void *KeyValueInformation,
    ULONG Length,
    ULONG *ResultLength)
{
    ULONG ValueType = 0;
    ULONG ValueData;

    if (Length < sizeof(ULONG) * 4) {

        ;

    //
    // for Adobe Acrobat Reader we need to disable protected mode
    //

    } else if (ValueNameLen == 14 &&
                    _wcsicmp(ValueNameBuf, L"bProtectedMode") == 0) {

        ValueType = REG_DWORD;
        ValueData = 0;                  // protected mode OFF

    //
    // for Adobe Acrobat Reader we need to disable update check
    //

    } else if (ValueNameLen == 12 &&
                    _wcsicmp(ValueNameBuf, L"iCheckReader") == 0) {

        ValueType = REG_DWORD;
        ValueData = 0;                  // update check OFF
    }

    //
    // finish
    //

    if (ValueType == REG_DWORD) {

        KEY_VALUE_PARTIAL_INFORMATION *kvpi =
            (KEY_VALUE_PARTIAL_INFORMATION *)KeyValueInformation;

        kvpi->TitleIndex     = 0;
        kvpi->Type           = ValueType;
        kvpi->DataLength     = sizeof(ULONG);
        *(ULONG *)kvpi->Data = ValueData;

        *ResultLength = sizeof(ULONG) * 4;

        return STATUS_SUCCESS;
    }

    return STATUS_BAD_INITIAL_PC;
}


//---------------------------------------------------------------------------
// Key_NtQueryValueKeyFakeForCreateProcess
//---------------------------------------------------------------------------


_FX NTSTATUS Key_NtQueryValueKeyFakeForCreateProcess(
    const WCHAR *TruePath,
    const WCHAR *ValueNameBuf,
    ULONG ValueNameLen,
    void *KeyValueInformation,
    ULONG Length,
    ULONG *ResultLength)
{
    ULONG ValueType = 0;
    ULONG ValueData;

    if (Length < sizeof(ULONG) * 4) {

        ;

    //
    // the AuthenticodeEnabled registry value for SRP is queried during
    // CreateProcess processing.  if enabled it causes a recursive
    // CreateProcess call to SandboxieCrypto, which will hang if
    // loading SandboxieRpcSs, a dependency of SandboxieCrypto
    //

    } else if (ValueNameLen == 19 &&
                    _wcsicmp(ValueNameBuf, L"AuthenticodeEnabled") == 0) {

        ValueType = REG_DWORD;
        ValueData = 0;                  // AuthenticodeEnabled OFF
    }

    //
    // finish
    //

    if (ValueType == REG_DWORD) {

        KEY_VALUE_PARTIAL_INFORMATION *kvpi =
            (KEY_VALUE_PARTIAL_INFORMATION *)KeyValueInformation;

        kvpi->TitleIndex     = 0;
        kvpi->Type           = ValueType;
        kvpi->DataLength     = sizeof(ULONG);
        *(ULONG *)kvpi->Data = ValueData;

        *ResultLength = sizeof(ULONG) * 4;

        return STATUS_SUCCESS;
    }

    return STATUS_BAD_INITIAL_PC;
}


//---------------------------------------------------------------------------
// Key_NtEnumerateValueKey
//---------------------------------------------------------------------------


_FX NTSTATUS Key_NtEnumerateValueKey(
    HANDLE KeyHandle,
    ULONG Index,
    KEY_VALUE_INFORMATION_CLASS KeyValueInformationClass,
    void *KeyValueInformation,
    ULONG Length,
    ULONG *ResultLength)
{
    ULONG LastError;
    THREAD_DATA *TlsData = Dll_GetTlsData(&LastError);

    NTSTATUS status;
    UNICODE_STRING objname;
    WCHAR *TruePath;
    WCHAR *CopyPath;
    KEY_MERGE *merge;
    KEY_MERGE_VALUE *value;
    BOOLEAN key_handles_locked;

    Dll_PushTlsNameBuffer(TlsData);

    key_handles_locked = FALSE;

    __try {

    //
    // get the full paths for the true and copy keys
    //

    RtlInitUnicodeString(&objname, L"");

    status = Key_GetName(KeyHandle, &objname, &TruePath, &CopyPath, NULL);
    if (! NT_SUCCESS(status))
        __leave;

    //
    // get or create the merge entry for this key handle.  if we get
    // STATUS_BAD_INITIAL_PC, it means there exists only one of the
    // counterpart keys, so we can let the system do the work, and
    // just check the delete mark
    //

    status = Key_Merge(KeyHandle, TruePath, CopyPath, FALSE, TRUE, &merge);

    if (! NT_SUCCESS(status)) {

        if (status == STATUS_BAD_INITIAL_PC) {

            status = __sys_NtEnumerateValueKey(
                KeyHandle, Index,
                KeyValueInformationClass, KeyValueInformation,
                Length, ResultLength);

            if (NT_SUCCESS(status) || status == STATUS_BUFFER_OVERFLOW)
            {
                if (Key_Delete_v2) {

                    WCHAR* ValueName;

                    if (KeyValueInformationClass == KeyValueBasicInformation)
                        ValueName = ((KEY_VALUE_BASIC_INFORMATION *)KeyValueInformation)->Name;
                    else if (KeyValueInformationClass == KeyValueFullInformation)
                        ValueName = ((KEY_VALUE_FULL_INFORMATION *)KeyValueInformation)->Name;
                    else
                        ValueName = 0;

                    if(ValueName && Key_IsDeletedEx_v2(TruePath, ValueName, TRUE))
                        status = STATUS_OBJECT_NAME_NOT_FOUND;
                }
                else {
                    if (Key_CheckDeletedValue(KeyValueInformationClass, KeyValueInformation))
                        status = STATUS_OBJECT_NAME_NOT_FOUND;
                }
            }
        }

        __leave;
    }

    //
    // scan the merge to find the requested value name
    //

    key_handles_locked = TRUE;

    value = List_Head(&merge->values);
    while (Index > 0 && value) {
        value = List_Next(value);
        --Index;
    }

    if (value) {
        status = Key_GetMergedValue(
            value, KeyValueInformationClass, KeyValueInformation,
            Length, ResultLength);
    } else
        status = STATUS_NO_MORE_ENTRIES;

    //
    // finish
    //

    } __except (EXCEPTION_EXECUTE_HANDLER) {
        status = GetExceptionCode();
    }

    if (key_handles_locked)
        LeaveCriticalSection(&Key_Handles_CritSec);

    Dll_PopTlsNameBuffer(TlsData);
    SetLastError(LastError);
    return status;
}


//---------------------------------------------------------------------------
// Key_CheckDeletedValue
//---------------------------------------------------------------------------


_FX BOOLEAN Key_CheckDeletedValue(
    KEY_VALUE_INFORMATION_CLASS KeyValueInformationClass, void *info)
{
    ULONG type;

    if (KeyValueInformationClass == KeyValueBasicInformation)
        type = ((KEY_VALUE_BASIC_INFORMATION *)info)->Type;
    else if (KeyValueInformationClass == KeyValueFullInformation)
        type = ((KEY_VALUE_FULL_INFORMATION *)info)->Type;
    else if (KeyValueInformationClass == KeyValuePartialInformation)
        type = ((KEY_VALUE_PARTIAL_INFORMATION *)info)->Type;
    else if (KeyValueInformationClass == KeyValuePartialInformationAlign64)
        type = ((KEY_VALUE_PARTIAL_INFORMATION_ALIGN64 *)info)->Type;
    else
        type = 0;

    return (type == tzuk);
}


//---------------------------------------------------------------------------
// Key_NtQueryMultipleValueKey
//---------------------------------------------------------------------------


_FX NTSTATUS Key_NtQueryMultipleValueKey(
    HANDLE KeyHandle,
    KEY_VALUE_ENTRY *ValueEntries,
    ULONG EntryCount,
    void *ValueBuffer,
    ULONG *BufferLength,
    ULONG *ResultLength)
{
    ULONG LastError;
    THREAD_DATA *TlsData = Dll_GetTlsData(&LastError);

    NTSTATUS status;
    UNICODE_STRING objname;
    WCHAR *TruePath;
    WCHAR *CopyPath;
    KEY_MERGE *merge;
    KEY_MERGE_VALUE *value;
    WCHAR *ValueNameBuf;
    ULONG ValueNameLen;
    ULONG used_len, need_len;
    BOOLEAN key_handles_locked;

    Dll_PushTlsNameBuffer(TlsData);

    key_handles_locked = FALSE;

    __try {

    //
    // get the full paths for the true and copy keys
    //

    RtlInitUnicodeString(&objname, L"");

    status = Key_GetName(KeyHandle, &objname, &TruePath, &CopyPath, NULL);
    if (! NT_SUCCESS(status))
        __leave;

    //
    // get or create the merge entry for this key handle.  if we get
    // STATUS_BAD_INITIAL_PC, it means there exists only one of the
    // counterpart keys, so we can let the system do the work, and
    // just check the delete mark
    //

    status = Key_Merge(KeyHandle, TruePath, CopyPath, FALSE, TRUE, &merge);

    if (! NT_SUCCESS(status)) {

        if (status == STATUS_BAD_INITIAL_PC) {

            status = __sys_NtQueryMultipleValueKey(
                KeyHandle, ValueEntries, EntryCount,
                ValueBuffer, BufferLength, ResultLength);
        }

        __leave;
    }

    //
    // fill the value buffer
    //

    key_handles_locked = TRUE;

    used_len = 0;
    need_len = 0;

    while (EntryCount > 0) {

        //
        // scan the merge to find the requested value name
        //

        if (ValueEntries->ValueName && ValueEntries->ValueName->Buffer) {
            ValueNameBuf = ValueEntries->ValueName->Buffer;
            ValueNameLen = wcslen(ValueNameBuf) * sizeof(WCHAR);
        } else {
            ValueNameBuf = L"";
            ValueNameLen = 0;
        }

        value = List_Head(&merge->values);
        while (value) {
            if (value->name_len == ValueNameLen &&
                    _wcsicmp(value->name, ValueNameBuf) == 0)
                break;
            value = List_Next(value);
        }

        if (! value) {
            status = STATUS_OBJECT_NAME_NOT_FOUND;
            break;
        }

        //
        // write the value we found into the output buffer.  note that
        // we round each entry length up to a multiple of ULONG
        //

        if (need_len + value->data_len > *BufferLength) {

            status = STATUS_BUFFER_OVERFLOW;

        } else {

            if (value->data_len) {

                memcpy((UCHAR *)ValueBuffer + need_len,
                       value->data_ptr, value->data_len);

                used_len = (used_len + value->data_len + sizeof(ULONG) - 1)
                         & ~(sizeof(ULONG) - 1);
            }

            ValueEntries->DataLength = value->data_len;
            ValueEntries->DataOffset = need_len;
            ValueEntries->Type = value->data_type;
        }

        need_len = (need_len + value->data_len + sizeof(ULONG) - 1)
                 & ~(sizeof(ULONG) - 1);

        ++ValueEntries;
        --EntryCount;
    }

    if (NT_SUCCESS(status) || (status == STATUS_BUFFER_OVERFLOW)) {
        *BufferLength = used_len;
        if (ResultLength)
            *ResultLength = need_len;
    }

    //
    // finish
    //

    } __except (EXCEPTION_EXECUTE_HANDLER) {
        status = GetExceptionCode();
    }

    if (key_handles_locked)
        LeaveCriticalSection(&Key_Handles_CritSec);

    Dll_PopTlsNameBuffer(TlsData);
    SetLastError(LastError);
    return status;
}


//---------------------------------------------------------------------------
// Key_NtNotifyChangeKey
//---------------------------------------------------------------------------


_FX NTSTATUS Key_NtNotifyChangeKey(
    HANDLE KeyHandle,
    HANDLE Event OPTIONAL,
    PIO_APC_ROUTINE ApcRoutine OPTIONAL,
    PVOID ApcContext OPTIONAL,
    PIO_STATUS_BLOCK IoStatusBlock,
    ULONG CompletionFilter,
    BOOLEAN WatchTree,
    PVOID Buffer,
    ULONG BufferSize,
    BOOLEAN Asynchronous)
{
    return Key_NtNotifyChangeMultipleKeys(
        KeyHandle, 0, NULL,
        Event, ApcRoutine, ApcContext, IoStatusBlock,
        CompletionFilter, WatchTree, Buffer, BufferSize, Asynchronous);
}


//---------------------------------------------------------------------------
// Key_NtNotifyChangeMultipleKeys
//---------------------------------------------------------------------------


_FX NTSTATUS Key_NtNotifyChangeMultipleKeys(
    HANDLE MasterKeyHandle,
    ULONG Count,
    OBJECT_ATTRIBUTES *SlaveObjects,
    HANDLE Event OPTIONAL,
    PIO_APC_ROUTINE ApcRoutine OPTIONAL,
    PVOID ApcContext OPTIONAL,
    PIO_STATUS_BLOCK IoStatusBlock,
    ULONG CompletionFilter,
    BOOLEAN WatchTree,
    PVOID Buffer,
    ULONG BufferSize,
    BOOLEAN Asynchronous)
{
    ULONG LastError;
    THREAD_DATA *TlsData = Dll_GetTlsData(&LastError);

    HANDLE OriginalMasterKeyHandle = MasterKeyHandle;

    NTSTATUS status;
    OBJECT_ATTRIBUTES objattrs;
    UNICODE_STRING objname;
    WCHAR *TruePath;
    WCHAR *CopyPath;
    BOOLEAN IsBoxedPath;

    //
    // check if the notify key is in the sandbox
    //

    Dll_PushTlsNameBuffer(TlsData);

    __try {

    RtlInitUnicodeString(&objname, L"");
    InitializeObjectAttributes(
        &objattrs, &objname, OBJ_CASE_INSENSITIVE, NULL, NULL);

    status = Key_GetName(
        MasterKeyHandle, &objname, &TruePath, &CopyPath, &IsBoxedPath);
    if (! NT_SUCCESS(status))
        __leave;

    if (! IsBoxedPath) {

        //
        // if this is an asynchronous call that doesn't specify an
        // event or an APC, then the original key handle itself is
        // supposed to become signalled.  but since we are going to
        // listen on a handle different than what the caller passed,
        // then at least we should complain about this
        //

        if (Asynchronous && (! Event) && (! ApcRoutine)) {

            SbieApi_Log(2205, L"NtNotifyChangeMultipleKeys");
        }

        //
        // open the key.  this will create a copy key, if necessary.
        // note that this handle will never be closed.
        //

        RtlInitUnicodeString(&objname, CopyPath);

        status = NtOpenKey(&MasterKeyHandle, KEY_ALL_ACCESS, &objattrs);

        if (status == STATUS_ACCESS_DENIED) {

            //
            // if this is a system key that also happens to be an
            // OpenKeyPath, then opening with KEY_ALL_ACCESS fails
            //

            status = NtOpenKey(&MasterKeyHandle, KEY_READ, &objattrs);
        }

        //
        // we can get access denied in a Google Chrome sandbox process,
        // in this case we reissue the request against the passed handle
        //

        if (Dll_RestrictedToken && status == STATUS_ACCESS_DENIED) {

            MasterKeyHandle = OriginalMasterKeyHandle;

            status = STATUS_SUCCESS;
            goto CallService;
        }

        if (! NT_SUCCESS(status))
            __leave;
    }

    //
    // check if the SlaveObjects is sandboxed
    //

    if (Count && SlaveObjects) {

        status = Key_GetName(
            SlaveObjects->RootDirectory, SlaveObjects->ObjectName,
            &TruePath, &CopyPath, &IsBoxedPath);

        if (NT_SUCCESS(status) && (! IsBoxedPath)) {

            //
            // make sure the key exists in the copy system
            //

            HANDLE handle;

            RtlInitUnicodeString(&objname, CopyPath);

            status = NtOpenKey(&handle, KEY_ALL_ACCESS, &objattrs);

            if (status == STATUS_ACCESS_DENIED) {

                //
                // if this is a system key that also happens to be an
                // OpenKeyPath, then opening with KEY_ALL_ACCESS fails
                //

                status = NtOpenKey(&handle, KEY_READ, &objattrs);
            }

            //
            // we can get access denied in a Google Chrome sandbox process,
            // in this case we reissue the request against the passed handle
            //

            if (Dll_RestrictedToken && status == STATUS_ACCESS_DENIED) {

                MasterKeyHandle = OriginalMasterKeyHandle;

                status = STATUS_SUCCESS;
                goto CallService;
            }

            if (! NT_SUCCESS(status))
                __leave;

            NtClose(handle);

            //
            // use our objattrs as the new SlaveObjects.  the objattrs
            // is already ready with a zero root directory and a
            // CopyPath object name
            //

            SlaveObjects = &objattrs;
        }
    }

    //
    // finish.  pop the name buffer only after we execute the call,
    // since the SlaveObjects pointer may use it
    //

    } __except (EXCEPTION_EXECUTE_HANDLER) {
        status = GetExceptionCode();
    }

CallService:

    if (NT_SUCCESS(status)) {

        status = __sys_NtNotifyChangeMultipleKeys(
            MasterKeyHandle, Count, SlaveObjects,
            Event, ApcRoutine, ApcContext, IoStatusBlock,
            CompletionFilter, WatchTree, Buffer, BufferSize, Asynchronous);

        //
        // we can get STATUS_INVALID_PARAMETER when there is a SlaveObject,
        // because NtNotifyChangeMultipleKeys does not support waiting on
        // a slave key that is in the same hive as the master key.  for
        // unsandboxed processes this may be used to wait on HKLM\CLSID and
        // HKCU\CLSID at the same time, but inside the sandbox, they both
        // exist in the same hive, so we just discard SlaveObject and retry
        //

        if (status == STATUS_INVALID_PARAMETER && Count == 1) {

            status = __sys_NtNotifyChangeMultipleKeys(
                MasterKeyHandle, 0, NULL,
                Event, ApcRoutine, ApcContext, IoStatusBlock,
                CompletionFilter, WatchTree, Buffer, BufferSize, Asynchronous);
        }
    }

    Dll_PopTlsNameBuffer(TlsData);
    SetLastError(LastError);
    return status;
}


//---------------------------------------------------------------------------
// Key_GetTrueHandle
//---------------------------------------------------------------------------


_FX HANDLE Key_GetTrueHandle(HANDLE KeyHandle, BOOLEAN *pIsOpenPath)
{
    ULONG LastError;
    THREAD_DATA *TlsData = Dll_GetTlsData(&LastError);

    NTSTATUS status;
    OBJECT_ATTRIBUTES objattrs;
    UNICODE_STRING objname;
    WCHAR *TruePath;
    WCHAR *CopyPath;
    HANDLE handle = NULL;

    Dll_PushTlsNameBuffer(TlsData);

    //
    // get key path for the handle
    //

    if (pIsOpenPath)
        *pIsOpenPath = FALSE;

    RtlInitUnicodeString(&objname, L"");
    InitializeObjectAttributes(
        &objattrs, &objname, OBJ_CASE_INSENSITIVE, NULL, NULL);

    status = Key_GetName(KeyHandle, &objname, &TruePath, &CopyPath, NULL);
    if (NT_SUCCESS(status)) {

        //
        // check if this is an open or closed path
        //

        ULONG mp_flags = SbieDll_MatchPath(L'k', TruePath);

        if (PATH_IS_OPEN(mp_flags) && pIsOpenPath)
            *pIsOpenPath = TRUE;

        if (! mp_flags) {

            //
            // open key
            //

            RtlInitUnicodeString(&objname, TruePath);

            status = __sys_NtOpenKey(
                &handle, Key_GetWow64Flag(TruePath, KEY_READ), &objattrs);

            if (! NT_SUCCESS(status))
                handle = NULL;
        }
    }

    Dll_PopTlsNameBuffer(TlsData);
    SetLastError(LastError);
    return handle;
}


//---------------------------------------------------------------------------
// Key_NtRenameKey
//---------------------------------------------------------------------------


_FX NTSTATUS Key_NtRenameKey(
    HANDLE KeyHandle, UNICODE_STRING *ReplacementName)
{
    THREAD_DATA *TlsData = Dll_GetTlsData(NULL);

    if (!Key_Delete_v2) {
        SbieApi_Log(2205, L"NtRenameKey");
        return __sys_NtRenameKey(KeyHandle, ReplacementName);
    }

    NTSTATUS status;
    OBJECT_ATTRIBUTES objattrs;
    UNICODE_STRING objname;
    HANDLE handle;
    WCHAR* TruePath;
    WCHAR* CopyPath;
    WCHAR* NewTruePath;

    Dll_PushTlsNameBuffer(TlsData);

    //
    // get the full new name of the key to be renamed
    //

    __try {

        status = Key_GetName(KeyHandle, NULL, &TruePath, &CopyPath, NULL);

        WCHAR* TruePathSlash = wcsrchr(TruePath, L'\\');
        if (!TruePathSlash){
            status = STATUS_INVALID_PARAMETER;
            __leave;
        }
    
        ULONG len = (ULONG)(TruePathSlash - TruePath + 1);

        NewTruePath = Dll_GetTlsNameBuffer(TlsData, MISC_NAME_BUFFER, 
            len * sizeof(WCHAR) + ReplacementName->Length + sizeof(WCHAR));

        wmemcpy(NewTruePath, TruePath, len);
        wmemcpy(NewTruePath + len, ReplacementName->Buffer, ReplacementName->Length / sizeof(WCHAR));
        NewTruePath[len + ReplacementName->Length / sizeof(WCHAR)] = L'\0';

    } __except (EXCEPTION_EXECUTE_HANDLER) {
        status = GetExceptionCode();
    }

    if (!NT_SUCCESS(status))
        goto finish;

    //
    // check if the target key already exists in the true path
    //

    WCHAR* NewTruePath2 = NewTruePath;
    WCHAR* OldTruePath = Key_GetRelocation(NewTruePath);
    if (OldTruePath)
        NewTruePath2 = OldTruePath;

    RtlInitUnicodeString(&objname, NewTruePath2);
    InitializeObjectAttributes(
        &objattrs, &objname, OBJ_CASE_INSENSITIVE, NULL, NULL);

    status = __sys_NtOpenKey(&handle, KEY_READ, &objattrs);

    if (NT_SUCCESS(status)) {

        if(Key_IsDeleted_v2(NewTruePath))
            status = STATUS_OBJECT_NAME_NOT_FOUND;

        File_NtCloseImpl(handle);
    }

    if (status != STATUS_OBJECT_NAME_NOT_FOUND)
        goto finish;

    //
    // rename the key ensuring we will have a boxed copy
    // try renaming if it fails with access denided try again with a new handle
    //

    status = __sys_NtRenameKey(KeyHandle, ReplacementName);

    if (status == STATUS_ACCESS_DENIED) {

        //
        // if we get STATUS_ACCESS_DENIED, the caller may be using a
        // TruePath handle that was opened with MAXIMUM_ALLOWED, but
        // reduced to read-only access in our NtCreateKey
        //

        OBJECT_ATTRIBUTES objattrs;
        UNICODE_STRING objname;
        HANDLE handle;

        RtlInitUnicodeString(&objname, L"");
        InitializeObjectAttributes(
            &objattrs, &objname, OBJ_CASE_INSENSITIVE, KeyHandle, NULL);

        status = NtOpenKey(&handle, KEY_WRITE, &objattrs);

        if (NT_SUCCESS(status)) {

            status = __sys_NtRenameKey(handle, ReplacementName);

            NtClose(handle);
        }
    }

    //
    // check if the true path exists and if so mark path deleted
    //

    BOOLEAN TrueExists = FALSE;

    WCHAR* TruePath2 = TruePath;

    OldTruePath = Key_GetRelocation(TruePath);
    if (OldTruePath)
        TruePath2 = OldTruePath;

    RtlInitUnicodeString(&objname, TruePath2);
    InitializeObjectAttributes(
        &objattrs, &objname, OBJ_CASE_INSENSITIVE, NULL, NULL);

    status = __sys_NtOpenKey(&handle, KEY_READ, &objattrs);

    if (NT_SUCCESS(status)) {

        //
        // if the true key exists mark it deleted
        //

        TrueExists = TRUE;

        File_NtCloseImpl(handle);
    }

    //
    // set the redirection information
    //

    if (TrueExists) {

        Key_SetRelocation(TruePath, NewTruePath);
    }

    //*TruePathSlash = L'\0';
    //Key_DiscardMergeByPath(TruePath, TRUE); // fix-me: act on Key_MergeCacheList
    //*TruePathSlash = L'\\';

    status = STATUS_SUCCESS;

finish:

    Dll_PopTlsNameBuffer(TlsData);

    return status;
}


//---------------------------------------------------------------------------
// Key_NtSaveKey
//---------------------------------------------------------------------------


_FX NTSTATUS Key_NtSaveKey(
    HANDLE KeyHandle, HANDLE FileHandle)
{
    // todo: copy all reg keys from host to box for the used KeyHandle such that all will be saved
    SbieApi_Log(2205, L"NtSaveKey");
    return __sys_NtSaveKey(KeyHandle, FileHandle);
}


//---------------------------------------------------------------------------
// Key_NtSaveKeyEx
//---------------------------------------------------------------------------


_FX NTSTATUS Key_NtSaveKeyEx(
    HANDLE KeyHandle, HANDLE FileHandle, ULONG Flags)
{
    // todo: copy all reg keys from host to box for the used KeyHandle such that all will be saved
    SbieApi_Log(2205, L"NtSaveKeyEx");
    return __sys_NtSaveKeyEx(KeyHandle, FileHandle, Flags);
}


//---------------------------------------------------------------------------
// Key_NtLoadKeyImpl
//---------------------------------------------------------------------------

_FX WCHAR* Key_NtLoadKey_GetPath(OBJECT_ATTRIBUTES* SourceObjectAttributes)
{
    NTSTATUS status;
    WCHAR *WorkPath;
    HANDLE FileHandle;

    WorkPath = Dll_Alloc(sizeof(WCHAR) * 8192);
    FileHandle = NULL;

    __try {

        IO_STATUS_BLOCK IoStatusBlock;

        //
        // open the file for write to migrate it into the box
        //

        status = NtCreateFile(
            &FileHandle, FILE_GENERIC_WRITE | SYNCHRONIZE,
            SourceObjectAttributes,
            &IoStatusBlock, NULL, 0, FILE_SHARE_VALID_FLAGS,
            FILE_OPEN,
            FILE_SYNCHRONOUS_IO_NONALERT | FILE_NON_DIRECTORY_FILE,
            NULL, 0);

        if (! NT_SUCCESS(status))
            __leave;

        BOOLEAN IsBoxedPath;
        status = SbieDll_GetHandlePath(FileHandle, WorkPath, &IsBoxedPath);

    } __except (EXCEPTION_EXECUTE_HANDLER) {
        status = GetExceptionCode();
    }

    if (FileHandle)
        NtClose(FileHandle);

    if (!NT_SUCCESS(status)) {
        Dll_Free(WorkPath);
        WorkPath = NULL;
    }

    return WorkPath;
}


//---------------------------------------------------------------------------
// Key_NtLoadKeyImpl
//---------------------------------------------------------------------------


_FX NTSTATUS Key_NtLoadKeyImpl(
    OBJECT_ATTRIBUTES *TargetObjectAttributes,
    WCHAR *WorkPath)
{
    ULONG LastError;
    THREAD_DATA *TlsData = Dll_GetTlsData(&LastError);

    NTSTATUS status;
    WCHAR *TruePath;
    WCHAR *CopyPath;
    FILE_LOAD_KEY_REQ *req;

    //
    // get the full paths for the registry key and hive file
    //

    req = Dll_Alloc(sizeof(FILE_LOAD_KEY_REQ));
    req->h.length = sizeof(FILE_LOAD_KEY_REQ);
    req->h.msgid = MSGID_FILE_LOAD_KEY;

    Dll_PushTlsNameBuffer(TlsData);

    __try {

        if (! SbieDll_TranslateNtToDosPath(WorkPath)) {
            status = STATUS_ACCESS_DENIED;
            __leave;
        }

        if (wcslen(WorkPath) > 127) { // todo // fix-me: make req->FilePath much longer
            status = STATUS_ACCESS_DENIED;
            __leave;
        }

        wcscpy(req->FilePath, WorkPath);

        //
        // query full key path
        //

        status = Key_GetName(TargetObjectAttributes->RootDirectory,
                             TargetObjectAttributes->ObjectName,
                             &TruePath, &CopyPath, NULL);

        if (! NT_SUCCESS(status))
            __leave;

        if (wcslen(TruePath) > 127) {
            status = STATUS_ACCESS_DENIED;
            __leave;
        }

        wcscpy(req->KeyPath, TruePath);

    } __except (EXCEPTION_EXECUTE_HANDLER) {
        status = GetExceptionCode();
    }

    Dll_PopTlsNameBuffer(TlsData);

    //
    // invoke SbieSvc and finish
    //

    if (NT_SUCCESS(status)) {

        MSG_HEADER *rpl = SbieDll_CallServer(&req->h);
        if (rpl) {
            status = rpl->status;
            Dll_Free(rpl);
        } else
            status = STATUS_ACCESS_DENIED;
    }

    Dll_Free(req);

    SetLastError(LastError);
    return status;
}


//---------------------------------------------------------------------------
// Key_NtLoadKey
//---------------------------------------------------------------------------


_FX NTSTATUS Key_NtLoadKey(
    OBJECT_ATTRIBUTES *TargetObjectAttributes,
    OBJECT_ATTRIBUTES *SourceObjectAttributes)
{
    NTSTATUS status;
    WCHAR* WorkPath;
    UNICODE_STRING objname;
    OBJECT_ATTRIBUTES objattrs;

    //
    // get the true file path
    //

    WorkPath = Key_NtLoadKey_GetPath(SourceObjectAttributes);
    if (WorkPath) {
        RtlInitUnicodeString(&objname, WorkPath);
        InitializeObjectAttributes(
            &objattrs, &objname, OBJ_CASE_INSENSITIVE, NULL, NULL);
        SourceObjectAttributes = &objattrs;
    }

    status = __sys_NtLoadKey(TargetObjectAttributes, SourceObjectAttributes);
    if (status == STATUS_PRIVILEGE_NOT_HELD && !Dll_CompartmentMode)
        status = Key_NtLoadKeyImpl(TargetObjectAttributes, WorkPath);

    if(WorkPath) Dll_Free(WorkPath);
    return status;
}


//---------------------------------------------------------------------------
// Key_NtLoadKey2
//---------------------------------------------------------------------------


_FX NTSTATUS Key_NtLoadKey2(
    OBJECT_ATTRIBUTES *TargetObjectAttributes,
    OBJECT_ATTRIBUTES *SourceObjectAttributes, ULONG Flags)
{
    NTSTATUS status;
    WCHAR* WorkPath;
    UNICODE_STRING objname;
    OBJECT_ATTRIBUTES objattrs;

    //
    // get the true file path
    //

    WorkPath = Key_NtLoadKey_GetPath(SourceObjectAttributes);
    if (WorkPath) {
        RtlInitUnicodeString(&objname, WorkPath);
        InitializeObjectAttributes(
            &objattrs, &objname, OBJ_CASE_INSENSITIVE, NULL, NULL);
        SourceObjectAttributes = &objattrs;
    }

    status = __sys_NtLoadKey2(TargetObjectAttributes, SourceObjectAttributes, Flags);
    if (status == STATUS_PRIVILEGE_NOT_HELD && !Dll_CompartmentMode)
        status = Key_NtLoadKeyImpl(TargetObjectAttributes, WorkPath);

    if(WorkPath) Dll_Free(WorkPath);
    return status;
}


//---------------------------------------------------------------------------
// Key_NtLoadKey3
//---------------------------------------------------------------------------


_FX NTSTATUS Key_NtLoadKey3(
    OBJECT_ATTRIBUTES *TargetObjectAttributes,
    OBJECT_ATTRIBUTES *SourceObjectAttributes, ULONG Flags,
    PVOID LoadArguments, ULONG LoadArgumentCount, ACCESS_MASK DesiredAccess, HANDLE KeyHandle, ULONG Unknown)
{
    NTSTATUS status;
    WCHAR* WorkPath;
    UNICODE_STRING objname;
    OBJECT_ATTRIBUTES objattrs;

    //
    // get the true file path
    //

    WorkPath = Key_NtLoadKey_GetPath(SourceObjectAttributes);
    if (WorkPath) {
        RtlInitUnicodeString(&objname, WorkPath);
        InitializeObjectAttributes(
            &objattrs, &objname, OBJ_CASE_INSENSITIVE, NULL, NULL);
        SourceObjectAttributes = &objattrs;
    }

    status = __sys_NtLoadKey3(TargetObjectAttributes, SourceObjectAttributes, Flags,
        LoadArguments, LoadArgumentCount, DesiredAccess, KeyHandle, Unknown);
    if (status == STATUS_PRIVILEGE_NOT_HELD && !Dll_CompartmentMode)
        SbieApi_Log(2205, L"NtLoadKey3");
        //status = Key_NtLoadKeyImpl(TargetObjectAttributes, WorkPath);

    if(WorkPath) Dll_Free(WorkPath);
    return status;
}


//---------------------------------------------------------------------------
// Key_NtLoadKeyEx
//---------------------------------------------------------------------------


_FX NTSTATUS Key_NtLoadKeyEx(
    OBJECT_ATTRIBUTES *TargetObjectAttributes,
    OBJECT_ATTRIBUTES *SourceObjectAttributes, ULONG Flags,
    HANDLE TrustClassKey, PVOID Reserved, PVOID ObjectContext, PVOID CallbackReserved, PIO_STATUS_BLOCK IoStatusBlock)
{
    NTSTATUS status;
    WCHAR* WorkPath;
    UNICODE_STRING objname;
    OBJECT_ATTRIBUTES objattrs;

    //
    // get the true file path
    //

    WorkPath = Key_NtLoadKey_GetPath(SourceObjectAttributes);
    if (WorkPath) {
        RtlInitUnicodeString(&objname, WorkPath);
        InitializeObjectAttributes(
            &objattrs, &objname, OBJ_CASE_INSENSITIVE, NULL, NULL);
        SourceObjectAttributes = &objattrs;
    }

    status = __sys_NtLoadKeyEx(TargetObjectAttributes, SourceObjectAttributes, Flags,
        TrustClassKey, Reserved, ObjectContext, CallbackReserved, IoStatusBlock);
    if (status == STATUS_PRIVILEGE_NOT_HELD && !Dll_CompartmentMode)
        SbieApi_Log(2205, L"NtLoadKey3");
        //status = Key_NtLoadKeyImpl(TargetObjectAttributes, WorkPath);

    if(WorkPath) Dll_Free(WorkPath);
    return status;
}


//---------------------------------------------------------------------------
// Key_NtQueryObjectName
//---------------------------------------------------------------------------


_FX ULONG Key_NtQueryObjectName(UNICODE_STRING *ObjectName, ULONG MaxLen)
{
    static const WCHAR *_Machine = L"\\MACHINE";
    static const WCHAR *_UserCurrent = L"\\USER\\CURRENT";

    //
    // adjust the path returned by NtQueryObject(ObjectNameInformation)
    // to not include a sandbox prefix
    //
    // this is important for two primary reasons:
    //
    // 1.  RegOpenKeyEx in Windows Vista does not forward the WOW64 access
    // flags to NtOpenKey in a 64-bit process.  an internal function
    // Wow64RegOpenKeyEx uses NtQueryObject to get the path to the registry
    // key, and we must make sure it sees the TruePath rather than CopyPath
    //
    // 2.  the Google Chrome broker uses NtQueryObject to make sure the path
    // to an opened handle is the same as the requested path
    //

    ULONG Len = ObjectName->Length;
    WCHAR *Buf = ObjectName->Buffer;

    if (Len >= (Dll_BoxKeyPathLen + 8) * sizeof(WCHAR) &&
            0 == Dll_NlsStrCmp(Buf, Dll_BoxKeyPath, Dll_BoxKeyPathLen)) {

        //
        // check \REGISTRY\MACHINE prefix
        //

        if (_wcsnicmp(Buf + Dll_BoxKeyPathLen, _Machine, 8) == 0) {

            ULONG MoveLen = Len / sizeof(WCHAR) - Dll_BoxKeyPathLen;
            wmemmove(Buf + 9, Buf + Dll_BoxKeyPathLen, MoveLen);
            Buf[9 + MoveLen] = L'\0';

            ObjectName->Length =
                (USHORT)((9 /* \REGISTRY */ + MoveLen) * sizeof(WCHAR));
            ObjectName->MaximumLength = ObjectName->Length + sizeof(WCHAR);

            return ObjectName->MaximumLength;

        //
        // check \REGISTRY\USER\CURRENT prefix
        //

        } else if (_wcsnicmp(
                        Buf + Dll_BoxKeyPathLen, _UserCurrent, 13) == 0) {

            WCHAR *TmpBuf;
            ULONG RemLen = wcslen(Buf + Dll_BoxKeyPathLen + 13);
            ULONG NeedLen = 15 /* \registry\user\ prefix */
                          + Dll_SidStringLen + 1 + RemLen + 1;
            NeedLen *= sizeof(WCHAR);
            if (MaxLen < sizeof(UNICODE_STRING) + NeedLen)
                return NeedLen;

            TmpBuf = Dll_AllocTemp((RemLen + 1) * sizeof(WCHAR));
            wmemcpy(TmpBuf, Buf + Dll_BoxKeyPathLen + 13, RemLen);
            TmpBuf[RemLen] = L'\0';

            wmemcpy(Buf + 9,            Key_UserCurrent, 6);
            wmemcpy(Buf + 9 + 6,        Dll_SidString, Dll_SidStringLen);
            wmemcpy(Buf + 9 + 6 + Dll_SidStringLen, TmpBuf, RemLen + 1);

            ObjectName->Length = (USHORT)(wcslen(Buf) * sizeof(WCHAR));
            ObjectName->MaximumLength = ObjectName->Length + sizeof(WCHAR);

            Dll_Free(TmpBuf);

            return ObjectName->MaximumLength;
        }
    }

    return 0;
}


//---------------------------------------------------------------------------
// Key_CreateBaseKeys
//---------------------------------------------------------------------------


_FX void Key_CreateBaseKeys()
{
    WCHAR buff[2048];
    OBJECT_ATTRIBUTES objattrs;
    UNICODE_STRING objname;
    WCHAR* base_keys[] = {
        L"\\machine\\system", L"\\machine\\software",
        L"\\user\\current\\software", L"\\user\\current_Classes",
        L"\\machine\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer",
        L"\\user\\current\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer",
        NULL};

    //
    // in privacy mode we need to pre create some keys or else the box initialization will fail
    //
 
    InitializeObjectAttributes(
            &objattrs, &objname, OBJ_CASE_INSENSITIVE, NULL, NULL);

    for (WCHAR** base_key = base_keys; *base_key; base_key++) {

        wcscpy(buff, Dll_BoxKeyPath);
        wcscat(buff, *base_key);

        RtlInitUnicodeString(&objname, buff);

        Key_CreatePath(&objattrs, NULL);
    }
}
