/*
 * Copyright 2004-2020 Sandboxie Holdings, LLC 
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
// Service Control Manager
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
// Defines
//---------------------------------------------------------------------------


#define SERVICE_NOTIFY_STOPPED              0x00000001
#define SERVICE_NOTIFY_START_PENDING        0x00000002
#define SERVICE_NOTIFY_STOP_PENDING         0x00000004
#define SERVICE_NOTIFY_RUNNING              0x00000008
#define SERVICE_NOTIFY_CONTINUE_PENDING     0x00000010
#define SERVICE_NOTIFY_PAUSE_PENDING        0x00000020
#define SERVICE_NOTIFY_PAUSED               0x00000040
#define SERVICE_NOTIFY_CREATED              0x00000080
#define SERVICE_NOTIFY_DELETED              0x00000100
#define SERVICE_NOTIFY_DELETE_PENDING       0x00000200


//---------------------------------------------------------------------------
// Structures and Types
//---------------------------------------------------------------------------


typedef void (*P_SCM_NOTIFY_CALLBACK)(ULONG_PTR arg);



typedef struct _SCM_NOTIFY_ELEM {

    LIST_ELEM list_elem;
    HANDLE hThread;
    SC_HANDLE hService;
    SERVICE_NOTIFY *data;
    ULONG mask;
    ULONG state;
    BOOLEAN active;

} SCM_NOTIFY_ELEM;


typedef struct _SCM_NOTIFY_GLOBAL {

    LIST list;
    HANDLE hThread;
    HANDLE hEvent;

} SCM_NOTIFY_GLOBAL;


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------


static BOOLEAN Scm_Notify_Init(HMODULE module);

static DWORD Scm_NotifyServiceStatusChangeW(
    SC_HANDLE hService, DWORD dwNotifyMask, void *pNotifyBuffer);

static DWORD Scm_NotifyServiceStatusChangeA(
    SC_HANDLE hService, DWORD dwNotifyMask, void *pNotifyBuffer);

static void Scm_Notify_CloseHandle(SC_HANDLE hService);

static ULONG Scm_Notify_ThreadProc(void *lpParameter);

static void Scm_Notify_ThreadProc2(SCM_NOTIFY_ELEM *notify_elem);

static void Scm_Notify_ApcProc(ULONG_PTR data);


//---------------------------------------------------------------------------


typedef DWORD (*P_NotifyServiceStatusChange)(
    SC_HANDLE hService, DWORD dwNotifyMask, void *pNotifyBuffer);


//---------------------------------------------------------------------------
// Variables
//---------------------------------------------------------------------------


static CRITICAL_SECTION *Scm_Notify_CritSec = NULL;
static SCM_NOTIFY_GLOBAL *Scm_Notify_Global = NULL;


//---------------------------------------------------------------------------


static P_NotifyServiceStatusChange __sys_NotifyServiceStatusChangeA = NULL;
static P_NotifyServiceStatusChange __sys_NotifyServiceStatusChangeW = NULL;


//---------------------------------------------------------------------------
// Scm_Notify_Init
//---------------------------------------------------------------------------


_FX BOOLEAN Scm_Notify_Init(HMODULE module)
{
    //
    // NotifyServiceStatusChange is available on Windows Vista and later
    //

    if (Dll_OsBuild < 6000)
        return TRUE;

    //
    // initialize critical section
    //

    Scm_Notify_CritSec = Dll_Alloc(sizeof(CRITICAL_SECTION));
    InitializeCriticalSectionAndSpinCount(Scm_Notify_CritSec, 1000);

    //
    // hook the API
    //

    SCM_IMPORT_AW(NotifyServiceStatusChange);

    SBIEDLL_HOOK_SCM(NotifyServiceStatusChangeA);
    SBIEDLL_HOOK_SCM(NotifyServiceStatusChangeW);

    return TRUE;
}


//---------------------------------------------------------------------------
// Scm_Notify_CloseHandle
//---------------------------------------------------------------------------


_FX void Scm_Notify_CloseHandle(SC_HANDLE hService)
{
    SCM_NOTIFY_ELEM *notify_elem;

    if (Scm_Notify_Global) {

        EnterCriticalSection(Scm_Notify_CritSec);

        notify_elem = List_Head(&Scm_Notify_Global->list);
        while (notify_elem) {

            if (notify_elem->hService != hService)
                notify_elem = List_Next(notify_elem);
            else {

                CloseHandle(notify_elem->hThread);
                List_Remove(&Scm_Notify_Global->list, notify_elem);
                Dll_Free(notify_elem);

                notify_elem = List_Head(&Scm_Notify_Global->list);
            }
        }

        LeaveCriticalSection(Scm_Notify_CritSec);
    }
}


//---------------------------------------------------------------------------
// Scm_NotifyServiceStatusChangeW
//---------------------------------------------------------------------------


_FX DWORD Scm_NotifyServiceStatusChangeW(
    SC_HANDLE hService, DWORD dwNotifyMask, void *pNotifyBuffer)
{
    WCHAR *ServiceNm;
    SCM_NOTIFY_ELEM *notify_elem;
    HANDLE handle;
    ULONG dwVersion;

    //
    // validate parameters
    //

    dwVersion = ((SERVICE_NOTIFY *)pNotifyBuffer)->dwVersion;
    if (dwVersion != 1 && dwVersion != 2) {
        SbieApi_Log(2205, L"NotifyServiceStatusChange (%d)", dwVersion);
        return ERROR_INVALID_PARAMETER;
    }

    ServiceNm = Scm_GetHandleName(hService);
    if (! ServiceNm)
        return ERROR_SERVICE_DOES_NOT_EXIST;

    //
    // look for an existing notification entry
    //

    EnterCriticalSection(Scm_Notify_CritSec);

    if (! Scm_Notify_Global) {
        Scm_Notify_Global = Dll_Alloc(sizeof(SCM_NOTIFY_GLOBAL));
        List_Init(&Scm_Notify_Global->list);
        Scm_Notify_Global->hThread = NULL;
        Scm_Notify_Global->hEvent = NULL;
    }

    notify_elem = List_Head(&Scm_Notify_Global->list);
    while (notify_elem) {
        if (notify_elem->hService == hService)
            break;
        notify_elem = List_Next(notify_elem);
    }

    if (! notify_elem) {

        //
        // open a thread handle so we can call QueueUserAPC later on
        //

        handle = OpenThread(THREAD_SET_CONTEXT, FALSE, GetCurrentThreadId());
        if (! handle) {
            ULONG LastError = GetLastError();
            LeaveCriticalSection(Scm_Notify_CritSec);
            return LastError;
        }

        //
        // create a new notification entry
        //

        notify_elem = Dll_Alloc(sizeof(SCM_NOTIFY_ELEM));
        notify_elem->hThread = handle;
        notify_elem->hService = hService;
        notify_elem->data = pNotifyBuffer;
        notify_elem->mask = dwNotifyMask;
        notify_elem->state = 0;
        notify_elem->active = TRUE;
        List_Insert_After(&Scm_Notify_Global->list, NULL, notify_elem);
    }

    notify_elem->active = TRUE;

    //
    // start the service watcher thread if necessary
    //

    if (! Scm_Notify_Global->hEvent)
        Scm_Notify_Global->hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

    if (! Scm_Notify_Global->hThread) {
        ULONG idThread;
        Scm_Notify_Global->hThread = CreateThread(
                        NULL, 0, Scm_Notify_ThreadProc, NULL, 0, &idThread);
    }

    if (Scm_Notify_Global->hThread && Scm_Notify_Global->hEvent)
        SetEvent(Scm_Notify_Global->hEvent);

    LeaveCriticalSection(Scm_Notify_CritSec);

    SetLastError(0);
    return ERROR_SUCCESS;
}


//---------------------------------------------------------------------------
// Scm_NotifyServiceStatusChangeA
//---------------------------------------------------------------------------


_FX DWORD Scm_NotifyServiceStatusChangeA(
    SC_HANDLE hService, DWORD dwNotifyMask, void *pNotifyBuffer)
{
    return Scm_NotifyServiceStatusChangeW(
                                    hService, dwNotifyMask, pNotifyBuffer);
}


//---------------------------------------------------------------------------
// Scm_Notify_ThreadProc
//---------------------------------------------------------------------------


_FX ULONG Scm_Notify_ThreadProc(void *lpParameter)
{
    SCM_NOTIFY_ELEM *notify_elem;

    while (1) {

        //
        // loop through notify handles and look for active ones
        //

        EnterCriticalSection(Scm_Notify_CritSec);

        notify_elem = List_Head(&Scm_Notify_Global->list);
        while (notify_elem) {

            if (notify_elem->active) {

                Scm_Notify_ThreadProc2(notify_elem);
            }

            notify_elem = List_Next(notify_elem);
        }

        LeaveCriticalSection(Scm_Notify_CritSec);

        //
        // wait two seconds, or until the event is triggered
        // to indicate that a new element was added or updated
        //

        WaitForSingleObject(Scm_Notify_Global->hEvent, 2000);
    }

    return 0;
}


//---------------------------------------------------------------------------
// Scm_Notify_ThreadProc2
//---------------------------------------------------------------------------


_FX void Scm_Notify_ThreadProc2(SCM_NOTIFY_ELEM *notify_elem)
{
    SERVICE_QUERY_RPL *rpl = (SERVICE_QUERY_RPL *)
        Scm_QueryServiceByHandle(notify_elem->hService, TRUE, 0);

    if (rpl) {
         if (rpl->h.status == 0) {

            ULONG state = 0;
            SERVICE_STATUS_PROCESS *ss = &rpl->service_status;
            if (ss->dwCurrentState == SERVICE_STOPPED)
                state = SERVICE_NOTIFY_STOPPED;
            else if (ss->dwCurrentState == SERVICE_START_PENDING)
                state = SERVICE_NOTIFY_START_PENDING;
            else if (ss->dwCurrentState == SERVICE_STOP_PENDING)
                state = SERVICE_NOTIFY_STOP_PENDING;
            else if (ss->dwCurrentState == SERVICE_RUNNING)
                state = SERVICE_NOTIFY_RUNNING;
            else if (ss->dwCurrentState == SERVICE_CONTINUE_PENDING)
                state = SERVICE_NOTIFY_CONTINUE_PENDING;
            else if (ss->dwCurrentState == SERVICE_PAUSE_PENDING)
                state = SERVICE_NOTIFY_PAUSE_PENDING;
            else if (ss->dwCurrentState == SERVICE_PAUSED)
                state = SERVICE_NOTIFY_PAUSED;

            if ((notify_elem->mask & state) != 0 &&
                                            (state != notify_elem->state)) {

                SERVICE_NOTIFY *data = notify_elem->data;

                memcpy(&data->ServiceStatus, ss,
                       sizeof(SERVICE_STATUS_PROCESS));
                data->dwNotificationStatus = ERROR_SUCCESS;

                if (data->dwVersion == 2) {
                    data->dwNotificationTriggered = state;
                    data->pszServiceNames = NULL;
                }

                notify_elem->state = state;

                notify_elem->active = FALSE;

                QueueUserAPC(Scm_Notify_ApcProc,
                             notify_elem->hThread, (ULONG_PTR)data);
            }
         }

         Dll_Free(rpl);
    }
}


//---------------------------------------------------------------------------
// Scm_Notify_ApcProc
//---------------------------------------------------------------------------


_FX void Scm_Notify_ApcProc(ULONG_PTR data)
{
    //
    // make sure the service notify handle was not closed, between time the
    // APC was scheduled in Scm_Notify_ThreadProc2, and now, which is the
    // time when the APC is actually executing
    //

    SCM_NOTIFY_ELEM *notify_elem;

    EnterCriticalSection(Scm_Notify_CritSec);

    notify_elem = List_Head(&Scm_Notify_Global->list);

    while (notify_elem) {

        // if SERVICE_NOTIFY is still there
        if (data == (ULONG_PTR)notify_elem->data) {

            // Scm_Notify_ThreadProc2 clears notify_elem->active before
            if (! notify_elem->active) {

                ((SERVICE_NOTIFY *)data)->pfnNotifyCallback((PVOID)data);
                break;
            }
        }

        notify_elem = List_Next(notify_elem);
    }

    LeaveCriticalSection(Scm_Notify_CritSec);
}
