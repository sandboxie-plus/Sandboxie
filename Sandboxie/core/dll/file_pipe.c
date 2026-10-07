/*
 * Copyright 2004-2020 Sandboxie Holdings, LLC 
 * Copyright 2020-2022 David Xanatos, xanasoft.com
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
// File (Pipe)
//---------------------------------------------------------------------------


#include "core/svc/NamedPipeWire.h"


//---------------------------------------------------------------------------
// Defines
//---------------------------------------------------------------------------


#define TYPE_NAMED_PIPE     1
#define TYPE_MAIL_SLOT      2
#define TYPE_NET_DEVICE     3


//---------------------------------------------------------------------------


#define METHOD_NEITHER                  3
#define CTL_CODE( DeviceType, Function, Method, Access ) (                 \
    ((DeviceType) << 16) | ((Access) << 14) | ((Function) << 2) | (Method) \
)

#define FILE_DEVICE_NAMED_PIPE          0x00000011

#define FSCTL_PIPE_TRANSCEIVE   \
    CTL_CODE(FILE_DEVICE_NAMED_PIPE, 5, METHOD_NEITHER,  \
             FILE_READ_DATA | FILE_WRITE_DATA)

#define FSCTL_PIPE_WAIT         \
    CTL_CODE(FILE_DEVICE_NAMED_PIPE, 6, METHOD_BUFFERED, 0)

#define FSCTL_PIPE_IMPERSONATE  \
    CTL_CODE(FILE_DEVICE_NAMED_PIPE, 7, METHOD_BUFFERED, 0)


//---------------------------------------------------------------------------
// Defines (HidePhysicalDiskSerial)
//---------------------------------------------------------------------------


//
// declared locally, as most of these are only available in the SDK
// headers for _WIN32_WINNT >= 0x0A00, which SbieDll does not target
//

#define FILE_IOCTL_STORAGE_QUERY_PROPERTY       0x002D1400  // CTL_CODE(IOCTL_STORAGE_BASE, 0x0500, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FILE_SMART_RCV_DRIVE_DATA               0x0007C088  // CTL_CODE(IOCTL_DISK_BASE, 0x0022, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)
#define FILE_IOCTL_ATA_PASS_THROUGH             0x0004D02C  // CTL_CODE(IOCTL_SCSI_BASE, 0x040B, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)
#define FILE_IOCTL_ATA_PASS_THROUGH_DIRECT      0x0004D030  // CTL_CODE(IOCTL_SCSI_BASE, 0x040C, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)

#define FILE_STORAGE_DEVICE_PROPERTY                        0   // StorageDeviceProperty
#define FILE_STORAGE_ADAPTER_PROTOCOL_SPECIFIC_PROPERTY     49  // StorageAdapterProtocolSpecificProperty
#define FILE_STORAGE_DEVICE_PROTOCOL_SPECIFIC_PROPERTY      50  // StorageDeviceProtocolSpecificProperty
#define FILE_PROPERTY_STANDARD_QUERY                        0   // PropertyStandardQuery

#define FILE_PROTOCOL_TYPE_ATA                  2   // ProtocolTypeAta
#define FILE_PROTOCOL_TYPE_NVME                 3   // ProtocolTypeNvme
#define FILE_ATA_DATA_TYPE_IDENTIFY             1   // AtaDataTypeIdentify
#define FILE_NVME_DATA_TYPE_IDENTIFY            1   // NVMeDataTypeIdentify
#define FILE_NVME_IDENTIFY_CNS_CONTROLLER       1   // NVME_IDENTIFY_CNS_CONTROLLER

#define FILE_ATA_ID_CMD                         0xEC    // IDENTIFY DEVICE
#define FILE_ATAPI_ID_CMD                       0xA1    // IDENTIFY PACKET DEVICE
#define FILE_ATA_TASK_FILE_COMMAND              6       // command register in the task file

#define FILE_ATA_IDENTIFY_SERIAL_OFFSET         20      // words 10-19
#define FILE_ATA_IDENTIFY_SERIAL_LENGTH         20
#define FILE_NVME_IDENTIFY_SERIAL_OFFSET        4       // SN in the Identify Controller data
#define FILE_NVME_IDENTIFY_SERIAL_LENGTH        20

#define FILE_SMART_IN_COMMAND_REG_OFFSET        10      // SENDCMDINPARAMS.irDriveRegs.bCommandReg (packed)
#define FILE_SMART_OUT_BUFFER_OFFSET            16      // SENDCMDOUTPARAMS.bBuffer (packed)


//---------------------------------------------------------------------------
// Structures
//---------------------------------------------------------------------------


typedef struct _FILE_PIPE_WAIT_FOR_BUFFER {

    LARGE_INTEGER   Timeout;
    ULONG           NameLength;
    BOOLEAN         TimeoutSpecified;
    WCHAR           Name[1];

} FILE_PIPE_WAIT_FOR_BUFFER;


typedef struct _FILE_STORAGE_PROPERTY_QUERY {   // STORAGE_PROPERTY_QUERY

    ULONG           PropertyId;
    ULONG           QueryType;
    UCHAR           AdditionalParameters[1];

} FILE_STORAGE_PROPERTY_QUERY;


typedef struct _FILE_STORAGE_DEVICE_DESCRIPTOR {    // STORAGE_DEVICE_DESCRIPTOR, up to SerialNumberOffset

    ULONG           Version;
    ULONG           Size;
    UCHAR           DeviceType;
    UCHAR           DeviceTypeModifier;
    BOOLEAN         RemovableMedia;
    BOOLEAN         CommandQueueing;
    ULONG           VendorIdOffset;
    ULONG           ProductIdOffset;
    ULONG           ProductRevisionOffset;
    ULONG           SerialNumberOffset;

} FILE_STORAGE_DEVICE_DESCRIPTOR;


typedef struct _FILE_STORAGE_PROTOCOL_SPECIFIC_DATA {   // STORAGE_PROTOCOL_SPECIFIC_DATA

    ULONG           ProtocolType;
    ULONG           DataType;
    ULONG           ProtocolDataRequestValue;
    ULONG           ProtocolDataRequestSubValue;
    ULONG           ProtocolDataOffset;
    ULONG           ProtocolDataLength;
    ULONG           FixedProtocolReturnData;
    ULONG           ProtocolDataRequestSubValue2;
    ULONG           ProtocolDataRequestSubValue3;
    ULONG           ProtocolDataRequestSubValue4;

} FILE_STORAGE_PROTOCOL_SPECIFIC_DATA;


typedef struct _FILE_STORAGE_PROTOCOL_DATA_DESCRIPTOR { // STORAGE_PROTOCOL_DATA_DESCRIPTOR

    ULONG           Version;
    ULONG           Size;
    FILE_STORAGE_PROTOCOL_SPECIFIC_DATA ProtocolSpecificData;

} FILE_STORAGE_PROTOCOL_DATA_DESCRIPTOR;


typedef struct _FILE_ATA_PASS_THROUGH {     // ATA_PASS_THROUGH_EX and ATA_PASS_THROUGH_DIRECT

    USHORT          Length;
    USHORT          AtaFlags;
    UCHAR           PathId;
    UCHAR           TargetId;
    UCHAR           Lun;
    UCHAR           ReservedAsUchar;
    ULONG           DataTransferLength;
    ULONG           TimeOutValue;
    ULONG           ReservedAsUlong;
    ULONG_PTR       DataBuffer;             // DataBufferOffset or DataBuffer
    UCHAR           PreviousTaskFile[8];
    UCHAR           CurrentTaskFile[8];

} FILE_ATA_PASS_THROUGH;


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------


static BOOLEAN File_IsPipeSuffix(const WCHAR *ptr);

static ULONG File_IsNamedPipe(const WCHAR *path, const WCHAR **server);

const BOOLEAN File_InternetBlockade_ManualBypass();

static NTSTATUS File_NtCreateFilePipe(
    HANDLE *FileHandle,
    ACCESS_MASK DesiredAccess,
    OBJECT_ATTRIBUTES *objattrs,
    PSECURITY_DESCRIPTOR SecurityDescriptor,
    PSECURITY_QUALITY_OF_SERVICE SecurityQualityOfService,
    IO_STATUS_BLOCK *IoStatusBlock,
    ULONG ShareAccess,
    ULONG CreateDisposition,
    ULONG CreateOptions,
    WCHAR *TruePath,
    ULONG PipeType,
    const WCHAR *PipeServer);

static void *File_GetBoxedPipeName(
    THREAD_DATA *TlsData, WCHAR *TruePath, ULONG PipeType);

static NTSTATUS File_OpenProxyPipe(
    HANDLE *FileHandle,
    ACCESS_MASK DesiredAccess,
    const WCHAR *ProxyPipeName,
    const WCHAR *ProxyPipeServer,
    IO_STATUS_BLOCK *IoStatusBlock,
    ULONG ShareAccess,
    ULONG CreateDisposition,
    ULONG CreateOptions);

static NTSTATUS File_CloseProxyPipe(
    HANDLE FileHandle);

static NTSTATUS File_SetProxyPipe(
    HANDLE FileHandle,
    IO_STATUS_BLOCK *IoStatusBlock,
    void *FileInformation,
    ULONG Length,
    FILE_INFORMATION_CLASS FileInformationClass);

static NTSTATUS File_NtReadFile(
    IN HANDLE FileHandle,
    IN HANDLE Event OPTIONAL,
    IN PIO_APC_ROUTINE ApcRoutine OPTIONAL,
    IN PVOID ApcContext OPTIONAL,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    OUT PVOID Buffer,
    IN ULONG Length,
    IN PLARGE_INTEGER ByteOffset OPTIONAL,
    IN PULONG Key OPTIONAL);

static NTSTATUS File_NtWriteFile(
    IN HANDLE FileHandle,
    IN HANDLE Event OPTIONAL,
    IN PIO_APC_ROUTINE ApcRoutine OPTIONAL,
    IN PVOID ApcContext OPTIONAL,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN PVOID Buffer,
    IN ULONG Length,
    IN PLARGE_INTEGER ByteOffset OPTIONAL,
    IN PULONG Key OPTIONAL);

static NTSTATUS File_NtFsControlFile(
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

static NTSTATUS File_WaitNamedPipe(
    HANDLE NamedPipesHandle, IO_STATUS_BLOCK *IoStatusBlock,
    void *InputBuffer, ULONG InputBufferLength);

static NTSTATUS File_NtDeviceIoControlFile(
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

static void File_HidePhysicalDiskSerial(
    ULONG IoControlCode, void *InputBuffer, ULONG InputBufferLength,
    void *OutputBuffer, IO_STATUS_BLOCK *IoStatusBlock);


//---------------------------------------------------------------------------
// Variables
//---------------------------------------------------------------------------


static ULONG *File_ProxyPipes = NULL;

static const WCHAR *File_NamedPipe = L"\\device\\namedpipe\\";
static const WCHAR *File_MailSlot  = L"\\device\\mailslot\\";

static const WCHAR *File_ACON_tag = L"AppContainerNamedObjects\\";
static const ULONG File_ACON_tag_len = 25;

static BOOLEAN File_BlockNetParam = FALSE;
static BOOLEAN File_HideDiskSerial = FALSE;


//---------------------------------------------------------------------------
// File_IsPipeSuffix
//---------------------------------------------------------------------------


__declspec(inline) BOOLEAN File_IsPipeSuffix(const WCHAR *ptr)
{
    //static const WCHAR *File_Pipe      = L"\\PIPE\\";
    //static const ULONG  File_Pipe_Len  = 6;

    if (ptr && _wcsnicmp(ptr, L"\\PIPE", 5) == 0 &&
            (ptr[5] == L'\\' || ptr[5] == L'\0'))
        return TRUE;
    else
        return FALSE;
}


//---------------------------------------------------------------------------
// File_IsNamedPipe
//---------------------------------------------------------------------------


_FX ULONG File_IsNamedPipe(const WCHAR *path, const WCHAR **server)
{
    ULONG len;

    //
    // check if this is \Device\NamedPipe or \Device\MailSlot,
    // or the various mup forms such as
    // \Device\Mup\NamedPipe or \Device\LanManRedirector\MailSlot
    //

    len = wcslen(path);

    if (len >= 18) {

        if (_wcsnicmp(path, File_NamedPipe, 18) == 0)
            return TYPE_NAMED_PIPE;
        if (_wcsnicmp(path, File_MailSlot, 17) == 0)
            return TYPE_MAIL_SLOT;

        if (_wcsnicmp(path, File_Redirector, File_RedirectorLen) == 0) {
            WCHAR *ptr = wcschr(path + File_RedirectorLen, L'\\');
            if (File_IsPipeSuffix(ptr)) {
                if (server)
                    *server = path + File_RedirectorLen;
                return TYPE_NAMED_PIPE;
            }
        }

        if (_wcsnicmp(path, File_MupRedir, File_MupRedirLen) == 0) {
            WCHAR *ptr = wcschr(path + File_MupRedirLen, L'\\');
            if (File_IsPipeSuffix(ptr)) {
                if (server)
                    *server = path + File_MupRedirLen;
                return TYPE_NAMED_PIPE;
            }
        }

        if (_wcsnicmp(path, File_Mup, File_MupLen) == 0) {
            WCHAR *ptr = wcschr(path + File_MupLen, L'\\');
            if (File_IsPipeSuffix(ptr)) {
                if (server)
                    *server = path + File_MupLen;
                return TYPE_NAMED_PIPE;
            }
        }
    }

    //
    // check if this is an Internet device matching a ClosedFilePath
    //

    if (len >= 10 && _wcsnicmp(path, File_Mup, 8) == 0) { // match \device\ only

		BOOLEAN prompt = SbieApi_QueryConfBool(NULL, L"PromptForInternetAccess", FALSE);
		if (SbieApi_CheckInternetAccess(0, path + 8, !prompt) == STATUS_ACCESS_DENIED
			&& (!prompt || !File_InternetBlockade_ManualBypass())) {

			return TYPE_NET_DEVICE;
		}
    }

    //
    // finish
    //

    return 0;
}


//---------------------------------------------------------------------------
// File_InternetBlockade_ManualBypass
//---------------------------------------------------------------------------


_FX const BOOLEAN File_InternetBlockade_ManualBypass()
{
	MAN_INET_BLOCKADE_REQ req;
	MAN_INET_BLOCKADE_RPL *rpl = NULL;
	BOOLEAN ok = FALSE;

	req.msgid = MAN_INET_BLOCKADE;

	rpl = SbieDll_CallServerQueue(INTERACTIVE_QUEUE_NAME, &req, sizeof(req), sizeof(*rpl));
	if (rpl)
	{
		ok = rpl->retval != 0;
		Dll_Free(rpl);
	}
	else if(SbieApi_QueryConfBool(NULL, L"NotifyInternetAccessDenied", TRUE))
		SbieApi_Log(1307, L"%S [%S]", Dll_ImageName, Dll_BoxName);

	//
	// Note: the granting process must notify the driver about the exemption 
	//			and we must ask the driver to update the open/closed path lists
	//

	if (ok) 
	{
		Dll_RefreshPathList();
	}

	return ok;
}


//---------------------------------------------------------------------------
// File_NtCreateMailslotFile
//---------------------------------------------------------------------------


_FX NTSTATUS File_NtCreateMailslotFile(
     HANDLE *FileHandle,
     ULONG DesiredAccess,
     OBJECT_ATTRIBUTES *ObjectAttributes,
     IO_STATUS_BLOCK *IoStatusBlock,
     ULONG CreateOptions,
     ULONG MailslotQuota,
     ULONG MaximumMessageSize,
     LARGE_INTEGER *ReadTimeout)
{
    ULONG LastError;
    THREAD_DATA *TlsData = Dll_GetTlsData(&LastError);

    NTSTATUS status;
    OBJECT_ATTRIBUTES objattrs;
    UNICODE_STRING objname;
    WCHAR *TruePath;
    WCHAR *CopyPath;
    ULONG PipeType;
    ULONG mp_flags;
    WCHAR *name;

    Dll_PushTlsNameBuffer(TlsData);

    __try {

    //
    // get the caller-provided name of the mailslot.  we must get
    // STATUS_BAD_INITIAL_PC, or else it isn't a mailslot path.
    //

    status = File_GetName(
        ObjectAttributes->RootDirectory, ObjectAttributes->ObjectName,
        &TruePath, &CopyPath, NULL);

    if (status != STATUS_BAD_INITIAL_PC) {

        status = STATUS_INVALID_PARAMETER;
        __leave;
    }

    InitializeObjectAttributes(&objattrs,
        &objname, OBJECT_ATTRIBUTES_ATTRIBUTES, NULL, NULL);

    //
    // check if this is an open or closed path
    //

    mp_flags = File_MatchPath(TruePath, NULL);

    if (PATH_IS_CLOSED(mp_flags)) {
        status = STATUS_ACCESS_DENIED;
        __leave;
    }

    if (PATH_IS_OPEN(mp_flags)) {

        RtlInitUnicodeString(&objname, TruePath);
        objattrs.SecurityDescriptor = ObjectAttributes->SecurityDescriptor;

        status = __sys_NtCreateMailslotFile(
            FileHandle, DesiredAccess, &objattrs, IoStatusBlock,
            CreateOptions, MailslotQuota, MaximumMessageSize, ReadTimeout);

        __leave;
    }

    //
    // try to create the mail slot in the sandbox
    //

    PipeType = File_IsNamedPipe(TruePath, NULL);

    name = File_GetBoxedPipeName(TlsData, TruePath, PipeType);
    if (! name) {
        status = STATUS_INVALID_PARAMETER;
        __leave;
    }

    RtlInitUnicodeString(&objname, name);
    objattrs.SecurityDescriptor = Secure_EveryoneSD;

    status = __sys_NtCreateMailslotFile(
        FileHandle, DesiredAccess, &objattrs, IoStatusBlock,
        CreateOptions, MailslotQuota, MaximumMessageSize, ReadTimeout);

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
// File_NtCreateNamedPipeFile
//---------------------------------------------------------------------------


_FX NTSTATUS File_NtCreateNamedPipeFile(
    HANDLE *FileHandle,
    ULONG DesiredAccess,
    OBJECT_ATTRIBUTES *ObjectAttributes,
    IO_STATUS_BLOCK *IoStatusBlock,
    ULONG ShareAccess,
    ULONG CreateDisposition,
    ULONG CreateOptions,
    ULONG NamedPipeType,
    ULONG ReadMode,
    ULONG CompletionMode,
    ULONG MaximumInstances,
    ULONG InboundQuota,
    ULONG OutboundQuota,
    LARGE_INTEGER *DefaultTimeout)
{
    ULONG LastError;
    THREAD_DATA *TlsData = Dll_GetTlsData(&LastError);

    NTSTATUS status;
    OBJECT_ATTRIBUTES objattrs;
    UNICODE_STRING objname;
    WCHAR *TruePath;
    WCHAR *CopyPath;
    ULONG PipeType;
    ULONG mp_flags;
    WCHAR *name;

    Dll_PushTlsNameBuffer(TlsData);

    __try {

    //
    // get the caller-provided name of the pipe.  we must get
    // STATUS_BAD_INITIAL_PC, or else it isn't a pipe path.
    //

    status = File_GetName(
        ObjectAttributes->RootDirectory, ObjectAttributes->ObjectName,
        &TruePath, &CopyPath, NULL);

    if (status != STATUS_BAD_INITIAL_PC) {

        status = STATUS_INVALID_PARAMETER;
        __leave;
    }

    //
    // special case:  null object name and root directory \Device\NamedPipe
    // (note, usually with no final backslash, i.e. 17 characters,
    // but we also accept the case with 18 characters case)
    //

    if (ObjectAttributes->RootDirectory && (
            (! ObjectAttributes->ObjectName) ||
                ObjectAttributes->ObjectName->Length == 0)) {

        ULONG len = wcslen(TruePath);
        if ((len == 17 || len == 18) &&
                _wcsnicmp(TruePath, File_NamedPipe, 17) == 0) {

            status = __sys_NtCreateNamedPipeFile(
                FileHandle, DesiredAccess, ObjectAttributes, IoStatusBlock,
                ShareAccess, CreateDisposition, CreateOptions,
                NamedPipeType, ReadMode, CompletionMode, MaximumInstances,
                InboundQuota, OutboundQuota, DefaultTimeout);

            __leave;
        }
    }

    //
    // check if this is an open or closed path
    //

    InitializeObjectAttributes(&objattrs,
        &objname, OBJECT_ATTRIBUTES_ATTRIBUTES, NULL, NULL);

    mp_flags = File_MatchPath2(TruePath, NULL, FALSE, TRUE); // File_MatchPath(TruePath, NULL);

    if (PATH_IS_CLOSED(mp_flags)) {
        status = STATUS_ACCESS_DENIED;
        __leave;
    }

    if (PATH_IS_OPEN(mp_flags)) {

        RtlInitUnicodeString(&objname, TruePath);
        objattrs.SecurityDescriptor = ObjectAttributes->SecurityDescriptor;

        status = __sys_NtCreateNamedPipeFile(
            FileHandle, DesiredAccess, &objattrs, IoStatusBlock,
            ShareAccess, CreateDisposition, CreateOptions,
            NamedPipeType, ReadMode, CompletionMode, MaximumInstances,
            InboundQuota, OutboundQuota, DefaultTimeout);

        __leave;
    }

    //
    // try to create the named pipe in the sandbox
    //

    PipeType = File_IsNamedPipe(TruePath, NULL);

    name = File_GetBoxedPipeName(TlsData, TruePath, PipeType);
    if (! name) {
        status = STATUS_INVALID_PARAMETER;
        __leave;
    }

    RtlInitUnicodeString(&objname, name);
    if (Dll_CompartmentMode)
        objattrs.SecurityDescriptor = ObjectAttributes->SecurityDescriptor;
    else 
        objattrs.SecurityDescriptor = Secure_EveryoneSD;

    status = __sys_NtCreateNamedPipeFile(
        FileHandle, DesiredAccess, &objattrs, IoStatusBlock,
        ShareAccess, CreateDisposition, CreateOptions,
        NamedPipeType, ReadMode, CompletionMode, MaximumInstances,
        InboundQuota, OutboundQuota, DefaultTimeout);

    if (status == STATUS_PRIVILEGE_NOT_HELD && !Dll_CompartmentMode) {

        objattrs.SecurityDescriptor = NULL;

        status = __sys_NtCreateNamedPipeFile(
            FileHandle, DesiredAccess, &objattrs, IoStatusBlock,
            ShareAccess, CreateDisposition, CreateOptions,
            NamedPipeType, ReadMode, CompletionMode, MaximumInstances,
            InboundQuota, OutboundQuota, DefaultTimeout);
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
// File_NtCreateFilePipe
//---------------------------------------------------------------------------


_FX NTSTATUS File_NtCreateFilePipe(
    HANDLE *FileHandle,
    ACCESS_MASK DesiredAccess,
    OBJECT_ATTRIBUTES *objattrs,
    PSECURITY_DESCRIPTOR SecurityDescriptor,
    PSECURITY_QUALITY_OF_SERVICE SecurityQualityOfService,
    IO_STATUS_BLOCK *IoStatusBlock,
    ULONG ShareAccess,
    ULONG CreateDisposition,
    ULONG CreateOptions,
    WCHAR *TruePath,
    ULONG PipeType,
    const WCHAR *PipeServer)
{
    ULONG LastError = GetLastError();
    THREAD_DATA *TlsData = Dll_GetTlsData(&LastError);

    NTSTATUS status;
    WCHAR *name;

    __try {

    //
    // check if this is an open or closed path
    //

    ULONG mp_flags = File_MatchPath2(TruePath, NULL, FALSE, FALSE);

    if (PATH_IS_CLOSED(mp_flags)) {
        status = STATUS_ACCESS_DENIED;
        __leave;
    }

    if (PATH_IS_OPEN(mp_flags)) {

        RtlInitUnicodeString(objattrs->ObjectName, TruePath);
        objattrs->SecurityDescriptor = SecurityDescriptor;

        status = __sys_NtCreateFile(
            FileHandle, DesiredAccess, objattrs, IoStatusBlock,
            NULL, 0, ShareAccess, CreateDisposition, CreateOptions,
            NULL, 0);

        __leave;
    }

    //
    // if the access is to a special pipe, which wasn't specified
    // as an OpenFilePath, de-administrator-ize and open the true path
    //
    // keep list of permitted pipes in sync with
    // SbieSvc::NamedPipeServer::OpenHandler
    // and with
    // SbieDrv::openPipesCM
    //

    if (PipeType == TYPE_NAMED_PIPE) {

        name = wcsrchr(TruePath, L'\\');
        if (name) {
            ++name;
            if (_wcsicmp(name, L"lsarpc")       == 0 ||
                _wcsicmp(name, L"srvsvc")       == 0 ||
                _wcsicmp(name, L"wkssvc")       == 0 ||
                _wcsicmp(name, L"samr")         == 0 ||
                _wcsicmp(name, L"netlogon")     == 0) {

                status = File_OpenProxyPipe(
                            FileHandle, DesiredAccess, name, PipeServer,
                            IoStatusBlock, ShareAccess,
                            CreateDisposition, CreateOptions);

                __leave;
            }
        }
    }

    //
    // try to open the named pipe or mail slot with the
    // CopyPath version of the name provided by the caller
    //

    name = File_GetBoxedPipeName(TlsData, TruePath, PipeType);
    if (! name) {
        status = STATUS_INVALID_PARAMETER;
        __leave;
    }

    RtlInitUnicodeString(objattrs->ObjectName, name);

    if (Dll_CompartmentMode)
        objattrs->SecurityDescriptor = SecurityDescriptor;

    objattrs->SecurityQualityOfService = SecurityQualityOfService;

    status = __sys_NtCreateFile(
        FileHandle, DesiredAccess, objattrs, IoStatusBlock,
        NULL, 0, ShareAccess, CreateDisposition, CreateOptions,
        NULL, 0);

    //
    // finish
    //

    } __except (EXCEPTION_EXECUTE_HANDLER) {
        status = GetExceptionCode();
    }

    if (status == STATUS_ACCESS_DENIED ||
        status == STATUS_OBJECT_NAME_NOT_FOUND ||
        status == STATUS_OBJECT_PATH_NOT_FOUND) {

        ULONG monflag = MONITOR_PIPE;
        if (status == STATUS_ACCESS_DENIED)
            monflag |= MONITOR_DENY;
        SbieApi_MonitorPut(monflag, TruePath);
    }
    else if (NT_SUCCESS(status))
    {
        File_MatchPath2(TruePath, NULL, FALSE, TRUE);
    }

    SetLastError(LastError);
    return status;
}


//---------------------------------------------------------------------------
// File_GetBoxedPipeName
//---------------------------------------------------------------------------


_FX void *File_GetBoxedPipeName(
    THREAD_DATA *TlsData, WCHAR *TruePath, ULONG PipeType)
{
    WCHAR *suffix, *name, *BoxPipePath, *ptr;
    ULONG len;

    if (PipeType == TYPE_NAMED_PIPE)
        suffix = TruePath + 18 - 1;
    else if (PipeType == TYPE_MAIL_SLOT)
        suffix = TruePath + 17 - 1;
    else
        return NULL;

    //
    // Check if this is an AppContainerNamedObjects path.
    // Pattern: Sessions\<N>\AppContainerNamedObjects\<SID>\<rest>
    // For such paths, we insert the sandbox prefix after the <SID>\
    // so that AppContainer isolation is preserved within the sandbox.
    //

    WCHAR *ACON_split = NULL;

    WCHAR *s;
    for (s = suffix; *s; s++) {
        if (_wcsnicmp(s, File_ACON_tag, File_ACON_tag_len) == 0) {
            WCHAR *sid_start = s + File_ACON_tag_len;
            WCHAR *sid_end = wcschr(sid_start, L'\\');
            if (sid_end) {
                ACON_split = sid_end;
            } else {
                // Directory creation for the SID itself, point to end of string so entire suffix
                // becomes the AppContainer prefix and pipe_name is empty
                ACON_split = wcschr(sid_start, L'\0');
            }
            break;
        }
    }
    
    ULONG ACON_prefix_len = ACON_split ? (ULONG)(ACON_split - suffix) : 0;
    WCHAR *pipe_name = ACON_split ? ACON_split : suffix;

    len = ((suffix - TruePath) + ACON_prefix_len + Dll_BoxIpcPathLen + 1 + wcslen(pipe_name)) * sizeof(WCHAR);
    name = Dll_GetTlsNameBuffer(TlsData, COPY_NAME_BUFFER, len);

    //
    // place a prefix, \Device\NamedPipe\ or \Device\MailSlot\ .
    //

    if (PipeType == TYPE_NAMED_PIPE)
        wcscpy(name, File_NamedPipe);
    else
        wcscpy(name, File_MailSlot);
    ptr = name + wcslen(name) - 1; // drop last \

    //
    // For AppContainer paths, copy everything up to and including
    // the SID backslash before inserting the sandbox prefix.
    //

    if (ACON_split) {
        wmemcpy(ptr, suffix, ACON_prefix_len);
        ptr += ACON_prefix_len;
    }

    //
    // Preserve LOCAL\ before the sandbox prefix so npfs can
    // recognize it and strip it during AppContainer redirection.
    // 
    // Don't do that when on normal mode as the driver will block it then
    //

    if (Dll_CompartmentMode) {
        if (_wcsnicmp(pipe_name, L"\\LOCAL", 6) == 0) {
            pipe_name += 6;
            wcscpy(ptr, L"\\LOCAL");
            ptr += 6;
        }
    }

    if (Dll_AlternateIpcNaming) {

		*ptr = L'\0';

        if (*pipe_name) {

            wcscpy(ptr, pipe_name);

            if (wcsstr(pipe_name, Dll_BoxIpcPath) == NULL) {
                wcscat(ptr, Dll_BoxIpcPath);
            }
        }

        return name;
    }

    //
    // translate Dll_BoxIpcPath to BoxPipePath by replacing
    // backslashes characters with underline characters
    //

    *ptr = L'\\';
    ++ptr;
    BoxPipePath = ptr;
    wcscpy(ptr, Dll_BoxIpcPath); // \Sandbox\DefaultBox\Session_1
    while (*ptr) {
        WCHAR *ptr2 = wcschr(ptr, L'\\');
        if (ptr2) {
            ptr = ptr2;
            *ptr = L'_';
        } else
            ptr += wcslen(ptr);
    }

    //
    // if pipe_name already begins with BoxPipePath, then
    // this is already a sandboxed pipe name, skip the prefix
    // to avoid double-sandboxing
    //

    *ptr = L'\0';
    len = wcslen(BoxPipePath);
    if (_wcsnicmp(pipe_name + 1, BoxPipePath, len) == 0)
        pipe_name += 1 + len;

    //
    // now append the (unsandboxed) pipe name past the BoxPipePath prefix
    //

    wcscpy(ptr, pipe_name);

    return name;
}


//---------------------------------------------------------------------------
//
// Named Pipe using the Named Pipe Proxy Server in SbieSvc
//
// Some named pipes (lsarpc, and others; see File_NtCreateFilePipe) will
// allow the caller to manage the computer if they are a member of the
// built-in Administrators group.  On the other hand, these pipes must be
// accessible for correct operation.
//
// Merely opening the pipe using a non-Administrator token cannot solve
// this problem, since ImpersonateNamedPipeClient() will impersonate
// the token last used to communicate on the pipe, not the token used
// to open the pipe.
//
// To work around these problems, the SbieDll will access these pipes via
// a proxy in SbieSvc, which impersonates using a non-Administrator token.
//
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
// File_AddProxyPipe
//---------------------------------------------------------------------------


_FX NTSTATUS File_AddProxyPipe(HANDLE *OutHandle, ULONG InHandle)
{
    ULONG i;

    for (i = 1; i < 256; ++i) {
        if (! File_ProxyPipes[i]) {

            if (InterlockedCompareExchange(
                        &File_ProxyPipes[i], InHandle, 0) == 0) {

                *OutHandle = (HANDLE)(PROXY_PIPE_MASK | i);
                return STATUS_SUCCESS;
            }
        }
    }

    return STATUS_INSUFFICIENT_RESOURCES;
}


//---------------------------------------------------------------------------
// File_GetProxyPipe
//---------------------------------------------------------------------------


_FX ULONG File_GetProxyPipe(HANDLE FakeHandle, UCHAR *FileIndex)
{
    if (((ULONG_PTR)FakeHandle & PROXY_PIPE_MASK) == PROXY_PIPE_MASK) {
        UCHAR idx = (UCHAR)((ULONG_PTR)FakeHandle & ~PROXY_PIPE_MASK);
        if (FileIndex)
            *FileIndex = idx;
        return File_ProxyPipes[idx];
    }
    return 0;
}


//---------------------------------------------------------------------------
// File_OpenProxyPipe
//---------------------------------------------------------------------------


_FX NTSTATUS File_OpenProxyPipe(
    HANDLE *FileHandle,
    ACCESS_MASK DesiredAccess,
    const WCHAR *ProxyPipeName,
    const WCHAR *ProxyPipeServer,
    IO_STATUS_BLOCK *IoStatusBlock,
    ULONG ShareAccess,
    ULONG CreateDisposition,
    ULONG CreateOptions)
{
    NAMED_PIPE_OPEN_REQ req;
    NAMED_PIPE_OPEN_RPL *rpl;
    NTSTATUS status;

    req.h.length = sizeof(NAMED_PIPE_OPEN_REQ);
    req.h.msgid = MSGID_NAMED_PIPE_OPEN;
    wcscpy(req.name, ProxyPipeName);
    memzero(req.server, sizeof(req.server));
    if (ProxyPipeServer) {
        WCHAR *ptr = wcschr(ProxyPipeServer, L'\\');
        if (ptr) {
            ULONG len = (ULONG)(ULONG_PTR)(ptr - ProxyPipeServer);
            if (len > 46)
                len = 46;
            wcsncpy(req.server, ProxyPipeServer, len);
        }
    }
    req.create_options = CreateOptions;

    rpl = (NAMED_PIPE_OPEN_RPL *)SbieDll_CallServer(&req.h);
    if (! rpl)
        status = STATUS_OBJECT_NAME_NOT_FOUND;
    else {

        status = rpl->h.status;
        if (rpl->h.length > sizeof(MSG_HEADER)) {

            IoStatusBlock->Status = (NTSTATUS)(ULONG_PTR)rpl->iosb.status;
            IoStatusBlock->Information = (ULONG_PTR)rpl->iosb.information;

            if (NT_SUCCESS(status)) {

                status = File_AddProxyPipe(FileHandle, rpl->handle);
            }
        }

        Dll_Free(rpl);
    }

    return status;
}


//---------------------------------------------------------------------------
// File_CloseProxyPipe
//---------------------------------------------------------------------------


_FX NTSTATUS File_CloseProxyPipe(HANDLE FileHandle)
{
    UCHAR FileIndex;
    NAMED_PIPE_CLOSE_REQ req;
    NAMED_PIPE_CLOSE_RPL *rpl;
    NTSTATUS status;

    req.h.length = sizeof(NAMED_PIPE_CLOSE_REQ);
    req.h.msgid = MSGID_NAMED_PIPE_CLOSE;

    req.handle = File_GetProxyPipe(FileHandle, &FileIndex);
    if (! req.handle)
        return STATUS_INVALID_HANDLE;

    rpl = (NAMED_PIPE_CLOSE_RPL *)SbieDll_CallServer(&req.h);
    if (! rpl)
        status = STATUS_INVALID_HANDLE;
    else {

        status = rpl->h.status;

        if (NT_SUCCESS(status))
            InterlockedExchange(&File_ProxyPipes[FileIndex], 0);

        Dll_Free(rpl);
    }

    return status;
}


//---------------------------------------------------------------------------
// File_SetProxyPipe
//---------------------------------------------------------------------------


_FX NTSTATUS File_SetProxyPipe(
    HANDLE FileHandle,
    IO_STATUS_BLOCK *IoStatusBlock,
    void *FileInformation,
    ULONG Length,
    FILE_INFORMATION_CLASS FileInformationClass)
{
    ULONG req_len;
    ULONG handle;
    NAMED_PIPE_SET_REQ *req;
    NAMED_PIPE_SET_RPL *rpl;
    NTSTATUS status;

    handle = File_GetProxyPipe(FileHandle, NULL);
    if (! handle)
        return STATUS_INVALID_HANDLE;

    //
    // FileCompletionInformation
    // FileIoCompletionNotificationInformation (Windows Vista and later)
    //
    // internal information classes related to asynchronous i/o
    // and completion ports, we just ignore and return success
    //

    if (FileInformationClass == FileCompletionInformation ||
        FileInformationClass == FileIoCompletionNotificationInformation)
    {
        memzero(&IoStatusBlock, sizeof(IO_STATUS_BLOCK));
        return STATUS_SUCCESS;
    }

    //
    // FilePipeInformation
    //
    // forward the request to the SbieSvc named pipe server
    //

    req_len = sizeof(NAMED_PIPE_SET_REQ) + Length;
    req = (NAMED_PIPE_SET_REQ *)Dll_AllocTemp(req_len);
    if (! req)
        return STATUS_INSUFFICIENT_RESOURCES;

    req->h.length = req_len;
    req->h.msgid = MSGID_NAMED_PIPE_SET;

    req->handle = handle;

    req->data_len = Length;
    memcpy(req->data, FileInformation, Length);

    rpl = (NAMED_PIPE_SET_RPL *)SbieDll_CallServer(&req->h);
    Dll_Free(req);
    if (! rpl)
        status = STATUS_INSUFFICIENT_RESOURCES;

    else {

        status = rpl->h.status;
        if (rpl->h.length > sizeof(MSG_HEADER)) {

            IoStatusBlock->Status = (NTSTATUS)(ULONG_PTR)rpl->iosb.status;
            IoStatusBlock->Information = (ULONG_PTR)rpl->iosb.information;
        }

        Dll_Free(rpl);
    }

    return status;
}


//---------------------------------------------------------------------------
// File_NtReadFile
//---------------------------------------------------------------------------


_FX NTSTATUS File_NtReadFile(
    IN HANDLE FileHandle,
    IN HANDLE Event OPTIONAL,
    IN PIO_APC_ROUTINE ApcRoutine OPTIONAL,
    IN PVOID ApcContext OPTIONAL,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    OUT PVOID Buffer,
    IN ULONG Length,
    IN PLARGE_INTEGER ByteOffset OPTIONAL,
    IN PULONG Key OPTIONAL)
{
    NAMED_PIPE_READ_REQ req;
    NAMED_PIPE_READ_RPL *rpl;
    NTSTATUS status;
    ULONG handle;

    handle = File_GetProxyPipe(FileHandle, NULL);
    if (! handle) {

        return __sys_NtReadFile(
                    FileHandle, Event, ApcRoutine, ApcContext,
                    IoStatusBlock, Buffer, Length, ByteOffset, Key);
    }

    req.h.length = sizeof(NAMED_PIPE_READ_REQ);
    req.h.msgid = MSGID_NAMED_PIPE_READ;

    req.handle = handle;

    req.read_len = Length;

    rpl = (NAMED_PIPE_READ_RPL *)SbieDll_CallServer(&req.h);
    if (! rpl)
        status = STATUS_INSUFFICIENT_RESOURCES;

    else {

        status = rpl->h.status;
        if (rpl->h.length > sizeof(MSG_HEADER)) {

            IoStatusBlock->Status = (NTSTATUS)(ULONG_PTR)rpl->iosb.status;
            IoStatusBlock->Information = (ULONG_PTR)rpl->iosb.information;
            memcpy(Buffer, rpl->data, rpl->data_len);
        }

        Dll_Free(rpl);

        if (Event)
            SetEvent(Event);
    }

    return status;
}


//---------------------------------------------------------------------------
// File_NtWriteFile
//---------------------------------------------------------------------------


_FX NTSTATUS File_NtWriteFile(
    IN HANDLE FileHandle,
    IN HANDLE Event OPTIONAL,
    IN PIO_APC_ROUTINE ApcRoutine OPTIONAL,
    IN PVOID ApcContext OPTIONAL,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN PVOID Buffer,
    IN ULONG Length,
    IN PLARGE_INTEGER ByteOffset OPTIONAL,
    IN PULONG Key OPTIONAL)
{
    ULONG req_len;
    NAMED_PIPE_WRITE_REQ *req;
    NAMED_PIPE_WRITE_RPL *rpl;
    NTSTATUS status;
    ULONG handle;

    handle = File_GetProxyPipe(FileHandle, NULL);
    if (! handle) {

        return __sys_NtWriteFile(
                    FileHandle, Event, ApcRoutine, ApcContext,
                    IoStatusBlock, Buffer, Length, ByteOffset, Key);
    }

    req_len = sizeof(NAMED_PIPE_WRITE_REQ) + Length;
    req = (NAMED_PIPE_WRITE_REQ *)Dll_AllocTemp(req_len);
    if (! req)
        return STATUS_INSUFFICIENT_RESOURCES;

    req->h.length = req_len;
    req->h.msgid = MSGID_NAMED_PIPE_WRITE;

    req->handle = handle;

    req->data_len = Length;
    memcpy(req->data, Buffer, Length);

    rpl = (NAMED_PIPE_WRITE_RPL *)SbieDll_CallServer(&req->h);
    Dll_Free(req);
    if (! rpl)
        status = STATUS_INSUFFICIENT_RESOURCES;
    else {

        status = rpl->h.status;
        if (rpl->h.length > sizeof(MSG_HEADER)) {

            IoStatusBlock->Status = (NTSTATUS)(ULONG_PTR)rpl->iosb.status;
            IoStatusBlock->Information = (ULONG_PTR)rpl->iosb.information;
        }

        Dll_Free(rpl);

        if (Event)
            SetEvent(Event);
    }

    return status;
}


//---------------------------------------------------------------------------
// File_NtFsControlFile
//---------------------------------------------------------------------------


_FX NTSTATUS File_NtFsControlFile(
    IN HANDLE FileHandle,
    IN HANDLE Event OPTIONAL,
    IN PIO_APC_ROUTINE ApcRoutine OPTIONAL,
    IN PVOID ApcContext OPTIONAL,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN ULONG IoControlCode,
    IN PVOID InputBuffer OPTIONAL,
    IN ULONG InputBufferLength,
    OUT PVOID OutputBuffer OPTIONAL,
    IN ULONG OutputBufferLength)
{
    ULONG LastError = GetLastError();
    NTSTATUS status;
    ULONG handle;

    handle = File_GetProxyPipe(FileHandle, NULL);
    if (! handle) {

        if (IoControlCode == FSCTL_SET_REPARSE_POINT) {

            status = File_SetReparsePoint(
                                FileHandle, InputBuffer, InputBufferLength);
            SetLastError(LastError);

        } else if (IoControlCode == FSCTL_PIPE_WAIT) {

            status = File_WaitNamedPipe(FileHandle, IoStatusBlock,
                                        InputBuffer, InputBufferLength);
            SetLastError(LastError);

        } else if (IoControlCode == FSCTL_PIPE_IMPERSONATE && !Dll_CompartmentMode) {

            SbieApi_Log(2205, L"ImpersonateNamedPipe");
            if (Proc_ImpersonateSelf(TRUE))
                status = STATUS_SUCCESS;
            else
                status = STATUS_ACCESS_DENIED;

        } else
            status = STATUS_BAD_INITIAL_PC;

        if (status == STATUS_BAD_INITIAL_PC) {

            status = __sys_NtFsControlFile(
                            FileHandle, Event, ApcRoutine, ApcContext,
                            IoStatusBlock, IoControlCode,
                            InputBuffer, InputBufferLength,
                            OutputBuffer, OutputBufferLength);
        }

        return status;
    }

    if (IoControlCode != FSCTL_PIPE_TRANSCEIVE)
        return STATUS_INVALID_PARAMETER;

    if (InputBuffer && InputBufferLength) {

        status = File_NtWriteFile(
                    FileHandle, NULL, NULL, NULL, IoStatusBlock,
                    InputBuffer, InputBufferLength, NULL, NULL);
        if (! NT_SUCCESS(status)) {
            SetLastError(LastError);
            return status;
        }
    }

    if (OutputBuffer && OutputBufferLength) {

        status = File_NtReadFile(
                    FileHandle, NULL, NULL, NULL, IoStatusBlock,
                    OutputBuffer, OutputBufferLength, NULL, NULL);
        if (! NT_SUCCESS(status)) {
            SetLastError(LastError);
            return status;
        }
    }

    if (Event)
        SetEvent(Event);

    SetLastError(LastError);
    return status;
}


//---------------------------------------------------------------------------
// File_WaitNamedPipe
//---------------------------------------------------------------------------


_FX NTSTATUS File_WaitNamedPipe(
    HANDLE NamedPipesHandle, IO_STATUS_BLOCK *IoStatusBlock,
    void *InputBuffer, ULONG InputBufferLength)
{
    FILE_PIPE_WAIT_FOR_BUFFER *ib = (FILE_PIPE_WAIT_FOR_BUFFER *)InputBuffer;
    FILE_PIPE_WAIT_FOR_BUFFER *ob = NULL;
    NTSTATUS status;

    //
    // the real WaitNamedPipeW typically passes the caller-provided PipeName
    // to NtFsControlFile to wait on it.  but unless the pipe name matches
    // OpenPipePath, we opened the pipe with a name different than what the
    // caller says.  so we need to adjust the name.
    //

    if (ib->NameLength) {

        WCHAR *ptr1;
        const WCHAR *ptr2;

        ULONG name_len = ib->NameLength / sizeof(WCHAR);
        ULONG buf_len  = sizeof(FILE_PIPE_WAIT_FOR_BUFFER)
                       + (name_len + Dll_BoxIpcPathLen + 64) * sizeof(WCHAR);
        ob = Dll_AllocTemp(buf_len);

        //
        // check that the rest of the caller-provided pipe name
        // does not already specify a sandboxed pipe name
        //

        ptr1 = (WCHAR *)ob;
        wmemcpy(ptr1, File_NamedPipe, 18);
        ptr1 += 18;
        wmemcpy(ptr1, ib->Name, name_len);
        ptr1[name_len] = L'\0';

        ptr2 = Dll_BoxIpcPath;

        for (; *ptr2; ++ptr1, ++ptr2) {
            if (*ptr1 == *ptr2)
                continue;
            if (*ptr1 == L'_' && *ptr2 == L'\\')
                continue;
            break;
        }

        if (*ptr2) {

            //
            // make sure the pipe name isn't an excluded path
            //

            ULONG mp_flags = File_MatchPath2((WCHAR *)ob, NULL, FALSE, TRUE);

            if (mp_flags) {

                ULONG monflag = MONITOR_PIPE;
                if (PATH_IS_CLOSED(mp_flags))
                    monflag |= MONITOR_DENY;
                else
                    monflag |= MONITOR_OPEN;
                SbieApi_MonitorPut2(monflag, (WCHAR *)ob, FALSE);

            } else {

                SbieApi_MonitorPut2(MONITOR_PIPE, (WCHAR *)ob, FALSE);

                //
                // create the sandboxed pipe name
                // _sandbox_ipc_path\CallerPipeName
                //
                // In compartment mode, preserve LOCAL\ before the sandbox
                // prefix to match File_GetBoxedPipeName behavior
                //

                WCHAR *caller_pipe_name = ib->Name;
                ULONG caller_name_len = name_len;

                ptr1 = ob->Name;

                if (Dll_CompartmentMode) {
                    if (caller_name_len >= 6 && _wcsnicmp(caller_pipe_name, L"LOCAL\\", 6) == 0) {
                        wcscpy(ptr1, L"LOCAL\\");
                        ptr1 += 6;
                        caller_pipe_name += 6;
                        caller_name_len -= 6;
                    }
                }

                ptr2 = Dll_BoxIpcPath;
                for (; *ptr2; ++ptr1, ++ptr2) {
                    if (*ptr2 == L'\\')
                        *ptr1 = L'_';
                    else
                        *ptr1 = *ptr2;
                }

                *ptr1 = L'\\';
                ++ptr1;

                wmemcpy(ptr1, caller_pipe_name, caller_name_len);
                ptr1[caller_name_len] = L'\0';

                //
                // initialize the rest of the new input buffer
                //

                ob->NameLength = wcslen(ob->Name) * sizeof(WCHAR);

                ob->Timeout.QuadPart = ib->Timeout.QuadPart;
                ob->TimeoutSpecified = ib->TimeoutSpecified;

                InputBuffer = ob;
                InputBufferLength = sizeof(FILE_PIPE_WAIT_FOR_BUFFER)
                                  + ob->NameLength;
            }
        }
    }

    status = __sys_NtFsControlFile(
        NamedPipesHandle, NULL, NULL, NULL, IoStatusBlock, FSCTL_PIPE_WAIT,
        InputBuffer, InputBufferLength, NULL, 0);

    if (ob)
        Dll_Free(ob);

    return status;
}


//---------------------------------------------------------------------------
// File_NtDeviceIoControlFile
//---------------------------------------------------------------------------


_FX NTSTATUS File_NtDeviceIoControlFile(
    IN HANDLE FileHandle,
    IN HANDLE Event OPTIONAL,
    IN PIO_APC_ROUTINE ApcRoutine OPTIONAL,
    IN PVOID ApcContext OPTIONAL,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN ULONG IoControlCode,
    IN PVOID InputBuffer OPTIONAL,
    IN ULONG InputBufferLength,
    OUT PVOID OutputBuffer OPTIONAL,
    IN ULONG OutputBufferLength)
{
    NTSTATUS status;

    //
    // check if this is an IOCTL that we want to deny
    //

    if (File_BlockNetParam && (
        IoControlCode == 0x00128004 ||      /* via \Device\TCP on XP */
        IoControlCode == 0x00120013))  {    /* via \Device\NSI on VISTA/7 */

        ULONG LastError;
        THREAD_DATA *TlsData = Dll_GetTlsData(&LastError);

        WCHAR *TruePath;
        WCHAR *CopyPath;

        BOOLEAN DenyAccess = FALSE;

        Dll_PushTlsNameBuffer(TlsData);

        __try {

            status = File_GetName(
                FileHandle, NULL, &TruePath, &CopyPath, NULL);

            if (_wcsnicmp(TruePath, File_NamedPipe, 8) == 0) {

                if (IoControlCode == 0x00128004 &&
                        _wcsicmp(TruePath + 8, L"TCP") == 0) {

                    DenyAccess = TRUE;

                } else if (IoControlCode == 0x00120013 &&
                        _wcsicmp(TruePath + 8, L"NSI") == 0) {

                    DenyAccess = TRUE;
                }
            }

        } __except (EXCEPTION_EXECUTE_HANDLER) {
            status = GetExceptionCode();
        }

        Dll_PopTlsNameBuffer(TlsData);
        SetLastError(LastError);

        if (DenyAccess) {
            SbieApi_Log(1314, Dll_ImageName);
            SetLastError(LastError);
            return STATUS_ACCESS_DENIED;
        }
    }

    //
    // HACK HACK: when hooking NtDeviceIoControlFile the syscall instrumentation 
    // will call SbieApi_MonitorPutMsg which will call NtDeviceIoControlFile
    // hence we need to check if the hooking is not yet done and just return
    // droppign the one log entry.
    //

    if (!__sys_NtDeviceIoControlFile) 
        return STATUS_BAD_INITIAL_PC;

    //
    // otherwise
    //

    status = __sys_NtDeviceIoControlFile(
        FileHandle, Event, ApcRoutine, ApcContext, IoStatusBlock,
        IoControlCode, InputBuffer, InputBufferLength,
        OutputBuffer, OutputBufferLength);

    //
    // replace hardware serial numbers of physical disks, note: requests
    // which complete asynchronously (STATUS_PENDING) are not handled
    //

    if (File_HideDiskSerial && status == STATUS_SUCCESS && OutputBuffer) {

        File_HidePhysicalDiskSerial(IoControlCode,
            InputBuffer, InputBufferLength, OutputBuffer, IoStatusBlock);
    }

    return status;
}


//---------------------------------------------------------------------------
// File_FakeSerialChars
//---------------------------------------------------------------------------


_FX void File_FakeSerialChars(char *Serial, ULONG Length)
{
    //
    // derive the replacement from the box name and the real serial number,
    // so that it is stable across processes and restarts but differs per box,
    // keep digits as digits and letters as letters to retain the format
    //

    ULONG Seed = 2166136261UL;    // FNV-1a
    const UCHAR *ptr;
    ULONG i, len;

    ptr = (const UCHAR *)Dll_BoxName;
    len = (ULONG)(wcslen(Dll_BoxName) * sizeof(WCHAR));
    for (i = 0; i < len; i++)
        Seed = (Seed ^ ptr[i]) * 16777619;
    for (i = 0; i < Length; i++)
        Seed = (Seed ^ (UCHAR)Serial[i]) * 16777619;
    if (! Seed)
        Seed = 1;

    for (i = 0; i < Length; i++) {

        char c = Serial[i];

        Seed ^= Seed << 13;         // xorshift32
        Seed ^= Seed >> 17;
        Seed ^= Seed << 5;

        if (c >= '0' && c <= '9')
            Serial[i] = (char)('0' + Seed % 10);
        else if (c >= 'A' && c <= 'Z')
            Serial[i] = (char)('A' + Seed % 26);
        else if (c >= 'a' && c <= 'z')
            Serial[i] = (char)('a' + Seed % 26);
    }
}


//---------------------------------------------------------------------------
// File_FakeSerialString
//---------------------------------------------------------------------------


_FX void File_FakeSerialString(char *Serial, ULONG MaxLength)
{
    //
    // serial numbers are often padded with spaces, only replace
    // the actual serial number so that the padding is preserved
    //

    ULONG Start = 0, End = 0;

    while (End < MaxLength && Serial[End])
        End++;
    while (Start < End && Serial[Start] == ' ')
        Start++;
    while (End > Start && Serial[End - 1] == ' ')
        End--;

    if (End > Start)
        File_FakeSerialChars(Serial + Start, End - Start);
}


//---------------------------------------------------------------------------
// File_FakeAtaSerial
//---------------------------------------------------------------------------


_FX void File_FakeAtaSerial(UCHAR *IdentifyData)
{
    //
    // the serial number is stored in the words 10-19 of the ATA IDENTIFY data,
    // with the bytes of each word swapped, unswap it first so that the result
    // matches the serial number reported by IOCTL_STORAGE_QUERY_PROPERTY
    //

    UCHAR *Field = IdentifyData + FILE_ATA_IDENTIFY_SERIAL_OFFSET;
    char Serial[FILE_ATA_IDENTIFY_SERIAL_LENGTH];
    ULONG i;

    for (i = 0; i < FILE_ATA_IDENTIFY_SERIAL_LENGTH; i += 2) {
        Serial[i] = Field[i + 1];
        Serial[i + 1] = Field[i];
    }

    File_FakeSerialString(Serial, FILE_ATA_IDENTIFY_SERIAL_LENGTH);

    for (i = 0; i < FILE_ATA_IDENTIFY_SERIAL_LENGTH; i += 2) {
        Field[i] = Serial[i + 1];
        Field[i + 1] = Serial[i];
    }
}


//---------------------------------------------------------------------------
// File_HidePhysicalDiskSerial
//---------------------------------------------------------------------------


_FX void File_HidePhysicalDiskSerial(
    ULONG IoControlCode, void *InputBuffer, ULONG InputBufferLength,
    void *OutputBuffer, IO_STATUS_BLOCK *IoStatusBlock)
{
    ULONG_PTR OutputLength;

    if (IoControlCode != FILE_IOCTL_STORAGE_QUERY_PROPERTY &&
        IoControlCode != FILE_SMART_RCV_DRIVE_DATA &&
        IoControlCode != FILE_IOCTL_ATA_PASS_THROUGH &&
        IoControlCode != FILE_IOCTL_ATA_PASS_THROUGH_DIRECT)
        return;

    //
    // the buffers belong to the caller, guard against invalid memory
    //

    __try {

        OutputLength = IoStatusBlock->Information;

        if (IoControlCode == FILE_IOCTL_STORAGE_QUERY_PROPERTY) {

            FILE_STORAGE_PROPERTY_QUERY *Query = (FILE_STORAGE_PROPERTY_QUERY *)InputBuffer;

            if (! Query || InputBufferLength < (ULONG)FIELD_OFFSET(FILE_STORAGE_PROPERTY_QUERY, AdditionalParameters))
                __leave;
            if (Query->QueryType != FILE_PROPERTY_STANDARD_QUERY)
                __leave;

            if (Query->PropertyId == FILE_STORAGE_DEVICE_PROPERTY) {

                FILE_STORAGE_DEVICE_DESCRIPTOR *Desc = (FILE_STORAGE_DEVICE_DESCRIPTOR *)OutputBuffer;
                ULONG Offset;

                if (OutputLength < sizeof(FILE_STORAGE_DEVICE_DESCRIPTOR))
                    __leave;
                Offset = Desc->SerialNumberOffset;
                if (Offset < sizeof(FILE_STORAGE_DEVICE_DESCRIPTOR) || Offset >= OutputLength)
                    __leave;

                File_FakeSerialString((char *)Desc + Offset, (ULONG)(OutputLength - Offset));

            } else if (Query->PropertyId == FILE_STORAGE_ADAPTER_PROTOCOL_SPECIFIC_PROPERTY ||
                       Query->PropertyId == FILE_STORAGE_DEVICE_PROTOCOL_SPECIFIC_PROPERTY) {

                FILE_STORAGE_PROTOCOL_DATA_DESCRIPTOR *Desc = (FILE_STORAGE_PROTOCOL_DATA_DESCRIPTOR *)OutputBuffer;
                FILE_STORAGE_PROTOCOL_SPECIFIC_DATA *Data = &Desc->ProtocolSpecificData;
                ULONG_PTR DataOffset;

                if (OutputLength < sizeof(FILE_STORAGE_PROTOCOL_DATA_DESCRIPTOR))
                    __leave;

                // ProtocolDataOffset is relative to ProtocolSpecificData
                DataOffset = (ULONG_PTR)FIELD_OFFSET(FILE_STORAGE_PROTOCOL_DATA_DESCRIPTOR, ProtocolSpecificData) + (ULONG_PTR)Data->ProtocolDataOffset;

                if (Data->ProtocolType == FILE_PROTOCOL_TYPE_NVME &&
                        Data->DataType == FILE_NVME_DATA_TYPE_IDENTIFY &&
                        Data->ProtocolDataRequestValue == FILE_NVME_IDENTIFY_CNS_CONTROLLER) {

                    if (Data->ProtocolDataLength < FILE_NVME_IDENTIFY_SERIAL_OFFSET + FILE_NVME_IDENTIFY_SERIAL_LENGTH ||
                            DataOffset + FILE_NVME_IDENTIFY_SERIAL_OFFSET + FILE_NVME_IDENTIFY_SERIAL_LENGTH > OutputLength)
                        __leave;

                    File_FakeSerialString((char *)Desc + DataOffset + FILE_NVME_IDENTIFY_SERIAL_OFFSET, FILE_NVME_IDENTIFY_SERIAL_LENGTH);

                } else if (Data->ProtocolType == FILE_PROTOCOL_TYPE_ATA &&
                        Data->DataType == FILE_ATA_DATA_TYPE_IDENTIFY) {

                    if (Data->ProtocolDataLength < FILE_ATA_IDENTIFY_SERIAL_OFFSET + FILE_ATA_IDENTIFY_SERIAL_LENGTH ||
                            DataOffset + FILE_ATA_IDENTIFY_SERIAL_OFFSET + FILE_ATA_IDENTIFY_SERIAL_LENGTH > OutputLength)
                        __leave;

                    File_FakeAtaSerial((UCHAR *)Desc + DataOffset);
                }
            }

        } else if (IoControlCode == FILE_SMART_RCV_DRIVE_DATA) {

            UCHAR Command;

            if (! InputBuffer || InputBufferLength <= FILE_SMART_IN_COMMAND_REG_OFFSET)
                __leave;
            Command = ((UCHAR *)InputBuffer)[FILE_SMART_IN_COMMAND_REG_OFFSET];
            if (Command != FILE_ATA_ID_CMD && Command != FILE_ATAPI_ID_CMD)
                __leave;
            if (OutputLength < FILE_SMART_OUT_BUFFER_OFFSET + FILE_ATA_IDENTIFY_SERIAL_OFFSET + FILE_ATA_IDENTIFY_SERIAL_LENGTH)
                __leave;

            File_FakeAtaSerial((UCHAR *)OutputBuffer + FILE_SMART_OUT_BUFFER_OFFSET);

        } else { // FILE_IOCTL_ATA_PASS_THROUGH or FILE_IOCTL_ATA_PASS_THROUGH_DIRECT

            FILE_ATA_PASS_THROUGH *AptIn = (FILE_ATA_PASS_THROUGH *)InputBuffer;
            FILE_ATA_PASS_THROUGH *AptOut = (FILE_ATA_PASS_THROUGH *)OutputBuffer;
            UCHAR *IdentifyData;

            if (! AptIn || InputBufferLength < sizeof(FILE_ATA_PASS_THROUGH) ||
                    OutputLength < sizeof(FILE_ATA_PASS_THROUGH))
                __leave;

            // the output task file contains the status, so check the input one
            if (AptIn->CurrentTaskFile[FILE_ATA_TASK_FILE_COMMAND] != FILE_ATA_ID_CMD &&
                    AptIn->CurrentTaskFile[FILE_ATA_TASK_FILE_COMMAND] != FILE_ATAPI_ID_CMD)
                __leave;
            if (AptOut->DataTransferLength < FILE_ATA_IDENTIFY_SERIAL_OFFSET + FILE_ATA_IDENTIFY_SERIAL_LENGTH)
                __leave;

            if (IoControlCode == FILE_IOCTL_ATA_PASS_THROUGH) {

                // the data follows the structure within the output buffer
                if (AptOut->DataBuffer < sizeof(FILE_ATA_PASS_THROUGH) ||
                        AptOut->DataBuffer + FILE_ATA_IDENTIFY_SERIAL_OFFSET + FILE_ATA_IDENTIFY_SERIAL_LENGTH > OutputLength)
                    __leave;
                IdentifyData = (UCHAR *)AptOut + AptOut->DataBuffer;

            } else {

                // the data is in a separate caller provided buffer
                IdentifyData = (UCHAR *)AptOut->DataBuffer;
                if (! IdentifyData)
                    __leave;
            }

            File_FakeAtaSerial(IdentifyData);
        }

    } __except (EXCEPTION_EXECUTE_HANDLER) {
    }
}
