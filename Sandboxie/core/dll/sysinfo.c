/*
 * Copyright 2004-2020 Sandboxie Holdings, LLC 
 * Copyright 2020-2021 David Xanatos, xanasoft.com
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
// System Information and Jobs
//---------------------------------------------------------------------------


#include "dll.h"
#include "common/my_version.h"
#include <stdio.h>


//---------------------------------------------------------------------------
// Defines
//---------------------------------------------------------------------------


#define OBJECT_ATTRIBUTES_ATTRIBUTES                            \
    (ObjectAttributes                                           \
        ? ObjectAttributes->Attributes | OBJ_CASE_INSENSITIVE   \
        : 0)


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------


static NTSTATUS SysInfo_NtQuerySystemInformation(
    SYSTEM_INFORMATION_CLASS SystemInformationClass,
    void *Buffer,
    ULONG BufferLength,
    ULONG *ReturnLength);

static void SysInfo_DiscardProcesses(SYSTEM_PROCESS_INFORMATION *buf);

static BOOL SysInfo_SetLocaleInfoW(
    LCID Locale, LCTYPE LCType, void *lpLCData);

static BOOL SysInfo_SetLocaleInfoA(
    LCID Locale, LCTYPE LCType, void *lpLCData);

static NTSTATUS SysInfo_NtTraceEvent(
    HANDLE TraceHandle, ULONG Flags, ULONG FieldSize, PVOID Fields);
 
static NTSTATUS SysInfo_NtDeleteWnfStateData(
    ULONG_PTR UnknownParameter1, ULONG_PTR UnknownParameter2);

static NTSTATUS SysInfo_NtCreateJobObject(
    HANDLE *JobHandle, ACCESS_MASK DesiredAccess,
    OBJECT_ATTRIBUTES *ObjectAttributes);

static NTSTATUS SysInfo_NtOpenJobObject(
    HANDLE *JobHandle, ACCESS_MASK DesiredAccess,
    OBJECT_ATTRIBUTES *ObjectAttributes);

static NTSTATUS SysInfo_NtAssignProcessToJobObject(
    HANDLE JobHandle, HANDLE ProcessHandle);

static NTSTATUS SysInfo_NtSetInformationJobObject(
    HANDLE JobHandle, JOBOBJECTINFOCLASS JobObjectInformationClass,
    void *JobObjectInformtion, ULONG JobObjectInformtionLength);

static void SysInfo_JobCallbackData_Set(
    HANDLE ProcessHandle, void *JobPortInformation);

static ULONG SysInfo_JobCallbackData_Thread(void *xHandles);


//---------------------------------------------------------------------------


typedef BOOL (*P_SetLocaleInfo)(LCID Locale, LCTYPE LCType, void *lpLCData);


//---------------------------------------------------------------------------


static P_NtQuerySystemInformation   __sys_NtQuerySystemInformation  = NULL;

static P_SetLocaleInfo              __sys_SetLocaleInfoW            = NULL;
static P_SetLocaleInfo              __sys_SetLocaleInfoA            = NULL;

static P_NtCreateJobObject          __sys_NtCreateJobObject         = NULL;
static P_NtOpenJobObject            __sys_NtOpenJobObject           = NULL;
static P_NtAssignProcessToJobObject __sys_NtAssignProcessToJobObject = NULL;
static P_NtSetInformationJobObject  __sys_NtSetInformationJobObject = NULL;

static P_NtTraceEvent  __sys_NtTraceEvent = NULL;


//---------------------------------------------------------------------------
// Variables
//---------------------------------------------------------------------------


static ULONG_PTR *SysInfo_JobCallbackData = NULL;

BOOLEAN SysInfo_UseSbieJob = FALSE;
BOOLEAN SysInfo_CanUseJobs = FALSE;

//---------------------------------------------------------------------------
// SysInfo_Init
//---------------------------------------------------------------------------


_FX BOOLEAN SysInfo_Init(void)
{
    HMODULE module = NULL;

    void *NtTraceEvent;

    if (! Dll_SkipHook(L"ntqsi")) {

        SBIEDLL_HOOK(SysInfo_,NtQuerySystemInformation);
    }

    extern BOOLEAN Gui_OpenAllWinClasses;
    SysInfo_UseSbieJob = !Gui_OpenAllWinClasses && !SbieApi_QueryConfBool(NULL, L"NoAddProcessToJob", FALSE);

    if (Dll_OsBuild >= 8400)
        SysInfo_CanUseJobs = SbieApi_QueryConfBool(NULL, L"AllowBoxedJobs", FALSE);

    SBIEDLL_HOOK(SysInfo_, NtCreateJobObject);
    SBIEDLL_HOOK(SysInfo_, NtOpenJobObject);
    if (SysInfo_UseSbieJob) {
        if (!SysInfo_CanUseJobs) {
            SBIEDLL_HOOK(SysInfo_, NtAssignProcessToJobObject);
        }
        SBIEDLL_HOOK(SysInfo_, NtSetInformationJobObject);
    }


    SBIEDLL_HOOK(SysInfo_,SetLocaleInfoW);
    SBIEDLL_HOOK(SysInfo_,SetLocaleInfoA);

    //
    // we don't want to hook NtTraceEvent in kernel mode
    // so we hook it in user mode
    //

    NtTraceEvent = GetProcAddress(Dll_Ntdll, "NtTraceEvent");
    if (NtTraceEvent) {

        SBIEDLL_HOOK(SysInfo_, NtTraceEvent);
    }

    if (Dll_OsBuild >= 8400) {

        //
        // on Windows 8, a new API returns STATUS_ACCESS_DENIED
        //

        void *NtDeleteWnfStateData =
                        GetProcAddress(Dll_Ntdll, "NtDeleteWnfStateData");
        if (NtDeleteWnfStateData) {

            P_NtDeleteWnfStateData __sys_NtDeleteWnfStateData = NULL;
            SBIEDLL_HOOK(SysInfo_,NtDeleteWnfStateData);
        }
    }

    return TRUE;
}


//---------------------------------------------------------------------------
// SysInfo_NtQuerySystemInformation
//---------------------------------------------------------------------------


_FX NTSTATUS SysInfo_NtQuerySystemInformation(
    SYSTEM_INFORMATION_CLASS SystemInformationClass,
    void *Buffer,
    ULONG BufferLength,
    ULONG *ReturnLength)
{
    NTSTATUS status;

    status = __sys_NtQuerySystemInformation(
        SystemInformationClass, Buffer, BufferLength, ReturnLength);

    if (NT_SUCCESS(status) &&
            (SystemInformationClass == SystemProcessInformation
			|| SystemInformationClass == SystemExtendedProcessInformation
			|| SystemInformationClass == SystemFullProcessInformation)) {

        SysInfo_DiscardProcesses(Buffer);
    }

    return status;
}


//---------------------------------------------------------------------------
// SysInfo_DiscardProcesses
//---------------------------------------------------------------------------


_FX void SysInfo_DiscardProcesses(SYSTEM_PROCESS_INFORMATION *buf)
{
    SYSTEM_PROCESS_INFORMATION *curr = buf;
    SYSTEM_PROCESS_INFORMATION *next;
    WCHAR boxname[48];

	BOOL hideOther = SbieApi_QueryConfBool(NULL, L"HideOtherBoxes", TRUE);

	WCHAR* hiddenProcesses = NULL;
	WCHAR* hiddenProcessesPtr = NULL;
	ULONG hiddenProcessesLen = 100 * 110; // we can hide up to 100 processes, sould be enough
	WCHAR hiddenProcess[110];

	for (ULONG index = 0; ; ++index) {
		NTSTATUS status = SbieApi_QueryConfAsIs(NULL, L"HideHostProcess", index, hiddenProcess, 108 * sizeof(WCHAR));
		if (NT_SUCCESS(status)) {
			if (hiddenProcesses == NULL) {
				hiddenProcesses = (WCHAR*)HeapAlloc(GetProcessHeap(), 0, hiddenProcessesLen * sizeof(WCHAR));
				if (!hiddenProcesses)
					break;
				hiddenProcessesPtr = hiddenProcesses;
			}
			ULONG nameLen = wcslen(hiddenProcess) + 1; // include terminating 0
			if ((hiddenProcessesPtr - hiddenProcesses) + nameLen + 1 > hiddenProcessesLen) {
				SbieApi_Log(2310, L", 'HideProcess'"); // todo add custom message
				break;
			}
			wmemcpy(hiddenProcessesPtr, hiddenProcess, nameLen);
			hiddenProcessesPtr += nameLen;
			*hiddenProcessesPtr = L'\0';
		}
		else if (status != STATUS_BUFFER_TOO_SMALL)
			break;
	}

    //
    // we assume the first record is always going to be the idle process or
    // a system process -- in any case, one we're not going to have to skip
    //

    while (1) {

        next = (SYSTEM_PROCESS_INFORMATION *) (((UCHAR *)curr) + curr->NextEntryOffset);
        if (next == curr)
            break;

		SbieApi_QueryProcess(next->UniqueProcessId, boxname, NULL, NULL, NULL);

		BOOL hideProcess = FALSE;
		if (hideOther && *boxname && _wcsicmp(boxname, Dll_BoxName) != 0) {
			hideProcess = TRUE;
		}
		else if(hiddenProcesses && next->ImageName.Buffer) {
            WCHAR* imagename = wcschr(next->ImageName.Buffer, L'\\');
			if (imagename)  imagename += 1; // skip L'\\'
			else			imagename = next->ImageName.Buffer;
			if (!*boxname || _wcsnicmp(imagename, L"Sandboxie", 9) == 0) {
				for (hiddenProcessesPtr = hiddenProcesses; *hiddenProcessesPtr != L'\0'; hiddenProcessesPtr += wcslen(hiddenProcessesPtr) + 1) {
					if (_wcsicmp(imagename, hiddenProcessesPtr) == 0) {
						hideProcess = TRUE;
						break;
					}
				}
			}
		}

        if (!hideProcess)
            curr = next;
        else if (next->NextEntryOffset)
            curr->NextEntryOffset += next->NextEntryOffset;
        else
            curr->NextEntryOffset = 0;
    }

	if(hiddenProcesses)
		HeapFree(GetProcessHeap(), HEAP_GENERATE_EXCEPTIONS, hiddenProcesses);
}


//---------------------------------------------------------------------------
// SysInfo_SetLocaleInfoW
//---------------------------------------------------------------------------


_FX BOOL SysInfo_SetLocaleInfoW(LCID Locale, LCTYPE LCType, void *lpLCData)
{
    SetLastError(ERROR_ACCESS_DENIED);
    return FALSE;
}


//---------------------------------------------------------------------------
// SysInfo_SetLocaleInfoA
//---------------------------------------------------------------------------


_FX BOOL SysInfo_SetLocaleInfoA(LCID Locale, LCTYPE LCType, void *lpLCData)
{
    SetLastError(ERROR_ACCESS_DENIED);
    return FALSE;
}


//---------------------------------------------------------------------------
// SysInfo_NtTraceEvent
//---------------------------------------------------------------------------


_FX NTSTATUS SysInfo_NtTraceEvent(
    HANDLE TraceHandle, ULONG Flags, ULONG FieldSize, PVOID Fields)
{
    // this prevents the CAPI2 application event 'The Cryptographic Services service failed to initialize the VSS backup "System Writer" object.'
    if (Dll_ImageType == DLL_IMAGE_SANDBOXIE_CRYPTO) {
        return STATUS_ACCESS_DENIED;
    }
    return __sys_NtTraceEvent(TraceHandle, Flags, FieldSize, Fields);
}


//---------------------------------------------------------------------------
// SysInfo_NtDeleteWnfStateData
//---------------------------------------------------------------------------


static NTSTATUS SysInfo_NtDeleteWnfStateData(
    ULONG_PTR UnknownParameter1, ULONG_PTR UnknownParameter2)
{
    if (Dll_ImageType != DLL_IMAGE_SANDBOXIE_RPCSS)
        SbieApi_Log(2205, L"NtDeleteWnfStateData (%S)", Dll_ImageName);
    return STATUS_SUCCESS;
}


//---------------------------------------------------------------------------
// SysInfo_QueryProcesses
//---------------------------------------------------------------------------


_FX void *SysInfo_QueryProcesses(ULONG *out_len)
{
    SYSTEM_PROCESS_INFORMATION *info;
    ULONG len, i;
    NTSTATUS status;

    info = NULL;
    len  = 8192;

    for (i = 0; i < 5; ++i) {

        info = Dll_AllocTemp(len);

        status = NtQuerySystemInformation(
                            SystemProcessInformation, info, len, &len);
        if (NT_SUCCESS(status))
            break;

        Dll_Free(info);
        info = NULL;

        if (status == STATUS_BUFFER_OVERFLOW ||
            status == STATUS_INFO_LENGTH_MISMATCH ||
            status == STATUS_BUFFER_TOO_SMALL) {

            len += 64;
            continue;
        }

        break;
    }

    if (out_len)
        *out_len = len;
    return info;
}


//---------------------------------------------------------------------------
// SysInfo_GetJobName
//---------------------------------------------------------------------------


_FX NTSTATUS SysInfo_GetJobName(OBJECT_ATTRIBUTES* ObjectAttributes, WCHAR** OutCopyPath)
{
    THREAD_DATA *TlsData = Dll_GetTlsData(NULL);

    WCHAR *name;
    ULONG objname_len;
    WCHAR *objname_buf;
    static volatile ULONG JobCounter = 0;
    WCHAR dummy_name[MAX_PATH];

    *OutCopyPath = NULL;

    if (ObjectAttributes && ObjectAttributes->ObjectName) {
        objname_len = ObjectAttributes->ObjectName->Length & ~1;
        objname_buf = ObjectAttributes->ObjectName->Buffer;
    } else {

        ULONG jobCounter = InterlockedIncrement(&JobCounter);
        Sbie_snwprintf(dummy_name, MAX_PATH, L"%s_DummyJob_%s_p%d_t%d_c%d",
                        SBIE, Dll_ImageName, GetCurrentProcessId(), GetCurrentThreadId(), jobCounter);
        
        objname_len = wcslen(dummy_name);
        objname_buf = dummy_name;
    }


    name = Dll_GetTlsNameBuffer(TlsData, COPY_NAME_BUFFER, Dll_BoxIpcPathLen + objname_len);

    *OutCopyPath = name;

    //if (Dll_AlernateIpcNaming)
    //{
    //    wmemcpy(name, objname_buf, objname_len);
    //    name += objname_len;
    //
    //    wmemcpy(name, Dll_BoxIpcPath, Dll_BoxIpcPathLen);
    //    name += Dll_BoxIpcPathLen;
    //}
    //else
    {
        wmemcpy(name, Dll_BoxIpcPath, Dll_BoxIpcPathLen);
        name += Dll_BoxIpcPathLen;

        *name = L'\\';
        name++;

        wmemcpy(name, objname_buf, objname_len);
        name += objname_len;
    }

    *name = L'\0';

    return STATUS_SUCCESS;
}


//---------------------------------------------------------------------------
// SysInfo_NtCreateJobObject
//---------------------------------------------------------------------------


_FX NTSTATUS SysInfo_NtCreateJobObject(
    HANDLE *JobHandle, ACCESS_MASK DesiredAccess,
    OBJECT_ATTRIBUTES *ObjectAttributes)
{
    NTSTATUS status;
    OBJECT_ATTRIBUTES objattrs;
    UNICODE_STRING objname;
    WCHAR* CopyPath;

    //
    // the driver requires us to specify a sandboxed path name to a
    // job object, and to not request some specific rights
    //

    if(!SysInfo_CanUseJobs && SysInfo_UseSbieJob)
        DesiredAccess &= ~(JOB_OBJECT_ASSIGN_PROCESS | JOB_OBJECT_TERMINATE);

    if (NT_SUCCESS(SysInfo_GetJobName(ObjectAttributes, &CopyPath))) {
        RtlInitUnicodeString(&objname, CopyPath);
        InitializeObjectAttributes(&objattrs, &objname, OBJECT_ATTRIBUTES_ATTRIBUTES, NULL, Secure_EveryoneSD);
        ObjectAttributes = &objattrs;
    }

    status = __sys_NtCreateJobObject(JobHandle, DesiredAccess, ObjectAttributes);

    return status;
}


//---------------------------------------------------------------------------
// SysInfo_NtCreateJobObject
//---------------------------------------------------------------------------


_FX NTSTATUS SysInfo_NtOpenJobObject(
    HANDLE* JobHandle, ACCESS_MASK DesiredAccess,
    OBJECT_ATTRIBUTES* ObjectAttributes)
{
    NTSTATUS status;
    OBJECT_ATTRIBUTES objattrs;
    UNICODE_STRING objname;
    WCHAR* CopyPath;

    if(!SysInfo_CanUseJobs && SysInfo_UseSbieJob)
        DesiredAccess &= ~(JOB_OBJECT_ASSIGN_PROCESS | JOB_OBJECT_TERMINATE);

    if (NT_SUCCESS(SysInfo_GetJobName(ObjectAttributes, &CopyPath))) {
        RtlInitUnicodeString(&objname, CopyPath);
        InitializeObjectAttributes(&objattrs, &objname, OBJECT_ATTRIBUTES_ATTRIBUTES, NULL, Secure_EveryoneSD);
        ObjectAttributes = &objattrs;
    }

    status = __sys_NtOpenJobObject(JobHandle, DesiredAccess, ObjectAttributes);

    return status;
}


//---------------------------------------------------------------------------
// SysInfo_NtAssignProcessToJobObject
//---------------------------------------------------------------------------


_FX NTSTATUS SysInfo_NtAssignProcessToJobObject(
    HANDLE JobHandle, HANDLE ProcessHandle)
{
    HANDLE DuplicatedProcessHandle;

    if (0 == NtDuplicateObject(
                NtCurrentProcess(), ProcessHandle,
                NtCurrentProcess(), &DuplicatedProcessHandle,
                PROCESS_QUERY_INFORMATION | SYNCHRONIZE, 0, 0)) {

        SysInfo_JobCallbackData_Set(DuplicatedProcessHandle, NULL);
    }

    return STATUS_SUCCESS;
}


//---------------------------------------------------------------------------
// SysInfo_NtSetInformationJobObject
//---------------------------------------------------------------------------


_FX NTSTATUS SysInfo_NtSetInformationJobObject(
    HANDLE JobHandle, JOBOBJECTINFOCLASS JobObjectInformationClass,
    void *JobObjectInformtion, ULONG JobObjectInformtionLength)
{
    //
    // Since windows 8 we can have nested jobs i.e. we can have all sandboxed processes 
    // be part of the box isoaltion job and also of for example a chrome sandbox job.
    // Howeever we booth jobs can not specify UIRestrictions since our own job allready
    // specified those restrictions, we dont allow a boxed process to spesify its own.
    //

    if (SysInfo_CanUseJobs && JobObjectInformationClass == JobObjectBasicUIRestrictions)
        return STATUS_SUCCESS;

    NTSTATUS status = __sys_NtSetInformationJobObject(
        JobHandle, JobObjectInformationClass,
        JobObjectInformtion, JobObjectInformtionLength);

    if(SysInfo_CanUseJobs)
        return status;

    if (NT_SUCCESS(status) &&
            JobObjectInformationClass ==
                    JobObjectAssociateCompletionPortInformation) {
    
        SysInfo_JobCallbackData_Set(NULL, JobObjectInformtion);
    }

    return status;
}


//---------------------------------------------------------------------------
// SysInfo_JobCallbackData_Set
//---------------------------------------------------------------------------


_FX void SysInfo_JobCallbackData_Set(
    HANDLE ProcessHandle, void *JobPortInformation)
{
    ULONG_PTR *Handles = SysInfo_JobCallbackData;
    if (! Handles) {
        Handles = Dll_Alloc(sizeof(ULONG_PTR) * 4);
        Handles[3] = 0;
    }

    if (JobPortInformation) {
        // copy CompletionKey and CompletionPort from
        // JOBOBJECT_ASSOCIATE_COMPLETION_PORT structure
        memcpy(Handles, JobPortInformation, sizeof(ULONG_PTR) * 2);
        Handles[3] |= 1;
    }

    if (ProcessHandle) {
        Handles[2] = (ULONG_PTR)ProcessHandle;
        Handles[3] |= 2;
    }

    if ((Handles[3] & 3) == 3) {

        HANDLE ThreadHandle = CreateThread(
            NULL, 0, SysInfo_JobCallbackData_Thread, Handles, 0, NULL);

        if (ThreadHandle)
            CloseHandle(ThreadHandle);

        Handles = NULL;
    }

    SysInfo_JobCallbackData = Handles;
}


//---------------------------------------------------------------------------
// SysInfo_JobCallbackData_Thread
//---------------------------------------------------------------------------


_FX ULONG SysInfo_JobCallbackData_Thread(void *xHandles)
{
    ULONG_PTR *Handles      = (ULONG_PTR *)xHandles;
    ULONG_PTR CompletionKey = Handles[0];
    HANDLE CompletionPort   = (HANDLE)Handles[1];
    HANDLE ProcessHandle    = (HANDLE)Handles[2];

    PROCESS_BASIC_INFORMATION info;
    ULONG len;

    Dll_Free(Handles);

    //
    // when a completion port is set, the job completion port receives
    // JOB_OBJECT_MSG_NEW_PROCESS notifications for processes in the job
    //

    if (0 != NtQueryInformationProcess(
                ProcessHandle, ProcessBasicInformation,
                &info, sizeof(PROCESS_BASIC_INFORMATION), &len))
        info.UniqueProcessId = 0;

    PostQueuedCompletionStatus(
        CompletionPort, JOB_OBJECT_MSG_NEW_PROCESS, CompletionKey,
        (LPOVERLAPPED)info.UniqueProcessId);

    //
    // send JOB_OBJECT_MSG_EXIT_PROCESS when the process ends
    //

    if (WaitForSingleObject(ProcessHandle, INFINITE) == 0) {

        CloseHandle(ProcessHandle);
        PostQueuedCompletionStatus(
            CompletionPort, JOB_OBJECT_MSG_EXIT_PROCESS, CompletionKey,
            (LPOVERLAPPED)info.UniqueProcessId);
    }

    return 0;
}
