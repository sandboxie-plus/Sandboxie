/*
 * Copyright 2004-2020 Sandboxie Holdings, LLC 
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
// Side-by-Side (WinSxS) Service for Sandboxed Programs
//---------------------------------------------------------------------------

#include <ntstatus.h>
#define WIN32_NO_STATUS
typedef long NTSTATUS;

#include <windows.h>
#define COBJMACROS
#include <objbase.h>
#include "core/dll/sbiedll.h"
#include "common/win32_ntddk.h"
#include "common/defines.h"
#include "common/my_version.h"


//---------------------------------------------------------------------------
// Defines
//---------------------------------------------------------------------------


#define SXS_TEXT_LEN    (512 * 1024)


//---------------------------------------------------------------------------
// Structures and Types
//---------------------------------------------------------------------------


typedef struct _SXS_GENERATE_ACTIVATION_CONTEXT_ARGS {

    ULONG       Zero00;
    USHORT      ArchId;
    LANGID      LangId;
    WCHAR      *StoreDir;
    ULONG       Zero0C;
    IUnknown   *ManifestStream;
    WCHAR      *SourcePath;
    ULONG       Two;
    IUnknown   *ConfigStream;
    WCHAR      *ConfigPath;
    ULONG       Zero24;
    ULONG       Zero28;
    ULONG       Zero2C;
    ULONG       Zero30;
    HANDLE      hSection;
    ULONG       Zero38;
    ULONG       Zero3C;
    ULONG       Zero40;
    ULONG       Zero44;
    ULONG       Zero48;
    ULONG       Zero4C;
    ULONG       Zero50;
    ULONG       Zero54;
    ULONG       Zero58;
    ULONG       Zero5C;
    ULONG       Zero60;
    ULONG       Zero64;
    ULONG       Zero68;
    ULONG       Zero6C;
    ULONG       Zero70;

} SXS_GENERATE_ACTIVATION_CONTEXT_ARGS;


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------


static void Sxs_Error(ULONG level, ULONG status);

static BOOLEAN Sxs_Init(void);

ULONG Sxs_Thread(void *arg);

static void Sxs_Request(
    UCHAR *data, ULONG data_len, UCHAR **rsp_data, ULONG *rsp_len);

static void Sxs_Generate(UCHAR *buf, HANDLE *hSection);

static IUnknown *Sxs_CreateStreamFromBuffer(UCHAR *Text);

static void Sxs_GenerateHelper(
    SXS_GENERATE_ACTIVATION_CONTEXT_ARGS *args, WCHAR *LangNames);


//---------------------------------------------------------------------------


typedef HRESULT (*P_CreateStreamOnHGlobal)(
    HGLOBAL hGlobal,
    BOOL fDeleteOnRelease,
    void **ppStream);

typedef ULONG (*P_SxsGenerateActivationContext)(
    SXS_GENERATE_ACTIVATION_CONTEXT_ARGS *args);


//---------------------------------------------------------------------------


static P_CreateStreamOnHGlobal        pCreateStreamOnHGlobal        = NULL;
static P_SxsGenerateActivationContext pSxsGenerateActivationContext = NULL;


//---------------------------------------------------------------------------
// Variables
//---------------------------------------------------------------------------


static BOOLEAN Sxs_WindowsVista     = FALSE;
static BOOLEAN Sxs_Windows7         = FALSE;
static BOOLEAN Sxs_Windows8         = FALSE;
static BOOLEAN Sxs_Windows10        = FALSE;

static const WCHAR *Sxs_QueueName   = L"RPCSS_SXS";
static HANDLE Sxs_QueueEventHandle  = NULL;


//---------------------------------------------------------------------------
// Sxs_Error
//---------------------------------------------------------------------------


_FX void Sxs_Error(ULONG level, ULONG status)
{
    extern const WCHAR *ServiceTitle;
    SbieApi_Log(2203, L"%S - (%S) [%02X/%08X]",
                Sxs_QueueName, ServiceTitle, level, status);
}


//---------------------------------------------------------------------------
// Sxs_Init
//---------------------------------------------------------------------------


_FX BOOLEAN Sxs_Init(void)
{
    HMODULE mod;
    OSVERSIONINFO osvi;
    WCHAR EvtName[128];
    HANDLE EvtHandle;
    ULONG status = -1;
    int retry = 0;

    //
    // initialize stuff needed to call SxsGenerateActivationContext
    //

    if (SbieApi_QueryConfBool(NULL, L"DisableBoxedWinSxS", FALSE))
        return FALSE;

    memzero(&osvi, sizeof(OSVERSIONINFO));
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    GetVersionEx((OSVERSIONINFO *)&osvi);

    if (osvi.dwMajorVersion == 10) {
        Sxs_Windows10 = TRUE;
    }
    else if (osvi.dwMajorVersion == 6) {
        if (osvi.dwMinorVersion >= 2)   // Windows 8, Windows 8.1, Windows 10 (Early builds)
            Sxs_Windows8 = TRUE;
        else if (osvi.dwMinorVersion == 1)
            Sxs_Windows7 = TRUE;
        else if (osvi.dwMinorVersion == 0)
            Sxs_WindowsVista = TRUE;
        else
            return FALSE;
    } else if (osvi.dwMajorVersion == 5) {
        if (osvi.dwMinorVersion == 0)
            return FALSE;
    } else
        return FALSE;
    mod = 0;

    // There is a chance for sxs.dll and ole32.dll to fail to load.  There
    // is a dependency on an internal component of rpcss for these dll's to
    // load.  A simple retry fixes this issue.
    //
    // Also, note that if this thread (the sxs thread) fails to start in rpcss no 
    // other process can start in the sandbox.  It is absolutely critical for 
    // Windows versions Vista and above.  Because of this, additional error messages 
    // are added to various thread exit paths to help troubleshoot issues with 
    // this thread starting.

    while (!mod && (retry++ < 20)) {
        mod = LoadLibrary(L"sxs.dll");
        if (!mod) {
            NtYieldExecution();
        }
    }
    if (mod) {
        pSxsGenerateActivationContext = (P_SxsGenerateActivationContext)
            GetProcAddress(mod, "SxsGenerateActivationContext");
    }
    else {
        Sxs_Error(0x77, status);
        return FALSE;
    }
    mod = 0;
    retry = 0;
    while (!mod && (retry++ < 20)) {
        mod = LoadLibrary(L"ole32.dll");
        if (!mod) {
            NtYieldExecution();
        }
    }
    if (mod) {
        pCreateStreamOnHGlobal = (P_CreateStreamOnHGlobal)
            GetProcAddress(mod, "CreateStreamOnHGlobal");
    }
    else {
        Sxs_Error(0x88, status);
        return FALSE;
    }

    if ((! pSxsGenerateActivationContext) || (! pCreateStreamOnHGlobal)) {
        Sxs_Error(0x44, status);
        return FALSE;
    }

    //
    // create the queue
    //

    for (retry = 0; retry < 5 * 3; ++retry) {
        status = SbieDll_QueueCreate(Sxs_QueueName, &Sxs_QueueEventHandle);
        if (status != STATUS_OBJECT_NAME_COLLISION)
            break;
        Sleep(1000 / 3);
    }
    if (status != 0) {
        Sxs_Error(0x11, status);
        return FALSE;
    }

    wcscpy(EvtName, SBIE_BOXED_);
    wcscat(EvtName, Sxs_QueueName);
    wcscat(EvtName, L"_READY");
    EvtHandle = CreateEvent(NULL, TRUE, FALSE, EvtName);
    if (EvtHandle) {
        SetEvent(EvtHandle);
    }

    return TRUE;
}


//---------------------------------------------------------------------------
// Sxs_Thread
//---------------------------------------------------------------------------


_FX ULONG Sxs_Thread(void *arg)
{
    ULONG req_id, data_len, rsp_len;
    UCHAR *data_ptr, *rsp_data;
    ULONG error = -1;
    ULONG status;

    if (! Sxs_Init())
        return 0;

    while (1) {

        WaitForSingleObject(Sxs_QueueEventHandle, INFINITE);

        while (1) {

            status = SbieDll_QueueGetReq(
                Sxs_QueueName, NULL, NULL, &req_id, &data_ptr, &data_len);

            if (status != 0 && status != STATUS_END_OF_FILE)
                Sxs_Error(0x22, status);

            if (! req_id)
                break;

            rsp_len = 0;
            if (data_len && data_len <= SXS_TEXT_LEN * 2) {

                Sxs_Request(data_ptr, data_len, &rsp_data, &rsp_len);
            }

            if (! rsp_len) {
                rsp_len = sizeof(ULONG);
                rsp_data = (UCHAR *)&error;
            }

            status = SbieDll_QueuePutRpl(
                Sxs_QueueName, req_id, rsp_data, rsp_len);

            if (status != 0 && status != STATUS_END_OF_FILE)
                Sxs_Error(0x33, status);

            if (rsp_data != (UCHAR *)&error)
                HeapFree(GetProcessHeap(), 0, rsp_data);

            SbieDll_FreeMem(data_ptr);
        }
    }

    return 0;
}


//---------------------------------------------------------------------------
// Sxs_Request
//---------------------------------------------------------------------------


_FX void Sxs_Request(
    UCHAR *data, ULONG data_len, UCHAR **rsp_data, ULONG *rsp_len)
{
    HANDLE hSection = NULL;
    Sxs_Generate(data, &hSection);

    if (hSection == (HANDLE)(ULONG_PTR)-1) {

        // see also Sxs_CallService in core/dll/sxs.c
        UCHAR *data = HeapAlloc(GetProcessHeap(), 0, 20);
        if (data) {
            memcpy(data, "*UseAltCreateActCtx*", 20);
            *rsp_len = 20;
            *rsp_data = data;
        }

    } else if (hSection) {

        const ULONG xViewShare = 1;

        NTSTATUS status;
        LARGE_INTEGER ZeroOffset;
        SIZE_T ViewSize = 0;
        void *MappedBase = NULL;
        ZeroOffset.QuadPart = 0;

        status = NtMapViewOfSection(
            hSection, NtCurrentProcess(), &MappedBase,
            0, 0, &ZeroOffset, &ViewSize, xViewShare,
            MEM_PHYSICAL, PAGE_READONLY);

        if (NT_SUCCESS(status)) {

            UCHAR *data = HeapAlloc(GetProcessHeap(), 0, ViewSize);
            if (data) {

                memcpy(data, MappedBase, ViewSize);
                *rsp_len = (ULONG)ViewSize;
                *rsp_data = data;
            }

            NtUnmapViewOfSection(NtCurrentProcess(), (void *)MappedBase);
        }

        CloseHandle(hSection);
    }
}


//---------------------------------------------------------------------------
// Sxs_Generate
//---------------------------------------------------------------------------


_FX void Sxs_Generate(UCHAR *buf, HANDLE *hSection)
{
    ULONG *ptr4;
    WCHAR *ptr2;
    UCHAR *ptr1;

    ULONG ArchId;
    ULONG LangId;
    WCHAR *LangNames;
    WCHAR *lpDirectory;
    WCHAR *lpSourcePath;
    WCHAR *lpConfigPath;
    UCHAR *lpManifestText;
    UCHAR *lpConfigText;

    SXS_GENERATE_ACTIVATION_CONTEXT_ARGS args;

    BOOLEAN ok;

    *hSection = NULL;

    ptr4 = (ULONG *)buf;
    if (*ptr4 != 'xobs')        // signature
        return;
    ++ptr4;                     // skip ULONG64
    ++ptr4;

    ArchId = *ptr4;
    ++ptr4;                     // skip ULONG

    LangId = *ptr4;
    ++ptr4;                     // skip ULONG

    ptr2 = (WCHAR *)ptr4;
    if (wcslen(ptr2) > 64)
        return;
    LangNames = ptr2;
    ptr2 += wcslen(ptr2) + 1;

    if (wcslen(ptr2) > 1024)
        return;
    lpDirectory = ptr2;
    ptr2 += wcslen(ptr2) + 1;

    if (wcslen(ptr2) > 1024)
        return;
    lpSourcePath = ptr2;
    ptr2 += wcslen(ptr2) + 1;

    if (wcslen(ptr2) > 1024)
        return;
    lpConfigPath = ptr2;
    ptr2 += wcslen(ptr2) + 1;

    ptr1 = (UCHAR *)ptr2;
    if (strlen(ptr1) > SXS_TEXT_LEN)
        return;
    lpManifestText = ptr1;
    ptr1 += strlen(ptr1) + 1;

    if (strlen(ptr1) > SXS_TEXT_LEN)
        return;
    lpConfigText = ptr1;
    ptr1 += strlen(ptr1) + 1;

    if (*ptr1 != '*')
        return;

    //
    //
    //

    /*{ WCHAR txt[512];
        wsprintf(txt, L"SandboxieRpcSs Generate Activation Context\n"); OutputDebugString(txt);
        wsprintf(txt, L"Directory  = %s\n", lpDirectory); OutputDebugString(txt);
        wsprintf(txt, L"SourcePath = %s\n", lpSourcePath); OutputDebugString(txt);
        wsprintf(txt, L"Languages  = %s\n", LangNames); OutputDebugString(txt);
        wsprintf(txt, L"------------------------------------------\n"); OutputDebugString(txt);
    }*/

    memzero(&args, sizeof(SXS_GENERATE_ACTIVATION_CONTEXT_ARGS));
    args.Two = 2;

    args.ArchId = (USHORT)ArchId;
    args.LangId = (LANGID)LangId;

    args.StoreDir   = lpDirectory;
    args.SourcePath = lpSourcePath;

    ok = TRUE;

    if (*lpManifestText) {
        args.ManifestStream = Sxs_CreateStreamFromBuffer(lpManifestText);
        if (! args.ManifestStream)
            ok = FALSE;
    } else
        ok = FALSE;

    if (ok && *lpConfigPath && *lpConfigText) {
        args.ConfigPath   = lpConfigPath;
        args.ConfigStream = Sxs_CreateStreamFromBuffer(lpConfigText);
        if (! args.ConfigStream)
            ok = FALSE;
    }

    if (ok)
        Sxs_GenerateHelper(&args, LangNames);

    /*{ WCHAR txt[512];
        wsprintf(txt, L"Result = %08X\n", args.hSection); OutputDebugString(txt);
        wsprintf(txt, L"------------------------------------------\n"); OutputDebugString(txt);
    }*/

    if (args.ConfigStream)
        IUnknown_Release(args.ConfigStream);
    if (args.ManifestStream)
        IUnknown_Release(args.ManifestStream);

    *hSection = args.hSection;
}


//---------------------------------------------------------------------------
// Sxs_CreateStreamFromBuffer
//---------------------------------------------------------------------------


_FX IUnknown *Sxs_CreateStreamFromBuffer(UCHAR *Text)
{
    IUnknown *pStream = NULL;
    ULONG Length = strlen(Text);
    HGLOBAL hGlobal = GlobalAlloc(GMEM_FIXED, Length);
    if (hGlobal) {
        void *pGlobal = GlobalLock(hGlobal);
        if (pGlobal) {
            memcpy(pGlobal, Text, Length);
            GlobalUnlock(pGlobal);
            if (pCreateStreamOnHGlobal(hGlobal, TRUE, &pStream) != S_OK)
                pStream = NULL;
        }
        if (! pStream)
            GlobalFree(hGlobal);
    }
    return pStream;
}


//---------------------------------------------------------------------------
// Sxs_GenerateHelper
//---------------------------------------------------------------------------


_FX void Sxs_GenerateHelper(
    SXS_GENERATE_ACTIVATION_CONTEXT_ARGS *args, WCHAR *LangNames)
{
    struct SXS_GENERATE_ACTIVATION_CONTEXT_VISTA_ARGS {

        ULONG       Zero00;
        USHORT      ArchId;
        USHORT      Zero06;
        WCHAR      *LangNames;
        WCHAR      *StoreDir;
        ULONG       Zero10;
        IUnknown   *ManifestStream;
        WCHAR      *SourcePath;
        ULONG       Two;
        IUnknown   *ConfigStream;
        WCHAR      *ConfigPath;

        ULONG64     Zero28_2C;

#ifdef _WIN64

        ULONG64     Zero30_34;
        ULONG64     Zero38_3C;
        HANDLE      hSectionWin8;
        HANDLE      hSectionVista;
        HANDLE      hSectionWin7;

#else ! _WIN64

        ULONG       Zero30;
        HANDLE      hSectionWin8;
        HANDLE      hSectionVista;
        HANDLE      hSectionWin7;
        ULONG64     Zero40_44;
        ULONG64     Zero48_4C;

#endif _WIN64

        ULONG64     Zero50_54;
        ULONG64     Zero58_5C;
        ULONG64     Zero60_64;
        ULONG64     Zero68_6C;
        ULONG64     Zero70_74;
        ULONG64     Zero74_78;          //  Offset +0x0a0 Zero74_78        : Uint8B
        BYTE        unknownBytes[241];  // Win 10 18980 has added a bunch of unknown fields to this structure

    } vista_args;

    if (Sxs_WindowsVista || Sxs_Windows7 || Sxs_Windows8 || Sxs_Windows10) {

        WCHAR *ptr = LangNames;
        while (*ptr) {
            if (*ptr == L'_')
                *ptr = L'\0';
            ++ptr;
        }

        memzero(&vista_args, sizeof(vista_args));

        vista_args.ArchId           = args->ArchId;
        vista_args.LangNames        = LangNames;
        vista_args.StoreDir         = args->StoreDir;
        vista_args.ManifestStream   = args->ManifestStream;
        vista_args.SourcePath       = args->SourcePath;
        vista_args.Two              = args->Two;
        vista_args.ConfigStream     = args->ConfigStream;
        vista_args.ConfigPath       = args->ConfigPath;

        if (pSxsGenerateActivationContext((void *)&vista_args)) {

            if (Sxs_WindowsVista)
                args->hSection = vista_args.hSectionVista;

            else if (Sxs_Windows7)
                args->hSection = vista_args.hSectionWin7;

            else if (Sxs_Windows8 || Sxs_Windows10)
                args->hSection = vista_args.hSectionWin8;

            else
                args->hSection = NULL;

            if (! args->hSection) {
                // in the esoteric case where pSxsGenerateActivationContext
                // returns success but zero section, we want to signal to
                // Sxs_CallService in core/dll/sxs.c to try alternate call
                args->hSection = (HANDLE)(ULONG_PTR)-1;
            }

        } else
            args->hSection = NULL;

    } else {

        if (! pSxsGenerateActivationContext(args))
            args->hSection = NULL;
    }
}
