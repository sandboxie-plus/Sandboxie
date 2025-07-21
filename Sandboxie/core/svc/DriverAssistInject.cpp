/*
 * Copyright 2004-2020 Sandboxie Holdings, LLC 
 * Copyright 2020-2023 David Xanatos, xanasoft.com
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
// Driver Assistant, inject SbieLow/LowLevel DLL into new process
//---------------------------------------------------------------------------

#include "GuiServer.h"
#include "core/low/lowdata.h"
#include "common/win32_ntddk.h"
#include "misc.h"
#include "ImageHlp.h"



//---------------------------------------------------------------------------
// InjectLow_Init
//---------------------------------------------------------------------------


bool DriverAssist::InjectLow_Init()
{
	ULONG errlvl = SbieDll_InjectLow_InitHelper();
    if (errlvl != 0) {
        LogEvent(MSG_9234, 0x9241, errlvl);
        return false;
    }

    return true;
}


//---------------------------------------------------------------------------
// InjectLow
//---------------------------------------------------------------------------


void DriverAssist::InjectLow(void *_msg)
{
	SVC_PROCESS_MSG *msg = (SVC_PROCESS_MSG *)_msg;

    NTSTATUS status = 0;
	ULONG errlvl = 0;
    UCHAR SandboxieLogonSid[SECURITY_MAX_SID_SIZE] = { 0 };
    WCHAR* file_root_path = NULL;
    WCHAR* reg_root_path = NULL;

    if (!m_DriverReady) {

		errlvl = 0xFF;
		goto finish;
    }

	//
	// open new process and verify process creation time
	//

	HANDLE hProcess = InjectLow_OpenProcess(_msg);
	if (!hProcess) {

		errlvl = 0x11;
		goto finish;
	}

	WCHAR boxname[BOXNAME_COUNT];
    if (!NT_SUCCESS(SbieApi_QueryProcessEx2((HANDLE)msg->process_id, 0, boxname, NULL, NULL, NULL, NULL))) {
        errlvl = 0x11;
        goto finish;
    }

    ULONG file_len = 0;
    ULONG reg_len = 0;
    if (!NT_SUCCESS(SbieApi_QueryProcessPath((HANDLE)msg->process_id, NULL, NULL, NULL, &file_len, &reg_len, NULL))) {
        errlvl = 0x12;
        goto finish;
    }
    file_root_path = (WCHAR*)HeapAlloc(GetProcessHeap(), 0, file_len + 16);
    reg_root_path = (WCHAR*)HeapAlloc(GetProcessHeap(), 0, reg_len + 16);
    if (!file_root_path || !reg_root_path) {
        errlvl = 0x13;
        goto finish;
    }
    if (!NT_SUCCESS(SbieApi_QueryProcessPath((HANDLE)msg->process_id, file_root_path, reg_root_path, NULL, &file_len, &reg_len, NULL))) {
        errlvl = 0x14;
        goto finish;
    }

    ULONG64 ProcessFlags = SbieApi_QueryProcessInfo((HANDLE)msg->process_id, 0);
    BOOLEAN CompartmentMode = (ProcessFlags & SBIE_FLAG_APP_COMPARTMENT) != 0;

    //
	// notify the box manager about a new process
	//

    //BOOLEAN IsFirst = FALSE;
    //if (!msg->bHostInject) {
    //    IsFirst = BoxManager::GetInstance()->ProcessCreated(msg->process_id, boxname, reg_root_path, msg->session_id);
    //}

	//
	// inject the lowlevel.dll into the target process
	//

    SBIELOW_FLAGS sbieLow;
    sbieLow.init_flags = 0;
#ifndef _M_ARM64
    sbieLow.is_wow64 = msg->is_wow64;
#endif
    sbieLow.bHostInject = msg->bHostInject;
    // NoSysCallHooks BEGIN
    sbieLow.bNoSysHooks = CompartmentMode || SbieApi_QueryConfBool(boxname, L"NoSysCallHooks", FALSE);
    // NoSysCallHooks END
    // NoSbieCons BEGIN
    sbieLow.bNoConsole = CompartmentMode || SbieApi_QueryConfBool(boxname, L"NoSandboxieConsole", FALSE);
    // NoSbieCons END
    //sbieLow.bIsFirst = IsFirst;

	errlvl = SbieDll_InjectLow(hProcess, sbieLow.init_flags, TRUE);
	if(errlvl != 0)
		goto finish;

    //
    // put process into a job for win32 restrictions
    //

    // NoSbieDesk BEGIN
    if (!(CompartmentMode || SbieApi_QueryConfBool(boxname, L"NoSandboxieDesktop", FALSE)))
    // NoSbieDesk END
    if (!msg->bHostInject)
    {
        if(! GuiServer::GetInstance()->InitProcess(
                hProcess, msg->process_id, msg->session_id,
                msg->add_to_job)) {

            errlvl = 0x88;
            goto finish;
        }
    }

    //
    // mount box container if needed
    //

    //if (IsFirst) {
        if (!MountManager::GetInstance()->AcquireBoxRoot(boxname, reg_root_path, file_root_path, msg->session_id)) {
            errlvl = 0xAA;
            goto finish;
        }
    //}

    //
    // notify driver that we successfully injected the lowlevel code
    //

    if (GetSandboxieSID(boxname, SandboxieLogonSid, sizeof(SandboxieLogonSid)))
        status = SbieApi_Call(API_INJECT_COMPLETE, 2, (ULONG_PTR)msg->process_id, SandboxieLogonSid);
    else // if that fails or is not enabled we fall back to using the anonymous logon token
        status = SbieApi_Call(API_INJECT_COMPLETE, 1, (ULONG_PTR)msg->process_id);

    if (status == 0)
        errlvl = 0;
    else
        errlvl = 0x99;

    //
    // finish
    //

finish:
    if (file_root_path) 
        HeapFree(GetProcessHeap(), 0, file_root_path);
    if (reg_root_path) 
        HeapFree(GetProcessHeap(), 0, reg_root_path);

#ifdef _M_ARM64
    if (errlvl == -1)
        SbieApi_LogEx(msg->session_id, 2338, L"%S (ARM32)", msg->process_name);
    else if (errlvl == -2)
        SbieApi_LogEx(msg->session_id, 2338, L"%S (CHPE)", msg->process_name);
    else 
#endif
    if (errlvl) {

        ULONG err = GetLastError();
        SbieApi_LogEx(msg->session_id, 2335,
                        L"%S [%02X / %d]", msg->process_name, errlvl, err);
    }

    if (hProcess) {

        if (errlvl) {
            SbieApi_Call(API_INJECT_COMPLETE, 3, (ULONG_PTR)msg->process_id, NULL, errlvl);
            //TerminateProcess(hProcess, 1);
        }

        CloseHandle(hProcess);
    }
}


//---------------------------------------------------------------------------
// InjectLow_OpenProcess
//---------------------------------------------------------------------------


HANDLE DriverAssist::InjectLow_OpenProcess(void *_msg)
{
    SVC_PROCESS_MSG *msg = (SVC_PROCESS_MSG *)_msg;

    ///
    // open target process with the access rights we need
    //

    const ULONG _DesiredAccess =
              PROCESS_DUP_HANDLE | PROCESS_TERMINATE | PROCESS_SUSPEND_RESUME
            | PROCESS_SET_INFORMATION | PROCESS_QUERY_INFORMATION
            | PROCESS_VM_OPERATION | PROCESS_VM_READ | PROCESS_VM_WRITE;

    HANDLE hProcess = OpenProcess(_DesiredAccess, FALSE, msg->process_id);

    if (hProcess) {

        FILETIME time, time1, time2, time3;
        BOOL ok = GetProcessTimes(hProcess, &time, &time1, &time2, &time3);
        if (ok && *(ULONG64 *)&time.dwLowDateTime == msg->create_time) {

            return hProcess;
        }

        CloseHandle(hProcess);
    }

    return NULL;
}

