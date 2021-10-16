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
// Environment Handling for Start.exe
//---------------------------------------------------------------------------

#include "stdafx.h"

#include "common/defines.h"
#include "core/dll/sbiedll.h"
#include "common/win32_ntddk.h"


//---------------------------------------------------------------------------
// Structures and Type
//---------------------------------------------------------------------------


typedef struct _ENV_VAR {

    struct _ENV_VAR *Next;
    BOOLEAN IsSystem;
    BOOLEAN IsExpand;
    WCHAR *Name;
    WCHAR *Data;
    WCHAR Space[1];

} ENV_VAR;


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------


static void Env_DoRefresh_1_Common(
    const WCHAR *BoxName, const WCHAR *SubKeyPath, BOOLEAN IsSystem);

static void Env_DoRefresh_1(const WCHAR *BoxName);

static WCHAR *Env_Expand(
    const WCHAR *EnvName, WCHAR *OldText, BOOLEAN IsSystem);

static void Env_DoRefresh_2(void);

static void Env_DoRefresh_3(void);


//---------------------------------------------------------------------------
// Variables
//---------------------------------------------------------------------------


static ENV_VAR *Env_Vars = NULL;


//---------------------------------------------------------------------------
// Env_DoRefresh_1_Common
//---------------------------------------------------------------------------


_FX void Env_DoRefresh_1_Common(
    const WCHAR *BoxName, const WCHAR *SubKeyPath, BOOLEAN IsSystem)
{
    NTSTATUS status;
    ULONG len1 = 0, len2 = 0, len3 = 0;
    WCHAR *path;
    HKEY hkey;
    WCHAR name[100];
    ULONG name_len;
    ULONG value_type;
    ULONG value_len;
    ULONG index;
    ENV_VAR *env;
    WCHAR *ptr;

    //
    // open a registry key inside the sandbox:
    //      MACHINE\SYSTEM\CurrentControlSet\
    //      Control\Session Manager\Environment
    //

    status = SbieApi_QueryBoxPath(
                BoxName, NULL, NULL, NULL, &len1, &len2, &len3);
    if (status != 0)
        return;

    path = (WCHAR*)HeapAlloc(GetProcessHeap(), 0, len2 + 256);
    if (! path)
        return;

    status = SbieApi_QueryBoxPath(
                BoxName, NULL, path, NULL, &len1, &len2, &len3);
    if (status == 0) {

        UNICODE_STRING objname;
        OBJECT_ATTRIBUTES objattrs;

        wcscat(path, SubKeyPath);
        wcscat(path, L"\\Environment");

        RtlInitUnicodeString(&objname, path);

        InitializeObjectAttributes(
            &objattrs, &objname, OBJ_CASE_INSENSITIVE, NULL, NULL);

        status = NtOpenKey((PHANDLE)&hkey, KEY_QUERY_VALUE, &objattrs);
    }

    HeapFree(GetProcessHeap(), 0, path);

    if (status != 0)
        return;

    //
    // collect values in the key
    //

    for (index = 0; ; ++index) {

        name_len = 99;
        value_len = 0;
        status = RegEnumValue(hkey, index, name, &name_len,
                              NULL, &value_type, NULL, &value_len);

        if (status == ERROR_MORE_DATA)
            status = ERROR_SUCCESS;
        if (status != ERROR_SUCCESS)
            break;
        if (value_type != REG_SZ && value_type != REG_EXPAND_SZ)
            continue;

        len1 = sizeof(ENV_VAR)
             + name_len * sizeof(WCHAR)
             + value_len
             + 8;
        env = (ENV_VAR*)HeapAlloc(GetProcessHeap(), 0, len1);
        if (! env)
            break;

        env->IsSystem = IsSystem;
        env->IsExpand =
                    (value_type == REG_SZ || value_type == REG_EXPAND_SZ);

        wmemcpy(env->Space, name, name_len);
        ptr = env->Space + name_len;
        *ptr = L'\0';
        ++ptr;

        name_len = 99;
        status = RegEnumValue(hkey, index, name, &name_len,
                              NULL, &value_type, (LPBYTE)ptr, &value_len);

        if (status != 0) {
            HeapFree(GetProcessHeap(), 0, env);
            break;
        }

        memzero(((UCHAR *)ptr) + value_len, 4);

        env->Next = Env_Vars;
        env->Name = env->Space;
        env->Data = ptr;

        Env_Vars = env;
    }

    //
    // finish
    //

    NtClose(hkey);
}


//---------------------------------------------------------------------------
// Env_DoRefresh_1
//---------------------------------------------------------------------------


_FX void Env_DoRefresh_1(const WCHAR *BoxName)
{
    static const WCHAR *_HKCU_Path = L"\\USER\\CURRENT";
    static const WCHAR *_HKLM_Path = L"\\MACHINE\\SYSTEM"
                        L"\\CurrentControlSet\\Control\\Session Manager";

    //
    // we scan USER variables before SYSTEM variables because the
    // scan adds variables in LIFO order, and we want USER variables
    // to override SYSTEM variables
    //

    Env_DoRefresh_1_Common(BoxName, _HKCU_Path, FALSE);
    Env_DoRefresh_1_Common(BoxName, _HKLM_Path, TRUE);
}


//---------------------------------------------------------------------------
// Env_Expand
//---------------------------------------------------------------------------


_FX WCHAR *Env_Expand(const WCHAR *EnvName, WCHAR *OldText, BOOLEAN IsSystem)
{
    WCHAR *PtrName, *ptr2;
    ENV_VAR *env;
    WCHAR *EnvData;
    BOOL FreeEnvData;
    BOOL FoundEnvData;
    ULONG len;
    WCHAR *NewText;

    //
    // find variable name between two percent signs
    //

    PtrName = wcschr(OldText, L'%');
    if (! PtrName)
        return OldText;
    ++PtrName;

    ptr2 = wcschr(PtrName, L'%');
    if (! ptr2)
        return OldText;
    if (ptr2 == PtrName)
        return OldText;

    //
    // if name is the same as the variable we're expanding,
    // i.e. self-reference recursion, then replace by an empty string
    //

    *ptr2 = L'\0';

    EnvData = L"";
    FreeEnvData = FALSE;
    FoundEnvData = FALSE;

    if (_wcsicmp(PtrName, EnvName) != 0) {

        //
        // locate variable in cached environment from the sandbox.
        //
        // a SYSTEM variable can only be expanded using other SYSTEM
        // variables, but not a USER variable
        //
        // we don't break on first match, because SYSTEM environment
        // variables are listed before USER variables, and we want to
        // match on a USER variable if possible
        //

        for (env = Env_Vars; env; env = env->Next) {

            if (IsSystem && (! env->IsSystem))
                continue;

            if (_wcsicmp(PtrName, env->Name) == 0) {
                EnvData = env->Data;
                FoundEnvData = TRUE;
            }
        }

        //
        // if not found, try in real environment
        //

        if (! FoundEnvData) {

            len = GetEnvironmentVariable(PtrName, NULL, 0);
            if (len) {
                WCHAR *buf = (WCHAR*)HeapAlloc(
                    GetProcessHeap(), 0, (len + 2) * sizeof(WCHAR));
                if (buf) {
                    GetEnvironmentVariable(PtrName, buf, len);
                    EnvData = buf;
                    FreeEnvData = TRUE;
                }
            }
        }
    }

    *ptr2 = L'%';

    //
    // allocate a new string
    //

    len = wcslen(OldText) + wcslen(EnvData) + 4;
    NewText = (WCHAR*)HeapAlloc(GetProcessHeap(), 0, len * sizeof(WCHAR));
    if (! NewText)
        return OldText;

    --PtrName;
    *PtrName = L'\0';
    wcscpy(NewText, OldText);
    *PtrName = L'%';

    wcscat(NewText, EnvData);

    wcscat(NewText, ptr2 + 1);

    if (FreeEnvData)
        HeapFree(GetProcessHeap(), 0, EnvData);

    return Env_Expand(EnvName, NewText, IsSystem);
}


//---------------------------------------------------------------------------
// Env_DoRefresh_2
//---------------------------------------------------------------------------


_FX void Env_DoRefresh_2(void)
{
    ENV_VAR *env;
    WCHAR *ExpandedData;

    for (env = Env_Vars; env; env = env->Next) {

        if (_wcsicmp(env->Name, L"USERNAME") == 0)      // don't overwrite USERNAME
            continue;

        if (env->IsExpand) {

            ExpandedData = Env_Expand(env->Name, env->Data, env->IsSystem);
            if (! ExpandedData)
                continue;

        } else
            ExpandedData = env->Data;

        SetEnvironmentVariable(env->Name, ExpandedData);

        if (ExpandedData != env->Data)
            HeapFree(GetProcessHeap(), 0, ExpandedData);
    }
}


//---------------------------------------------------------------------------
// Env_DoRefresh_3
//---------------------------------------------------------------------------


_FX void Env_DoRefresh_3(void)
{
    ENV_VAR *UsrPath = NULL;
    ENV_VAR *SysPath = NULL;
    ENV_VAR *env;

    for (env = Env_Vars; env; env = env->Next) {

        if ((*env->Name == L'P' || *env->Name == L'p') &&
                                _wcsicmp(env->Name, L"PATH") == 0) {
            if (env->IsSystem)
                SysPath = env;
            else
                UsrPath = env;
        }
    }

    if (SysPath && UsrPath) {

        WCHAR *ExpSysPath =
                Env_Expand(SysPath->Name, SysPath->Data, SysPath->IsSystem);
        WCHAR *ExpUsrPath =
                Env_Expand(UsrPath->Name, UsrPath->Data, UsrPath->IsSystem);

        if (ExpSysPath && ExpUsrPath) {

            ULONG SysLen = wcslen(ExpSysPath);
            ULONG UsrLen = wcslen(ExpUsrPath);
            WCHAR *CombinedPath = (WCHAR*)HeapAlloc(GetProcessHeap(), 0,
                                    (SysLen + UsrLen + 8) * sizeof(WCHAR));

            if (CombinedPath) {

                wmemcpy(CombinedPath, ExpSysPath, SysLen);
                CombinedPath[SysLen] = L';';
                wmemcpy(CombinedPath + SysLen + 1, ExpUsrPath, UsrLen + 1);

                SetEnvironmentVariable(SysPath->Name, CombinedPath);

                HeapFree(GetProcessHeap(), 0, CombinedPath);
            }

            if (ExpUsrPath != UsrPath->Data)
                HeapFree(GetProcessHeap(), 0, ExpUsrPath);
            if (ExpSysPath != SysPath->Data)
                HeapFree(GetProcessHeap(), 0, ExpSysPath);
        }
    }
}


//---------------------------------------------------------------------------
// Env_Refresh
//---------------------------------------------------------------------------


_FX void Env_Refresh(const WCHAR *BoxName)
{
    WCHAR BoxNameSpace[34];

    if (BoxName[0] == L'-') {

        HANDLE pid = (HANDLE)(ULONG_PTR)_wtoi(BoxName + 1);
        LONG rc = SbieApi_QueryProcess(pid, BoxNameSpace, NULL, NULL, NULL);
        if (rc != 0)
            return;
        BoxName = BoxNameSpace;
    }

    Env_DoRefresh_1(BoxName);

    Env_DoRefresh_2();

    Env_DoRefresh_3();
}
