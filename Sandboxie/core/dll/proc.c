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
// Process
//---------------------------------------------------------------------------

#include "dll.h"

#include <windows.h>
#include <stdio.h>
#include "dll.h"
#include "common/win32_ntddk.h"
#include "advapi.h"
#include "core/svc/ServiceWire.h"
#include "core/drv/api_defs.h"
#include "msgs/msgs.h"


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------

static BOOL Proc_CreateProcessInternalW(
    HANDLE hToken,
    const WCHAR *lpApplicationName,
    WCHAR *lpCommandLine,
    LPSECURITY_ATTRIBUTES lpProcessAttributes,
    LPSECURITY_ATTRIBUTES lpThreadAttributes,
    BOOL bInheritHandles,
    ULONG dwCreationFlags,
    void *lpEnvironment,
    void *lpCurrentDirectory,
    LPSTARTUPINFOW lpStartupInfo,
    LPPROCESS_INFORMATION lpProcessInformation,
    HANDLE *hNewToken);

static BOOL Proc_CreateProcessInternalW_RS5(
    HANDLE hToken,
    const WCHAR *lpApplicationName,
    WCHAR *lpCommandLine,
    LPSECURITY_ATTRIBUTES lpProcessAttributes,
    LPSECURITY_ATTRIBUTES lpThreadAttributes,
    BOOL bInheritHandles,
    ULONG dwCreationFlags,
    void *lpEnvironment,
    void *lpCurrentDirectory,
    LPSTARTUPINFOW lpStartupInfo,
    LPPROCESS_INFORMATION lpProcessInformation,
    HANDLE *hNewToken);

static BOOL Proc_UpdateProcThreadAttribute(
	_Inout_ LPPROC_THREAD_ATTRIBUTE_LIST lpAttributeList,
	_In_ DWORD dwFlags,
	_In_ DWORD_PTR Attribute,
	_In_reads_bytes_opt_(cbSize) PVOID lpValue,
	_In_ SIZE_T cbSize,
	_Out_writes_bytes_opt_(cbSize) PVOID lpPreviousValue,
	_In_opt_ PSIZE_T lpReturnSize);

static BOOL Proc_SetProcessMitigationPolicy(
    _In_ PROCESS_MITIGATION_POLICY MitigationPolicy,
    _In_reads_bytes_(dwLength) PVOID lpBuffer,
    _In_ SIZE_T dwLength);

static BOOL Proc_AlternateCreateProcess(
    const WCHAR *lpApplicationName, WCHAR *lpCommandLine,
    void *lpCurrentDirectory, LPPROCESS_INFORMATION lpProcessInformation,
    BOOL *ReturnValue);

static BOOL Proc_CreateProcessInternalW_Impersonate(HANDLE TokenHandle);

/*static BOOL Proc_CreateProcessInternalW_3(
    HANDLE TokenHandle, WCHAR *cmd, WCHAR *dir, ULONG creation_flags,
    LPSTARTUPINFOW StartupInfo, LPPROCESS_INFORMATION ProcessInformation);*/

static WCHAR *Proc_SelectCurrentDirectory(const WCHAR *lpCurrentDirectory);

static void Proc_QuoteCommandLine_XP(
    THREAD_DATA *TlsData, const WCHAR *CommandLine, const WCHAR *ImagePath);

static _FX void Proc_FixBatchCommandLine(
    THREAD_DATA *TlsData, const WCHAR *CommandLine, const WCHAR *ImagePath);

static void Proc_StoreImagePath(THREAD_DATA *TlsData, HANDLE FileHandle);

static NTSTATUS Proc_RtlCreateProcessParametersEx(
    void *ProcessParameters,
    UNICODE_STRING *ImagePathName,
    UNICODE_STRING *DllPath,
    UNICODE_STRING *CurrentDirectory,
    UNICODE_STRING *CommandLine,
    void *Environment,
    UNICODE_STRING *WindowTitle,
    UNICODE_STRING *DesktopInfo,
    UNICODE_STRING *ShellInfo,
    UNICODE_STRING *RuntimeData,
    void *UnknownParameter11);

static BOOL Proc_CreateProcessWithTokenW(
    HANDLE hToken,
    ULONG dwLogonFlags,
    void *lpApplicationName,
    void *lpCommandLine,
    ULONG dwCreationFlags,
    void *lpEnvironment,
    void *lpCurrentDirectory,
    LPSTARTUPINFOW lpStartupInfo,
    LPPROCESS_INFORMATION lpProcessInformation);

static UINT Proc_WinExec(LPCSTR lpCmdLine, UINT uCmdShow);

static void Proc_ExitProcess(UINT ExitCode);

static BOOLEAN Proc_CheckMailer(const WCHAR *ImagePath, BOOLEAN IsBoxedPath);

static BOOLEAN Proc_IsSoftwareUpdateW(const WCHAR *path);

static BOOLEAN Proc_IsProcessRunning(const WCHAR *ImageToFind);

static BOOLEAN Proc_IsSplWow64(
    const WCHAR *lpApplicationName, const WCHAR *lpCommandLine,
    LPPROCESS_INFORMATION lpProcessInformation);

static HRESULT Proc_RunSetupCommandW(
    HWND hWnd,
    const WCHAR *szCmdName,
    const WCHAR *szInfSection,
    const WCHAR *szDir,
    const WCHAR *szTitle,
    HANDLE *phEXE,
    ULONG dwFlags,
    void *pvReserved);

static BOOLEAN Proc_IsForcedElevation(
    const WCHAR *lpCommandLine,
    const WCHAR *lpCurrentDirectory,
    LPPROCESS_INFORMATION lpProcessInformation);

static NTSTATUS Proc_NtSetInformationProcess(
    HANDLE ProcessHandle, ULONG InfoClass, void *Info, ULONG InfoLength);

static NTSTATUS Proc_NtQueryInformationProcess(
    HANDLE ProcessHandle, PROCESSINFOCLASS ProcessInformationClass, PVOID ProcessInformation, ULONG ProcessInformationLength, PULONG ReturnLength);

static NTSTATUS Proc_NtCreateProcessEx(
    HANDLE *ProcessHandle, ACCESS_MASK DesiredAccess,
    OBJECT_ATTRIBUTES *ObjectAttributes, HANDLE *ParentProcessHandle,
    BOOLEAN InheritHandles, HANDLE SectionHandle, HANDLE DebugPort,
    HANDLE ExceptionPort, ULONG  UnknownExtraParameter);

extern NTSTATUS File_GetName(
    HANDLE RootDirectory, UNICODE_STRING *ObjectName,
    WCHAR **OutTruePath, WCHAR **OutCopyPath, ULONG *OutFlags);

//---------------------------------------------------------------------------


typedef BOOL (*P_CreateProcessInternal)(
    HANDLE hToken,
    const void *lpApplicationName,
    void *lpCommandLine,
    LPSECURITY_ATTRIBUTES lpProcessAttributes,
    LPSECURITY_ATTRIBUTES lpThreadAttributes,
    BOOL bInheritHandles,
    ULONG dwCreationFlags,
    void *lpEnvironment,
    void *lpCurrentDirectory,
    void *lpStartupInfo,
    LPPROCESS_INFORMATION lpProcessInformation,
    HANDLE *hNewToken);

typedef BOOL (*P_CreateProcessInternal_RS5)(
    HANDLE hToken,
    const void *lpApplicationName,
    void *lpCommandLine,
    LPSECURITY_ATTRIBUTES lpProcessAttributes,
    LPSECURITY_ATTRIBUTES lpThreadAttributes,
    BOOL bInheritHandles,
    ULONG dwCreationFlags,
    void *lpEnvironment,
    void *lpCurrentDirectory,
    void *lpStartupInfo,
    LPPROCESS_INFORMATION lpProcessInformation,
    HANDLE *hNewToken);

typedef BOOL (*P_CreateProcessWithTokenW)(
    HANDLE hToken,
    ULONG dwLogonFlags,
    const void *lpApplicationName,
    void *lpCommandLine,
    ULONG dwCreationFlags,
    void *lpEnvironment,
    void *lpCurrentDirectory,
    void *lpStartupInfo,
    LPPROCESS_INFORMATION lpProcessInformation);

typedef NTSTATUS (*P_RtlCreateProcessParametersEx)(
    void *ProcessParameters,
    UNICODE_STRING *ImagePathName,
    UNICODE_STRING *DllPath,
    UNICODE_STRING *CurrentDirectory,
    UNICODE_STRING *CommandLine,
    void *Environment,
    UNICODE_STRING *WindowTitle,
    UNICODE_STRING *DesktopInfo,
    UNICODE_STRING *ShellInfo,
    UNICODE_STRING *RuntimeData,
    void *UnknownParameter11);

typedef void (*P_ExitProcess)(UINT ExitCode);

typedef UINT (*P_WinExec)(LPCSTR lpCmdLine, UINT uCmdShow);

typedef HRESULT (*P_RunSetupCommandW)(
    HWND hWnd,
    const WCHAR *szCmdName,
    const WCHAR *szInfSection,
    const WCHAR *szDir,
    const WCHAR *szTitle,
    HANDLE *phEXE,
    ULONG dwFlags,
    void *pvReserved);

typedef BOOL(*P_GetTokenInformation)(
    _In_ HANDLE TokenHandle,
    _In_ TOKEN_INFORMATION_CLASS TokenInformationClass,
    _Out_writes_bytes_to_opt_(TokenInformationLength, *ReturnLength) LPVOID TokenInformation,
    _In_ DWORD TokenInformationLength,
    _Out_ PDWORD ReturnLength);

typedef BOOL(*P_SetTokenInformation)(
	_In_ HANDLE TokenHandle,
	_In_ TOKEN_INFORMATION_CLASS TokenInformationClass,
	_In_reads_bytes_(TokenInformationLength) LPVOID TokenInformation,
	_In_ DWORD TokenInformationLength);

typedef BOOL(*P_AddAccessAllowedAceEx)(
	_Inout_ PACL pAcl,
	_In_ DWORD dwAceRevision,
	_In_ DWORD AccessMask,
	_In_ PSID pSid);

typedef BOOL(*P_GetLengthSid)(
	_In_ _Post_readable_byte_size_(return) PSID pSid);

typedef BOOL(*P_UpdateProcThreadAttribute)(
	_Inout_ LPPROC_THREAD_ATTRIBUTE_LIST lpAttributeList,
	_In_ DWORD dwFlags,
	_In_ DWORD_PTR Attribute,
	_In_reads_bytes_opt_(cbSize) PVOID lpValue,
	_In_ SIZE_T cbSize,
	_Out_writes_bytes_opt_(cbSize) PVOID lpPreviousValue,
	_In_opt_ PSIZE_T lpReturnSize);

typedef BOOL (*P_SetProcessMitigationPolicy)(
    _In_ PROCESS_MITIGATION_POLICY MitigationPolicy,
    _In_reads_bytes_(dwLength) PVOID lpBuffer,
    _In_ SIZE_T dwLength);

//---------------------------------------------------------------------------


static P_CreateProcessInternal      __sys_CreateProcessInternalW    = NULL;

static P_CreateProcessInternal_RS5      __sys_CreateProcessInternalW_RS5 = NULL;

static P_CreateProcessWithTokenW    __sys_CreateProcessWithTokenW   = NULL;

static P_RtlCreateProcessParametersEx
                                  __sys_RtlCreateProcessParametersEx = NULL;

static P_ExitProcess                __sys_ExitProcess               = NULL;

static P_WinExec                    __sys_WinExec                   = NULL;

static P_RunSetupCommandW           __sys_RunSetupCommandW          = NULL;

static P_NtSetInformationProcess    __sys_NtSetInformationProcess   = NULL;
static P_NtQueryInformationProcess  __sys_NtQueryInformationProcess = NULL;

static P_NtCreateProcessEx          __sys_NtCreateProcessEx         = NULL;

static P_GetTokenInformation        __sys_GetTokenInformation       = NULL;
/*static P_SetTokenInformation        __sys_SetTokenInformation		= NULL;

static P_AddAccessAllowedAceEx      __sys_AddAccessAllowedAceEx		= NULL;

static P_GetLengthSid				__sys_GetLengthSid				= NULL;*/

static P_UpdateProcThreadAttribute	__sys_UpdateProcThreadAttribute = NULL;

static P_SetProcessMitigationPolicy	__sys_SetProcessMitigationPolicy = NULL;

//---------------------------------------------------------------------------
// Variables
//---------------------------------------------------------------------------


static HANDLE Proc_LastCreatedProcessHandle = NULL;

static BOOL     g_boolWasWerFaultLastProcess = FALSE;

//---------------------------------------------------------------------------
// Proc_Init
//---------------------------------------------------------------------------


_FX BOOLEAN Proc_Init(void)
{
    P_CreateProcessInternal CreateProcessInternalW;
    ANSI_STRING ansi;
    NTSTATUS status;

    //
    // abort if we should not hook any process creation functions
    //

    if (Dll_SkipHook(L"createproc"))
        return TRUE;

    //
    // processes that call CreateProcessAsUser will fail if we don't
    // intercept and fake success for NtSetInformationProcess
    //

    SBIEDLL_HOOK(Proc_,NtSetInformationProcess);

    SBIEDLL_HOOK(Proc_,NtQueryInformationProcess);

    //
    // RtlCreateProcessParameters
    //

    if (Dll_OsBuild >= 6000) {

        P_RtlCreateProcessParametersEx RtlCreateProcessParametersEx =
            (P_RtlCreateProcessParametersEx) GetProcAddress(
                Dll_Ntdll, "RtlCreateProcessParametersEx");

        SBIEDLL_HOOK(Proc_,RtlCreateProcessParametersEx);
    }

    //
    // UpdateProcThreadAttribute
    //

	// fix for chrome 86+
	if (Dll_OsBuild >= 7600) {
		void* UpdateProcThreadAttribute = NULL;
		RtlInitString(&ansi, "UpdateProcThreadAttribute");
		status = LdrGetProcedureAddress(
			Dll_KernelBase, &ansi, 0, (void **)&UpdateProcThreadAttribute);
		if (NT_SUCCESS(status))
			SBIEDLL_HOOK(Proc_, UpdateProcThreadAttribute);
	}

    //
    // SetProcessMitigationPolicy
    //

    // fox for SBIE2303 Could not hook ... (33, 1655) due to mitigation policies
    if (Dll_OsBuild >= 8400)    // win8
    {
        void* SetProcessMitigationPolicy = NULL;
        RtlInitString(&ansi, "SetProcessMitigationPolicy");
        status = LdrGetProcedureAddress(
            Dll_KernelBase, &ansi, 0, (void**)&SetProcessMitigationPolicy);
        if (NT_SUCCESS(status))
            SBIEDLL_HOOK(Proc_, SetProcessMitigationPolicy);
    }

    //
    // CreateProcessInternal
    //

    RtlInitString(&ansi, "CreateProcessInternalW");

    if (Dll_OsBuild >= 7600) {
        status = LdrGetProcedureAddress(
            Dll_KernelBase, &ansi, 0, (void **)&CreateProcessInternalW);
    } else
        status = STATUS_UNSUCCESSFUL;

    if (! NT_SUCCESS(status)) {
        status = LdrGetProcedureAddress(
            Dll_Kernel32, &ansi, 0, (void **)&CreateProcessInternalW);
    }

    if(Dll_OsBuild < 17677) {
    
        SBIEDLL_HOOK(Proc_,CreateProcessInternalW);
    }
    else {
        P_CreateProcessInternal_RS5 CreateProcessInternalW_RS5 = CreateProcessInternalW;
        SBIEDLL_HOOK(Proc_,CreateProcessInternalW_RS5);
    }

    //
    // ExitProcess
    //

    if (Dll_ImageType == DLL_IMAGE_SERVICE_MODEL_REG) {

        SBIEDLL_HOOK(Proc_,ExitProcess);
    }

    //
    // WinExec
    //

    SBIEDLL_HOOK(Proc_,WinExec);

    //
    // NtCreateProcessEx
    //

    //if (Dll_OsBuild >= 9600) {

    //    P_NtCreateProcessEx NtCreateProcessEx =
    //        (P_NtCreateProcessEx) GetProcAddress(
    //            Dll_Ntdll, "NtCreateProcessEx");

    //    SBIEDLL_HOOK(Proc_,NtCreateProcessEx);
    //}

    return TRUE;
}


//---------------------------------------------------------------------------
// Proc_Init_AdvApi
//---------------------------------------------------------------------------


_FX BOOLEAN Proc_Init_AdvApi(HMODULE module)
{
    if (__sys_CreateProcessInternalW && Dll_OsBuild >= 6000) {

        P_CreateProcessWithTokenW CreateProcessWithTokenW =
            (P_CreateProcessWithTokenW) GetProcAddress(
                module, "CreateProcessWithTokenW");

        SBIEDLL_HOOK(Proc_,CreateProcessWithTokenW);
    }

    __sys_GetTokenInformation = (P_GetTokenInformation) GetProcAddress(module, "GetTokenInformation");
	/*__sys_SetTokenInformation = (P_SetTokenInformation) GetProcAddress(module, "SetTokenInformation");

	__sys_AddAccessAllowedAceEx = (P_AddAccessAllowedAceEx) GetProcAddress(module, "AddAccessAllowedAceEx");

	__sys_GetLengthSid = (P_GetLengthSid) GetProcAddress(module, "GetLengthSid");*/

    return TRUE;
}

_FX void SetTokenDefaultDaclToProcess(HANDLE hToken, HANDLE hProcess)
{
    DWORD dwTokenInfoLen = 0;
    DWORD dwTokenInfoLenReturn = 0;
    PTOKEN_DEFAULT_DACL pTokenInfo = NULL;

    // Use hToken's default dacl on the process's security descriptor
    if (__sys_GetTokenInformation && hToken && hProcess) {

        __sys_GetTokenInformation(hToken, TokenDefaultDacl, NULL, 0, &dwTokenInfoLen);

        if (dwTokenInfoLen) {

            pTokenInfo = Dll_Alloc(dwTokenInfoLen);

            if (pTokenInfo) {

                if (__sys_GetTokenInformation(hToken, TokenDefaultDacl, pTokenInfo, dwTokenInfoLen, &dwTokenInfoLenReturn)) {

                    if (pTokenInfo && pTokenInfo->DefaultDacl) {

                        if (__sys_SetSecurityInfo) {

                            if (ERROR_SUCCESS != __sys_SetSecurityInfo(hProcess, SE_KERNEL_OBJECT, DACL_SECURITY_INFORMATION,
                                NULL, NULL, pTokenInfo->DefaultDacl, NULL)) {

                                // OutputDebugStringW(L"__sys_SetSecurityInfo failed\n");
                            }
                        }
                    }
                }
                else {

                    //OutputDebugStringW(L"__sys_GetTokenInformation failed\n");
                }

                Dll_Free(pTokenInfo);
            }
        }
        else {
            // OutputDebugStringW(L"Invalid parameter\n");
        }
    }
}

//---------------------------------------------------------------------------
// Proc_CreateProcessInternalW
//---------------------------------------------------------------------------


_FX BOOL Proc_CreateProcessInternalW(
    HANDLE hToken,
    const WCHAR *lpApplicationName,
    WCHAR *lpCommandLine,
    LPSECURITY_ATTRIBUTES lpProcessAttributes,
    LPSECURITY_ATTRIBUTES lpThreadAttributes,
    BOOL bInheritHandles,
    ULONG dwCreationFlags,
    void *lpEnvironment,
    void *lpCurrentDirectory,
    LPSTARTUPINFOW lpStartupInfo,
    LPPROCESS_INFORMATION lpProcessInformation,
    HANDLE *hNewToken)
{
    THREAD_DATA *TlsData = Dll_GetTlsData(NULL);

    void *SaveOwnerProcess;
    void *SaveOwnerThread;
    void *SaveCurrentDirectory;
    ULONG err;
    BOOL ok;
    BOOL resume_thread = FALSE;

    Proc_LastCreatedProcessHandle = NULL;

    //
    // check if we block the process or launch it some other way
    //

    if (Proc_AlternateCreateProcess(
                lpApplicationName, lpCommandLine, lpCurrentDirectory,
                lpProcessInformation, &ok)) {

        return ok;
    }

    //
    // hack:  recent versions of Flash Player use the Chrome sandbox
    // architecture which conflicts with our restricted process model
    //

    if (Dll_ImageType == DLL_IMAGE_FLASH_PLAYER_SANDBOX ||
        Dll_ImageType == DLL_IMAGE_ACROBAT_READER ||
        Dll_ImageType == DLL_IMAGE_PLUGIN_CONTAINER)
        hToken = NULL;

    //
    // use a copy path for the current directory
    // if there is a copy directory in the sandbox
    //

    SaveCurrentDirectory = lpCurrentDirectory;

    lpCurrentDirectory = Proc_SelectCurrentDirectory(lpCurrentDirectory);

    if (! lpCurrentDirectory)
        lpCurrentDirectory = SaveCurrentDirectory;

    //
    // alter environment to pass some strings to the child process
    //

    lpEnvironment = File_AllocAndInitEnvironment(
            lpEnvironment,
            (dwCreationFlags & CREATE_UNICODE_ENVIRONMENT ? TRUE : FALSE),
            FALSE, NULL);

    dwCreationFlags |= CREATE_UNICODE_ENVIRONMENT;

    //
    // invoke the real CreateProcessInternal so it can record acurate
    // paths in NtCreateSection/Proc_SectionCallback, and then fail.
    //

    ++TlsData->proc_create_process;

    TlsData->proc_create_process_capture_image = TRUE;

    ok = __sys_CreateProcessInternalW(
                    NULL, lpApplicationName, lpCommandLine,
                    NULL, NULL, FALSE, dwCreationFlags,
                    lpEnvironment, lpCurrentDirectory,
                    lpStartupInfo, lpProcessInformation, hNewToken);

    err = GetLastError();

    TlsData->proc_create_process_capture_image = FALSE;

    //
    // note that if NtCreateSection is not hooked (OpenIpcPath=*)
    // then the call to CreateProcessInternal does not go through
    // Proc_SectionCallback and we have nothing else to do here
    //

    if (ok || (err != ERROR_BAD_EXE_FORMAT))
        goto finish;

    if (! TlsData->proc_image_path) {

        ok = FALSE;
        err = ERROR_BAD_EXE_FORMAT;
        goto finish;
    }

    //
    // abort if trying to run the email program before properly configured
    //

    if (! Proc_CheckMailer(
                TlsData->proc_image_path, TlsData->proc_image_is_copy)) {

        ok = FALSE;
        err = ERROR_ACCESS_DENIED;
        goto finish;
    }

    //
    // on Vista, skip process creation if elevation is required
    // and go directly to UAC through SH32_DoRunAs
    //

    if (TlsData->proc_create_process == 1) {

        err = Sxs_CheckManifestForCreateProcess(TlsData->proc_image_path);
        if (err == ERROR_ELEVATION_REQUIRED)
            goto finish;
    }

    //
    // on XP, we need to make sure we have the right image name
    //

    Proc_QuoteCommandLine_XP(
                    TlsData, lpCommandLine, TlsData->proc_image_path);

    if (TlsData->proc_command_line)
        lpCommandLine = TlsData->proc_command_line;

    //
    // if this is one of our service process (e.g. SandboxieDcomLaunch)
    // which is running without system privileges and does not have the
    // SeAssignPrimaryTokenPrivilege, the CreateProcessAsUser call will
    // fail to replace the process token later when it calls
    // NtSetInformationProcess.  to work around this, we use SbieSvc
    // to start the new process
    //

    /*if (hToken && (! Dll_IsSystemSid)
               && (Dll_ProcessFlags & SBIE_FLAG_IMAGE_FROM_SBIE_DIR)) {

        ok = Proc_CreateProcessInternalW_3(
                hToken, lpCommandLine, lpCurrentDirectory, dwCreationFlags,
                lpStartupInfo, lpProcessInformation);
        err = GetLastError();
        goto finish;
    }*/

    //
    // don't let the caller specify an owner in the security descriptor
    // for the new process or thread objects, because the CreateProcess
    // call would fail with STATUS_INVALID_OWNER
    //

    SaveOwnerProcess = NULL;
    SaveOwnerThread = NULL;

    if (lpProcessAttributes) {
        SECURITY_DESCRIPTOR *sd = (SECURITY_DESCRIPTOR *)
                            lpProcessAttributes->lpSecurityDescriptor;
        if (sd) {

            if (sd->Control & SE_SELF_RELATIVE) {

                SaveOwnerProcess = (void *)(ULONG_PTR)
                    ((SECURITY_DESCRIPTOR_RELATIVE *)sd)->Owner;
                if (SaveOwnerProcess)
                    ((SECURITY_DESCRIPTOR_RELATIVE *)sd)->Owner = 0;

            } else {

                SaveOwnerProcess = sd->Owner;
                if (SaveOwnerProcess)
                    sd->Owner = NULL;
            }
        }
    }

    if (lpThreadAttributes) {
        SECURITY_DESCRIPTOR *sd = (SECURITY_DESCRIPTOR *)
                            lpThreadAttributes->lpSecurityDescriptor;
        if (sd) {

            if (sd->Control & SE_SELF_RELATIVE) {

                SaveOwnerThread = (void *)(ULONG_PTR)
                    ((SECURITY_DESCRIPTOR_RELATIVE *)sd)->Owner;
                if (SaveOwnerThread)
                    ((SECURITY_DESCRIPTOR_RELATIVE *)sd)->Owner = 0;

            } else {

                SaveOwnerThread = sd->Owner;
                if (SaveOwnerThread)
                    sd->Owner = NULL;
            }
        }
    }

    //
    // if caller did not specify lpApplicationName, and the image path
    // specifies a .bat or .cmd file, then we should not pass the
    // lpApplicationName parameter, because doing so would inhibit
    // correct quoting of the command line
    //

    if ((! lpApplicationName) && TlsData->proc_image_path) {

        WCHAR *dot = wcsrchr(TlsData->proc_image_path, L'.');
        if (dot) {
            ++dot;
            if (_wcsicmp(dot, L"bat") == 0 || _wcsicmp(dot, L"cmd") == 0) {

                if (TlsData->proc_image_is_copy) {

                    Proc_FixBatchCommandLine(
                        TlsData, lpCommandLine, TlsData->proc_image_path);

                    if (TlsData->proc_command_line)
                        lpCommandLine = TlsData->proc_command_line;
                }

                Dll_Free(TlsData->proc_image_path);
                TlsData->proc_image_path = NULL;
            }
        }
    }

    // OriginalToken BEGIN
    if (SbieApi_QueryConfBool(NULL, L"OriginalToken", FALSE))
    {
        ok = __sys_CreateProcessInternalW(
            hToken, lpApplicationName, lpCommandLine,
            lpProcessAttributes, lpThreadAttributes, bInheritHandles, dwCreationFlags,
            lpEnvironment, lpCurrentDirectory,
            lpStartupInfo, lpProcessInformation, hNewToken);

        err = GetLastError();

        goto finish;
    }
    // OriginalToken END

    //
    // create the new process
    //

    if (! (dwCreationFlags & CREATE_SUSPENDED))
        resume_thread = TRUE;
    dwCreationFlags |= CREATE_SUSPENDED;

    dwCreationFlags &= ~CREATE_BREAKAWAY_FROM_JOB;

    ok = __sys_CreateProcessInternalW(
                    NULL, TlsData->proc_image_path, lpCommandLine,
                    lpProcessAttributes, lpThreadAttributes, bInheritHandles,
                    dwCreationFlags, lpEnvironment, lpCurrentDirectory,
                    lpStartupInfo, lpProcessInformation, hNewToken);

    err = GetLastError();

    //
    // restore the original owner pointers in the security descriptors
    //

    if (SaveOwnerProcess) {
        SECURITY_DESCRIPTOR *sd = (SECURITY_DESCRIPTOR *)
                            lpProcessAttributes->lpSecurityDescriptor;
        if (sd->Control & SE_SELF_RELATIVE) {
            ((SECURITY_DESCRIPTOR_RELATIVE *)sd)->Owner =
                                    (ULONG)(ULONG_PTR)SaveOwnerProcess;
        } else
            sd->Owner = SaveOwnerProcess;
    }
    if (SaveOwnerThread) {
        SECURITY_DESCRIPTOR *sd = (SECURITY_DESCRIPTOR *)
                            lpThreadAttributes->lpSecurityDescriptor;
        if (sd->Control & SE_SELF_RELATIVE) {
            ((SECURITY_DESCRIPTOR_RELATIVE *)sd)->Owner =
                                    (ULONG)(ULONG_PTR)SaveOwnerThread;
        } else
            sd->Owner = SaveOwnerThread;
    }

    // For all versions of windows before Windows 10 RS5
    // since we are running with a highly restricted primary token,
    // Windows will not let us start the new process with any other token,
    // because it consults the primary process token, and that token is
    // too restricted to permit use of any other token.
    //
    // to work around this, we started the process with the suspended flag,
    // and we now ask our driver to pass a token to the new process.  this
    // will be the token parameter, if it was specified, otherwise it will
    // be our original unrestricted primary token
    //
    // see also Thread_SetInformationProcess_PrimaryToken
    // in core/drv/thread_token.c
    //
    // For windows 10 RS5 see Proc_CreateProcessInternalW_RS5   

    if (ok) {

        if (hToken) {

            //
            // if a token was specified, we need to impersonate it for
            // the special NtSetInformationProcess call
            //

            ok = Proc_CreateProcessInternalW_Impersonate(hToken);
            err = GetLastError();
        }

		// OriginalToken BEGIN
		if (!SbieApi_QueryConfBool(NULL, L"OriginalToken", FALSE))
		// OriginalToken END
        if (ok) {

            //
            // invoke the special NtSetInformationProcess call to copy either
            // the impersonation or the primary token into the new process
            //

            HANDLE NullToken[2] = { NULL, NULL };   // PROCESS_ACCESS_TOKEN
            NTSTATUS status = NtSetInformationProcess(
                lpProcessInformation->hProcess, ProcessAccessToken,
                NullToken, sizeof(NullToken));

            if (NT_SUCCESS(status)) {

                // Firefox audio issue -- 
                // We may enable below code for different processes if we see the similar issue
                // in different processes. Try our best to set the proper security descriptor.
                // Ignore the error for now. Firefox is still working fine without audio.
                //if (Dll_ImageType == DLL_IMAGE_MOZILLA_FIREFOX) {

                    SetTokenDefaultDaclToProcess(hToken, lpProcessInformation->hProcess);
                //}

            } else {
                ok = FALSE;
                err = RtlNtStatusToDosError(status);
            }
        }

        if (hToken) {

            //
            // cancel active impersonation if we activated it
            //

            Proc_CreateProcessInternalW_Impersonate(NULL);
        }

        if (ok) {

            //
            // record the last created process handle so we can skip any
            // call to NtSetInformationProcess with this handle, and
            // then resume the thread now that the token is properly set
            //

            Proc_LastCreatedProcessHandle = lpProcessInformation->hProcess;

            if (resume_thread)
            {
                // WerFault has some design flaws.  If we want crash DMPs we have to make adjustments
                if (lpApplicationName && (wcsstr(lpApplicationName, L"WerFault.exe")))
                {
                    // Windows will start WerFault 3 times.  So to prevent duplicate DMPs, filter them out here.
                    if (g_boolWasWerFaultLastProcess == TRUE)
                    {
                        TerminateProcess(lpProcessInformation->hProcess, 1);
                        WaitForSingleObject(lpProcessInformation->hProcess, 30000);
                        CloseHandle(lpProcessInformation->hProcess);
                        CloseHandle(lpProcessInformation->hThread);
                    }
                    else
                    {
                        ResumeThread(lpProcessInformation->hThread);
                        SbieApi_Log(2224, L"%S [%S]", Dll_ImageName, Dll_BoxName);
                        g_boolWasWerFaultLastProcess = TRUE;
                        // let WerFault run for a while to create its DMP before we let the crashing process exit.
                        WaitForSingleObject(lpProcessInformation->hProcess, 30000);
                    }
                }
                else
                    ResumeThread(lpProcessInformation->hThread);
            }

        } else {

            //
            // if the proper token cannot be set, terminate the new process
            //

            TerminateProcess(lpProcessInformation->hProcess, 1);
            CloseHandle(lpProcessInformation->hProcess);
            CloseHandle(lpProcessInformation->hThread);
        }
    }

    //
    // handle CreateProcessInternal returning ERROR_ELEVATION_REQUIRED
    //

finish:

    --TlsData->proc_create_process;

    if ((! ok) && (err == ERROR_ELEVATION_REQUIRED)) {

        BOOL cancelled = FALSE;
        if (SH32_DoRunAs(lpCommandLine, lpCurrentDirectory,
                         lpProcessInformation, &cancelled)) {
            err = 0;
            ok = TRUE;
        }
        if (cancelled)
            err = ERROR_CANCELLED;
    }

    /*if ((! ok) && (err == ERROR_ELEVATION_REQUIRED)) {

        insert env var SBIE_OVERRIDE_PARENT_PID
        run through SbieSvc
        hook NtQueryInformationProcess if env var SBIE_OVERRIDE_PARENT_PID exists
    }*/

    //
    // free work areas and return
    //

    Dll_Free(lpEnvironment);

    if (lpCurrentDirectory && lpCurrentDirectory != SaveCurrentDirectory)
        Dll_Free(lpCurrentDirectory);

    if (TlsData->proc_image_path) {
        Dll_Free(TlsData->proc_image_path);
        TlsData->proc_image_path = NULL;
    }
    TlsData->proc_image_is_copy = FALSE;

    if (TlsData->proc_command_line) {
        Dll_Free(TlsData->proc_command_line);
        TlsData->proc_command_line = NULL;
    }

    {
        WCHAR msg[1024];
        Sbie_snwprintf(msg, 1024, L"CreateProcess: %s (%s); err=%d", lpApplicationName ? lpApplicationName : L"[noName]", lpCommandLine ? lpCommandLine : L"[noCmd]", ok ? 0 : err);
        SbieApi_MonitorPut2(MONITOR_OTHER | MONITOR_TRACE, msg, FALSE);
    }

    SetLastError(err);
    return ok;
}


_FX BOOL Proc_UpdateProcThreadAttribute(
	_Inout_ LPPROC_THREAD_ATTRIBUTE_LIST lpAttributeList,
	_In_ DWORD dwFlags,
	_In_ DWORD_PTR Attribute,
	_In_reads_bytes_opt_(cbSize) PVOID lpValue,
	_In_ SIZE_T cbSize,
	_Out_writes_bytes_opt_(cbSize) PVOID lpPreviousValue,
	_In_opt_ PSIZE_T lpReturnSize)
{
	// fix for chreom 86+
	// when the PROC_THREAD_ATTRIBUTE_JOB_LIST is set the call CreateProcessAsUserW -> CreateProcessInternalW -> NtCreateProcess 
	// fals with an access denided error, so we need to block this attribute form being set
	// if(Dll_ImageType == DLL_IMAGE_GOOGLE_CHROME)
    if (Attribute == 0x0002000d) //PROC_THREAD_ATTRIBUTE_JOB_LIST
    {
        if (!SbieApi_QueryConfBool(NULL, L"NoAddProcessToJob", FALSE))
            return TRUE;
    }

	// some mitigation flags break SbieDll.dll Injection, so we disable them
	if (Attribute == 0x00020007) //PROC_THREAD_ATTRIBUTE_MITIGATION_POLICY
	{
		DWORD64* policy_value_1 = cbSize >= sizeof(DWORD64) ? lpValue : NULL;
		//DWORD64* policy_value_2 = cbSize >= sizeof(DWORD64) * 2 ? &((DWORD64*)lpValue)[1] : NULL;

		if (policy_value_1 != NULL)
		{
			*policy_value_1 &= ~(0x00000001ui64 << 44); // PROCESS_CREATION_MITIGATION_POLICY_BLOCK_NON_MICROSOFT_BINARIES_ALWAYS_ON;
			//*policy_value_1 |= (0x00000002ui64 << 44); // PROCESS_CREATION_MITIGATION_POLICY_BLOCK_NON_MICROSOFT_BINARIES_ALWAYS_OFF
		}
	}

	return __sys_UpdateProcThreadAttribute(lpAttributeList, dwFlags, Attribute, lpValue, cbSize, lpPreviousValue, lpReturnSize);
}


_FX BOOL Proc_SetProcessMitigationPolicy(
    _In_ PROCESS_MITIGATION_POLICY MitigationPolicy,
    _In_reads_bytes_(dwLength) PVOID lpBuffer,
    _In_ SIZE_T dwLength)
{
    // fix for SBIE2303 Could not hook ... (33, 1655)
    // This Mitigation Policy breaks our ability to hook functions once its enabled,
    // As we need to be able to hook them we prevent the activation of this policy.
    if (MitigationPolicy == ProcessDynamicCodePolicy)
        return TRUE;

    return __sys_SetProcessMitigationPolicy(MitigationPolicy, lpBuffer, dwLength);
}

void *Proc_GetImageFullPath(const WCHAR *lpApplicationName, const WCHAR *lpCommandLine)
{
    if ((lpApplicationName == NULL) && (lpCommandLine == NULL))
        return NULL;

    const WCHAR *start = NULL;
    int len = 0;

    if (lpApplicationName) {
        start = lpApplicationName;
        len = wcslen(start) + 1;    // add 1 for NULL
    }
    else {
        start = lpCommandLine;
        const WCHAR *end;

        // if command line is not quoted, look for 1st space
        if (*start != L'\"') {
            end = start;
            while (*end != 0 && *end != L' ')
                end++;
        }
        // else, look for end quote
        else {
            start++;
            end = start;
            while (*end != 0 && *end != L'\"')
                end++;
        }
        len = (int)(end - start) + 1;
    }

    WCHAR *mybuf = Dll_Alloc(len * sizeof(WCHAR));
    if (!mybuf) {
        return NULL;
    }

    memset(mybuf, 0xcd, len * 2);
    wcsncpy(mybuf, start, len - 1);
    mybuf[len - 1] = L'\0';

    return mybuf;
}

// Processes in Windows 10 RS5 will start with the Sandboxie restricted token.  
// Thus the expected failure of the original call to CreateProcessInternalW doesn't 
// happen.  Proc_CreateProcessInternalW_RS5 handles this case.  The main difference
// added to RS5 is the first call to CreateProcessInteralW need to be suspended so
// the special call to the sbieDrv to change the restricted primary token to the original
// token can happen properly.

// see also Thread_SetInformationProcess_PrimaryToken
// in core/drv/thread_token.c

_FX BOOL Proc_CreateProcessInternalW_RS5(
    HANDLE hToken,
    const WCHAR *lpApplicationName,
    WCHAR *lpCommandLine,
    LPSECURITY_ATTRIBUTES lpProcessAttributes,
    LPSECURITY_ATTRIBUTES lpThreadAttributes,
    BOOL bInheritHandles,
    ULONG dwCreationFlags,
    void *lpEnvironment,
    void *lpCurrentDirectory,
    LPSTARTUPINFOW lpStartupInfo,
    LPPROCESS_INFORMATION lpProcessInformation,
    HANDLE *hNewToken)
{
    THREAD_DATA *TlsData = Dll_GetTlsData(NULL);
    HANDLE FileHandle = INVALID_HANDLE_VALUE;
    void *SaveCurrentDirectory;
    ULONG err;
    BOOL ok;
    BOOL resume_thread = FALSE;
    WCHAR* lpAlteredCommandLine = NULL;

    Proc_LastCreatedProcessHandle = NULL;

    //
    // check if we block the process or launch it some other way
    //

    if (Proc_AlternateCreateProcess(
        lpApplicationName, lpCommandLine, lpCurrentDirectory,
        lpProcessInformation, &ok)) {

        return ok;
    }

    //
    // Electron based applications which work like chrome seam to fail with HW acceleration even when 
    // thay get the same treatment as chrome and chromium derivatives.
    // hack: by adding a parameter to the gpu renderer process we can fix the issue.
    //

    if (Dll_ImageType == DLL_IMAGE_UNSPECIFIED)
    {
        if(lpApplicationName && lpCommandLine)
        {
            WCHAR* backslash = wcsrchr(lpApplicationName, L'\\');
            if ((backslash && _wcsicmp(backslash + 1, Dll_ImageName) == 0)
                && wcsstr(lpCommandLine, L" --type=gpu-process")
                && !wcsstr(lpCommandLine, L" --use-gl=swiftshader-webgl")) {

                lpAlteredCommandLine = Dll_Alloc((wcslen(lpCommandLine) + 32 + 1) * sizeof(WCHAR));

                wcscpy(lpAlteredCommandLine, lpCommandLine);
                wcscat(lpAlteredCommandLine, L" --use-gl=swiftshader-webgl");

                lpCommandLine = lpAlteredCommandLine;
            }
        }
    }

    //
    // hack:  recent versions of Flash Player use the Chrome sandbox
    // architecture which conflicts with our restricted process model
    //

    if (Dll_ImageType == DLL_IMAGE_FLASH_PLAYER_SANDBOX ||
        Dll_ImageType == DLL_IMAGE_ACROBAT_READER ||
        Dll_ImageType == DLL_IMAGE_PLUGIN_CONTAINER)
        hToken = NULL;

    //
    // use a copy path for the current directory
    // if there is a copy directory in the sandbox
    //

    SaveCurrentDirectory = lpCurrentDirectory;

    lpCurrentDirectory = Proc_SelectCurrentDirectory(lpCurrentDirectory);

    if (!lpCurrentDirectory)
        lpCurrentDirectory = SaveCurrentDirectory;

    //
    // alter environment to pass some strings to the child process
    //

    lpEnvironment = File_AllocAndInitEnvironment(
        lpEnvironment,
        (dwCreationFlags & CREATE_UNICODE_ENVIRONMENT ? TRUE : FALSE),
        FALSE, NULL);

    dwCreationFlags |= CREATE_UNICODE_ENVIRONMENT;

    //Logic for windows 10 RS5
    ++TlsData->proc_create_process;
    WCHAR *mybuf = Proc_GetImageFullPath(lpApplicationName, lpCommandLine);
    if (mybuf == NULL)
        return FALSE;

    FileHandle = CreateFileW(mybuf, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);

    Dll_Free(mybuf);

    if (FileHandle != INVALID_HANDLE_VALUE) {
        Proc_StoreImagePath(TlsData, FileHandle);
        NtClose(FileHandle);
    }

    //
    // the system may have quoted the first part of the command line,
    // store this final command line
    //

    if (TlsData->proc_command_line)
        Dll_Free(TlsData->proc_command_line);

    ULONG len = 0;
    WCHAR *buf = NULL;

    if (lpApplicationName) {
        len = wcslen(lpApplicationName) + 2;        // +1 for space, +1 for NULL
    }

    if (lpCommandLine) {
        len += wcslen(lpCommandLine) + 1;           // +1 for NULL
    }

    buf = Dll_Alloc(len * sizeof(WCHAR));
    memset(buf, 0, len * sizeof(WCHAR));

    if (lpApplicationName) {
        wcscpy(buf, lpApplicationName);
        wcscat(buf, L" ");
    }

    if (lpCommandLine) {
        wcscat(buf, lpCommandLine);
    }

    TlsData->proc_command_line = buf;

    if ((!lpApplicationName) && TlsData->proc_image_path) {
        if (TlsData->proc_image_path) {
            lpApplicationName = TlsData->proc_image_path;
            WCHAR *dot = wcsrchr(TlsData->proc_image_path, L'.');
            if (dot) {
                ++dot;
                if (_wcsicmp(dot, L"bat") == 0 || _wcsicmp(dot, L"cmd") == 0 || _wcsicmp(dot,L"tmp") == 0) {

                    if (TlsData->proc_image_is_copy) {

                        Proc_FixBatchCommandLine(
                            TlsData, lpCommandLine, TlsData->proc_image_path);

                        if (TlsData->proc_command_line)
                            lpCommandLine = TlsData->proc_command_line;
                    }
                    Dll_Free(TlsData->proc_image_path);
                    TlsData->proc_image_path = NULL;
                    lpApplicationName = NULL;
                }
            }
        }
    }

    // OriginalToken BEGIN
    if (SbieApi_QueryConfBool(NULL, L"OriginalToken", FALSE))
    {
        ok = __sys_CreateProcessInternalW_RS5(
            hToken, lpApplicationName, lpCommandLine,
            lpProcessAttributes, lpThreadAttributes, bInheritHandles,
            dwCreationFlags, lpEnvironment, lpCurrentDirectory,
            lpStartupInfo, lpProcessInformation, hNewToken);

        err = GetLastError();

        goto finish;
    }
    // OriginalToken END

    if (!(dwCreationFlags & CREATE_SUSPENDED))
        resume_thread = TRUE;
    dwCreationFlags |= CREATE_SUSPENDED;

    dwCreationFlags &= ~CREATE_BREAKAWAY_FROM_JOB;

    if (TlsData->proc_image_path) {
        lpApplicationName = TlsData->proc_image_path;
    }

	if (Dll_OsBuild >= 17763) {
		// Fix-Me: this is a workaround for the MSI installer to work properly
		lpProcessAttributes = NULL;
	}

    ok = __sys_CreateProcessInternalW_RS5(
        NULL, lpApplicationName, lpCommandLine,
        lpProcessAttributes, lpThreadAttributes, bInheritHandles,
        dwCreationFlags, lpEnvironment, lpCurrentDirectory,
        lpStartupInfo, lpProcessInformation, hNewToken);

    err = GetLastError();

    if (ok) {

        if (hToken) {

            //
            // if a token was specified, we need to impersonate it for
            // the special NtSetInformationProcess call
            //

            ok = Proc_CreateProcessInternalW_Impersonate(hToken);
            err = GetLastError();
        }

		// OriginalToken BEGIN
		if (!SbieApi_QueryConfBool(NULL, L"OriginalToken", FALSE))
		// OriginalToken END
        if (ok) {

            //
            // invoke the special NtSetInformationProcess call to copy either
            // the impersonation or the primary token into the new process
            //

            HANDLE NullToken[2] = { NULL, NULL };   // PROCESS_ACCESS_TOKEN
            NTSTATUS status = NtSetInformationProcess(
                lpProcessInformation->hProcess, ProcessAccessToken,
                NullToken, sizeof(NullToken));

            if (NT_SUCCESS(status)) {

                // Firefox audio issue -- 
                // We may enable below code for different processes if we see the similar issue
                // in different processes. Try our best to set the proper security descriptor.
                // Ignore the error for now. Firefox is still working fine without audio.
                //if (Dll_ImageType == DLL_IMAGE_MOZILLA_FIREFOX) {

                SetTokenDefaultDaclToProcess(hToken, lpProcessInformation->hProcess);
                //}

            }
            else {
                ok = FALSE;
                err = RtlNtStatusToDosError(status);
            }
        }

        if (hToken) {

            //
            // cancel active impersonation if we activated it
            //

            Proc_CreateProcessInternalW_Impersonate(NULL);
        }

        if (ok) {

            //
            // record the last created process handle so we can skip any
            // call to NtSetInformationProcess with this handle, and
            // then resume the thread now that the token is properly set
            //

            Proc_LastCreatedProcessHandle = lpProcessInformation->hProcess;

            if (resume_thread)
            {
                // WerFault has some design flaws.  If we want crash DMPs we have to make adjustments
                if (lpApplicationName && (wcsstr(lpApplicationName, L"WerFault.exe")))
                {
                    // Windows will start WerFault 3 times.  So to prevent duplicate DMPs, filter them out here.
                    if (g_boolWasWerFaultLastProcess == TRUE)
                    {
                        TerminateProcess(lpProcessInformation->hProcess, 1);
                        WaitForSingleObject(lpProcessInformation->hProcess, 30000);
                        CloseHandle(lpProcessInformation->hProcess);
                        CloseHandle(lpProcessInformation->hThread);
                    }
                    else
                    {
                        ResumeThread(lpProcessInformation->hThread);
                        SbieApi_Log(2224, L"%S [%S]", Dll_ImageName, Dll_BoxName);
                        g_boolWasWerFaultLastProcess = TRUE;
                        // let WerFault run for a while to create its DMP before we let the crashing process exit.
                        WaitForSingleObject(lpProcessInformation->hProcess, 30000);
                    }
                }
                else
                    ResumeThread(lpProcessInformation->hThread);
            }

        }
        else {

            //
            // if the proper token cannot be set, terminate the new process
            //
            TerminateProcess(lpProcessInformation->hProcess, 1);
            CloseHandle(lpProcessInformation->hProcess);
            CloseHandle(lpProcessInformation->hThread);
        }
    }

    //
    // handle CreateProcessInternal returning ERROR_ELEVATION_REQUIRED
    //

finish:

    --TlsData->proc_create_process;

    if ((!ok) && (err == ERROR_ELEVATION_REQUIRED)) {

        BOOL cancelled = FALSE;
        if (SH32_DoRunAs(lpCommandLine, lpCurrentDirectory,
            lpProcessInformation, &cancelled)) {
            err = 0;
            ok = TRUE;
        }
        if (cancelled)
            err = ERROR_CANCELLED;
    }

    Dll_Free(lpEnvironment);

    if (lpCurrentDirectory && lpCurrentDirectory != SaveCurrentDirectory)
        Dll_Free(lpCurrentDirectory);

    if(lpAlteredCommandLine)
        Dll_Free(lpAlteredCommandLine);

    if (TlsData->proc_image_path) {
        Dll_Free(TlsData->proc_image_path);
        TlsData->proc_image_path = NULL;
    }
    TlsData->proc_image_is_copy = FALSE;

    if (TlsData->proc_command_line) {
        Dll_Free(TlsData->proc_command_line);
        TlsData->proc_command_line = NULL;
    }

    {
        WCHAR msg[1024];
        Sbie_snwprintf(msg, 1024, L"CreateProcess: %s (%s); err=%d", lpApplicationName ? lpApplicationName : L"[noName]", lpCommandLine ? lpCommandLine : L"[noCmd]", ok ? 0 : err);
        SbieApi_MonitorPut2(MONITOR_OTHER | MONITOR_TRACE, msg, FALSE);
    }

    SetLastError(err);
    return ok;
}


//---------------------------------------------------------------------------
// Proc_AlternateCreateProcess
//---------------------------------------------------------------------------


_FX BOOL Proc_AlternateCreateProcess(
    const WCHAR *lpApplicationName, WCHAR *lpCommandLine,
    void *lpCurrentDirectory, LPPROCESS_INFORMATION lpProcessInformation,
    BOOL *ReturnValue)
{
    if (SbieApi_QueryConfBool(NULL, L"BlockSoftwareUpdaters", TRUE))
    if (Proc_IsSoftwareUpdateW(lpApplicationName ? lpApplicationName : lpCommandLine)) {

        SetLastError(ERROR_ACCESS_DENIED);
        *ReturnValue = FALSE;

        SbieApi_MonitorPut(MONITOR_OTHER, L"Blocked start of an updater");
        return TRUE;        // exit CreateProcessInternal
    }

    if (Proc_IsSplWow64(lpApplicationName, lpCommandLine,
                        lpProcessInformation)) {

        return FALSE;       // continue with CreateProcessInternal
    }

    if (Proc_IsForcedElevation(lpCommandLine,
                               lpCurrentDirectory,
                               lpProcessInformation)) {

        *ReturnValue = ((GetLastError() == ERROR_CANCELLED) ? FALSE : TRUE);
        return TRUE;        // exit CreateProcessInternal
    }

    if (Dll_ImageType == DLL_IMAGE_SANDBOXIE_DCOMLAUNCH && lpApplicationName
            && wcsstr(lpApplicationName, L"klwtblfs.exe")) {
        // don't start Kaspersky Anti Virus klwtblfs.exe component
        // because Kaspersky protects the process and we can't put
        // it into a job or inject SbieLow and so on
        SbieApi_MonitorPut(MONITOR_OTHER, L"Blocked start of klwtblfs.exe");
        return TRUE;        // exit CreateProcessInternal
    }
    if (Dll_ImageType == DLL_IMAGE_SANDBOXIE_DCOMLAUNCH && lpCommandLine
        && wcsstr(lpCommandLine, L"smartscreen.exe")) {

        SbieApi_MonitorPut(MONITOR_OTHER, L"Blocked start of smartscreen.exe");
        return TRUE;        // exit CreateProcessInternal
    }
    return FALSE;           // continue with CreateProcessInternal
}


//---------------------------------------------------------------------------
// Proc_CreateProcessInternalW_Impersonate
//---------------------------------------------------------------------------


_FX BOOL Proc_CreateProcessInternalW_Impersonate(HANDLE TokenHandle)
{
    HANDLE NewTokenHandle;
    NTSTATUS status;

    if (TokenHandle) {

        OBJECT_ATTRIBUTES objattrs;
        SECURITY_QUALITY_OF_SERVICE QoS;

        InitializeObjectAttributes(&objattrs, NULL, 0, NULL, NULL);
        QoS.Length = sizeof(SECURITY_QUALITY_OF_SERVICE);
        QoS.ImpersonationLevel = SecurityImpersonation;
        QoS.ContextTrackingMode = SECURITY_STATIC_TRACKING;
        QoS.EffectiveOnly = FALSE;
        objattrs.SecurityQualityOfService = &QoS;

        status = NtDuplicateToken(
                    TokenHandle, TOKEN_IMPERSONATE | TOKEN_QUERY, &objattrs,
                    FALSE, TokenImpersonation, &NewTokenHandle);

    } else {

        NewTokenHandle = NULL;
        status = STATUS_SUCCESS;
    }

    if (NT_SUCCESS(status)) {

        status = NtSetInformationThread(
                    NtCurrentThread(), ThreadImpersonationToken,
                    &NewTokenHandle, sizeof(HANDLE));

        if (NewTokenHandle)
            NtClose(NewTokenHandle);
    }

    if (! NT_SUCCESS(status)) {
        SetLastError(ERROR_ACCESS_DENIED);
        return FALSE;
    }

    return TRUE;
}


//---------------------------------------------------------------------------
// Proc_ImpersonateSelf
//---------------------------------------------------------------------------


_FX BOOL Proc_ImpersonateSelf(BOOLEAN Enable)
{
    HANDLE ProcessToken;
    NTSTATUS status;
    BOOL ok = FALSE;

    if (Enable) {

        status = NtOpenProcessToken(
                    NtCurrentProcess(), TOKEN_ALL_ACCESS, &ProcessToken);
        if (NT_SUCCESS(status)) {

            ok = Proc_CreateProcessInternalW_Impersonate(ProcessToken);

            CloseHandle(ProcessToken);
        }

    } else {

        ok = Proc_CreateProcessInternalW_Impersonate(NULL);
    }

    return ok;
}


//---------------------------------------------------------------------------
// Proc_CreateProcessInternalW_3
//---------------------------------------------------------------------------


/*_FX BOOL Proc_CreateProcessInternalW_3(
    HANDLE TokenHandle, WCHAR *cmd, WCHAR *dir, ULONG creation_flags,
    LPSTARTUPINFOW StartupInfo, LPPROCESS_INFORMATION ProcessInformation)
{
    ULONG err;
    BOOL ok;
    BOOLEAN free_dir;

    if (! Proc_CreateProcessInternalW_Impersonate(TokenHandle))
        return FALSE;

    if (dir)
        free_dir = FALSE;
    else {
        dir = Dll_AllocTemp(sizeof(WCHAR) * 8192);
        dir[0] = L'\0';
        RtlGetCurrentDirectory_U(sizeof(WCHAR) * 8190, dir);
        dir[8190] = L'\0';
        free_dir = TRUE;
    }

    creation_flags &= ~CREATE_NEW_CONSOLE;

    ok = SbieDll_RunSandboxed(L"*THREAD*", cmd, dir, creation_flags,
                              StartupInfo, ProcessInformation);

    err = GetLastError();

    if (free_dir)
        Dll_Free(dir);

    Proc_CreateProcessInternalW_Impersonate(NULL);

    SetLastError(err);
    return ok;
}*/


//---------------------------------------------------------------------------
// Proc_SelectCurrentDirectory
//---------------------------------------------------------------------------


_FX WCHAR *Proc_SelectCurrentDirectory(const WCHAR *lpCurrentDirectory)
{
    HANDLE FileHandle;

    WCHAR *path = Dll_Alloc(sizeof(WCHAR) * 8192);

    if (! lpCurrentDirectory) {

        path[0] = L'\0';
        RtlGetCurrentDirectory_U(sizeof(WCHAR) * 8190, path);
        path[8190] = L'\0';

        lpCurrentDirectory = path;
    }

    FileHandle = CreateFileW(
                    lpCurrentDirectory, GENERIC_READ, FILE_SHARE_READ, NULL,
                    OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);

    if (FileHandle != INVALID_HANDLE_VALUE) {

        BOOLEAN IsBoxedPath;
        NTSTATUS status =
                    SbieDll_GetHandlePath(FileHandle, path, &IsBoxedPath);

        NtClose(FileHandle);

        if (NT_SUCCESS(status) || status == STATUS_BAD_INITIAL_PC) {
            if (IsBoxedPath && SbieDll_TranslateNtToDosPath(path)) {

                return path;
            }
        }
    }

    Dll_Free(path);
    return NULL;
}


//---------------------------------------------------------------------------
// Proc_QuoteCommandLine_XP
//---------------------------------------------------------------------------


_FX void Proc_QuoteCommandLine_XP(
    THREAD_DATA *TlsData, const WCHAR *CommandLine, const WCHAR *ImagePath)
{
#ifndef _WIN64

    ULONG  ImageNameLen;
    WCHAR *ImageName;
    WCHAR *CommandPtr, *CommandStart;

    //
    // the system CreateProcessInternal may quote the first argument on
    // the command line.  on Vista and later, we get the quoted string
    // through Proc_RtlCreateProcessParametersEx, but on Windows XP we
    // abort CreateProcessInternal during NtCreateSection, before it can
    // show us the quoted command line, so add the quotes here
    //

    if (Dll_OsBuild >= 6000)
        return;

    if (! CommandLine)
        return;

    if (*CommandLine == L'\"')
        return;

    ImageName = wcsrchr(ImagePath, L'\\');
    if (! ImageName)
        return;
    ++ImageName;
    ImageNameLen = wcslen(ImageName);
    if (! ImageNameLen)
        return;

    //
    // scan the command line for a match on just the image name
    //

    CommandPtr = (WCHAR *)CommandLine;
    while (*CommandPtr && iswspace(*CommandPtr))
        ++CommandPtr;
    CommandStart = CommandPtr;

    while (*CommandPtr) {

        if (_wcsnicmp(CommandPtr, ImageName, ImageNameLen) == 0 &&
                (   iswspace(CommandPtr[ImageNameLen])
                 || CommandPtr[ImageNameLen] == L'\0')) {

            ULONG len = wcslen(CommandStart) + 8;
            WCHAR *buf = Dll_Alloc(len * sizeof(WCHAR));
            *buf = L'\"';
            CommandPtr += ImageNameLen;
            len = CommandPtr - CommandStart;
            wmemcpy(buf + 1, CommandStart, len);
            buf[1 + len] = L'\"';
            buf[1 + len + 1] = L'\0';

            if (wcschr(buf, L' '))
                wcscpy(buf + 1 + len + 1, CommandPtr);
            else {
                // if the part we quoted doesn't contain any spaces
                // then don't use a modified command line
                Dll_Free(buf);
                buf = NULL;
            }

            if (TlsData->proc_command_line)
                Dll_Free(TlsData->proc_command_line);
            TlsData->proc_command_line = buf;
        }

        CommandPtr = wcschr(CommandPtr, L'\\');
        if (! CommandPtr)
            break;
        ++CommandPtr;
    }

#endif ! _WIN64
}


//---------------------------------------------------------------------------
// Proc_FixBatchCommandLine
//---------------------------------------------------------------------------


_FX void Proc_FixBatchCommandLine(
    THREAD_DATA *TlsData, const WCHAR *CommandLine, const WCHAR *ImagePath)
{
    WCHAR *ptr;
    WCHAR *cmd;
    ULONG len;

    //
    // when passing an lpCommandLine for a batch file, and not specifying
    // lpApplication, the system CreateProcessInternalW will try to create
    // a process using the batch file path, and will fail with "not found"
    // instead of "not exe file" if the batch file exists only in the
    // sandbox.  to prepare for this case, we adjust the command line if
    // the batch file is inside the sandbox.
    // see Proc_CreateProcessInternalW above.
    //

    if (! CommandLine)
        return;

    while (*CommandLine == L' ')
        ++CommandLine;

    if (*CommandLine == L'\"') {

        //
        // quoted:  "c:\dir\batch.bat" arguments
        //

        ptr = wcschr(CommandLine + 1, L'"');
        if (ptr) {

            len = wcslen(ImagePath) + wcslen(ptr) + 8;
            cmd = Dll_Alloc(len * sizeof(WCHAR));
            *cmd = L'\"';
            wcscpy(cmd + 1, ImagePath);
            wcscat(cmd, ptr);

            if (TlsData->proc_command_line)
                Dll_Free(TlsData->proc_command_line);
            TlsData->proc_command_line = cmd;

            return;
        }

    } else if (*CommandLine) {

        //
        // unquoted:  dir\batch.bat arguments
        //

        ptr = wcschr(CommandLine, L' ');
        if (! ptr)
            ptr = (WCHAR *)(CommandLine + wcslen(CommandLine));
        if (ptr && ptr > CommandLine) {

            len = wcslen(ImagePath) + wcslen(ptr) + 8;
            cmd = Dll_Alloc(len * sizeof(WCHAR));
            *cmd = L'\"';
            wcscpy(cmd + 1, ImagePath);
            wcscat(cmd, L"\"");
            wcscat(cmd, ptr);

            if (TlsData->proc_command_line)
                Dll_Free(TlsData->proc_command_line);
            TlsData->proc_command_line = cmd;

            return;
        }
    }

    SbieApi_Log(2205, L"Batch Command Line");
}


//---------------------------------------------------------------------------
// Proc_StoreImagePath
//---------------------------------------------------------------------------


_FX void Proc_StoreImagePath(THREAD_DATA *TlsData, HANDLE FileHandle)
{
    WCHAR *path = Dll_Alloc(sizeof(WCHAR) * 8192);

    NTSTATUS status = SbieDll_GetHandlePath(
                        FileHandle, path, &TlsData->proc_image_is_copy);

    if (NT_SUCCESS(status)) {
        if (! SbieDll_TranslateNtToDosPath(path))
            status = STATUS_UNSUCCESSFUL;
    }

    if (TlsData->proc_image_path)
        Dll_Free(TlsData->proc_image_path);

    if (NT_SUCCESS(status))
        TlsData->proc_image_path = path;
    else {
        TlsData->proc_image_path = NULL;
        TlsData->proc_image_is_copy = FALSE;
        Dll_Free(path);
    }
}


//---------------------------------------------------------------------------
// Proc_SectionCallback
//---------------------------------------------------------------------------


_FX NTSTATUS Proc_SectionCallback(HANDLE FileHandle)
{
    //
    // on Windows XP and earlier:  we determine the image path that
    // CreateProcessInternal uses its use of NtCreateSection, which
    // invokes this function (in Ipc_NtCreateSection)
    //
    // earlier versions used RtlCreateProcessParameters however that
    // function is invoked only after a new process object was created
    //

    if (Dll_OsBuild < 6000) {

        ULONG LastError;
        THREAD_DATA *TlsData = Dll_GetTlsData(&LastError);

        if (TlsData->proc_create_process_capture_image) {

            TlsData->proc_create_process_capture_image = FALSE;

            Proc_StoreImagePath(TlsData, FileHandle);

            SetLastError(LastError);
            return STATUS_BAD_INITIAL_PC;
        }
    }

    return STATUS_SUCCESS;
}


//---------------------------------------------------------------------------
// Proc_RtlCreateProcessParametersEx
//---------------------------------------------------------------------------


_FX NTSTATUS Proc_RtlCreateProcessParametersEx(
    void *ProcessParameters,
    UNICODE_STRING *ImagePathName,
    UNICODE_STRING *DllPath,
    UNICODE_STRING *CurrentDirectory,
    UNICODE_STRING *CommandLine,
    void *Environment,
    UNICODE_STRING *WindowTitle,
    UNICODE_STRING *DesktopInfo,
    UNICODE_STRING *ShellInfo,
    UNICODE_STRING *RuntimeData,
    void *UnknownParameter11)
{
    //
    // on Windows Vista and later we can extract the image path from
    // the invocation of RtlCreateProcessParametersEx.  it is invoked
    // before a new process object is created
    //

    NTSTATUS status;
    ULONG LastError;
    THREAD_DATA *TlsData = Dll_GetTlsData(&LastError);

    if (TlsData->proc_create_process_capture_image) {

        HANDLE FileHandle = CreateFileW(
            ImagePathName->Buffer, GENERIC_READ, FILE_SHARE_READ, NULL,
            OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);

        if (FileHandle != INVALID_HANDLE_VALUE) {

            Proc_StoreImagePath(TlsData, FileHandle);

            NtClose(FileHandle);
        }

        //
        // the system may have quoted the first part of the command line,
        // store this final command line
        //

        if (TlsData->proc_command_line)
            Dll_Free(TlsData->proc_command_line);

        if (CommandLine && CommandLine->Length) {
            ULONG len = CommandLine->Length / sizeof(WCHAR);
            WCHAR *buf = Dll_Alloc((len + 1) * sizeof(WCHAR));
            wmemcpy(buf, CommandLine->Buffer, len);
            buf[len] = L'\0';
            TlsData->proc_command_line = buf;

        } else
            TlsData->proc_command_line = NULL;

        //
        // abort CreateProcessInternalW
        //

        status = STATUS_BAD_INITIAL_PC;

    } else {

        //
        // the first invocation for each CreateProcessInternalW is used to
        // identify the image path, for other invocations we call the system
        //

        status = __sys_RtlCreateProcessParametersEx(
            ProcessParameters, ImagePathName, DllPath, CurrentDirectory,
            CommandLine, Environment, WindowTitle, DesktopInfo, ShellInfo,
            RuntimeData, UnknownParameter11);
    }

    SetLastError(LastError);
    return status;
}


//---------------------------------------------------------------------------
// Proc_CreateProcessWithTokenW
//---------------------------------------------------------------------------


_FX BOOL Proc_CreateProcessWithTokenW(
    HANDLE hToken,
    DWORD dwLogonFlags,
    LPVOID lpApplicationName,
    LPVOID lpCommandLine,
    DWORD dwCreationFlags,
    LPVOID lpEnvironment,
    LPVOID lpCurrentDirectory,
    LPSTARTUPINFOW lpStartupInfo,
    LPPROCESS_INFORMATION lpProcessInformation)
{
    STARTUPINFOW si;
    if (! lpStartupInfo) {
        memzero(&si, sizeof(STARTUPINFO));
        si.cb = sizeof(STARTUPINFO);
        lpStartupInfo = &si;
    }

    return Proc_CreateProcessInternalW(
        hToken, lpApplicationName, lpCommandLine, NULL, NULL, FALSE,
        dwCreationFlags, lpEnvironment, lpCurrentDirectory,
        lpStartupInfo, lpProcessInformation, NULL);
}


//---------------------------------------------------------------------------
// WinExec
//---------------------------------------------------------------------------


_FX UINT Proc_WinExec(LPCSTR lpCmdLine, UINT uCmdShow)
{
    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    BOOL ok;

    memzero(&si, sizeof(STARTUPINFO));
    si.cb = sizeof(STARTUPINFO);
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = (WORD)uCmdShow;

    memzero(&pi, sizeof(PROCESS_INFORMATION));

    ok = CreateProcessA(
        NULL, (char *)lpCmdLine, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);

    if (ok) {
        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);
        return (UINT)(ULONG_PTR)pi.hProcess;
    } else
        return ERROR_FILE_NOT_FOUND;
}


//---------------------------------------------------------------------------
// Proc_ExitProcess
//---------------------------------------------------------------------------


_FX void Proc_ExitProcess(UINT ExitCode)
{
    if (ExitCode != 0)
        SbieApi_Log(2108, Dll_ImageName);
    __sys_ExitProcess(0);
}


//---------------------------------------------------------------------------
// SbieDll_RunFromHome
//---------------------------------------------------------------------------


_FX BOOLEAN SbieDll_RunFromHome(
    const WCHAR *pgmName, const WCHAR *pgmArgs,
    STARTUPINFOW *si, PROCESS_INFORMATION *pi)
{
    ULONG len, i, err;
    WCHAR *path, *ptr;
    BOOL ok, inherit;
    ULONG creation_flags;

    //
    // get the full path to 'pgm' in the Sandboxie directory:
    // ""\path\to\pgmName" pgmArgs"
    //

    len = MAX_PATH * 2 + wcslen(pgmName);
    if (pgmArgs)
        len += wcslen(pgmArgs);
    path = Dll_AllocTemp(len * sizeof(WCHAR));

    ptr = wcsrchr(pgmName, L'.');
    if (ptr && _wcsicmp(ptr, L".exe") == 0) {
        path[0] = L'\"';
        i = 1;
    } else
        i = 0;

    if (Dll_BoxName) {
        SbieApi_GetHomePath(NULL, 0, &path[i], MAX_PATH);
        wcscat(path, L"\\");
    } else {
        GetModuleFileName(NULL, &path[i], MAX_PATH);
        ptr = wcsrchr(path, L'\\');
        if (ptr)
            ptr[1] = L'\0';
    }

    wcscat(path, pgmName);
    if (i) {
        if (pgmArgs) {
            wcscat(path, L"\" ");
            wcscat(path, pgmArgs);
        } else
            wcscat(path, L"\"");
    }

    //
    // if PROCESS_INFORMATION *pi was not specified, then just return
    // the command line to the caller
    //

    if (! pi) {

        len = (wcslen(path) + 1) * sizeof(WCHAR);
        si->lpReserved = HeapAlloc(GetProcessHeap(), 0, len);
        if (si->lpReserved) {
            memcpy(si->lpReserved, path, len);
            Dll_Free(path);
            return TRUE;
        } else {
            Dll_Free(path);
            return FALSE;
        }
    }

    //
    // run the command
    //

    if (si->lpReserved) {
        inherit = TRUE;
        si->lpReserved = NULL;
    } else
        inherit = FALSE;

    if (si->dwXCountChars == tzuk && si->dwYCountChars == tzuk) {
        creation_flags = ABOVE_NORMAL_PRIORITY_CLASS;
        si->dwXCountChars = si->dwYCountChars = 0;
    } else
        creation_flags = 0;

    memzero(pi, sizeof(PROCESS_INFORMATION));
    ok = CreateProcess(NULL, path, NULL, NULL, inherit, creation_flags,
                       NULL, NULL, si, pi);
    err = GetLastError();

    if ((! ok) && (err == ERROR_ACCESS_DISABLED_BY_POLICY) && Dll_BoxName) {

        //
        // disable SRP/AppLocker for the duration of this CreateProcess
        //

        AdvApi_EnableDisableSRP(FALSE);
        ok = CreateProcess(NULL, path, NULL, NULL, inherit, creation_flags,
                           NULL, NULL, si, pi);
        err = GetLastError();
        AdvApi_EnableDisableSRP(TRUE);
    }

    Dll_Free(path);

    if (! ok) {
        SbieApi_Log(2313, L"%S (%d)", pgmName, err);
        SetLastError(err);
        return FALSE;
    }

    return TRUE;
}


//---------------------------------------------------------------------------
// Proc_CheckMailer
//---------------------------------------------------------------------------


_FX BOOLEAN Proc_CheckMailer(const WCHAR *ImagePath, BOOLEAN IsBoxedPath)
{
    BOOLEAN ok;
    WCHAR *tmp;
    const WCHAR *imgName;
    ULONG imgType;

    BOOLEAN should_check_openfilepath = FALSE;

    if (IsBoxedPath)
        return TRUE;

    imgName = wcsrchr(ImagePath, L'\\');
    if (imgName)
        ++imgName;
    else
        imgName = ImagePath;

    imgType = Dll_GetImageType(imgName);

    //
    // check if image name matches a well-known email program
    //

    if (imgType == DLL_IMAGE_OFFICE_OUTLOOK     ||
        imgType == DLL_IMAGE_WINDOWS_LIVE_MAIL  ||
        imgType == DLL_IMAGE_OTHER_MAIL_CLIENT  ||
        0)
    {
        should_check_openfilepath = TRUE;
    }

    //
    // compare image name to default mail program
    //

    if (! should_check_openfilepath) {

        WCHAR *mail_pgm = SbieDll_AssocQueryProgram(L"mailto");

        if (mail_pgm) {

            WCHAR *pgmName;
            pgmName = wcsrchr(mail_pgm, L'\\');
            if (pgmName)
                ++pgmName;
            else
                pgmName = mail_pgm;

            if (_wcsicmp(imgName, pgmName) == 0) {

                should_check_openfilepath = TRUE;
            }

            Dll_Free(mail_pgm);
        }
    }

    //
    // ignore rundll32.exe, because Windows Live Mail sets
    // it as the default mail program.
    //
    // ignore opera.exe, because Opera may only be used for
    // browsing and not email
    //
    // ignore other common browsers
    //

    if (_wcsicmp(imgName, L"rundll32.exe") == 0  ||
        imgType == DLL_IMAGE_INTERNET_EXPLORER   ||
        imgType == DLL_IMAGE_MOZILLA_FIREFOX     ||
        imgType == DLL_IMAGE_GOOGLE_CHROME       ||
        imgType == DLL_IMAGE_OTHER_WEB_BROWSER   ||
        0) {

        should_check_openfilepath = FALSE;
    }

    //
    // image name seems to belong to some mail program.  check if
    // the image name is referenced by any OpenFilePath settings
    //

    if (! should_check_openfilepath) {

        ok = TRUE;

    } else {

        WCHAR value[504];
        int idx = 0;
        ok = FALSE;

        memzero(value, sizeof(value));

        while (! ok) {
            LONG rc = SbieApi_QueryConfAsIs(
                NULL, L"OpenFilePath", idx, value, 500 * sizeof(WCHAR));
            ++idx;
            if (rc == 0) {
                tmp = wcschr(value, L',');
                if (tmp) {
                    *tmp = L'\0';
                    if (_wcsicmp(imgName, value) == 0)
                        ok = TRUE;
                }
            } else if (rc != STATUS_BUFFER_TOO_SMALL)
                break;
        }

        while (! ok) {
            LONG rc = SbieApi_QueryConfAsIs(
                NULL, L"OpenPipePath", idx, value, 500 * sizeof(WCHAR));
            ++idx;
            if (rc == 0) {
                tmp = wcschr(value, L',');
                if (tmp) {
                    *tmp = L'\0';
                    if (_wcsicmp(imgName, value) == 0)
                        ok = TRUE;
                }
            } else if (rc != STATUS_BUFFER_TOO_SMALL)
                break;
        }
    }

    if (! ok)
        SbieApi_Log(2212, imgName);

    return ok;
}


//---------------------------------------------------------------------------
// Proc_IsSoftwareUpdateW
//---------------------------------------------------------------------------


_FX BOOLEAN Proc_IsSoftwareUpdateW(const WCHAR *path)
{
    WCHAR *MatchExe, *MatchDir, *SoftName;
    WCHAR *backslash;
    ULONG mp_flags;
    BOOLEAN IsUpdate;

    //
    // don't interfere with browser update sequence if the EXE is inside
    // the sandbox, or if the EXE matches an open path
    //

    if (! path)
        return FALSE;

    if (Ldr_BoxedImage)
        return FALSE;

    mp_flags = SbieDll_MatchPath(L'f', Ldr_ImageTruePath);
    if (PATH_IS_OPEN(mp_flags))
        return FALSE;

    //
    // check if this is a web browser process
    // which was not installed into the sandbox
    //

    if (Dll_ImageType == DLL_IMAGE_MOZILLA_FIREFOX) {

        MatchExe = L"updater.exe";
        MatchDir = L"\\mozilla firefox\\updates\\";
        SoftName = L"Mozilla Firefox";

    } else if (Dll_ImageType == DLL_IMAGE_GOOGLE_UPDATE) {

        if (! Proc_IsProcessRunning(L"chrome.exe"))
            return FALSE;

        MatchExe = L"chrome_installer.exe";
        MatchDir = L"\\google\\update\\";
        SoftName = L"Google Chrome";

    } else if (Dll_ImageType == DLL_IMAGE_SANDBOXIE_DCOMLAUNCH) {

        if (! Proc_IsProcessRunning(L"msedge.exe"))
            return FALSE;

        MatchExe = L"microsoftedgeupdatebroker.exe";
        MatchDir = L"\\microsoft\\edgeupdate";
        SoftName = L"Microsoft Edge";

    } else
        return FALSE;

    //
    // check if launching an update process
    //

    IsUpdate = FALSE;

    backslash = wcsrchr(path, L'\\');
    if (backslash && _wcsnicmp(backslash + 1, MatchExe, wcslen(MatchExe)) == 0) {

        ULONG len = wcslen(path) + 1;
        WCHAR *path2 = Dll_AllocTemp(len * sizeof(WCHAR));
        wmemcpy(path2, path, len);
        _wcslwr(path2);

        if (wcsstr(path2, MatchDir)) {

            IsUpdate = TRUE;
        }

        Dll_Free(path2);
    }

    //
    // issue message and return
    //

    if (IsUpdate) {

        SbieApi_Log(2191, SoftName);
        SbieApi_Log(2192, NULL);
        SbieApi_Log(2193, NULL);
    }

    return IsUpdate;
}


//---------------------------------------------------------------------------
// Proc_IsProcessRunning
//---------------------------------------------------------------------------


_FX BOOLEAN Proc_IsProcessRunning(const WCHAR *ImageToFind)
{
    ULONG *pids, i;
    BOOLEAN found = FALSE;

    pids = Dll_AllocTemp(sizeof(ULONG) * 512);
    SbieApi_EnumProcess(NULL, pids);

    for (i = 1; i <= pids[0]; ++i) {

        WCHAR image[128];
        HANDLE pids_i = (HANDLE) (ULONG_PTR) pids[i];
        SbieApi_QueryProcess(pids_i, NULL, image, NULL, NULL);
        if (_wcsicmp(image, ImageToFind) == 0) {

            found = TRUE;
            break;
        }
    }

    Dll_Free(pids);
    return found;
}


//---------------------------------------------------------------------------
// Proc_IsSplWow64
//---------------------------------------------------------------------------


_FX BOOLEAN Proc_IsSplWow64(
    const WCHAR *lpApplicationName, const WCHAR *lpCommandLine,
    LPPROCESS_INFORMATION lpProcessInformation)
{
    //
    // if this is WINSPOOL.DLL trying to launch splwow64.exe, then
    // we need to adjust the integrity level parameter on the command line

    WCHAR *ptr;
    if (! Dll_IsWow64)
        return FALSE;
    if (! lpApplicationName)
        return FALSE;
    ptr = wcsrchr(lpApplicationName, L'\\');
    if (! ptr)
        return FALSE;
    if (_wcsnicmp(ptr, L"\\splwow64.exe", 13) != 0)
        return FALSE;

    ptr = wcsrchr(lpCommandLine, L' ');
    if (! ptr)
        return FALSE;
    ptr[1] = L'0';
    ptr[2] = L'\0';

    return TRUE;
}


//---------------------------------------------------------------------------
// Proc_Init_AdvPack
//---------------------------------------------------------------------------


_FX BOOLEAN Proc_Init_AdvPack(HMODULE module)
{
    //
    // in Secure_Init, we are faking administrator status
    // for Internet Explorer tab process to prevent use of
    // the Protected Mode ActiveX Installation Broker
    // but this means that an ActiveX installation which involves running
    // an EXE file may not elevate correctly.
    // so we hook RunSetupCommand to force elevation.
    //

    extern BOOLEAN Secure_IsInternetExplorerTabProcess;

    if (module && Dll_OsBuild >= 6000
               && Secure_IsInternetExplorerTabProcess) {

        P_RunSetupCommandW RunSetupCommandW = (P_RunSetupCommandW)
            GetProcAddress(module, "RunSetupCommandW");
        if (RunSetupCommandW) {
            SBIEDLL_HOOK(Proc_,RunSetupCommandW);
        }
    }

    return TRUE;
}


//---------------------------------------------------------------------------
// Proc_RunSetupCommandW
//---------------------------------------------------------------------------


_FX HRESULT Proc_RunSetupCommandW(
    HWND hWnd,
    const WCHAR *szCmdName,
    const WCHAR *szInfSection,
    const WCHAR *szDir,
    const WCHAR *szTitle,
    HANDLE *phEXE,
    ULONG dwFlags,
    void *pvReserved)
{
    THREAD_DATA *TlsData = Dll_GetTlsData(NULL);
    HRESULT hr;
    TlsData->proc_create_process_force_elevate = TRUE;
    hr = __sys_RunSetupCommandW(
        hWnd, szCmdName, szInfSection, szDir, szTitle, phEXE, dwFlags,
        pvReserved);
    TlsData->proc_create_process_force_elevate = FALSE;
    return hr;
}


//---------------------------------------------------------------------------
// Proc_IsForcedElevation
//---------------------------------------------------------------------------


_FX BOOLEAN Proc_IsForcedElevation(
    const WCHAR *lpCommandLine,
    const WCHAR *lpCurrentDirectory,
    LPPROCESS_INFORMATION lpProcessInformation)
{
    THREAD_DATA *TlsData = Dll_GetTlsData(NULL);
    WCHAR *cmd, *dir;
    ULONG len;
    BOOL ok, cancelled;

    //
    // force elevation.  note that even when Drop Rights is in effect,
    // we still want to elevate in order to display a message about
    // this operation, and then return FALSE to start a normal process
    //

    if (! TlsData->proc_create_process_force_elevate)
        return FALSE;
    TlsData->proc_create_process_force_elevate = FALSE;

    len = (wcslen(lpCommandLine) + 1) * sizeof(WCHAR);
    cmd = Dll_AllocTemp(len);
    memcpy(cmd, lpCommandLine, len);
    len = (wcslen(lpCurrentDirectory) + 1) * sizeof(WCHAR);
    dir = Dll_AllocTemp(len);
    memcpy(dir, lpCurrentDirectory, len);

    cancelled = FALSE;
    ok = SH32_DoRunAs(cmd, dir, lpProcessInformation, &cancelled);

    Dll_Free(dir);
    Dll_Free(cmd);

    if (ok) {
        SetLastError(ERROR_SUCCESS);
        return TRUE;
    }

    if (cancelled && (Dll_ProcessFlags & SBIE_FLAG_DROP_RIGHTS))
        cancelled = FALSE;

    if (cancelled) {
        SetLastError(ERROR_CANCELLED);
        return TRUE;
    }

    return FALSE;
}


//---------------------------------------------------------------------------
// Proc_WaitForParentExit
//---------------------------------------------------------------------------


_FX ULONG Proc_WaitForParentExit(void *DoExitProcess)
{
    NTSTATUS status;
    ULONG len;
    PROCESS_BASIC_INFORMATION info;
    HANDLE hParentProcess;

    status = NtQueryInformationProcess(
        NtCurrentProcess(), ProcessBasicInformation,
        &info, sizeof(PROCESS_BASIC_INFORMATION), &len);

    if (! NT_SUCCESS(status))
        hParentProcess = NULL;
    else {
        hParentProcess = OpenProcess(
            SYNCHRONIZE, FALSE, (ULONG)info.InheritedFromUniqueProcessId);
    }
    if (! hParentProcess)
        hParentProcess = NtCurrentProcess();

    WaitForSingleObject(hParentProcess, INFINITE);

    if (DoExitProcess)
        ExitProcess(0);
    return 0;
}


//---------------------------------------------------------------------------
// Proc_GetRtlUserProcessParameters
//---------------------------------------------------------------------------


_FX RTL_USER_PROCESS_PARAMETERS *Proc_GetRtlUserProcessParameters(void)
{
    RTL_USER_PROCESS_PARAMETERS *ProcessParms =
        (RTL_USER_PROCESS_PARAMETERS *)
#ifdef _WIN64
            (*(ULONG_PTR *)(__readgsqword(0x60) + 0x20));
#else
            (*(ULONG_PTR *)(__readfsdword(0x30) + 0x10));
#endif _WIN64
    return ProcessParms;
}


//---------------------------------------------------------------------------
// Proc_NtSetInformationProcess
//---------------------------------------------------------------------------


_FX NTSTATUS Proc_NtSetInformationProcess(
    HANDLE ProcessHandle, ULONG InfoClass, void *Info, ULONG InfoLength)
{
    NTSTATUS status;

    //
    // CreateProcessAsUser involves a call to NtSetInformationProcess after
    // the new process was created, but we want to disregard such a call
    // because we already set the proper token in CreateProcessInternalW.
    //

    if (InfoClass == ProcessAccessToken
            && ProcessHandle == Proc_LastCreatedProcessHandle) {

        status = STATUS_SUCCESS;

    } else {

        status = __sys_NtSetInformationProcess(
                                ProcessHandle, InfoClass, Info, InfoLength);

        if ((InfoClass == ProcessAccessToken) && (! NT_SUCCESS(status))) {

            //SbieApi_Log(2205, L"SetInformationProcess");
        }
    }

    return status;
}

//---------------------------------------------------------------------------
// Proc_NtQueryInformationProcess
//---------------------------------------------------------------------------

_FX NTSTATUS Proc_NtQueryInformationProcess(
    HANDLE ProcessHandle, PROCESSINFOCLASS ProcessInformationClass, PVOID ProcessInformation, ULONG ProcessInformationLength, PULONG ReturnLength)
{
    NTSTATUS status;

    status = __sys_NtQueryInformationProcess(
        ProcessHandle, ProcessInformationClass, ProcessInformation, ProcessInformationLength, ReturnLength);

    if ((ProcessInformationClass == ProcessImageFileName) && (NT_SUCCESS(status))) {    // caller requesting image file name?
        if (NT_SUCCESS(SbieApi_QueryProcess(ProcessHandle, NULL, NULL, NULL, NULL)))    // is process sandboxed?
        {
            WCHAR *TruePath, *CopyPath;
            if (NT_SUCCESS(File_GetName(NULL, (UNICODE_STRING*)ProcessInformation, &TruePath, &CopyPath, NULL)))
                RtlInitUnicodeString((UNICODE_STRING*)ProcessInformation, TruePath);    // return non-sandboxed path so caller can't tell he's sandboxed.
        }
    }

    return status;
}


//---------------------------------------------------------------------------
// Proc_NtCreateProcessEx
//---------------------------------------------------------------------------


_FX NTSTATUS Proc_NtCreateProcessEx(
    HANDLE *ProcessHandle, ACCESS_MASK DesiredAccess,
    OBJECT_ATTRIBUTES *ObjectAttributes, HANDLE *ParentProcessHandle,
    BOOLEAN InheritHandles, HANDLE SectionHandle, HANDLE DebugPort,
    HANDLE ExceptionPort, ULONG  UnknownExtraParameter)
{
    //
    // Ntdll!PssNtCaptureSnapshot uses NtCreateProcessEx directly,
    // but we only support process creation through CreateProcessInternal
    //
    SbieApi_Log(2205, L"NtCreateProcessEx (%d)", Dll_ProcessId);
    return STATUS_ACCESS_DENIED;
}


//---------------------------------------------------------------------------
// Proc_RestartProcessOutOfPcaJob
//---------------------------------------------------------------------------


_FX void Proc_RestartProcessOutOfPcaJob(void)
{
    WCHAR *CommandLine;
    WCHAR *Directory;
    HANDLE FileHandle;
    STARTUPINFO StartupInfo;
    PROCESS_INFORMATION ProcessInformation;
    BOOL ok;

    //
    // if this is a forced process that was started by a parent process
    // which is inside a Program Compatibility Assistant (PCA) job, then
    // this current process is also in the same PCA job, and can't be
    // placed in the sandbox job by GuiServer::InitProcess in SbieSvc.
    //
    // this scenario is detected by Process_NotifyProcess_Create
    // in core/drv/process.c and the SBIE_FLAG_PROCESS_IN_PCA_JOB is set
    // which causes Dll_Ordinal1 to call us.
    //
    // we work around the PCA job problem by getting SbieSvc to start
    // a new instance of the current process, which would be outside
    // the PCA job, and therefore can be put into the sandbox job.
    //

    if (! Proc_ImpersonateSelf(TRUE))
        return;

    CommandLine = Dll_AllocTemp(sizeof(WCHAR) * 8192);
    wcscpy(CommandLine, GetCommandLine());

    Directory = Dll_AllocTemp(sizeof(WCHAR) * 8192);
    Directory[0] = L'\0';
    RtlGetCurrentDirectory_U(sizeof(WCHAR) * 8190, Directory);
    Directory[8190] = L'\0';

    FileHandle = CreateFile(
                    Directory, FILE_GENERIC_READ, FILE_SHARE_VALID_FLAGS,
                    NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);

    if (FileHandle != INVALID_HANDLE_VALUE) {

        //
        // if the directory exists in the sandbox, we want to specify
        // the path inside the sandbox, in case there is no corresponding
        // directory outside the sandbox
        //

        BOOLEAN is_copy;
        WCHAR *BoxedDirectory = Dll_AllocTemp(sizeof(WCHAR) * 8192);
        NTSTATUS status =
            SbieDll_GetHandlePath(FileHandle, BoxedDirectory, &is_copy);

        if (NT_SUCCESS(status) &&
                        SbieDll_TranslateNtToDosPath(BoxedDirectory)) {

            wcscpy(Directory, BoxedDirectory);
        }

        CloseHandle(FileHandle);
    }

    memzero(&StartupInfo, sizeof(STARTUPINFO));
    StartupInfo.cb = sizeof(STARTUPINFO);
    StartupInfo.dwFlags = STARTF_FORCEOFFFEEDBACK;
    memzero(&ProcessInformation, sizeof(PROCESS_INFORMATION));

    ok = SbieDll_RunSandboxed(L"*THREAD*", CommandLine, Directory, 0,
                              &StartupInfo, &ProcessInformation);

    if (ok) {
        if(Dll_OsBuild < 15025) {
            WaitForSingleObject(ProcessInformation.hProcess, INFINITE);
        }
        else {
            WaitForSingleObject(ProcessInformation.hProcess, 250);
        }
    }

    ExitProcess(0);
}
