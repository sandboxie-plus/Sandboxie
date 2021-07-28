/*
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
// Objects:  Object Filter for Windows Vista SP1 and later
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------


static BOOLEAN Obj_Init_Filter(void);

static void Obj_Unload_Filter(void);

static OB_PREOP_CALLBACK_STATUS Obj_PreOperationCallback(
    _In_ PVOID RegistrationContext, _Inout_ POB_PRE_OPERATION_INFORMATION PreInfo);

//static VOID Obj_PostOperationCallback(
//    _In_ PVOID RegistrationContext, _In_ POB_POST_OPERATION_INFORMATION PostInfo);

#ifdef ALLOC_PRAGMA
#pragma alloc_text (INIT, Obj_Init_Filter)
#endif // ALLOC_PRAGMA


//---------------------------------------------------------------------------
// Variables
//---------------------------------------------------------------------------

static P_ObRegisterCallbacks pObRegisterCallbacks = NULL;
static P_ObUnRegisterCallbacks pObUnRegisterCallbacks = NULL;

static PVOID Obj_FilterCookie = NULL;

static OB_CALLBACK_REGISTRATION  Obj_CallbackRegistration = { 0 };
static OB_OPERATION_REGISTRATION Obj_OperationRegistrations[2] = { { 0 }, { 0 } };

static BOOLEAN Obj_CallbackInstalled = FALSE;


//---------------------------------------------------------------------------
// Obj_Init_Filter
//---------------------------------------------------------------------------


_FX BOOLEAN Obj_Init_Filter(void)
{
    NTSTATUS status;
    UNICODE_STRING uni;

    // Don't use experimental features by default
    if (!Conf_Get_Boolean(NULL, L"UseObjectKernelCallbacks", 0, FALSE))
        return TRUE;

    RtlInitUnicodeString(&uni, L"ObRegisterCallbacks");
    pObRegisterCallbacks = (P_ObRegisterCallbacks)MmGetSystemRoutineAddress(&uni);

    RtlInitUnicodeString(&uni, L"ObUnRegisterCallbacks");
    pObUnRegisterCallbacks = (P_ObUnRegisterCallbacks)MmGetSystemRoutineAddress(&uni);

    if (!pObRegisterCallbacks || !pObUnRegisterCallbacks)
        status = STATUS_PROCEDURE_NOT_FOUND;
    else {

        Obj_OperationRegistrations[0].ObjectType = PsProcessType;
        Obj_OperationRegistrations[0].Operations = OB_OPERATION_HANDLE_CREATE | OB_OPERATION_HANDLE_DUPLICATE;
        Obj_OperationRegistrations[0].PreOperation = Obj_PreOperationCallback;
        Obj_OperationRegistrations[0].PostOperation = NULL;

        Obj_OperationRegistrations[1].ObjectType = PsThreadType;
        Obj_OperationRegistrations[1].Operations = OB_OPERATION_HANDLE_CREATE | OB_OPERATION_HANDLE_DUPLICATE;
        Obj_OperationRegistrations[1].PreOperation = Obj_PreOperationCallback;
        Obj_OperationRegistrations[1].PostOperation = NULL;


        Obj_CallbackRegistration.Version                    = OB_FLT_REGISTRATION_VERSION;
        Obj_CallbackRegistration.OperationRegistrationCount = 2;
        Obj_CallbackRegistration.Altitude                   = Driver_Altitude;
        Obj_CallbackRegistration.RegistrationContext        = NULL;
        Obj_CallbackRegistration.OperationRegistration      = Obj_OperationRegistrations;

        status = pObRegisterCallbacks (&Obj_CallbackRegistration, &Obj_FilterCookie);
    }

    if (! NT_SUCCESS(status)) {
        Log_Status_Ex(MSG_OBJ_HOOK_ANY_PROC, 0x81, status, L"Objects");
        return FALSE;
    }

    Obj_CallbackInstalled = TRUE;

    return TRUE;
}


//---------------------------------------------------------------------------
// Obj_Unload_Filter
//---------------------------------------------------------------------------


_FX void Obj_Unload_Filter(void)
{
    if (Obj_CallbackInstalled) {

        pObUnRegisterCallbacks(Obj_FilterCookie);

        Obj_CallbackInstalled = FALSE;
    }
}


//---------------------------------------------------------------------------
// Obj_PreOperationCallback
//---------------------------------------------------------------------------


_FX OB_PREOP_CALLBACK_STATUS Obj_PreOperationCallback(
    _In_ PVOID RegistrationContext, _Inout_ POB_PRE_OPERATION_INFORMATION PreInfo)
{
    PROCESS *proc = NULL;
    NTSTATUS status = STATUS_SUCCESS;

    if (ExGetPreviousMode() == KernelMode)
        return status;

    proc = Process_Find(NULL, NULL);
    if (!proc || (proc == PROCESS_TERMINATED))
        return OB_PREOP_SUCCESS;


    DbgPrint("Obj_PreOperationCallback for %S\r\n", proc->image_name);


    /*
    typedef struct _OB_PRE_CREATE_HANDLE_INFORMATION {
        _Inout_ ACCESS_MASK         DesiredAccess;
        _In_ ACCESS_MASK            OriginalDesiredAccess;
    } OB_PRE_CREATE_HANDLE_INFORMATION, *POB_PRE_CREATE_HANDLE_INFORMATION;

    typedef struct _OB_PRE_DUPLICATE_HANDLE_INFORMATION {
        _Inout_ ACCESS_MASK         DesiredAccess;
        _In_ ACCESS_MASK            OriginalDesiredAccess;
        _In_ PVOID                  SourceProcess;
        _In_ PVOID                  TargetProcess;
    } OB_PRE_DUPLICATE_HANDLE_INFORMATION, * POB_PRE_DUPLICATE_HANDLE_INFORMATION;
    */

    /*PTD_CALLBACK_REGISTRATION CallbackRegistration;

    ACCESS_MASK AccessBitsToClear     = 0;
    ACCESS_MASK AccessBitsToSet       = 0;
    ACCESS_MASK InitialDesiredAccess  = 0;
    ACCESS_MASK OriginalDesiredAccess = 0;


    PACCESS_MASK DesiredAccess = NULL;

    LPCWSTR ObjectTypeName = NULL;
    LPCWSTR OperationName = NULL;

    // Not using driver specific values at this time
    CallbackRegistration = (PTD_CALLBACK_REGISTRATION)RegistrationContext;


    TD_ASSERT (PreInfo->CallContext == NULL);

    // Only want to filter attempts to access protected process
    // all other processes are left untouched

    if (PreInfo->ObjectType == *PsProcessType)  {

	    //HANDLE pid = PsGetProcessId((PEPROCESS)PreInfo->Object);
	    //char szProcName[16] = { 0 };
	    //UNREFERENCED_PARAMETER(RegistrationContext);
	    //strcpy(szProcName, GetProcessNameByProcessID(pid));

        //
        // Ignore requests for processes other than our target process.
        //

        // if (TdProtectedTargetProcess != NULL &&
        //    TdProtectedTargetProcess != PreInfo->Object)
        if (TdProtectedTargetProcess != PreInfo->Object)
        {
            goto Exit;
        }

        //
        // Also ignore requests that are trying to open/duplicate the current
        // process.
        //

        if (PreInfo->Object == PsGetCurrentProcess())   {
            DbgPrintEx (
                DPFLTR_IHVDRIVER_ID, DPFLTR_TRACE_LEVEL,
                "ObCallbackTest: CBTdPreOperationCallback: ignore process open/duplicate from the protected process itself\n");
            goto Exit;
        }

        ObjectTypeName = L"PsProcessType";
        AccessBitsToClear     = CB_PROCESS_TERMINATE;
        AccessBitsToSet       = 0;
    }
    else if (PreInfo->ObjectType == *PsThreadType)  {
        HANDLE ProcessIdOfTargetThread = PsGetThreadProcessId ((PETHREAD)PreInfo->Object);

        //
        // Ignore requests for threads belonging to processes other than our
        // target process.
        //

        // if (CallbackRegistration->TargetProcess   != NULL &&
        //     CallbackRegistration->TargetProcessId != ProcessIdOfTargetThread)
        if (TdProtectedTargetProcessId != ProcessIdOfTargetThread)  {
            goto Exit;
        }

        //
        // Also ignore requests for threads belonging to the current processes.
        //

        if (ProcessIdOfTargetThread == PsGetCurrentProcessId()) {
            DbgPrintEx (
                DPFLTR_IHVDRIVER_ID, DPFLTR_TRACE_LEVEL,
                "ObCallbackTest: CBTdPreOperationCallback: ignore thread open/duplicate from the protected process itself\n");
            goto Exit;
        }

        ObjectTypeName = L"PsThreadType";
        AccessBitsToClear     = CB_THREAD_TERMINATE;
        AccessBitsToSet       = 0;
    }
    else    {
        DbgPrintEx (
            DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
            "ObCallbackTest: CBTdPreOperationCallback: unexpected object type\n");
        goto Exit;
    }

    switch (PreInfo->Operation) {
    case OB_OPERATION_HANDLE_CREATE:
        DesiredAccess = &PreInfo->Parameters->CreateHandleInformation.DesiredAccess;
        OriginalDesiredAccess = PreInfo->Parameters->CreateHandleInformation.OriginalDesiredAccess;

        OperationName = L"OB_OPERATION_HANDLE_CREATE";
        break;

    case OB_OPERATION_HANDLE_DUPLICATE:
        DesiredAccess = &PreInfo->Parameters->DuplicateHandleInformation.DesiredAccess;
        OriginalDesiredAccess = PreInfo->Parameters->DuplicateHandleInformation.OriginalDesiredAccess;

        OperationName = L"OB_OPERATION_HANDLE_DUPLICATE";
        break;

    default:
        TD_ASSERT (FALSE);
        break;
    }

    InitialDesiredAccess = *DesiredAccess;

    // Filter only if request made outside of the kernel
    if (PreInfo->KernelHandle != 1) {
        *DesiredAccess &= ~AccessBitsToClear;
        *DesiredAccess |=  AccessBitsToSet;
    }

    //
    // Set call context.
    //

    TdSetCallContext (PreInfo, CallbackRegistration);


    DbgPrintEx (
        DPFLTR_IHVDRIVER_ID, DPFLTR_TRACE_LEVEL, "ObCallbackTest: CBTdPreOperationCallback: PROTECTED process %p (ID 0x%p)\n",
        TdProtectedTargetProcess,
        (PVOID)TdProtectedTargetProcessId
    );

    DbgPrintEx (
        DPFLTR_IHVDRIVER_ID, DPFLTR_TRACE_LEVEL,
        "ObCallbackTest: CBTdPreOperationCallback\n"
        "    Client Id:    %p:%p\n"
        "    Object:       %p\n"
        "    Type:         %ls\n"
        "    Operation:    %ls (KernelHandle=%d)\n"
        "    OriginalDesiredAccess: 0x%x\n"
        "    DesiredAccess (in):    0x%x\n"
        "    DesiredAccess (out):   0x%x\n",
        PsGetCurrentProcessId(),
        PsGetCurrentThreadId(),
        PreInfo->Object,
        ObjectTypeName,
        OperationName,
        PreInfo->KernelHandle,
        OriginalDesiredAccess,
        InitialDesiredAccess,
        *DesiredAccess
    );

Exit:*/

    return OB_PREOP_SUCCESS;
}


//---------------------------------------------------------------------------
// Obj_PostOperationCallback
//---------------------------------------------------------------------------


//_FX VOID Obj_PostOperationCallback(
//    _In_ PVOID RegistrationContext, _In_ POB_POST_OPERATION_INFORMATION PostInfo)
//{
//}