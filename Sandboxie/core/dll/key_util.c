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
// Key Utilities
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
// Key_OpenIfBoxed
//---------------------------------------------------------------------------


_FX NTSTATUS Key_OpenIfBoxed(
    HANDLE *out_handle, ACCESS_MASK access, OBJECT_ATTRIBUTES *objattrs)
{
    NTSTATUS status;
    KEY_NAME_INFORMATION *info;
    WCHAR *name;

    if (objattrs->RootDirectory) {

        ULONG len = PAGE_SIZE;
        info = Dll_Alloc(len);
        status = NtQueryKey(
            objattrs->RootDirectory, KeyNameInformation, info, len, &len);
        if (NT_SUCCESS(status)) {
            WCHAR *name2 = info->Name + info->NameLength / sizeof(WCHAR);
            *name2 = L'\\';
            wcscpy(name2 + 1, objattrs->ObjectName->Buffer);
            name = info->Name;
        }

    } else {

        info = NULL;
        name = objattrs->ObjectName->Buffer;
        status = STATUS_SUCCESS;
    }

    if (NT_SUCCESS(status)) {

        ULONG mp_flags = SbieDll_MatchPath(L'k', name);

        if (mp_flags)
            status = STATUS_BAD_INITIAL_PC;
        else
            status = NtOpenKey(out_handle, access, objattrs);
    }

    return status;
}


//---------------------------------------------------------------------------
// Key_OpenOrCreateIfBoxed
//---------------------------------------------------------------------------


_FX NTSTATUS Key_OpenOrCreateIfBoxed(
    HANDLE *out_handle, ACCESS_MASK access, OBJECT_ATTRIBUTES *objattrs)
{
    NTSTATUS status = Key_OpenIfBoxed(out_handle, access, objattrs);

    if (status == STATUS_OBJECT_NAME_NOT_FOUND) {

        PSECURITY_DESCRIPTOR *SaveSD = objattrs->SecurityDescriptor;
        objattrs->SecurityDescriptor = Secure_EveryoneSD;

        status = NtCreateKey(
            out_handle, access, objattrs, 0, NULL, 0, NULL);

        objattrs->SecurityDescriptor = SaveSD;
    }

    return status;
}


//---------------------------------------------------------------------------
// Key_DeleteValueFromCLSID
//---------------------------------------------------------------------------


_FX void Key_DeleteValueFromCLSID(
    const WCHAR *Xxxid, const WCHAR *Guid, const WCHAR *ValueName)
{
    static const WCHAR *_HKLM_Classes =
        L"";
    NTSTATUS status;
    OBJECT_ATTRIBUTES objattrs;
    UNICODE_STRING objname;
    ULONG DesiredAccess;
    WCHAR *path;
    HANDLE handle;

    DesiredAccess = KEY_SET_VALUE;
    if (Dll_IsWow64)
        DesiredAccess |= KEY_WOW64_64KEY;

    path = Dll_AllocTemp(128 * sizeof(WCHAR));

    wcscpy(path, L"\\registry\\machine\\software\\classes\\");
    wcscat(path, Xxxid);
    wcscat(path, L"\\{");
    wcscat(path, Guid);
    wcscat(path, L"}");
    RtlInitUnicodeString(&objname, path);

    InitializeObjectAttributes(
        &objattrs, &objname, OBJ_CASE_INSENSITIVE, NULL, NULL);

    status = Key_OpenIfBoxed(&handle, DesiredAccess, &objattrs);
    if (NT_SUCCESS(status)) {

        RtlInitUnicodeString(&objname, ValueName);
        NtDeleteValueKey(handle, &objname);

        NtClose(handle);
    }
}
