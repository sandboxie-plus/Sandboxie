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


SBIEDLL_EXPORT  void *SbieDll_Hook(
    const char *SourceFuncName, void *SourceFunc, void *DetourFunc);

#define SBIEDLL_HOOK(pfx,proc)                  \
    *(ULONG_PTR *)&__sys_##proc = (ULONG_PTR)   \
        SbieDll_Hook(#proc, proc, pfx##proc);   \
    if (! __sys_##proc) return FALSE;

SBIEDLL_EXPORT  void SbieDll_DeviceChange(WPARAM wParam, LPARAM lParam);

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

SBIEDLL_EXPORT  ULONG SbieDll_UpdateConf(
    WCHAR OpCode, const WCHAR *Password, const WCHAR *Section,
    const WCHAR *Setting, const WCHAR *Value);


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

SBIEDLL_EXPORT  HRESULT SbieDll_ComCreateProxy(
    REFIID riid, void *pUnkOuter, void *pChannel, void **ppUnknown);

SBIEDLL_EXPORT  HRESULT SbieDll_ComCreateStub(
    REFIID riid, void *pUnknown, void **ppStub, void **ppChannel);

SBIEDLL_EXPORT  BOOLEAN SbieDll_IsOpenClsid(
    REFCLSID rclsid, ULONG clsctx, const WCHAR *BoxName);

SBIEDLL_EXPORT  void SbieDll_DisableElevationHook(void);

SBIEDLL_EXPORT  BOOLEAN SbieDll_RegisterDllCallback(void *Callback);

SBIEDLL_EXPORT  BOOLEAN SbieDll_ExpandAndRunProgram(const WCHAR *Command);


SBIEDLL_EXPORT  ULONG SbieDll_InjectLow_InitHelper();
SBIEDLL_EXPORT  ULONG SbieDll_InjectLow_InitSyscalls(BOOLEAN drv_init);
SBIEDLL_EXPORT  ULONG SbieDll_InjectLow(HANDLE hProcess, BOOLEAN is_wow64, BOOLEAN bHostInject, BOOLEAN dup_drv_handle);


SBIEDLL_EXPORT  BOOLEAN SbieDll_MatchImage(const WCHAR* pat_str, const WCHAR* test_str, const WCHAR* BoxName);

SBIEDLL_EXPORT  BOOLEAN SbieDll_GetStringForStringList(const WCHAR* string, const WCHAR* boxname, const WCHAR* setting, WCHAR* value, ULONG value_size);
SBIEDLL_EXPORT  BOOLEAN SbieDll_CheckStringInList(const WCHAR* string, const WCHAR* boxname, const WCHAR* setting);
//SBIEDLL_EXPORT  BOOLEAN SbieDll_GetBoolForStringFromList(const WCHAR* string, const WCHAR* boxname, const WCHAR* setting, BOOLEAN def_found, BOOLEAN not_found);

SBIEDLL_EXPORT  BOOLEAN SbieDll_GetBorderColor(const WCHAR* box_name, COLORREF* color, BOOL* title, int* width);


//---------------------------------------------------------------------------

SBIEDLL_EXPORT  BOOLEAN SbieDll_IsReservedFileName(const WCHAR* name);

//---------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif /* _MY_SBIEDLL_H */
