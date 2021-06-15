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

#ifndef _MY_WINNT_H
#define _MY_WINNT_H

#pragma warning(disable : 4267)     //  warning C4267: '=': conversion from 'size_t' to 'ULONG', possible loss of data

#include <ntifs.h>
#include "alpc.h"

#define NTOS_API(type)  NTSYSAPI type NTAPI
#define NTOS_NTSTATUS   NTOS_API(NTSTATUS)


// ------------------------------------------------------------------
// Security related
// ------------------------------------------------------------------

//
// Group attributes
//

#define SE_GROUP_MANDATORY              (0x00000001L)
#define SE_GROUP_ENABLED_BY_DEFAULT     (0x00000002L)
#define SE_GROUP_ENABLED                (0x00000004L)
#define SE_GROUP_OWNER                  (0x00000008L)
#define SE_GROUP_USE_FOR_DENY_ONLY      (0x00000010L)
#define SE_GROUP_INTEGRITY              (0x00000020L)
#define SE_GROUP_INTEGRITY_ENABLED      (0x00000040L)
#define SE_GROUP_LOGON_ID               (0xC0000000L)
#define SE_GROUP_RESOURCE               (0x20000000L)

#ifdef OLD_DDK
typedef enum _TOKEN_INFORMATION_CLASS2 {
    TokenIsAppContainer = 29,
    TokenCapabilities,
    TokenAppContainerSid,
    TokenAppContainerNumber,
    TokenUserClaimAttributes,
    TokenDeviceClaimAttributes,
    TokenRestrictedUserClaimAttributes,
    TokenRestrictedDeviceClaimAttributes,
    TokenDeviceGroups,
    TokenRestrictedDeviceGroups,
    TokenSecurityAttributes,
    TokenIsRestricted,
    TokenProcessTrustLevel,
    TokenPrivateNameSpace//,
    //MaxTokenInfoClass  // MaxTokenInfoClass should always be the last enum
} TOKEN_INFORMATION_CLASS2;
#endif // OLD_DDK

NTOS_NTSTATUS   ZwOpenThreadToken(
    IN HANDLE       ThreadHandle,
    IN ACCESS_MASK  DesiredAccess,
    IN BOOLEAN      OpenAsSelf,
    OUT PHANDLE     TokenHandle);

NTOS_NTSTATUS   ZwOpenProcessToken(
    IN HANDLE       ProcessHandle,
    IN ACCESS_MASK  DesiredAccess,
    OUT PHANDLE     TokenHandle);

NTOS_API(ULONG) SeTokenImpersonationLevel(
    IN PACCESS_TOKEN Token);


// ------------------------------------------------------------------
// Misc
// ------------------------------------------------------------------

//ULONG __cdecl sprintf(char *buffer, const char *format, ...);
//ULONG __cdecl swprintf(wchar_t *buffer, const wchar_t *format, ...);

NTOS_NTSTATUS   ZwYieldExecution(void);

NTOS_NTSTATUS   ZwLoadDriver(
    IN PUNICODE_STRING RegistryPath);

NTOS_NTSTATUS   ZwCreateSymbolicLinkObject(
    OUT PHANDLE                 LinkHandle,
    IN  ACCESS_MASK             DesiredAccess,
    IN  POBJECT_ATTRIBUTES      ObjectAttributes,
    IN  PUNICODE_STRING         LinkTarget);

// incomplete declarations, just enough to get the import name right

NTOS_NTSTATUS   ZwAccessCheckAndAuditAlarm(
    PVOID parm1,
    PVOID parm2,
    PVOID parm3,
    PVOID parm4,
    PVOID parm5,
    PVOID parm6,
    PVOID parm7,
    PVOID parm8,
    PVOID parm9,
    PVOID parm10,
    PVOID parm11);

NTOS_NTSTATUS   ZwAlertThread(
    IN HANDLE ThreadHandle);

NTOS_NTSTATUS   RtlSetSaclSecurityDescriptor(
    IN PSECURITY_DESCRIPTOR SecurityDescriptor,
    IN BOOLEAN SaclPresent,
    IN PACL Sacl,
    IN BOOLEAN SaclDefaulted);

// ------------------------------------------------------------------
// Job related
// ------------------------------------------------------------------


#define JOB_OBJECT_ASSIGN_PROCESS           (0x0001)
#define JOB_OBJECT_SET_ATTRIBUTES           (0x0002)
#define JOB_OBJECT_QUERY                    (0x0004)
#define JOB_OBJECT_TERMINATE                (0x0008)
#define JOB_OBJECT_SET_SECURITY_ATTRIBUTES  (0x0010)
#define JOB_OBJECT_ALL_ACCESS       (STANDARD_RIGHTS_REQUIRED | SYNCHRONIZE | \
    0x1F )


typedef enum _JOBOBJECTINFOCLASS {
    JobObjectBasicAccountingInformation = 1,
    JobObjectBasicLimitInformation,
    JobObjectBasicProcessIdList,
    JobObjectBasicUIRestrictions,
    JobObjectSecurityLimitInformation,
    JobObjectEndOfJobTimeInformation,
    JobObjectAssociateCompletionPortInformation,
    JobObjectBasicAndIoAccountingInformation,
    JobObjectExtendedLimitInformation,
    JobObjectJobSetInformation,
    MaxJobObjectInfoClass
} JOBOBJECTINFOCLASS;


// ------------------------------------------------------------------
// Registry related
// ------------------------------------------------------------------


NTOS_NTSTATUS   ZwLoadKey(
    IN POBJECT_ATTRIBUTES   TargetKey,
    IN POBJECT_ATTRIBUTES   SourceFile);

NTOS_NTSTATUS   ZwUnloadKey(
    IN HANDLE               KeyHandle);


typedef NTSTATUS(*PEX_CALLBACK_FUNCTION) (
    IN PVOID CallbackContext,
    IN PVOID Argument1,
    IN PVOID Argument2
    );

typedef NTSTATUS(*P_CmRegisterCallback)(
    IN PEX_CALLBACK_FUNCTION  Function,
    IN PVOID  Context,
    OUT PLARGE_INTEGER  Cookie);

typedef NTSTATUS(*P_CmRegisterCallbackEx)(
    IN PEX_CALLBACK_FUNCTION  Function,
    IN PCUNICODE_STRING  Altitude,
    IN PVOID  Driver,
    IN PVOID  Context,
    OUT PLARGE_INTEGER  Cookie,
    PVOID  Reserved
    );

typedef struct _REG_OPEN_CREATE_KEY_INFORMATION_VISTA {
    PUNICODE_STRING     CompleteName; // IN
    PVOID               RootObject;   // IN
    PVOID               ObjectType;   // new to Windows Vista
    ULONG               CreateOptions;// new to Windows Vista
    PUNICODE_STRING     Class;        // new to Windows Vista
    PVOID               SecurityDescriptor;// new to Windows Vista
    PVOID               SecurityQualityOfService;// new to Windows Vista
    ACCESS_MASK         DesiredAccess;// new to Windows Vista
    ACCESS_MASK         GrantedAccess;// new to Windows Vista
                                      // to be filled in by callbacks
                                      // when bypassing native code
    PULONG              Disposition;  // new to Windows Vista
                                      // on pass through, callback should fill
                                      // in disposition
    PVOID               *ResultObject;// new to Windows Vista
                                      // on pass through, callback should return
                                      // object to be used for the return handle
    PVOID               CallContext;  // new to Windows Vista
    PVOID               RootObjectContext;  // new to Windows Vista
    PVOID               Transaction;  // new to Windows Vista
    PVOID               Reserved;     // new to Windows Vista
} REG_OPEN_CREATE_KEY_INFORMATION_VISTA;


// ------------------------------------------------------------------
// File related
// ------------------------------------------------------------------


#define IO_OPEN_TARGET_DIRECTORY        0x0004
#define IO_IGNORE_SHARE_ACCESS_CHECK    0x0800

#define FILE_SHARE_ANY \
    (FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE)


// ------------------------------------------------------------------


typedef void(*P_KeSetSystemAffinityThread)(KAFFINITY Affinity);

typedef void(*P_KeRevertToUserAffinityThread)(void);

typedef KAFFINITY(*P_KeSetSystemAffinityThreadEx)(KAFFINITY Affinity);

typedef void(*P_KeRevertToUserAffinityThreadEx)(KAFFINITY Affinity);


// ------------------------------------------------------------------
// Process related
// ------------------------------------------------------------------


// DesiredAccess flags for ZwOpenProcess
#define PROCESS_TERMINATE         (0x0001)
#define PROCESS_CREATE_THREAD     (0x0002)
#define PROCESS_SET_SESSIONID     (0x0004)
#define PROCESS_VM_OPERATION      (0x0008)
#define PROCESS_VM_READ           (0x0010)
#define PROCESS_VM_WRITE          (0x0020)
#define PROCESS_DUP_HANDLE        (0x0040)
#define PROCESS_CREATE_PROCESS    (0x0080)
#define PROCESS_SET_QUOTA         (0x0100)
#define PROCESS_SET_INFORMATION   (0x0200)
#define PROCESS_QUERY_INFORMATION (0x0400)
#define PROCESS_SUSPEND_RESUME    (0x0800)
#define PROCESS_QUERY_LIMITED_INFORMATION  (0x1000)     // vista
#define PROCESS_SET_LIMITED_INFORMATION    (0x2000)
#if (NTDDI_VERSION >= NTDDI_VISTA)
#define PROCESS_ALL_ACCESS        (STANDARD_RIGHTS_REQUIRED | SYNCHRONIZE | \
                                   0xFFFF)
#else
#define PROCESS_ALL_ACCESS        (STANDARD_RIGHTS_REQUIRED | SYNCHRONIZE | \
                                   0xFFF)
#endif


extern POBJECT_TYPE *PsProcessType;
extern POBJECT_TYPE *ExWindowStationObjectType;

NTOS_API(ULONG_PTR) PsGetThreadWin32Thread(PETHREAD Thread);

NTOS_API(HANDLE) PsGetProcessWin32WindowStation(PEPROCESS EProcess);

NTOS_API(ULONG) PsGetProcessSessionId(PEPROCESS EProcess);

NTOS_API(ULONG_PTR) PsGetProcessPeb(PEPROCESS EProcess);

NTOS_API(PEPROCESS) PsGetThreadProcess(PETHREAD Thread);

NTOS_API(ULONG_PTR) PsGetProcessJob(PEPROCESS Process);

NTOS_API(BOOLEAN) PsGetProcessExitProcessCalled(PEPROCESS Process);

NTOS_NTSTATUS   ZwQueryInformationProcess(
    IN HANDLE           ProcessHandle,
    IN PROCESSINFOCLASS ProcessInformationClass,
    OUT PVOID           ProcessInformation,
    IN ULONG            ProcessInformationLength,
    OUT PULONG          ReturnLength OPTIONAL);

NTOS_NTSTATUS   ZwSetInformationProcess(
    IN HANDLE           ProcessHandle,
    IN PROCESSINFOCLASS ProcessInformationClass,
    IN PVOID            ProcessInformation,
    IN ULONG            ProcessInformationLength);


// ------------------------------------------------------------------
// Thread related
// ------------------------------------------------------------------


#define THREAD_TERMINATE               (0x0001)
#define THREAD_SUSPEND_RESUME          (0x0002)
#define THREAD_GET_CONTEXT             (0x0008)
#define THREAD_SET_CONTEXT             (0x0010)
#define THREAD_SET_INFORMATION         (0x0020)
#define THREAD_QUERY_INFORMATION       (0x0040)
#define THREAD_SET_THREAD_TOKEN        (0x0080)
#define THREAD_IMPERSONATE             (0x0100)
#define THREAD_DIRECT_IMPERSONATION    (0x0200)
#define THREAD_SET_LIMITED_INFORMATION   (0x0400)       // vista
#define THREAD_QUERY_LIMITED_INFORMATION (0x0800)       // vista
#if (NTDDI_VERSION >= NTDDI_VISTA)
#define THREAD_ALL_ACCESS         (STANDARD_RIGHTS_REQUIRED | SYNCHRONIZE | \
                                   0xFFFF)
#else
#define THREAD_ALL_ACCESS         (STANDARD_RIGHTS_REQUIRED | SYNCHRONIZE | \
                                   0x3FF)
#endif


NTOS_NTSTATUS  PsSetThreadHardErrorsAreDisabled(
    PETHREAD        EThread,
    BOOLEAN         Disabled);

// ------------------------------------------------------------------
// Security related
// ------------------------------------------------------------------


//
//  The following are the inherit flags that go into the AceFlags field
//  of an Ace header.
//

#define OBJECT_INHERIT_ACE                (0x1)
#define CONTAINER_INHERIT_ACE             (0x2)
#define NO_PROPAGATE_INHERIT_ACE          (0x4)
#define INHERIT_ONLY_ACE                  (0x8)
#define INHERITED_ACE                     (0x10)
#define VALID_INHERIT_FLAGS               (0x1F)

#define DISABLE_MAX_PRIVILEGE   0x1
#define SANDBOX_INERT           0x2
#define LUA_TOKEN               0x4
#define WRITE_RESTRICTED        0x8


// ------------------------------------------------------------------
// Object Manager
// ------------------------------------------------------------------


typedef struct _OBJECT_DIRECTORY_INFORMATION
{
    UNICODE_STRING ObjectName;
    UNICODE_STRING ObjectTypeName;
} OBJECT_DIRECTORY_INFORMATION, *POBJECT_DIRECTORY_INFORMATION;


NTOS_NTSTATUS   NtDuplicateObject(
    IN HANDLE       SourceProcessHandle,
    IN HANDLE       SourceHandle,
    IN HANDLE       TargetProcessHandle OPTIONAL,
    OUT PHANDLE     TargetHandle OPTIONAL,
    IN ACCESS_MASK  DesiredAccess,
    IN ULONG        HandleAttributes,
    IN ULONG        Options);


NTOS_NTSTATUS   ObReferenceObjectByName(
    IN PUNICODE_STRING              ObjectName,
    IN ULONG                        Attributes,
    IN PACCESS_STATE                PassedAccessState OPTIONAL,
    IN ACCESS_MASK                  DesiredAccess OPTIONAL,
    IN POBJECT_TYPE                 ObjectType,
    IN KPROCESSOR_MODE              AccessMode,
    IN OUT PVOID                    ParseContext OPTIONAL,
    OUT PVOID *                     Object);

NTOS_NTSTATUS   ZwQueryDirectoryObject(
    IN HANDLE               DirectoryHandle,
    OUT PVOID               Buffer,
    IN ULONG                BufferLength,
    IN BOOLEAN              ReturnSingleEntry,
    IN BOOLEAN              RestartScan,
    IN OUT PULONG           Context,
    OUT PULONG              ReturnLength OPTIONAL);

NTOS_NTSTATUS   ObOpenObjectByName(
    IN POBJECT_ATTRIBUTES   ObjectAttributes,
    IN POBJECT_TYPE         ObjectType OPTIONAL,
    IN KPROCESSOR_MODE      AccessMode,
    IN OUT PACCESS_STATE    AccessState OPTIONAL,
    IN ACCESS_MASK          DesiredAccess OPTIONAL,
    IN OUT PVOID            ParseContext OPTIONAL,
    OUT PHANDLE             Handle);

typedef struct _OBJECT_DUMP_CONTROL {
    PVOID Stream;
    ULONG Detail;
} OB_DUMP_CONTROL, *POB_DUMP_CONTROL;


typedef enum _OB_OPEN_REASON {
    ObCreateHandle,
    ObOpenHandle,
    ObDuplicateHandle,
    ObInheritHandle,
    ObMaxOpenReason
} OB_OPEN_REASON;


typedef VOID(*OB_DUMP_METHOD)(
    IN PVOID Object,
    IN POB_DUMP_CONTROL Control OPTIONAL);


typedef NTSTATUS(*OB_OPEN_METHOD)(
    IN OB_OPEN_REASON OpenReason,
    IN PEPROCESS Process OPTIONAL,
    IN PVOID Object,
    IN ACCESS_MASK GrantedAccess,
    IN ULONG HandleCount);


typedef NTSTATUS(*OB_OPEN_METHOD_VISTA)(
    IN OB_OPEN_REASON OpenReason,
    IN ACCESS_MASK GrantedAccess,
    IN PEPROCESS Process OPTIONAL,
    IN PVOID Object,
    PVOID Unknown1,
    IN ULONG HandleCount);


typedef VOID(*OB_CLOSE_METHOD)(
    IN PEPROCESS Process OPTIONAL,
    IN PVOID Object,
    IN ACCESS_MASK GrantedAccess,
    IN ULONG ProcessHandleCount,
    IN ULONG SystemHandleCount);


typedef VOID(*OB_DELETE_METHOD)(
    IN PVOID Object);


typedef NTSTATUS(*OB_PARSE_METHOD)(
    IN PVOID ParseObject,
    IN PVOID ObjectType,
    IN OUT PACCESS_STATE AccessState,
    IN KPROCESSOR_MODE AccessMode,
    IN ULONG Attributes,
    IN OUT PUNICODE_STRING CompleteName,
    IN OUT PUNICODE_STRING RemainingName,
    IN OUT PVOID Context OPTIONAL,
    IN PSECURITY_QUALITY_OF_SERVICE SecurityQos OPTIONAL,
    OUT PVOID *Object);


typedef NTSTATUS(*OB_SECURITY_METHOD)(
    IN PVOID Object,
    IN SECURITY_OPERATION_CODE OperationCode,
    IN PSECURITY_INFORMATION SecurityInformation,
    IN OUT PSECURITY_DESCRIPTOR SecurityDescriptor,
    IN OUT PULONG CapturedLength,
    IN OUT PSECURITY_DESCRIPTOR *ObjectsSecurityDescriptor,
    IN POOL_TYPE PoolType,
    IN PGENERIC_MAPPING GenericMapping);


typedef NTSTATUS(*OB_QUERYNAME_METHOD)(
    IN PVOID Object,
    IN BOOLEAN HasObjectName,
    OUT POBJECT_NAME_INFORMATION ObjectNameInfo,
    IN ULONG Length,
    OUT PULONG ReturnLength);


typedef BOOLEAN(*OB_OKAYTOCLOSE_METHOD)(
    IN PEPROCESS Process OPTIONAL,
    IN PVOID Object,
    IN HANDLE Handle);


typedef struct _OBJECT_TYPE_INITIALIZER {
    USHORT Length;
    BOOLEAN UseDefaultObject;
    BOOLEAN Reserved;
    ULONG InvalidAttributes;
    GENERIC_MAPPING GenericMapping;
    ULONG ValidAccessMask;
    BOOLEAN SecurityRequired;
    BOOLEAN MaintainHandleCount;
    BOOLEAN MaintainTypeList;
    POOL_TYPE PoolType;
    ULONG DefaultPagedPoolCharge;
    ULONG DefaultNonPagedPoolCharge;
    OB_DUMP_METHOD DumpProcedure;
    OB_OPEN_METHOD OpenProcedure;
    OB_CLOSE_METHOD CloseProcedure;
    OB_DELETE_METHOD DeleteProcedure;
    OB_PARSE_METHOD ParseProcedure;
    OB_SECURITY_METHOD SecurityProcedure;
    OB_QUERYNAME_METHOD QueryNameProcedure;
    OB_OKAYTOCLOSE_METHOD OkayToCloseProcedure;
} OBJECT_TYPE_INITIALIZER, *POBJECT_TYPE_INITIALIZER;


typedef struct _OBJECT_TYPE {
    ERESOURCE Mutex;
    LIST_ENTRY TypeList;
    UNICODE_STRING Name;            // Copy from object header for convenience
    PVOID DefaultObject;
    ULONG Index;
    ULONG TotalNumberOfObjects;
    ULONG TotalNumberOfHandles;
    ULONG HighWaterNumberOfObjects;
    ULONG HighWaterNumberOfHandles;
    OBJECT_TYPE_INITIALIZER TypeInfo;
} OBJECT_TYPE, *POBJECT_TYPE;


typedef struct _OBJECT_HEADER {
    LONG PointerCount;
    union {
        LONG HandleCount;
        PSINGLE_LIST_ENTRY SEntry;
    };
    POBJECT_TYPE Type;
    UCHAR NameInfoOffset;
    UCHAR HandleInfoOffset;
    UCHAR QuotaInfoOffset;
    UCHAR Flags;
    union {
        PVOID ObjectCreateInfo;
        PVOID QuotaBlockCharged;
    };
    PSECURITY_DESCRIPTOR SecurityDescriptor;
    QUAD Body;
} OBJECT_HEADER, *POBJECT_HEADER;


#define NUMBER_HASH_BUCKETS 37


typedef struct _OBJECT_DIRECTORY_ENTRY {
    struct _OBJECT_DIRECTORY_ENTRY *ChainLink;
    PVOID Object;
} OBJECT_DIRECTORY_ENTRY, *POBJECT_DIRECTORY_ENTRY;


struct _DEVICE_MAP;


typedef struct _OBJECT_DIRECTORY {
    struct _OBJECT_DIRECTORY_ENTRY *HashBuckets[NUMBER_HASH_BUCKETS];
    struct _OBJECT_DIRECTORY_ENTRY **LookupBucket;
    BOOLEAN LookupFound;
    USHORT SymbolicLinkUsageCount;
    struct _DEVICE_MAP *DeviceMap;
} OBJECT_DIRECTORY, *POBJECT_DIRECTORY;


typedef struct _OBJECT_HEADER_NAME_INFO {
    POBJECT_DIRECTORY Directory;
    UNICODE_STRING Name;
    ULONG Reserved;
} OBJECT_HEADER_NAME_INFO, *POBJECT_HEADER_NAME_INFO;


#define OBJECT_TO_OBJECT_HEADER( o ) \
    CONTAINING_RECORD( (o), OBJECT_HEADER, Body )

#define OBJECT_HEADER_TO_NAME_INFO( oh ) ((POBJECT_HEADER_NAME_INFO) \
    ((oh)->NameInfoOffset == 0 ? NULL : ((PCHAR)(oh) - (oh)->NameInfoOffset)))


// ------------------------------------------------------------------
// Object Manager -- Vista SP1 Changes
// ------------------------------------------------------------------


typedef struct _OBJECT_TYPE_INITIALIZER_VISTA_SP1 {
    USHORT Length;
    BOOLEAN ObjectTypeFlags;
    ULONG ObjectTypeCode;
    ULONG InvalidAttributes;
    GENERIC_MAPPING GenericMapping;
    ULONG ValidAccessMask;
    ULONG RetainAccess;
    POOL_TYPE PoolType;
    ULONG DefaultPagedPoolCharge;
    ULONG DefaultNonPagedPoolCharge;
    OB_DUMP_METHOD DumpProcedure;
    OB_OPEN_METHOD OpenProcedure;
    OB_CLOSE_METHOD CloseProcedure;
    OB_DELETE_METHOD DeleteProcedure;
    OB_PARSE_METHOD ParseProcedure;
    OB_SECURITY_METHOD SecurityProcedure;
    OB_QUERYNAME_METHOD QueryNameProcedure;
    OB_OKAYTOCLOSE_METHOD OkayToCloseProcedure;
} OBJECT_TYPE_INITIALIZER_VISTA_SP1, *POBJECT_TYPE_INITIALIZER_VISTA_SP1;


typedef struct _OBJECT_TYPE_VISTA_SP1 {
    LIST_ENTRY TypeList;
    UNICODE_STRING Name;            // Copy from object header for convenience
    PVOID DefaultObject;
    ULONG Index;
    ULONG TotalNumberOfObjects;
    ULONG TotalNumberOfHandles;
    ULONG HighWaterNumberOfObjects;
    ULONG HighWaterNumberOfHandles;
    OBJECT_TYPE_INITIALIZER_VISTA_SP1 TypeInfo;
    ERESOURCE Mutex;
    EX_PUSH_LOCK TypeLock;
    ULONG Key;
    EX_PUSH_LOCK ObjectLocks[32];
    LIST_ENTRY CallbackList;
} OBJECT_TYPE_VISTA_SP1, *POBJECT_TYPE_VISTA_SP1;

#define OB_FLT_REGISTRATION_VERSION_0100  0x0100

#define OB_PREOP_SUCCESS 0

typedef ULONG OB_OPERATION;

#define OB_OPERATION_HANDLE_CREATE              0x00000001
#define OB_OPERATION_HANDLE_DUPLICATE           0x00000002

#if (NTDDI_VERSION < NTDDI_VISTASP1)
NTOS_NTSTATUS   ObRegisterCallbacks(
    __in POB_CALLBACK_REGISTRATION CallbackRegistration,
    __deref_out PVOID *RegistrationHandle);


NTOS_NTSTATUS   ObUnRegisterCallbacks(
    __in PVOID RegistrationHandle);
#endif


// ------------------------------------------------------------------
// System Information
// ------------------------------------------------------------------


typedef enum _SYSTEM_INFORMATION_CLASS {
    SystemBasicInformation,
    SystemProcessorInformation,
    SystemPerformanceInformation,
    SystemTimeOfDayInformation,
    SystemNotImplemented1,
    SystemProcessesAndThreadsInformation,
    SystemCallCounts,
    SystemConfigurationInformation,
    SystemProcessorTimes,
    SystemGlobalFlag,
    SystemNotImplemented2,
    SystemModuleInformation,
    SystemLockInformation,
    SystemNotImplemented3,
    SystemNotImplemented4,
    SystemNotImplemented5,
    SystemHandleInformation,
    SystemObjectInformation,
    SystemPagefileInformation,
    SystemInstructionEmulationCounts,
    SystemInvalidInfoClass1,
    SystemCacheInformation,
    SystemPoolTagInformation,
    SystemProcessorStatistics,
    SystemDpcInformation,
    SystemNotImplemented6,
    SystemLoadImage,
    SystemUnloadImage,
    SystemTimeAdjustment,
    SystemNotImplemented7,
    SystemNotImplemented8,
    SystemNotImplemented9,
    SystemCrashDumpInformation,
    SystemExceptionInformation,
    SystemCrashDumpStateInformation,
    SystemKernelDebuggerInformation,
    SystemContextSwitchInformation,
    SystemRegistryQuotaInformation,
    SystemLoadAndCallImage,
    SystemPrioritySeparation,
    SystemNotImplemented10,
    SystemNotImplemented11,
    SystemInvalidInfoClass2,
    SystemInvalidInfoClass3,
    SystemTimeZoneInformation,
    SystemLookasideInformation,
    SystemSetTimeSlipEvent,
    SystemSessionCreate,
    SystemSessionDetach,
    SystemInvalidInfoClass4,
    SystemRangeStartInformation,
    SystemVerifierInformation,
    SystemAddVerifier,
    SystemSessionProcessesInformation
} SYSTEM_INFORMATION_CLASS;


typedef struct _MODULE_INFO {
    ULONG_PTR   Reserved1;
    ULONG_PTR   MappedBase;
    ULONG_PTR   ImageBaseAddress;
    ULONG       ImageSize;
    ULONG       Flags;
    USHORT      LoadCount;
    USHORT      LoadOrderIndex;
    USHORT      InitOrderIndex;
    USHORT      NameOffset;
    UCHAR       Path[256];
} MODULE_INFO;


typedef struct _SYSTEM_THREAD_INFORMATION {
    LARGE_INTEGER    KernelTime;
    LARGE_INTEGER    UserTime;
    LARGE_INTEGER    CreateTime;
    ULONG            WaitTime;
    PVOID            StartAddress;
    CLIENT_ID        ClientId;
    KPRIORITY        Priority;
    KPRIORITY        BasePriority;
    ULONG            ContextSwitchCount;
    LONG             State;
    LONG             WaitReason;
} SYSTEM_THREAD_INFORMATION;

typedef struct _SYSTEM_PROCESS_INFORMATION {
    ULONG            NextEntryOffset;
    ULONG            ThreadCount;
    ULONG            Reserved1[6];
    LARGE_INTEGER    CreateTime;
    LARGE_INTEGER    UserTime;
    LARGE_INTEGER    KernelTime;
    UNICODE_STRING   ProcessName;
    KPRIORITY        BasePriority;
    ULONG_PTR        ProcessId;
    ULONG            InheritedFromProcessId;
    ULONG            HandleCount;
    ULONG            Reserved2[2];
    VM_COUNTERS      VmCounters;
    IO_COUNTERS      IoCounters;
#ifdef _WIN64
    LARGE_INTEGER    UnknownNewIn64Bit[2];
#endif
    SYSTEM_THREAD_INFORMATION Threads[0];
} SYSTEM_PROCESS_INFORMATION;


typedef struct _SYSTEM_MODULE_INFORMATION {
    ULONG       ModuleCount;
#ifdef _WIN64
    ULONG       Reserved;
#endif
    MODULE_INFO ModuleInfo[0];
} SYSTEM_MODULE_INFORMATION;

NTOS_NTSTATUS   ZwQuerySystemInformation(
    IN  SYSTEM_INFORMATION_CLASS    SystemInformationClass,
    OUT PVOID                       SystemInformation,
    IN  ULONG                       SystemInformationLength,
    OUT PULONG                      ReturnLength OPTIONAL);

// ------------------------------------------------------------------
// Thread related
// ------------------------------------------------------------------

typedef void *PINITIAL_TEB;

typedef struct _THREAD_BASIC_INFORMATION {
    NTSTATUS ExitStatus;
    PVOID TebBaseAddress;
    CLIENT_ID ClientId;
    KAFFINITY AffinityMask;
    KPRIORITY Priority;
    KPRIORITY BasePriority;
} THREAD_BASIC_INFORMATION, *PTHREAD_BASIC_INFORMATION;


NTOS_NTSTATUS   ZwOpenThread(
    OUT PHANDLE             ThreadHandle,
    IN ACCESS_MASK          DesiredAccess,
    IN POBJECT_ATTRIBUTES   ObjectAttributes,
    IN PCLIENT_ID           ClientId OPTIONAL);

NTOS_NTSTATUS   ZwQueryInformationThread(
    IN HANDLE           ThreadHandle,
    IN THREADINFOCLASS  ThreadInformationClass,
    OUT PVOID           ThreadInformation,
    IN ULONG            ThreadInformationLength,
    OUT PULONG          ReturnLength OPTIONAL);


//
// ------------------------------------------------------------------
// Memory related
// ------------------------------------------------------------------


typedef ULONG MEMORY_INFORMATION_CLASS;

#endif