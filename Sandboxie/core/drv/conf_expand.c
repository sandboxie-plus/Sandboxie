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
// Configuration (Expand Configuration Data)
//---------------------------------------------------------------------------


#include <stdlib.h>         // itow
#include "conf.h"
#include "file.h"
#include "util.h"


//---------------------------------------------------------------------------
// Defines
//---------------------------------------------------------------------------


#define RTL_REGISTRY_MACHINE        1234


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------


static BOOLEAN Conf_Expand_Override(
    CONF_EXPAND_ARGS *args, const WCHAR *Prefix, const WCHAR *ValueName,
    WCHAR *PageSizeBuffer);

static NTSTATUS Conf_Expand_RegValue(
    CONF_EXPAND_ARGS *args,
    ULONG RootKey, const WCHAR *SubKeyPath, const WCHAR *ValueName,
    WCHAR *PageSizeBuffer);

static NTSTATUS Conf_Expand_Template(
    CONF_EXPAND_ARGS *args, const WCHAR *varname, WCHAR *varvalue);

static WCHAR *Conf_Expand_Helper(
    CONF_EXPAND_ARGS *args, const WCHAR *model_value, WCHAR *varvalue);

static WCHAR *Conf_Expand_2(
    CONF_EXPAND_ARGS *args, const WCHAR *model_value);


//---------------------------------------------------------------------------
// Variables
//---------------------------------------------------------------------------


extern const WCHAR *Conf_TemplateSettings;
extern const WCHAR *Conf_Tmpl;


//---------------------------------------------------------------------------
// Conf_Expand_Override
//---------------------------------------------------------------------------


_FX BOOLEAN Conf_Expand_Override(
    CONF_EXPAND_ARGS *args, const WCHAR *Prefix, const WCHAR *ValueName,
    WCHAR *PageSizeBuffer)
{
    ULONG IniName_len;
    WCHAR *IniName;
    const WCHAR *IniValue;

    BOOLEAN Found = FALSE;

    IniName_len = (wcslen(Prefix) + wcslen(ValueName) + 4) * sizeof(WCHAR);
    IniName = Mem_Alloc(args->pool, IniName_len);
    if (IniName) {

        wcscpy(IniName, Prefix);
        wcscat(IniName, ValueName);

        Conf_AdjustUseCount(TRUE);

        IniValue = Conf_Get(args->sandbox, IniName, 0);
        if (IniValue) {

            ULONG IniValue_len = wcslen(IniValue);
            if (IniValue_len > 1024)
                IniValue_len = 1024;
            wmemcpy(PageSizeBuffer, IniValue, IniValue_len);
            PageSizeBuffer[IniValue_len] = L'\0';

            //DbgPrint("Override Found %S = %S\n", IniName, PageSizeBuffer);

            Found = TRUE;
        }

        Conf_AdjustUseCount(FALSE);
        Mem_Free(IniName, IniName_len);
    }

    return Found;
}


//---------------------------------------------------------------------------
// Conf_Expand_RegValue
//---------------------------------------------------------------------------


_FX NTSTATUS Conf_Expand_RegValue(
    CONF_EXPAND_ARGS *args,
    ULONG RootKey, const WCHAR *SubKeyPath, const WCHAR *ValueName,
    WCHAR *PageSizeBuffer)
{
    NTSTATUS status;
    //RTL_QUERY_REGISTRY_TABLE qrt[2];
    UNICODE_STRING uni;
    WCHAR *KeyPath;
    ULONG KeyPath_len;

    memzero(PageSizeBuffer, sizeof(WCHAR) * (1024 + 4));

    uni.Length = 0;
    uni.MaximumLength = sizeof(WCHAR) * (1024 + 1); // only half the PageSizeBuffer
    uni.Buffer = PageSizeBuffer;

    //memzero(qrt, sizeof(qrt));
    //qrt[0].Flags =  RTL_QUERY_REGISTRY_REQUIRED |
    //                RTL_QUERY_REGISTRY_TYPECHECK | 
    //                RTL_QUERY_REGISTRY_NOEXPAND;
    //qrt[0].Name = (WCHAR *)ValueName;
    //qrt[0].EntryContext = &uni;
    //qrt[0].DefaultType = REG_NONE;

    if (RootKey == RTL_REGISTRY_USER) {

        KeyPath_len = (32 + wcslen(args->sid) + wcslen(SubKeyPath))
                    * sizeof(WCHAR);
        KeyPath = Mem_Alloc(args->pool, KeyPath_len);
        if (! KeyPath)
            return STATUS_INSUFFICIENT_RESOURCES;

        wcscpy(KeyPath, L"\\REGISTRY\\USER\\");
        wcscat(KeyPath, args->sid);
        wcscat(KeyPath, L"\\");
        wcscat(KeyPath, SubKeyPath);

        RootKey = RTL_REGISTRY_ABSOLUTE;

    } else if (RootKey == RTL_REGISTRY_MACHINE) {

        KeyPath_len = (32 + wcslen(SubKeyPath))
                    * sizeof(WCHAR);
        KeyPath = Mem_Alloc(args->pool, KeyPath_len);
        if (! KeyPath)
            return STATUS_INSUFFICIENT_RESOURCES;

        wcscpy(KeyPath, Key_Registry_Machine);
        wcscat(KeyPath, L"\\");
        wcscat(KeyPath, SubKeyPath);

        RootKey = RTL_REGISTRY_ABSOLUTE;

    } else {

        KeyPath_len = 0;
        KeyPath = (WCHAR *)SubKeyPath;
    }

    //status = RtlQueryRegistryValues(RootKey, KeyPath, qrt, NULL, NULL);
    status = GetRegString(RootKey, KeyPath, ValueName, &uni);

    if (status == STATUS_SUCCESS) {

        //
        // if the result value references itself, then assume the
        // registry value was not found
        //

        ULONG ValueName_len = wcslen(ValueName);

        WCHAR *ptr2, *ptr1 = PageSizeBuffer;
        while (1) {
            ptr1 = wcschr(ptr1, L'%');
            if (! ptr1)
                break;
            ++ptr1;
            ptr2 = wcschr(ptr1, L'%');
            if (! ptr2)
                break;

            //DbgPrint("Value <%S> Ptr1=%d Ptr2=%d\n", PageSizeBuffer, ptr1-PageSizeBuffer, ptr2-PageSizeBuffer);

            if (ptr2 - ptr1 == ValueName_len &&
                _wcsnicmp(ptr1, ValueName, ValueName_len) == 0) {

                //DbgPrint("Self reference in <%S>\n", PageSizeBuffer);
                status = STATUS_OBJECT_NAME_NOT_FOUND;
                break;
            }

            ptr1 = ptr2;
        }

    }

    if (status == STATUS_OBJECT_NAME_NOT_FOUND ||
        status == STATUS_OBJECT_PATH_NOT_FOUND) {

        //
        // if we could not find the registry value, check if
        // the configuration file has a fallback value
        //

        if (Conf_Expand_Override(args, L"Reg.", ValueName, PageSizeBuffer))
            status = STATUS_SUCCESS;
    }

    if (KeyPath_len)
        Mem_Free(KeyPath, KeyPath_len);

    return status;
}


//---------------------------------------------------------------------------
// Conf_Expand_Template
//---------------------------------------------------------------------------


_FX NTSTATUS Conf_Expand_Template(
    CONF_EXPAND_ARGS *args, const WCHAR *varname, WCHAR *varvalue)
{
    ULONG varname2_len;
    WCHAR *varname2;
    const WCHAR *IniValue;

    Conf_Expand_UserName(args, varvalue);
    varname2_len = (wcslen(varname) + wcslen(varvalue) + 8) * sizeof(WCHAR);
    varname2 = Mem_Alloc(args->pool, varname2_len);
    if (! varname2)
        return STATUS_INSUFFICIENT_RESOURCES;
    RtlStringCbPrintfW(varname2, varname2_len, L"%s.%s", varname, varvalue);

    Conf_AdjustUseCount(TRUE);

    IniValue = Conf_Get(Conf_TemplateSettings, varname2, 0);

    if (IniValue && IniValue[0] == L'?' && IniValue[1] == L'\0')
        IniValue = NULL;
    if (! IniValue) {
        IniValue = Conf_Get(Conf_TemplateSettings, varname, 0);
        if (IniValue && IniValue[0] == L'?' && IniValue[1] == L'\0')
            IniValue = NULL;
    }

    if (IniValue) {

        ULONG IniValue_len = wcslen(IniValue);
        if (IniValue_len > 1024)
            IniValue_len = 1024;
        wmemcpy(varvalue, IniValue, IniValue_len);
        varvalue[IniValue_len] = L'\0';
    }

    Conf_AdjustUseCount(FALSE);
    Mem_Free(varname2, varname2_len);

    if (! IniValue)
        return STATUS_OBJECT_NAME_NOT_FOUND;

    return STATUS_SUCCESS;
}


//---------------------------------------------------------------------------
// Conf_Expand_Helper
//---------------------------------------------------------------------------


_FX WCHAR *Conf_Expand_Helper(
    CONF_EXPAND_ARGS *args, const WCHAR *model_value, WCHAR *varvalue)
{
    static const WCHAR *_Registry_User = L"\\REGISTRY\\USER\\%SID%";
    static const WCHAR *_Windows =
        L"\\REGISTRY\\MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion";
    static const WCHAR *_ProgramFiles = L"ProgramFiles";
    static const WCHAR *_ProgramFilesDir = L"ProgramFilesDir";
    static const WCHAR *_ProgramFiles32 = L"ProgramFiles(x86)";
    static const WCHAR *_ProgramFilesDir32 = L"ProgramFilesDir (x86)";
    static const WCHAR *_CommonProgramFiles = L"CommonProgramFiles";
    static const WCHAR *_CommonProgramFilesDir = L"CommonFilesDir";
    static const WCHAR *_CommonProgramFiles32 = L"CommonProgramFiles(x86)";
    static const WCHAR *_CommonProgramFilesDir32 = L"CommonFilesDir (x86)";
    static const WCHAR *_SystemRoot = L"SystemRoot";
    static const WCHAR *_ProfileList = L"ProfileList";
    static const WCHAR *_homedrive = L"homedrive";
    static const WCHAR *_homepath  = L"homepath";
    static const WCHAR *_homeshare = L"homeshare";
    static const WCHAR *_temp = L"temp";
    static const WCHAR *_tmp = L"tmp";
    static const WCHAR *_SystemTemp = L"SystemTemp";
    static const WCHAR *_VolatileEnvironment = L"Volatile Environment";
    static const WCHAR *_Environment = L"Environment";
    static const WCHAR *_DefaultSpoolDirectoryPrefix =
        L"%SystemRoot%\\System32\\spool\\";
    NTSTATUS status;
    const WCHAR *ptr1, *ptr2 = L"";
    WCHAR varname[66];
    ULONG len;
    WCHAR *buf, *ptr;

    //
    // if string begins with HKEY_, translate the user-mode registry prefix
    //

    if (_wcsnicmp(model_value, L"HKEY_", 5) == 0) {

        ptr1 = model_value + 5;
        if (_wcsnicmp(ptr1, L"LOCAL_MACHINE", 13) == 0) {
            ptr1 += 13;
            ptr2 = Key_Registry_Machine;
        } else if (_wcsnicmp(ptr1, L"CURRENT_USER", 12) == 0) {
            ptr1 += 12;
            ptr2 = _Registry_User;
        } else
            ptr1 = NULL;

        if (ptr1 && (*ptr1 == L'\\' || *ptr1 == L'_' || *ptr1 == L'\0')) {

            len = (wcslen(ptr2) + wcslen(ptr1) + 1) * sizeof(WCHAR);
            buf = Mem_Alloc(args->pool, len);
            if (buf) {
                wcscpy(buf, ptr2);
                wcscat(buf, ptr1);
            } else {
                status = STATUS_INSUFFICIENT_RESOURCES;
                Log_Status_Ex(MSG_CONF_EXPAND, 0, status, ptr2);
            }

            return buf;
        }
    }

    //
    // if string is a DOS style path name, translate to NT style
    //

    status = File_TranslateDosToNt(model_value, args->pool, &ptr, &len);

    if (NT_SUCCESS(status)) {

        buf = Mem_AllocString(args->pool, ptr);
        Mem_Free(ptr, len);
        return buf;

    } else if (status != STATUS_OBJECT_PATH_SYNTAX_BAD) {
        Log_Status_Ex(MSG_CONF_EXPAND, 0, status, L"(DOS path)");
        return NULL;
    }

    //
    // find two percent signs separated by at least one character,
    // they bind the name of an expansion variable
    //

    ptr1 = model_value;
    ptr2 = NULL;

    while (1) {

        ptr1 = wcschr(ptr1, L'%');
        if (ptr1)
            ptr2 = wcschr(ptr1 + 1, L'%');

        if ((! ptr1) || (! ptr2))
            return (WCHAR *)model_value;

        if (ptr2 == ptr1 + 1)
            ptr1 = ptr2;
        else
            break;
    }

    //
    // copy variable name between two percent signs into buffer
    //

    len = (ULONG)((ptr2 - 1) - (ptr1 + 1) + 1);
    if (len > sizeof(varname) / sizeof(WCHAR) - 1)
        len = sizeof(varname) / sizeof(WCHAR) - 1;
    memzero(varname, sizeof(varname));
    wcsncpy(varname, ptr1 + 1, len);

    //
    // lookup register variables given in CONF_EXPAND_ARGS
    //

    status = STATUS_SUCCESS;

    if (Conf_Expand_Override(args, L"Ovr.", varname, varvalue)) {

        // do nothing; value already placed in buffer

    } else if (_wcsicmp(varname, L"sandbox") == 0) {

        wcscpy(varvalue, args->sandbox);

    } else if (_wcsicmp(varname, L"SbieHome") == 0) {

        wcscpy(varvalue, Driver_HomePathNt);

    } else if (_wcsicmp(varname, L"sid") == 0) {

        wcscpy(varvalue, args->sid);

    } else if (_wcsicmp(varname, L"user") == 0 ||
               _wcsicmp(varname, L"username") == 0) {

        Conf_Expand_UserName(args, varvalue);

    } else if (_wcsicmp(varname, L"session") == 0) {

        _itow(*args->session, varvalue, 10);

    } else if (_wcsicmp(varname, _ProgramFiles) == 0) {

        status = Conf_Expand_RegValue(args,
            RTL_REGISTRY_ABSOLUTE, _Windows, _ProgramFilesDir, varvalue);

    } else if (_wcsicmp(varname, _ProgramFiles32) == 0) {

        status = Conf_Expand_RegValue(args,
            RTL_REGISTRY_ABSOLUTE, _Windows, _ProgramFilesDir32, varvalue);

    } else if (_wcsicmp(varname, _CommonProgramFiles) == 0) {

        status = Conf_Expand_RegValue(args,
            RTL_REGISTRY_ABSOLUTE, _Windows, _CommonProgramFilesDir, varvalue);

    } else if (_wcsicmp(varname, _CommonProgramFiles32) == 0) {

        status = Conf_Expand_RegValue(args,
            RTL_REGISTRY_ABSOLUTE, _Windows, _CommonProgramFilesDir32, varvalue);

    } else if (_wcsicmp(varname, _SystemRoot) == 0) {

        status = Conf_Expand_RegValue(args,
            RTL_REGISTRY_WINDOWS_NT, Driver_Empty, varname, varvalue);

    } else if (_wcsicmp(varname, L"SystemDrive") == 0) {

        status = Conf_Expand_RegValue(args,
            RTL_REGISTRY_WINDOWS_NT, Driver_Empty, _SystemRoot, varvalue);

        if (NT_SUCCESS(status)) {
            if (varvalue[1] == L':')
                varvalue[2] = L'\0';
            else
                status = STATUS_OBJECT_NAME_INVALID;
        }

    } else if (_wcsicmp(varname, L"DefaultSpoolDirectory") == 0) {

        status = Conf_Expand_RegValue(args,
            RTL_REGISTRY_CONTROL, L"Print\\Printers", varname, varvalue);

        if (status == STATUS_OBJECT_NAME_NOT_FOUND ||
            status == STATUS_OBJECT_PATH_NOT_FOUND) {

            wcscpy(varvalue, _DefaultSpoolDirectoryPrefix);
            wcscat(varvalue, L"PRINTERS");
            status = STATUS_SUCCESS;
        }

    } else if (_wcsicmp(varname, L"DefaultSpoolDirectory2") == 0) {

        wcscpy(varvalue, _DefaultSpoolDirectoryPrefix);
        wcscat(varvalue, L"SERVERS");

    } else if (_wcsicmp(varname, L"UserProfile") == 0) {

        ULONG KeyNameLen = (wcslen(args->sid) + 32) * sizeof(WCHAR);
        WCHAR *KeyName = Mem_Alloc(args->pool, KeyNameLen);
        if (! KeyName)
            status = STATUS_INSUFFICIENT_RESOURCES;
        else {

            wcscpy(KeyName, _ProfileList);
            wcscat(KeyName, L"\\");
            wcscat(KeyName, args->sid);

            status = Conf_Expand_RegValue(args,
                RTL_REGISTRY_WINDOWS_NT, KeyName,
                L"ProfileImagePath", varvalue);

            Mem_Free(KeyName, KeyNameLen);
        }

    } else if (_wcsicmp(varname, L"AllUsersProfile") == 0 ||
               _wcsicmp(varname, L"ProgramData") == 0) {

        //
        // on Windows Vista, "ProgramData" replaces "AllUsersProfile"
        // and contains a full path
        //

        status = Conf_Expand_RegValue(args,
            RTL_REGISTRY_WINDOWS_NT, _ProfileList,
            L"ProgramData", varvalue);

        if (status == STATUS_OBJECT_NAME_NOT_FOUND) {

            //
            // On Windows XP, we have to query "ProfilesDirectory"
            // and append the suffix in "AllUsersProfile"
            //

            status = Conf_Expand_RegValue(args,
                RTL_REGISTRY_WINDOWS_NT, _ProfileList,
                L"ProfilesDirectory", varvalue);

            if (NT_SUCCESS(status)) {

                ULONG PrefixLen = wcslen(varvalue);
                if (PrefixLen && varvalue[PrefixLen - 1] != L'\\') {
                    varvalue[PrefixLen] = L'\\';
                    ++PrefixLen;
                    varvalue[PrefixLen] = L'\0';
                }

                status = Conf_Expand_RegValue(args,
                    RTL_REGISTRY_WINDOWS_NT, _ProfileList,
                    L"AllUsersProfile", varvalue + PrefixLen);
            }
        }

    } else if (_wcsicmp(varname, L"PUBLIC") == 0) {

        wcscpy(varvalue, L"%SystemDrive%\\Users\\Public");

    //
    // environment variables from HKCU\Volatile Environment
    //

    } else if (_wcsicmp(varname, _homedrive) == 0 ||
               _wcsicmp(varname, _homepath) == 0 ||
               _wcsicmp(varname, _homeshare) == 0) {

        status = Conf_Expand_RegValue(args,
            RTL_REGISTRY_USER, _VolatileEnvironment, varname, varvalue);

    //
    // "temp" environment variables from HKCU\Environment
    //

    } else if (_wcsicmp(varname, _temp) == 0 ||
               _wcsicmp(varname, _tmp) == 0) {

        status = Conf_Expand_RegValue(args,
            RTL_REGISTRY_USER, _Environment, _temp, varvalue);

        if (status != STATUS_SUCCESS) {

            status = Conf_Expand_RegValue(args,
                RTL_REGISTRY_USER, _Environment, _tmp, varvalue);
        }
    }
    else if (_wcsicmp(varname, _SystemTemp) == 0) {

        status = Conf_Expand_RegValue(args,
            RTL_REGISTRY_CONTROL, L"Session Manager\\Environment", L"Temp", varvalue);

    //
    // template variables
    //

    } else if (_wcsnicmp(varname, Conf_Tmpl, 5) == 0) {

        status = Conf_Expand_Template(args, varname, varvalue);

    //
    // folder names from the (User) Shell Folders registry key
    //

    } else {

        static const WCHAR *User_Shell_Folders_Key =
            L"Software\\Microsoft\\Windows\\CurrentVersion\\"
            L"Explorer\\User Shell Folders";
        static const WCHAR *Shell_Folders_Key =
            L"Software\\Microsoft\\Windows\\CurrentVersion\\"
            L"Explorer\\Shell Folders";

        //
        // according to TechNet, User Shell Folders take precedence
        // over Shell Folders
        //

        if (_wcsicmp(varname, L"LocalAppData") == 0)
            wcscpy(varname, L"Local AppData");

        status = Conf_Expand_RegValue(args,
            RTL_REGISTRY_USER, User_Shell_Folders_Key, varname, varvalue);

        if (status == STATUS_OBJECT_NAME_NOT_FOUND ||
            status == STATUS_OBJECT_PATH_NOT_FOUND) {

            status = Conf_Expand_RegValue(args,
                RTL_REGISTRY_USER, Shell_Folders_Key, varname, varvalue);
        }

        if (status == STATUS_OBJECT_NAME_NOT_FOUND ||
            status == STATUS_OBJECT_PATH_NOT_FOUND) {

            status = Conf_Expand_RegValue(args,
                RTL_REGISTRY_MACHINE, User_Shell_Folders_Key,
                varname, varvalue);
        }

        if (status == STATUS_OBJECT_NAME_NOT_FOUND ||
            status == STATUS_OBJECT_PATH_NOT_FOUND) {

            status = Conf_Expand_RegValue(args,
                RTL_REGISTRY_MACHINE, Shell_Folders_Key,
                varname, varvalue);
        }
    }

    if (! NT_SUCCESS(status)) {
        Log_Status_Ex(MSG_CONF_EXPAND, 0, status, varname);
        return NULL;                // variable expansion failed
    }

    //if (_wcsicmp(varname, L"appdata") == 0) {
    //    DbgPrint("Conf_Expand: <%S> replaced with <%S>\n", varname, varvalue);
    //}

    //
    // need to concatenate three strings into output buffer
    //   from start of model_value to ptr1
    //   the value of the expanded variable
    //   from ptr2 to end of model_value
    //

    len = ((ptr1 - model_value) + wcslen(varvalue) + wcslen(ptr2 + 1) + 1)
        * sizeof(WCHAR);
    buf = Mem_Alloc(args->pool, len);
    if (! buf) {

        status = STATUS_INSUFFICIENT_RESOURCES;
        Log_Status_Ex(MSG_CONF_EXPAND, 0, status, varname);
        return NULL;
    }

    len = (ULONG)(ptr1 - model_value);
    wcsncpy(buf, model_value, len);
    buf[len] = L'\0';
    wcscat(buf, varvalue);
    wcscat(buf, ptr2 + 1);

    return buf;
}


//---------------------------------------------------------------------------
// Conf_Expand_2
//---------------------------------------------------------------------------


_FX WCHAR *Conf_Expand_2(CONF_EXPAND_ARGS *args, const WCHAR *model_value)
{
    WCHAR *old_value;
    WCHAR *new_value;
    WCHAR *Conf_Expand_Buffer;
    int retries;

    Conf_Expand_Buffer = ExAllocatePoolWithTag(PagedPool, PAGE_SIZE, tzuk);
    if (! Conf_Expand_Buffer) {
        Log_Msg0(MSG_1201);
        return NULL;
    }

    retries = 0;

    new_value = (WCHAR *)model_value;
    while (new_value) {

        if (wcslen(new_value) > 1024) {
            Log_Status_Ex(
                MSG_CONF_EXPAND, 0, STATUS_UNSUCCESSFUL, L"(TooLong)");
            return NULL;
        }

        old_value = new_value;
        new_value = Conf_Expand_Helper(args, old_value, Conf_Expand_Buffer);
        //DbgPrint("CONF TRANSLATION:\n");
        //DbgPrint("    %S\n", old_value);
        //DbgPrint("INTO             \n");
        //DbgPrint("    %S\n", new_value);
        if (new_value == old_value) {
            if (old_value == model_value)
                new_value = Mem_AllocString(args->pool, old_value);
            break;
        }
        if (old_value != model_value)
            Mem_FreeString(old_value);

        ++retries;
        if (retries > 10) {
            Log_Status_Ex(
                MSG_CONF_EXPAND, 0, STATUS_UNSUCCESSFUL, L"(Recursion)");
            return NULL;
        }
    }

    ExFreePoolWithTag(Conf_Expand_Buffer, tzuk);

    return new_value;
}


//---------------------------------------------------------------------------
// Conf_Expand
//---------------------------------------------------------------------------


_FX WCHAR *Conf_Expand(
    CONF_EXPAND_ARGS *args, const WCHAR *model_value,
    const WCHAR *setting_name)
{
    WCHAR *new_value = Conf_Expand_2(args, model_value);
    if (! new_value) {

        WCHAR *text;
        ULONG len;

        len = wcslen(args->sandbox) + 16 + wcslen(model_value);
        if (setting_name)
            len += wcslen(setting_name);
        len *= sizeof(WCHAR);
        text = Mem_Alloc(args->pool, len);
        if (text) {

            *text = L'[';
            wcscpy(text + 1, args->sandbox);
            wcscat(text, L"] ");
            if (setting_name) {
                wcscat(text, setting_name);
                wcscat(text, L"=");
            }
            wcscat(text, model_value);

            Log_Msg1(MSG_CONF_SOURCE_TEXT, text);

            Mem_Free(text, len);
        }
    }

    return new_value;
}
