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
// Process force checking
//---------------------------------------------------------------------------


#include "process.h"
#include "box.h"
#include "util.h"
#include "session.h"
#include "conf.h"
#include "file.h"
#include "token.h"
#include "common/pattern.h"
#include "common/my_version.h"


//---------------------------------------------------------------------------
// Structures and Types
//---------------------------------------------------------------------------


typedef struct _FORCE_BOX {

    LIST_ELEM list_elem;
    BOX *box;
    LIST ForceFolder;
    LIST ForceProcess;
    LIST ForceChildren;
	LIST AlertFolder;
    LIST AlertProcess;
    LIST HostInjectProcess;

} FORCE_BOX;

typedef struct _FORCE_ENTRY {

    LIST_ELEM list_elem;
    ULONG buf_len;
    ULONG len;
    WCHAR *buf;
    PATTERN *pat;

} FORCE_ENTRY;

#define MAX_FORCE_PROCESS_VALUE_LEN 1024

typedef struct _FORCE_PROCESS {

    LIST_ELEM list_elem;
    WCHAR value[MAX_FORCE_PROCESS_VALUE_LEN];
} FORCE_PROCESS;


typedef struct _FORCE_PROCESS_2 {

#ifndef USE_PROCESS_MAP
    LIST_ELEM list_elem;
#endif
    HANDLE pid;
    BOOLEAN silent;

} FORCE_PROCESS_2;


typedef struct _FORCE_PROCESS_3 {

#ifndef USE_PROCESS_MAP
    LIST_ELEM list_elem;
#endif
    HANDLE pid;
    WCHAR boxname[BOXNAME_COUNT];

} FORCE_PROCESS_3;

//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------


PEPROCESS Process_OpenAndQuery(
    HANDLE ProcessId, UNICODE_STRING *SidString, ULONG *SessionId);

static NTSTATUS Process_TranslateDosToNt(
    const WCHAR *in_path, WCHAR **out_path, ULONG *out_len);

static void Process_GetCurDir(
    PEPROCESS ProcessObject, WCHAR **pCurDir, ULONG *pCurDirLen);

static void Process_GetDocArg(
    PEPROCESS ProcessObject, WCHAR **pDocArg, ULONG *pDocArgLen);

static BOOLEAN Process_IsDcomLaunchParent(HANDLE ParentId);

//static BOOLEAN Process_IsWindowsExplorerParent(HANDLE ParentId);

static BOOLEAN Process_IsImmersiveProcess(
    PEPROCESS ProcessObject, HANDLE ParentId, ULONG SessionId);

//static BOOLEAN Process_IsProcessParent(HANDLE ParentId, WCHAR* Name);

void Process_CreateForceData(
    LIST *boxes, const WCHAR *SidString, ULONG SessionId);

void Process_DeleteForceData(LIST *boxes);

static BOX *Process_CheckBoxPath(LIST *boxes, const WCHAR *path);

static BOX *Process_CheckForceFolder(
    LIST *boxes, const WCHAR *path, BOOLEAN alert, ULONG *IsAlert);

static BOX *Process_CheckForceProcess(
    LIST *boxes, const WCHAR *name, const WCHAR* path, BOOLEAN alert, ULONG *IsAlert, const WCHAR *ParentName, const WCHAR *ParentPath);

static void Process_CheckAlertFolder(
	LIST *boxes, const WCHAR *path, ULONG *IsAlert);

static void Process_CheckAlertProcess(
    LIST *boxes, const WCHAR *name, const WCHAR *path, ULONG *IsAlert);

static BOX *Process_CheckHostInjectProcess(
    LIST *boxes, const WCHAR *name);

static BOOLEAN Process_CheckMoTW(const WCHAR *path);


//---------------------------------------------------------------------------
// Process_GetForcedStartBox
//---------------------------------------------------------------------------


_FX BOX *Process_GetForcedStartBox(
    HANDLE ProcessId, HANDLE ParentId, const WCHAR *ImagePath, BOOLEAN* pHostInject, const WCHAR *pSidString)
{
    NTSTATUS status;
    ULONG SessionId;
    UNICODE_STRING SidString;
    WCHAR *ImagePath2 = L"";
    ULONG ImagePath2_len;
    const WCHAR *ImageName = L"";

    PEPROCESS ProcessObject;
    WCHAR *CurDir, *DocArg;
    ULONG CurDir_len, DocArg_len;
    LIST boxes;
    BOX *box;
    ULONG alert;
    BOOLEAN check_force;
    BOOLEAN is_start_exe;
    BOOLEAN image_sbie;
    BOOLEAN force_alert;
    BOOLEAN dfp_already_added;
    BOOLEAN same_image_name;

	void* nbuf = NULL;
	ULONG nlen = 0;
	WCHAR* ParentName = NULL;
    WCHAR* ParentPath = NULL;

    check_force = TRUE;

    //
    // get process object to access SID string, session ID and PEB data
    //
    // note that a child of DcomLaunch may use the LocalSystem SID at this
    // point (particularly on XP/2003), but we still want to force such a
    // process into the sandbox, so we use Process_IsDcomLaunchParent
    //

    ProcessObject = Process_OpenAndQuery(ProcessId, &SidString, &SessionId);

    if (! pSidString)
        pSidString = SidString.Buffer;

    if (! ProcessObject)
        return NULL;

    if ((! Conf_Get_Boolean(NULL, L"AllowForceSystem", 0, FALSE)) &&
            (_wcsicmp(pSidString, Driver_S_1_5_18) == 0 || //	System
             _wcsicmp(pSidString, Driver_S_1_5_19) == 0 || //	Local Service
             _wcsicmp(pSidString, Driver_S_1_5_20) == 0)   //	Network Service
                && (! Process_IsDcomLaunchParent(ParentId)) ){

        check_force = FALSE;
    } 
    
    //
    // get adjusted image path and image name
    //

    status = Process_TranslateDosToNt(
                                ImagePath, &ImagePath2, &ImagePath2_len);

    if (NT_SUCCESS(status)) {

        ImageName = wcsrchr(ImagePath2, L'\\');
        if (ImageName && ImageName[1])
            ++ImageName;
        else
            status = STATUS_OBJECT_PATH_SYNTAX_BAD;     // random
    }

    if (! NT_SUCCESS(status)) {

        RtlFreeUnicodeString(&SidString);
        ObDereferenceObject(ProcessObject);
        return NULL;
    }

    //
    // initialize ParentName but only if the parent is not a system process
    // 

    if (!MyIsProcessRunningAsSystemAccount(ParentId)) {

        Process_GetProcessName(
            Driver_Pool, (ULONG_PTR)ParentId, &nbuf, &nlen, &ParentName);
        if (nbuf)
            ParentPath = ((UNICODE_STRING *)nbuf)->Buffer;
    }

    //
    // initialize some more state before checking process
    //

    dfp_already_added = FALSE;
    same_image_name = FALSE;

    Process_GetCurDir(ProcessObject, &CurDir, &CurDir_len);

    Process_GetDocArg(ProcessObject, &DocArg, &DocArg_len);

    dfp_already_added = Process_DfpCheck(ProcessId, &same_image_name);
    if (dfp_already_added)
        force_alert = TRUE;
    else
        force_alert = Session_IsForceDisabled(SessionId);

    box = NULL;
    alert = 0;

    Process_CreateForceData(&boxes, pSidString, SessionId);

    //
    // check if process can be forced
    //

    if (check_force) {

        box = Process_CheckBoxPath(&boxes, ImagePath2);

        //
        // when the process is start.exe we ignore the CurDir and DocArg
        //

        Process_IsSbieImage(ImagePath, &image_sbie, &is_start_exe);

        if ((! box) && CurDir && !is_start_exe)
            box = Process_CheckBoxPath(&boxes, CurDir);

        if (!box) {

            box = Process_CheckForceFolder(
                        &boxes, ImagePath2, force_alert, &alert);

            if ((! box) && (! alert)) {
                box = Process_CheckForceProcess(
                    &boxes, ImageName, ImagePath2, force_alert, &alert, ParentName, ParentPath);
            }

            if ((! box) && CurDir && !is_start_exe && (! alert)) {
                box = Process_CheckForceFolder(
                        &boxes, CurDir, force_alert, &alert);
            }

            if ((! box) && DocArg && !is_start_exe && (! alert)) {
                box = Process_CheckForceFolder(
                        &boxes, DocArg, force_alert, &alert);
            }

            if (box && (! Conf_Get_Boolean(NULL, L"AllowForceImmersive", 0, FALSE)) &&
                        Process_IsImmersiveProcess(
                                        ProcessObject, ParentId, SessionId)) {
                box = NULL;
                alert = 1;
                same_image_name = TRUE;
            }

            if ((alert == 1) && (! dfp_already_added))
                Process_DfpInsert(PROCESS_TERMINATED, ProcessId);
        }

        //
        // Check if the parent process has its children forced to be sandboxes
        // exempt sandboxie components from this as start.exe can be used to 
        // open selected processes in other boxes or set Dfp when desired.
        // 
        // we also must exempt conhost.exe for console applications
        //

        if (!box && !image_sbie && _wcsicmp(ImageName, L"conhost.exe") != 0) {

            WCHAR boxname[BOXNAME_COUNT];

            if (Process_FcpCheck(ParentId, boxname)) {

                ULONG boxname_len = (wcslen(boxname) + 1) * sizeof(WCHAR);
                for (FORCE_BOX* cur_box = List_Head(&boxes); cur_box; cur_box = List_Next(cur_box)) {
                    if (cur_box->box->name_len == boxname_len
                        && _wcsicmp(cur_box->box->name, boxname) == 0) {
                        box = cur_box->box;
                        break;
                    }
                }
            }
        }

		if (alert != 1)
			force_alert = FALSE;

		if ((! box) && (alert != 1))
			Process_CheckAlertFolder(&boxes, ImagePath2, &alert);

		//
		// for alerting we only care about the process path not about the working dir or command line
		//

        if ((! box) && (alert != 1))
            Process_CheckAlertProcess(&boxes, ImageName, ImagePath2, &alert);
    }

    //
    // check mark of the web (MoTW)
    //

    if (! box) {

        if (Conf_Get_Boolean(NULL, L"ForceMarkOfTheWeb", 0, FALSE)) {

            if (Process_CheckMoTW(ImagePath2) || (DocArg && Process_CheckMoTW(DocArg))) {

                const WCHAR* MoTW_Box = Conf_Get(NULL, L"MarkOfTheWebBox", 0);
                if (!MoTW_Box || !*MoTW_Box)
                    MoTW_Box = L"DefaultBox";

                box = (BOX*)-1; // when box not found cancel process

                ULONG MoTW_Box_len = (wcslen(MoTW_Box) + 1) * sizeof(WCHAR);
                FORCE_BOX* fbox = List_Head(&boxes);
                while (fbox) {
                    if (MoTW_Box_len == fbox->box->name_len && _wcsicmp(MoTW_Box, fbox->box->name) == 0) {
                        box = fbox->box;
                        break;
                    }
                    fbox = List_Next(fbox);
                }
            }
        }
    }

    //
    // check if process should be injected
    //

    if ((! box) && (alert != 1) && pHostInject != NULL) {
        
        box = Process_CheckHostInjectProcess(&boxes, ImageName);

        if (box)
            *pHostInject = TRUE;
    }

    //
    // complete evaluation
    //

    if (box && box != (BOX *)-1) {

        box = Box_Clone(Driver_Pool, box);
        if (!box)
            box = (BOX *)-1;
    }

    if ((alert == 1) && (! same_image_name)) {

		if ((force_alert == 0) && Conf_Get_Boolean(NULL, L"StartRunAlertDenied", 0, FALSE))
		{
			if(Conf_Get_Boolean(NULL, L"AlertStartRunAccessDenied", 0, TRUE))
				Log_Msg_Process(MSG_1308, ImageName, NULL, SessionId, ProcessId);

			box = (BOX *)-1;
		}
		else if ((force_alert == 0) || ((force_alert == 1) && Conf_Get_Boolean(NULL, L"NotifyForceProcessDisabled", 0, FALSE)))
		{
			Log_Msg_Process(MSG_1301, ImageName, NULL, SessionId, ProcessId);
		}
    }

    if (box && box != (BOX *)-1 && Conf_Get_Boolean(NULL, L"NotifyForceProcessEnabled", 0, FALSE))
    {
        Log_Msg_Process(MSG_1321, ImageName, box->name, SessionId, ProcessId);
    }

    //
    // finish
    //

    Process_DeleteForceData(&boxes);

    if (nbuf)
		Mem_Free(nbuf, nlen);

    if (DocArg)
        Mem_Free(DocArg, DocArg_len);

    if (CurDir)
        Mem_Free(CurDir, CurDir_len);

    Mem_Free(ImagePath2, ImagePath2_len);

    RtlFreeUnicodeString(&SidString);
    ObDereferenceObject(ProcessObject);

    return box;
}


//---------------------------------------------------------------------------
// Process_OpenAndQuery
//---------------------------------------------------------------------------


_FX PEPROCESS Process_OpenAndQuery(
    HANDLE ProcessId, UNICODE_STRING *SidString, ULONG *SessionId)
{
    PEPROCESS ProcessObject;
    PACCESS_TOKEN TokenObject;

    NTSTATUS status = PsLookupProcessByProcessId(ProcessId, &ProcessObject);
    if (! NT_SUCCESS(status)) {

        ProcessObject = NULL;
        *SessionId = -1;

    } else {

        *SessionId = PsGetProcessSessionId(ProcessObject);

        if (SidString) {

            TokenObject = PsReferencePrimaryToken(ProcessObject);
            status = Token_QuerySidString(TokenObject, SidString);
            PsDereferencePrimaryToken(TokenObject);

            if (! NT_SUCCESS(status)) {

                ObDereferenceObject(ProcessObject);
                ProcessObject = NULL;
            }
        }
    }

    return ProcessObject;
}


//---------------------------------------------------------------------------
// Process_TranslateDosToNt
//---------------------------------------------------------------------------


_FX NTSTATUS Process_TranslateDosToNt(
    const WCHAR *in_path, WCHAR **out_path, ULONG *out_len)
{
    NTSTATUS status = File_TranslateDosToNt(
                            in_path, Driver_Pool, out_path, out_len);

    if (! NT_SUCCESS(status)) {

        WCHAR *buf;
        ULONG buf_len;

        if (status != STATUS_OBJECT_PATH_SYNTAX_BAD)
            return status;

        buf_len = (wcslen(in_path) + 1) * sizeof(WCHAR);
        buf = Mem_Alloc(Driver_Pool, buf_len);
        if (! buf)
            return STATUS_INSUFFICIENT_RESOURCES;
        memcpy(buf, in_path, buf_len);

        *out_path = buf;
        *out_len = buf_len;
    }

    return STATUS_SUCCESS;
}


//---------------------------------------------------------------------------
// Process_GetStringFromPeb
//---------------------------------------------------------------------------


_FX void Process_GetStringFromPeb(
    PEPROCESS ProcessObject, ULONG StringOffset, ULONG StringMaxLenInChars,
    WCHAR **OutBuffer, ULONG *OutLength)
{
    ULONG_PTR Peb;
    ULONG_PTR RtlUserProcessParms;
    UNICODE_STRING *uni;
    WCHAR *LocalBuffer, *Buffer;
    ULONG LocalBufferLength, Length, MaximumLength;
    KAPC_STATE ApcState;

    *OutBuffer = NULL;
    *OutLength = 0;

    Peb = PsGetProcessPeb(ProcessObject);
    if (! Peb)
        return;

    LocalBufferLength = (StringMaxLenInChars + 16) * sizeof(WCHAR);
    LocalBuffer = Mem_Alloc(Driver_Pool, LocalBufferLength);
    if (! LocalBuffer)
        return;

    KeStackAttachProcess(ProcessObject, &ApcState);

    __try {

        //
        // make sure PEB block is accessible
        //

        const ULONG UserProcessParms_offset =
#ifdef _WIN64
                                                0x20;   // 64-bit
#else
                                                0x10;   // 32-bit
#endif

        ProbeForRead((void *)Peb, 0x20, sizeof(ULONG_PTR));
        RtlUserProcessParms = *(ULONG_PTR *)(Peb + UserProcessParms_offset);
        ProbeForRead((void *)RtlUserProcessParms, 0x50, sizeof(ULONG_PTR));

        //
        // make sure the requested string is accessible
        //

        uni = (UNICODE_STRING *)(RtlUserProcessParms + StringOffset);
        ProbeForRead(uni, sizeof(UNICODE_STRING), sizeof(ULONG));

        Length = uni->Length / sizeof(WCHAR);
        MaximumLength = uni->MaximumLength / sizeof(WCHAR);
        Buffer = uni->Buffer;

        if (Length && MaximumLength && Buffer &&
                Length <= StringMaxLenInChars && Length <= MaximumLength) {

            if ((ULONG_PTR)Buffer < (ULONG_PTR)RtlUserProcessParms) {
                Buffer = (WCHAR *)
                    ((ULONG_PTR)RtlUserProcessParms + (ULONG_PTR)Buffer);
            }

            ProbeForRead(Buffer, Length * sizeof(WCHAR), sizeof(WCHAR));

            //
            // success
            //

            wmemcpy(LocalBuffer, Buffer, Length);
            LocalBuffer[Length] = L'\0';

            *OutBuffer = LocalBuffer;
            *OutLength = LocalBufferLength;
            LocalBuffer = NULL;
        }

    } __except (EXCEPTION_EXECUTE_HANDLER) {
    }

    KeUnstackDetachProcess(&ApcState);

    //
    // free when not used
    //

    if (LocalBuffer)
        Mem_Free(LocalBuffer, LocalBufferLength);
}


//---------------------------------------------------------------------------
// Process_GetCurDir
//---------------------------------------------------------------------------


_FX void Process_GetCurDir(
    PEPROCESS ProcessObject, WCHAR **pCurDir, ULONG *pCurDirLen)
{
    WCHAR *Buffer;
    ULONG Length;

    const ULONG CurDir_offset =
#ifdef _WIN64
                                0x38;   // 64-bit
#else
                                0x24;   // 32-bit
#endif

    *pCurDir = NULL;
    *pCurDirLen = 0;

    Process_GetStringFromPeb(
                ProcessObject, CurDir_offset, 600, &Buffer, &Length);
    if (Buffer && Length) {

        //
        // buffer was allocated with some extra space
        //

        NTSTATUS status;
        WCHAR *ptr = Buffer + wcslen(Buffer);
        while (ptr > Buffer && ptr[-1] == L'\\')
            --ptr;
        if (ptr > Buffer) {

            ptr[0] = L'\\';
            ptr[1] = L'x';
            ptr[2] = L'\0';

            //
            // get canonical path
            //

            status = Process_TranslateDosToNt(
                                            Buffer, pCurDir, pCurDirLen);
            if (! NT_SUCCESS(status)) {

                *pCurDir = NULL;
                *pCurDirLen = 0;
            }
        }

        Mem_Free(Buffer, Length);
    }
}


//---------------------------------------------------------------------------
// Process_GetDocArg
//---------------------------------------------------------------------------


_FX void Process_GetDocArg(
    PEPROCESS ProcessObject, WCHAR **pDocArg, ULONG *pDocArgLen)
{
    WCHAR *Buffer;
    ULONG Length;

    const ULONG CmdLin_offset =
#ifdef _WIN64
                                0x70;   // 64-bit
#else
                                0x40;   // 32-bit
#endif

    *pDocArg = NULL;
    *pDocArgLen = 0;

    Process_GetStringFromPeb(
                ProcessObject, CmdLin_offset, 600, &Buffer, &Length);
    if (Buffer && Length) {

        //
        // eat first command line argument, the image name
        //

        ULONG len = wcslen(Buffer);
        WCHAR *ptr = Buffer;

        // DbgPrint("Process_GetDocArg - Full CmdLine: [%S]\n", Buffer);

        while (len && *ptr == L' ') {
            --len;
            ++ptr;
        }

        if (len) {

            if (*ptr == L'\"') {

                --len;
                ++ptr;
                while (len && *ptr != L'\"') {
                    --len;
                    ++ptr;
                }
                if (len) {
                    --len;
                    ++ptr;
                }

            } else {

                while (len && *ptr != L' ') {
                    --len;
                    ++ptr;
                }
            }
        }

        //
        // eat optional arguments
        //

        while (len) {

            while (len && *ptr == L' ') {
                --len;
                ++ptr;
            }

            if (*ptr != L'-' && *ptr != L'/')
                break;

            while (len && *ptr != L' ') {
                --len;
                ++ptr;
            }
        }

        //
        // eat second command line argument, the document path
        //

        if (len) {

            WCHAR *doc;

            // Remove any leading spaces or quotes
            while (len && (*ptr == L' ' || *ptr == L'"')) {
                --len;
                ++ptr;
            }


            // Now strip any trailing backslashes, quotes or spaces
            while (len && (ptr[len - 1] == L'\\' || ptr[len - 1] == L' ' || ptr[len - 1] == L'"')) {
                --len;
                ptr[len] = 0;
            }

            doc = ptr;
            // DbgPrint("After Doc Trim: [%S]\n", doc);

            //
            // now that we've stripped any quotes, leading spaces
            // and suffix backslashes, get a canonical path
            //

            if (*doc) {

                NTSTATUS status = Process_TranslateDosToNt(doc, pDocArg, pDocArgLen);
                if (! NT_SUCCESS(status)) {

                    *pDocArg = NULL;
                    *pDocArgLen = 0;
                }
            }
        }

        Mem_Free(Buffer, Length);
    }
}


//---------------------------------------------------------------------------
// Process_GetCommandLine
//---------------------------------------------------------------------------


_FX void Process_GetCommandLine(
    HANDLE ProcessId,
    WCHAR** OutBuffer, ULONG* OutLength)
{
    const ULONG CmdLin_offset =
#ifdef _WIN64
                                0x70;   // 64-bit
#else
                                0x40;   // 32-bit
#endif

    PEPROCESS ProcessObject;
    NTSTATUS status =
        PsLookupProcessByProcessId(ProcessId, &ProcessObject);
    if (NT_SUCCESS(status)) {

        Process_GetStringFromPeb(
                ProcessObject, CmdLin_offset, 600, OutBuffer, OutLength);

        ObDereferenceObject(ProcessObject);
    }
}


//---------------------------------------------------------------------------
// Process_IsDcomLaunchParent
//---------------------------------------------------------------------------


_FX BOOLEAN Process_IsDcomLaunchParent(HANDLE ParentId)
{
    static HANDLE DcomLaunchPid = NULL;

    const ULONG CmdLin_offset =
#ifdef _WIN64
                                0x70;   // 64-bit
#else
                                0x40;   // 32-bit
#endif

    if (! DcomLaunchPid) {

        WCHAR *Buffer;
        ULONG Length;
        Process_GetCommandLine(ParentId, &Buffer, &Length);
        if (Buffer && Length) {

            ULONG len = wcslen(Buffer);
            if (len > 10 &&
                    _wcsicmp(Buffer + len - 10, L"DcomLaunch") == 0) {

                DcomLaunchPid = ParentId;
            }

            Mem_Free(Buffer, Length);
        }
    }

    return (BOOLEAN)(ParentId == DcomLaunchPid);
}


//---------------------------------------------------------------------------
// Process_IsProcessParent
//---------------------------------------------------------------------------


//_FX BOOLEAN Process_IsProcessParent(HANDLE ParentId, WCHAR* Name)
//{
//	BOOLEAN retval = FALSE;
//
//	void* nbuf;
//	ULONG nlen;
//	WCHAR* nptr;
//
//	Process_GetProcessName(
//		Driver_Pool, (ULONG_PTR)ParentId, &nbuf, &nlen, &nptr);
//	if (nbuf) {
//
//		if (_wcsicmp(nptr, Name) == 0) {
//
//			retval = TRUE;
//		}
//
//		Mem_Free(nbuf, nlen);
//	}
//
//	return retval;
//}


//---------------------------------------------------------------------------
// Process_IsWindowsExplorerParent
//---------------------------------------------------------------------------


//_FX BOOLEAN Process_IsWindowsExplorerParent(HANDLE ParentId)
//{
//    return Process_IsProcessParent(ParentId,L"explorer.exe");
//}


//---------------------------------------------------------------------------
// Process_IsImmersiveProcess
//---------------------------------------------------------------------------


_FX BOOLEAN Process_IsImmersiveProcess(
    PEPROCESS ProcessObject, HANDLE ParentId, ULONG SessionId)
{
    ULONG TokenIsAppContainerValue;
    ULONG Length;
    WCHAR *Buffer;
    BOOLEAN immersive;

    const ULONG CmdLin_offset =
#ifdef _WIN64       // 64-bit
            0x70;
#else               // 32-bit
            0x40;
#endif

    if (Driver_OsVersion < DRIVER_WINDOWS_8)
        return FALSE;

    //
    // immersive processes generally have the TokenIsAppContainer
    // information set to 1 in the token.  this is true for child
    // iexplore.exe process for an immersive parent
    //

    TokenIsAppContainerValue =
        (ULONG)(ULONG_PTR)Token_QueryPrimary(TokenIsAppContainer, SessionId);

    if (TokenIsAppContainerValue)
        return TRUE;

    //
    // immersive iexplore.exe (possibly other processes as well?)
    // don't have the TokenIsAppContainer detail, but are started
    // by DcomLaunch (a LocalSystem process) and have a command line
    // that includes the parameter ServerName
    //

    if (! MyIsProcessRunningAsSystemAccount(ParentId))
        return FALSE;

    immersive = FALSE;

    Process_GetStringFromPeb(
                ProcessObject, CmdLin_offset, 600, &Buffer, &Length);
    if (Buffer && Length) {

        //
        // scan buffer for immersive server parameter
        //

        WCHAR *ptr = Buffer;
        ULONG len = wcslen(ptr);

        while ((ptr < Buffer + len) && *ptr) {

            if (*ptr == L'-' && (ptr + 12) < (Buffer + len)) {

                if (memcmp(ptr + 1, L"ServerName:", 11) == 0) {

                    immersive = TRUE;
                    break;
                }
            }

            ++ptr;
        }

        Mem_Free(Buffer, Length);
    }

    return immersive;
}


//---------------------------------------------------------------------------
// Process_AddForceFolders
//---------------------------------------------------------------------------


_FX void Process_AddForceFolders(
    LIST* Folders, const WCHAR* Setting, BOX *box, const WCHAR *section)
{
    ULONG index2;
    const WCHAR *value;
    FORCE_ENTRY *folder;

    index2 = 0;

    while (1) {

        WCHAR *expnd, *buf;
        ULONG buf_len;

        value = Conf_Get(section, Setting, index2);
        if (! value)
            break;
        ++index2;

        if (wcschr(value, L'\\') != NULL) { // folder, full_path or path_pattern

            expnd = Conf_Expand(box->expand_args, value, Setting);

            buf = NULL;

            if (expnd) {

                //
                // remove duplicate backslashes and translate reparse points
                //

                WCHAR* tmp1, * tmp2;
                buf_len = (wcslen(expnd) + 1) * sizeof(WCHAR);
                tmp1 = Mem_Alloc(Driver_Pool, buf_len);

                if (tmp1) {

                    WCHAR* src_ptr = expnd;
                    WCHAR* dst_ptr = tmp1;
                    while (*src_ptr) {
                        if (src_ptr[0] == L'\\' && src_ptr[1] == L'\\') {
                            ++src_ptr;
                            continue;
                        }
                        *dst_ptr = *src_ptr;
                        ++src_ptr;
                        ++dst_ptr;
                    }
                    *dst_ptr = L'\0';

                    tmp2 = File_TranslateReparsePoints(tmp1, Driver_Pool);
                    if (tmp2) {

                        Mem_Free(tmp1, buf_len);
                        buf = tmp2;
                        buf_len = (wcslen(buf) + 1) * sizeof(WCHAR);

                    }
                    else
                        buf = tmp1;
                }

                Mem_FreeString(expnd);
            }
        }
        else { // image name

            buf_len = (wcslen(value) + 1) * sizeof(WCHAR);
            buf = Mem_Alloc(Driver_Pool, buf_len);
            if (buf)
                wcscpy(buf, value);
        }

        if (! buf)
            continue;

        folder = Mem_Alloc(Driver_Pool, sizeof(FORCE_ENTRY));
        if (! folder) {
            Mem_Free(buf, buf_len);
            break;
        }

        if (wcschr(buf, L'*')) {

            folder->pat =
                Pattern_Create(box->expand_args->pool, buf, TRUE, 0);

            Mem_Free(buf, buf_len);

            if (! folder->pat) {
                Mem_Free(folder, sizeof(FORCE_ENTRY));
                break;
            }

            folder->buf_len = 0;
            folder->len = 0;
            folder->buf = NULL;

        } else {

            ULONG len = wcslen(buf);
            while (len && buf[len - 1] == L'\\')
                --len;

            folder->buf_len = buf_len;
            folder->len = len;
            folder->buf = buf;

            folder->pat = NULL;
        }

        List_Insert_After(Folders, NULL, folder);
    }
}


//---------------------------------------------------------------------------
// Process_AddForceProcesses
//---------------------------------------------------------------------------


_FX void Process_AddForceProcesses(
    LIST* Processes, const WCHAR* Setting, const WCHAR *section)
{
    ULONG index2;
    const WCHAR *value;
    FORCE_PROCESS *process;

    index2 = 0;

    while (1) {

        value = Conf_Get(section, Setting, index2);
        if (! value)
            break;
        ++index2;

        process = Mem_Alloc(Driver_Pool, sizeof(FORCE_PROCESS));
        if (! process)
            break;

        // Ideally, RtlStringCchCopy() would be used here - Build issues
        RtlZeroMemory(process->value, sizeof(process->value));
        if (wcslen(value) < MAX_FORCE_PROCESS_VALUE_LEN)
        {
            wcscpy(process->value, value);
        }

        List_Insert_After(Processes, NULL, process);
    }
}


//---------------------------------------------------------------------------
// Process_CreateForceData
//---------------------------------------------------------------------------


_FX void Process_CreateForceData(
    LIST *boxes, const WCHAR *SidString, ULONG SessionId)
{
    ULONG index1;
    const WCHAR *section;
    FORCE_BOX *box;
    
    //
    // scan list of boxes and create FORCE_BOX elements
    //

    List_Init(boxes);

    Conf_AdjustUseCount(TRUE);

    index1 = 0;

    while (1) {

        section = Conf_Get(NULL, NULL, index1);
        if (! section)
            break;
        ++index1;

        if (! Conf_IsBoxEnabled(section, SidString, SessionId))
            continue;

        if (Conf_Get_Boolean(section, L"DisableForceRules", 0, FALSE))
            continue;

        //
        // create FORCE_BOX
        //

        box = Mem_Alloc(Driver_Pool, sizeof(FORCE_BOX));
        if (! box)
            break;

        box->box = Box_CreateEx(
                        Driver_Pool, section, SidString, SessionId, TRUE);
        if (! box->box) {
            Mem_Free(box, sizeof(FORCE_BOX));
            continue;
        }

        List_Init(&box->ForceFolder);
        List_Init(&box->ForceProcess);
        List_Init(&box->ForceChildren);
		List_Init(&box->AlertFolder);
        List_Init(&box->AlertProcess);
        List_Init(&box->HostInjectProcess);

        List_Insert_After(boxes, NULL, box);

        //
        // scan list of ForceFolder settings for the box
        //

        Process_AddForceFolders(&box->ForceFolder, L"ForceFolder", box->box, section);

        //
        // scan list of ForceProcess settings for the box
        //

        //Process_AddForceProcesses(&box->ForceProcess, L"ForceProcess", section);
        Process_AddForceFolders(&box->ForceProcess, L"ForceProcess", box->box, section);

        //
        // scan list of ForceChildren settings for the box
        //

        //Process_AddForceProcesses(&box->ForceChildren, L"ForceChildren", section);
        Process_AddForceFolders(&box->ForceChildren, L"ForceChildren", box->box, section);

		//
        // scan list of AlertFolder settings for the box
        //

        Process_AddForceFolders(&box->AlertFolder, L"AlertFolder", box->box, section);

        //
        // scan list of AlertProcess settings for the box
        //

        //Process_AddForceProcesses(&box->AlertProcess, L"AlertProcess", section);
        Process_AddForceFolders(&box->AlertProcess, L"AlertProcess", box->box, section);

        //
        // scan list of HostInjectProcess settings for the box
        //

        Process_AddForceProcesses(&box->HostInjectProcess, L"HostInjectProcess", section);
    }

    Conf_AdjustUseCount(FALSE);
}


//---------------------------------------------------------------------------
// Process_DeleteForceDataFolders
//---------------------------------------------------------------------------


_FX void Process_DeleteForceDataFolders(LIST* Folders)
{
    FORCE_ENTRY *folder;
	while (1) {

		folder = List_Head(Folders);
		if (!folder)
			break;

		List_Remove(Folders, folder);

		if (folder->pat)
			Pattern_Free(folder->pat);
		else
			Mem_Free(folder->buf, folder->buf_len);

		Mem_Free(folder, sizeof(FORCE_ENTRY));
	}
}


//---------------------------------------------------------------------------
// Process_DeleteForceDataProcesses
//---------------------------------------------------------------------------


_FX void Process_DeleteForceDataProcesses(LIST* Folders)
{
    FORCE_PROCESS *process;

    while (1) {

        process = List_Head(Folders);
        if (! process)
            break;

        List_Remove(Folders, process);

        Mem_Free(process, sizeof(FORCE_PROCESS));
    }
}


//---------------------------------------------------------------------------
// Process_DeleteForceData
//---------------------------------------------------------------------------


_FX void Process_DeleteForceData(LIST *boxes)
{
    FORCE_BOX *box;

    while (1) {

        box = List_Head(boxes);
        if (! box)
            break;

        List_Remove(boxes, box);

        Process_DeleteForceDataFolders(&box->ForceFolder);
        //Process_DeleteForceDataProcesses(&box->ForceProcess);
        Process_DeleteForceDataFolders(&box->ForceProcess);
        //Process_DeleteForceDataProcesses(&box->ForceChildren);
        Process_DeleteForceDataFolders(&box->ForceChildren);
        Process_DeleteForceDataFolders(&box->AlertFolder);
        //Process_DeleteForceDataProcesses(&box->AlertProcess);
        Process_DeleteForceDataFolders(&box->AlertProcess);
        Process_DeleteForceDataProcesses(&box->HostInjectProcess);

        Box_Free(box->box);

        Mem_Free(box, sizeof(FORCE_BOX));
    }
}


//---------------------------------------------------------------------------
// Process_CheckBoxPath
//---------------------------------------------------------------------------


_FX BOX *Process_CheckBoxPath(LIST *boxes, const WCHAR *path)
{
    UNICODE_STRING uni;
    FORCE_BOX *box;

    RtlInitUnicodeString(&uni, path);

    box = List_Head(boxes);
    while (box) {

        if (Box_IsBoxedPath(box->box, file, &uni))
            return box->box;

        box = List_Next(box);
    }

    return NULL;
}


//---------------------------------------------------------------------------
// Process_CheckForceFolder
//---------------------------------------------------------------------------


_FX BOOLEAN Process_CheckForceFolderList(
    BOX *box, LIST* ForceFolder, ULONG prefix_len, const WCHAR *path)
{
    ULONG path_lwr_len = 0;
    WCHAR *path_lwr = NULL;

    FORCE_ENTRY *folder = List_Head(ForceFolder);
    while (folder) {

        BOOLEAN match = FALSE;

        if (folder->pat) {

            //
            // wildcards in ForceFolder:  match using pattern
            //

            if (! path_lwr) {
                path_lwr = Mem_AllocString(Driver_Pool, path);
                if (path_lwr) {
                    path_lwr[prefix_len] = L'\0';
                    _wcslwr(path_lwr);
                    path_lwr_len = wcslen(path_lwr);
                }
            }

            if (path_lwr) {
                match = Pattern_Match(
                                    folder->pat, path_lwr, path_lwr_len);
            }

        } else {

            //
            // no wildcards:  match using nls-aware string comparison
            //

            ULONG folder_len = folder->len;
            if (folder_len && prefix_len >= folder_len &&
                    path[folder_len] == L'\\' &&
                    Box_NlsStrCmp(path, folder->buf, folder_len) == 0) {

                match = TRUE;
            }
        }

        if (match) {

            break;
        }

        folder = List_Next(folder);
    }

    if (path_lwr)
        Mem_FreeString(path_lwr);

    if (folder) // found
        return TRUE;
    return FALSE;
}


//---------------------------------------------------------------------------
// Process_CheckForceFolder
//---------------------------------------------------------------------------


_FX BOX *Process_CheckForceFolder(
    LIST *boxes, const WCHAR *path, BOOLEAN alert, ULONG *IsAlert)
{
    const WCHAR *ptr;
    ULONG prefix_len;
    FORCE_BOX *box;

    //
    // make sure we have a proper path
    //

    ptr = wcsrchr(path, L'\\');
    if (ptr && ptr[1])
        prefix_len = (ULONG)(ptr - path);
    else
        prefix_len = 0;

    if (! prefix_len)
        return NULL;

    //
    // never force a program from the Sandboxie home directory
    //

    if (wcslen(path) > Driver_HomePathNt_Len + 1
        && _wcsnicmp(path, Driver_HomePathNt, Driver_HomePathNt_Len) == 0
        && path[Driver_HomePathNt_Len] == L'\\') {

        *IsAlert = 2;
        return NULL;
    }

    //
    // check if the folder is forced to any box
    //

    box = List_Head(boxes);
    while (box) {

        if (Process_CheckForceFolderList(box->box, &box->ForceFolder, prefix_len, path)) {

            if (alert) {
                *IsAlert = 1;
                return NULL;
            }

            return box->box;
        }

        box = List_Next(box);
    }

    return NULL;
}


//---------------------------------------------------------------------------
// Process_CheckForceProcessList
//---------------------------------------------------------------------------


_FX BOOLEAN Process_CheckForceProcessList(
    BOX *box, LIST* ForceProcess, const WCHAR* name, const WCHAR* path)
{
    //FORCE_PROCESS *process = List_Head(ForceProcess);
    //while (process) {

    //    const WCHAR *value = process->value;
    //    if (Process_MatchImage(box, value, 0, name, 1)) {

    //        return TRUE;
    //    }

    //    process = List_Next(process);
    //}

    ULONG path_lwr_len = 0;
    WCHAR *path_lwr = NULL;

    FORCE_ENTRY *folder = List_Head(ForceProcess);
    while (folder) {

        BOOLEAN match = FALSE;

        if (folder->pat) {

            //
            // wildcards in ForceProcess:  match using pattern
            //

            if (! path_lwr) {
                path_lwr = Mem_AllocString(Driver_Pool, path);
                if (path_lwr) {
                    _wcslwr(path_lwr);
                    path_lwr_len = wcslen(path_lwr);
                }
            }

            if (path_lwr) {
                match = Pattern_Match(
                                    folder->pat, path_lwr, path_lwr_len);
            }

        } else {

            ULONG folder_len = folder->len;
            if (Process_MatchImage(box, folder->buf, 0, name, 1)) {

                match = TRUE;
            }
        }

        if (match) {

            break;
        }

        folder = List_Next(folder);
    }

    if (path_lwr)
        Mem_FreeString(path_lwr);

    if (folder) // found
        return TRUE;
    return FALSE;
}


//---------------------------------------------------------------------------
// Process_CheckForceProcess
//---------------------------------------------------------------------------


_FX BOX *Process_CheckForceProcess(
    LIST *boxes, const WCHAR *name, const WCHAR* path, BOOLEAN alert, ULONG *IsAlert, const WCHAR *ParentName, const WCHAR *ParentPath)
{
    FORCE_BOX *box;

    //
    // never force a program from the Sandboxie home directory
    //

    if (wcslen(path) > Driver_HomePathNt_Len + 1
        && _wcsnicmp(path, Driver_HomePathNt, Driver_HomePathNt_Len) == 0
        && path[Driver_HomePathNt_Len] == L'\\') {

        *IsAlert = 2;
        return NULL;
    }

    //
    // check if the process name is forced to any box
    //

    box = List_Head(boxes);
    while (box) {

        if (Process_CheckForceProcessList(box->box, &box->ForceProcess, name, path)) {
            if (alert) {
                *IsAlert = 1;
                return NULL;
            }

            return box->box;
        }

        if (ParentName && Process_CheckForceProcessList(box->box, &box->ForceChildren, ParentName, ParentPath) && _wcsicmp(name, L"Sandman.exe") != 0) { // except for sandman exe
            if (alert) {
                *IsAlert = 1;
                return NULL;
            }

            return box->box;
        }

        //if (Process_IsWindowsExplorerParent(ParentId) && Conf_Get_Boolean(box->box->name, L"ForceExplorerChild", 0, FALSE)) {
        //    if (_wcsicmp(name, L"Sandman.exe") != 0)
        //        return box->box;
        //}

        box = List_Next(box);
    }

    return NULL;
}


//---------------------------------------------------------------------------
// Process_CheckAlertFolder
//---------------------------------------------------------------------------


_FX void Process_CheckAlertFolder(
    LIST *boxes, const WCHAR *path, ULONG *IsAlert)
{
    const WCHAR *ptr;
    ULONG prefix_len;
    FORCE_BOX *box;

    //
    // make sure we have a proper path
    //

    ptr = wcsrchr(path, L'\\');
    if (ptr && ptr[1])
        prefix_len = (ULONG)(ptr - path);
    else
        prefix_len = 0;

    if (! prefix_len)
        return;

    //
    // check if the folder is alerted to any box
    //

    box = List_Head(boxes);
    while (box) {

        if (Process_CheckForceFolderList(box->box, &box->AlertFolder, prefix_len, path)) {

            *IsAlert = 1;
			return;
        }

        box = List_Next(box);
    }
}


//---------------------------------------------------------------------------
// Process_CheckAlertProcess
//---------------------------------------------------------------------------


static _FX void Process_CheckAlertProcess(
    LIST *boxes, const WCHAR *name, const WCHAR* path, ULONG *IsAlert)
{
    FORCE_BOX *box;

    //
    // check if the process name has an alert in any box
    //

    box = List_Head(boxes);
    while (box) {

        if (Process_CheckForceProcessList(box->box, &box->AlertProcess, name, path)) {
            *IsAlert = 1;
            return;
        }

        box = List_Next(box);
    }
}

_FX BOX *Process_CheckHostInjectProcess(
    LIST *boxes, const WCHAR *name)
{
    FORCE_BOX *box;

    //
    // check if the process name has an alert in any box
    //

    box = List_Head(boxes);
    while (box) {

        FORCE_PROCESS *process = List_Head(&box->HostInjectProcess);
        while (process) {

            const WCHAR *value = process->value;
            // format: HostInjectProcess=<pgm name>|<service name>
            const WCHAR *image_end = wcschr(value, '|');
            ULONG nImageLen = 0;
            if (image_end)
                nImageLen = (ULONG)(image_end - value);
            if (Process_MatchImage(box->box, value, nImageLen, name, 1)) {

                return box->box;
            }

            process = List_Next(process);
        }

        box = List_Next(box);
    }

    return NULL;
}


//---------------------------------------------------------------------------
// Process_IsBreakoutProcess
//---------------------------------------------------------------------------

#ifdef DRV_BREAKOUT
_FX BOOLEAN Process_IsBreakoutProcess(
    BOX *box, const WCHAR *ImagePath)
{
    NTSTATUS status;
    LIST BreakoutFolder;
    LIST BreakoutProcess;
    WCHAR *ImagePath2 = L"";
    ULONG ImagePath2_len;
    const WCHAR *ImageName = L"";
    BOOLEAN IsBreakout = FALSE;

    //
    // get adjusted image path and image name
    //

    status = Process_TranslateDosToNt(
                                ImagePath, &ImagePath2, &ImagePath2_len);

    if (NT_SUCCESS(status)) {

        ImageName = wcsrchr(ImagePath2, L'\\');
        if (ImageName && ImageName[1])
            ++ImageName;
        else
            status = STATUS_OBJECT_PATH_SYNTAX_BAD;     // random
    }

    if (!NT_SUCCESS(status)) {
        return FALSE;
    }

    //
    // never break out a program from the Sandboxie home directory
    //

    if (wcslen(ImagePath2) > Driver_HomePathNt_Len + 1
        && _wcsnicmp(ImagePath2, Driver_HomePathNt, Driver_HomePathNt_Len) == 0
        && ImagePath2[Driver_HomePathNt_Len] == L'\\') {

        goto finish;
    }

    //
    // init breakout presets
    //

    List_Init(&BreakoutFolder);
    List_Init(&BreakoutProcess);

    Conf_AdjustUseCount(TRUE);

    Process_AddForceFolders(&BreakoutFolder, L"BreakoutFolder", box, box->name);

    Process_AddForceProcesses(&BreakoutProcess, L"BreakoutProcess", box->name);
        
    Conf_AdjustUseCount(FALSE);

    IsBreakout = Process_CheckForceProcessList(box, &BreakoutProcess, ImageName);
    if (!IsBreakout) {
        const WCHAR *ptr;
        ULONG prefix_len;

        ptr = wcsrchr(ImagePath2, L'\\');
        if (ptr && ptr[1])
            prefix_len = (ULONG)(ptr - ImagePath2);
        else
            prefix_len = 0;

        if (prefix_len > 0)
            IsBreakout = Process_CheckForceFolderList(box, &BreakoutFolder, prefix_len, ImagePath2);
    }

    Process_DeleteForceDataFolders(&BreakoutFolder);
    Process_DeleteForceDataProcesses(&BreakoutProcess);

finish:
    Mem_Free(ImagePath2, ImagePath2_len);

    return IsBreakout;
}
#endif

//---------------------------------------------------------------------------
// Process_DfpInsert
//---------------------------------------------------------------------------


_FX BOOLEAN Process_DfpInsert(HANDLE ParentId, HANDLE ProcessId)
{
    FORCE_PROCESS_2 *proc;
    KIRQL irql;
    BOOLEAN added;

    if (ParentId == PROCESS_TERMINATED) {

        //
        // called by Session_Api_DisableForce, process list not locked
        //

        KeRaiseIrql(APC_LEVEL, &irql);
        ExAcquireResourceExclusiveLite(Process_ListLock, TRUE);

        Process_DfpDelete(ProcessId);

        proc = Mem_Alloc(Driver_Pool, sizeof(FORCE_PROCESS_2));
        proc->pid = ProcessId;
        proc->silent = FALSE;

#ifdef USE_PROCESS_MAP
        map_insert(&Process_MapDfp, ProcessId, proc, 0);
#else
        List_Insert_After(&Process_ListDfp, NULL, proc);
#endif

        ExReleaseResourceLite(Process_ListLock);
        KeLowerIrql(irql);

        added = TRUE;

    } else {

        //
        // called by Process_NotifyProcess_Create, process list locked
        //

        Process_DfpDelete(ProcessId);

        added = FALSE;

#ifdef USE_PROCESS_MAP
        proc = map_get(&Process_MapDfp, ParentId);
        if (proc) {
#else
        proc = List_Head(&Process_ListDfp);
        while (proc) {

            if (proc->pid == ParentId) {
#endif

                proc = Mem_Alloc(Driver_Pool, sizeof(FORCE_PROCESS_2));
                proc->pid = ProcessId;
                proc->silent = FALSE;

#ifdef USE_PROCESS_MAP
                map_insert(&Process_MapDfp, ProcessId, proc, 0);
#else
                List_Insert_After(&Process_ListDfp, NULL, proc);
#endif

                added = TRUE;

#ifndef USE_PROCESS_MAP
                break;
            }

            proc = List_Next(proc);
#endif
        }
    }

    return added;
}


//---------------------------------------------------------------------------
// Process_DfpDelete
//---------------------------------------------------------------------------


_FX void Process_DfpDelete(HANDLE ProcessId)
{
    FORCE_PROCESS_2 *proc;

#ifdef USE_PROCESS_MAP
    if(map_take(&Process_MapDfp, ProcessId, &proc, 0))
        Mem_Free(proc, sizeof(FORCE_PROCESS_2));
#else
    proc = List_Head(&Process_ListDfp);
    while (proc) {

        if (proc->pid == ProcessId) {

            List_Remove(&Process_ListDfp, proc);

            Mem_Free(proc, sizeof(FORCE_PROCESS_2));

            return;
        }

        proc = List_Next(proc);
    }
#endif
}


//---------------------------------------------------------------------------
// Process_DfpCheck
//---------------------------------------------------------------------------


_FX BOOLEAN Process_DfpCheck(HANDLE ProcessId, BOOLEAN *silent)
{
    FORCE_PROCESS_2 *proc;
    KIRQL irql;
    BOOLEAN found = FALSE;

    KeRaiseIrql(APC_LEVEL, &irql);
    ExAcquireResourceExclusiveLite(Process_ListLock, TRUE);

#ifdef USE_PROCESS_MAP
    proc = map_get(&Process_MapDfp, ProcessId);
    if (proc) {
#else
    proc = List_Head(&Process_ListDfp);
    while (proc) {

        if (proc->pid == ProcessId) {
#endif

            if (*silent)
                proc->silent = TRUE;
            else
                *silent = proc->silent;

            found = TRUE;
#ifndef USE_PROCESS_MAP
            break;
        }

        proc = List_Next(proc);
#endif
    }

    ExReleaseResourceLite(Process_ListLock);
    KeLowerIrql(irql);

    return found;
}


//---------------------------------------------------------------------------
// Process_FcpInsert
//---------------------------------------------------------------------------


_FX VOID Process_FcpInsert(HANDLE ProcessId, const WCHAR* boxname)
{
    FORCE_PROCESS_3 *proc;
    KIRQL irql;

    //
    // called by Session_Api_ForceChildren, process list not locked
    //

    KeRaiseIrql(APC_LEVEL, &irql);
    ExAcquireResourceExclusiveLite(Process_ListLock, TRUE);

    Process_FcpDelete(ProcessId);

    proc = Mem_Alloc(Driver_Pool, sizeof(FORCE_PROCESS_3));
    proc->pid = ProcessId;
    wmemcpy(proc->boxname, boxname, BOXNAME_COUNT);

#ifdef USE_PROCESS_MAP
    map_insert(&Process_MapFcp, ProcessId, proc, 0);
#else
    List_Insert_After(&Process_ListFcp, NULL, proc);
#endif

    ExReleaseResourceLite(Process_ListLock);
    KeLowerIrql(irql);


}


//---------------------------------------------------------------------------
// Process_FcpDelete
//---------------------------------------------------------------------------


_FX void Process_FcpDelete(HANDLE ProcessId)
{
    FORCE_PROCESS_3 *proc;

#ifdef USE_PROCESS_MAP
    if(map_take(&Process_MapFcp, ProcessId, &proc, 0))
        Mem_Free(proc, sizeof(FORCE_PROCESS_3));
#else
    proc = List_Head(&Process_ListFcp);
    while (proc) {

        if (proc->pid == ProcessId) {

            List_Remove(&Process_ListFcp, proc);

            Mem_Free(proc, sizeof(FORCE_PROCESS_3));

            return;
        }

        proc = List_Next(proc);
    }
#endif
}


//---------------------------------------------------------------------------
// Process_FcpCheck
//---------------------------------------------------------------------------


_FX BOOLEAN Process_FcpCheck(HANDLE ProcessId, WCHAR* boxname)
{
    FORCE_PROCESS_3 *proc;
    KIRQL irql;
    BOOLEAN found = FALSE;

    KeRaiseIrql(APC_LEVEL, &irql);
    ExAcquireResourceExclusiveLite(Process_ListLock, TRUE);

#ifdef USE_PROCESS_MAP
    proc = map_get(&Process_MapFcp, ProcessId);
    if (proc) {
#else
    proc = List_Head(&Process_ListFcp);
    while (proc) {

        if (proc->pid == ProcessId) {
#endif
            if(boxname)
                wmemcpy(boxname, proc->boxname, BOXNAME_COUNT);

            found = TRUE;
#ifndef USE_PROCESS_MAP
            break;
        }

        proc = List_Next(proc);
#endif
    }

    ExReleaseResourceLite(Process_ListLock);
    KeLowerIrql(irql);

    return found;
}


//---------------------------------------------------------------------------
// Process_FcpCheck
//---------------------------------------------------------------------------


_FX BOOLEAN Process_CheckMoTW(const WCHAR *path)
{
    NTSTATUS status;
    UNICODE_STRING uni;
    OBJECT_ATTRIBUTES objattrs;
    IO_STATUS_BLOCK MyIoStatusBlock;
    WCHAR* adsPath = NULL;
    ULONG adsPath_len = 0;
    const WCHAR ads_suffix[] = L":Zone.Identifier";
    HANDLE handle = NULL;
    //CHAR buffer[512] = {0};
    
    if (!path)
        return FALSE;

    adsPath_len = wcslen(path) * sizeof(WCHAR) + sizeof(ads_suffix);
    adsPath = Mem_Alloc(Driver_Pool, adsPath_len);
    if (!adsPath)
        return FALSE;
    wcscpy(adsPath, path);
    wcscat(adsPath, ads_suffix);
    
    RtlInitUnicodeString(&uni, adsPath);
    InitializeObjectAttributes(&objattrs,
        &uni, OBJ_CASE_INSENSITIVE, NULL, NULL);

    status = ZwCreateFile(
        &handle,
        FILE_GENERIC_READ,      // DesiredAccess
        &objattrs,
        &MyIoStatusBlock,
        NULL,                   // AllocationSize
        0,                      // FileAttributes
        FILE_SHARE_READ,        // ShareAccess
        FILE_OPEN,              // CreateDisposition
        FILE_NON_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT,
        NULL, 0);               // EaBuffer, EaLength
    if (!NT_SUCCESS(status))
        goto finish;

    /*status = ZwReadFile(
        handle,
        NULL, NULL, NULL,
        &MyIoStatusBlock,
        buffer,
        sizeof(buffer) - 1,
        NULL,
        NULL);

    if (!NT_SUCCESS(status))
        goto finish;

    buffer[MyIoStatusBlock.Information] = '\0'; // null-terminate
    DbgPrint("ADS content: %s\n", buffer);

    status = STATUS_NOT_FOUND;*/
    
finish:
    if (handle)
        ZwClose(handle);

    if (adsPath)
        Mem_Free(adsPath, adsPath_len);

    return NT_SUCCESS(status);
}