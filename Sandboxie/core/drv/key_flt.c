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
// Registry:  Registry Filter for Windows Vista and later
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------


static BOOLEAN Key_Init_Filter(void);

static void Key_Unload_Filter(void);

static NTSTATUS Key_Callback(void *Context, void *Arg1, void *Arg2);

static _FX NTSTATUS Key_PreDataInject(REG_QUERY_VALUE_KEY_INFORMATION *PreInfo,ULONG spid,PROCESS *proc);
static _FX NTSTATUS Key_StoreValue(PROCESS *proc,REG_SET_VALUE_KEY_INFORMATION *pSetInfo,ULONG spid);
static _FX WCHAR* Key_GetSandboxPath(ULONG spid,void *Object);
static _FX PUNICODE_STRING Key_GetKeyName(void * Object);

#ifdef ALLOC_PRAGMA
#pragma alloc_text (INIT, Key_Init_Filter)
#endif // ALLOC_PRAGMA


//---------------------------------------------------------------------------
// Variables
//---------------------------------------------------------------------------


static LARGE_INTEGER Key_Cookie;

static BOOLEAN Key_CallbackInstalled = FALSE;

//---------------------------------------------------------------------------
// Key_Init_Filter
//---------------------------------------------------------------------------


_FX BOOLEAN Key_Init_Filter(void)
{
    NTSTATUS status;
    UNICODE_STRING uni;
    P_CmRegisterCallbackEx pCmRegisterCallbackEx;

    //
    // Registry notifications on Windows Vista and later include the
    // DesiredAccess field, so we can use this official method rather
    // than relying on parse procedure hooks like on Windows XP
    //

    RtlInitUnicodeString(&uni, L"CmRegisterCallbackEx");
    pCmRegisterCallbackEx = (P_CmRegisterCallbackEx)
        MmGetSystemRoutineAddress(&uni);

    if (! pCmRegisterCallbackEx)
        status = STATUS_PROCEDURE_NOT_FOUND;
    else {

        status = pCmRegisterCallbackEx(
            Key_Callback, &Driver_Altitude, Driver_Object, NULL,
            &Key_Cookie, NULL);
    }

    if (! NT_SUCCESS(status)) {
        Log_Status_Ex(MSG_OBJ_HOOK_ANY_PROC, 0x81, status, L"Registry");
        return FALSE;
    }

    Key_CallbackInstalled = TRUE;

    return TRUE;
}


//---------------------------------------------------------------------------
// Key_Unload_Filter
//---------------------------------------------------------------------------


_FX void Key_Unload_Filter(void)
{
    if (Key_CallbackInstalled) {

        CmUnRegisterCallback(Key_Cookie);

        Key_CallbackInstalled = FALSE;
    }
}


//---------------------------------------------------------------------------
// Key_Callback
//---------------------------------------------------------------------------


_FX NTSTATUS Key_Callback(void *Context, void *Arg1, void *Arg2)
{
    PROCESS *proc = NULL;
    enum _REG_NOTIFY_CLASS NotifyEvent = (enum _REG_NOTIFY_CLASS)Arg1;
    REG_OPEN_CREATE_KEY_INFORMATION_VISTA *Info;
    ACCESS_STATE AccessState;
    ULONG NtCreateKey;
    UNICODE_STRING RemainingName, *pRemainingName;
    void *nbuf = 0;
    ULONG nlen = 0;
    WCHAR *nptr = 0;
    NTSTATUS status = STATUS_SUCCESS;
    ULONG spid;  //sandboxed process id

    // In later versions of Win 10, there are several cases where sandboxed IE will be making RPC calls into host processes for certain functions.
    // We can detect this because the host process will be using an Sbie thread token.
    // In these situations, we must manually redirect the host registry settings into the sandbox.
    //
    // 1 example is when IE reads/sets proxy server settings (e.g. winhttp!ReadProxySettings).
    // A host process of svchost.exe "LocalServiceNetworkRestricted" will be making the changes on behalf of the sandboxed IE to key:
    // "\REGISTRY\MACHINE\SOFTWARE\Policies\Microsoft\Windows\CurrentVersion\Internet Settings"
    //
    // Another example is when IE sets up the Extensible Cache (e.g. wininet!CreateExtensibleContainer). In this case, a host instance of taskhostw.exe will be
    // setting registry keys/values under \user\current\software\Microsoft\Windows\CurrentVersion\Internet Settings\5.0\Cache\Extensible Cache.
    // We don't need to catch these changes at this time, but may need to in the future.

    if (Driver_OsBuild >= DRIVER_BUILD_WINDOWS_10_CU)
    {
        if (ExGetPreviousMode() == KernelMode)
            return status;

        proc = Process_Find(NULL, NULL);
        if (proc == PROCESS_TERMINATED)
            return STATUS_PROCESS_IS_TERMINATING;

        spid = GetThreadTokenOwnerPid();    // Get pid for the sandboxed process that created this token
        if (NotifyEvent == RegNtPreQueryValueKey)
        {
            if ((!proc) && spid)    // if current process not in sandbox (i.e. a host process), and current thread is using an Sbie token
            {
                Process_GetProcessName(Driver_Pool, (ULONG_PTR)PsGetCurrentProcessId(), &nbuf, &nlen, &nptr);
                if (nbuf)
                {
                    if ((_wcsicmp(nptr, L"svchost.exe") == 0))
                    {
                        REG_QUERY_VALUE_KEY_INFORMATION *pPreInfo = (REG_QUERY_VALUE_KEY_INFORMATION *)Arg2;
                        status = Key_PreDataInject(pPreInfo, spid, proc);
                    }
                    Mem_Free(nbuf, nlen);
                }
            }
            return status;
        }

        else if (NotifyEvent == RegNtPreSetValueKey)
        {
            if ((!proc) && spid)
            {
                REG_SET_VALUE_KEY_INFORMATION *pSetInfo = (REG_SET_VALUE_KEY_INFORMATION*)Arg2;
                status = Key_StoreValue(proc, pSetInfo, spid);
            }
            return status;
        }
    }

    //
    // we only handle RegNtPreCreateKeyEx and RegNtPreOpenKeyEx events
    //

    if (NotifyEvent != RegNtPreCreateKeyEx &&
       NotifyEvent != RegNtPreOpenKeyEx) {
      return status;
    }

    //
    // check if the caller is sandboxed before proceeding
    //
    if (Driver_OsBuild < DRIVER_BUILD_WINDOWS_10_CU)
    {
        proc = Process_Find(NULL, NULL);
        if (proc == PROCESS_TERMINATED)
            return STATUS_PROCESS_IS_TERMINATING;
    }

    Info = (REG_OPEN_CREATE_KEY_INFORMATION_VISTA *)Arg2;

    // HACK ALERT! If you click a link in a Word doc, it will try to start an embedded IE, which cannot be forced into Sandboxie.
    // Hiding this key appears to fix that problem. It causes IE to start a normal IE process, which can be forced into the sandbox

    if (SearchUnicodeString(Info->CompleteName, L"CLSID\\{0002df01-0000-0000-c000-000000000046}", TRUE))
    {
        if (Conf_Get_Boolean(NULL, L"BlockIEEmbedding", 0, FALSE))
        {
            Process_GetProcessName(Driver_Pool, (ULONG_PTR)PsGetCurrentProcessId(), &nbuf, &nlen, &nptr);
            if (nbuf)
            {
                if (!proc)
                {
                    if ((_wcsicmp(nptr, L"winword.exe") == 0) ||
                        (_wcsicmp(nptr, L"powerpnt.exe") == 0) ||
                        (_wcsicmp(nptr, L"excel.exe") == 0) ||
                        (_wcsicmp(nptr, L"explorer.exe") == 0) ||
                        (_wcsicmp(nptr, L"svchost.exe") == 0))
                    {
                        status = STATUS_ACCESS_DENIED;
                    }
                }
                Mem_Free(nbuf, nlen);
            }
        }
    }

    if (status != STATUS_SUCCESS)
        return status;

    if (!proc || proc->bHostInject || proc->disable_key_flt)
        return STATUS_SUCCESS;

    //
    // prepare parameters for Key_MyParseProc_2
    //

    memzero(&AccessState, sizeof(ACCESS_STATE));
    AccessState.OriginalDesiredAccess = Info->DesiredAccess;

    if (NotifyEvent == RegNtPreCreateKeyEx)
        NtCreateKey = 1;
    else
        NtCreateKey = 0;

    //
    // Key_MyParseProc_2 calls Obj_GetParseName which combines
    // ParseObject and RemainingName into a full path.  however,
    // Info->CompleteName passed to this function might already
    // start with a \REGISTRY\ prefix while also specifying
    // ParseObject = root registry key.  Therefore in this case
    // we use a modified RemainingName parameter which passes
    // over the \REGISTRY\ prefix, as this prefix will be
    // restored from the ParseObject by Obj_GetParseName.
    //

    if (Info->CompleteName && Info->CompleteName->Buffer) {

        if (*Info->CompleteName->Buffer == L'\\') {

            RemainingName.Buffer = Info->CompleteName->Buffer + 9;
            RemainingName.Length = Info->CompleteName->Length - (9 * 2);
            RemainingName.MaximumLength = Info->CompleteName->MaximumLength - (9 * 2);
            pRemainingName = &RemainingName;

        } else {

            pRemainingName = Info->CompleteName;
        }

    } else {

        RtlInitUnicodeString(&RemainingName, L"\\X");
        pRemainingName = &RemainingName;
    }

    //
    // Store app container support
    //

    if (Driver_OsVersion >= DRIVER_WINDOWS_10) {
        if (_wcsnicmp(pRemainingName->Buffer, proc->box->key_path + 9, (proc->box->key_path_len - (sizeof(WCHAR) * (9 + 1))) / sizeof(WCHAR)) == 0) // +9 skip \REGISTRY, +1 don't compare '\0'
            return STATUS_SUCCESS;
    }

    //
    // call Key_MyParseProc_2
    //

    return Key_MyParseProc_2(   proc,
                                Info->RootObject,   // ParseObject
                                NULL,               // ObjectType
                                &AccessState,       // AcessState
                                UserMode,           // AccessMode
                                0,                  // Attributes
                                NULL,               // CompleteName
                                pRemainingName,     // RemainingName
                                &NtCreateKey,       // Context
                                NULL,               // SecurityQos
                                NULL                // Object
                            );
}


NTSTATUS Key_StoreValue(PROCESS *proc, REG_SET_VALUE_KEY_INFORMATION *pSetInfo, ULONG spid)
{
    NTSTATUS  status = STATUS_SUCCESS;
    NTSTATUS  rc = STATUS_SUCCESS;
    PUNICODE_STRING KeyName = NULL;
    UNICODE_STRING SBKeyName;
    HANDLE  handle;
    OBJECT_ATTRIBUTES target;
    WCHAR * targetName = NULL;

    if (proc || !pSetInfo)  // in sandbox
        return status;

    targetName = Key_GetSandboxPath(spid, pSetInfo->Object);
    if (targetName)
    {
        ULONG Disp;
        RtlInitUnicodeString(&SBKeyName, targetName);
        InitializeObjectAttributes(&target, &SBKeyName, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, NULL, NULL);
        rc = ZwCreateKey(&handle, KEY_WRITE, &target, 0, NULL, REG_OPTION_NON_VOLATILE, &Disp);
        if (rc == STATUS_SUCCESS)
        {
			rc = STATUS_ACCESS_DENIED; //Block Path

			// Note: Driver verifyer does not like ZwXxx unctions being fed userspace memory
			PUNICODE_STRING ValueName = (pSetInfo->ValueName && pSetInfo->ValueName->MaximumLength > 0) ?
				(PUNICODE_STRING)ExAllocatePoolWithTag(NonPagedPool, sizeof(UNICODE_STRING) + pSetInfo->ValueName->MaximumLength + 8, tzuk) : NULL;
			if (ValueName)
			{
				PVOID Data = pSetInfo->DataSize > 0 ? ExAllocatePoolWithTag(NonPagedPool, pSetInfo->DataSize + 8, tzuk) : NULL;
				if (Data)
				{
					__try
					{
						ValueName->Length = pSetInfo->ValueName->Length;
						ValueName->MaximumLength = pSetInfo->ValueName->MaximumLength;
						ValueName->Buffer = (PWCH)(((UCHAR*)ValueName) + sizeof(UNICODE_STRING));
						if (ValueName->Length > ValueName->MaximumLength)
							ValueName->Length = ValueName->MaximumLength;
						memcpy(ValueName->Buffer, pSetInfo->ValueName->Buffer, ValueName->Length);

						if (pSetInfo->Data) memcpy(Data, pSetInfo->Data, pSetInfo->DataSize);

						rc = ZwSetValueKey(handle, ValueName, pSetInfo->TitleIndex, pSetInfo->Type, Data, pSetInfo->DataSize);
					}
					__except (EXCEPTION_EXECUTE_HANDLER) { }

					ExFreePoolWithTag(Data, tzuk);
				}

				ExFreePoolWithTag(ValueName, tzuk);
			}

            //DbgPrint("SBIE: Write redirect to sandbox: %x, %S, disp = %d\n",rc,targetName,Disp);
            ZwClose(handle);
        }
        if (rc != STATUS_SUCCESS)
        {
            //DbgPrint("SBIE: Write target creation failed blocking: rc = %x, %S\n",rc,targetName);
            status = rc; //block write to host registry
        }
        else
        {
            status = STATUS_CALLBACK_BYPASS;
        }
        ExFreePoolWithTag(targetName, tzuk);
    }

    return status;
}


NTSTATUS Key_PreDataInject(REG_QUERY_VALUE_KEY_INFORMATION *pPreInfo, ULONG spid, PROCESS *proc)
{
    NTSTATUS  status = STATUS_SUCCESS;
    NTSTATUS  rc = STATUS_SUCCESS;
    UNICODE_STRING SBKeyName;
    HANDLE  handle;
    OBJECT_ATTRIBUTES target;
    WCHAR * targetName = NULL;

    if (proc || !pPreInfo)  // in sandbox
        return status;

    targetName = Key_GetSandboxPath(spid, pPreInfo->Object);
    if (targetName)
    {
        PKEY_VALUE_PARTIAL_INFORMATION info = NULL;
        RtlInitUnicodeString(&SBKeyName, targetName);
        InitializeObjectAttributes(&target, &SBKeyName, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, NULL, NULL);

        rc = ZwOpenKey(&handle, KEY_READ, &target);
        if (rc == STATUS_SUCCESS)
        {

			// Note: Driver verifyer does not like ZwXxx unctions being fed userspace memory
			PUNICODE_STRING ValueName = (pPreInfo->ValueName && pPreInfo->ValueName->MaximumLength > 0) ?
				(PUNICODE_STRING)ExAllocatePoolWithTag(NonPagedPool, sizeof(UNICODE_STRING) + pPreInfo->ValueName->MaximumLength + 8, tzuk) : NULL;
			if (ValueName)
			{
				PVOID KeyValueInformation = pPreInfo->Length > 0 ? ExAllocatePoolWithTag(NonPagedPool, pPreInfo->Length + 8, tzuk) : NULL;
				if (KeyValueInformation)
				{
					__try
					{
						ValueName->Length = pPreInfo->ValueName->Length;
						ValueName->MaximumLength = pPreInfo->ValueName->MaximumLength;
						ValueName->Buffer = (PWCH)(((UCHAR*)ValueName) + sizeof(UNICODE_STRING));
						if (ValueName->Length > ValueName->MaximumLength)
							ValueName->Length = ValueName->MaximumLength;
						memcpy(ValueName->Buffer, pPreInfo->ValueName->Buffer, ValueName->Length);

						ULONG  ResultLength = pPreInfo->ResultLength ? *pPreInfo->ResultLength : 0;

						rc = ZwQueryValueKey(handle, ValueName, pPreInfo->KeyValueInformationClass, KeyValueInformation, pPreInfo->Length, &ResultLength);

						if (pPreInfo->ResultLength) *pPreInfo->ResultLength = ResultLength;
						if (ResultLength > pPreInfo->Length)
							ResultLength = pPreInfo->Length;
						if (pPreInfo->KeyValueInformation) memcpy(pPreInfo->KeyValueInformation, KeyValueInformation, ResultLength);
					}
					__except (EXCEPTION_EXECUTE_HANDLER) {}

					ExFreePoolWithTag(KeyValueInformation, tzuk);
				}

				ExFreePoolWithTag(ValueName, tzuk);
			}

            if (rc == STATUS_SUCCESS)
            {
                status = STATUS_CALLBACK_BYPASS;
                //DbgPrint("SBIE: QueryValue redirect to sandbox: length = %d, Result = %d, %S\n",pPreInfo->Length,*pPreInfo->ResultLength,targetName);
            }
            else if (rc == STATUS_OBJECT_NAME_NOT_FOUND)    // Value not in sandbox - read from host return success to continue to host value read
            {
                status = STATUS_SUCCESS; // Read from host
            }
            else
            {
                //DbgPrint("SBIE: Sandbox target not read reason %x, status = %x: %S\n",rc,status,targetName);
                status = rc;  //pass all other error codes: note "buffer too small" error code is set here
            }
            ZwClose(handle);
        }
        //if (rc)
            //  DbgPrint("SBIE: Sandbox target not read reason %x, status = %x: %S\n",rc,status,targetName);

        ExFreePoolWithTag(targetName, tzuk);
    }

    //  DbgPrint("SBIE: Exit status = %x, rc = %x, flag = %d\n",status,rc,flag);
    return status;
}


#define HEADER_USER L"\\REGISTRY\\USER\\"
#define HEADER_MACHINE L"\\REGISTRY\\MACHINE\\"
#define USERS   L"S-1-5-21"
#define CLASSES L"_Classes"
#define MAX_USER_SID_SIZE  128 //in bytes

WCHAR * Key_GetSandboxPath(ULONG spid, void *Object)
{
    PROCESS *SBproc = NULL;
    PUNICODE_STRING KeyName = NULL;
    WCHAR *targetName = NULL;
    ULONG targetFound = 0;
    ULONG nSize;

    if (!spid || !Object)
        return NULL;

    SBproc = Process_Find((HANDLE)spid, NULL);
    if (SBproc == PROCESS_TERMINATED || !SBproc)
        return NULL;

    KeyName = Key_GetKeyName(Object);
    if (KeyName)
    {
        ULONG path_len = wcslen(SBproc->box->key_path);
        nSize = KeyName->Length + (path_len << 1) + (wcslen(L"\\user\\current_classes") << 1);
        targetName = (WCHAR *)ExAllocatePoolWithTag(NonPagedPool, nSize + sizeof(WCHAR), tzuk);
        if (targetName)
        {
            ULONG head_len = wcslen(HEADER_USER);
            WCHAR *temp = NULL;
            memset(targetName, 0, nSize + sizeof(WCHAR));
            wcsncpy(targetName, SBproc->box->key_path, path_len);
            // starts with "\REGISTRY\USER\"
            if (!wcsncmp(KeyName->Buffer, HEADER_USER, head_len))
            {
                temp = wcsstr(&KeyName->Buffer[head_len], L"\\");
                if (temp)
                {
                    // Matches "\REGISTRY\USER\S-1-5-21*\"
                    if (!_wcsnicmp(&KeyName->Buffer[head_len], USERS, wcslen(USERS)))
                    {
                        ULONG sidSize = (ULONG)temp - (ULONG)&KeyName->Buffer[head_len];
                        if (sidSize < MAX_USER_SID_SIZE)
                        {
                            // Matches "\REGISTRY\USER\S-1-5-21*_Classes\"
                            if (!_wcsnicmp(temp - wcslen(CLASSES), L"_Classes", wcslen(CLASSES)))
                            {
                                wcscpy(targetName + path_len, L"\\user\\current_classes");
                                path_len += wcslen(L"\\user\\current_classes");
                            }
                            else
                            {
                                wcscpy(targetName + path_len, L"\\user\\current");
                                path_len += wcslen(L"\\user\\current");
                            }
                            wcscpy(targetName + path_len, temp);
                            targetFound = 1;
                        }
                    }
                }
            }
            // starts with "\REGISTRY\\MACHINE\"
            else if (!_wcsnicmp(KeyName->Buffer, HEADER_MACHINE, wcslen(HEADER_MACHINE)))
            {
                wcscpy(targetName + path_len, KeyName->Buffer + 9);
                targetFound = 1;
            }

            if (!targetFound)
            {
                ExFreePoolWithTag(targetName, tzuk);
                targetName = NULL;
            }
            ExFreePoolWithTag(KeyName, tzuk);
        }
    }

    return targetName;
}

PUNICODE_STRING Key_GetKeyName(void * Object)
{
    NTSTATUS rc = 0;
    PUNICODE_STRING KeyName;
    ULONG nSize;
    ULONG ActualLength;
    nSize = 1024 * sizeof(WCHAR) + sizeof(UNICODE_STRING);

    // add extra null to end of buffer.
    KeyName = (PUNICODE_STRING)ExAllocatePoolWithTag(NonPagedPool, nSize + sizeof(WCHAR), tzuk);

    if (KeyName)
    {
        memset(KeyName, 0, nSize + sizeof(WCHAR));
        rc = ObQueryNameString(Object, (POBJECT_NAME_INFORMATION)KeyName, nSize, &ActualLength);
        if (NT_SUCCESS(rc) && KeyName->Length)
        {
            //DbgPrint("SBIE: KeyName = %S\n",KeyName->Buffer);
            return KeyName;
        }
        ExFreePoolWithTag(KeyName, tzuk);
    }
    return NULL;
}
