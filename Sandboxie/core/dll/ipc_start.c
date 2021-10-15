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
// IPC (Start Server)
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------


static BOOLEAN Ipc_StartServer(const WCHAR *TruePath, BOOLEAN Async);

static DWORD Ipc_StartServer_Thread(const WCHAR *TruePath);


//---------------------------------------------------------------------------
// Ipc_GetServerEvent
//---------------------------------------------------------------------------


_FX HANDLE Ipc_GetServerEvent(const WCHAR *service, BOOLEAN *create_flag)
{
    WCHAR event_name[64];
    HANDLE hEvent;

    Sbie_snwprintf(event_name, 64, SBIE_BOXED_ L"ServiceInitComplete_%s", service);
    if (create_flag) {
        *create_flag = FALSE;
        hEvent = CreateEvent(NULL, TRUE, FALSE, event_name);
        if (hEvent && GetLastError() == 0)
            *create_flag = TRUE;
    } else
        hEvent = OpenEvent(EVENT_ALL_ACCESS, FALSE, event_name);
    return hEvent;
}


//---------------------------------------------------------------------------
// Ipc_StartServer
//---------------------------------------------------------------------------


_FX BOOLEAN Ipc_StartServer(const WCHAR *TruePath, BOOLEAN Async)
{
    static const WCHAR *_format    = L"%S (%d)";
    static const WCHAR *_formatHex = L"%S (%08X)";
    static const WCHAR *_rpcss = L"RpcSs";
    static const WCHAR *_dcomlaunch = L"DcomLaunch";
    const WCHAR *service;
    HANDLE hServerEvent, hServerProcess;
    BOOLEAN event_created = FALSE;
    BOOLEAN bRet = TRUE;

    //
    // in async mode, several args are passed through the TruePath pointer
    //

    if (Async == '*') {

        ULONG_PTR *args = (ULONG_PTR *)TruePath;
        TruePath = (const WCHAR *)args[0];
        service = (const WCHAR *)args[1];
        hServerEvent = (HANDLE)args[2];
        hServerProcess = (HANDLE)args[3];
        Dll_Free(args);

        Async = FALSE;

    } else {

    //
    // -    SandboxieDcomLaunch is started when SandboxieRpcSs asks
    //      for the 'actkernel' port.
    //
    // -    SandboxieRpcSs is started when any process that is not
    //      SandboxieDcomLaunch asks for the 'epmapper' port
    //

        if (Dll_ImageType == DLL_IMAGE_SANDBOXIE_RPCSS) {

            if (_wcsicmp(TruePath, Ipc_actkernel) == 0)
                service = _dcomlaunch;
            else
                return FALSE;

        } else if (Dll_ImageType != DLL_IMAGE_SANDBOXIE_DCOMLAUNCH) {

            if (_wcsicmp(TruePath, Ipc_epmapper) == 0)
                service = _rpcss;
            else
                return FALSE;

        } else
            return FALSE;

        //
        // open server event
        //

        hServerEvent = Ipc_GetServerEvent(service, &event_created);
        if (! hServerEvent)
            return FALSE;

        if (! event_created)
            hServerProcess = NULL;
        else {

            //
            // launch the process
            //
            // note that we launch the process from the calling thread even
            // when called in async mode.  this is because CreateProcess
            // is a complicated call that can end up attempting to acquire
            // the loader lock.  while at the same time, some other thread
            // may already have taken the loader lock, and might be waiting
            // for SandboxieRpcSs to start
            //
            // v4 note:  CreateProcess uses LoadLibrary and because we are
            // invoked during the DLL init phase, this may cause problems
            // with DLL initialization.  this is in addition to the problem
            // described in the paragraph above, and other problems such as
            // an SRP or AppLocker rule preventing us from starting the
            // SandboxieRpcSs.  requesting SbieSvc ProcessServevr to run
            // SandboxieRpcSs for us can fix all of these problems.
            //

            ULONG errnum;
            STARTUPINFO si;
            PROCESS_INFORMATION pi;

            memzero(&si, sizeof(STARTUPINFO));
            si.cb = sizeof(STARTUPINFO);
            si.dwFlags = STARTF_FORCEOFFFEEDBACK;
            si.dwXCountChars = si.dwYCountChars = tzuk;

            WCHAR program[64];
            wcscpy(program, SANDBOXIE);
            wcscat(program, service);
            wcscat(program, L".exe");

            if (service == _rpcss) {

                //
                // starting rpcss: go thrugh SbieSvc ProcessServer
                //

                WCHAR homedir[MAX_PATH];
                SbieApi_GetHomePath(NULL, 0, homedir, MAX_PATH);

                if (! Proc_ImpersonateSelf(TRUE))
                    errnum = ERROR_NO_IMPERSONATION_TOKEN;
                else {

                    //WCHAR *fullpath = Dll_AllocTemp(512 * sizeof(WCHAR));
                    //Sbie_snwprintf(fullpath, 512, L"\"%s\\%s\"", homedir, program);

                    //
					// Note: ServiceServer::CanAccessSCM has a special case to permit DcomLaunch to start services without being system
					//

                    //if (! SbieDll_RunSandboxed(
                    //        L"*THREAD*", fullpath, homedir, 0, &si, &pi))
                    if (! SbieDll_RunSandboxed(
                              L"", L"*RPCSS*", homedir, 0, &si, &pi))
                        errnum = GetLastError();
                    else
                        errnum = -1;
                    Proc_ImpersonateSelf(FALSE);

                    //Dll_Free(fullpath);
                }

            } else {

                //
                // rpcss starting dcomlaunch:  normal CreateProcess
                // (SandboxieRpcSs has to be parent of SandboxieDcomLaunch)
                //

                if (! SbieDll_RunFromHome(program, NULL, &si, &pi))
                    errnum = GetLastError();
                else
                    errnum = -1;
            }

            if (errnum != -1) {

                SbieApi_Log(2204, _format, service, errnum);

                CloseHandle(hServerEvent);

                return FALSE;
            }

            hServerProcess = pi.hProcess;
            CloseHandle(pi.hThread);
        }
    }

    //
    // if async mode requested, restart ourselves in a separate thread
    //

    if (Async) {

        //
        // if async mode,
        //

        ULONG_PTR *args;
        ULONG idThread;
        HANDLE hThread;

        args = (ULONG_PTR *)Dll_AllocTemp(sizeof(ULONG_PTR) * 4);
        args[0] = (ULONG_PTR)TruePath;
        args[1] = (ULONG_PTR)service;
        args[2] = (ULONG_PTR)hServerEvent;
        args[3] = (ULONG_PTR)hServerProcess;

        hThread = CreateThread(
                    NULL, 0, Ipc_StartServer_Thread,
                    (void *)args, CREATE_SUSPENDED, &idThread);
        if (hThread) {

            SetThreadPriority(hThread, THREAD_PRIORITY_BELOW_NORMAL);
            ResumeThread(hThread);
            CloseHandle(hThread);
        }

    } else {

        //
        // continued processing in the main line or the separate thread
        //
        // here we wait for the server process to signal the event
        //

        ULONG NumWaitHandles = (hServerProcess) ? 2 : 1;
        HANDLE WaitHandles[2];
        WaitHandles[0] = hServerEvent;
        WaitHandles[1] = hServerProcess;

        while (1) {

            ULONG rc = WaitForMultipleObjects(
                NumWaitHandles, WaitHandles, FALSE, 30 * 1000);

            if (rc == WAIT_OBJECT_0)
                break;

            if (rc == (WAIT_OBJECT_0 + 1)) {

                if (GetExitCodeProcess(hServerProcess, &rc)
                        && rc != 0 && rc != STILL_ACTIVE) {

                    if (rc == STATUS_LICENSE_QUOTA_EXCEEDED) {

                        // duplicate copy of SandboxieRpcSs.exe,
                        // so don't indicate any error.
                        // see also apps/com/rpcss/rpcss.c

                    } else {

                        const WCHAR *FormatToUse =
                            (rc & 0x80000000) ? _formatHex : _format;
                        SbieApi_Log(2204, FormatToUse, service, rc, rc);
                    }

                } else
                    SbieApi_Log(2204, _format, service, -3);

                break;
            }

            SbieApi_Log(2204, _format, service, -1);
        }

        //
        // close handles
        //

        CloseHandle(hServerEvent);

        //
        // special case:  if we were waiting for the rpcss server to load,
        // we should now also try to wait for the dcomlaunch server to load.
        // this is because rpcss signals its event _before_ starting the
        // dcomlaunch server.
        //
        // exception to special case:  (1) if running with 'open COM', then
        // SandboxieDcomLaunch does not load.  (2) if the DcomLaunch service
        // is not available (i.e. Windows pre XP SP 2)
        //

        if (service == _rpcss && (! SbieDll_IsOpenCOM())) {

            HANDLE hKey = Scm_OpenKeyForService(_dcomlaunch, FALSE);
            if (hKey) {

                HANDLE hEvent = NULL;
                NtClose(hKey);

                while (1) {

                    if (! hEvent) {
                        
                        hEvent = Ipc_GetServerEvent(_dcomlaunch, NULL);

                        if (!hEvent) {

                            ULONG rc = 0;

                            // hServerProcess should stay running. If hServerProcess exits, probably a crash,
                            // we have no chance to open the ServiceInitComplete event. Break to loop.
                            if (GetExitCodeProcess(hServerProcess, &rc)
                                && rc != 0 && rc != STILL_ACTIVE) {

                                SbieApi_Log(2204, _format, _dcomlaunch, -4);
                                bRet = FALSE;
                                break;
                            }

                            Sleep(50);
                            continue;
                        }
                    }

                    if (WAIT_OBJECT_0 == WaitForSingleObject(
                                                hEvent, 30 * 1000))
                        break;

                    SbieApi_Log(2204, _format, _dcomlaunch, -2);
                }

                if (hEvent)
                    CloseHandle(hEvent);
            }
        }

        if (hServerProcess)
            CloseHandle(hServerProcess);
    }

    //
    // finish
    //

    return bRet;
}


//---------------------------------------------------------------------------
// Ipc_StartServer_Thread
//---------------------------------------------------------------------------


_FX DWORD Ipc_StartServer_Thread(const WCHAR *TruePath)
{
    Ipc_StartServer(TruePath, '*');

    return 0;
}


//---------------------------------------------------------------------------
// SbieDll_StartCOM
//---------------------------------------------------------------------------


_FX BOOLEAN SbieDll_StartCOM(BOOLEAN Async)
{
    if (Dll_ImageType == DLL_IMAGE_SANDBOXIE_RPCSS ||
        Dll_ImageType == DLL_IMAGE_SANDBOXIE_DCOMLAUNCH ||
        Dll_RestrictedToken) {

        return TRUE;
    }

    if (Async) {

        SYSTEM_INFO sysinfo;
        GetSystemInfo(&sysinfo);
        if (sysinfo.dwNumberOfProcessors < 2)
            Async = FALSE;
    }

    return Ipc_StartServer(Ipc_epmapper, Async);
}


//---------------------------------------------------------------------------
// SbieDll_IsOpenCOM
//---------------------------------------------------------------------------


_FX BOOLEAN SbieDll_IsOpenCOM(void)
{
    static BOOLEAN init_flag = FALSE;
    static BOOLEAN open_flag = FALSE;

    if (! init_flag) {
        ULONG mp_flags = SbieDll_MatchPath(L'i', Ipc_epmapper);
        if (PATH_IS_OPEN(mp_flags))
            open_flag = TRUE;
        init_flag = TRUE;
    }

    return open_flag;
}
