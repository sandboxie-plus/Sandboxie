/*
 *
 * Copyright (c) 2020-2022, David Xanatos
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "stdafx.h"
#include "BoxedProcess.h"
#include "SandBox.h"
#include "../SbieAPI.h"
#include "../Helpers/DbgHelper.h"

#include <ntstatus.h>
#define WIN32_NO_STATUS
typedef long NTSTATUS;

#include <windows.h>
#include "..\..\Sandboxie\common\win32_ntddk.h"
//#include <psapi.h> // For access to GetModuleFileNameEx

#include <winnt.h>

struct SBoxedProcess
{
	HANDLE Handle;
};

CBoxedProcess::CBoxedProcess(quint32 ProcessId, class CSandBox* pBox)
{
	m_pBox = pBox;
	if (pBox) m_BoxName = pBox->GetName();

	m = new SBoxedProcess;
	m->Handle = NULL;

	m_ProcessId = ProcessId;

	m_ParendPID = 0;
	m_SessionId = -1;

	m_ProcessFlags = 0;
	m_ImageType = -1;
	m_ReturnCode = STATUS_PENDING;

	m_uTerminated = 0;
	m_bSuspended = false;

	m_ProcessInfo.Flags = 0;
}

CBoxedProcess::~CBoxedProcess()
{
	if (m->Handle)
		NtClose(m->Handle);
	delete m;
}


//typedef enum _PEB_OFFSET
//{
//	PhpoCurrentDirectory,
//	PhpoDllPath,
//	PhpoImagePathName,
//	PhpoCommandLine,
//	PhpoWindowTitle,
//	PhpoDesktopInfo,
//	PhpoShellInfo,
//	PhpoRuntimeData,
//	PhpoTypeMask = 0xffff,
//	PhpoWow64 = 0x10000
//} PEB_OFFSET;
//
//
//typedef struct _STRING32
//{
//	USHORT Length;
//	USHORT MaximumLength;
//	ULONG Buffer;
//} UNICODE_STRING32, * PUNICODE_STRING32;
//
////typedef struct _STRING64 {
////  USHORT Length;
////  USHORT MaximumLength;
////  PVOID64 Buffer;
////} UNICODE_STRING64, * PUNICODE_STRING64;
//
//
////// PROCESS_BASIC_INFORMATION for pure 32 and 64-bit processes
////typedef struct _PROCESS_BASIC_INFORMATION {
////    PVOID Reserved1;
////    PVOID PebBaseAddress;
////    PVOID Reserved2[2];
////    ULONG_PTR UniqueProcessId;
////    PVOID Reserved3;
////} PROCESS_BASIC_INFORMATION;
//
//// PROCESS_BASIC_INFORMATION for 32-bit process on WOW64
//typedef struct _PROCESS_BASIC_INFORMATION_WOW64 {
//    PVOID Reserved1[2];
//    PVOID64 PebBaseAddress;
//    PVOID Reserved2[4];
//    ULONG_PTR UniqueProcessId[2];
//    PVOID Reserved3[2];
//} PROCESS_BASIC_INFORMATION_WOW64;
//
//
//typedef NTSTATUS (NTAPI *_NtQueryInformationProcess)(IN HANDLE ProcessHandle, ULONG ProcessInformationClass,
//    OUT PVOID ProcessInformation, IN ULONG ProcessInformationLength, OUT PULONG ReturnLength OPTIONAL );
//
////typedef NTSTATUS (NTAPI *_NtReadVirtualMemory)(IN HANDLE ProcessHandle, IN PVOID BaseAddress,
////    OUT PVOID Buffer, IN SIZE_T Size, OUT PSIZE_T NumberOfBytesRead);
//
//typedef NTSTATUS (NTAPI *_NtWow64ReadVirtualMemory64)(IN HANDLE ProcessHandle,IN PVOID64 BaseAddress,
//    OUT PVOID Buffer, IN ULONG64 Size, OUT PULONG64 NumberOfBytesRead);
//
//
//QString CBoxedProcess__GetPebString(HANDLE ProcessHandle, PEB_OFFSET Offset)
//{
//	BOOL is64BitOperatingSystem;
//	BOOL isWow64Process = FALSE;
//#ifdef _WIN64
//	is64BitOperatingSystem = TRUE;
//#else // ! _WIN64
//	isWow64Process = CSbieAPI::IsWow64();
//	is64BitOperatingSystem = isWow64Process;
//#endif _WIN64
//
//	BOOL isTargetWow64Process = FALSE;
//	IsWow64Process(ProcessHandle, &isTargetWow64Process);
//	BOOL isTarget64BitProcess = is64BitOperatingSystem && !isTargetWow64Process;
//
//	ULONG processParametersOffset = isTarget64BitProcess ? 0x20 : 0x10;
//
//	ULONG offset = 0;
//	switch (Offset)
//	{
//	case PhpoCurrentDirectory:	offset = isTarget64BitProcess ? 0x38 : 0x24; break;
//	case PhpoCommandLine:		offset = isTarget64BitProcess ? 0x70 : 0x40; break;
//	default:
//		return QString();
//	}
//
//	std::wstring s;
//	if (isTargetWow64Process) // OS : 64Bit, Cur : 32 or 64, Tar: 32bit
//	{
//		PVOID peb32;
//		if (!NT_SUCCESS(NtQueryInformationProcess(ProcessHandle, ProcessWow64Information, &peb32, sizeof(PVOID), NULL))) 
//			return QString();
//
//		ULONG procParams;
//		if (!NT_SUCCESS(NtReadVirtualMemory(ProcessHandle, (PVOID)((ULONG64)peb32 + processParametersOffset), &procParams, sizeof(ULONG), NULL)))
//			return QString();
//
//		UNICODE_STRING32 us;
//		if (!NT_SUCCESS(NtReadVirtualMemory(ProcessHandle, (PVOID)(procParams + offset), &us, sizeof(UNICODE_STRING32), NULL)))
//			return QString();
//
//		if ((us.Buffer == 0) || (us.Length == 0))
//			return QString();
//
//		s.resize(us.Length / 2);
//		if (!NT_SUCCESS(NtReadVirtualMemory(ProcessHandle, (PVOID)us.Buffer, (PVOID)s.c_str(), s.length() * 2, NULL)))
//			return QString();
//	}
//	else if (isWow64Process) //Os : 64Bit, Cur 32, Tar 64
//	{
//		static _NtQueryInformationProcess query = (_NtQueryInformationProcess)GetProcAddress(GetModuleHandleA("ntdll.dll"), "NtWow64QueryInformationProcess64");
//		static _NtWow64ReadVirtualMemory64 read = (_NtWow64ReadVirtualMemory64)GetProcAddress(GetModuleHandleA("ntdll.dll"), "NtWow64ReadVirtualMemory64");
//
//        PROCESS_BASIC_INFORMATION_WOW64 pbi;
//		if (!NT_SUCCESS(query(ProcessHandle, ProcessBasicInformation, &pbi, sizeof(PROCESS_BASIC_INFORMATION_WOW64), NULL))) 
//			return QString();
//        
//		ULONGLONG procParams;
//		if (!NT_SUCCESS(read(ProcessHandle, (PVOID64)((ULONGLONG)pbi.PebBaseAddress + processParametersOffset), &procParams, sizeof(ULONGLONG), NULL)))
//			return QString();
//
//		UNICODE_STRING64 us;
//		if (!NT_SUCCESS(read(ProcessHandle, (PVOID64)(procParams + offset), &us, sizeof(UNICODE_STRING64), NULL)))
//			return QString();
//
//		if ((us.Buffer == 0) || (us.Length == 0))
//			return QString();
//		
//		s.resize(us.Length / 2);
//		if (!NT_SUCCESS(read(ProcessHandle, (PVOID64)us.Buffer, (PVOID64)s.c_str(), s.length() * 2, NULL)))
//			return QString();
//	}
//	else // Os,Cur,Tar : 64 or 32
//	{
//		PROCESS_BASIC_INFORMATION pbi;
//		if (!NT_SUCCESS(NtQueryInformationProcess(ProcessHandle, ProcessBasicInformation, &pbi, sizeof(PROCESS_BASIC_INFORMATION), NULL))) 
//			return QString();
//
//		ULONG_PTR procParams;
//		if (!NT_SUCCESS(NtReadVirtualMemory(ProcessHandle, (PVOID)((ULONG64)pbi.PebBaseAddress + processParametersOffset), &procParams, sizeof(ULONG_PTR), NULL)))
//			return QString();
//
//		UNICODE_STRING us;
//		if (!NT_SUCCESS(NtReadVirtualMemory(ProcessHandle, (PVOID)(procParams + offset), &us, sizeof(UNICODE_STRING), NULL)))
//			return QString();
//
//		if ((us.Buffer == 0) || (us.Length == 0))
//			return QString();
//		
//		s.resize(us.Length / 2);
//		if (!NT_SUCCESS(NtReadVirtualMemory(ProcessHandle, (PVOID)us.Buffer, (PVOID)s.c_str(), s.length() * 2, NULL)))
//			return QString();
//	}
//
//	return QString::fromWCharArray(s.c_str());
//}

bool CBoxedProcess::InitProcessInfo()
{
	HANDLE ProcessHandle;
	//ProcessHandle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, (DWORD)m_ProcessId);
	//if (ProcessHandle == NULL) // try with less rights
	//	ProcessHandle = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, (DWORD)m_ProcessId);
	//if (ProcessHandle == NULL) // try with even less rights
		ProcessHandle = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, (DWORD)m_ProcessId);
	if (ProcessHandle != NULL)
	{
		m->Handle = ProcessHandle;

		//InitProcessInfoImpl(ProcessHandle);
	}

	SB_STATUS Status = m_pBox->Api()->GetProcessInfo(m_ProcessId, &m_ParendPID, &m_ProcessInfo.Flags, &m_bSuspended,
		&m_ImagePath, &m_CommandLine, &m_WorkingDir);

	if (m_ImagePath.left(8) == "\\Device\\" && m_ImagePath.left(m_pBox->m_FileRePath.length()).compare(m_pBox->m_FileRePath, Qt::CaseInsensitive) == 0)
		m_ImagePath = m_pBox->m_FilePath + m_ImagePath.mid(m_pBox->m_FileRePath.length());
	
	return !Status.IsError();
}

//SID SeLocalSystemSid = { SID_REVISION, 1, SECURITY_NT_AUTHORITY, { SECURITY_LOCAL_SYSTEM_RID } };
//
//void CBoxedProcess::InitProcessInfoImpl(void* ProcessHandle)
//{
//	PROCESS_BASIC_INFORMATION BasicInformation;
//	NTSTATUS status = NtQueryInformationProcess(ProcessHandle, ProcessBasicInformation, &BasicInformation, sizeof(PROCESS_BASIC_INFORMATION), NULL);
//	if (NT_SUCCESS(status)) {
//		m_ParendPID = (quint32)BasicInformation.InheritedFromUniqueProcessId;
//	}
//	
//	TCHAR filename[MAX_PATH];
//	DWORD dwSize = MAX_PATH;
//	if(QueryFullProcessImageNameW(ProcessHandle, 0, filename, &dwSize))
//		m_ImagePath = QString::fromWCharArray(filename);
//
//	BOOL isTargetWow64Process = FALSE;
//	IsWow64Process(ProcessHandle, &isTargetWow64Process);
//	m_ProcessInfo.IsWoW64 = isTargetWow64Process;
//
//	HANDLE TokenHandle = (HANDLE)m_pBox->Api()->QueryProcessInfo(m_ProcessId, 'ptok');
//	if (!TokenHandle)
//		NtOpenProcessToken(ProcessHandle, TOKEN_QUERY, &TokenHandle);
//	if (TokenHandle)
//	{
//		ULONG returnLength;
//
//		TOKEN_ELEVATION_TYPE elevationType;
//		if (NT_SUCCESS(NtQueryInformationToken(TokenHandle, (TOKEN_INFORMATION_CLASS)TokenElevationType, &elevationType, sizeof(TOKEN_ELEVATION_TYPE), &returnLength))) {
//			m_ProcessInfo.IsElevated = elevationType == TokenElevationTypeFull;
//		}
//
//		BYTE tokenUserBuff[0x80] = { 0 };
//		if (NT_SUCCESS(NtQueryInformationToken(TokenHandle, TokenUser, tokenUserBuff, sizeof(tokenUserBuff), &returnLength))){
//			m_ProcessInfo.IsSystem = EqualSid(((PTOKEN_USER)tokenUserBuff)->User.Sid, &SeLocalSystemSid);
//		}
//
//		ULONG restricted;
//		if (NT_SUCCESS(NtQueryInformationToken(TokenHandle, (TOKEN_INFORMATION_CLASS)TokenIsRestricted, &restricted, sizeof(ULONG), &returnLength))) {
//			m_ProcessInfo.IsRestricted = !!restricted;
//		}
//		
//        BYTE appContainerBuffer[0x80];
//        if (NT_SUCCESS(NtQueryInformationToken(TokenHandle, (TOKEN_INFORMATION_CLASS)TokenAppContainerSid, appContainerBuffer, sizeof(appContainerBuffer), &returnLength))) {
//            PTOKEN_APPCONTAINER_INFORMATION appContainerInfo = (PTOKEN_APPCONTAINER_INFORMATION)appContainerBuffer;
//			m_ProcessInfo.IsAppContainer = appContainerInfo->TokenAppContainer != NULL;
//        }
//
//		CloseHandle(TokenHandle);
//	}
//
//	if (m_CommandLine.isEmpty()) // windows 8.1 and later
//	{
//#define ProcessCommandLineInformation ((PROCESSINFOCLASS)60)
//		ULONG returnLength = 0;
//		NTSTATUS status = NtQueryInformationProcess(ProcessHandle, ProcessCommandLineInformation, NULL, 0, &returnLength);
//		if (!(status != STATUS_BUFFER_OVERFLOW && status != STATUS_BUFFER_TOO_SMALL && status != STATUS_INFO_LENGTH_MISMATCH))
//		{
//			PUNICODE_STRING commandLine = (PUNICODE_STRING)malloc(returnLength);
//			status = NtQueryInformationProcess(ProcessHandle, ProcessCommandLineInformation, commandLine, returnLength, &returnLength);
//			if (NT_SUCCESS(status) && commandLine->Buffer != NULL)
//				m_CommandLine = QString::fromWCharArray(commandLine->Buffer);
//			free(commandLine);
//		}
//#undef ProcessCommandLineInformation
//	}
//
//	if (m_CommandLine.isEmpty()) // fall back to the win 7 method - requirers PROCESS_VM_READ
//		m_CommandLine = CBoxedProcess__GetPebString(ProcessHandle, PhpoCommandLine);
//
//	m_WorkingDir = CBoxedProcess__GetPebString(ProcessHandle, PhpoCurrentDirectory);
//
//	TestSuspended();
//}

void CBoxedProcess::UpdateProcessInfo()
{
	if (m_ProcessFlags == 0 && m_pBox)
		m_ProcessFlags = m_pBox->Api()->QueryProcessInfo(m_ProcessId);
	m_ImageType = m_pBox->Api()->QueryProcessInfo(m_ProcessId, 'gpit');

	if (m_bSuspended)
		TestSuspended();
}

//extern "C"
//{
//NTSYSCALLAPI NTSTATUS NTAPI NtTerminateProcess(_In_opt_ HANDLE ProcessHandle, _In_ NTSTATUS ExitStatus);
//NTSYSCALLAPI NTSTATUS NTAPI NtSuspendProcess(_In_ HANDLE ProcessHandle);
//NTSYSCALLAPI NTSTATUS NTAPI NtResumeProcess(_In_ HANDLE ProcessHandle);
//
//NTSYSCALLAPI NTSTATUS NTAPI NtGetNextThread(HANDLE ProcessHandle, HANDLE ThreadHandle, ACCESS_MASK DesiredAccess, ULONG HandleAttributes, ULONG Flags, PHANDLE NewThreadHandle);
//}

SB_STATUS CBoxedProcess::Terminate()
{
	SB_STATUS Status = m_pBox->Api()->Terminate(m_ProcessId);
	if (!Status.IsError())
		SetTerminated();
	return Status;
}

void CBoxedProcess::SetTerminated()
{
	m_uTerminated = ::GetTickCount64();

	DWORD ExitCode = 0;
	if (m->Handle)
		GetExitCodeProcess(m->Handle, &ExitCode);
	m_ReturnCode = ExitCode;
}

bool CBoxedProcess::IsTerminated(quint64 forMs) const 
{ 
	if (m_uTerminated == 0)
		return false;
	if (forMs == 0)
		return true;
	return ::GetTickCount64() - m_uTerminated > forMs;
}

SB_STATUS CBoxedProcess::SetSuspended(bool bSuspended)
{
	SB_STATUS Status = m_pBox->Api()->SetSuspended(m_ProcessId, bSuspended);
	if (!Status.IsError())
		m_bSuspended = bSuspended;
	return Status;
}

bool CBoxedProcess::TestSuspended()
{
	m_pBox->Api()->GetProcessInfo(m_ProcessId, NULL, NULL, &m_bSuspended);
	return m_bSuspended;
}

//SB_STATUS CBoxedProcess::SetSuspended(bool bSet)
//{
//	HANDLE ProcessHandle = OpenProcess(PROCESS_SUSPEND_RESUME, FALSE, (DWORD)m_ProcessId);
//	if (ProcessHandle != INVALID_HANDLE_VALUE)
//	{
//		NTSTATUS status;
//		if(bSet)
//			status = NtSuspendProcess(ProcessHandle);
//		else
//			status = NtResumeProcess(ProcessHandle);
//		NtClose(ProcessHandle);
//
//		if (!NT_SUCCESS(status))
//			return SB_ERR(status);
//		TestSuspended();
//		return SB_OK;
//	}
//	return SB_ERR();
//}
//
//bool CBoxedProcess::TestSuspended()
//{
//	int iSuspended = 0;
//	int iRunning = 0;
//
//	for (HANDLE hThread = NULL;;)
//	{
//		HANDLE nNextThread = NULL;
//		NTSTATUS status = NtGetNextThread(m->Handle, hThread, THREAD_QUERY_INFORMATION | THREAD_SUSPEND_RESUME, 0, 0, &nNextThread);
//		if (hThread) NtClose(hThread);
//		if (!NT_SUCCESS(status))
//			break;
//		hThread = nNextThread;
//
//		ULONG IsTerminated = 0;
//		if (!NT_SUCCESS(NtQueryInformationThread(hThread, ThreadIsTerminated, &IsTerminated, sizeof(ULONG), NULL)) || IsTerminated)
//			continue;
//
//		ULONG SuspendCount = 0;
//		status = NtQueryInformationThread(hThread, (THREADINFOCLASS)35/*ThreadSuspendCount*/, &SuspendCount, sizeof(ULONG), NULL);
//		if (status == STATUS_INVALID_INFO_CLASS) { // windows 7
//			SuspendCount = SuspendThread(hThread);
//			ResumeThread(hThread);
//		}
//		if (SuspendCount > 0)
//			iSuspended++;
//		else
//			iRunning++;
//    }
//
//	m_bSuspended = iSuspended > 0 && iRunning == 0;
//	return m_bSuspended;
//}

void CBoxedProcess::ResolveSymbols(const QVector<quint64>& Addresses)
{
	foreach(quint64 Address, Addresses) 
	{
		if (!m_Symbols.contains(Address)) {
			SSymbol Symbol;
			//Symbol.Name = CSymbolProvider::Instance()->Resolve(m_ProcessId, Address);
			m_Symbols[Address] = Symbol;
			CSymbolProvider::ResolveAsync(m_ProcessId, Address, this, SLOT(OnSymbol(quint64, const QString&)));
		}
	}
}
