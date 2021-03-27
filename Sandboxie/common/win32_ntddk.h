/*
 * Copyright 2004-2020 Sandboxie Holdings, LLC 
 * Copyright 2020 David Xanatos, xanasoft.com
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


#ifndef _WIN32_NTDDK_
#define _WIN32_NTDDK_

#ifdef __cplusplus
extern "C" {
#endif

#include <ntstatus.h>
#define WIN32_NO_STATUS
typedef long NTSTATUS;

//---------------------------------------------------------------------------


typedef LONG NTSTATUS, *PNTSTATUS;

#ifdef HATE_RONEN
#define STATUS_SUCCESS                   ((NTSTATUS)0x00000000L)
#define STATUS_WAIT_0                    ((NTSTATUS)0x00000000L)
#define STATUS_WAIT_1                    ((NTSTATUS)0x00000001L)
#define STATUS_WAIT_2                    ((NTSTATUS)0x00000002L)
#define STATUS_WAIT_63                   ((NTSTATUS)0x0000003FL)
#define STATUS_TIMEOUT                   ((NTSTATUS)0x00000102L)
#define STATUS_PENDING                   ((NTSTATUS)0x00000103L)
#define STATUS_REPARSE                   ((NTSTATUS)0x00000104L)
#define STATUS_MORE_ENTRIES              ((NTSTATUS)0x00000105L)
#define STATUS_NOT_ALL_ASSIGNED          ((NTSTATUS)0x00000106L)
#define STATUS_BUFFER_OVERFLOW           ((NTSTATUS)0x80000005L)
#define STATUS_NO_MORE_FILES             ((NTSTATUS)0x80000006L)
#define STATUS_NO_MORE_ENTRIES           ((NTSTATUS)0x8000001AL)
#define STATUS_UNSUCCESSFUL              ((NTSTATUS)0xC0000001L)
#define STATUS_NOT_IMPLEMENTED           ((NTSTATUS)0xC0000002L)
#define STATUS_INVALID_INFO_CLASS        ((NTSTATUS)0xC0000003L)
#define STATUS_INFO_LENGTH_MISMATCH      ((NTSTATUS)0xC0000004L)
#define STATUS_ACCESS_VIOLATION          ((NTSTATUS)0xC0000005L)
#define STATUS_BAD_INITIAL_STACK         ((NTSTATUS)0xC0000009L)
#define STATUS_BAD_INITIAL_PC            ((NTSTATUS)0xC000000AL)
#define STATUS_INVALID_CID               ((NTSTATUS)0xC000000BL)
#define STATUS_INVALID_HANDLE            ((NTSTATUS)0xC0000008L)
#define STATUS_INVALID_PARAMETER         ((NTSTATUS)0xC000000DL)
#define STATUS_NO_SUCH_DEVICE            ((NTSTATUS)0xC000000EL)
#define STATUS_NO_SUCH_FILE              ((NTSTATUS)0xC000000FL)
#define STATUS_INVALID_DEVICE_REQUEST    ((NTSTATUS)0xC0000010L)
#define STATUS_END_OF_FILE               ((NTSTATUS)0xC0000011L)
#define STATUS_NO_MEDIA_IN_DEVICE        ((NTSTATUS)0xC0000013L)
#define STATUS_INVALID_SYSTEM_SERVICE    ((NTSTATUS)0xC000001CL)
#define STATUS_ALREADY_COMMITTED         ((NTSTATUS)0xC0000021L)
#define STATUS_ACCESS_DENIED             ((NTSTATUS)0xC0000022L)
#define STATUS_BUFFER_TOO_SMALL          ((NTSTATUS)0xC0000023L)
#define STATUS_OBJECT_TYPE_MISMATCH      ((NTSTATUS)0xC0000024L)
#define STATUS_NONCONTINUABLE_EXCEPTION  ((NTSTATUS)0xC0000025L)
#define STATUS_PORT_MESSAGE_TOO_LONG     ((NTSTATUS)0xC000002FL)
#define STATUS_OBJECT_NAME_INVALID       ((NTSTATUS)0xC0000033L)
#define STATUS_OBJECT_NAME_NOT_FOUND     ((NTSTATUS)0xC0000034L)
#define STATUS_OBJECT_NAME_COLLISION     ((NTSTATUS)0xC0000035L)
#define STATUS_OBJECT_PATH_INVALID       ((NTSTATUS)0xC0000039L)
#define STATUS_OBJECT_PATH_NOT_FOUND     ((NTSTATUS)0xC000003AL)
#define STATUS_OBJECT_PATH_SYNTAX_BAD    ((NTSTATUS)0xC000003BL)
#define STATUS_PORT_CONNECTION_REFUSED   ((NTSTATUS)0xC0000041L)
#define STATUS_SHARING_VIOLATION         ((NTSTATUS)0xC0000043L)
#define STATUS_DELETE_PENDING            ((NTSTATUS)0xC0000056L)
#define STATUS_NO_IMPERSONATION_TOKEN    ((NTSTATUS)0xC000005CL)
#define STATUS_PRIVILEGE_NOT_HELD        ((NTSTATUS)0xC0000061L)
#define STATUS_LAST_ADMIN                ((NTSTATUS)0xC0000069L)
#define STATUS_WRONG_PASSWORD            ((NTSTATUS)0xC000006AL)
#define STATUS_ACCOUNT_RESTRICTION       ((NTSTATUS)0xC000006EL)
#define STATUS_INVALID_IMAGE_FORMAT      ((NTSTATUS)0xC000007BL)
#define STATUS_NO_TOKEN                  ((NTSTATUS)0xC000007CL)
#define STATUS_DISK_FULL                 ((NTSTATUS)0xC000007FL)
#define STATUS_SERVER_DISABLED           ((NTSTATUS)0xC0000080L)
#define STATUS_RESOURCE_NAME_NOT_FOUND   ((NTSTATUS)0xC000008BL)
#define STATUS_INSUFFICIENT_RESOURCES    ((NTSTATUS)0xC000009AL)
#define STATUS_DEVICE_NOT_READY          ((NTSTATUS)0xC00000A3L)
#define STATUS_BAD_IMPERSONATION_LEVEL   ((NTSTATUS)0xC00000A5L)
#define STATUS_CANT_OPEN_ANONYMOUS       ((NTSTATUS)0xC00000A6L)
#define STATUS_FILE_IS_A_DIRECTORY       ((NTSTATUS)0xC00000BAL)
#define STATUS_NOT_SUPPORTED             ((NTSTATUS)0xC00000BBL)
#define STATUS_NOT_SAME_DEVICE           ((NTSTATUS)0xC00000D4L)
#define STATUS_DIRECTORY_NOT_EMPTY       ((NTSTATUS)0xC0000101L)
#define STATUS_NOT_A_DIRECTORY           ((NTSTATUS)0xC0000103L)
#define STATUS_NAME_TOO_LONG             ((NTSTATUS)0xC0000106L)
#define STATUS_CONNECTION_IN_USE         ((NTSTATUS)0xC0000108L)
#define STATUS_PROCESS_IS_TERMINATING    ((NTSTATUS)0xC000010AL)
#define STATUS_IMAGE_ALREADY_LOADED      ((NTSTATUS)0xC000010EL)
#define STATUS_CANCELLED                 ((NTSTATUS)0xC0000120L)
#define STATUS_CANNOT_DELETE             ((NTSTATUS)0xC0000121L)
#define STATUS_DLL_NOT_FOUND             ((NTSTATUS)0xC0000135L)
#define STATUS_ORDINAL_NOT_FOUND         ((NTSTATUS)0xC0000138L)
#define STATUS_ENTRYPOINT_NOT_FOUND      ((NTSTATUS)0xC0000139L)
#define STATUS_DLL_INIT_FAILED           ((NTSTATUS)0xC0000142L)
#define STATUS_INVALID_LEVEL             ((NTSTATUS)0xC0000148L)
#define STATUS_REGISTRY_CORRUPT          ((NTSTATUS)0xC000014CL)
#define STATUS_REGISTRY_IO_FAILED        ((NTSTATUS)0xC000014DL)
#define STATUS_LOGON_NOT_GRANTED         ((NTSTATUS)0xC0000155L)
#define STATUS_KEY_DELETED               ((NTSTATUS)0xC000017CL)
#define STATUS_CHILD_MUST_BE_VOLATILE    ((NTSTATUS)0xC0000181L)
#define STATUS_REPLY_MESSAGE_MISMATCH    ((NTSTATUS)0xC000021FL)
#define STATUS_LICENSE_QUOTA_EXCEEDED    ((NTSTATUS)0xC0000259L)
#define STATUS_EVALUATION_EXPIRATION     ((NTSTATUS)0xC0000268L)
#define STATUS_SERVER_SID_MISMATCH       ((NTSTATUS)0xC00002A0L)
#define STATUS_ELEVATION_REQUIRED        ((NTSTATUS)0xC000042CL)
#define STATUS_SXS_CANT_GEN_ACTCTX       ((NTSTATUS)0xC0150002L)

#endif


#define NT_SUCCESS(Status) ((NTSTATUS)(Status) >= 0)

#define NT_STATUS_CHECK(status) \
    if (! NT_SUCCESS(status)) return (status);


//---------------------------------------------------------------------------

#define ERROR_ELEVATION_REQUIRED         740L

//---------------------------------------------------------------------------

typedef struct _UNICODE_STRING {
    USHORT Length;
    USHORT MaximumLength;
    WCHAR *Buffer;
} UNICODE_STRING;
typedef UNICODE_STRING *PUNICODE_STRING;
typedef const UNICODE_STRING *PCUNICODE_STRING;
#define UNICODE_NULL ((WCHAR)0)

typedef struct _ANSI_STRING {
    USHORT Length;
    USHORT MaximumLength;
    UCHAR *Buffer;
} ANSI_STRING;
typedef ANSI_STRING *PANSI_STRING;
typedef const ANSI_STRING *PCANSI_STRING;

typedef struct _UNICODE_STRING64 {
    USHORT Length;
    USHORT MaximumLength;
    __declspec(align(8)) unsigned __int64 Buffer;
} UNICODE_STRING64;

//---------------------------------------------------------------------------

#define OBJ_INHERIT             0x00000002L
#define OBJ_PERMANENT           0x00000010L
#define OBJ_EXCLUSIVE           0x00000020L
#define OBJ_CASE_INSENSITIVE    0x00000040L
#define OBJ_OPENIF              0x00000080L
#define OBJ_OPENLINK            0x00000100L
#define OBJ_KERNEL_HANDLE       0x00000200L
#define OBJ_FORCE_ACCESS_CHECK  0x00000400L
#define OBJ_VALID_ATTRIBUTES    0x000007F2L

typedef struct _OBJECT_ATTRIBUTES {
    ULONG Length;
    HANDLE RootDirectory;
    PUNICODE_STRING ObjectName;
    ULONG Attributes;
    PVOID SecurityDescriptor;        // Points to type SECURITY_DESCRIPTOR
    PVOID SecurityQualityOfService;  // Points to type SECURITY_QUALITY_OF_SERVICE
} OBJECT_ATTRIBUTES;
typedef OBJECT_ATTRIBUTES *POBJECT_ATTRIBUTES;
typedef CONST OBJECT_ATTRIBUTES *PCOBJECT_ATTRIBUTES;

#define InitializeObjectAttributes( p, n, a, r, s ) { \
    (p)->Length = sizeof( OBJECT_ATTRIBUTES );          \
    (p)->RootDirectory = r;                             \
    (p)->Attributes = a;                                \
    (p)->ObjectName = n;                                \
    (p)->SecurityDescriptor = s;                        \
    (p)->SecurityQualityOfService = NULL;               \
    }

//---------------------------------------------------------------------------

#define PAGE_SIZE 4096

//---------------------------------------------------------------------------

typedef enum _OBJECT_INFORMATION_CLASS {
    ObjectBasicInformation,
    ObjectNameInformation,
    ObjectTypeInformation,
    ObjectAllTypesInformation,
    ObjectDataInformation
} OBJECT_INFORMATION_CLASS;

typedef struct _OBJECT_BASIC_INFORMATION {
    ULONG           Attributes;
    ACCESS_MASK     DesiredAccess;
    ULONG           HandleCount;
    ULONG           PointerCount;
    ULONG           PagedPoolUsage;
    ULONG           NonPagedPoolUsage;
    ULONG           Reserved[3];
    ULONG           NameInformationLength;
    ULONG           TypeInformationLength;
    ULONG           SecurityDescriptorLength;
    LARGE_INTEGER   CreationTime;
} OBJECT_BASIC_INFORMATION, *POBJECT_BASIC_INFORMATION;

typedef struct _OBJECT_NAME_INFORMATION {
    UNICODE_STRING Name;
} OBJECT_NAME_INFORMATION, *POBJECT_NAME_INFORMATION;

typedef struct __PUBLIC_OBJECT_TYPE_INFORMATION {

    UNICODE_STRING TypeName;

    ULONG Reserved [22];    // reserved for internal use

    BYTE  ExtraPadding[48]; // NtQueryObject often requires more space than MSDN says

} PUBLIC_OBJECT_TYPE_INFORMATION, *PPUBLIC_OBJECT_TYPE_INFORMATION;

__declspec(dllimport) NTSTATUS __stdcall
NtQueryObject(
    IN HANDLE ObjectHandle OPTIONAL,
    IN OBJECT_INFORMATION_CLASS ObjectInformationClass,
    OUT PVOID ObjectInformation,
    IN ULONG Length,
    OUT PULONG ResultLength);

__declspec(dllimport) NTSTATUS __stdcall
NtClose(
    IN HANDLE                       FileHandle
);

//---------------------------------------------------------------------------

__declspec(dllimport) NTSTATUS __stdcall
NtQuerySecurityObject(
    IN  HANDLE Handle,
    IN  SECURITY_INFORMATION SecurityInformation,
    OUT PSECURITY_DESCRIPTOR SecurityDescriptor,
    IN  ULONG Length,
    OUT PULONG LengthNeeded);

__declspec(dllimport) NTSTATUS __stdcall
NtSetSecurityObject(
    IN  HANDLE ObjectHandle OPTIONAL,
    IN  SECURITY_INFORMATION SecurityInformation,
    IN  PSECURITY_DESCRIPTOR SecurityDescriptor);

__declspec(dllimport) NTSTATUS __stdcall
RtlSetDaclSecurityDescriptor(
    IN  OUT PSECURITY_DESCRIPTOR  SecurityDescriptor,
    IN  BOOLEAN  DaclPresent,
    IN  PACL  Dacl  OPTIONAL,
    IN  BOOLEAN  DaclDefaulted  OPTIONAL
    );

__declspec(dllimport) NTSTATUS __stdcall
RtlSetSaclSecurityDescriptor(
    IN  OUT PSECURITY_DESCRIPTOR  SecurityDescriptor,
    IN  BOOLEAN  SaclPresent,
    IN  PACL  Sacl  OPTIONAL,
    IN  BOOLEAN  SaclDefaulted  OPTIONAL
    );

__declspec(dllimport) NTSTATUS __stdcall
RtlGetDaclSecurityDescriptor(
    IN  PSECURITY_DESCRIPTOR  SecurityDescriptor,
    OUT PBOOLEAN  DaclPresent,
    OUT PACL  *Dacl  OPTIONAL,
    OUT PBOOLEAN  DaclDefaulted
    );

//---------------------------------------------------------------------------

#define DIRECTORY_QUERY                 (0x0001)
#define DIRECTORY_TRAVERSE              (0x0002)
#define DIRECTORY_CREATE_OBJECT         (0x0004)
#define DIRECTORY_CREATE_SUBDIRECTORY   (0x0008)

#define DIRECTORY_ALL_ACCESS (STANDARD_RIGHTS_REQUIRED | 0xF)

typedef struct _OBJECT_DIRECTORY_INFORMATION {
    UNICODE_STRING Name;
    UNICODE_STRING TypeName;
} OBJECT_DIRECTORY_INFORMATION, *POBJECT_DIRECTORY_INFORMATION;

__declspec(dllimport) NTSTATUS __stdcall
NtCreateDirectoryObject(
    OUT PHANDLE             DirectoryHandle,
    IN ACCESS_MASK          DesiredAccess,
    IN POBJECT_ATTRIBUTES   ObjectAttributes
);

__declspec(dllimport) NTSTATUS __stdcall
NtOpenDirectoryObject(
    OUT PHANDLE             DirectoryHandle,
    IN ACCESS_MASK          DesiredAccess,
    IN POBJECT_ATTRIBUTES   ObjectAttributes
);

__declspec(dllimport) NTSTATUS __stdcall
NtQueryDirectoryObject(
    IN  HANDLE              DirectoryHandle,
    OUT PVOID               Buffer,
    IN  ULONG               Length,
    IN  BOOLEAN             ReturnSingleEntry,
    IN  BOOLEAN             RestartScan,
    IN  OUT PULONG          Context,
    OUT PULONG              ReturnLength
);


//---------------------------------------------------------------------------

typedef struct _IO_STATUS_BLOCK {
    union {
        NTSTATUS Status;
        PVOID Pointer;
    };
    ULONG_PTR Information;
} IO_STATUS_BLOCK, *PIO_STATUS_BLOCK;

#define FILE_SUPERSEDE                  0x00000000
#define FILE_OPEN                       0x00000001
#define FILE_CREATE                     0x00000002
#define FILE_OPEN_IF                    0x00000003
#define FILE_OVERWRITE                  0x00000004
#define FILE_OVERWRITE_IF               0x00000005

#define FILE_SUPERSEDED                 0x00000000
#define FILE_OPENED                     0x00000001
#define FILE_CREATED                    0x00000002
#define FILE_OVERWRITTEN                0x00000003
#define FILE_EXISTS                     0x00000004
#define FILE_DOES_NOT_EXIST             0x00000005

#define FILE_DIRECTORY_FILE                     0x00000001
#define FILE_SYNCHRONOUS_IO_ALERT               0x00000010
#define FILE_SYNCHRONOUS_IO_NONALERT            0x00000020
#define FILE_NON_DIRECTORY_FILE                 0x00000040
#define FILE_DELETE_ON_CLOSE                    0x00001000
#define FILE_OPEN_BY_FILE_ID                    0x00002000
#define FILE_OPEN_FOR_BACKUP_INTENT             0x00004000
#define FILE_RESERVE_OPFILTER                   0x00100000
#define FILE_OPEN_REPARSE_POINT                 0x00200000
#define FILE_OPEN_NO_RECALL                     0x00400000
#define FILE_OPEN_FOR_FREE_SPACE_QUERY          0x00800000

#define FILE_ATTRIBUTE_DIRECTORY            0x00000010  // winnt
#define FILE_ATTRIBUTE_NORMAL               0x00000080  // winnt

#define FILE_SHARE_VALID_FLAGS          0x00000007

#define IO_REPARSE_TAG_SYMLINK          (0xA000000CL)
#define SYMLINK_FLAG_RELATIVE           1

typedef struct _REPARSE_DATA_BUFFER {
    ULONG  ReparseTag;
    USHORT ReparseDataLength;
    USHORT Reserved;
    union {
        struct {
            USHORT SubstituteNameOffset;
            USHORT SubstituteNameLength;
            USHORT PrintNameOffset;
            USHORT PrintNameLength;
            ULONG  Flags;
            WCHAR  PathBuffer[1];
        } SymbolicLinkReparseBuffer;
        struct {
            USHORT SubstituteNameOffset;
            USHORT SubstituteNameLength;
            USHORT PrintNameOffset;
            USHORT PrintNameLength;
            WCHAR  PathBuffer[1];
        } MountPointReparseBuffer;
        struct {
            UCHAR  DataBuffer[1];
        } GenericReparseBuffer;
    };
} REPARSE_DATA_BUFFER, *PREPARSE_DATA_BUFFER;

typedef enum _FILE_INFORMATION_CLASS {
// end_wdm
    FileDirectoryInformation         = 1,
    FileFullDirectoryInformation,   // 2
    FileBothDirectoryInformation,   // 3
    FileBasicInformation,           // 4  wdm
    FileStandardInformation,        // 5  wdm
    FileInternalInformation,        // 6
    FileEaInformation,              // 7
    FileAccessInformation,          // 8
    FileNameInformation,            // 9
    FileRenameInformation,          // 10
    FileLinkInformation,            // 11
    FileNamesInformation,           // 12
    FileDispositionInformation,     // 13
    FilePositionInformation,        // 14 wdm
    FileFullEaInformation,          // 15
    FileModeInformation,            // 16
    FileAlignmentInformation,       // 17
    FileAllInformation,             // 18
    FileAllocationInformation,      // 19
    FileEndOfFileInformation,       // 20 wdm
    FileAlternateNameInformation,   // 21
    FileStreamInformation,          // 22
    FilePipeInformation,            // 23
    FilePipeLocalInformation,       // 24
    FilePipeRemoteInformation,      // 25
    FileMailslotQueryInformation,   // 26
    FileMailslotSetInformation,     // 27
    FileCompressionInformation,     // 28
    FileObjectIdInformation,        // 29
    FileCompletionInformation,      // 30
    FileMoveClusterInformation,     // 31
    FileQuotaInformation,           // 32
    FileReparsePointInformation,    // 33
    FileNetworkOpenInformation,     // 34
    FileAttributeTagInformation,    // 35
    FileTrackingInformation,        // 36
    FileIdBothDirectoryInformation, // 37
    FileIdFullDirectoryInformation, // 38
    FileValidDataLengthInformation, // 39
    FileShortNameInformation,       // 40
    FileIoCompletionNotificationInformation, // 41
    FileIoStatusBlockRangeInformation,       // 42
    FileIoPriorityHintInformation,           // 43
    FileSfioReserveInformation,              // 44
    FileSfioVolumeInformation,               // 45
    FileHardLinkInformation,                 // 46
    FileProcessIdsUsingFileInformation,      // 47
    FileNormalizedNameInformation,           // 48
    FileNetworkPhysicalNameInformation,      // 49
    FileIdGlobalTxDirectoryInformation,      // 50
    FileIsRemoteDeviceInformation,           // 51
    FileAttributeCacheInformation,           // 52
    FileNumaNodeInformation,                 // 53
    FileStandardLinkInformation,             // 54
    FileRemoteProtocolInformation,           // 55
    FileRenameInformationBypassAccessCheck,  // 56
    FileLinkInformationBypassAccessCheck,    // 57
    FileVolumeNameInformation,               // 58
    FileIdInformation,                       // 59
    FileIdExtdDirectoryInformation,          // 60
    FileReplaceCompletionInformation,
    FileHardLinkFullIdInformation,
    FileIdExtdBothDirectoryInformation,
    FileDispositionInformationEx,
    FileRenameInformationEx,                 // 65
    FileRenameInformationExBypassAccessCheck,
    FileMaximumInformation
} FILE_INFORMATION_CLASS, *PFILE_INFORMATION_CLASS;

// FileDirectoryInformation
typedef struct _FILE_DIRECTORY_INFORMATION {
    ULONG           NextEntryOffset;
    ULONG           FileIndex;
    LARGE_INTEGER   CreationTime;
    LARGE_INTEGER   LastAccessTime;
    LARGE_INTEGER   LastWriteTime;
    LARGE_INTEGER   ChangeTime;
    LARGE_INTEGER   EndOfFile;
    LARGE_INTEGER   AllocationSize;
    ULONG           FileAttributes;
    ULONG           FileNameLength;
    WCHAR           FileName[1];
} FILE_DIRECTORY_INFORMATION, *PFILE_DIRECTORY_INFORMATION;

// FileFullDirectoryInformation
typedef struct _FILE_FULL_DIRECTORY_INFORMATION {
    ULONG           NextEntryOffset;
    ULONG           FileIndex;
    LARGE_INTEGER   CreationTime;
    LARGE_INTEGER   LastAccessTime;
    LARGE_INTEGER   LastWriteTime;
    LARGE_INTEGER   ChangeTime;
    LARGE_INTEGER   EndOfFile;
    LARGE_INTEGER   AllocationSize;
    ULONG           FileAttributes;
    ULONG           FileNameLength;
    ULONG           EaInformationLength;
    WCHAR           FileName[1];
} FILE_FULL_DIRECTORY_INFORMATION, *PFILE_FULL_DIRECTORY_INFORMATION;

// FileBothDirectoryInformation
typedef struct _FILE_BOTH_DIRECTORY_INFORMATION {
    ULONG           NextEntryOffset;
    ULONG           FileIndex;
    LARGE_INTEGER   CreationTime;
    LARGE_INTEGER   LastAccessTime;
    LARGE_INTEGER   LastWriteTime;
    LARGE_INTEGER   ChangeTime;
    LARGE_INTEGER   EndOfFile;
    LARGE_INTEGER   AllocationSize;
    ULONG           FileAttributes;
    ULONG           FileNameLength;
    ULONG           EaInformationLength;
    CCHAR           ShortNameLength;
    WCHAR           ShortName[12];
    WCHAR           FileName[1];
} FILE_BOTH_DIRECTORY_INFORMATION, *PFILE_BOTH_DIRECTORY_INFORMATION;

// FileNamesInformation
typedef struct _FILE_NAMES_INFORMATION {
    ULONG NextEntryOffset;
    ULONG FileIndex;
    ULONG FileNameLength;
    WCHAR FileName[1];
} FILE_NAMES_INFORMATION, *PFILE_NAMES_INFORMATION;

// FileBasicInformation
typedef struct _FILE_BASIC_INFORMATION {
    LARGE_INTEGER CreationTime;
    LARGE_INTEGER LastAccessTime;
    LARGE_INTEGER LastWriteTime;
    LARGE_INTEGER ChangeTime;
    ULONG FileAttributes;
} FILE_BASIC_INFORMATION, *PFILE_BASIC_INFORMATION;

// FileNameInformation
typedef struct _FILE_NAME_INFORMATION {
    ULONG FileNameLength;
    WCHAR FileName[1];
} FILE_NAME_INFORMATION, *PFILE_NAME_INFORMATION;

// FileRenameInformation
typedef struct _FILE_RENAME_INFORMATION {
    BOOLEAN ReplaceIfExists;
    HANDLE  RootDirectory;
    ULONG   FileNameLength;
    WCHAR   FileName[1];
} FILE_RENAME_INFORMATION, *PFILE_RENAME_INFORMATION;

// FileShortNameInformation
typedef struct _FILE_SHORT_NAME_INFORMATION {
    ULONG FileNameLength;
    WCHAR FileName[12];
} FILE_SHORT_NAME_INFORMATION, *PFILE_SHORT_NAME_INFORMATION;

// FileDispositionInformation
typedef struct _FILE_DISPOSITION_INFORMATION {
    BOOLEAN DeleteFile;
} FILE_DISPOSITION_INFORMATION, *PFILE_DISPOSITION_INFORMATION;

// FilePositionInformation
typedef struct _FILE_POSITION_INFORMATION {
    LARGE_INTEGER CurrentByteOffset;
} FILE_POSITION_INFORMATION, *PFILE_POSITION_INFORMATION;

// FileStreamInformation
typedef struct _FILE_STREAM_INFORMATION {
    ULONG NextEntryOffset;
    ULONG StreamNameLength;
    LARGE_INTEGER StreamSize;
    LARGE_INTEGER StreamAllocationSize;
    WCHAR StreamName[1];
} FILE_STREAM_INFORMATION, *PFILE_STREAM_INFORMATION;

// FileNetworkOpenInformation
typedef struct _FILE_NETWORK_OPEN_INFORMATION {
    LARGE_INTEGER CreationTime;
    LARGE_INTEGER LastAccessTime;
    LARGE_INTEGER LastWriteTime;
    LARGE_INTEGER ChangeTime;
    LARGE_INTEGER AllocationSize;
    LARGE_INTEGER EndOfFile;
    ULONG FileAttributes;
} FILE_NETWORK_OPEN_INFORMATION, *PFILE_NETWORK_OPEN_INFORMATION;

// FileAttributeTagInformation
typedef struct _FILE_ATTRIBUTE_TAG_INFORMATION {
    ULONG FileAttributes;
    ULONG ReparseTag;
} FILE_ATTRIBUTE_TAG_INFORMATION, *PFILE_ATTRIBUTE_TAG_INFORMATION;

// FileIdBothDirectoryInformation
typedef struct _FILE_ID_BOTH_DIR_INFORMATION {
    ULONG           NextEntryOffset;
    ULONG           FileIndex;
    LARGE_INTEGER   CreationTime;
    LARGE_INTEGER   LastAccessTime;
    LARGE_INTEGER   LastWriteTime;
    LARGE_INTEGER   ChangeTime;
    LARGE_INTEGER   EndOfFile;
    LARGE_INTEGER   AllocationSize;
    ULONG           FileAttributes;
    ULONG           FileNameLength;
    ULONG           EaInformationLength;
    CCHAR           ShortNameLength;
    WCHAR           ShortName[12];
    LARGE_INTEGER   FileId;
    WCHAR           FileName[1];
} FILE_ID_BOTH_DIR_INFORMATION, *PFILE_ID_BOTH_DIR_INFORMATION;

// FileIdFullDirectoryInformation
typedef struct _FILE_ID_FULL_DIR_INFORMATION {
    ULONG           NextEntryOffset;
    ULONG           FileIndex;
    LARGE_INTEGER   CreationTime;
    LARGE_INTEGER   LastAccessTime;
    LARGE_INTEGER   LastWriteTime;
    LARGE_INTEGER   ChangeTime;
    LARGE_INTEGER   EndOfFile;
    LARGE_INTEGER   AllocationSize;
    ULONG           FileAttributes;
    ULONG           FileNameLength;
    ULONG           EaInformationLength;
    LARGE_INTEGER   FileId;
    WCHAR           FileName[1];
} FILE_ID_FULL_DIR_INFORMATION, *PFILE_ID_FULL_DIR_INFORMATION;

// FileInternalInformation
typedef struct _FILE_INTERNAL_INFORMATION {
    LARGE_INTEGER IndexNumber;
} FILE_INTERNAL_INFORMATION, *PFILE_INTERNAL_INFORMATION;

// FileStandardInformation
typedef struct _FILE_STANDARD_INFORMATION {
    LARGE_INTEGER AllocationSize;
    LARGE_INTEGER EndOfFile;
    ULONG NumberOfLinks;
    BOOLEAN DeletePending;
    BOOLEAN Directory;
} FILE_STANDARD_INFORMATION, *PFILE_STANDARD_INFORMATION;

// FileEaInformation
typedef struct _FILE_EA_INFORMATION {
    ULONG EaSize;
} FILE_EA_INFORMATION, *PFILE_EA_INFORMATION;

// FileAccessInformation
typedef struct _FILE_ACCESS_INFORMATION {
    ACCESS_MASK AccessFlags;
} FILE_ACCESS_INFORMATION, *PFILE_ACCESS_INFORMATION;

// FileModeInformation
typedef struct _FILE_MODE_INFORMATION {
    ULONG Mode;
} FILE_MODE_INFORMATION, *PFILE_MODE_INFORMATION;

// FileAlignmentInformation
typedef struct _FILE_ALIGNMENT_INFORMATION {
    ULONG AlignmentRequirement;
} FILE_ALIGNMENT_INFORMATION, *PFILE_ALIGNMENT_INFORMATION;

// FileAllInformation
typedef struct _FILE_ALL_INFORMATION {
    FILE_BASIC_INFORMATION BasicInformation;
    FILE_STANDARD_INFORMATION StandardInformation;
    FILE_INTERNAL_INFORMATION InternalInformation;
    FILE_EA_INFORMATION EaInformation;
    FILE_ACCESS_INFORMATION AccessInformation;
    FILE_POSITION_INFORMATION PositionInformation;
    FILE_MODE_INFORMATION ModeInformation;
    FILE_ALIGNMENT_INFORMATION AlignmentInformation;
    FILE_NAME_INFORMATION NameInformation;
} FILE_ALL_INFORMATION, *PFILE_ALL_INFORMATION;

__declspec(dllimport) NTSTATUS __stdcall
NtCreateFile(
    OUT PHANDLE                     FileHandle,
    IN ACCESS_MASK                  DesiredAccess,
    IN POBJECT_ATTRIBUTES           ObjectAttributes,
    OUT PIO_STATUS_BLOCK            IoStatusBlock,
    IN PLARGE_INTEGER               AllocationSize OPTIONAL,
    IN ULONG                        FileAttributes,
    IN ULONG                        ShareAccess,
    IN ULONG                        CreateDisposition,
    IN ULONG                        CreateOptions,
    IN PVOID                        EaBuffer OPTIONAL,
    IN ULONG                        EaLength
    );

__declspec(dllimport) NTSTATUS __stdcall
NtOpenFile(
    OUT PHANDLE                     FileHandle,
    IN ACCESS_MASK                  DesiredAccess,
    IN POBJECT_ATTRIBUTES           ObjectAttributes,
    OUT PIO_STATUS_BLOCK            IoStatusBlock,
    IN ULONG                        ShareAccess,
    IN ULONG                        OpenOptions
);

__declspec(dllimport) NTSTATUS __stdcall
NtQueryDirectoryFile(
    IN HANDLE                       FileHandle,
    IN HANDLE                       Event OPTIONAL,
    IN PVOID                        ApcRoutine OPTIONAL,
    IN PVOID                        ApcContext OPTIONAL,
    OUT PIO_STATUS_BLOCK            IoStatusBlock,
    OUT PVOID                       FileInformation,
    IN ULONG                        Length,
    IN FILE_INFORMATION_CLASS       FileInformationClass,
    IN BOOLEAN                      ReturnSingleEntry,
    IN PUNICODE_STRING              FileMask OPTIONAL,
    IN BOOLEAN                      RestartScan
);

__declspec(dllimport) NTSTATUS __stdcall
NtQueryInformationFile(
    IN HANDLE                       FileHandle,
    OUT PIO_STATUS_BLOCK            IoStatusBlock,
    OUT PVOID                       FileInformation,
    IN ULONG                        Length,
    IN FILE_INFORMATION_CLASS       FileInformationClass
);

__declspec(dllimport) NTSTATUS __stdcall
NtQueryAttributesFile(
    IN POBJECT_ATTRIBUTES           ObjectAttributes,
    OUT PFILE_BASIC_INFORMATION     FileInformation
);

__declspec(dllimport) NTSTATUS __stdcall
NtQueryFullAttributesFile(
    IN POBJECT_ATTRIBUTES                   ObjectAttributes,
    OUT PFILE_NETWORK_OPEN_INFORMATION      FileInformation
);

__declspec(dllimport) NTSTATUS __stdcall
NtSetInformationFile(
    IN HANDLE                       FileHandle,
    OUT PIO_STATUS_BLOCK            IoStatusBlock,
    IN PVOID                        FileInformation,
    IN ULONG                        Length,
    IN FILE_INFORMATION_CLASS       FileInformationClass
);

__declspec(dllimport) NTSTATUS __stdcall
NtDeleteFile(
    IN POBJECT_ATTRIBUTES           ObjectAttributes
);

__declspec(dllimport) NTSTATUS __stdcall
NtQueryAttributesFile(
    IN POBJECT_ATTRIBUTES           ObjectAttributes,
    OUT PFILE_BASIC_INFORMATION     FileInformation
);

typedef
VOID
(NTAPI *PIO_APC_ROUTINE) (
    IN PVOID ApcContext,
    IN PIO_STATUS_BLOCK IoStatusBlock,
    IN ULONG Reserved
    );

__declspec(dllimport) NTSTATUS __stdcall
NtReadFile(
    IN HANDLE                       FileHandle,
    IN HANDLE                       Event  OPTIONAL,
    IN PIO_APC_ROUTINE              ApcRoutine  OPTIONAL,
    IN PVOID                        ApcContext  OPTIONAL,
    OUT PIO_STATUS_BLOCK            IoStatusBlock,
    OUT PVOID                       Buffer,
    IN ULONG                        Length,
    IN PLARGE_INTEGER               ByteOffset  OPTIONAL,
    IN PULONG                       Key  OPTIONAL
);

__declspec(dllimport) NTSTATUS __stdcall
NtWriteFile(
    IN HANDLE                       FileHandle,
    IN HANDLE                       Event  OPTIONAL,
    IN PIO_APC_ROUTINE              ApcRoutine  OPTIONAL,
    IN PVOID                        ApcContext  OPTIONAL,
    OUT PIO_STATUS_BLOCK            IoStatusBlock,
    IN PVOID                        Buffer,
    IN ULONG                        Length,
    IN PLARGE_INTEGER               ByteOffset  OPTIONAL,
    IN PULONG                       Key  OPTIONAL
);

__declspec(dllimport) NTSTATUS __stdcall
NtDeviceIoControlFile(
    IN HANDLE                       FileHandle,
    IN HANDLE                       Event  OPTIONAL,
    IN PIO_APC_ROUTINE              ApcRoutine  OPTIONAL,
    IN PVOID                        ApcContext  OPTIONAL,
    OUT PIO_STATUS_BLOCK            IoStatusBlock,
    IN ULONG                        IoControlCode,
    IN PVOID                        InputBuffer OPTIONAL,
    IN ULONG                        InputBufferLength,
    OUT PVOID                       OutputBuffer OPTIONAL,
    IN ULONG                        OutputBufferLength
);

__declspec(dllimport) NTSTATUS __stdcall
NtFsControlFile(
    IN HANDLE FileHandle,
    IN HANDLE Event OPTIONAL,
    IN PIO_APC_ROUTINE ApcRoutine OPTIONAL,
    IN PVOID ApcContext OPTIONAL,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN ULONG IoControlCode,
    IN PVOID InputBuffer OPTIONAL,
    IN ULONG InputBufferLength,
    OUT PVOID OutputBuffer OPTIONAL,
    IN ULONG OutputBufferLength);

__declspec(dllimport) NTSTATUS __stdcall
NtCreateMailslotFile(
    OUT PHANDLE                     FileHandle,
    IN ACCESS_MASK                  DesiredAccess,
    IN POBJECT_ATTRIBUTES           ObjectAttributes,
    OUT PIO_STATUS_BLOCK            IoStatusBlock,
    IN ULONG                        CreateOptions,
    IN ULONG                        MailslotQuota,
    IN ULONG                        MaximumMessageSize,
    IN PLARGE_INTEGER               ReadTimeout
);

__declspec(dllimport) NTSTATUS __stdcall
NtCreateNamedPipeFile(
    OUT PHANDLE                     FileHandle,
    IN ACCESS_MASK                  DesiredAccess,
    IN POBJECT_ATTRIBUTES           ObjectAttributes,
    OUT PIO_STATUS_BLOCK            IoStatusBlock,
    IN ULONG                        ShareAccess,
    IN ULONG                        CreateDisposition,
    IN ULONG                        CreateOptions,
    IN ULONG                        NamedPipeType,
    IN ULONG                        ReadMode,
    IN ULONG                        CompletionMode,
    IN ULONG                        MaximumInstances,
    IN ULONG                        InboundQuota,
    IN ULONG                        OutboundQuota,
    IN PLARGE_INTEGER               DefaultTimeout OPTIONAL
);

//---------------------------------------------------------------------------

typedef enum _FSINFOCLASS {
    FileFsVolumeInformation       = 1,
    FileFsLabelInformation,      // 2
    FileFsSizeInformation,       // 3
    FileFsDeviceInformation,     // 4
    FileFsAttributeInformation,  // 5
    FileFsControlInformation,    // 6
    FileFsFullSizeInformation,   // 7
    FileFsObjectIdInformation,   // 8
    FileFsDriverPathInformation, // 9
    FileFsMaximumInformation
} FS_INFORMATION_CLASS, *PFS_INFORMATION_CLASS;

__declspec(dllimport) NTSTATUS __stdcall
NtQueryVolumeInformationFile(
    IN  HANDLE FileHandle,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    OUT PVOID FsInformation,
    IN  ULONG Length,
    IN  ULONG FsInformationClass
);

//---------------------------------------------------------------------------

typedef enum _TIMER_TYPE {
    NotificationTimer,
    SynchronizationTimer
} TIMER_TYPE;

__declspec(dllimport) NTSTATUS __stdcall
ZwCreateTimer(
    OUT PHANDLE TimerHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes OPTIONAL,
    IN TIMER_TYPE TimerType);

//---------------------------------------------------------------------------

#define NtCurrentProcess() ( (HANDLE)(LONG_PTR) -1 )
#define NtCurrentThread() ( (HANDLE)(LONG_PTR) -2 )

#define PROCESS_QUERY_LIMITED_INFORMATION  (0x1000)     // vista

#define THREAD_SET_LIMITED_INFORMATION   (0x0400)       // vista
#define THREAD_QUERY_LIMITED_INFORMATION (0x0800)       // vista

typedef enum _PROCESSINFOCLASS {
    ProcessBasicInformation,                            // 0
    ProcessQuotaLimits,
    ProcessIoCounters,
    ProcessVmCounters,
    ProcessTimes,
    ProcessBasePriority,
    ProcessRaisePriority,
    ProcessDebugPort,
    ProcessExceptionPort,
    ProcessAccessToken,
    ProcessLdtInformation,                              // 10
    ProcessLdtSize,
    ProcessDefaultHardErrorMode,
    ProcessIoPortHandlers,          // Note: this is kernel mode only
    ProcessPooledUsageAndLimits,
    ProcessWorkingSetWatch,
    ProcessUserModeIOPL,
    ProcessEnableAlignmentFaultFixup,
    ProcessPriorityClass,
    ProcessWx86Information,
    ProcessHandleCount,                                 // 20
    ProcessAffinityMask,
    ProcessPriorityBoost,
    ProcessDeviceMap,
    ProcessSessionInformation,
    ProcessForegroundInformation,
    ProcessWow64Information,
    ProcessImageFileName,                               // 27
    ProcessLUIDDeviceMapsEnabled,
    ProcessBreakOnTermination,
    ProcessDebugObjectHandle,                           // 30
    ProcessDebugFlags,
    ProcessHandleTracing,
	ProcessIoPriority,
	ProcessExecuteFlags,
	ProcessResourceManagement, // ProcessTlsInformation
	ProcessCookie,
	ProcessImageInformation,
	ProcessCycleTime,
	ProcessPagePriority,
	ProcessInstrumentationCallback,						// 40
	ProcessThreadStackAllocation,
	ProcessWorkingSetWatchEx,
	ProcessImageFileNameWin32,
	ProcessImageFileMapping,
	ProcessAffinityUpdateMode,
	ProcessMemoryAllocationMode,
	ProcessGroupInformation,
	ProcessTokenVirtualizationEnabled,
	ProcessConsoleHostProcess,
	ProcessWindowInformation,							// 50
    MaxProcessInfoClass             // MaxProcessInfoClass should always be the last enum
    } PROCESSINFOCLASS;

typedef struct _PROCESS_BASIC_INFORMATION {
    NTSTATUS ExitStatus;
    PVOID PebBaseAddress;   // was type PPEB
    ULONG_PTR AffinityMask;
    LONG BasePriority;      // was type KPRIORITY
    ULONG_PTR UniqueProcessId;
    ULONG_PTR InheritedFromUniqueProcessId;
} PROCESS_BASIC_INFORMATION, *PPROCESS_BASIC_INFORMATION;

typedef struct _PROCESS_IMAGE_FILE_NAME {
    USHORT  Length;
    USHORT  MaximumLength;
    ULONG   Unknown;
    WCHAR   ImagePath[1];
} PROCESS_IMAGE_FILE_NAME;

typedef struct _PROCESS_DEVICEMAP_INFORMATION {
    union {
        struct {
            HANDLE DirectoryHandle;
        } Set;
        struct {
            ULONG DriveMap;
            UCHAR DriveType[ 32 ];
        } Query;
    };
} PROCESS_DEVICEMAP_INFORMATION, *PPROCESS_DEVICEMAP_INFORMATION;

typedef struct _PROCESS_PRIORITY_CLASS {
    BOOLEAN  Foreground;
    UCHAR    PriorityClass;
} PROCESS_PRIORITY_CLASS;

typedef enum _THREADINFOCLASS {
    ThreadBasicInformation,
    ThreadTimes,
    ThreadPriority,
    ThreadBasePriority,
    ThreadAffinityMask,
    ThreadImpersonationToken,
    ThreadDescriptorTableEntry,
    ThreadEnableAlignmentFaultFixup,
    ThreadEventPair_Reusable,
    ThreadQuerySetWin32StartAddress,
    ThreadZeroTlsCell,
    ThreadPerformanceCount,
    ThreadAmILastThread,
    ThreadIdealProcessor,
    ThreadPriorityBoost,
    ThreadSetTlsArrayAddress,
    ThreadIsIoPending,
    ThreadHideFromDebugger,
    ThreadBreakOnTermination,
    ThreadSwitchLegacyState,
    ThreadIsTerminated,
    MaxThreadInfoClass
    } THREADINFOCLASS;

__declspec(dllimport) NTSTATUS __stdcall
NtQueryInformationProcess(
    IN HANDLE                       ProcessHandle,
    IN PROCESSINFOCLASS             ProcessInformationClass,
    OUT PVOID                       ProcessInformation,
    IN ULONG                        ProcessInformationLength,
    OUT PULONG                      ReturnLength OPTIONAL
);

__declspec(dllimport) NTSTATUS __stdcall
NtSetInformationProcess(
    IN HANDLE                       ProcessHandle,
    IN PROCESSINFOCLASS             ProcessInformationClass,
    IN PVOID                        ProcessInformation,
    IN ULONG                        ProcessInformationLength
);

__declspec(dllimport) NTSTATUS __stdcall
NtTerminateProcess(
    IN HANDLE ProcessHandle OPTIONAL,
    IN NTSTATUS ExitStatus
);

__declspec(dllimport) NTSTATUS __stdcall
NtTerminateThread(
    IN HANDLE ThreadHandle OPTIONAL,
    IN NTSTATUS ExitStatus
);

__declspec(dllimport) NTSTATUS __stdcall 
NtQueryInformationThread(
    IN  HANDLE ThreadHandle,
    IN  THREADINFOCLASS ThreadInformationClass,
    OUT PVOID ThreadInformation,
    IN  ULONG ThreadInformationLength,
    OUT PULONG ReturnLength OPTIONAL);

__declspec(dllimport) NTSTATUS __stdcall
NtSetInformationThread(
    IN HANDLE ThreadHandle,
    IN THREADINFOCLASS ThreadInformationClass,
    IN PVOID ThreadInformation,
    IN ULONG ThreadInformationLength);

typedef struct _CLIENT_ID {
    HANDLE UniqueProcess;
    HANDLE UniqueThread;
} CLIENT_ID;
typedef CLIENT_ID *PCLIENT_ID;

typedef struct _THREAD_BASIC_INFORMATION {
    NTSTATUS ExitStatus;
    PVOID TebBaseAddress;
    CLIENT_ID ClientId;
    /*KAFFINITY*/ ULONG_PTR AffinityMask;
    /*KPRIORITY*/ LONG Priority;
    /*KPRIORITY*/ LONG BasePriority;
} THREAD_BASIC_INFORMATION, *PTHREAD_BASIC_INFORMATION;

__declspec(dllimport) NTSTATUS __stdcall NtYieldExecution(void);

//---------------------------------------------------------------------------

typedef enum _KEY_INFORMATION_CLASS {
    KeyBasicInformation,
    KeyNodeInformation,
    KeyFullInformation,
    KeyNameInformation,
    KeyCachedInformation,
    KeyFlagsInformation,
    KeyVirtualizationInformation,   // Windows Vista
    KeyHandleTagsInformation,       // Windows 7
    MaxKeyInfoClass
} KEY_INFORMATION_CLASS;

typedef enum _KEY_VALUE_INFORMATION_CLASS {
    KeyValueBasicInformation,
    KeyValueFullInformation,
    KeyValuePartialInformation,
    KeyValueFullInformationAlign64,
    KeyValuePartialInformationAlign64,
    MaxKeyValueInfoClass  // MaxKeyValueInfoClass should always be the last enum
} KEY_VALUE_INFORMATION_CLASS;

typedef enum _KEY_SET_INFORMATION_CLASS {
    KeyWriteTimeInformation,
    KeyUserFlagsInformation,
    MaxKeySetInfoClass  // MaxKeySetInfoClass should always be the last enum
} KEY_SET_INFORMATION_CLASS;

typedef struct _KEY_BASIC_INFORMATION {
    LARGE_INTEGER LastWriteTime;
    ULONG   TitleIndex;
    ULONG   NameLength;
    WCHAR   Name[1];            // Variable length string
} KEY_BASIC_INFORMATION, *PKEY_BASIC_INFORMATION;

typedef struct _KEY_NODE_INFORMATION {
    LARGE_INTEGER LastWriteTime;
    ULONG   TitleIndex;
    ULONG   ClassOffset;
    ULONG   ClassLength;
    ULONG   NameLength;
    WCHAR   Name[1];            // Variable length string
//          Class[1];           // Variable length string not declared
} KEY_NODE_INFORMATION, *PKEY_NODE_INFORMATION;

typedef struct _KEY_FULL_INFORMATION {
    LARGE_INTEGER LastWriteTime;
    ULONG   TitleIndex;
    ULONG   ClassOffset;
    ULONG   ClassLength;
    ULONG   SubKeys;
    ULONG   MaxNameLen;
    ULONG   MaxClassLen;
    ULONG   Values;
    ULONG   MaxValueNameLen;
    ULONG   MaxValueDataLen;
    WCHAR   Class[1];           // Variable length
} KEY_FULL_INFORMATION, *PKEY_FULL_INFORMATION;

typedef struct _KEY_NAME_INFORMATION {
    ULONG   NameLength;
    WCHAR   Name[1];            // Variable length string
} KEY_NAME_INFORMATION, *PKEY_NAME_INFORMATION;

typedef struct _KEY_CACHED_INFORMATION {
    LARGE_INTEGER LastWriteTime;
    ULONG   TitleIndex;
    ULONG   SubKeys;
    ULONG   MaxNameLen;
    ULONG   Values;
    ULONG   MaxValueNameLen;
    ULONG   MaxValueDataLen;
    ULONG   NameLength;
    WCHAR   Name[1];            // Variable length string
} KEY_CACHED_INFORMATION, *PKEY_CACHED_INFORMATION;

typedef struct _KEY_WRITE_TIME_INFORMATION {
    LARGE_INTEGER LastWriteTime;
} KEY_WRITE_TIME_INFORMATION, *PKEY_WRITE_TIME_INFORMATION;

typedef struct _KEY_VALUE_BASIC_INFORMATION {
    ULONG   TitleIndex;
    ULONG   Type;
    ULONG   NameLength;
    WCHAR   Name[1];            // Variable size
} KEY_VALUE_BASIC_INFORMATION, *PKEY_VALUE_BASIC_INFORMATION;

typedef struct _KEY_VALUE_FULL_INFORMATION {
    ULONG   TitleIndex;
    ULONG   Type;
    ULONG   DataOffset;
    ULONG   DataLength;
    ULONG   NameLength;
    WCHAR   Name[1];            // Variable size
//          Data[1];            // Variable size data not declared
} KEY_VALUE_FULL_INFORMATION, *PKEY_VALUE_FULL_INFORMATION;

typedef struct _KEY_VALUE_PARTIAL_INFORMATION {
    ULONG   TitleIndex;
    ULONG   Type;
    ULONG   DataLength;
    UCHAR   Data[1];            // Variable size
} KEY_VALUE_PARTIAL_INFORMATION, *PKEY_VALUE_PARTIAL_INFORMATION;

typedef struct _KEY_VALUE_PARTIAL_INFORMATION_ALIGN64 {
    ULONG   Type;
    ULONG   DataLength;
    UCHAR   Data[1];            // Variable size
} KEY_VALUE_PARTIAL_INFORMATION_ALIGN64, *PKEY_VALUE_PARTIAL_INFORMATION_ALIGN64;

typedef struct _KEY_VALUE_ENTRY {
    PUNICODE_STRING ValueName;
    ULONG           DataLength;
    ULONG           DataOffset;
    ULONG           Type;
} KEY_VALUE_ENTRY, *PKEY_VALUE_ENTRY;

__declspec(dllimport) NTSTATUS __stdcall
NtOpenKey(
    OUT PHANDLE KeyHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes
);

__declspec(dllimport) NTSTATUS __stdcall
NtCreateKey(
    OUT PHANDLE KeyHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    IN ULONG TitleIndex,
    IN PUNICODE_STRING Class OPTIONAL,
    IN ULONG CreateOptions,
    OUT PULONG Disposition OPTIONAL
);

__declspec(dllimport) NTSTATUS __stdcall
NtEnumerateKey(
    IN HANDLE KeyHandle,
    IN ULONG Index,
    IN KEY_INFORMATION_CLASS KeyInformationClass,
    OUT PVOID KeyInformation,
    IN ULONG Length,
    OUT PULONG ResultLength
);

__declspec(dllimport) NTSTATUS __stdcall 
NtQueryKey(
    IN HANDLE KeyHandle,
    IN KEY_INFORMATION_CLASS KeyInformationClass,
    OUT PVOID KeyInformation,
    IN ULONG Length,
    OUT PULONG ResultLength);

__declspec(dllimport) NTSTATUS __stdcall
NtSetValueKey(
    IN HANDLE KeyHandle,
    IN PUNICODE_STRING ValueName,
    IN ULONG TitleIndex OPTIONAL,
    IN ULONG Type,
    IN PVOID Data,
    IN ULONG DataSize
);

__declspec(dllimport) NTSTATUS __stdcall
NtSetInformationKey(
    IN HANDLE KeyHandle,
    IN KEY_SET_INFORMATION_CLASS KeySetInformationClass,
    IN PVOID KeySetInformation,
    IN ULONG KeySetInformationLength
);

__declspec(dllimport) NTSTATUS __stdcall
NtDeleteKey(
    IN HANDLE KeyHandle
);

__declspec(dllimport) NTSTATUS __stdcall
NtUnloadKey(
    IN POBJECT_ATTRIBUTES ObjectAttributes
);

__declspec(dllimport) NTSTATUS __stdcall
NtDeleteValueKey(
    HANDLE KeyHandle, UNICODE_STRING *ValueName
);

#define RTL_REGISTRY_ABSOLUTE     0   // Path is a full path

__declspec(dllimport) NTSTATUS __stdcall
RtlCreateRegistryKey(
    ULONG RelativeTo, const WCHAR *Path);

// ------------------------------------------------------------------

typedef enum _SYSTEM_INFORMATION_CLASS {
    SystemBasicInformation,                                 // 0
    SystemProcessorInformation,
    SystemPerformanceInformation,
    SystemTimeOfDayInformation,
	SystemPathInformation,
    SystemProcessInformation,                               // 5
	SystemCallCountInformation,
	SystemDeviceInformation,
	SystemProcessorPerformanceInformation,
	SystemFlagsInformation,
    SystemCallTimeInformation,                              // 10
    SystemModuleInformation,                                // 11
	SystemLocksInformation,
	SystemStackTraceInformation,
	SystemPagedPoolInformation,
	SystemNonPagedPoolInformation,
    SystemHandleInformation,
    SystemObjectInformation,
	SystemPageFileInformation,
	SystemVdmInstemulInformation,
	SystemVdmBopInformation,
	SystemFileCacheInformation,
	SystemPoolTagInformation,
	SystemInterruptInformation,
	SystemDpcBehaviorInformation,
	SystemFullMemoryInformation,
	SystemLoadGdiDriverInformation,
	SystemUnloadGdiDriverInformation,
	SystemTimeAdjustmentInformation,
	SystemSummaryMemoryInformation,
	SystemMirrorMemoryInformation,
	SystemPerformanceTraceInformation,
    SystemCrashDumpInformation,
    SystemExceptionInformation,                             // 33
    SystemCrashDumpStateInformation,
    SystemKernelDebuggerInformation,
    SystemContextSwitchInformation,
    SystemRegistryQuotaInformation,                         // 37
	SystemExtendServiceTableInformation,
	SystemPrioritySeperation,
	SystemVerifierAddDriverInformation,
	SystemVerifierRemoveDriverInformation,
	SystemProcessorIdleInformation,
	SystemLegacyDriverInformation,
    SystemCurrentTimeZoneInformation,
    SystemLookasideInformation,                             // 45
	SystemTimeSlipNotification,
    SystemSessionCreate,
    SystemSessionDetach,
	SystemSessionInformation,
    SystemRangeStartInformation,                            // 50
    SystemVerifierInformation,
	SystemVerifierThunkExtend,
	SystemSessionProcessInformation,
	SystemLoadGdiDriverInSystemSpace,
	SystemNumaProcessorMap,
	SystemPrefetcherInformation,
	SystemExtendedProcessInformation,
	SystemRecommendedSharedDataAlignment,
	SystemComPlusPackage,
	SystemNumaAvailableMemory,
	SystemProcessorPowerInformation,
	SystemEmulationBasicInformation,				// WOW64
	SystemEmulationProcessorInformation,		// WOW64
	SystemExtendedHandleInformation,
	SystemLostDelayedWriteInformation,
	SystemBigPoolInformation,
	SystemSessionPoolTagInformation,
	SystemSessionMappedViewInformation,
	SystemHotpatchInformation,
	SystemObjectSecurityMode,
	SystemWatchdogTimerHandler,
	SystemWatchdogTimerInformation,
	SystemLogicalProcessorInformation,
	SystemWow64SharedInformationObsolete,
	SystemRegisterFirmwareTableInformationHandler,
	SystemFirmwareTableInformation,
	SystemModuleInformationEx,
	SystemVerifierTriageInformation,
	SystemSuperfetchInformation,
	SystemMemoryListInformation,
	SystemFileCacheInformationEx,
	SystemThreadPriorityClientIdInformation,
	SystemProcessorIdleCycleTimeInformation,
	SystemVerifierCancellationInformation,
	SystemProcessorPowerInformationEx,
	SystemRefTraceInformation,
	SystemSpecialPoolInformation,
	SystemProcessIdInformation,
	SystemErrorPortInformation,
	SystemBootEnvironmentInformation,
	SystemHypervisorInformation,
	SystemVerifierInformationEx,
	SystemTimeZoneInformation,
	SystemImageFileExecutionOptionsInformation,
	SystemCoverageInformation,
	SystemPrefetchPatchInformation,
	SystemVerifierFaultsInformation,
	SystemSystemPartitionInformation,
	SystemSystemDiskInformation,
	SystemProcessorPerformanceDistribution,
	SystemNumaProximityNodeInformation,
	SystemDynamicTimeZoneInformation,
	SystemCodeIntegrityInformation,
	SystemProcessorMicrocodeUpdateInformation,
	SystemProcessorBrandString,
	SystemVirtualAddressInformation,
	SystemLogicalProcessorAndGroupInformation,
	SystemProcessorCycleTimeInformation,
	SystemStoreInformation,
	SystemRegistryAppendString,
	SystemAitSamplingValue,
	SystemVhdBootInformation,
	SystemCpuQuotaInformation,
	SystemNativeBasicInformation,
	SystemErrorPortTimeouts,
	SystemLowPriorityIoInformation,
	SystemTpmBootEntropyInformation,
	SystemVerifierCountersInformation,
	SystemPagedPoolInformationEx,
	SystemSystemPtesInformationEx,
	SystemNodeDistanceInformation,
	SystemAcpiAuditInformation,
	SystemBasicPerformanceInformation,
	SystemQueryPerformanceCounterInformation,
	SystemSessionBigPoolInformation,
	SystemBootGraphicsInformation,
	SystemScrubPhysicalMemoryInformation,
	SystemBadPageInformation,
	SystemProcessorProfileControlArea,
	SystemCombinePhysicalMemoryInformation,
	SystemEntropyInterruptTimingInformation,
	SystemConsoleInformation,
	SystemPlatformBinaryInformation,
	SystemPolicyInformation,
	SystemHypervisorProcessorCountInformation,
	SystemDeviceDataInformation, 
	SystemDeviceDataEnumerationInformation, 
	SystemMemoryTopologyInformation, 
	SystemMemoryChannelInformation, 
	SystemBootLogoInformation, 
	SystemProcessorPerformanceInformationEx,
	SystemCriticalProcessErrorLogInformation,
	SystemSecureBootPolicyInformation, 
	SystemPageFileInformationEx, 
	SystemSecureBootInformation, 
	SystemEntropyInterruptTimingRawInformation,
	SystemPortableWorkspaceEfiLauncherInformation, 
	SystemFullProcessInformation, 
	SystemKernelDebuggerInformationEx, 
	SystemBootMetadataInformation, 
	SystemSoftRebootInformation, 
	SystemElamCertificateInformation, 
	SystemOfflineDumpConfigInformation,
	SystemProcessorFeaturesInformation, 
	SystemRegistryReconciliationInformation, 
	SystemEdidInformation,
	SystemManufacturingInformation, 
	SystemEnergyEstimationConfigInformation, 
	SystemHypervisorDetailInformation, 
	SystemProcessorCycleStatsInformation, 
	SystemVmGenerationCountInformation,
	SystemTrustedPlatformModuleInformation, 
	SystemKernelDebuggerFlags, 
	SystemCodeIntegrityPolicyInformation, 
	SystemIsolatedUserModeInformation, 
	SystemHardwareSecurityTestInterfaceResultsInformation,
	SystemSingleModuleInformation, 
	SystemAllowedCpuSetsInformation,
	SystemVsmProtectionInformation, 
	SystemInterruptCpuSetsInformation, 
	SystemSecureBootPolicyFullInformation, 
	SystemCodeIntegrityPolicyFullInformation,
	SystemAffinitizedInterruptProcessorInformation,
	SystemRootSiloInformation, 
	SystemCpuSetInformation, 
	SystemCpuSetTagInformation, 
	SystemWin32WerStartCallout,
	SystemSecureKernelProfileInformation, 
	SystemCodeIntegrityPlatformManifestInformation, 
	SystemInterruptSteeringInformation, 
	SystemSupportedProcessorArchitectures,
	SystemMemoryUsageInformation, 
	SystemCodeIntegrityCertificateInformation, 
	SystemPhysicalMemoryInformation, 
	SystemControlFlowTransition,
	SystemKernelDebuggingAllowed, 
	SystemActivityModerationExeState, 
	SystemActivityModerationUserSettings, 
	SystemCodeIntegrityPoliciesFullInformation,
	SystemCodeIntegrityUnlockInformation, 
	SystemIntegrityQuotaInformation,
	SystemFlushInformation, 
	SystemProcessorIdleMaskInformation, 
	SystemSecureDumpEncryptionInformation,
	SystemWriteConstraintInformation, 
	SystemKernelVaShadowInformation, 
	SystemHypervisorSharedPageInformation, 
	SystemFirmwareBootPerformanceInformation,
	SystemCodeIntegrityVerificationInformation, 
	SystemFirmwarePartitionInformation, 
	SystemSpeculationControlInformation,
	SystemDmaGuardPolicyInformation, 
	SystemEnclaveLaunchControlInformation, 
	SystemWorkloadAllowedCpuSetsInformation, 
	SystemCodeIntegrityUnlockModeInformation,
	SystemLeapSecondInformation, 
	SystemFlags2Information, 
	SystemSecurityModelInformation, 
	SystemCodeIntegritySyntheticCacheInformation,
	SystemFeatureConfigurationInformation, 
	SystemFeatureConfigurationSectionInformation,
	SystemFeatureUsageSubscriptionInformation,
	SystemSecureSpeculationControlInformation,
	MaxSystemInfoClass
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

typedef struct _HANDLE_INFO {
   ULONG        ProcessId;
   UCHAR        ObjectTypeNumber;
   UCHAR        Flags;
   USHORT       Handle;
   PVOID        Object;
   ACCESS_MASK  GrantedAccess;
} HANDLE_INFO;

#ifndef __cplusplus

typedef struct _SYSTEM_MODULE_INFORMATION {
    ULONG       ModuleCount;
#ifdef _WIN64
    ULONG       Reserved;
#endif
    MODULE_INFO ModuleInfo[0];
} SYSTEM_MODULE_INFORMATION;

typedef struct _SYSTEM_PROCESS_INFORMATION {
    ULONG NextEntryOffset;
    BYTE Reserved1[52];
	UNICODE_STRING ImageName;
	PVOID Reserved2[1];
    HANDLE UniqueProcessId;
    HANDLE InheritedFromProcessId;
    ULONG HandleCount;
    BYTE Reserved4[4];
    PVOID Reserved5[11];
    SIZE_T PeakPagefileUsage;
    SIZE_T PrivatePageCount;
    LARGE_INTEGER Reserved6[6];
} SYSTEM_PROCESS_INFORMATION;

typedef struct _SYSTEM_HANDLE_INFORMATION {
   ULONG Count;
   HANDLE_INFO HandleInfo[0];
} SYSTEM_HANDLE_INFORMATION, *PSYSTEM_HANDLE_INFORMATION;

#endif

__declspec(dllimport) NTSTATUS __stdcall NtQuerySystemInformation(
    IN SYSTEM_INFORMATION_CLASS SystemInformationClass,
    OUT PVOID                   SystemInformation,
    IN ULONG                    SystemInformationLength,
    OUT PULONG                  ReturnLength OPTIONAL);

__declspec(dllimport) NTSTATUS __stdcall NtSetSystemInformation(
    IN SYSTEM_INFORMATION_CLASS SystemInformationClass,
    IN OUT PVOID                   SystemInformation,
    IN ULONG                    SystemInformationLength);

//---------------------------------------------------------------------------

#define MAX_PORTMSG_LENGTH 328

typedef enum _LPC_TYPE {
    LPC_NEW_MESSAGE,            // 0
    LPC_REQUEST,                // 1
    LPC_REPLY,                  // 2
    LPC_DATAGRAM,               // 3
    LPC_LOST_REPLY,             // 4
    LPC_PORT_CLOSED,            // 5
    LPC_CLIENT_DIED,            // 6
    LPC_EXCEPTION,              // 7
    LPC_DEBUG_EVENT,            // 8
    LPC_ERROR_EVENT,            // 9
    LPC_CONNECTION_REQUEST,     // 10
    LPC_CONNECTION_REFUSED,     // 11
    LPC_MAXIMUM,
    LPC_KERNELMODE_MESSAGE      = 0x8000
} LPC_TYPE;

#pragma warning( push )
#pragma warning( disable : 4200)

typedef struct _PORT_MESSAGE {
    union {
        struct {
            USHORT DataLength;
            USHORT TotalLength;
        } s1;
        ULONG Length;
    } u1;
    union {
        struct {
            USHORT Type;
            USHORT DataInfoOffset;
        } s2;
        ULONG ZeroInit;
    } u2;
    union {
        CLIENT_ID ClientId;
        double DoNotUseThisField;       // force quadword alignment
    };
    ULONG MessageId;
    union {
        ULONG_PTR ClientViewSize;       // for LPC_CONNECTION_REQUEST message
        ULONG CallbackId;               // for LPC_REQUEST message
    };
    UCHAR Data[0];
} PORT_MESSAGE, *PPORT_MESSAGE;

#pragma warning( pop )

typedef struct _PORT_DATA_INFO {

    ULONG NumDataInfo;
    UCHAR *Buffer;
    ULONG BufferLen;

} PORT_DATA_INFO;

typedef void *PLPC_SECTION_MEMORY;
typedef void *PLPC_SECTION_OWNER_MEMORY;
typedef void *PPORT_VIEW;
typedef void *PREMOTE_PORT_VIEW;

// begin ALPC_INFO structure from LPC-ALPC-paper.pdf

#define PORT_INFO_LPCMODE               0x001000    // Behave like an LPC port
#define PORT_INFO_CANIMPERSONATE        0x010000    // Accept impersonation
#define PORT_INFO_REQUEST_ALLOWED       0x020000    // Allow messages
#define PORT_INFO_SEMAPHORE             0x040000    // Synchronization system
#define PORT_INFO_HANDLE_EXPOSE         0x080000    // Accept handle expose
#define PORT_INFO_PARENT_SYSTEM_PROCESS 0x100000    // Kernel ALPC interface
#define PORT_INFO_WOW64_PROCESS         0x40000000  // WOW64 process

#define ALPC_SYNC_CONNECTION            0x020000    // Synchronous connection request
#define ALPC_USER_WAIT_MODE             0x100000    // Wait in user mode
#define ALPC_WAIT_IS_ALERTABLE          0x200000    // Wait in alertable mode
#define ALPC_MESSAGE_FLAG_VIEW          0x40000000  // Message buffer type for section view

typedef struct _ALPC_PORT_ATTRIBUTES {

    ULONG       Flags;
    SECURITY_QUALITY_OF_SERVICE SecurityQos;
    ULONG       MaxMessageLength;
    ULONG_PTR   MemoryBandwidth;
    ULONG_PTR   MaxPoolUsage;
    ULONG_PTR   MaxSectionSize;
    ULONG_PTR   MaxViewSize;
    ULONG_PTR   MaxTotalSectionSize;
    ULONG_PTR   DupObjectTypes;

} ALPC_PORT_ATTRIBUTES;

typedef struct _ALPC_MESSAGE_VIEW {

    ULONG               SendFlags;
    ULONG               ReceiveFlags;
    union {
        ULONG           Unknowns[12];
        struct {
            ULONG       ReplyLength;
            ULONG       Unknown1;
            ULONG       Unknown2;
#ifdef _WIN64
            ULONG       Unknown3;
            ULONG       Unknown4;
#endif _WIN64
            ULONG       MessageId;
            ULONG       CallbackId;
        } s1;
        struct {
            ULONG       ViewAttrs;
            ULONG       Unknown1;
#ifdef _WIN64
            ULONG       Unknown2;
            ULONG       Unknown3;
#endif _WIN64
            ULONG_PTR   ViewBase;
            ULONG       ViewSize;
        } s2;
    } u;

} ALPC_MESSAGE_VIEW;


// end ALPC_INFO structure from LPC-ALPC-paper.pdf

__declspec(dllimport) NTSTATUS __stdcall
NtCreatePort(
    OUT PHANDLE PortHandle,
    IN  POBJECT_ATTRIBUTES ObjectAttributes,
    IN  ULONG MaxConnectInfoLength,
    IN  ULONG MaxMsgLength,
    IN  OUT PULONG Reserved OPTIONAL);

__declspec(dllimport) NTSTATUS __stdcall
NtConnectPort(
    OUT PHANDLE ClientPortHandle,
    IN  PUNICODE_STRING ServerPortName,
    IN  PSECURITY_QUALITY_OF_SERVICE SecurityQos,
    IN  OUT PLPC_SECTION_OWNER_MEMORY ClientSharedMemory OPTIONAL,
    OUT PLPC_SECTION_MEMORY ServerSharedMemory OPTIONAL,
    OUT PULONG MaximumMessageLength OPTIONAL,
    IN  OUT PVOID ConnectionInfo OPTIONAL,
    IN  OUT PULONG ConnectionInfoLength OPTIONAL);

__declspec(dllimport) NTSTATUS __stdcall
NtSecureConnectPort(
    OUT PHANDLE ClientPortHandle,
    IN  PUNICODE_STRING ServerPortName,
    IN  PSECURITY_QUALITY_OF_SERVICE SecurityQos,
    IN  OUT PLPC_SECTION_OWNER_MEMORY ClientSharedMemory OPTIONAL,
    IN  PSID ServerSid OPTIONAL,
    OUT PLPC_SECTION_MEMORY ServerSharedMemory OPTIONAL,
    OUT PULONG MaximumMessageLength OPTIONAL,
    IN  OUT PVOID ConnectionInfo OPTIONAL,
    IN  OUT PULONG ConnectionInfoLength OPTIONAL);

__declspec(dllimport) NTSTATUS __stdcall
NtAcceptConnectPort(
    OUT PHANDLE PortHandle,
    IN PVOID PortContext OPTIONAL,
    IN PPORT_MESSAGE ConnectionRequest,
    IN BOOLEAN AcceptConnection,
    IN OUT PPORT_VIEW ServerView OPTIONAL,
    OUT PREMOTE_PORT_VIEW ClientView OPTIONAL);

__declspec(dllimport) NTSTATUS __stdcall
NtCompleteConnectPort(
    IN  HANDLE PortHandle);

__declspec(dllimport) NTSTATUS __stdcall
NtRegisterThreadTerminatePort(
    IN  HANDLE PortHandle);

__declspec(dllimport) NTSTATUS __stdcall
NtRequestPort(
    IN HANDLE PortHandle,
    IN PPORT_MESSAGE RequestMessage);

__declspec(dllimport) NTSTATUS __stdcall
NtReplyPort(
    IN HANDLE PortHandle,
    IN PPORT_MESSAGE ReplyMessage);

__declspec(dllimport) NTSTATUS __stdcall
NtRequestWaitReplyPort(
    IN HANDLE PortHandle,
    IN PPORT_MESSAGE RequestMessage,
    OUT PPORT_MESSAGE ReplyMessage);

__declspec(dllimport) NTSTATUS __stdcall
NtReplyWaitReceivePort(
    IN  HANDLE PortHandle,
    OUT PVOID *PortContext OPTIONAL,
    IN  PPORT_MESSAGE ReplyMessage OPTIONAL,
    OUT PPORT_MESSAGE ReceiveMessage);

__declspec(dllimport) NTSTATUS __stdcall
NtImpersonateClientOfPort(
    IN  HANDLE PortHandle,
    IN  PPORT_MESSAGE PortMessage);


//---------------------------------------------------------------------------


__declspec(dllimport) NTSTATUS __stdcall
RtlInitUnicodeString(
    PUNICODE_STRING DestinationString,
    const WCHAR *SourceString
);

__declspec(dllimport) NTSTATUS __stdcall
RtlInitString(
    PANSI_STRING DestinationString,
    const UCHAR *SourceString
);

__declspec(dllimport) NTSTATUS __stdcall
RtlConvertSidToUnicodeString(
    OUT PUNICODE_STRING UnicodeString,
    IN  PSID Sid,
    IN  BOOLEAN AllocateDestinationString
);

__declspec(dllimport) NTSTATUS __stdcall
RtlAnsiStringToUnicodeString(
    OUT PUNICODE_STRING UnicodeString,
    IN  PCANSI_STRING   AnsiString,
    IN  BOOLEAN AllocateDestinationString
);

__declspec(dllimport) NTSTATUS __stdcall
RtlUnicodeStringToAnsiString(
    OUT PCANSI_STRING   AnsiString,
    IN  PUNICODE_STRING UnicodeString,
    IN  BOOLEAN AllocateDestinationString
);

__declspec(dllimport) NTSTATUS __stdcall
RtlFreeUnicodeString(
    IN  PUNICODE_STRING UnicodeString
);

__declspec(dllimport) NTSTATUS __stdcall
RtlFreeAnsiString(
    IN  PANSI_STRING AnsiString
);

__declspec(dllimport) LONG __stdcall
RtlCompareUnicodeString(
    PCUNICODE_STRING String1,
    PCUNICODE_STRING String2,
    BOOLEAN CaseInSensitive
);

//---------------------------------------------------------------------------

//#if _MSC_VER != 1200        // Visual C++ 6.0
//__declspec(dllimport) int __cdecl
//_wtoi(
//    const WCHAR *string
//);
//#endif

//---------------------------------------------------------------------------

__declspec(dllimport) NTSTATUS LdrLoadDll(
    WCHAR *PathString,
    ULONG *Flags,
    UNICODE_STRING *ModuleName,
    HANDLE *ModuleHandle);

__declspec(dllimport) NTSTATUS LdrUnloadDll(
    HANDLE ModuleHandle);

__declspec(dllimport) NTSTATUS LdrGetProcedureAddress(
    IN HANDLE ModuleHandle,
    IN PANSI_STRING ProcedureName,
    IN ULONG ProcedureNumber,
    OUT PVOID *ProcedureAddress);

__declspec(dllimport) NTSTATUS LdrQueryProcessModuleInformation(
    OUT struct _SYSTEM_MODULE_INFORMATION *SystemModuleInformationBuffer,
    IN ULONG BufferSize,
    OUT ULONG *RequiredSize);

__declspec(dllimport) NTSTATUS LdrFindEntryForAddress(
    PVOID Address, void **DataTableEntry);

__declspec(dllimport) NTSTATUS LdrQueryImageFileExecutionOptions(
    UNICODE_STRING *SubKey, const WCHAR *ValueName, ULONG Type,
    void *Buffer, ULONG BufferSize, ULONG *ReturnedLength);

typedef NTSTATUS (*P_LdrGetDllHandleEx)(
    ULONG_PTR One, ULONG_PTR Unknown1, ULONG_PTR Unknown2,
    UNICODE_STRING *DllName, ULONG_PTR *hModule);

//---------------------------------------------------------------------------

#define SYMBOLIC_LINK_QUERY (0x0001)
#define SYMBOLIC_LINK_ALL_ACCESS (STANDARD_RIGHTS_REQUIRED | 0x1)

__declspec(dllimport) NTSTATUS __stdcall NtCreateSymbolicLinkObject(
    OUT PHANDLE SymbolicLinkHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    IN PUNICODE_STRING LinkTarget);

__declspec(dllimport) NTSTATUS __stdcall NtOpenSymbolicLinkObject(
    OUT PHANDLE SymbolicLinkHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes);

__declspec(dllimport) NTSTATUS __stdcall NtQuerySymbolicLinkObject(
    IN HANDLE SymbolicLinkHandle,
    IN OUT PUNICODE_STRING LinkTarget,
    OUT PULONG ReturnedLength);

//---------------------------------------------------------------------------

#define RTL_REGISTRY_WINDOWS_NT   3   // \Registry\Machine\Software\Microsoft\Windows NT\CurrentVersion

#define RTL_QUERY_REGISTRY_REQUIRED 0x00000004
#define RTL_QUERY_REGISTRY_NOEXPAND 0x00000010
#define RTL_QUERY_REGISTRY_DIRECT   0x00000020

typedef struct _RTL_QUERY_REGISTRY_TABLE {
    void *QueryRoutine;
    ULONG Flags;
    PWSTR Name;
    PVOID EntryContext;
    ULONG DefaultType;
    PVOID DefaultData;
    ULONG DefaultLength;

} RTL_QUERY_REGISTRY_TABLE, *PRTL_QUERY_REGISTRY_TABLE;

__declspec(dllimport) NTSTATUS RtlQueryRegistryValues(
    IN ULONG RelativeTo,
    IN PCWSTR Path,
    IN PRTL_QUERY_REGISTRY_TABLE QueryTable,
    IN PVOID Context,
    IN PVOID Environment OPTIONAL
    );

//---------------------------------------------------------------------------

typedef void *PINITIAL_TEB;
//typedef ULONG EVENT_TYPE;
typedef enum _EVENT_TYPE { NotificationEvent, SynchronizationEvent } EVENT_TYPE;

//---------------------------------------------------------------------------

__declspec(dllimport) NTSTATUS __stdcall NtLoadDriver(
    UNICODE_STRING *RegistryPath);

__declspec(dllimport) NTSTATUS __stdcall NtUnloadDriver(
    UNICODE_STRING *RegistryPath);

//---------------------------------------------------------------------------

typedef enum _MEMORY_INFORMATION_CLASS {
	MemoryBasicInformation,
	MemoryWorkingSetInformation,
	MemoryMappedFilenameInformation,
	MemoryRegionInformation,
	MemoryWorkingSetExInformation
} MEMORY_INFORMATION_CLASS;

__declspec(dllimport) NTSTATUS __stdcall NtAllocateVirtualMemory(
    IN  HANDLE ProcessHandle,
        PVOID *BaseAddress,
    IN  ULONG_PTR ZeroBits,
        PSIZE_T RegionSize,
    IN  ULONG AllocationType,
    IN  ULONG Protect);

__declspec(dllimport) NTSTATUS __stdcall NtReadVirtualMemory(
    IN  HANDLE ProcessHandle,
    IN  PVOID BaseAddress,
    OUT PVOID Buffer,
    IN  SIZE_T BufferSize,
    OUT PSIZE_T NumberOfBytesRead OPTIONAL);

__declspec(dllimport) NTSTATUS __stdcall NtWriteVirtualMemory(
    IN  HANDLE ProcessHandle,
    OUT PVOID BaseAddress,
    IN  PVOID Buffer,
    IN  SIZE_T BufferSize,
    OUT PSIZE_T NumberOfBytesWritten OPTIONAL);

__declspec(dllimport) NTSTATUS __stdcall NtProtectVirtualMemory(
    IN  HANDLE ProcessHandle,
    IN  OUT PVOID *BaseAddress,
    IN  OUT PSIZE_T RegionSize,
    IN  ULONG NewProtect,
    OUT PULONG OldProtect);

__declspec(dllimport) NTSTATUS __stdcall NtQueryVirtualMemory(
    IN  HANDLE ProcessHandle,
    IN  PVOID BaseAddress,
    IN  MEMORY_INFORMATION_CLASS MemoryInformationClass,
    OUT PVOID MemoryInformation,
    IN  SIZE_T MemoryInformationLength,
    OUT PSIZE_T ReturnLength);

//---------------------------------------------------------------------------

__declspec(dllimport) NTSTATUS __stdcall NtSetEvent(
    IN  HANDLE EventHandle,
    OUT PLONG PreviousState OPTIONAL);

__declspec(dllimport) NTSTATUS __stdcall NtFlushInstructionCache(
    IN  HANDLE ProcessHandle,
    IN  PVOID BaseAddress OPTIONAL,
    IN  ULONG Length OPTIONAL);

//---------------------------------------------------------------------------

__declspec(dllimport) NTSTATUS __stdcall NtLoadKey(
    POBJECT_ATTRIBUTES TargetObjectAttributes,
    POBJECT_ATTRIBUTES SourceObjectAttributes);

__declspec(dllimport) NTSTATUS __stdcall NtSaveKey(
    HANDLE KeyHandle,
    HANDLE FileHandle);

__declspec(dllimport) NTSTATUS __stdcall NtQueryValueKey(
    HANDLE KeyHandle,
    UNICODE_STRING *ValueName,
    KEY_VALUE_INFORMATION_CLASS KeyValueInformationClass,
    void *KeyValueInformation,
    ULONG Length,
    ULONG *ResultLength);

__declspec(dllimport) NTSTATUS __stdcall NtQueryMultipleValueKey(
    HANDLE KeyHandle,
    PKEY_VALUE_ENTRY ValueEntries,
    ULONG EntryCount,
    void *ValueBuffer,
    ULONG *Length,
    ULONG *ResultLength);

__declspec(dllimport) NTSTATUS __stdcall NtEnumerateValueKey(
    HANDLE KeyHandle,
    ULONG Index,
    KEY_VALUE_INFORMATION_CLASS KeyValueInformationClass,
    void *KeyValueInformation,
    ULONG Length,
    ULONG *ResultLength);

__declspec(dllimport) NTSTATUS __stdcall NtNotifyChangeKey(
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

__declspec(dllimport) NTSTATUS __stdcall NtNotifyChangeMultipleKeys(
    HANDLE MasterKeyHandle,
    ULONG Count,
    OBJECT_ATTRIBUTES SlaveObjects[],
    HANDLE Event OPTIONAL,
    PIO_APC_ROUTINE ApcRoutine OPTIONAL,
    PVOID ApcContext OPTIONAL,
    PIO_STATUS_BLOCK IoStatusBlock,
    ULONG CompletionFilter,
    BOOLEAN WatchTree,
    PVOID Buffer,
    ULONG BufferSize,
    BOOLEAN Asynchronous);

//---------------------------------------------------------------------------

__declspec(dllimport) NTSTATUS __stdcall NtCreateEvent(
    OUT PHANDLE EventHandle,
    IN  ACCESS_MASK DesiredAccess,
    IN  POBJECT_ATTRIBUTES ObjectAttributes,
    IN  EVENT_TYPE EventType,
    IN  BOOLEAN InitialState);

__declspec(dllimport) NTSTATUS __stdcall NtWaitForSingleObject(
    IN HANDLE Handle,
    IN BOOLEAN Alertable,
    IN PLARGE_INTEGER Timeout
    );

__declspec(dllimport) NTSTATUS __stdcall NtOpenEvent(
    OUT PHANDLE EventHandle,
    IN  ACCESS_MASK DesiredAccess,
    IN  POBJECT_ATTRIBUTES ObjectAttributes);

__declspec(dllimport) NTSTATUS __stdcall NtCreateMutant(
    OUT PHANDLE MutantHandle,
    IN  ACCESS_MASK DesiredAccess,
    IN  POBJECT_ATTRIBUTES ObjectAttributes,
    IN  BOOLEAN InitialOwner);

__declspec(dllimport) NTSTATUS __stdcall NtOpenMutant(
    OUT PHANDLE MutantHandle,
    IN  ACCESS_MASK DesiredAccess,
    IN  POBJECT_ATTRIBUTES ObjectAttributes);

__declspec(dllimport) NTSTATUS __stdcall NtCreateSemaphore(
    OUT PHANDLE SemaphoreHandle,
    IN  ACCESS_MASK DesiredAccess,
    IN  POBJECT_ATTRIBUTES ObjectAttributes,
    IN  ULONG InitialCount,
    IN  ULONG MaximumCount);

__declspec(dllimport) NTSTATUS __stdcall NtOpenSemaphore(
    OUT PHANDLE SemaphoreHandle,
    IN  ACCESS_MASK DesiredAccess,
    IN  POBJECT_ATTRIBUTES ObjectAttributes);

__declspec(dllimport) NTSTATUS __stdcall NtCreateSection(
    OUT PHANDLE SectionHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes OPTIONAL,
    IN PLARGE_INTEGER MaximumSize OPTIONAL,
    IN ULONG PageAttributes,
    IN ULONG SectionAttributes,
    IN HANDLE FileHandle OPTIONAL);

__declspec(dllimport) NTSTATUS __stdcall NtOpenSection(
    OUT PHANDLE SectionHandle,
    IN  ACCESS_MASK DesiredAccess,
    IN  POBJECT_ATTRIBUTES ObjectAttributes);

__declspec(dllimport) NTSTATUS __stdcall NtMapViewOfSection(
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

__declspec(dllimport) NTSTATUS __stdcall NtUnmapViewOfSection(
    IN  HANDLE ProcessHandle,
    IN  PVOID BaseAddress);

//---------------------------------------------------------------------------

#define TokenElevationType          18
#define TokenLinkedToken            19
#define TokenIntegrityLevel         25
#define TokenUIAccess               26

#define TokenElevationTypeDefault   1
#define TokenElevationTypeFull      2
#define TokenElevationTypeLimited   3

__declspec(dllimport) NTSTATUS __stdcall NtOpenProcess(
    OUT PHANDLE ProcessHandle,
    IN  ACCESS_MASK DesiredAccess,
    IN  POBJECT_ATTRIBUTES ObjectAttributes,
    IN  PCLIENT_ID ClientId);

__declspec(dllimport) NTSTATUS __stdcall NtOpenThread(
    OUT PHANDLE ThreadHandle,
    IN  ACCESS_MASK DesiredAccess,
    IN  POBJECT_ATTRIBUTES ObjectAttributes,
    IN  PCLIENT_ID ClientId);

__declspec(dllimport) NTSTATUS __stdcall NtOpenProcessToken(
    IN HANDLE       ProcessHandle,
    IN ACCESS_MASK  DesiredAccess,
    OUT PHANDLE     TokenHandle);

__declspec(dllimport) NTSTATUS __stdcall NtOpenThreadToken(
    IN HANDLE       ThreadHandle,
    IN ACCESS_MASK  DesiredAccess,
    IN BOOLEAN      OpenAsSelf,
    OUT PHANDLE     TokenHandle);

__declspec(dllimport) NTSTATUS __stdcall NtQueryInformationToken(
    IN HANDLE                   TokenHandle,
    IN TOKEN_INFORMATION_CLASS  TokenInformationClass,
    OUT PVOID                   TokenInformation,
    IN ULONG                    TokenInformationLength,
    OUT PULONG                  ReturnLength);

__declspec(dllimport) NTSTATUS __stdcall NtSetInformationToken(
    IN HANDLE                   TokenHandle,
    IN TOKEN_INFORMATION_CLASS  TokenInformationClass,
    OUT PVOID                   TokenInformation,
    IN ULONG                    TokenInformationLength);

__declspec(dllimport) NTSTATUS __stdcall NtDuplicateObject(
    IN  HANDLE SourceProcessHandle,
    IN  HANDLE SourceHandle,
    IN  HANDLE TargetProcessHandle,
    OUT PHANDLE TargetHandle,
    IN  ACCESS_MASK DesiredAccess,
    IN  ULONG HandleAttributes,
    IN  ULONG Options);

__declspec(dllimport) NTSTATUS __stdcall NtDuplicateToken(
    IN HANDLE ExistingTokenHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    IN BOOLEAN EffectiveOnly,
    IN TOKEN_TYPE TokenType,
    OUT PHANDLE NewTokenHandle);

//__declspec(dllimport) NTSTATUS __stdcall NtSetSecurityObject(
//    IN  HANDLE Handle,
//    IN  SECURITY_INFORMATION SecurityInformation,
//    IN  PSECURITY_DESCRIPTOR SecurityDescriptor);

__declspec(dllimport) NTSTATUS __stdcall NtFilterToken(
    IN HANDLE ExistingTokenHandle,
    IN ULONG Flags,
    IN PTOKEN_GROUPS SidsToDisable OPTIONAL,
    IN PTOKEN_PRIVILEGES PrivilegesToDelete OPTIONAL,
    IN PTOKEN_GROUPS RestrictedSids OPTIONAL,
    OUT PHANDLE NewTokenHandle);

__declspec(dllimport) NTSTATUS __stdcall NtAdjustPrivilegesToken(
    IN HANDLE TokenHandle,
    IN BOOLEAN DisableAllPrivileges,
    IN PTOKEN_PRIVILEGES NewState OPTIONAL,
    IN ULONG BufferLength OPTIONAL,
    OUT PTOKEN_PRIVILEGES PreviousState OPTIONAL,
    OUT PULONG ReturnLength);

__declspec(dllimport) NTSTATUS __stdcall NtPrivilegeCheck(
    IN HANDLE TokenHandle,
    IN OUT PPRIVILEGE_SET RequiredPrivileges,
    OUT PBOOLEAN Result);

typedef NTSTATUS (*P_RtlQueryElevationFlags)(ULONG *Flags);

typedef NTSTATUS (*P_RtlCheckTokenMembershipEx)(
    HANDLE tokenHandle,
    PSID sidToCheck,
    DWORD flags,
    PBOOL isMember);

__declspec(dllimport) NTSTATUS RtlQueryElevationFlags(ULONG *Flags);

__declspec(dllimport) NTSTATUS __stdcall NtContinue(
    PCONTEXT ThreadContext, BOOLEAN RaiseAlert);

__declspec(dllimport) NTSTATUS __stdcall NtTestAlert(void);

__declspec(dllimport) NTSTATUS __stdcall NtImpersonateThread(
    HANDLE ServerThreadHandle, HANDLE ClientThreadHandle,
    PSECURITY_QUALITY_OF_SERVICE SecurityQos);

__declspec(dllimport) NTSTATUS __stdcall NtImpersonateAnonymousToken(
    HANDLE ThreadHandle);

//---------------------------------------------------------------------------

__declspec(dllimport) NTSTATUS RtlCreateAcl(
    PACL Acl, ULONG AclLength, ULONG AclRevision);

__declspec(dllimport) NTSTATUS RtlAddAccessAllowedAce(
    PACL Acl, ULONG AceRevision, ACCESS_MASK AccessMask, PSID Sid);

__declspec(dllimport) NTSTATUS RtlAddAccessAllowedAceEx(
    PACL Acl, ULONG AceRevision, ULONG AceFlags,
    ACCESS_MASK AccessMask, PSID Sid);

__declspec(dllimport) NTSTATUS RtlCreateSecurityDescriptor(
    PSECURITY_DESCRIPTOR SecurityDescriptor,
    ULONG Revision
);

//---------------------------------------------------------------------------

typedef struct _RTL_DRIVE_LETTER_CURDIR {
    USHORT Flags;
    USHORT Length;
    ULONG TimeStamp;
    UNICODE_STRING DosPath;
} RTL_DRIVE_LETTER_CURDIR, *PRTL_DRIVE_LETTER_CURDIR;

typedef struct _RTL_USER_PROCESS_PARAMETERS {
    ULONG MaximumLength;
    ULONG Length;
    ULONG Flags;
    ULONG DebugFlags;
    PVOID ConsoleHandle;
    ULONG ConsoleFlags;
    HANDLE StdInputHandle;
    HANDLE StdOutputHandle;
    HANDLE StdErrorHandle;
    UNICODE_STRING CurrentDirectoryPath;
    HANDLE CurrentDirectoryHandle;
    UNICODE_STRING DllPath;
    UNICODE_STRING ImagePathName;
    UNICODE_STRING CommandLine;
    PVOID Environment;
    ULONG StartingPositionLeft;
    ULONG StartingPositionTop;
    ULONG Width;
    ULONG Height;
    ULONG CharWidth;
    ULONG CharHeight;
    ULONG ConsoleTextAttributes;
    ULONG WindowFlags;
    ULONG ShowWindowFlags;
    UNICODE_STRING WindowTitle;
    UNICODE_STRING DesktopName;
    UNICODE_STRING ShellInfo;
    UNICODE_STRING RuntimeData;
    RTL_DRIVE_LETTER_CURDIR DLCurrentDirectory[0x20];
} RTL_USER_PROCESS_PARAMETERS, *PRTL_USER_PROCESS_PARAMETERS;

__declspec(dllimport) NTSTATUS RtlCreateProcessParameters(
    void *ProcessParameters,
    UNICODE_STRING *ImagePathName,
    UNICODE_STRING *DllPath,
    UNICODE_STRING *CurrentDirectory,
    UNICODE_STRING *CommandLine,
    void *Environment,
    UNICODE_STRING *WindowTitle,
    UNICODE_STRING *DesktopInfo,
    UNICODE_STRING *ShellInfo,
    UNICODE_STRING *RuntimeData);

__declspec(dllimport) NTSTATUS __stdcall NtCreateJobObject(
    OUT PHANDLE JobHandle,
    IN  ACCESS_MASK DesiredAccess,
    IN  POBJECT_ATTRIBUTES ObjectAttributes OPTIONAL);

__declspec(dllimport) NTSTATUS __stdcall NtAssignProcessToJobObject(
    HANDLE hJob, HANDLE hProcess);

__declspec(dllimport) NTSTATUS __stdcall NtSetInformationJobObject(
    IN  HANDLE JobHandle,
    IN  JOBOBJECTINFOCLASS JobObjectInformationClass,
    IN  PVOID JobObjectInformation,
    IN  ULONG JobObjectInformationLength);

//---------------------------------------------------------------------------

typedef ULONG (*P_RtlGetCurrentDirectory_U)(
    ULONG nBufferLength, WCHAR *lpBuffer);

__declspec(dllimport) ULONG RtlGetCurrentDirectory_U(
    ULONG nBufferLength, WCHAR *lpBuffer);

typedef NTSTATUS (*P_RtlSetCurrentDirectory_U)(
    UNICODE_STRING *PathName);

__declspec(dllimport) NTSTATUS RtlSetCurrentDirectory_U(
    PUNICODE_STRING PathName);

typedef ULONG (*P_RtlGetFullPathName_U)(
    PCWSTR lpFileName,
    ULONG nBufferLength, PWSTR lpBuffer,
    PWSTR *lpFilePart OPTIONAL);

__declspec(dllimport) ULONG RtlGetFullPathName_U(
    PCWSTR lpFileName,
    ULONG nBufferLength, PWSTR lpBuffer,
    PWSTR *lpFilePart OPTIONAL);

typedef NTSTATUS (*P_RtlGetFullPathName_UEx)(
    PCWSTR lpFileName,
    ULONG nBufferLength, PWSTR lpBuffer,
    PWSTR *lpFilePart OPTIONAL,
    ULONG *ResultLength);

typedef ULONG (*P_GetVolumeInformation)(
    const void *lpRootPathName,
    void *lpVolumeNameBuffer, ULONG nVolumeNameSize,
    ULONG *lpVolumeSerialNumber,
    ULONG *lpMaximumComponentLength,
    ULONG *lpFileSystemFlags,
    void *lpFileSystemNameBuffer, ULONG nFileSystemNameSize);

typedef BOOL (*P_WriteProcessMemory)(
    HANDLE hProcess,
    LPVOID lpBaseAddress,
    LPCVOID lpBuffer,
    SIZE_T nSize,
    SIZE_T * lpNumberOfBytesWritten
    );


typedef BOOL (*P_MoveFileWithProgress)(
    const void *lpExistingFileName,
    const void *lpNewFileName,
    void *lpProgressRoutine, void *lpData,
    ULONG dwFlags);

typedef BOOL (*P_ReplaceFile)(
    const WCHAR *lpReplacedFileName,
    const WCHAR *lpReplacementFileName,
    const WCHAR *lpBackupFileName,
    ULONG dwReplaceFlags,
    void *lpExclude,
    void *lpReserved);

typedef BOOL (*P_DefineDosDevice)(
    ULONG Flags,
    void *DeviceName,
    void *TargetPath);

typedef HMODULE (*P_LoadLibraryEx)(
    const void *lpFileName, HANDLE hFile, DWORD dwFlags);

typedef void (*P_RtlGetNtVersionNumbers)(
    ULONG *pMajorVersion,
    ULONG *pMinorVersion,
    ULONG *pBuildVersion);

typedef ULONG (*P_GetFinalPathNameByHandle)(
    HANDLE hFile,
    WCHAR *lpszFilePath,
    ULONG cchFilePath,
    ULONG dwFlags);

typedef ULONG (*P_GetTempPath)(ULONG nBufferLength, void *lpBuffer);

//---------------------------------------------------------------------------

__declspec(dllimport) ULONG __stdcall
RtlNtStatusToDosError(NTSTATUS Status);

__declspec(dllimport) void __stdcall RtlRaiseStatus(NTSTATUS Status);

__declspec(dllimport) PULONG __stdcall
RtlSubAuthoritySid(
    _In_ PSID Sid,
    _In_ ULONG SubAuthority
    );

__declspec(dllimport) NTSTATUS __stdcall
RtlInitializeSid(
    _Out_ PSID Sid,
    _In_ PSID_IDENTIFIER_AUTHORITY IdentifierAuthority,
    _In_ UCHAR SubAuthorityCount
    );

//---------------------------------------------------------------------------

//__declspec(dllimport) USHORT RtlCaptureStackBackTrace(
//    ULONG FramesToSkip,
//    ULONG FramesToCapture,
//    PVOID *BackTrace,
//    ULONG *BackTraceHash
//);

__declspec(dllimport) NTSTATUS __stdcall NtRaiseHardError(
    NTSTATUS ErrorStatus,
    ULONG NumberOfParameters,
    ULONG UnicodeBitMask,
    ULONG_PTR *Parameters,
    ULONG ErrorOption,
    ULONG *ErrorReturn);

//---------------------------------------------------------------------------

#ifdef _WIN64
#define NtCurrentPeb() ((ULONG_PTR)__readgsqword(0x60))
#else ! _WIN64
#define NtCurrentPeb() ((ULONG_PTR)__readfsdword(0x30))
#endif _WIN64

//---------------------------------------------------------------------------

#include "ntproto.h"

//---------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif /* _WIN32_NTDDK */

