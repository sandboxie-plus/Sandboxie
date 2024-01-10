#include "framework.h"
#include <stdio.h>
#include "..\Common\helpers.h"
#include "..\ImDisk\inc\imdproxy.h"
#include "..\ImDisk\inc\imdisk.h"
#include "ImDiskIO.h"
#include "ImBox.h"


extern "C" {

#define ObjectNameInformation 1

typedef struct _OBJECT_NAME_INFORMATION {
    UNICODE_STRING Name;
} OBJECT_NAME_INFORMATION, *POBJECT_NAME_INFORMATION;

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


typedef enum _FSINFOCLASS {
    FileFsVolumeInformation       = 1,  // FILE_FS_VOLUME_INFORMATION
    FileFsLabelInformation,      // 2
    FileFsSizeInformation,       // 3
    FileFsDeviceInformation,     // 4
    FileFsAttributeInformation,  // 5   // FILE_FS_ATTRIBUTE_INFORMATION
    FileFsControlInformation,    // 6
    FileFsFullSizeInformation,   // 7
    FileFsObjectIdInformation,   // 8
    FileFsDriverPathInformation, // 9
    FileFsMaximumInformation
} FS_INFORMATION_CLASS, *PFS_INFORMATION_CLASS;

NTSYSCALLAPI NTSTATUS NTAPI NtQueryVolumeInformationFile(
    IN  HANDLE FileHandle,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    OUT PVOID FsInformation,
    IN  ULONG Length,
    IN  ULONG FsInformationClass
);

typedef struct _FILE_FS_VOLUME_INFORMATION {
  LARGE_INTEGER VolumeCreationTime;
  ULONG         VolumeSerialNumber;
  ULONG         VolumeLabelLength;
  BOOLEAN       SupportsObjects;
  WCHAR         VolumeLabel[1];
} FILE_FS_VOLUME_INFORMATION, *PFILE_FS_VOLUME_INFORMATION;

typedef struct _FILE_FS_ATTRIBUTE_INFORMATION {
    ULONG FileSystemAttributes;
    LONG MaximumComponentNameLength;
    ULONG FileSystemNameLength;
    WCHAR FileSystemName[1];
} FILE_FS_ATTRIBUTE_INFORMATION, *PFILE_FS_ATTRIBUTE_INFORMATION;

NTSTATUS NTAPI RtlSetThreadErrorMode(IN ULONG NewMode, OUT PULONG OldMode);


bool IsVolumeUnRecognized(std::wstring NtPath);
bool FormatVolume(LPCWSTR root, LPCWSTR fs, LPCWSTR label);


WCHAR WINAPI MyImDiskFindFreeDriveLetter();
HANDLE WINAPI MyImDiskOpenDeviceByMountPoint(LPCWSTR MountPoint, DWORD AccessMode);
BOOL WINAPI MyImDiskCliValidateDriveLetterTarget(LPCWSTR DriveLetter, LPCWSTR ValidTargetPath);
}

struct SImDiskIO
{
	std::wstring Mount;
	std::wstring Format;
	std::wstring Params;
	HANDLE hImDisk;

    std::wstring Proxy;
    HANDLE hEvent;
    HANDLE hMapping;
    WCHAR* pSection;
};

CImDiskIO::CImDiskIO(CAbstractIO* pIO, const std::wstring& Mount, const std::wstring& Format, const std::wstring& Params)
{
	m = new SImDiskIO;

	m->Mount = Mount;
	m->Format = Format;
	m->Params = Params;
	m->hImDisk = INVALID_HANDLE_VALUE;

    m->hEvent = NULL;
    m->hMapping = NULL;
    m->pSection = NULL;

	m_pIO = pIO;
	m_hThread = INVALID_HANDLE_VALUE;
}

CImDiskIO::~CImDiskIO()
{
	if (m_hThread != INVALID_HANDLE_VALUE) {
        if (WaitForSingleObject(m_hThread, 60 * 1000) == WAIT_TIMEOUT)
            TerminateThread(m_hThread, -1);
		CloseHandle(m_hThread);
	}

	if(m) delete m;
}

void CImDiskIO::SetProxyName(const std::wstring& Name)
{
    if (m) m->Proxy = Name;
}

void CImDiskIO::SetMountEvent(HANDLE hEvent)
{
    if (m) m->hEvent = hEvent;
}

void CImDiskIO::SetMountSection(HANDLE hMapping, WCHAR* pSection)
{
    if (m) m->hMapping = hMapping;
    if (m) m->pSection = pSection;
}

DWORD WINAPI CImDiskIO_Thread(LPVOID lpThreadParameter)
{
	//
	// !!! NOTE !!! After this thread was created the rest of the code does not touch 
	// the m members which we use here so we don't bother with explicit synchronization
	// hence this thread is responsible freeing m
	//

	SImDiskIO* m = ((SImDiskIO*)lpThreadParameter);
	
	std::wstring Device;

	DWORD exit_code;
    do {
        Sleep(100);
        HANDLE handle = (HANDLE)MyImDiskOpenDeviceByMountPoint(m->Mount.c_str(), 0);
        if (handle != INVALID_HANDLE_VALUE) {

            BYTE buffer[MAX_PATH];
            DWORD length = sizeof(buffer);
            if (NT_SUCCESS(NtQueryObject(handle, (OBJECT_INFORMATION_CLASS)ObjectNameInformation, buffer, length, &length))) {
                UNICODE_STRING* uni = &((OBJECT_NAME_INFORMATION*)buffer)->Name;
                length = uni->Length / sizeof(WCHAR);
                if (uni->Buffer) {
                    uni->Buffer[length] = 0;
                    Device = uni->Buffer;
                }
            }

			CloseHandle(handle);

            break;
        }
        GetExitCodeProcess(m->hImDisk, &exit_code);
    } while (exit_code == STILL_ACTIVE);

	CloseHandle(m->hImDisk); m->hImDisk = INVALID_HANDLE_VALUE;

	
	if (!Device.empty() && !m->Format.empty()) {

		std::wstring Drive;

		if (!IsVolumeUnRecognized(Device)) {
			DbgPrint(L"The volume: %s was recognized, format skipped.\n", Device.c_str());
		} 
		else

     	if ((m->Mount.length() == 2 && m->Mount[1] == L':') // check if mount is a drive letter
			|| (m->Mount.length() == 3 && m->Mount[1] == L':' && m->Mount[2] == L'\\')) {
			Drive = m->Mount;
		}
		else {

			WCHAR drive = MyImDiskFindFreeDriveLetter();
			if (!drive) {
				DbgPrint(L"No free drive letter found.\n");
			}
			else {
				Drive = L" :";
				Drive[0] = drive;
			}

			if (!DefineDosDevice(DDD_RAW_TARGET_PATH, Drive.c_str(), Device.c_str())) {
				DbgPrint(L"Failed to Mount drive letter.\n");
				Drive.clear();
			}
		}

		if (!Drive.empty()) {

			if (MyImDiskCliValidateDriveLetterTarget(Drive.c_str(), Device.c_str())) {

				LPCWSTR fs = m->Format.c_str();
				LPCWSTR label = wcschr(fs, L':');
				if (!label) label = L"";
				else *(*(LPWSTR*)&label)++ = L'\0';

                //for (int i = 0; i < 3; i++) {
                    if (FormatVolume(Drive.c_str(), fs, label)) {

                        if (!IsVolumeUnRecognized(Device)) // check success
                            DbgPrint(L"Successfully Formatted: %s\n", m->Mount.c_str());
                        else {
                            DbgPrint(L"Failed to Format: %s\n", m->Mount.c_str());
                //            Sleep(1000);
                //            continue; // retry
                        }
                    }
                    else // fails only when lib is not available
                        DbgPrint(L"Can not Format: %s\n", m->Mount.c_str());
                //    break;
                //}
			}

			if (Drive != m->Mount) {
				if (!DefineDosDevice(DDD_REMOVE_DEFINITION | DDD_EXACT_MATCH_ON_REMOVE | DDD_RAW_TARGET_PATH, Drive.c_str(), Device.c_str())) {
					DbgPrint(L"Failed to Unmount drive letter.\n");
				}
			}
		}
	}

    if (m->pSection) {
        wmemcpy(m->pSection, Device.c_str(), Device.length() + 1);
        UnmapViewOfFile(m->pSection);
    }
    if(m->hMapping) 
        CloseHandle(m->hMapping);
    
    if (m->hEvent) {
        SetEvent(m->hEvent);
        CloseHandle(m->hEvent);
    }

	delete m;

	return 0;
}

int CImDiskIO::DoComm()
{
	HANDLE hFileMap;
	ULARGE_INTEGER map_size;
	BYTE *shm_view, *main_buf;
	struct s_req_block {unsigned char request_code, pad[7]; ULONGLONG offset; ULONGLONG length;} *req_block;
	struct s_resp_block {unsigned char errorno, pad[7]; ULONGLONG length;} *resp_block;
	struct s_trim_block {unsigned char request_code, pad[7]; unsigned int length;} *trim_block;
	HANDLE shm_request_event, shm_response_event;

    if (m->Proxy.empty()){
#ifdef _M_ARM64
	    ULONG64 ctr = _ReadStatusReg(ARM64_CNTVCT);
#else
	    ULONG64 ctr = __rdtsc();
#endif
        m->Proxy = L"ImBox" + std::to_wstring(ctr);
    }

	map_size.QuadPart = DEF_BUFFER_SIZE + IMDPROXY_HEADER_SIZE;

	if (!(hFileMap = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE | SEC_COMMIT, map_size.HighPart, map_size.LowPart, (L"Global\\" + m->Proxy).c_str())))
        return ERR_FILE_MAPPING;
	if (!(shm_view = (BYTE*)MapViewOfFile(hFileMap, FILE_MAP_WRITE, 0, 0, 0)))
		return ERR_FILE_MAPPING;

	m_pIO->PrepViewOfFile(shm_view);

	main_buf = shm_view + IMDPROXY_HEADER_SIZE;
	req_block = (s_req_block*)shm_view;
	resp_block = (s_resp_block*)shm_view;
	trim_block = (s_trim_block*)shm_view;

	if (!(shm_request_event = CreateEvent(NULL, FALSE, FALSE, (L"Global\\" + m->Proxy + L"_Request").c_str())) || GetLastError() == ERROR_ALREADY_EXISTS)
		return ERR_CREATE_EVENT;
	if (!(shm_response_event = CreateEvent(NULL, FALSE, FALSE, (L"Global\\" + m->Proxy + L"_Response").c_str())))
		return ERR_CREATE_EVENT;

	STARTUPINFO si = {sizeof si};
	PROCESS_INFORMATION pi;
	std::wstring cmd = L"imdisk -a -t proxy -o shm -f " + m->Proxy;
	if (!m->Mount.empty()) cmd += L" -m \"" + m->Mount + L"\"";
	if (!m->Params.empty())cmd += L" " + m->Params;
	if (!CreateProcess(NULL, (WCHAR*)cmd.c_str(), NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
		DbgPrint(L"Failed to run imdisk.exe.\n");
		return ERR_IMDISK_FAILED;
	}
	NtClose(pi.hThread);

	if (!m->Mount.empty()) {

        //
        // clear format directive if formatting is not allowed for this disk e.g. non empty image file
        // to force format you can use imdisk: params="-p \"/fs:ntfs /q /y\""
        //

        if (!m_pIO->CanBeFormated())
            m->Format.clear();

		m->hImDisk = pi.hProcess;
		m_hThread = CreateThread(NULL, 0, CImDiskIO_Thread, m, 0, NULL);
		m = NULL;
	} 
	else {
		NtClose(pi.hProcess);
	}

	LARGE_INTEGER t;
	t.QuadPart = -100000000; // A negative value specifies an interval relative to the current time in units of 100 nanoseconds.
	if (NtWaitForSingleObject(shm_request_event, FALSE, &t) != STATUS_SUCCESS || req_block->request_code != IMDPROXY_REQ_INFO)
		return ERR_IMDISK_TIMEOUT;

	IMDPROXY_INFO_RESP proxy_info = {0};
	proxy_info.file_size = m_pIO->GetDiskSize();
	proxy_info.req_alignment = 1;
	proxy_info.flags = IMDPROXY_FLAG_SUPPORTS_UNMAP; // TRIM
	memcpy(shm_view, &proxy_info, sizeof proxy_info);

	for (;;) {
		NtSignalAndWaitForSingleObject(shm_response_event, shm_request_event, FALSE, NULL);

		if (req_block->request_code == IMDPROXY_REQ_READ) {
			if (!m_pIO->DiskRead(main_buf, req_block->length, req_block->offset)) {
				DbgPrint(L"DiskRead error.\n");
			}
		}
		else if (req_block->request_code == IMDPROXY_REQ_WRITE) {
			if (!m_pIO->DiskWrite(main_buf, req_block->length, req_block->offset)) {
				DbgPrint(L"DiskWrite error, SOME DATA WILL BE LOST.");
			}
		}
		else if (req_block->request_code == IMDPROXY_REQ_UNMAP) {
			m_pIO->TrimProcess((DEVICE_DATA_SET_RANGE*)main_buf, trim_block->length / sizeof(DEVICE_DATA_SET_RANGE));
		}
		else if (req_block->request_code == IMDPROXY_REQ_CLOSE) {
			return ERR_OK;
		}
		else { // unknown command
			DbgPrint(L"Unknown Command: %d\n", req_block->request_code);
			return ERR_UNKNOWN_COMMAND;
		}

		resp_block->errorno = 0;
		resp_block->length = req_block->length;
	}
}


//static void disp_message(WCHAR *disp_text, WCHAR *arg, BOOL wait)
//{
//	DWORD dw;
//
//	_snwprintf(txt, _countof(txt) - 1, disp_text, arg);
//	WTSSendMessage(WTS_CURRENT_SERVER_HANDLE, WTSGetActiveConsoleSessionId(), L"ImDisk", 14, txt, (wcslen(txt) + 1) * sizeof(WCHAR), MB_OK | MB_ICONERROR, 0, &dw, wait);
//}
//
//static void disp_err_mem()
//{
//	if (GetTickCount() - err_time >= 10000) {
//		disp_message(L"Not enough memory to write data into %s.\nSome data will be lost.", drive_arg, TRUE);
//		err_time = GetTickCount();
//	}
//}

extern "C" {

bool IsVolumeUnRecognized(std::wstring NtPath)
{
    if (NtPath.back() != L'\\') NtPath.push_back(L'\\');

    HANDLE handle;
    IO_STATUS_BLOCK iosb;

    UNICODE_STRING objname;
    RtlInitUnicodeString(&objname, NtPath.c_str());

    OBJECT_ATTRIBUTES objattrs;
    InitializeObjectAttributes(
        &objattrs, &objname, OBJ_CASE_INSENSITIVE, NULL, NULL);
    
    ULONG OldMode;
    RtlSetThreadErrorMode(0x10u, &OldMode);
    NTSTATUS status = NtCreateFile(
        &handle, GENERIC_READ | SYNCHRONIZE, &objattrs,
        &iosb, NULL, 0, FILE_SHARE_VALID_FLAGS,
        FILE_OPEN,
        FILE_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT,
        NULL, 0);
    RtlSetThreadErrorMode(OldMode, 0i64);

    if (NT_SUCCESS(status))
    {
        union {
            FILE_FS_ATTRIBUTE_INFORMATION fsInfo;
            BYTE fsInfoBuff[64];
        } u;
        if (NT_SUCCESS(NtQueryVolumeInformationFile(handle, &iosb, &u.fsInfo, sizeof(u), FileFsAttributeInformation))) {
            u.fsInfo.FileSystemName[u.fsInfo.FileSystemNameLength / sizeof(wchar_t)] = 0;
            DbgPrint(L"Recognized FileSystem: %s\n", u.fsInfo.FileSystemName);
        }

        NtClose(handle);
    }

    if (status == STATUS_UNRECOGNIZED_VOLUME)
        return true;

    if (!NT_SUCCESS(status))
        DbgPrint(L"NtQueryVolumeInformationFile failed 0x%08X.\n", status);
    return false;
}

// types from winfile
typedef BOOLEAN(WINAPI *FMIFS_CALLBACK)(ULONG PacketType, ULONG PacketLength, PVOID PacketData);
typedef void (WINAPI* PFORMAT) (PWSTR DriveName, ULONG MediaType, PWSTR FileSystemName, PWSTR Label, BOOLEAN Quick, FMIFS_CALLBACK Callback);

static BOOLEAN WINAPI my_format_callback(ULONG PacketType, ULONG PacketLength, PVOID PacketData) {
	return 1;
}

bool FormatVolume(LPCWSTR root, LPCWSTR fs, LPCWSTR label)
{
    bool ret = false;
    HMODULE fmifs = LoadLibrary(L"fmifs");
    if (fmifs != NULL) {
        PFORMAT Format = (PFORMAT)GetProcAddress(fmifs, "Format");
        if (Format != NULL) {
            Format((PWSTR)root, 0, (PWSTR)fs, (PWSTR)label, TRUE, my_format_callback);
            ret = true;
        }
        FreeLibrary(fmifs);
    }
	return ret;
}


// ImDisk

HANDLE WINAPI MyImDiskOpenDeviceByName(PUNICODE_STRING FileName, DWORD AccessMode)
{
    NTSTATUS status;
    HANDLE handle;
    OBJECT_ATTRIBUTES object_attrib;
    IO_STATUS_BLOCK io_status;

    InitializeObjectAttributes(&object_attrib,
        FileName,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL);

    status = NtOpenFile(&handle,
        SYNCHRONIZE | AccessMode,
        &object_attrib,
        &io_status,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        FILE_NON_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT);

    if (!NT_SUCCESS(status))
    {
        SetLastError(RtlNtStatusToDosError(status));
        return INVALID_HANDLE_VALUE;
    }

    return handle;
}

HANDLE WINAPI MyImDiskOpenDeviceByNumber(DWORD DeviceNumber, DWORD AccessMode)
{
    WCHAR device_path[_countof(IMDISK_DEVICE_BASE_NAME) + 16];

    UNICODE_STRING file_name;

    // Build device path, e.g. \Device\ImDisk2
    _snwprintf_s(device_path, ARRAYSIZE(device_path), _countof(device_path),
        IMDISK_DEVICE_BASE_NAME L"%u", DeviceNumber);
    device_path[_countof(device_path) - 1] = 0;

    RtlInitUnicodeString(&file_name, device_path);

    return MyImDiskOpenDeviceByName(&file_name, AccessMode);
}

HANDLE WINAPI MyImDiskOpenDeviceByMountPoint(LPCWSTR MountPoint, DWORD AccessMode)
{
    UNICODE_STRING DeviceName;
    WCHAR DriveLetterPath[] = L"\\DosDevices\\ :";
    PREPARSE_DATA_BUFFER ReparseData = NULL;
    HANDLE h;

    if ((MountPoint[0] != 0) &&
        ((wcscmp(MountPoint + 1, L":") == 0) ||
        (wcscmp(MountPoint + 1, L":\\") == 0)))
    {
        DriveLetterPath[12] = MountPoint[0];

        RtlInitUnicodeString(&DeviceName, DriveLetterPath);
    }
    else if (((wcsncmp(MountPoint, L"\\\\?\\", 4) == 0) ||
        (wcsncmp(MountPoint, L"\\\\.\\", 4) == 0)) &&
        (wcschr(MountPoint + 4, L'\\') == NULL))
    {
        return CreateFile(MountPoint, AccessMode,
            FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
    }
    else
    {
        HANDLE hDir;
        DWORD dw;
        DWORD buffer_size =
            FIELD_OFFSET(REPARSE_DATA_BUFFER, MountPointReparseBuffer) +
            MAXIMUM_REPARSE_DATA_BUFFER_SIZE;

        hDir = CreateFile(MountPoint, GENERIC_READ,
            FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
            OPEN_EXISTING,
            FILE_FLAG_BACKUP_SEMANTICS |
            FILE_FLAG_OPEN_REPARSE_POINT, NULL);

        if (hDir == INVALID_HANDLE_VALUE)
            return INVALID_HANDLE_VALUE;

        ReparseData = (PREPARSE_DATA_BUFFER)HeapAlloc(GetProcessHeap(),
            HEAP_GENERATE_EXCEPTIONS | HEAP_ZERO_MEMORY,
            buffer_size);

        if (!DeviceIoControl(hDir, FSCTL_GET_REPARSE_POINT,
            NULL, 0,
            ReparseData, buffer_size,
            &dw, NULL))
        {
            DWORD last_error = GetLastError();
            CloseHandle(hDir);
            HeapFree(GetProcessHeap(), 0, ReparseData);
            SetLastError(last_error);
            return INVALID_HANDLE_VALUE;
        }

        CloseHandle(hDir);

        if (ReparseData->ReparseTag != IO_REPARSE_TAG_MOUNT_POINT)
        {
            HeapFree(GetProcessHeap(), 0, ReparseData);
            SetLastError(ERROR_NOT_A_REPARSE_POINT);
            return INVALID_HANDLE_VALUE;
        }

        DeviceName.Length =
            ReparseData->MountPointReparseBuffer.SubstituteNameLength;
        
        DeviceName.Buffer = (PWSTR)
            ((PUCHAR)ReparseData->MountPointReparseBuffer.PathBuffer +
            ReparseData->MountPointReparseBuffer.SubstituteNameOffset);
        
        DeviceName.MaximumLength = DeviceName.Length;
    }
    
    if (DeviceName.Buffer[(DeviceName.Length >> 1) - 1] == L'\\')
    {
        DeviceName.Buffer[(DeviceName.Length >> 1) - 1] = 0;
        DeviceName.Length -= 2;
    }

    h = MyImDiskOpenDeviceByName(&DeviceName, AccessMode);

    if (ReparseData != NULL)
        HeapFree(GetProcessHeap(), 0, ReparseData);

    return h;
}

BOOL WINAPI MyImDiskCliValidateDriveLetterTarget(LPCWSTR DriveLetter, LPCWSTR ValidTargetPath)
{
    WCHAR target[MAX_PATH];

    if (QueryDosDevice(DriveLetter, target, _countof(target)))
    {
        if (wcscmp(target, ValidTargetPath) == 0)
        {
            return TRUE;
        }

        DbgPrint(L"Drive letter %s points to %s instead of expected %s.\n", DriveLetter, target, ValidTargetPath);
    }
    else if (GetLastError() != ERROR_FILE_NOT_FOUND)
    {
        DbgPrint(L"Error verifying temporary drive letter:");
    }
    
    return FALSE;
}

//BOOL WINAPI MyImDiskGetDeviceListEx(IN ULONG ListLength, OUT ULONG *DeviceList)
//{
//    UNICODE_STRING file_name;
//    HANDLE driver;
//    ULONG dw;
//
//    RtlInitUnicodeString(&file_name, IMDISK_CTL_DEVICE_NAME);
//
//    driver = MyImDiskOpenDeviceByName(&file_name, GENERIC_READ);
//    if (driver == INVALID_HANDLE_VALUE)
//        return 0;
//
//    if (!DeviceIoControl(driver,
//        IOCTL_IMDISK_QUERY_DRIVER,
//        NULL, 0,
//        DeviceList, ListLength << 2,
//        &dw, NULL))
//    {
//        DWORD dwLastError = GetLastError();
//        NtClose(driver);
//        SetLastError(dwLastError);
//        return FALSE;
//    }
//
//    NtClose(driver);
//
//    if ((dw == sizeof(ULONG)) &
//        (*DeviceList > 0))
//    {
//        SetLastError(ERROR_MORE_DATA);
//        return FALSE;
//    }
//
//    SetLastError(NO_ERROR);
//    return TRUE;
//}

WCHAR WINAPI MyImDiskFindFreeDriveLetter()
{
    DWORD logical_drives = GetLogicalDrives();
    WCHAR search;

    for (search = L'Z'; search >= L'I'; search--)
    {
        if ((logical_drives & (1 << (search - L'A'))) == 0)
        {
            return search;
        }
    }

    return 0;
}

}


#include <intrin.h>
#ifndef _M_ARM64
#include <emmintrin.h>
#endif

bool data_search_std(void *_ptr, int size)
{
	unsigned char* ptr = (unsigned char*)_ptr;
	long *scan_ptr;

	if (!size) return FALSE;
	scan_ptr = (long*)ptr;
	ptr = (BYTE*)ptr + size - sizeof(long);
	if (*(long*)ptr) return TRUE;	// check if the last long not 0
	*(long*)ptr = 1;				// set last long to 1 to ensure termination
	while (!*(scan_ptr++));
	*(long*)ptr = 0;				// restore last long to 0
	return --scan_ptr != (long*)ptr;
}

#ifdef _M_ARM64
bool (*data_search)(void* ptr, int size) = data_search_std;
#else
bool data_search_sse2(void *_ptr, int size)
{
	unsigned char* ptr = (unsigned char*)_ptr;
	unsigned char *end_ptr;
	__m128i zero;

	if (!size) return FALSE;
	zero = _mm_setzero_si128();
	end_ptr = ptr + size - sizeof(__m128i);
	if ((unsigned short)_mm_movemask_epi8(_mm_cmpeq_epi8(*(__m128i*)end_ptr, zero)) != 0xffff) return TRUE;
	*end_ptr = 1;
	while ((unsigned short)_mm_movemask_epi8(_mm_cmpeq_epi8(*(__m128i*)ptr, zero)) == 0xffff) ptr += sizeof(__m128i);
	*end_ptr = 0;
	return ptr != end_ptr;
}

bool data_search_avx(void *_ptr, int size)
{
	unsigned char* ptr = (unsigned char*)_ptr;
	unsigned char *end_ptr;
	__m256i one;

	if (!size) return FALSE;
	one = _mm256_set1_epi8(0xff);
	end_ptr = ptr + size - sizeof(__m256i);
	if (!_mm256_testz_si256(*(__m256i*)end_ptr, one)) return TRUE;
	*end_ptr = 1;
	while (_mm256_testz_si256(*(__m256i*)ptr, one)) ptr += sizeof(__m256i);
	*end_ptr = 0;
	return ptr != end_ptr;
}

extern "C" {
	void* pick_data_search()
	{
		bool (*ret)(void*, int);

		int cpuInfo[4];
		__cpuid(cpuInfo, 1);
#ifndef _WIN64
		ret = data_search_std;
		if (cpuInfo[3] & 0x4000000)
#endif
			ret = data_search_sse2;

		bool osUsesXSAVE_XRSTORE = cpuInfo[2] & (1 << 27);
		bool cpuAVXSuport = cpuInfo[2] & (1 << 28);
		if (osUsesXSAVE_XRSTORE && cpuAVXSuport) {
			unsigned long long xcrFeatureMask = _xgetbv(_XCR_XFEATURE_ENABLED_MASK);
			if ((xcrFeatureMask & 0x6) == 0x6)
				ret = data_search_avx;
		}

		return ret;
	}
}

bool (*data_search)(void* ptr, int size) = (bool (*)(void*, int))pick_data_search();
#endif
