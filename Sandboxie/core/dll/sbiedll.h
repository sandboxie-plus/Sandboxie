/*
 * Copyright 2004-2020 Sandboxie Holdings, LLC 
 * Copyright 2020-2024 David Xanatos, xanasoft.com
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
// Sandboxie DLL
//---------------------------------------------------------------------------


#ifndef _MY_SBIEDLL_H
#define _MY_SBIEDLL_H


#include "sbieapi.h"


#ifdef __cplusplus
extern "C" {
#endif


#ifndef SBIEDLL_EXPORT
#define SBIEDLL_EXPORT  __declspec(dllexport)
#endif


//---------------------------------------------------------------------------
// Defines
//---------------------------------------------------------------------------

#define USE_MATCH_PATH_EX

#define TokenElevationTypeNone 99

#define ENV_VAR_PFX            L"00000000_" SBIE L"_"
#define DATA_SLOTS 5
#define SESSION_PROCESS L"SboxSession"

typedef struct _PROCESS_DATA {
    ULONG tid;
    ULONG initFlag;
    HANDLE hStartLingerEvent;
    DWORD state;
    DWORD errorCode;
    DWORD checkpoint;
    WCHAR * name;
    WCHAR * EventName;
} PROCESS_DATA;

PROCESS_DATA *my_findProcessData(WCHAR *name,int createNew);

//---------------------------------------------------------------------------
// Functions (DllMain)
//---------------------------------------------------------------------------

SBIEDLL_EXPORT  void SbieDll_HookInit();

SBIEDLL_EXPORT  void *SbieDll_Hook(
    const char *SourceFuncName, void *SourceFunc, void *DetourFunc, HMODULE module);

#define SBIEDLL_HOOK(pfx,proc)                  \
    *(ULONG_PTR *)&__sys_##proc = (ULONG_PTR)   \
        SbieDll_Hook(#proc, proc, pfx##proc, module);   \
    if (! __sys_##proc) return FALSE;

SBIEDLL_EXPORT  void SbieDll_TraceModule(HMODULE module);

SBIEDLL_EXPORT  void SbieDll_UnHookModule(HMODULE module);

SBIEDLL_EXPORT  void SbieDll_DeviceChange(WPARAM wParam, LPARAM lParam);

SBIEDLL_EXPORT  BOOL SbieDll_QueryFileAttributes(const WCHAR *NtPath, ULONG64 *size, ULONG64 *date, ULONG *attrs);

SBIEDLL_EXPORT  const WCHAR *SbieDll_GetDrivePath(ULONG DriveIndex);

SBIEDLL_EXPORT  const WCHAR *SbieDll_GetUserPathEx(WCHAR which);

SBIEDLL_EXPORT  BOOLEAN SbieDll_TranslateNtToDosPath(WCHAR *path);

SBIEDLL_EXPORT  BOOLEAN SbieDll_StartSbieSvc(BOOLEAN retry);

SBIEDLL_EXPORT  const WCHAR *SbieDll_GetStartError(void);

SBIEDLL_EXPORT  BOOLEAN SbieDll_GetServiceRegistryValue(
    const WCHAR *name, void *kvpi, ULONG sizeof_kvpi);

SBIEDLL_EXPORT  ULONG SbieDll_GetLanguage(BOOLEAN *rtl);

SBIEDLL_EXPORT  BOOLEAN SbieDll_KillOne(ULONG ProcessId);

SBIEDLL_EXPORT  BOOLEAN SbieDll_KillAll(
    ULONG SessionId, const WCHAR *BoxName);

SBIEDLL_EXPORT  BOOLEAN SbieDll_Mount(const WCHAR *BoxName, const WCHAR * BoxKey, BOOLEAN Protect);

SBIEDLL_EXPORT  BOOLEAN SbieDll_Unmount(const WCHAR *BoxName);

SBIEDLL_EXPORT  ULONG SbieDll_GetTokenElevationType(void);

SBIEDLL_EXPORT  WCHAR *SbieDll_FormatMessage(ULONG code, const WCHAR **ins);

SBIEDLL_EXPORT  WCHAR *SbieDll_FormatMessage0(ULONG code);

SBIEDLL_EXPORT  WCHAR *SbieDll_FormatMessage1(ULONG code, const WCHAR *ins1);

SBIEDLL_EXPORT  WCHAR *SbieDll_FormatMessage2(
    ULONG code, const WCHAR *ins1, const WCHAR *ins2);

SBIEDLL_EXPORT  BOOL SbieDll_RunSandboxed(
    const WCHAR *box_name, const WCHAR *cmd, const WCHAR *dir,
    ULONG creation_flags, STARTUPINFO *si, PROCESS_INFORMATION *pi);

//---------------------------------------------------------------------------
// Functions (CallSvc)
//---------------------------------------------------------------------------


SBIEDLL_EXPORT  const WCHAR *SbieDll_PortName(void);

SBIEDLL_EXPORT  struct _MSG_HEADER *SbieDll_CallServer(
    struct _MSG_HEADER *req);

SBIEDLL_EXPORT  void *SbieDll_CallServerQueue(
	const WCHAR* queue, void *req, ULONG req_len, ULONG rpl_min_len);

SBIEDLL_EXPORT  void* SbieDll_AllocMem(ULONG size);

SBIEDLL_EXPORT  void SbieDll_FreeMem(void *data);

SBIEDLL_EXPORT  ULONG SbieDll_QueueCreate(
    const WCHAR *QueueName, HANDLE *out_EventHandle);

SBIEDLL_EXPORT  ULONG SbieDll_QueueGetReq(
    const WCHAR *QueueName, ULONG *out_ClientPid, ULONG *out_ClientTid,
    ULONG *out_RequestId, void **out_DataPtr, ULONG *out_DataLen);

SBIEDLL_EXPORT  ULONG SbieDll_QueuePutRpl(
    const WCHAR *QueueName, ULONG RequestId, void *DataPtr, ULONG DataLen);

SBIEDLL_EXPORT  ULONG SbieDll_QueuePutReq(
    const WCHAR *QueueName, void *DataPtr, ULONG DataLen,
    ULONG *out_RequestId, HANDLE *out_EventHandle);

SBIEDLL_EXPORT  ULONG SbieDll_QueueGetRpl(
    const WCHAR *QueueName, ULONG RequestId,
    void **out_DataPtr, ULONG *out_DataLen);

SBIEDLL_EXPORT  void *SbieDll_CallProxySvr(
    WCHAR *QueueName, void *req, ULONG req_len, ULONG rpl_min_len, DWORD timeout_sec);

SBIEDLL_EXPORT  ULONG SbieDll_UpdateConf(
    WCHAR OpCode, const WCHAR *Password, const WCHAR *Section,
    const WCHAR *Setting, const WCHAR *Value);

SBIEDLL_EXPORT  ULONG SbieDll_QueryConf(
    const WCHAR *Section, const WCHAR *Setting,
    ULONG setting_index, WCHAR *out_buffer, ULONG buffer_len);

//---------------------------------------------------------------------------
// Functions (Other)
//---------------------------------------------------------------------------


SBIEDLL_EXPORT  BOOLEAN SbieDll_StartCOM(BOOLEAN Async);

SBIEDLL_EXPORT  BOOLEAN SbieDll_IsOpenCOM(void);

SBIEDLL_EXPORT  BOOLEAN SbieDll_IsDirectory(const WCHAR *PathW);

SBIEDLL_EXPORT  void *SbieDll_InitPStore(void);

SBIEDLL_EXPORT  ULONG SbieDll_GetHandlePath(
    HANDLE FileHandle, WCHAR *OutWchar8192, BOOLEAN *IsBoxedPath);

SBIEDLL_EXPORT  BOOLEAN SbieDll_RunFromHome(
    const WCHAR *pgmName, const WCHAR *pgmArgs,
    STARTUPINFOW *si, PROCESS_INFORMATION *pi);

SBIEDLL_EXPORT  WCHAR *SbieDll_AssocQueryCommand(const WCHAR *subj);

SBIEDLL_EXPORT  WCHAR *SbieDll_AssocQueryProgram(const WCHAR *subj);

SBIEDLL_EXPORT  BOOLEAN SbieDll_IsBoxedService(HANDLE hService);

SBIEDLL_EXPORT  BOOL SbieDll_StartBoxedService(
    const WCHAR *ServiceName, BOOLEAN WithAdd);

SBIEDLL_EXPORT  BOOL SbieDll_CheckProcessLocalSystem(HANDLE ProcessHandle);

SBIEDLL_EXPORT  VOID SbieDll_SetFakeAdmin(BOOLEAN FakeAdmin);

SBIEDLL_EXPORT  HANDLE SbieDll_OpenProcess(ACCESS_MASK DesiredAccess, HANDLE idProcess);

SBIEDLL_EXPORT  HRESULT SbieDll_ComCreateProxy(
    REFIID riid, void *pUnkOuter, void *pChannel, void **ppUnknown);

SBIEDLL_EXPORT  HRESULT SbieDll_ComCreateStub(
    REFIID riid, void *pUnknown, void **ppStub, void **ppChannel);

SBIEDLL_EXPORT  BOOLEAN SbieDll_IsOpenClsid(
    REFCLSID rclsid, ULONG clsctx, const WCHAR *BoxName);

SBIEDLL_EXPORT  void SbieDll_DisableElevationHook(void);

SBIEDLL_EXPORT  BOOLEAN SbieDll_RegisterDllCallback(void *Callback);

SBIEDLL_EXPORT  BOOLEAN SbieDll_IsDllSkipHook(const WCHAR* ImageName);

SBIEDLL_EXPORT  BOOLEAN SbieDll_ExpandAndRunProgram(const WCHAR *Command);


SBIEDLL_EXPORT  ULONG SbieDll_InjectLow_InitHelper();
SBIEDLL_EXPORT  ULONG SbieDll_InjectLow_InitSyscalls(BOOLEAN drv_init);
SBIEDLL_EXPORT  ULONG SbieDll_InjectLow(HANDLE hProcess, ULONG init_flags, BOOLEAN dup_drv_handle);


SBIEDLL_EXPORT  BOOLEAN SbieDll_MatchImage(const WCHAR* pat_str, const WCHAR* test_str, const WCHAR* BoxName);

SBIEDLL_EXPORT  BOOLEAN SbieDll_GetStringForStringList(const WCHAR* string, const WCHAR* boxname, const WCHAR* setting, WCHAR* value, ULONG value_size);
SBIEDLL_EXPORT  BOOLEAN SbieDll_GetStringsForStringList(const WCHAR* string, const WCHAR* boxname, const WCHAR* setting, int pos, WCHAR* value, ULONG value_size);
SBIEDLL_EXPORT  BOOLEAN SbieDll_CheckStringInList(const WCHAR* string, const WCHAR* boxname, const WCHAR* setting);
SBIEDLL_EXPORT  BOOLEAN SbieDll_CheckStringInListA(const char* string, const WCHAR* boxname, const WCHAR* setting);

SBIEDLL_EXPORT  BOOLEAN SbieDll_CheckPatternInList(const WCHAR* string, ULONG length, const WCHAR* boxname, const WCHAR* setting);

SBIEDLL_EXPORT  BOOLEAN SbieDll_GetSettingsForName(
    const WCHAR* boxname, const WCHAR* name, const WCHAR* setting, WCHAR* value, ULONG value_size, const WCHAR* deftext);

SBIEDLL_EXPORT  BOOLEAN SbieDll_GetSettingsForName_bool(
    const WCHAR* boxname, const WCHAR* name, const WCHAR* setting, BOOLEAN defval);

SBIEDLL_EXPORT  BOOLEAN SbieDll_GetBorderColor(const WCHAR* box_name, COLORREF* color, BOOL* title, int* width);

SBIEDLL_EXPORT  BOOLEAN SbieDll_IsReservedFileName(const WCHAR* name);

SBIEDLL_EXPORT  PSECURITY_DESCRIPTOR SbieDll_GetPublicSD();

SBIEDLL_EXPORT  const WCHAR* SbieDll_FindArgumentEnd(const WCHAR* arguments);

SBIEDLL_EXPORT  ULONG_PTR Scm_HookSetServiceStatus(VOID* hook);
SBIEDLL_EXPORT  ULONG_PTR Scm_HookStartServiceCtrlDispatcherW(VOID* hook);
SBIEDLL_EXPORT  ULONG_PTR Scm_HookOpenServiceW(VOID* hook);
SBIEDLL_EXPORT  ULONG_PTR Scm_HookCloseServiceHandle(VOID* hook);
SBIEDLL_EXPORT  ULONG_PTR Scm_HookQueryServiceStatusEx(VOID* hook);
SBIEDLL_EXPORT  ULONG_PTR Scm_HookQueryServiceStatus(VOID* hook);
SBIEDLL_EXPORT  ULONG_PTR Scm_HookStartServiceW(VOID* hook);
SBIEDLL_EXPORT  ULONG_PTR Scm_HookControlService(VOID* hook);


#ifdef USE_MATCH_PATH_EX
//SBIEDLL_EXPORT ULONG SbieDll_MatchPathImpl(BOOLEAN use_rule_specificity, BOOLEAN use_privacy_mode, const WCHAR* path, void* normal_list, void* open_list, void* closed_list, void* write_list, void* read_list);
SBIEDLL_EXPORT ULONG SbieDll_MatchPathImpl(BOOLEAN use_rule_specificity, const WCHAR* path, void* normal_list, void* open_list, void* closed_list, void* write_list, void* read_list);
#else
SBIEDLL_EXPORT ULONG SbieDll_MatchPathImpl(const WCHAR* path, void* open_list, void* closed_list, void* write_list);
#endif

#define PATH_OPEN_FLAG      0x10
#define PATH_CLOSED_FLAG    0x20
#define PATH_WRITE_FLAG     0x40
#define PATH_READ_FLAG      0x80

#ifdef USE_MATCH_PATH_EX
// for read only paths, handle like open and let the driver deny the write access
#define PATH_IS_OPEN(f)     ((((f) & PATH_OPEN_FLAG) != 0) || PATH_IS_READ(f))
#define PATH_NOT_OPEN(f)    ((((f) & PATH_OPEN_FLAG) == 0) && PATH_NOT_READ(f))
#else
#define PATH_IS_OPEN(f)     (((f) & PATH_OPEN_FLAG) != 0)
#define PATH_NOT_OPEN(f)    (((f) & PATH_OPEN_FLAG) == 0)
#endif

#define PATH_IS_CLOSED(f)   (((f) & PATH_CLOSED_FLAG) != 0)
#define PATH_NOT_CLOSED(f)  (((f) & PATH_CLOSED_FLAG) == 0)

#define PATH_IS_WRITE(f)    (((f) & PATH_WRITE_FLAG) != 0)
#define PATH_NOT_WRITE(f)   (((f) & PATH_WRITE_FLAG) == 0)

#define PATH_IS_READ(f)     (((f) & PATH_READ_FLAG) != 0)
#define PATH_NOT_READ(f)    (((f) & PATH_READ_FLAG) == 0)



SBIEDLL_EXPORT  void DbgPrint(const char* format, ...);
SBIEDLL_EXPORT  void DbgTrace(const char* format, ...);

SBIEDLL_EXPORT  BOOLEAN SbieDll_DisableCHPE(void);

//---------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif /* _MY_SBIEDLL_H */
