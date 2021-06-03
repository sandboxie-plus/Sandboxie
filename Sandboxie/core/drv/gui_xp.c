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
// GUI Services, 32-bit Windows XP hooks
//---------------------------------------------------------------------------


#include "hook.h"
#include "session.h"
#include "dll.h"
#include "process.h"
#include "ipc.h"
#include "common/pattern.h"


//---------------------------------------------------------------------------
// Defines
//---------------------------------------------------------------------------


#undef  PRINT_SERVICES

#define GUI_HOOK_FAILED         0x10000000


// --------------------------------------------------------------------------
// Function Prototypes for WIN32K
// --------------------------------------------------------------------------


typedef HANDLE (*P_NtUserQueryWindow)(
    void *hWnd, ULONG_PTR QueryClass);

typedef void *(*P_NtUserGetForegroundWindow)(void);

typedef ULONG_PTR (*P_NtUserPostThreadMessage)(
    ULONG idThread, ULONG Msg, void *wParam, void *lParam);

typedef ULONG_PTR (*P_NtUserSendInput)(
    ULONG_PTR nInputs, void *pInputs, ULONG_PTR cbInput);

typedef ULONG_PTR (*P_NtUserBlockInput)(
    ULONG_PTR fBlockIt);

typedef ULONG_PTR (*P_NtUserSetWindowsHookEx)(
    void *hModule, UNICODE_STRING *DllName, ULONG idThread,
    ULONG HookType, void *HookProc, ULONG_PTR Flags);

typedef ULONG_PTR (*P_NtUserSetWinEventHook)(
    ULONG EventMin, ULONG EventMax, void *hModule, UNICODE_STRING *DllName,
    void *HookProc, ULONG idProcess, ULONG idThread, ULONG Flags);


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------


static BOOLEAN Gui_Init_XpHook(void);

static NTSTATUS Gui_Api_Init_XpHook(PROCESS *proc, ULONG64 *parms);

static void Gui_Unload_XpHook(void);

static BOOLEAN Gui_HookService(
    void **pSourceFunc, ULONG_PTR *pJumpStub, ULONG_PTR TargetFunc);


//---------------------------------------------------------------------------


static ULONG_PTR Gui_NtUserPostThreadMessage(
    ULONG idThread, ULONG Msg, void *wParam, void *lParam);

static ULONG_PTR Gui_NtUserSendInput(
    ULONG_PTR nInputs, void *pInputs, ULONG_PTR cbInput);

static ULONG_PTR Gui_NtUserBlockInput(ULONG_PTR BlockIt);

static ULONG_PTR Gui_NtUserSetWindowsHookEx(
    void *hModule, UNICODE_STRING *DllName, ULONG idThread,
    ULONG HookType, void *HookProc, ULONG_PTR Flags);

static ULONG_PTR Gui_NtUserSetWinEventHook(
    ULONG EventMin, ULONG EventMax, void *hModule, UNICODE_STRING *DllName,
    void *HookProc, ULONG idProcess, ULONG idThread, ULONG Flags);



//---------------------------------------------------------------------------
// Variables
//---------------------------------------------------------------------------


volatile ULONG Gui_ThreadModifyCount = 0;

static volatile ULONG Gui_HookCount = 0;


//---------------------------------------------------------------------------


static ULONG                        __idx_NtUserQueryWindow         = 0;
static ULONG                        __idx_NtUserGetForegroundWindow = 0;

static P_NtUserQueryWindow          __sys_NtUserQueryWindow         = NULL;
static P_NtUserGetForegroundWindow  __sys_NtUserGetForegroundWindow = NULL;


//---------------------------------------------------------------------------


static ULONG                        __idx_NtUserPostThreadMessage     = 0;
static ULONG                        __idx_NtUserSendInput             = 0;
static ULONG                        __idx_NtUserBlockInput            = 0;
static ULONG                        __idx_NtUserSetWindowsHookEx      = 0;
static ULONG                        __idx_NtUserSetWinEventHook       = 0;

static P_NtUserPostThreadMessage    __sys_NtUserPostThreadMessage     = NULL;
static P_NtUserSendInput            __sys_NtUserSendInput             = NULL;
static P_NtUserBlockInput           __sys_NtUserBlockInput            = NULL;
static P_NtUserSetWindowsHookEx     __sys_NtUserSetWindowsHookEx      = NULL;
static P_NtUserSetWinEventHook      __sys_NtUserSetWinEventHook       = NULL;

static ULONG_PTR                    __jmp_NtUserPostThreadMessage     = 0;
static ULONG_PTR                    __jmp_NtUserSendInput             = 0;
static ULONG_PTR                    __jmp_NtUserBlockInput            = 0;
static ULONG_PTR                    __jmp_NtUserSetWindowsHookEx      = 0;
static ULONG_PTR                    __jmp_NtUserSetWinEventHook       = 0;


//---------------------------------------------------------------------------


static ULONG                        __idx_NtRequestPort               = 0;
static ULONG                        __idx_NtRequestWaitReplyPort      = 0;

       P_NtRequestPort              __sys_NtRequestPort               = NULL;
       P_NtRequestWaitReplyPort     __sys_NtRequestWaitReplyPort      = NULL;

static ULONG_PTR                    __jmp_NtRequestPort               = 0;
static ULONG_PTR                    __jmp_NtRequestWaitReplyPort      = 0;


//---------------------------------------------------------------------------


#ifdef ALLOC_PRAGMA
#pragma alloc_text (INIT, Gui_Init_XpHook)
#endif // ALLOC_PRAGMA


//---------------------------------------------------------------------------
//
//
// GUI Initialization
//
//
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
// FIND_SERVICE
//---------------------------------------------------------------------------


#define FIND_SERVICE_X(dllname,svc,range)                       \
    {                                                           \
    static const char *ProcName = #svc;                         \
    ptr = Dll_GetProc(dllname, ProcName, FALSE);                \
    if (! ptr)                                                  \
        return FALSE;                                           \
    svc_num = Hook_GetServiceIndex(ptr, SkipIndexes);           \
    if (svc_num <= 0 || (svc_num & 0xF000) != range) {          \
        RtlStringCbPrintfW(err_txt, 128*sizeof(WCHAR), L"%s.%S", dllname, ProcName);         \
        Log_Msg1(MSG_1108, err_txt);                            \
        return FALSE;                                           \
    }                                                           \
    }


#define FIND_SERVICE(svc) FIND_SERVICE_X(Dll_USER,svc,0x1000)

#define FIND_SERVICE_NTDLL(svc) FIND_SERVICE_X(Dll_NTDLL,svc,0)


//---------------------------------------------------------------------------
// SKIP_SERVICE
//---------------------------------------------------------------------------


#define SKIP_SERVICE(svc)                                       \
    FIND_SERVICE(svc)                                           \
    ++SkipIndexes[0];                                           \
    SkipIndexes[SkipIndexes[0]] = svc_num;


//---------------------------------------------------------------------------
// Gui_Init_XpHook
//---------------------------------------------------------------------------


_FX BOOLEAN Gui_Init_XpHook(void)
{
    LONG SkipIndexes[32];
    LONG svc_num;
    void *ptr;
    WCHAR err_txt[128];

    //
    // In NTDLL, NtXxx routines are exported and make it easily possible to
    // get the associated service index.  In USER32, NtUserXxx routines
    // have the same 1:1 relationship with kernel-mode services, but
    // NtUserXxx are not exported.  They are indirectly used by public APIs
    // such as SendMessage, which invoke any number of NtUserXxx services.
    //
    // To extract the real kernel-mode counterpart for SendMessage, we first
    // have to analyze simple public APIs that use just one NtUserXxx call.
    // Later we can analyze more complex APIs such as SendMessage, and look
    // for NtUserXxx services that we don't know already.  Those will have
    // to be the kernel-mode counterpart for SendMessage-like functions.
    //

    SkipIndexes[0] = 0;

    SKIP_SERVICE(IsHungAppWindow);              // NtUserQueryWindow
    SKIP_SERVICE(CreateMenu);                   // NtUserCallNoParam
    SKIP_SERVICE(ShowCursor);                   // NtUserCallOneParam
    SKIP_SERVICE(IsWindow);                     // NtUserValidateHandleSecure
    SKIP_SERVICE(GetFocus);                     // NtUserGetThreadState
    SKIP_SERVICE(GetDoubleClickTime);           // NtUserGetDoubleClickTime

    //
    // save indexes for utility services that we are going to need
    //

    __idx_NtUserQueryWindow = SkipIndexes[1];

    FIND_SERVICE(GetForegroundWindow);
    __idx_NtUserGetForegroundWindow = svc_num;

    //
    // find addresses of Windows API routines which we need to hook
    // on Windows XP because they cannot be restricted by use of a
    // job object
    //
    // note that on Windows Vista and later, these routines are subject
    // to UIPI restrictions, so we use a primary process token with an
    // untrusted integrity level instead of hooking
    //

    FIND_SERVICE(PostThreadMessageW);
    __idx_NtUserPostThreadMessage = svc_num;

    FIND_SERVICE(SendInput);
    __idx_NtUserSendInput = svc_num;

    FIND_SERVICE(BlockInput);
    __idx_NtUserBlockInput = svc_num;

    FIND_SERVICE(SetWindowsHookExW);
    __idx_NtUserSetWindowsHookEx = svc_num;

    FIND_SERVICE(SetWinEventHook);
    __idx_NtUserSetWinEventHook = svc_num;

    //
    // get syscall indexes for services in NTOSKRNL
    //

    FIND_SERVICE_NTDLL(NtRequestPort);
    __idx_NtRequestPort = svc_num;

    FIND_SERVICE_NTDLL(NtRequestWaitReplyPort);
    __idx_NtRequestWaitReplyPort = svc_num;

    //
    // print indexes
    //

#ifdef PRINT_SERVICES

    DbgPrint("__idx_NtUserQueryWindow = %X\n", __idx_NtUserQueryWindow);
    DbgPrint("__idx_NtUserGetForegroundWindow = %X\n", __idx_NtUserGetForegroundWindow);

    DbgPrint("__idx_NtUserPostThreadMessage = %X\n", __idx_NtUserPostThreadMessage);
    DbgPrint("__idx_NtUserSendInput = %X\n", __idx_NtUserSendInput);
    DbgPrint("__idx_NtUserBlockInput = %X\n", __idx_NtUserBlockInput);
    DbgPrint("__idx_NtUserSetWindowsHookEx = %X\n", __idx_NtUserSetWindowsHookEx);
    DbgPrint("__idx_NtUserSetWinEventHook = %X\n", __idx_NtUserSetWinEventHook);

    DbgPrint("__idx_NtRequestPort = %X\n", __idx_NtRequestPort);
    DbgPrint("__idx_NtRequestWaitReplyPort = %X\n", __idx_NtRequestWaitReplyPort);

#endif PRINT_SERVICES

    return TRUE;
}


//---------------------------------------------------------------------------
// GET_SERVICE
//---------------------------------------------------------------------------


#define GET_SERVICE(svc,prmcnt)                                 \
    if (ok) {                                                   \
    if (! Hook_GetService(                                      \
            (void *)(ULONG_PTR)__idx_##svc, NULL, prmcnt,       \
            (void **)&__sys_##svc, NULL)) {                     \
        Log_Msg1(MSG_GUI_SERVICE_MISMATCH, L#svc);              \
        ok = FALSE;                                             \
    }                                                           \
    }


//---------------------------------------------------------------------------
// HOOK_SERVICE
//---------------------------------------------------------------------------


#define HOOK_SERVICE_X(pfx,svc)                                             \
    if (ok) {                                                               \
    if (! Gui_HookService(                                                  \
            (void **)&__sys_##svc, &__jmp_##svc, (ULONG_PTR)pfx##svc)) {    \
        Log_Msg1(MSG_GUI_SERVICE_HOOK, L#svc);                              \
        ok = FALSE;                                                         \
    }                                                                       \
    }


#define HOOK_SERVICE(svc) HOOK_SERVICE_X(Gui_,svc)

#define HOOK_SERVICE_IPC(svc) HOOK_SERVICE_X(Ipc_,svc)


//---------------------------------------------------------------------------
// Gui_Api_Init_XpHook
//---------------------------------------------------------------------------


_FX NTSTATUS Gui_Api_Init_XpHook(PROCESS *proc, ULONG64 *parms)
{
    BOOLEAN ok = TRUE;

    //
    // hook the GUI just once
    //

    if (Gui_HookCount) {
        if (Gui_HookCount & GUI_HOOK_FAILED)
            return STATUS_UNSUCCESSFUL;
        return STATUS_SUCCESS;
    }

    if (InterlockedIncrement(&Gui_HookCount) != 1) {
        if (Gui_HookCount & GUI_HOOK_FAILED)
            return STATUS_UNSUCCESSFUL;
        return STATUS_SUCCESS;
    }

    //
    // get the kernel mode addresses (in session space) for the
    // service index numbers we collected in Gui_Init
    //

    GET_SERVICE(NtUserQueryWindow, 2);
    GET_SERVICE(NtUserGetForegroundWindow, 0);

    GET_SERVICE(NtUserPostThreadMessage, 4);
    GET_SERVICE(NtUserSendInput, 3);
    GET_SERVICE(NtUserBlockInput, 1);
    GET_SERVICE(NtUserSetWindowsHookEx, 6);
    GET_SERVICE(NtUserSetWinEventHook, 8);

    GET_SERVICE(NtRequestPort, 2);
    GET_SERVICE(NtRequestWaitReplyPort, 3);

#ifdef PRINT_SERVICES

    DbgPrint("---------------------------------------\n");
    DbgPrint("__sys_NtUserQueryWindow = %p\n", __sys_NtUserQueryWindow);
    DbgPrint("__sys_NtUserGetForegroundWindow = %p\n", __sys_NtUserGetForegroundWindow);

    DbgPrint("__sys_NtUserPostThreadMessage = %p\n", __sys_NtUserPostThreadMessage);
    DbgPrint("__sys_NtUserSendInput = %p\n", __sys_NtUserSendInput);
    DbgPrint("__sys_NtUserBlockInput = %p\n", __sys_NtUserBlockInput);
    DbgPrint("__sys_NtUserSetWindowsHookEx = %p\n", __sys_NtUserSetWindowsHookEx);
    DbgPrint("__sys_NtUserSetWinEventHook = %p\n", __sys_NtUserSetWinEventHook);

    DbgPrint("__sys_NtRequestPort = %p\n", __sys_NtRequestPort);
    DbgPrint("__sys_NtRequestWaitReplyPort = %p\n", __sys_NtRequestWaitReplyPort);

#endif

    if (! ok) {
        InterlockedExchange(&Gui_HookCount, Gui_HookCount | GUI_HOOK_FAILED);
        Log_MsgP0(MSG_GUI_INIT_FAILED, proc->pid);
        return STATUS_UNSUCCESSFUL;
    }

    //
    // hook the kernel services in WIN32K
    //

    HOOK_SERVICE(NtUserPostThreadMessage);
    HOOK_SERVICE(NtUserSendInput);
    HOOK_SERVICE(NtUserBlockInput);
    HOOK_SERVICE(NtUserSetWindowsHookEx);
    HOOK_SERVICE(NtUserSetWinEventHook);

    //
    // hook the kernel services in NTOSKRNL
    //

    HOOK_SERVICE_IPC(NtRequestPort);
    HOOK_SERVICE_IPC(NtRequestWaitReplyPort);

    //
    // enable sandboxing and finish
    //

    if (! ok) {
        InterlockedExchange(&Gui_HookCount, Gui_HookCount | GUI_HOOK_FAILED);
        Log_MsgP0(MSG_GUI_INIT_FAILED, proc->pid);
        return STATUS_UNSUCCESSFUL;
    }

    return STATUS_SUCCESS;
}


//---------------------------------------------------------------------------
// Gui_Unload_XpHook
//---------------------------------------------------------------------------


_FX void Gui_Unload_XpHook(void)
{
    //
    // this function may be called through an API_UNLOAD_DRIVER request
    // even before an API_GUI_INIT request, so we tell Gui_Api_Init
    // to not initialize, and if it's already initializing, then we
    // turn on GUI_HOOK_FAILED to abort that init
    //

    InterlockedIncrement(&Gui_HookCount);
    InterlockedExchange(&Gui_HookCount, Gui_HookCount | GUI_HOOK_FAILED);

    //
    // disable our hook entry code so it always jumps back to the
    // original system procedures
    //

#define UNHOOK_SERVICE(svc)                                     \
    if (__sys_##svc && __jmp_##svc) {                           \
        Process_DisableHookEntry(__jmp_##svc);                  \
        __jmp_##svc = 0;                                     \
    }

    UNHOOK_SERVICE(NtUserPostThreadMessage);
    UNHOOK_SERVICE(NtUserSendInput);
    UNHOOK_SERVICE(NtUserBlockInput);
    UNHOOK_SERVICE(NtUserSetWindowsHookEx);
    UNHOOK_SERVICE(NtUserSetWinEventHook);

    //
    // disable our hooks for kernel services in NTOSKRNL
    //

    UNHOOK_SERVICE(NtRequestPort);
    UNHOOK_SERVICE(NtRequestWaitReplyPort);

#undef  UNHOOK_SEVICE
}


//---------------------------------------------------------------------------
//
//
// GUI hook logic.  This deals with modifying WIN32K kernel code to branch
// to our Gui_NtUserXxx functions below.  A JMP instruction (which replaces
// more than one instruction) is placed at the beginning of each hooked
// function.  We must make sure no other thread in the system is executing
// at or near the code we are about to modify.  To accomplish this, we
// schedule DPC routines to keep all other processors busy-looping.
// We only actually modify, if all Win32 threads are in a Waiting state.
//
//
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
// Structures and Types
//---------------------------------------------------------------------------


typedef struct _GUI_HOOKSERVICE_CONTEXT {

    volatile LONG NumRaised;
    volatile LONG AllRaised;
    NTSTATUS status;
    void *SourceAddr;
    void *TargetAddr;
    void *SaveBytesAddr;
    UCHAR SaveBytes[24];
    ULONG ThreadCount;
    PETHREAD *Threads;
    KDPC *dpcs[MAXIMUM_PROCESSORS];

} GUI_HOOKSERVICE_CONTEXT;


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------


static NTSTATUS Gui_HookSaveThreads(GUI_HOOKSERVICE_CONTEXT *context);

static NTSTATUS Gui_HookCheckThreads(GUI_HOOKSERVICE_CONTEXT *context);

static NTSTATUS Gui_HookFreeze(GUI_HOOKSERVICE_CONTEXT *context, char op);

static void Gui_HookFreezeDpc(
    KDPC *dpc, GUI_HOOKSERVICE_CONTEXT *context,
    void *unused1, void *unused2);


//---------------------------------------------------------------------------
// Gui_HookService
//---------------------------------------------------------------------------


_FX BOOLEAN Gui_HookService(
    void **pSourceFunc, ULONG_PTR *pJumpStub, ULONG_PTR TargetFunc)
{
    NTSTATUS status;
    void *Trampoline;
    GUI_HOOKSERVICE_CONTEXT *context;
    ULONG retries;
    KIRQL irql;
    PMDL mdl;
    UCHAR *WriteAddr;
    ULONG push_jmp_target = 0;
    ULONG SaveBytesLen;

    Trampoline = NULL;

    context = ExAllocatePoolWithTag(
        NonPagedPool, sizeof(GUI_HOOKSERVICE_CONTEXT), tzuk);
    if (! context) {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto finish;
    }

    memzero(context, sizeof(GUI_HOOKSERVICE_CONTEXT));

    context->SaveBytesAddr = *pSourceFunc;
    if (! context->SaveBytesAddr) {
        status = STATUS_INVALID_PARAMETER;
        goto finish;
    }

    status = Gui_HookFreeze(context, 'i');
    if (! NT_SUCCESS(status))
        goto finish;

    retries = 0;

    //
    // main loop.  in most cases the hook can be completed in one iteration
    //

retry:

    if (Gui_HookCount & GUI_HOOK_FAILED) {
        status = STATUS_REQUEST_ABORTED;
        goto finish;
    }

    if (retries >= 600)
        goto finish;

    if (retries != 0) {

        LARGE_INTEGER time;
        time.QuadPart = -(SECONDS(1) / 2);
        KeDelayExecutionThread(KernelMode, FALSE, &time);

        if (Gui_HookCount & GUI_HOOK_FAILED) {
            status = STATUS_REQUEST_ABORTED;
            goto finish;
        }
    }

    ++retries;

    //
    // normally we copy and compare the sizeof(SaveBytes) at the code to
    // hook.  but if the first instruction is JMP then our code inspection
    // routines will not go past 5 bytes.  in this case we can copy and
    // compare only 5 bytes, and avoid a potential conflict with Avira
    // driver avipbb.sys which sometimes puts a JMP instruction at the end
    // of a 4K page that has no following page
    //

    SaveBytesLen = sizeof(context->SaveBytes);

    if (*(UCHAR *)context->SaveBytesAddr == 0xE9)
        SaveBytesLen = 5;

    //
    // first thing, keep a copy of the original top of function.
    // before we actually activate our detour, we will want to
    // make sure that the top of function is still the same,
    // i.e., that the trampoline we built is still valid
    //

    memcpy(context->SaveBytes, context->SaveBytesAddr, SaveBytesLen);

    //
    // collect Win32 threads into our hook context
    //

    status = Gui_HookSaveThreads(context);
    if (! NT_SUCCESS(status))
        goto finish;

    //
    // if the Source function begins with 'mov edi,edi', which is
    // a place holder for a potential hot fix, leave it as it is,
    // and hook the following address.  also if it begins with
    // 'jmp short offset', which indicates a hot fix or someone
    // else has already hooked it. hopefully, the other hook
    // eventually returns to SourceAddr + 2
    //

    context->SourceAddr = context->SaveBytesAddr;

    if (*(USHORT *)context->SourceAddr == 0xFF8B ||     // mov edi, edi
        *(USHORT *)context->SourceAddr == 0xC08B ||     // mov eax, eax
        *(UCHAR *)context->SourceAddr == 0xEB ||        // jmp short
        *(UCHAR *)context->SourceAddr == 0xE8) {        // call (TrendMicro)

        UCHAR SpySweeperProlog[] = {
            0x8B, 0xFF,                                 // mov edi, edi
            0x90,                                       // nop
            0x8B, 0xFF,                                 // mov edi, edi
            0xE8, 0x00, 0x00, 0x00, 0x00                // call next
        };

        UCHAR TrendMicroProlog[] = {
            0xE8, 0x00, 0x00, 0x00, 0x00,               // call next
            0x50,                                       // push eax
            0x53,                                       // push ebx
            0x8B, 0x44, 0x24, 0x08,                     // mov eax, [esp+08]
            0x8B, 0x5C, 0x24, 0x0C                      // mov ebx, [esp+0C]
        };

        if (memcmp(context->SourceAddr, SpySweeperProlog,
                   sizeof(SpySweeperProlog)) == 0) {

            //
            // replace 'call next' with 'jump to our jump stub',
            // and indicate that all our trampoline has to do
            // is 'push next'
            //

            push_jmp_target = (ULONG)((UCHAR *)context->SourceAddr + 10);
            context->SourceAddr = (UCHAR *)context->SourceAddr + 5;

        } else if (memcmp(context->SourceAddr, TrendMicroProlog,
                          sizeof(TrendMicroProlog)) == 0) {

            // conceptually same as above, except that there is no need
            // to advance the SourceAddr because the CALL is at offset 0

            push_jmp_target = (ULONG)((UCHAR *)context->SourceAddr + 5);

        } else if (*(UCHAR *)context->SourceAddr != 0xE8) {

            //
            // regular hotfix, advance hook
            //

            context->SourceAddr =
                (void *)((ULONG_PTR)context->SourceAddr + 2);
        }
    }

    //
    // get a trampoline if we don't have one yet
    //

    if (Trampoline) {

        //
        // if we already built a trampoline once, then the returned
        // address was to &tramp->code (see Hook_BuildTramp), which
        // is 16 bytes past the allocated trampoline
        //

        Trampoline = (void *)HOOK_TRAMP_CODE_TO_TRAMP_HEAD(Trampoline);
    }

    Trampoline = Hook_BuildTramp(
                        context->SourceAddr, Trampoline, FALSE, FALSE);

    //
    // if we detected SpySweeper hooks, we need a special trampoline
    // which does push target, jmp target
    //

    if (push_jmp_target) {

        UCHAR *tr = (UCHAR *)Trampoline;
        *(UCHAR *)(tr + 0) = 0x68;              // push target
        *(ULONG *)(tr + 1) = push_jmp_target;
        Hook_BuildJump(tr + 5, tr + 5, (void *)push_jmp_target);
    }

    if (! Trampoline) {
        status = STATUS_UNSUCCESSFUL;
        goto finish;
    }

    //
    // update the __sys_Xxx pointer so if the hook is successful,
    // our Gui_Xxx hook can use the right pointer as soon as possible
    //

    *pSourceFunc = Trampoline;

    //
    // build a hook entry that jumps to OldProc (Trampoline) or
    // NewProc (TargetFunc)
    //

    if (! *pJumpStub) {

        *pJumpStub = Process_BuildHookEntry(
            TargetFunc, (ULONG_PTR)Trampoline, NULL);
        if (! *pJumpStub) {
            status = STATUS_UNSUCCESSFUL;
            goto finish;
        }

        context->TargetAddr = (void *)*pJumpStub;
    }

    //
    // compare the top of function against our saved bytes.  this means
    // we can abort early if a change has already been detected, and it
    // also touches the (possibly pageable) function page, just before
    // we raise to DPC_LEVEL and lose access to non-resident pages
    //

    if (0 != memcmp(context->SaveBytes,
                    context->SaveBytesAddr,
                    SaveBytesLen))
        goto retry;

    //
    // lock the page(s) containing the top of the function and
    // get a writable address for the executable/read-only address
    //

    mdl = IoAllocateMdl(context->SaveBytesAddr, 32, FALSE, FALSE, NULL);
    if (! mdl) {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto finish;
    }

    __try {
        MmProbeAndLockPages(mdl, KernelMode, IoModifyAccess);
        status = STATUS_SUCCESS;
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        status = GetExceptionCode();
    }

    if (! NT_SUCCESS(status)) {
        IoFreeMdl(mdl);
        goto finish;
    }

    WriteAddr = MmGetSystemAddressForMdlSafe(mdl, NormalPagePriority);
    if (WriteAddr) {
        WriteAddr += (UCHAR *)context->SourceAddr
                   - (UCHAR *)context->SaveBytesAddr;
    } else
        WriteAddr = context->SourceAddr;

    //
    // raise to DISPATCH_LEVEL and keep other processors busy
    //

    KeMemoryBarrier();

    KeRaiseIrql(DISPATCH_LEVEL, &irql);

    status = Gui_HookFreeze(context, 'f');
    if (NT_SUCCESS(status)) {

        //
        // now that we're the only thing running in the system (except
        // for busy-loops on other processors), make sure there we did
        // not stop any Win32 threads that were in a non-Waiting state
        //

        if (Gui_HookCount & GUI_HOOK_FAILED)
            status = STATUS_REQUEST_ABORTED;
        else
            status = Gui_HookCheckThreads(context);

        if (NT_SUCCESS(status)) {

            //
            // make sure (again) that the original bytes are same
            //

            if (0 == memcmp(context->SaveBytes,
                            context->SaveBytesAddr,
                            SaveBytesLen)) {

                //
                // all is well -- instrument the function
                //

                Hook_BuildJump(
                    WriteAddr, context->SourceAddr, context->TargetAddr);

                status = STATUS_SUCCESS;

            } else
                status = STATUS_RETRY;
        }

        //
        // release other processors
        //

        Gui_HookFreeze(context, 't');
    }

    //
    // restore irql, unlock page(s), and possibly retry
    //

    KeLowerIrql(irql);

    MmUnlockPages(mdl);
    IoFreeMdl(mdl);

    if (status == STATUS_RETRY)
        goto retry;

    //
    // release resources
    //

finish:

    if (context) {

        Gui_HookFreeze(context, 'u');

        if (context->Threads)
            ExFreePoolWithTag(context->Threads, tzuk);

        ExFreePoolWithTag(context, tzuk);
    }

    if (! NT_SUCCESS(status)) {

        Log_Status(MSG_GUI_HOOK_ERROR, 0, status);
        *pJumpStub = 0;
        *pSourceFunc = context->SaveBytesAddr;

        return FALSE;
    }

    return TRUE;
}


//---------------------------------------------------------------------------
// Gui_HookSaveThreads
//---------------------------------------------------------------------------


_FX NTSTATUS Gui_HookSaveThreads(GUI_HOOKSERVICE_CONTEXT *context)
{
    NTSTATUS status;
    SYSTEM_PROCESS_INFORMATION *info;
    ULONG len, dummy_len;
    void *buf;
    ULONG AllThreadCount;

    //
    // reset the thread modify count so we know if any changes were made
    // to threads (most important would be creating or deleting threads)
    //

    InterlockedExchange(&Gui_ThreadModifyCount, 0);

    //
    // collect information about all threads in the system
    //

    len = 0;
    while (1) {

        len += PAGE_SIZE * 2;
        buf = ExAllocatePoolWithTag(PagedPool, len, tzuk);
        if (! buf) {
            status = STATUS_INSUFFICIENT_RESOURCES;
            break;
        }

        dummy_len = 0;
        status = ZwQuerySystemInformation(
            SystemProcessesAndThreadsInformation, buf, len, &dummy_len);
        if (NT_SUCCESS(status))
            break;

        ExFreePoolWithTag(buf, tzuk);
        if (status != STATUS_INFO_LENGTH_MISMATCH)
            break;
    }

    if (! NT_SUCCESS(status))
        return status;

    //
    // count the number of all threads
    //

    AllThreadCount = 0;

    info = (SYSTEM_PROCESS_INFORMATION *)buf;
    while (1) {

        AllThreadCount += info->ThreadCount;

        if (! info->NextEntryOffset)
            break;
        info = (SYSTEM_PROCESS_INFORMATION *)
                                    ((UCHAR *)info + info->NextEntryOffset);
    }

    if (! AllThreadCount) {
        ExFreePoolWithTag(buf, tzuk);
        return STATUS_THREAD_NOT_IN_PROCESS;
    }

    //
    // allocate a buffer large enough to hold pointers to all threads
    //

    if (context->Threads)
        ExFreePoolWithTag(context->Threads, tzuk);

    context->Threads = ExAllocatePoolWithTag(
        NonPagedPool, sizeof(ULONG_PTR) * AllThreadCount, tzuk);
    if (! context->Threads) {
        ExFreePoolWithTag(buf, tzuk);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    //
    // scan all threads again, and record each Win32 thread in the buffer
    //

    context->ThreadCount = 0;

    info = (SYSTEM_PROCESS_INFORMATION *)buf;
    while (1) {

        while (info->ThreadCount) {

            PETHREAD Thread;
            HANDLE idThread =
                info->Threads[info->ThreadCount - 1].ClientId.UniqueThread;
            status = PsLookupThreadByThreadId(idThread, &Thread);

            if (NT_SUCCESS(status)) {

                ULONG_PTR Win32Thread = PsGetThreadWin32Thread(Thread);

                if (Win32Thread && (void *)Thread != KeGetCurrentThread()) {

                    context->Threads[context->ThreadCount] = Thread;
                    ++context->ThreadCount;
                }

                ObDereferenceObject(Thread);
            }

            --info->ThreadCount;
        }

        if (! info->NextEntryOffset)
            break;
        info = (SYSTEM_PROCESS_INFORMATION *)
                                    ((UCHAR *)info + info->NextEntryOffset);
    }

    //
    // release storage
    //

    ExFreePoolWithTag(buf, tzuk);

    if (! context->ThreadCount)
        return STATUS_THREAD_NOT_IN_PROCESS;

    return STATUS_SUCCESS;
}


//---------------------------------------------------------------------------
// Gui_HookCheckThreads
//---------------------------------------------------------------------------


_FX NTSTATUS Gui_HookCheckThreads(GUI_HOOKSERVICE_CONTEXT *context)
{
    NTSTATUS status;
    ULONG ThreadStateOffset;
    const UCHAR Ready = 1;              // for KThread.State
    const UCHAR Waiting = 5;            // for KThread.State
    const UCHAR MaxThreadState = 8;     // for KThread.State
    ULONG i;

    //
    // if any threads have been created since we collected information
    // about threads, then we no longer have an up-to-date image of
    // thread status
    //
    // if any threads have been deleted since we collected information,
    // then we may get a page fault when looking at ETHREAD structures
    //
    // in either case, we need to retry
    //
    // note that this code runs in a DPC on a particular process and
    // all other processors are tied up with other busy-looping DPCs,
    // so once we assert here that no thread has been modified, then
    // no other threads can be modified until the DPC ends
    //

    if (InterlockedOr(&Gui_ThreadModifyCount, 0))
        return STATUS_RETRY;

    //
    // check the status of each Win32 thread recorded earlier.  if any
    // of those is not in the thread Waiting state, then it may be running
    // at or near the code we're about to change (unlikely but possible)
    //

    if (Driver_OsVersion == DRIVER_WINDOWS_2003)
        ThreadStateOffset = 0x4C;
    else    // DRIVER_WINDOWS_2K || DRIVER_WINDOWS_XP
        ThreadStateOffset = 0x2D;

    status = STATUS_SUCCESS;

    for (i = 0; i < context->ThreadCount; ++i) {
        UCHAR *Thread = (UCHAR *)context->Threads[i];
        UCHAR *State = Thread + ThreadStateOffset;
        if (*State != Waiting && *State != Ready) {
            status = STATUS_RETRY;
            if (*State > MaxThreadState) {
                status = STATUS_UNKNOWN_REVISION | (((ULONG)*State) << 20);
                break;
            }
        }
    }

    return status;
}


//---------------------------------------------------------------------------
// Gui_HookFreeze
//---------------------------------------------------------------------------


_FX NTSTATUS Gui_HookFreeze(GUI_HOOKSERVICE_CONTEXT *context, char op)
{
    KAFFINITY mask;
    LONG num, i, cur_proc;

    //
    // op == i:  init storage for each DPC on each processor
    //

    if (op == 'i') {

        mask = KeQueryActiveProcessors();
        if (! mask)
            return STATUS_MP_PROCESSOR_MISMATCH;

        i = 0;
        while (mask) {
            if (mask & 1) {

                //
                // allocate the DPC entry for processor 'i' in index 'i'
                //

                context->dpcs[i] =
                    ExAllocatePoolWithTag(NonPagedPool, sizeof(KDPC), tzuk);
                if (! context->dpcs[i])
                    return STATUS_INSUFFICIENT_RESOURCES;
            }

            mask >>= 1;
            ++i;
        }

        return STATUS_SUCCESS;
    }

    //
    // op == u:  un-initialize by freeing storage
    //

    else if (op == 'u') {

        for (i = 0; i < MAXIMUM_PROCESSORS; ++i) {
            if (context->dpcs[i])
                ExFreePoolWithTag(context->dpcs[i], tzuk);
        }

        return STATUS_SUCCESS;
    }

    //
    // for op == f and op == t, we need to count processors
    //

    cur_proc = KeGetCurrentProcessorNumber();

    num = 0;
    for (i = 0; i < MAXIMUM_PROCESSORS; ++i) {
        if (context->dpcs[i] && i != cur_proc)
            ++num;
    }

    if (num == 0)
        return STATUS_SUCCESS;

    //
    // op == f:  freeze other processors by scheduling busy-loop DPCs
    //

    if (op == 'f') {

        InterlockedAnd(&context->NumRaised, 0);
        InterlockedAnd(&context->AllRaised, 0);

        for (i = 0; i < MAXIMUM_PROCESSORS; ++i) {
            if (context->dpcs[i] && i != cur_proc) {

                KeInitializeDpc(
                    context->dpcs[i], Gui_HookFreezeDpc, context);
                KeSetImportanceDpc(context->dpcs[i], HighImportance);
                KeSetTargetProcessorDpc(context->dpcs[i], (UCHAR)i);

                KeInsertQueueDpc(context->dpcs[i], NULL, NULL);
            }
        }

        while (InterlockedCompareExchange(
                    &context->NumRaised, num, num) != num) {
            for (i = 0; i < 10; ++i) YieldProcessor();
        }
    }

    //
    // op == t:  thaw other processors by signalling DPCs to end
    //

    else if (op == 't') {

        InterlockedIncrement(&context->AllRaised);

        while (InterlockedCompareExchange(
                    &context->NumRaised, 0, 0) != 0) {
            for (i = 0; i < 10; ++i) YieldProcessor();
        }
    }

    return STATUS_SUCCESS;
}


//---------------------------------------------------------------------------
// Gui_HookFreezeDpc
//---------------------------------------------------------------------------


_FX void Gui_HookFreezeDpc(
    KDPC *dpc, GUI_HOOKSERVICE_CONTEXT *context,
    void *unused1, void *unused2)
{
    //
    // this routine is scheduled as a DPC on as many processors as we have,
    // minus one.  each DPC decrements a global counter, then waits for
    // the counter to go above 0x1000, at which point, it will increment
    // the counter, and end
    //

    ULONG i;

    InterlockedIncrement(&context->NumRaised);

    while (InterlockedCompareExchange(&context->AllRaised, 1, 1) == 0) {
        for (i = 0; i < 10; ++i) YieldProcessor();
    }

    InterlockedDecrement(&context->NumRaised);
}


//---------------------------------------------------------------------------
//
//
// GUI Hooks
//
//
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
// Win32k defines
//---------------------------------------------------------------------------


#define QUERYWINDOW_PROCESS             0

#define WH_JOURNALRECORD                0
#define WH_JOURNALPLAYBACK              1
#define WH_SYSMSGFILTER                 6
#define WH_KEYBOARD_LL                  13
#define WH_MOUSE_LL                     14

#define WINEVENT_INCONTEXT              0x0004

#define MAXINTATOM                      0xC000
#define ATOM_MASK                       ((LONG_PTR)0xFFFF)


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------


static NTSTATUS Gui_CheckBoxedThread(
    PROCESS *proc, ULONG idThread, ULONG *out_idProcess);


//---------------------------------------------------------------------------
// GUI_NTUSER_IF
//---------------------------------------------------------------------------


#define GUI_NTUSER_IF                                                   \
    PROCESS *proc = Process_GetCurrent();                               \
    if (proc == PROCESS_TERMINATED)                                     \
        return 0;                                                       \
    if (! proc->open_all_win_classes)


//---------------------------------------------------------------------------
// Gui_NtUserPostThreadMessage
//---------------------------------------------------------------------------


_FX ULONG_PTR Gui_NtUserPostThreadMessage(
    ULONG idThread, ULONG Msg, void *wParam, void *lParam)
{
    GUI_NTUSER_IF {

        NTSTATUS status;
        ULONG idProcess = 0;

        if (! idThread)
            idThread = (ULONG)(ULONG_PTR)PsGetCurrentThreadId();

        if ((Msg & ~ATOM_MASK) == 0 && Msg > MAXINTATOM)
            status = STATUS_SUCCESS;
        else if (proc->open_all_win_classes)
            status = STATUS_SUCCESS;
        else {
            status = Gui_CheckBoxedThread(proc, idThread, &idProcess);
            if (status == STATUS_ACCESS_DENIED)
                status = Process_CheckProcessName(
                            proc, &proc->open_win_classes, idProcess, NULL);
        }

        if (Session_MonitorCount) {

            void *nbuf;
            ULONG nlen;
            WCHAR *nptr;

            Process_GetProcessName(
                proc->pool, idProcess, &nbuf, &nlen, &nptr);
            if (nbuf) {

                ULONG mon_type = MONITOR_WINCLASS;
                if (NT_SUCCESS(status))
                    mon_type |= MONITOR_OPEN;
                else
                    mon_type |= MONITOR_DENY;

                --nptr; *nptr = L':';
                --nptr; *nptr = L'$';

                Session_MonitorPut(mon_type, nptr, proc->pid);

                Mem_Free(nbuf, nlen);
            }
        }

        if (! NT_SUCCESS(status)) {

            if (proc->gui_trace & TRACE_DENY) {

                WCHAR access_str[80];
                RtlStringCbPrintfW(access_str, sizeof(access_str),
                    L"(GD) ThrdMessage %05d (%04X) to tid=%06d    pid=%06d",
                    Msg, Msg, idThread, idProcess);
                Log_Debug_Msg(MONITOR_WINCLASS, access_str, Driver_Empty);
            }

            return 0;
        }
    }

    return __sys_NtUserPostThreadMessage(idThread, Msg, wParam, lParam);
}


//---------------------------------------------------------------------------
// Gui_NtUserSendInput
//---------------------------------------------------------------------------


_FX ULONG_PTR Gui_NtUserSendInput(
    ULONG_PTR nInputs, void *pInputs, ULONG_PTR cbInput)
{
    GUI_NTUSER_IF {

        //
        // we are going to pass the input to the system one INPUT structure
        // at a time, so we can check for change in the foreground window
        //

        ULONG retval = 0;
        while (nInputs && nInputs < 16) {

            BOOLEAN is_active_window_sandboxed = FALSE;
            void *active_hwnd = NULL;

            active_hwnd = __sys_NtUserGetForegroundWindow();
            if (active_hwnd) {
                PROCESS *proc2;
                HANDLE idProcess = __sys_NtUserQueryWindow(
                                        active_hwnd, QUERYWINDOW_PROCESS);
                if (idProcess)
                    proc2 = Process_Find(idProcess, NULL);
                else
                    proc2 = NULL;   // probably, window is outside job
                if (proc2)
                    is_active_window_sandboxed = TRUE;
            }

            //
            // if the foreground window is not sandboxed, stop
            //

            if (! is_active_window_sandboxed) {

                if (proc->block_fake_input_hwnd != active_hwnd) {
                    proc->block_fake_input_hwnd = active_hwnd;
                    //Log_Msg1(MSG_GUI_FAKE_INPUT, proc->image_name);
                    //Process_LogMessage(proc, MSG_GUI_FAKE_INPUT);
                }

                break;
            }

            //
            // otherwise pass one INPUT structure to the system, then advance
            // to the next INPUT structure.  note that we must get a return
            // of exactly 1
            //

            if (__sys_NtUserSendInput(1, pInputs, cbInput) != 1)
                break;

            ++retval;
            --nInputs;
            pInputs = (UCHAR *)pInputs + cbInput;
        }

        //
        // trace request
        //

        if (proc->gui_trace & (TRACE_ALLOW | TRACE_DENY)) {

            WCHAR access_str[24];
            WCHAR letter;

            if (retval == 0 && (proc->gui_trace & TRACE_DENY))
                letter = L'D';
            else if (retval != 0 && (proc->gui_trace & TRACE_ALLOW))
                letter = L'A';
            else
                letter = 0;

            if (letter) {

                RtlStringCbPrintfW(access_str, sizeof(access_str), L"(G%c) SendInput", letter);
                Log_Debug_Msg(MONITOR_WINCLASS | MONITOR_TRACE, access_str, Driver_Empty);
            }
        }

        //
        // return number of INPUT structures actually handled
        //

        return retval;
    }

    return __sys_NtUserSendInput(nInputs, pInputs, cbInput);
}


//---------------------------------------------------------------------------
// Gui_NtUserBlockInput
//---------------------------------------------------------------------------


_FX ULONG_PTR Gui_NtUserBlockInput(ULONG_PTR fBlockIt)
{
    GUI_NTUSER_IF {

        if (fBlockIt) {

            void *active_hwnd = __sys_NtUserGetForegroundWindow();

            if (proc->block_fake_input_hwnd != active_hwnd) {
                proc->block_fake_input_hwnd = active_hwnd;
                //Log_Msg1(MSG_GUI_FAKE_INPUT, proc->image_name);
                //Process_LogMessage(proc, MSG_GUI_FAKE_INPUT);
            }

            return 0;
        }
    }

    return __sys_NtUserBlockInput(fBlockIt);
}


//---------------------------------------------------------------------------
// Gui_NtUserSetWindowsHookEx
//---------------------------------------------------------------------------


_FX ULONG_PTR Gui_NtUserSetWindowsHookEx(
    void *hModule, UNICODE_STRING *DllName, ULONG idThread,
    ULONG HookType, void *HookProc, ULONG_PTR Flags)
{
    GUI_NTUSER_IF {

        //
        // the sandboxed process must specify a thread to be hooked.
        // this also means global-only hooks are not supported.
        //

        NTSTATUS status;
        ULONG idProcess = 0;

        if (HookType == WH_JOURNALRECORD || HookType == WH_JOURNALPLAYBACK ||
            HookType == WH_KEYBOARD_LL || HookType == WH_MOUSE_LL) {

            // MSDN explictly says these hooks are NOT injected into the target

            status = STATUS_SUCCESS;

        } else if (idThread && HookType != WH_SYSMSGFILTER) {

            status = Gui_CheckBoxedThread(proc, idThread, &idProcess);

        } else
            status = STATUS_ACCESS_DENIED;

        //
        // trace request
        //

        if (proc->gui_trace & (TRACE_ALLOW | TRACE_DENY)) {

            WCHAR access_str[64];
            WCHAR letter;

            if ((! NT_SUCCESS(status)) && (proc->gui_trace & TRACE_DENY))
                letter = L'D';
            else if (NT_SUCCESS(status) && (proc->gui_trace & TRACE_ALLOW))
                letter = L'A';
            else
                letter = 0;

            if (letter) {

                RtlStringCbPrintfW(access_str, sizeof(access_str),
                         L"(G%c) WinHook %04d on tid=%06d pid=%06d",
                         letter, HookType, idThread, idProcess);
                Log_Debug_Msg(MONITOR_WINCLASS | MONITOR_TRACE, access_str, Driver_Empty);
            }
        }

        if (! NT_SUCCESS(status))
            return 0;
    }

    return __sys_NtUserSetWindowsHookEx(
                hModule, DllName, idThread, HookType, HookProc, Flags);
}


//---------------------------------------------------------------------------
// Gui_NtUserSetWinEventHook
//---------------------------------------------------------------------------


_FX ULONG_PTR Gui_NtUserSetWinEventHook(
    ULONG EventMin, ULONG EventMax, void *hModule, UNICODE_STRING *DllName,
    void *HookProc, ULONG idProcess, ULONG idThread, ULONG Flags)
{
    GUI_NTUSER_IF {

        NTSTATUS status = STATUS_SUCCESS;

        if (Flags & WINEVENT_INCONTEXT) {

            if (idThread)
                status = Gui_CheckBoxedThread(proc, idThread, &idProcess);
            else if (idProcess) {
                if (! Process_IsSameBox(proc, NULL, (ULONG_PTR)idProcess))
                    status = STATUS_ACCESS_DENIED;
            } else
                status = STATUS_ACCESS_DENIED;
        }

        //
        // trace request
        //

        if (proc->gui_trace & (TRACE_ALLOW | TRACE_DENY)) {

            WCHAR access_str[64];
            WCHAR letter;

            if ((! NT_SUCCESS(status)) && (proc->gui_trace & TRACE_DENY))
                letter = L'D';
            else if (NT_SUCCESS(status) && (proc->gui_trace & TRACE_ALLOW))
                letter = L'A';
            else
                letter = 0;

            if (letter) {

                RtlStringCbPrintfW(access_str, sizeof(access_str), L"(G%c) AccHook on tid=%06d pid=%06d",
                         letter, idThread, idProcess);
                Log_Debug_Msg(MONITOR_WINCLASS | MONITOR_TRACE, access_str, Driver_Empty);
            }
        }

        if (! NT_SUCCESS(status))
            return 0;
    }

    return __sys_NtUserSetWinEventHook(
                    EventMin, EventMax, hModule, DllName,
                    HookProc, idProcess, idThread, Flags);
}


//---------------------------------------------------------------------------
// Gui_CheckBoxedThread
//---------------------------------------------------------------------------


_FX NTSTATUS Gui_CheckBoxedThread(
    PROCESS *proc, ULONG idThread, ULONG *out_idProcess)
{
    NTSTATUS status;
    HANDLE handle;
    OBJECT_ATTRIBUTES objattrs;
    CLIENT_ID cid;
    THREAD_BASIC_INFORMATION info;
    ULONG len;

    InitializeObjectAttributes(&objattrs,
        NULL, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, NULL, NULL);
    cid.UniqueProcess = 0;
    cid.UniqueThread = (HANDLE)(ULONG_PTR)idThread;

    status = ZwOpenThread(
        &handle, THREAD_QUERY_INFORMATION, &objattrs, &cid);

    if (NT_SUCCESS(status)) {

        len = sizeof(THREAD_BASIC_INFORMATION);
        status = ZwQueryInformationThread(
            handle, ThreadBasicInformation, &info, len, &len);
        if (NT_SUCCESS(status)) {

            HANDLE idProcess = info.ClientId.UniqueProcess;
            if (out_idProcess)
                *out_idProcess = (ULONG)(ULONG_PTR)idProcess;
            if (! Process_IsSameBox(proc, NULL, (ULONG_PTR)idProcess))
                status = STATUS_ACCESS_DENIED;
        }

        ZwClose(handle);
    }

    return status;
}
