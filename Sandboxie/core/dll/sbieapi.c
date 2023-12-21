/*
 * Copyright 2004-2020 Sandboxie Holdings, LLC 
 * Copyright 2020-2022 David Xanatos, xanasoft.com
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
// Sandboxie Application Programming Interface
//---------------------------------------------------------------------------

#include "dll.h"

#include <windows.h>
#include <stdio.h>

#include "core/drv/api_defs.h"
#include "core/svc/msgids.h"
#include "common/my_version.h"
//#include "core/low/lowdata.h"
//
//extern SBIELOW_DATA* SbieApi_data;
//#define SBIELOW_CALL(x) ((P_##x)&SbieApi_data->x##_code)

#pragma optimize("",off)


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------


static NTSTATUS SbieApi_Ioctl(ULONG64 *parms);


//---------------------------------------------------------------------------
// Variables
//---------------------------------------------------------------------------


HANDLE SbieApi_DeviceHandle = INVALID_HANDLE_VALUE;

// SboxDll does not link in the CRT. Instead, it piggybacks onto the CRT routines that are in ntdll.dll.
// However, the ntdll.lib from the 7600 DDK does not export everything we need. So we must use runtime dynamic linking.

int __CRTDECL Sbie_snwprintf(wchar_t *_Buffer, size_t Count, const wchar_t * const _Format, ...)
{
	int _Result;
	va_list _ArgList;

	extern int(*P_vsnwprintf)(wchar_t *_Buffer, size_t Count, const wchar_t * const, va_list Args);

	va_start(_ArgList, _Format);
	_Result = P_vsnwprintf(_Buffer, Count, _Format, _ArgList);
	va_end(_ArgList);

	if (_Result == -1 && _Buffer != NULL && Count != 0)
		_Buffer[Count - 1] = 0; // ensure the resulting string is always null terminated

	return _Result;
}

int __CRTDECL Sbie_snprintf(char *_Buffer, size_t Count, const char * const _Format, ...)
{
	int _Result;
	va_list _ArgList;

	extern int(*P_vsnprintf)(char *_Buffer, size_t Count, const char * const, va_list Args);

	va_start(_ArgList, _Format);
	_Result = P_vsnprintf(_Buffer, Count, _Format, _ArgList);
	va_end(_ArgList);

	if (_Result == -1 && _Buffer != NULL && Count != 0)
		_Buffer[Count - 1] = 0; // ensure the resulting string is always null terminated

	return _Result;
}

//---------------------------------------------------------------------------
// SbieApi_Ioctl
//---------------------------------------------------------------------------


_FX NTSTATUS SbieApi_Ioctl(ULONG64 *parms)
{
    NTSTATUS status;
    OBJECT_ATTRIBUTES objattrs;
    UNICODE_STRING uni;
    IO_STATUS_BLOCK MyIoStatusBlock;

    if (parms == NULL) { // close request as used by kmdutil
        if(SbieApi_DeviceHandle != INVALID_HANDLE_VALUE)
            NtClose(SbieApi_DeviceHandle);
        SbieApi_DeviceHandle = INVALID_HANDLE_VALUE;
    }

    if (Dll_SbieTrace && parms[0] != API_MONITOR_PUT2) {
        WCHAR dbg[1024];
        extern const wchar_t* Trace_SbieDrvFunc2Str(ULONG func);
        Sbie_snwprintf(dbg, 1024, L"SbieApi_Ioctl: %s %s", Dll_ImageName, Trace_SbieDrvFunc2Str((ULONG)parms[0]));
        SbieApi_MonitorPutMsg(MONITOR_OTHER | MONITOR_TRACE, dbg);
    }

    if (SbieApi_DeviceHandle == INVALID_HANDLE_VALUE) {

        RtlInitUnicodeString(&uni, API_DEVICE_NAME);
        InitializeObjectAttributes(
            &objattrs, &uni, OBJ_CASE_INSENSITIVE, NULL, NULL);

        status = NtOpenFile(
            &SbieApi_DeviceHandle, FILE_GENERIC_READ, &objattrs,
            &MyIoStatusBlock,
            FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
            0);

        if (status == STATUS_OBJECT_NAME_NOT_FOUND ||
            status == STATUS_NO_SUCH_DEVICE)
            status = STATUS_SERVER_DISABLED;

    } else
        status = STATUS_SUCCESS;

    if (status != STATUS_SUCCESS) {

        SbieApi_DeviceHandle = INVALID_HANDLE_VALUE;

    } else {

        //
        // note that all requests are synchronous which means
        // NtDeviceIoControlFile will wait until SbieDrv has finished
        // processing a request before sending the next request
        //

        extern P_NtDeviceIoControlFile __sys_NtDeviceIoControlFile;
        if (__sys_NtDeviceIoControlFile) {
        
            //
            // once NtDeviceIoControlFile is hooked, bypass it
            //
        
            status = __sys_NtDeviceIoControlFile(
                SbieApi_DeviceHandle, NULL, NULL, NULL, &MyIoStatusBlock,
                API_SBIEDRV_CTLCODE, parms, sizeof(ULONG64) * 8, NULL, 0);
        
        } else {
        
            status = NtDeviceIoControlFile(
                SbieApi_DeviceHandle, NULL, NULL, NULL, &MyIoStatusBlock,
                API_SBIEDRV_CTLCODE, parms, sizeof(ULONG64) * 8, NULL, 0);
        }

        // that would be even better but would only work in the native case
        //status = SBIELOW_CALL(NtDeviceIoControlFile)(
        //        SbieApi_DeviceHandle, NULL, NULL, NULL, &MyIoStatusBlock,
        //        API_SBIEDRV_CTLCODE, parms, sizeof(ULONG64) * 8, NULL, 0);
    }

    return status;
}


//---------------------------------------------------------------------------
// SbieApi_CallFunc
//---------------------------------------------------------------------------


_FX LONG SbieApi_Call(ULONG api_code, LONG arg_num, ...) 
{
    va_list valist;
    NTSTATUS status;
    __declspec(align(8)) ULONG64 parms[API_NUM_ARGS];

    memzero(parms, sizeof(parms));
    parms[0] = api_code;

    if (arg_num >= (API_NUM_ARGS - 1))
        return STATUS_INVALID_PARAMETER;

    va_start(valist, arg_num);
    for (LONG i = 1; i <= arg_num; i++)
        parms[i] = (ULONG64)va_arg(valist, ULONG_PTR);
    va_end(valist);

    status = SbieApi_Ioctl(parms);

    return status;
}


//---------------------------------------------------------------------------
// SbieApi_GetVersion
//---------------------------------------------------------------------------


_FX LONG SbieApi_GetVersion(
    WCHAR *out_version)     // WCHAR [16]
{
    return SbieApi_GetVersionEx(out_version, NULL);
}


//---------------------------------------------------------------------------
// SbieApi_GetVersionEx
//---------------------------------------------------------------------------


_FX LONG SbieApi_GetVersionEx(
    WCHAR* version_string,  // WCHAR [16]
    ULONG* abi_version)
{
    NTSTATUS status;
    __declspec(align(8)) ULONG64 parms[API_NUM_ARGS];
    API_GET_VERSION_ARGS *args = (API_GET_VERSION_ARGS *)parms;

    memzero(parms, sizeof(parms));
    args->func_code = API_GET_VERSION;
    args->string.val = version_string;
    args->abi_ver.val = abi_version;

    status = SbieApi_Ioctl(parms);

    if (! NT_SUCCESS(status)){
        if (version_string) wcscpy(version_string, L"unknown");
        if (abi_version) *abi_version = 0;
    }

    return status;
}


//---------------------------------------------------------------------------
// KmdUtil
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
// SbieApi_GetWork
//---------------------------------------------------------------------------

/*
_FX LONG SbieApi_GetWork(
    ULONG SessionId,
    void *Buffer,
    ULONG *Length)
{
    NTSTATUS status;
    __declspec(align(8)) ULONG64 parms[API_NUM_ARGS];
    API_GET_WORK_ARGS *args = (API_GET_WORK_ARGS *)parms;

    memzero(parms, sizeof(parms));
    args->func_code = API_GET_WORK;
    args->session_id.val = SessionId;
    args->buffer.val64 = (ULONG64)(ULONG_PTR)Buffer;
    args->buffer_len.val64 = *Length;
    args->result_len_ptr.val64 = (ULONG64)(ULONG_PTR)Length;

    status = SbieApi_Ioctl(parms);

    return status;
}
*/

//---------------------------------------------------------------------------
// SbieApi_GetMessage
//---------------------------------------------------------------------------


_FX ULONG SbieApi_GetMessage(
	ULONG* MessageNum,
	ULONG SessionId,
	ULONG *MessageId,
	ULONG *Pid,
	wchar_t *Buffer,
	ULONG Length)
{
	NTSTATUS status;
	__declspec(align(8)) UNICODE_STRING64 msgtext;
	__declspec(align(8)) ULONG64 parms[API_NUM_ARGS];
	API_GET_MESSAGE_ARGS *args = (API_GET_MESSAGE_ARGS *)parms;

	msgtext.MaximumLength = (USHORT)Length;
	msgtext.Buffer = (ULONG_PTR)Buffer;
	msgtext.Length = 0;

	memzero(parms, sizeof(parms));
	args->func_code = API_GET_MESSAGE;
	args->msg_num.val = MessageNum;
	args->session_id.val = SessionId;
	args->msgid.val = MessageId;
	args->msgtext.val = &msgtext;
	args->process_id.val = Pid;

	status = SbieApi_Ioctl(parms);

	return status;
}


//---------------------------------------------------------------------------
// SbieApi_Log
//---------------------------------------------------------------------------


_FX LONG SbieApi_Log(
    ULONG msgid,
    const WCHAR *format, ...)
{
    ULONG status;
    va_list va_args;
    va_start(va_args, format);
    status = SbieApi_vLogEx(-1, msgid, format, va_args);
    va_end(va_args);
    return status;
}


//---------------------------------------------------------------------------
// SbieApi_LogEx
//---------------------------------------------------------------------------


_FX LONG SbieApi_LogEx(
    ULONG session_id,
    ULONG msgid,
    const WCHAR *format, ...)
{
    ULONG status;
    va_list va_args;
    va_start(va_args, format);
    status = SbieApi_vLogEx(session_id, msgid, format, va_args);
    va_end(va_args);
    return status;
}


//---------------------------------------------------------------------------
// SbieApi_vLogEx
//---------------------------------------------------------------------------


_FX LONG SbieApi_vLogEx(
    ULONG session_id,
    ULONG msgid,
    const WCHAR *format,
    va_list va_args)
{
	NTSTATUS status;
    UCHAR *tmp1, *tmp2;

    // make sure to allocate at least twice API_LOG_MESSAGE_MAX_LEN
    tmp1 = Dll_AllocTemp((API_LOG_MESSAGE_MAX_LEN + 4) * 2);
    tmp2 = (UCHAR *)tmp1 + API_LOG_MESSAGE_MAX_LEN * 2 - 510;
    if (format) {

        extern int(*P_vsnprintf)(char *_Buffer, size_t Count, const char * const, va_list Args);

        Sbie_snprintf(tmp1, 510, "%S", format);
        P_vsnprintf(tmp2, 510, tmp1, va_args);

    } else
        *tmp2 = '\0';

    Sbie_snwprintf((WCHAR *)tmp1, 510, L"%S", tmp2);

	status = SbieApi_LogMsgEx(session_id, msgid, (WCHAR*)tmp1, (USHORT)wcslen((WCHAR *)tmp1) * sizeof(WCHAR));

    Dll_Free(tmp1);

    return status;
}


//---------------------------------------------------------------------------
// SbieApi_LogMsgEx
//---------------------------------------------------------------------------


_FX LONG SbieApi_LogMsgEx(
	ULONG session_id,
	ULONG msgid,
	const WCHAR* msg_data,
	USHORT msg_len)
{
	NTSTATUS status;
	__declspec(align(8)) UNICODE_STRING64 msgtext;
	__declspec(align(8)) ULONG64 parms[API_NUM_ARGS];
	API_LOG_MESSAGE_ARGS *args = (API_LOG_MESSAGE_ARGS *)parms;

	//
	// the msg_data can contain multiple strings separated by L'\0' characters
	//

	msgtext.Buffer = (ULONG_PTR)msg_data;
	msgtext.Length = msg_len;
	msgtext.MaximumLength = msgtext.Length + sizeof(WCHAR);

	memzero(parms, sizeof(parms));
	args->func_code = API_LOG_MESSAGE;
	args->session_id.val = session_id;
	args->msgid.val = msgid;
	args->msgtext.val = &msgtext;
	//args->process_id.val = 
	status = SbieApi_Ioctl(parms);

	return status;
}


//---------------------------------------------------------------------------
// SbieApi_LogMsgExt
//---------------------------------------------------------------------------


_FX LONG SbieApi_LogMsgExt(
	ULONG session_id,
	ULONG msgid,
	const WCHAR** strings)
{
	NTSTATUS status;
	ULONG size = 0;

	for (const WCHAR** string = strings; *string != NULL; string++)
		size += (wcslen(*string) + 1) * sizeof(WCHAR); // include null char

	if (size < API_LOG_MESSAGE_MAX_LEN) {

		WCHAR *buff, *temp;
		temp = buff = Dll_AllocTemp(size);

		for (const WCHAR** string = strings; *string != NULL; string++) {
			ULONG len = wcslen(*string) + 1;
			wmemcpy(temp, *string, len);
			temp += len;
		}

		status = SbieApi_LogMsgEx(session_id, msgid, buff, (USHORT)size);

		Dll_Free(buff);

	}
	else
		status = STATUS_INSUFFICIENT_RESOURCES;

	return status;
}


//---------------------------------------------------------------------------
// SbieApi_GetHomePath
//---------------------------------------------------------------------------


_FX LONG SbieApi_GetHomePath(
    WCHAR *NtPath, ULONG NtPathMaxLen, WCHAR *DosPath, ULONG DosPathMaxLen)
{
    NTSTATUS status;
    __declspec(align(8)) UNICODE_STRING64 nt_path_uni;
    __declspec(align(8)) UNICODE_STRING64 dos_path_uni;
    __declspec(align(8)) ULONG64 parms[API_NUM_ARGS];
    API_GET_HOME_PATH_ARGS *args = (API_GET_HOME_PATH_ARGS *)parms;

    nt_path_uni.Length          = 0;
    nt_path_uni.MaximumLength   = (USHORT)(NtPathMaxLen * sizeof(WCHAR));
    nt_path_uni.Buffer          = (ULONG64)(ULONG_PTR)NtPath;

    dos_path_uni.Length         = 0;
    dos_path_uni.MaximumLength  = (USHORT)(DosPathMaxLen * sizeof(WCHAR));
    dos_path_uni.Buffer         = (ULONG64)(ULONG_PTR)DosPath;

    memzero(parms, sizeof(parms));
    args->func_code             = API_GET_HOME_PATH;
    if (NtPath)
        args->nt_path.val64     = (ULONG64)(ULONG_PTR)&nt_path_uni;
    if (DosPath)
        args->dos_path.val64    = (ULONG64)(ULONG_PTR)&dos_path_uni;
    status = SbieApi_Ioctl(parms);

    if (! NT_SUCCESS(status)) {
        if (NtPath)
            NtPath[0] = L'\0';
        if (DosPath)
            DosPath[0] = L'\0';
    }

    return status;
}


//---------------------------------------------------------------------------
// SbieApi_QueryProcess
//---------------------------------------------------------------------------


_FX LONG SbieApi_QueryProcess(
    HANDLE ProcessId,
    WCHAR *out_box_name_wchar34,
    WCHAR *out_image_name_wchar96,
    WCHAR *out_sid_wchar96,
    ULONG *out_session_id)
{
    return SbieApi_QueryProcessEx2(
        ProcessId, 96, out_box_name_wchar34, out_image_name_wchar96,
        out_sid_wchar96, out_session_id, NULL);
}


//---------------------------------------------------------------------------
// SbieApi_QueryProcessEx
//---------------------------------------------------------------------------


_FX LONG SbieApi_QueryProcessEx(
    HANDLE ProcessId,
    ULONG image_name_len_in_wchars,
    WCHAR *out_box_name_wchar34,
    WCHAR *out_image_name_wcharXXX,
    WCHAR *out_sid_wchar96,
    ULONG *out_session_id)
{
    return SbieApi_QueryProcessEx2(
        ProcessId, image_name_len_in_wchars, out_box_name_wchar34,
        out_image_name_wcharXXX, out_sid_wchar96, out_session_id, NULL);
}


//---------------------------------------------------------------------------
// SbieApi_QueryProcessEx2
//---------------------------------------------------------------------------


_FX LONG SbieApi_QueryProcessEx2(
    HANDLE ProcessId,
    ULONG image_name_len_in_wchars,
    WCHAR *out_box_name_wchar34,
    WCHAR *out_image_name_wcharXXX,
    WCHAR *out_sid_wchar96,
    ULONG *out_session_id,
    ULONG64 *out_create_time)
{
    NTSTATUS status;
    __declspec(align(8)) UNICODE_STRING64 BoxName;
    __declspec(align(8)) UNICODE_STRING64 ImageName;
    __declspec(align(8)) UNICODE_STRING64 SidString;
    __declspec(align(8)) ULONG64 parms[API_NUM_ARGS];
    API_QUERY_PROCESS_ARGS *args = (API_QUERY_PROCESS_ARGS *)parms;

    memzero(parms, sizeof(parms));
    args->func_code = API_QUERY_PROCESS;

    args->process_id.val64 = (ULONG64)(ULONG_PTR)ProcessId;

    if (out_box_name_wchar34) {
        BoxName.Length = 0;
        BoxName.MaximumLength = (USHORT)(sizeof(WCHAR) * BOXNAME_COUNT);
        BoxName.Buffer = (ULONG64)(ULONG_PTR)out_box_name_wchar34;
        args->box_name.val64 = (ULONG64)(ULONG_PTR)&BoxName;
    }

    if (out_image_name_wcharXXX) {
        ImageName.Length = 0;
        ImageName.MaximumLength =
            (USHORT)(sizeof(WCHAR) * image_name_len_in_wchars);
        ImageName.Buffer = (ULONG64)(ULONG_PTR)out_image_name_wcharXXX;
        args->image_name.val64 = (ULONG64)(ULONG_PTR)&ImageName;
    }

    if (out_sid_wchar96) {
        SidString.Length = 0;
        SidString.MaximumLength = (USHORT)(sizeof(WCHAR) * 96);
        SidString.Buffer = (ULONG64)(ULONG_PTR)out_sid_wchar96;
        args->sid_string.val64 = (ULONG64)(ULONG_PTR)&SidString;
    }

    if (out_session_id)
        args->session_id.val64 = (ULONG64)(ULONG_PTR)out_session_id;

    if (out_create_time)
        args->create_time.val64 = (ULONG64)(ULONG_PTR)out_create_time;

    status = SbieApi_Ioctl(parms);

    if (! NT_SUCCESS(status)) {

        ULONG_PTR x = (ULONG_PTR)out_session_id;
        if (x == 0 || x > 4) {

            //
            // reset parameters on error except when out_session_id
            // is a special internal flag in the range 1 to 4
            //

            if (out_box_name_wchar34)
                *out_box_name_wchar34 = L'\0';
            if (out_image_name_wcharXXX)
                *out_image_name_wcharXXX = L'\0';
            if (out_sid_wchar96)
                *out_sid_wchar96 = L'\0';
            if (out_session_id)
                *out_session_id = 0;
        }
    }

    return status;
}


//---------------------------------------------------------------------------
// SbieApi_QueryProcessInfo
//---------------------------------------------------------------------------


_FX ULONG64 SbieApi_QueryProcessInfo(
    HANDLE ProcessId,
    ULONG info_type)
{
    return SbieApi_QueryProcessInfoEx(ProcessId, info_type, 0);
}

//---------------------------------------------------------------------------
// SbieApi_QueryProcessInfoEx
//---------------------------------------------------------------------------


_FX ULONG64 SbieApi_QueryProcessInfoEx(
    HANDLE ProcessId,
    ULONG info_type,
    ULONG64 ext_data)
{
    NTSTATUS status;
    __declspec(align(8)) ULONG64 ResultValue;
    __declspec(align(8)) ULONG64 parms[API_NUM_ARGS];
    API_QUERY_PROCESS_INFO_ARGS *args = (API_QUERY_PROCESS_INFO_ARGS *)parms;

    memzero(parms, sizeof(parms));
    args->func_code             = API_QUERY_PROCESS_INFO;

    args->process_id.val64      = (ULONG64)(ULONG_PTR)ProcessId;
    args->info_type.val64       = (ULONG64)(ULONG_PTR)info_type;
    args->info_data.val64       = (ULONG64)(ULONG_PTR)&ResultValue;
    args->ext_data.val64        = (ULONG64)(ULONG_PTR)ext_data;

    status = SbieApi_Ioctl(parms);

    if (! NT_SUCCESS(status))
        ResultValue = 0;

    return ResultValue;
}


//---------------------------------------------------------------------------
// SbieApi_QueryProcessInfoStr
//---------------------------------------------------------------------------


_FX LONG SbieApi_QueryProcessInfoStr(
    HANDLE ProcessId,
    ULONG info_type,
    WCHAR *out_str,
    ULONG *inout_str_len)
{
    NTSTATUS status;
    __declspec(align(8)) UNICODE_STRING64 UniStr;
    __declspec(align(8)) ULONG64 parms[API_NUM_ARGS];
    API_QUERY_PROCESS_INFO_ARGS *args = (API_QUERY_PROCESS_INFO_ARGS *)parms;

    memzero(parms, sizeof(parms));
    args->func_code             = API_QUERY_PROCESS_INFO;

    args->process_id.val64      = (ULONG64)(ULONG_PTR)ProcessId;
    args->info_type.val64       = (ULONG64)(ULONG_PTR)info_type;
    args->info_data.val64       = (ULONG64)(ULONG_PTR)inout_str_len;

    if (out_str) {
        UniStr.Length = 0;
        UniStr.MaximumLength = (USHORT)*inout_str_len;
        UniStr.Buffer = (ULONG64)(ULONG_PTR)out_str;
        args->ext_data.val64 = (ULONG64)(ULONG_PTR)&UniStr;
    }

    status = SbieApi_Ioctl(parms);

    if (!NT_SUCCESS(status)) {
        if (out_str)
            *out_str = L'\0';
    }

    return status;
}


//---------------------------------------------------------------------------
// SbieApi_QueryBoxPath
//---------------------------------------------------------------------------


_FX LONG SbieApi_QueryBoxPath(
    const WCHAR *box_name,              // WCHAR [BOXNAME_COUNT]
    WCHAR *out_file_path,
    WCHAR *out_key_path,
    WCHAR *out_ipc_path,
    ULONG *inout_file_path_len,
    ULONG *inout_key_path_len,
    ULONG *inout_ipc_path_len)
{
    NTSTATUS status;
    __declspec(align(8)) UNICODE_STRING64 FilePath;
    __declspec(align(8)) UNICODE_STRING64 KeyPath;
    __declspec(align(8)) UNICODE_STRING64 IpcPath;
    __declspec(align(8)) ULONG64 parms[API_NUM_ARGS];
    API_QUERY_BOX_PATH_ARGS *args = (API_QUERY_BOX_PATH_ARGS *)parms;

    memzero(parms, sizeof(parms));
    args->func_code = API_QUERY_BOX_PATH;

    args->box_name.val64 = (ULONG64)(ULONG_PTR)box_name;

    if (out_file_path) {
        FilePath.Length = 0;
        FilePath.MaximumLength = (USHORT)*inout_file_path_len;
        FilePath.Buffer = (ULONG64)(ULONG_PTR)out_file_path;
        args->file_path.val64 = (ULONG64)(ULONG_PTR)&FilePath;
    }

    if (out_key_path) {
        KeyPath.Length = 0;
        KeyPath.MaximumLength = (USHORT)*inout_key_path_len;
        KeyPath.Buffer = (ULONG64)(ULONG_PTR)out_key_path;
        args->key_path.val64 = (ULONG64)(ULONG_PTR)&KeyPath;
    }

    if (out_ipc_path) {
        IpcPath.Length = 0;
        IpcPath.MaximumLength = (USHORT)*inout_ipc_path_len;
        IpcPath.Buffer = (ULONG64)(ULONG_PTR)out_ipc_path;
        args->ipc_path.val64 = (ULONG64)(ULONG_PTR)&IpcPath;
    }

    args->file_path_len.val = inout_file_path_len;
    args->key_path_len.val  = inout_key_path_len;
    args->ipc_path_len.val  = inout_ipc_path_len;

    status = SbieApi_Ioctl(parms);

    if (! NT_SUCCESS(status)) {
        if (out_file_path)
            *out_file_path = L'\0';
        if (out_key_path)
            *out_key_path = L'\0';
        if (out_ipc_path)
            *out_ipc_path = L'\0';
    }

    return status;
}


//---------------------------------------------------------------------------
// SbieApi_QueryProcessPath
//---------------------------------------------------------------------------


_FX LONG SbieApi_QueryProcessPath(
    HANDLE ProcessId,
    WCHAR *out_file_path,
    WCHAR *out_key_path,
    WCHAR *out_ipc_path,
    ULONG *inout_file_path_len,
    ULONG *inout_key_path_len,
    ULONG *inout_ipc_path_len)
{
    NTSTATUS status;
    __declspec(align(8)) UNICODE_STRING64 FilePath;
    __declspec(align(8)) UNICODE_STRING64 KeyPath;
    __declspec(align(8)) UNICODE_STRING64 IpcPath;
    __declspec(align(8)) ULONG64 parms[API_NUM_ARGS];
    API_QUERY_PROCESS_PATH_ARGS *args = (API_QUERY_PROCESS_PATH_ARGS *)parms;

    memzero(parms, sizeof(parms));
    args->func_code = API_QUERY_PROCESS_PATH;

    args->process_id.val64 = (ULONG64)(ULONG_PTR)ProcessId;

    if (out_file_path) {
        FilePath.Length = 0;
        FilePath.MaximumLength = (USHORT)*inout_file_path_len;
        FilePath.Buffer = (ULONG64)(ULONG_PTR)out_file_path;
        args->file_path.val64 = (ULONG64)(ULONG_PTR)&FilePath;
    }

    if (out_key_path) {
        KeyPath.Length = 0;
        KeyPath.MaximumLength = (USHORT)*inout_key_path_len;
        KeyPath.Buffer = (ULONG64)(ULONG_PTR)out_key_path;
        args->key_path.val64 = (ULONG64)(ULONG_PTR)&KeyPath;
    }

    if (out_ipc_path) {
        IpcPath.Length = 0;
        IpcPath.MaximumLength = (USHORT)*inout_ipc_path_len;
        IpcPath.Buffer = (ULONG64)(ULONG_PTR)out_ipc_path;
        args->ipc_path.val64 = (ULONG64)(ULONG_PTR)&IpcPath;
    }

    args->file_path_len.val = inout_file_path_len;
    args->key_path_len.val  = inout_key_path_len;
    args->ipc_path_len.val  = inout_ipc_path_len;

    status = SbieApi_Ioctl(parms);

    if (! NT_SUCCESS(status)) {
        if (out_file_path)
            *out_file_path = L'\0';
        if (out_key_path)
            *out_key_path = L'\0';
        if (out_ipc_path)
            *out_ipc_path = L'\0';
    }

    return status;
}


//---------------------------------------------------------------------------
// SbieApi_QueryPathList
//---------------------------------------------------------------------------


_FX LONG SbieApi_QueryPathList(
    ULONG path_code,
    ULONG *path_len,
    WCHAR *path_str,
    HANDLE process_id,
    BOOLEAN prepend_level)
{
    NTSTATUS status;
    __declspec(align(8)) ULONG64 parms[API_NUM_ARGS];
    API_QUERY_PATH_LIST_ARGS *args = (API_QUERY_PATH_LIST_ARGS *)parms;

    memzero(parms, sizeof(parms));
    args->func_code = API_QUERY_PATH_LIST;
    args->path_code.val = path_code;
    args->path_len.val64 = (ULONG64)(ULONG_PTR)path_len;
    args->path_str.val64 = (ULONG64)(ULONG_PTR)path_str;
    args->process_id.val64 = (ULONG64)(ULONG_PTR)process_id;
    args->prepend_level.val = prepend_level;
    status = SbieApi_Ioctl(parms);

    return status;
}


//---------------------------------------------------------------------------
// SbieApi_EnumProcessEx
//---------------------------------------------------------------------------


_FX LONG SbieApi_EnumProcessEx(
    const WCHAR *box_name,          // WCHAR [BOXNAME_COUNT]
    BOOLEAN all_sessions,
    ULONG which_session,            // -1 for current session
    ULONG *boxed_pids,              // ULONG [512]
    ULONG *boxed_count)
{
    NTSTATUS status;
    __declspec(align(8)) ULONG64 parms[API_NUM_ARGS];

    memset(parms, 0, sizeof(parms));
    parms[0] = API_ENUM_PROCESSES;
    parms[1] = (ULONG64)(ULONG_PTR)boxed_pids;
    parms[2] = (ULONG64)(ULONG_PTR)box_name;
    parms[3] = (ULONG64)(ULONG_PTR)all_sessions;
    parms[4] = (ULONG64)(LONG_PTR)which_session;
    parms[5] = (ULONG64)(LONG_PTR)boxed_count;
    status = SbieApi_Ioctl(parms);

    if (! NT_SUCCESS(status))
        boxed_pids[0] = 0;

    return status;
}


//---------------------------------------------------------------------------
// SbieApi_DisableForceProcess
//---------------------------------------------------------------------------


_FX LONG SbieApi_DisableForceProcess(
    ULONG *NewState,
    ULONG *OldState)
{
    NTSTATUS status;
    __declspec(align(8)) ULONG64 parms[API_NUM_ARGS];
    API_DISABLE_FORCE_PROCESS_ARGS *args =
        (API_DISABLE_FORCE_PROCESS_ARGS *)parms;

    memset(parms, 0, sizeof(parms));
    args->func_code               = API_DISABLE_FORCE_PROCESS;
    args->set_flag.val64          = (ULONG64)(ULONG_PTR)NewState;
    args->get_flag.val64          = (ULONG64)(ULONG_PTR)OldState;
    status = SbieApi_Ioctl(parms);

    if (! NT_SUCCESS(status)) {
        if (OldState)
            *OldState = FALSE;
    }

    return status;
}


//---------------------------------------------------------------------------
// SbieApi_HookTramp
//---------------------------------------------------------------------------


/*_FX LONG SbieApi_HookTramp(void *Source, void *Trampoline)
{
    NTSTATUS status;
    __declspec(align(8)) ULONG64 parms[API_NUM_ARGS];

    memset(parms, 0, sizeof(parms));
    parms[0] = API_HOOK_TRAMP;
    parms[1] = (ULONG64)(ULONG_PTR)Source;
    parms[2] = (ULONG64)(ULONG_PTR)Trampoline;
    status = SbieApi_Ioctl(parms);

    return status;
}*/


//---------------------------------------------------------------------------
// SbieApi_RenameFile
//---------------------------------------------------------------------------


_FX LONG SbieApi_RenameFile(
    HANDLE FileHandle,
    const WCHAR *TargetDir, const WCHAR *TargetName,
    BOOLEAN ReplaceIfExists)
{
    NTSTATUS status;
    __declspec(align(8)) UNICODE_STRING64 dir_uni;
    __declspec(align(8)) UNICODE_STRING64 name_uni;
    __declspec(align(8)) ULONG64 parms[API_NUM_ARGS];
    API_RENAME_FILE_ARGS *args = (API_RENAME_FILE_ARGS *)parms;

    dir_uni.Length = wcslen(TargetDir) * sizeof(WCHAR);
    dir_uni.MaximumLength = dir_uni.Length + sizeof(WCHAR);
    dir_uni.Buffer = (ULONG_PTR)TargetDir;

    name_uni.Length = wcslen(TargetName) * sizeof(WCHAR);
    name_uni.MaximumLength = name_uni.Length + sizeof(WCHAR);
    name_uni.Buffer = (ULONG_PTR)TargetName;

    memzero(parms, sizeof(parms));
    args->func_code               = API_RENAME_FILE;
    args->file_handle.val64       = (ULONG64)(ULONG_PTR)FileHandle;
    args->target_dir.val64        = (ULONG64)(ULONG_PTR)&dir_uni;
    args->target_name.val64       = (ULONG64)(ULONG_PTR)&name_uni;
    args->replace_if_exists.val64 = (ULONG64)(ULONG_PTR)ReplaceIfExists;

    status = SbieApi_Ioctl(parms);

    return status;
}


//---------------------------------------------------------------------------
// SbieApi_GetFileName
//---------------------------------------------------------------------------


_FX LONG SbieApi_GetFileName(
    HANDLE FileHandle,
    WCHAR *NameBuf,
    ULONG *NameLen,
    ULONG *ObjType)
{
    NTSTATUS status;
    __declspec(align(8)) ULONG64 parms[API_NUM_ARGS];
    API_GET_FILE_NAME_ARGS *args = (API_GET_FILE_NAME_ARGS *)parms;

    memset(parms, 0, sizeof(parms));
    args->func_code               = API_GET_FILE_NAME;
    args->handle.val64            = (ULONG64)(ULONG_PTR)FileHandle;
    args->name_len.val64          = (ULONG64)(ULONG_PTR)NameLen;
    args->name_buf.val64          = (ULONG64)(ULONG_PTR)NameBuf;
    args->type_buf.val64          = (ULONG64)(ULONG_PTR)ObjType;
    status = SbieApi_Ioctl(parms);

    if (! NT_SUCCESS(status)) {
        if (NameBuf)
            *NameBuf = L'\0';
    }

    return status;
}


//---------------------------------------------------------------------------
// SbieApi_OpenFile
//---------------------------------------------------------------------------


_FX LONG SbieApi_OpenFile(
    HANDLE *FileHandle,
    const WCHAR *Path)
{
    NTSTATUS status;
    __declspec(align(8)) ULONG64 ResultHandle;
    __declspec(align(8)) ULONG64 parms[API_NUM_ARGS];
    API_OPEN_FILE_ARGS *args = (API_OPEN_FILE_ARGS *)parms;

    memzero(parms, sizeof(parms));
    args->func_code               = API_OPEN_FILE;
    args->path_len.val64          = (ULONG64)(wcslen(Path) * sizeof(WCHAR));
    args->path_str.val64          = (ULONG64)(ULONG_PTR)Path;
    args->file_handle.val64       = (ULONG64)(ULONG_PTR)&ResultHandle;

    status = SbieApi_Ioctl(parms);
    if (! NT_SUCCESS(status))
        ResultHandle = 0;
    *FileHandle = (HANDLE *)ResultHandle;

    return status;
}


//---------------------------------------------------------------------------
// SbieApi_CheckInternetAccess
//---------------------------------------------------------------------------


_FX LONG SbieApi_CheckInternetAccess(
    HANDLE ProcessId,
    const WCHAR *DeviceName32,
    BOOLEAN IssueMessage)
{
    NTSTATUS status;
    __declspec(align(8)) ULONG64 parms[API_NUM_ARGS];
    API_CHECK_INTERNET_ACCESS_ARGS *args =
        (API_CHECK_INTERNET_ACCESS_ARGS *)parms;
    WCHAR MyDeviceName[34];
    ULONG len;

    if (DeviceName32) {
        len = wcslen(DeviceName32);
        if (len > 32)
            len = 32;
        memzero(MyDeviceName, sizeof(MyDeviceName));
        wmemcpy(MyDeviceName, DeviceName32, len);

        const WCHAR* ptr = DeviceName32;

        static const WCHAR *File_RawIp = L"rawip6";
        static const WCHAR *File_Http  = L"http\\";
        static const WCHAR *File_Tcp   = L"tcp6";
        static const WCHAR *File_Udp   = L"udp6";
        static const WCHAR *File_Ip    = L"ip6";
        static const WCHAR *File_Afd   = L"afd";
        static const WCHAR *File_Nsi   = L"nsi";

        //
        // check if this is an internet device
        //

        BOOLEAN chk = FALSE;
        if (len == 12) {
            
            if (_wcsnicmp(ptr, File_Afd, 3) == 0) // Afd\Endpoint
                chk = TRUE;

        } else if (len == 6) {
            
            if (_wcsnicmp(ptr, File_RawIp, 6) == 0)
                chk = TRUE;

        } else if (len == 5) {

            if (_wcsnicmp(ptr, File_RawIp, 5) == 0)
                chk = TRUE;

        } else if (len == 4) {

            if (_wcsnicmp(ptr, File_Http, 4) == 0)
                chk = TRUE;
            if (_wcsnicmp(ptr, File_Tcp, 4) == 0)
                chk = TRUE;
            else if (_wcsnicmp(ptr, File_Udp, 4) == 0)
                chk = TRUE;

        } else if (len == 3) {

            if (_wcsnicmp(ptr, File_Tcp, 3) == 0)
                chk = TRUE;
            else if (_wcsnicmp(ptr, File_Udp, 3) == 0)
                chk = TRUE;
            else if (_wcsnicmp(ptr, File_Ip, 3) == 0)
                chk = TRUE;
            else if (_wcsnicmp(ptr, File_Afd, 3) == 0)
                chk = TRUE;
            else if (_wcsnicmp(ptr, File_Nsi, 3) == 0)
                chk = TRUE;

        } else if (len == 2) {

            if (_wcsnicmp(ptr, File_Ip, 2) == 0)
                chk = TRUE;
        }

        if (! chk) // quick bail out, don't bother the driver with irrelevant devices
            return STATUS_OBJECT_NAME_INVALID;
    }

    memzero(parms, sizeof(parms));
    args->func_code               = API_CHECK_INTERNET_ACCESS;
    args->process_id.val64        = (ULONG64)(ULONG_PTR)ProcessId;
    args->device_name.val64       = (ULONG64)(ULONG_PTR)(DeviceName32 ? MyDeviceName : NULL);
    args->issue_message.val64     = (ULONG64)(ULONG_PTR)IssueMessage;

    status = SbieApi_Ioctl(parms);

    return status;
}


//---------------------------------------------------------------------------
// SbieApi_GetBlockedDll
//---------------------------------------------------------------------------


_FX LONG SbieApi_GetBlockedDll(
    WCHAR *DllNameBuf,
    ULONG DllNameLen)
{
    NTSTATUS status;
    __declspec(align(8)) ULONG64 parms[API_NUM_ARGS];
    API_GET_BLOCKED_DLL_ARGS *args = (API_GET_BLOCKED_DLL_ARGS *)parms;

    memzero(parms, sizeof(parms));
    args->func_code             = API_GET_BLOCKED_DLL;
    args->dll_name_buf.val64    = (ULONG64)(ULONG_PTR)DllNameBuf;
    args->dll_name_len.val64    = (ULONG64)(ULONG_PTR)DllNameLen;

    status = SbieApi_Ioctl(parms);

    if (! NT_SUCCESS(status)) {
        if (DllNameBuf)
            *DllNameBuf = L'\0';
    }

    return status;
}


//---------------------------------------------------------------------------
// SbieApi_CreateDirOrLink
//---------------------------------------------------------------------------


_FX LONG SbieApi_CreateDirOrLink(
    const WCHAR *ObjectName,
    const WCHAR *TargetName)
{
    NTSTATUS status;
    __declspec(align(8)) UNICODE_STRING64 objname;
    __declspec(align(8)) UNICODE_STRING64 target;
    __declspec(align(8)) ULONG64 parms[API_NUM_ARGS];
    API_CREATE_DIR_OR_LINK_ARGS *args =
        (API_CREATE_DIR_OR_LINK_ARGS *)parms;

    objname.Length        = wcslen(ObjectName) * sizeof(WCHAR);
    objname.MaximumLength = objname.Length + sizeof(WCHAR);
    objname.Buffer        = (ULONG64)(ULONG_PTR)ObjectName;

    if (TargetName) {
        target.Length        = wcslen(TargetName) * sizeof(WCHAR);
        target.MaximumLength = target.Length + sizeof(WCHAR);
        target.Buffer        = (ULONG64)(ULONG_PTR)TargetName;
    }

    memzero(parms, sizeof(parms));
    args->func_code         = API_CREATE_DIR_OR_LINK;
    args->objname.val64     = (ULONG64)(ULONG_PTR)&objname;
    if (TargetName)
        args->target.val64  = (ULONG64)(ULONG_PTR)&target;

    status = SbieApi_Ioctl(parms);

    return status;
}


//---------------------------------------------------------------------------
// SbieApi_DuplicateObject
//---------------------------------------------------------------------------


_FX LONG SbieApi_DuplicateObject(
    HANDLE *TargetHandle,
    HANDLE OtherProcessHandle,
    HANDLE SourceHandle,
    ACCESS_MASK DesiredAccess,
    ULONG Options)
{
    NTSTATUS status;
    __declspec(align(8)) ULONG64 ResultHandle;
    __declspec(align(8)) ULONG64 parms[API_NUM_ARGS];
    API_DUPLICATE_OBJECT_ARGS *args = (API_DUPLICATE_OBJECT_ARGS *)parms;

    memzero(parms, sizeof(parms));
    args->func_code             = API_DUPLICATE_OBJECT;

    args->target_handle.val64   = (ULONG64)(ULONG_PTR)&ResultHandle;
    args->process_handle.val64  = (ULONG64)(ULONG_PTR)OtherProcessHandle;
    args->source_handle.val64   = (ULONG64)(ULONG_PTR)SourceHandle;
    args->desired_access.val    = DesiredAccess;
    args->options.val           = Options;

    status = SbieApi_Ioctl(parms);

    if (! NT_SUCCESS(status))
        ResultHandle = 0;
    if (TargetHandle)
        *TargetHandle = (HANDLE *)ResultHandle;

    return status;
}


//---------------------------------------------------------------------------
// SbieApi_OpenProcess
//---------------------------------------------------------------------------


_FX LONG SbieApi_OpenProcess(
    HANDLE *ProcessHandle,
    HANDLE ProcessId)
{
    NTSTATUS status;
    __declspec(align(8)) ULONG64 ResultHandle;
    __declspec(align(8)) ULONG64 parms[API_NUM_ARGS];
    API_OPEN_PROCESS_ARGS *args = (API_OPEN_PROCESS_ARGS *)parms;

    memzero(parms, sizeof(parms));
    args->func_code             = API_OPEN_PROCESS;

    args->process_id.val64      = (ULONG64)(ULONG_PTR)ProcessId;
    args->process_handle.val64  = (ULONG64)(ULONG_PTR)&ResultHandle;

    status = SbieApi_Ioctl(parms);

    if (! NT_SUCCESS(status))
        ResultHandle = 0;
    if (ProcessHandle)
        *ProcessHandle = (HANDLE *)ResultHandle;

    return status;
}


//---------------------------------------------------------------------------
// SbieApi_OpenDeviceMap
//---------------------------------------------------------------------------


_FX LONG SbieApi_OpenDeviceMap(
    HANDLE *DirectoryHandle)
{
    NTSTATUS status;
    __declspec(align(8))ULONG64 ResultHandle;
    __declspec(align(8)) ULONG64 parms[API_NUM_ARGS];
    API_OPEN_DEVICE_MAP_ARGS *args = (API_OPEN_DEVICE_MAP_ARGS *)parms;

    memset(parms, 0, sizeof(parms));
    args->func_code               = API_OPEN_DEVICE_MAP;
    args->handle.val64            = (ULONG64)(ULONG_PTR)&ResultHandle;
    status = SbieApi_Ioctl(parms);

    if (! NT_SUCCESS(status))
        ResultHandle = 0;
    if (DirectoryHandle)
        *DirectoryHandle = (HANDLE *)ResultHandle;

    return status;
}


//---------------------------------------------------------------------------
// SbieApi_QuerySymbolicLink
//---------------------------------------------------------------------------


_FX LONG SbieApi_QuerySymbolicLink(
    WCHAR *NameBuf,
    ULONG NameLen)
{
    NTSTATUS status;
    __declspec(align(8)) ULONG64 parms[API_NUM_ARGS];
    API_QUERY_SYMBOLIC_LINK_ARGS *args =
        (API_QUERY_SYMBOLIC_LINK_ARGS *)parms;

    memzero(parms, sizeof(parms));
    args->func_code         = API_QUERY_SYMBOLIC_LINK;
    args->name_buf.val64    = (ULONG64)(ULONG_PTR)NameBuf;
    args->name_len.val64    = (ULONG64)(ULONG_PTR)NameLen;

    status = SbieApi_Ioctl(parms);

    if (! NT_SUCCESS(status)) {
        if (NameBuf)
            *NameBuf = L'\0';
    }

    return status;
}


//---------------------------------------------------------------------------
// SbieApi_ReloadConf
//---------------------------------------------------------------------------


_FX LONG SbieApi_ReloadConf(ULONG session_id, ULONG flags)
{
    NTSTATUS status;
    __declspec(align(8)) ULONG64 parms[API_NUM_ARGS];

    memset(parms, 0, sizeof(parms));
    parms[0] = API_RELOAD_CONF;
    parms[1] = session_id;
    parms[2] = flags;
    status = SbieApi_Ioctl(parms);

    return status;
}


//---------------------------------------------------------------------------
// SbieApi_QueryConf
//---------------------------------------------------------------------------


_FX LONG SbieApi_QueryConf(
    const WCHAR *section_name,      // WCHAR [66]
    const WCHAR *setting_name,      // WCHAR [66]
    ULONG setting_index,
    WCHAR *out_buffer,
    ULONG buffer_len)
{
    NTSTATUS status;
    __declspec(align(8)) UNICODE_STRING64 Output;
    __declspec(align(8)) ULONG64 parms[API_NUM_ARGS];
    WCHAR x_section[66];
    WCHAR x_setting[66];

    memzero(x_section, sizeof(x_section));
    memzero(x_setting, sizeof(x_setting));
    if (section_name)
        wcsncpy(x_section, section_name, 64);
    if (setting_name)
        wcsncpy(x_setting, setting_name, 64);

    Output.Length        = 0;
    Output.MaximumLength = (USHORT)buffer_len;
    Output.Buffer        = (ULONG64)(ULONG_PTR)out_buffer;

    memset(parms, 0, sizeof(parms));
    parms[0] = API_QUERY_CONF;
    parms[1] = (ULONG64)(ULONG_PTR)x_section;
    parms[2] = (ULONG64)(ULONG_PTR)x_setting;
    parms[3] = (ULONG64)(ULONG_PTR)&setting_index;
    parms[4] = (ULONG64)(ULONG_PTR)&Output;
    status = SbieApi_Ioctl(parms);

    if (! NT_SUCCESS(status)) {
        if (buffer_len > sizeof(WCHAR))
            out_buffer[0] = L'\0';
    }

    return status;
}


//---------------------------------------------------------------------------
// SbieApi_QueryConf
//---------------------------------------------------------------------------


/*_FX LONG SbieApi_QueryConf(
    const WCHAR* section_name,      // WCHAR [66]
    const WCHAR* setting_name,      // WCHAR [66]
    ULONG setting_index,
    WCHAR* out_buffer,
    ULONG buffer_len)
{
    //if(SbieApi_DeviceHandle != INVALID_HANDLE_VALUE)
        return SbieApi_QueryConfDrv(section_name, setting_name, setting_index, out_buffer, buffer_len);
    // else try service    
    //return STATUS_CONNECTION_INVALID;
}*/


//---------------------------------------------------------------------------
// SbieApi_QueryConfBool
//---------------------------------------------------------------------------


_FX BOOLEAN SbieApi_QueryConfBool(
    const WCHAR *section_name,      // WCHAR [66]
    const WCHAR *setting_name,      // WCHAR [66]
    BOOLEAN def)
{
    WCHAR value[16];
    *value = L'\0';
    SbieApi_QueryConfAsIs(
                    section_name, setting_name, 0, value, sizeof(value));
    if (*value == L'y' || *value == L'Y')
        return TRUE;
    if (*value == L'n' || *value == L'N')
        return FALSE;
    return def;
}


//---------------------------------------------------------------------------
// SbieApi_QueryConfBool
//---------------------------------------------------------------------------


_FX ULONG SbieApi_QueryConfNumber(
    const WCHAR *section_name,      // WCHAR [66]
    const WCHAR *setting_name,      // WCHAR [66]
    ULONG def)
{
    WCHAR value[32];
    *value = L'\0';
    if (!NT_SUCCESS(SbieApi_QueryConfAsIs(
                    section_name, setting_name, 0, value, sizeof(value)))
        || *value == L'\0') // empty string
        return def;
    ULONG num = _wtoi(value);
    if (num == 0) {
        WCHAR* ptr = value;
        //if(*ptr == L'-')
        //    ptr++;
        while (*ptr == L'0')
            ptr++;
        if(*ptr == L'\0')
            return def;
    }
    return num;
}


//---------------------------------------------------------------------------
// SbieApi_EnumBoxes
//---------------------------------------------------------------------------


_FX LONG SbieApi_EnumBoxes(
    LONG index,                     // initialize to -1
    WCHAR *box_name)                // WCHAR [BOXNAME_COUNT]
{
    return SbieApi_EnumBoxesEx(index, box_name, FALSE);
}


//---------------------------------------------------------------------------
// SbieApi_EnumBoxes
//---------------------------------------------------------------------------


_FX LONG SbieApi_EnumBoxesEx(
    LONG index,                     // initialize to -1
    WCHAR *box_name,                // WCHAR [BOXNAME_COUNT]
    BOOLEAN return_all_sections)
{
    LONG rc;
    while (1) {
        ++index;
        rc = SbieApi_QueryConf(NULL, NULL, index | CONF_GET_NO_TEMPLS | CONF_GET_NO_EXPAND,
                               box_name, sizeof(WCHAR) * BOXNAME_COUNT);
        if (rc == STATUS_BUFFER_TOO_SMALL)
            continue;
        if (! box_name[0])
            return -1;
        if (return_all_sections ||
                (SbieApi_IsBoxEnabled(box_name) == STATUS_SUCCESS))
            return index;
    }
}


//---------------------------------------------------------------------------
// SbieApi_SetUserName
//---------------------------------------------------------------------------


_FX LONG SbieApi_SetUserName(
    const WCHAR *SidString,
    const WCHAR *UserName)
{
    NTSTATUS status;
    __declspec(align(8)) UNICODE_STRING64 uni_sid;
    __declspec(align(8)) UNICODE_STRING64 uni_name;
    __declspec(align(8)) ULONG64 parms[API_NUM_ARGS];
    API_SET_USER_NAME_ARGS *args = (API_SET_USER_NAME_ARGS *)parms;

    uni_sid.Length         = wcslen(SidString) * sizeof(WCHAR);
    uni_sid.MaximumLength  = uni_sid.Length + sizeof(WCHAR);
    uni_sid.Buffer         = (ULONG64)(ULONG_PTR)SidString;

    uni_name.Length        = wcslen(UserName) * sizeof(WCHAR);
    uni_name.MaximumLength = uni_name.Length + sizeof(WCHAR);
    uni_name.Buffer        = (ULONG64)(ULONG_PTR)UserName;

    memzero(parms, sizeof(parms));
    args->func_code         = API_SET_USER_NAME;
    args->sidstring.val64   = (ULONG64)(ULONG_PTR)&uni_sid;
    args->username.val64    = (ULONG64)(ULONG_PTR)&uni_name;

    status = SbieApi_Ioctl(parms);

    return status;
}


//---------------------------------------------------------------------------
// SbieApi_MonitorControl
//---------------------------------------------------------------------------


_FX LONG SbieApi_MonitorControl(
    ULONG *NewState,
    ULONG *OldState)
{
    NTSTATUS status;
    __declspec(align(8)) ULONG64 parms[API_NUM_ARGS];
    API_MONITOR_CONTROL_ARGS *args = (API_MONITOR_CONTROL_ARGS *)parms;

    memset(parms, 0, sizeof(parms));
    args->func_code               = API_MONITOR_CONTROL;
    args->set_flag.val64          = (ULONG64)(ULONG_PTR)NewState;
    args->get_flag.val64          = (ULONG64)(ULONG_PTR)OldState;
    status = SbieApi_Ioctl(parms);

    if (! NT_SUCCESS(status)) {
        if (OldState)
            *OldState = FALSE;
    }

    return status;
}


//---------------------------------------------------------------------------
// SbieApi_MonitorPut
//---------------------------------------------------------------------------


_FX LONG SbieApi_MonitorPut(
    ULONG Type,
    const WCHAR *Name)
{
    //NTSTATUS status;
    //__declspec(align(8)) ULONG64 parms[API_NUM_ARGS];
    //API_MONITOR_GET_PUT_ARGS *args = (API_MONITOR_GET_PUT_ARGS *)parms;

    //memset(parms, 0, sizeof(parms));
    //args->func_code               = API_MONITOR_PUT;
    //args->log_type.val           = Type;
    //args->log_len.val64          = wcslen(Name) * sizeof(WCHAR);
    //args->log_ptr.val64          = (ULONG64)(ULONG_PTR)Name;
    //status = SbieApi_Ioctl(parms);

    //return status;

    return SbieApi_MonitorPut2(Type, Name, TRUE);
}


//---------------------------------------------------------------------------
// SbieApi_MonitorPut2
//---------------------------------------------------------------------------


_FX LONG SbieApi_MonitorPut2(
    ULONG Type,
    const WCHAR *Name,
    BOOLEAN bCheckObjectExists)
{
    return SbieApi_MonitorPut2Ex(Type, wcslen(Name), Name, bCheckObjectExists, FALSE);
}


//---------------------------------------------------------------------------
// SbieApi_MonitorPutMsg
//---------------------------------------------------------------------------


_FX LONG SbieApi_MonitorPutMsg(
    ULONG Type,
    const WCHAR* Message)
{
    return SbieApi_MonitorPut2Ex(Type, wcslen(Message), Message, FALSE, TRUE);
}


//---------------------------------------------------------------------------
// SbieApi_MonitorPut2Ex
//---------------------------------------------------------------------------


_FX LONG SbieApi_MonitorPut2Ex(
    ULONG Type,
    ULONG NameLen,
    const WCHAR *Name,
    BOOLEAN bCheckObjectExists,
    BOOLEAN bIsMessage)
{
    NTSTATUS status;
    __declspec(align(8)) ULONG64 parms[API_NUM_ARGS];
    API_MONITOR_PUT2_ARGS *args = (API_MONITOR_PUT2_ARGS *)parms;

    memset(parms, 0, sizeof(parms));
    args->func_code                 = API_MONITOR_PUT2;
    args->log_type.val              = Type;
    args->log_len.val64             = NameLen * sizeof(WCHAR);
    args->log_ptr.val64             = (ULONG64)(ULONG_PTR)Name;
    args->check_object_exists.val64 = bCheckObjectExists;
    args->is_message.val64          = bIsMessage;
    status = SbieApi_Ioctl(parms);

    return status;
}

//---------------------------------------------------------------------------
// SbieApi_MonitorGet
//---------------------------------------------------------------------------


//_FX LONG SbieApi_MonitorGet(
//    ULONG *Type,
//    WCHAR *Name)                    // WCHAR [256]
//{
//    NTSTATUS status;
//    __declspec(align(8)) ULONG64 parms[API_NUM_ARGS];
//    API_MONITOR_GET_PUT_ARGS *args = (API_MONITOR_GET_PUT_ARGS *)parms;
//
//    memset(parms, 0, sizeof(parms));
//    args->func_code               = API_MONITOR_GET;
//    args->log_type.val64         = (ULONG64)(ULONG_PTR)Type;
//    args->log_len.val64          = 256 * sizeof(WCHAR);
//    args->log_ptr.val64          = (ULONG64)(ULONG_PTR)Name;
//    status = SbieApi_Ioctl(parms);
//
//    if (! NT_SUCCESS(status)) {
//        if (Type)
//            *Type = 0;
//        if (Name)
//            *Name = L'\0';
//    }
//
//    return status;
//}


//---------------------------------------------------------------------------
// SbieApi_MonitorGetEx
//---------------------------------------------------------------------------


_FX LONG SbieApi_MonitorGetEx(
	ULONG *Type,
	ULONG *Pid,
    ULONG *Tid,
	WCHAR *Name)                    // WCHAR [256]
{
	NTSTATUS status;
    __declspec(align(8)) UNICODE_STRING64 log_buffer = { 0, (USHORT)(256 * sizeof(WCHAR)), (ULONG64)Name };
	__declspec(align(8)) ULONG64 parms[API_NUM_ARGS];
	API_MONITOR_GET_EX_ARGS *args = (API_MONITOR_GET_EX_ARGS *)parms;

    memset(parms, 0, sizeof(parms));
	args->func_code = API_MONITOR_GET_EX;
	//args->log_seq.val = SeqNum;
	args->log_type.val = Type;
	args->log_pid.val = Pid;
    args->log_tid.val = Tid;
    args->log_data.val = &log_buffer;
	status = SbieApi_Ioctl(parms);

	if (!NT_SUCCESS(status)) {
		if (Type)
			*Type = 0;
		if (Name)
			*Name = L'\0';
	}

	return status;
}


//---------------------------------------------------------------------------
// SbieApi_GetUnmountHive
//---------------------------------------------------------------------------


_FX LONG SbieApi_GetUnmountHive(
    WCHAR *path)                    // WCHAR [256]
{
    NTSTATUS status;
    __declspec(align(8)) ULONG64 parms[API_NUM_ARGS];
    API_GET_UNMOUNT_HIVE_ARGS *args = (API_GET_UNMOUNT_HIVE_ARGS *)parms;

    memset(parms, 0, sizeof(parms));
    args->func_code               = API_GET_UNMOUNT_HIVE;
    args->path.val64              = (ULONG64)(ULONG_PTR)path;
    status = SbieApi_Ioctl(parms);

    if (! NT_SUCCESS(status)) {
        if (path)
            *path = L'\0';
    }

    return status;
}


//---------------------------------------------------------------------------
// SbieApi_SessionLeader
//---------------------------------------------------------------------------


_FX LONG SbieApi_SessionLeader(HANDLE TokenHandle, HANDLE *ProcessId)
{
    NTSTATUS status;
    __declspec(align(8)) ULONG64 ResultValue;
    __declspec(align(8)) ULONG64 parms[API_NUM_ARGS];
    API_SESSION_LEADER_ARGS *args = (API_SESSION_LEADER_ARGS *)parms;

    memset(parms, 0, sizeof(parms));
    args->func_code               = API_SESSION_LEADER;
    if (ProcessId) {
        args->token_handle.val64  = (ULONG64)(ULONG_PTR)TokenHandle;
        args->process_id.val64    = (ULONG64)(ULONG_PTR)&ResultValue;
    } else {
        args->token_handle.val64  = 0;
        args->process_id.val64    = 0;
    }
    status = SbieApi_Ioctl(parms);

    if (! NT_SUCCESS(status))
        ResultValue = 0;
    if (ProcessId)
        *ProcessId = (HANDLE)ResultValue;

    return status;
}


//---------------------------------------------------------------------------
// SbieApi_IsBoxEnabled
//---------------------------------------------------------------------------


_FX LONG SbieApi_IsBoxEnabled(
    const WCHAR *box_name)          // WCHAR [BOXNAME_COUNT]
{
    NTSTATUS status;
    __declspec(align(8)) ULONG64 parms[API_NUM_ARGS];
    API_IS_BOX_ENABLED_ARGS *args = (API_IS_BOX_ENABLED_ARGS *)parms;

    memzero(parms, sizeof(parms));
    args->func_code = API_IS_BOX_ENABLED;

    args->box_name.val64   = (ULONG64)(ULONG_PTR)box_name;

    status = SbieApi_Ioctl(parms);

    return status;
}


//---------------------------------------------------------------------------
// SbieApi_OpenKey
//---------------------------------------------------------------------------


_FX LONG SbieApi_OpenKey(
    HANDLE *KeyHandle,
    const WCHAR *Path)
{
    NTSTATUS status;
    __declspec(align(8)) ULONG64 ResultHandle;
    __declspec(align(8)) ULONG64 parms[API_NUM_ARGS];
    API_OPEN_KEY_ARGS *args = (API_OPEN_KEY_ARGS *)parms;

    memzero(parms, sizeof(parms));
    args->func_code               = API_OPEN_KEY;
    args->path_len.val64          = (ULONG64)(wcslen(Path) * sizeof(WCHAR));
    args->path_str.val64          = (ULONG64)(ULONG_PTR)Path;
    args->key_handle.val64        = (ULONG64)(ULONG_PTR)&ResultHandle;

    status = SbieApi_Ioctl(parms);
    if (! NT_SUCCESS(status))
        ResultHandle = 0;
    *KeyHandle = (HANDLE *)ResultHandle;

    return status;
}


//---------------------------------------------------------------------------
// SbieApi_SetLowLabelKey
//---------------------------------------------------------------------------


_FX LONG SbieApi_SetLowLabelKey(
    const WCHAR *Path)
{
    NTSTATUS status;
    __declspec(align(8)) ULONG64 parms[API_NUM_ARGS];
    API_SET_LOW_LABEL_ARGS *args = (API_SET_LOW_LABEL_ARGS *)parms;

    memzero(parms, sizeof(parms));
    args->func_code               = API_SET_LOW_LABEL_KEY;
    args->path_len.val64          = (ULONG64)(wcslen(Path) * sizeof(WCHAR));
    args->path_str.val64          = (ULONG64)(ULONG_PTR)Path;

    status = SbieApi_Ioctl(parms);

    return status;
}


//---------------------------------------------------------------------------
// SbieApi_MonitorControl
//---------------------------------------------------------------------------


_FX LONG SbieApi_ProcessExemptionControl(
	HANDLE process_id,
	ULONG action_id,
	ULONG *NewState,
	ULONG *OldState)
{
	NTSTATUS status;
	__declspec(align(8)) ULONG64 parms[API_NUM_ARGS];
	API_PROCESS_EXEMPTION_CONTROL_ARGS *args = (API_PROCESS_EXEMPTION_CONTROL_ARGS *)parms;

	memzero(parms, sizeof(parms));
	args->func_code = API_PROCESS_EXEMPTION_CONTROL;
	args->process_id.val = process_id;
	args->action_id.val = action_id;
	args->set_flag.val64 = (ULONG64)(ULONG_PTR)NewState;
	args->get_flag.val64 = (ULONG64)(ULONG_PTR)OldState;
	status = SbieApi_Ioctl(parms);

	if (!NT_SUCCESS(status)) {
		if (OldState)
			*OldState = FALSE;
	}

	return status;
}


//---------------------------------------------------------------------------
// SbieDll_GetSysFunction
//---------------------------------------------------------------------------

extern HANDLE                       SbieApi_DeviceHandle;
extern P_NtCreateFile               __sys_NtCreateFile;
extern P_NtQueryDirectoryFile       __sys_NtQueryDirectoryFile;
extern P_NtOpenKey                  __sys_NtOpenKey;
extern P_NtEnumerateValueKey        __sys_NtEnumerateValueKey;
extern P_NtDeviceIoControlFile      __sys_NtDeviceIoControlFile;


void* SbieDll_GetSysFunction(const WCHAR* name)
{
    if (name == NULL)                                       return SbieApi_DeviceHandle;
    if (_wcsicmp(name, L"NtCreateFile") == 0)               return __sys_NtCreateFile;
    if (_wcsicmp(name, L"NtQueryDirectoryFile") == 0)       return __sys_NtQueryDirectoryFile;
    if (_wcsicmp(name, L"NtOpenKey") == 0)                  return __sys_NtOpenKey;
    if (_wcsicmp(name, L"NtEnumerateValueKey") == 0)        return __sys_NtEnumerateValueKey;
    if (_wcsicmp(name, L"NtDeviceIoControlFile") == 0)      return __sys_NtDeviceIoControlFile;
    return NULL;
}


//---------------------------------------------------------------------------
// SbieDll_RunStartExe
//---------------------------------------------------------------------------


BOOL SbieDll_RunStartExe(const WCHAR* cmd, const wchar_t* boxname)
{
    WCHAR cmdline[MAX_PATH] = L"";

    if (boxname) {
        wcscat(cmdline, L"/box:");
        wcscat(cmdline, boxname);
        wcscat(cmdline, L" ");
    }
    wcscat(cmdline, cmd);

    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    memzero(&si, sizeof(si));
    si.cb = sizeof(STARTUPINFO);
    //if (inherit) si.lpReserved = (LPTSTR)1;
    BOOL ret = FALSE;

    if ( SbieDll_RunFromHome(START_EXE, cmdline, &si, &pi)) {

        ret = TRUE;

        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }

    return ret;
}
