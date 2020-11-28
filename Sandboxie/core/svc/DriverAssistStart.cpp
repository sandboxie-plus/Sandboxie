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
// Driver Assistant, start driver
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
// Imported Functions
//---------------------------------------------------------------------------


extern "C" {

NTSTATUS LsaConnectUntrusted(PHANDLE LsaHandle);

NTSTATUS LsaDeregisterLogonProcess(HANDLE LsaHandle);

NTSTATUS LsaLookupAuthenticationPackage(
  HANDLE LsaHandle, ANSI_STRING *PackageName, PULONG AuthenticationPackage);

}


//---------------------------------------------------------------------------
// StartDriverAsync
//---------------------------------------------------------------------------


ULONG DriverAssist::StartDriverAsync(void *arg)
{
    //
    // get windows version
    //

    OSVERSIONINFO osvi;
    memzero(&osvi, sizeof(osvi));
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    GetVersionEx(&osvi);

    //
    // start the driver, but only if it isn't already active
    //

    bool ok = false;

    WCHAR driver_version[16];
    LONG rc = SbieApi_GetVersion(driver_version);
    if (rc == 0) {
        ok = true;
        goto driver_started;
    }

    //
    // otherwise, try to start it
    //

    UNICODE_STRING uni;
    RtlInitUnicodeString(&uni,
        L"\\Registry\\Machine\\System\\CurrentControlSet"
        L"\\Services\\" SBIEDRV);

    rc = NtLoadDriver(&uni);
    if (rc == 0 || rc == STATUS_IMAGE_ALREADY_LOADED) {
        ok = true;
        goto driver_started;
    }

    if (rc != STATUS_PRIVILEGE_NOT_HELD || rc == STATUS_ACCESS_DENIED) {
        LogEvent(MSG_9234, 0x9153, rc);
        goto driver_started;
    }

    //
    // we have to enable a privilege to load the driver
    //

    WCHAR priv_space[64];
    TOKEN_PRIVILEGES *privs = (TOKEN_PRIVILEGES *)priv_space;
    HANDLE hToken;

    BOOL b = LookupPrivilegeValue(
                L"", SE_LOAD_DRIVER_NAME, &privs->Privileges[0].Luid);
    if (b) {

        privs->Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
        privs->PrivilegeCount = 1;

        b = OpenProcessToken(
                GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &hToken);
        if (b) {

            b = AdjustTokenPrivileges(hToken, FALSE, privs, 0, NULL, NULL);
            CloseHandle(hToken);
        }
    }

    rc = NtLoadDriver(&uni);
    if (rc == 0 || rc == STATUS_IMAGE_ALREADY_LOADED)
        ok = true;
    else
        LogEvent(MSG_9234, 0x9153, rc);

    //
    // the driver has been started (or was started already), check
    // version number before we continue the initialization
    //

driver_started:

    if (ok) {

        for (ULONG retries = 0; retries < 20; ++retries) {

            rc = SbieApi_GetVersion(driver_version);
            if (rc == 0)
                break;
            driver_version[0] = L'!';
            driver_version[1] = L'\0';
            Sleep(500);
        }

        WCHAR application_version[16];
        wsprintf(application_version, L"%S", MY_VERSION_COMPAT);

        if (wcscmp(application_version, driver_version) != 0) {
            LogEvent(MSG_9234, 0x9154, 0);
            ok = false;
        }
    }

    //
    // version numbers match, continue with driver/service init
    //

    if (ok) {
        rc = SbieApi_CallOne(
                API_SET_SERVICE_PORT, (ULONG_PTR)m_instance->m_PortHandle);
        if (rc != 0) {
            LogEvent(MSG_9234, 0x9361, rc);
            ok = false;
        }
    }

    if (ok) {
        SbieDll_InjectLow_InitSyscalls(TRUE);
        if (rc != 0) {
            LogEvent(MSG_9234, 0x9362, rc);
            ok = false;
        }
    }

    if (ok) {

        if (osvi.dwMajorVersion >= 6) {

            InitClipboard();
        }

        rc = SbieApi_CallZero(API_INIT_GUI);

        if (rc != 0) {
            LogEvent(MSG_9234, 0x9156, rc);
            ok = false;
        }
    }

#ifndef _WIN64

    if (ok) {

        //
        // prior to Windows Vista, we need to query the number for
        // the MSV10 authentication package, and tell our driver
        //

        if (osvi.dwMajorVersion == 5) {

            HANDLE LsaHandle;
            ANSI_STRING AuthPkgName;
            ULONG AuthPkgNum;

            rc = LsaConnectUntrusted(&LsaHandle);
            if (rc == 0) {

                RtlInitString(&AuthPkgName,
                    (const UCHAR *)"MICROSOFT_AUTHENTICATION_PACKAGE_V1_0");
                rc = LsaLookupAuthenticationPackage(
                    LsaHandle, &AuthPkgName, &AuthPkgNum);

                if (rc == 0)
                    SbieApi_CallOne(API_SET_LSA_AUTH_PKG, AuthPkgNum);

                LsaDeregisterLogonProcess(LsaHandle);
            }
        }
    }


#endif ! _WIN64

    if (ok) {

        //
        // trigger manual invocation of LogMessage to collect any
        // messages that were logged while the driver was starting
        //

        m_instance->LogMessage();

        m_instance->m_DriverReady = true;
    }

    if (! ok) {

        AbortServer();
    }

    return 0;
}


//---------------------------------------------------------------------------
// IsDriverReady
//---------------------------------------------------------------------------


bool DriverAssist::IsDriverReady()
{
    if (m_instance && m_instance->m_DriverReady)
        return true;
    else
        return false;
}


//---------------------------------------------------------------------------
// InitClipboard
//---------------------------------------------------------------------------


void DriverAssist::InitClipboard()
{
    //
    // on Windows Vista and later, we need to figure out the
    // structure of the internal clipboard item.  we put some
    // data on the clipboard, and let Gui_InitClipboard in file
    // core/drv/gui.c figure out the internal structure
    //

    HANDLE hGlobal1 = GlobalAlloc(GMEM_MOVEABLE, 8 * sizeof(WCHAR));
    HANDLE hGlobal2 = GlobalAlloc(GMEM_MOVEABLE, 8 * sizeof(WCHAR));

    if (hGlobal1 && hGlobal2) {

        WCHAR *pGlobal = (WCHAR *)GlobalLock(hGlobal1);
        *pGlobal = L'\0';
        GlobalUnlock(hGlobal1);
        pGlobal = (WCHAR *)GlobalLock(hGlobal2);
        *pGlobal = L'\0';
        GlobalUnlock(hGlobal2);

        for (int retry = 0; retry < 8 * (1000 / 250); ++retry) {

            if (OpenClipboard(NULL)) {

                EmptyClipboard();
                SetClipboardData(0x111111, hGlobal1);
                SetClipboardData(0x222222, hGlobal1);
                SetClipboardData(0x333333, hGlobal2);
                SetClipboardData(0x444444, hGlobal2);

                SbieApi_CallOne(API_GUI_CLIPBOARD, -1);

                EmptyClipboard();
                CloseClipboard();

                break;

            } else
                Sleep(250);
        }
    }

    if (hGlobal1)
        GlobalFree(hGlobal1);

    if (hGlobal2)
        GlobalFree(hGlobal2);
}
