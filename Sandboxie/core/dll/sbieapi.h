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
// Sandboxie Application Programming Interface
//---------------------------------------------------------------------------


#ifndef _MY_SBIEAPI_H
#define _MY_SBIEAPI_H


#ifdef __cplusplus
extern "C" {
#endif


#ifndef SBIEAPI_EXPORT
#define SBIEAPI_EXPORT  __declspec(dllexport)
#endif


//---------------------------------------------------------------------------
// Sandboxie API Defines
//---------------------------------------------------------------------------


#include "core/drv/api_flags.h"


//---------------------------------------------------------------------------
// Sandboxie API Calls
//---------------------------------------------------------------------------


SBIEAPI_EXPORT
LONG SbieApi_Call(ULONG api_code, LONG arg_num, ...);

SBIEAPI_EXPORT LONG SbieApi_GetVersion(
    WCHAR *version_string);         // WCHAR [16]

/*SBIEAPI_EXPORT
LONG SbieApi_GetWork(
    ULONG SessionId,
    void *Buffer,
    ULONG *Length);*/

SBIEAPI_EXPORT 
ULONG SbieApi_GetMessage(
	ULONG* MessageNum,
	ULONG SessionId,
	ULONG *MessageId,
	ULONG *Pid,
	wchar_t *Buffer,
	ULONG Length);

SBIEAPI_EXPORT LONG SbieApi_Log(
    ULONG msgid, const WCHAR *format, ...);

SBIEAPI_EXPORT LONG SbieApi_LogEx(
    ULONG session_id, ULONG msgid, const WCHAR *format, ...);

SBIEAPI_EXPORT LONG SbieApi_vLogEx(
    ULONG session_id, ULONG msgid, const WCHAR *format, va_list va_args);

SBIEAPI_EXPORT LONG SbieApi_LogMsgEx(
	ULONG session_id, ULONG msgid, const WCHAR* msg_data, USHORT msg_len);

SBIEAPI_EXPORT LONG SbieApi_LogMsgExt(
	ULONG msgid, const WCHAR** strings);

SBIEAPI_EXPORT
LONG SbieApi_GetHomePath(
    WCHAR *NtPath, ULONG NtPathMaxLen,
    WCHAR *DosPath, ULONG DosPathMaxLen);


//---------------------------------------------------------------------------


SBIEAPI_EXPORT
LONG SbieApi_QueryProcess(
    HANDLE ProcessId,
    WCHAR *out_box_name_wchar34,        // WCHAR [34]
    WCHAR *out_image_name_wchar96,      // WCHAR [96]
    WCHAR *out_sid_wchar96,             // WCHAR [96]
    ULONG *out_session_id);             // ULONG

SBIEAPI_EXPORT
LONG SbieApi_QueryProcessEx(
    HANDLE ProcessId,
    ULONG image_name_len_in_wchars,
    WCHAR *out_box_name_wchar34,        // WCHAR [34]
    WCHAR *out_image_name_wcharXXX,     // WCHAR [?]
    WCHAR *out_sid_wchar96,             // WCHAR [96]
    ULONG *out_session_id);             // ULONG

SBIEAPI_EXPORT
LONG SbieApi_QueryProcessEx2(
    HANDLE ProcessId,
    ULONG image_name_len_in_wchars,
    WCHAR *out_box_name_wchar34,        // WCHAR [34]
    WCHAR *out_image_name_wcharXXX,     // WCHAR [?]
    WCHAR *out_sid_wchar96,             // WCHAR [96]
    ULONG *out_session_id,              // ULONG
    ULONG64 *out_create_time);          // ULONG64

SBIEAPI_EXPORT
ULONG64 SbieApi_QueryProcessInfo(
    HANDLE ProcessId,
    ULONG info_type);

SBIEAPI_EXPORT
ULONG64 SbieApi_QueryProcessInfoEx(
    HANDLE ProcessId,
    ULONG info_type,
    ULONG64 ext_data);

SBIEAPI_EXPORT
LONG SbieApi_QueryBoxPath(
    const WCHAR *box_name,              // WCHAR [34]
    WCHAR *out_file_path,
    WCHAR *out_key_path,
    WCHAR *out_ipc_path,
    ULONG *inout_file_path_len,
    ULONG *inout_key_path_len,
    ULONG *inout_ipc_path_len);

SBIEAPI_EXPORT
LONG SbieApi_QueryProcessPath(
    HANDLE ProcessId,
    WCHAR *out_file_path,
    WCHAR *out_key_path,
    WCHAR *out_ipc_path,
    ULONG *inout_file_path_len,
    ULONG *inout_key_path_len,
    ULONG *inout_ipc_path_len);

SBIEAPI_EXPORT
LONG SbieApi_QueryPathList(
    ULONG path_code,
    ULONG *path_len,
    WCHAR *path_str,
    HANDLE process_id);

SBIEAPI_EXPORT
LONG SbieApi_EnumProcessEx(
    const WCHAR* box_name,          // WCHAR [34]
    BOOLEAN all_sessions,
    ULONG which_session,            // -1 for current session
    ULONG* boxed_pids,             // ULONG [512]
    ULONG* boxed_count);

#define SbieApi_EnumProcess(box_name,boxed_pids) \
    SbieApi_EnumProcessEx(box_name,FALSE,-1,boxed_pids, NULL)


//---------------------------------------------------------------------------


SBIEAPI_EXPORT
LONG SbieApi_SessionLeader(
    HANDLE TokenHandle,
    HANDLE *ProcessId);

SBIEAPI_EXPORT
LONG SbieApi_DisableForceProcess(
    ULONG *NewState,
    ULONG *OldState);


//---------------------------------------------------------------------------


SBIEAPI_EXPORT
LONG SbieApi_MonitorControl(
    ULONG *NewState,
    ULONG *OldState);

SBIEAPI_EXPORT
    LONG SbieApi_MonitorPut(
    ULONG Type,
    const WCHAR *Name);

SBIEAPI_EXPORT
LONG SbieApi_MonitorPut2(
    ULONG Type,
    const WCHAR *Name,
    BOOLEAN bCheckObjectExists);

//SBIEAPI_EXPORT
//LONG SbieApi_MonitorGet(
//    ULONG *Type,
//    WCHAR *Name);                   // WCHAR [256]

SBIEAPI_EXPORT
LONG SbieApi_MonitorGetEx(
	ULONG *SeqNum,
	ULONG *Type,
    ULONG *Pid,
    ULONG *Tid,
	WCHAR *Name);                   // WCHAR [256]


//---------------------------------------------------------------------------


SBIEAPI_EXPORT
LONG SbieApi_HookTramp(void *Source, void *Trampoline);


//---------------------------------------------------------------------------


//SBIEAPI_EXPORT
LONG SbieApi_RenameFile(
    HANDLE FileHandle,
    const WCHAR *TargetDir, const WCHAR *TargetName,
    BOOLEAN ReplaceIfExists);

SBIEAPI_EXPORT
LONG SbieApi_GetFileName(
    HANDLE FileHandle,
    ULONG NameLen,
    WCHAR *NameBuf);

//SBIEAPI_EXPORT
LONG SbieApi_OpenFile(
    HANDLE *FileHandle,
    const WCHAR *Path);

SBIEAPI_EXPORT
LONG SbieApi_CheckInternetAccess(
    HANDLE ProcessId,
    const WCHAR *DeviceName32,
    BOOLEAN IssueMessage);

//SBIEAPI_EXPORT
LONG SbieApi_GetBlockedDll(
    WCHAR *DllNameBuf,
    ULONG DllNameLen);


//---------------------------------------------------------------------------


//SBIEAPI_EXPORT
LONG SbieApi_OpenKey(
    HANDLE *KeyHandle,
    const WCHAR *Path);

//SBIEAPI_EXPORT
LONG SbieApi_SetLowLabelKey(
    const WCHAR *Path);


//---------------------------------------------------------------------------


//SBIEAPI_EXPORT
LONG SbieApi_CreateDirOrLink(
    const WCHAR *ObjectName, const WCHAR *TargetName);


//---------------------------------------------------------------------------


//SBIEAPI_EXPORT
LONG SbieApi_DuplicateObject(
    HANDLE *TargetHandle,
    HANDLE OtherProcessHandle,
    HANDLE SourceHandle,
    ACCESS_MASK DesiredAccess,
    ULONG Options);

SBIEAPI_EXPORT
LONG SbieApi_OpenProcess(
    HANDLE *ProcessHandle,
    HANDLE ProcessId);

//SBIEAPI_EXPORT
LONG SbieApi_OpenDeviceMap(
    HANDLE *DirectoryHandle);

//SBIEAPI_EXPORT
LONG SbieApi_QuerySymbolicLink(
    WCHAR *NameBuf,
    ULONG NameLen);

//---------------------------------------------------------------------------


SBIEAPI_EXPORT
LONG SbieApi_ReloadConf(ULONG session_id);


SBIEAPI_EXPORT
LONG SbieApi_QueryConf(
    const WCHAR *section_name,      // WCHAR [66]
    const WCHAR *setting_name,      // WCHAR [66]
    ULONG setting_index,
    WCHAR *out_buffer,
    ULONG buffer_len);

#define SbieApi_QueryConfAsIs(bx, st, idx, buf, buflen) \
    SbieApi_QueryConf((bx), (st), ((idx) | CONF_GET_NO_EXPAND), buf, buflen)

SBIEAPI_EXPORT
BOOLEAN SbieApi_QueryConfBool(
    const WCHAR *section_name,      // WCHAR [66]
    const WCHAR *setting_name,      // WCHAR [66]
    BOOLEAN def);

SBIEAPI_EXPORT
LONG SbieApi_EnumBoxes(
    LONG index,                     // initialize to -1
    WCHAR *box_name);               // WCHAR [34]

SBIEAPI_EXPORT
LONG SbieApi_EnumBoxesEx(
    LONG index,                     // initialize to -1
    WCHAR *box_name,                // WCHAR [34]
    BOOLEAN ignore_hidden);

SBIEAPI_EXPORT
LONG SbieApi_IsBoxEnabled(
    const WCHAR *box_name);         // WCHAR [34]

SBIEAPI_EXPORT
LONG SbieApi_SetUserName(
    const WCHAR *SidString,
    const WCHAR *UserName);


//---------------------------------------------------------------------------


SBIEAPI_EXPORT
LONG SbieApi_GetUnmountHive(
    WCHAR *path);                   // WCHAR [256]


//---------------------------------------------------------------------------

SBIEAPI_EXPORT
LONG SbieApi_ProcessExemptionControl(
	HANDLE process_id,
	ULONG action_id,
	ULONG *NewState,
	ULONG *OldState);

//---------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif /* _MY_SBIEAPI_H */
