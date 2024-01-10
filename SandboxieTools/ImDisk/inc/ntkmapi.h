#ifndef VER_PRODUCTBUILD
#include <ntverp.h>
#endif

///
/// Some additional native kernel-mode API functions we use
///

//
// Ensures that we build a pre Win 2000 compatible x86 sys file
// (without ExFreePoolWithTag()). // Olof Lagerkvist
//
#ifndef _WIN64
#ifdef ExFreePool
#undef ExFreePool
#endif
#ifdef ExFreePoolWithTag
#undef ExFreePoolWithTag
#endif
#define ExFreePoolWithTag(b, t) ExFreePool(b)
#endif

#pragma warning(disable: 4996)

//
// We include some stuff from newer DDK:s here so that one
// version of the driver for all versions of Windows can
// be compiled with the Windows 2000 DDK.
//
#if (VER_PRODUCTBUILD < 2195)

#define IOCTL_DISK_GET_PARTITION_INFO_EX    CTL_CODE(IOCTL_DISK_BASE, 0x0012, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_DISK_GET_LENGTH_INFO          CTL_CODE(IOCTL_DISK_BASE, 0x0017, METHOD_BUFFERED, FILE_READ_ACCESS)
#define IOCTL_DISK_GET_PARTITION_INFO_EX    CTL_CODE(IOCTL_DISK_BASE, 0x0012, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_DISK_SET_PARTITION_INFO_EX    CTL_CODE(IOCTL_DISK_BASE, 0x0013, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)

typedef enum _PARTITION_STYLE
{
    PARTITION_STYLE_MBR,
    PARTITION_STYLE_GPT
} PARTITION_STYLE;

typedef unsigned __int64 ULONG64, *PULONG64;

typedef struct _PARTITION_INFORMATION_MBR
{
    UCHAR PartitionType;
    BOOLEAN BootIndicator;
    BOOLEAN RecognizedPartition;
    ULONG HiddenSectors;
} PARTITION_INFORMATION_MBR, *PPARTITION_INFORMATION_MBR;

typedef struct _PARTITION_INFORMATION_GPT
{
    GUID PartitionType;
    GUID PartitionId;
    ULONG64 Attributes;
    WCHAR Name[36];
} PARTITION_INFORMATION_GPT, *PPARTITION_INFORMATION_GPT;

typedef struct _PARTITION_INFORMATION_EX
{
    PARTITION_STYLE PartitionStyle;
    LARGE_INTEGER StartingOffset;
    LARGE_INTEGER PartitionLength;
    ULONG PartitionNumber;
    BOOLEAN RewritePartition;
    union
    {
        PARTITION_INFORMATION_MBR Mbr;
        PARTITION_INFORMATION_GPT Gpt;
    };
} PARTITION_INFORMATION_EX, *PPARTITION_INFORMATION_EX;

typedef struct _GET_LENGTH_INFORMATION
{
    LARGE_INTEGER Length;
} GET_LENGTH_INFORMATION, *PGET_LENGTH_INFORMATION;

typedef SET_PARTITION_INFORMATION SET_PARTITION_INFORMATION_MBR;
typedef PARTITION_INFORMATION_GPT SET_PARTITION_INFORMATION_GPT;

typedef struct _SET_PARTITION_INFORMATION_EX {
    PARTITION_STYLE PartitionStyle;
    union {
        SET_PARTITION_INFORMATION_MBR Mbr;
        SET_PARTITION_INFORMATION_GPT Gpt;
    };
} SET_PARTITION_INFORMATION_EX, *PSET_PARTITION_INFORMATION_EX;

#endif // (VER_PRODUCTBUILD < 2600)

#if (VER_PRODUCTBUILD < 3790)

#define IOCTL_STORAGE_GET_HOTPLUG_INFO        CTL_CODE(IOCTL_STORAGE_BASE, 0x0305, METHOD_BUFFERED, FILE_ANY_ACCESS)

//
// IOCTL_STORAGE_GET_HOTPLUG_INFO
//

typedef struct _STORAGE_HOTPLUG_INFO {
    ULONG Size;
    BOOLEAN MediaRemovable;
    BOOLEAN MediaHotplug;
    BOOLEAN DeviceHotplug;
    BOOLEAN WriteCacheEnableOverride;
} STORAGE_HOTPLUG_INFO, *PSTORAGE_HOTPLUG_INFO;

#endif // (VER_PRODUCTBUILD < 3790)

NTSYSAPI
NTSTATUS
NTAPI
ZwPulseEvent(IN HANDLE EventHandle,
OUT PLONG PreviousState OPTIONAL);

//
// We include some stuff from ntifs.h here so that
// the driver can be compiled with only the Win2K DDK.
//
#ifndef _NTIFS_INCLUDED_

NTSYSAPI
NTSTATUS
NTAPI
ZwSetEvent(IN HANDLE EventHandle,
OUT PLONG PreviousState OPTIONAL);

NTSYSAPI
NTSTATUS
NTAPI
ZwWaitForSingleObject(IN HANDLE Handle,
IN BOOLEAN Alertable,
IN PLARGE_INTEGER Timeout OPTIONAL);

NTSYSAPI
NTSTATUS
NTAPI
ZwAllocateVirtualMemory(IN HANDLE               ProcessHandle,
IN OUT PVOID            *BaseAddress,
IN ULONG_PTR            ZeroBits,
IN OUT PSIZE_T          RegionSize,
IN ULONG                AllocationType,
IN ULONG                Protect);

NTSYSAPI
NTSTATUS
NTAPI
ZwFreeVirtualMemory(IN HANDLE               ProcessHandle,
IN PVOID                *BaseAddress,
IN OUT PSIZE_T          RegionSize,
IN ULONG                FreeType);

NTSYSAPI
NTSTATUS
NTAPI
ZwFsControlFile(
__in HANDLE FileHandle,
__in_opt HANDLE Event,
__in_opt PIO_APC_ROUTINE ApcRoutine,
__in_opt PVOID ApcContext,
__out PIO_STATUS_BLOCK IoStatusBlock,
__in ULONG FsControlCode,
__in_bcount_opt(InputBufferLength) PVOID InputBuffer,
__in ULONG InputBufferLength,
__out_bcount_opt(OutputBufferLength) PVOID OutputBuffer,
__in ULONG OutputBufferLength
);

#define FSCTL_SET_SPARSE                CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 49, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)

typedef enum _TOKEN_TYPE {
    TokenPrimary = 1,
    TokenImpersonation
} TOKEN_TYPE;
typedef TOKEN_TYPE *PTOKEN_TYPE;

#define TOKEN_SOURCE_LENGTH 8

typedef struct _TOKEN_SOURCE {
    CHAR SourceName[TOKEN_SOURCE_LENGTH];
    LUID SourceIdentifier;
} TOKEN_SOURCE, *PTOKEN_SOURCE;

typedef struct _TOKEN_CONTROL
{
    LUID TokenId;
    LUID AuthenticationId;
    LUID ModifiedId;
    TOKEN_SOURCE TokenSource;
} TOKEN_CONTROL, *PTOKEN_CONTROL;

typedef struct _SECURITY_CLIENT_CONTEXT
{
    SECURITY_QUALITY_OF_SERVICE SecurityQos;
    PACCESS_TOKEN ClientToken;
    BOOLEAN DirectlyAccessClientToken;
    BOOLEAN DirectAccessEffectiveOnly;
    BOOLEAN ServerIsRemote;
    TOKEN_CONTROL ClientTokenControl;
} SECURITY_CLIENT_CONTEXT, *PSECURITY_CLIENT_CONTEXT;

NTKERNELAPI
NTSTATUS
SeCreateClientSecurity(IN PETHREAD Thread,
IN PSECURITY_QUALITY_OF_SERVICE QualityOfService,
IN BOOLEAN RemoteClient,
OUT PSECURITY_CLIENT_CONTEXT ClientContext);

NTKERNELAPI
VOID
SeImpersonateClient(IN PSECURITY_CLIENT_CONTEXT ClientContext,
IN PETHREAD ServerThread OPTIONAL);

NTKERNELAPI
TOKEN_TYPE
SeTokenType(IN PACCESS_TOKEN Token);

// PsRevertToSelf() removed for Windows NT 3.51 compatibility, Olof Lagerkvist.

#define SeDeleteClientSecurity(C)  				\
  ((SeTokenType((C)->ClientToken) == TokenPrimary) ?		\
   (PsDereferencePrimaryToken( (C)->ClientToken )) :		\
   (PsDereferenceImpersonationToken( (C)->ClientToken )))

#endif // _NTIFS_INCLUDED_

#define PsDereferenceImpersonationToken(T)	\
  ((ARGUMENT_PRESENT((T))) ?			\
   (ObDereferenceObject((T))) : 0)

#define PsDereferencePrimaryToken(T) (ObDereferenceObject((T)))

//
// For backward compatibility with <= Windows NT 4.0 by Bruce Engle.
//
#ifndef _WIN64
#ifdef MmGetSystemAddressForMdlSafe
#undef MmGetSystemAddressForMdlSafe
#endif
#define MmGetSystemAddressForMdlSafe(MDL, PRIORITY) \
  MmGetSystemAddressForMdlPrettySafe(MDL)

__forceinline PVOID
MmGetSystemAddressForMdlPrettySafe(PMDL Mdl)
{
    CSHORT MdlMappingCanFail;
    PVOID MappedSystemVa;

    if (Mdl == NULL)
        return NULL;

#pragma warning(suppress: 28145)
    if (Mdl->MdlFlags & (MDL_MAPPED_TO_SYSTEM_VA | MDL_SOURCE_IS_NONPAGED_POOL))
        return Mdl->MappedSystemVa;

#pragma warning(suppress: 28145)
    MdlMappingCanFail = Mdl->MdlFlags & MDL_MAPPING_CAN_FAIL;

#pragma warning(suppress: 28145)
    Mdl->MdlFlags |= MDL_MAPPING_CAN_FAIL;

#pragma warning(suppress: 28159)
    MappedSystemVa = MmMapLockedPages(Mdl, KernelMode);

    if (MdlMappingCanFail == 0)
    {
#pragma warning(suppress: 28145)
        Mdl->MdlFlags &= ~MDL_MAPPING_CAN_FAIL;
    }

    return MappedSystemVa;
}
#endif

#ifndef IOCTL_STORAGE_MANAGE_DATA_SET_ATTRIBUTES

#define IOCTL_STORAGE_MANAGE_DATA_SET_ATTRIBUTES    CTL_CODE(IOCTL_STORAGE_BASE, 0x0501, METHOD_BUFFERED, FILE_WRITE_ACCESS)

//
//  Defines the various actions
//

typedef ULONG DEVICE_DATA_MANAGEMENT_SET_ACTION;

#define DeviceDsmAction_None                0
#define DeviceDsmAction_Trim                1

//
//  Structure used to describe the list of ranges to process
//

typedef struct _DEVICE_DATA_SET_RANGE {
    LONGLONG    StartingOffset;        //in bytes,  must align to sector
    ULONGLONG   LengthInBytes;         // multiple of sector size.
} DEVICE_DATA_SET_RANGE, *PDEVICE_DATA_SET_RANGE;

//
// input structure for IOCTL_STORAGE_MANAGE_DATA_SET_ATTRIBUTES
// 1. Value ofParameterBlockOffset or ParameterBlockLength is 0 indicates that Parameter Block does not exist.
// 2. Value of DataSetRangesOffset or DataSetRangesLength is 0 indicates that DataSetRanges Block does not exist.
//     If DataSetRanges Block exists, it contains contiguous DEVICE_DATA_SET_RANGE structures.
// 3. The total size of buffer should be at least:
//      sizeof (DEVICE_MANAGE_DATA_SET_ATTRIBUTES) + ParameterBlockLength + DataSetRangesLength
//
typedef struct _DEVICE_MANAGE_DATA_SET_ATTRIBUTES {
    ULONG                               Size;                   // Size of structure DEVICE_MANAGE_DATA_SET_ATTRIBUTES
    DEVICE_DATA_MANAGEMENT_SET_ACTION   Action;

    ULONG                               Flags;                  // Global flags across all actions

    ULONG                               ParameterBlockOffset;   // must be aligned to corresponding structure alignment
    ULONG                               ParameterBlockLength;   // 0 means Parameter Block does not exist.

    ULONG                               DataSetRangesOffset;    // must be aligned to DEVICE_DATA_SET_RANGE structure alignment.
    ULONG                               DataSetRangesLength;    // 0 means DataSetRanges Block does not exist.

} DEVICE_MANAGE_DATA_SET_ATTRIBUTES, *PDEVICE_MANAGE_DATA_SET_ATTRIBUTES;

#endif

#ifndef FSCTL_FILE_LEVEL_TRIM

#define FSCTL_FILE_LEVEL_TRIM               CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 130, METHOD_BUFFERED, FILE_WRITE_DATA)

//
//======================== FSCTL_FILE_LEVEL_TRIM ===========================
//
//  Structure definitions for supporint file level trim
//

typedef struct _FILE_LEVEL_TRIM_RANGE {

    //
    //  Bytes offset from the front of the given file to trim at
    //

    ULONGLONG Offset;

    //
    //  Length in bytes to trim from the given offset
    //

    ULONGLONG Length;
} FILE_LEVEL_TRIM_RANGE, *PFILE_LEVEL_TRIM_RANGE;

//
//  Input buffer defining what ranges to trim
//

typedef struct _FILE_LEVEL_TRIM {

    //
    // Used when interacting with byte range locks. Set to zero if not SMB or
    //  similar.
    //

    ULONG Key;

    //
    // A count of how many Offset:Length pairs are given
    //

    ULONG NumRanges;

    //
    //  All the pairs.
    //

    FILE_LEVEL_TRIM_RANGE Ranges[1];

} FILE_LEVEL_TRIM, *PFILE_LEVEL_TRIM;

//
//  This is an optional output buffer
//

typedef struct _FILE_LEVEL_TRIM_OUTPUT {

    //
    //  Receives the number of input ranges
    //  that were processed
    //

    ULONG NumRangesProcessed;

} FILE_LEVEL_TRIM_OUTPUT, *PFILE_LEVEL_TRIM_OUTPUT;

#endif

#if (VER_PRODUCTBUILD < 7600)

typedef VOID
KSTART_ROUTINE(IN PVOID StartContext);

#endif

#ifndef __drv_maxIRQL
#define __drv_maxIRQL(i)
#endif
#ifndef __drv_when
#define __drv_when(c,a)
#endif
#ifndef __drv_savesIRQLGlobal
#define __drv_savesIRQLGlobal(t,o)
#endif
#ifndef __drv_setsIRQL
#define __drv_setsIRQL(i)
#endif
#ifndef __drv_acquiresExclusiveResource
#define __drv_acquiresExclusiveResource(t)
#endif
#ifndef __drv_releasesExclusiveResource
#define __drv_releasesExclusiveResource(t)
#endif
#ifndef __drv_requiresIRQL
#define __drv_requiresIRQL(i)
#endif
#ifndef __drv_dispatchType
#define __drv_dispatchType(f)
#endif
#ifndef _Dispatch_type_
#define _Dispatch_type_ __drv_dispatchType
#endif
#ifndef _Inout_
#define _Inout_
#endif
