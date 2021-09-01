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
// Box Management
//---------------------------------------------------------------------------


#include "box.h"
#include "util.h"
#include "conf.h"
#include "file.h"
#include "process.h"


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------


static BOX *Box_Alloc(POOL *pool, const WCHAR *boxname, ULONG session_id);

static BOOLEAN Box_InitKeys(
    POOL *pool, BOX *box, const WCHAR *sidstring, ULONG session_id);

static BOOLEAN Box_InitConfExpandArgs(POOL *pool, BOX *box);

static BOOLEAN Box_InitPaths(POOL *pool, BOX *box);

static BOOLEAN Box_ExpandString(
    BOX *box, const WCHAR *model, const WCHAR *suffix,
    WCHAR **path, ULONG *path_len);


//---------------------------------------------------------------------------
// Box_IsValidName
//---------------------------------------------------------------------------


_FX BOOLEAN Box_IsValidName(const WCHAR *name)
{
    int i;

    for (i = 0; i < BOXNAME_MAXLEN; ++i) {
        if (! name[i])
            break;
        if (name[i] >= L'0' && name[i] <= L'9')
            continue;
        if (name[i] >= L'A' && name[i] <= L'Z')
            continue;
        if (name[i] >= L'a' && name[i] <= L'z')
            continue;
        if (name[i] == L'_')
            continue;
        return FALSE;
    }
    if (i == 0 || name[i])
        return FALSE;
    return TRUE;
}


//---------------------------------------------------------------------------
// Box_Alloc
//---------------------------------------------------------------------------


_FX BOX *Box_Alloc(POOL *pool, const WCHAR *boxname, ULONG session_id)
{
    BOX *box = Mem_Alloc(pool, sizeof(BOX));
    if (! box) {

        Log_Status_Ex_Session(
            MSG_BOX_CREATE, 0x10, STATUS_INSUFFICIENT_RESOURCES,
            boxname, session_id);
        return NULL;
    }

    memzero(box, sizeof(BOX));

    wcscpy(box->name, boxname);
    box->name_len = (wcslen(box->name) + 1) * sizeof(WCHAR);

    return box;
}


//---------------------------------------------------------------------------
// Box_Free
//---------------------------------------------------------------------------


_FX void Box_Free(BOX *box)
{
    if (box) {
        if (box->sid)
            Mem_Free(box->sid, box->sid_len);
        if (box->expand_args)
            Mem_Free(box->expand_args, sizeof(CONF_EXPAND_ARGS));
        if (box->file_path)
            Mem_Free(box->file_path, box->file_path_len);
        if (box->key_path)
            Mem_Free(box->key_path, box->key_path_len);
        if (box->ipc_path)
            Mem_Free(box->ipc_path, box->ipc_path_len);
        if (box->pipe_path)
            Mem_Free(box->pipe_path, box->pipe_path_len);
        if (box->system_temp_path)
            Mem_Free(box->system_temp_path, box->system_temp_path_len);
        if (box->user_temp_path)
            Mem_Free(box->user_temp_path, box->user_temp_path_len);
        if (box->spooler_directory)
            Mem_Free(box->spooler_directory, box->spooler_directory_len);
        Mem_Free(box, sizeof(BOX));
    }
}


//---------------------------------------------------------------------------
// Box_CreateEx
//---------------------------------------------------------------------------


_FX BOX *Box_CreateEx(
    POOL *pool, const WCHAR *boxname,
    const WCHAR *sidstring, ULONG session_id,
    BOOLEAN init_paths)
{
    BOX *box;

    box = Box_Alloc(pool, boxname, session_id);
    if (! box)
        return NULL;

    if (! Box_InitKeys(pool, box, sidstring, session_id)) {
        Box_Free(box);
        return NULL;
    }

    if (! Box_InitConfExpandArgs(pool, box)) {
        Box_Free(box);
        return NULL;
    }

    if (init_paths) {

        BOOLEAN ok;
        Conf_AdjustUseCount(TRUE);
        ok = Box_InitPaths(pool, box);
        Conf_AdjustUseCount(FALSE);

        if (! ok) {
            Box_Free(box);
            return NULL;
        }
    }

    return box;
}


//---------------------------------------------------------------------------
// Box_Create
//---------------------------------------------------------------------------


_FX BOX *Box_Create(POOL *pool, const WCHAR *boxname, BOOLEAN init_paths)
{
    BOX *box;

    UNICODE_STRING SidString;
    ULONG SessionId;
    NTSTATUS status = Process_GetSidStringAndSessionId(
                        NtCurrentProcess(), NULL, &SidString, &SessionId);

    if (NT_SUCCESS(status)) {

        box = Box_CreateEx(
                pool, boxname, SidString.Buffer, SessionId, init_paths);
        RtlFreeUnicodeString(&SidString);

    } else {

        Log_Status_Ex(MSG_BOX_CREATE, 0x11, status, boxname);
        box = NULL;
    }

    return box;
}


//---------------------------------------------------------------------------
// Box_InitKeys
//---------------------------------------------------------------------------


_FX BOOLEAN Box_InitKeys(
    POOL *pool, BOX *box, const WCHAR *sidstring, ULONG session_id)
{
    //
    // copy sidstring
    //

    box->sid_len = (wcslen(sidstring) + 1) * sizeof(WCHAR);
    box->sid = Mem_Alloc(pool, box->sid_len);
    if (! box->sid) {

        Log_Status_Ex_Session(
            MSG_BOX_CREATE, 0x12, STATUS_INSUFFICIENT_RESOURCES,
            box->name, session_id);
        return FALSE;
    }

    memcpy(box->sid, sidstring, box->sid_len);

    //
    // get Terminal Services Session ID from parameter
    //

    box->session_id = session_id;

    return TRUE;
}


//---------------------------------------------------------------------------
// Box_InitConfExpandArgs
//---------------------------------------------------------------------------


_FX BOOLEAN Box_InitConfExpandArgs(POOL *pool, BOX *box)
{
    box->expand_args = Mem_Alloc(pool, sizeof(CONF_EXPAND_ARGS));
    if (! box->expand_args) {

        Log_Status_Ex_Session(
            MSG_BOX_CREATE, 0x13, STATUS_INSUFFICIENT_RESOURCES,
            box->name, box->session_id);
        return FALSE;
    }

    box->expand_args->pool = pool;
    box->expand_args->sandbox = box->name;
    box->expand_args->sid = box->sid;
    box->expand_args->session = &box->session_id;

    if (! Conf_Expand_UserName(box->expand_args, NULL))
        return FALSE;

    return TRUE;
}


//---------------------------------------------------------------------------
// Box_InitPaths
//---------------------------------------------------------------------------


_FX BOOLEAN Box_InitPaths(POOL *pool, BOX *box)
{
    static const WCHAR *_FileRootPath_Default =
        L"\\??\\%SystemDrive%\\Sandbox\\%USER%\\%SANDBOX%";
    static const WCHAR *_KeyRootPath_Default  =
        L"\\REGISTRY\\USER\\Sandbox_%USER%_%SANDBOX%";
    static const WCHAR *_IpcRootPath_Default  =
        L"\\Sandbox\\%USER%\\%SANDBOX%\\Session_%SESSION%";

    const WCHAR *value;
    WCHAR suffix[80];
    BOOLEAN ok;
    WCHAR *ptr1;
    WCHAR KeyPath[256];

    //
    // get the file path.  if we don't have a FileRootPath setting,
    // we look for BoxRootFolder before reverting to the default.
    // if we find it, we use old-style suffix \Sandbox\BoxName.
    //

    suffix[0] = L'\0';

    value = Conf_Get(box->name, L"FileRootPath", 0);
    if (! value) {

        value = Conf_Get(box->name, L"BoxRootFolder", 0);
        if (value) {
            wcscpy(suffix, Driver_Sandbox);     // L"\\Sandbox"
            wcscat(suffix, L"\\");
            wcscat(suffix, box->name);
        }
    }

    if (! value)
        value = _FileRootPath_Default;

    ok = Box_ExpandString(
        box, value, suffix, &box->file_path, &box->file_path_len);
    if (! ok) {

        Log_Status_Ex_Session(
            MSG_BOX_CREATE, 0x21, STATUS_UNSUCCESSFUL,
            box->name, box->session_id);
        return FALSE;
    }

    //
    // get the key paths
    //

    ok = Box_ExpandString(box, L"%SystemTemp%", L"", &box->system_temp_path, &box->system_temp_path_len);
    ok = Box_ExpandString(box, L"%DefaultSpoolDirectory%", L"", &box->spooler_directory, &box->spooler_directory_len);

    if (!ok) {

        Log_Status_Ex_Session(
            MSG_BOX_CREATE, 0x22, STATUS_UNSUCCESSFUL,
            box->name, box->session_id);
        return FALSE;
    }

    // sometimes we get here without a user temp var being set.  Check first to avoid an error popup.
    RtlStringCbPrintfW(KeyPath, sizeof(KeyPath), L"\\REGISTRY\\USER\\%.184s\\Environment", box->sid);
    if (DoesRegValueExist(RTL_REGISTRY_ABSOLUTE, KeyPath, L"temp"))
        Box_ExpandString(box, L"%temp%", L"", &box->user_temp_path, &box->user_temp_path_len);

    suffix[0] = L'\0';

    value = Conf_Get(box->name, L"KeyRootPath", 0);
    if (! value)
        value = _KeyRootPath_Default;

    ok = Box_ExpandString(
        box, value, suffix, &box->key_path, &box->key_path_len);
    if (! ok) {

        Log_Status_Ex_Session(
            MSG_BOX_CREATE, 0x23, STATUS_UNSUCCESSFUL,
            box->name, box->session_id);
        return FALSE;
    }

    //
    // get the ipc path
    //

    suffix[0] = L'\0';

    value = Conf_Get(box->name, L"IpcRootPath", 0);
    if (! value)
        value = _IpcRootPath_Default;

    ok = Box_ExpandString(
        box, value, suffix, &box->ipc_path, &box->ipc_path_len);
    if (! ok) {

        Log_Status_Ex_Session(
            MSG_BOX_CREATE, 0x24, STATUS_UNSUCCESSFUL,
            box->name, box->session_id);
        return FALSE;
    }

    //
    // translate ipc path into pipe path
    //

    box->pipe_path_len = box->ipc_path_len;
    box->pipe_path = Mem_Alloc(pool, box->pipe_path_len);
    if (! box->pipe_path) {

        Log_Status_Ex_Session(
            MSG_BOX_CREATE, 0x25, STATUS_UNSUCCESSFUL,
            box->name, box->session_id);
        return FALSE;
    }

    memcpy(box->pipe_path, box->ipc_path, box->pipe_path_len);
    ptr1 = box->pipe_path;
    while (*ptr1) {
        WCHAR *ptr2 = wcschr(ptr1, L'\\');
        if (ptr2) {
            ptr1 = ptr2;
            *ptr1 = L'_';
        } else
            ptr1 += wcslen(ptr1);
    }

    return TRUE;
}


//---------------------------------------------------------------------------
// Box_ExpandString
//---------------------------------------------------------------------------


_FX BOOLEAN Box_ExpandString(
    BOX *box, const WCHAR *model, const WCHAR *suffix,
    WCHAR **path, ULONG *path_len)
{
    WCHAR *value1, *value2, *ptr;
    ULONG suffix_len, len;
    BOOLEAN ok = FALSE;

    value1 = Conf_Expand(box->expand_args, model, NULL);
    if (! value1)
        return FALSE;

    suffix_len = wcslen(suffix);
    while (suffix_len && suffix[suffix_len - 1] == L'\\')
        --suffix_len;

    len = (wcslen(value1) + suffix_len + 1) * sizeof(WCHAR);
    value2 = Mem_Alloc(box->expand_args->pool, len);
    if (value2) {

        wcscpy(value2, value1);
        ptr = value2 + wcslen(value2);
        wmemcpy(ptr, suffix, suffix_len);
        ptr += suffix_len;
        *ptr = L'\0';

        *path = value2;
        *path_len = len;

        //
        // remove duplicate backslashes and the final backslash
        //

        ptr = value2;
        len = wcslen(ptr);

        while (ptr[0]) {
            if (ptr[0] == L'\\' && ptr[1] == L'\\') {

                ULONG move_len = len - (ULONG)(ptr - value2) + 1;
                wmemmove(ptr, ptr + 1, move_len);
                --len;

            } else
                ++ptr;
        }

        if (len && value2[len - 1] == L'\\') {
            value2[len - 1] = L'\0';
            --len;
        }

        //
        // if removal of the backslash caused a change in the length
        // of the string, then re-allocate the output buffer
        //

        if (len) {

            len = (len + 1) * sizeof(WCHAR);
            if (len != *path_len) {

                WCHAR *value3 = Mem_Alloc(box->expand_args->pool, len);
                if (value3) {

                    memcpy(value3, value2, len);
                    Mem_Free(*path, *path_len);

                    *path = value3;
                    *path_len = len;

                    ok = TRUE;
                }

            } else
                ok = TRUE;
        }

        if (! ok) {
            Mem_Free(*path, *path_len);
            *path = NULL;
            *path_len = 0;
        }
    }

    Mem_FreeString(value1);
    return ok;
}


//---------------------------------------------------------------------------
// Box_Clone
//---------------------------------------------------------------------------


_FX BOX *Box_Clone(POOL *pool, const BOX *model)
{
    BOX *box;

    box = Box_Alloc(pool, model->name, model->session_id);
    if (! box)
        return NULL;

#define CLONE_MEMBER(m)                                                 \
    if (model->m) {                                                     \
        box->m = Mem_Alloc(pool, model->m##_len);                       \
        if (! box->m) {                                                 \
            Log_Status_Ex_Session(                                      \
                MSG_BOX_CREATE, 0x20, STATUS_INSUFFICIENT_RESOURCES,    \
                model->name, model->session_id);                        \
            Box_Free(box);                                              \
            return NULL;                                                \
        }                                                               \
        wcscpy(box->m, model->m);                                       \
        box->m##_len = model->m##_len;                                  \
    }

    CLONE_MEMBER(sid);
    CLONE_MEMBER(file_path);
    CLONE_MEMBER(key_path);
    CLONE_MEMBER(ipc_path);
    CLONE_MEMBER(pipe_path);
    CLONE_MEMBER(spooler_directory);
    CLONE_MEMBER(system_temp_path);
    CLONE_MEMBER(user_temp_path);

#undef CLONE_MEMBER

    box->session_id = model->session_id;

    if (! Box_InitConfExpandArgs(pool, box)) {
        Box_Free(box);
        return NULL;
    }

    return box;
}


//---------------------------------------------------------------------------
// Box_NlsStrCmp
//---------------------------------------------------------------------------


_FX int Box_NlsStrCmp(const WCHAR *s1, const WCHAR *s2, ULONG len)
{
    UNICODE_STRING u1, u2;

    u1.Length = u1.MaximumLength = u2.Length = u2.MaximumLength =
        (USHORT)(len * sizeof(WCHAR));
    u1.Buffer = (WCHAR *)s1;
    u2.Buffer = (WCHAR *)s2;

    return RtlCompareUnicodeString(&u1, &u2, TRUE);
}


//---------------------------------------------------------------------------
// Box_IsBoxedPath_Helper
//---------------------------------------------------------------------------


_FX BOOLEAN Box_IsBoxedPath_Helper(
    UNICODE_STRING *uni, const WCHAR *box_path, ULONG box_path_len)
{
    box_path_len -= sizeof(WCHAR);      // remove count of final NULL WCHAR
    if (uni->Length < box_path_len)
        return FALSE;

    box_path_len /= sizeof(WCHAR);      // convert byte count to WCHAR count
    if (Box_NlsStrCmp(uni->Buffer, box_path, box_path_len) != 0)
        return FALSE;

    if (uni->Buffer[box_path_len] != L'\\'
            && uni->Buffer[box_path_len] != L'\0')
        return FALSE;

    return TRUE;
}
