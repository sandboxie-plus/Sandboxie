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

BOOLEAN Obj_CallbackInstalled = FALSE;


//---------------------------------------------------------------------------
// Obj_Init_Filter
//---------------------------------------------------------------------------


_FX BOOLEAN Obj_Init_Filter(void)
{
    UNICODE_STRING uni;

    RtlInitUnicodeString(&uni, L"ObRegisterCallbacks");
    pObRegisterCallbacks = (P_ObRegisterCallbacks)MmGetSystemRoutineAddress(&uni);

    RtlInitUnicodeString(&uni, L"ObUnRegisterCallbacks");
    pObUnRegisterCallbacks = (P_ObUnRegisterCallbacks)MmGetSystemRoutineAddress(&uni);

    // note: Obj_Load_Filter needs a few other things to be initialized first, hence it will be called by Ipc_Init

    return TRUE;
}


//---------------------------------------------------------------------------
// Obj_Load_Filter
//---------------------------------------------------------------------------


_FX BOOLEAN Obj_Load_Filter(void)
{
    NTSTATUS status;

    if (Obj_CallbackInstalled)
        return TRUE;

    // 
    // from Syscall_DuplicateHandle_2:
    // note that files and registry keys [...] don't support adding new permissions
    // on the handle during duplication.  (this is true for any
    // object type which specifies a SecurityProcedure.)
    // 
    // opening/creation of files is handled by a minifilter installed with FltRegisterFilter
    // and opening/creation of registry keys is handled by CmRegisterCallbackEx
    //
    // The types handled by the Syscall_DuplicateHandle are as follows
    // 
    // "Process"    -> Thread_CheckProcessObject        <- PsProcessType
    // "Thread"     -> Thread_CheckThreadObject         <- PsThreadType
    // 
    // "File"       -> File_CheckFileObject             <- IoFileObjectType // given the the note above why do we double filter for files ???
    // 
    // "Event"      -> Ipc_CheckGenericObject
    // "EventPair"  -> Ipc_CheckGenericObject           <- ExEventPairObjectType not exported
    // "KeyedEvent" -> Ipc_CheckGenericObject           <- ExpKeyedEventObjectType not exported
    // "Mutant"     -> Ipc_CheckGenericObject           <- ExMutantObjectType not exported
    // "Semaphore"  -> Ipc_CheckGenericObject           <- ExSemaphoreObjectType
    // "Section"    -> Ipc_CheckGenericObject           <- MmSectionObjectType
    // 
    // "JobObject"  -> Ipc_CheckJobObject               <- PsJobType
    // 
    // "Port" / "ALPC Port" -> Ipc_CheckPortObject      <- AlpcPortObjectType and LpcWaitablePortObjectType not exported, LpcPortObjectType exported
    //      Note: proper  IPC isolation requires filtering of NtRequestPort, NtRequestWaitReplyPort, and NtAlpcSendWaitReceivePort calls
    // 
    // "Token"      -> Thread_CheckTokenObject          <- SeTokenObjectType
    //

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
        Obj_FilterCookie = NULL;
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
        Obj_FilterCookie = NULL;

        Obj_CallbackInstalled = FALSE;
    }
}


//---------------------------------------------------------------------------
// Obj_PreOperationCallback
//---------------------------------------------------------------------------


_FX OB_PREOP_CALLBACK_STATUS Obj_PreOperationCallback(
    _In_ PVOID RegistrationContext, _Inout_ POB_PRE_OPERATION_INFORMATION PreInfo)
{
    //
    // Filter only if request made outside of the kernel
    //

    //if (ExGetPreviousMode() == KernelMode)
    if (PreInfo->KernelHandle == 1)
        return OB_PREOP_SUCCESS;

    //
    // Get information about the intended operation
    //

    PACCESS_MASK DesiredAccess = NULL;
    ACCESS_MASK InitialDesiredAccess  = 0;
    //ACCESS_MASK OriginalDesiredAccess = 0;

    switch (PreInfo->Operation) {
    case OB_OPERATION_HANDLE_CREATE:
        DesiredAccess = &PreInfo->Parameters->CreateHandleInformation.DesiredAccess;
        //OriginalDesiredAccess = PreInfo->Parameters->CreateHandleInformation.OriginalDesiredAccess;
        //OperationName = L"OB_OPERATION_HANDLE_CREATE";
        break;

    case OB_OPERATION_HANDLE_DUPLICATE:
        DesiredAccess = &PreInfo->Parameters->DuplicateHandleInformation.DesiredAccess;
        //OriginalDesiredAccess = PreInfo->Parameters->DuplicateHandleInformation.OriginalDesiredAccess;
        //OperationName = L"OB_OPERATION_HANDLE_DUPLICATE";
        break;

    default:
        DbgPrint("Sbie ObCallback: unexpected callback type\n");
        goto Exit;
    }

    InitialDesiredAccess = *DesiredAccess;

    //
    // Based on the object type apply the appropriate filter
    //

    if (PreInfo->ObjectType == *PsProcessType)  {

        HANDLE TargetProcessId = PsGetProcessId((PEPROCESS)PreInfo->Object);
        PEPROCESS ProcessObject = (PEPROCESS)PreInfo->Object;
        *DesiredAccess = Thread_CheckObject_CommonEx(TargetProcessId, ProcessObject, InitialDesiredAccess, TRUE, TRUE);
    }
    else if (PreInfo->ObjectType == *PsThreadType)  {

        HANDLE TargetProcessId = PsGetThreadProcessId ((PETHREAD)PreInfo->Object);
        PEPROCESS ProcessObject = PsGetThreadProcess((PETHREAD)PreInfo->Object);
        *DesiredAccess = Thread_CheckObject_CommonEx(TargetProcessId, ProcessObject, InitialDesiredAccess, FALSE, TRUE);
    }
    else {
        DbgPrint("Sbie ObCallback: unexpected object type\n");
        goto Exit;
    }

Exit:
    return OB_PREOP_SUCCESS;
}


//---------------------------------------------------------------------------
// Obj_PostOperationCallback
//---------------------------------------------------------------------------


//_FX VOID Obj_PostOperationCallback(
//    _In_ PVOID RegistrationContext, _In_ POB_POST_OPERATION_INFORMATION PostInfo)
//{
//}
