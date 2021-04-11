/*
 * Copyright 2004-2020 Sandboxie Holdings, LLC 
 * Copyright 2020 David Xanatos, xanasoft.com
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
// Driver API
//---------------------------------------------------------------------------


#include "api.h"
#include "process.h"
#include "util.h"
#include "hook.h"
#include "common/my_version.h"
#include "log_buff.h"


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------


static KIRQL Api_EnterCriticalSection(void);

static void Api_LeaveCriticalSection(KIRQL oldirql);

static NTSTATUS Api_Irp_Finish(IRP *irp, NTSTATUS status);

static NTSTATUS Api_Irp_CREATE(DEVICE_OBJECT *device_object, IRP *irp);

static NTSTATUS Api_Irp_CLEANUP(DEVICE_OBJECT *device_object, IRP *irp);

static BOOLEAN Api_FastIo_DEVICE_CONTROL(
    FILE_OBJECT *FileObject, BOOLEAN Wait,
    void *InputBuffer, ULONG InputBufferLength,
    void *OutputBuffer, ULONG OutputBufferLength,
    ULONG IoControlCode, IO_STATUS_BLOCK *IoStatus,
    DEVICE_OBJECT *DeviceObject);

//static void Api_DelWork(API_WORK_ITEM *work_item);


//---------------------------------------------------------------------------


static NTSTATUS Api_GetVersion(PROCESS *proc, ULONG64 *parms);

static NTSTATUS Api_LogMessage(PROCESS *proc, ULONG64 *parms);

static NTSTATUS Api_GetMessage(PROCESS *proc, ULONG64 *parms);

//static NTSTATUS Api_GetWork(PROCESS *proc, ULONG64 *parms);

static NTSTATUS Api_GetHomePath(PROCESS *proc, ULONG64 *parms);

static NTSTATUS Api_SetServicePort(PROCESS *proc, ULONG64 *parms);

static NTSTATUS Api_ProcessExemptionControl(PROCESS *proc, ULONG64 *parms);


//---------------------------------------------------------------------------


#ifdef ALLOC_PRAGMA
#pragma alloc_text (INIT, Api_Init)
#pragma alloc_text (INIT, Api_SetFunction)
#endif // ALLOC_PRAGMA


//---------------------------------------------------------------------------
// Variables
//---------------------------------------------------------------------------


static P_Api_Function *Api_Functions = NULL;

static DEVICE_OBJECT *Api_DeviceObject = NULL;

static FAST_IO_DISPATCH *Api_FastIoDispatch = NULL;

static void *Api_ServicePortObject = NULL;

volatile HANDLE Api_ServiceProcessId = NULL;

static PERESOURCE Api_LockResource = NULL;

//static LIST Api_WorkList;
static BOOLEAN Api_WorkListInitialized = FALSE;

static LOG_BUFFER* Api_LogBuffer = NULL;

static volatile LONG Api_UseCount = -1;


//---------------------------------------------------------------------------
// Api_Init
//---------------------------------------------------------------------------


_FX BOOLEAN Api_Init(void)
{
    NTSTATUS status;
    UNICODE_STRING uni;

	//
	// initialize log buffer
	//

	Api_LogBuffer = log_buffer_init(8 * 8 * 1024);

    //
    // initialize work list
    //

    //List_Init(&Api_WorkList);

    if (! Mem_GetLockResource(&Api_LockResource, TRUE))
        return FALSE;

    Api_WorkListInitialized = TRUE;

    //
    // initialize Fast IO dispatch pointers
    //

    Api_FastIoDispatch = ExAllocatePoolWithTag(NonPagedPool, sizeof(FAST_IO_DISPATCH), tzuk);
    if (! Api_FastIoDispatch) {
        Log_Status(MSG_API_DEVICE, 0, STATUS_INSUFFICIENT_RESOURCES);
        return FALSE;
    }

    memzero(Api_FastIoDispatch, sizeof(FAST_IO_DISPATCH));
    Api_FastIoDispatch->SizeOfFastIoDispatch = sizeof(FAST_IO_DISPATCH);
    Api_FastIoDispatch->FastIoDeviceControl = Api_FastIo_DEVICE_CONTROL;

    Driver_Object->FastIoDispatch = Api_FastIoDispatch;

    //
    // initialize IRP dispatch pointers
    //

    Driver_Object->MajorFunction[IRP_MJ_CREATE] = Api_Irp_CREATE;
    Driver_Object->MajorFunction[IRP_MJ_CLEANUP] = Api_Irp_CLEANUP;

    //
    // create device object
    //

    RtlInitUnicodeString(&uni, API_DEVICE_NAME);
    status = IoCreateDevice(
        Driver_Object, 0, &uni,
        FILE_DEVICE_UNKNOWN, FILE_DEVICE_SECURE_OPEN, FALSE,
        &Api_DeviceObject);

    if (! NT_SUCCESS(status)) {
        Api_DeviceObject = NULL;
        Log_Status(MSG_API_DEVICE, 0, status);
        return FALSE;
    }

    Api_DeviceObject->Flags &= ~DO_DEVICE_INITIALIZING;

    //
    // set API functions
    //

    Api_SetFunction(API_GET_VERSION,        Api_GetVersion);
    //Api_SetFunction(API_GET_WORK,           Api_GetWork);
    Api_SetFunction(API_LOG_MESSAGE,        Api_LogMessage);
	Api_SetFunction(API_GET_MESSAGE,        Api_GetMessage);
    Api_SetFunction(API_GET_HOME_PATH,      Api_GetHomePath);
    Api_SetFunction(API_SET_SERVICE_PORT,   Api_SetServicePort);

    Api_SetFunction(API_UNLOAD_DRIVER,      Driver_Api_Unload);

    //Api_SetFunction(API_HOOK_TRAMP,         Hook_Api_Tramp);

	Api_SetFunction(API_PROCESS_EXEMPTION_CONTROL, Api_ProcessExemptionControl);

    if ((! Api_Functions) || (Api_Functions == (void *)-1))
        return FALSE;

    //
    // indicate API is ready for use
    //

    InterlockedExchange(&Api_UseCount, 0);

    return TRUE;
}


//---------------------------------------------------------------------------
// Api_Unload
//---------------------------------------------------------------------------


_FX void Api_Unload(void)
{
    if (Api_DeviceObject) {
        IoDeleteDevice(Api_DeviceObject);
        Api_DeviceObject = NULL;
    }

    if (Api_FastIoDispatch) {
        ExFreePoolWithTag(Api_FastIoDispatch, tzuk);
        Api_FastIoDispatch = NULL;
    }

    if (Api_WorkListInitialized) {

		if (Api_LogBuffer) {
			log_buffer_free(Api_LogBuffer);
			Api_LogBuffer = NULL;
		}

        /*API_WORK_ITEM *work_item;
		while (1) {
            work_item = List_Head(&Api_WorkList);
            if (! work_item)
                break;
            Api_DelWork(work_item);
        }*/

        Mem_FreeLockResource(&Api_LockResource);

        Api_WorkListInitialized = FALSE;
    }

    if (Api_ServicePortObject) {
        ObDereferenceObject(Api_ServicePortObject);
        Api_ServicePortObject = NULL;
    }
}


//---------------------------------------------------------------------------
// Api_SetFunction
//---------------------------------------------------------------------------


_FX void Api_SetFunction(ULONG func_code, P_Api_Function func_ptr)
{
    if (! Api_Functions) {

        ULONG len = (API_LAST - API_FIRST - 1) * sizeof(P_Api_Function);
        Api_Functions = Mem_AllocEx(Driver_Pool, len, TRUE);

        if (Api_Functions)
            memzero(Api_Functions, len);
        else
            Api_Functions = (void *)-1;
    }

    if ((Api_Functions != (void *)-1) &&
            (func_code > API_FIRST) && (func_code < API_LAST)) {

        Api_Functions[func_code - API_FIRST - 1] = func_ptr;
    }
}


//---------------------------------------------------------------------------
// Api_EnterCriticalSection
//---------------------------------------------------------------------------


_FX KIRQL Api_EnterCriticalSection(void)
{
    KIRQL irql;

    KeRaiseIrql(APC_LEVEL, &irql);
    ExAcquireResourceExclusiveLite(Api_LockResource, TRUE);

    return irql;
}


//---------------------------------------------------------------------------
// Api_LeaveCriticalSection
//---------------------------------------------------------------------------


_FX void Api_LeaveCriticalSection(KIRQL oldirql)
{
    ExReleaseResourceLite(Api_LockResource);
    KeLowerIrql(oldirql);
}


//---------------------------------------------------------------------------
// Api_Disable
//---------------------------------------------------------------------------


_FX BOOLEAN Api_Disable(void)
{
    BOOLEAN CanDisable = FALSE;

    if (Api_DeviceObject) {

        //
        // disable the api device only if it is currently in use by a
        // single client -- typically that's KmdUtil asking to stop SbieDrv
        //

        KIRQL irql = Api_EnterCriticalSection();

        if (InterlockedCompareExchange(&Api_UseCount, -1, 1) == 1)
            CanDisable = TRUE;

        Api_LeaveCriticalSection(irql);
    }

    return CanDisable;
}


//---------------------------------------------------------------------------
// Api_Irp_Finish
//---------------------------------------------------------------------------


_FX NTSTATUS Api_Irp_Finish(IRP *irp, NTSTATUS status)
{
    irp->IoStatus.Status = status;
    irp->IoStatus.Information = 0;
    IoCompleteRequest(irp, IO_NO_INCREMENT);

    return status;
}


//---------------------------------------------------------------------------
// Api_Irp_CREATE
//---------------------------------------------------------------------------


_FX NTSTATUS Api_Irp_CREATE(DEVICE_OBJECT *device_object, IRP *irp)
{
    NTSTATUS status;
    KIRQL irql = Api_EnterCriticalSection();

    if (Api_UseCount == -1)
        status = Api_Irp_Finish(irp, STATUS_NOT_SUPPORTED);
    else {
        InterlockedIncrement(&Api_UseCount);
        status = Api_Irp_Finish(irp, STATUS_SUCCESS);
    }

    Api_LeaveCriticalSection(irql);
    return status;
}


//---------------------------------------------------------------------------
// Api_Irp_CLEANUP
//---------------------------------------------------------------------------


_FX NTSTATUS Api_Irp_CLEANUP(DEVICE_OBJECT *device_object, IRP *irp)
{
    NTSTATUS status;
    KIRQL irql = Api_EnterCriticalSection();

    if (Api_UseCount != -1) {
        InterlockedDecrement(&Api_UseCount);
    }
    status = Api_Irp_Finish(irp, STATUS_SUCCESS);

    Api_LeaveCriticalSection(irql);
    return status;
}


//---------------------------------------------------------------------------
// Api_FastIo_DEVICE_CONTROL
//---------------------------------------------------------------------------


_FX BOOLEAN Api_FastIo_DEVICE_CONTROL(
    FILE_OBJECT *FileObject, BOOLEAN Wait,
    void *InputBuffer, ULONG InputBufferLength,
    void *OutputBuffer, ULONG OutputBufferLength,
    ULONG IoControlCode, IO_STATUS_BLOCK *IoStatus,
    DEVICE_OBJECT *DeviceObject)
{
    NTSTATUS status;
    ULONG buf_len, func_code;
    ULONG64 *buf;
    ULONG64 user_args[API_NUM_ARGS];
    PROCESS *proc;
    P_Api_Function func_ptr;
    BOOLEAN ApcsDisabled;

    //
    // SeFilterToken in kernel mode
    //

    if (ExGetPreviousMode() == KernelMode && IoControlCode == API_SBIEDRV_FILTERTOKEN_CTLCODE)
    {
        Sbie_SeFilterTokenArg* arg = (Sbie_SeFilterTokenArg *)InputBuffer;

        if (InputBufferLength != sizeof(Sbie_SeFilterTokenArg))
        {
            IoStatus->Status = STATUS_INFO_LENGTH_MISMATCH;
            *arg->status = STATUS_INFO_LENGTH_MISMATCH;
            return TRUE;
        }

        *arg->status = SeFilterToken(arg->ExistingToken, arg->Flags, arg->SidsToDisable, arg->PrivilegesToDelete, arg->RestrictedSids, arg->NewToken);

        return TRUE;
    }
    else if (ExGetPreviousMode() == KernelMode && IoControlCode == API_SBIEDRV_PFILTERTOKEN_CTLCODE)
    {
        Sbie_SepFilterTokenArg* arg = (Sbie_SepFilterTokenArg *)InputBuffer;

        if (InputBufferLength != sizeof(Sbie_SepFilterTokenArg))
        {
            IoStatus->Status = STATUS_INFO_LENGTH_MISMATCH;
            *arg->status = STATUS_INFO_LENGTH_MISMATCH;
            return TRUE;
        }

        *arg->status = Sbie_SepFilterTokenHandler(arg->TokenObject, arg->SidCount, arg->SidPtr, arg->LengthIncrease, arg->NewToken);

        return TRUE;
    }


    //
    // we only handle direct invocations by user-mode callers at low irql
    //

    IoStatus->Information = 0;

    if (KeGetCurrentIrql() != PASSIVE_LEVEL
                                    || ExGetPreviousMode() != UserMode) {

        IoStatus->Status = STATUS_INVALID_LEVEL;
        return TRUE;
    }

    //
    // get pointer to user buffer and length
    //

    buf = NULL;

    if (IoControlCode == API_SBIEDRV_CTLCODE) {

        buf_len = InputBufferLength;
        if (buf_len >= sizeof(ULONG64)
                            && buf_len <= sizeof(ULONG64) * API_NUM_ARGS)
            buf = InputBuffer;
    }

    if (! buf) {

        IoStatus->Status = STATUS_INVALID_DEVICE_REQUEST;
        return TRUE;
    }

    //
    // find calling process
    //

    ApcsDisabled = KeAreApcsDisabled();

    if (PsGetCurrentProcessId() == Api_ServiceProcessId)
        proc = NULL;
    else {

        proc = Process_Find(NULL, NULL);
        if (proc == PROCESS_TERMINATED) {

            IoStatus->Status = STATUS_PROCESS_IS_TERMINATING;
            return TRUE;
        }
    }

    //
    // capture parameter and call function
    //

    func_code = 0;
    func_ptr = NULL;

    __try {

        ProbeForRead(
            buf, sizeof(ULONG64) * API_NUM_ARGS, sizeof(ULONG64));

        memzero(user_args, sizeof(ULONG64) * API_NUM_ARGS);
        memcpy(user_args, buf, buf_len);

        func_code = (ULONG)user_args[0];

        if (func_code > API_FIRST && func_code < API_LAST)
            func_ptr = Api_Functions[func_code - API_FIRST - 1];

        if (func_ptr) {

            status = func_ptr(proc, user_args);

        } else
            status = STATUS_INVALID_DEVICE_REQUEST;

    } __except (EXCEPTION_EXECUTE_HANDLER) {
        status = GetExceptionCode();
    }

    //
    // make sure IRQL and APCs were restored correctly
    //

#ifdef _DEBUG
    if (KeGetCurrentIrql() != PASSIVE_LEVEL
                                || (KeAreApcsDisabled() ^ ApcsDisabled)) {

        KeBugCheckEx(DRIVER_CORRUPTED_MMPOOL,
                     tzuk, KeGetCurrentIrql(), 0x123400, func_code);
    }
#endif

    IoStatus->Status = status;
    return TRUE;
}


//---------------------------------------------------------------------------
// Api_GetVersion
//---------------------------------------------------------------------------


_FX NTSTATUS Api_GetVersion(PROCESS *proc, ULONG64 *parms)
{
    API_GET_VERSION_ARGS *args = (API_GET_VERSION_ARGS *)parms;
    size_t len;
    WCHAR *pwcResult = args->string.val;

    if (pwcResult == NULL)
        return STATUS_INVALID_PARAMETER;
    len = (wcslen(Driver_Version) + 1) * sizeof(WCHAR);
    ProbeForWrite(pwcResult, len, sizeof(WCHAR));
    memcpy(pwcResult, Driver_Version, len);

    return STATUS_SUCCESS;
}


//---------------------------------------------------------------------------
// Api_LogMessage
//---------------------------------------------------------------------------


_FX NTSTATUS Api_LogMessage(PROCESS *proc, ULONG64 *parms)
{
    API_LOG_MESSAGE_ARGS *args = (API_LOG_MESSAGE_ARGS *)parms;
    NTSTATUS status;
    ULONG msgid;
    UNICODE_STRING64 *msgtext;
    ULONG msgtext_length;
    WCHAR *msgtext_buffer;
    POOL *pool;
    WCHAR *text;
	HANDLE pid;

    msgid = args->msgid.val;
    if (msgid >= 2101 && msgid <= 2199)
        msgid = msgid - 2101 + MSG_2101;
    else if (msgid >= 2201 && msgid <= 2299)
        msgid = msgid - 2201 + MSG_2201;
    else if (msgid >= 2301 && msgid <= 2399)
        msgid = msgid - 2301 + MSG_2301;
    else if (msgid == 1314)
        msgid = MSG_1314;
    else if (msgid == 1307)
        msgid = MSG_1307;
    else
        msgid = MSG_2301; // unknown message

    msgtext = args->msgtext.val;
    if (! msgtext)
        return STATUS_INVALID_PARAMETER;
    ProbeForRead(msgtext, sizeof(UNICODE_STRING64), sizeof(ULONG));

    msgtext_length = msgtext->Length;
    if (msgtext_length > API_LOG_MESSAGE_MAX_LEN)
        msgtext_length = API_LOG_MESSAGE_MAX_LEN;

    msgtext_buffer = (WCHAR *)msgtext->Buffer;
    if (! msgtext_buffer)
        return STATUS_INVALID_PARAMETER;
    ProbeForRead(msgtext_buffer, msgtext_length, sizeof(WCHAR));

	pid = (HANDLE)args->process_id.val;
	if (proc) {
		pool = proc->pool;
		if (!pid) pid = proc->pid;
	}
	else {
		pool = Driver_Pool;
		if (!pid) pid = PsGetCurrentProcessId();
	}

    text = Mem_Alloc(pool, msgtext_length + 8);
    if (! text)
        return STATUS_INSUFFICIENT_RESOURCES;

    __try {
        memcpy(text, msgtext_buffer, msgtext_length);
        status = STATUS_SUCCESS;
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        status = GetExceptionCode();
    }

    if (status == STATUS_SUCCESS) {
        text[msgtext_length / sizeof(WCHAR)] = L'\0';
		Log_Popup_MsgEx(msgid, text, msgtext_length / sizeof(WCHAR), NULL, 0, args->session_id.val, pid);
    }

    Mem_Free(text, msgtext_length + 8);

    return status;
}


//---------------------------------------------------------------------------
// Api_AddMessage
//---------------------------------------------------------------------------


_FX void Api_AddMessage(
	NTSTATUS error_code,
	const WCHAR *string1, ULONG string1_len,
	const WCHAR *string2, ULONG string2_len,
	ULONG session_id,
	ULONG process_id)
{
	KIRQL irql;

	if (!Api_WorkListInitialized) // if (!Api_LogBuffer)
		return;

	//
	// add work at the end of the work list
	//

	irql = Api_EnterCriticalSection();

	ULONG entry_size = sizeof(ULONG)	// session_id
		+ sizeof(ULONG)					// process_id
		+ sizeof(ULONG)					// error_code
		+ (string1_len + 1) * sizeof(WCHAR)
		+ (string2_len + 1) * sizeof(WCHAR);

	CHAR* write_ptr = log_buffer_push_entry((LOG_BUFFER_SIZE_T)entry_size, Api_LogBuffer);
	if (write_ptr) {
		//[session_id 4][process_id 4][error_code 4][string1 n*2][\0 2][string2 n*2][\0 2]
		WCHAR null_char = L'\0';
		log_buffer_push_bytes((CHAR*)&session_id, sizeof(ULONG), &write_ptr, Api_LogBuffer);
		log_buffer_push_bytes((CHAR*)&process_id, sizeof(ULONG), &write_ptr, Api_LogBuffer);
		log_buffer_push_bytes((CHAR*)&error_code, sizeof(ULONG), &write_ptr, Api_LogBuffer);
		log_buffer_push_bytes((CHAR*)string1, string1_len * sizeof(WCHAR), &write_ptr, Api_LogBuffer);
		log_buffer_push_bytes((CHAR*)&null_char, sizeof(WCHAR), &write_ptr, Api_LogBuffer);
		log_buffer_push_bytes((CHAR*)string2, string2_len * sizeof(WCHAR), &write_ptr, Api_LogBuffer);
		log_buffer_push_bytes((CHAR*)&null_char, sizeof(WCHAR), &write_ptr, Api_LogBuffer);
	}
	// else // this can only happen when the entire buffer is to small to hold this entire entry
		// if loging fails we can't log this error :/

	Api_LeaveCriticalSection(irql);
}


//---------------------------------------------------------------------------
// Api_GetMessage
//---------------------------------------------------------------------------


_FX NTSTATUS Api_GetMessage(PROCESS *proc, ULONG64 *parms)
{
	API_GET_MESSAGE_ARGS *args = (API_GET_MESSAGE_ARGS *)parms;
	NTSTATUS status = STATUS_SUCCESS;
	UNICODE_STRING64 *msgtext;
	WCHAR *msgtext_buffer;
	KIRQL irql;

	if (proc) // sandboxed processes can't read the log
		return STATUS_NOT_IMPLEMENTED;

	ProbeForRead(args->msg_num.val, sizeof(ULONG), sizeof(ULONG));
	ProbeForWrite(args->msg_num.val, sizeof(ULONG), sizeof(ULONG));

	ProbeForWrite(args->msgid.val, sizeof(ULONG), sizeof(ULONG));

	msgtext = args->msgtext.val;
	if (!msgtext)
		return STATUS_INVALID_PARAMETER;
	ProbeForRead(msgtext, sizeof(UNICODE_STRING64), sizeof(ULONG));
	ProbeForWrite(msgtext, sizeof(UNICODE_STRING64), sizeof(ULONG));

	msgtext_buffer = (WCHAR *)msgtext->Buffer;
	if (!msgtext_buffer)
		return STATUS_INVALID_PARAMETER;

	irql = Api_EnterCriticalSection();

	__try {

		LOG_BUFFER_SEQ_T seq_number = *args->msg_num.val;
		for (;;) {

			CHAR* read_ptr = log_buffer_get_next(seq_number, Api_LogBuffer);
			if (!read_ptr) {

				status = STATUS_NO_MORE_ENTRIES;
				break;
			}

			LOG_BUFFER_SIZE_T entry_size = log_buffer_get_size(&read_ptr, Api_LogBuffer);
			seq_number = log_buffer_get_seq_num(&read_ptr, Api_LogBuffer);

			//if (seq_number != *args->msg_num.val + 1) {
			//
			//	status = STATUS_REQUEST_OUT_OF_SEQUENCE;
			//	*args->msg_num.val = seq_number - 1;
			//	break;
			//}

			//[session_id 4][process_id 4][error_code 4][string1 n*2][\0 2][string2 n*2][\0 2]...[stringN n*2][\0 2][\0 2]

			ULONG session_id;
			log_buffer_get_bytes((CHAR*)&session_id, 4, &read_ptr, Api_LogBuffer);
			entry_size -= 4;

			if (args->session_id.val != -1 && session_id != args->session_id.val) // Note: the service (session_id == -1) gets all the entries
				continue;

			ULONG process_id;
			log_buffer_get_bytes((CHAR*)&process_id, 4, &read_ptr, Api_LogBuffer);
			entry_size -= 4;

			log_buffer_get_bytes((CHAR*)args->msgid.val, 4, &read_ptr, Api_LogBuffer);
			entry_size -= 4;

			if (args->process_id.val != NULL)
			{
				ProbeForWrite(args->process_id.val, sizeof(ULONG), sizeof(ULONG));
				*args->process_id.val = process_id;
			}

			// we return all strings in one
			if (entry_size <= msgtext->MaximumLength)
			{
				msgtext->Length = (USHORT)entry_size;
				ProbeForWrite(msgtext_buffer, entry_size, sizeof(WCHAR));
				memcpy(msgtext_buffer, read_ptr, entry_size);
			}
			else
			{
				status = STATUS_BUFFER_TOO_SMALL;
			}

			*args->msg_num.val = seq_number; // update when everything went fine
			break;
		}

	} __except (EXCEPTION_EXECUTE_HANDLER) {
		status = GetExceptionCode();
	}

	Api_LeaveCriticalSection(irql);

	return status;
}


//---------------------------------------------------------------------------
// Api_SendServiceMessage
//---------------------------------------------------------------------------


_FX void Api_ResetServiceProcess(void)
{
    if (Api_ServicePortObject) {

        KIRQL irql = Api_EnterCriticalSection();

        void *Old = InterlockedExchangePointer(&Api_ServicePortObject, NULL);

        ObDereferenceObject(Old);

        Api_LeaveCriticalSection(irql);
    }

    InterlockedExchangePointer(&Api_ServiceProcessId, NULL);
}


//---------------------------------------------------------------------------
// Api_SendServiceMessage
//---------------------------------------------------------------------------


_FX BOOLEAN Api_SendServiceMessage(ULONG msgid, ULONG data_len, void *data)
{
    UCHAR space[MAX_PORTMSG_LENGTH];
    PORT_MESSAGE *msg = (PORT_MESSAGE *)space;
    void *PortObject;
    KIRQL irql;
    BOOLEAN ok;

    //
    // abort if we know in advance that we won't have a service port
    //

    if (! Api_ServiceProcessId)
        return FALSE;

    //
    // prepare the request message structure.  the data area of the message
    // follows the PORT_MESSAGE header.  the first ULONG is the msgid field,
    // followed by the rest of the caller data
    //

    if (data_len > API_MAX_SVC_DATA_LEN)
        return FALSE;

    memzero(msg, sizeof(PORT_MESSAGE));
    msg->u1.s1.DataLength = (USHORT)(data_len + sizeof(ULONG));
    msg->u1.s1.TotalLength = msg->u1.s1.DataLength + sizeof(PORT_MESSAGE);
    msg->u2.s2.Type = LPC_DATAGRAM;

    *(ULONG *)(space + sizeof(PORT_MESSAGE)) = msgid;
    memcpy(space + sizeof(PORT_MESSAGE) + sizeof(ULONG), data, data_len);

    //
    // send the message to SbieSvc on the LPC port
    //

    irql = Api_EnterCriticalSection();

    PortObject = Api_ServicePortObject;

    if (PortObject)
        ObReferenceObject(PortObject);

    Api_LeaveCriticalSection(irql);

    if (PortObject) {

        // port must have a name, or LpcRequestPort will fail
        // see also core/svc/driverassist.cpp
        NTSTATUS status = LpcRequestPort(PortObject, msg);

        ObDereferenceObject(PortObject);

        if (NT_SUCCESS(status))
            ok = TRUE;
        else
            ok = FALSE;

    } else
        ok = FALSE;

    return ok;
}


//---------------------------------------------------------------------------
// Api_AddWork
//---------------------------------------------------------------------------


/*_FX BOOLEAN Api_AddWork(API_WORK_ITEM *work_item)
{
    KIRQL irql;

    if (! Api_WorkListInitialized)
        return FALSE;

    //
    // add work at the end of the work list
    //

    irql = Api_EnterCriticalSection();

    List_Insert_After(&Api_WorkList, NULL, work_item);

    Api_LeaveCriticalSection(irql);

    //
    // set the work event so SbieSvc wakes up
    //

    if (work_item->session_id != -1)
        return TRUE;

    return TRUE;
}*/


//---------------------------------------------------------------------------
// Api_DelWork
//---------------------------------------------------------------------------


/*_FX void Api_DelWork(API_WORK_ITEM *work_item)
{
    // this assumes Api_WorkList is already locked using Api_Lock

    List_Remove(&Api_WorkList, work_item);
    Mem_Free(work_item, work_item->length);
}*/


//---------------------------------------------------------------------------
// Api_GetWork
//---------------------------------------------------------------------------

//
//_FX NTSTATUS Api_GetWork(PROCESS *proc, ULONG64 *parms)
//{
//	return STATUS_NOT_IMPLEMENTED;
//
//    /*API_GET_WORK_ARGS *args = (API_GET_WORK_ARGS *)parms;
//    NTSTATUS status;
//    void *buffer_ptr;
//    ULONG buffer_len;
//    ULONG *result_len;
//    ULONG length;
//    API_WORK_ITEM *work_item;
//    KIRQL irql;
//
//    //
//    // caller must not be sandboxed, and caller has to be SbieSvc
//    // if session parameter is -1
//    //
//
//    if (proc)
//        return STATUS_NOT_IMPLEMENTED;
//
//    if (args->session_id.val == -1 &&
//            PsGetCurrentProcessId() != Api_ServiceProcessId)
//        return STATUS_ACCESS_DENIED;
//
//    //
//    // find next work/log item for the session
//    //
//
//    buffer_ptr = args->buffer.val;
//    buffer_len = args->buffer_len.val;
//    result_len = args->result_len_ptr.val;
//
//    irql = Api_EnterCriticalSection();
//
//    work_item = List_Head(&Api_WorkList);
//    while (work_item) {
//        if (work_item->session_id == args->session_id.val)
//            break;
//        work_item = List_Next(work_item);
//    }
//
//    __try {
//
//    if (! work_item) {
//
//        status = STATUS_NO_MORE_ENTRIES;
//
//    } else {
//
//        if (work_item->length <= buffer_len) {
//
//            length = work_item->length
//                   - FIELD_OFFSET(API_WORK_ITEM, type);
//            ProbeForWrite(buffer_ptr, length, sizeof(UCHAR));
//            memcpy(buffer_ptr, &work_item->type, length);
//
//            status = STATUS_SUCCESS;
//
//        } else {
//
//            length = work_item->length;
//            status = STATUS_BUFFER_TOO_SMALL;
//        }
//
//        if (result_len) {
//            ProbeForWrite(result_len, sizeof(ULONG), sizeof(ULONG));
//            *result_len = length;
//        }
//
//        if (status == STATUS_SUCCESS)
//            Api_DelWork(work_item);
//    }
//
//    } __except (EXCEPTION_EXECUTE_HANDLER) {
//        status = GetExceptionCode();
//    }
//
//    Api_LeaveCriticalSection(irql);
//
//    return status;*/
//}


//---------------------------------------------------------------------------
// Api_GetHomePath
//---------------------------------------------------------------------------


_FX NTSTATUS Api_GetHomePath(PROCESS *proc, ULONG64 *parms)
{
    API_GET_HOME_PATH_ARGS *args = (API_GET_HOME_PATH_ARGS *)parms;
    UNICODE_STRING64 *user_uni;
    WCHAR *ptr;
    size_t len;

    user_uni = args->nt_path.val;
    if (user_uni) {
        ptr = Driver_HomePathNt;
        len = (wcslen(ptr) + 1) * sizeof(WCHAR);
        Api_CopyStringToUser(user_uni, ptr, len);
    }

    user_uni = args->dos_path.val;
    if (user_uni) {
        ptr = Driver_HomePathDos;
        if (wcsncmp(ptr, L"\\??\\", 4) == 0)
            ptr += 4;
        len = (wcslen(ptr) + 1) * sizeof(WCHAR);
        Api_CopyStringToUser(user_uni, ptr, len);
    }

    return STATUS_SUCCESS;
}



//---------------------------------------------------------------------------
// Api_SetServicePort
//---------------------------------------------------------------------------


_FX NTSTATUS Api_SetServicePort(PROCESS *proc, ULONG64 *parms)
{
    //
    // confirm the caller is an unsandboxed system process in
    // our installation folder, i.e. the Sandboxie service
    //

    NTSTATUS status = STATUS_ACCESS_DENIED;

    if ((! proc) && MyIsCallerMyServiceProcess()) {

        status = STATUS_SUCCESS;
    }

    //
    // take a reference on the specified LPC port object
    //

    if (NT_SUCCESS(status)) {

        void *PortObject, *OldObject;

        HANDLE PortHandle = (HANDLE)(ULONG_PTR)parms[1];
        if (PortHandle) {

            status = ObReferenceObjectByHandle(
                        PortHandle, 0, *LpcPortObjectType, KernelMode,
                        &PortObject, NULL);
        } else {

            PortObject = NULL;
            status = STATUS_SUCCESS;
        }

        //
        // replace the stored port object reference,
        // release the reference on the old stored port object
        //

        if (NT_SUCCESS(status)) {

            KIRQL irql = Api_EnterCriticalSection();

            OldObject = InterlockedExchangePointer(
                                    &Api_ServicePortObject, PortObject);

            InterlockedExchangePointer(
                &Api_ServiceProcessId, PsGetCurrentProcessId());

            Api_LeaveCriticalSection(irql);

            if (OldObject)
                ObDereferenceObject(OldObject);
        }
    }

    return status;
}


//---------------------------------------------------------------------------
// Api_CopyBoxNameFromUser
//---------------------------------------------------------------------------


_FX BOOLEAN Api_CopyBoxNameFromUser(
    WCHAR *boxname34, const WCHAR *user_boxname)
{
    wmemzero(boxname34, 34);
    if (user_boxname) {
        ProbeForRead((WCHAR *)user_boxname,
                     sizeof(WCHAR) * 32,
                     sizeof(UCHAR));
        if (user_boxname[0])
            wcsncpy(boxname34, user_boxname, 32);
    }
    if (boxname34[0] && Box_IsValidName(boxname34))
        return TRUE;
    return FALSE;
}


//---------------------------------------------------------------------------
// Api_CopySidStringFromUser
//---------------------------------------------------------------------------


_FX BOOLEAN Api_CopySidStringFromUser(
    WCHAR *sidstring96, const WCHAR *user_sidstring)
{
    wmemzero(sidstring96, 96);
    if (user_sidstring) {
        ProbeForRead(
            (WCHAR *)user_sidstring, sizeof(WCHAR) * 96, sizeof(UCHAR));
        if (user_sidstring[0])
            wcsncpy(sidstring96, user_sidstring, 94);
    }
    if (sidstring96[0] == L'S' && sidstring96[1] == L'-')
        return TRUE;
    return FALSE;
}


//---------------------------------------------------------------------------
// Api_CopyStringToUser
//---------------------------------------------------------------------------


_FX void Api_CopyStringToUser(
    UNICODE_STRING64 *uni, WCHAR *str, size_t len)
{
    if (uni) {
        ProbeForRead(uni, sizeof(UNICODE_STRING64), sizeof(ULONG_PTR));
        ProbeForWrite(uni, sizeof(UNICODE_STRING64), sizeof(ULONG_PTR));
        if (len > uni->MaximumLength)
            ExRaiseStatus(STATUS_BUFFER_TOO_SMALL);
        else {
            WCHAR *buf = (WCHAR *)uni->Buffer;
            ProbeForWrite(buf, len, sizeof(WCHAR));
            if (len) {
                memcpy(buf, str, len);
                uni->Length = (USHORT)len - sizeof(WCHAR);
            } else
                uni->Length = 0;
        }
    }
}


//---------------------------------------------------------------------------
// Api_ProcessExemptionControl
//---------------------------------------------------------------------------

_FX NTSTATUS Api_ProcessExemptionControl(PROCESS *proc, ULONG64 *parms)
{
    NTSTATUS status = STATUS_SUCCESS;
    //KIRQL irql;
	API_PROCESS_EXEMPTION_CONTROL_ARGS *pArgs = (API_PROCESS_EXEMPTION_CONTROL_ARGS *)parms;
	ULONG *in_flag;
	ULONG *out_flag;

	if (proc) // is caller sandboxed?
		return STATUS_ACCESS_DENIED;

	if (pArgs->process_id.val == 0)
		return STATUS_INVALID_PARAMETER;

	in_flag = pArgs->set_flag.val;
	if (in_flag) {
		ProbeForRead(in_flag, sizeof(ULONG), sizeof(ULONG));
	}

	out_flag = pArgs->get_flag.val;
	if (out_flag) {
		ProbeForWrite(out_flag, sizeof(ULONG), sizeof(ULONG));
	}

	if(!in_flag && !out_flag)
		return STATUS_INVALID_PARAMETER;

    //proc = Process_Find(pArgs->process_id.val, &irql);
    proc = Process_Find(pArgs->process_id.val, NULL);
    if (proc && (proc != PROCESS_TERMINATED))
    {
        if (pArgs->action_id.val == 'splr')
        {
            if (in_flag)
                proc->ipc_allowSpoolerPrintToFile = *in_flag != 0;
            if (out_flag)
                *out_flag = proc->ipc_allowSpoolerPrintToFile;
        }
        else if (pArgs->action_id.val == 'inet')
        {
            if (in_flag)
                proc->AllowInternetAccess = *in_flag != 0;
            if (out_flag)
                *out_flag = proc->AllowInternetAccess;
        }
        else
            status = STATUS_INVALID_INFO_CLASS;
    }
    else
        status = STATUS_NOT_FOUND;
    //ExReleaseResourceLite(Process_ListLock);
    //KeLowerIrql(irql);

	return status;
}