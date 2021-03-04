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
// Message log
//---------------------------------------------------------------------------


#ifndef _MY_LOG_H
#define _MY_LOG_H

#include "msgs/msgs.h"


//---------------------------------------------------------------------------
// Messages
//---------------------------------------------------------------------------


#define MSG_DRIVER_ENTRY_OK                                 MSG_1101
#define MSG_DRIVER_UNLOAD                                   MSG_1102
#define MSG_DRIVER_ENTRY_FAILED                             MSG_1103
//#define MSG_NO_MEMORY_FOR_INIT                              MSG_1104
#define MSG_DRIVER_FIND_HOME_PATH                           MSG_1106
#define MSG_LICENSE_READ_REGISTRY                           MSG_1109
#define MSG_OBJ_HOOK_ANY_PROC                               MSG_1110
#define MSG_DLL_LOAD                                        MSG_1111
#define MSG_DLL_GET_PROC                                    MSG_1112
#define MSG_HOOK_NT_SERVICE                                 MSG_1113
#define MSG_HOOK_ZW_SERVICE                                 MSG_1114
//#define MSG_PROCESS_NO_IMAGE                                MSG_1215
#define MSG_PROCESS_NOTIFY                                  MSG_1116
#define MSG_API_DEVICE                                      MSG_1119
#define MSG_GUI_SERVICE_MISMATCH                            MSG_1120
#define MSG_GUI_SERVICE_HOOK                                MSG_1121
#define MSG_GUI_HOOK_ERROR                                  MSG_1122

#define MSG_HOOK_ANALYZE                                    MSG_1151
#define MSG_HOOK_TRAMP_GET                                  MSG_1152
#define MSG_GUI_INIT_FAILED                                 MSG_1153

//#define MSG_NO_MEMORY                                       MSG_1201
#define MSG_LICENSE_WRITE_REGISTRY                          MSG_1202
#define MSG_INIT_PATHS                                      MSG_1203
#define MSG_BOX_CREATE                                      MSG_1204
//#define MSG_PROCESS_CREATE                                  MSG_1211
#define MSG_FILE_CREATE_BOX_PATH                            MSG_1212
#define MSG_IPC_CREATE_BOX_PATH                             MSG_1213
//#define MSG_PROCESS_INJECT_SBIEDLL                          MSG_1214
//#define MSG_PROCESS_SID                                     MSG_1216
//#define MSG_SECURE_CREATE_RESTRICTED_TOKEN                  MSG_1222
//#define MSG_SECURE_REPLACE_TOKEN                            MSG_1223
#define MSG_MOUNT_FAILED                                    MSG_1241
#define MSG_MONITOR_OVERFLOW                                MSG_1242

//#define MSG_PROCESS_NON_SANDBOXED                           MSG_1301
//#define MSG_TOO_MANY_BOXES                                  MSG_1303
//#define MSG_GUI_FAKE_INPUT                                  MSG_1304
#define MSG_CANNOT_UNLOAD_DRIVER                            MSG_1306
#define MSG_INTERNET_ACCESS_DENIED                          MSG_1307
#define MSG_STARTRUN_ACCESS_DENIED                          MSG_1308
#define MSG_PASSWORD_CHANGE_DENIED                          MSG_1309
//#define MSG_LICENSE_DEACTIVATED                             MSG_1310
//#define MSG_GUI_WALLPAPER                                   MSG_1311
#define MSG_BLOCKED_16_BIT                                  MSG_1312
#define MSG_BLOCKED_DIRECT_DISK_ACCESS                      MSG_1313

#define MSG_CONF_NO_FILE                                    MSG_1401
#define MSG_CONF_READ                                       MSG_1402
#define MSG_CONF_LINE_TOO_LONG                              MSG_1403
#define MSG_CONF_FILE_TOO_LONG                              MSG_1404
#define MSG_CONF_SYNTAX_ERROR                               MSG_1405
#define MSG_CONF_EXPAND                                     MSG_1406
#define MSG_CONF_USER_NAME                                  MSG_1408
#define MSG_CONF_NO_TMPL_FILE                               MSG_1409
#define MSG_CONF_BAD_TMPL_FILE                              MSG_1410
#define MSG_CONF_MISSING_TMPL                               MSG_1411
#define MSG_CONF_SOURCE_TEXT                                MSG_1412


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------


void Log_Msg(
    NTSTATUS error_code,
    const WCHAR *string1,
    const WCHAR *string2);

void Log_Msg_Session(
    NTSTATUS error_code,
    const WCHAR *string1,
    const WCHAR *string2,
    ULONG session_id);

void Log_Msg_Process(
	NTSTATUS error_code,
	const WCHAR *string1,
	const WCHAR *string2,
	ULONG session_id,
	HANDLE process_id);

void Log_Popup_Msg(
    NTSTATUS error_code,
    const WCHAR *string1,
    const WCHAR *string2,
    ULONG session_id,
	HANDLE pid);

void Log_Popup_MsgEx(
	NTSTATUS error_code,
	const WCHAR *string1, ULONG string1_len,
	const WCHAR *string2, ULONG string2_len,
	ULONG session_id,
	HANDLE pid);

void Log_Status_Ex(
    NTSTATUS error_code,
    ULONG error_subcode,
    NTSTATUS nt_status,
    const WCHAR *string2 OPTIONAL);

void Log_Status_Ex_Session(
    NTSTATUS error_code,
    ULONG error_subcode,
    NTSTATUS nt_status,
    const WCHAR *string2 OPTIONAL,
    ULONG session_id);

void Log_Status_Ex_Process(
	NTSTATUS error_code,
	ULONG error_subcode,
	NTSTATUS nt_status,
	const WCHAR *string2 OPTIONAL,
	ULONG session_id,
	HANDLE pocess_id);

#define Log_Msg0(error_code) \
    Log_Msg(error_code,NULL,NULL)

#define Log_Msg1(error_code,str1) \
    Log_Msg(error_code,str1,NULL)

#define Log_Msg2(error_code,str1,str2) \
    Log_Msg(error_code,str1,str2)

#define Log_MsgP0(error_code, proc_id) \
    Log_Msg_Process(error_code,NULL,NULL, -1, proc_id)

#define Log_MsgP1(error_code,str1, proc_id) \
    Log_Msg_Process(error_code,str1,NULL, -1, proc_id)

#define Log_MsgP2(error_code,str1,str2, proc_id) \
    Log_Msg_Process(error_code,str1,str2, -1, proc_id)

#define Log_Status(error_code,error_subcode,ntstatus) \
    Log_Status_Ex(error_code, error_subcode, ntstatus, NULL)

void Log_Debug_Msg(ULONG type, const WCHAR *string1, const WCHAR *string2);


//---------------------------------------------------------------------------


#endif // _MY_LOG_H
