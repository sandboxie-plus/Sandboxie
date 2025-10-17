#include "framework.h"
#include "ImageFileIO.h"
#include "ImBox.h"
#include "..\Common\helpers.h"

typedef enum _FILE_INFORMATION_CLASS_ {
    // end_wdm
    FileDirectoryInformation_         = 1,
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
    FileRenameInformationBypassAccessCheck,  // 56 - kernel mode only
    FileLinkInformationBypassAccessCheck,    // 57 - kernel mode only
    FileVolumeNameInformation,               // 58
    FileIdInformation,                       // 59
    FileIdExtdDirectoryInformation,          // 60
    FileReplaceCompletionInformation,
    FileHardLinkFullIdInformation,
    FileIdExtdBothDirectoryInformation,
    FileDispositionInformationEx,
    FileRenameInformationEx,                        // 65
    FileRenameInformationExBypassAccessCheck,       // 66 - kernel mode only
    FileDesiredStorageClassInformation,             // 67
    FileStatInformation,                            // 68
    FileMemoryPartitionInformation,                 // 69
    FileStatLxInformation,                          // 70
    FileCaseSensitiveInformation,                   // 71
    FileLinkInformationEx,                          // 72
    FileLinkInformationExBypassAccessCheck,         // 73 - kernel mode only
    FileStorageReserveIdInformation,                // 74
    FileCaseSensitiveInformationForceAccessCheck,   // 75

    FileMaximumInformation
} FILE_INFORMATION_CLASS_, *PFILE_INFORMATION_CLASS_;


// FileBasicInformation
typedef struct _FILE_BASIC_INFORMATION {
    LARGE_INTEGER CreationTime;
    LARGE_INTEGER LastAccessTime;
    LARGE_INTEGER LastWriteTime;
    LARGE_INTEGER ChangeTime;
    ULONG FileAttributes;
} FILE_BASIC_INFORMATION, *PFILE_BASIC_INFORMATION;

extern "C" __declspec(dllimport) NTSTATUS __stdcall
NtQueryInformationFile(
    IN HANDLE                       FileHandle,
    OUT PIO_STATUS_BLOCK            IoStatusBlock,
    OUT PVOID                       FileInformation,
    IN ULONG                        Length,
    IN FILE_INFORMATION_CLASS_       FileInformationClass
);

extern "C" __declspec(dllimport) NTSTATUS __stdcall
NtSetInformationFile(
    IN HANDLE                       FileHandle,
    OUT PIO_STATUS_BLOCK            IoStatusBlock,
    IN PVOID                        FileInformation,
    IN ULONG                        Length,
    IN FILE_INFORMATION_CLASS_       FileInformationClass
);

BOOL GetSparseRanges(HANDLE hFile);

struct SImageFileIO
{
	std::wstring FilePath;
    ULONG64 uSize = 0;
	HANDLE Handle = INVALID_HANDLE_VALUE;

    LARGE_INTEGER fileCreationTime = { 0, 0 };
    LARGE_INTEGER fileLastAccessTime = { 0, 0 };
    LARGE_INTEGER fileLastWriteTime = { 0, 0 };
    LARGE_INTEGER fileLastChangeTime = { 0, 0 };
    BOOL bTimeStampValid = FALSE;
};

CImageFileIO::CImageFileIO(std::wstring& FilePath, ULONG64 uSize)
{
	m = new SImageFileIO;
	m->FilePath = FilePath;
    m->uSize = uSize;
}

CImageFileIO::~CImageFileIO()
{
    if (m->Handle != INVALID_HANDLE_VALUE)
    {
        FILE_BASIC_INFORMATION FileBasicInfo;
        IO_STATUS_BLOCK IoStatusBlock;

        if (m->bTimeStampValid)
        {
            NTSTATUS status = NtQueryInformationFile (m->Handle, &IoStatusBlock, &FileBasicInfo, sizeof (FileBasicInfo), FileBasicInformation);
            if (NT_SUCCESS (status))
            {
                FileBasicInfo.CreationTime = m->fileCreationTime;
                FileBasicInfo.LastAccessTime = m->fileLastAccessTime;
                FileBasicInfo.LastWriteTime = m->fileLastWriteTime;
                FileBasicInfo.ChangeTime = m->fileLastChangeTime;

                NtSetInformationFile(m->Handle, &IoStatusBlock, &FileBasicInfo, sizeof (FileBasicInfo), FileBasicInformation);
			}
        }

        CloseHandle(m->Handle);
    }
	delete m;
}

ULONG64 CImageFileIO::GetDiskSize() const
{
    return m->uSize;
}

ULONG64 CImageFileIO::GetAllocSize() const
{
    LARGE_INTEGER liSparseFileCompressedSize;
    liSparseFileCompressedSize.LowPart = GetCompressedFileSize(m->FilePath.c_str(), (LPDWORD)&liSparseFileCompressedSize.HighPart);
    return liSparseFileCompressedSize.QuadPart;
}

bool CImageFileIO::CanBeFormated() const
{
    if (m->Handle == INVALID_HANDLE_VALUE) 
		return false;

	ULONGLONG uSize;
	GetFileSizeEx(m->Handle, (LARGE_INTEGER*)&uSize);
    return (uSize == 0);
}

int CImageFileIO::Init()
{
    //
    //  !!! WARNING !!!
    // 
    // Without disabling tile caching ImDisk will cause a deadlock in 
    // ntoskrnl.exe!CcCanIWrite 
    // Ntfs.sys!NtfsCopyWriteA 
    // 
    // This issue also affects DiscUtilsDevio.exe
    //

    bool bCreating = false;

	m->Handle = CreateFile(m->FilePath.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_NO_BUFFERING | FILE_FLAG_WRITE_THROUGH, NULL);
    if (m->Handle == INVALID_HANDLE_VALUE) {
        
        //
        // Create new image, but only if a size was specified
        //

        bCreating = true;

        if(m->uSize)
            m->Handle = CreateFile(m->FilePath.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_NEW, FILE_FLAG_NO_BUFFERING | FILE_FLAG_WRITE_THROUGH, NULL);

        if (m->Handle == INVALID_HANDLE_VALUE)
            return ERR_FILE_NOT_OPENED;
    }
    else {

        //
        // check if the image is not empty, and take its size
        //

        ULONGLONG uSize;
        GetFileSizeEx(m->Handle, (LARGE_INTEGER*)&uSize);
        if (uSize != 0)
            m->uSize = uSize;
    }

	//
	// make file sparse if it is not yet already
	//

    DWORD dwVolFlags;
    GetVolumeInformation(m->FilePath.substr(0, 3).c_str(), NULL, MAX_PATH, NULL, NULL, &dwVolFlags, NULL, MAX_PATH);
    if (!(dwVolFlags & FILE_SUPPORTS_SPARSE_FILES)) {
        DbgPrint(L"Volume %s does not support sparse files.\n", m->FilePath.substr(0, 3).c_str());
    }
    else {
        BY_HANDLE_FILE_INFORMATION bhfi;
        GetFileInformationByHandle(m->Handle, &bhfi);
        if ((bhfi.dwFileAttributes & FILE_ATTRIBUTE_SPARSE_FILE) == 0) {
            // Use the DeviceIoControl function with the FSCTL_SET_SPARSE control
            // code to mark the file as sparse. If you don't mark the file as sparse,
            // the FSCTL_SET_ZERO_DATA control code will actually write zero bytes to
            // the file instead of marking the region as sparse zero area.
            DWORD dwTemp;
            if (!DeviceIoControl(m->Handle, FSCTL_SET_SPARSE, NULL, 0, NULL, 0, &dwTemp, NULL)) {
                DbgPrint(L"Failed to make image file sparse: %s\n", m->FilePath.c_str());
            }
        }
        //else {
        //    GetSparseRanges(m->Handle);
        //}
    }

    if (!bCreating)
    {
        FILE_BASIC_INFORMATION FileBasicInfo;
        IO_STATUS_BLOCK IoStatusBlock;

        NTSTATUS status = NtQueryInformationFile(m->Handle, &IoStatusBlock, &FileBasicInfo, sizeof(FileBasicInfo), FileBasicInformation);

        if (NT_SUCCESS(status))
        {
            m->fileCreationTime = FileBasicInfo.CreationTime;
            m->fileLastAccessTime = FileBasicInfo.LastAccessTime;
            m->fileLastWriteTime = FileBasicInfo.LastWriteTime;
            m->fileLastChangeTime = FileBasicInfo.ChangeTime;
            m->bTimeStampValid = TRUE;

            // we tell the system not to update LastAccessTime, LastWriteTime, and ChangeTime
            FileBasicInfo.CreationTime.QuadPart = 0;
            FileBasicInfo.LastAccessTime.QuadPart = -1;
            FileBasicInfo.LastWriteTime.QuadPart = -1;
            FileBasicInfo.ChangeTime.QuadPart = -1;

            NtSetInformationFile(m->Handle, &IoStatusBlock, &FileBasicInfo, sizeof(FileBasicInfo), FileBasicInformation);
        }
    }

	return ERR_OK;
}

bool CImageFileIO::DiskWrite(void* buf, int size, __int64 offset)
{
	SetFilePointerEx(m->Handle, *(LARGE_INTEGER*)&offset, NULL, FILE_BEGIN);
    DWORD BytesWritten;
	return !!WriteFile(m->Handle, buf, size, &BytesWritten, NULL);
}

bool CImageFileIO::DiskRead(void* buf, int size, __int64 offset)
{
	SetFilePointerEx(m->Handle, *(LARGE_INTEGER*)&offset, NULL, FILE_BEGIN);
    DWORD BytesRead;
	return !!ReadFile(m->Handle, buf, size, &BytesRead, NULL);
}

void CImageFileIO::TrimProcess(DEVICE_DATA_SET_RANGE* range, int n)
{
	while (n) {

        // Specify the starting and the ending address (not the size) of the sparse zero block
        FILE_ZERO_DATA_INFORMATION fzdi;
        fzdi.FileOffset.QuadPart = range->StartingOffset;
        fzdi.BeyondFinalZero.QuadPart = range->StartingOffset + range->LengthInBytes;

        DWORD dwTemp;
        DeviceIoControl(m->Handle, FSCTL_SET_ZERO_DATA, &fzdi, sizeof(fzdi), NULL, 0, &dwTemp, NULL);

		range++;
		n--;
	}
}

BOOL GetSparseRanges(HANDLE hFile)
{
    LARGE_INTEGER liFileSize;
    GetFileSizeEx(hFile, &liFileSize);

    // Range to be examined (the whole file)
    FILE_ALLOCATED_RANGE_BUFFER queryRange;
    queryRange.FileOffset.QuadPart = 0;
    queryRange.Length = liFileSize;

    // Allocated areas info
    FILE_ALLOCATED_RANGE_BUFFER allocRanges[1024];

    DWORD nbytes;
    BOOL fFinished;
    DbgPrint(L"\nAllocated ranges in the file:");
    do
    {
        fFinished = DeviceIoControl(hFile,
                                    FSCTL_QUERY_ALLOCATED_RANGES,
                                    &queryRange,
                                    sizeof(queryRange),
                                    allocRanges,
                                    sizeof(allocRanges),
                                    &nbytes,
                                    NULL);

        if (!fFinished)
        {
            DWORD dwError = GetLastError();

            // ERROR_MORE_DATA is the only error that is normal
            if (dwError != ERROR_MORE_DATA)
            {
                DbgPrint(L"DeviceIoControl failed w/err 0x%08lx\n", dwError);
                CloseHandle(hFile);
                return FALSE;
            }
        }

        // Calculate the number of records returned
        DWORD dwAllocRangeCount = nbytes / sizeof(FILE_ALLOCATED_RANGE_BUFFER);

        // Print each allocated range
        for (DWORD i = 0; i < dwAllocRangeCount; i++)
        {
            DbgPrint(L"allocated range: [%I64u] [%I64u]\n",
                    allocRanges[i].FileOffset.QuadPart,
                    allocRanges[i].Length.QuadPart);
        }

        // Set starting address and size for the next query
        if (!fFinished && dwAllocRangeCount > 0)
        {
            queryRange.FileOffset.QuadPart = allocRanges[dwAllocRangeCount - 1].FileOffset.QuadPart +
                                             allocRanges[dwAllocRangeCount - 1].Length.QuadPart;

            queryRange.Length.QuadPart = liFileSize.QuadPart - queryRange.FileOffset.QuadPart;
        }

    } while (!fFinished);

    return TRUE;
}
