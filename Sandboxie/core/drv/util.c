/*
 * Copyright 2004-2020 Sandboxie Holdings, LLC 
 * Copyright 2020-2024 David Xanatos, xanasoft.com
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
// Utilities
//---------------------------------------------------------------------------


#include "util.h"
#include "process.h"
#include "common/my_version.h"
#define KERNEL_MODE
#include "verify.h"


//---------------------------------------------------------------------------
// MyIsCurrentProcessRunningAsLocalSystem
//---------------------------------------------------------------------------


_FX BOOLEAN MyIsCurrentProcessRunningAsLocalSystem(void)
{
    extern const WCHAR *Driver_S_1_5_18;
    UNICODE_STRING SidString;
    ULONG SessionId;
    BOOLEAN s_1_5_18 = FALSE;
    NTSTATUS status = Process_GetSidStringAndSessionId(
                        NtCurrentProcess(), NULL, &SidString, &SessionId);
    if (NT_SUCCESS(status)) {
        if (_wcsicmp(SidString.Buffer, Driver_S_1_5_18) == 0)
            s_1_5_18 = TRUE;
        RtlFreeUnicodeString(&SidString);
    }
    return s_1_5_18;
}


//---------------------------------------------------------------------------
// MyIsProcessRunningAsSystemAccount
//---------------------------------------------------------------------------


_FX BOOLEAN MyIsProcessRunningAsSystemAccount(HANDLE ProcessId)
{
    UNICODE_STRING SidString;
    ULONG SessionId;
    BOOLEAN system;

    NTSTATUS status = Process_GetSidStringAndSessionId(
                            NULL, ProcessId, &SidString, &SessionId);

    if (NT_SUCCESS(status)) {

        extern const WCHAR *Driver_S_1_5_18;
        extern const WCHAR *Driver_S_1_5_19;
        extern const WCHAR *Driver_S_1_5_20;

        system = (_wcsicmp(SidString.Buffer, Driver_S_1_5_18) == 0 ||
                  _wcsicmp(SidString.Buffer, Driver_S_1_5_19) == 0 ||
                  _wcsicmp(SidString.Buffer, Driver_S_1_5_20) == 0);

        RtlFreeUnicodeString(&SidString);

    } else
        system = FALSE;

    return system;
}


//---------------------------------------------------------------------------
// MyIsCallerMyServiceProcess
//---------------------------------------------------------------------------


_FX BOOLEAN MyIsCallerMyServiceProcess(void)
{
    BOOLEAN ok = FALSE;

    if (MyIsCurrentProcessRunningAsLocalSystem()) {

        void *nbuf;
        ULONG nlen;
        WCHAR *nptr;

        const ULONG ProcessId = (ULONG)(ULONG_PTR)PsGetCurrentProcessId();
        Process_GetProcessName(Driver_Pool, ProcessId, &nbuf, &nlen, &nptr);
        if (nbuf) {

            UNICODE_STRING *uni = (UNICODE_STRING *)nbuf;

            if ((uni->Length > Driver_HomePathNt_Len * sizeof(WCHAR)) &&
                    (0 == _wcsnicmp(uni->Buffer, Driver_HomePathNt,
                                    Driver_HomePathNt_Len))) {

                if (_wcsicmp(nptr, SBIESVC_EXE) == 0) {

                    ok = TRUE;
                }
            }

            Mem_Free(nbuf, nlen);
        }
    }

    return ok;
}


//---------------------------------------------------------------------------
// MyGetSessionId
//---------------------------------------------------------------------------


_FX NTSTATUS MyGetSessionId(ULONG *SessionId)
{
    NTSTATUS status;
    PROCESS_SESSION_INFORMATION info;
    ULONG len;

    len = sizeof(info);
    status = ZwQueryInformationProcess(
        NtCurrentProcess(), ProcessSessionInformation,
        &info, sizeof(info), &len);

    if (NT_SUCCESS(status))
        *SessionId = info.SessionId;
    else
        *SessionId = 0;

    return status;
}


//---------------------------------------------------------------------------
// MyGetParentId
//---------------------------------------------------------------------------


_FX NTSTATUS MyGetParentId(HANDLE *ParentId)
{
    NTSTATUS status;
    PROCESS_BASIC_INFORMATION info;
    ULONG len;

    len = sizeof(info);
    status = ZwQueryInformationProcess(
        NtCurrentProcess(), ProcessBasicInformation,
        &info, len, &len);

    if (NT_SUCCESS(status))
        *ParentId = (HANDLE)info.InheritedFromUniqueProcessId;
    else
        *ParentId = NULL;

    return status;
}


PWCHAR SearchUnicodeString(PCUNICODE_STRING pString1, PWCHAR pString2, BOOLEAN boolCaseInSensitive)
{
    if ((pString1 == NULL) || (pString1->Buffer == NULL) || (pString1->Length == 0) || (pString2 == NULL))
        return NULL;

    PWCHAR  pEOS;       // End Of String
    UNICODE_STRING  usSearch;
    UNICODE_STRING  usStr = *pString1;

    // make the search string a UNICODE_STRING
    RtlInitUnicodeString(&usSearch, pString2);
    if (usStr.Length < usSearch.Length)
        return NULL;

    usStr.Length = usSearch.Length;
    pEOS = pString1->Buffer + (pString1->Length - usSearch.Length) / 2;

    while (usStr.Buffer <= pEOS)
    {
        if (RtlCompareUnicodeString(&usStr, &usSearch, boolCaseInSensitive) == 0)
            return usStr.Buffer;
        usStr.Buffer++;
    }
    return NULL;
}

BOOLEAN UnicodeStringStartsWith(PCUNICODE_STRING pString1, PWCHAR pString2, BOOLEAN boolCaseInSensitive)
{
    if ((pString1 == NULL) || (pString1->Buffer == NULL) || (pString1->Length == 0) || (pString2 == NULL))
        return FALSE;

    UNICODE_STRING  usSearch;
    UNICODE_STRING  usStr = *pString1;

    // make a UNICODE_STRING from the search string
    RtlInitUnicodeString(&usSearch, pString2);
    if (usStr.Length < usSearch.Length)
        return FALSE;

    usStr.Length = usSearch.Length;
    return (RtlCompareUnicodeString(&usStr, &usSearch, boolCaseInSensitive) == 0);
}

BOOLEAN UnicodeStringEndsWith(PCUNICODE_STRING pString1, PWCHAR pString2, BOOLEAN boolCaseInSensitive)
{
    if ((pString1 == NULL) || (pString1->Buffer == NULL) || (pString1->Length == 0) || (pString2 == NULL))
        return FALSE;

    UNICODE_STRING  usSearch;
    UNICODE_STRING  usStr = *pString1;

    // make a UNICODE_STRING from the search string
    RtlInitUnicodeString(&usSearch, pString2);
    if (usStr.Length < usSearch.Length)
        return FALSE;

    usStr.Buffer += (usStr.Length - usSearch.Length) / 2;
    usStr.Length = usSearch.Length;
    return (RtlCompareUnicodeString(&usStr, &usSearch, boolCaseInSensitive) == 0);
}

BOOLEAN DoesRegValueExist(ULONG RelativeTo, WCHAR *Path, WCHAR *ValueName)
{
    WCHAR DummyBuffer[1] = {0}; // if we provide a NULL buffer, this will cause a memory pool leak somewhere in the kernel
    UNICODE_STRING Dummy = { 0, sizeof(DummyBuffer), DummyBuffer };
    NTSTATUS status = GetRegString(RelativeTo, Path, ValueName, &Dummy);
    return (status == STATUS_SUCCESS || status == STATUS_OBJECT_TYPE_MISMATCH);
}

NTSTATUS GetRegString(ULONG RelativeTo, const WCHAR *Path, const WCHAR *ValueName, UNICODE_STRING* pData)
{
	NTSTATUS status;
	RTL_QUERY_REGISTRY_TABLE qrt[2];

	memzero(qrt, sizeof(qrt));
	qrt[0].Flags = RTL_QUERY_REGISTRY_REQUIRED |
		RTL_QUERY_REGISTRY_DIRECT |
        RTL_QUERY_REGISTRY_TYPECHECK | // fixes security violation but causes STATUS_OBJECT_TYPE_MISMATCH when buffer to small
		RTL_QUERY_REGISTRY_NOVALUE |
		RTL_QUERY_REGISTRY_NOEXPAND;
	qrt[0].Name = (WCHAR *)ValueName;
	qrt[0].EntryContext = pData;
	qrt[0].DefaultType = (REG_SZ << RTL_QUERY_REGISTRY_TYPECHECK_SHIFT) | REG_NONE;

	status = RtlQueryRegistryValues(
		RelativeTo, Path, qrt, NULL, NULL);

    if (status == STATUS_OBJECT_TYPE_MISMATCH) {

        qrt[0].DefaultType = (REG_EXPAND_SZ << RTL_QUERY_REGISTRY_TYPECHECK_SHIFT) | REG_NONE;

	    status = RtlQueryRegistryValues(
		    RelativeTo, Path, qrt, NULL, NULL);
    }

    return status;
}

_FX ULONG GetRegDword(const WCHAR *KeyPath, const WCHAR *ValueName)
{
    NTSTATUS status;
    RTL_QUERY_REGISTRY_TABLE qrt[2];
    UNICODE_STRING uni;
    ULONG value;

    value = -1;

    uni.Length = 4;
    uni.MaximumLength = 4;
    uni.Buffer = (WCHAR *)&value;

    memzero(qrt, sizeof(qrt));
    qrt[0].Flags =  RTL_QUERY_REGISTRY_REQUIRED |
        RTL_QUERY_REGISTRY_DIRECT |
        RTL_QUERY_REGISTRY_TYPECHECK |
        RTL_QUERY_REGISTRY_NOEXPAND;
    qrt[0].Name = (WCHAR *)ValueName;
    qrt[0].EntryContext = &uni;
    qrt[0].DefaultType = (REG_DWORD << RTL_QUERY_REGISTRY_TYPECHECK_SHIFT) | REG_NONE;

    status = RtlQueryRegistryValues(
        RTL_REGISTRY_ABSOLUTE, KeyPath, qrt, NULL, NULL);

    if (status != STATUS_SUCCESS)
        return 0;

    if (value == -1) {

        //
        // if value is not string, RtlQueryRegistryValues writes
        // it directly into EntryContext
        //

        value = *(ULONG *)&uni;
    }

    return value;
}

NTSTATUS SetRegValue(const WCHAR *KeyPath, const WCHAR *ValueName, const void *Data, ULONG uSize)
{
    NTSTATUS status = STATUS_SUCCESS;
    HANDLE handle = NULL;
    UNICODE_STRING keyPath;
    UNICODE_STRING valueName;
    OBJECT_ATTRIBUTES objattrs;
    ULONG disposition = 0;
    PSECURITY_DESCRIPTOR sd = NULL;
    PACL pEmptyAcl = NULL;

    if (!KeyPath || !ValueName || !Data)
        return STATUS_INVALID_PARAMETER;

    sd = (PSECURITY_DESCRIPTOR)ExAllocatePoolWithTag(PagedPool, SECURITY_DESCRIPTOR_MIN_LENGTH, tzuk);
    if (!sd) {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto finish;
    }

    status = RtlCreateSecurityDescriptor(sd, SECURITY_DESCRIPTOR_REVISION);
    if (!NT_SUCCESS(status))
        goto finish;

    //
    // Define a static SID for S-1-5-100.
    // Revision = 1, SubAuthorityCount = 1,
    // IdentifierAuthority = {0,0,0,0,0,5} (SECURITY_NT_AUTHORITY),
    // and SubAuthority[0] = 100.
    //
    SID OwnerSid = { 
        1,                   // Revision
        1,                   // SubAuthorityCount
        {0, 0, 0, 0, 0, 5},   // IdentifierAuthority
        {100}                // SubAuthority[0]
    };

    status = RtlSetOwnerSecurityDescriptor(sd, &OwnerSid, FALSE);
    if (!NT_SUCCESS(status))
        goto finish;

    pEmptyAcl = (PACL)ExAllocatePoolWithTag(PagedPool, sizeof(ACL), tzuk);
    if (!pEmptyAcl) {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto finish;
    }

    status = RtlCreateAcl(pEmptyAcl, sizeof(ACL), ACL_REVISION);
    if (!NT_SUCCESS(status))
        goto finish;

    status = RtlSetDaclSecurityDescriptor(sd, TRUE, pEmptyAcl, FALSE);
    if (!NT_SUCCESS(status))
        goto finish;

    RtlInitUnicodeString(&keyPath, KeyPath);
    InitializeObjectAttributes(&objattrs, &keyPath, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, NULL, sd);

    status = ZwCreateKey(&handle, KEY_ALL_ACCESS, &objattrs, 0, NULL, REG_OPTION_NON_VOLATILE, &disposition);
    if (!NT_SUCCESS(status))
        goto finish;

    if (disposition == REG_OPENED_EXISTING_KEY) {
        status = ZwSetSecurityObject(handle, OWNER_SECURITY_INFORMATION | DACL_SECURITY_INFORMATION, sd);
        //if (!NT_SUCCESS(status))
        //    goto finish;
    }

    RtlInitUnicodeString(&valueName, ValueName);
    status = ZwSetValueKey(handle, &valueName, 0, REG_BINARY, (PVOID)Data, uSize);

finish:
    if (handle)
        ZwClose(handle);

    if (pEmptyAcl)
        ExFreePoolWithTag(pEmptyAcl, tzuk);

    if (sd)
        ExFreePoolWithTag(sd, tzuk);

    return status;
}

NTSTATUS GetRegValue(const WCHAR *KeyPath, const WCHAR *ValueName, PVOID* ppData, ULONG* pSize)
{
    NTSTATUS status = STATUS_SUCCESS;
    HANDLE handle = NULL;
    UNICODE_STRING keyPath;
    UNICODE_STRING valueName;
    OBJECT_ATTRIBUTES objattrs;
    ULONG disp;
    ULONG length;
    UCHAR buffer[sizeof(KEY_VALUE_PARTIAL_INFORMATION) + 2048];
    UCHAR* data = buffer;
    ULONG data_len = 0;
    
    if (!KeyPath || !ValueName || !ppData || !pSize)
        return STATUS_INVALID_PARAMETER;

    RtlInitUnicodeString(&keyPath, KeyPath);
    InitializeObjectAttributes(&objattrs, &keyPath, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, NULL, NULL);
    status = ZwOpenKey(&handle, KEY_WRITE, &objattrs);
    if (status == STATUS_SUCCESS) {

        RtlInitUnicodeString(&valueName, ValueName);
        status = ZwQueryValueKey(handle, &valueName, KeyValuePartialInformation, data, sizeof(buffer), &length);
        if (status == STATUS_BUFFER_OVERFLOW || status == STATUS_BUFFER_TOO_SMALL) {

            data_len = length;
            data = Mem_Alloc(Driver_Pool, data_len);
            if (!data) {
                data_len = 0;
                status = STATUS_INSUFFICIENT_RESOURCES;
                goto finish;
            }

            status = ZwQueryValueKey(handle, &valueName, KeyValuePartialInformation, data, data_len, &length);
        }

        if (NT_SUCCESS(status)) {

            //
            // guard the data copying as the pointer we get may be from user space!!!
            //

            __try {

                PKEY_VALUE_PARTIAL_INFORMATION info = (PKEY_VALUE_PARTIAL_INFORMATION)data;

                if (!*ppData) {
                    *ppData = Mem_Alloc(Driver_Pool, info->DataLength);
                    if (!*ppData) {
                        status = STATUS_INSUFFICIENT_RESOURCES;
                        goto finish;
                    }
                }
                else if (info->DataLength > *pSize) {
                    status = STATUS_BUFFER_TOO_SMALL;
                    goto finish;
                }

                *pSize = info->DataLength;
                memcpy(*ppData, info->Data, info->DataLength);

            } __except (EXCEPTION_EXECUTE_HANDLER) {
                status = GetExceptionCode();
            }
        }
    }

finish:
    if(handle)
        ZwClose(handle);

    if (data_len)
        Mem_Free(data, data_len);

    return status;
}

void *memmem(const void *pSearchBuf,
    size_t nBufSize,
    const void *pPattern,
    size_t nPatternSize)
{
    UCHAR *pBuf = (UCHAR *)pSearchBuf;
    UCHAR *pEos = pBuf + nBufSize - nPatternSize;

    if (!(pBuf && pEos && nBufSize && nPatternSize))
        return NULL;

    while (pBuf <= pEos) {
        if (*pBuf == *(UCHAR*)pPattern)
            if (memcmp(pBuf, pPattern, nPatternSize) == 0)
                return pBuf;
        ++pBuf;
    }

    return NULL;
}


//---------------------------------------------------------------------------
// MyIsTestSigning
//---------------------------------------------------------------------------

typedef struct _SYSTEM_CODEINTEGRITY_INFORMATION
{
    ULONG Length;
    ULONG CodeIntegrityOptions;
} SYSTEM_CODEINTEGRITY_INFORMATION, *PSYSTEM_CODEINTEGRITY_INFORMATION;

_FX BOOLEAN MyIsTestSigning(void)
{
    SYSTEM_CODEINTEGRITY_INFORMATION sci = {sizeof(SYSTEM_CODEINTEGRITY_INFORMATION)};
	if(NT_SUCCESS(ZwQuerySystemInformation(/*SystemCodeIntegrityInformation*/ 103, &sci, sizeof(sci), NULL)))
	{
		//BOOLEAN bCodeIntegrityEnabled = !!(sci.CodeIntegrityOptions & /*CODEINTEGRITY_OPTION_ENABLED*/ 0x1);
		BOOLEAN bTestSigningEnabled = !!(sci.CodeIntegrityOptions & /*CODEINTEGRITY_OPTION_TESTSIGN*/ 0x2);

        //DbgPrint("Test Signing: %d; Code Integrity: %d\r\n", bTestSigningEnabled, bCodeIntegrityEnabled);

        //if (bTestSigningEnabled || !bCodeIntegrityEnabled)
        if (bTestSigningEnabled)
            return TRUE;
	}
    return FALSE;
}


//---------------------------------------------------------------------------
// MyIsCallerSigned
//---------------------------------------------------------------------------


_FX BOOLEAN MyIsCallerSigned(void)
{
    NTSTATUS status;

    // in test signing mode don't verify the signature
    if (Driver_OsTestSigning)
        return TRUE;

    // if this is a node locked develoepr certificate don't verify the signature
    if (Verify_CertInfo.type == eCertDeveloper && Verify_CertInfo.active)
        return TRUE;

    status = KphVerifyCurrentProcess();

    //DbgPrint("Image Signature Verification result: 0x%08x\r\n", status);

    if (!NT_SUCCESS(status)) {

        //Log_Status(MSG_1330, 0, status);

        return FALSE;
    }

    return TRUE;
}


//---------------------------------------------------------------------------
// MyValidateCertificate
//---------------------------------------------------------------------------

NTSTATUS KphValidateCertificate();

extern wchar_t g_uuid_str[40];
void InitFwUuid();

_FX NTSTATUS MyValidateCertificate(void)
{
    if(!*g_uuid_str)
        InitFwUuid();

    NTSTATUS status = KphValidateCertificate();

    if (status == STATUS_ACCOUNT_EXPIRED)
        status = STATUS_SUCCESS;

    return status;
}


//---------------------------------------------------------------------------
// Util_GetProcessPidByName
//---------------------------------------------------------------------------


_FX HANDLE Util_GetProcessPidByName(const WCHAR* name) 
{
    HANDLE pid = (HANDLE)-1;
    USHORT name_len = wcslen(name) * sizeof(WCHAR);
    PVOID buffer = NULL;
    ULONG buffer_size = 0x10000;
    ULONG retry_count = 0;
    NTSTATUS status;

retry:
    buffer = ExAllocatePoolWithTag(PagedPool, buffer_size, tzuk);
    if (buffer){

        status = ZwQuerySystemInformation(SystemProcessesAndThreadsInformation, buffer, buffer_size, &buffer_size);
        if (status == STATUS_INFO_LENGTH_MISMATCH && retry_count++ < 100) {
            ExFreePoolWithTag(buffer, tzuk);
            buffer_size += 0x1000 * retry_count;
            goto retry;
        }

        if (NT_SUCCESS(status)) {

            SYSTEM_PROCESS_INFORMATION* processEntry = (SYSTEM_PROCESS_INFORMATION*)buffer;
            for (;;) {

                if (processEntry->ProcessName.Length == name_len && _wcsicmp(name, processEntry->ProcessName.Buffer) == 0) {

                    pid = (HANDLE)processEntry->ProcessId;
                    break;
                }

                if (!processEntry->NextEntryOffset)
                    break;

                processEntry = (SYSTEM_PROCESS_INFORMATION*)((UCHAR*)processEntry + processEntry->NextEntryOffset);
            }
        }

        ExFreePoolWithTag(buffer, tzuk);
    }

    return pid;
}


//---------------------------------------------------------------------------
// Util_IsSystemProcess
//---------------------------------------------------------------------------

NTKERNELAPI PCHAR NTAPI PsGetProcessImageFileName(_In_ PEPROCESS Process);

_FX BOOLEAN Util_IsSystemProcess(HANDLE pid, const char* name)
{
    PEPROCESS ProcessObject;
    NTSTATUS status;
    PCHAR ImageFileName;
    BOOLEAN ret = FALSE;

    if (!MyIsProcessRunningAsSystemAccount(pid))
        return FALSE;

    status = PsLookupProcessByProcessId(pid, &ProcessObject);
    if (NT_SUCCESS(status)) {

        ImageFileName = PsGetProcessImageFileName(ProcessObject);

        ret = (_stricmp(ImageFileName, name) == 0);

        ObDereferenceObject(ProcessObject);
    }

    return ret;
}


//---------------------------------------------------------------------------
// Util_IsProtectedProcess
//---------------------------------------------------------------------------

NTKERNELAPI BOOLEAN NTAPI PsIsProtectedProcess(_In_ PEPROCESS Process);

_FX BOOLEAN Util_IsProtectedProcess(HANDLE pid)
{
    PEPROCESS ProcessObject;
    NTSTATUS status;
    BOOLEAN ret = FALSE;

    //
    // Check if this process is a protected process,
    // as protected processes are integral windows processes or trusted antimalware services
    // we allow such processes to access even confidential sandboxed programs.
    //

    status = PsLookupProcessByProcessId(pid, &ProcessObject);
    if (NT_SUCCESS(status)) {
        
        ret = PsIsProtectedProcess(ProcessObject);

        ObDereferenceObject(ProcessObject);
    }

    return ret;
}


//---------------------------------------------------------------------------
// Util_GetTime
//---------------------------------------------------------------------------


_FX LARGE_INTEGER Util_GetTimestamp(void)
{
    static LARGE_INTEGER gMonitorStartCounter;
    static LARGE_INTEGER gPerformanceFrequency;
    static LARGE_INTEGER gMonitorStartTime = { 0 };

    if (gMonitorStartTime.QuadPart == 0) {
        KeQuerySystemTime(&gMonitorStartTime);
        gMonitorStartCounter = KeQueryPerformanceCounter(&gPerformanceFrequency);
    }

	LARGE_INTEGER Time;
	LARGE_INTEGER CounterNow = KeQueryPerformanceCounter(NULL);
	LONGLONG CounterOff = CounterNow.QuadPart - gMonitorStartCounter.QuadPart;

	Time.QuadPart = gMonitorStartTime.QuadPart +
	(10000000 * (CounterOff / gPerformanceFrequency.QuadPart)) +
		((10000000 * (CounterOff % gPerformanceFrequency.QuadPart)) / gPerformanceFrequency.QuadPart);

	return Time;
}


//---------------------------------------------------------------------------
// Util_CaptureStack
//---------------------------------------------------------------------------


ULONG Util_CaptureStack(_Out_ PVOID* Frames, _In_ ULONG Count)
{
    ULONG frames;

    NT_ASSERT(KeGetCurrentIrql() <= DISPATCH_LEVEL);

    frames = RtlWalkFrameChain(Frames, Count, 0);

    if (KeGetCurrentIrql() < DISPATCH_LEVEL)
    {
        if (frames >= Count)
        {
            return frames;
        }

        frames += RtlWalkFrameChain(&Frames[frames],
                                    (Count - frames),
                                    RTL_WALK_USER_MODE_STACK);
    }

    return frames;
}