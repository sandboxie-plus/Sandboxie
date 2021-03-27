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
// Process Management
//---------------------------------------------------------------------------


#ifndef _MY_PROCESS_H
#define _MY_PROCESS_H


#include "driver.h"
#include "box.h"


//---------------------------------------------------------------------------
// Defines
//---------------------------------------------------------------------------


#ifdef _WIN64
#define PROCESS_TERMINATED      ((PROCESS *) 0xFFFFFFFFFFFFFFFF)
#else
#define PROCESS_TERMINATED      ((PROCESS *) 0xFFFFFFFF)
#endif


//---------------------------------------------------------------------------
// Structures and Types
//---------------------------------------------------------------------------


struct _PROCESS {

    // changes to the linked list of PROCESS blocks are synchronized by
    // an exclusive lock on Process_ListLock

    LIST_ELEM list_elem;

    // process id

    HANDLE pid;

    // process pool.  created on process creation.  it is freed in its
    // entirety when the process terminates

    POOL *pool;

    // box parameters

    BOX *box;

    // full name and extension of executable image, and an indication if
    // the image was loaded from within the copy system

    WCHAR  *image_path;

    WCHAR  *image_name;
    ULONG   image_name_len;             // in bytes, including NULL

    BOOLEAN image_from_box;
    BOOLEAN image_sbie;

    // process creation time and integrity level

    ULONG64 create_time;

    ULONG integrity_level;

    ULONG ntdll32_base;

    // original process primary access token

    void *primary_token;

    // thread data

    PERESOURCE threads_lock;

    LIST threads;

    // flags

    volatile BOOLEAN initialized;       // process initialized once?
    volatile BOOLEAN terminated;        // process termination started?
    volatile ULONG   reason;            // process termination reason

    BOOLEAN untouchable;                // protected process?

    BOOLEAN drop_rights;                // admin rights should be dropped?
    BOOLEAN rights_dropped;             // admin rights were dropped?

    BOOLEAN forced_process;

    BOOLEAN sbiedll_loaded;
    BOOLEAN sbielow_loaded;

    BOOLEAN is_start_exe;
    BOOLEAN parent_was_start_exe;
    BOOLEAN parent_was_sandboxed;

    BOOLEAN change_notify_token_flag;

    BOOLEAN in_pca_job;

    UCHAR   create_console_flag;

    ULONG call_trace;

    // file-related

    PERESOURCE file_lock;
    LIST open_file_paths;               // PATTERN elements
    LIST closed_file_paths;             // PATTERN elements
    LIST read_file_paths;               // PATTERN elements
    LIST write_file_paths;              // PATTERN elements
    LIST blocked_dlls;
    ULONG file_trace;
    ULONG pipe_trace;
    BOOLEAN file_warn_internet;
    BOOLEAN file_warn_direct_access;
	BOOLEAN AllowInternetAccess;
    BOOLEAN file_open_devapi_cmapi;

    // key-related

    PERESOURCE key_lock;
    KEY_MOUNT *key_mount;
    LIST open_key_paths;                // PATTERN elements
    LIST closed_key_paths;              // PATTERN elements
    LIST read_key_paths;                // PATTERN elements
    LIST write_key_paths;               // PATTERN elements
    ULONG key_trace;

    // ipc-related

    PERESOURCE ipc_lock;
    LIST open_ipc_paths;                // PATTERN elements
    LIST closed_ipc_paths;              // PATTERN elements
    ULONG ipc_trace;
    BOOLEAN ipc_warn_startrun;
    BOOLEAN ipc_block_password;
    BOOLEAN ipc_open_lsa_endpoint;
    BOOLEAN ipc_open_sam_endpoint;
    BOOLEAN ipc_allowSpoolerPrintToFile;
    BOOLEAN ipc_openPrintSpooler;

    // gui-related

    PERESOURCE gui_lock;
    BOOLEAN open_all_win_classes;
    void *block_fake_input_hwnd;
    void *gui_user_area;
    WCHAR *gui_class_name;
    LIST open_win_classes;
    ULONG gui_trace;

    BOOLEAN bHostInject;

};


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------


BOOLEAN Process_Init(void);

void Process_Unload(BOOLEAN FreeLock);


// Process_Find finds the PROCESS block for the specified process id.
// If ProcessId is NULL, returns current sandboxed process.
// If ProcessId is NULL and current sandboxed process is scheduled for
// termination, the return value is PROCESS_TERMINATED

PROCESS *Process_Find(HANDLE ProcessId, KIRQL *out_irql);

PROCESS *Process_FindSandboxed(HANDLE ProcessId, KIRQL *out_irql);


// Start supervising a new process

void Process_NotifyProcess_Create(
    HANDLE ProcessId, HANDLE ParentId, HANDLE CallerId, BOX *box);


// Process_IsSameBox returns TRUE if the other process identified by
// 'proc2' or 'proc2_pid' is in the same sandbox as the caller 'proc'.
// note that 'proc2_pid' is used only if 'proc2' is passed as NULL.

BOOLEAN Process_IsSameBox(PROCESS *proc, PROCESS *proc2, ULONG_PTR proc2_pid);


// Process_MatchImage:  given an image name pattern 'pat_str', which
// may contain wild cards, tests the image name 'test_str' against
// the pattern.  If 'pat_len' is specified, only the first 'pat_len'
// characters of 'pat_str' are used for the pattern.  All memory
// temporarily needed is is allocated from specified pool

BOOLEAN Process_MatchImage(
    BOX *box, const WCHAR *pat_str, ULONG pat_len, const WCHAR *test_str,
    ULONG depth);


// Process_GetPaths:  given a process and the name of a path-list
// setting (e.g. OpenFilePath), adds the settings into the specified list.
// if AddStar is TRUE, then for each value for this setting, a star (*)
// is suffixed unless the value already contains a star anywhere

BOOLEAN Process_GetPaths(
    PROCESS *proc, LIST *list, const WCHAR *setting_name, BOOLEAN AddStar);


// Process_GetPaths2:  similar to Process_GetPaths, but adds the path
// only if it does not already match the second path-list

BOOLEAN Process_GetPaths2(
    PROCESS *proc, LIST *list, LIST *list2,
    const WCHAR *setting_name, BOOLEAN AddStar);


// Process_AddPath:   given a process and the name of a path-list
// setting (e.g. OpenFilePath), add the value as the first or last
// entry, depending on AddFirst.  does not add a suffix wildcard

BOOLEAN Process_AddPath(
    PROCESS *proc, LIST *list, const WCHAR *setting_name,
    BOOLEAN AddFirst, const WCHAR *value, BOOLEAN AddStar);


// Process_MatchPath:  given a list that was previously initialized with
// Process_GetPaths, tests if the passed string 'path' matches any pattern.
// path_len specifies the number of characters in path, excluding the
// null terminator, or in other words, path_len is wcslen(path).
// Returns the source string for the pattern that matched the path.

const WCHAR *Process_MatchPath(
    POOL *pool, const WCHAR *path, ULONG path_len,
    LIST *open_list, LIST *closed_list,
    BOOLEAN *is_open, BOOLEAN *is_closed);


// Build a standard entry for hooks.  The standard entry calls
// Process_Find(NULL, NULL).  If non-zero (this includes -1 for
// PROCESS_TERMINATED), the entry jumps to NewProc, where
// the process object is available to _GetAx (see util.h).
// If Process_Find returns zero, the entry jumps to OldProc.
// In either case, if IncPtr was specified, the entry includes
// code to increment the ULONG at IncPtr

ULONG_PTR Process_BuildHookEntry(
    ULONG_PTR NewProc, ULONG_PTR OldProc, ULONG *IncPtr);

void Process_DisableHookEntry(ULONG_PTR HookEntry);


// Return the current process as prepared by the standard hook entry

PROCESS *Process_GetCurrent(void);


// Check for use of multiple sandboxes at once

BOOLEAN Process_CheckTooManyBoxes(const BOX *box);


// Returns ProcessName.exe for idProcess, allocated from the specified pool.
// On return, *out_buf points to a UNICODE_STRING structure which points
// to the NULL-terminated process name immediately following the structure.
// *out_len contains the length of *out_buf.
// *out_ptr receives out_buf->Buffer.
// Free buffer using Mem_Free(*out_buf, *out_len).
// On error, out_buf, out_len, out_ptr receive NULL

void Process_GetProcessName(
    POOL *pool, ULONG_PTR idProcess,
    void **out_buf, ULONG *out_len, WCHAR **out_ptr);


// Check if open_path contains setting "$:ProcessName.exe"
// where ProcessName matches the specified idProcess.
// If not contained, returns STATUS_ACCESS_DENIED with *pSetting = NULL
// If contained, returns STATUS_SUCCESS with *pSetting -> matching setting

NTSTATUS Process_CheckProcessName(
    PROCESS *proc, LIST *open_paths, ULONG_PTR idProcess,
    const WCHAR **pSetting);


// Return SID string and session ID for the process specified by the
// process handle or process id number.
// To free the returned SidString, use RtlFreeUnicodeString

NTSTATUS Process_GetSidStringAndSessionId(
    HANDLE ProcessHandle, HANDLE ProcessId,
    UNICODE_STRING *SidString, ULONG *SessionId);


// Get a box for a forced sandboxed process

BOX *Process_GetForcedStartBox(
    HANDLE ProcessId, HANDLE ParentId, const WCHAR *ImagePath, BOOLEAN bHostInject);


// Manipulation of the List of Disabled Forced Processes:  (Process_List2)
// Add ProcessId to list if ParentId is already listed
// Hold Process_ListLock before calling, if ParentId != PROCESS_TERMINATED

BOOLEAN Process_DfpInsert(HANDLE ParentId, HANDLE ProcessId);


// Manipulation of the List of Disabled Forced Processes:  (Process_List2)
// Delete ProcessId from list
// Hold Process_ListLock before calling

void Process_DfpDelete(HANDLE ProcessId);


// Manipulation of the List of Disabled Forced Processes:  (Process_List2)
// Check if ProcessId is listed
// Do not hold Process_ListLock before calling

BOOLEAN Process_DfpCheck(HANDLE ProcessId, BOOLEAN *silent);


// Enumerate or count processes in a sandbox

NTSTATUS Process_Enumerate(
    const WCHAR *boxname, BOOLEAN all_sessions, ULONG session_id,
    ULONG *pids, ULONG *count);


// Issue log message with detail "ProcessName [BoxName]"

void Process_LogMessage(PROCESS *proc, ULONG msgid);


// Track process limit

//void Process_TrackProcessLimit(PROCESS *proc);


// Cancel process through SbieSvc

void Process_CancelProcess(PROCESS *proc);


// Check if process is running within a
// Program Compatibility Assistant (PCA) job

BOOLEAN Process_IsInPcaJob(HANDLE ProcessId);


void Process_SetTerminated(PROCESS *proc, ULONG reason);


//
// low level syscal interface (process_low.c)
//


BOOLEAN Process_Low_Init(void);

void Process_Low_Unload(void);

BOOLEAN Process_Low_Inject(
    HANDLE process_id, ULONG session_id, ULONG64 create_time,
    const WCHAR *image_name, BOOLEAN add_process_to_job, BOOLEAN bHostInject);

BOOLEAN Process_Low_InitConsole(PROCESS *proc);


//---------------------------------------------------------------------------


NTSTATUS Process_Api_Start(PROCESS *proc, ULONG64 *parms);

NTSTATUS Process_Api_Query(PROCESS *proc, ULONG64 *parms);

NTSTATUS Process_Api_QueryInfo(PROCESS *proc, ULONG64 *parms);

NTSTATUS Process_Api_QueryBoxPath(PROCESS *proc, ULONG64 *parms);

NTSTATUS Process_Api_QueryProcessPath(PROCESS *proc, ULONG64 *parms);

NTSTATUS Process_Api_QueryPathList(PROCESS *proc, ULONG64 *parms);

NTSTATUS Process_Api_Enum(PROCESS *proc, ULONG64 *parms);


//---------------------------------------------------------------------------
// Variables
//---------------------------------------------------------------------------


extern LIST Process_List;
extern LIST Process_ListDfp;
extern PERESOURCE Process_ListLock;

extern volatile BOOLEAN Process_ReadyToSandbox;

extern const WCHAR *Process_UnknownImageName;


//---------------------------------------------------------------------------


#endif // _MY_PROCESS_H
