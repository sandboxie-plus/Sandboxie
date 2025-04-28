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

#include "core/drv/verify.h"

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
    DriverAssist* This = (DriverAssist*)arg;

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

    LONG rc = SbieApi_GetVersionEx(NULL, NULL);
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
        ULONG drv_abi_ver = 0;

        for (ULONG retries = 0; retries < 20; ++retries) {

            rc = SbieApi_GetVersionEx(NULL, &drv_abi_ver);
            if (rc == 0)
                break;
            Sleep(500);
        }

        if (drv_abi_ver != MY_ABI_VERSION) {
            LogEvent(MSG_9234, 0x9154, 0);
            ok = false;
        }
    }

    //
    // version numbers match
    //

    if (ok) {
        
        const char BlockList0[] =
            // Stolen:
            "2687F3F7D9DBF317A05251E2ED3C3D0A" "\r\n"
            // Refund:
            "6AF28A3722ADC9FDB0F4B298508DD9E6" "\r\n" // C
            "54F2DF068889C1789AE9D7E948618EB6" "\r\n" // C
            "665FBAD025408A5B9355230EBD3381DC" "\r\n" // C
            "622D831B13B70B7CFFEC12E3778BF740" "\r\n"
            "2FDEB3584ED4DA007C2A1D49CFFF1062" "\r\n"
            "413616148FA9D3793B0E9BA4A396D3EE" "\r\n"
            "CC4DCCD36A13097B4478ADEB0FEF00CD" "\r\n"
            "32DE2C3B8E8859B6ECB6FF98BDF8DB15" "\r\n"
            "A99F919ECD99AB7664CC7C136BDD4CA8" "\r\n"
            "0CC1B62B7734DEFB4556F9E6E82AAABA" "\r\n"
            "16B862ACAAA7DF4B79BEDCB4F3323450" "\r\n"
            "CAD7290FD6068ADD1C5BF550D6EB51D1" "\r\n"
            "D1C2507AB1C5CF34D04109620E28B69C" "\r\n"
            "2FAA79DFDDB7DE7972898E895EC4B9A9" "\r\n"
            "B260D52B07E41055E0F5D8E2CBAE973A" "\r\n"
            "48A9426DDADC854DE655021E13397E00" "\r\n"
            "C6009AFDC9F1626290D252A151E253D6" "\r\n"
            "778D8878F8906D32185A9FEF1A057209" "\r\n"
            "622D831B13B7DB7CFFEC12E3778BF740" "\r\n"
            "BA29C3EBED4944B0ACD962E3CF7289FF" "\r\n"
            "2F8B2F5002FF0FB593B2F054559CEE2A" "\r\n"
            "EA0159390311DCB79234A28104CD10D5" "\r\n" // SC
            // Piracy:
            "E0787E0A1E4160B33DF2A81E947189BA" "\r\n"
            "7494411B703814B40F9669F2B459CA56" "\r\n"
            "56F7D98FAF57E0AAB5F33256CCFA43F6" "\r\n"
            "01962DAC53357EA1E544314F332A2173" "\r\n"
            "C527B299EEE007202E7C54DC4BE337C7" "\r\n"
            "ED106515928FD0F23070A6A15B596B3D" "\r\n"
            "3FB86F9A50A5121AFED46866B525D1D3" "\r\n"
            "795C156C25AFC3DDC1CDD61DD4BF10B2" "\r\n"
            "0E56E599CC52C5A41FF43A453602B4C5" "\r\n"
            "828746BE2A77DABE73707DB9FE31595E" "\r\n"
            "4693FCE670EC8193E123787EC9934B21" "\r\n"
            "024EFE6509072ED4427C11ADBAB2BF11" "\r\n"
            "C7E9E20B48659CBAB562C0280410EEA6" "\r\n"
            "6F4B4D8F3F7F9E7B6B0A7A7AA6288AAF" "\r\n"
            "99834840B8476B46D8364582DF1A72EF" "\r\n"
            "D4C0AA55A0574059A2D8E10D03479BD0" "\r\n"
            "5F208DC10A34AD293E96194C7104F9EF" "\r\n"
            "3F3525CBF19D355D100541E600D755A8" "\r\n"
            "3875E5493F7A96D880E3CF07EA42AEEA" "\r\n"
            "D6946F31731C62BD71CECF3AD152B070" "\r\n"
            "F085F5CFC803F0F160971FB0D6C94ADC" "\r\n"
            "C7F7C603E7C129995087CC601CB19E30" "\r\n"
            "E00142521EB0B70BE6E70D23A61F9482" "\r\n"
            "0E3463B22B04114C6E21566A7F50EC07" "\r\n"
            "B46333E52DAF34E45457284091B97F29" "\r\n"
            "AA8C57A01358CD9CBFA4A7D9CD5013F1" "\r\n"
            "351B38D2120082D281BA7816B40E5898" "\r\n"
            "F1F14450F704487A07E4807C8620C93B" "\r\n"
            "6F9BA3587C983C8D6CDACCE30A4D352E" "\r\n"
            "93F45633ABAAD29FBBB05F6DA5C2E831" "\r\n"
            "5786CEBBA5F24E8440621EEA43D12A13" "\r\n"
            "02C3E639EC6DC4CAE00C9F4AE9F5CE3A" "\r\n"
            "680BAA9FC86EB5C5739E19971327B2C7" "\r\n"
            "3A3B17EFC1103995B23E23A61B6B4978" "\r\n"
            "B087A34DF4784ACBF7317B38B5224097" "\r\n"
            "9AB48E19A1ABF37886F144469B13196A" "\r\n"
            "141630B9AEAC4808E8B279662DB6E405" "\r\n"
            "BFDFBC3C26EA5CD5BB71C9ED009C0ED9" "\r\n"
            "6D89C689563A3938B15D308F8CD63A3A" "\r\n"
            "C9E6EEFB07D490BBD1904A75B3600885" "\r\n"
            "ACC546A79AEECF8AEDBEDF59F11EAEF3" "\r\n"
            "C0674B83CFF857A696AAA06E4102D85A" "\r\n"
            "BF513D3769F3398BE05E66DA2AFCCAFB" "\r\n"
            "108006B5EE087653E7208775C3EFC107" "\r\n"
            "9462D20AB47FD1765B99DBBB362E729B" "\r\n"
            "39F3EA1EE1875272964E28F7D1ECEDAD" "\r\n"
            "F2A021BC4B3D88C9694BCC330F8B08E6" "\r\n"
            "F41DB2D0D47DF290693B066575B792CD" "\r\n"
            "13828E6C1BBFF06CCFA177D17EC33B6D" "\r\n"
            "1BCA37EEF8D433B21FD9FCCA13E72345" "\r\n"
            "958D0F7EC38BDC8C735235B7285685BB" "\r\n"
            "57E273C21CAB40E996E43A5EAB0B66FD" "\r\n"
            "402D20066C7812EE31FF658DA9AE1DC0" "\r\n"
            "016BCE2A20762AAA45CA3192B19EC5C7" "\r\n"
            "9F66514482707499B1D253B2C4105964" "\r\n"
            "0FE9EF95C42175DE64F5C22864144CDF" "\r\n"
            "39ECB66BDC97AFEDD138380BEF5A3424" "\r\n"
            "398B34D0F6313C435EC8CC8C2D510567" "\r\n"
            "D27581F4F13DB47E55A80520D4BFA9E6" "\r\n"
            "501CAC9CFA90863D41E1BEF1474FBB41" "\r\n"
            "DD10ED7D8CA304683CB436997856D915" "\r\n"
            "DAC1F31ADD71BA55755F468440444BEF" "\r\n"
            "297AF26CD68FD3A0BBAC978E6A4AD8AF" "\r\n"
            "BF340F6A52521936C26AB828728459D5" "\r\n"
            "6CDC87320936DDA0BF1D11CBEA8DD639" "\r\n"
            "BF2F68822B40381AFA6FF89C04734254" "\r\n"
            "4AAC76E42998283A6E9EF7DC6227AA39" "\r\n"
            "3CA45976445F5710CE5A3B7D70DA2161" "\r\n"
            "90A259286103794D6C4F23679F20C786" "\r\n"
            "D6B28C74E278091752C2455CA326FB80" "\r\n"
            "A0462EB5DFF006D3A93886415618FB8D" "\r\n"
            "8F6FB5EA0CABD1013F04E56F4B25D65C" "\r\n"
            "F2525828B95596FB64138BDB97F0BEE7" "\r\n"
            "0A04C484EB86851B931DF16D99C26A16" "\r\n"
            "8057DB74ABE8FFDBDF32E529F03F1BEB" "\r\n"
            "6249AD9FB65CDF6E6C919A608475D8D3" "\r\n"
            // Other:
            "45923506432956493562935693478346" "\r\n" // F
            "46329469461254954325945934569378" "\r\n" // Y
            "63F49D96BDBA28F8428B4A5008D1A587";       // X

        const unsigned char BlockListSig0[64] =  {
            0xf8, 0xfd, 0x88, 0x16, 0x8b, 0x33, 0x1b, 0x71,
            0x3f, 0x0d, 0x6b, 0xec, 0xf1, 0x22, 0x21, 0x4b,
            0x2e, 0x19, 0x9b, 0xf8, 0xa1, 0xbd, 0x6d, 0x30,
            0x59, 0x80, 0xdb, 0xec, 0xad, 0xc4, 0x10, 0xe4,
            0x1a, 0xcc, 0x7d, 0xdb, 0x77, 0x79, 0xe8, 0x38,
            0xeb, 0x55, 0xa0, 0x40, 0x29, 0xf3, 0x90, 0x05,
            0xb3, 0x8f, 0x50, 0x3c, 0x12, 0x63, 0xa6, 0xe5,
            0xfb, 0xf7, 0xda, 0xa4, 0x5f, 0x60, 0x32, 0x4d, };

        std::string BlockList;
        BlockList.resize(0x10000, 0); // 64 kb should be enough
        ULONG BlockListLen = 0;
        ULONG status = SbieApi_Call(API_GET_SECURE_PARAM, 5, L"CertBlockList", (ULONG_PTR)BlockList.c_str(), BlockList.size(), (ULONG_PTR)&BlockListLen, 1);
        //BlockList.resize(BlockListLen);
        if (status != 0) // error
            BlockListLen = 0;

        if (BlockListLen < sizeof(BlockList0) - 1)
        {
            SbieApi_Call(API_SET_SECURE_PARAM, 3, L"CertBlockList", BlockList0, sizeof(BlockList0) - 1);
            SbieApi_Call(API_SET_SECURE_PARAM, 3, L"CertBlockListSig", BlockListSig0, sizeof(BlockListSig0));
            //BlockListLen = sizeof(BlockList0) - 1;
            //BlockList = BlockList0;
        }
    }

    //
    // continue with driver/service init
    //

    if (ok) {
        rc = SbieApi_Call(API_SET_SERVICE_PORT, 1, (ULONG_PTR)m_instance->m_PortHandle);
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

        rc = SbieApi_Call(API_INIT_GUI, 0);

        if (rc != 0) {
            LogEvent(MSG_9234, 0x9156, rc);
            ok = false;
        }
    }

#ifdef XP_SUPPORT
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
                    SbieApi_Call(API_SET_LSA_AUTH_PKG, 1, (ULONG_PTR)AuthPkgNum);

                LsaDeregisterLogonProcess(LsaHandle);
            }
        }
    }


#endif ! _WIN64
#endif

    if (ok) {

#ifdef _M_ARM64

        //
        // Sandboxie on ARM64 requires x86 applications NOT to use the CHPE binaries.
        // 
        // So when ever the service starts it uses the global xtajit config to disable the use of CHPE binaries,
        // for x86 processes and restores the original value on service shutdown.
        //
        // See comment in HookImageOptionsEx core/low/init.c for more details.
        //

        extern BOOLEAN DisableCHPE;
        DisableCHPE = SbieApi_QueryConfBool(NULL, L"DisableCHPE", TRUE);

        if (DisableCHPE) {
            HKEY hkey = NULL;
            LSTATUS rc = RegCreateKeyEx(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Wow64\\x86\\xtajit",
                0, NULL, 0, KEY_ALL_ACCESS, NULL, &hkey, NULL);
            if (rc == 0)
            {
                DWORD value;
                DWORD size = sizeof(value);
                rc = RegQueryValueEx(hkey, L"LoadCHPEBinaries_old", NULL, NULL, (BYTE*)&value, &size);
                if (rc != 0) { // only save the old value when its not already saved
                    rc = RegQueryValueEx(hkey, L"LoadCHPEBinaries", NULL, NULL, (BYTE*)&value, &size);
                    if (rc == 0)
                        RegSetValueEx(hkey, L"LoadCHPEBinaries_old", NULL, REG_DWORD, (BYTE*)&value, size);
                }

                value = 0;
                RegSetValueEx(hkey, L"LoadCHPEBinaries", NULL, REG_DWORD, (BYTE*)&value, sizeof(value));

                RegCloseKey(hkey);
            }
        }
#endif

        //
        // trigger manual invocation of LogMessage to collect any
        // messages that were logged while the driver was starting
        //

        m_instance->LogMessage(NULL);

        m_instance->m_DriverReady = true;

        //
        // check if there are boxes configured to be run in bSession0 
        // at system boot and run them on service start
        //

        WCHAR boxname[BOXNAME_COUNT];
        for (ULONG i = 0; ; ++i) {

            rc = SbieApi_QueryConfAsIs(
                NULL, L"StartSystemBox", i, boxname, (BOXNAME_COUNT - 2) * sizeof(WCHAR));
            if (rc != 0)
                break;

            SbieDll_RunStartExe(L"auto_run", boxname);
        }
    }

    NTSTATUS status = SbieApi_ReloadConf(0, SBIE_CONF_FLAG_RELOAD_CERT);
    if (status == STATUS_CONTENT_BLOCKED) {

        BYTE CertBlocked = 1;
        SbieApi_Call(API_SET_SECURE_PARAM, 3, L"CertBlocked", &CertBlocked, sizeof(CertBlocked));

        m_instance->m_DriverReady = false;
    }
    else {

        BYTE CertBlocked = 0;
        SbieApi_Call(API_GET_SECURE_PARAM, 3, L"CertBlocked", &CertBlocked, sizeof(CertBlocked));
        if (CertBlocked) {
            SCertInfo CertInfo = { 0 };
            if (NT_SUCCESS(status) && NT_SUCCESS(SbieApi_QueryDrvInfo(-1, &CertInfo, sizeof(CertInfo))) && CertInfo.type != eCertEvaluation) {
                CertBlocked = 0;
                SbieApi_Call(API_SET_SECURE_PARAM, 3, L"CertBlocked", &CertBlocked, sizeof(CertBlocked));
            } else
                m_instance->m_DriverReady = false;
        }
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

                SbieApi_Call(API_GUI_CLIPBOARD, 1, (ULONG_PTR)-1);

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
