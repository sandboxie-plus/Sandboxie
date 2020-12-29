/*
 *
 * Copyright (c) 2020, David Xanatos
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

#include <ntstatus.h>
#define WIN32_NO_STATUS
typedef long NTSTATUS;

#include <windows.h>
#include "..\..\Sandboxie\common\win32_ntddk.h"
#include <psapi.h> // For access to GetModuleFileNameEx

//struct SBoxedProcess
//{
//};

CBoxedProcess::CBoxedProcess(quint32 ProcessId, class CSandBox* pBox)
{
	m_pBox = pBox;

	//m = new SBoxedProcess;

	m_ProcessId = ProcessId;

	m_ParendPID = 0;
	m_SessionId = 0;

	m_uTerminated = 0;
	//m_bSuspended = IsSuspended();
}

CBoxedProcess::~CBoxedProcess()
{
	//delete m;
}

bool CBoxedProcess::InitProcessInfo()
{
	HANDLE ProcessHandle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, (DWORD)m_ProcessId);
	if (ProcessHandle == INVALID_HANDLE_VALUE)
		return false;
	
	PROCESS_BASIC_INFORMATION BasicInformation;
	NTSTATUS status = NtQueryInformationProcess(ProcessHandle, ProcessBasicInformation, &BasicInformation, sizeof(PROCESS_BASIC_INFORMATION), NULL);
	if (NT_SUCCESS(status)) {
		m_ParendPID = (quint32)BasicInformation.InheritedFromUniqueProcessId;
	}

	TCHAR filename[MAX_PATH];
	if (DWORD size = GetModuleFileNameEx(ProcessHandle, NULL, filename, MAX_PATH))
		m_ImagePath = QString::fromWCharArray(filename);

	NtClose(ProcessHandle);
	return true;
}

extern "C"
{
	NTSYSCALLAPI NTSTATUS NTAPI NtTerminateProcess(_In_opt_ HANDLE ProcessHandle, _In_ NTSTATUS ExitStatus);
	NTSYSCALLAPI NTSTATUS NTAPI NtSuspendProcess(_In_ HANDLE ProcessHandle);
	NTSYSCALLAPI NTSTATUS NTAPI NtResumeProcess(_In_ HANDLE ProcessHandle);
}

#include <TlHelp32.h>

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
}

bool CBoxedProcess::IsTerminated(quint64 forMs) const 
{ 
	if (m_uTerminated == 0)
		return false;
	if (forMs == 0)
		return true;
	return ::GetTickCount64() - m_uTerminated > forMs;
}

/*SB_STATUS CBoxedProcess::SetSuspend(bool bSet)
{
	HANDLE ProcessHandle = OpenProcess(PROCESS_SUSPEND_RESUME, FALSE, (DWORD)m_ProcessId);
	if (ProcessHandle != INVALID_HANDLE_VALUE)
	{
		NTSTATUS status;
		if(bSet)
			status = NtSuspendProcess(ProcessHandle);
		else
			status = NtResumeProcess(ProcessHandle);
		NtClose(ProcessHandle);

		if (!NT_SUCCESS(status))
			return SB_ERR(status);
		m_bSuspended = IsSuspended();
		return SB_OK;
	}
	return SB_ERR();
}

bool CBoxedProcess::IsSuspended() const
{
	bool isSuspended = true;

	// todo: do that globaly once per sec for all boxed processes

	// Note: If the specified process is a 64-bit process and the caller is a 32-bit process, this function fails and the last error code is ERROR_PARTIAL_COPY (299).
	HANDLE hThreadSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
	if (hThreadSnap == INVALID_HANDLE_VALUE)
		return false;

	THREADENTRY32 te32 = { 0 };
	te32.dwSize = sizeof(THREADENTRY32);
	if (Thread32First(hThreadSnap, &te32))
	{
		do
		{
			if (te32.th32OwnerProcessID != m_ProcessId)
				continue;
			
			HANDLE hThread = OpenThread(THREAD_QUERY_INFORMATION, FALSE, te32.th32ThreadID);

			ULONG SuspendCount = 0;
			NTSTATUS status = NtQueryInformationThread(hThread, (THREADINFOCLASS)35/ThreadSuspendCount/, &SuspendCount, sizeof(ULONG), NULL);

			CloseHandle(hThread);

			if (SuspendCount == 0)
			{
				isSuspended = false;
				break;
			}
			
		} while (Thread32Next(hThreadSnap, &te32));
	}
	
	CloseHandle(hThreadSnap);

	return isSuspended;
}
*/

QString CBoxedProcess::GetBoxName() const 
{ 
	return m_pBox->GetName(); 
}