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
// Driver API -- definitions common to kernel driver and user mode callers
//---------------------------------------------------------------------------


#ifndef _MY_API_DEFS_H
#define _MY_API_DEFS_H


//---------------------------------------------------------------------------
// Standard IOCTL defines, if not included yet
//---------------------------------------------------------------------------


#ifndef FILE_DEVICE_UNKNOWN
#define FILE_DEVICE_UNKNOWN             0x00000022
#endif  FILE_DEVICE_UNKNOWN

#ifndef METHOD_NEITHER
#define METHOD_NEITHER                  3
#endif  METHOD_NEITHER

#ifndef FILE_ANY_ACCESS
#define FILE_ANY_ACCESS                 0
#endif  FILE_ANY_ACCESS

#ifndef CTL_CODE
#define CTL_CODE( DeviceType, Function, Method, Access ) (                 \
    ((DeviceType) << 16) | ((Access) << 14) | ((Function) << 2) | (Method) \
)
#endif  CTL_CODE


//---------------------------------------------------------------------------
// Defines
//---------------------------------------------------------------------------


#define API_DEVICE_NAME         L"\\Device\\" SANDBOXIE L"DriverApi"


#define API_SBIEDRV_CTLCODE \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x801, METHOD_NEITHER, FILE_ANY_ACCESS)

#define API_SBIEDRV_FILTERTOKEN_CTLCODE \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x802, METHOD_BUFFERED, FILE_ANY_ACCESS)  // call SeFilterToken in kernel mode

#define API_SBIEDRV_PFILTERTOKEN_CTLCODE \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x803, METHOD_BUFFERED, FILE_ANY_ACCESS)  // call SepFilterToken in kernel mode

// keep API_NUM_ARGS in sync with system service code in core/low/entry.asm
#define API_NUM_ARGS                8


#define API_MAX_SVC_DATA_LEN    \
            (MAX_PORTMSG_LENGTH - (sizeof(PORT_MESSAGE) + sizeof(ULONG)))

#define API_LOG_MESSAGE_MAX_LEN     800

#define API_MAX_PIDS                512

#define MAX_REG_ROOT_LEN            256

//---------------------------------------------------------------------------
// Driver API Codes
//---------------------------------------------------------------------------


enum {
    API_FIRST                   = 0x12340000L,

    API_GET_VERSION,
    API_GET_WORK_DEPRECATED,    			    // deprecated
    API_LOG_MESSAGE,
    API_GET_LICENSE_PRE_V3_48_DEPRECATED,       // deprecated
    API_SET_LICENSE_PRE_V3_48_DEPRECATED,       // deprecated
    API_START_PROCESS_PRE_V3_44_DEPRECATED,     // deprecated
    API_QUERY_PROCESS,
    API_QUERY_BOX_PATH,
    API_QUERY_PROCESS_PATH,
    API_QUERY_PATH_LIST,
    API_ENUM_PROCESSES,
    API_DISABLE_FORCE_PROCESS,
    API_HOOK_TRAMP_DEPRECATED,					// deprecated
    API_UNMOUNT_HIVES_DEPRECATED,               // deprecated
    API_QUERY_CONF,
    API_RELOAD_CONF,
    API_CREATE_DIR_OR_LINK,
    API_DUPLICATE_OBJECT,
    API_GET_INJECT_SAVE_AREA_DEPRECATED,        // deprecated
    API_RENAME_FILE,
    API_SET_USER_NAME,
    API_INIT_GUI,
    API_UNLOAD_DRIVER,
    API_GET_SET_DEVICE_MAP_DEPRECATED,          // deprecated
    API_SESSION_SET_LEADER_DEPRECATED,          // deprecated
    API_GLOBAL_FORCE_PROCESS_DEPRECATED,        // deprecated
    API_MONITOR_CONTROL,
    API_MONITOR_PUT_DEPRECATED,                 // deprecated
    API_MONITOR_GET_DEPRECATED,                 // deprecated
    API_GET_UNMOUNT_HIVE,
    API_GET_FILE_NAME,
    API_REFRESH_FILE_PATH_LIST,
    API_SET_LSA_AUTH_PKG,
    API_OPEN_FILE,
    API_SESSION_CHECK_LEADER_DEPRECATED,        // deprecated
    API_START_PROCESS,
    API_CHECK_INTERNET_ACCESS,
    API_GET_HOME_PATH,
    API_GET_BLOCKED_DLL,
    API_QUERY_LICENSE,
    API_ACTIVATE_LICENSE_DEPRECATED,            // deprecated
    API_OPEN_DEVICE_MAP,
    API_OPEN_PROCESS,
    API_QUERY_PROCESS_INFO,
    API_IS_BOX_ENABLED,
    API_SESSION_LEADER,
    API_QUERY_SYMBOLIC_LINK,
    API_OPEN_KEY,
    API_SET_LOW_LABEL_KEY,
    API_OVERRIDE_PROCESS_TOKEN_DEPRECATED,      // deprecated
    API_SET_SERVICE_PORT,
    API_INJECT_COMPLETE,
    API_QUERY_SYSCALLS,
    API_INVOKE_SYSCALL,
    API_GUI_CLIPBOARD,
    API_ALLOW_SPOOLER_PRINT_TO_FILE_DEPRECATED, // deprecated
    API_RELOAD_CONF2,                           // unused
    API_MONITOR_PUT2,
    API_GET_SPOOLER_PORT_DEPRECATED,            // deprecated
    API_GET_WPAD_PORT_DEPRECATED,               // deprecated
    API_SET_GAME_CONFIG_STORE_PORT_DEPRECATED,  // deprecated
    API_SET_SMART_CARD_PORT_DEPRECATED,         // deprecated
	API_MONITOR_GET_EX,
	API_GET_MESSAGE,
	API_PROCESS_EXEMPTION_CONTROL,
    API_GET_DYNAMIC_PORT_FROM_PID,
    API_OPEN_DYNAMIC_PORT,
    API_QUERY_DRIVER_INFO,
    API_FILTER_TOKEN,
    API_SET_SECURE_PARAM,
    API_GET_SECURE_PARAM,
    API_MONITOR_GET2,
    API_PROTECT_ROOT,
    API_UNPROTECT_ROOT,
    API_KILL_PROCESS,
    API_FORCE_CHILDREN,
    API_MONITOR_PUT_EX,
    API_UPDATE_CONF,
    API_VERIFY,

    API_LAST
};


//---------------------------------------------------------------------------
// Service API Codes
//---------------------------------------------------------------------------


enum {
    SVC_FIRST                   = 0x23450000L,

    SVC_LOOKUP_SID,
    SVC_INJECT_PROCESS,
    SVC_CANCEL_PROCESS,
    SVC_UNMOUNT_HIVE,
    SVC_LOG_MESSAGE,
    SVC_CONFIG_UPDATED,
    SVC_MOUNTED_HIVE,

    SVC_LAST
};


#include "api_flags.h"


//---------------------------------------------------------------------------
// Parameter Structures for calls from user mode to driver
//---------------------------------------------------------------------------


#define API_ARGS_BEGIN(x)           typedef struct _##x { ULONG64 func_code;
#define API_ARGS_FIELD(t,m)         union { ULONG64 val64; t val; } m;
#define API_ARGS_CLOSE(x)           } x;


API_ARGS_BEGIN(API_GET_VERSION_ARGS)
API_ARGS_FIELD(WCHAR *,string)
API_ARGS_FIELD(ULONG *,abi_ver)
API_ARGS_CLOSE(API_GET_VERSION_ARGS)


//API_ARGS_BEGIN(API_GET_WORK_ARGS)
//API_ARGS_FIELD(ULONG,session_id)
//API_ARGS_FIELD(void *,buffer)
//API_ARGS_FIELD(ULONG,buffer_len)
//API_ARGS_FIELD(ULONG *,result_len_ptr)
//API_ARGS_CLOSE(API_GET_WORK_ARGS)


API_ARGS_BEGIN(API_LOG_MESSAGE_ARGS)
API_ARGS_FIELD(ULONG, session_id)
API_ARGS_FIELD(ULONG, msgid)
API_ARGS_FIELD(UNICODE_STRING64 *, msgtext)
API_ARGS_FIELD(ULONG, process_id)
API_ARGS_CLOSE(API_LOG_MESSAGE_ARGS)

API_ARGS_BEGIN(API_GET_MESSAGE_ARGS)
API_ARGS_FIELD(ULONG *, msg_num)
API_ARGS_FIELD(ULONG, session_id)
API_ARGS_FIELD(ULONG *, msgid)
API_ARGS_FIELD(UNICODE_STRING64 *, msgtext)
API_ARGS_FIELD(ULONG *, process_id)
API_ARGS_CLOSE(API_GET_MESSAGE_ARGS)

API_ARGS_BEGIN(API_QUERY_PROCESS_ARGS)
API_ARGS_FIELD(HANDLE,process_id)
API_ARGS_FIELD(UNICODE_STRING64 *,box_name)
API_ARGS_FIELD(UNICODE_STRING64 *,image_name)
API_ARGS_FIELD(UNICODE_STRING64 *,sid_string)
API_ARGS_FIELD(ULONG *,session_id)
API_ARGS_FIELD(ULONG64 *,create_time)
API_ARGS_CLOSE(API_QUERY_PROCESS_ARGS)


API_ARGS_BEGIN(API_QUERY_PROCESS_INFO_ARGS)
API_ARGS_FIELD(HANDLE,process_id)
API_ARGS_FIELD(ULONG,info_type) // in
API_ARGS_FIELD(ULONG64 *,info_data) // out
API_ARGS_FIELD(ULONG64, ext_data) // opt in
API_ARGS_CLOSE(API_QUERY_PROCESS_INFO_ARGS)


API_ARGS_BEGIN(API_QUERY_BOX_PATH_ARGS)
API_ARGS_FIELD(WCHAR *,box_name)
API_ARGS_FIELD(UNICODE_STRING64 *,file_path)
API_ARGS_FIELD(UNICODE_STRING64 *,key_path)
API_ARGS_FIELD(UNICODE_STRING64 *,ipc_path)
API_ARGS_FIELD(ULONG *,file_path_len)
API_ARGS_FIELD(ULONG *,key_path_len)
API_ARGS_FIELD(ULONG *,ipc_path_len)
API_ARGS_CLOSE(API_QUERY_BOX_PATH_ARGS)


API_ARGS_BEGIN(API_QUERY_PROCESS_PATH_ARGS)
API_ARGS_FIELD(HANDLE,process_id)
API_ARGS_FIELD(UNICODE_STRING64 *,file_path)
API_ARGS_FIELD(UNICODE_STRING64 *,key_path)
API_ARGS_FIELD(UNICODE_STRING64 *,ipc_path)
API_ARGS_FIELD(ULONG *,file_path_len)
API_ARGS_FIELD(ULONG *,key_path_len)
API_ARGS_FIELD(ULONG *,ipc_path_len)
API_ARGS_CLOSE(API_QUERY_PROCESS_PATH_ARGS)


API_ARGS_BEGIN(API_QUERY_PATH_LIST_ARGS)
API_ARGS_FIELD(ULONG,path_code)
API_ARGS_FIELD(ULONG *,path_len)
API_ARGS_FIELD(WCHAR *,path_str)
API_ARGS_FIELD(HANDLE,process_id)
API_ARGS_FIELD(BOOLEAN,prepend_level)
API_ARGS_CLOSE(API_QUERY_PATH_LIST_ARGS)


API_ARGS_BEGIN(API_CREATE_DIR_OR_LINK_ARGS)
API_ARGS_FIELD(UNICODE_STRING64 *,objname)
API_ARGS_FIELD(UNICODE_STRING64 *,target)
API_ARGS_CLOSE(API_CREATE_DIR_OR_LINK_ARGS)


API_ARGS_BEGIN(API_DUPLICATE_OBJECT_ARGS)
API_ARGS_FIELD(HANDLE *,target_handle)
API_ARGS_FIELD(HANDLE,process_handle)
API_ARGS_FIELD(HANDLE,source_handle)
API_ARGS_FIELD(ACCESS_MASK,desired_access)
API_ARGS_FIELD(ULONG,options)
API_ARGS_CLOSE(API_DUPLICATE_OBJECT_ARGS)


API_ARGS_BEGIN(API_OPEN_PROCESS_ARGS)
API_ARGS_FIELD(HANDLE,process_id)
API_ARGS_FIELD(HANDLE *,process_handle)
API_ARGS_CLOSE(API_OPEN_PROCESS_ARGS)


//API_ARGS_BEGIN(API_GET_INJECT_SAVE_AREA_ARGS)
//API_ARGS_FIELD(ULONG64 *,save_area_base)
//API_ARGS_FIELD(ULONG *,save_area_len)
//API_ARGS_FIELD(UCHAR *,save_area_ptr)
//API_ARGS_CLOSE(API_GET_INJECT_SAVE_AREA_ARGS)


API_ARGS_BEGIN(API_RENAME_FILE_ARGS)
API_ARGS_FIELD(HANDLE,file_handle)
API_ARGS_FIELD(UNICODE_STRING64 *,target_dir)
API_ARGS_FIELD(UNICODE_STRING64 *,target_name)
API_ARGS_FIELD(BOOLEAN,replace_if_exists)
API_ARGS_CLOSE(API_RENAME_FILE_ARGS)


API_ARGS_BEGIN(API_SET_USER_NAME_ARGS)
API_ARGS_FIELD(UNICODE_STRING64 *,sidstring)
API_ARGS_FIELD(UNICODE_STRING64 *,username)
API_ARGS_CLOSE(API_SET_USER_NAME_ARGS)


API_ARGS_BEGIN(API_DISABLE_FORCE_PROCESS_ARGS)
API_ARGS_FIELD(ULONG *,set_flag)
API_ARGS_FIELD(ULONG *,get_flag)
API_ARGS_CLOSE(API_DISABLE_FORCE_PROCESS_ARGS)


API_ARGS_BEGIN(API_MONITOR_CONTROL_ARGS)
API_ARGS_FIELD(ULONG *,set_flag)
API_ARGS_FIELD(ULONG *,get_flag)
//API_ARGS_FIELD(ULONG *,get_used)
API_ARGS_CLOSE(API_MONITOR_CONTROL_ARGS)

//API_ARGS_BEGIN(API_MONITOR_GET_PUT_ARGS)
//API_ARGS_FIELD(ULONG, log_type)
//API_ARGS_FIELD(ULONG, log_len)
//API_ARGS_FIELD(WCHAR *, log_ptr)
//API_ARGS_CLOSE(API_MONITOR_GET_PUT_ARGS)

API_ARGS_BEGIN(API_MONITOR_PUT_EX_ARGS)
API_ARGS_FIELD(ULONG, log_type)
API_ARGS_FIELD(ULONG, log_len)
API_ARGS_FIELD(WCHAR *, log_ptr)
API_ARGS_FIELD(ULONG, log_pid)
API_ARGS_FIELD(ULONG, log_tid)
//API_ARGS_FIELD(ULONG*, log_aux)
API_ARGS_CLOSE(API_MONITOR_PUT_EX_ARGS)

API_ARGS_BEGIN(API_MONITOR_GET_EX_ARGS)
//API_ARGS_FIELD(ULONG *, log_seq)
API_ARGS_FIELD(ULONG *, log_type)
API_ARGS_FIELD(UNICODE_STRING64*, log_data)
API_ARGS_FIELD(ULONG*, log_pid)
API_ARGS_FIELD(ULONG*, log_tid)
//API_ARGS_FIELD(ULONG*, log_aux)
API_ARGS_CLOSE(API_MONITOR_GET_EX_ARGS)

API_ARGS_BEGIN(API_MONITOR_PUT2_ARGS)
API_ARGS_FIELD(ULONG, log_type)
API_ARGS_FIELD(ULONG, log_len)
API_ARGS_FIELD(WCHAR *, log_ptr)
API_ARGS_FIELD(BOOLEAN,check_object_exists)
API_ARGS_FIELD(BOOLEAN,is_message)
//API_ARGS_FIELD(ULONG, log_aux)
API_ARGS_CLOSE(API_MONITOR_PUT2_ARGS)

API_ARGS_BEGIN(API_MONITOR_GET2_ARGS)
API_ARGS_FIELD(WCHAR *, buffer_ptr)
API_ARGS_FIELD(ULONG *, buffer_len)
API_ARGS_CLOSE(API_MONITOR_GET2_ARGS)

API_ARGS_BEGIN(API_GET_UNMOUNT_HIVE_ARGS)
API_ARGS_FIELD(WCHAR *,path)
API_ARGS_CLOSE(API_GET_UNMOUNT_HIVE_ARGS)


API_ARGS_BEGIN(API_GET_FILE_NAME_ARGS)
API_ARGS_FIELD(HANDLE,handle)
API_ARGS_FIELD(ULONG *,name_len)
API_ARGS_FIELD(WCHAR *,name_buf)
API_ARGS_FIELD(WCHAR *,type_buf)
API_ARGS_CLOSE(API_GET_FILE_NAME_ARGS)


API_ARGS_BEGIN(API_OPEN_FILE_ARGS)
API_ARGS_FIELD(ULONG,path_len)
API_ARGS_FIELD(WCHAR *,path_str)
API_ARGS_FIELD(HANDLE *,file_handle)
API_ARGS_CLOSE(API_OPEN_FILE_ARGS)


API_ARGS_BEGIN(API_CHECK_INTERNET_ACCESS_ARGS)
API_ARGS_FIELD(HANDLE,process_id)
API_ARGS_FIELD(WCHAR *,device_name)
API_ARGS_FIELD(BOOLEAN,issue_message)
API_ARGS_CLOSE(API_CHECK_INTERNET_ACCESS_ARGS)


API_ARGS_BEGIN(API_GET_HOME_PATH_ARGS)
API_ARGS_FIELD(UNICODE_STRING64 *,nt_path)
API_ARGS_FIELD(UNICODE_STRING64 *,dos_path)
API_ARGS_CLOSE(API_GET_HOME_PATH_ARGS)


API_ARGS_BEGIN(API_GET_BLOCKED_DLL_ARGS)
API_ARGS_FIELD(WCHAR *,dll_name_buf)
API_ARGS_FIELD(ULONG,dll_name_len)
API_ARGS_CLOSE(API_GET_BLOCKED_DLL_ARGS)


API_ARGS_BEGIN(API_OPEN_DEVICE_MAP_ARGS)
API_ARGS_FIELD(HANDLE *,handle)
API_ARGS_CLOSE(API_OPEN_DEVICE_MAP_ARGS)


API_ARGS_BEGIN(API_SESSION_LEADER_ARGS)
API_ARGS_FIELD(HANDLE,token_handle)
API_ARGS_FIELD(ULONG64 *,process_id)
API_ARGS_FIELD(ULONG,session_id)
API_ARGS_CLOSE(API_SESSION_LEADER_ARGS)


API_ARGS_BEGIN(API_IS_BOX_ENABLED_ARGS)
API_ARGS_FIELD(WCHAR *,box_name)
API_ARGS_FIELD(WCHAR *,sid_string)
API_ARGS_FIELD(ULONG, session_id)
API_ARGS_CLOSE(API_IS_BOX_ENABLED_ARGS)


API_ARGS_BEGIN(API_QUERY_SYMBOLIC_LINK_ARGS)
API_ARGS_FIELD(WCHAR *,name_buf)
API_ARGS_FIELD(ULONG,name_len)
API_ARGS_CLOSE(API_QUERY_SYMBOLIC_LINK_ARGS)


API_ARGS_BEGIN(API_OPEN_KEY_ARGS)
API_ARGS_FIELD(ULONG,path_len)
API_ARGS_FIELD(WCHAR *,path_str)
API_ARGS_FIELD(HANDLE *,key_handle)
API_ARGS_CLOSE(API_OPEN_KEY_ARGS)


API_ARGS_BEGIN(API_SET_LOW_LABEL_ARGS)
API_ARGS_FIELD(ULONG,path_len)
API_ARGS_FIELD(WCHAR *,path_str)
API_ARGS_CLOSE(API_SET_LOW_LABEL_ARGS)


//API_ARGS_BEGIN(API_ALLOW_SPOOLER_PRINT_TO_FILE_ARGS)
//API_ARGS_FIELD(HANDLE,process_id)
//API_ARGS_CLOSE(API_ALLOW_SPOOLER_PRINT_TO_FILE_ARGS)


//API_ARGS_BEGIN(API_RELOAD_CONF2_ARGS)
//API_ARGS_FIELD(ULONG,session_id)
//API_ARGS_FIELD(UNICODE_STRING64 *,config_path)
//API_ARGS_CLOSE(API_RELOAD_CONF2_ARGS)


API_ARGS_BEGIN(API_GET_DYNAMIC_PORT_FROM_PID_ARGS)
API_ARGS_FIELD(HANDLE,process_id)
//API_ARGS_FIELD(WCHAR *,exe_name)
API_ARGS_FIELD(WCHAR *,full_port_name)
API_ARGS_CLOSE(API_GET_DYNAMIC_PORT_FROM_PID_ARGS)


API_ARGS_BEGIN(API_PROCESS_EXEMPTION_CONTROL_ARGS)
API_ARGS_FIELD(HANDLE,process_id)
API_ARGS_FIELD(ULONG,action_id)
API_ARGS_FIELD(ULONG *,set_flag)
API_ARGS_FIELD(ULONG *,get_flag)
API_ARGS_CLOSE(API_PROCESS_EXEMPTION_CONTROL_ARGS)


API_ARGS_BEGIN(API_OPEN_DYNAMIC_PORT_ARGS)
API_ARGS_FIELD(WCHAR*,port_name)
API_ARGS_FIELD(HANDLE,process_id)
API_ARGS_FIELD(WCHAR*,port_id)
API_ARGS_FIELD(ULONG,filter_num)
API_ARGS_FIELD(ULONG*,filter_ids)
API_ARGS_CLOSE(API_OPEN_DYNAMIC_PORT_ARGS)


API_ARGS_BEGIN(API_QUERY_DRIVER_INFO_ARGS)
API_ARGS_FIELD(ULONG,info_class)
API_ARGS_FIELD(VOID *,info_data)
API_ARGS_FIELD(ULONG ,info_len)
API_ARGS_CLOSE(API_QUERY_DRIVER_INFO_ARGS)


API_ARGS_BEGIN(API_SECURE_PARAM_ARGS)
API_ARGS_FIELD(WCHAR *,param_name)
API_ARGS_FIELD(VOID* ,param_data)
API_ARGS_FIELD(ULONG ,param_size)
API_ARGS_FIELD(ULONG* ,param_size_out)
API_ARGS_FIELD(BOOLEAN ,param_verify)
API_ARGS_CLOSE(API_SECURE_PARAM_ARGS)

#undef API_ARGS_BEGIN
#undef API_ARGS_FIELD
#undef API_ARGS_CLOSE


//---------------------------------------------------------------------------
// Parameter Structures for requests from driver to user mode service
//---------------------------------------------------------------------------


typedef struct _SVC_LOOKUP_SID_MSG {

    ULONG session_id;
    WCHAR sid_string[1];

} SVC_LOOKUP_SID_MSG;


typedef struct _SVC_PROCESS_MSG {

    ULONG process_id;
    ULONG session_id;
    ULONG64 create_time;
    BOOLEAN is_wow64;
    BOOLEAN add_to_job;
    BOOLEAN bHostInject;
    ULONG reason;
    WCHAR process_name[64];

} SVC_PROCESS_MSG;


typedef struct _SVC_REGHIVE_MSG {

    ULONG process_id;
    ULONG session_id;
    WCHAR boxname[BOXNAME_COUNT];

} SVC_REGHIVE_MSG;


//---------------------------------------------------------------------------


#endif // _MY_API_DEFS_H
