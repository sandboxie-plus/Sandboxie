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
// WinSxS Services
//---------------------------------------------------------------------------


#include "dll.h"
#include "common/my_version.h"
#include <stdlib.h>
#include <stdio.h>


//---------------------------------------------------------------------------
// Structures and Types
//---------------------------------------------------------------------------


typedef struct _SXS_ARGS {

    ULONG ArchId;
    ULONG LangId;
    WCHAR *LangNames;
    WCHAR *Directory;
    WCHAR *SourcePath;
    UCHAR *ManifestText;
    WCHAR *ConfigPath;
    UCHAR *ConfigText;

} SXS_ARGS;


#define SXS_PATH_LEN    (2 * 1024)

#define SXS_TEXT_LEN    (512 * 1024)


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------


static BOOLEAN Sxs_AllocOrFreeBuffers(SXS_ARGS *args, BOOLEAN alloc);

static BOOLEAN Sxs_GetLanguage(ACTCTX *ActCtx, SXS_ARGS *args);

static BOOL Sxs_GetManifestResource_Helper(
    HMODULE hModule, LPCTSTR lpszType, LPTSTR lpszName, LONG_PTR lParam);

static BOOL Sxs_GetManifestResource(
    HMODULE hModule, ULONG *ResourceId, HRSRC *hResource);

static BOOL Sxs_ReadTextAndClose(HANDLE hFile, UCHAR *Text);

static void Sxs_ConvertUnicodeToAscii(UCHAR *Text);

static BOOLEAN Sxs_GetPathAndText(ACTCTX *ActCtx, SXS_ARGS *args);

static void *Sxs_CallService(SXS_ARGS *args, BOOLEAN *UseAltCreateActCtx);

static void Sxs_ActivationContextNotificationRoutine(
    ULONG Operation,
    HANDLE ActivationContext,
    ULONG_PTR MappedViewBaseAddress,
    ULONG_PTR Unknown1,
    ULONG_PTR Unknown2,
    BOOLEAN *pSuccess);

static HANDLE Sxs_CreateActCtxW_Alt(ACTCTX *ActCtx);

static BOOL Sxs_QueryActCtxW(DWORD dwFlags, HANDLE hActCtx,
    void *pvSubInstance, ULONG ulInfoClass, void *pvBuffer,
    SIZE_T cbBuffer, SIZE_T *pcbWrittenOrRequired);

static void Sxs_QueryActCtxW_2(ULONG *p_len, WCHAR **p_path);

static BOOL Sxs_SxsInstallW(void *info);

static BOOLEAN Sxs_CheckCompatLayer(
    const WCHAR *DosPath, const WCHAR *Setting);

static BOOLEAN Sxs_CheckCompatLayer_2(
    const WCHAR *DosPath, BOOLEAN User, WCHAR *Text, ULONG Length);

static BOOLEAN Sxs_CheckCompatLayer_3(
    const WCHAR *Text, const WCHAR *Setting);


//---------------------------------------------------------------------------

static NTSTATUS Sxs_NtSetInformationThread(
    HANDLE          ThreadHandle,
    THREADINFOCLASS ThreadInformationClass,
    PVOID           ThreadInformation,
    ULONG           ThreadInformationLength);

static NTSTATUS Sxs_NtCreateTransaction(
    HANDLE *TransactionHandle,
    ACCESS_MASK DesiredAccess,
    OBJECT_ATTRIBUTES *ObjectAttributes,
    void *UnknownParameter04,
    void *UnknownParameter05,
    void *UnknownParameter06,
    void *UnknownParameter07,
    void *UnknownParameter08,
    void *UnknownParameter09,
    void *UnknownParameter10);

static NTSTATUS Sxs_NtOpenTransaction(
    HANDLE *TransactionHandle,
    ACCESS_MASK DesiredAccess,
    OBJECT_ATTRIBUTES *ObjectAttributes,
    void *UnknownParameter04,
    void *UnknownParameter05);

static NTSTATUS Sxs_NtCommitTransaction(
    HANDLE TransactionHandle,
    ULONG_PTR UnknownParameter02);

static NTSTATUS Sxs_NtRollbackTransaction(
    HANDLE TransactionHandle,
    ULONG_PTR UnknownParameter02);

static void Sxs_RtlRaiseException(EXCEPTION_RECORD *ExceptionRecord);

static BOOL Sxs_CheckTokenMembership(
    HANDLE hToken, void *pSid, BOOL *IsMember);


//---------------------------------------------------------------------------


typedef HANDLE (*P_CreateActCtx)(void *pActCtx);

typedef BOOL (*P_QueryActCtx)(DWORD dwFlags, HANDLE hActCtx,
    void *pvSubInstance, ULONG ulInfoClass, void *pvBuffer,
    SIZE_T cbBuffer, SIZE_T *pcbWrittenOrRequired);

typedef BOOL (*P_SxsInstall)(void *info);

typedef NTSTATUS (*P_RtlCreateActivationContext)(
    ULONG_PTR Zero1,
    void *ActivationContextMappedBase,
    ULONG_PTR Zero2,
    ULONG_PTR CallbackRoutine,
    ULONG_PTR CallbackRoutineData,
    HANDLE *OutActCtx);

typedef LANGID (*P_GetUserDefaultUILanguage)(void);

typedef LANGID (*P_SetThreadUILanguage)(WORD LangId);

typedef NTSTATUS (*P_RtlGetThreadPreferredUILanguages)(
    ULONG_PTR Flags, ULONG *Count, WCHAR *Buffer, ULONG *Length);

typedef NTSTATUS (*P_RtlSetThreadPreferredUILanguages)(
    ULONG_PTR Flags, WCHAR *Buffer, ULONG *Count);

typedef void (*P_RtlRaiseException)(EXCEPTION_RECORD *ExceptionRecord);


//---------------------------------------------------------------------------


static P_CreateActCtx               __sys_CreateActCtxW             = NULL;

static P_QueryActCtx                __sys_QueryActCtxW              = NULL;

static P_RtlCreateActivationContext
                                   __sys_RtlCreateActivationContext = NULL;

static P_RtlGetThreadPreferredUILanguages
                             __sys_RtlGetThreadPreferredUILanguages = NULL;

static P_RtlSetThreadPreferredUILanguages
                             __sys_RtlSetThreadPreferredUILanguages = NULL;

static P_SetThreadUILanguage        __sys_SetThreadUILanguage       = NULL;

static P_GetUserDefaultUILanguage   __sys_GetUserDefaultUILanguage  = NULL;

P_NtSetInformationThread            __sys_NtSetInformationThread    = NULL;
static P_NtCreateTransaction        __sys_NtCreateTransaction       = NULL;
static P_NtOpenTransaction          __sys_NtOpenTransaction         = NULL;
static P_NtCommitTransaction        __sys_NtCommitTransaction       = NULL;
static P_NtRollbackTransaction      __sys_NtRollbackTransaction     = NULL;

static P_RtlRaiseException          __sys_RtlRaiseException         = NULL;

static P_SxsInstall                 __sys_SxsInstallW               = NULL;

extern P_NtOpenKey            __sys_NtOpenKey;

//---------------------------------------------------------------------------
// Variables
//---------------------------------------------------------------------------


static BOOLEAN Sxs_UseAltCreateActCtx = FALSE;

static void *Sxs_ActivateDefaultManifest_ImageBase = NULL;

static const WCHAR *Sxs_manifest = L".manifest";
static const WCHAR *Sxs_config   = L".config";

static const WCHAR *Sxs_QueueName = L"RPCSS_SXS";

static const WCHAR *Sxs_Manifest_Length = L"Manifest Length (%d)";

extern const WCHAR *File_BQQB;


//---------------------------------------------------------------------------
// Sxs_AllocOrFreeBuffers
//---------------------------------------------------------------------------


_FX BOOLEAN Sxs_AllocOrFreeBuffers(SXS_ARGS *args, BOOLEAN alloc)
{
    if (alloc) {

        ULONG size = /* SourcePath   */  SXS_PATH_LEN * sizeof(WCHAR)
                   + /* ManifestText */  SXS_TEXT_LEN * sizeof(UCHAR)
                   + /* ConfigPath   */  SXS_PATH_LEN * sizeof(WCHAR)
                   + /* ConfigText   */  SXS_TEXT_LEN * sizeof(UCHAR);

        UCHAR *buf = VirtualAlloc(0, size,
            MEM_RESERVE | MEM_COMMIT | MEM_TOP_DOWN, PAGE_READWRITE);

        if (! buf)
            return FALSE;

        args->SourcePath = (WCHAR *)buf;
        buf += SXS_PATH_LEN * sizeof(WCHAR);    // skip SourcePath
        args->ManifestText = buf;
        buf += SXS_TEXT_LEN * sizeof(UCHAR);    // skip ManifestText
        args->ConfigPath = (WCHAR *)buf;
        buf += SXS_PATH_LEN * sizeof(WCHAR);    // skip ConfigPath
        args->ConfigText = buf;

    } else {

        if (args->SourcePath)
            VirtualFree(args->SourcePath, 0, MEM_RELEASE);
    }

    return TRUE;
}


//---------------------------------------------------------------------------
// Sxs_GetLanguage
//---------------------------------------------------------------------------


_FX BOOLEAN Sxs_GetLanguage(ACTCTX *ActCtx, SXS_ARGS *args)
{
    BOOLEAN ok;

    //
    // basic parameter checking
    //

    if ((ActCtx->dwFlags & ACTCTX_FLAG_LANGID_VALID) && (! ActCtx->wLangId))
        return FALSE;

    //
    // get language code or language name
    //

    ok = TRUE;

    if (Dll_OsBuild >= 6000) {

        //
        // Windows Vista and later
        //

        ULONG count, len = 256;
        BOOLEAN ok = TRUE;

        args->LangNames = Dll_AllocTemp(len * sizeof(WCHAR));

        if ((! __sys_RtlGetThreadPreferredUILanguages)    ||
            (! __sys_RtlSetThreadPreferredUILanguages)    ||
            (! __sys_SetThreadUILanguage)) {

            return FALSE;
        }

        if (ActCtx->dwFlags & ACTCTX_FLAG_LANGID_VALID) {

            ULONG OrigCount;
            WCHAR *OrigLangNames = Dll_AllocTemp(len * sizeof(WCHAR));

            --len;

            if (__sys_RtlGetThreadPreferredUILanguages(
                    0x08, &OrigCount, OrigLangNames, &len) != 0) {

                ok = FALSE;

            } else {

                if (__sys_SetThreadUILanguage(ActCtx->wLangId) == 0) {

                    ok = FALSE;

                } else {

                    if (__sys_RtlGetThreadPreferredUILanguages(
                            0x38, &count, args->LangNames, &len) != 0) {

                        ok = FALSE;
                    }

                    __sys_RtlSetThreadPreferredUILanguages(
                        0x08, OrigLangNames, &OrigCount);
                }

            }

            Dll_Free(OrigLangNames);

        } else {

            if (__sys_RtlGetThreadPreferredUILanguages(
                    0x38, &count, args->LangNames, &len) != 0) {

                ok = FALSE;
            }
        }

        if (ok) {

            for (count = 0; count < len; ++count) {
                if (args->LangNames[count] == L'\0')
                    args->LangNames[count] = L'_';
            }
            args->LangNames[len] = L'\0';
        }

    } else {

        //
        // Windows XP
        //

        if (ActCtx->dwFlags & ACTCTX_FLAG_LANGID_VALID) {

            args->LangId = ActCtx->wLangId;

        } else {

            args->LangId = __sys_GetUserDefaultUILanguage();
        }
    }

    return ok;
}


//---------------------------------------------------------------------------
// Sxs_GetManifestResource_Helper
//---------------------------------------------------------------------------


_FX BOOL Sxs_GetManifestResource_Helper(
    HMODULE hModule, LPCTSTR lpszType, LPTSTR lpszName, LONG_PTR lParam)
{
    if (lpszType != RT_MANIFEST || (! lpszName))
        return TRUE;
    if (lParam) {
        ULONG_PTR *args = (ULONG_PTR *)lParam;
        if (IS_INTRESOURCE(lpszName))
            args[0] = (ULONG_PTR)lpszName;
        else if (*lpszName == L'#')
            args[0] = (USHORT)_wtoi(lpszName);
        args[1] = (ULONG_PTR) FindResource(hModule, lpszName, lpszType);
    }
    return FALSE;
}


//---------------------------------------------------------------------------
// Sxs_GetManifestResource
//---------------------------------------------------------------------------


_FX BOOL Sxs_GetManifestResource(
    HMODULE hModule, ULONG *ResourceId, HRSRC *hResource)
{
    ULONG_PTR args[2] = { 0, 0 };
    EnumResourceNames(hModule, RT_MANIFEST,
                      Sxs_GetManifestResource_Helper, (LONG_PTR)args);
    if (args[1]) {

        if (ResourceId)
            *ResourceId = (ULONG)args[0];
        if (hResource)
            *hResource = (HRSRC)args[1];

        SetLastError(ERROR_SUCCESS);
        return TRUE;

    } else {

        if (ResourceId)
            *ResourceId = 0;
        if (hResource)
            *hResource = 0;

        SetLastError(ERROR_RESOURCE_TYPE_NOT_FOUND);
        return FALSE;
    }
}


//---------------------------------------------------------------------------
// Sxs_ReadTextAndClose
//---------------------------------------------------------------------------


_FX BOOL Sxs_ReadTextAndClose(HANDLE hFile, UCHAR *Text)
{
    ULONG LastError;
    ULONG len;
    BOOL ok = ReadFile(hFile, Text, SXS_TEXT_LEN, &len, NULL);

    if (ok && ((! len) || (len >= SXS_TEXT_LEN))) {

        if (len >= SXS_TEXT_LEN)
            SbieApi_Log(2205, Sxs_Manifest_Length, len);
        LastError = ERROR_SXS_MANIFEST_FORMAT_ERROR;
        ok = FALSE;

    } else
        LastError = GetLastError();

    CloseHandle(hFile);

    if (ok)
        Text[len] = L'\0';
    else
        SetLastError(LastError);

    return ok;
}


//---------------------------------------------------------------------------
// Sxs_GetPathAndText
//---------------------------------------------------------------------------


_FX void Sxs_ConvertUnicodeToAscii(UCHAR *Text)
{
    UCHAR *ptri, *ptro;
    BOOLEAN warn = FALSE;

    if (Text[0] != 0xFF || Text[1] != 0xFE)
        return;

    ptro = Text;
    ptri = Text + 2;
    while (*ptri) {

        if (ptri[1] != '\0')
            warn = TRUE;

        *ptro = *ptri;
        ++ptro;
        ++ptri;
        ++ptri;
    }
    *ptro = '\0';

    ptri = strstr(Text, "encoding=\"");
    if (ptri) {
        ptri += 10;
        ptro = strchr(ptri, '\"');
        if (ptro && (ptro - ptri) == 5)
            memcpy(ptri, "UTF-8", 5);
        else
            warn = TRUE;
    }

    if (warn)
        SbieApi_Log(2205, L"SXS/UNICODE");
}


//---------------------------------------------------------------------------
// Sxs_GetPathAndText
//---------------------------------------------------------------------------


_FX BOOLEAN Sxs_GetPathAndText(ACTCTX *ActCtx, SXS_ARGS *args)
{
    ULONG LastError;
    ULONG len;
    HANDLE hFile;

    ULONG ResourceId = 0;
    void *ImageBase = NULL;
    BOOLEAN ShouldUnmap = FALSE;
    BOOLEAN IsExe = FALSE;

    if (ActCtx->dwFlags & ACTCTX_FLAG_HMODULE_VALID) {

        //
        // if ACTCTX_FLAG_HMODULE_VALID was passed, we're clearly
        // dealing with an image file
        //

        ImageBase = ActCtx->hModule;

        if (! GetModuleFileName(
                        ImageBase, args->SourcePath, SXS_PATH_LEN - 2))
            return FALSE;

        args->SourcePath[SXS_PATH_LEN - 2] = L'\0';

        if (Sxs_ActivateDefaultManifest_ImageBase == ImageBase) {

            //
            // when called from Sxs_ActivateDefaultManifest, we need to
            // emulate the behavior of the Windows loader and disregard
            // the resource number for external manifest/config files
            //

            IsExe = TRUE;
        }

    } else if (! ActCtx->lpSource) {

        //
        // without the ACTCTX_FLAG_HMODULE_VALID, we require lpSource
        //

        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;

    } else {

        //
        // open lpSource and try to map it as an image
        //

        HANDLE hSection;

        len = wcslen(ActCtx->lpSource);
        if (len > (SXS_PATH_LEN - 2))
            len = SXS_PATH_LEN - 2;
        wmemcpy(args->SourcePath, ActCtx->lpSource, len);
        args->SourcePath[len] = L'\0';

        hFile = CreateFile(
            args->SourcePath, FILE_READ_DATA, FILE_SHARE_READ, NULL,
            OPEN_EXISTING, 0, NULL);

        //
        // we can get a file not found error if lpSource does not specify
        // a path, in which case we retry with lpAssemblyDirectory
        //

        if ((hFile == INVALID_HANDLE_VALUE)
                && (ActCtx->dwFlags & ACTCTX_FLAG_ASSEMBLY_DIRECTORY_VALID)
                && ActCtx->lpAssemblyDirectory) {

            ULONG dir_len = wcslen(ActCtx->lpAssemblyDirectory);
            if ((dir_len < SXS_PATH_LEN)
                            && (dir_len + len <= (SXS_PATH_LEN - 2))) {

                wmemcpy(args->SourcePath,
                        ActCtx->lpAssemblyDirectory, dir_len);
                args->SourcePath[dir_len] = L'\\';
                wmemcpy(&args->SourcePath[dir_len + 1],
                        ActCtx->lpSource, len);
                args->SourcePath[dir_len + 1 + len] = L'\0';

                hFile = CreateFile(
                    args->SourcePath, FILE_READ_DATA, FILE_SHARE_READ, NULL,
                    OPEN_EXISTING, 0, NULL);
            }
        }

        if (hFile == INVALID_HANDLE_VALUE)
            return FALSE;

        hSection = CreateFileMapping(
                        hFile, NULL, PAGE_READONLY | SEC_IMAGE, 0, 0, NULL);

        if (! hSection) {

            //
            // map failed, which is acceptable if the file is not an image
            //

            LastError = GetLastError();

            if ((LastError != ERROR_BAD_EXE_FORMAT) ||
                    (ActCtx->dwFlags & ACTCTX_FLAG_RESOURCE_NAME_VALID)) {

                CloseHandle(hFile);

                if (LastError == ERROR_BAD_EXE_FORMAT)
                    RtlNtStatusToDosError(STATUS_INVALID_IMAGE_FORMAT);

                SetLastError(LastError);
                return FALSE;

            } else {

                //
                // this is probably a text file
                //

                if (! Sxs_ReadTextAndClose(hFile, args->ManifestText))
                    return FALSE;
            }

        } else {

            //
            // this is an EXE or a DLL file
            //

            CloseHandle(hFile);

            ImageBase = MapViewOfFileEx(
                                    hSection, FILE_MAP_READ, 0, 0, 0, NULL);

            LastError = GetLastError();

            CloseHandle(hSection);

            if (! ImageBase) {

                SetLastError(LastError);
                return FALSE;
            }

            ShouldUnmap = TRUE;
        }
    }

    //
    // for an image source, find resource ID
    //

    if (ImageBase) {

        HRSRC hResource = NULL;
        BOOLEAN ok = FALSE;
        BOOLEAN CheckExternalManifest = FALSE;

        if (ActCtx->dwFlags & ACTCTX_FLAG_RESOURCE_NAME_VALID) {

            if (! ActCtx->lpResourceName) {

                SetLastError(ERROR_INVALID_PARAMETER);
                goto unmap_and_return_false;

            } else {

                if (IS_INTRESOURCE(ActCtx->lpResourceName))
                    ResourceId = (ULONG)(ULONG_PTR)ActCtx->lpResourceName;

                hResource = FindResource(
                    ImageBase, ActCtx->lpResourceName, RT_MANIFEST);

                LastError = GetLastError();
            }

        } else {

            Sxs_GetManifestResource(ImageBase, &ResourceId, &hResource);
            LastError = GetLastError();
        }

        //
        // decide if we should check for an external manifest file
        //

        if (Dll_OsBuild >= 6000) {

            //
            // Windows Vista only looks for external manifest file for
            // the main EXE, and if it has an embedded resource id != 1,
            // or if there is no embedded manifest at all
            //
            // note that behavior should be same as XP if the setting
            // PreferExternalManifest is enabled, but we ignore that
            //

            if (IsExe && ResourceId != 1)
                CheckExternalManifest = TRUE;

        } else {

            //
            // Windows XP:
            // for EXEs, always look for an external manifest file
            // for DLLs, only we found or were given a manifest resource id
            //

            if (ResourceId || IsExe)
                CheckExternalManifest = TRUE;
        }

        //
        // external manifest file can override embedded manifest
        // note that we use ConfigPath as temporary storage
        //

        if (CheckExternalManifest) {

            wcscpy(args->ConfigPath, args->SourcePath);
            args->ConfigPath[SXS_PATH_LEN - 10] = L'\0';

            if ((! IsExe) && ResourceId > 1) {
                len = wcslen(args->ConfigPath);
                args->ConfigPath[len] = L'.';
                _itow(ResourceId, &args->ConfigPath[len + 1], 10);
            }

            wcscat(args->ConfigPath, Sxs_manifest);

            hFile = CreateFile(
                args->ConfigPath, FILE_READ_DATA, FILE_SHARE_READ, NULL,
                OPEN_EXISTING, 0, NULL);

            if (hFile != INVALID_HANDLE_VALUE) {

                if (! Sxs_ReadTextAndClose(hFile, args->ManifestText))
                    goto unmap_and_return_false;

                wcscpy(args->SourcePath, args->ConfigPath);

                hResource = NULL;
                ok = TRUE;
            }
        }

        if (hResource) {

            len = SizeofResource(ImageBase, hResource);
            if ((! len) || (len >= SXS_TEXT_LEN)) {

                if (len >= SXS_TEXT_LEN)
                    SbieApi_Log(2205, Sxs_Manifest_Length, len);
                LastError = ERROR_SXS_MANIFEST_FORMAT_ERROR;

            } else {

                HGLOBAL hGlobal = LoadResource(ImageBase, hResource);
                if (hGlobal) {
                    memcpy(args->ManifestText, LockResource(hGlobal), len);
                    ok = TRUE;
                }
            }
        }

        if (! ok) {
            SetLastError(LastError);
            goto unmap_and_return_false;
        }

        //
        // if we were called from Sxs_ActivateDefaultManifest, then neither
        // ACTCTX_FLAG_RESOURCE_NAME_VALID nor lpResourceName were given.
        // we fill these missing fields now, in case Sxs_CallService fails,
        // and we end up calling the system CreateActCtx.
        //

        if (Sxs_ActivateDefaultManifest_ImageBase == ImageBase) {

            ULONG ActCtxResourceId = ResourceId ? ResourceId : 1;
            ActCtx->lpResourceName = (WCHAR *)(ULONG_PTR)ActCtxResourceId;
            ActCtx->dwFlags |= ACTCTX_FLAG_RESOURCE_NAME_VALID;
        }
    }

    Sxs_ConvertUnicodeToAscii(args->ManifestText);

    //
    // read config file
    //

    wcscpy(args->ConfigPath, args->SourcePath);
    args->ConfigPath[SXS_PATH_LEN - 10] = L'\0';

    if (ImageBase) {

        if ((! IsExe) && ResourceId > 1) {
            len = wcslen(args->ConfigPath);
            args->ConfigPath[len] = L'.';
            _itow(ResourceId, &args->ConfigPath[len + 1], 10);
        }

        wcscat(args->ConfigPath, Sxs_config);

    } else {

        WCHAR *dot = wcsrchr(args->ConfigPath, L'.');
        if (dot && _wcsicmp(dot, Sxs_manifest) == 0)
            wcscpy(dot, Sxs_config);
        else
            wcscat(args->ConfigPath, Sxs_config);
    }

    hFile = CreateFile(
        args->ConfigPath, FILE_READ_DATA, FILE_SHARE_READ, NULL,
        OPEN_EXISTING, 0, NULL);

    if (hFile == INVALID_HANDLE_VALUE)
        *args->ConfigPath = L'\0';

    else if (! Sxs_ReadTextAndClose(hFile, args->ConfigText))
        goto unmap_and_return_false;

    Sxs_ConvertUnicodeToAscii(args->ConfigText);

    //
    // successful exit
    //

    if (ShouldUnmap)
        UnmapViewOfFile(ImageBase);

    SetLastError(ERROR_SUCCESS);
    return TRUE;

    //
    // error exit
    //

unmap_and_return_false:

    if (ShouldUnmap) {
        LastError = GetLastError();
        UnmapViewOfFile(ImageBase);
        SetLastError(LastError);
    }

    return FALSE;
}


//---------------------------------------------------------------------------
// Sxs_CallService
//---------------------------------------------------------------------------


_FX void *Sxs_CallService(SXS_ARGS *args, BOOLEAN *UseAltCreateActCtx)
{
    void *MappedBase = NULL;

    ULONG len1 = sizeof(ULONG)          // signature
               + sizeof(ULONG)
               + sizeof(ULONG)          // ArchId
               + sizeof(ULONG)          // LangId
               + (wcslen(args->LangNames) + 1) * sizeof(WCHAR)
               + (wcslen(args->Directory) + 1) * sizeof(WCHAR)
               + (wcslen(args->SourcePath) + 1) * sizeof(WCHAR)
               + (wcslen(args->ConfigPath) + 1) * sizeof(WCHAR)
               + (strlen(args->ManifestText) + 1) * sizeof(UCHAR)
               + (strlen(args->ConfigText) + 1) * sizeof(UCHAR)
               + sizeof(UCHAR);         // final '*'

    ULONG *buf1 = Dll_AllocTemp(len1);

    WCHAR *ptr2;
    UCHAR *ptr1;

    HANDLE handle;
    ULONG RequestId;
    ULONG status;
    ULONG retries;

    ULONG len2;
    ULONG *buf2;
    LARGE_INTEGER i64;
    ACCESS_MASK DesiredAccess;

    //
    // prepare memory block that will be read by the RpcSs SxS service
    // see Sxs_Generate in RpcSs/sxs.c
    //

    *(ULONG *)(buf1 + 0)   = tzuk;      // signature
    *(ULONG *)(buf1 + 1)   = 0;

    *(ULONG *)(buf1 + 2)   = args->ArchId;
    *(ULONG *)(buf1 + 3)   = args->LangId;

    ptr2 = (WCHAR *)(buf1 + 4);
    wcscpy(ptr2, args->LangNames);
    ptr2 += wcslen(ptr2) + 1;

    wcscpy(ptr2, args->Directory);
    ptr2 += wcslen(ptr2) + 1;

    wcscpy(ptr2, args->SourcePath);
    ptr2 += wcslen(ptr2) + 1;
    wcscpy(ptr2, args->ConfigPath);
    ptr2 += wcslen(ptr2) + 1;

    ptr1 = (UCHAR *)ptr2;
    strcpy(ptr1, args->ManifestText);
    ptr1 += strlen(ptr1) + 1;
    strcpy(ptr1, args->ConfigText);
    ptr1 += strlen(ptr1) + 1;

    *ptr1 = '*';

    //
    // send the request message through an SbieSvc queue
    //

    for (retries = 0; retries < 3; ++retries) {

        status = SbieDll_QueuePutReq(
                            Sxs_QueueName, buf1, len1, &RequestId, &handle);

        if (status == STATUS_OBJECT_NAME_NOT_FOUND) {

            if (Dll_ProcessFlags & SBIE_FLAG_FORCED_PROCESS) {

                status = STATUS_BAD_INITIAL_PC;     // see below
                break;

            } else {

                WCHAR EvtName[128];
                HANDLE EvtHandle;
                wcscpy(EvtName, SBIE_BOXED_);
                wcscat(EvtName, Sxs_QueueName);
                wcscat(EvtName, L"_READY");
                EvtHandle = CreateEvent(NULL, TRUE, FALSE, EvtName);

                SbieDll_StartCOM(FALSE);

                WaitForSingleObject(EvtHandle, 30 * 1000);
                CloseHandle(EvtHandle);

                continue;
            }

        } else if (status != 0)
            break;

        status = WaitForSingleObject(handle, 30 * 1000);
        CloseHandle(handle);
        if (status != 0)
            continue;

        status = SbieDll_QueueGetRpl(Sxs_QueueName, RequestId, &buf2, &len2);

        break;
    }

    Dll_Free(buf1);

    if (status != 0) {

        if (status == STATUS_PRIVILEGE_NOT_HELD) {

            //
            // SbieSvc could not open our caller process, for instance Avira
            // blocks access to its avscan.exe process
            //

            Sxs_UseAltCreateActCtx = TRUE;
            *UseAltCreateActCtx = TRUE;

        } else if (status == STATUS_BAD_INITIAL_PC) {

            //
            // if we are a forced process then we probably don't rely on
            // SXS assemblies in the sandbox, so if the RPCSS/SXS service
            // is not yet ready, then just use the system CreateActCtx
            //

            *UseAltCreateActCtx = TRUE;

        } else {

            SbieApi_Log(2203, L"%S - %S [%08X]",
                        Sxs_QueueName, Dll_ImageName, status);
        }

        return NULL;
    }

    if (len2 == sizeof(ULONG) && *(ULONG *)buf2 == -1) {
        Dll_Free(buf2);
        return NULL;
    }

    //
    // check for special and esoteric case where SandboxieRpcSs
    // is telling us to call the system CreateActCtx instead.
    // see also Sxs_Request and Sxs_GenerateHelper in apps/com/RpcSs/sxs.c
    //

    if (len2 == 20 && memcmp(buf2, "*UseAltCreateActCtx*", 20) == 0) {

        *UseAltCreateActCtx = TRUE;
        Dll_Free(buf2);
        return NULL;
    }

    //
    // create a section and fill it with the data from the reply
    //

    i64.QuadPart = len2;
    DesiredAccess = STANDARD_RIGHTS_REQUIRED
                  | SECTION_QUERY | SECTION_MAP_WRITE | SECTION_MAP_READ;
    status = NtCreateSection(&handle, DesiredAccess, NULL, &i64,
                             PAGE_READWRITE, SEC_COMMIT, NULL);

    if (NT_SUCCESS(status)) {

        const ULONG xViewShare = 1;

        SIZE_T ViewSize = len2;
        i64.QuadPart = 0;

        status = NtMapViewOfSection(
            handle, NtCurrentProcess(), &MappedBase, 0, 0,
            &i64, &ViewSize, xViewShare, MEM_PHYSICAL, PAGE_READWRITE);

        if (NT_SUCCESS(status)) {

            memcpy(MappedBase, buf2, len2);

            NtUnmapViewOfSection(NtCurrentProcess(), MappedBase);

            MappedBase = NULL;
            ViewSize = 0;
            i64.QuadPart = 0;

            status = NtMapViewOfSection(
                handle, NtCurrentProcess(), &MappedBase, 0, 0,
                &i64, &ViewSize, xViewShare, MEM_PHYSICAL, PAGE_READONLY);

            if (! NT_SUCCESS(status))
                MappedBase = 0;
        }

        NtClose(handle);
    }

    Dll_Free(buf2);
    return MappedBase;
}


//---------------------------------------------------------------------------
// Sxs_ActivationContextNotificationRoutine
//---------------------------------------------------------------------------


_FX void Sxs_ActivationContextNotificationRoutine(
    ULONG Operation,
    HANDLE ActivationContext,
    ULONG_PTR MappedViewBaseAddress,
    ULONG_PTR Unknown1,
    ULONG_PTR Unknown2,
    BOOLEAN *pSuccess)
{
    //
    // ideally we would pass kernel32!BasepSxsActivationContextNotification
    // to RtlCreateActivationContext, however that is not an exported symbol
    // and the simplest solution is to duplicate its functionality
    //

    if (Operation == 1) {
        NtUnmapViewOfSection(
            NtCurrentProcess(), (void *)MappedViewBaseAddress);
    } else
        *pSuccess = TRUE;
}


//---------------------------------------------------------------------------
// Sxs_CreateActCtxW
//---------------------------------------------------------------------------


_FX HANDLE Sxs_CreateActCtxW(ACTCTX *ActCtx)
{
    static const WCHAR *_Empty = L"";
    SXS_ARGS args;
    ULONG LastError;
    ULONG_PTR *pActivationContextData = NULL;
    HANDLE hActCtx = INVALID_HANDLE_VALUE;

    //
    // special processing if we are running in the context of SandboxieRpcSs
    //

    if (Sxs_UseAltCreateActCtx)
        return Sxs_CreateActCtxW_Alt(ActCtx);

    //
    // otherwise normal processing
    //

    memzero(&args, sizeof(args));

    //
    // processor architecture
    //

    if (ActCtx->dwFlags & ACTCTX_FLAG_PROCESSOR_ARCHITECTURE_VALID) {

        args.ArchId = ActCtx->wProcessorArchitecture;
        if (args.ArchId != PROCESSOR_ARCHITECTURE_INTEL         &&
            args.ArchId != PROCESSOR_ARCHITECTURE_MSIL          &&
            args.ArchId != PROCESSOR_ARCHITECTURE_AMD64         &&
            args.ArchId != PROCESSOR_ARCHITECTURE_IA32_ON_WIN64 &&
            args.ArchId != PROCESSOR_ARCHITECTURE_UNKNOWN) {

            LastError = ERROR_INVALID_PARAMETER;
            goto finish;
        }

    } else {

#ifdef _WIN64
        args.ArchId = PROCESSOR_ARCHITECTURE_AMD64;
#else
        args.ArchId = (Dll_IsWow64 ? PROCESSOR_ARCHITECTURE_IA32_ON_WIN64
                                   : PROCESSOR_ARCHITECTURE_INTEL);
#endif _WIN64
    }

    //
    // language id and name
    //

    args.LangNames = (WCHAR *)_Empty;

    if (! Sxs_GetLanguage(ActCtx, &args)) {

        LastError = ERROR_INVALID_PARAMETER;
        goto finish;
    }

    //
    // source file and manifest text
    //

    if (! Sxs_AllocOrFreeBuffers(&args, TRUE)) {

        LastError = STATUS_INSUFFICIENT_RESOURCES;
        goto finish;
    }

    if (! Sxs_GetPathAndText(ActCtx, &args)) {

        LastError = GetLastError();
        goto finish;
    }

    //
    // assembly store directory
    //

    if (ActCtx->dwFlags & ACTCTX_FLAG_ASSEMBLY_DIRECTORY_VALID) {

        args.Directory = (WCHAR *)ActCtx->lpAssemblyDirectory;

    } else {

        WCHAR *ptr;

        ULONG len = wcslen(args.SourcePath);
        args.Directory = Dll_AllocTemp((len + 4) * sizeof(WCHAR));
        wmemcpy(args.Directory, args.SourcePath, len + 1);

        ptr = wcsrchr(args.Directory, L'\\');
        if (ptr)
            *ptr = L'\0';
        else {

            LastError = ERROR_INVALID_PARAMETER;
            goto finish;
        }
    }

    //
    // prepare to set process default activation context
    //

    if (ActCtx->dwFlags & ACTCTX_FLAG_SET_PROCESS_DEFAULT) {

#ifdef _WIN64
        pActivationContextData = (ULONG_PTR *)(NtCurrentPeb() + 0x2F8);
#else
        pActivationContextData = (ULONG_PTR *)(NtCurrentPeb() + 0x1F8);
#endif _WIN64

        if (*pActivationContextData) {

            LastError = ERROR_SXS_PROCESS_DEFAULT_ALREADY_SET;
            goto finish;
        }
    }

    //
    // generate activation context data and create a handle to it
    //

    if (! __sys_RtlCreateActivationContext) {

        LastError = ERROR_PROC_NOT_FOUND;
        goto finish;

    } else {

        BOOLEAN UseAltCreateActCtx = FALSE;
        void *MappedBase = Sxs_CallService(&args, &UseAltCreateActCtx);

        if (MappedBase) {

            //
            // process result from SXS service in SandboxieRpcSs
            //

            NTSTATUS status = __sys_RtlCreateActivationContext(
                0, MappedBase, 0,
                (ULONG_PTR)Sxs_ActivationContextNotificationRoutine, 0,
                &hActCtx);

            if (! NT_SUCCESS(status)) {

                NtUnmapViewOfSection(NtCurrentProcess(), (void *)MappedBase);

            } else if (pActivationContextData) {

                *pActivationContextData = (ULONG_PTR)MappedBase;
            }

        } else if (UseAltCreateActCtx) {

            //
            // we get here if Sxs_CallService cannot talk to RPCSS/SXS
            //

            hActCtx = Sxs_CreateActCtxW_Alt(ActCtx);
            LastError = GetLastError();
            goto finish;
        }
    }

    if (hActCtx == INVALID_HANDLE_VALUE) {

        LastError = ERROR_SXS_CANT_GEN_ACTCTX;
        goto finish;
    }

    //
    // finish
    //

finish:

    if (args.LangNames && args.LangNames != _Empty)
        Dll_Free(args.LangNames);

    Sxs_AllocOrFreeBuffers(&args, FALSE);

    if (args.Directory &&
                    args.Directory != (WCHAR *)ActCtx->lpAssemblyDirectory)
        Dll_Free(args.Directory);

    SetLastError(LastError);
    return hActCtx;
}


//---------------------------------------------------------------------------
// Sxs_CreateActCtxW_Alt
//---------------------------------------------------------------------------


_FX HANDLE Sxs_CreateActCtxW_Alt(ACTCTX *ActCtx)
{
    //
    // the SandboxieRpcSs process implements our SXS service, so it is
    // not reliable to use our SXS service from the SandboxieRpcSs process
    // for at least two reasons.
    //
    // - the service pipe may not be ready for use yet
    // - even if the service pipe (and related thread) are ready,
    // some other thread in the SandboxieRpcSs process may block the
    // request.  for example, by holding the loader lock.
    //
    // workaround:  in the context of RpcSs, use the real SXS from CSRSS.
    //

    THREAD_DATA *TlsData = Dll_GetTlsData(NULL);
    ACTCTX MyActCtx;
    WCHAR *MySource = NULL;
    HANDLE hActCtx;
    ULONG LastError;

    //
    // get the real path to lpSource if it specifies a file in the sandbox
    //

    if (ActCtx->lpSource) {

        HANDLE hFile = CreateFileW(ActCtx->lpSource, GENERIC_READ,
                   FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);

        if (hFile != INVALID_HANDLE_VALUE) {

            NTSTATUS status;
            BOOLEAN IsBoxedPath;

            MySource = Dll_AllocTemp(sizeof(WCHAR) * 8192);

            status = SbieDll_GetHandlePath(hFile, MySource, &IsBoxedPath);

            CloseHandle(hFile);

            if (NT_SUCCESS(status) && IsBoxedPath) {

                if (SbieDll_TranslateNtToDosPath(MySource)) {

                    memcpy(&MyActCtx, ActCtx, sizeof(ACTCTX));
                    MyActCtx.lpSource = MySource;
                    ActCtx = &MyActCtx;

                    ++TlsData->proc_create_process;
                }
            }
        }
    }

    //
    // invoke the system service and finish.  note that we use the
    // proc_create_process flag to prevent File_RtlGetFullPathName_U
    // from changing the adjusted lpSource back into true path form
    //

    hActCtx = __sys_CreateActCtxW(ActCtx);

    LastError = GetLastError();

    if (ActCtx == &MyActCtx)
        --TlsData->proc_create_process;

    if (MySource)
        Dll_Free(MySource);

    SetLastError(LastError);
    return hActCtx;
}


//---------------------------------------------------------------------------
// Sxs_QueryActCtxW
//---------------------------------------------------------------------------


_FX BOOL Sxs_QueryActCtxW(DWORD dwFlags, HANDLE hActCtx,
    void *pvSubInstance, ULONG ulInfoClass, void *pvBuffer,
    SIZE_T cbBuffer, SIZE_T *pcbWrittenOrRequired)
{
    ULONG err;
    BOOL ok;

    ok = __sys_QueryActCtxW(dwFlags, hActCtx, pvSubInstance, ulInfoClass,
                            pvBuffer, cbBuffer, pcbWrittenOrRequired);
    err = GetLastError();

    if (ok && (! dwFlags) && (! hActCtx) && (! pvSubInstance) &&
            ulInfoClass == ActivationContextDetailedInformation) {

        ACTIVATION_CONTEXT_DETAILED_INFORMATION *info =
            (ACTIVATION_CONTEXT_DETAILED_INFORMATION *)pvBuffer;

        Sxs_QueryActCtxW_2( (ULONG *)&info->ulRootManifestPathChars,
                           (WCHAR **)&info->lpRootManifestPath);

        Sxs_QueryActCtxW_2( (ULONG *)&info->ulRootConfigurationPathChars,
                           (WCHAR **)&info->lpRootConfigurationPath);

        Sxs_QueryActCtxW_2( (ULONG *)&info->ulAppDirPathChars,
                           (WCHAR **)&info->lpAppDirPath);
    }

    SetLastError(err);
    return ok;
}


//---------------------------------------------------------------------------
// Sxs_QueryActCtxW_2
//---------------------------------------------------------------------------


_FX void Sxs_QueryActCtxW_2(ULONG *p_len, WCHAR **p_path)
{
    if (*p_len && *p_path) {

        WCHAR *TruePath = File_GetTruePathForBoxedPath(*p_path, TRUE);
        if (TruePath) {

            if ((*p_path)[*p_len - 1] == L'\\') {

                ULONG TruePath_len = wcslen(TruePath);
                if (TruePath_len && TruePath[TruePath_len - 1] != L'\\') {

                    WCHAR *TruePath2 =
                        Dll_AllocTemp((TruePath_len + 2) * sizeof(WCHAR));
                    wmemcpy(TruePath2, TruePath, TruePath_len);
                    TruePath2[TruePath_len] = L'\\';
                    TruePath2[TruePath_len + 1] = L'\0';

                    Dll_Free(TruePath);
                    TruePath = TruePath2;
                }
            }
        }

        if (TruePath) {

            ULONG TruePath_len = wcslen(TruePath);
            if (TruePath_len <= *p_len) {

                wmemcpy(*p_path, TruePath, TruePath_len + 1);
                *p_len = TruePath_len;
            }

            Dll_Free(TruePath);
        }
    }
}


//---------------------------------------------------------------------------
// Sxs_NtSetInformationThread
//---------------------------------------------------------------------------


_FX NTSTATUS Sxs_NtSetInformationThread(
    HANDLE          ThreadHandle,
    THREADINFOCLASS ThreadInformationClass,
    PVOID           ThreadInformation,
    ULONG           ThreadInformationLength)
{
    return __sys_NtSetInformationThread(ThreadHandle,
        ThreadInformationClass,
        ThreadInformation,
        ThreadInformationLength);
}


//---------------------------------------------------------------------------
// Sxs_NtCreateTransaction
//---------------------------------------------------------------------------


_FX NTSTATUS Sxs_NtCreateTransaction(
    HANDLE *TransactionHandle,
    ACCESS_MASK DesiredAccess,
    OBJECT_ATTRIBUTES *ObjectAttributes,
    void *UnknownParameter04,
    void *UnknownParameter05,
    void *UnknownParameter06,
    void *UnknownParameter07,
    void *UnknownParameter08,
    void *UnknownParameter09,
    void *UnknownParameter10)
{
    *TransactionHandle = NULL;
    return STATUS_SUCCESS;
}



//---------------------------------------------------------------------------
// Sxs_NtOpenTransaction
//---------------------------------------------------------------------------


_FX NTSTATUS Sxs_NtOpenTransaction(
    HANDLE *TransactionHandle,
    ACCESS_MASK DesiredAccess,
    OBJECT_ATTRIBUTES *ObjectAttributes,
    void *UnknownParameter04,
    void *UnknownParameter05)
{
    *TransactionHandle = NULL;
    return STATUS_SUCCESS;
}


//---------------------------------------------------------------------------
// Sxs_NtCommitTransaction
//---------------------------------------------------------------------------


_FX NTSTATUS Sxs_NtCommitTransaction(
    HANDLE TransactionHandle,
    ULONG_PTR UnknownParameter02)
{
    return STATUS_SUCCESS;
}


//---------------------------------------------------------------------------
// Sxs_NtRollbackTransaction
//---------------------------------------------------------------------------


_FX NTSTATUS Sxs_NtRollbackTransaction(
    HANDLE TransactionHandle,
    ULONG_PTR UnknownParameter02)
{
    return STATUS_SUCCESS;
}


//---------------------------------------------------------------------------
// Sxs_RtlRaiseException
//---------------------------------------------------------------------------


_FX void Sxs_RtlRaiseException(EXCEPTION_RECORD *ExceptionRecord)
{
    ULONG code = ExceptionRecord->ExceptionCode;
    if ((code & 0xFFFF0000) != 0x40010000)  // ignore debug exceptions
        SbieApi_Log(2205, L"TrustedInstaller %08X", code);
    __sys_RtlRaiseException(ExceptionRecord);
}


//---------------------------------------------------------------------------
// Sxs_CheckTokenMembership
//---------------------------------------------------------------------------


_FX BOOL Sxs_CheckTokenMembership(
    HANDLE hToken, void *pSid, BOOL *IsMember)
{
    *IsMember = TRUE;
    SetLastError(ERROR_SUCCESS);
    return TRUE;
}


//---------------------------------------------------------------------------
// Sxs_SxsInstallW
//---------------------------------------------------------------------------


_FX BOOL Sxs_SxsInstallW(void *info)
{
    BOOL ok = __sys_SxsInstallW(info);
    if (! ok) {
        SbieApi_Log(2205, L"SxsInstallW");
        SetLastError(0);
        ok = TRUE;
    }
    return ok;
}


//---------------------------------------------------------------------------
// Sxs_InitKernel32
//---------------------------------------------------------------------------


_FX BOOLEAN Sxs_InitKernel32(void)
{
    HMODULE module;
    void *CreateActCtxW;
    void *NtSetInformationThread;
    void *NtCreateTransaction;
    void *NtOpenTransaction;
    void *NtCommitTransaction;
    void *NtRollbackTransaction;
    void *RtlRaiseException;

    //
    // CreateActCtx function is only supported in Windows XP, so hook
    // only if we find it in kernel32.dll (kernelbase.dll on Windows 8)
    //
    // note that the real CreateActCtxA internally calls CreateActCtxW
    // after doing ANSI-to-UNICODE conversion, so we need not hook it
    //

    if (Dll_OsBuild >= 8400)
        module = Dll_KernelBase;
    else
        module = Dll_Kernel32;

    CreateActCtxW = GetProcAddress(module, "CreateActCtxW");

    if (CreateActCtxW) {

        //
        // import stuff that CreateActCtx is going to need
        //

#define SXS_IMPORT(b,a) __sys_##a = (P_##a) GetProcAddress(b, #a);

        SXS_IMPORT(Dll_Ntdll, RtlCreateActivationContext);

        SXS_IMPORT(Dll_Kernel32, GetUserDefaultUILanguage);

        if (Dll_OsBuild >= 6000) {

            SXS_IMPORT(Dll_Ntdll, RtlGetThreadPreferredUILanguages);

            SXS_IMPORT(Dll_Ntdll, RtlSetThreadPreferredUILanguages);

            SXS_IMPORT(Dll_Kernel32, SetThreadUILanguage);
        }

#undef  SXS_IMPORT

        //
        // disable in-sandbox SXS resolutions in three cases:
        //
        // if this is the SandboxieRpcSs process which itself implements
        // the in-sandbox SXS service
        //
        // if the sandbox setting DisableBoxedWinSxS=y
        //

        if (Dll_ImageType == DLL_IMAGE_SANDBOXIE_RPCSS || Dll_AppContainerToken ||
                Config_GetSettingsForImageName_bool(L"DisableBoxedWinSxS", FALSE)) {

            Sxs_UseAltCreateActCtx = TRUE;
        }

        //
        // hook CreateActCtx
        //

        SBIEDLL_HOOK(Sxs_,CreateActCtxW);
    }

    //
    // hook QueryActCtx in the context of a forced program because because
    // the activation context might have been with sandboxed paths
    //

    if (Dll_ProcessFlags & SBIE_FLAG_FORCED_PROCESS) {

        void *QueryActCtxW = GetProcAddress(module, "QueryActCtxW");
        if (QueryActCtxW) {

            SBIEDLL_HOOK(Sxs_,QueryActCtxW);
        }
    }

    module = Dll_Ntdll;

    //
    // Opera hooks NtSetInformationThread. SboxDll calls __sys_NtSetInformationThread to bypass Opera hook.
    // See the comment about Thread_SetInformationThread_ChangeNotifyToken in Gui_ConnectToWindowStationAndDesktop
    // 
    // Tested with opera 117 and this workaround seems no longer required
    // 

    NtSetInformationThread = GetProcAddress(Dll_Ntdll, "NtSetInformationThread");
    if (NtSetInformationThread) {
        SBIEDLL_HOOK(Sxs_, NtSetInformationThread);
    }

    //
    // place ntdll.dll hooks only if TrustedInstaller
    //

    if (Dll_ImageType != DLL_IMAGE_TRUSTED_INSTALLER)
        return TRUE;

    NtCreateTransaction   = GetProcAddress(module, "NtCreateTransaction");
    NtOpenTransaction     = GetProcAddress(module, "NtOpenTransaction");
    NtCommitTransaction   = GetProcAddress(module, "NtCommitTransaction");
    NtRollbackTransaction = GetProcAddress(module, "NtRollbackTransaction");
    RtlRaiseException     = GetProcAddress(module, "RtlRaiseException");

    if (NtCreateTransaction) {
        SBIEDLL_HOOK(Sxs_,NtCreateTransaction);
    }
    if (NtOpenTransaction) {
        SBIEDLL_HOOK(Sxs_,NtOpenTransaction);
    }
    if (NtCommitTransaction) {
        SBIEDLL_HOOK(Sxs_,NtCommitTransaction);
    }
    if (NtRollbackTransaction) {
        SBIEDLL_HOOK(Sxs_,NtRollbackTransaction);
    }
    if (RtlRaiseException) {
        SBIEDLL_HOOK(Sxs_,RtlRaiseException);
    }

    //
    // In Windows 7, the TrustedInstaller service mounts HKLM\COMPONENTS
    // if the registry key/hive is not present.  Which means it will not
    // mount the hive when the corresponding sandbox key has been created.
    // to work around this, we try to explicitly mount the hive here.
    //

    if (Dll_OsBuild >= 7600) {

        OBJECT_ATTRIBUTES TargetObjectAttributes;
        OBJECT_ATTRIBUTES SourceObjectAttributes;
        UNICODE_STRING TargetObjectName;
        UNICODE_STRING SourceObjectName;

        WCHAR buf[MAX_PATH + 48];
        GetSystemWindowsDirectory(buf + 4, MAX_PATH);
        wmemcpy(buf, File_BQQB, 4);
        wcscat(buf, L"\\System32\\config\\COMPONENTS");
        RtlInitUnicodeString(&SourceObjectName, buf);
        RtlInitUnicodeString(&TargetObjectName,
            L"\\REGISTRY\\MACHINE\\COMPONENTS");

        InitializeObjectAttributes(&SourceObjectAttributes,
            &SourceObjectName, OBJ_CASE_INSENSITIVE, NULL, NULL);
        InitializeObjectAttributes(&TargetObjectAttributes,
            &TargetObjectName, OBJ_CASE_INSENSITIVE, NULL, NULL);

        NtLoadKey(&TargetObjectAttributes, &SourceObjectAttributes);
    }

    //
    // on Windows 8, hook kernelbase!CheckTokenMembership which is
    // called from TiWorker!AclCheckTrustedInstallerAccess
    //

    if (Dll_OsBuild >= 8400) {

        module = Dll_KernelBase;

        typedef BOOL (*P_CheckTokenMembership)(
            HANDLE hToken, void *pSid, BOOL *IsMember);
        P_CheckTokenMembership CheckTokenMembership =
            (P_CheckTokenMembership) GetProcAddress(
                Dll_KernelBase, "CheckTokenMembership");
        P_CheckTokenMembership __sys_CheckTokenMembership;

        SBIEDLL_HOOK(Sxs_,CheckTokenMembership);
    }

    return TRUE;
}


//---------------------------------------------------------------------------
// Sxs_Init
//---------------------------------------------------------------------------


_FX BOOLEAN Sxs_Init(HMODULE module)
{
    P_SxsInstall SxsInstallW;

    SxsInstallW = (P_SxsInstall)GetProcAddress(module, "SxsInstallW");

    SBIEDLL_HOOK(Sxs_,SxsInstallW);

    return TRUE;
}


//---------------------------------------------------------------------------
// Sxs_ActivateDefaultManifest
//---------------------------------------------------------------------------


_FX void Sxs_ActivateDefaultManifest(void *ImageBase)
{
    if (! __sys_CreateActCtxW)
        return;

    if (Dll_ProcessFlags & SBIE_FLAG_FORCED_PROCESS) {

        //
        // if we are a forced process then csrss already applied the
        // process default activation context
        //

        return;
    }

    //
    // reset the process default activation context that was created
    // by our dummy manifest/config files.  but note that don't do this
    // if we are a ForcedProcessComServer where the activaton context
    // was already set by csrss.
    //

    if (! Sxs_UseAltCreateActCtx) {

        ULONG_PTR *pActivationContextData;

#ifdef _WIN64
        pActivationContextData = (ULONG_PTR *)(NtCurrentPeb() + 0x2F8);
#else
        pActivationContextData = (ULONG_PTR *)(NtCurrentPeb() + 0x1F8);
#endif _WIN64

        *pActivationContextData = 0;
    }

    //
    // create and activate process default activation context
    //

    if (! Sxs_UseAltCreateActCtx) {

        ACTCTX ActCtx;

        WCHAR *DosPath =
            Dll_Alloc((wcslen(Ldr_ImageTruePath) + 4) * sizeof(WCHAR));
        wcscpy(DosPath, Ldr_ImageTruePath);
        SbieDll_TranslateNtToDosPath(DosPath);

        memzero(&ActCtx, sizeof(ACTCTX));
        ActCtx.cbSize = sizeof(ACTCTX);
        ActCtx.dwFlags = ACTCTX_FLAG_HMODULE_VALID
                       | ACTCTX_FLAG_SET_PROCESS_DEFAULT;
        ActCtx.hModule = ImageBase;
        ActCtx.lpSource = DosPath;

        Sxs_ActivateDefaultManifest_ImageBase = ImageBase;

        Sxs_CreateActCtxW(&ActCtx);

        Sxs_ActivateDefaultManifest_ImageBase = NULL;

        Dll_Free(DosPath);
    }
}


//---------------------------------------------------------------------------
// Sxs_CheckManifestForElevation
//---------------------------------------------------------------------------


_FX ULONG Sxs_CheckManifestForElevation(
    const WCHAR* DosPath, 
    BOOLEAN *pAsInvoker, 
    BOOLEAN *pRequireAdministrator, 
    BOOLEAN *pHighestAvailable)
{
    ACTCTX ActCtx;
    SXS_ARGS args;
    ULONG rc;

    if (Dll_OsBuild < 6000)
        return STATUS_NOT_IMPLEMENTED;

    //
    // invoke Sxs_GetPathAndText to get the manifest text
    //

    memzero(&args, sizeof(args));

    if (! Sxs_AllocOrFreeBuffers(&args, TRUE))
        return STATUS_INSUFFICIENT_RESOURCES;

    memzero(&ActCtx, sizeof(ACTCTX));
    ActCtx.cbSize = sizeof(ACTCTX);
    ActCtx.lpSource = DosPath;

    rc = STATUS_UNSUCCESSFUL;

    if (Sxs_GetPathAndText(&ActCtx, &args)) {

        rc = STATUS_SUCCESS; // manifest found

        _strlwr(args.ManifestText);

        if (pAsInvoker) *pAsInvoker = 
            (strstr(args.ManifestText, "level='asinvoker'") 
            || strstr(args.ManifestText, "level=\"asinvoker\""));

        if (pRequireAdministrator) *pRequireAdministrator = 
            (strstr(args.ManifestText, "level='requireadministrator'")
            || strstr(args.ManifestText, "level=\"requireadministrator\""));

        if (pHighestAvailable) *pHighestAvailable = 
            (strstr(args.ManifestText, "level='highestavailable'")
            || strstr(args.ManifestText, "level=\"highestavailable\""));
    }

    Sxs_AllocOrFreeBuffers(&args, FALSE);

    return rc;
}


//---------------------------------------------------------------------------
// Sxs_CheckManifestForCreateProcess
//---------------------------------------------------------------------------


_FX ULONG Sxs_CheckManifestForCreateProcess(const WCHAR *DosPath)
{
    THREAD_DATA *TlsData = Dll_GetTlsData(NULL);
    ULONG rc, ElvType;
    BOOLEAN AsInvoker, RequireAdministrator, HighestAvailable;

    //
    // Windows Vista UAC auto-elevates program names that includes words
    // like setup/installer/update.  a program may specify
    //              requestedExecutionLevel level="asInvoker"
    // in its manifest to inhibit auto-elevation.  but since we generally
    // use an empty manifest file, it interferes with the asInvoker request.
    //
    // in this function we examine the real manifest, and if it specifies
    // asInvoker then we will set a flag to use alternate manifest/config
    // files that also specify asInvoker.
    //

    TlsData->proc_create_process_as_invoker = FALSE;

    if (! __sys_CreateActCtxW)
        return 0;
    if (Dll_OsBuild < 6000)
        return 0;

    //
    // if we are already administrator then pretend asInvoker
    // was used, to prevent any more interference from UAC
    //

    ElvType = SbieDll_GetTokenElevationType();

    if (ElvType == TokenElevationTypeFull) {
        TlsData->proc_create_process_as_invoker = TRUE;
        return 0;
    }

    rc = Sxs_CheckManifestForElevation(DosPath, &AsInvoker, &RequireAdministrator, &HighestAvailable);

    if (NT_SUCCESS(rc)) {

        //
        // asInvoker means to use alternate manifest files in
        // Sxs_FileCallback, see there
        //
        // highestAvailable or requireAdministrator means we tell
        // our Proc_CreateProcess caller to use SH32_DoRunAs
        //

        if (AsInvoker)
            TlsData->proc_create_process_as_invoker = TRUE;

        if (RequireAdministrator ||
                (HighestAvailable && ElvType != TokenElevationTypeDefault)) {

            //
            // if we are running in the SbieSvc UAC elevation process
            // then we cannot return ERROR_ELEVATION_REQUIRED because
            // that would cause an infinite loop
            //

            if (Dll_ImageType != DLL_IMAGE_SANDBOXIE_SBIESVC) {

                if (Sxs_CheckCompatLayer(DosPath, L"RunAsInvoker"))
                    TlsData->proc_create_process_as_invoker = TRUE;
                else
                    rc = ERROR_ELEVATION_REQUIRED;
            }
        }
    }

    return rc;
}


//---------------------------------------------------------------------------
// Sxs_PreferExternal
//---------------------------------------------------------------------------


_FX BOOLEAN Sxs_PreferExternal(THREAD_DATA *TlsData)
{
    if (!TlsData->proc_image_path)
        return FALSE;

    //
    // KB5014019 breaks Edge, it seems that making Edge to start its child processes with
    // PreferExternalManifest fixes the issue, but the main process must be loaded normally
    // hence we have ExternalManifestHack that checks the parent and not the target
    //

    if (Config_GetSettingsForImageName_bool(L"ExternalManifestHack", FALSE))
        return TRUE;

    WCHAR *ptr1 = wcsrchr(TlsData->proc_image_path, L'\\');

    WCHAR value[16];
    SbieDll_GetSettingsForName(NULL, ptr1 + 1, L"PreferExternalManifest", value, sizeof(value), NULL);
    return Config_String2Bool(value, FALSE);
}


//---------------------------------------------------------------------------
// Sxs_KeyCallback
//---------------------------------------------------------------------------


_FX BOOLEAN Sxs_KeyCallback(const WCHAR *path, HANDLE *out_handle)
{
    //
    // this callback is invoked by Key_NtCreateKey when it is used during
    // CreateProcess.  this allows us to intercept the open for SideBySide
    // key so we can redirect to the SbieSvc service key which includes
    // a pre-set value for PreferExternalManifest
    //

    THREAD_DATA *TlsData = Dll_GetTlsData(NULL);

    if (!Sxs_PreferExternal(TlsData))
        return FALSE;

    if (_wcsnicmp(path, L"\\Registry\\Machine\\", 18) == 0) {

        BOOLEAN redirect = FALSE;

        path += 18;
        if (_wcsnicmp(path, L"Components", 10) == 0)
            redirect = TRUE;

        else if (_wcsnicmp(path, L"Software\\", 9) == 0) {

            path += 9;

            if (_wcsnicmp(path, L"Wow6432Node\\", 12) == 0)
                path += 12;

            if (0 == _wcsicmp(path,
                    L"Microsoft\\Windows\\CurrentVersion\\SideBySide"))
                redirect = TRUE;
        }

        if (redirect) {

            const WCHAR *ValueName = L"PreferExternalManifest";
            extern WCHAR *Support_SbieSvcKeyPath;

            UNICODE_STRING objname;
            OBJECT_ATTRIBUTES objattrs;
            HANDLE handle;

            InitializeObjectAttributes(
                &objattrs, &objname, OBJ_CASE_INSENSITIVE, NULL, NULL);

            RtlInitUnicodeString(&objname, Support_SbieSvcKeyPath);

            if (0 != __sys_NtOpenKey(&handle, KEY_READ, &objattrs))
                handle = NULL;

            if (handle) {

                union {
                    KEY_VALUE_PARTIAL_INFORMATION kvpi;
                    ULONG space[6];
                } info;
                ULONG len;

                RtlInitUnicodeString(&objname, ValueName);

                if (0 != NtQueryValueKey(
                        handle, &objname, KeyValuePartialInformation,
                        &info, sizeof(info), &len))
                    info.kvpi.Type = 0;

                if (info.kvpi.Type == REG_DWORD             &&
                    info.kvpi.DataLength == sizeof(ULONG)   &&
                    *(ULONG *)info.kvpi.Data != 0) {

                    //WCHAR txt[1024];
                    //Sbie_snwprintf(txt, 1024, L"REDIR KEY - %s\n", path);
                    //OutputDebugString(txt);

                    *out_handle = handle;
                    return TRUE;
                }

                CloseHandle(handle);
            }

            SbieApi_Log(2205, ValueName);
        }
    }

    return FALSE;
}


//---------------------------------------------------------------------------
// Sxs_FileCallback
//---------------------------------------------------------------------------


_FX BOOLEAN Sxs_FileCallback(const WCHAR *path, HANDLE *out_handle)
{
    //
    // this callback is invoked by File_NtCreateFile when it is used during
    // CreateProcess.  this allows us to intercept any opens for .manifest
    // and .config files and redirect them to our dummy/empty manifest from
    // our installation home directory
    //

    THREAD_DATA *TlsData = Dll_GetTlsData(NULL);

    if (!Sxs_PreferExternal(TlsData))
        return FALSE;

    const WCHAR *_Manifest_Txt = L"\\Manifest1.txt";
    const WCHAR *_Config_Txt   = L"\\Manifest2.txt";
    const WCHAR *_Empty_Txt    = L"\\Manifest0.txt";
    const WCHAR *FileName = NULL;
    ULONG  FileSize = 0;

    WCHAR *ptr1 = wcsrchr(TlsData->proc_image_path, L'\\');
    WCHAR *ptr2 = wcsrchr(path, L'\\');
    ULONG len = 0;
    if (ptr1)
        len = wcslen(ptr1);

    if (len && ptr2 && _wcsnicmp(ptr1, ptr2, len) == 0) {

        if (_wcsicmp(ptr2 + len, Sxs_manifest) == 0) {
            FileName = _Manifest_Txt;
            FileSize = 364;

        } else if (_wcsicmp(ptr2 + len, Sxs_config) == 0) {
            FileName = _Config_Txt;
            FileSize = 92;
        }
    }

    if (FileName && (! TlsData->proc_create_process_as_invoker)) {

        //
        // if the EXE does not explicitly say in its manifest:
        //          requestedExecutionLevel level="asInvoker"
        // then we use the empty manifest/config files, to allow
        // UAC to use its auto-elevation heuristics
        //

        FileName = _Empty_Txt;
        FileSize = 2;
    }

    if (FileName) {

        HANDLE handle;

        WCHAR *FilePath = Dll_AllocTemp(MAX_PATH * 2 * sizeof(WCHAR));
        wcscpy(FilePath, Dll_HomeDosPath);
        wcscat(FilePath, FileName);

        //OutputDebugString(L"*** *** ***\n");
        //OutputDebugString(FilePath);
        //OutputDebugString(L"*** *** ***\n");

        handle = CreateFile(FilePath, FILE_READ_DATA, FILE_SHARE_READ,
                            NULL, OPEN_EXISTING, 0, NULL);

        Dll_Free(FilePath);

        if (handle == INVALID_HANDLE_VALUE)
            handle = NULL;

        if (handle) {

            IO_STATUS_BLOCK IoStatusBlock;
            FILE_NETWORK_OPEN_INFORMATION open_info;

            NTSTATUS status = NtQueryInformationFile(
                handle, &IoStatusBlock, &open_info,
                sizeof(FILE_NETWORK_OPEN_INFORMATION),
                FileNetworkOpenInformation);

            if (NT_SUCCESS(status) &&
                            open_info.EndOfFile.QuadPart == FileSize) {

                //WCHAR txt[1024];
                //Sbie_snwprintf(txt, 1024, L"REDIR FILE - %s\n", path);
                //OutputDebugString(txt);

                *out_handle = handle;
                return TRUE;
            }

            CloseHandle(handle);
        }

        SbieApi_Log(2205, FileName + 1);
    }

    return FALSE;
}


//---------------------------------------------------------------------------
// Sxs_CheckCompatLayer
//---------------------------------------------------------------------------


_FX BOOLEAN Sxs_CheckCompatLayer(const WCHAR *DosPath, const WCHAR *Setting)
{
    BOOLEAN HaveSetting = FALSE;

    WCHAR *text = Dll_AllocTemp(1024 * sizeof(WCHAR));

    text[0] = 0;
    GetEnvironmentVariable(L"__COMPAT_LAYER", text, 1020);
    if (text[0]) {
        if (Sxs_CheckCompatLayer_3(text, Setting))
            HaveSetting = TRUE;
    }

    if (! HaveSetting) {

        if (Sxs_CheckCompatLayer_2(DosPath, TRUE, text, 1020)) {
            if (Sxs_CheckCompatLayer_3(text, Setting))
                HaveSetting = TRUE;
        }
    }

    if (! HaveSetting) {

        if (Sxs_CheckCompatLayer_2(DosPath, FALSE, text, 1020)) {
            if (Sxs_CheckCompatLayer_3(text, Setting))
                HaveSetting = TRUE;
        }
    }

    Dll_Free(text);
    return HaveSetting;
}


//---------------------------------------------------------------------------
// Sxs_CheckCompatLayer_2
//---------------------------------------------------------------------------


_FX BOOLEAN Sxs_CheckCompatLayer_2(
    const WCHAR *DosPath, BOOLEAN User, WCHAR *Text, ULONG Length)
{
    extern const WCHAR *Key_Registry;
    extern const WCHAR *Key_UserCurrent;

    UNICODE_STRING objname;
    OBJECT_ATTRIBUTES objattrs;
    HANDLE handle;
    BOOLEAN HaveKey = FALSE;

    wcscpy(Text, Key_Registry);
    if (User)
        wcscat(Text, Key_UserCurrent);
    else
        wcscat(Text, L"\\machine");
    wcscat(Text, L"\\software\\Microsoft\\Windows NT"
                 L"\\CurrentVersion\\AppCompatFlags\\Layers");

    RtlInitUnicodeString(&objname, Text);

    InitializeObjectAttributes(
        &objattrs, &objname, OBJ_CASE_INSENSITIVE, NULL, NULL);

    if (0 == __sys_NtOpenKey(&handle, KEY_READ, &objattrs)) {

        RtlInitUnicodeString(&objname, DosPath);
        Length *= sizeof(WCHAR);

        if (0 == NtQueryValueKey(
                    handle, &objname, KeyValuePartialInformation,
                    Text, Length, &Length)) {

            KEY_VALUE_PARTIAL_INFORMATION *kvpi =
                                (KEY_VALUE_PARTIAL_INFORMATION *)Text;
            Length = kvpi->DataLength / sizeof(WCHAR);
            wmemmove(Text, (WCHAR *)kvpi->Data, Length);
            Text[Length] = L'\0';

            HaveKey = TRUE;
        }

        NtClose(handle);
    }

    return HaveKey;
}


//---------------------------------------------------------------------------
// Sxs_CheckCompatLayer_3
//---------------------------------------------------------------------------


_FX BOOLEAN Sxs_CheckCompatLayer_3(const WCHAR *Text, const WCHAR *Setting)
{
    ULONG SettingLen = wcslen(Setting);
    const WCHAR *TextPtr = Text;
    while (1) {
        while (*TextPtr == L' ')
            ++TextPtr;
        if (! *TextPtr)
            return FALSE;
        if (_wcsnicmp(TextPtr, Setting, SettingLen) == 0
                && (   TextPtr[SettingLen] == L' '
                    || TextPtr[SettingLen] == L'\0')) {
            return TRUE;
        }
        while (*TextPtr != L' ') {
            if (! *TextPtr)
                return FALSE;
            ++TextPtr;
        }
    }
    return FALSE;
}
