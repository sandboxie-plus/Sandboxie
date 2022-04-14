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

// We can't use fltKernel.h from the DDK because it won't compile if _WIN32_WINNT < NTDDI_VISTA,
// which will prevent us from compiling a driver that will run under XP

#ifndef _MY_FLTKERNEL_H
#define _MY_FLTKERNEL_H


#define FLTAPI NTAPI


#define IRP_MJ_ACQUIRE_FOR_SECTION_SYNCHRONIZATION   ((UCHAR)-1)
#define IRP_MJ_RELEASE_FOR_SECTION_SYNCHRONIZATION   ((UCHAR)-2)
#define IRP_MJ_ACQUIRE_FOR_MOD_WRITE                 ((UCHAR)-3)
#define IRP_MJ_RELEASE_FOR_MOD_WRITE                 ((UCHAR)-4)
#define IRP_MJ_ACQUIRE_FOR_CC_FLUSH                  ((UCHAR)-5)
#define IRP_MJ_RELEASE_FOR_CC_FLUSH                  ((UCHAR)-6)


#define IRP_MJ_FAST_IO_CHECK_IF_POSSIBLE             ((UCHAR)-13)
#define IRP_MJ_NETWORK_QUERY_OPEN                    ((UCHAR)-14)
#define IRP_MJ_MDL_READ                              ((UCHAR)-15)
#define IRP_MJ_MDL_READ_COMPLETE                     ((UCHAR)-16)
#define IRP_MJ_PREPARE_MDL_WRITE                     ((UCHAR)-17)
#define IRP_MJ_MDL_WRITE_COMPLETE                    ((UCHAR)-18)
#define IRP_MJ_VOLUME_MOUNT                          ((UCHAR)-19)
#define IRP_MJ_VOLUME_DISMOUNT                       ((UCHAR)-20)


#define IRP_MJ_OPERATION_END                        ((UCHAR)0x80)


#define FLT_CONTEXT_END             0xffff


#define FLT_REGISTRATION_VERSION_0202  0x0202
#define FLTFL_REGISTRATION_DO_NOT_SUPPORT_SERVICE_STOP  0x00000001


typedef enum _FLT_FILESYSTEM_TYPE {

    FLT_FSTYPE_UNKNOWN,         //an UNKNOWN file system type
    FLT_FSTYPE_RAW,             //Microsoft's RAW file system       (\FileSystem\RAW)
    FLT_FSTYPE_NTFS,            //Microsoft's NTFS file system      (\FileSystem\Ntfs)
    FLT_FSTYPE_FAT,             //Microsoft's FAT file system       (\FileSystem\Fastfat)
    FLT_FSTYPE_CDFS,            //Microsoft's CDFS file system      (\FileSystem\Cdfs)
    FLT_FSTYPE_UDFS,            //Microsoft's UDFS file system      (\FileSystem\Udfs)
    FLT_FSTYPE_LANMAN,          //Microsoft's LanMan Redirector     (\FileSystem\MRxSmb)
    FLT_FSTYPE_WEBDAV,          //Microsoft's WebDav redirector     (\FileSystem\MRxDav)
    FLT_FSTYPE_RDPDR,           //Microsoft's Terminal Server redirector    (\Driver\rdpdr)
    FLT_FSTYPE_NFS,             //Microsoft's NFS file system       (\FileSystem\NfsRdr)
    FLT_FSTYPE_MS_NETWARE,      //Microsoft's NetWare redirector    (\FileSystem\nwrdr)
    FLT_FSTYPE_NETWARE,         //Novell's NetWare redirector
    FLT_FSTYPE_BSUDF,           //The BsUDF CD-ROM driver           (\FileSystem\BsUDF)
    FLT_FSTYPE_MUP,             //Microsoft's Mup redirector        (\FileSystem\Mup)
    FLT_FSTYPE_RSFX,            //Microsoft's WinFS redirector      (\FileSystem\RsFxDrv)
    FLT_FSTYPE_ROXIO_UDF1,      //Roxio's UDF writeable file system (\FileSystem\cdudf_xp)
    FLT_FSTYPE_ROXIO_UDF2,      //Roxio's UDF readable file system  (\FileSystem\UdfReadr_xp)
    FLT_FSTYPE_ROXIO_UDF3,      //Roxio's DVD file system           (\FileSystem\DVDVRRdr_xp)
    FLT_FSTYPE_TACIT,           //Tacit FileSystem                  (\Device\TCFSPSE)
    FLT_FSTYPE_FS_REC,          //Microsoft's File system recognizer (\FileSystem\Fs_rec)
    FLT_FSTYPE_INCD,            //Nero's InCD file system           (\FileSystem\InCDfs)
    FLT_FSTYPE_INCD_FAT,        //Nero's InCD FAT file system       (\FileSystem\InCDFat)
    FLT_FSTYPE_EXFAT,           //Microsoft's EXFat FILE SYSTEM     (\FileSystem\exfat)
    FLT_FSTYPE_PSFS             //PolyServ's file system            (\FileSystem\psfs)

} FLT_FILESYSTEM_TYPE, *PFLT_FILESYSTEM_TYPE;


typedef enum _FLT_PREOP_CALLBACK_STATUS {

    FLT_PREOP_SUCCESS_WITH_CALLBACK,
    FLT_PREOP_SUCCESS_NO_CALLBACK,
    FLT_PREOP_PENDING,
    FLT_PREOP_DISALLOW_FASTIO,
    FLT_PREOP_COMPLETE,
    FLT_PREOP_SYNCHRONIZE

} FLT_PREOP_CALLBACK_STATUS, *PFLT_PREOP_CALLBACK_STATUS;


typedef enum _FLT_POSTOP_CALLBACK_STATUS {

    FLT_POSTOP_FINISHED_PROCESSING,
    FLT_POSTOP_MORE_PROCESSING_REQUIRED

} FLT_POSTOP_CALLBACK_STATUS, *PFLT_POSTOP_CALLBACK_STATUS;


//typedef enum _FS_FILTER_SECTION_SYNC_TYPE {
//    SyncTypeOther = 0,
//    SyncTypeCreateSection
//} FS_FILTER_SECTION_SYNC_TYPE, *PFS_FILTER_SECTION_SYNC_TYPE;
//


typedef ULONG FLT_CALLBACK_DATA_FLAGS;
typedef ULONG FLT_INSTANCE_QUERY_TEARDOWN_FLAGS;
typedef ULONG FLT_OPERATION_REGISTRATION_FLAGS;
typedef ULONG FLT_POST_OPERATION_FLAGS;
typedef ULONG FLT_REGISTRATION_FLAGS;
typedef ULONG FLT_FILTER_UNLOAD_FLAGS;
typedef ULONG FLT_INSTANCE_SETUP_FLAGS;
typedef ULONG FLT_INSTANCE_TEARDOWN_FLAGS;
typedef ULONG FLT_FILE_NAME_OPTIONS;
typedef ULONG FLT_NORMALIZE_NAME_FLAGS;
typedef USHORT FLT_CONTEXT_REGISTRATION_FLAGS;
typedef USHORT FLT_CONTEXT_TYPE;
typedef PVOID PFLT_CONTEXT;
typedef ULONG FLT_FILE_NAME_OPTIONS;

typedef struct _FLT_FILTER *PFLT_FILTER;
typedef struct _FLT_VOLUME *PFLT_VOLUME;
typedef struct _FLT_INSTANCE *PFLT_INSTANCE;
typedef struct _KTRANSACTION *PKTRANSACTION;


typedef struct _FLT_RELATED_OBJECTS {

    USHORT CONST Size;
    USHORT CONST TransactionContext;            //TxF mini-version
    PFLT_FILTER CONST Filter;
    PFLT_VOLUME CONST Volume;
    PFLT_INSTANCE CONST Instance;
    PFILE_OBJECT CONST FileObject;
    PKTRANSACTION CONST Transaction;

} FLT_RELATED_OBJECTS, *PFLT_RELATED_OBJECTS;

typedef CONST struct _FLT_RELATED_OBJECTS *PCFLT_RELATED_OBJECTS;


#if !defined(_AMD64_) && !defined(_IA64_)
#include "pshpack4.h"
#endif

typedef union _FLT_PARAMETERS {

    //
    //  IRP_MJ_CREATE
    //

    struct {
        PIO_SECURITY_CONTEXT SecurityContext;

        //
        //  The low 24 bits contains CreateOptions flag values.
        //  The high 8 bits contains the CreateDisposition values.
        //

        ULONG Options;

        USHORT POINTER_ALIGNMENT FileAttributes;
        USHORT ShareAccess;
        ULONG POINTER_ALIGNMENT EaLength;

        PVOID EaBuffer;                 //Not in IO_STACK_LOCATION parameters list
        LARGE_INTEGER AllocationSize;   //Not in IO_STACK_LOCATION parameters list
    } Create;

    //
    //  IRP_MJ_CREATE_NAMED_PIPE
    //
    //  Notice that the fields in the following parameter structure must
    //  match those for the create structure other than the last longword.
    //  This is so that no distinctions need be made by the I/O system's
    //  parse routine other than for the last longword.
    //

    struct {
        PIO_SECURITY_CONTEXT SecurityContext;
        ULONG Options;
        USHORT POINTER_ALIGNMENT Reserved;
        USHORT ShareAccess;
        PVOID Parameters; // PNAMED_PIPE_CREATE_PARAMETERS
    } CreatePipe;

    //
    //  IRP_MJ_CREATE_MAILSLOT
    //
    //  Notice that the fields in the following parameter structure must
    //  match those for the create structure other than the last longword.
    //  This is so that no distinctions need be made by the I/O system's
    //  parse routine other than for the last longword.
    //

    struct {
        PIO_SECURITY_CONTEXT SecurityContext;
        ULONG Options;
        USHORT POINTER_ALIGNMENT Reserved;
        USHORT ShareAccess;
        PVOID Parameters; // PMAILSLOT_CREATE_PARAMETERS
    } CreateMailslot;

    //
    //  IRP_MJ_SET_INFORMATION
    //

    struct {
        ULONG Length;
        FILE_INFORMATION_CLASS POINTER_ALIGNMENT FileInformationClass;
        PFILE_OBJECT ParentOfTarget;
        union {
            struct {
                BOOLEAN ReplaceIfExists;
                BOOLEAN AdvanceOnly;
            };
            ULONG ClusterCount;
            HANDLE DeleteHandle;
        };

        PVOID InfoBuffer;       //Not in IO_STACK_LOCATION parameters list
    } SetFileInformation;

    //
    //  IRP_MJ_QUERY_EA
    //

    struct {
        ULONG Length;
        PVOID EaList;
        ULONG EaListLength;
        ULONG POINTER_ALIGNMENT EaIndex;

        PVOID EaBuffer;         //Not in IO_STACK_LOCATION parameters list
        PMDL MdlAddress;        //Mdl address for the buffer  (maybe NULL)
    } QueryEa;

    //
    //  IRP_MJ_QUERY_VOLUME_INFORMATION
    //

    struct {
        ULONG Length;
        FS_INFORMATION_CLASS POINTER_ALIGNMENT FsInformationClass;

        PVOID VolumeBuffer;     //Not in IO_STACK_LOCATION parameters list
    } QueryVolumeInformation;

    //
    //  IRP_MJ_SET_VOLUME_INFORMATION
    //

    struct {
        ULONG Length;
        FS_INFORMATION_CLASS POINTER_ALIGNMENT FsInformationClass;

        PVOID VolumeBuffer;     //Not in IO_STACK_LOCATION parameters list
    } SetVolumeInformation;

    //
    //  IRP_MJ_DIRECTORY_CONTROL
    //

    union {

        //
        //  IRP_MN_QUERY_DIRECTORY or IRP_MN_QUERY_OLE_DIRECTORY
        //

        struct {
            ULONG Length;
            PUNICODE_STRING FileName;
            FILE_INFORMATION_CLASS FileInformationClass;
            ULONG POINTER_ALIGNMENT FileIndex;

            PVOID DirectoryBuffer;  //Not in IO_STACK_LOCATION parameters list
            PMDL MdlAddress;        //Mdl address for the buffer  (maybe NULL)
        } QueryDirectory;

        //
        //  IRP_MN_NOTIFY_CHANGE_DIRECTORY
        //

        struct {
            ULONG Length;
            ULONG POINTER_ALIGNMENT CompletionFilter;

            //
            // These spares ensure that the offset of DirectoryBuffer is
            // exactly the same as that for QueryDirectory minor code. This
            // needs to be the same because filter manager code makes the assumption
            // they are the same
            //

            ULONG POINTER_ALIGNMENT Spare1;
            ULONG POINTER_ALIGNMENT Spare2;

            PVOID DirectoryBuffer;  //Not in IO_STACK_LOCATION parameters list
            PMDL MdlAddress;        //Mdl address for the buffer  (maybe NULL)
        } NotifyDirectory;

    } DirectoryControl;

    //
    //  IRP_MJ_FILE_SYSTEM_CONTROL
    //
    //  Note that the user's output buffer is stored in the UserBuffer field
    //  and the user's input buffer is stored in the SystemBuffer field.
    //

    union {

        //
        //  IRP_MN_VERIFY_VOLUME
        //

        struct {
            PVPB Vpb;
            PDEVICE_OBJECT DeviceObject;
        } VerifyVolume;

        //
        //  IRP_MN_KERNEL_CALL and IRP_MN_USER_FS_REQUEST
        //  The parameters are broken out into 3 separate unions based on the
        //  method of the FSCTL Drivers should use the method-appropriate
        //  union for accessing parameters
        //

        struct {
            ULONG OutputBufferLength;
            ULONG POINTER_ALIGNMENT InputBufferLength;
            ULONG POINTER_ALIGNMENT FsControlCode;
        } Common;

        //
        //  METHOD_NEITHER Fsctl parameters
        //

        struct {
            ULONG OutputBufferLength;
            ULONG POINTER_ALIGNMENT InputBufferLength;
            ULONG POINTER_ALIGNMENT FsControlCode;

            //
            //  Type3InputBuffer: name changed from IO_STACK_LOCATION parameters
            //  Note for this mothod, both input & output buffers are 'raw',
            //  i.e. unbuffered, and should be treated with caution ( either
            //  probed & captured before access, or use try-except to enclose
            //  access to the buffer)
            //

            PVOID InputBuffer;
            PVOID OutputBuffer;

            //
            //  Mdl address for the output buffer  (maybe NULL)
            //

            PMDL OutputMdlAddress;
        } Neither;

        //
        //  METHOD_BUFFERED Fsctl parameters
        //

        struct {
            ULONG OutputBufferLength;
            ULONG POINTER_ALIGNMENT InputBufferLength;
            ULONG POINTER_ALIGNMENT FsControlCode;

            //
            //  For method buffered, this buffer is used both for input and
            //  output
            //

            PVOID SystemBuffer;

        } Buffered;

        //
        //  METHOD_IN_DIRECT/METHOD_OUT_DIRECT Fsctl parameters
        //

        struct {
            ULONG OutputBufferLength;
            ULONG POINTER_ALIGNMENT InputBufferLength;
            ULONG POINTER_ALIGNMENT FsControlCode;

            //
            //  Note the input buffer is already captured & buffered here - so
            //  can be safely accessed from kernel mode.  The output buffer is
            //  locked down - so also safe to access, however the OutputBuffer
            //  pointer is the user virtual address, so if the driver wishes to
            //  access the buffer in a different process context than that of
            //  the original i/o - it will have to obtain the system address
            //  from the MDL
            //

            PVOID InputSystemBuffer;

            //
            //  User virtual address of output buffer
            //

            PVOID OutputBuffer;

            //
            //  Mdl address for the locked down output buffer (should be
            //  non-NULL)
            //

            PMDL OutputMdlAddress;
        } Direct;

    } FileSystemControl;

    //
    //  IRP_MJ_DEVICE_CONTROL or IRP_MJ_INTERNAL_DEVICE_CONTROL
    //

    union {

        struct {
            ULONG OutputBufferLength;
            ULONG POINTER_ALIGNMENT InputBufferLength;
            ULONG POINTER_ALIGNMENT IoControlCode;
        } Common;

        //
        //  The parameters are broken out into 3 separate unions based on the
        //  method of the IOCTL.  Drivers should use the method-appropriate
        //  union for accessing parameters.
        //

        //
        //  METHOD_NEITHER Ioctl parameters for IRP path
        //

        struct {
            ULONG OutputBufferLength;
            ULONG POINTER_ALIGNMENT InputBufferLength;
            ULONG POINTER_ALIGNMENT IoControlCode;

            //
            //  Type3InputBuffer: name changed from IO_STACK_LOCATION parameters
            //  Note for this mothod, both input & output buffers are 'raw',
            //  i.e. unbuffered, and should be treated with caution ( either
            //  probed & captured before access, or use try-except to enclose
            //  access to the buffer)
            //

            PVOID InputBuffer;
            PVOID OutputBuffer;

            //
            //  Mdl address for the output buffer  (maybe NULL)
            //

            PMDL OutputMdlAddress;
        } Neither;

        //
        //  METHOD_BUFFERED Ioctl parameters for IRP path
        //

        struct {
            ULONG OutputBufferLength;
            ULONG POINTER_ALIGNMENT InputBufferLength;
            ULONG POINTER_ALIGNMENT IoControlCode;

            //
            //  For method buffered, this buffer is used both for input and
            //  output
            //

            PVOID SystemBuffer;

        } Buffered;

        //
        //  METHOD_IN_DIRECT/METHOD_OUT_DIRECT Ioctl parameters
        //

        struct {
            ULONG OutputBufferLength;
            ULONG POINTER_ALIGNMENT InputBufferLength;
            ULONG POINTER_ALIGNMENT IoControlCode;

            //
            //  Note the input buffer is already captured & buffered here - so
            //  can be safely accessed from kernel mode.  The output buffer is
            //  locked down - so also safe to access, however the OutputBuffer
            //  pointer is the user virtual address, so if the driver wishes to
            //  access the buffer in a different process context than that of
            //  the original i/o - it will have to obtain the system address
            //  from the MDL
            //

            PVOID InputSystemBuffer;

            //
            //  User virtual address of output buffer
            //

            PVOID OutputBuffer;

            //
            //  Mdl address for the locked down output buffer (should be non-NULL)
            //

            PMDL OutputMdlAddress;
        } Direct;

        //
        //  Regardless of method, if the CALLBACK_DATA represents a fast i/o
        //  device IOCTL, this structure must be used to access the parameters
        //

        struct {
            ULONG OutputBufferLength;
            ULONG POINTER_ALIGNMENT InputBufferLength;
            ULONG POINTER_ALIGNMENT IoControlCode;

            //
            //  Both buffers are 'raw', i.e. unbuffered
            //

            PVOID InputBuffer;
            PVOID OutputBuffer;

        } FastIo;

    } DeviceIoControl;

    //
    //  IRP_MJ_SET_SECURITY
    //

    struct {
        SECURITY_INFORMATION SecurityInformation;
        PSECURITY_DESCRIPTOR SecurityDescriptor;
    } SetSecurity;

    //
    //  IRP_MJ_SYSTEM_CONTROL
    //

    struct {
        ULONG_PTR ProviderId;
        PVOID DataPath;
        ULONG BufferSize;
        PVOID Buffer;
    } WMI;

    //
    //  IRP_MJ_QUERY_QUOTA
    //

    /*struct {
        ULONG Length;
        PSID StartSid;
        PFILE_GET_QUOTA_INFORMATION SidList;
        ULONG SidListLength;

        PVOID QuotaBuffer;      //Not in IO_STACK_LOCATION parameters list
        PMDL MdlAddress;        //Mdl address for the buffer  (maybe NULL)
    } QueryQuota;*/

    //
    //  IRP_MJ_SET_QUOTA
    //

    struct {
        ULONG Length;

        PVOID QuotaBuffer;      //Not in IO_STACK_LOCATION parameters list
        PMDL MdlAddress;        //Mdl address for the buffer  (maybe NULL)
    } SetQuota;

    //
    //  IRP_MJ_PNP
    //

    union {

        //
        //  IRP_MN_START_DEVICE
        //

        struct {
            PCM_RESOURCE_LIST AllocatedResources;
            PCM_RESOURCE_LIST AllocatedResourcesTranslated;
        } StartDevice;

        //
        //  IRP_MN_QUERY_DEVICE_RELATIONS
        //

        struct {
            DEVICE_RELATION_TYPE Type;
        } QueryDeviceRelations;

        //
        //  IRP_MN_QUERY_INTERFACE
        //

        struct {
            CONST GUID *InterfaceType;
            USHORT Size;
            USHORT Version;
            PINTERFACE Interface;
            PVOID InterfaceSpecificData;
        } QueryInterface;

        //
        //  IRP_MN_QUERY_CAPABILITIES
        //

        struct {
            PDEVICE_CAPABILITIES Capabilities;
        } DeviceCapabilities;

        //
        //  IRP_MN_FILTER_RESOURCE_REQUIREMENTS
        //

        struct {
            PIO_RESOURCE_REQUIREMENTS_LIST IoResourceRequirementList;
        } FilterResourceRequirements;

        //
        //  IRP_MN_READ_CONFIG and IRP_MN_WRITE_CONFIG
        //

        struct {
            ULONG WhichSpace;
            PVOID Buffer;
            ULONG Offset;
            ULONG POINTER_ALIGNMENT Length;
        } ReadWriteConfig;

        //
        //  IRP_MN_SET_LOCK
        //

        struct {
            BOOLEAN Lock;
        } SetLock;

        //
        //  IRP_MN_QUERY_ID
        //

        struct {
            BUS_QUERY_ID_TYPE IdType;
        } QueryId;

        //
        //  IRP_MN_QUERY_DEVICE_TEXT
        //

        struct {
            DEVICE_TEXT_TYPE DeviceTextType;
            LCID POINTER_ALIGNMENT LocaleId;
        } QueryDeviceText;

        //
        //  IRP_MN_DEVICE_USAGE_NOTIFICATION
        //

        struct {
            BOOLEAN InPath;
            BOOLEAN Reserved[3];
            DEVICE_USAGE_NOTIFICATION_TYPE POINTER_ALIGNMENT Type;
        } UsageNotification;

    } Pnp;

    //
    //  ***** Start of Emulated IRP definitions
    //

    //
    //  IRP_MJ_ACQUIRE_FOR_SECTION_SYNCHRONIZATION
    //

    struct {
        FS_FILTER_SECTION_SYNC_TYPE SyncType;
        ULONG PageProtection;
    } AcquireForSectionSynchronization;

    //
    //  IRP_MJ_ACQUIRE_FOR_MOD_WRITE
    //

    struct {
        PLARGE_INTEGER EndingOffset;
        PERESOURCE *ResourceToRelease;
    } AcquireForModifiedPageWriter;

    //
    //  IRP_MJ_RELEASE_FOR_MOD_WRITE
    //

    struct {
        PERESOURCE ResourceToRelease;
    } ReleaseForModifiedPageWriter;


    //
    //  FAST_IO_CHECK_IF_POSSIBLE
    //

    struct {
        LARGE_INTEGER FileOffset;
        ULONG Length;
        ULONG POINTER_ALIGNMENT LockKey;
        BOOLEAN POINTER_ALIGNMENT CheckForReadOperation;
    } FastIoCheckIfPossible;

    //
    //  IRP_MJ_NETWORK_QUERY_OPEN
    //

    struct {
        PIRP Irp;
        PFILE_NETWORK_OPEN_INFORMATION NetworkInformation;
    } NetworkQueryOpen;

    //
    //  IRP_MJ_MDL_READ
    //

    struct {
        LARGE_INTEGER FileOffset;
        ULONG POINTER_ALIGNMENT Length;
        ULONG POINTER_ALIGNMENT Key;
        PMDL *MdlChain;
    } MdlRead;

    //
    //  IRP_MJ_MDL_READ_COMPLETE
    //

    struct {
        PMDL MdlChain;
    } MdlReadComplete;

    //
    //  IRP_MJ_PREPARE_MDL_WRITE
    //

    struct {
        LARGE_INTEGER FileOffset;
        ULONG POINTER_ALIGNMENT Length;
        ULONG POINTER_ALIGNMENT Key;
        PMDL *MdlChain;
    } PrepareMdlWrite;

    //
    //  IRP_MJ_MDL_WRITE_COMPLETE
    //

    struct {
        LARGE_INTEGER FileOffset;
        PMDL MdlChain;
    } MdlWriteComplete;

    //
    //  IRP_MJ_VOLUME_MOUNT
    //

    struct {
        ULONG DeviceType;
    } MountVolume;


    //
    // Others - driver-specific
    //

    struct {
        PVOID Argument1;
        PVOID Argument2;
        PVOID Argument3;
        PVOID Argument4;
        PVOID Argument5;
        LARGE_INTEGER Argument6;
    } Others;

} FLT_PARAMETERS, *PFLT_PARAMETERS;


#if !defined(_AMD64_) && !defined(_IA64_)
#include "poppack.h"
#endif


typedef struct _FLT_IO_PARAMETER_BLOCK {


    //
    //  Fields from IRP
    //  Flags

    ULONG IrpFlags;

    //
    //  Major/minor functions from IRP
    //

    UCHAR MajorFunction;
    UCHAR MinorFunction;

    //
    //  The flags associated with operations.
    //  The IO_STACK_LOCATION.Flags field in the old model (SL_* flags)
    //

    UCHAR OperationFlags;

    //
    //  For alignment
    //

    UCHAR Reserved;


    //
    //  The FileObject that is the target for this
    //  IO operation.
    //

    PFILE_OBJECT TargetFileObject;

    //
    //  Instance that i/o is directed to
    //

    PFLT_INSTANCE TargetInstance;

    //
    //  Normalized parameters for the operation
    //

    FLT_PARAMETERS Parameters;

} FLT_IO_PARAMETER_BLOCK, *PFLT_IO_PARAMETER_BLOCK;


typedef struct _FLT_CALLBACK_DATA {

    //
    //  Flags
    //

    FLT_CALLBACK_DATA_FLAGS Flags;

    //
    //  Thread that initiated this operation.
    //

    PETHREAD CONST Thread;

    //
    //  Pointer to the changeable i/o parameters
    //

    PFLT_IO_PARAMETER_BLOCK CONST Iopb;

    //
    //  For pre-op calls: if filter returns STATUS_IO_COMPLETE, then it should
    //  set the return i/o status here.  For post-operation calls, this is set
    //  by filter-manager indicating the completed i/o status.
    //

    IO_STATUS_BLOCK IoStatus;


    struct _FLT_TAG_DATA_BUFFER *TagData;

    union {
        struct {

            //
            //  Queue links if the FltMgr queue is used to
            //  pend the callback
            //

            LIST_ENTRY QueueLinks;

            //
            //  Additional context
            //

            PVOID QueueContext[2];
        };

        //
        //  The following are available to filters to use
        //  in whatever manner desired if not using the filter manager
        //  queues.
        //  NOTE:  These fields are only valid while the filter is
        //         processing this operation which is inside the operation
        //         callback or while the operation is pended.
        //

        PVOID FilterContext[4];
    };

    //
    //  Original requester mode of caller
    //

    KPROCESSOR_MODE RequestorMode;

} FLT_CALLBACK_DATA, *PFLT_CALLBACK_DATA;


#define FLTFL_CALLBACK_DATA_IRP_OPERATION           0x00000001    // Set for Irp operations
#define FLTFL_CALLBACK_DATA_FAST_IO_OPERATION       0x00000002    // Set for Fast Io operations
#define FLTFL_CALLBACK_DATA_FS_FILTER_OPERATION     0x00000004    // Set for Fs Filter operations

#define FLT_IS_IRP_OPERATION(Data)          ((Data)->Flags & FLTFL_CALLBACK_DATA_IRP_OPERATION)
#define FLT_IS_FASTIO_OPERATION(Data)       ((Data)->Flags & FLTFL_CALLBACK_DATA_FAST_IO_OPERATION)
#define FLT_IS_FS_FILTER_OPERATION(Data)    ((Data)->Flags & FLTFL_CALLBACK_DATA_FS_FILTER_OPERATION)


typedef FLT_PREOP_CALLBACK_STATUS
(FLTAPI *PFLT_PRE_OPERATION_CALLBACK) (
    __inout PFLT_CALLBACK_DATA Data,
    __in PCFLT_RELATED_OBJECTS FltObjects,
    __deref_out_opt PVOID *CompletionContext
    );


typedef FLT_POSTOP_CALLBACK_STATUS
(FLTAPI *PFLT_POST_OPERATION_CALLBACK) (
    __inout PFLT_CALLBACK_DATA Data,
    __in PCFLT_RELATED_OBJECTS FltObjects,
    __in_opt PVOID CompletionContext,
    __in FLT_POST_OPERATION_FLAGS Flags
    );


typedef VOID
(FLTAPI *PFLT_CONTEXT_CLEANUP_CALLBACK) (
    __in PFLT_CONTEXT Context,
    __in FLT_CONTEXT_TYPE ContextType
    );


typedef PVOID
(FLTAPI *PFLT_CONTEXT_ALLOCATE_CALLBACK)(
    __in POOL_TYPE PoolType,
    __in SIZE_T Size,
    __in FLT_CONTEXT_TYPE ContextType
    );

typedef VOID
(FLTAPI *PFLT_CONTEXT_FREE_CALLBACK)(
    __in PVOID Pool,
    __in FLT_CONTEXT_TYPE ContextType
    );


typedef struct _FLT_OPERATION_REGISTRATION {

    UCHAR MajorFunction;
    FLT_OPERATION_REGISTRATION_FLAGS Flags;
    PFLT_PRE_OPERATION_CALLBACK PreOperation;
    PFLT_POST_OPERATION_CALLBACK PostOperation;

    PVOID Reserved1;

} FLT_OPERATION_REGISTRATION, *PFLT_OPERATION_REGISTRATION;


typedef struct _FLT_CONTEXT_REGISTRATION {

    //
    //  Identifies the type of this context
    //

    FLT_CONTEXT_TYPE ContextType;

    //
    //  Local flags
    //

    FLT_CONTEXT_REGISTRATION_FLAGS Flags;

    //
    //  Routine to call to cleanup the context before it is deleted.
    //  This may be NULL if not cleanup is needed.
    //

    PFLT_CONTEXT_CLEANUP_CALLBACK ContextCleanupCallback;

    //
    //  Defines the size & pool tag the mini-filter wants for the given context.
    //  FLT_VARIABLE_SIZED_CONTEXTS may be specified for the size if variable
    //  sized contexts are used.  A size of zero is valid.  If an empty pooltag
    //  value is specified, the FLTMGR will use a context type specific tag.
    //
    //  If an explicit size is specified, the FLTMGR internally optimizes the
    //  allocation of that entry.
    //
    //  NOTE:  These fields are ignored if Allocate & Free routines are
    //         specified.
    //

    SIZE_T Size;
    ULONG PoolTag;

    //
    //  Specifies the ALLOCATE and FREE routines that should be used
    //  when allocating a context for this mini-filter.
    //
    //  NOTE: The above size & PoolTag fields are ignored when these routines
    //        are defined.
    //

    PFLT_CONTEXT_ALLOCATE_CALLBACK ContextAllocateCallback;
    PFLT_CONTEXT_FREE_CALLBACK ContextFreeCallback;

    //
    //  Reserved for future expansion
    //

    PVOID Reserved1;

} FLT_CONTEXT_REGISTRATION, *PFLT_CONTEXT_REGISTRATION;

typedef const FLT_CONTEXT_REGISTRATION *PCFLT_CONTEXT_REGISTRATION;


typedef struct _FLT_NAME_CONTROL {

    //
    //  The unicode string where the name should be set.
    //

    UNICODE_STRING Name;

} FLT_NAME_CONTROL, *PFLT_NAME_CONTROL;


typedef NTSTATUS
(FLTAPI *PFLT_FILTER_UNLOAD_CALLBACK) (
    FLT_FILTER_UNLOAD_FLAGS Flags
    );


typedef NTSTATUS
(FLTAPI *PFLT_INSTANCE_SETUP_CALLBACK) (
    __in PCFLT_RELATED_OBJECTS FltObjects,
    __in FLT_INSTANCE_SETUP_FLAGS Flags,
    __in DEVICE_TYPE VolumeDeviceType,
    __in FLT_FILESYSTEM_TYPE VolumeFilesystemType
    );


typedef NTSTATUS
(FLTAPI *PFLT_INSTANCE_QUERY_TEARDOWN_CALLBACK) (
    __in PCFLT_RELATED_OBJECTS FltObjects,
    __in FLT_INSTANCE_QUERY_TEARDOWN_FLAGS Flags
    );


typedef VOID
(FLTAPI *PFLT_INSTANCE_TEARDOWN_CALLBACK) (
    __in PCFLT_RELATED_OBJECTS FltObjects,
    __in FLT_INSTANCE_TEARDOWN_FLAGS Reason
    );


typedef NTSTATUS
(FLTAPI *PFLT_GENERATE_FILE_NAME) (
    __in PFLT_INSTANCE Instance,
    __in PFILE_OBJECT FileObject,
    __in_opt PFLT_CALLBACK_DATA CallbackData,
    __in FLT_FILE_NAME_OPTIONS NameOptions,
    __out PBOOLEAN CacheFileNameInformation,
    __out PFLT_NAME_CONTROL FileName
    );


typedef NTSTATUS
(FLTAPI *PFLT_NORMALIZE_NAME_COMPONENT) (
    __in PFLT_INSTANCE Instance,
    __in PCUNICODE_STRING ParentDirectory,
    __in USHORT VolumeNameLength,
    __in PCUNICODE_STRING Component,
    __out_bcount(ExpandCompnentNameLength) PFILE_NAMES_INFORMATION ExpandComponentName,
    __in ULONG ExpandComponentNameLength,
    __in FLT_NORMALIZE_NAME_FLAGS Flags,
    __deref_out_opt PVOID *NormalizationContext
    );


typedef VOID
(FLTAPI *PFLT_NORMALIZE_CONTEXT_CLEANUP) (
    __in_opt PVOID *NormalizationContext
    );


typedef NTSTATUS
(FLTAPI *PFLT_TRANSACTION_NOTIFICATION_CALLBACK) (
    __in PCFLT_RELATED_OBJECTS FltObjects,
    __in PFLT_CONTEXT TransactionContext,
    __in ULONG NotificationMask
    );


typedef NTSTATUS
(FLTAPI *PFLT_NORMALIZE_NAME_COMPONENT_EX) (
    __in PFLT_INSTANCE Instance,
    __in PFILE_OBJECT FileObject,
    __in PCUNICODE_STRING ParentDirectory,
    __in USHORT VolumeNameLength,
    __in PCUNICODE_STRING Component,
    __out_bcount(ExpandCompnentNameLength) PFILE_NAMES_INFORMATION ExpandComponentName,
    __in ULONG ExpandComponentNameLength,
    __in FLT_NORMALIZE_NAME_FLAGS Flags,
    __deref_out_opt PVOID *NormalizationContext
    );


typedef struct _FLT_REGISTRATION {

    //
    //  The size, in bytes, of this registration structure.
    //

    USHORT Size;
    USHORT Version;

    //
    //  Flag values
    //

    FLT_REGISTRATION_FLAGS Flags;

    //
    //  Variable length array of routines that are used to manage contexts in
    //  the system.
    //

    CONST FLT_CONTEXT_REGISTRATION *ContextRegistration;

    //
    //  Variable length array of routines used for processing pre- and post-
    //  file system operations.
    //

    CONST FLT_OPERATION_REGISTRATION *OperationRegistration;

    //
    //  This is called before a filter is unloaded.  If an ERROR or WARNING
    //  status is returned then the filter is NOT unloaded.  A mandatory unload
    //  can not be failed.
    //
    //  If a NULL is specified for this routine, then the filter can never be
    //  unloaded.
    //

    PFLT_FILTER_UNLOAD_CALLBACK FilterUnloadCallback;

    //
    //  This is called to see if a filter would like to attach an instance
    //  to the given volume.  If an ERROR or WARNING status is returned, an
    //  attachment is not made.
    //
    //  If a NULL is specified for this routine, the attachment is always made.
    //

    PFLT_INSTANCE_SETUP_CALLBACK InstanceSetupCallback;

    //
    //  This is called to see if the filter wants to detach from the given
    //  volume.  This is only called for manual detach requests.  If an
    //  ERROR or WARNING status is returned, the filter is not detached.
    //
    //  If a NULL is specified for this routine, then instances can never be
    //  manually detached.
    //

    PFLT_INSTANCE_QUERY_TEARDOWN_CALLBACK InstanceQueryTeardownCallback;

    //
    //  This is called at the start of a filter detaching from a volume.
    //
    //  It is OK for this field to be NULL.
    //

    PFLT_INSTANCE_TEARDOWN_CALLBACK InstanceTeardownStartCallback;

    //
    //  This is called at the end of a filter detaching from a volume.  All
    //  outstanding operations have been completed by the time this routine
    //  is called.
    //
    //  It is OK for this field to be NULL.
    //

    PFLT_INSTANCE_TEARDOWN_CALLBACK InstanceTeardownCompleteCallback;

    //
    //  The following callbacks are provided by a filter only if it is
    //  interested in modifying the name space.
    //
    //  If NULL is specified for these callbacks, it is assumed that the
    //  filter would not affect the name being requested.
    //

    PFLT_GENERATE_FILE_NAME GenerateFileNameCallback;

    PFLT_NORMALIZE_NAME_COMPONENT NormalizeNameComponentCallback;

    PFLT_NORMALIZE_CONTEXT_CLEANUP NormalizeContextCleanupCallback;

    //
    //  The PFLT_NORMALIZE_NAME_COMPONENT_EX callback is also a name
    //  provider callback. It is not included here along with the
    //  other name provider callbacks to take care of the registration
    //  structure versioning issues.
    //

    //
    //  This is called for transaction notifications received from the KTM
    //  when a filter has enlisted on that transaction.
    //

    PFLT_TRANSACTION_NOTIFICATION_CALLBACK TransactionNotificationCallback;

    //
    //  This is the extended normalize name component callback
    //  If a mini-filter provides this callback, then  this callback
    //  will be used as opposed to using PFLT_NORMALIZE_NAME_COMPONENT
    //
    //  The PFLT_NORMALIZE_NAME_COMPONENT_EX provides an extra parameter
    //  (PFILE_OBJECT) in addition to the parameters provided to
    //  PFLT_NORMALIZE_NAME_COMPONENT. A mini-filter may use this parameter
    //  to get to additional information like the TXN_PARAMETER_BLOCK.
    //
    //  A mini-filter that has no use for the additional parameter may
    //  only provide a PFLT_NORMALIZE_NAME_COMPONENT callback.
    //
    //  A mini-filter may provide both a PFLT_NORMALIZE_NAME_COMPONENT
    //  callback and a PFLT_NORMALIZE_NAME_COMPONENT_EX callback. The
    //  PFLT_NORMALIZE_NAME_COMPONENT_EX callback will be used by fltmgr
    //  versions that understand this callback (Vista RTM and beyond)
    //  and PFLT_NORMALIZE_NAME_COMPONENT callback will be used by fltmgr
    //  versions that do not understand the PFLT_NORMALIZE_NAME_COMPONENT_EX
    //  callback (prior to Vista RTM). This allows the same mini-filter
    //  binary to run with all versions of fltmgr.
    //

    PFLT_NORMALIZE_NAME_COMPONENT_EX NormalizeNameComponentExCallback;

} FLT_REGISTRATION, *PFLT_REGISTRATION;


NTSTATUS
FLTAPI
FltRegisterFilter(
    __in PDRIVER_OBJECT Driver,
    __in CONST FLT_REGISTRATION *Registration,
    __deref_out PFLT_FILTER *RetFilter
);

VOID
FLTAPI
FltUnregisterFilter(
    __in PFLT_FILTER Filter
);

NTSTATUS
FLTAPI
FltStartFiltering(
    __in PFLT_FILTER Filter
);

VOID
FLTAPI
FltSetCallbackDataDirty(
    __inout PFLT_CALLBACK_DATA Data
);


///////////////////////////////////////////////////////////////////////////////
//
//  Routines for getting file, directory and volume names.
//
///////////////////////////////////////////////////////////////////////////////

//
//  The FLT_FILE_NAME_OPTIONS is a ULONG that gets broken down into three
//  sections:
//   bits 0-7:  enumeration representing the file name formats available
//   bits 8-15: enumeration representing the querying methods available
//   bits 16-23:  Currently unused
//   bits 24-31:  Flags
//

typedef ULONG FLT_FILE_NAME_OPTIONS;

//
//  Name format options
//

#define FLT_VALID_FILE_NAME_FORMATS 0x000000ff

#define FLT_FILE_NAME_NORMALIZED    0x01
#define FLT_FILE_NAME_OPENED        0x02
#define FLT_FILE_NAME_SHORT         0x03

#define FltGetFileNameFormat( _NameOptions ) \
    ((_NameOptions) & FLT_VALID_FILE_NAME_FORMATS)

//
//  Name query methods.
//

#define FLT_VALID_FILE_NAME_QUERY_METHODS 0x0000ff00

//
//  In the default mode, if it is safe to query the file system,
//  the Filter Manager try to retrieve the name from the cache first, and,
//  if a name is not found, the name will be generated by querying the file
//  system.
//
#define FLT_FILE_NAME_QUERY_DEFAULT     0x0100

//
//  Query the Filter Manager's name cache for the name, but don't try
//  to query the file system if the name is not in the cache.
//
#define FLT_FILE_NAME_QUERY_CACHE_ONLY  0x0200

//
//  Only query the file system for the name, bypassing the Filter Manager's
//  name cache completely.  Any name retrieved will not be cached.
//
#define FLT_FILE_NAME_QUERY_FILESYSTEM_ONLY 0x0300

//
//  Query the Filter Manager's name cache, but if the name is not
//  found try to query the file system if it is safe to do so.
//
#define FLT_FILE_NAME_QUERY_ALWAYS_ALLOW_CACHE_LOOKUP 0x0400

#define FltGetFileNameQueryMethod( _NameOptions ) \
    ((_NameOptions) & FLT_VALID_FILE_NAME_QUERY_METHODS)

//
//  File name option flags
//

#define FLT_VALID_FILE_NAME_FLAGS 0xff000000

//
//  This flag is to be used by name provider filters to specify that a name
//  query request they are making should be redirected to their filter rather
//  than being satisfied by the name providers lower in the stack.
//
#define FLT_FILE_NAME_REQUEST_FROM_CURRENT_PROVIDER 0x01000000

//
//  This flag denotes that the name retrieved from this query should not
//  be cached.  This is used by name providers as they perform intermediate
//  queries to generate a name.
//
#define FLT_FILE_NAME_DO_NOT_CACHE                  0x02000000

#if FLT_MGR_AFTER_XPSP2

//
//  This flag denotes that it is safe to query the name in post-CREATE if
//  STATUS_REPARSE was returned.  To ensure the name returned is valid,
//  the call must know that the FileObject->FileName was not changed before
//  STATUS_REPARSE was returned.
//
#define FLT_FILE_NAME_ALLOW_QUERY_ON_REPARSE        0x04000000

#endif

//
//  The flags are used to tell the file name routines which types of names
//  you would like parsed from the full name.  They are also used to specify
//  which names have been filled in for a given FLT_FILE_NAME_INFORMATION
//  structure.
//

typedef USHORT FLT_FILE_NAME_PARSED_FLAGS;

#define FLTFL_FILE_NAME_PARSED_FINAL_COMPONENT      0x0001
#define FLTFL_FILE_NAME_PARSED_EXTENSION            0x0002
#define FLTFL_FILE_NAME_PARSED_STREAM               0x0004
#define FLTFL_FILE_NAME_PARSED_PARENT_DIR           0x0008

//
//  This structure holds the different types of name information that
//  can be given for a file.  The NamesParsed field will have the
//  appropriate flags set to denote which names are filled in inside
//  the structure.
//

typedef struct _FLT_FILE_NAME_INFORMATION {

    USHORT Size;

    //
    //  For each bit that is set in the NamesParsed flags field, the
    //  corresponding substring from Name has been appropriately
    //  parsed into one of the unicode strings below.
    //

    FLT_FILE_NAME_PARSED_FLAGS NamesParsed;

    //
    //  The name format that this FLT_FILE_NAME_INFORMATION structure
    //  represents.
    //

    FLT_FILE_NAME_OPTIONS Format;

    //
    //  For normalized and opened names, this name contains the version of
    //  name in the following format:
    //
    //    [Volume name][Full path to file][File name][Stream Name]
    //
    //    For example, the above components would map to this example name as
    //    follows:
    //
    //    \Device\HarddiskVolume1\Documents and Settings\MyUser\My Documents\Test Results.txt:stream1
    //
    //    [Volume name] = "\Device\HarddiskVolume1"
    //    [Full path to file] = "\Documents and Settings\MyUser\My Documents\"
    //    [File name] = "Test Results.txt"
    //    [Stream name] = ":stream1"
    //
    //  For short names, only the short name for the final name component is
    //  returned in the Name unicode string.  Therefore, if you requested
    //  the short name of the file object representing an open on the file:
    //
    //    \Device\HarddiskVolume1\Documents and Settings\MyUser\My Documents\Test Results.txt
    //
    //  The name returned in Name will be at most 8 characters followed by a '.'
    //  then at most 3 more characters, like:
    //
    //    testre~1.txt
    //

    UNICODE_STRING Name;

    //
    //  The Volume is only filled in for name requested in normalized and opened
    //  formats.
    //

    UNICODE_STRING Volume;

    //
    //  The share component of the file name requested.  This will only be
    //  set for normalized and opened name formats on files that opened across
    //  redirectors.  For local files, this string will always be 0 length.
    //

    UNICODE_STRING Share;

    //
    //  To exemplify what each of the following substrings refer to, let's
    //  look again at the first example string from above:
    //
    //    \Device\HarddiskVolume1\Documents and Settings\MyUser\My Documents\Test Results.txt:stream1
    //
    //  Extension = "txt"
    //  Stream = ":stream1"
    //  FinalComponent = "Test Results.txt:stream1"
    //  ParentDir = "\Documents and Settings\MyUser\My Documents\"
    //

    //
    //  This can be parsed from a normalized, opened, or short name.
    //

    UNICODE_STRING Extension;

    //
    //  The following parse formats are only available for normalized and
    //  opened name formats, but not short names.
    //

    UNICODE_STRING Stream;
    UNICODE_STRING FinalComponent;
    UNICODE_STRING ParentDir;

} FLT_FILE_NAME_INFORMATION, *PFLT_FILE_NAME_INFORMATION;

__checkReturn
__drv_maxIRQL(APC_LEVEL)
NTSTATUS
FLTAPI
FltGetFileNameInformation(
    __in PFLT_CALLBACK_DATA CallbackData,
    __in FLT_FILE_NAME_OPTIONS NameOptions,
    __deref_out PFLT_FILE_NAME_INFORMATION *FileNameInformation
);

__drv_maxIRQL(APC_LEVEL)
VOID
FLTAPI
FltReleaseFileNameInformation(
    __in PFLT_FILE_NAME_INFORMATION FileNameInformation
);

#endif _MY_FLTKERNEL_H
