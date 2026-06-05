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
#define PROGRAM_CONTROL_RULE_NO_CRT
#define PROGRAM_CONTROL_RULE_NO_QUERY_HELPERS
#include "common/program_control_rule.h"
#undef PROGRAM_CONTROL_RULE_NO_CRT
#undef PROGRAM_CONTROL_RULE_NO_QUERY_HELPERS

static BOOLEAN Process_UseRuleExtensions(const WCHAR *boxname, const WCHAR *setting)
{
    if (!ProgramControl_IsRuleExtensionSetting(setting))
        return TRUE;

    return Conf_Get_Boolean(boxname, L"UseForceBreakoutRuleExtensions", 0, FALSE);
}

static BOOLEAN Process_AreBreakoutRulesEnabled(const WCHAR *boxname)
{
    return Conf_Get_Boolean(boxname, L"DisableBreakoutRules", 0, FALSE) ? FALSE : TRUE;
}


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
    BOOLEAN has_priority;
    LONG priority;
    BOOLEAN has_recursive;
    LONG recursive_min_depth;
    LONG recursive_depth;
    BOOLEAN recursive_anchor_from_last;

} FORCE_ENTRY;

#define MAX_FORCE_PROCESS_VALUE_LEN 1024

typedef struct _FORCE_PROCESS {

    LIST_ELEM list_elem;
    WCHAR value[MAX_FORCE_PROCESS_VALUE_LEN];
} FORCE_PROCESS;


typedef struct _FORCE_PROCESS_2 {

    HANDLE pid;
    BOOLEAN silent;

} FORCE_PROCESS_2;


typedef struct _FORCE_PROCESS_3 {

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
    LIST *boxes, const WCHAR *path, BOOLEAN alert, ULONG *IsAlert,
    const WCHAR *prioritizeName, const WCHAR *prioritizePath,
    HANDLE ProcessId);

static BOOLEAN Process_CheckForceProcessList(
    BOX *box, LIST* ForceProcess, const WCHAR* name, const WCHAR* path,
    BOOLEAN *outHasPriority, LONG *outPriority);

static BOOLEAN Process_CheckForceFolderList(
    BOX *box, LIST* ForceFolder, ULONG prefix_len, const WCHAR *path,
    BOOLEAN *outHasPriority, LONG *outPriority);

static BOOLEAN Process_CheckBreakoutProcessList(
    BOX *box, LIST* BreakoutProcess, const WCHAR* name, const WCHAR* path);

static BOOLEAN Process_IsSelfTargetedBreakoutMatch(
    BOX *box, const WCHAR *processName, const WCHAR *folderScopeName, const WCHAR *path);

static FORCE_BOX* Process_FindForceBoxByName(
    LIST *boxes, const WCHAR *boxname);

static BOOLEAN Process_GetMatchedBreakoutTarget(
    BOX *box, const WCHAR *processName, const WCHAR *folderScopeName, const WCHAR *path, WCHAR *outTarget, ULONG outTargetCch);

static BOOLEAN Process_MatchBreakoutProcessRuleRaw(
    BOX *box, const WCHAR *rule, const WCHAR *name, const WCHAR *path);

static BOOLEAN Process_MatchBreakoutFolderRuleRaw(
    BOX *box, const SBIE_NORMALIZED_RULE *normalized, const WCHAR *name, const WCHAR *path);

static BOOLEAN Process_FindMatchingBreakoutFolderRule(
    BOX *box, const WCHAR *name, const WCHAR *path, BOOLEAN requireTarget,
    WCHAR *outTarget, ULONG outTargetCch);

static int Process_BreakoutMatchImage(const WCHAR *pattern, const WCHAR *imageName, void *context);

static WCHAR* Process_NormalizeBreakoutRulePath(
    BOX *box, const WCHAR *rule, const WCHAR *setting, ULONG *out_len);

static BOX *Process_CheckForceProcess(
    LIST *boxes, const WCHAR *name, const WCHAR* path, const WCHAR *docPath, BOOLEAN alert, ULONG *IsAlert, const WCHAR *ParentName, const WCHAR *ParentPath, BOOLEAN *pForcedByChildren, HANDLE ProcessId);

static BOOLEAN Process_GetBreakoutDocumentPriority(
    BOX *box, const WCHAR *scopeName, const WCHAR *docPath,
    BOOLEAN *outHasPriority, LONG *outPriority);

static BOOLEAN Process_GetBreakoutDocumentTarget(
    BOX *box, const WCHAR *scopeName, const WCHAR *docPath,
    WCHAR *outTarget, ULONG outTargetCch,
    BOOLEAN *outHasTarget, BOOLEAN *outHasPriority, LONG *outPriority,
    ULONG *outLevel);

static void Process_GetBreakoutDocumentPriorityBest(
    BOX *box,
    const WCHAR *primaryScopeName,
    const WCHAR *secondaryScopeName,
    const WCHAR *docPath,
    BOOLEAN *outMatched,
    BOOLEAN *outHasPriority,
    LONG *outPriority);

static BOOLEAN Process_GetBreakoutDocumentTargetBest(
    BOX *box,
    const WCHAR *primaryScopeName,
    const WCHAR *secondaryScopeName,
    const WCHAR *docPath,
    WCHAR *outTarget,
    ULONG outTargetCch);

static BOOLEAN Process_IsPrioritizedBreakoutMatch(
    BOX *box, const WCHAR *processName, const WCHAR *folderScopeName, const WCHAR *path);

static BOOLEAN Process_GetMatchedBreakoutPriority(
    BOX *box, const WCHAR *processName, const WCHAR *folderScopeName, const WCHAR *path,
    BOOLEAN *outHasPriority, LONG *outPriority);

/*
 * These breakout helpers stay driver-local. They operate on BOX/LIST/FORCE_ENTRY
 * state in kernel memory and are reused by multiple force-capture entry points.
 * Keep only pure parsing and arbitration helpers in program_control_rule.h.
 */

static void Process_CheckAlertFolder(
	LIST *boxes, const WCHAR *path, ULONG *IsAlert);

static void Process_CheckAlertProcess(
    LIST *boxes, const WCHAR *name, const WCHAR *path, ULONG *IsAlert);

static BOX *Process_CheckHostInjectProcess(
    LIST *boxes, const WCHAR *name);

static BOOLEAN Process_CheckMoTW(const WCHAR *path);

static BOOLEAN Process_GetSettingsForImageName_Bool(
    BOX *box, const WCHAR *imageName, const WCHAR *setting, BOOLEAN defval);

//---------------------------------------------------------------------------
// Process_GetForcedStartBox
//---------------------------------------------------------------------------

_FX BOX *Process_GetForcedStartBox(
    HANDLE ProcessId, HANDLE ParentId, const WCHAR *ImagePath, BOOLEAN* pHostInject, const WCHAR *pSidString, BOOLEAN *pForcedByChildren)
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
    if (pForcedByChildren)
        *pForcedByChildren = FALSE;

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

        if ((! box) && DocArg && !is_start_exe && Conf_Get_Boolean(NULL, L"ForceBoxDocs", 0, FALSE))
            box = Process_CheckBoxPath(&boxes, DocArg);

        if (!box) {

            box = Process_CheckForceFolder(
                        &boxes, ImagePath2, force_alert, &alert, ParentName ? ParentName : ImageName, ImagePath2, ProcessId);
            if ((! box) && (! alert)) {
                box = Process_CheckForceProcess(
                    &boxes, ImageName, ImagePath2, DocArg, force_alert, &alert, ParentName, ParentPath, pForcedByChildren, ProcessId);
            }

            if ((! box) && CurDir && !is_start_exe && (! alert)) {
                box = Process_CheckForceFolder(
                    &boxes, CurDir, force_alert, &alert, ParentName ? ParentName : ImageName, ImagePath2, ProcessId);
            }

            if ((! box) && DocArg && !is_start_exe && (! alert)) {
                box = Process_CheckForceFolder(
                    &boxes, DocArg, force_alert, &alert, ParentName ? ParentName : ImageName, ImagePath2, ProcessId);
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
                        if (pForcedByChildren)
                            *pForcedByChildren = TRUE;
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
    

        //
        // check mark of the web (MoTW)
        //

        if (! box && (alert != 1)) {

            if (Conf_Get_Boolean(NULL, L"ForceMarkOfTheWeb", 0, FALSE)) {

                //
                // Check the program and check the passed document (except for start.exe)
                //

                if (Process_CheckMoTW(ImagePath2) || (!(is_start_exe || _wcsicmp(ImageName, L"sandman.exe") == 0) && DocArg && Process_CheckMoTW(DocArg))) {

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
            WCHAR *doc_end;
            WCHAR saved_end = 0;

            while (len && *ptr == L' ') {
                --len;
                ++ptr;
            }

            if (len && *ptr == L'"') {
                --len;
                ++ptr;

                doc = ptr;
                doc_end = ptr;
                while (len && *doc_end != L'"') {
                    --len;
                    ++doc_end;
                }
            } else {
                doc = ptr;
                doc_end = ptr;
                while (len && *doc_end != L' ' && *doc_end != L'\t') {
                    --len;
                    ++doc_end;
                }
            }

            saved_end = *doc_end;
            *doc_end = L'\0';

            // Strip trailing separators from the extracted token only.
            while (*doc) {
                SIZE_T doc_len = wcslen(doc);
                if (!doc_len)
                    break;
                if (doc[doc_len - 1] != L'\\' && doc[doc_len - 1] != L' ' && doc[doc_len - 1] != L'"')
                    break;
                doc[doc_len - 1] = L'\0';
            }

            if (*doc) {

                NTSTATUS status = Process_TranslateDosToNt(doc, pDocArg, pDocArgLen);
                if (! NT_SUCCESS(status)) {

                    *pDocArg = NULL;
                    *pDocArgLen = 0;
                }
            }

            *doc_end = saved_end;
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
    const BOOLEAN breakout_setting = (_wcsicmp(Setting, L"BreakoutFolder") == 0 || _wcsicmp(Setting, L"BreakoutProcess") == 0);
    const BOOLEAN image_wildcard_setting =
        (_wcsicmp(Setting, L"ForceProcess") == 0 ||
         _wcsicmp(Setting, L"ForceChildren") == 0 ||
         _wcsicmp(Setting, L"AlertProcess") == 0);
    const BOOLEAN use_rule_extensions = Process_UseRuleExtensions(section, Setting);

    index2 = 0;

    while (1) {

        WCHAR *expnd, *buf;
        ULONG buf_len;
        WCHAR *value_copy;
        WCHAR *value_norm;
        ULONG value_norm_len;
        SBIE_NORMALIZED_RULE normalized;

        value = Conf_Get(section, Setting, index2);
        if (! value)
            break;
        ++index2;

        value_copy = Mem_AllocString(Driver_Pool, value);
        if (!value_copy)
            continue;

        if (!ProgramControl_ParseRuleExtensionsInPlace(value_copy, &normalized, use_rule_extensions)) {
            Mem_FreeString(value_copy);
            continue;
        }

        value = normalized.base_rule;

        if (breakout_setting) {

            // Accept drive-relative wildcard forms like "C:*\firefox.exe"
            // by normalizing to rooted DOS form "C:\*\firefox.exe".
            value_norm = NULL;
            value_norm_len = 0;
            if (value[0] && value[1] == L':' && value[2] && value[2] != L'\\' && value[2] != L'/') {
                ULONG value_len = (ULONG)wcslen(value);
                value_norm_len = (value_len + 2) * sizeof(WCHAR);
                value_norm = Mem_Alloc(Driver_Pool, value_norm_len);
                if (value_norm) {
                    value_norm[0] = value[0];
                    value_norm[1] = L':';
                    value_norm[2] = L'\\';
                    wcscpy(value_norm + 3, value + 2);
                    value = value_norm;
                }
            }
        } else {
            value_norm = NULL;
            value_norm_len = 0;
        }

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

        if (value_copy)
            Mem_FreeString(value_copy);
        if (value_norm)
            Mem_Free(value_norm, value_norm_len);

        if (! buf)
            continue;

        folder = Mem_Alloc(Driver_Pool, sizeof(FORCE_ENTRY));
        if (! folder) {
            Mem_Free(buf, buf_len);
            break;
        }

        folder->has_priority = normalized.has_priority ? TRUE : FALSE;
        folder->priority = normalized.has_priority ? normalized.priority : -1;
        folder->has_recursive = normalized.has_recursive ? TRUE : FALSE;
        folder->recursive_min_depth = normalized.has_recursive ? normalized.recursive_min_depth : 0;
        folder->recursive_depth = normalized.has_recursive ? normalized.recursive_depth : -1;
        folder->recursive_anchor_from_last = normalized.has_recursive ? (normalized.recursive_anchor_from_last ? TRUE : FALSE) : TRUE;

        if (wcschr(buf, L'*') || wcschr(buf, L'?')) {

            folder->pat =
                Pattern_Create(box->expand_args->pool, buf, TRUE, 0);

            if (! folder->pat) {
                Mem_Free(buf, buf_len);
                Mem_Free(folder, sizeof(FORCE_ENTRY));
                break;
            }

            // Keep wildcard rule text so shared folder/process match helpers can
            // apply recursive and path-shape semantics consistently.
            folder->buf_len = buf_len;
            folder->len = wcslen(buf);
            folder->buf = buf;

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

        if (folder->pat) {
			Pattern_Free(folder->pat);
            if (folder->buf)
                Mem_Free(folder->buf, folder->buf_len);
        } else
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
    BOX *box, LIST* ForceFolder, ULONG prefix_len, const WCHAR *path,
    BOOLEAN *outHasPriority, LONG *outPriority)
{
    ULONG path_lwr_len = 0;
    WCHAR *path_lwr = NULL;
    BOOLEAN matched = FALSE;
    BOOLEAN hasPriority = FALSE;
    LONG bestPriority = -1;

    if (outHasPriority)
        *outHasPriority = FALSE;
    if (outPriority)
        *outPriority = -1;

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
                match = Pattern_Match(folder->pat, path_lwr, path_lwr_len);

                if (match && folder->has_recursive && folder->buf) {
                    size_t base_len = 0;
                    if (ProgramControl_FindWildcardAnchorBaseLen(
                            folder->buf,
                            folder->recursive_anchor_from_last ? 1 : 0,
                            path_lwr,
                            path_lwr_len,
                            &base_len)) {
                        if (base_len < path_lwr_len) {
                            const WCHAR *dir_begin = path_lwr + base_len;
                            const WCHAR *dir_end = path_lwr + path_lwr_len;
                            LONG depth = 0;

                            if (*dir_begin == L'\\')
                                ++dir_begin;

                            if (dir_begin < dir_end) {
                                const WCHAR *p = dir_begin;
                                depth = 1;
                                while (p < dir_end) {
                                    if (*p == L'\\')
                                        ++depth;
                                    ++p;
                                }
                            }

                            if (depth < folder->recursive_min_depth ||
                                    (folder->recursive_depth >= 0 && depth > folder->recursive_depth))
                                match = FALSE;
                        }
                    }
                }
            }

        } else {

            //
            // no wildcards:  match using nls-aware string comparison
            //

            ULONG folder_len = folder->len;
            if (folder_len && prefix_len >= folder_len &&
                    path[folder_len] == L'\\' &&
                    Box_NlsStrCmp(path, folder->buf, folder_len) == 0) {
                if (folder->has_recursive) {
                    const WCHAR *dir_begin = path + folder_len;
                    const WCHAR *dir_end = path + prefix_len;
                    LONG depth = 0;

                    if (*dir_begin == L'\\')
                        ++dir_begin;

                    if (dir_begin < dir_end) {
                        const WCHAR *p = dir_begin;
                        depth = 1;
                        while (p < dir_end) {
                            if (*p == L'\\')
                                ++depth;
                            ++p;
                        }
                    }

                    if (depth >= folder->recursive_min_depth && (folder->recursive_depth < 0 || depth <= folder->recursive_depth))
                        match = TRUE;
                }
                else {
                    match = TRUE;
                }
            }
        }

        if (match) {
            matched = TRUE;

            if (folder->has_priority && (!hasPriority || folder->priority < bestPriority)) {
                hasPriority = TRUE;
                bestPriority = folder->priority;
            }

            if (!outHasPriority && !outPriority)
                break;
        }

        folder = List_Next(folder);
    }

    if (path_lwr)
        Mem_FreeString(path_lwr);

    if (hasPriority) {
        if (outHasPriority)
            *outHasPriority = TRUE;
        if (outPriority)
            *outPriority = bestPriority;
    }

    return matched;
}


//---------------------------------------------------------------------------
// Process_IsPrioritizedBreakoutMatch
//---------------------------------------------------------------------------


_FX BOOLEAN Process_IsPrioritizedBreakoutMatch(
    BOX *box, const WCHAR *processName, const WCHAR *folderScopeName, const WCHAR *path)
{
    BOOLEAN breakout_match = FALSE;
    LIST BreakoutProcess;

    if (!box || !Process_AreBreakoutRulesEnabled(box->name))
        return FALSE;

    List_Init(&BreakoutProcess);

    Conf_AdjustUseCount(TRUE);
    Process_AddForceFolders(&BreakoutProcess, L"BreakoutProcess", box, box->name);
    Conf_AdjustUseCount(FALSE);

    if (Process_CheckBreakoutProcessList(box, &BreakoutProcess, processName, path)) {
        breakout_match = TRUE;
        goto finish;
    }

    if (Process_FindMatchingBreakoutFolderRule(box, (folderScopeName && *folderScopeName) ? folderScopeName : processName, path, FALSE, NULL, 0))
        breakout_match = TRUE;

    // If the matching breakout rule explicitly targets this same box
    // (rule|ThisBox), do not suppress this box's force capture.
    if (breakout_match && Process_IsSelfTargetedBreakoutMatch(box, processName, folderScopeName, path))
        breakout_match = FALSE;

finish:
    Process_DeleteForceDataFolders(&BreakoutProcess);

    return breakout_match;
}

static BOOLEAN Process_GetMatchedBreakoutPriority(
    BOX *box, const WCHAR *processName, const WCHAR *folderScopeName, const WCHAR *path,
    BOOLEAN *outHasPriority, LONG *outPriority)
{
    const WCHAR *scopeName;
    const WCHAR *value;
    ULONG index;
    BOOLEAN matched = FALSE;
    BOOLEAN hasPriority = FALSE;
    LONG bestPriority = -1;
    BOOLEAN use_breakout_process_extensions;
    BOOLEAN use_breakout_folder_extensions;

    if (outHasPriority)
        *outHasPriority = FALSE;
    if (outPriority)
        *outPriority = -1;

    if (!box || !processName || !*processName || !path || !*path)
        return FALSE;

    if (!Process_AreBreakoutRulesEnabled(box->name))
        return FALSE;

    use_breakout_process_extensions = Process_UseRuleExtensions(box->name, L"BreakoutProcess");
    use_breakout_folder_extensions = Process_UseRuleExtensions(box->name, L"BreakoutFolder");

    scopeName = (folderScopeName && *folderScopeName) ? folderScopeName : processName;

    index = 0;
    while (1) {
        WCHAR *ruleCopy;
        WCHAR *rule;
        SBIE_NORMALIZED_RULE normalized;

        value = Conf_Get(box->name, L"BreakoutProcess", index);
        if (!value)
            break;
        ++index;

        ruleCopy = Mem_AllocString(Driver_Pool, value);
        if (!ruleCopy)
            continue;

        rule = ProgramControl_MatchImageScopeAndGetValue(ruleCopy, processName, Process_BreakoutMatchImage, box);
        if (!rule) {
            Mem_FreeString(ruleCopy);
            continue;
        }

        if (!ProgramControl_ParseRuleExtensionsInPlace(rule, &normalized, use_breakout_process_extensions)) {
            Mem_FreeString(ruleCopy);
            continue;
        }

        if ((wcschr(normalized.base_rule, L'*') || wcschr(normalized.base_rule, L'?')) &&
            !ProgramControl_RuleLooksLikePath(normalized.base_rule) &&
            ProgramControl_IsBroadWildcardImageRule(normalized.base_rule)) {
            Mem_FreeString(ruleCopy);
            continue;
        }

        if (Process_MatchBreakoutProcessRuleRaw(box, normalized.base_rule, processName, path)) {
            matched = TRUE;
            if (normalized.has_priority && (!hasPriority || normalized.priority < bestPriority)) {
                hasPriority = TRUE;
                bestPriority = normalized.priority;
            }
        }

        Mem_FreeString(ruleCopy);
    }

    index = 0;
    while (1) {
        WCHAR *ruleCopy;
        WCHAR *rule;
        SBIE_NORMALIZED_RULE normalized;

        value = Conf_Get(box->name, L"BreakoutFolder", index);
        if (!value)
            break;
        ++index;

        ruleCopy = Mem_AllocString(Driver_Pool, value);
        if (!ruleCopy)
            continue;

        rule = ProgramControl_MatchImageScopeAndGetValue(ruleCopy, scopeName, Process_BreakoutMatchImage, box);
        if (!rule) {
            Mem_FreeString(ruleCopy);
            continue;
        }

        if (!ProgramControl_ParseRuleExtensionsInPlace(rule, &normalized, use_breakout_folder_extensions)) {
            Mem_FreeString(ruleCopy);
            continue;
        }

        if (Process_MatchBreakoutFolderRuleRaw(box, &normalized, scopeName, path)) {
            matched = TRUE;
            if (normalized.has_priority && (!hasPriority || normalized.priority < bestPriority)) {
                hasPriority = TRUE;
                bestPriority = normalized.priority;
            }
        }

        Mem_FreeString(ruleCopy);
    }

    if (hasPriority) {
        if (outHasPriority)
            *outHasPriority = TRUE;
        if (outPriority)
            *outPriority = bestPriority;
    }

    return matched;
}


//---------------------------------------------------------------------------
// Process_GetSettingsForImageName_Bool
//---------------------------------------------------------------------------


_FX BOOLEAN Process_GetSettingsForImageName_Bool(
    BOX *box, const WCHAR *imageName, const WCHAR *setting, BOOLEAN defval)
{
    ULONG index = 0;
    ULONG found_level = (ULONG)-1;
    BOOLEAN result = defval;

    while (TRUE) {

        const WCHAR *entry = Conf_Get(box->name, setting, index);
        if (!entry)
            break;

        ++index;

        const WCHAR *value = entry;
        ULONG level = 2;
        const WCHAR *comma = wcschr(value, L',');
        if (comma) {

            BOOLEAN inv = FALSE;
            BOOLEAN match;
            ULONG len;

            if (!imageName)
                continue;

            if (*value == L'!') {
                inv = TRUE;
                ++value;
            }

            len = (ULONG)(comma - value);
            if (len) {
                match = Process_MatchImage(box, value, len, imageName, 1);
                if (inv)
                    match = !match;
                if (!match)
                    continue;

                if (len == 1 && *value == L'*')
                    level = 2;
                else
                    level = inv ? 1 : 0;
            }

            value = comma + 1;
        }

        if (!*value)
            continue;

        if (level > found_level)
            continue;

        if (*value == L'y' || *value == L'Y')
            result = TRUE;
        else if (*value == L'n' || *value == L'N')
            result = FALSE;
        else
            result = defval;

        found_level = level;
    }

    return result;
}


//---------------------------------------------------------------------------
// Process_CheckForceFolder
//---------------------------------------------------------------------------


_FX BOX *Process_CheckForceFolder(
    LIST *boxes, const WCHAR *path, BOOLEAN alert, ULONG *IsAlert,
    const WCHAR *prioritizeName, const WCHAR *prioritizePath,
    HANDLE ProcessId)
{
    const WCHAR *ptr;
    const WCHAR *name;
    const WCHAR *breakout_name;
    const WCHAR *folder_scope_name;
    const WCHAR *breakout_path;
    ULONG prefix_len;
    FORCE_BOX *box;
    FORCE_BOX *force_winner = NULL;
    BOOLEAN force_winner_has_priority = FALSE;
    LONG force_winner_priority = -1;
    BOOLEAN have_force_winner = FALSE;

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

    name = ptr + 1;
    folder_scope_name = (prioritizeName && *prioritizeName) ? prioritizeName : name;
    breakout_path = (prioritizePath && *prioritizePath) ? prioritizePath : path;
    breakout_name = name;

    // If ForceFolder matched via CurDir/DocArg, keep BreakoutProcess image-name
    // matching anchored to the executable path instead of the folder/doc leaf.
    ptr = wcsrchr(breakout_path, L'\\');
    if (ptr && ptr[1])
        breakout_name = ptr + 1;

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
        BOOLEAN force_has_priority = FALSE;
        LONG force_priority = -1;

        if (Process_CheckForceFolderList(box->box, &box->ForceFolder, prefix_len, path, &force_has_priority, &force_priority)) {
            if (have_force_winner &&
                !ProgramControl_ShouldReplacePriorityWinner(
                    have_force_winner ? 1 : 0,
                    force_winner_has_priority ? 1 : 0,
                    force_winner_priority,
                    force_has_priority ? 1 : 0,
                    force_priority)) {
                box = List_Next(box);
                continue;
            }

            BOOLEAN effective_prioritize_breakout = FALSE;
            BOOLEAN breakout_has_priority = FALSE;
            LONG breakout_priority = -1;
            BOOLEAN has_target_override = FALSE;
            WCHAR target_box[BOXNAME_COUNT] = { 0 };

            Process_GetMatchedBreakoutPriority(box->box, breakout_name, folder_scope_name, breakout_path, &breakout_has_priority, &breakout_priority);

            effective_prioritize_breakout = SbiePolicy_ShouldPrioritizeBreakout(
                FALSE,
                force_has_priority ? 1 : 0,
                force_priority,
                breakout_has_priority ? 1 : 0,
                breakout_priority) ? TRUE : FALSE;

            if (effective_prioritize_breakout)
                has_target_override = Process_GetMatchedBreakoutTarget(box->box, breakout_name, folder_scope_name, breakout_path, target_box, BOXNAME_COUNT);

            if (has_target_override) {
                FORCE_BOX *target = Process_FindForceBoxByName(boxes, target_box);
                if (target) {
                    if (alert) {
                        *IsAlert = 1;
                        return NULL;
                    }
                    return target->box;
                }
            }

            if (effective_prioritize_breakout && !has_target_override && Process_IsPrioritizedBreakoutMatch(box->box, breakout_name, folder_scope_name, breakout_path)) {
                box = List_Next(box);
                continue;
            }

            if (alert) {
                *IsAlert = 1;
                return NULL;
            }

            if (ProgramControl_ShouldReplacePriorityWinner(
                    have_force_winner ? 1 : 0,
                    force_winner_has_priority ? 1 : 0,
                    force_winner_priority,
                    force_has_priority ? 1 : 0,
                    force_priority)) {
                force_winner = box;
                force_winner_has_priority = force_has_priority;
                force_winner_priority = force_priority;
                have_force_winner = TRUE;
            }

            box = List_Next(box);
            continue;
        }

        box = List_Next(box);
    }

    if (have_force_winner)
        return force_winner->box;

    return NULL;
}


//---------------------------------------------------------------------------
// Process_CheckForceProcessList
//---------------------------------------------------------------------------


_FX BOOLEAN Process_CheckForceProcessList(
	BOX* box, LIST* ForceProcess, const WCHAR* name, const WCHAR* path,
	BOOLEAN* outHasPriority, LONG* outPriority)
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
	WCHAR* path_lwr = NULL;
	BOOLEAN matched = FALSE;
	BOOLEAN hasPriority = FALSE;
	LONG bestPriority = -1;

	if (outHasPriority)
		*outHasPriority = FALSE;
	if (outPriority)
		*outPriority = -1;

	FORCE_ENTRY* folder = List_Head(ForceProcess);
	while (folder) {

		BOOLEAN match = FALSE;

		if (folder->pat) {

            if (folder->buf && !ProgramControl_RuleLooksLikePath(folder->buf)) {

				// Non-path wildcard ForceProcess rules are image-name patterns.
				if (Process_MatchImage(box, folder->buf, 0, name, 1)) {

					match = TRUE;
				}
			}
			else {

				//
				// wildcards in ForceProcess:  match using pattern
				//

				if (!path_lwr) {
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
			}

		}
		else {

			if (Process_MatchImage(box, folder->buf, 0, name, 1)) {

				match = TRUE;
			}
		}

		if (match) {
			matched = TRUE;

			if (folder->has_priority && (!hasPriority || folder->priority < bestPriority)) {
				hasPriority = TRUE;
				bestPriority = folder->priority;
			}

			if (!outHasPriority && !outPriority)
				break;
		}

		folder = List_Next(folder);
	}

	if (path_lwr)
		Mem_FreeString(path_lwr);

	if (hasPriority) {
		if (outHasPriority)
			*outHasPriority = TRUE;
		if (outPriority)
			*outPriority = bestPriority;
	}

	return matched;
}

static BOOLEAN Process_MatchBreakoutProcessRuleRaw(
    BOX *box, const WCHAR *rule, const WCHAR *name, const WCHAR *path)
{
    BOOLEAN match = FALSE;

    if (!rule || !*rule)
        return FALSE;

    if (ProgramControl_RuleLooksLikePath(rule)) {
        WCHAR *norm = Process_NormalizeBreakoutRulePath(box, rule, L"BreakoutProcess", NULL);
        const WCHAR *match_rule = norm ? norm : rule;

        if (wcschr(rule, L'*') || wcschr(rule, L'?')) {
            ULONG path_lwr_len = 0;
            WCHAR *path_lwr = Mem_AllocString(Driver_Pool, path);
            PATTERN *pat = Pattern_Create(box->expand_args->pool, match_rule, TRUE, 0);

            if (path_lwr) {
                _wcslwr(path_lwr);
                path_lwr_len = wcslen(path_lwr);
            }

            if (pat && path_lwr)
                match = Pattern_Match(pat, path_lwr, path_lwr_len);

            if (pat)
                Pattern_Free(pat);
            if (path_lwr)
                Mem_FreeString(path_lwr);
        }
        else {
            ULONG rule_len = (ULONG)wcslen(match_rule);
            if (rule_len == wcslen(path) && _wcsnicmp(match_rule, path, rule_len) == 0)
                match = TRUE;
        }

        if (norm)
            Mem_FreeString(norm);
    }
    else {
        if (Process_MatchImage(box, rule, 0, name, 1))
            match = TRUE;
    }

    return match;
}

static BOOLEAN Process_MatchBreakoutFolderRuleRaw(
    BOX *box, const SBIE_NORMALIZED_RULE *normalized, const WCHAR *name, const WCHAR *path)
{
    const WCHAR *rule;
    const WCHAR *ptr;
    ULONG prefix_len;
    BOOLEAN match = FALSE;

    if (!normalized)
        return FALSE;

    rule = normalized->base_rule;
    if (!rule || !*rule)
        return FALSE;

    if (!ProgramControl_RuleLooksLikePath(rule))
        return FALSE;

    ptr = wcsrchr(path, L'\\');
    if (ptr && ptr[1])
        prefix_len = (ULONG)(ptr - path);
    else
        prefix_len = 0;

    if (!prefix_len)
        return FALSE;

    WCHAR *norm = Process_NormalizeBreakoutRulePath(box, rule, L"BreakoutFolder", NULL);
    const WCHAR *match_rule = norm ? norm : rule;

    if (wcschr(match_rule, L'*') || wcschr(match_rule, L'?')) {
        ULONG path_lwr_len = 0;
        WCHAR *path_lwr = Mem_AllocString(Driver_Pool, path);
        PATTERN *pat = Pattern_Create(box->expand_args->pool, match_rule, TRUE, 0);

        if (path_lwr) {
            _wcslwr(path_lwr);
            path_lwr_len = wcslen(path_lwr);
        }

        if (pat && path_lwr)
            match = Pattern_Match(pat, path_lwr, path_lwr_len);

        if (!match && pat && path_lwr) {
            path_lwr[prefix_len] = L'\0';
            match = Pattern_Match(pat, path_lwr, wcslen(path_lwr));
        }

        if (pat)
            Pattern_Free(pat);
        if (path_lwr)
            Mem_FreeString(path_lwr);

        // Apply Recursive depth limit for wildcard rules that matched.
        // Base folder anchor follows Recursive anchor mode.
        if (match && normalized->has_recursive) {
            size_t base_len = 0;
            if (ProgramControl_FindWildcardAnchorBaseLen(
                    match_rule,
                    normalized->recursive_anchor_from_last,
                    path,
                    prefix_len,
                    &base_len)) {
                if (base_len < prefix_len) {
                    const WCHAR *dir_begin = path + base_len;
                    const WCHAR *dir_end = path + prefix_len;
                    LONG depth = 0;

                    if (*dir_begin == L'\\')
                        ++dir_begin;

                    if (dir_begin < dir_end) {
                        const WCHAR *dp = dir_begin;
                        depth = 1;
                        while (dp < dir_end) {
                            if (*dp == L'\\')
                                ++depth;
                            ++dp;
                        }
                    }

                    if (depth < normalized->recursive_min_depth || (normalized->recursive_depth >= 0 && depth > normalized->recursive_depth))
                        match = FALSE;
                }
            }
        }
    }
    else {
        ULONG rule_len = (ULONG)wcslen(match_rule);
        while (rule_len && match_rule[rule_len - 1] == L'\\')
            --rule_len;

        if (rule_len && prefix_len >= rule_len &&
            path[rule_len] == L'\\' &&
            Box_NlsStrCmp(path, match_rule, rule_len) == 0) {
            if (normalized->has_recursive) {
                const WCHAR *dir_begin = path + rule_len;
                const WCHAR *dir_end = path + prefix_len;
                LONG depth = 0;

                if (*dir_begin == L'\\')
                    ++dir_begin;

                if (dir_begin < dir_end) {
                    const WCHAR *p = dir_begin;
                    depth = 1;
                    while (p < dir_end) {
                        if (*p == L'\\')
                            ++depth;
                        ++p;
                    }
                }

                if (depth >= normalized->recursive_min_depth && (normalized->recursive_depth < 0 || depth <= normalized->recursive_depth))
                    match = TRUE;
            }
            else {
                match = TRUE;
            }
        }
    }

    if (norm)
        Mem_FreeString(norm);

    return match;
}

static int Process_BreakoutMatchImage(const WCHAR *pattern, const WCHAR *imageName, void *context)
{
    BOX *box = (BOX *)context;
    return Process_MatchImage(box, pattern, 0, imageName, 1) ? 1 : 0;
}

static BOOLEAN Process_FindMatchingBreakoutFolderRule(
    BOX *box, const WCHAR *name, const WCHAR *path, BOOLEAN requireTarget,
    WCHAR *outTarget, ULONG outTargetCch)
{
    ULONG index = 0;
    const WCHAR *value;
    BOOLEAN use_breakout_folder_extensions;

    if (outTarget && outTargetCch)
        outTarget[0] = L'\0';

    use_breakout_folder_extensions = Process_UseRuleExtensions(box->name, L"BreakoutFolder");

    while (1) {
        WCHAR *rule_copy;
        WCHAR *rule;
        SBIE_NORMALIZED_RULE normalized;
        const WCHAR *target = NULL;
        BOOLEAN match;

        value = Conf_Get(box->name, L"BreakoutFolder", index);
        if (!value)
            break;
        ++index;

        rule_copy = Mem_AllocString(Driver_Pool, value);
        if (!rule_copy)
            continue;

        rule = ProgramControl_MatchImageScopeAndGetValue(rule_copy, name, Process_BreakoutMatchImage, box);
        if (!rule) {
            Mem_FreeString(rule_copy);
            continue;
        }

        if (!ProgramControl_ParseRuleExtensionsInPlace(rule, &normalized, use_breakout_folder_extensions)) {
            Mem_FreeString(rule_copy);
            continue;
        }

        if (normalized.has_target_box)
            target = normalized.target_box;

        if (requireTarget && (!target || !*target)) {
            Mem_FreeString(rule_copy);
            continue;
        }

        match = Process_MatchBreakoutFolderRuleRaw(box, &normalized, name, path);
        if (match) {
            if (requireTarget && outTarget && outTargetCch && target) {
                wcsncpy(outTarget, target, outTargetCch - 1);
                outTarget[outTargetCch - 1] = L'\0';
            }

            Mem_FreeString(rule_copy);
            return TRUE;
        }

        Mem_FreeString(rule_copy);
    }

    return FALSE;
}

static WCHAR* Process_NormalizeBreakoutRulePath(
    BOX *box, const WCHAR *rule, const WCHAR *setting, ULONG *out_len)
{
    WCHAR *expnd;
    WCHAR *tmp1;
    WCHAR *tmp2;
    WCHAR *buf;
    ULONG buf_len;

    if (!rule || !*rule || !wcschr(rule, L'\\')) {
        buf = Mem_AllocString(Driver_Pool, rule ? rule : L"");
        if (buf && out_len)
            *out_len = (ULONG)((wcslen(buf) + 1) * sizeof(WCHAR));
        return buf;
    }

    expnd = Conf_Expand(box->expand_args, rule, setting);
    if (!expnd) {
        buf = Mem_AllocString(Driver_Pool, rule);
        if (buf && out_len)
            *out_len = (ULONG)((wcslen(buf) + 1) * sizeof(WCHAR));
        return buf;
    }

    buf_len = (ULONG)((wcslen(expnd) + 1) * sizeof(WCHAR));
    tmp1 = Mem_Alloc(Driver_Pool, buf_len);
    if (!tmp1) {
        Mem_FreeString(expnd);
        return NULL;
    }

    {
        const WCHAR *src_ptr = expnd;
        WCHAR *dst_ptr = tmp1;
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
    }

    //
    // If the path is in DOS format (e.g. "E:\Files\*.txt"), translate it
    // to NT device format (e.g. "\Device\HarddiskVolume2\Files\*.txt")
    // before calling File_TranslateReparsePoints, which only accepts paths
    // starting with "\Device\".  File_TranslateDosToNt translates only the
    // drive letter portion, so wildcard suffixes are preserved correctly.
    //

    if (tmp1[0] && tmp1[1] == L':') {
        WCHAR *nt_path = NULL;
        ULONG nt_path_len = 0;
        NTSTATUS status = File_TranslateDosToNt(tmp1, Driver_Pool, &nt_path, &nt_path_len);
        if (NT_SUCCESS(status) && nt_path) {
            Mem_Free(tmp1, buf_len);
            tmp1 = nt_path;
            buf_len = nt_path_len;
        }
    }

    tmp2 = File_TranslateReparsePoints(tmp1, Driver_Pool);
    if (tmp2) {
        Mem_Free(tmp1, buf_len);
        buf = tmp2;
        buf_len = (ULONG)((wcslen(buf) + 1) * sizeof(WCHAR));
    }
    else {
        buf = tmp1;
    }

    Mem_FreeString(expnd);

    if (out_len)
        *out_len = buf_len;

    return buf;
}

static FORCE_BOX* Process_FindForceBoxByName(
    LIST *boxes, const WCHAR *boxname)
{
    FORCE_BOX *box = List_Head(boxes);
    ULONG boxname_len;

    if (!boxname || !*boxname)
        return NULL;

    boxname_len = (ULONG)((wcslen(boxname) + 1) * sizeof(WCHAR));

    while (box) {
        if (box->box->name_len == boxname_len && _wcsicmp(box->box->name, boxname) == 0)
            return box;

        box = List_Next(box);
    }

    return NULL;
}

static BOOLEAN Process_GetMatchedBreakoutTarget(
    BOX *box, const WCHAR *processName, const WCHAR *folderScopeName, const WCHAR *path, WCHAR *outTarget, ULONG outTargetCch)
{
    ULONG index = 0;
    const WCHAR *value;
    const WCHAR *scopeName;
    BOOLEAN process_match = FALSE;
    BOOLEAN process_has_priority = FALSE;
    LONG process_priority = -1;
    BOOLEAN process_target_match = FALSE;
    BOOLEAN process_target_has_priority = FALSE;
    LONG process_target_priority = -1;
    BOOLEAN folder_match = FALSE;
    BOOLEAN folder_has_priority = FALSE;
    LONG folder_priority = -1;
    BOOLEAN folder_target_match = FALSE;
    BOOLEAN folder_target_has_priority = FALSE;
    LONG folder_target_priority = -1;
    WCHAR process_target_box[BOXNAME_COUNT] = { 0 };
    WCHAR folder_target_box[BOXNAME_COUNT] = { 0 };
    BOOLEAN use_breakout_process_extensions;
    BOOLEAN use_breakout_folder_extensions;

    if (!outTarget || outTargetCch == 0)
        return FALSE;

    if (!box || !processName || !*processName || !path || !*path)
        return FALSE;

    if (!Process_AreBreakoutRulesEnabled(box->name))
        return FALSE;

    use_breakout_process_extensions = Process_UseRuleExtensions(box->name, L"BreakoutProcess");
    use_breakout_folder_extensions = Process_UseRuleExtensions(box->name, L"BreakoutFolder");

    outTarget[0] = L'\0';
    scopeName = (folderScopeName && *folderScopeName) ? folderScopeName : processName;

    while (1) {
        WCHAR *rule;
        WCHAR *rule_copy;
        SBIE_NORMALIZED_RULE normalized;

        value = Conf_Get(box->name, L"BreakoutProcess", index);
        if (!value)
            break;
        ++index;

        rule_copy = Mem_AllocString(Driver_Pool, value);
        if (!rule_copy)
            continue;

        rule = ProgramControl_MatchImageScopeAndGetValue(rule_copy, processName, Process_BreakoutMatchImage, box);
        if (!rule) {
            Mem_FreeString(rule_copy);
            continue;
        }

        if (!ProgramControl_ParseRuleExtensionsInPlace(rule, &normalized, use_breakout_process_extensions)) {
            Mem_FreeString(rule_copy);
            continue;
        }

        if ((wcschr(normalized.base_rule, L'*') || wcschr(normalized.base_rule, L'?')) &&
            !ProgramControl_RuleLooksLikePath(normalized.base_rule) &&
            ProgramControl_IsBroadWildcardImageRule(normalized.base_rule)) {
            Mem_FreeString(rule_copy);
            continue;
        }

        if (Process_MatchBreakoutProcessRuleRaw(box, normalized.base_rule, processName, path)) {
            process_match = TRUE;
            if (normalized.has_priority && (!process_has_priority || normalized.priority < process_priority)) {
                process_has_priority = TRUE;
                process_priority = normalized.priority;
            }

            if (normalized.has_target_box && normalized.target_box && *normalized.target_box) {
                if (ProgramControl_ShouldReplacePriorityWinner(
                    process_target_match ? 1 : 0,
                    process_target_has_priority ? 1 : 0,
                    process_target_priority,
                    normalized.has_priority ? 1 : 0,
                    normalized.priority)) {
                    wcsncpy(process_target_box, normalized.target_box, BOXNAME_COUNT - 1);
                    process_target_box[BOXNAME_COUNT - 1] = L'\0';
                    process_target_match = TRUE;
                    process_target_has_priority = normalized.has_priority ? TRUE : FALSE;
                    process_target_priority = normalized.priority;
                }
            }
        }

        Mem_FreeString(rule_copy);
    }

    index = 0;
    while (1) {
        WCHAR *rule_copy;
        WCHAR *rule;
        SBIE_NORMALIZED_RULE normalized;

        value = Conf_Get(box->name, L"BreakoutFolder", index);
        if (!value)
            break;
        ++index;

        rule_copy = Mem_AllocString(Driver_Pool, value);
        if (!rule_copy)
            continue;

        rule = ProgramControl_MatchImageScopeAndGetValue(rule_copy, scopeName, Process_BreakoutMatchImage, box);
        if (!rule) {
            Mem_FreeString(rule_copy);
            continue;
        }

        if (!ProgramControl_ParseRuleExtensionsInPlace(rule, &normalized, use_breakout_folder_extensions)) {
            Mem_FreeString(rule_copy);
            continue;
        }

        if (Process_MatchBreakoutFolderRuleRaw(box, &normalized, scopeName, path)) {
            folder_match = TRUE;
            if (normalized.has_priority && (!folder_has_priority || normalized.priority < folder_priority)) {
                folder_has_priority = TRUE;
                folder_priority = normalized.priority;
            }

            if (normalized.has_target_box && normalized.target_box && *normalized.target_box) {
                if (ProgramControl_ShouldReplacePriorityWinner(
                    folder_target_match ? 1 : 0,
                    folder_target_has_priority ? 1 : 0,
                    folder_target_priority,
                    normalized.has_priority ? 1 : 0,
                    normalized.priority)) {
                    wcsncpy(folder_target_box, normalized.target_box, BOXNAME_COUNT - 1);
                    folder_target_box[BOXNAME_COUNT - 1] = L'\0';
                    folder_target_match = TRUE;
                    folder_target_has_priority = normalized.has_priority ? TRUE : FALSE;
                    folder_target_priority = normalized.priority;
                }
            }
        }

        Mem_FreeString(rule_copy);
    }

    if (!process_match && !folder_match)
        return FALSE;

    // Pick winning breakout side first, then allow only that side's target.
    BOOLEAN use_process_target = process_match ? TRUE : FALSE;
    if (process_match && folder_match) {
        if (process_has_priority != folder_has_priority)
            use_process_target = process_has_priority ? TRUE : FALSE;
        else if (process_has_priority && folder_has_priority) {
            if (process_priority < folder_priority)
                use_process_target = TRUE;
            else if (process_priority > folder_priority)
                use_process_target = FALSE;
            else
                use_process_target = TRUE;
        }
        else {
            // No explicit priority on either side -> BreakoutProcess wins tie.
            use_process_target = TRUE;
        }
    }

    if (use_process_target) {
        if (!process_target_match)
            return FALSE;

        // Do not apply a weaker targeted rule when a stronger non-target
        // breakout match already won on the same side.
        if (process_has_priority) {
            if (!process_target_has_priority)
                return FALSE;
            if (process_target_priority > process_priority)
                return FALSE;
        }

        wcsncpy(outTarget, process_target_box, outTargetCch - 1);
        outTarget[outTargetCch - 1] = L'\0';
        return TRUE;
    }

    if (!folder_target_match)
        return FALSE;

    // Do not apply a weaker targeted rule when a stronger non-target
    // breakout match already won on the same side.
    if (folder_has_priority) {
        if (!folder_target_has_priority)
            return FALSE;
        if (folder_target_priority > folder_priority)
            return FALSE;
    }

    wcsncpy(outTarget, folder_target_box, outTargetCch - 1);
    outTarget[outTargetCch - 1] = L'\0';
    return TRUE;
}

static BOOLEAN Process_IsSelfTargetedBreakoutMatch(
    BOX *box, const WCHAR *processName, const WCHAR *folderScopeName, const WCHAR *path)
{
    WCHAR target_box[BOXNAME_COUNT] = { 0 };

    if (!Process_GetMatchedBreakoutTarget(box, processName, folderScopeName, path, target_box, BOXNAME_COUNT))
        return FALSE;

    return (_wcsicmp(target_box, box->name) == 0) ? TRUE : FALSE;
}

static BOOLEAN Process_CheckBreakoutProcessList(
    BOX *box, LIST* BreakoutProcess, const WCHAR* name, const WCHAR* path)
{
    ULONG path_lwr_len = 0;
    WCHAR *path_lwr = NULL;

    FORCE_ENTRY *entry = List_Head(BreakoutProcess);
    while (entry) {

        BOOLEAN match = FALSE;

        if (entry->pat) {

            // Ignore overly broad wildcard-only BreakoutProcess image rules.
            if (!ProgramControl_RuleLooksLikePath(entry->buf)) {
                if (ProgramControl_IsBroadWildcardImageRule(entry->buf)) {
                    entry = List_Next(entry);
                    continue;
                }

                if (Process_MatchImage(box, entry->buf, 0, name, 1))
                    match = TRUE;
            }

            else {
                if (! path_lwr) {
                    path_lwr = Mem_AllocString(Driver_Pool, path);
                    if (path_lwr) {
                        _wcslwr(path_lwr);
                        path_lwr_len = wcslen(path_lwr);
                    }
                }

                if (path_lwr)
                    match = Pattern_Match(entry->pat, path_lwr, path_lwr_len);
            }

        } else {

            if (ProgramControl_RuleLooksLikePath(entry->buf)) {
                ULONG entry_len = entry->len;
                if (entry_len == wcslen(path) && _wcsnicmp(entry->buf, path, entry_len) == 0)
                    match = TRUE;
            } else {
                if (Process_MatchImage(box, entry->buf, 0, name, 1))
                    match = TRUE;
            }
        }

        if (match)
            break;

        entry = List_Next(entry);
    }

    if (path_lwr)
        Mem_FreeString(path_lwr);

    return (entry != NULL);
}


//---------------------------------------------------------------------------
// Process_MatchBreakoutDocumentRule
//---------------------------------------------------------------------------

static BOOLEAN Process_MatchBreakoutDocumentRule(
    BOX *box, const WCHAR *rule, const WCHAR *docPath)
{
    BOOLEAN match = FALSE;
    BOOLEAN rule_has_wildcard;
    BOOLEAN rule_has_separator;

    if (!rule || !*rule || !docPath || !*docPath)
        return FALSE;

    rule_has_wildcard = (wcschr(rule, L'*') || wcschr(rule, L'?')) ? TRUE : FALSE;
    rule_has_separator = (wcschr(rule, L'\\') || wcschr(rule, L'/')) ? TRUE : FALSE;
    if (!ProgramControl_RuleLooksLikePath(rule)) {
        // Allow simple filename-style wildcard rules like "*.txt".
        if (!(rule_has_wildcard && !rule_has_separator))
            return FALSE;
    }

    WCHAR *norm = Process_NormalizeBreakoutRulePath(box, rule, L"BreakoutDocument", NULL);
    const WCHAR *match_rule = norm ? norm : rule;

    if (wcschr(match_rule, L'*') || wcschr(match_rule, L'?')) {
        WCHAR *path_lwr = Mem_AllocString(Driver_Pool, docPath);
        PATTERN *pat = Pattern_Create(box->expand_args->pool, match_rule, TRUE, 0);

        if (path_lwr) {
            _wcslwr(path_lwr);
            ULONG path_lwr_len = wcslen(path_lwr);
            if (pat)
                match = Pattern_Match(pat, path_lwr, path_lwr_len);
        }

        if (pat)
            Pattern_Free(pat);
        if (path_lwr)
            Mem_FreeString(path_lwr);
    } else {
        ULONG rule_len = (ULONG)wcslen(match_rule);
        if (rule_len == wcslen(docPath) && _wcsnicmp(match_rule, docPath, rule_len) == 0)
            match = TRUE;
    }

    if (norm)
        Mem_FreeString(norm);

    return match;
}


//---------------------------------------------------------------------------
// Process_GetBreakoutDocumentPriority
//---------------------------------------------------------------------------

static BOOLEAN Process_GetBreakoutDocumentPriority(
    BOX *box, const WCHAR *scopeName, const WCHAR *docPath,
    BOOLEAN *outHasPriority, LONG *outPriority)
{
    const WCHAR *value;
    ULONG index;
    BOOLEAN matched = FALSE;
    BOOLEAN hasPriority = FALSE;
    LONG bestPriority = -1;
    BOOLEAN use_breakout_document_extensions;

    if (outHasPriority)
        *outHasPriority = FALSE;
    if (outPriority)
        *outPriority = -1;

    if (!box || !docPath || !*docPath)
        return FALSE;

    if (!Process_AreBreakoutRulesEnabled(box->name))
        return FALSE;

    use_breakout_document_extensions = Process_UseRuleExtensions(box->name, L"BreakoutDocument");

    index = 0;
    while (1) {
        WCHAR *ruleCopy;
        WCHAR *rule;
        SBIE_NORMALIZED_RULE normalized;

        value = Conf_Get(box->name, L"BreakoutDocument", index);
        if (!value)
            break;
        ++index;

        ruleCopy = Mem_AllocString(Driver_Pool, value);
        if (!ruleCopy)
            continue;

        rule = scopeName
            ? ProgramControl_MatchImageScopeAndGetValue(ruleCopy, scopeName, Process_BreakoutMatchImage, box)
            : ruleCopy;
        if (!rule) {
            Mem_FreeString(ruleCopy);
            continue;
        }

        if (!ProgramControl_ParseRuleExtensionsInPlace(rule, &normalized, use_breakout_document_extensions)) {
            Mem_FreeString(ruleCopy);
            continue;
        }

        if (Process_MatchBreakoutDocumentRule(box, normalized.base_rule, docPath)) {
            matched = TRUE;
            if (normalized.has_priority && (!hasPriority || normalized.priority < bestPriority)) {
                hasPriority = TRUE;
                bestPriority = normalized.priority;
            }
        }

        Mem_FreeString(ruleCopy);
    }

    if (hasPriority) {
        if (outHasPriority)
            *outHasPriority = TRUE;
        if (outPriority)
            *outPriority = bestPriority;
    }

    return matched;
}


//---------------------------------------------------------------------------
// Process_GetBreakoutDocumentTarget
//---------------------------------------------------------------------------

static BOOLEAN Process_GetBreakoutDocumentTarget(
    BOX *box, const WCHAR *scopeName, const WCHAR *docPath,
    WCHAR *outTarget, ULONG outTargetCch,
    BOOLEAN *outHasTarget, BOOLEAN *outHasPriority, LONG *outPriority,
    ULONG *outLevel)
{
    const WCHAR *value;
    ULONG index;
    BOOLEAN hasMatch = FALSE;
    BOOLEAN bestHasTarget = FALSE;
    BOOLEAN bestHasPriority = FALSE;
    LONG bestPriority = -1;
    ULONG bestLevel = (ULONG)-1;
    BOOLEAN use_breakout_document_extensions;

    if (!outTarget || outTargetCch == 0)
        return FALSE;

    if (!box || !docPath || !*docPath)
        return FALSE;

    if (!Process_AreBreakoutRulesEnabled(box->name))
        return FALSE;

    use_breakout_document_extensions = Process_UseRuleExtensions(box->name, L"BreakoutDocument");

    outTarget[0] = L'\0';
    if (outHasTarget)
        *outHasTarget = FALSE;
    if (outHasPriority)
        *outHasPriority = FALSE;
    if (outPriority)
        *outPriority = -1;
    if (outLevel)
        *outLevel = (ULONG)-1;

    index = 0;
    while (1) {
        WCHAR *ruleCopy;
        WCHAR *rule;
        SBIE_NORMALIZED_RULE normalized;
        ULONG level = (ULONG)-1;

        value = Conf_Get(box->name, L"BreakoutDocument", index);
        if (!value)
            break;
        ++index;

        ruleCopy = Mem_AllocString(Driver_Pool, value);
        if (!ruleCopy)
            continue;

        rule = scopeName
            ? ProgramControl_MatchImageScopeAndGetValueEx(ruleCopy, scopeName, Process_BreakoutMatchImage, box, &level)
            : ruleCopy;
        if (!scopeName)
            level = 2;
        if (!rule) {
            Mem_FreeString(ruleCopy);
            continue;
        }

        if (!ProgramControl_ParseRuleExtensionsInPlace(rule, &normalized, use_breakout_document_extensions)) {
            Mem_FreeString(ruleCopy);
            continue;
        }

        if (Process_MatchBreakoutDocumentRule(box, normalized.base_rule, docPath)) {
            if (ProgramControl_ShouldReplaceTargetMatch(
                    hasMatch ? 1 : 0,
                    bestHasPriority ? 1 : 0,
                    bestPriority,
                    bestLevel,
                    normalized.has_priority ? 1 : 0,
                    normalized.priority,
                    level)) {
                hasMatch = TRUE;
                bestHasTarget = (normalized.has_target_box && normalized.target_box && *normalized.target_box) ? TRUE : FALSE;
                bestHasPriority = normalized.has_priority ? TRUE : FALSE;
                bestPriority = normalized.has_priority ? normalized.priority : -1;
                bestLevel = level;
                if (bestHasTarget) {
                    wcsncpy(outTarget, normalized.target_box, outTargetCch - 1);
                    outTarget[outTargetCch - 1] = L'\0';
                }
                else {
                    outTarget[0] = L'\0';
                }
            }
        }

        Mem_FreeString(ruleCopy);
    }

    if (hasMatch) {
        if (outHasTarget)
            *outHasTarget = bestHasTarget;
        if (outHasPriority)
            *outHasPriority = bestHasPriority;
        if (outPriority)
            *outPriority = bestHasPriority ? bestPriority : -1;
        if (outLevel)
            *outLevel = bestLevel;
    }

    return hasMatch;
}

static void Process_GetBreakoutDocumentPriorityBest(
    BOX *box,
    const WCHAR *primaryScopeName,
    const WCHAR *secondaryScopeName,
    const WCHAR *docPath,
    BOOLEAN *outMatched,
    BOOLEAN *outHasPriority,
    LONG *outPriority)
{
    BOOLEAN matched = FALSE;
    BOOLEAN hasPriority = FALSE;
    LONG priority = -1;

    if (outMatched)
        *outMatched = FALSE;
    if (outHasPriority)
        *outHasPriority = FALSE;
    if (outPriority)
        *outPriority = -1;

    if (!box || !docPath || !*docPath)
        return;

    if (primaryScopeName && *primaryScopeName) {
        matched = Process_GetBreakoutDocumentPriority(box, primaryScopeName, docPath, &hasPriority, &priority);
    }

    if (secondaryScopeName && *secondaryScopeName &&
        (!primaryScopeName || _wcsicmp(primaryScopeName, secondaryScopeName) != 0)) {
        BOOLEAN matched2 = FALSE;
        BOOLEAN hasPriority2 = FALSE;
        LONG priority2 = -1;

        matched2 = Process_GetBreakoutDocumentPriority(box, secondaryScopeName, docPath, &hasPriority2, &priority2);
        if (matched2) {
                if (!matched || ProgramControl_ShouldReplacePriorityWinner(
                    matched ? 1 : 0,
                    hasPriority ? 1 : 0,
                    priority,
                    hasPriority2 ? 1 : 0,
                    priority2)) {
                matched = TRUE;
                hasPriority = hasPriority2;
                priority = priority2;
            }
        }
    }

    if (outMatched)
        *outMatched = matched;
    if (outHasPriority)
        *outHasPriority = hasPriority;
    if (outPriority)
        *outPriority = hasPriority ? priority : -1;
}

static BOOLEAN Process_GetBreakoutDocumentTargetBest(
    BOX *box,
    const WCHAR *primaryScopeName,
    const WCHAR *secondaryScopeName,
    const WCHAR *docPath,
    WCHAR *outTarget,
    ULONG outTargetCch)
{
    WCHAR bestTarget[BOXNAME_COUNT] = { 0 };
    BOOLEAN hasBest = FALSE;
    BOOLEAN bestHasTarget = FALSE;
    BOOLEAN bestHasPriority = FALSE;
    LONG bestPriority = -1;
    ULONG bestLevel = (ULONG)-1;

    if (!outTarget || outTargetCch == 0)
        return FALSE;

    outTarget[0] = L'\0';

    if (!box || !docPath || !*docPath)
        return FALSE;

    if (primaryScopeName && *primaryScopeName) {
        BOOLEAN hasTarget1 = FALSE;
        BOOLEAN hasPriority1 = FALSE;
        LONG priority1 = -1;
        ULONG level1 = (ULONG)-1;
        hasBest = Process_GetBreakoutDocumentTarget(
            box,
            primaryScopeName,
            docPath,
            bestTarget,
            BOXNAME_COUNT,
            &hasTarget1,
            &hasPriority1,
            &priority1,
            &level1);
        bestHasTarget = hasTarget1;
        bestHasPriority = hasPriority1;
        bestPriority = priority1;
        bestLevel = level1;
    }

    if (secondaryScopeName && *secondaryScopeName &&
        (!primaryScopeName || _wcsicmp(primaryScopeName, secondaryScopeName) != 0)) {
        WCHAR target2[BOXNAME_COUNT] = { 0 };
        BOOLEAN hasTarget2 = FALSE;
        BOOLEAN hasPriority2 = FALSE;
        LONG priority2 = -1;
        ULONG level2 = (ULONG)-1;
        BOOLEAN has2 = Process_GetBreakoutDocumentTarget(
            box,
            secondaryScopeName,
            docPath,
            target2,
            BOXNAME_COUNT,
            &hasTarget2,
            &hasPriority2,
            &priority2,
            &level2);

        if (has2 && ProgramControl_ShouldReplaceTargetMatch(
            hasBest ? 1 : 0,
            bestHasPriority ? 1 : 0,
            bestPriority,
            bestLevel,
            hasPriority2 ? 1 : 0,
            priority2,
            level2)) {
            hasBest = TRUE;
            bestHasTarget = hasTarget2;
            bestHasPriority = hasPriority2;
            bestPriority = priority2;
            bestLevel = level2;
            if (hasTarget2) {
                wcsncpy(bestTarget, target2, BOXNAME_COUNT - 1);
                bestTarget[BOXNAME_COUNT - 1] = L'\0';
            }
            else {
                bestTarget[0] = L'\0';
            }
        }
    }

    if (hasBest && bestHasTarget) {
        wcsncpy(outTarget, bestTarget, outTargetCch - 1);
        outTarget[outTargetCch - 1] = L'\0';
        return TRUE;
    }

    return FALSE;
}


//---------------------------------------------------------------------------
// Process_CheckForceProcess
//---------------------------------------------------------------------------


_FX BOX *Process_CheckForceProcess(
    LIST *boxes, const WCHAR *name, const WCHAR* path, const WCHAR *docPath, BOOLEAN alert, ULONG *IsAlert, const WCHAR *ParentName, const WCHAR *ParentPath, BOOLEAN *pForcedByChildren, HANDLE ProcessId)
{
    FORCE_BOX *box;
    FORCE_BOX *force_winner = NULL;
    BOOLEAN force_winner_from_children = FALSE;
    BOOLEAN force_winner_has_priority = FALSE;
    LONG force_winner_priority = -1;
    BOOLEAN have_force_winner = FALSE;

    if (pForcedByChildren)
        *pForcedByChildren = FALSE;

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
        BOOLEAN force_has_priority = FALSE;
        LONG force_priority = -1;

        if (Process_CheckForceProcessList(box->box, &box->ForceProcess, name, path, &force_has_priority, &force_priority)) {

            if (have_force_winner &&
                !ProgramControl_ShouldReplacePriorityWinner(
                    have_force_winner ? 1 : 0,
                    force_winner_has_priority ? 1 : 0,
                    force_winner_priority,
                    force_has_priority ? 1 : 0,
                    force_priority)) {
                box = List_Next(box);
                continue;
            }

            const WCHAR *folder_scope_name = (ParentName && *ParentName) ? ParentName : name;
            BOOLEAN effective_prioritize_breakout = FALSE;
            BOOLEAN breakout_has_priority = FALSE;
            LONG breakout_priority = -1;
            BOOLEAN has_target_override = FALSE;
            WCHAR target_box[BOXNAME_COUNT] = { 0 };

            Process_GetMatchedBreakoutPriority(box->box, name, folder_scope_name, path, &breakout_has_priority, &breakout_priority);

            // Also check BreakoutDocument rules against the document argument.
            // Track whether BreakoutDocument won or tied the priority contest
            // so its TargetBox owns this document-open path.
            BOOLEAN bd_contributed_priority = FALSE;
            if (docPath && *docPath) {
                BOOLEAN bd_matched = FALSE;
                BOOLEAN bd_has_priority = FALSE;
                LONG bd_priority = -1;
                Process_GetBreakoutDocumentPriorityBest(
                    box->box,
                    folder_scope_name,
                    name,
                    docPath,
                    &bd_matched,
                    &bd_has_priority,
                    &bd_priority);
                if (bd_matched) {
                    if (bd_has_priority) {
                        if (!breakout_has_priority || bd_priority < breakout_priority) {
                            breakout_has_priority = TRUE;
                            breakout_priority = bd_priority;
                            bd_contributed_priority = TRUE;
                        } else if (bd_priority == breakout_priority) {
                            // Tie: BreakoutDocument may also contribute its target.
                            bd_contributed_priority = TRUE;
                        }
                    } else if (!breakout_has_priority) {
                        // Neither has an explicit priority: equal footing,
                        // BreakoutDocument target may apply.
                        bd_contributed_priority = TRUE;
                    }
                }
            }

            effective_prioritize_breakout = SbiePolicy_ShouldPrioritizeBreakout(
                FALSE,
                force_has_priority ? 1 : 0,
                force_priority,
                breakout_has_priority ? 1 : 0,
                breakout_priority) ? TRUE : FALSE;

            if (effective_prioritize_breakout) {
                if (bd_contributed_priority && docPath && *docPath) {
                    // BreakoutDocument won or tied the priority contest, so it owns
                    // target resolution for this document-open path.
                    has_target_override = Process_GetBreakoutDocumentTargetBest(
                        box->box,
                        folder_scope_name,
                        name,
                        docPath,
                        target_box,
                        BOXNAME_COUNT);
                } else {
                    has_target_override = Process_GetMatchedBreakoutTarget(box->box, name, folder_scope_name, path, target_box, BOXNAME_COUNT);
                }
            }

            if (has_target_override) {
                FORCE_BOX *target = Process_FindForceBoxByName(boxes, target_box);
                if (target) {
                    if (alert) {
                        *IsAlert = 1;
                        return NULL;
                    }
                    return target->box;
                }
            }

            if (effective_prioritize_breakout && !has_target_override &&
                (bd_contributed_priority || Process_IsPrioritizedBreakoutMatch(box->box, name, folder_scope_name, path))) {
                if (bd_contributed_priority)
                    Process_DfpInsert(PROCESS_TERMINATED, ProcessId);
                box = List_Next(box);
                continue;
            }

            if (alert) {
                *IsAlert = 1;
                return NULL;
            }

                if (ProgramControl_ShouldReplacePriorityWinner(
                    have_force_winner ? 1 : 0,
                    force_winner_has_priority ? 1 : 0,
                    force_winner_priority,
                    force_has_priority ? 1 : 0,
                    force_priority)) {
                force_winner = box;
                force_winner_from_children = FALSE;
                force_winner_has_priority = force_has_priority;
                force_winner_priority = force_priority;
                have_force_winner = TRUE;
            }

            box = List_Next(box);
            continue;
        }

        if (ParentName && Process_CheckForceProcessList(box->box, &box->ForceChildren, ParentName, ParentPath, &force_has_priority, &force_priority) && _wcsicmp(name, L"SandMan.exe") != 0) { // except for SandMan exe

            if (have_force_winner &&
                !ProgramControl_ShouldReplacePriorityWinner(
                    have_force_winner ? 1 : 0,
                    force_winner_has_priority ? 1 : 0,
                    force_winner_priority,
                    force_has_priority ? 1 : 0,
                    force_priority)) {
                box = List_Next(box);
                continue;
            }

            const WCHAR *folder_scope_name = ParentName;
            BOOLEAN effective_prioritize_breakout = FALSE;
            BOOLEAN breakout_has_priority = FALSE;
            LONG breakout_priority = -1;
            BOOLEAN has_target_override = FALSE;
            WCHAR target_box[BOXNAME_COUNT] = { 0 };

            Process_GetMatchedBreakoutPriority(box->box, name, folder_scope_name, path, &breakout_has_priority, &breakout_priority);

            // Also check BreakoutDocument rules against the document argument.
            // Track whether BreakoutDocument won or tied the priority contest
            // so its TargetBox owns this document-open path.
            BOOLEAN bd_contributed_priority = FALSE;
            if (docPath && *docPath) {
                BOOLEAN bd_matched = FALSE;
                BOOLEAN bd_has_priority = FALSE;
                LONG bd_priority = -1;
                Process_GetBreakoutDocumentPriorityBest(
                    box->box,
                    folder_scope_name,
                    name,
                    docPath,
                    &bd_matched,
                    &bd_has_priority,
                    &bd_priority);
                if (bd_matched) {
                    if (bd_has_priority) {
                        if (!breakout_has_priority || bd_priority < breakout_priority) {
                            breakout_has_priority = TRUE;
                            breakout_priority = bd_priority;
                            bd_contributed_priority = TRUE;
                        } else if (bd_priority == breakout_priority) {
                            // Tie: BreakoutDocument may also contribute its target.
                            bd_contributed_priority = TRUE;
                        }
                    } else if (!breakout_has_priority) {
                        // Neither has an explicit priority: equal footing,
                        // BreakoutDocument target may apply.
                        bd_contributed_priority = TRUE;
                    }
                }
            }

            effective_prioritize_breakout = SbiePolicy_ShouldPrioritizeBreakout(
                FALSE,
                force_has_priority ? 1 : 0,
                force_priority,
                breakout_has_priority ? 1 : 0,
                breakout_priority) ? TRUE : FALSE;

            if (effective_prioritize_breakout) {
                if (bd_contributed_priority && docPath && *docPath) {
                    // BreakoutDocument won or tied the priority contest, so it owns
                    // target resolution for this document-open path.
                    has_target_override = Process_GetBreakoutDocumentTargetBest(
                        box->box,
                        folder_scope_name,
                        name,
                        docPath,
                        target_box,
                        BOXNAME_COUNT);
                } else {
                    has_target_override = Process_GetMatchedBreakoutTarget(box->box, name, folder_scope_name, path, target_box, BOXNAME_COUNT);
                }
            }

            if (has_target_override) {
                FORCE_BOX *target = Process_FindForceBoxByName(boxes, target_box);
                if (target) {
                    if (pForcedByChildren)
                        *pForcedByChildren = TRUE;
                    if (alert) {
                        *IsAlert = 1;
                        return NULL;
                    }
                    return target->box;
                }
            }

            if (effective_prioritize_breakout && !has_target_override &&
                (bd_contributed_priority || Process_IsPrioritizedBreakoutMatch(box->box, name, folder_scope_name, path))) {
                if (bd_contributed_priority)
                    Process_DfpInsert(PROCESS_TERMINATED, ProcessId);
                box = List_Next(box);
                continue;
            }

            if (alert) {
                *IsAlert = 1;
                return NULL;
            }

                if (ProgramControl_ShouldReplacePriorityWinner(
                    have_force_winner ? 1 : 0,
                    force_winner_has_priority ? 1 : 0,
                    force_winner_priority,
                    force_has_priority ? 1 : 0,
                    force_priority)) {
                force_winner = box;
                force_winner_from_children = TRUE;
                force_winner_has_priority = force_has_priority;
                force_winner_priority = force_priority;
                have_force_winner = TRUE;
            }

            box = List_Next(box);
            continue;
        }

        //if (Process_IsWindowsExplorerParent(ParentId) && Conf_Get_Boolean(box->box->name, L"ForceExplorerChild", 0, FALSE)) {
        //    if (_wcsicmp(name, L"Sandman.exe") != 0)
        //        return box->box;
        //}

        box = List_Next(box);
    }

    if (have_force_winner) {
        if (pForcedByChildren)
            *pForcedByChildren = force_winner_from_children;
        return force_winner->box;
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
    //
    // check if the folder is alerted to any box
    //

    box = List_Head(boxes);
    while (box) {

        if (Process_CheckForceFolderList(box->box, &box->AlertFolder, prefix_len, path, NULL, NULL)) {
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

        if (Process_CheckForceProcessList(box->box, &box->AlertProcess, name, path, NULL, NULL)) {
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

    if (!Process_AreBreakoutRulesEnabled(box->name))
        goto finish;

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

    List_Init(&BreakoutProcess);

    Conf_AdjustUseCount(TRUE);

    Process_AddForceFolders(&BreakoutProcess, L"BreakoutProcess", box, box->name);
        
    Conf_AdjustUseCount(FALSE);

    IsBreakout = Process_CheckBreakoutProcessList(box, &BreakoutProcess, ImageName, ImagePath2);
    if (!IsBreakout)
        IsBreakout = Process_FindMatchingBreakoutFolderRule(box, ImageName, ImagePath2, FALSE, NULL, 0);

    Process_DeleteForceDataFolders(&BreakoutProcess);

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

        map_insert(&Process_MapDfp, ProcessId, proc, 0);

        ExReleaseResourceLite(Process_ListLock);
        KeLowerIrql(irql);

        added = TRUE;

    } else {

        //
        // called by Process_NotifyProcess_Create, process list locked
        //

        Process_DfpDelete(ProcessId);

        added = FALSE;

        proc = map_get(&Process_MapDfp, ParentId);
        if (proc) {

            proc = Mem_Alloc(Driver_Pool, sizeof(FORCE_PROCESS_2));
            proc->pid = ProcessId;
            proc->silent = FALSE;

            map_insert(&Process_MapDfp, ProcessId, proc, 0);

            added = TRUE;
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

    if(map_take(&Process_MapDfp, ProcessId, &proc, 0))
        Mem_Free(proc, sizeof(FORCE_PROCESS_2));
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

    proc = map_get(&Process_MapDfp, ProcessId);
    if (proc) {

        if (*silent)
            proc->silent = TRUE;
        else
            *silent = proc->silent;

        found = TRUE;
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

    map_insert(&Process_MapFcp, ProcessId, proc, 0);

    ExReleaseResourceLite(Process_ListLock);
    KeLowerIrql(irql);


}


//---------------------------------------------------------------------------
// Process_FcpDelete
//---------------------------------------------------------------------------


_FX void Process_FcpDelete(HANDLE ProcessId)
{
    FORCE_PROCESS_3 *proc;

    if(map_take(&Process_MapFcp, ProcessId, &proc, 0))
        Mem_Free(proc, sizeof(FORCE_PROCESS_3));
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

    proc = map_get(&Process_MapFcp, ProcessId);
    if (proc) {

        if(boxname)
            wmemcpy(boxname, proc->boxname, BOXNAME_COUNT);

        found = TRUE;
    }

    ExReleaseResourceLite(Process_ListLock);
    KeLowerIrql(irql);

    return found;
}


//---------------------------------------------------------------------------
// Process_MatchForceChildrenRule
//---------------------------------------------------------------------------


_FX BOOLEAN Process_MatchForceChildrenRule(
    BOX *box, const WCHAR *parent_name, const WCHAR *parent_path,
    BOOLEAN *outHasPriority, LONG *outPriority)
{
    LIST boxes;
    FORCE_BOX *force_box;
    BOOLEAN matched = FALSE;

    if (outHasPriority)
        *outHasPriority = FALSE;
    if (outPriority)
        *outPriority = -1;

    if (!box || !box->sid || !parent_name || !*parent_name || !parent_path || !*parent_path)
        return FALSE;

    Process_CreateForceData(&boxes, box->sid, box->session_id);

    force_box = List_Head(&boxes);
    while (force_box) {
        if (force_box->box->name_len == box->name_len
                && _wcsicmp(force_box->box->name, box->name) == 0) {
            matched = Process_CheckForceProcessList(
                force_box->box,
                &force_box->ForceChildren,
                parent_name,
                parent_path,
                outHasPriority,
                outPriority);
            break;
        }

        force_box = List_Next(force_box);
    }

    Process_DeleteForceData(&boxes);
    return matched;
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

    adsPath_len = (wcslen(path) + 1) * sizeof(WCHAR) + sizeof(ads_suffix);
    adsPath = Mem_Alloc(Driver_Pool, adsPath_len);
    if (!adsPath)
        return FALSE;
    wcscpy(adsPath, path);
    wcscat(adsPath, ads_suffix);
    
    RtlInitUnicodeString(&uni, adsPath);
    InitializeObjectAttributes(&objattrs,
        &uni, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, NULL, NULL);

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