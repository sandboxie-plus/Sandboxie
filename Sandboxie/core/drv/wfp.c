/*
 * Copyright 2021 David Xanatos, xanasoft.com
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
// Windows Filtering Platform
//---------------------------------------------------------------------------


#include "wfp.h"
#include "process.h"
#include "conf.h"
#include "session.h"
#include "api_flags.h"
#include "common/map.h"
#include "common/netfw.h"
#include "common/my_version.h"
#define NO_IP_DEFS
#include "common/my_wsa.h"


extern DEVICE_OBJECT *Api_DeviceObject;

#ifdef _M_ARM64
#define NDIS630 1 // windows 8.1
#else
#define NDIS61 1 // windows 7
#endif


#include "Ntifs.h"
#include <ntddk.h>				// Windows Driver Development Kit

#pragma warning(push)
#pragma warning(disable: 4201)	// Disable "Nameless struct/union" compiler warning for fwpsk.h only!
#include <fwpsk.h>				// Functions and enumerated types used to implement callouts in kernel mode
#pragma warning(pop)			// Re-enable "Nameless struct/union" compiler warning

#include <guiddef.h>			// Used to define GUID's
#include <initguid.h>			// Used to define GUID's
#include <fwpmk.h>				// Functions used for managing IKE and AuthIP main mode (MM) policy and security associations
#include <fwpvi.h>				// Mappings of OS specific function versions (i.e. fn's that end in 0 or 1)
#include "devguid.h"


//---------------------------------------------------------------------------
// Defines
//---------------------------------------------------------------------------

#define WFP_SUBLAYER_NAME L"SbieSublayer"
#define WFP_SUBLAYER_DESCRIPTION L"A sublayer used by sandboxie to implement internet restrictions"

DEFINE_GUID(WFP_SUBLAYER_GUID,	// e1d364e9-cd84-4a48-aba4-608ce83e31ee
	0xe1d364e9, 0xcd84, 0x4a48, 0xab, 0xa4, 0x60, 0x8c, 0xe8, 0x3e, 0x31, 0xee);

#define WFP_CALLOUT_NAME		L"SbieCallout"
#define WFP_CALLOUT_DESCRIPTION	L"A callout used by sandboxie to implement internet restrictions"

//DEFINE_GUID(WPF_CALLOUT_GUID_V4,	// 0bf56435-71e4-4de7-bd0b-1af0b4cbb8f4
//	0x0bf56435, 0x71e4, 0x4de7, 0xbd, 0x0b, 0x1a, 0xf0, 0xb4, 0xcb, 0xb8, 0xf4);
//DEFINE_GUID(WPF_CALLOUT_GUID_V6,	// 0bf56435-71e4-4de7-bd0b-1af0b4cbb9f5
//	0x0bf56435, 0x71e4, 0x4de7, 0xbd, 0x0b, 0x1a, 0xf0, 0xb4, 0xcb, 0xb9, 0xf5);
DEFINE_GUID(WPF_SEND_CALLOUT_GUID_V4,	// 0bf56435-71e4-4de7-bd0b-1af0b4cbb8f6
	0x0bf56435, 0x71e4, 0x4de7, 0xbd, 0x0b, 0x1a, 0xf0, 0xb4, 0xcb, 0xb8, 0xf6);
DEFINE_GUID(WPF_SEND_CALLOUT_GUID_V6,	// 0bf56435-71e4-4de7-bd0b-1af0b4cbb9f7
	0x0bf56435, 0x71e4, 0x4de7, 0xbd, 0x0b, 0x1a, 0xf0, 0xb4, 0xcb, 0xb9, 0xf7);
DEFINE_GUID(WPF_RECV_CALLOUT_GUID_V4,	// 0bf56435-71e4-4de7-bd0b-1af0b4cbb8f8
	0x0bf56435, 0x71e4, 0x4de7, 0xbd, 0x0b, 0x1a, 0xf0, 0xb4, 0xcb, 0xb8, 0xf8);
DEFINE_GUID(WPF_RECV_CALLOUT_GUID_V6,	// 0bf56435-71e4-4de7-bd0b-1af0b4cbb9f9
	0x0bf56435, 0x71e4, 0x4de7, 0xbd, 0x0b, 0x1a, 0xf0, 0xb4, 0xcb, 0xb9, 0xf9);

#define WFP_FILTER_NAME L"SbieFilter"
#define WFP_FILTER_DESCRIPTION L"A filter that uses by sandboxie to implement internet restrictions"


//---------------------------------------------------------------------------
// Structures and Types
//---------------------------------------------------------------------------


typedef struct _WFP_PROCESS {

	HANDLE ProcessId;
	BOOLEAN LogTraffic;
	BOOLEAN BlockInternet;
	LIST NetFwRules;

} WFP_PROCESS;


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------

BOOLEAN WFP_Install_Callbacks(void);

void WFP_Uninstall_Callbacks(void);

NTSTATUS WFP_RegisterSubLayer();

NTSTATUS WFP_RegisterCallout(const GUID* calloutKey, const GUID* applicableLayer, UINT32* callout_id, UINT64* filter_id);

const WCHAR* Process_MatchImageAndGetValue(BOX* box, const WCHAR* value, const WCHAR* ImageName, ULONG* pLevel);

ULONG Process_GetTraceFlag(PROCESS *proc, const WCHAR *setting);

void WFP_FreeRules(LIST* NetFwRules);

#ifdef ALLOC_PRAGMA
#pragma alloc_text (INIT, WFP_Init)
#endif // ALLOC_PRAGMA

void WFP_state_changed(
	_Inout_ void* context,
	_In_ FWPM_SERVICE_STATE newState);

/*	The "classifyFn" callout function for this Callout.
For more information about a Callout's classifyFn, see:
http://msdn.microsoft.com/en-us/library/windows/hardware/ff544893(v=vs.85).aspx
*/
void WFP_classify(
	const FWPS_INCOMING_VALUES * inFixedValues,
	const FWPS_INCOMING_METADATA_VALUES * inMetaValues,
	void * layerData,
	const void * classifyContext,
	const FWPS_FILTER * filter,
	UINT64 flowContext,
	FWPS_CLASSIFY_OUT * classifyOut);

/*	The "notifyFn" callout function for this Callout.
This function manages setting up global resources and a worker thread
managed by this Callout. For more information about a Callout's notifyFn, see:
http://msdn.microsoft.com/en-us/library/windows/hardware/ff568804(v=vs.85).aspx
*/
NTSTATUS WFP_notify(
	FWPS_CALLOUT_NOTIFY_TYPE notifyType,
	const GUID * filterKey,
	const FWPS_FILTER * filter);

/*	The "flowDeleteFn" callout function for this Callout.
This function doesn't do anything.
http://msdn.microsoft.com/en-us/library/windows/hardware/ff550025(v=vs.85).aspx
*/
NTSTATUS WFP_flow_delete(
	UINT16 layerId,
	UINT32 calloutId,
	UINT64 flowContext);

void GetNetwork5TupleIndexesForLayer(
	_In_ UINT16 layerId,
	_Out_ UINT* localAddressIndex,
	_Out_ UINT* remoteAddressIndex,
	_Out_ UINT* localPortIndex,
	_Out_ UINT* remotePortIndex,
	_Out_ UINT* protocolIndex);


//---------------------------------------------------------------------------
// Variables
//---------------------------------------------------------------------------


BOOLEAN WFP_Enabled = FALSE;
static PERESOURCE WFP_InitLock = NULL;

static HANDLE WFP_state_handle = NULL;

// Global handle to the WFP Base Filter Engine
static HANDLE WFP_engine_handle = NULL;

//static UINT32 WFP_callout_id_v4 = 0;
//static UINT32 WFP_callout_id_v6 = 0;
static UINT32 WFP_send_callout_id_v4 = 0;
static UINT32 WFP_send_callout_id_v6 = 0;
static UINT32 WFP_recv_callout_id_v4 = 0;
static UINT32 WFP_recv_callout_id_v6 = 0;

//static UINT64 WFP_filter_id_v4 = 0;
//static UINT64 WFP_filter_id_v6 = 0;
static UINT64 WFP_send_filter_id_v4 = 0;
static UINT64 WFP_send_filter_id_v6 = 0;
static UINT64 WFP_recv_filter_id_v4 = 0;
static UINT64 WFP_recv_filter_id_v6 = 0;

static BOOLEAN WPF_MapInitialized = FALSE;
static map_base_t WFP_Processes;
static KSPIN_LOCK WFP_MapLock;


//---------------------------------------------------------------------------
// WFP_Alloc
//---------------------------------------------------------------------------


_FX VOID* WFP_Alloc(void* pool, size_t size)
{
	return ExAllocatePoolWithTag(NonPagedPool, size, tzuk);
}


//---------------------------------------------------------------------------
// WFP_Free
//---------------------------------------------------------------------------


_FX VOID WFP_Free(void* pool, void* ptr)
{
	ExFreePoolWithTag(ptr, tzuk);
}


//---------------------------------------------------------------------------
// WFP_Init
//---------------------------------------------------------------------------


_FX BOOLEAN WFP_Init(void)
{
	map_init(&WFP_Processes, NULL);
	WFP_Processes.func_malloc = &WFP_Alloc;
	WFP_Processes.func_free = &WFP_Free;

	KeInitializeSpinLock(&WFP_MapLock);

	WPF_MapInitialized = TRUE;

	if (!Conf_Get_Boolean(NULL, L"NetworkEnableWFP", 0, FALSE))
		return TRUE;

	return WFP_Load();
}


//---------------------------------------------------------------------------
// WFP_Load
//---------------------------------------------------------------------------


_FX BOOLEAN WFP_Load(void)
{
	if (WFP_Enabled)
		return TRUE;

	WFP_Enabled = TRUE;

	map_resize(&WFP_Processes, 128); // prepare some buckets for better performance

	DbgPrint("Sbie WFP enabled\r\n");

	if (!Mem_GetLockResource(&WFP_InitLock, TRUE))
		return FALSE;

	NTSTATUS status = FwpmBfeStateSubscribeChanges((void*)Api_DeviceObject, WFP_state_changed, NULL, &WFP_state_handle);
	if (!NT_SUCCESS(status)) {
		DbgPrint("Sbie WFP failed to install state change callback\r\n");
		Mem_FreeLockResource(&WFP_InitLock);
		WFP_InitLock = NULL;
		return FALSE;
	}

	if (FwpmBfeStateGet() == FWPM_SERVICE_RUNNING) {

		ExAcquireResourceSharedLite(WFP_InitLock, TRUE);

		WFP_Install_Callbacks();

		ExReleaseResourceLite(WFP_InitLock);
	}
	else
		DbgPrint("Sbie WFP is not ready\r\n");

	return TRUE;
}


//---------------------------------------------------------------------------
// WFP_Unload
//---------------------------------------------------------------------------


_FX void WFP_Unload(void)
{
	WFP_Enabled = FALSE;

	if (WFP_state_handle != NULL) {

		FwpmBfeStateUnsubscribeChanges(WFP_state_handle);
		WFP_state_handle = NULL;
	}

	if (WFP_InitLock) {

		ExAcquireResourceSharedLite(WFP_InitLock, TRUE);

		WFP_Uninstall_Callbacks();

		ExReleaseResourceLite(WFP_InitLock);

		Mem_FreeLockResource(&WFP_InitLock);
		WFP_InitLock = NULL;
	}

	if (WPF_MapInitialized) {

		KIRQL irql; 

#ifdef _WIN64
		irql = KeAcquireSpinLockRaiseToDpc(&WFP_MapLock);
#else
		KeAcquireSpinLock(&WFP_MapLock, &irql);
#endif

		map_iter_t iter = map_iter();
		while (map_next(&WFP_Processes, &iter)) {
			WFP_PROCESS* wfp_proc = iter.value;
			
			WFP_FreeRules(&wfp_proc->NetFwRules);
			WFP_Free(NULL, wfp_proc);
		}

		map_clear(&WFP_Processes);

		KeReleaseSpinLock(&WFP_MapLock, irql);
	}
}


//---------------------------------------------------------------------------
// WFP_state_changed
//---------------------------------------------------------------------------


_FX void WFP_state_changed(_Inout_ void* context, _In_ FWPM_SERVICE_STATE newState)
{
	ExAcquireResourceSharedLite(WFP_InitLock, TRUE);

	if (newState == FWPM_SERVICE_STOP_PENDING)
		WFP_Uninstall_Callbacks();
	else if (newState == FWPM_SERVICE_RUNNING)
		WFP_Install_Callbacks();

	ExReleaseResourceLite(WFP_InitLock);
}


//---------------------------------------------------------------------------
// WFP_Install_Callbacks
//---------------------------------------------------------------------------


_FX BOOLEAN WFP_Install_Callbacks(void)
{
	if (WFP_engine_handle != NULL)
		return TRUE; // already initialized

	NTSTATUS status = STATUS_SUCCESS;
	DWORD stage = 0;

	FWPM_SESSION wdf_session = { 0 };
	BOOLEAN in_transaction = FALSE;
	BOOLEAN callout_registered = FALSE;


	// Begin a transaction to the FilterEngine. You must register objects (filter, callouts, sublayers)
	//to the filter engine in the context of a 'transaction'
	wdf_session.flags = FWPM_SESSION_FLAG_DYNAMIC;	// <-- Automatically destroys all filters and removes all callouts after this wdf_session ends
	status = FwpmEngineOpen(NULL, RPC_C_AUTHN_WINNT, NULL, &wdf_session, &WFP_engine_handle);
	stage = 0x10; if (!NT_SUCCESS(status)) goto Exit;
	status = FwpmTransactionBegin(WFP_engine_handle, 0);
	stage = 0x20; if (!NT_SUCCESS(status)) goto Exit;
	in_transaction = TRUE;

	// Register a new sublayer to the filter engine
	status = WFP_RegisterSubLayer();
	stage = 0x30; if (!NT_SUCCESS(status)) goto Exit;

	//status = WFP_RegisterCallout(&WPF_CALLOUT_GUID_V4, &FWPM_LAYER_ALE_RESOURCE_ASSIGNMENT_V4, &WFP_callout_id_v4, &WFP_filter_id_v4);
	//if (!NT_SUCCESS(status)) goto Exit;
	//status = WFP_RegisterCallout(&WPF_CALLOUT_GUID_V6, &FWPM_LAYER_ALE_RESOURCE_ASSIGNMENT_V6, &WFP_callout_id_v6, &WFP_filter_id_v6);
	//if (!NT_SUCCESS(status)) goto Exit;
	status = WFP_RegisterCallout(&WPF_SEND_CALLOUT_GUID_V4, &FWPM_LAYER_ALE_AUTH_CONNECT_V4, &WFP_send_callout_id_v4, &WFP_send_filter_id_v4);
	stage = 0x41; if (!NT_SUCCESS(status)) goto Exit;
	callout_registered = TRUE;
	status = WFP_RegisterCallout(&WPF_SEND_CALLOUT_GUID_V6, &FWPM_LAYER_ALE_AUTH_CONNECT_V6, &WFP_send_callout_id_v6, &WFP_send_filter_id_v6);
	stage = 0x42; if (!NT_SUCCESS(status)) goto Exit;
	status = WFP_RegisterCallout(&WPF_RECV_CALLOUT_GUID_V4, &FWPM_LAYER_ALE_AUTH_RECV_ACCEPT_V4, &WFP_recv_callout_id_v4, &WFP_recv_filter_id_v4);
	stage = 0x43; if (!NT_SUCCESS(status)) goto Exit;
	status = WFP_RegisterCallout(&WPF_RECV_CALLOUT_GUID_V6, &FWPM_LAYER_ALE_AUTH_RECV_ACCEPT_V6, &WFP_recv_callout_id_v6, &WFP_recv_filter_id_v6);
	stage = 0x44; if (!NT_SUCCESS(status)) goto Exit;

	// note: we could also setup FWPM_LAYER_ALE_AUTH_LISTEN_V4 but since we block all accepts we don't have to


	// Commit transaction to the Filter Engine
	status = FwpmTransactionCommit(WFP_engine_handle);
	stage = 0x50; if (!NT_SUCCESS(status)) goto Exit;
	in_transaction = FALSE;

	// Cleanup and handle any errors
Exit:
	if (!NT_SUCCESS(status)) {
		DbgPrint("Sbie WFP initialization failed, stage %02x, status 0x%08x\r\n", stage, status);

		if (in_transaction == TRUE) {
			FwpmTransactionAbort(WFP_engine_handle);
			_Analysis_assume_lock_not_held_(WFP_engine_handle); // Potential leak if "FwpmTransactionAbort" fails
		}
		if (callout_registered) {
			FwpsCalloutUnregisterById(WFP_send_callout_id_v4);
			FwpsCalloutUnregisterById(WFP_send_callout_id_v6);
			FwpsCalloutUnregisterById(WFP_recv_callout_id_v4);
			FwpsCalloutUnregisterById(WFP_recv_callout_id_v6);
		}
		if (WFP_engine_handle) {
			FwpmEngineClose(WFP_engine_handle);
			WFP_engine_handle = NULL;
		}

		return FALSE;
	}

	DbgPrint("Sbie WFP initialized successfully\r\n");
	return TRUE;
}


//---------------------------------------------------------------------------
// WFP_Uninstall_Callbacks
//---------------------------------------------------------------------------


_FX void WFP_Uninstall_Callbacks(void)
{
	if (WFP_engine_handle == NULL)
		return; // not initialized

	NTSTATUS status = STATUS_SUCCESS;
	UNICODE_STRING symlink = { 0 };

	//status = FwpsCalloutUnregisterById(WFP_callout_id_v4);
	//if (!NT_SUCCESS(status)) DbgPrint("Failed to unregister callout, status: 0x%08x\r\n", status);
	//status = FwpsCalloutUnregisterById(WFP_callout_id_v6);
	//if (!NT_SUCCESS(status)) DbgPrint("Failed to unregister callout, status: 0x%08x\r\n", status);
	status = FwpsCalloutUnregisterById(WFP_send_callout_id_v4);
	//if (!NT_SUCCESS(status)) DbgPrint("Failed to unregister callout, status: 0x%08x\r\n", status);
	status = FwpsCalloutUnregisterById(WFP_send_callout_id_v6);
	//if (!NT_SUCCESS(status)) DbgPrint("Failed to unregister callout, status: 0x%08x\r\n", status);
	status = FwpsCalloutUnregisterById(WFP_recv_callout_id_v4);
	//if (!NT_SUCCESS(status)) DbgPrint("Failed to unregister callout, status: 0x%08x\r\n", status);
	status = FwpsCalloutUnregisterById(WFP_recv_callout_id_v6);
	//if (!NT_SUCCESS(status)) DbgPrint("Failed to unregister callout, status: 0x%08x\r\n", status);

	// Close handle to the WFP Filter Engine
	if (WFP_engine_handle) {
		FwpmEngineClose(WFP_engine_handle);
		WFP_engine_handle = NULL;
	}

	DbgPrint("Sbie WFP uninitialized\r\n");
}


//---------------------------------------------------------------------------
// WFP_RegisterSubLayer
//---------------------------------------------------------------------------


NTSTATUS WFP_RegisterSubLayer()
{
	NTSTATUS status = STATUS_SUCCESS;

	FWPM_SUBLAYER sublayer = { 0 };
	sublayer.subLayerKey = WFP_SUBLAYER_GUID;
	sublayer.displayData.name = WFP_SUBLAYER_NAME;
	sublayer.displayData.description = WFP_SUBLAYER_DESCRIPTION;
	sublayer.flags = 0;
	sublayer.weight = 0x0f;
	status = FwpmSubLayerAdd(WFP_engine_handle, &sublayer, NULL);
	if (!NT_SUCCESS(status)){
		//DbgPrint("Failed to register Sbie sublayer, status 0x%08x\r\n", status);
		goto Exit;
	}

Exit:
	return status;
}


//---------------------------------------------------------------------------
// WFP_RegisterCallout
//---------------------------------------------------------------------------


NTSTATUS WFP_RegisterCallout(const GUID* calloutKey, const GUID* applicableLayer, UINT32* callout_id, UINT64* filter_id)
{
	NTSTATUS status = STATUS_SUCCESS;
	
	if (WFP_engine_handle == NULL)
		return STATUS_INVALID_HANDLE;

	// Register a new Callout with the Filter Engine using the provided callout functions
	FWPS_CALLOUT s_callout = { 0 };
	s_callout.calloutKey = *calloutKey;
	s_callout.classifyFn = WFP_classify;
	s_callout.notifyFn = WFP_notify;
	s_callout.flowDeleteFn = WFP_flow_delete;
	status = FwpsCalloutRegister((void *)Api_DeviceObject, &s_callout, callout_id);
	if (!NT_SUCCESS(status)){
		//DbgPrint("Failed to register callout functions for sbie callout, status 0x%08x\r\n", status);
		goto Exit;
	}

	// Setup a FWPM_CALLOUT structure to store/track the state associated with the FWPS_CALLOUT
	FWPM_CALLOUT m_callout = { 0 };
	m_callout.displayData.name = WFP_CALLOUT_NAME;
	m_callout.displayData.description = WFP_CALLOUT_DESCRIPTION;
	m_callout.calloutKey = *calloutKey;
	m_callout.applicableLayer = *applicableLayer;
	m_callout.flags = 0;
	status = FwpmCalloutAdd(WFP_engine_handle, &m_callout, NULL, NULL);
	if (!NT_SUCCESS(status)){
		//DbgPrint("Failed to register sbie callout, status 0x%08x\r\n", status);
		goto Exit;
	}

	// Setup a FWPM_FILTER structure
	FWPM_FILTER filter = { 0 };
	filter.displayData.name = WFP_FILTER_NAME;
	filter.displayData.description = WFP_FILTER_DESCRIPTION;
	filter.action.type = FWP_ACTION_CALLOUT_TERMINATING;	// Says this filter's callout MUST make a block/permit decision
	filter.subLayerKey = WFP_SUBLAYER_GUID;
	filter.weight.type = FWP_UINT8;
	filter.weight.uint8 = 0xf;		// The weight of this filter within its sublayer
	filter.numFilterConditions = 0;	// If you specify 0, this filter invokes its callout for all traffic in its layer
	filter.layerKey = *applicableLayer;	// This layer must match the layer that ExampleCallout is registered to
	filter.action.calloutKey = *calloutKey;
	status = FwpmFilterAdd(WFP_engine_handle, &filter, NULL, filter_id);
	if (status != STATUS_SUCCESS){
		//DbgPrint("Failed to register Sbie filter, status 0x%08x\r\n", status);
		goto Exit;
	}

Exit:
	return status;
}


//---------------------------------------------------------------------------
// WFP_FreeRules
//---------------------------------------------------------------------------


void WFP_FreeRules(LIST* NetFwRules)
{
	// clear Firewall Rules
	while (1) {
		NETFW_RULE* rule = List_Head(NetFwRules);
		if (!rule)
			break;
		List_Remove(NetFwRules, rule);
		NetFw_FreeRule(rule);
	}
}


//---------------------------------------------------------------------------
// WFP_LoadRules
//---------------------------------------------------------------------------


BOOLEAN WFP_LoadRules(LIST* NetFwRules, PROCESS* proc)
{
	List_Init(NetFwRules);

    for (ULONG index = 0; ; ++index) {

        const WCHAR *value = Conf_Get(proc->box->name, L"NetworkAccess", index);
        if (! value)
            break;

        ULONG level = -1;
        const WCHAR *found_value = Process_MatchImageAndGetValue(proc->box, value, proc->image_name, &level);
        if (!found_value)
            continue;

        NETFW_RULE* rule = NetFw_AllocRule(NULL, level);
        if (!rule) {
            Log_Msg_Process(MSG_1201, NULL, NULL, proc->box->session_id, proc->pid);
			return FALSE;
        }

		NetFw_ParseRule(rule, found_value);

		NetFw_AddRule(NetFwRules, rule);
    }

	return TRUE;
}


//---------------------------------------------------------------------------
// WFP_InitProcess
//---------------------------------------------------------------------------


BOOLEAN WFP_InitProcess(PROCESS* proc)
{
	if (!WFP_Enabled)
		return TRUE; // nothing to do

	if (WFP_engine_handle == NULL)
		return FALSE; // WFP was not ready report failure, cancel process creation

	BOOLEAN ok = TRUE;
	KIRQL irql; 

	WFP_PROCESS* wfp_proc = WFP_Alloc(NULL, sizeof(WFP_PROCESS));
	if (wfp_proc == NULL) {
		ok = FALSE;
		goto finish;
	}
	memzero(wfp_proc, sizeof(WFP_PROCESS));

	wfp_proc->ProcessId = proc->pid;

	List_Init(&wfp_proc->NetFwRules);

#ifdef _WIN64
	irql = KeAcquireSpinLockRaiseToDpc(&WFP_MapLock);
#else
	KeAcquireSpinLock(&WFP_MapLock, &irql);
#endif

	if(map_get(&WFP_Processes, wfp_proc->ProcessId) != NULL)
		ok = FALSE; // that would be a duplicate, should not happen, but in case
	else if (!map_insert(&WFP_Processes, wfp_proc->ProcessId, wfp_proc, 0))
		ok = FALSE;
    
	KeReleaseSpinLock(&WFP_MapLock, irql);

finish:
	if(!ok && wfp_proc)
		WFP_Free(NULL, wfp_proc);
	return ok;
}


//---------------------------------------------------------------------------
// WFP_UpdateProcess
//---------------------------------------------------------------------------


BOOLEAN WFP_UpdateProcess(PROCESS* proc)
{
	if (!WFP_Enabled)
		return TRUE; // nothing to do

	BOOLEAN ok = FALSE;
	KIRQL irql;
	WFP_PROCESS* wfp_proc;
	HANDLE processId = proc->pid;
	BOOLEAN LogTraffic = FALSE;
	BOOLEAN BlockInternet = FALSE;
	LIST NewNetFwRules, OldNetFwRules;
	
	List_Init(&NewNetFwRules);
	List_Init(&OldNetFwRules);

	LogTraffic = Process_GetTraceFlag(proc, L"NetFwTrace") != 0;

	if (!proc->AllowInternetAccess) { // if the process isn't exempted check the config

		BlockInternet = !Process_GetConf_bool(proc, L"AllowNetworkAccess", TRUE);
	}

	if (!BlockInternet) {

		ok = WFP_LoadRules(&NewNetFwRules, proc);

		if (!ok) {
			memcpy(&OldNetFwRules, &NewNetFwRules, sizeof(LIST));
			BlockInternet = TRUE; // on roule failure we lust block everything
			// todo: log error
		}
	}

#ifdef _WIN64
	irql = KeAcquireSpinLockRaiseToDpc(&WFP_MapLock);
#else
	KeAcquireSpinLock(&WFP_MapLock, &irql);
#endif

	wfp_proc = map_get(&WFP_Processes, processId);
	if (wfp_proc) {

		wfp_proc->LogTraffic = LogTraffic;
		wfp_proc->BlockInternet = BlockInternet;

		if (ok) {
			memcpy(&OldNetFwRules, &wfp_proc->NetFwRules, sizeof(LIST));
			memcpy(&wfp_proc->NetFwRules, &NewNetFwRules, sizeof(LIST));
		}
		ok = TRUE;
	}
	else {
		if (ok) {
			memcpy(&OldNetFwRules, &NewNetFwRules, sizeof(LIST));
		}
		ok = FALSE;
	}
    
	KeReleaseSpinLock(&WFP_MapLock, irql);

	WFP_FreeRules(&OldNetFwRules);

	return ok;
}


//---------------------------------------------------------------------------
// WFP_DeleteProcess
//---------------------------------------------------------------------------


void WFP_DeleteProcess(PROCESS* proc)
{
	if (!WFP_Enabled)
		return; // nothing to do

	KIRQL irql; 
	WFP_PROCESS* wfp_proc = NULL;
	HANDLE processId = proc->pid;

#ifdef _WIN64
	irql = KeAcquireSpinLockRaiseToDpc(&WFP_MapLock);
#else
	KeAcquireSpinLock(&WFP_MapLock, &irql);
#endif

	map_take(&WFP_Processes, processId, &wfp_proc, 0);
    
	KeReleaseSpinLock(&WFP_MapLock, irql);

	if (wfp_proc)
	{
		WFP_FreeRules(&wfp_proc->NetFwRules);
		WFP_Free(NULL, wfp_proc);
	}
}

//---------------------------------------------------------------------------
// WFP_classify
//---------------------------------------------------------------------------


void WFP_classify(
	const FWPS_INCOMING_VALUES * inFixedValues,
	const FWPS_INCOMING_METADATA_VALUES * inMetaValues,
	void * layerData,
	const void * classifyContext,
	const FWPS_FILTER * filter,
	UINT64 flowContext,
	FWPS_CLASSIFY_OUT * classifyOut)
{
	// https://docs.microsoft.com/en-us/windows-hardware/drivers/network/metadata-fields-at-each-filtering-layer

	UNREFERENCED_PARAMETER(inMetaValues);
	UNREFERENCED_PARAMETER(layerData);
	UNREFERENCED_PARAMETER(classifyContext);
	UNREFERENCED_PARAMETER(flowContext);
	UNREFERENCED_PARAMETER(filter);

	//
	// We don't have the necessary right to alter the classify, exit.
	//
	if ((classifyOut->rights & FWPS_RIGHT_ACTION_WRITE) == 0)
	{
		return;
	}

	if (FWPS_IS_METADATA_FIELD_PRESENT(inMetaValues, FWPS_METADATA_FIELD_PROCESS_ID))
	{
		UINT localAddrIndex, remoteAddrIndex, localPortIndex, remotePortIndex, protocolIndex;
		GetNetwork5TupleIndexesForLayer(inFixedValues->layerId,
		  &localAddrIndex, &remoteAddrIndex, &localPortIndex, &remotePortIndex,&protocolIndex);


		int protocol = inFixedValues->incomingValue[protocolIndex].value.uint8;
		IP_ADDRESS remote_ip;
		if ((filter->filterId == WFP_send_filter_id_v6) || (filter->filterId == WFP_recv_filter_id_v6))
		{
			//remote_ip.Type = AF_INET6;
			//UINT8* local_address = inFixedValues->incomingValue[localAddrIndex].value.byteArray16->byteArray16;
			UINT8* remote_address = inFixedValues->incomingValue[remoteAddrIndex].value.byteArray16->byteArray16;

			memcpy(remote_ip.Data, remote_address, 16);
		}
		else
		{
			//remote_ip.Type = AF_INET;
			//UINT32 local_address = inFixedValues->incomingValue[localAddrIndex].value.uint32;
			UINT32 remote_address = inFixedValues->incomingValue[remoteAddrIndex].value.uint32;

			// IPv4-mapped IPv6 addresses, eg. ::FFFF:192.168.0.1
            remote_ip.Data32[0] = 0;
            remote_ip.Data32[1] = 0;
            remote_ip.Data32[2] = 0xFFFF0000;
            remote_ip.Data32[3] = _ntohl(remote_address); // to network order, as ipv6 is also in network order
			//*((ULONG*)remote_ip.Data) = _ntohl(remote_address);
		}
		//UINT16 local_port = inFixedValues->incomingValue[localPortIndex].value.uint16;
		UINT16 remote_port = inFixedValues->incomingValue[remotePortIndex].value.uint16;


		BOOLEAN log = FALSE;
		BOOLEAN block = FALSE;


		KIRQL irql; 
		WFP_PROCESS* wfp_proc;
		HANDLE processId = (HANDLE)inMetaValues->processId;

#ifdef _WIN64
		irql = KeAcquireSpinLockRaiseToDpc(&WFP_MapLock);
#else
		KeAcquireSpinLock(&WFP_MapLock, &irql);
#endif

		wfp_proc = map_get(&WFP_Processes, processId);
		if (wfp_proc) {

			log = wfp_proc->LogTraffic;
			block = wfp_proc->BlockInternet;

			if (!block) {

				block = NetFw_BlockTraffic(&wfp_proc->NetFwRules, &remote_ip, remote_port, protocol);
			}
		}
    
		KeReleaseSpinLock(&WFP_MapLock, irql);


        if (log){

			BOOLEAN send = (filter->filterId == WFP_send_filter_id_v4) || (filter->filterId == WFP_send_filter_id_v6);
			BOOLEAN v6 = (filter->filterId == WFP_send_filter_id_v6) || (filter->filterId == WFP_recv_filter_id_v6);

			/*
			RtlStringCbPrintfW at DISPATCH_LEVEL or higher can cause a BSOD, 
			the issue is with accessing unicode tables, which may be paged out.

			The documentation for KdPrint() states it this way:

			<wdk>
			Format
			Specifies a pointer to the format string to print. The Format string
			supports all the printf-style formatting codes. However, the Unicode format
			codes (%C, %S, %lc, %ls, %wc, %ws, and %wZ) can only be used with IRQL =
			PASSIVE_LEVEL.
			</wdk>

			RtlStringCbPrintfA is technically also not permitted so a better solution needs to be found
			*/

			char trace_strA[256];
			if (v6) {
				RtlStringCbPrintfA(trace_strA, sizeof(trace_strA), "%s Network Traffic; Port: %u; Prot: %u; IPv6: %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x", 
					send ? "Outgoing " : "Incoming ", remote_port, protocol,
					remote_ip.Data[0], remote_ip.Data[1], remote_ip.Data[2], remote_ip.Data[3], remote_ip.Data[4], remote_ip.Data[5], remote_ip.Data[6], remote_ip.Data[7],
					remote_ip.Data[8], remote_ip.Data[9], remote_ip.Data[10], remote_ip.Data[11], remote_ip.Data[12], remote_ip.Data[13], remote_ip.Data[14], remote_ip.Data[15]);
			}
			else {
				RtlStringCbPrintfA(trace_strA, sizeof(trace_strA), "%s Network Traffic; Port: %u; Prot: %u; IPv4: %d.%d.%d.%d", 
					send ? "Outgoing " : "Incoming ", remote_port, protocol,
					remote_ip.Data[12], remote_ip.Data[13], remote_ip.Data[14], remote_ip.Data[15]);
			}

			WCHAR trace_str[256];
			char* cptr = trace_strA;
			WCHAR* wptr = trace_str;
			while (*cptr != '\0')
				*wptr++ = *cptr++;
			*wptr = L'\0';

            Session_MonitorPut(MONITOR_NETFW | (block ? MONITOR_DENY : MONITOR_OPEN), trace_str, PsGetCurrentProcessId());
        }

		if (block) {

			classifyOut->actionType = FWP_ACTION_BLOCK;
			classifyOut->rights &= ~FWPS_RIGHT_ACTION_WRITE;
			return;
		}
	}

	classifyOut->actionType = FWP_ACTION_PERMIT;
	return;
}


//---------------------------------------------------------------------------
// WFP_notify
//---------------------------------------------------------------------------


NTSTATUS WFP_notify(
	FWPS_CALLOUT_NOTIFY_TYPE notifyType,
	const GUID * filterKey,
	const FWPS_FILTER * filter)
{
	UNREFERENCED_PARAMETER(notifyType);
	UNREFERENCED_PARAMETER(filterKey);
	UNREFERENCED_PARAMETER(filter);
	return STATUS_SUCCESS;
}


//---------------------------------------------------------------------------
// WFP_flow_delete
//---------------------------------------------------------------------------


NTSTATUS WFP_flow_delete(UINT16 layerId, UINT32 calloutId, UINT64 flowContext)
{
	UNREFERENCED_PARAMETER(layerId);
	UNREFERENCED_PARAMETER(calloutId);
	UNREFERENCED_PARAMETER(flowContext);
	return STATUS_SUCCESS;
}


//---------------------------------------------------------------------------
// GetNetwork5TupleIndexesForLayer
//---------------------------------------------------------------------------


void
GetNetwork5TupleIndexesForLayer(
   _In_ UINT16 layerId,
   _Out_ UINT* localAddressIndex,
   _Out_ UINT* remoteAddressIndex,
   _Out_ UINT* localPortIndex,
   _Out_ UINT* remotePortIndex,
   _Out_ UINT* protocolIndex
   )
{
   switch (layerId)
   {
   case FWPS_LAYER_ALE_AUTH_CONNECT_V4:
      *localAddressIndex = FWPS_FIELD_ALE_AUTH_CONNECT_V4_IP_LOCAL_ADDRESS;
      *remoteAddressIndex = FWPS_FIELD_ALE_AUTH_CONNECT_V4_IP_REMOTE_ADDRESS;
      *localPortIndex = FWPS_FIELD_ALE_AUTH_CONNECT_V4_IP_LOCAL_PORT;
      *remotePortIndex = FWPS_FIELD_ALE_AUTH_CONNECT_V4_IP_REMOTE_PORT;
      *protocolIndex = FWPS_FIELD_ALE_AUTH_CONNECT_V4_IP_PROTOCOL;
      break;
   case FWPS_LAYER_ALE_AUTH_CONNECT_V6:
      *localAddressIndex = FWPS_FIELD_ALE_AUTH_CONNECT_V6_IP_LOCAL_ADDRESS;
      *remoteAddressIndex = FWPS_FIELD_ALE_AUTH_CONNECT_V6_IP_REMOTE_ADDRESS;
      *localPortIndex = FWPS_FIELD_ALE_AUTH_CONNECT_V6_IP_LOCAL_PORT;
      *remotePortIndex = FWPS_FIELD_ALE_AUTH_CONNECT_V6_IP_REMOTE_PORT;
      *protocolIndex = FWPS_FIELD_ALE_AUTH_CONNECT_V6_IP_PROTOCOL;
      break;
   case FWPS_LAYER_ALE_AUTH_RECV_ACCEPT_V4:
      *localAddressIndex = FWPS_FIELD_ALE_AUTH_RECV_ACCEPT_V4_IP_LOCAL_ADDRESS;
      *remoteAddressIndex = FWPS_FIELD_ALE_AUTH_RECV_ACCEPT_V4_IP_REMOTE_ADDRESS;
      *localPortIndex = FWPS_FIELD_ALE_AUTH_RECV_ACCEPT_V4_IP_LOCAL_PORT;
      *remotePortIndex = FWPS_FIELD_ALE_AUTH_RECV_ACCEPT_V4_IP_REMOTE_PORT;
      *protocolIndex = FWPS_FIELD_ALE_AUTH_RECV_ACCEPT_V4_IP_PROTOCOL;
      break;
   case FWPS_LAYER_ALE_AUTH_RECV_ACCEPT_V6:
      *localAddressIndex = FWPS_FIELD_ALE_AUTH_RECV_ACCEPT_V6_IP_LOCAL_ADDRESS;
      *remoteAddressIndex = FWPS_FIELD_ALE_AUTH_RECV_ACCEPT_V6_IP_REMOTE_ADDRESS;
      *localPortIndex = FWPS_FIELD_ALE_AUTH_RECV_ACCEPT_V6_IP_LOCAL_PORT;
      *remotePortIndex = FWPS_FIELD_ALE_AUTH_RECV_ACCEPT_V6_IP_REMOTE_PORT;
      *protocolIndex = FWPS_FIELD_ALE_AUTH_RECV_ACCEPT_V6_IP_PROTOCOL;
      break;
   case FWPS_LAYER_OUTBOUND_TRANSPORT_V4:
      *localAddressIndex = FWPS_FIELD_OUTBOUND_TRANSPORT_V4_IP_LOCAL_ADDRESS;
      *remoteAddressIndex = FWPS_FIELD_OUTBOUND_TRANSPORT_V4_IP_REMOTE_ADDRESS;
      *localPortIndex = FWPS_FIELD_OUTBOUND_TRANSPORT_V4_IP_LOCAL_PORT;
      *remotePortIndex = FWPS_FIELD_OUTBOUND_TRANSPORT_V4_IP_REMOTE_PORT;
      *protocolIndex = FWPS_FIELD_OUTBOUND_TRANSPORT_V4_IP_PROTOCOL;
      break;
   case FWPS_LAYER_OUTBOUND_TRANSPORT_V6:
      *localAddressIndex = FWPS_FIELD_OUTBOUND_TRANSPORT_V6_IP_LOCAL_ADDRESS;
      *remoteAddressIndex = FWPS_FIELD_OUTBOUND_TRANSPORT_V6_IP_REMOTE_ADDRESS;
      *localPortIndex = FWPS_FIELD_OUTBOUND_TRANSPORT_V6_IP_LOCAL_PORT;
      *remotePortIndex = FWPS_FIELD_OUTBOUND_TRANSPORT_V6_IP_REMOTE_PORT;
      *protocolIndex = FWPS_FIELD_OUTBOUND_TRANSPORT_V6_IP_PROTOCOL;
      break;
   case FWPS_LAYER_INBOUND_TRANSPORT_V4:
      *localAddressIndex = FWPS_FIELD_INBOUND_TRANSPORT_V4_IP_LOCAL_ADDRESS;
      *remoteAddressIndex = FWPS_FIELD_INBOUND_TRANSPORT_V4_IP_REMOTE_ADDRESS;
      *localPortIndex = FWPS_FIELD_INBOUND_TRANSPORT_V4_IP_LOCAL_PORT;
      *remotePortIndex = FWPS_FIELD_INBOUND_TRANSPORT_V4_IP_REMOTE_PORT;
      *protocolIndex = FWPS_FIELD_INBOUND_TRANSPORT_V4_IP_PROTOCOL;
      break;
   case FWPS_LAYER_INBOUND_TRANSPORT_V6:
      *localAddressIndex = FWPS_FIELD_INBOUND_TRANSPORT_V6_IP_LOCAL_ADDRESS;
      *remoteAddressIndex = FWPS_FIELD_INBOUND_TRANSPORT_V6_IP_REMOTE_ADDRESS;
      *localPortIndex = FWPS_FIELD_INBOUND_TRANSPORT_V6_IP_LOCAL_PORT;
      *remotePortIndex = FWPS_FIELD_INBOUND_TRANSPORT_V6_IP_REMOTE_PORT;
      *protocolIndex = FWPS_FIELD_INBOUND_TRANSPORT_V6_IP_PROTOCOL;
      break;
   default:
      *localAddressIndex = -1;
      *remoteAddressIndex = -1;
      *localPortIndex = -1;
      *remotePortIndex = -1;
      *protocolIndex = -1;      
      NT_ASSERT(0);
   }
}
