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
// IPC
//---------------------------------------------------------------------------


#include "dll.h"
#include "obj.h"
#include "handle.h"
#include <stdio.h>
#include "common/my_version.h"
#include "core/svc/namedpipewire.h"


//---------------------------------------------------------------------------
// Defines
//---------------------------------------------------------------------------


#define OBJECT_ATTRIBUTES_ATTRIBUTES                            \
    (ObjectAttributes                                           \
        ? ObjectAttributes->Attributes | OBJ_CASE_INSENSITIVE   \
        : 0)


//---------------------------------------------------------------------------
// Structures and Types
//---------------------------------------------------------------------------


typedef struct _IPC_MERGE {

    LIST_ELEM list_elem;

    HANDLE handle;

    LIST objects;

} IPC_MERGE;


typedef struct _IPC_MERGE_ENTRY
{
    LIST_ELEM list_elem;

    UNICODE_STRING Name;
    UNICODE_STRING TypeName;

} IPC_MERGE_ENTRY;


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------


static void Ipc_CreateObjects(void);

static BOOLEAN Ipc_GetName_AdjustSplWow64Path(WCHAR *TruePath, BOOLEAN adj);

static NTSTATUS Ipc_GetName(
    HANDLE RootDirectory, UNICODE_STRING *ObjectName,
    WCHAR **OutTruePath, WCHAR **OutCopyPath, BOOLEAN *OutIsBoxedPath);

static NTSTATUS Ipc_GetName2(
    OBJECT_ATTRIBUTES *ObjectAttributes,
    WCHAR **OutTruePath, WCHAR **OutCopyPath, const WCHAR* NtName);

static NTSTATUS Ipc_CreatePath(WCHAR *TruePath, WCHAR *CopyPath);


//---------------------------------------------------------------------------


static void Ipc_AdjustPortPath(UNICODE_STRING *ObjectName);

static NTSTATUS Ipc_NtCreatePort(
    HANDLE *PortHandle,
    OBJECT_ATTRIBUTES *ObjectAttributes,
    ULONG MaxConnectInfoLength,
    ULONG MaxDataLength,
    ULONG *Reserved);

static NTSTATUS Ipc_NtConnectPort(
    HANDLE *PortHandle,
    UNICODE_STRING *PortName,
    SECURITY_QUALITY_OF_SERVICE *SecurityQos,
    PLPC_SECTION_OWNER_MEMORY ClientSharedMemory,
    PLPC_SECTION_MEMORY ServerSharedMemory,
    ULONG *MaximumMessageLength,
    void *ConnectionInfo,
    ULONG *ConnectionInfoLength);

static NTSTATUS Ipc_NtSecureConnectPort(
    HANDLE *PortHandle,
    UNICODE_STRING *PortName,
    SECURITY_QUALITY_OF_SERVICE *SecurityQos,
    PLPC_SECTION_OWNER_MEMORY ClientSharedMemory,
    PSID ServerSid,
    PLPC_SECTION_MEMORY ServerSharedMemory,
    ULONG *MaximumMessageLength,
    void *ConnectionInfo,
    ULONG *ConnectionInfoLength);

static NTSTATUS Ipc_NtAlpcCreatePort(
    HANDLE *PortHandle,
    OBJECT_ATTRIBUTES *ObjectAttributes,
    void *AlpcCreateInfo);

static NTSTATUS Ipc_NtAlpcConnectPort(
    HANDLE *PortHandle,
    UNICODE_STRING *PortName,
    OBJECT_ATTRIBUTES *ObjectAttributes,
    ALPC_PORT_ATTRIBUTES *AlpcConnectInfo,
    ULONG ConnectionFlags,
    PSID ServerSid,
    void *ConnectionInfo,
    ULONG *ConnectionInfoLength,
    void *InMessageBuffer,
    void *OutMessageBuffer,
    LARGE_INTEGER *Timeout);

static NTSTATUS Ipc_NtAlpcConnectPortEx(
    HANDLE *PortHandle,
    OBJECT_ATTRIBUTES *ObjectAttributes1,
    OBJECT_ATTRIBUTES *ObjectAttributes2,
    ALPC_PORT_ATTRIBUTES *AlpcConnectInfo,
    ULONG ConnectionFlags,
    SECURITY_DESCRIPTOR *ServerSd,
    void *ConnectionInfo,
    ULONG *ConnectionInfoLength,
    void *InMessageBuffer,
    void *OutMessageBuffer,
    LARGE_INTEGER *Timeout);

static NTSTATUS Ipc_NtAlpcQueryInformation(
    HANDLE *PortHandle,
    ULONG InformationClass,
    void *Information,
    ULONG InformationLength,
    ULONG_PTR UnknownParameter5);

static NTSTATUS Ipc_NtAlpcQueryInformationMessage(
    HANDLE *PortHandle,
    void *RpcMessage,
    ULONG InformationClass,
    void *Information,
    ULONG InformationLength,
    ULONG_PTR UnknownParameter6);

BOOLEAN Ipc_IsAdminToken(HANDLE hToken);

static NTSTATUS Ipc_ImpersonateSelf(PORT_MESSAGE *PortMsg);

static NTSTATUS Ipc_NtImpersonateClientOfPort(
    HANDLE PortHandle, PORT_MESSAGE *RequestMessage);

static NTSTATUS Ipc_NtAlpcImpersonateClientOfPort(
    HANDLE PortHandle, PORT_MESSAGE *RequestMessage,
    ULONG_PTR UnknownParameter3);

static NTSTATUS Ipc_NtImpersonateAnonymousToken(HANDLE ThreadHandle);

static NTSTATUS Ipc_NtImpersonateThread(
    HANDLE ServerThreadHandle, HANDLE ClientThreadHandle,
    PSECURITY_QUALITY_OF_SERVICE SecurityQos);

static BOOLEAN Ipc_IsKnownDllInSandbox(
    THREAD_DATA *TlsData, const WCHAR *TruePath);

static NTSTATUS Ipc_ConnectProxyPort(
    HANDLE *PortHandle,
    const WCHAR *TruePath,
    ULONG AlpcConnectionFlags,
    void *ConnectionInfo,
    ULONG *ConnectionInfoLength,
    void *MaximumMessageLengthOrAlpcInfo,
    void *ShouldBeNull1, void *ShouldBeNull2, void *ShouldBeNull3);

static NTSTATUS Ipc_NtRequestWaitReplyPort(
    HANDLE PortHandle, PORT_MESSAGE *RequestMsg, PORT_MESSAGE *ReplyMsg);

static NTSTATUS Ipc_NtAlpcSendWaitReceivePort(
    HANDLE PortHandle, ULONG SendFlags,
    PORT_MESSAGE *SendMsg, ALPC_MESSAGE_VIEW *SendView,
    PORT_MESSAGE *ReceiveMsg, ULONG *ReceiveMsgSize,
    ALPC_MESSAGE_VIEW *ReceiveView, LARGE_INTEGER *Timeout);


//---------------------------------------------------------------------------


static NTSTATUS Ipc_NtCreateEvent(
    HANDLE *EventHandle,
    ACCESS_MASK DesiredAccess,
    OBJECT_ATTRIBUTES *ObjectAttributes,
    EVENT_TYPE EventType,
    BOOLEAN InitialState);

static NTSTATUS Ipc_NtOpenEvent(
    HANDLE *EventHandle,
    ACCESS_MASK DesiredAccess,
    OBJECT_ATTRIBUTES *ObjectAttributes);


//---------------------------------------------------------------------------


static NTSTATUS Ipc_NtCreateMutant(
    HANDLE *MutantHandle,
    ACCESS_MASK DesiredAccess,
    OBJECT_ATTRIBUTES *ObjectAttributes,
    BOOLEAN InitialOwner);

static NTSTATUS Ipc_NtOpenMutant(
    HANDLE *MutantHandle,
    ACCESS_MASK DesiredAccess,
    OBJECT_ATTRIBUTES *ObjectAttributes);


//---------------------------------------------------------------------------


static NTSTATUS Ipc_NtCreateSemaphore(
    HANDLE *SemaphoreHandle,
    ACCESS_MASK DesiredAccess,
    OBJECT_ATTRIBUTES *ObjectAttributes,
    ULONG InitialCount,
    ULONG MaximumCount);

static NTSTATUS Ipc_NtOpenSemaphore(
    HANDLE *SemaphoreHandle,
    ACCESS_MASK DesiredAccess,
    OBJECT_ATTRIBUTES *ObjectAttributes);


//---------------------------------------------------------------------------


static NTSTATUS Ipc_NtCreateSection(
    HANDLE *SectionHandle,
    ACCESS_MASK DesiredAccess,
    OBJECT_ATTRIBUTES *ObjectAttributes,
    LARGE_INTEGER *MaximumSize,
    ULONG PageAttributes,
    ULONG SectionAttributes,
    HANDLE FileHandle);

static NTSTATUS Ipc_NtCreateSectionEx(
    HANDLE *SectionHandle,
    ACCESS_MASK DesiredAccess,
    OBJECT_ATTRIBUTES *ObjectAttributes,
    LARGE_INTEGER *MaximumSize,
    ULONG PageAttributes,
    ULONG SectionAttributes,
    HANDLE FileHandle,
    PMEM_EXTENDED_PARAMETER ExtendedParameters,
    ULONG ExtendedParameterCount);

static NTSTATUS Ipc_NtOpenSection(
    HANDLE *SectionHandle,
    ACCESS_MASK DesiredAccess,
    OBJECT_ATTRIBUTES *ObjectAttributes);


//---------------------------------------------------------------------------

static NTSTATUS Ipc_NtCreateSymbolicLinkObject (
    PHANDLE SymbolicLinkHandle,
    ACCESS_MASK DesiredAccess,
    POBJECT_ATTRIBUTES ObjectAttributes,
    PUNICODE_STRING DestinationName);

static NTSTATUS Ipc_NtOpenSymbolicLinkObject(
    PHANDLE SymbolicLinkHandle,
    ACCESS_MASK DesiredAccess,
    POBJECT_ATTRIBUTES ObjectAttributes);

//---------------------------------------------------------------------------

static NTSTATUS Ipc_NtCreateDirectoryObject(
    PHANDLE DirectoryHandle,
    ACCESS_MASK DesiredAccess,
    POBJECT_ATTRIBUTES ObjectAttributes);

static NTSTATUS Ipc_NtCreateDirectoryObjectEx(
    PHANDLE DirectoryHandle,
    ACCESS_MASK DesiredAccess,
    POBJECT_ATTRIBUTES ObjectAttributes,
    HANDLE ShadowDirectoryHandle,
    ULONG Flags);

static NTSTATUS Ipc_NtOpenDirectoryObject(
    PHANDLE DirectoryHandle,
    ACCESS_MASK DesiredAccess,
    POBJECT_ATTRIBUTES ObjectAttributes);

static NTSTATUS Ipc_NtQueryDirectoryObject(
    HANDLE DirectoryHandle,
    PVOID Buffer,
    ULONG Length,
    BOOLEAN ReturnSingleEntry,
    BOOLEAN RestartScan,
    PULONG Context,
    PULONG ReturnLength);

//---------------------------------------------------------------------------

static P_NtCreatePort               __sys_NtCreatePort              = NULL;
static P_NtConnectPort              __sys_NtConnectPort             = NULL;
static P_NtSecureConnectPort        __sys_NtSecureConnectPort       = NULL;
static P_NtAlpcCreatePort           __sys_NtAlpcCreatePort          = NULL;
static P_NtAlpcConnectPort          __sys_NtAlpcConnectPort         = NULL;
static P_NtAlpcConnectPortEx        __sys_NtAlpcConnectPortEx       = NULL;
static P_NtAlpcQueryInformation     __sys_NtAlpcQueryInformation    = NULL;
static P_NtAlpcQueryInformationMessage
                                    __sys_NtAlpcQueryInformationMessage
                                                                    = NULL;
static P_NtImpersonateClientOfPort  __sys_NtImpersonateClientOfPort = NULL;
static P_NtAlpcImpersonateClientOfPort
                                    __sys_NtAlpcImpersonateClientOfPort
                                                                    = NULL;
static P_NtRequestWaitReplyPort     __sys_NtRequestWaitReplyPort    = NULL;
static P_NtAlpcSendWaitReceivePort  __sys_NtAlpcSendWaitReceivePort = NULL;
static P_NtCreateEvent              __sys_NtCreateEvent             = NULL;
static P_NtOpenEvent                __sys_NtOpenEvent               = NULL;
static P_NtCreateMutant             __sys_NtCreateMutant            = NULL;
static P_NtOpenMutant               __sys_NtOpenMutant              = NULL;
static P_NtCreateSemaphore          __sys_NtCreateSemaphore         = NULL;
static P_NtOpenSemaphore            __sys_NtOpenSemaphore           = NULL;
static P_NtCreateSection            __sys_NtCreateSection           = NULL;
static P_NtCreateSectionEx          __sys_NtCreateSectionEx         = NULL;
static P_NtOpenSection              __sys_NtOpenSection             = NULL;

static P_NtCreateSymbolicLinkObject __sys_NtCreateSymbolicLinkObject= NULL;
static P_NtOpenSymbolicLinkObject   __sys_NtOpenSymbolicLinkObject  = NULL;

static P_NtCreateDirectoryObject    __sys_NtCreateDirectoryObject   = NULL;
static P_NtCreateDirectoryObjectEx  __sys_NtCreateDirectoryObjectEx = NULL;
static P_NtOpenDirectoryObject      __sys_NtOpenDirectoryObject     = NULL;
static P_NtQueryDirectoryObject     __sys_NtQueryDirectoryObject    = NULL;

static P_NtImpersonateAnonymousToken
                                    __sys_NtImpersonateAnonymousToken
                                                                    = NULL;
static P_NtImpersonateThread        __sys_NtImpersonateThread       = NULL;


//---------------------------------------------------------------------------
// Variables
//---------------------------------------------------------------------------


static ULONG_PTR *Ipc_ProxyViews = NULL;

const WCHAR *Ipc_SandboxieRpcSs      = SANDBOXIE L"RpcSs.exe";

const WCHAR *Ipc_epmapper  = L"\\RPC Control\\epmapper";
const WCHAR *Ipc_actkernel = L"\\RPC Control\\actkernel";

BOOLEAN ipc_namespace_isoaltion;

extern const WCHAR *File_BQQB;

LIST Ipc_DynamicPortNames;

BOOLEAN RpcRt_IsDynamicPortOpen(const WCHAR* wszPortName);

static LIST Ipc_Handles;
static CRITICAL_SECTION Ipc_Handles_CritSec;

//---------------------------------------------------------------------------
// IPC (other modules)
//---------------------------------------------------------------------------


#include "ipc_start.c"


//---------------------------------------------------------------------------
// Ipc_Init
//---------------------------------------------------------------------------


_FX BOOLEAN Ipc_Init(void)
{
    HMODULE module = Dll_Ntdll;

    InitializeCriticalSection(&Ipc_Handles_CritSec);

    List_Init(&Ipc_Handles);

    void *NtAlpcCreatePort;
    void *NtAlpcConnectPort;
    void *NtAlpcConnectPortEx;
    void *NtAlpcQueryInformation;
    void *NtAlpcQueryInformationMessage;
    void *NtAlpcImpersonateClientOfPort;
    void *NtAlpcSendWaitReceivePort;

    //
    // initialize cache of open and closed IPC paths
    //

    SbieDll_MatchPath(L'i', (const WCHAR *)-1);

    ipc_namespace_isoaltion = SbieApi_QueryConfBool(NULL, L"NtNamespaceIsolation", TRUE);

    //
    // intercept NTDLL entry points
    //

#define SBIEDLL_HOOK_IF(nm)                                     \
        nm = GetProcAddress(Dll_Ntdll, #nm);                    \
    if (nm) {                                                   \
        SBIEDLL_HOOK(Ipc_,nm);                                  \
    }

    SBIEDLL_HOOK(Ipc_,NtCreatePort);
    SBIEDLL_HOOK(Ipc_,NtConnectPort);
    SBIEDLL_HOOK(Ipc_,NtSecureConnectPort);

    SBIEDLL_HOOK_IF(NtAlpcCreatePort);
    SBIEDLL_HOOK_IF(NtAlpcConnectPort);
    SBIEDLL_HOOK_IF(NtAlpcConnectPortEx);
    SBIEDLL_HOOK_IF(NtAlpcQueryInformation);
    SBIEDLL_HOOK_IF(NtAlpcQueryInformationMessage);

    // OriginalToken BEGIN
    if (!Dll_CompartmentMode && !SbieApi_QueryConfBool(NULL, L"OriginalToken", FALSE))
    // OriginalToken END
    {
        SBIEDLL_HOOK(Ipc_, NtImpersonateClientOfPort);
        SBIEDLL_HOOK_IF(NtAlpcImpersonateClientOfPort);
    }

    SBIEDLL_HOOK(Ipc_,NtRequestWaitReplyPort);
    SBIEDLL_HOOK_IF(NtAlpcSendWaitReceivePort);

    if (NtAlpcSendWaitReceivePort) {
        Ipc_ProxyViews = Dll_Alloc(sizeof(ULONG_PTR) * 256);
        memzero(Ipc_ProxyViews, sizeof(ULONG_PTR) * 256);
    }

    SBIEDLL_HOOK(Ipc_,NtCreateEvent);
    SBIEDLL_HOOK(Ipc_,NtOpenEvent);

    SBIEDLL_HOOK(Ipc_,NtCreateMutant);
    SBIEDLL_HOOK(Ipc_,NtOpenMutant);

    SBIEDLL_HOOK(Ipc_,NtCreateSemaphore);
    SBIEDLL_HOOK(Ipc_,NtOpenSemaphore);

    SBIEDLL_HOOK(Ipc_,NtCreateSection);
    void* NtCreateSectionEx = GetProcAddress(Dll_Ntdll, "NtCreateSectionEx");
    if (NtCreateSectionEx) { // windows 10 1809
        SBIEDLL_HOOK(Ipc_, NtCreateSectionEx);
    }
    SBIEDLL_HOOK(Ipc_,NtOpenSection);

    SBIEDLL_HOOK(Ipc_,NtCreateSymbolicLinkObject);
    SBIEDLL_HOOK(Ipc_,NtOpenSymbolicLinkObject);

    SBIEDLL_HOOK(Ipc_,NtCreateDirectoryObject);
	void* NtCreateDirectoryObjectEx = GetProcAddress(Dll_Ntdll, "NtCreateDirectoryObjectEx");
    if (NtCreateDirectoryObjectEx) { // windows 8
	    SBIEDLL_HOOK(Ipc_,NtCreateDirectoryObjectEx);
	}
    SBIEDLL_HOOK(Ipc_,NtOpenDirectoryObject);
    SBIEDLL_HOOK(Ipc_,NtQueryDirectoryObject);

    // OriginalToken BEGIN
    if (!Dll_CompartmentMode && !SbieApi_QueryConfBool(NULL, L"OriginalToken", FALSE))
    // OriginalToken END
    {
        SBIEDLL_HOOK(Ipc_, NtImpersonateAnonymousToken);
        SBIEDLL_HOOK(Ipc_, NtImpersonateThread);
    }

    //if (!Dll_AlernateIpcNaming) // alternate naming does not need an own namespace
    if (Dll_FirstProcessInBox) {
        Ipc_CreateObjects();
    }

    List_Init(&Ipc_DynamicPortNames);

    return TRUE;
}


//---------------------------------------------------------------------------
// Ipc_CreateObjects
//---------------------------------------------------------------------------


_FX void Ipc_CreateObjects(void)
{
    NTSTATUS status;
    WCHAR *TruePath;
    WCHAR *CopyPath;
    WCHAR *backslash;
    WCHAR *buffer = NULL;
    WCHAR *BNOLINKS = NULL;
    WCHAR *GLOBAL = NULL;
    WCHAR *buffer2 = NULL;
    HANDLE handle;
    WCHAR str[64];
    ULONG errlvl = 0;

    //
    // find the path to the sandboxed BaseNamedObjects directory,
    // by creating a dummy object, getting its name, then removing
    // the last path component (the dummy name itself)
    //

    Sbie_snwprintf(str, 64, SBIE_BOXED_ L"DummyEvent_%d", Dll_ProcessId);
    handle = CreateEvent(NULL, FALSE, FALSE, str);
    if (! handle) {
        errlvl = 11;
        status = GetLastError();
        goto finish;
    }

    backslash = NULL;
    status = Ipc_GetName(handle, NULL, &TruePath, &CopyPath, NULL);
    if (NT_SUCCESS(status)) {
        if (CopyPath)
            backslash = wcsrchr(CopyPath, L'\\');
        if (backslash)
            *backslash = L'\0';
        else
            status = STATUS_UNSUCCESSFUL;
    }
    if (! NT_SUCCESS(status)) {
        errlvl = 22;
        goto finish;
    }

    NtClose(handle);

    //
    // create main directory, in case it isn't there yet
    //

    status = SbieApi_CreateDirOrLink(CopyPath, NULL);

    if (status == STATUS_OBJECT_PATH_NOT_FOUND) {

        status = Ipc_CreatePath(TruePath, CopyPath);

        if (NT_SUCCESS(status))
            status = SbieApi_CreateDirOrLink(CopyPath, NULL);
    }

    if (! NT_SUCCESS(status)) {
        errlvl = 33;
        goto finish;
    }

    // todo: fix-me: properly reparse symbolic links in IPC paths instead of creating dummy for everything

    buffer = Dll_Alloc((wcslen(CopyPath) + 32) * sizeof(WCHAR));

    //
    // create BNOLINKS directory and symbolic links
    //

    BNOLINKS  = Dll_Alloc((wcslen(CopyPath) + 32) * sizeof(WCHAR));

    wcscpy(BNOLINKS, CopyPath);

    status = STATUS_UNSUCCESSFUL;
    backslash = wcsrchr(BNOLINKS, L'\\');
    if (backslash) {
        *backslash = L'\0';
        backslash = wcsrchr(BNOLINKS, L'\\');
        if (backslash) {
            wcscpy(str, backslash);
            *backslash = L'\0';
            wcscat(BNOLINKS, L"\\BNOLINKS");
            status = STATUS_SUCCESS;
        }
    }

    if (NT_SUCCESS(status))
        status = SbieApi_CreateDirOrLink(BNOLINKS, NULL);
    
    if (NT_SUCCESS(status)) {
        wcscpy(buffer, BNOLINKS);
        wcscat(buffer, str);
        status = SbieApi_CreateDirOrLink(buffer, CopyPath);
    }

    if (! NT_SUCCESS(status)) {
        errlvl = 77;
        goto finish;
    }

    //
    // create Global directory and symbolic links
    //

    buffer2 = Dll_Alloc((Dll_BoxIpcPathLen + 32) * sizeof(WCHAR));

    wcscpy(buffer2, Dll_BoxIpcPath);
    wcscat(buffer2, L"\\BaseNamedObjects");

    status = SbieApi_CreateDirOrLink(buffer2, NULL);

    if (! NT_SUCCESS(status)) {
        errlvl = 88;
        goto finish;
    }

    wcscpy(buffer, buffer2);
    wcscat(buffer, L"\\Global");

    GLOBAL  = Dll_Alloc((wcslen(buffer) + 32) * sizeof(WCHAR));

    wcscpy(GLOBAL, buffer);

    status = SbieApi_CreateDirOrLink(buffer, buffer2);

    if (! NT_SUCCESS(status)) {
        errlvl = 51;
        goto finish;
    }

    wcscpy(buffer, buffer2);
    wcscat(buffer, L"\\Local");

    status = SbieApi_CreateDirOrLink(buffer, buffer2);

    if (! NT_SUCCESS(status)) {
        errlvl = 52;
        goto finish;
    }

    wcscpy(buffer, buffer2);
    wcscat(buffer, L"\\Session");

    status = SbieApi_CreateDirOrLink(buffer, BNOLINKS);

    if (! NT_SUCCESS(status)) {
        errlvl = 53;
        goto finish;
    }

    if (NT_SUCCESS(status)) {
        wcscpy(buffer, BNOLINKS);
        wcscat(buffer, L"\\0");
        status = SbieApi_CreateDirOrLink(buffer, buffer2);
    }

    if (! NT_SUCCESS(status)) {
        errlvl = 66;
        goto finish;
    }

    //
    // create Global,Local,Session symbolic links
    //

    wcscpy(buffer, CopyPath);
    wcscat(buffer, L"\\Global");

    status = SbieApi_CreateDirOrLink(buffer, GLOBAL);

    if (! NT_SUCCESS(status)) {
        errlvl = 41;
        goto finish;
    }

    wcscpy(buffer, CopyPath);
    wcscat(buffer, L"\\Local");

    status = SbieApi_CreateDirOrLink(buffer, CopyPath);

    if (! NT_SUCCESS(status)) {
        errlvl = 42;
        goto finish;
    }

    wcscpy(buffer, CopyPath);
    wcscat(buffer, L"\\Session");

    status = SbieApi_CreateDirOrLink(buffer, BNOLINKS);

    if (! NT_SUCCESS(status)) {
        errlvl = 43;
        goto finish;
    }

    //
    // finish
    //

finish:

    if(buffer)
        Dll_Free(buffer);
    if(BNOLINKS)
        Dll_Free(BNOLINKS);
    if(GLOBAL)
        Dll_Free(GLOBAL);
    if(buffer2)
        Dll_Free(buffer2);

    if (errlvl)
        SbieApi_Log(2308, L"[%d / %08X]", errlvl, status);
}


//---------------------------------------------------------------------------
// Ipc_GetName
//---------------------------------------------------------------------------


_FX NTSTATUS Ipc_GetName(
    HANDLE RootDirectory, UNICODE_STRING *ObjectName,
    WCHAR **OutTruePath, WCHAR **OutCopyPath, BOOLEAN *OutIsBoxedPath)
{
    THREAD_DATA *TlsData = Dll_GetTlsData(NULL);

    NTSTATUS status;
    ULONG length;
    WCHAR *name;
    ULONG objname_len;
    WCHAR *objname_buf;

    *OutTruePath = NULL;
    *OutCopyPath = NULL;
    if (OutIsBoxedPath)
        *OutIsBoxedPath = FALSE;

    if (ObjectName && ObjectName != (UNICODE_STRING*)-1) {
        objname_len = ObjectName->Length & ~1;
        objname_buf = ObjectName->Buffer;

        //if (Dll_AlernateIpcNaming) {
        //    
        //    //
        //    // Since in this mode we don't call Ipc_CreateObjects we don't have a boxed namespace
        //    // and are using existing namespaces only with a name suffix
        //    // hence we can't use Global without system privileges, so we strip it
        //    //
        //
        //    if (_wcsnicmp(objname_buf, L"Global\\", 7) == 0) {
        //        objname_len -= 7;
        //        objname_buf += 7;
        //    }
        //}

    } else {
        objname_len = 0;
        objname_buf = NULL;
    }

    //
    // if a root handle is specified, we query the full name of the
    // root key, and append the ObjectName
    //

    if (RootDirectory) {

        length = 256;
        name = Dll_GetTlsNameBuffer(
                        TlsData, TRUE_NAME_BUFFER, length + objname_len);

      if (((! objname_len) || (! *objname_buf)) && ObjectName != (UNICODE_STRING*)-1) {

            //
            // an object handle was specified, but the object name is an
            // empty string or NULL.  if the handle is for a directory
            // object, then we treat this as an unnamed object, otherwise
            // we go on as usual
            //

            if (Obj_GetObjectType(RootDirectory) == OBJ_TYPE_DIRECTORY) {

                *OutTruePath = NULL;
                *OutCopyPath = NULL;

                return STATUS_SUCCESS;
            }
        }

        if (objname_len && *objname_buf == L'\\') {

            //
            // if the caller specifies both a directory and an object
            // name that begins with a backslash, return special status
            //

            return STATUS_OBJECT_PATH_SYNTAX_BAD;
        }

        status = Obj_GetObjectName(RootDirectory, name, &length);

        if (status == STATUS_BUFFER_OVERFLOW || status == STATUS_BUFFER_TOO_SMALL || status == STATUS_INFO_LENGTH_MISMATCH) {

            name = Dll_GetTlsNameBuffer(
                        TlsData, TRUE_NAME_BUFFER, length + objname_len);

            status = Obj_GetObjectName(RootDirectory, name, &length);
        }

        if (! NT_SUCCESS(status))
            return status;

        *OutTruePath = ((OBJECT_NAME_INFORMATION *)name)->Name.Buffer;

        if (! *OutTruePath) {

            //
            // object attributes indicate an unnamed parent directory
            //

            *OutCopyPath = NULL;
            return status;
        }

        name = (*OutTruePath)
             + ((OBJECT_NAME_INFORMATION *)name)->Name.Length
                    / sizeof(WCHAR);

        if (objname_len) {

            *name = L'\\';
            ++name;
            memcpy(name, objname_buf, objname_len);

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

        memcpy(name, objname_buf, objname_len);

        name += objname_len / sizeof(WCHAR);
        *name = L'\0';

    //
    // if no root handle, and no object name, then must be unnamed object
    //

    } else {

        *OutTruePath = NULL;
        *OutCopyPath = NULL;

        return STATUS_SUCCESS;
    }

    //
    // for 32-bit Internet Explorer running on 64-bit Windows 7 (Wow64),
    // check for SplWow64 resources, and adjust the integrity level
    // which is the last digit of the resource name.
    //
    // - always 1 if we are running as a standard user (note that outside
    // the sandbox, IE low integrity processes use integrity level 0)
    // - always 2 if we are running as an elevated user
    //
    // this is necessary due Internet Explorer assuming it is going to
    // run with low integrity, while in fact it runs with medium integrity
    // inside the sandbox, and Secure_NtQueryInformationToken will fake
    // a high integrity result
    //
    // on Sandboxie 4 with highly restricted process token, we always need
    // to adjust the SplWow64 object path
    //

    Ipc_GetName_AdjustSplWow64Path(*OutTruePath, TRUE);

    //
    // if the true path leads inside the box, we advance the pointer
    //

    length = wcslen(*OutTruePath);

check_sandbox_prefix:

    //if (Dll_AlernateIpcNaming)
    //{
    //    if (length >= Dll_BoxIpcPathLen &&
    //        0 == Dll_NlsStrCmp(
    //            &(*OutTruePath)[length - Dll_BoxIpcPathLen], Dll_BoxIpcPath, Dll_BoxIpcPathLen))
    //    {
    //        (*OutTruePath)[length - Dll_BoxIpcPathLen] = L'\0';
    //        length -= Dll_BoxIpcPathLen;
    //        if (OutIsBoxedPath)
    //            *OutIsBoxedPath = TRUE;
    //
    //        goto check_sandbox_prefix;
    //    }
    //}
    //else
    if (length >= Dll_BoxIpcPathLen &&
            0 == Dll_NlsStrCmp(
                *OutTruePath, Dll_BoxIpcPath, Dll_BoxIpcPathLen))
    {
        *OutTruePath += Dll_BoxIpcPathLen;
        length -= Dll_BoxIpcPathLen;
        if (OutIsBoxedPath)
            *OutIsBoxedPath = TRUE;

        goto check_sandbox_prefix;
    }

    //
    // now create the copy path, which is the box prefix prepended
    // to the true path that we have.  note that the copy path will
    // still be missing its null terminator.
    //

    name = Dll_GetTlsNameBuffer(
                TlsData, COPY_NAME_BUFFER, Dll_BoxIpcPathLen + length);

    *OutCopyPath = name;

    //if (Dll_AlernateIpcNaming)
    //{
    //    wmemcpy(name, *OutTruePath, length);
    //    name += length;
    //
    //    wmemcpy(name, Dll_BoxIpcPath, Dll_BoxIpcPathLen);
    //    name += Dll_BoxIpcPathLen;
    //}
    //else
    {
        wmemcpy(name, Dll_BoxIpcPath, Dll_BoxIpcPathLen);
        name += Dll_BoxIpcPathLen;

        wmemcpy(name, *OutTruePath, length);
        name += length;
    }

    *name = L'\0';

    //
    // finish
    //

    return STATUS_SUCCESS;
}


//---------------------------------------------------------------------------
// Ipc_GetName_AdjustSplWow64Path
//---------------------------------------------------------------------------


_FX BOOLEAN Ipc_GetName_AdjustSplWow64Path(WCHAR *TruePath, BOOLEAN adj)
{
    //
    // SplWow64 is a mechanism implemented in WinSpool.drv and win32k.sys
    // where 32-bit processes that need a printer device context get a
    // proxy DC through the SplWow64 process
    //
    // SplWow64 is expected to create some IPC objects which include the
    // process integrity level in their name.  we need to override these
    // names to specify zero.  this is because win32k.sys will try to
    // connect to the IPC port, and will use the primary process token
    // (which has a zero integrity level) to construct the object name.
    //
    // (but note that on Windows 8, the SplWow64 mechanism in win32k
    // assumes an integrity level not less than 0x2000.)
    //
    // also for this reason, core/drv/ipc.c includes these objects in the
    // list of default open IPC paths.  this further means that only one
    // SplWow64 process can be running for all zero integrity processes,
    // even if they are in different sandboxes.
    //
    // Gdi_SplWow64 and GuiServer::SplWow64Slave help with making sure
    // a sandbox that wants to print has a corresponding SplWow64 process.
    //

    // NoSbieDesk BEGIN
    if (Dll_CompartmentMode || SbieApi_QueryConfBool(NULL, L"NoSandboxieDesktop", FALSE))
        return TRUE;
	// NoSbieDesk END

    WCHAR *nameStart = wcsrchr(TruePath, L'\\');
    if (nameStart && (0 == _wcsnicmp(nameStart + 1, L"SplWow64_", 9)
             ||  0 == _wcsnicmp(nameStart + 1, L"UmpdProxy_", 10)
             ||  0 == _wcsnicmp(nameStart + 1, L"WinSpl64to32Mutex_", 18))) {

        WCHAR *name = wcsrchr(nameStart, L'_');

        if (name && name[1]) {

            if (! adj)
                return TRUE;

            Gdi_SplWow64(FALSE);

            if (Dll_OsBuild == 7600 && name[2] == L'\0') {

                //
                // Windows 7 RTM: splwow64_x_yyyyy_z_n (n=0,1,2,3,4)
                // n=0 corresponds to SECURITY_MANDATORY_LOW_RID
                //

                name[1] = L'0';

            } else if (Dll_OsBuild >= 7601
                    && name[2] == L'0' && name[3] == L'0' && name[4] == L'0'
                    && name[5] == L'\0') {

                // Windows 7 SP1:  splwow64_x_yyyyy_z_nnnn
                // n=1000,2000,3000,4000,5000
                // as per SECURITY_MANDATORY_xxx_RID defines

                if (Dll_OsBuild >= 8400) {
                    WCHAR *authid = NULL;

                    // Windows 8: force nnnn=2000 and yyyyy = 3e6 (ANONYMOUS_LOGON_LUID see WinNT.h)

                    name[1] = L'2';

                    // debug log
                    // OutputDebugString(nameStart);
                    // OutputDebugString(L"\n");

                    authid = wcschr(nameStart, L'_');
                    if (authid)
                    {
                        if (0 != _wcsnicmp(nameStart + 1, L"WinSpl64to32Mutex_", 18))
                        {
                            authid = wcschr(authid+1, L'_');
                        }

                        if (authid)
                        {   
                            // make sure truepath's authid is long enough
                            if  (   (name > authid) 
                                &&  (name - authid) >= (3*sizeof(WCHAR))
                                )
                            {
                                if (0 != memcmp(authid+1, L"3e6", 3*sizeof(WCHAR)))
                                {
                                    memcpy(authid+1, L"3e6", 3*sizeof(WCHAR));

                                    name -= 2;
                                    authid += 4;
                                    while (*name != L'\0')
                                    {
                                        *authid = *name;
                                        name++;
                                        authid++;
                                    }

                                    *authid = L'\0';
                                }
                                else
                                {
                                    // OutputDebugString(L"already 3e6\n");
                                }
                            }
                            else
                            {
                                // OutputDebugString(L"true path is not long enough\n");
                            }
                        }
                    }

                } else {

                    // Windows 7: force nnnn=0

                    name[1] = L'0';
                    name[2] = L'\0';
                }
            }

            return TRUE;
        }
    }

    return FALSE;
}


//---------------------------------------------------------------------------
// Ipc_GetName2
//---------------------------------------------------------------------------


_FX NTSTATUS Ipc_GetName2(
    OBJECT_ATTRIBUTES *ObjectAttributes,
    WCHAR **OutTruePath, WCHAR **OutCopyPath,
    const WCHAR* NtName)
{
    NTSTATUS status;
    HANDLE RootDirectory;
    UNICODE_STRING *ObjectName;

    if (ObjectAttributes) {

        RootDirectory = ObjectAttributes->RootDirectory;
        ObjectName = ObjectAttributes->ObjectName;

    } else {

        RootDirectory = NULL;
        ObjectName = NULL;
    }

    status = Ipc_GetName(
        RootDirectory, ObjectName, OutTruePath, OutCopyPath, NULL);

    if (NT_SUCCESS(status)) 
    {
        WCHAR msg[1024];
        Sbie_snwprintf(msg, 1024, L"%s: %s", NtName, *OutTruePath);
        SbieApi_MonitorPutMsg(MONITOR_IPC | MONITOR_TRACE, msg);
    }

    return status;
}


//---------------------------------------------------------------------------
// Ipc_CreatePath
//---------------------------------------------------------------------------


_FX NTSTATUS Ipc_CreatePath(WCHAR *TruePath, WCHAR *CopyPath)
{
    NTSTATUS status;
    HANDLE handle;
    OBJECT_ATTRIBUTES objattrs;
    UNICODE_STRING objname;
    WCHAR *backslash;

    //if (Dll_AlernateIpcNaming)
    //    return STATUS_OBJECT_PATH_NOT_FOUND;

    //
    // open the TruePath object directory containing the object
    // for which we got STATUS_OBJECT_PATH_NOT_FOUND
    //

    backslash = wcsrchr(TruePath, L'\\');
    if (! backslash)
        return STATUS_OBJECT_PATH_NOT_FOUND;

    *backslash = L'\0';

    InitializeObjectAttributes(
        &objattrs, &objname, OBJ_CASE_INSENSITIVE, NULL, NULL);

    RtlInitUnicodeString(&objname, TruePath);

    status = __sys_NtOpenDirectoryObject(&handle, DIRECTORY_QUERY, &objattrs);

    *backslash = L'\\';

    if (! NT_SUCCESS(status))
        return status;

    NtClose(handle);

    //
    // create the parent directories along the CopyPath
    //

    backslash = CopyPath;

    while (1) {

        backslash = wcschr(backslash + 1, L'\\');
        if (! backslash)
            break;

        if ((ULONG)(backslash - CopyPath) > Dll_BoxIpcPathLen) {

            *backslash = L'\0';

            status = SbieApi_CreateDirOrLink(CopyPath, NULL);

            *backslash = L'\\';

            if (! NT_SUCCESS(status))
                return status;
        }
    }

    return STATUS_SUCCESS;
}


//---------------------------------------------------------------------------
// Ipc_AdjustPortPath
//---------------------------------------------------------------------------


_FX void Ipc_AdjustPortPath(UNICODE_STRING *ObjectName)
{
    NTSTATUS status;
    WCHAR *Buffer;
    WCHAR *BackslashPtr;
    ULONG ParentLength;
    OBJECT_ATTRIBUTES objattrs;
    UNICODE_STRING objname;
    HANDLE handle;

    //
    // NtConnectPort family of functions are case sensitive, so if we
    // get an error STATUS_OBJECT_PATH_NOT_FOUND, then open the directory
    // containing the target port object, and query its path, to make
    // sure case sensitivity is correct
    //

    Buffer = ObjectName->Buffer;

    BackslashPtr = wcsrchr(Buffer, L'\\');
    if (! BackslashPtr)
        return;

    ParentLength = (ULONG)(BackslashPtr - Buffer);
    *BackslashPtr = L'\0';

    RtlInitUnicodeString(&objname, Buffer);

    InitializeObjectAttributes(
        &objattrs, &objname, OBJ_CASE_INSENSITIVE, NULL, NULL);

    status = __sys_NtOpenDirectoryObject(&handle, DIRECTORY_QUERY, &objattrs);

    if (NT_SUCCESS(status)) {

        ULONG length = sizeof(OBJECT_NAME_INFORMATION)
                     + (ParentLength + 8) * sizeof(WCHAR);
        OBJECT_NAME_INFORMATION *name = Dll_AllocTemp(length);

        status = Obj_GetObjectName(handle, name, &length);

        if (NT_SUCCESS(status) &&
                name->Name.Length >= ParentLength * sizeof(WCHAR) &&
            0 == _wcsnicmp(name->Name.Buffer, Buffer, ParentLength)) {

            wmemcpy(Buffer, name->Name.Buffer, ParentLength);
        }

        Dll_Free(name);
    }

    *BackslashPtr = L'\\';
}


//---------------------------------------------------------------------------
// Ipc_NtCreatePort
//---------------------------------------------------------------------------


_FX NTSTATUS Ipc_NtCreatePort(
    HANDLE *PortHandle,
    OBJECT_ATTRIBUTES *ObjectAttributes,
    ULONG MaxConnectInfoLength,
    ULONG MaxDataLength,
    ULONG *Reserved)
{
    ULONG LastError;
    THREAD_DATA *TlsData = Dll_GetTlsData(&LastError);

    NTSTATUS status;
    OBJECT_ATTRIBUTES objattrs;
    UNICODE_STRING objname;
    WCHAR *TruePath;
    WCHAR *CopyPath;
    ULONG mp_flags;

    Dll_PushTlsNameBuffer(TlsData);

    __try {

    //
    // get the full paths for the true and copy objects
    //

    status = Ipc_GetName2(ObjectAttributes, &TruePath, &CopyPath, L"CreatePort");
    if (! NT_SUCCESS(status))
        __leave;

    InitializeObjectAttributes(&objattrs,
        &objname, OBJECT_ATTRIBUTES_ATTRIBUTES, NULL, Secure_EveryoneSD);

    //
    // check if this is an open or closed path
    //

    mp_flags = SbieDll_MatchPath2(L'i', TruePath, FALSE, TRUE); // SbieDll_MatchPath(L'i', TruePath);

    if (PATH_IS_CLOSED(mp_flags)) {
        status = STATUS_ACCESS_DENIED;
        __leave;
    }

    if (PATH_IS_OPEN(mp_flags)) {

        RtlInitUnicodeString(&objname, TruePath);
        objattrs.SecurityDescriptor = ObjectAttributes->SecurityDescriptor;

        status = __sys_NtCreatePort(
            PortHandle, &objattrs,
            MaxConnectInfoLength, MaxDataLength, Reserved);

        __leave;
    }

    //
    // try to create the object name by its CopyPath, creating the
    // CopyPath hierarchy if needed
    //

    RtlInitUnicodeString(&objname, CopyPath);

    status = __sys_NtCreatePort(
        PortHandle, &objattrs,
        MaxConnectInfoLength, MaxDataLength, Reserved);

    if (status == STATUS_OBJECT_PATH_NOT_FOUND) {

        status = Ipc_CreatePath(TruePath, CopyPath);

        if (NT_SUCCESS(status)) {
            status = __sys_NtCreatePort(
                PortHandle, &objattrs,
                MaxConnectInfoLength, MaxDataLength, Reserved);
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
// Ipc_NtConnectPort
//---------------------------------------------------------------------------


_FX NTSTATUS Ipc_NtConnectPort(
    HANDLE *PortHandle,
    UNICODE_STRING *PortName,
    SECURITY_QUALITY_OF_SERVICE *SecurityQos,
    PLPC_SECTION_OWNER_MEMORY ClientSharedMemory,
    PLPC_SECTION_MEMORY ServerSharedMemory,
    ULONG *MaximumMessageLength,
    void *ConnectionInfo,
    ULONG *ConnectionInfoLength)
{
    ULONG LastError;
    THREAD_DATA *TlsData = Dll_GetTlsData(&LastError);

    NTSTATUS status;
    UNICODE_STRING objname;
    WCHAR *TruePath;
    WCHAR *CopyPath;
    ULONG mp_flags;

    Dll_PushTlsNameBuffer(TlsData);

    __try {

    //
    // get the full paths for the true and copy objects
    //

    status = Ipc_GetName(NULL, PortName, &TruePath, &CopyPath, NULL);
    if (! NT_SUCCESS(status))
        __leave;

    //
    // check if this is an open or closed path
    //

    mp_flags = SbieDll_MatchPath(L'i', TruePath);

    if (PATH_IS_CLOSED(mp_flags)) {
        status = STATUS_ACCESS_DENIED;
        __leave;
    }

    if (PATH_IS_OPEN(mp_flags)) goto OpenTruePath;

    //
    // check for proxy LPC port
    //

    status = Ipc_ConnectProxyPort(
        PortHandle, TruePath, -1,
        ConnectionInfo, ConnectionInfoLength, MaximumMessageLength,
        ClientSharedMemory, NULL, ServerSharedMemory);

    if (status == STATUS_BAD_INITIAL_STACK)
        goto OpenTruePath;
    if (status != STATUS_BAD_INITIAL_PC)
        __leave;

    //
    // if trying to connect to a COM port, start our COM servers first
    //

    Ipc_StartServer(TruePath, FALSE);

    //
    // try to connect to the port object by its CopyPath
    //

    RtlInitUnicodeString(&objname, CopyPath);

    status = __sys_NtConnectPort(
        PortHandle, &objname, SecurityQos,
        ClientSharedMemory, ServerSharedMemory,
        MaximumMessageLength, ConnectionInfo, ConnectionInfoLength);

    if (status == STATUS_OBJECT_PATH_NOT_FOUND) {

        status = Ipc_CreatePath(TruePath, CopyPath);

        if (NT_SUCCESS(status)) {

            Ipc_AdjustPortPath(&objname);

            status = __sys_NtConnectPort(
                PortHandle, &objname, SecurityQos,
                ClientSharedMemory, ServerSharedMemory,
                MaximumMessageLength, ConnectionInfo, ConnectionInfoLength);
        }
    }

    __leave;

    //
    // try the TruePath
    //

OpenTruePath:

    RtlInitUnicodeString(&objname, TruePath);

    status = __sys_NtConnectPort(
        PortHandle, &objname, SecurityQos,
        ClientSharedMemory, ServerSharedMemory,
        MaximumMessageLength, ConnectionInfo, ConnectionInfoLength);

    if (PATH_NOT_OPEN(mp_flags) && (status == STATUS_ACCESS_DENIED))
        status = STATUS_OBJECT_NAME_NOT_FOUND;

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
// Ipc_NtSecureConnectPort
//---------------------------------------------------------------------------


_FX NTSTATUS Ipc_NtSecureConnectPort(
    HANDLE *PortHandle,
    UNICODE_STRING *PortName,
    SECURITY_QUALITY_OF_SERVICE *SecurityQos,
    PLPC_SECTION_OWNER_MEMORY ClientSharedMemory,
    PSID ServerSid,
    PLPC_SECTION_MEMORY ServerSharedMemory,
    ULONG *MaximumMessageLength,
    void *ConnectionInfo,
    ULONG *ConnectionInfoLength)
{
    ULONG LastError;
    THREAD_DATA *TlsData = Dll_GetTlsData(&LastError);

    NTSTATUS status;
    UNICODE_STRING objname;
    WCHAR *TruePath;
    WCHAR *CopyPath;
    ULONG mp_flags;

    Dll_PushTlsNameBuffer(TlsData);

    __try {

    //
    // get the full paths for the true and copy objects
    //

    status = Ipc_GetName(NULL, PortName, &TruePath, &CopyPath, NULL);
    if (! NT_SUCCESS(status))
        __leave;

    //
    // check if this is an open or closed path
    //

    mp_flags = SbieDll_MatchPath(L'i', TruePath);

    if (PATH_IS_CLOSED(mp_flags)) {
        status = STATUS_ACCESS_DENIED;
        __leave;
    }

    if (PATH_IS_OPEN(mp_flags)) goto OpenTruePath;

    //
    // check for proxy LPC port
    //

    status = Ipc_ConnectProxyPort(
        PortHandle, TruePath, -1,
        ConnectionInfo, ConnectionInfoLength, MaximumMessageLength,
        ClientSharedMemory, ServerSid, ServerSharedMemory);

    if (status == STATUS_BAD_INITIAL_STACK)
        goto OpenTruePath;
    if (status != STATUS_BAD_INITIAL_PC)
        __leave;

    //
    // if trying to connect to a COM port, start our COM servers first
    //

    Ipc_StartServer(TruePath, FALSE);

    //
    // try to create the object name by its CopyPath.
    //

    RtlInitUnicodeString(&objname, CopyPath);

    status = __sys_NtSecureConnectPort(
        PortHandle, &objname, SecurityQos,
        ClientSharedMemory, ServerSid, ServerSharedMemory,
        MaximumMessageLength, ConnectionInfo, ConnectionInfoLength);

    if (status == STATUS_OBJECT_PATH_NOT_FOUND) {

        status = Ipc_CreatePath(TruePath, CopyPath);

        if (NT_SUCCESS(status)) {

            Ipc_AdjustPortPath(&objname);

            status = __sys_NtSecureConnectPort(
                PortHandle, &objname, SecurityQos,
                ClientSharedMemory, ServerSid, ServerSharedMemory,
                MaximumMessageLength, ConnectionInfo, ConnectionInfoLength);
        }
    }

    //
    // the sandboxed port server may not be running under the expected
    // system SID, but under the user's own SID, so try again with a
    // NULL ServerSid parameter
    //

    if (status == STATUS_SERVER_SID_MISMATCH) {

        ServerSid = NULL;

        status = __sys_NtSecureConnectPort(
            PortHandle, &objname, SecurityQos,
            ClientSharedMemory, ServerSid, ServerSharedMemory,
            MaximumMessageLength, ConnectionInfo, ConnectionInfoLength);
    }

    __leave;

    //
    // try the TruePath
    //

OpenTruePath:

    RtlInitUnicodeString(&objname, TruePath);

    status = __sys_NtSecureConnectPort(
        PortHandle, &objname, SecurityQos,
        ClientSharedMemory, ServerSid, ServerSharedMemory,
        MaximumMessageLength, ConnectionInfo, ConnectionInfoLength);

    if (status == STATUS_SERVER_SID_MISMATCH) {

        ServerSid = NULL;

        status = __sys_NtSecureConnectPort(
            PortHandle, &objname, SecurityQos,
            ClientSharedMemory, ServerSid, ServerSharedMemory,
            MaximumMessageLength, ConnectionInfo, ConnectionInfoLength);
    }

    if (PATH_NOT_OPEN(mp_flags) && (status == STATUS_ACCESS_DENIED))
        status = STATUS_OBJECT_NAME_NOT_FOUND;

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
// Ipc_NtAlpcCreatePort
//---------------------------------------------------------------------------


_FX NTSTATUS Ipc_NtAlpcCreatePort(
    HANDLE *PortHandle,
    OBJECT_ATTRIBUTES *ObjectAttributes,
    void *AlpcCreateInfo)
{
    ULONG LastError;
    THREAD_DATA *TlsData = Dll_GetTlsData(&LastError);

    NTSTATUS status;
    OBJECT_ATTRIBUTES objattrs;
    UNICODE_STRING objname;
    WCHAR *TruePath;
    WCHAR *CopyPath;
    ULONG mp_flags;

    Dll_PushTlsNameBuffer(TlsData);

    __try {

    //
    // get the full paths for the true and copy objects
    //

    status = Ipc_GetName2(ObjectAttributes, &TruePath, &CopyPath, L"AlpcCreatePort");
    if (! NT_SUCCESS(status))
        __leave;

    InitializeObjectAttributes(&objattrs,
        &objname, OBJECT_ATTRIBUTES_ATTRIBUTES, NULL, Secure_EveryoneSD);

    //
    // check if this is an open or closed path
    //

    mp_flags = SbieDll_MatchPath2(L'i', TruePath, FALSE, TRUE); // SbieDll_MatchPath(L'i', TruePath);

    if (PATH_IS_CLOSED(mp_flags)) {
        status = STATUS_ACCESS_DENIED;
        __leave;
    }

    if (PATH_IS_OPEN(mp_flags)) {

        RtlInitUnicodeString(&objname, TruePath);
        objattrs.SecurityDescriptor = ObjectAttributes->SecurityDescriptor;

        status = __sys_NtAlpcCreatePort(
            PortHandle, &objattrs, AlpcCreateInfo);

        __leave;
    }

    //
    // try to create the object name by its CopyPath, creating the
    // CopyPath hierarchy if needed
    //

    RtlInitUnicodeString(&objname, CopyPath);

    status = __sys_NtAlpcCreatePort(
        PortHandle, &objattrs, AlpcCreateInfo);

    if (status == STATUS_OBJECT_PATH_NOT_FOUND) {

        status = Ipc_CreatePath(TruePath, CopyPath);

        if (NT_SUCCESS(status)) {
            status = __sys_NtAlpcCreatePort(
                PortHandle, &objattrs, AlpcCreateInfo);
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
// Ipc_NtAlpcConnectPort
//---------------------------------------------------------------------------


_FX NTSTATUS Ipc_NtAlpcConnectPort(
    HANDLE *PortHandle,
    UNICODE_STRING *PortName,
    OBJECT_ATTRIBUTES *ObjectAttributes,
    ALPC_PORT_ATTRIBUTES *AlpcConnectInfo,
    ULONG ConnectionFlags,
    PSID ServerSid,
    void *ConnectionInfo,
    ULONG *ConnectionInfoLength,
    void *InMessageBuffer,
    void *OutMessageBuffer,
    LARGE_INTEGER *Timeout)
{
    ULONG LastError;
    THREAD_DATA *TlsData = Dll_GetTlsData(&LastError);

    NTSTATUS status;
    UNICODE_STRING objname;
    WCHAR *TruePath;
    WCHAR *CopyPath;
    ULONG mp_flags;

    Dll_PushTlsNameBuffer(TlsData);

    __try {

    //
    // get the full paths for the true and copy objects
    //

    status = Ipc_GetName(NULL, PortName, &TruePath, &CopyPath, NULL);
    if (! NT_SUCCESS(status))
        __leave;

    //
    // check if this is an open or closed path
    //

    mp_flags = SbieDll_MatchPath(L'i', TruePath);

    if (PATH_IS_CLOSED(mp_flags)) {
        status = STATUS_ACCESS_DENIED;
        __leave;
    }

    if (PATH_IS_OPEN(mp_flags))
        goto OpenTruePath;

    // Is this a dynamic RPC port that we need to open?
    if(RpcRt_IsDynamicPortOpen(TruePath))
        goto OpenTruePath;

    //
    // check for proxy LPC port
    //

    status = Ipc_ConnectProxyPort(
        PortHandle, TruePath, ConnectionFlags,
        ConnectionInfo, ConnectionInfoLength, AlpcConnectInfo,
        NULL, ServerSid, NULL);

    if (status == STATUS_BAD_INITIAL_STACK)
        goto OpenTruePath;
    if (status != STATUS_BAD_INITIAL_PC)
        __leave;

    //
    // if trying to connect to a COM port, start our COM servers first
    //

    Ipc_StartServer(TruePath, FALSE);

    //
    // try to connect to the port object by its CopyPath
    //

    RtlInitUnicodeString(&objname, CopyPath);

    status = __sys_NtAlpcConnectPort(
        PortHandle, &objname, ObjectAttributes,
        AlpcConnectInfo, ConnectionFlags, ServerSid,
        ConnectionInfo, ConnectionInfoLength,
        InMessageBuffer, OutMessageBuffer, Timeout);

    if (status == STATUS_OBJECT_PATH_NOT_FOUND) {

        status = Ipc_CreatePath(TruePath, CopyPath);

        if (NT_SUCCESS(status)) {

            Ipc_AdjustPortPath(&objname);

            status = __sys_NtAlpcConnectPort(
                PortHandle, &objname, ObjectAttributes,
                AlpcConnectInfo, ConnectionFlags, ServerSid,
                ConnectionInfo, ConnectionInfoLength,
                InMessageBuffer, OutMessageBuffer, Timeout);
        }
    }

    //
    // the sandboxed port server may not be running under the expected
    // system SID, but under the user's own SID, so try again with a
    // NULL ServerSid parameter
    //

    if (status == STATUS_SERVER_SID_MISMATCH) {

        ServerSid = NULL;

        status = __sys_NtAlpcConnectPort(
            PortHandle, &objname, ObjectAttributes,
            AlpcConnectInfo, ConnectionFlags, ServerSid,
            ConnectionInfo, ConnectionInfoLength,
            InMessageBuffer, OutMessageBuffer, Timeout);
    }

    __leave;

    //
    // try the TruePath
    //

OpenTruePath:

    RtlInitUnicodeString(&objname, TruePath);

    status = __sys_NtAlpcConnectPort(
        PortHandle, &objname, ObjectAttributes,
        AlpcConnectInfo, ConnectionFlags, ServerSid,
        ConnectionInfo, ConnectionInfoLength,
        InMessageBuffer, OutMessageBuffer, Timeout);

    if (status == STATUS_SERVER_SID_MISMATCH) {

        ServerSid = NULL;

        status = __sys_NtAlpcConnectPort(
            PortHandle, &objname, ObjectAttributes,
            AlpcConnectInfo, ConnectionFlags, ServerSid,
            ConnectionInfo, ConnectionInfoLength,
            InMessageBuffer, OutMessageBuffer, Timeout);
    }

    if (PATH_NOT_OPEN(mp_flags) && (status == STATUS_ACCESS_DENIED))
        status = STATUS_OBJECT_NAME_NOT_FOUND;

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
// Ipc_NtAlpcConnectPortEx
//---------------------------------------------------------------------------


_FX NTSTATUS Ipc_NtAlpcConnectPortEx(
    HANDLE *PortHandle,
    OBJECT_ATTRIBUTES *ObjectAttributes1,
    OBJECT_ATTRIBUTES *ObjectAttributes2,
    ALPC_PORT_ATTRIBUTES *AlpcConnectInfo,
    ULONG ConnectionFlags,
    SECURITY_DESCRIPTOR *ServerSd,
    void *ConnectionInfo,
    ULONG *ConnectionInfoLength,
    void *InMessageBuffer,
    void *OutMessageBuffer,
    LARGE_INTEGER *Timeout)
{
    ULONG LastError;
    THREAD_DATA *TlsData = Dll_GetTlsData(&LastError);

    NTSTATUS status;
    OBJECT_ATTRIBUTES *ObjectAttributes;
    OBJECT_ATTRIBUTES objattrs;
    UNICODE_STRING objname;
    WCHAR *TruePath;
    WCHAR *CopyPath;
    ULONG mp_flags;

    Dll_PushTlsNameBuffer(TlsData);

    __try {

    //
    // get the full paths for the true and copy objects
    //

    status = Ipc_GetName(NULL, ObjectAttributes1->ObjectName,
                         &TruePath, &CopyPath, NULL);
    if (! NT_SUCCESS(status))
        __leave;

    ObjectAttributes = ObjectAttributes1;
    InitializeObjectAttributes(&objattrs,
        &objname, OBJECT_ATTRIBUTES_ATTRIBUTES, NULL, Secure_EveryoneSD);

    //
    // check if this is an open or closed path
    //

    mp_flags = SbieDll_MatchPath(L'i', TruePath);

    if (PATH_IS_CLOSED(mp_flags)) {
        status = STATUS_ACCESS_DENIED;
        __leave;
    }

    if (PATH_IS_OPEN(mp_flags))
        goto OpenTruePath;

    // copied from Ipc_NtAlpcConnectPort above. We need to see if code from Ipc_NtAlpcConnectPortEx
    // and Ipc_NtAlpcConnectPort can be merged to eliminate code duplication.

    // Is this a dynamic RPC port that we need to open?
    if(RpcRt_IsDynamicPortOpen(TruePath))
        goto OpenTruePath;

    //
    // check for proxy LPC port
    //

    status = Ipc_ConnectProxyPort(
        PortHandle, TruePath, ConnectionFlags,
        ConnectionInfo, ConnectionInfoLength, AlpcConnectInfo,
        NULL, ServerSd, NULL);
    
    if (status == STATUS_BAD_INITIAL_STACK)
        goto OpenTruePath;
    if (status != STATUS_BAD_INITIAL_PC)
        __leave;

    //
    // if trying to connect to a COM port, start our COM servers first
    //

    Ipc_StartServer(TruePath, FALSE);

    //
    // try to connect to the port object by its CopyPath
    //

    RtlInitUnicodeString(&objname, CopyPath);

    status = __sys_NtAlpcConnectPortEx(
        PortHandle, &objattrs, ObjectAttributes2,
        AlpcConnectInfo, ConnectionFlags, ServerSd,
        ConnectionInfo, ConnectionInfoLength,
        InMessageBuffer, OutMessageBuffer, Timeout);

    if (status == STATUS_OBJECT_PATH_NOT_FOUND) {

        status = Ipc_CreatePath(TruePath, CopyPath);

        if (NT_SUCCESS(status)) {

            Ipc_AdjustPortPath(&objname);

            status = __sys_NtAlpcConnectPortEx(
                PortHandle, &objattrs, ObjectAttributes2,
                AlpcConnectInfo, ConnectionFlags, ServerSd,
                ConnectionInfo, ConnectionInfoLength,
                InMessageBuffer, OutMessageBuffer, Timeout);
        }
    }

    //
    // the sandboxed port server may not be running under the expected
    // system SID, but under the user's own SID, so try again with a
    // NULL ServerSid parameter
    //

    if (status == STATUS_SERVER_SID_MISMATCH) {

        ServerSd = NULL;

        status = __sys_NtAlpcConnectPortEx(
            PortHandle, &objattrs, ObjectAttributes2,
            AlpcConnectInfo, ConnectionFlags, ServerSd,
            ConnectionInfo, ConnectionInfoLength,
            InMessageBuffer, OutMessageBuffer, Timeout);
    }

    __leave;

    //
    // try the TruePath
    //

OpenTruePath:

    if (Dll_OsBuild >= 8400 && ServerSd &&
            _wcsnicmp(TruePath, L"\\RPC Control\\splwow64_", 12) == 0) {
        //
        // on Windows 8, we want to discard the server SD argument
        // when connecting on the ALPC port for SplWow64
        //
        ServerSd = NULL;
    }

    RtlInitUnicodeString(&objname, TruePath);

    status = __sys_NtAlpcConnectPortEx(
        PortHandle, &objattrs, ObjectAttributes2,
        AlpcConnectInfo, ConnectionFlags, ServerSd,
        ConnectionInfo, ConnectionInfoLength,
        InMessageBuffer, OutMessageBuffer, Timeout);

    if (status == STATUS_SERVER_SID_MISMATCH) {

        ServerSd = NULL;

        status = __sys_NtAlpcConnectPortEx(
            PortHandle, &objattrs, ObjectAttributes2,
            AlpcConnectInfo, ConnectionFlags, ServerSd,
            ConnectionInfo, ConnectionInfoLength,
            InMessageBuffer, OutMessageBuffer, Timeout);
    }

    if (PATH_NOT_OPEN(mp_flags) && (status == STATUS_ACCESS_DENIED))
        status = STATUS_OBJECT_NAME_NOT_FOUND;

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
// Ipc_NtAlpcQueryInformation
//---------------------------------------------------------------------------


_FX NTSTATUS Ipc_NtAlpcQueryInformation(
    HANDLE *PortHandle,
    ULONG InformationClass,
    void *Information,
    ULONG InformationLength,
    ULONG_PTR UnknownParameter5)
{
    NTSTATUS status;

    status = __sys_NtAlpcQueryInformation(
        PortHandle, InformationClass, Information, InformationLength,
        UnknownParameter5);

    //
    // in Vista, COM-related libraries use Information Class 3 to verify
    // that the SID of the port server is a known system SID.
    // (InformationClass = 3, Information --> SID to verify, and
    //  InformationLength = Sid Length.  Returns STATUS_SERVER_SID_MISMATCH
    //  if the actual server SID is not the same as the one passed.)
    //
    // inside the sandbox, the port server runs under the user account, so
    // we have to force a successful status here
    //

    if (status == STATUS_SERVER_SID_MISMATCH)
        status = STATUS_SUCCESS;

    return status;
}


//---------------------------------------------------------------------------
// Ipc_NtAlpcQueryInformationMessage
//---------------------------------------------------------------------------


_FX NTSTATUS Ipc_NtAlpcQueryInformationMessage(
    HANDLE *PortHandle,
    void *RpcMessage,
    ULONG InformationClass,
    void *Information,
    ULONG InformationLength,
    ULONG_PTR UnknownParameter6)
{
    NTSTATUS status;

    status = __sys_NtAlpcQueryInformationMessage(
        PortHandle, RpcMessage, InformationClass, Information,
        InformationLength, UnknownParameter6);

    //
    // in Windows 7, COM-related libraries use Information Class 1 to verify
    // something.  Status STATUS_BAD_IMPERSONATION_LEVEL is returned, and
    // we need to force a successful status.
    //

    if (status == STATUS_BAD_IMPERSONATION_LEVEL)
        status = STATUS_SUCCESS;

    return status;
}


//---------------------------------------------------------------------------
// Ipc_IsAdminToken
//---------------------------------------------------------------------------


_FX BOOLEAN Ipc_IsAdminToken(HANDLE hToken)
{
    static UCHAR AdministratorsSid[16] = {
        1,                                      // Revision
        2,                                      // SubAuthorityCount
        0,0,0,0,0,5, // SECURITY_NT_AUTHORITY   // IdentifierAuthority
        0x20, 0, 0, 0,   // SubAuthority 1 - SECURITY_BUILTIN_DOMAIN_RID
        0x20, 2, 0, 0    // SubAuthority 2 - DOMAIN_ALIAS_RID_ADMINS
    };

    BOOL IsMember;

    typedef BOOL (*P_CheckTokenMembership)(
        HANDLE hToken, void *pSid, BOOL *IsMember);

    P_CheckTokenMembership pCheckTokenMembership = (P_CheckTokenMembership)
        Ldr_GetProcAddrNew(
            (Dll_KernelBase ? DllName_kernelbase : DllName_advapi32),
                L"CheckTokenMembership","CheckTokenMembership");

    if (! pCheckTokenMembership)
        return FALSE;

    if (! pCheckTokenMembership(hToken, &AdministratorsSid, &IsMember))
        return FALSE;

    if (! IsMember)
        return FALSE;

    return TRUE;
}


//---------------------------------------------------------------------------
// Ipc_ImpersonateSelf
//---------------------------------------------------------------------------


_FX NTSTATUS Ipc_ImpersonateSelf(PORT_MESSAGE *PortMsg)
{
    NTSTATUS status;
    HANDLE hOldToken, hNewToken;
    HANDLE hPriToken;
    OBJECT_ATTRIBUTES objattrs;
    SECURITY_QUALITY_OF_SERVICE QoS;

    //
    // this function is called after NtImpersonateClientOfPort.
    // if impersonating is successful at SecurityImpersonation level,
    // then do nothing.  otherwise, impersonate our own primary token
    //

    status = NtOpenThreadToken(
                        NtCurrentThread(), TOKEN_QUERY, FALSE, &hOldToken);

    if (! NT_SUCCESS(status)) {

        hOldToken = NULL;

    } else {

        ULONG ImpLevel, len;

        status = NtQueryInformationToken(
            hOldToken, TokenImpersonationLevel,
            &ImpLevel, sizeof(ImpLevel), &len);

        if (! NT_SUCCESS(status))
            ImpLevel = SecurityAnonymous;

        if (PortMsg && (ImpLevel >= SecurityImpersonation)
                && (! (Dll_ProcessFlags & SBIE_FLAG_IMAGE_FROM_SBIE_DIR))
                && (! Ipc_IsAdminToken(hOldToken))) {

            //
            // the following scenario needs special consideration:
            // - SandboxeRpcSs/SandboxieDcomLaunch are started under a
            //   limited user account
            // - the program creates an instance of some LocalServer32 COM
            //   object which requires elevation and is therefore started
            //   (by SandboxieDcomLaunch) under an administrator user account
            // - the LocalServer32 starts and issues CoRegisterClassObject,
            //   which causes SandboxeRpcSs to connect to it via ALPC
            // - the LocalServer32 tries to impersonate SandboxeRpcSs which
            //   means it effectively loses its administrator privileges
            //
            // to deal with this, we check (1) if the ALPC impersonation
            // request was successful (implying the process had administrator
            // privileges, or was the same account as the other process);
            // (2) if the new token is not an administrator; (3) if the
            // process on the other end of the ALPC port is RPCSS.
            // if all conditions are true, we do not actually impersonate.
            //

            HANDLE OtherProcessId;

            __try {
                OtherProcessId = PortMsg->ClientId.UniqueProcess;
            } __except (EXCEPTION_EXECUTE_HANDLER) {
                OtherProcessId = NULL;
            }
            if (OtherProcessId) {
                WCHAR ImageName[96];
                SbieApi_QueryProcess(
                                OtherProcessId, NULL, ImageName, NULL, NULL);
                if (_wcsicmp(ImageName, Ipc_SandboxieRpcSs) == 0) {

                    ImpLevel = SecurityAnonymous;
                }
            }
        }

        if (ImpLevel >= SecurityImpersonation) {

            NtClose(hOldToken);
            return STATUS_SUCCESS;
        }
    }

    //
    // cancel any active impersonation
    //

    hNewToken = NULL;

    status = NtSetInformationThread(
        NtCurrentThread(), ThreadImpersonationToken,
        &hNewToken, sizeof(HANDLE));

    //
    // duplicate our primary token into an impersonation token,
    // and activate impersonation on this new token
    //

    status = NtOpenProcessToken(
                        NtCurrentProcess(), TOKEN_DUPLICATE, &hPriToken);

    if (NT_SUCCESS(status)) {

        InitializeObjectAttributes(&objattrs, NULL, 0, NULL, NULL);
        QoS.Length = sizeof(SECURITY_QUALITY_OF_SERVICE);
        QoS.ImpersonationLevel = SecurityImpersonation;
        QoS.ContextTrackingMode = SECURITY_STATIC_TRACKING;
        QoS.EffectiveOnly = FALSE;
        objattrs.SecurityQualityOfService = &QoS;

        status = NtDuplicateToken(
            hPriToken, TOKEN_IMPERSONATE | TOKEN_QUERY, &objattrs,
            FALSE, TokenImpersonation, &hNewToken);

        if (NT_SUCCESS(status)) {

            status = NtSetInformationThread(
                NtCurrentThread(), ThreadImpersonationToken,
                &hNewToken, sizeof(HANDLE));

            NtClose(hNewToken);
        }

        NtClose(hPriToken);
    }

    //
    // on error, try to restore the original impersonation token
    //

    if (! NT_SUCCESS(status)) {

        if (hOldToken) {

            NtSetInformationThread(
                NtCurrentThread(), ThreadImpersonationToken,
                &hOldToken, sizeof(HANDLE));
        }
    }

    if (hOldToken)
        NtClose(hOldToken);

    return STATUS_SUCCESS;
}


//---------------------------------------------------------------------------
// Ipc_NtImpersonateClientOfPort
//---------------------------------------------------------------------------


_FX NTSTATUS Ipc_NtImpersonateClientOfPort(
    HANDLE PortHandle, PORT_MESSAGE *RequestMessage)
{
    //
    // NtImpersonateClientOfPort is generally not needed on Windows XP
    // because most users run as Administrators with SeImpersonatePrivilege
    // privilege that permits impersonation.  nevertheless this can't hurt.
    // see NtAlpcImpersonateClientOfPort below for more information.
    //

    NTSTATUS status =
        __sys_NtImpersonateClientOfPort(PortHandle, RequestMessage);

    if (! Dll_IsSystemSid)
        status = Ipc_ImpersonateSelf(NULL);

    return status;
}


//---------------------------------------------------------------------------
// Ipc_NtAlpcImpersonateClientOfPort
//---------------------------------------------------------------------------


_FX NTSTATUS Ipc_NtAlpcImpersonateClientOfPort(
    HANDLE PortHandle, PORT_MESSAGE *RequestMessage,
    ULONG_PTR UnknownParameter3)
{
    //
    // in a scenario where SandboxieRpcSs is running with low privileges
    // (typically in Windows Vista with UAC enabled), while the caller is
    // running as SYSTEM, NtAlpcImpersonateClientOfPort in SandboxieRpcSs
    // might fail, or else silently impersonate at identification level
    // depending on the situation.  (identification-level impersonation
    // will cause STATUS_BAD_IMPERSONATION_LEVEL in NtAlpcSendWaitReceive,
    // in NtQueryFullAttributesFile, and other cases as well.)
    //
    // this workaround allows this to succeed by impersonating our own
    // credentials, which is reasonable, because presumably everything in
    // the sandbox is running with the same set of credentials anyway
    // (even if some processes are running elevated tokens and others
    // use split UAC tokens)
    //

    NTSTATUS status = __sys_NtAlpcImpersonateClientOfPort(
                            PortHandle, RequestMessage, UnknownParameter3);

    if (! Dll_IsSystemSid)
        status = Ipc_ImpersonateSelf(RequestMessage);

    return status;
}


//---------------------------------------------------------------------------
// Ipc_NtImpersonateAnonymousToken
//---------------------------------------------------------------------------


_FX NTSTATUS Ipc_NtImpersonateAnonymousToken(HANDLE ThreadHandle)
{
    NTSTATUS status = __sys_NtImpersonateAnonymousToken(ThreadHandle);

    // note that an error STATUS_ACCESS_DENIED, in a new version of
    // Windows, would usually indicate that Dll_FixWow64Syscall has to
    // be fixed for the new Windows to properly select thunk zero

    if (status == STATUS_ACCESS_DENIED)
        SbieApi_Log(2205, L"ImpersonateAnonymousToken %08X", ThreadHandle);

    return status;
}


//---------------------------------------------------------------------------
// Ipc_NtImpersonateThread
//---------------------------------------------------------------------------


_FX NTSTATUS Ipc_NtImpersonateThread(
    HANDLE ServerThreadHandle, HANDLE ClientThreadHandle,
    PSECURITY_QUALITY_OF_SERVICE SecurityQos)
{
    SbieApi_Log(2205, L"ImpersonateThread");
    return STATUS_ACCESS_DENIED;
}


//---------------------------------------------------------------------------
// Ipc_NtCreateEvent
//---------------------------------------------------------------------------


_FX NTSTATUS Ipc_NtCreateEvent(
    HANDLE *EventHandle,
    ACCESS_MASK DesiredAccess,
    OBJECT_ATTRIBUTES *ObjectAttributes,
    EVENT_TYPE EventType,
    BOOLEAN InitialState)
{
    ULONG LastError;
    THREAD_DATA *TlsData;

    NTSTATUS status;
    OBJECT_ATTRIBUTES objattrs;
    UNICODE_STRING objname;
    WCHAR *TruePath;
    WCHAR *CopyPath;
    ULONG mp_flags;

    //
    // shortcut processing when object name is not specified
    //

    if ((! ObjectAttributes) || (! ObjectAttributes->ObjectName)) {

        return __sys_NtCreateEvent(
            EventHandle, DesiredAccess, ObjectAttributes,
            EventType, InitialState);
    }

    //
    // normal processing
    //

    TlsData = Dll_GetTlsData(&LastError);

    Dll_PushTlsNameBuffer(TlsData);

    __try {

    //
    // get the full paths for the true and copy objects
    //

        status = Ipc_GetName2(ObjectAttributes, &TruePath, &CopyPath, L"CreateEvent");
    if (! NT_SUCCESS(status))
        __leave;

    if (! TruePath) {

        if(ObjectAttributes->ObjectName->Buffer)
            SbieApi_MonitorPut2(MONITOR_IPC, ObjectAttributes->ObjectName->Buffer, FALSE);

        status = __sys_NtCreateEvent(
            EventHandle, DesiredAccess, ObjectAttributes,
            EventType, InitialState);

        __leave;
    }

    InitializeObjectAttributes(&objattrs,
        &objname, OBJECT_ATTRIBUTES_ATTRIBUTES, NULL, Secure_EveryoneSD);

    //
    // check if this is an open or closed path
    //

    mp_flags = SbieDll_MatchPath2(L'i', TruePath, FALSE, TRUE); // SbieDll_MatchPath(L'i', TruePath);

    if (PATH_IS_CLOSED(mp_flags)) {
        status = STATUS_ACCESS_DENIED;
        __leave;
    }

    if (PATH_IS_OPEN(mp_flags)) {

        RtlInitUnicodeString(&objname, TruePath);
        objattrs.SecurityDescriptor = ObjectAttributes->SecurityDescriptor;

        status = __sys_NtCreateEvent(
            EventHandle, DesiredAccess, &objattrs,
            EventType, InitialState);

        __leave;
    }

    //
    // try to create the object name by its CopyPath, creating the
    // CopyPath hierarchy if needed
    //

    RtlInitUnicodeString(&objname, CopyPath);

    status = __sys_NtCreateEvent(
        EventHandle, DesiredAccess, &objattrs,
        EventType, InitialState);

    if (status == STATUS_OBJECT_PATH_NOT_FOUND) {

        status = Ipc_CreatePath(TruePath, CopyPath);

        if (NT_SUCCESS(status)) {
            status = __sys_NtCreateEvent(
                EventHandle, DesiredAccess, &objattrs,
                EventType, InitialState);
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
// Ipc_NtOpenEvent
//---------------------------------------------------------------------------


_FX NTSTATUS Ipc_NtOpenEvent(
    HANDLE *EventHandle,
    ACCESS_MASK DesiredAccess,
    OBJECT_ATTRIBUTES *ObjectAttributes)
{
    ULONG LastError;
    THREAD_DATA *TlsData = Dll_GetTlsData(&LastError);

    NTSTATUS status;
    OBJECT_ATTRIBUTES objattrs;
    UNICODE_STRING objname;
    WCHAR *TruePath;
    WCHAR *CopyPath;
    ULONG mp_flags;

    Dll_PushTlsNameBuffer(TlsData);

    __try {

    //
    // get the full paths for the true and copy objects
    //

        status = Ipc_GetName2(ObjectAttributes, &TruePath, &CopyPath, L"OpenEvent");
    if (! NT_SUCCESS(status))
        __leave;

    if (! TruePath) {

        status = __sys_NtOpenEvent(
            EventHandle, DesiredAccess, ObjectAttributes);

        __leave;
    }

    InitializeObjectAttributes(
        &objattrs, &objname, OBJECT_ATTRIBUTES_ATTRIBUTES, NULL, NULL);

    //
    // check if this is an open or closed path
    //

    mp_flags = SbieDll_MatchPath(L'i', TruePath);

    if (PATH_IS_CLOSED(mp_flags)) {
        status = STATUS_ACCESS_DENIED;
        __leave;
    }

    if (PATH_IS_OPEN(mp_flags)) goto OpenTruePath;

    //
    // open the object by its CopyPath
    //

    RtlInitUnicodeString(&objname, CopyPath);

    status = __sys_NtOpenEvent(
        EventHandle, DesiredAccess, &objattrs);

    if (status == STATUS_OBJECT_PATH_NOT_FOUND) {

        status = Ipc_CreatePath(TruePath, CopyPath);

        if (NT_SUCCESS(status))
            status = STATUS_OBJECT_NAME_NOT_FOUND;
    }

    __leave;

    //
    // try the TruePath
    //

OpenTruePath:

    RtlInitUnicodeString(&objname, TruePath);

    status = __sys_NtOpenEvent(
        EventHandle, DesiredAccess, &objattrs);

    if (PATH_NOT_OPEN(mp_flags) && (status == STATUS_ACCESS_DENIED))
        status = STATUS_OBJECT_NAME_NOT_FOUND;

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
// Ipc_NtCreateMutant
//---------------------------------------------------------------------------


_FX NTSTATUS Ipc_NtCreateMutant(
    HANDLE *MutantHandle,
    ACCESS_MASK DesiredAccess,
    OBJECT_ATTRIBUTES *ObjectAttributes,
    BOOLEAN InitialOwner)
{
    ULONG LastError;
    THREAD_DATA *TlsData;

    NTSTATUS status;
    OBJECT_ATTRIBUTES objattrs;
    UNICODE_STRING objname;
    WCHAR *TruePath;
    WCHAR *CopyPath;
    ULONG mp_flags;

    //
    // shortcut processing when object name is not specified
    //

    if ((! ObjectAttributes) || (! ObjectAttributes->ObjectName)) {

        return __sys_NtCreateMutant(
            MutantHandle, DesiredAccess, ObjectAttributes,
            InitialOwner);
    }

    //
    // normal processing
    //

    TlsData = Dll_GetTlsData(&LastError);

    Dll_PushTlsNameBuffer(TlsData);

    __try {

    //
    // get the full paths for the true and copy objects
    //

    status = Ipc_GetName2(ObjectAttributes, &TruePath, &CopyPath, L"CreateMutant");
    if (! NT_SUCCESS(status))
        __leave;

    if (! TruePath) {

        if(ObjectAttributes->ObjectName->Buffer)
            SbieApi_MonitorPut2(MONITOR_IPC, ObjectAttributes->ObjectName->Buffer, FALSE);

        status = __sys_NtCreateMutant(
            MutantHandle, DesiredAccess, ObjectAttributes,
            InitialOwner);

        __leave;
    }

    InitializeObjectAttributes(&objattrs,
        &objname, OBJECT_ATTRIBUTES_ATTRIBUTES, NULL, Secure_EveryoneSD);

    //
    // check if this is an open or closed path
    //

    mp_flags = SbieDll_MatchPath2(L'i', TruePath, FALSE, TRUE); // SbieDll_MatchPath(L'i', TruePath);

    if (PATH_IS_CLOSED(mp_flags)) {
        status = STATUS_ACCESS_DENIED;
        __leave;
    }

    if (PATH_IS_OPEN(mp_flags)) {

        RtlInitUnicodeString(&objname, TruePath);
        objattrs.SecurityDescriptor = ObjectAttributes->SecurityDescriptor;

        status = __sys_NtCreateMutant(
            MutantHandle, DesiredAccess, &objattrs,
            InitialOwner);

        __leave;
    }

    //
    // try to create the object name by its CopyPath, creating the
    // CopyPath hierarchy if needed
    //

    RtlInitUnicodeString(&objname, CopyPath);

    status = __sys_NtCreateMutant(
        MutantHandle, DesiredAccess, &objattrs,
        InitialOwner);

    if (status == STATUS_OBJECT_PATH_NOT_FOUND) {

        status = Ipc_CreatePath(TruePath, CopyPath);

        if (NT_SUCCESS(status)) {
            status = __sys_NtCreateMutant(
                MutantHandle, DesiredAccess, &objattrs,
                InitialOwner);
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
// Ipc_NtOpenMutant
//---------------------------------------------------------------------------


_FX NTSTATUS Ipc_NtOpenMutant(
    HANDLE *MutantHandle,
    ACCESS_MASK DesiredAccess,
    OBJECT_ATTRIBUTES *ObjectAttributes)
{
    ULONG LastError;
    THREAD_DATA *TlsData = Dll_GetTlsData(&LastError);

    NTSTATUS status;
    OBJECT_ATTRIBUTES objattrs;
    UNICODE_STRING objname;
    WCHAR *TruePath;
    WCHAR *CopyPath;
    ULONG mp_flags;

    Dll_PushTlsNameBuffer(TlsData);

    __try {

    //
    // get the full paths for the true and copy objects
    //

    status = Ipc_GetName2(ObjectAttributes, &TruePath, &CopyPath, L"OpenMutant");
    if (! NT_SUCCESS(status))
        __leave;

    if (! TruePath) {

        status = __sys_NtOpenMutant(
            MutantHandle, DesiredAccess, ObjectAttributes);

        __leave;
    }

    InitializeObjectAttributes(
        &objattrs, &objname, OBJECT_ATTRIBUTES_ATTRIBUTES, NULL, NULL);

    //
    // check if this is an open or closed path
    //

    mp_flags = SbieDll_MatchPath(L'i', TruePath);

    if (PATH_IS_CLOSED(mp_flags)) {
        status = STATUS_ACCESS_DENIED;
        __leave;
    }

    if (PATH_IS_OPEN(mp_flags)) goto OpenTruePath;

    //
    // otherwise open the object by its CopyPath
    //

    RtlInitUnicodeString(&objname, CopyPath);

    status = __sys_NtOpenMutant(
        MutantHandle, DesiredAccess, &objattrs);

    if (status == STATUS_OBJECT_PATH_NOT_FOUND) {

        status = Ipc_CreatePath(TruePath, CopyPath);

        if (NT_SUCCESS(status))
            status = STATUS_OBJECT_NAME_NOT_FOUND;
    }

    __leave;

    //
    // try the TruePath
    //

OpenTruePath:

    RtlInitUnicodeString(&objname, TruePath);

    status = __sys_NtOpenMutant(
        MutantHandle, DesiredAccess, &objattrs);

    if (PATH_NOT_OPEN(mp_flags) && (status == STATUS_ACCESS_DENIED))
        status = STATUS_OBJECT_NAME_NOT_FOUND;

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
// Ipc_NtCreateSemaphore
//---------------------------------------------------------------------------


_FX NTSTATUS Ipc_NtCreateSemaphore(
    HANDLE *SemaphoreHandle,
    ACCESS_MASK DesiredAccess,
    OBJECT_ATTRIBUTES *ObjectAttributes,
    ULONG InitialCount,
    ULONG MaximumCount)
{
    ULONG LastError;
    THREAD_DATA *TlsData;

    NTSTATUS status;
    OBJECT_ATTRIBUTES objattrs;
    UNICODE_STRING objname;
    WCHAR *TruePath;
    WCHAR *CopyPath;
    ULONG mp_flags;

    //
    // shortcut processing when object name is not specified
    //

    if ((! ObjectAttributes) || (! ObjectAttributes->ObjectName)) {

        return __sys_NtCreateSemaphore(
            SemaphoreHandle, DesiredAccess, ObjectAttributes,
            InitialCount, MaximumCount);
    }

    //
    // normal processing
    //

    TlsData = Dll_GetTlsData(&LastError);

    Dll_PushTlsNameBuffer(TlsData);

    __try {

    //
    // get the full paths for the true and copy objects
    //

        status = Ipc_GetName2(ObjectAttributes, &TruePath, &CopyPath, L"CreateSemaphore");
    if (! NT_SUCCESS(status))
        __leave;

    if (! TruePath) {

        if(ObjectAttributes->ObjectName->Buffer)
            SbieApi_MonitorPut2(MONITOR_IPC, ObjectAttributes->ObjectName->Buffer, FALSE);

        status = __sys_NtCreateSemaphore(
            SemaphoreHandle, DesiredAccess, ObjectAttributes,
            InitialCount, MaximumCount);

        __leave;
    }

    InitializeObjectAttributes(&objattrs,
        &objname, OBJECT_ATTRIBUTES_ATTRIBUTES, NULL, Secure_EveryoneSD);

    //
    // check if this is an open or closed path
    //

    mp_flags = SbieDll_MatchPath2(L'i', TruePath, FALSE, TRUE); // SbieDll_MatchPath(L'i', TruePath);

    if (PATH_IS_CLOSED(mp_flags)) {
        status = STATUS_ACCESS_DENIED;
        __leave;
    }

    if (PATH_IS_OPEN(mp_flags)) {

        RtlInitUnicodeString(&objname, TruePath);
        objattrs.SecurityDescriptor = ObjectAttributes->SecurityDescriptor;

        status = __sys_NtCreateSemaphore(
            SemaphoreHandle, DesiredAccess, &objattrs,
            InitialCount, MaximumCount);

        __leave;
    }

    //
    // try to create the object name by its CopyPath, creating the
    // CopyPath hierarchy if needed
    //

    RtlInitUnicodeString(&objname, CopyPath);

    status = __sys_NtCreateSemaphore(
        SemaphoreHandle, DesiredAccess, &objattrs,
        InitialCount, MaximumCount);

    if (status == STATUS_OBJECT_PATH_NOT_FOUND) {

        status = Ipc_CreatePath(TruePath, CopyPath);

        if (NT_SUCCESS(status)) {
            status = __sys_NtCreateSemaphore(
                SemaphoreHandle, DesiredAccess, &objattrs,
                InitialCount, MaximumCount);
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
// Ipc_NtOpenSemaphore
//---------------------------------------------------------------------------


_FX NTSTATUS Ipc_NtOpenSemaphore(
    HANDLE *SemaphoreHandle,
    ACCESS_MASK DesiredAccess,
    OBJECT_ATTRIBUTES *ObjectAttributes)
{
    ULONG LastError;
    THREAD_DATA *TlsData = Dll_GetTlsData(&LastError);

    NTSTATUS status;
    OBJECT_ATTRIBUTES objattrs;
    UNICODE_STRING objname;
    WCHAR *TruePath;
    WCHAR *CopyPath;
    ULONG mp_flags;

    Dll_PushTlsNameBuffer(TlsData);

    __try {

    //
    // get the full paths for the true and copy objects
    //

    status = Ipc_GetName2(ObjectAttributes, &TruePath, &CopyPath, L"OpenSemaphore");
    if (! NT_SUCCESS(status))
        __leave;

    if (! TruePath) {

        status = __sys_NtOpenSemaphore(
            SemaphoreHandle, DesiredAccess, ObjectAttributes);

        __leave;
    }

    InitializeObjectAttributes(
        &objattrs, &objname, OBJECT_ATTRIBUTES_ATTRIBUTES, NULL, NULL);

    //
    // check if this is an open or closed path
    //

    mp_flags = SbieDll_MatchPath(L'i', TruePath);

    if (PATH_IS_CLOSED(mp_flags)) {
        status = STATUS_ACCESS_DENIED;
        __leave;
    }

    if (PATH_IS_OPEN(mp_flags)) goto OpenTruePath;

    //
    // otherwise open the object by its CopyPath
    //

    RtlInitUnicodeString(&objname, CopyPath);

    status = __sys_NtOpenSemaphore(
        SemaphoreHandle, DesiredAccess, &objattrs);

    if (status == STATUS_OBJECT_PATH_NOT_FOUND) {

        status = Ipc_CreatePath(TruePath, CopyPath);

        if (NT_SUCCESS(status))
            status = STATUS_OBJECT_NAME_NOT_FOUND;
    }

    __leave;

    //
    // try the TruePath
    //

OpenTruePath:

    RtlInitUnicodeString(&objname, TruePath);

    status = __sys_NtOpenSemaphore(
        SemaphoreHandle, DesiredAccess, &objattrs);

    if (PATH_NOT_OPEN(mp_flags) && (status == STATUS_ACCESS_DENIED))
        status = STATUS_OBJECT_NAME_NOT_FOUND;

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
// Ipc_NtCreateSection
//---------------------------------------------------------------------------


_FX NTSTATUS Ipc_NtCreateSection(
    HANDLE *SectionHandle,
    ACCESS_MASK DesiredAccess,
    OBJECT_ATTRIBUTES *ObjectAttributes,
    LARGE_INTEGER *MaximumSize,
    ULONG PageAttributes,
    ULONG SectionAttributes,
    HANDLE FileHandle)
{
    ULONG LastError;
    THREAD_DATA *TlsData;

    NTSTATUS status;
    OBJECT_ATTRIBUTES objattrs;
    UNICODE_STRING objname;
    WCHAR *TruePath;
    WCHAR *CopyPath;
    ULONG mp_flags;

    //
    // shortcut processing when object name is not specified
    //

    if ((! ObjectAttributes) || (! ObjectAttributes->ObjectName)) {

        if ((! MaximumSize) && (PageAttributes == PAGE_EXECUTE)
                            && (SectionAttributes == SEC_IMAGE)) {
            //
            // allow Proc_SectionCallback to record the image name
            //
            status = Proc_SectionCallback(FileHandle);
            if (! NT_SUCCESS(status))
                return status;
        }

        return __sys_NtCreateSection(
            SectionHandle, DesiredAccess, ObjectAttributes,
            MaximumSize, PageAttributes, SectionAttributes, FileHandle);
    }

    //
    // normal processing
    //

    TlsData = Dll_GetTlsData(&LastError);

    Dll_PushTlsNameBuffer(TlsData);

    __try {

    //
    // get the full paths for the true and copy objects
    //

    status = Ipc_GetName2(ObjectAttributes, &TruePath, &CopyPath, L"CreateSection");
    if (! NT_SUCCESS(status))
        __leave;

    if (! TruePath) {

        if(ObjectAttributes->ObjectName->Buffer)
            SbieApi_MonitorPut2(MONITOR_IPC, ObjectAttributes->ObjectName->Buffer, FALSE);

        status = __sys_NtCreateSection(
            SectionHandle, DesiredAccess, ObjectAttributes,
            MaximumSize, PageAttributes, SectionAttributes, FileHandle);

        __leave;
    }

    InitializeObjectAttributes(&objattrs,
        &objname, OBJECT_ATTRIBUTES_ATTRIBUTES, NULL, Secure_EveryoneSD);

    //
    // check if this is an open or closed path
    //

    mp_flags = SbieDll_MatchPath2(L'i', TruePath, FALSE, TRUE); // SbieDll_MatchPath(L'i', TruePath);

    if (PATH_IS_CLOSED(mp_flags)) {
        status = STATUS_ACCESS_DENIED;
        __leave;
    }

    if (PATH_IS_OPEN(mp_flags)) {

        RtlInitUnicodeString(&objname, TruePath);
        objattrs.SecurityDescriptor = ObjectAttributes->SecurityDescriptor;

        status = __sys_NtCreateSection(
            SectionHandle, DesiredAccess, &objattrs,
            MaximumSize, PageAttributes, SectionAttributes, FileHandle);

        __leave;
    }

    //
    // try to create the object name by its CopyPath, creating the
    // CopyPath hierarchy if needed
    //

    RtlInitUnicodeString(&objname, CopyPath);

    status = __sys_NtCreateSection(
        SectionHandle, DesiredAccess, &objattrs,
        MaximumSize, PageAttributes, SectionAttributes, FileHandle);

    if (status == STATUS_OBJECT_PATH_NOT_FOUND) {

        status = Ipc_CreatePath(TruePath, CopyPath);

        if (NT_SUCCESS(status)) {
            status = __sys_NtCreateSection(
                SectionHandle, DesiredAccess, &objattrs,
                MaximumSize, PageAttributes, SectionAttributes, FileHandle);
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
// Ipc_NtCreateSectionEx
//---------------------------------------------------------------------------


_FX NTSTATUS Ipc_NtCreateSectionEx(
    HANDLE *SectionHandle,
    ACCESS_MASK DesiredAccess,
    OBJECT_ATTRIBUTES *ObjectAttributes,
    LARGE_INTEGER *MaximumSize,
    ULONG PageAttributes,
    ULONG SectionAttributes,
    HANDLE FileHandle,
    PMEM_EXTENDED_PARAMETER ExtendedParameters,
    ULONG ExtendedParameterCount)
{
    ULONG LastError;
    THREAD_DATA *TlsData;

    NTSTATUS status;
    OBJECT_ATTRIBUTES objattrs;
    UNICODE_STRING objname;
    WCHAR *TruePath;
    WCHAR *CopyPath;
    ULONG mp_flags;

    //
    // shortcut processing when object name is not specified
    //

    if ((! ObjectAttributes) || (! ObjectAttributes->ObjectName)) {

        if ((! MaximumSize) && (PageAttributes == PAGE_EXECUTE)
                            && (SectionAttributes == SEC_IMAGE)) {
            //
            // allow Proc_SectionCallback to record the image name
            //
            status = Proc_SectionCallback(FileHandle);
            if (! NT_SUCCESS(status))
                return status;
        }

        return __sys_NtCreateSectionEx(
            SectionHandle, DesiredAccess, ObjectAttributes,
            MaximumSize, PageAttributes, SectionAttributes, FileHandle, ExtendedParameters, ExtendedParameterCount);
    }

    //
    // normal processing
    //

    TlsData = Dll_GetTlsData(&LastError);

    Dll_PushTlsNameBuffer(TlsData);

    __try {

    //
    // get the full paths for the true and copy objects
    //

    status = Ipc_GetName2(ObjectAttributes, &TruePath, &CopyPath, L"CreateSectionEx");
    if (! NT_SUCCESS(status))
        __leave;

    if (! TruePath) {

        if(ObjectAttributes->ObjectName->Buffer)
            SbieApi_MonitorPut2(MONITOR_IPC, ObjectAttributes->ObjectName->Buffer, FALSE);

        status = __sys_NtCreateSectionEx(
            SectionHandle, DesiredAccess, ObjectAttributes,
            MaximumSize, PageAttributes, SectionAttributes, FileHandle, ExtendedParameters, ExtendedParameterCount);

        __leave;
    }

    InitializeObjectAttributes(&objattrs,
        &objname, OBJECT_ATTRIBUTES_ATTRIBUTES, NULL, Secure_EveryoneSD);

    //
    // check if this is an open or closed path
    //

    mp_flags = SbieDll_MatchPath2(L'i', TruePath, FALSE, TRUE); // SbieDll_MatchPath(L'i', TruePath);

    if (PATH_IS_CLOSED(mp_flags)) {
        status = STATUS_ACCESS_DENIED;
        __leave;
    }

    if (PATH_IS_OPEN(mp_flags)) {

        RtlInitUnicodeString(&objname, TruePath);
        objattrs.SecurityDescriptor = ObjectAttributes->SecurityDescriptor;

        status = __sys_NtCreateSectionEx(
            SectionHandle, DesiredAccess, &objattrs,
            MaximumSize, PageAttributes, SectionAttributes, FileHandle, ExtendedParameters, ExtendedParameterCount);

        __leave;
    }

    //
    // try to create the object name by its CopyPath, creating the
    // CopyPath hierarchy if needed
    //

    RtlInitUnicodeString(&objname, CopyPath);

    status = __sys_NtCreateSectionEx(
        SectionHandle, DesiredAccess, &objattrs,
        MaximumSize, PageAttributes, SectionAttributes, FileHandle, ExtendedParameters, ExtendedParameterCount);

    if (status == STATUS_OBJECT_PATH_NOT_FOUND) {

        status = Ipc_CreatePath(TruePath, CopyPath);

        if (NT_SUCCESS(status)) {
            status = __sys_NtCreateSectionEx(
                SectionHandle, DesiredAccess, &objattrs,
                MaximumSize, PageAttributes, SectionAttributes, FileHandle, ExtendedParameters, ExtendedParameterCount);
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
// Ipc_NtOpenSection
//---------------------------------------------------------------------------


_FX NTSTATUS Ipc_NtOpenSection(
    HANDLE *SectionHandle,
    ACCESS_MASK DesiredAccess,
    OBJECT_ATTRIBUTES *ObjectAttributes)
{
    ULONG LastError;
    THREAD_DATA *TlsData = Dll_GetTlsData(&LastError);

    NTSTATUS status;
    OBJECT_ATTRIBUTES objattrs;
    UNICODE_STRING objname;
    WCHAR *TruePath;
    WCHAR *CopyPath;
    ULONG mp_flags;

    Dll_PushTlsNameBuffer(TlsData);

    __try {

    //
    // get the full paths for the true and copy objects
    //

    status = Ipc_GetName2(ObjectAttributes, &TruePath, &CopyPath, L"OpenSection");
    if (! NT_SUCCESS(status))
        __leave;

    if (! TruePath) {

        status = __sys_NtOpenSection(
            SectionHandle, DesiredAccess, ObjectAttributes);

        __leave;
    }

    InitializeObjectAttributes(
        &objattrs, &objname, OBJECT_ATTRIBUTES_ATTRIBUTES, NULL, NULL);

    //
    // check if this is an open or closed path
    //

    mp_flags = SbieDll_MatchPath(L'i', TruePath);

    if (PATH_IS_CLOSED(mp_flags)) {
        status = STATUS_ACCESS_DENIED;
        __leave;
    }

    if (PATH_IS_OPEN(mp_flags)) goto OpenTruePath;

    //
    // otherwise open the object by its CopyPath
    //

    RtlInitUnicodeString(&objname, CopyPath);

    status = __sys_NtOpenSection(
        SectionHandle, DesiredAccess, &objattrs);

    if (status == STATUS_OBJECT_PATH_NOT_FOUND) {

        status = Ipc_CreatePath(TruePath, CopyPath);

        if (NT_SUCCESS(status))
            status = STATUS_OBJECT_NAME_NOT_FOUND;
    }

    __leave;

    //
    // try the TruePath
    //

OpenTruePath:

    if (Ipc_IsKnownDllInSandbox(TlsData, TruePath)) {

        status = STATUS_OBJECT_NAME_NOT_FOUND;
        __leave;
    }

    RtlInitUnicodeString(&objname, TruePath);

    status = __sys_NtOpenSection(
        SectionHandle, DesiredAccess, &objattrs);

    if (PATH_NOT_OPEN(mp_flags) && (status == STATUS_ACCESS_DENIED))
        status = STATUS_OBJECT_NAME_NOT_FOUND;

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
// Ipc_NtCreateSymbolicLinkObject
//---------------------------------------------------------------------------


_FX NTSTATUS Ipc_NtCreateSymbolicLinkObject(
    PHANDLE SymbolicLinkHandle, ACCESS_MASK DesiredAccess,
    POBJECT_ATTRIBUTES ObjectAttributes, PUNICODE_STRING DestinationName)
{
    ULONG LastError;
    THREAD_DATA *TlsData;

    NTSTATUS status;
    OBJECT_ATTRIBUTES objattrs;
    UNICODE_STRING objname;
    WCHAR *TruePath;
    WCHAR *CopyPath;
    ULONG mp_flags;

    //
    // shortcut processing when object name is not specified
    //

    if ((! ObjectAttributes) || (! ObjectAttributes->ObjectName)) {

        return __sys_NtCreateSymbolicLinkObject(
            SymbolicLinkHandle, DesiredAccess, ObjectAttributes,
            DestinationName);
    }

    //
    // normal processing
    //

    TlsData = Dll_GetTlsData(&LastError);

    Dll_PushTlsNameBuffer(TlsData);

    __try {

    //
    // get the full paths for the true and copy objects
    //

    status = Ipc_GetName2(ObjectAttributes, &TruePath, &CopyPath, L"CreateSymbolicLinkObject");
    if (! NT_SUCCESS(status))
        __leave;

    if (! TruePath) {

        if(ObjectAttributes->ObjectName->Buffer)
            SbieApi_MonitorPut2(MONITOR_IPC, ObjectAttributes->ObjectName->Buffer, FALSE);

        status = __sys_NtCreateSymbolicLinkObject(
            SymbolicLinkHandle, DesiredAccess, ObjectAttributes,
            DestinationName);

        __leave;
    }

    InitializeObjectAttributes(&objattrs,
        &objname, OBJECT_ATTRIBUTES_ATTRIBUTES, NULL, Secure_EveryoneSD);

    //
    // check if this is an open or closed path
    //

    mp_flags = SbieDll_MatchPath2(L'i', TruePath, FALSE, TRUE); // SbieDll_MatchPath(L'i', TruePath);

    if (PATH_IS_CLOSED(mp_flags)) {
        status = STATUS_ACCESS_DENIED;
        __leave;
    }

    if (PATH_IS_OPEN(mp_flags)) {

        RtlInitUnicodeString(&objname, TruePath);
        objattrs.SecurityDescriptor = ObjectAttributes->SecurityDescriptor;

        status = __sys_NtCreateSymbolicLinkObject(
            SymbolicLinkHandle, DesiredAccess, &objattrs,
            DestinationName);

        __leave;
    }

    //
    // try to create the object name by its CopyPath, creating the
    // CopyPath hierarchy if needed
    //

    RtlInitUnicodeString(&objname, CopyPath);

    status = __sys_NtCreateSymbolicLinkObject(
        SymbolicLinkHandle, DesiredAccess, &objattrs,
        DestinationName);

    if (status == STATUS_OBJECT_PATH_NOT_FOUND) {

        status = Ipc_CreatePath(TruePath, CopyPath);

        if (NT_SUCCESS(status)) {
            status = __sys_NtCreateSymbolicLinkObject(
                SymbolicLinkHandle, DesiredAccess, &objattrs,
                DestinationName);
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
// Ipc_NtOpenSymbolicLinkObject
//---------------------------------------------------------------------------


_FX NTSTATUS Ipc_NtOpenSymbolicLinkObject(
    HANDLE *SymbolicLinkHandle,
    ACCESS_MASK DesiredAccess,
    OBJECT_ATTRIBUTES *ObjectAttributes)
{
    ULONG LastError;
    THREAD_DATA *TlsData = Dll_GetTlsData(&LastError);

    NTSTATUS status;
    OBJECT_ATTRIBUTES objattrs;
    UNICODE_STRING objname;
    WCHAR *TruePath;
    WCHAR *CopyPath;
    ULONG mp_flags;

    Dll_PushTlsNameBuffer(TlsData);

    __try {

    //
    // get the full paths for the true and copy objects
    //

    status = Ipc_GetName2(ObjectAttributes, &TruePath, &CopyPath, L"OpenSymbolicLinkObject");
    if (! NT_SUCCESS(status))
        __leave;

    if (! TruePath) {

        status = __sys_NtOpenSymbolicLinkObject(
            SymbolicLinkHandle, DesiredAccess, ObjectAttributes);

        __leave;
    }

    InitializeObjectAttributes(
        &objattrs, &objname, OBJECT_ATTRIBUTES_ATTRIBUTES, NULL, NULL);

    //
    // check if this is an open or closed path
    //

    mp_flags = SbieDll_MatchPath(L'i', TruePath);

    if (PATH_IS_CLOSED(mp_flags)) {
        status = STATUS_ACCESS_DENIED;
        __leave;
    }

    if (PATH_IS_OPEN(mp_flags)) goto OpenTruePath;

    //
    // open the object by its CopyPath first
    // finish on success, else try true path
    //

    RtlInitUnicodeString(&objname, CopyPath);

    status = __sys_NtOpenSymbolicLinkObject(
        SymbolicLinkHandle, DesiredAccess, &objattrs);

    if(NT_SUCCESS(status))
        __leave;

    if (status == STATUS_OBJECT_PATH_NOT_FOUND || status == STATUS_OBJECT_NAME_NOT_FOUND) {
    
        ACCESS_MASK PermissibleAccess = READ_CONTROL | SYMBOLIC_LINK_QUERY;
        if (DesiredAccess == MAXIMUM_ALLOWED)
            DesiredAccess = PermissibleAccess;
        else
            DesiredAccess &= PermissibleAccess;

        goto OpenTruePath;
    }
    
    __leave;

    //
    // try the TruePath
    //

OpenTruePath:

    RtlInitUnicodeString(&objname, TruePath);

    status = __sys_NtOpenSymbolicLinkObject(
        SymbolicLinkHandle, DesiredAccess, &objattrs);

    if (PATH_NOT_OPEN(mp_flags) && (status == STATUS_ACCESS_DENIED))
        status = STATUS_OBJECT_NAME_NOT_FOUND;

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
// Ipc_NtCreateDirectoryObject
//---------------------------------------------------------------------------


_FX NTSTATUS Ipc_NtCreateDirectoryObject(
    PHANDLE DirectoryHandle,
    ACCESS_MASK DesiredAccess,
    POBJECT_ATTRIBUTES ObjectAttributes)
{
    ULONG LastError;
    THREAD_DATA *TlsData;

    NTSTATUS status;
    OBJECT_ATTRIBUTES objattrs;
    UNICODE_STRING objname;
    WCHAR *TruePath;
    WCHAR *CopyPath;
    ULONG mp_flags;

    //
    // shortcut processing when object name is not specified
    //

    if ((! ObjectAttributes) || (! ObjectAttributes->ObjectName)) {

        return __sys_NtCreateDirectoryObject(DirectoryHandle, DesiredAccess, ObjectAttributes);
    }

    //
    // normal processing
    //

    TlsData = Dll_GetTlsData(&LastError);

    Dll_PushTlsNameBuffer(TlsData);

    __try {

    //
    // get the full paths for the true and copy objects
    //

    status = Ipc_GetName2(ObjectAttributes, &TruePath, &CopyPath, L"CreateDirectoryObject");
    if (! NT_SUCCESS(status))
        __leave;

    if (! TruePath) {

        if(ObjectAttributes->ObjectName->Buffer)
            SbieApi_MonitorPut2(MONITOR_IPC, ObjectAttributes->ObjectName->Buffer, FALSE);

        status = __sys_NtCreateDirectoryObject(DirectoryHandle, DesiredAccess, ObjectAttributes);

        __leave;
    }

    InitializeObjectAttributes(&objattrs,
        &objname, OBJECT_ATTRIBUTES_ATTRIBUTES, NULL, Secure_EveryoneSD);

    //
    // check if this is an open or closed path
    //

    mp_flags = SbieDll_MatchPath2(L'i', TruePath, FALSE, TRUE); // SbieDll_MatchPath(L'i', TruePath);

    if (PATH_IS_CLOSED(mp_flags)) {
        status = STATUS_ACCESS_DENIED;
        __leave;
    }

    if (PATH_IS_OPEN(mp_flags)) {

        RtlInitUnicodeString(&objname, TruePath);
        objattrs.SecurityDescriptor = ObjectAttributes->SecurityDescriptor;

        status = __sys_NtCreateDirectoryObject(DirectoryHandle, DesiredAccess, &objattrs);

        __leave;
    }

    //
    // try to create the object name by its CopyPath, creating the
    // CopyPath hierarchy if needed
    //

    RtlInitUnicodeString(&objname, CopyPath);

    status = __sys_NtCreateDirectoryObject(DirectoryHandle, DesiredAccess, &objattrs);

    if (status == STATUS_OBJECT_PATH_NOT_FOUND) {

        status = Ipc_CreatePath(TruePath, CopyPath);

        if (NT_SUCCESS(status)) {
            status = __sys_NtCreateDirectoryObject(DirectoryHandle, DesiredAccess, &objattrs);
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
// Ipc_NtCreateDirectoryObjectEx
//---------------------------------------------------------------------------


_FX NTSTATUS Ipc_NtCreateDirectoryObjectEx(
    PHANDLE DirectoryHandle,
    ACCESS_MASK DesiredAccess,
    POBJECT_ATTRIBUTES ObjectAttributes,
    HANDLE ShadowDirectoryHandle,
    ULONG Flags)
{
    ULONG LastError;
    THREAD_DATA *TlsData;

    NTSTATUS status;
    OBJECT_ATTRIBUTES objattrs;
    UNICODE_STRING objname;
    WCHAR *TruePath;
    WCHAR *CopyPath;
    ULONG mp_flags;

    //
    // shortcut processing when object name is not specified
    //

    if ((! ObjectAttributes) || (! ObjectAttributes->ObjectName)) {

        return __sys_NtCreateDirectoryObjectEx(DirectoryHandle, DesiredAccess, ObjectAttributes
                                                , ShadowDirectoryHandle, Flags);
    }

    //
    // normal processing
    //

    TlsData = Dll_GetTlsData(&LastError);

    Dll_PushTlsNameBuffer(TlsData);

    __try {

    //
    // get the full paths for the true and copy objects
    //

    status = Ipc_GetName2(ObjectAttributes, &TruePath, &CopyPath, L"CreateDirectoryObjectEx");
    if (! NT_SUCCESS(status))
        __leave;

    if (! TruePath) {

        if(ObjectAttributes->ObjectName->Buffer)
            SbieApi_MonitorPut2(MONITOR_IPC, ObjectAttributes->ObjectName->Buffer, FALSE);

        status = __sys_NtCreateDirectoryObjectEx(DirectoryHandle, DesiredAccess, ObjectAttributes
                                                    , ShadowDirectoryHandle, Flags);

        __leave;
    }

    InitializeObjectAttributes(&objattrs,
        &objname, OBJECT_ATTRIBUTES_ATTRIBUTES, NULL, Secure_EveryoneSD);

    //
    // check if this is an open or closed path
    //

    mp_flags = SbieDll_MatchPath2(L'i', TruePath, FALSE, TRUE); // SbieDll_MatchPath(L'i', TruePath);

    if (PATH_IS_CLOSED(mp_flags)) {
        status = STATUS_ACCESS_DENIED;
        __leave;
    }

    if (PATH_IS_OPEN(mp_flags)) {

        RtlInitUnicodeString(&objname, TruePath);
        objattrs.SecurityDescriptor = ObjectAttributes->SecurityDescriptor;

        status = __sys_NtCreateDirectoryObjectEx(DirectoryHandle, DesiredAccess, &objattrs
                                                    , ShadowDirectoryHandle, Flags);

        __leave;
    }

    //
    // try to create the object name by its CopyPath, creating the
    // CopyPath hierarchy if needed
    //

    RtlInitUnicodeString(&objname, CopyPath);

    status = __sys_NtCreateDirectoryObjectEx(DirectoryHandle, DesiredAccess, &objattrs
                                                , ShadowDirectoryHandle, Flags);

    if (status == STATUS_OBJECT_PATH_NOT_FOUND) {

        status = Ipc_CreatePath(TruePath, CopyPath);

        if (NT_SUCCESS(status)) {
            status = __sys_NtCreateDirectoryObjectEx(DirectoryHandle, DesiredAccess, &objattrs
                                                        , ShadowDirectoryHandle, Flags);
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
// Ipc_NtOpenDirectoryObject
//---------------------------------------------------------------------------


_FX NTSTATUS Ipc_NtOpenDirectoryObject(
    PHANDLE DirectoryHandle,
    ACCESS_MASK DesiredAccess,
    POBJECT_ATTRIBUTES ObjectAttributes)
{
    ULONG LastError;
    THREAD_DATA *TlsData = Dll_GetTlsData(&LastError);

    NTSTATUS status;
    OBJECT_ATTRIBUTES objattrs;
    UNICODE_STRING objname;
    WCHAR *TruePath;
    WCHAR *CopyPath;
    ULONG mp_flags;

    Dll_PushTlsNameBuffer(TlsData);

    __try {

    //
    // get the full paths for the true and copy objects
    //

    status = Ipc_GetName2(ObjectAttributes, &TruePath, &CopyPath, L"OpenDirectoryObject");
    if (! NT_SUCCESS(status))
        __leave;

    if (! TruePath) {

        status = __sys_NtOpenDirectoryObject(DirectoryHandle, DesiredAccess, ObjectAttributes);

        __leave;
    }

    InitializeObjectAttributes(
        &objattrs, &objname, OBJECT_ATTRIBUTES_ATTRIBUTES, NULL, NULL);

    //
    // check if this is an open or closed path
    //

    mp_flags = SbieDll_MatchPath(L'i', TruePath);

    if (PATH_IS_CLOSED(mp_flags)) {
        status = STATUS_ACCESS_DENIED;
        __leave;
    }

    if (PATH_IS_OPEN(mp_flags)) goto OpenTruePath;

    //
    // otherwise open the object by its CopyPath
    //

    RtlInitUnicodeString(&objname, CopyPath);

    status = __sys_NtOpenDirectoryObject(DirectoryHandle, DesiredAccess, &objattrs);

    if (status == STATUS_OBJECT_PATH_NOT_FOUND || status == STATUS_OBJECT_NAME_NOT_FOUND || status == STATUS_OBJECT_NAME_INVALID) {

        //
        // if the directory does not exist in the sandbox try opening the original one
        // for this we strip the create access if we later try to create an object
        // a sandboxed Directory will be created and the object created within it
        //

        if (ipc_namespace_isoaltion) {
            ACCESS_MASK PermissibleAccess = READ_CONTROL | DIRECTORY_QUERY | DIRECTORY_TRAVERSE;
            if (DesiredAccess == MAXIMUM_ALLOWED)
                DesiredAccess = PermissibleAccess;
            else
                DesiredAccess &= PermissibleAccess;
        }

        goto OpenTruePath;
    }

    __leave;

    //
    // try the TruePath
    //

OpenTruePath:

    RtlInitUnicodeString(&objname, TruePath);

    status = __sys_NtOpenDirectoryObject(DirectoryHandle, DesiredAccess, &objattrs);

    if (PATH_NOT_OPEN(mp_flags) && (status == STATUS_ACCESS_DENIED))
        status = STATUS_OBJECT_NAME_NOT_FOUND;

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
// Ipc_MergeFree
//---------------------------------------------------------------------------


_FX void Ipc_MergeFree(IPC_MERGE *merge)
{
    while (1) {
        IPC_MERGE_ENTRY *entry = List_Head(&merge->objects);
        if (! entry)
            break;
        List_Remove(&merge->objects, entry);
        Dll_Free(entry);
    }

    Dll_Free(merge);
}


//---------------------------------------------------------------------------
// Ipc_NtClose
//---------------------------------------------------------------------------


_FX void Ipc_NtClose(HANDLE IpcHandle, void* CloseParams)
{
    IPC_MERGE *merge;

    EnterCriticalSection(&Ipc_Handles_CritSec);

    merge = List_Head(&Ipc_Handles);
    while (merge) {
        if (merge->handle == IpcHandle) {

            Handle_UnRegisterHandler(merge->handle, Ipc_NtClose, NULL);
            List_Remove(&Ipc_Handles, merge);
            Ipc_MergeFree(merge);

            break;
        }
        merge = List_Next(merge);
    }

    LeaveCriticalSection(&Ipc_Handles_CritSec);
}


//---------------------------------------------------------------------------
// Ipc_MergeDirectoryObject
//---------------------------------------------------------------------------


_FX NTSTATUS Ipc_MergeDirectoryObject(IPC_MERGE *merge, WCHAR* path, BOOLEAN join)
{
    NTSTATUS status;
    HANDLE directoryHandle;
    OBJECT_ATTRIBUTES objattrs;
    UNICODE_STRING objname;

    RtlInitUnicodeString(&objname, path);

    InitializeObjectAttributes(
        &objattrs, &objname, OBJ_CASE_INSENSITIVE, NULL, NULL);

    status = __sys_NtOpenDirectoryObject(&directoryHandle, DIRECTORY_QUERY, &objattrs);
    if (!NT_SUCCESS(status))
        return status;

    ULONG bufferSize = 4096;
    PVOID buffer = Dll_Alloc(bufferSize);
    BOOLEAN firstTime = TRUE;
    ULONG indexCounter = 0;
    ULONG returnLength;

    while (1)
    {
        status = __sys_NtQueryDirectoryObject(directoryHandle, buffer, bufferSize, FALSE, firstTime, &indexCounter, &returnLength);
        firstTime = FALSE;

        if (status == STATUS_NO_MORE_ENTRIES)
            break; // done
        if (!NT_SUCCESS(status))
            break; // error

        for (POBJECT_DIRECTORY_INFORMATION directoryInfo = buffer; directoryInfo->Name.Length != 0; directoryInfo++)
        {
            ULONG len = sizeof(IPC_MERGE_ENTRY) + (directoryInfo->Name.MaximumLength + directoryInfo->TypeName.MaximumLength) * sizeof(WCHAR);

            //
            // when we are joining we remove the older entries when a duplicate is encountered
            //

            if (join) {

                IPC_MERGE_ENTRY* entry = List_Head(&merge->objects);
                while (entry) {

                    if (entry->Name.Length == directoryInfo->Name.Length && memcmp(entry->Name.Buffer, directoryInfo->Name.Buffer, entry->Name.Length) == 0)
                        break;

                    entry = List_Next(entry);
                }

                if (entry) {

                    if (entry->TypeName.Length == directoryInfo->TypeName.Length && memcmp(entry->TypeName.Buffer, directoryInfo->TypeName.Buffer, entry->TypeName.Length) == 0)
                        continue; // identical entry, nothing to do

                    // same name but different type, remove old entry
                    List_Remove(&merge->objects, entry);
                    Dll_Free(entry);
                }
            }

            //
            // add new entry
            //

            IPC_MERGE_ENTRY* entry = Dll_Alloc(len);
            WCHAR* ptr = entry + 1;

            entry->Name.Length = directoryInfo->Name.Length;
            entry->Name.MaximumLength = directoryInfo->Name.MaximumLength;
            entry->Name.Buffer = ptr;
            memcpy(ptr, directoryInfo->Name.Buffer, directoryInfo->Name.MaximumLength);
            ptr += directoryInfo->Name.MaximumLength / sizeof(WCHAR);

            entry->TypeName.Length = directoryInfo->TypeName.Length;
            entry->TypeName.MaximumLength = directoryInfo->TypeName.MaximumLength;
            entry->TypeName.Buffer = ptr;
            memcpy(ptr, directoryInfo->TypeName.Buffer, directoryInfo->TypeName.MaximumLength);
            //ptr += directoryInfo->TypeName.MaximumLength / sizeof(WCHAR);

            List_Insert_After(&merge->objects, NULL, entry);
        }
    }

    Dll_Free(buffer);

    extern P_NtClose __sys_NtClose;
    __sys_NtClose(directoryHandle);

    return status;
}


//---------------------------------------------------------------------------
// Ipc_NtQueryDirectoryObject
//---------------------------------------------------------------------------


_FX NTSTATUS Ipc_NtQueryDirectoryObject(
    HANDLE DirectoryHandle,
    PVOID Buffer,
    ULONG Length,
    BOOLEAN ReturnSingleEntry,
    BOOLEAN RestartScan,
    PULONG Context,
    PULONG ReturnLength)
{
    IPC_MERGE *merge;

    EnterCriticalSection(&Ipc_Handles_CritSec);

    merge = List_Head(&Ipc_Handles);
    while (merge) {

        IPC_MERGE *next = List_Next(merge);

        if (merge->handle == DirectoryHandle)
            break;

        merge = next;
    }

    if (RestartScan && merge != NULL) {

        Handle_UnRegisterHandler(merge->handle, Ipc_NtClose, NULL);
        List_Remove(&Ipc_Handles, merge);
        Ipc_MergeFree(merge);

        merge = NULL;
    }

    if (! merge) {

        merge = Dll_Alloc(sizeof(IPC_MERGE));
        memzero(merge, sizeof(IPC_MERGE));

        merge->handle = DirectoryHandle;

        List_Insert_Before(&Ipc_Handles, NULL, merge);
        Handle_RegisterHandler(merge->handle, Ipc_NtClose, NULL, FALSE);

        WCHAR *TruePath;
        WCHAR *CopyPath;
        NTSTATUS status = Ipc_GetName(DirectoryHandle, (UNICODE_STRING*)-1, &TruePath, &CopyPath, NULL);

        if (!NT_SUCCESS(status))
            return status;

        Ipc_MergeDirectoryObject(merge, TruePath, FALSE);

        ULONG len = wcslen(CopyPath); // fix root copy path, remove trailing '\\'
        if (CopyPath[len - 1] == L'\\') CopyPath[len - 1] = 0;

        Ipc_MergeDirectoryObject(merge, CopyPath, TRUE);
    }

    //
    // goto index, for better performacne we could cache indexes
    //

    IPC_MERGE_ENTRY* entry = List_Head(&merge->objects);

    ULONG indexCounter = 0;
    if (Context) {
        for (; entry && indexCounter < *Context; indexCounter++)
            entry = List_Next(entry);
    }
    if (!entry)
        return STATUS_NO_MORE_ENTRIES;

    //
    // count the buffer space
    //

    ULONG CountToGo = 0;
    ULONG TotalLength = sizeof(OBJECT_DIRECTORY_INFORMATION);
    for (IPC_MERGE_ENTRY* cur = entry; cur; cur = List_Next(cur)) {

        ULONG len = sizeof(OBJECT_DIRECTORY_INFORMATION) + (cur->Name.MaximumLength + cur->TypeName.MaximumLength) * sizeof(WCHAR);

        if (TotalLength + len > Length)
            break; // not enough space for this entry

        CountToGo++;
        TotalLength += len;

        if (ReturnSingleEntry)
            break;
    }

    //
    // fill output buffer
    //

    POBJECT_DIRECTORY_INFORMATION directoryInfo = Buffer;
    WCHAR* ptr = directoryInfo + CountToGo + 1;

    ULONG EndIndex = indexCounter + CountToGo;
    for (; entry && indexCounter < EndIndex; indexCounter++) {

        if (directoryInfo) {

            directoryInfo->Name.Length = entry->Name.Length;
            directoryInfo->Name.MaximumLength = entry->Name.MaximumLength;
            directoryInfo->Name.Buffer = ptr;
            memcpy(ptr, entry->Name.Buffer, entry->Name.MaximumLength);
            ptr += directoryInfo->Name.MaximumLength / sizeof(WCHAR);

            directoryInfo->TypeName.Length = entry->TypeName.Length;
            directoryInfo->TypeName.MaximumLength = entry->TypeName.MaximumLength;
            directoryInfo->TypeName.Buffer = ptr;
            memcpy(ptr, entry->TypeName.Buffer, entry->TypeName.MaximumLength);
            ptr += directoryInfo->TypeName.MaximumLength / sizeof(WCHAR);

            directoryInfo++;
        }

        entry = List_Next(entry);
    }

    //
    // terminate listing with an empty entry
    //

    if (directoryInfo) {

        directoryInfo->Name.Length = directoryInfo->TypeName.Length = 0;
        directoryInfo->Name.MaximumLength = directoryInfo->TypeName.MaximumLength = 0;
        directoryInfo->Name.Buffer = directoryInfo->TypeName.Buffer = NULL;
    }

    //
    // set return values
    //

    if (ReturnLength) *ReturnLength = TotalLength;
    if (Context) *Context = indexCounter;
    if (indexCounter < (ULONG)merge->objects.count)
        return STATUS_MORE_ENTRIES;
    return STATUS_SUCCESS;
}


//---------------------------------------------------------------------------
// Ipc_IsKnownDllInSandbox
//---------------------------------------------------------------------------


_FX BOOLEAN Ipc_IsKnownDllInSandbox(
    THREAD_DATA *TlsData, const WCHAR *TruePath)
{
    static const WCHAR *_KnownDllPath = L"KnownDllPath";
    ULONG len;
    const WCHAR *DllName;
    NTSTATUS status;
    HANDLE handle;
    OBJECT_ATTRIBUTES objattrs;
    UNICODE_STRING objname;
    IO_STATUS_BLOCK IoStatusBlock;

    BOOLEAN is_known_dll_in_sandbox = FALSE;

    WCHAR *path = NULL;

    if (TlsData->ipc_KnownDlls_lock)
        return FALSE;

    TlsData->ipc_KnownDlls_lock = TRUE;

    //
    // if ntdll is looking for a KnownDll but the sandboxed System32 folder
    // contains this DLL, then pretend there is no matching section object
    //
    // first, make sure the request is for a KnownDlls DLL objects,
    // and that it is not a recursive invocation
    //

    len = wcslen(TruePath);
    if (len > 96)
        goto finish;

    if (TruePath[0] != L'\\' || TruePath[1] != L'K')
        goto finish;
    if (_wcsnicmp(TruePath, L"\\KnownDlls", 10) != 0)
        goto finish;

    DllName = wcsrchr(TruePath, L'\\');
    if (DllName == TruePath)
        goto finish;
    if (_wcsnicmp(DllName + 1, _KnownDllPath, 12) == 0)
        goto finish;

    //
    // open and read the \KnownDlls\KnownDllPath symlink
    //

    path = Dll_AllocTemp(512);

    len = (ULONG)(DllName + 1 - TruePath);
    wmemcpy(path, TruePath, len);
    wmemcpy(path + len, _KnownDllPath, 13); // including NULL

    RtlInitUnicodeString(&objname, path);

    InitializeObjectAttributes(
        &objattrs, &objname, OBJ_CASE_INSENSITIVE, NULL, NULL);

    status = NtOpenSymbolicLinkObject(
                        &handle, SYMBOLIC_LINK_QUERY, &objattrs);
    if (! NT_SUCCESS(status))
        goto finish;

    wmemcpy(path, File_BQQB, 4);

    objname.Length = (USHORT)256;
    objname.MaximumLength = objname.Length;
    objname.Buffer = path + 4;

    status = NtQuerySymbolicLinkObject(handle, &objname, NULL);

    NtClose(handle);

    if (! NT_SUCCESS(status))
        goto finish;

    //
    // try to open the specified DLL file
    //

    objname.Buffer[objname.Length / sizeof(WCHAR)] = L'\0';
    wcscat(path, DllName);

    RtlInitUnicodeString(&objname, path);

    status = NtCreateFile(
        &handle, FILE_GENERIC_READ, &objattrs,
        &IoStatusBlock, NULL, 0, FILE_SHARE_VALID_FLAGS,
        FILE_OPEN,
        FILE_NON_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT,
        NULL, 0);

    if (NT_SUCCESS(status)) {

        BOOLEAN IsBoxedPath;
        status = SbieDll_GetHandlePath(handle, NULL, &IsBoxedPath);

        if (NT_SUCCESS(status) && IsBoxedPath)
            is_known_dll_in_sandbox = TRUE;

        NtClose(handle);
    }

    //
    // finish
    //

finish:

    TlsData->ipc_KnownDlls_lock = FALSE;

    if (path)
        Dll_Free(path);

    return is_known_dll_in_sandbox;
}


//---------------------------------------------------------------------------
// Ipc_ConnectProxyPort
//---------------------------------------------------------------------------


_FX NTSTATUS Ipc_ConnectProxyPort(
    HANDLE *PortHandle,
    const WCHAR *TruePath,
    ULONG AlpcConnectionFlags,
    void *ConnectionInfo,
    ULONG *ConnectionInfoLength,
    void *MaximumMessageLengthOrAlpcInfo,
    void *ShouldBeNull1, void *ShouldBeNull2, void *ShouldBeNull3)
{
    NTSTATUS status;
    NAMED_PIPE_LPC_CONNECT_REQ *req;
    NAMED_PIPE_LPC_CONNECT_RPL *rpl;
    ALPC_PORT_ATTRIBUTES *alpc_info;
    ULONG req_len;
    ULONG info_len;
    ULONG err = 0;

    //
    // check if the requested LPC port is one that we can proxy
    //

    if (_wcsicmp(TruePath, L"\\RPC Control\\ntsvcs") != 0 &&
        _wcsicmp(TruePath, L"\\RPC Control\\plugplay") != 0)
        return STATUS_BAD_INITIAL_PC;

    //
    // check if we are in app mode in which case proxying is not needed, but we must indicate to open true path
    //

    if (Dll_CompartmentMode) // NoServiceAssist
        return STATUS_BAD_INITIAL_STACK;

    status = STATUS_SUCCESS;

    //
    // ClientSharedMemory, ServerSid, ServerSharedMemory should be null
    //

    if (ShouldBeNull1 || ShouldBeNull2 || ShouldBeNull3) {
        err = 0x11;
        goto finish;
    }

    //
    // if alpc, make sure specific (yet unknown) parameters are given
    //

    if (AlpcConnectionFlags != -1) {
        AlpcConnectionFlags &= ~0x40000000;     // turn off WOW64 flag
        if (AlpcConnectionFlags != 0x20000) {   // sync-connection flag??
            err = 0x12;
            goto finish;
        }
        alpc_info = (ALPC_PORT_ATTRIBUTES *)MaximumMessageLengthOrAlpcInfo;
        if (alpc_info->Flags != 0x10000) {      // can-impersonate flag??
            err = 0x13;
            goto finish;
        }
        if (ConnectionInfo || ConnectionInfoLength) {
            err = 0x14;
            goto finish;
        }
    } else
        alpc_info = NULL;

    //
    // determine request length based on ConnectionInfo parameter
    // (note that alpc calls should not use ConnectionInfo)
    //

    req_len = sizeof(NAMED_PIPE_LPC_CONNECT_REQ);
    info_len = 0;
    if (ConnectionInfoLength && *ConnectionInfoLength) {
        if ((*ConnectionInfoLength > 1024) || (! ConnectionInfo)) {
            err = 0x15;
            goto finish;
        }
        info_len = *ConnectionInfoLength;
    }
    req_len += info_len;

    //
    // issue request to SbieSvc to connect to an LPC port
    //

    req = Dll_AllocTemp(req_len);

    req->h.length = req_len;
    req->h.msgid = MSGID_NAMED_PIPE_LPC_CONNECT;
    wcscpy(req->name, TruePath + 13);
    req->info_len = info_len;
    if (info_len)
        memcpy(req->info_data, ConnectionInfo, info_len);

    if (alpc_info)
        req->max_msg_len = alpc_info->MaxMessageLength;
    else
        req->max_msg_len = -1;

    rpl = (NAMED_PIPE_LPC_CONNECT_RPL *)SbieDll_CallServer(&req->h);
    Dll_Free(req);

    //
    // create proxy port and return connection information data
    // (note that alpc calls should not use ConnectionInfo)
    //

    if (! rpl) {
        status = STATUS_INSUFFICIENT_RESOURCES;
        err = 0x18;

    } else {

        status = rpl->h.status;
        if (NT_SUCCESS(status)) {

            status = File_AddProxyPipe(PortHandle, rpl->handle);
        }

        if (NT_SUCCESS(status) && (! alpc_info)) {

            if (MaximumMessageLengthOrAlpcInfo)
                *(ULONG *)MaximumMessageLengthOrAlpcInfo = rpl->max_msg_len;

            if (info_len) {
                if (rpl->info_len < info_len)
                    info_len = info_len;
                *ConnectionInfoLength = rpl->info_len;
                memcpy(ConnectionInfo, rpl->info_data, info_len);
            }
        }

        Dll_Free(rpl);

        if (! NT_SUCCESS(status))
            err = 0x19;
    }

    //
    // finish
    //

finish:

    if (Dll_ChromeSandbox && err == 0x19 && status == STATUS_ACCESS_DENIED) {
        // some people reportedly see this error occur in the context of
        // a Google Chrome restricted process, but there doesn't seem to
        // be any negative effect, so we simply hide the message
        err = 0;
    }

    if (err) {
        SbieApi_Log(2205, L"ConnectPort (%02X/%08X) %S", err, status, TruePath);
        if (status == STATUS_SUCCESS)
            status = STATUS_INVALID_PARAMETER;
    }

    return status;
}


//---------------------------------------------------------------------------
// Ipc_NtRequestWaitReplyPort
//---------------------------------------------------------------------------


_FX NTSTATUS Ipc_NtRequestWaitReplyPort(
    HANDLE PortHandle, PORT_MESSAGE *RequestMsg, PORT_MESSAGE *ReplyMsg)
{
    NAMED_PIPE_LPC_REQUEST_REQ *req;
    NAMED_PIPE_LPC_REQUEST_RPL *rpl = NULL;
    PORT_DATA_INFO *info = NULL;
    NTSTATUS status;
    ULONG req_len;
    ULONG err = 0;
    ULONG handle;

    handle = File_GetProxyPipe(PortHandle, NULL);
    if (! handle) {

        return __sys_NtRequestWaitReplyPort(
            PortHandle, RequestMsg, ReplyMsg);
    }

    status = STATUS_SUCCESS;

    //
    // proxy channel processing, make sure we have valid input
    //

    if ((! RequestMsg) || (! ReplyMsg)) {
        err = 0x21;
        goto finish;
    }

    if (RequestMsg->u2.s2.DataInfoOffset) {

        //
        // an LPC data info is used to send/receive more data than
        // there is room in a single PORT_MESSAGE.  we can handle
        // at most one data info entry.
        //

        info = (PORT_DATA_INFO *)
                    ((UCHAR *)RequestMsg + RequestMsg->u2.s2.DataInfoOffset);
        if (info->NumDataInfo != 1) {
            err = 0x22;
            goto finish;
        }
    }

    //
    // initialize request data with message data and possible data info
    //

    req_len = sizeof(NAMED_PIPE_LPC_REQUEST_REQ);
    if (info)
        req_len += info->BufferLen;
    req = Dll_AllocTemp(req_len);

    req->h.length = req_len;
    req->h.msgid = MSGID_NAMED_PIPE_LPC_REQUEST;

    req->handle = handle;

    memcpy(req->data, RequestMsg, MAX_PORTMSG_LENGTH);
    if (info)
        memcpy(req->info, info->Buffer, info->BufferLen);

    rpl = (NAMED_PIPE_LPC_REQUEST_RPL *)SbieDll_CallServer(&req->h);
    Dll_Free(req);

    //
    // copy data from the reply into the caller buffers
    //

    if (! rpl) {
        status = STATUS_INSUFFICIENT_RESOURCES;
        err = 0x28;

    } else {

        status = rpl->h.status;
        if (! NT_SUCCESS(status))
            err = 0x29;
        else {

            PORT_MESSAGE *msg = (PORT_MESSAGE *)rpl->data;

            msg->ClientId.UniqueProcess =
                (HANDLE)(ULONG_PTR)GetCurrentProcessId();
            msg->ClientId.UniqueThread =
                (HANDLE)(ULONG_PTR)GetCurrentThreadId();

            if (msg->u2.s2.DataInfoOffset) {

                //
                // if a data info entry was used, copy the info data
                // from the reply into where the caller specified in
                // the input data info entry
                //

                PORT_DATA_INFO *info2 = (PORT_DATA_INFO *)
                    ((UCHAR *)msg + msg->u2.s2.DataInfoOffset);
                if (! info) {
                    err = 0x23;
                    goto finish;
                }
                if ((info2->NumDataInfo != info->NumDataInfo)
                        || (info2->BufferLen > info->BufferLen)) {
                    err = 0x24;
                    goto finish;
                }
                info2->Buffer = info->Buffer;
                memcpy(info2->Buffer, rpl->info, info2->BufferLen);
            }

            memcpy(ReplyMsg, msg, msg->u1.s1.TotalLength);
        }
    }

    //
    // finish
    //

finish:

    if (rpl)
        Dll_Free(rpl);

    if (err) {
        SbieApi_Log(2205, L"RequestPort (%02X/%08X)", err, status);
        if (status == STATUS_SUCCESS)
            status = STATUS_INVALID_PARAMETER;
    }

    return status;
}


//---------------------------------------------------------------------------
// Ipc_NtAlpcSendWaitReceivePort
//---------------------------------------------------------------------------


_FX NTSTATUS Ipc_NtAlpcSendWaitReceivePort(
    HANDLE PortHandle, ULONG SendFlags,
    PORT_MESSAGE *SendMsg, ALPC_MESSAGE_VIEW *SendView,
    PORT_MESSAGE *ReceiveMsg, ULONG *ReceiveMsgSize,
    ALPC_MESSAGE_VIEW *ReceiveView, LARGE_INTEGER *Timeout)
{
    NAMED_PIPE_ALPC_REQUEST_REQ *req;
    NAMED_PIPE_ALPC_REQUEST_RPL *rpl = NULL;
    NTSTATUS status;
    ULONG req_len;
    ULONG err = 0;
    ULONG handle;
    UCHAR FileIndex;

    handle = File_GetProxyPipe(PortHandle, &FileIndex);
    if (! handle) {

        return __sys_NtAlpcSendWaitReceivePort(
            PortHandle, SendFlags, SendMsg, SendView,
            ReceiveMsg, ReceiveMsgSize, ReceiveView, Timeout);
    }

    status = STATUS_SUCCESS;

    //
    // proxy channel processing, make sure we have valid input
    //

    if ((! SendMsg) || (! SendView) || (! ReceiveMsg) || (! ReceiveView)
            || (! ReceiveMsgSize)) {

        SendFlags &= ~0x40000000;               // turn off WOW64 flag
        if (SendFlags == 0 && SendView && SendView->SendFlags == 0x40000000
                && SendView->ReceiveFlags == SendView->SendFlags
                && SendView->u.s2.ViewAttrs == MEM_FREE) {

            //
            // if the response was supposed to use a mapped view (see below)
            // then this request is intended to unmap the view.  used by
            // RPCRT4!LRPC_CASSOCIATION::DropView but we just ignore it here
            //

            return STATUS_SUCCESS;
        }

        /*SbieApi_Log(2205, L"SendPort - SF=%08X SM=%c SV=%c RM=%c RV=%c",
            SendFlags,
            (SendMsg ? 'Y' : 'N'), (SendView ? 'Y' : 'N'),
            (ReceiveMsg ? 'Y' : 'N'), (ReceiveView ? 'Y' : 'N'));
        if (SendView) {
            SbieApi_Log(2205, L"SendPort - SVSF=%08X SVRF=%08X SVVA=%08X",
                SendView->SendFlags,  SendView->ReceiveFlags, SendView->u.s2.ViewAttrs);
        }*/

        err = 0x31;
        goto finish;
    }

    if (SendMsg->u2.s2.DataInfoOffset) {
        err = 0x32;
        goto finish;
    }

    //
    // initialize request data with message data and view data
    //

    req_len = sizeof(NAMED_PIPE_ALPC_REQUEST_REQ)
            + SendMsg->u1.s1.TotalLength;
    req = Dll_AllocTemp(req_len);

    req->h.length = req_len;
    req->h.msgid = MSGID_NAMED_PIPE_ALPC_REQUEST;

    req->handle = handle;

    req->msg_len = *ReceiveMsgSize;

    req->view[0] = SendView->SendFlags;
    req->view[1] = SendView->ReceiveFlags;

    memcpy(req->data, SendMsg, SendMsg->u1.s1.TotalLength);

    rpl = (NAMED_PIPE_ALPC_REQUEST_RPL *)SbieDll_CallServer(&req->h);
    Dll_Free(req);

    //
    // copy data from the reply into the caller buffers
    //

    if (! rpl) {
        status = STATUS_INSUFFICIENT_RESOURCES;
        err = 0x38;

    } else {

        status = rpl->h.status;
        if (! NT_SUCCESS(status))
            err = 0x39;
        else {

            PORT_MESSAGE *msg = (PORT_MESSAGE *)rpl->data;

            msg->ClientId.UniqueProcess =
                (HANDLE)(ULONG_PTR)GetCurrentProcessId();
            msg->ClientId.UniqueThread =
                (HANDLE)(ULONG_PTR)GetCurrentThreadId();

            memcpy(ReceiveMsg, msg, msg->u1.s1.TotalLength);

            *ReceiveMsgSize = rpl->msg_len;

            ReceiveView->SendFlags = rpl->view[0];
            ReceiveView->ReceiveFlags = rpl->view[1];
            if (ReceiveView->ReceiveFlags & 0x40000000) {

                //
                // flag 0x40000000 indicates that a section view was mapped
                // with response data.  in this case SbieSvc copied the data
                // from the mapped view into the reply buffer.  now we need
                // to copy this data into a VirtualAlloc-ed area to emulate
                // the behavior of this memory being mapped externally
                //

                ULONG ViewSize = rpl->view[2];
                UCHAR *ViewBase = (UCHAR *)VirtualAlloc(NULL, ViewSize,
                    MEM_RESERVE | MEM_COMMIT | MEM_TOP_DOWN, PAGE_READWRITE);

                if (! ViewBase)
                    status = STATUS_INSUFFICIENT_RESOURCES;
                else {

                    UCHAR *OldViewBase = InterlockedExchangePointer(
                        (void **)&Ipc_ProxyViews[FileIndex], ViewBase);

                    ReceiveView->u.s2.ViewAttrs = MEM_MAPPED;
                    ReceiveView->u.s2.ViewBase  = (ULONG_PTR)ViewBase;
                    ReceiveView->u.s2.ViewSize  = ViewSize;

                    memcpy(ViewBase,
                           &rpl->data[(rpl->msg_len + 7) & (~7)],
                           ViewSize);

                    if (OldViewBase)
                        VirtualFree(OldViewBase, 0, MEM_RELEASE);
                }

            } else {

                //
                // otherwise just return the length of the response buffer
                //

                ReceiveView->u.s1.ReplyLength = rpl->view[2];
                ReceiveView->u.s1.MessageId   = msg->MessageId;
                ReceiveView->u.s1.CallbackId  = msg->CallbackId;
            }
        }
    }

    //
    // finish
    //

finish:

    if (rpl)
        Dll_Free(rpl);

    if (err) {
        SbieApi_Log(2205, L"SendPort (%02X/%08X)", err, status);
        if (status == STATUS_SUCCESS)
            status = STATUS_INVALID_PARAMETER;
    }

    return status;
}


//---------------------------------------------------------------------------
// Ipc_NtQueryObjectName
//---------------------------------------------------------------------------


_FX ULONG Ipc_NtQueryObjectName(UNICODE_STRING *ObjectName, ULONG MaxLen)
{
    //
    // adjust the path returned by NtQueryObject(ObjectNameInformation)
    // to not include a sandbox prefix
    //
    // this is important because the Google Chrome broker uses NtQueryObject
    // to confirm the opened handle matches the requested path
    //

    ULONG Len = ObjectName->Length;
    WCHAR *Buf = ObjectName->Buffer;

    //if (Dll_AlernateIpcNaming)
    //{
    //    if (Len >= Dll_BoxIpcPathLen * sizeof(WCHAR) &&
    //            0 == Dll_NlsStrCmp(&Buf[Len - Dll_BoxIpcPathLen], Dll_BoxIpcPath, Dll_BoxIpcPathLen)) {
    //
    //        Buf[Len - Dll_BoxIpcPathLen] = L'\0';
    //
    //        ObjectName->Length -= (USHORT)Dll_BoxIpcPathLen;
    //        ObjectName->MaximumLength = ObjectName->Length + sizeof(WCHAR);
    //
    //        return ObjectName->MaximumLength;
    //    }
    //}
    //else
    if (Len >= Dll_BoxIpcPathLen * sizeof(WCHAR) &&
            0 == Dll_NlsStrCmp(Buf, Dll_BoxIpcPath, Dll_BoxIpcPathLen)) {

        ULONG MoveLen = Len / sizeof(WCHAR) - Dll_BoxIpcPathLen;
        wmemmove(Buf, Buf + Dll_BoxIpcPathLen, MoveLen);
        Buf[MoveLen] = L'\0';

        ObjectName->Length = (USHORT)(MoveLen * sizeof(WCHAR));
        ObjectName->MaximumLength = ObjectName->Length + sizeof(WCHAR);

        return ObjectName->MaximumLength;
    }

    return 0;
}
