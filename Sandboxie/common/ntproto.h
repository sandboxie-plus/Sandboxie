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

typedef NTSTATUS (*P_NtAdjustPrivilegesToken)(
    IN  HANDLE TokenHandle,
    IN  BOOLEAN DisableAllPrivileges,
    IN  PTOKEN_PRIVILEGES NewState OPTIONAL,
    IN  ULONG BufferLength OPTIONAL,
    OUT PTOKEN_PRIVILEGES PreviousState OPTIONAL,
    OUT PULONG ReturnLength);

typedef NTSTATUS (*P_NtAlpcConnectPort)(
    OUT PHANDLE ClientPortHandle,
    IN  PUNICODE_STRING ServerPortName,
    IN  POBJECT_ATTRIBUTES ObjectAttributes,
    IN  void *AlpcPortAttributes OPTIONAL,
    IN  ULONG ConnectionFlags,
    IN  PSID ServerSid OPTIONAL,
    IN  OUT PVOID ConnectionInfo OPTIONAL,
    IN  OUT PULONG ConnectionInfoLength OPTIONAL,
    IN  PVOID InMessageBuffer OPTIONAL,
    IN  PVOID OutMessageBuffer OPTIONAL,
    IN  PLARGE_INTEGER Timeout OPTIONAL);

typedef NTSTATUS (*P_NtAlpcConnectPortEx)(
    OUT PHANDLE ClientPortHandle,
    IN  POBJECT_ATTRIBUTES ObjectAttributes1,
    IN  POBJECT_ATTRIBUTES ObjectAttributes2,
    IN  void *AlpcPortAttributes OPTIONAL,
    IN  ULONG ConnectionFlags,
    IN  PSECURITY_DESCRIPTOR ServerSd OPTIONAL,
    IN  OUT PVOID ConnectionInfo OPTIONAL,
    IN  OUT PULONG ConnectionInfoLength OPTIONAL,
    IN  PVOID InMessageBuffer OPTIONAL,
    IN  PVOID OutMessageBuffer OPTIONAL,
    IN  PLARGE_INTEGER Timeout OPTIONAL);

typedef NTSTATUS (*P_NtAlpcCreatePort)(
    OUT PHANDLE ServerPortHandle,
    IN  POBJECT_ATTRIBUTES ObjectAttributes,
    IN  OUT void *PortInformation OPTIONAL);

typedef NTSTATUS (*P_NtAlpcImpersonateClientOfPort)(
    IN  HANDLE PortHandle,
    IN  void *RequestMessage,
    IN  ULONG_PTR UnknownParameter3);

typedef NTSTATUS (*P_NtAlpcQueryInformation)(
    IN  PHANDLE PortHandle,
    IN  ULONG InformationClass,
    IN  OUT PVOID Information,
    IN  ULONG InformationLength,
    IN  ULONG_PTR UnknownParameter5);

typedef NTSTATUS (*P_NtAlpcQueryInformationMessage)(
    IN  PHANDLE PortHandle,
    IN  void *RpcMessage,
    IN  ULONG InformationClass,
    IN  OUT PVOID Information,
    IN  ULONG InformationLength,
    IN  ULONG_PTR UnknownParameter6);

typedef NTSTATUS (*P_NtAlpcSendWaitReceivePort)(
    IN  HANDLE PortHandle,
    IN  ULONG SendFlags,
    IN  void *SendMessage           OPTIONAL,
    IN  void *InMessageBuffer       OPTIONAL,
    IN  void *ReceiveBuffer         OPTIONAL,
    IN  ULONG *ReceiveBufferSize    OPTIONAL,
    IN  void *OutMessageBuffer      OPTIONAL,
    IN  LARGE_INTEGER *Timeout      OPTIONAL);

typedef NTSTATUS (*P_NtAssignProcessToJobObject)(
    IN  HANDLE JobHandle,
    IN  HANDLE ProcessHandle);

typedef NTSTATUS (*P_NtClose)(
    IN  HANDLE Handle);

typedef NTSTATUS (*P_NtCommitTransaction)(
    IN  HANDLE TransactionHandle,
    IN  ULONG_PTR UnknownParameter02);

typedef NTSTATUS (*P_NtConnectPort)(
    OUT PHANDLE ClientPortHandle,
    IN  PUNICODE_STRING ServerPortName,
    IN  PSECURITY_QUALITY_OF_SERVICE SecurityQos,
    IN  OUT PLPC_SECTION_OWNER_MEMORY ClientSharedMemory OPTIONAL,
    OUT PLPC_SECTION_MEMORY ServerSharedMemory OPTIONAL,
    OUT PULONG MaximumMessageLength OPTIONAL,
    IN  OUT PVOID ConnectionInfo OPTIONAL,
    IN  OUT PULONG ConnectionInfoLength OPTIONAL);

typedef NTSTATUS (*P_NtConnectPort)(
    OUT PHANDLE ClientPortHandle,
    IN  PUNICODE_STRING ServerPortName,
    IN  PSECURITY_QUALITY_OF_SERVICE SecurityQos,
    IN  OUT PLPC_SECTION_OWNER_MEMORY ClientSharedMemory OPTIONAL,
    OUT PLPC_SECTION_MEMORY ServerSharedMemory OPTIONAL,
    OUT PULONG MaximumMessageLength OPTIONAL,
    IN  OUT PVOID ConnectionInfo OPTIONAL,
    IN  OUT PULONG ConnectionInfoLength OPTIONAL);

typedef NTSTATUS (*P_NtContinue)(
    IN  PCONTEXT ThreadContext,
    IN  BOOLEAN RaiseAlert);

typedef NTSTATUS (*P_NtCreateEvent)(
    OUT PHANDLE EventHandle,
    IN  ACCESS_MASK DesiredAccess,
    IN  POBJECT_ATTRIBUTES ObjectAttributes,
    IN  EVENT_TYPE EventType,
    IN  BOOLEAN InitialState);

typedef NTSTATUS (*P_NtCreateFile)(
    OUT PHANDLE FileHandle,
    IN  ACCESS_MASK DesiredAccess,
    IN  POBJECT_ATTRIBUTES ObjectAttributes,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN  PLARGE_INTEGER AllocationSize OPTIONAL,
    IN  ULONG FileAttributes,
    IN  ULONG ShareAccess,
    IN  ULONG CreateDisposition,
    IN  ULONG CreateOptions,
    IN  PVOID EaBuffer OPTIONAL,
    IN  ULONG EaLength);

typedef NTSTATUS (*P_NtCreateJobObject)(
    OUT PHANDLE JobHandle,
    IN  ACCESS_MASK DesiredAccess,
    IN  POBJECT_ATTRIBUTES ObjectAttributes OPTIONAL);

typedef NTSTATUS (*P_NtCreateMailslotFile)(
    OUT PHANDLE FileHandle,
    IN  ACCESS_MASK DesiredAccess,
    IN  POBJECT_ATTRIBUTES ObjectAttributes,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN  ULONG CreateOptions,
    IN  ULONG MailslotQuota,
    IN  ULONG MaximumMessageSize,
    IN  PLARGE_INTEGER ReadTimeout);

typedef NTSTATUS (*P_NtCreateNamedPipeFile)(
    OUT PHANDLE FileHandle,
    IN  ACCESS_MASK DesiredAccess,
    IN  POBJECT_ATTRIBUTES ObjectAttributes,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN  ULONG ShareAccess,
    IN  ULONG CreateDisposition,
    IN  ULONG CreateOptions,
    IN  ULONG NamedPipeType,
    IN  ULONG ReadMode,
    IN  ULONG CompletionMode,
    IN  ULONG MaximumInstances,
    IN  ULONG InboundQuota,
    IN  ULONG OutboundQuota,
    IN  PLARGE_INTEGER DefaultTimeout OPTIONAL);

typedef NTSTATUS (*P_NtCreateKey)(
    OUT PHANDLE KeyHandle,
    IN  ACCESS_MASK DesiredAccess,
    IN  POBJECT_ATTRIBUTES ObjectAttributes,
    IN  ULONG TitleIndex,
    IN  PUNICODE_STRING Class OPTIONAL,
    IN  ULONG CreateOptions,
    OUT PULONG Disposition OPTIONAL);

typedef NTSTATUS (*P_NtCreateKeyTransacted)(
    OUT PHANDLE KeyHandle,
    IN  ACCESS_MASK DesiredAccess,
    IN  POBJECT_ATTRIBUTES ObjectAttributes,
    IN  ULONG TitleIndex,
    IN  PUNICODE_STRING Class OPTIONAL,
    IN  ULONG CreateOptions,
    OUT PULONG Disposition OPTIONAL,
    IN  PVOID Transaction);

typedef NTSTATUS (*P_NtCreateMutant)(
    OUT PHANDLE MutantHandle,
    IN  ACCESS_MASK DesiredAccess,
    IN  POBJECT_ATTRIBUTES ObjectAttributes,
    IN  BOOLEAN InitialOwner);

typedef NTSTATUS (*P_NtCreatePort)(
    OUT PHANDLE PortHandle,
    IN  POBJECT_ATTRIBUTES ObjectAttributes,
    IN  ULONG MaxConnectInfoLength,
    IN  ULONG MaxDataLength,
    IN  OUT PULONG Reserved OPTIONAL);

typedef NTSTATUS (*P_NtCreateProcess)(
    OUT PHANDLE ProcessHandle,
    IN  ACCESS_MASK DesiredAccess,
    IN  POBJECT_ATTRIBUTES ObjectAttributes OPTIONAL,
    IN  HANDLE ParentProcessHandle,
    IN  BOOLEAN InheritHandles,
    IN  HANDLE SectionHandle OPTIONAL,
    IN  HANDLE DebugPort OPTIONAL,
    IN  HANDLE ExceptionPort OPTIONAL);

typedef NTSTATUS (*P_NtCreateProcessEx)(
    OUT PHANDLE ProcessHandle,
    IN  ACCESS_MASK DesiredAccess,
    IN  POBJECT_ATTRIBUTES ObjectAttributes OPTIONAL,
    IN  HANDLE ParentProcessHandle,
    IN  BOOLEAN InheritHandles,
    IN  HANDLE SectionHandle OPTIONAL,
    IN  HANDLE DebugPort OPTIONAL,
    IN  HANDLE ExceptionPort OPTIONAL,
    IN  ULONG UnknownExtraParameter);

typedef NTSTATUS (*P_NtCreateSection)(
    OUT PHANDLE SectionHandle,
    IN  ACCESS_MASK DesiredAccess,
    IN  POBJECT_ATTRIBUTES ObjectAttributes OPTIONAL,
    IN  PLARGE_INTEGER MaximumSize OPTIONAL,
    IN  ULONG PageAttributes,
    IN  ULONG SectionAttributes,
    IN  HANDLE FileHandle OPTIONAL);

typedef NTSTATUS (*P_NtCreateSemaphore)(
    OUT PHANDLE SemaphoreHandle,
    IN  ACCESS_MASK DesiredAccess,
    IN  POBJECT_ATTRIBUTES ObjectAttributes,
    IN  ULONG InitialCount,
    IN  ULONG MaximumCount);

typedef NTSTATUS (*P_NtCreateThread)(
    OUT PHANDLE ThreadHandle,
    IN  ACCESS_MASK DesiredAccess,
    IN  POBJECT_ATTRIBUTES ObjectAttributes OPTIONAL,
    IN  HANDLE ProcessHandle,
    OUT PCLIENT_ID ClientId,
    IN  PCONTEXT ThreadContext,
    IN  PINITIAL_TEB InitialTeb,
    IN  BOOLEAN CreateSuspended);

typedef NTSTATUS (*P_NtCreateTimer)(
    OUT PHANDLE TimerHandle,
    IN  ACCESS_MASK DesiredAccess,
    IN  POBJECT_ATTRIBUTES ObjectAttributes OPTIONAL,
    IN  TIMER_TYPE TimerType);

typedef NTSTATUS (*P_NtCreateToken)(
    OUT PHANDLE TokenHandle,
    IN  ACCESS_MASK DesiredAccess,
    IN  POBJECT_ATTRIBUTES ObjectAttributes OPTIONAL,
    IN  TOKEN_TYPE TokenType,
    IN  PLUID AuthenticationId,
    IN  PLARGE_INTEGER ExpirationTime,
    IN  PTOKEN_USER User,
    IN  PTOKEN_GROUPS Groups,
    IN  PTOKEN_PRIVILEGES Privileges,
    IN  PTOKEN_OWNER Owner OPTIONAL,
    IN  PTOKEN_PRIMARY_GROUP PrimaryGroup,
    IN  PTOKEN_DEFAULT_DACL DefaultDacl OPTIONAL,
    IN  PTOKEN_SOURCE TokenSource);

typedef NTSTATUS (*P_NtSetInformationThread)(
    HANDLE          ThreadHandle,
    THREADINFOCLASS ThreadInformationClass,
    PVOID           ThreadInformation,
    ULONG           ThreadInformationLength);

typedef NTSTATUS (*P_NtCreateTransaction)(
    HANDLE *TransactionHandle,
    ACCESS_MASK DesiredAccess,
    OBJECT_ATTRIBUTES *ObjectAttributes,
    void *UnknownParameter04,
    void *UnknownParameter05,
    void *UnknownParameter06,
    void *UnknownParameter07,
    void *UnknownParameter08,
    void *UnknownParameter09,
    void *UnknownParameter10);

typedef NTSTATUS (*P_NtCreateSection)(
    OUT PHANDLE SectionHandle,
    IN  ACCESS_MASK DesiredAccess,
    IN  POBJECT_ATTRIBUTES ObjectAttributes OPTIONAL,
    IN  PLARGE_INTEGER MaximumSize OPTIONAL,
    IN  ULONG PageAttributes,
    IN  ULONG SectionAttributes,
    IN  HANDLE FileHandle OPTIONAL);

typedef NTSTATUS (*P_NtDelayExecution)(
    IN  BOOLEAN Alertable,
    IN  PLARGE_INTEGER DelayInterval);

typedef NTSTATUS (*P_NtDeleteFile)(
    IN  POBJECT_ATTRIBUTES ObjectAttributes);

typedef NTSTATUS (*P_NtDeleteKey)(
    IN  HANDLE KeyHandle);

typedef NTSTATUS (*P_NtDeleteValueKey)(
    IN  HANDLE KeyHandle,
    IN  PUNICODE_STRING ValueName);

typedef NTSTATUS (*P_NtDeleteWnfStateData)(
    IN  ULONG_PTR UnknownParameter1,
    IN  ULONG_PTR UnknownParameter2);

typedef NTSTATUS (*P_NtDuplicateObject)(
    IN  HANDLE SourceProcessHandle,
    IN  HANDLE SourceHandle,
    IN  HANDLE TargetProcessHandle,
    OUT PHANDLE TargetHandle,
    IN  ACCESS_MASK DesiredAccess,
    IN  ULONG HandleAttributes,
    IN  ULONG Options);

typedef NTSTATUS (*P_NtEnumerateKey)(
    IN  HANDLE KeyHandle,
    IN  ULONG Index,
    IN  KEY_INFORMATION_CLASS KeyInformationClass,
    OUT PVOID KeyInformation,
    IN  ULONG Length,
    OUT PULONG ResultLength);

typedef NTSTATUS (*P_NtEnumerateValueKey)(
    IN  HANDLE KeyHandle,
    IN  ULONG Index,
    IN  KEY_VALUE_INFORMATION_CLASS KeyValueInformationClass,
    OUT PVOID KeyValueInformation,
    IN  ULONG Length,
    OUT PULONG ResultLength);

typedef NTSTATUS(*P_NtDuplicateToken)(
    _In_ HANDLE ExistingTokenHandle,
    _In_ ACCESS_MASK DesiredAccess,
    _In_opt_ POBJECT_ATTRIBUTES ObjectAttributes,
    _In_ BOOLEAN EffectiveOnly,
    _In_ TOKEN_TYPE TokenType,
    _Out_ PHANDLE NewTokenHandle);

typedef NTSTATUS (*P_NtFilterToken)(
    IN  HANDLE ExistingTokenHandle,
    IN  ULONG Flags,
    IN  PTOKEN_GROUPS SidsToDisable OPTIONAL,
    IN  PTOKEN_PRIVILEGES PrivilegesToDelete OPTIONAL,
    IN  PTOKEN_GROUPS RestrictedSids OPTIONAL,
    OUT PHANDLE NewTokenHandle);

typedef NTSTATUS(*P_NtFilterTokenEx)(
    _In_ HANDLE ExistingTokenHandle,
    _In_ ULONG Flags,
    _In_opt_ PTOKEN_GROUPS SidsToDisable,
    _In_opt_ PTOKEN_PRIVILEGES PrivilegesToDelete,
    _In_opt_ PTOKEN_GROUPS RestrictedSids,
    _In_ ULONG DisableUserClaimsCount,
    _In_opt_ PUNICODE_STRING UserClaimsToDisable,
    _In_ ULONG DisableDeviceClaimsCount,
    _In_opt_ PUNICODE_STRING DeviceClaimsToDisable,
    _In_opt_ PTOKEN_GROUPS DeviceGroupsToDisable,
    _In_opt_ PVOID RestrictedUserAttributes,
    _In_opt_ PVOID RestrictedDeviceAttributes,
    _In_opt_ PTOKEN_GROUPS RestrictedDeviceGroups,
    _Out_ PHANDLE NewTokenHandle);

typedef NTSTATUS (*P_NtFlushInstructionCache)(
    IN  HANDLE ProcessHandle,
    IN  PVOID BaseAddress OPTIONAL,
    IN  ULONG Length);

typedef NTSTATUS (*P_NtFsControlFile)(
    IN  HANDLE FileHandle,
    IN  HANDLE Event OPTIONAL,
    IN  PIO_APC_ROUTINE ApcRoutine OPTIONAL,
    IN  PVOID ApcContext OPTIONAL,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN  ULONG IoControlCode,
    IN  PVOID InputBuffer OPTIONAL,
    IN  ULONG InputBufferLength,
    OUT PVOID OutputBuffer OPTIONAL,
    IN  ULONG OutputBufferLength);

typedef NTSTATUS (*P_NtDeviceIoControlFile)(
    IN  HANDLE FileHandle,
    IN  HANDLE Event OPTIONAL,
    IN  PIO_APC_ROUTINE ApcRoutine OPTIONAL,
    IN  PVOID ApcContext OPTIONAL,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN  ULONG IoControlCode,
    IN  PVOID InputBuffer OPTIONAL,
    IN  ULONG InputBufferLength,
    OUT PVOID OutputBuffer OPTIONAL,
    IN  ULONG OutputBufferLength);

typedef NTSTATUS (*P_NtImpersonateClientOfPort)(
    IN  HANDLE PortHandle,
    IN  void *RequestMessage);

typedef NTSTATUS (*P_NtImpersonateAnonymousToken)(
    IN  HANDLE ThreadHandle);

typedef NTSTATUS (*P_NtImpersonateThread)(
    IN  HANDLE ServerThreadHandle,
    IN  HANDLE ClientThreadHandle,
    IN  PSECURITY_QUALITY_OF_SERVICE SecurityQos);

typedef NTSTATUS (*P_NtLoadDriver)(
    IN  PUNICODE_STRING RegistryPath);

typedef NTSTATUS (*P_NtLoadKey)(
    IN  POBJECT_ATTRIBUTES TargetObjectAttributes,
    IN  POBJECT_ATTRIBUTES SourceObjectAttributes);

typedef NTSTATUS (*P_NtMapViewOfSection)(
    IN  HANDLE SectionHandle,
    IN  HANDLE ProcessHandle,
    IN  OUT PVOID *BaseAddress,
    IN  ULONG_PTR ZeroBits,
    IN  SIZE_T CommitSize,
    IN  OUT PLARGE_INTEGER SectionOffset OPTIONAL,
    IN  OUT PSIZE_T ViewSize,
    IN  ULONG InheritDisposition,
    IN  ULONG AllocationType,
    IN  ULONG Protect);

typedef NTSTATUS (*P_NtNotifyChangeDirectoryFile)(
    IN  HANDLE FileHandle,
    IN  HANDLE Event OPTIONAL,
    IN  PIO_APC_ROUTINE ApcRoutine OPTIONAL,
    IN  PVOID ApcContext OPTIONAL,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    OUT PVOID Buffer,
    IN  ULONG BufferSize,
    IN  ULONG CompletionFilter,
    IN  BOOLEAN WatchTree);

typedef NTSTATUS (*P_NtNotifyChangeKey)(
    IN  HANDLE KeyHandle,
    IN  HANDLE Event OPTIONAL,
    IN  PIO_APC_ROUTINE ApcRoutine OPTIONAL,
    IN  PVOID ApcContext OPTIONAL,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN  ULONG CompletionFilter,
    IN  BOOLEAN WatchTree,
    OUT PVOID Buffer,
    IN  ULONG BufferSize,
    IN  BOOLEAN Asynchronous);

typedef NTSTATUS (*P_NtNotifyChangeMultipleKeys)(
    IN  HANDLE MasterKeyHandle,
    IN  ULONG Count,
    IN  OBJECT_ATTRIBUTES SlaveObjects[],
    IN  HANDLE Event OPTIONAL,
    IN  PIO_APC_ROUTINE ApcRoutine OPTIONAL,
    IN  PVOID ApcContext OPTIONAL,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN  ULONG CompletionFilter,
    IN  BOOLEAN WatchTree,
    OUT PVOID Buffer,
    IN  ULONG BufferSize,
    IN  BOOLEAN Asynchronous);

typedef NTSTATUS (*P_NtOpenDirectoryObject)(
    OUT PHANDLE DirectoryHandle,
    IN  ACCESS_MASK DesiredAccess,
    IN  POBJECT_ATTRIBUTES ObjectAttributes);

typedef NTSTATUS (*P_NtOpenEvent)(
    OUT PHANDLE EventHandle,
    IN  ACCESS_MASK DesiredAccess,
    IN  POBJECT_ATTRIBUTES ObjectAttributes);

typedef NTSTATUS (*P_NtOpenFile)(
    OUT PHANDLE FileHandle,
    IN  ACCESS_MASK DesiredAccess,
    IN  POBJECT_ATTRIBUTES ObjectAttributes,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN  ULONG ShareAccess,
    IN  ULONG OpenOptions);

typedef NTSTATUS (*P_NtOpenJobObject)(
	OUT PHANDLE JobHandle,
	IN ACCESS_MASK DesiredAccess,
	IN POBJECT_ATTRIBUTES ObjectAttributes);

typedef NTSTATUS (*P_NtOpenKey)(
    OUT PHANDLE KeyHandle,
    IN  ACCESS_MASK DesiredAccess,
    IN  POBJECT_ATTRIBUTES ObjectAttributes);

typedef NTSTATUS (*P_NtOpenKeyEx)(
    OUT PHANDLE KeyHandle,
    IN  ACCESS_MASK DesiredAccess,
    IN  POBJECT_ATTRIBUTES ObjectAttributes,
    IN  ULONG OpenOptions);

typedef NTSTATUS (*P_NtOpenKeyTransacted)(
    OUT PHANDLE KeyHandle,
    IN  ACCESS_MASK DesiredAccess,
    IN  POBJECT_ATTRIBUTES ObjectAttributes,
    IN  PVOID Transaction);

typedef NTSTATUS (*P_NtOpenMutant)(
    OUT PHANDLE MutantHandle,
    IN  ACCESS_MASK DesiredAccess,
    IN  POBJECT_ATTRIBUTES ObjectAttributes);

typedef NTSTATUS (*P_NtOpenProcess)(
    OUT PHANDLE ProcessHandle,
    IN  ACCESS_MASK DesiredAccess,
    IN  POBJECT_ATTRIBUTES ObjectAttributes,
    IN  PCLIENT_ID ClientId);

typedef NTSTATUS (*P_NtOpenProcessToken)(
    IN  HANDLE ProcessHandle,
    IN  ACCESS_MASK DesiredAccess,
    OUT PHANDLE TokenHandle);

typedef NTSTATUS (*P_NtOpenProcessTokenEx)(
    IN  HANDLE ProcessHandle,
    IN  ACCESS_MASK DesiredAccess,
    IN  ULONG HandleAttributes,
    OUT PHANDLE TokenHandle);

typedef NTSTATUS (*P_NtOpenSemaphore)(
    OUT PHANDLE SemaphoreHandle,
    IN  ACCESS_MASK DesiredAccess,
    IN  POBJECT_ATTRIBUTES ObjectAttributes);

typedef NTSTATUS (*P_NtOpenThread)(
    OUT PHANDLE ThreadHandle,
    IN  ACCESS_MASK DesiredAccess,
    IN  POBJECT_ATTRIBUTES ObjectAttributes,
    IN  PCLIENT_ID ClientId);

typedef NTSTATUS (*P_NtOpenThreadToken)(
    IN  HANDLE ThreadHandle,
    IN  ACCESS_MASK DesiredAccess,
    IN  BOOLEAN OpenAsSelf,
    OUT PHANDLE TokenHandle);

typedef NTSTATUS (*P_NtOpenThreadTokenEx)(
    IN  HANDLE ThreadHandle,
    IN  ACCESS_MASK DesiredAccess,
    IN  BOOLEAN OpenAsSelf,
    IN  ULONG HandleAttributes,
    OUT PHANDLE TokenHandle);

typedef NTSTATUS (*P_NtOpenTransaction)(
    HANDLE *TransactionHandle,
    ACCESS_MASK DesiredAccess,
    OBJECT_ATTRIBUTES *ObjectAttributes,
    void *UnknownParameter04,
    void *UnknownParameter05);

typedef NTSTATUS (*P_NtOpenSection)(
    OUT PHANDLE SectionHandle,
    IN  ACCESS_MASK DesiredAccess,
    IN  POBJECT_ATTRIBUTES ObjectAttributes);

typedef NTSTATUS (*P_NtQueryAttributesFile)(
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    OUT PFILE_BASIC_INFORMATION FileInformation);

typedef NTSTATUS (*P_NtQueryDirectoryFile)(
    IN  HANDLE FileHandle,
    IN  HANDLE Event OPTIONAL,
    IN  PIO_APC_ROUTINE ApcRoutine OPTIONAL,
    IN  PVOID ApcContext OPTIONAL,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    OUT PVOID FileInformation,
    IN  ULONG Length,
    IN  FILE_INFORMATION_CLASS FileInformationClass,
    IN  BOOLEAN ReturnSingleEntry,
    IN  PUNICODE_STRING FileMask OPTIONAL,
    IN  BOOLEAN RestartScan);

typedef NTSTATUS(*P_NtQueryDirectoryFileEx)(
    IN  HANDLE FileHandle,
    IN  HANDLE Event OPTIONAL,
    IN  PIO_APC_ROUTINE ApcRoutine OPTIONAL,
    IN  PVOID ApcContext OPTIONAL,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    OUT PVOID FileInformation,
    IN  ULONG Length,
    IN  FILE_INFORMATION_CLASS FileInformationClass,
    IN  BOOLEAN ReturnSingleEntry,
    IN  PUNICODE_STRING FileMask OPTIONAL);

typedef NTSTATUS (*P_NtQueryFullAttributesFile)(
    IN  POBJECT_ATTRIBUTES ObjectAttributes,
    OUT PFILE_NETWORK_OPEN_INFORMATION FileInformation);

typedef NTSTATUS (*P_NtQueryInformationFile)(
    IN  HANDLE FileHandle,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    OUT PVOID FileInformation,
    IN  ULONG Length,
    IN  FILE_INFORMATION_CLASS FileInformationClass);

typedef NTSTATUS (*P_NtQueryInformationJobObject)(
    IN  HANDLE JobHandle,
    IN  JOBOBJECTINFOCLASS JobObjectInformationClass,
    OUT PVOID JobObjectInformation,
    IN  ULONG JobObjectInformationLength,
    OUT PULONG ReturnLength OPTIONAL);

typedef NTSTATUS (*P_NtQueryInformationProcess)(
    IN HANDLE ProcessHandle,
    IN PROCESSINFOCLASS ProcessInformationClass,
    OUT PVOID ProcessInformation,
    IN ULONG ProcessInformationLength,
    OUT PULONG ReturnLength OPTIONAL);

typedef NTSTATUS (*P_NtQueryInformationThread)(
    IN  HANDLE ThreadHandle,
    IN  THREADINFOCLASS ThreadInformationClass,
    OUT PVOID ThreadInformation,
    IN  ULONG ThreadInformationLength,
    OUT PULONG ReturnLength OPTIONAL);

typedef NTSTATUS (*P_NtQueryInformationToken)(
    IN  HANDLE                  TokenHandle,
    IN  TOKEN_INFORMATION_CLASS TokenInformationClass,
    OUT PVOID                   TokenInformation,
    IN  ULONG                   TokenInformationLength,
    OUT PULONG                  ReturnLength);

typedef NTSTATUS (*P_NtQuerySecurityAttributesToken)(
    IN HANDLE TokenHandle, 
    IN PUNICODE_STRING Attributes,
    IN ULONG NumberOfAttributes,
    OUT PVOID Buffer, 
    IN ULONG Length, 
    OUT PULONG ReturnLength);

typedef NTSTATUS (*P_NtAccessCheck)(
  IN PSECURITY_DESCRIPTOR SecurityDescriptor,
  IN HANDLE               ClientToken,
  IN ACCESS_MASK          DesiredAccess,
  IN PGENERIC_MAPPING     GenericMapping OPTIONAL,
  OUT PPRIVILEGE_SET      RequiredPrivilegesBuffer,
  IN OUT PULONG           BufferLength,
  OUT PACCESS_MASK        GrantedAccess,
  OUT PNTSTATUS           AccessStatus );

typedef NTSTATUS (*P_NtAccessCheckByTypeResultList) (
    PSECURITY_DESCRIPTOR SecurityDescriptor,
    PSID PrincipalSelfSid,
    HANDLE ClientToken,
    ACCESS_MASK     DesiredAccess,
    POBJECT_TYPE_LIST ObjectTypeList,
    ULONG ObjectTypeListLength,
    PGENERIC_MAPPING GenericMapping,
    PPRIVILEGE_SET  PrivilegeSet,
    PULONG PrivilegeSetLength,
    PACCESS_MASK    GrantedAccess,
    PNTSTATUS   AccessStatus 
    );

typedef NTSTATUS (*P_NtAccessCheckByType) (
    PSECURITY_DESCRIPTOR SecurityDescriptor,
    PSID PrincipalSelfSid,
    HANDLE ClientToken,
    ACCESS_MASK DesiredAccess,
    POBJECT_TYPE_LIST ObjectTypeList,
    ULONG ObjectTypeListLength,
    PGENERIC_MAPPING GenericMapping,
    PPRIVILEGE_SET PrivilegeSet,
    PULONG PrivilegeSetLength,
    PACCESS_MASK GrantedAccess,
    PNTSTATUS AccessStatus);

typedef BOOLEAN (*P_RtlEqualSid) (void * sid1, void * sid2);

typedef NTSTATUS (*P_NtQuerySystemInformation)(
    IN SYSTEM_INFORMATION_CLASS SystemInformationClass,
    OUT PVOID                   SystemInformation,
    IN  ULONG                   SystemInformationLength,
    OUT PULONG                  ReturnLength OPTIONAL);

typedef NTSTATUS (*P_NtQueryKey)(
    IN  HANDLE KeyHandle,
    IN  KEY_INFORMATION_CLASS KeyInformationClass,
    OUT PVOID KeyInformation,
    IN  ULONG Length,
    OUT PULONG ResultLength);

typedef NTSTATUS (*P_NtQueryMultipleValueKey)(
    IN  HANDLE KeyHandle,
    IN  PKEY_VALUE_ENTRY ValueEntries,
    IN  ULONG EntryCount,
    OUT PVOID ValueBuffer,
    IN  ULONG *Length,
    OUT PULONG ResultLength);

typedef NTSTATUS (*P_NtQueryObject)(
    IN  HANDLE ObjectHandle,
    IN  OBJECT_INFORMATION_CLASS ObjectInformationClass,
    OUT PVOID ObjectInformation,
    IN  ULONG Length,
    OUT PULONG ResultLength);

typedef NTSTATUS (*P_NtQuerySecurityObject)(
    IN  HANDLE Handle,
    IN  SECURITY_INFORMATION SecurityInformation,
    OUT PSECURITY_DESCRIPTOR SecurityDescriptor,
    IN  ULONG Length,
    OUT PULONG LengthNeeded);

typedef NTSTATUS (*P_NtQueryValueKey)(
    IN  HANDLE KeyHandle,
    IN  PUNICODE_STRING ValueName,
    IN  KEY_VALUE_INFORMATION_CLASS KeyValueInformationClass,
    OUT PVOID KeyValueInformation,
    IN  ULONG Length,
    OUT PULONG ResultLength);

typedef NTSTATUS (*P_NtQueryVirtualMemory)(
    IN  HANDLE ProcessHandle,
    IN  PVOID BaseAddress,
    IN  MEMORY_INFORMATION_CLASS MemoryInformationClass,
    OUT PVOID MemoryInformation,
    IN  SIZE_T MemoryInformationLength,
    OUT PSIZE_T ReturnLength OPTIONAL);

typedef NTSTATUS (*P_NtQueryVolumeInformationFile)(
    IN  HANDLE FileHandle,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    OUT PVOID FsInformation,
    IN  ULONG Length,
    IN  ULONG FsInformationClass);

typedef NTSTATUS (*P_NtReadFile)(
    IN  HANDLE FileHandle,
    IN  HANDLE Event OPTIONAL,
    IN  PIO_APC_ROUTINE ApcRoutine OPTIONAL,
    IN  PVOID ApcContext OPTIONAL,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    OUT PVOID Buffer,
    IN  ULONG Length,
    IN  PLARGE_INTEGER ByteOffset OPTIONAL,
    IN  PULONG Key OPTIONAL);

typedef NTSTATUS (*P_NtReadVirtualMemory)(
    IN  HANDLE ProcessHandle,
    IN  PVOID BaseAddress,
    OUT PVOID Buffer,
    IN  SIZE_T BufferSize,
    OUT PSIZE_T NumberOfBytesRead OPTIONAL);

typedef NTSTATUS (*P_NtRenameKey)(
    IN  HANDLE KeyHandle,
    IN  PUNICODE_STRING ReplacementName);

typedef NTSTATUS (*P_NtRequestPort)(
    IN  HANDLE PortHandle,
    IN  void *RequestMessage);

typedef NTSTATUS (*P_NtRequestWaitReplyPort)(
    IN  HANDLE PortHandle,
    IN  void *RequestMessage,
    OUT void *ReplyMessage);

typedef NTSTATUS (*P_NtResumeThread)(
    IN  HANDLE ThreadHandle,
    OUT ULONG *PreviousSuspendCount OPTIONAL);

typedef NTSTATUS (*P_NtRollbackTransaction)(
    IN  HANDLE TransactionHandle,
    IN  ULONG_PTR UnknownParameter02);

typedef NTSTATUS (*P_NtSaveKey)(
    IN  HANDLE KeyHandle,
    IN  HANDLE FileHandle);

typedef NTSTATUS (*P_NtSecureConnectPort)(
    OUT PHANDLE ClientPortHandle,
    IN  PUNICODE_STRING ServerPortName,
    IN  PSECURITY_QUALITY_OF_SERVICE SecurityQos,
    IN  OUT PLPC_SECTION_OWNER_MEMORY ClientSharedMemory OPTIONAL,
    IN  PSID ServerSid OPTIONAL,
    OUT PLPC_SECTION_MEMORY ServerSharedMemory OPTIONAL,
    OUT PULONG MaximumMessageLength OPTIONAL,
    IN  OUT PVOID ConnectionInfo OPTIONAL,
    IN  OUT PULONG ConnectionInfoLength OPTIONAL);

typedef NTSTATUS (*P_NtSetInformationFile)(
    IN  HANDLE FileHandle,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN  PVOID FileInformation,
    IN  ULONG Length,
    IN  FILE_INFORMATION_CLASS FileInformationClass);

typedef NTSTATUS (*P_NtSetInformationJobObject)(
    IN  HANDLE JobHandle,
    IN  JOBOBJECTINFOCLASS JobObjectInformationClass,
    IN  PVOID JobObjectInformation,
    IN  ULONG JobObjectInformationLength);

typedef NTSTATUS (*P_NtSetInformationProcess)(
    IN  HANDLE ProcessHandle,
    IN  PROCESSINFOCLASS ProcessInformationClass,
    IN  PVOID ProcessInformation,
    IN  ULONG ProcessInformationLength);

typedef NTSTATUS (*P_NtSetInformationToken)(
    IN  HANDLE TokenHandle,
    IN  TOKEN_INFORMATION_CLASS TokenInformationClass,
    IN  PVOID TokenInformation,
    IN  ULONG TokenInformationLength);

typedef NTSTATUS (*P_NtSetSecurityObject)(
    IN  HANDLE Handle,
    IN  SECURITY_INFORMATION SecurityInformation,
    IN  PSECURITY_DESCRIPTOR SecurityDescriptor);

typedef NTSTATUS (*P_NtSetValueKey)(
    IN  HANDLE KeyHandle,
    IN  PUNICODE_STRING ValueName,
    IN  ULONG TitleIndex OPTIONAL,
    IN  ULONG Type,
    IN  PVOID Data,
    IN  ULONG DataSize);

typedef NTSTATUS (*P_NtSuspendThread)(
    IN  HANDLE ThreadHandle,
    OUT ULONG *PreviousSuspendCount OPTIONAL);

typedef NTSTATUS (*P_NtTraceEvent)(
    IN  HANDLE TraceHandle,
    IN  ULONG Flags,
    IN  ULONG FieldSize,
    IN  PVOID Fields);

typedef NTSTATUS (*P_NtProtectVirtualMemory)(
    IN  HANDLE ProcessHandle,
    IN  OUT PVOID *BaseAddress,
    IN  OUT PSIZE_T RegionSize,
    IN  ULONG NewProtect,
    OUT PULONG OldProtect);

typedef NTSTATUS (*P_NtWriteFile)(
    IN  HANDLE FileHandle,
    IN  HANDLE Event OPTIONAL,
    IN  PIO_APC_ROUTINE ApcRoutine OPTIONAL,
    IN  PVOID ApcContext OPTIONAL,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN  PVOID Buffer,
    IN  ULONG Length,
    IN  PLARGE_INTEGER ByteOffset OPTIONAL,
    IN  PULONG Key OPTIONAL);

typedef NTSTATUS (*P_NtWriteVirtualMemory)(
    IN  HANDLE ProcessHandle,
    IN  PVOID BaseAddress,
    IN  PVOID Buffer,
    IN  SIZE_T BufferSize,
    OUT PSIZE_T NumberOfBytesWritten OPTIONAL);
