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

//---------------------------------------------------------------------------
// Driver Server -- UAC Elevation
//---------------------------------------------------------------------------

#include "stdafx.h"

#include <shellapi.h>
#include <stdlib.h>
#include <sddl.h>
#include "serviceserver.h"
#include "servicewire.h"
#include "common/defines.h"
#include "common/my_version.h"
#include "core/dll/sbiedll.h"
#include <aclapi.h>

#define MISC_H_WITHOUT_WIN32_NTDDK_H
#include "misc.h"


//---------------------------------------------------------------------------
// CanCallerDoElevation
//---------------------------------------------------------------------------


bool ServiceServer::CanCallerDoElevation(
        HANDLE idProcess, const WCHAR *ServiceName, ULONG *pSessionId)
{
    WCHAR boxname[48];
    WCHAR exename[99];

    if (0 != SbieApi_QueryProcess(
                        idProcess, boxname, exename, NULL, pSessionId))
        return false;

    bool DropRights = CheckDropRights(boxname);

    if (DropRights && ServiceName) {

        WCHAR buf[66];
        ULONG index = 0;
        while (1) {
            NTSTATUS status = SbieApi_QueryConfAsIs(
                boxname, L"StartService", index,
                buf, 64 * sizeof(WCHAR));
            ++index;
            if (NT_SUCCESS(status)) {
                if (_wcsicmp(buf, ServiceName) == 0) {
                    DropRights = false;
                    break;
                }
            } else if (status != STATUS_BUFFER_TOO_SMALL)
                break;
        }
    }

    if (DropRights) {

        if (ServiceName)
            SbieApi_LogEx(*pSessionId, 2214, ServiceName);
        else
            SbieApi_LogEx(*pSessionId, 2217, NULL);

        SbieApi_LogEx(*pSessionId, 2219, L"%S [%S]", exename, boxname);

        return false;
    }

    return true;
}

//---------------------------------------------------------------------------
// CanCallerDoElevation
//---------------------------------------------------------------------------


bool ServiceServer::CanAccessSCM(HANDLE idProcess)
{
	WCHAR boxname[48] = { 0 };
	WCHAR imagename[128] = { 0 };
	SbieApi_QueryProcess(idProcess, boxname, imagename, NULL, NULL); // if this fail we take the global config if present
	if (SbieApi_QueryConfBool(boxname, L"UnrestrictedSCM", FALSE))
		return true;

	//
	// Note: when RpcSs and DcomLaunch are not running as system, thay still are alowed to access the SCM
	//
	if (!SbieApi_QueryConfBool(boxname, L"ProtectRpcSs", FALSE))
	{
		if (_wcsicmp(imagename, SANDBOXIE L"DcomLaunch.exe") == 0)
			return true;
	}

	bool bRet = false;

	PSECURITY_DESCRIPTOR securityDescriptor = NULL;
	SC_HANDLE scHandle = OpenSCManager(NULL, NULL, READ_CONTROL);
	if (scHandle != NULL) {
		GetSecurityInfo(scHandle, SE_SERVICE, OWNER_SECURITY_INFORMATION | GROUP_SECURITY_INFORMATION | DACL_SECURITY_INFORMATION, NULL, NULL, NULL, NULL, &securityDescriptor);
		CloseServiceHandle(scHandle);
	}
	if (!securityDescriptor)
		return bRet;

	/*HANDLE hToken = NULL;
	HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, (DWORD)(UINT_PTR)idProcess);
	if (hProcess != NULL) {
		OpenProcessToken(hProcess, TOKEN_IMPERSONATE | TOKEN_QUERY | TOKEN_DUPLICATE | STANDARD_RIGHTS_READ, &hToken);
		CloseHandle(hProcess);
	}*/

	HANDLE hToken = (HANDLE)SbieApi_QueryProcessInfo(idProcess, 'ptok');
	if (hToken) {
		HANDLE hImpersonatedToken = NULL;
		if (DuplicateToken(hToken, SecurityImpersonation, &hImpersonatedToken)) {
			DWORD accessRights = SC_MANAGER_ALL_ACCESS;
			GENERIC_MAPPING mapping = { 0xFFFFFFFF };
			PRIVILEGE_SET privileges = { 0 };
			DWORD grantedAccess = 0, privilegesLength = sizeof(privileges);
			BOOL result = FALSE;
			//::MapGenericMask(&genericAccessRights, &mapping);
			if (::AccessCheck(securityDescriptor, hImpersonatedToken, accessRights,
				&mapping, &privileges, &privilegesLength, &grantedAccess, &result)) {
				bRet = (result == TRUE);
			}
			CloseHandle(hImpersonatedToken);
		}
		CloseHandle(hToken);
	}
	LocalFree(securityDescriptor);

	return bRet;
}


//---------------------------------------------------------------------------
// ReportError2218
//---------------------------------------------------------------------------


void ServiceServer::ReportError2218(HANDLE idProcess, ULONG errlvl)
{
    ULONG LastError = GetLastError();

    WCHAR boxname[48];
    WCHAR imagename[99];
    ULONG session_id;

    if (0 == SbieApi_QueryProcess(
                        idProcess, boxname, imagename, NULL, &session_id)) {

        SbieApi_LogEx(session_id, 2218, L"[%02X / %08X]", errlvl, LastError);
        SbieApi_LogEx(session_id, 2219, L"%S [%S]", imagename, boxname);
    }
}


//---------------------------------------------------------------------------
// BuildPathForStartExe
//---------------------------------------------------------------------------


WCHAR *ServiceServer::BuildPathForStartExe(
    HANDLE idProcess, const WCHAR *devmap, const WCHAR *svcname,
    const WCHAR *InArgs, WCHAR **OutArgs)
{
    const WCHAR *_env_fmt = L"/env:" ENV_VAR_PFX L"%s=\"%s\" ";

    ULONG args_len = (wcslen(InArgs) + wcslen(devmap) + 192) * sizeof(WCHAR);
    if (svcname)
        args_len += (wcslen(svcname) + 96) * sizeof(WCHAR);

    WCHAR *args = (WCHAR *)HeapAlloc(GetProcessHeap(), 0, args_len);
    if (! args)
        return NULL;

    WCHAR *argptr = args;
    argptr += wsprintf(argptr, L"/box:-%d ", idProcess);
    if (devmap)
        argptr += wsprintf(argptr, _env_fmt, L"DEVICE_MAP", devmap);
    if (svcname) {
        argptr += wsprintf(argptr, _env_fmt, L"SERVICE_NAME", svcname);
        wcscat(argptr, L"/hide_window ");
        argptr += wcslen(argptr);
    }
    wcscpy(argptr, InArgs);

    STARTUPINFO si;
    si.lpReserved = NULL;

    WCHAR *OutPath = NULL;
    if (SbieDll_RunFromHome(START_EXE, args, &si, NULL))
        OutPath = (WCHAR *)si.lpReserved;

    if (OutPath && OutArgs)
        *OutArgs = wcsstr(OutPath, args);

    HeapFree(GetProcessHeap(), HEAP_GENERATE_EXCEPTIONS, args);

    return OutPath;
}


//---------------------------------------------------------------------------
// RunHandler
//---------------------------------------------------------------------------


MSG_HEADER *ServiceServer::RunHandler(MSG_HEADER *msg, HANDLE idProcess)
{
    SetThreadToken(NULL, NULL);

    //
    // parse request packet
    //

    SERVICE_RUN_REQ *req = (SERVICE_RUN_REQ *)msg;
    if (req->h.length < sizeof(SERVICE_RUN_REQ))
        return SHORT_REPLY(ERROR_INVALID_PARAMETER);
    if (req->h.length > PIPE_MAX_DATA_LEN)
        return SHORT_REPLY(ERROR_INVALID_PARAMETER);
    if (req->path_len > PIPE_MAX_DATA_LEN)
        return SHORT_REPLY(STATUS_INVALID_PARAMETER);
    ULONG offset = FIELD_OFFSET(SERVICE_RUN_REQ, path);
    if (offset + req->path_len > req->h.length)
        return SHORT_REPLY(ERROR_INVALID_PARAMETER);

    //
    // execute request
    //

    req->name[63] = L'\0';
    req->devmap[95] = L'\0';

    ULONG error;
    ULONG idSession;

    if (! CanCallerDoElevation(idProcess, req->name, &idSession) || !CanAccessSCM(idProcess))
        error = ERROR_ACCESS_DENIED;
    else {
        WCHAR *svcname = NULL;
        if (req->type & SERVICE_WIN32_OWN_PROCESS)
            svcname = req->name;
        error = RunHandler2(idProcess, idSession, req->devmap,
                            svcname, req->path);
    }

    return SHORT_REPLY(error);
}


//---------------------------------------------------------------------------
// RunHandler2
//---------------------------------------------------------------------------


ULONG ServiceServer::RunHandler2(
    HANDLE idProcess, ULONG idSession,
    const WCHAR *devmap, const WCHAR *svcname, const WCHAR *path)
{
    const ULONG TOKEN_RIGHTS = TOKEN_QUERY          | TOKEN_DUPLICATE
                             | TOKEN_ADJUST_DEFAULT | TOKEN_ADJUST_SESSIONID
                             | TOKEN_ADJUST_GROUPS  | TOKEN_ASSIGN_PRIMARY;

    WCHAR *ExePath   = NULL;
    HANDLE hOldToken = NULL;
    HANDLE hNewToken = NULL;

    ULONG error;
    ULONG errlvl;
    BOOL  ok = TRUE;

    if (ok) {
        errlvl = 0x21;
        ExePath =
            BuildPathForStartExe(idProcess, devmap, svcname, path, NULL);
        if (! ExePath) {
            ok = FALSE;
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        }
    }

    if (ok) {
        errlvl = 0x22;
        ok = OpenProcessToken(GetCurrentProcess(), TOKEN_RIGHTS, &hOldToken);
    }

    if (ok) {
        errlvl = 0x23;
        ok = DuplicateTokenEx(
                hOldToken, TOKEN_RIGHTS, NULL, SecurityAnonymous,
                TokenPrimary, &hNewToken);
    }

    if (ok) {
        errlvl = 0x24;
        ok = SetTokenInformation(
                hNewToken, TokenSessionId, &idSession, sizeof(ULONG));
    }

    if (ok) {
		WCHAR boxname[48] = { 0 };
		SbieApi_QueryProcess(idProcess, boxname, NULL, NULL, NULL); // if this fail we take the global config if present
		if (SbieApi_QueryConfBool(boxname, L"ExposeBoxedSystem", FALSE))
			SetTokenCustomDacl(hNewToken, idProcess, GENERIC_ALL, TRUE); 
		else //if (_wcsicmp(svcname, L"MSIServer") == 0)
			// The MSIServer needs to be extra allowances to work correctly
			SetTokenCustomDacl(hNewToken, idProcess, GENERIC_READ, FALSE);
    }

    if (ok) {

        STARTUPINFO si;
        PROCESS_INFORMATION pi;

        memzero(&si, sizeof(STARTUPINFO));
        si.cb = sizeof(STARTUPINFO);
        si.dwFlags = STARTF_FORCEOFFFEEDBACK;

        errlvl = 0x25;
        ok = CreateProcessAsUser(
                hNewToken, NULL, ExePath, NULL, NULL, FALSE, 0, NULL, NULL,
                &si, &pi);

        if (ok) {

            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);
        }
    }

    if (ok)
        error = ERROR_SUCCESS;
    else {
        error = GetLastError();
        ReportError2218(idProcess, errlvl);
    }

    if (hNewToken)
        CloseHandle(hNewToken);
    if (hOldToken)
        CloseHandle(hOldToken);
    if (ExePath)
        HeapFree(GetProcessHeap(), HEAP_GENERATE_EXCEPTIONS, ExePath);

    return error;
}


//---------------------------------------------------------------------------
// SetTokenCustomDacl
//---------------------------------------------------------------------------


void ServiceServer::SetTokenCustomDacl(HANDLE hNewToken, HANDLE idProcess, DWORD AccessMask, bool useUserSID)
{
    static UCHAR AnonymousLogonSid[12] = {
        1,                                      // Revision
        1,                                      // SubAuthorityCount
        0,0,0,0,0,5, // SECURITY_NT_AUTHORITY   // IdentifierAuthority
        SECURITY_ANONYMOUS_LOGON_RID,0,0,0      // SubAuthority
    };

    HANDLE hProcess;
    HANDLE hToken;
    ULONG len;
    BOOL ok;

    //
    // When SbieSvc launches a service process as SYSTEM, make sure the
    // default DACL of the new process includes the caller's SID.  This
    // resolves a problem where a client MsiExec invokes the service
    // MsiExec, which in turn invokes a custom action MsiExec process,
    // and the client MsiExec fails to open the custom action process.
    //

    UCHAR *WorkSpace = (UCHAR *)HeapAlloc(GetProcessHeap(), 0, 1024);
    if (! WorkSpace)
        return;

	TOKEN_GROUPS	   *pLogOn = (TOKEN_GROUPS *)WorkSpace;
    TOKEN_USER         *pUser = (TOKEN_USER *)WorkSpace;
    TOKEN_DEFAULT_DACL *pDacl = (TOKEN_DEFAULT_DACL *)(WorkSpace + 128);
	PSID pSid;
	
    //
    // get the token for the calling process, extract the user SID
    //

    hProcess = OpenProcess(
        PROCESS_QUERY_INFORMATION, FALSE, (ULONG)(ULONG_PTR)idProcess);
    if (! hProcess)
        goto finish;

    ok = OpenProcessToken(hProcess, TOKEN_QUERY, &hToken);

    CloseHandle(hProcess);

    if (! ok)
        goto finish;

	if (useUserSID)
	{
		ok = GetTokenInformation(hToken, TokenUser, pUser, 128, &len);

		//
		// in Sandboxie version 4, the primary process token is going to be
		// the anonymous token which isn't very useful here, so get the
		// textual SID string and convert it into a SID value
		//

		if (ok && memcmp(pUser->User.Sid, AnonymousLogonSid,
			sizeof(AnonymousLogonSid)) == 0) {

			PSID TempSid;
			WCHAR SidString[96];
			SbieApi_QueryProcess(idProcess, NULL, NULL, SidString, NULL);
			if (SidString[0]) {
				if (ConvertStringSidToSid(SidString, &TempSid)) {
					memcpy(pUser + 1, TempSid, GetLengthSid(TempSid));
					pUser->User.Sid = (PSID)(pUser + 1);
					LocalFree(TempSid);
				}
			}
		}

		pSid = pUser->User.Sid;
	}
	else
	{
		ok = GetTokenInformation(hToken, TokenLogonSid, pLogOn, 128, &len);

		pSid = pLogOn->Groups[0].Sid; // use the LogonSessionId token
	}
	
	CloseHandle(hToken);

	if (!ok)
		goto finish;

	//
    // extract the default DACL, update it and store it back
    //

    ok = GetTokenInformation(
            hNewToken, TokenDefaultDacl, pDacl, (1024 - 128), &len);
    if (! ok)
        goto finish;

    PACL pAcl = pDacl->DefaultDacl;

    pAcl->AclSize += sizeof(ACCESS_ALLOWED_ACE)
                   - sizeof(DWORD)              // minus SidStart member
		+ (WORD)GetLengthSid(pSid);

	AddAccessAllowedAce(pAcl, ACL_REVISION, AccessMask, pSid);

    ok = SetTokenInformation(
            hNewToken, TokenDefaultDacl, pDacl, (1024 - 128));

    //
    // finish
    //

finish:

    HeapFree(GetProcessHeap(), HEAP_GENERATE_EXCEPTIONS, WorkSpace);
}


//---------------------------------------------------------------------------
// UacHandler
//---------------------------------------------------------------------------


MSG_HEADER *ServiceServer::UacHandler(
    MSG_HEADER *msg, HANDLE idProcess, HANDLE idThread)
{
    SetThreadToken(NULL, NULL);

    //
    // parse request packet
    //

    SERVICE_UAC_REQ *req = (SERVICE_UAC_REQ *)msg;
    if (req->h.length < sizeof(SERVICE_UAC_REQ))
        return SHORT_REPLY(ERROR_INVALID_PARAMETER);
    if (req->h.length > PIPE_MAX_DATA_LEN)
        return SHORT_REPLY(ERROR_INVALID_PARAMETER);

    req->devmap[95] = L'\0';

    //
    // uac step 1.  execute request
    //

    ULONG error;
    ULONG idSession;

    if (! CanCallerDoElevation(idProcess, NULL, &idSession))
        error = ERROR_ACCESS_DENIED;
    else {
        error = UacHandler2(idProcess, idThread, req->devmap,
                            (ULONG_PTR)req->uac_pkt_addr, req->uac_pkt_len);
    }

    return SHORT_REPLY(error);
}


//---------------------------------------------------------------------------
// UacHandler2
//---------------------------------------------------------------------------


ULONG ServiceServer::UacHandler2(
    HANDLE idProcess, HANDLE idThread, const WCHAR *devmap,
    ULONG_PTR pkt_addr, ULONG pkt_len)
{
    const ULONG TOKEN_RIGHTS =
        TOKEN_QUERY | TOKEN_DUPLICATE | TOKEN_ASSIGN_PRIMARY;

    WCHAR *ExePath   = NULL;
    HANDLE hThread   = NULL;
    HANDLE hProcess  = NULL;
    HANDLE hOldToken = NULL;
    HANDLE hNewToken = NULL;

    ULONG error;
    ULONG errlvl;
    BOOL  ok = TRUE;

    if (ok) {

        STARTUPINFO si;
        LARGE_INTEGER pkt_addr_64;
        WCHAR cmdline[256];

        pkt_addr_64.QuadPart = pkt_addr;
        wsprintf(cmdline, L"%s_UacProxy:%08X_%08X_%08X_%08X_@%s",
            SANDBOXIE, (ULONG)(ULONG_PTR)idProcess,
            pkt_addr_64.HighPart, pkt_addr_64.LowPart, pkt_len,
            devmap);

        errlvl = 0x41;
        if (SbieDll_RunFromHome(SBIESVC_EXE, cmdline, &si, NULL))
            ExePath = (WCHAR *)si.lpReserved;
        else {
            ok = FALSE;
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        }
    }

    //
    // get caller thread token if available, otherwise use process token
    //

    if (ok) {
        errlvl = 0x42;
        hThread = OpenThread(
            THREAD_QUERY_INFORMATION, FALSE, (ULONG)(ULONG_PTR)idThread);
        if (! hThread)
            ok = FALSE;
    }

    if (ok) {
        errlvl = 0x43;
        ok = OpenThreadToken(hThread, TOKEN_RIGHTS, TRUE, &hOldToken);
        if (ok) {

            errlvl = 0x44;
            ok = DuplicateTokenEx(
                    hOldToken, TOKEN_RIGHTS, NULL, SecurityAnonymous,
                    TokenPrimary, &hNewToken);

        } else {

            ULONG LastError = GetLastError();
            if (LastError == ERROR_NO_TOKEN ||
                LastError == ERROR_BAD_IMPERSONATION_LEVEL ||
                LastError == ERROR_CANT_OPEN_ANONYMOUS) {

                errlvl = 0x45;
                hProcess = OpenProcess(PROCESS_QUERY_INFORMATION,
                                       FALSE, (ULONG)(ULONG_PTR)idProcess);
                if (! hProcess)
                    ok = FALSE;
                else {

                    errlvl = 0x46;
                    ok = OpenProcessToken(
                                hProcess, TOKEN_RIGHTS, &hNewToken);
                }
            }
        }
    }

    if (ok) {

        //
        // uac step 2.  run SbieSvc.exe as a normal program (not service)
        // in the logon session of the caller.  pass UacProxy command line
        //

        STARTUPINFO si;
        PROCESS_INFORMATION pi;

        memzero(&si, sizeof(STARTUPINFO));
        si.cb = sizeof(STARTUPINFO);
        si.dwFlags = STARTF_FORCEOFFFEEDBACK;

        errlvl = 0x45;
        ok = CreateProcessAsUser(
                hNewToken, NULL, ExePath, NULL, NULL,
                FALSE, 0, NULL, NULL, &si, &pi);

        if (ok) {

            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);
        }
    }

    if (ok)
        error = ERROR_SUCCESS;
    else {
        error = GetLastError();
        ReportError2218(idProcess, errlvl);
    }

    if (hNewToken)
        CloseHandle(hNewToken);
    if (hOldToken)
        CloseHandle(hOldToken);
    if (hProcess)
        CloseHandle(hProcess);
    if (hThread)
        CloseHandle(hThread);
    if (ExePath)
        HeapFree(GetProcessHeap(), HEAP_GENERATE_EXCEPTIONS, ExePath);

    return error;
}


//---------------------------------------------------------------------------
// RunUacSlave
//---------------------------------------------------------------------------


void ServiceServer::RunUacSlave(const WCHAR *cmdline)
{
    //
    // collect parameters from command line
    //

    HANDLE         idProcess;
    ULARGE_INTEGER pkt_addr;
    ULONG          pkt_len;
    WCHAR         *ptr;

    ptr = (WCHAR*)wcsrchr(cmdline, L':');
    if (! ptr)
        return;

    idProcess = (HANDLE)(ULONG_PTR)wcstol(ptr + 1, &ptr, 16);
    if (*ptr != L'_')
        return;
    if (! idProcess)
        return;

    pkt_addr.HighPart = wcstoul(ptr + 1, &ptr, 16);
    if (*ptr != L'_')
        return;
    pkt_addr.LowPart = wcstoul(ptr + 1, &ptr, 16);
    if (*ptr != L'_')
        return;

    pkt_len = wcstol(ptr + 1, &ptr, 16);
    if (*ptr != L'_')
        return;

    //
    // invoke sub function
    //

    if (ptr[1] == L'@') {

        //
        // uac step 3.  paint a notification window and run uac step 4
        //

        ULONG_PTR ThreadArgs[5];
        ThreadArgs[0] = (ULONG_PTR)idProcess;
        ThreadArgs[1] = (ULONG_PTR)cmdline;
        ThreadArgs[2] = (ULONG_PTR)&ptr[2];
        ThreadArgs[3] = (ULONG_PTR)pkt_addr.QuadPart;
        ThreadArgs[4] = (ULONG_PTR)pkt_len;

        RunUacSlave2(ThreadArgs);

    } else {

        //
        // uac step 4.  running as an elevated administrator.
        //

        RunUacSlave3(idProcess,
                     (ULONG_PTR)pkt_addr.QuadPart, pkt_len,
                     false, NULL);
    }
}


//---------------------------------------------------------------------------
// RunUacSlave2
//---------------------------------------------------------------------------


void ServiceServer::RunUacSlave2(ULONG_PTR *ThreadArgs)
{
    //
    // extract some information to display
    //

    HANDLE idProcess = (HANDLE)ThreadArgs[0];

    WCHAR BoxName[48];
    if (0 != SbieApi_QueryProcess(idProcess, BoxName, NULL, NULL, NULL))
        return;

    WCHAR *AppName = NULL;
    RunUacSlave3(idProcess, (ULONG_PTR)ThreadArgs[3], (ULONG)ThreadArgs[4],
                 false, &AppName);

#ifdef EXTENSION_DLLNAME
    WCHAR *OrigAppName = L"";
    if (AppName) {
        ULONG AppNameLen = (wcslen(AppName) + 1) * sizeof(WCHAR);
        OrigAppName = (WCHAR *)HeapAlloc(GetProcessHeap(), 0, AppNameLen);
        if (OrigAppName)
            memcpy(OrigAppName, AppName, AppNameLen);
        else
            OrigAppName = L"";
    }
#endif EXTENSION_DLLNAME

    if (AppName) {
        WCHAR *backslash = wcsrchr(AppName, L'\\');
        if (backslash) {
            AppName = backslash + 1;
            WCHAR *quote = wcsrchr(AppName, L'\"');
            if (quote)
                *quote = L'\0';
        } else if (memcmp(AppName, L"*MSI*", 5 * sizeof(WCHAR)) == 0)
            AppName = L"Windows Installer";
    } else
        AppName = L"?";

    //
    // don't display window if caller is already an administrator
    //

    bool isAdmin = IsAdmin();

#ifdef EXTENSION_DLLNAME
    if (CallUacDialogHook(ThreadArgs, idProcess, OrigAppName, isAdmin))
        isAdmin = true;     // pre-approve elevation
#endif EXTENSION_DLLNAME

    if (isAdmin) {

        CreateThread(
            NULL, 0, RunUacSlave2Thread1, (void *)ThreadArgs, 0, NULL); // fix-me: i'm leaking a thread
        CreateThread(
            NULL, 0, RunUacSlave2Thread2, (void *)ThreadArgs, 0, NULL); // fix-me: i'm leaking a thread

        while (1)
            SuspendThread(GetCurrentThread());
    }

    //
    // the strings array is used for communication between
    // code in this function and code in the window procedure
    //

    WCHAR *strings[3];
    strings[0] = BoxName;
    strings[1] = AppName;
    strings[2] = NULL;

    //
    // get UAC shield icon
    //

    HICON hShieldIcon = NULL;
    HICON hShieldIconSm = NULL;

    if (1) {

        typedef struct {
            DWORD cbSize;
            HICON hIcon;
            int iSysImageIndex;
            int iIcon;
            WCHAR szPath[MAX_PATH];
        } SHSTOCKICONINFO;
        typedef HRESULT (*P_SHGetStockIconInfo)(
            ULONG_PTR siid, UINT uFlags, SHSTOCKICONINFO *psii);
        const ULONG SIID_SHIELD = 77;
        const ULONG SHGSI_ICON = 0x000000100;
        const ULONG SHGSI_SMALLICON = 0x000000001;

        HMODULE hShell32 = LoadLibrary(L"shell32.dll");
        if (hShell32) {

            P_SHGetStockIconInfo pSHGetStockIconInfo = (P_SHGetStockIconInfo)
                GetProcAddress(hShell32, "SHGetStockIconInfo");
            if (pSHGetStockIconInfo) {

                SHSTOCKICONINFO sii;
                memzero(&sii, sizeof(SHSTOCKICONINFO));
                sii.cbSize = sizeof(SHSTOCKICONINFO);
                HRESULT hr = pSHGetStockIconInfo(
                                        SIID_SHIELD, SHGSI_ICON, &sii);
                if (SUCCEEDED(hr))
                    hShieldIcon = sii.hIcon;

                hr = pSHGetStockIconInfo(SIID_SHIELD,
                                         SHGSI_ICON | SHGSI_SMALLICON, &sii);
                if (SUCCEEDED(hr))
                    hShieldIconSm = sii.hIcon;
            }
        }
    }

    //
    // create window
    //

    WNDCLASSEX wc;
    memzero(&wc, sizeof(WNDCLASSEX));
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_NOCLOSE;
    wc.lpfnWndProc = RunUacSlave2WndProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.hbrBackground = CreateSolidBrush(0x00404040);
    wc.lpszClassName = SANDBOXIE L"_UAC_WindowClass";
    wc.hIcon = hShieldIcon;
    wc.hIconSm = hShieldIconSm;
    ATOM atom = RegisterClassEx(&wc);

    BOOLEAN rtl;
    SbieDll_GetLanguage(&rtl);

    HWND hWnd = CreateWindowEx(WS_EX_TOPMOST |
                               (rtl ? WS_EX_LAYOUTRTL : 0),
                               (LPCWSTR)atom, SANDBOXIE,
                               WS_SYSMENU | WS_MINIMIZEBOX,
                               10, 10, 600, 400,
                               NULL, NULL, NULL, strings);

    HMONITOR hmonitor = MonitorFromWindow(hWnd, MONITOR_DEFAULTTONULL);
    if (hmonitor) {
        MONITORINFO monitor;
        memzero(&monitor, sizeof(MONITORINFO));
        monitor.cbSize = sizeof(MONITORINFO);
        if (GetMonitorInfo(hmonitor, &monitor)) {
            const ULONG x = monitor.rcWork.left + 10;
            const ULONG y = monitor.rcWork.top  + 10;
            if (x || y) {
                SetWindowPos(hWnd, NULL, x, y, 0, 0,
                    SWP_NOSIZE | SWP_NOZORDER | SWP_NOSENDCHANGING |
                    SWP_NOREDRAW | SWP_NOACTIVATE | SWP_DEFERERASE);
            }
        }
    }

    ShowWindow(hWnd, SW_SHOW);

    //
    // do message loop while waiting for signal to start threads
    //

    while (1) {

        MSG msg;
        if (GetMessageW(&msg, NULL, 0, 0) != -1)
            DispatchMessage(&msg);

        if (strings[2] == strings[0]) {

            strings[2] = strings[1];
            CreateThread(
                NULL, 0, RunUacSlave2Thread1, (void *)ThreadArgs, 0, NULL); // fix-me: i'm leaking a thread
            CreateThread(
                NULL, 0, RunUacSlave2Thread2, (void *)ThreadArgs, 0, NULL); // fix-me: i'm leaking a thread
        }
    }
}


//---------------------------------------------------------------------------
// RunUacSlave2WndProc
//---------------------------------------------------------------------------


LRESULT ServiceServer::RunUacSlave2WndProc(
    HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    static WCHAR **strings;

    if (msg == WM_CREATE) {

        LPCREATESTRUCT pcs = (LPCREATESTRUCT)lParam;
        strings = (WCHAR **)pcs->lpCreateParams;

        /*hMenu = GetSystemMenu(hwnd, FALSE);
        if (hMenu)
            EnableMenuItem(hMenu, SC_CLOSE, MF_GRAYED | MF_BYCOMMAND);*/

        return 0;

    } else if (msg == WM_CLOSE) {

        return 0;

    } else if (msg != WM_PAINT) {

        return DefWindowProc(hwnd, msg, wParam, lParam);
    }

    //
    // paint the contents of the notification window
    //

    PAINTSTRUCT ps;

    HDC hdc = BeginPaint(hwnd, &ps);

    int height = MulDiv(12, GetDeviceCaps(hdc, LOGPIXELSY), 72);
    HFONT hfont = CreateFont(-height, 0, 0, 0,
                             FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET,
                             OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                             DEFAULT_QUALITY, DEFAULT_PITCH,
                             L"Arial");
    SelectObject(hdc, hfont);
    SetBkColor(hdc, 0x00404040);
    SetTextColor(hdc, 0x00FFFFFF);

    int y = 10;
    y = RunUacSlave2WndProcTextOut(hdc, y, height, 3241);

    WCHAR *txtSandbox = SbieDll_FormatMessage0(MSG_3742);
    y += 20 + height;
    TextOut(hdc, 10, y, txtSandbox, wcslen(txtSandbox));
    y += 5 + height;
    SetTextColor(hdc, 0x0080FFFF);
    TextOut(hdc, 10, y, strings[0], wcslen(strings[0]));
    SetTextColor(hdc, 0x00FFFFFF);

    WCHAR *txtProgram = SbieDll_FormatMessage0(MSG_3743);
    y += 20 + height;
    TextOut(hdc, 10, y, txtProgram, wcslen(txtProgram));
    y += 5 + height;
    SetTextColor(hdc, 0x0080FFFF);
    TextOut(hdc, 10, y, strings[1], wcslen(strings[1]));
    SetTextColor(hdc, 0x00FFFFFF);

    y += 20 + height;
    y = RunUacSlave2WndProcTextOut(hdc, y, height, 3242);

    height = MulDiv(9, GetDeviceCaps(hdc, LOGPIXELSY), 72);
    hfont  = CreateFont(-height, 0, 0, 0,
                        FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET,
                        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                        DEFAULT_QUALITY, DEFAULT_PITCH,
                        L"Arial");
    SelectObject(hdc, hfont);
    SetTextColor(hdc, 0x00AAAAAA);

    y += 40 + height;
    y = RunUacSlave2WndProcTextOut(hdc, y, height, 3243);

    EndPaint(hwnd, &ps);

    //
    // signal RunUacSlave2 to create the threads
    //

    if (! strings[2])
        strings[2] = strings[0];

    return 0;
}


//---------------------------------------------------------------------------
// RunUacSlave2WndProcTextOut
//---------------------------------------------------------------------------


int ServiceServer::RunUacSlave2WndProcTextOut(
    HDC hdc, int y, int height, int msgid)
{
    WCHAR txt[512];
    wcscpy(txt, SbieDll_FormatMessage0(msgid));
    WCHAR *txt1 = txt;
    while (1) {
        WCHAR *txt2 = wcschr(txt1, L'\n');
        if (txt2) {
            *txt2 = L'\0';
            ++txt2;
        }
        TextOut(hdc, 10, y, txt1, wcslen(txt1));
        if (! txt2)
            return y;
        y += 5 + height;
        txt1 = txt2;
    }
}


//---------------------------------------------------------------------------
// RunUacSlave2Thread1
//---------------------------------------------------------------------------


ULONG ServiceServer::RunUacSlave2Thread1(void *lpParameters)
{
    ULONG_PTR *ThreadArgs   = (ULONG_PTR *)lpParameters;
    HANDLE     idProcess    = (HANDLE)ThreadArgs[0];
    WCHAR     *cmdline      = (WCHAR *)ThreadArgs[1];
    WCHAR     *devmap       = (WCHAR *)ThreadArgs[2];
    ULONG_PTR  pkt_addr     = ThreadArgs[3];
    ULONG      pkt_len      = (ULONG)ThreadArgs[4];

    //
    // re-build command line as
    //      Start.exe /model... SbieSvc.exe ...UacProxy...
    // remove the @-prefixed device map in the new command line
    //

    int len = (wcslen(cmdline) + MAX_PATH * 2) * sizeof(WCHAR);
    WCHAR *arg = (WCHAR *)HeapAlloc(GetProcessHeap(), 0, len);
    if (! arg) {
        ReportError2218(idProcess, 0x61);
        ExitProcess(0);
    }

    arg[0] = L'\"';
    GetModuleFileName(NULL, &arg[1], MAX_PATH);
    wcscat(arg, L"\" ");
    wcscat(arg, cmdline);
    WCHAR *ptr = wcschr(arg, L'@');
    if (ptr)
        *ptr = L'\0';

    WCHAR *pgm = BuildPathForStartExe(idProcess, devmap, NULL, arg, &arg);
    if (! pgm) {
        ReportError2218(idProcess, 0x61);
        ExitProcess(0);
    }
    arg[-1] = L'\0';        // separate Start.exe from its parameters

    //
    // request elevation for the new process
    //

    SHELLEXECUTEINFO shex;
    memzero(&shex, sizeof(SHELLEXECUTEINFO));
    shex.cbSize = sizeof(SHELLEXECUTEINFO);
    shex.fMask = 0;
    shex.hwnd = NULL;
    shex.lpVerb = L"runas";
    shex.lpFile = pgm;
    shex.lpParameters = arg;
    shex.lpDirectory = NULL;
    shex.nShow = SW_SHOWNORMAL;
    shex.hInstApp = NULL;

    typedef BOOL (*P_ShellExecuteEx)(void *);
    HMODULE shell32 = LoadLibrary(L"shell32.dll");
    P_ShellExecuteEx pShellExecuteEx = (P_ShellExecuteEx)
        GetProcAddress(shell32, "ShellExecuteExW");
    if (pShellExecuteEx) {

        BOOL ok = pShellExecuteEx(&shex);
        if ((! ok) || ((int)(ULONG_PTR)shex.hInstApp <= 32)) {

            if (GetLastError() != ERROR_CANCELLED)
                ReportError2218(idProcess, 0x62);

            RunUacSlave3(idProcess, pkt_addr, pkt_len, true, NULL);
        }
    }

    ExitProcess(0);
}


//---------------------------------------------------------------------------
// RunUacSlave2Thread2
//---------------------------------------------------------------------------


ULONG ServiceServer::RunUacSlave2Thread2(void *lpParameters)
{
    ULONG_PTR *ThreadArgs   = (ULONG_PTR *)lpParameters;
    HANDLE     idProcess    = (HANDLE)ThreadArgs[0];

    HANDLE hProcess = OpenProcess(
                            SYNCHRONIZE, FALSE, (ULONG)(ULONG_PTR)idProcess);
    if (hProcess) {

        if (WaitForSingleObject(hProcess, INFINITE) == WAIT_OBJECT_0)
            ExitProcess(0);
    }

    return 0;
}


//---------------------------------------------------------------------------
// SECURE_UAC_PACKET
//---------------------------------------------------------------------------


typedef struct _SECURE_UAC_PACKET {

    //
    // keep in sync with SbieDll / secure.c
    //

    ULONG   tzuk;
    ULONG   len;
    ULONG   app_len;
    ULONG   app_ofs;
    ULONG   cmd_len;
    ULONG   cmd_ofs;
    ULONG   dir_len;
    ULONG   dir_ofs;
    ULONG   inv_len;
    ULONG64 hEvent;
    ULONG64 hResult;
    ULONG64 ret_code;
    WCHAR   text[1];

} SECURE_UAC_PACKET;


//---------------------------------------------------------------------------
// RunUacSlave3
//---------------------------------------------------------------------------


void ServiceServer::RunUacSlave3(
    HANDLE idProcess, ULONG_PTR pkt_addr, ULONG pkt_len,
    bool JustFail, WCHAR **OutAppName)
{
    HANDLE hProcess        = NULL;
    SECURE_UAC_PACKET *pkt = NULL;
    HANDLE hEvent          = NULL;

    SIZE_T copy_len;

    ULONG errlvl;
    BOOL  ok = TRUE;

    //
    // open client process
    //

    if (ok) {
        errlvl = 0x81;
        if (0 != SbieApi_QueryProcess(idProcess, NULL, NULL, NULL, NULL)) {
            ok = FALSE;
            SetLastError(ERROR_ACCESS_DENIED);
        }
    }

    if (ok) {

        WCHAR BoxName[48];
        SbieApi_QueryProcess((HANDLE)(ULONG_PTR)GetCurrentProcessId(),
                             BoxName, NULL, NULL, NULL);
        if (BoxName[0]){

            errlvl = 0x89;
            hProcess = NULL;
            SbieApi_OpenProcess(&hProcess, idProcess);
            if (! hProcess)
                SetLastError(ERROR_ACCESS_DENIED);

        } else {

            errlvl = 0x82;
            hProcess = OpenProcess(PROCESS_VM_READ | PROCESS_VM_WRITE |
                                   PROCESS_VM_OPERATION | PROCESS_DUP_HANDLE,
                                   FALSE, (ULONG)(ULONG_PTR)idProcess);

            if ((! hProcess) && OutAppName) {

                //
                // if the thread that called UacHandler is impersonating
                // a token that can't open its own process, then OpenProcess
                // above will fail, so fallback to just show the image name
                //

                WCHAR *AppName = (WCHAR *)HeapAlloc(
                            GetProcessHeap(), 0, 99 * sizeof(WCHAR));
                if (AppName) {
                    SbieApi_QueryProcess(
                                    idProcess, NULL, AppName, NULL, NULL);
                    if (*AppName) {
                        *OutAppName = AppName;
                        return;
                    }
                }

                errlvl = 0x8A;
                ok = FALSE;
            }
        }

        if (! hProcess)
            ok = FALSE;
    }

    //
    // get and validate request packet
    //

    if (ok) {
        errlvl = 0x83;
        pkt = (SECURE_UAC_PACKET *)HeapAlloc(GetProcessHeap(), 0, pkt_len);
        if (! pkt) {
            ok = FALSE;
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        }
    }

    if (ok) {
        errlvl = 0x84;
        ok = ReadProcessMemory(hProcess, (void *)(ULONG_PTR)pkt_addr, pkt,
                               pkt_len, &copy_len);
        if (ok && (copy_len  != pkt_len ||
                   pkt->tzuk != tzuk ||
                   pkt->len  != pkt_len ||
                   pkt_len   != ~pkt->inv_len)) {

            ok = FALSE;
            SetLastError(ERROR_INVALID_DATA);
        }
    }

    //
    // execute request and copy results back to caller
    //

    if (ok) {

        pkt->hResult  = 0;
        pkt->ret_code = ERROR_CANCELLED;

        if (! JustFail) {

            if (! RunUacSlave4(hProcess, pkt, OutAppName)) {

                ReportError2218(idProcess, 0x85);
            }

            if (OutAppName)
                return;
        }
    }

    if (ok) {

        ULONG_PTR copy_addr = (UCHAR *)&pkt->hResult - (UCHAR *)pkt;
        copy_addr += pkt_addr;

        errlvl = 0x86;
        ok = WriteProcessMemory(
                hProcess, (void *)(ULONG_PTR)copy_addr, &pkt->hResult,
                sizeof(ULONG64) * 2, &copy_len);

        if (ok && pkt->hEvent) {

            errlvl = 0x87;
            ok = DuplicateHandle(
                    hProcess, (HANDLE)(ULONG_PTR)pkt->hEvent,
                    GetCurrentProcess(), &hEvent,
                    EVENT_MODIFY_STATE, FALSE, 0);
            if (ok) {

                errlvl = 0x88;
                ok = SetEvent(hEvent);
            }
        }
    }

    if (! ok)
        ReportError2218(idProcess, errlvl);
}


//---------------------------------------------------------------------------
// RunUacSlave4
//---------------------------------------------------------------------------


bool ServiceServer::RunUacSlave4(
    HANDLE hClientProcess, void *xpkt, WCHAR **OutAppName)
{
    SECURE_UAC_PACKET *pkt = (SECURE_UAC_PACKET *)xpkt;

    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    ULONG app_len, cmd_len, dir_len;
    UCHAR *ptr;
    WCHAR *app, *cmd, *dir;
    BOOL ok;

    app_len = pkt->app_len;
    if (app_len > 1024)
        return false;

    cmd_len = pkt->cmd_len;
    if (cmd_len > 1024)
        return false;

    dir_len = pkt->dir_len;
    if (dir_len > 1024)
        return false;

    ptr = (UCHAR *)pkt;

    app = (WCHAR *)HeapAlloc(
                        GetProcessHeap(), 0, (app_len + 1) * sizeof(WCHAR));
    if (! app)
        return false;
    wmemcpy(app, (WCHAR*)(ptr + pkt->app_ofs), app_len);
    app[app_len] = L'\0';

    cmd = (WCHAR *)HeapAlloc(
                        GetProcessHeap(), 0, (cmd_len + 1) * sizeof(WCHAR));
    if (! cmd)
        return false;
    wmemcpy(cmd, (WCHAR*)(ptr + pkt->cmd_ofs), cmd_len);
    cmd[cmd_len] = L'\0';

    if (OutAppName) {
        *OutAppName = cmd;
        return true;
    }

    dir = (WCHAR *)HeapAlloc(
                        GetProcessHeap(), 0, (dir_len + 1) * sizeof(WCHAR));
    if (! dir)
        return false;
    wmemcpy(dir, (WCHAR*)(ptr + pkt->dir_ofs), dir_len);
    dir[dir_len] = L'\0';

    //
    // elevation type 2:  when input is *MSI*, just return token handle
    //

    if (memcmp(app, L"*MSI*", 5 * sizeof(WCHAR)) == 0 &&
        memcmp(app, cmd,      5 * sizeof(WCHAR)) == 0 &&
        memcmp(app, dir,      5 * sizeof(WCHAR)) == 0) {

        HANDLE hOldToken, hNewToken;

        ok = OpenProcessToken(
                GetCurrentProcess(), TOKEN_ALL_ACCESS, &hOldToken);
        if (ok) {

            ok = DuplicateToken(
                    hOldToken, SecurityImpersonation, &hNewToken);

            if (ok) {

                ok = DuplicateHandle(GetCurrentProcess(), hNewToken,
                                     hClientProcess, (HANDLE *)&pkt->hResult,
                                     0, FALSE, DUPLICATE_SAME_ACCESS);

                CloseHandle(hNewToken);
            }

            CloseHandle(hOldToken);
        }

        if (ok)
            pkt->ret_code = ERROR_SUCCESS;
        else
            pkt->ret_code = GetLastError();

        return true;
    }

    //
    // elevation type 1:  create process and return process handle
    //
    // note that SbieDll hooks RtlQueryElevationFlags in order to prevent
    // a potential loop when dealing with EXEs that get auto elevation.
    // note also a similar behavior in Sxs_CheckManifestForCreateProcess.
    //

    memzero(&si, sizeof(STARTUPINFO));
    si.cb = sizeof(STARTUPINFO);
    memzero(&pi, sizeof(PROCESS_INFORMATION));

    ok = CreateProcess(app, cmd, NULL, NULL, FALSE, 0, NULL, dir, &si, &pi);
    if (! ok)
        pkt->ret_code = GetLastError();
    else {

        ok = DuplicateHandle(GetCurrentProcess(), pi.hProcess,
                             hClientProcess, (HANDLE *)&pkt->hResult,
                             0, FALSE, DUPLICATE_SAME_ACCESS);

        if (ok)
            pkt->ret_code = ERROR_SUCCESS;
        else
            pkt->ret_code = GetLastError();
    }

    return true;
}


//---------------------------------------------------------------------------
// CallUacDialogHook
//---------------------------------------------------------------------------


#ifdef EXTENSION_DLLNAME


bool ServiceServer::CallUacDialogHook(
    ULONG_PTR *ThreadArgs, HANDLE idProcess,
    const WCHAR *cmdline, bool isAdmin)
{
    WCHAR dllpath[384+64];
    if (0 != SbieApi_GetHomePath(NULL, 0, dllpath, 384))
        return false;
    wcscat(dllpath, L"\\" EXTENSION_DLLNAME);
    HMODULE extdll = LoadLibrary(dllpath);
    if (! extdll)
        return false;

    typedef LONG_PTR (*P_UacDialog)(HMODULE, ULONG, const WCHAR *, int);
    P_UacDialog pUacDialog =
                    (P_UacDialog)GetProcAddress(extdll, "UacDialog");
#ifndef _WIN64
    if (! pUacDialog)
        pUacDialog = (P_UacDialog)GetProcAddress(extdll, "_UacDialog@16");
#endif ! _WIN64
    if (! pUacDialog)
        return false;

    LONG_PTR ReturnCode = pUacDialog(
                GetModuleHandle(SBIEDLL L".dll"),
                (ULONG)(ULONG_PTR)idProcess, cmdline, isAdmin ? 1 : 0);

    if (ReturnCode == 1)        // allow elevation request
        return true;

    if (ReturnCode == -1) {     // deny elevation request
        ULONG_PTR  pkt_addr     = ThreadArgs[3];
        ULONG      pkt_len      = (ULONG)ThreadArgs[4];
        RunUacSlave3(idProcess, pkt_addr, pkt_len, true, NULL);
        ExitProcess(0);
    }

    return false;               // continue with default processing
}


#endif EXTENSION_DLLNAME


//---------------------------------------------------------------------------
// IsAdmin
//---------------------------------------------------------------------------


bool ServiceServer::IsAdmin()
{
    //
    // check if token is member of the Administrators group
    //

    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
    PSID AdministratorsGroup;
    BOOL b = AllocateAndInitializeSid(
                &NtAuthority,
                2,
                SECURITY_BUILTIN_DOMAIN_RID,
                DOMAIN_ALIAS_RID_ADMINS,
                0, 0, 0, 0, 0, 0,
                &AdministratorsGroup);
    if (b) {

        if (! CheckTokenMembership(NULL, AdministratorsGroup, &b))
            b = FALSE;

        FreeSid(AdministratorsGroup);
    }

    return b ? true : false;
}
