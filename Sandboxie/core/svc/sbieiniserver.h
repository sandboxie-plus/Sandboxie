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
// Sandboxie Ini Server -- using PipeServer
//---------------------------------------------------------------------------


#ifndef _MY_SBIEINISERVER_H
#define _MY_SBIEINISERVER_H


#include "PipeServer.h"


class SbieIniServer
{

public:

    SbieIniServer(PipeServer *pipeServer);

    ~SbieIniServer();

    static void NotifyConfigReloaded();

    static bool TokenIsAdmin(HANDLE hToken, bool OnlyFull = false);

protected:

    static MSG_HEADER *Handler(void *_this, MSG_HEADER *msg);

    MSG_HEADER *Handler2(MSG_HEADER *msg);

    MSG_HEADER *GetVersion(MSG_HEADER *msg);

    MSG_HEADER *GetWaitHandle(HANDLE idProcess);

    MSG_HEADER *GetUser(MSG_HEADER *msg);

    MSG_HEADER *GetPath(MSG_HEADER *msg);

    ULONG CheckRequest(MSG_HEADER *msg);

    bool SetUserSettingsSectionName(HANDLE hToken);

    static bool SetUserSettingsSectionName(HANDLE hToken, WCHAR* m_username, WCHAR* m_sectionname);

    bool UserCanEdit(HANDLE hToken);

    ULONG CacheConfig();

    MSG_HEADER *GetSetting(MSG_HEADER *msg);

    ULONG SetSetting(MSG_HEADER *msg);

    ULONG AddSetting(MSG_HEADER *msg, bool insert);

    ULONG DelSetting(MSG_HEADER *msg);

    ULONG SetTemplate(MSG_HEADER *msg);

    ULONG SetOrTestPassword(MSG_HEADER *msg);

    ULONG RefreshConf();

    bool GetIniPath(WCHAR **IniPath, BOOLEAN *IsHomePath = NULL);

    ULONG IsCallerAuthorized(HANDLE hToken, const WCHAR *Password, const WCHAR *Section = NULL);

    void LockConf(WCHAR *IniPath);

    void UnlockConf();

    bool HashPassword(const WCHAR *Password, WCHAR *Hash41);

    MSG_HEADER *RunSbieCtrl(MSG_HEADER *msg, HANDLE idProcess, bool isSandboxed);

    MSG_HEADER *SetDatFile(MSG_HEADER *msg, HANDLE idProcess);
	
    //MSG_HEADER *GetDatFile(MSG_HEADER *msg, HANDLE idProcess);

    MSG_HEADER *RC4Crypt(MSG_HEADER *msg, HANDLE idProcess, bool isSandboxed);

public:

    static NTSTATUS RunSbieCtrl(HANDLE hToken, const WCHAR* DeskName, const WCHAR* CtrlCmd = NULL, size_t CtrlCmdLen = 0);

protected:

    CRITICAL_SECTION m_critsec;
    static SbieIniServer* m_instance;
    WCHAR m_username[256];
    WCHAR m_sectionname[128];
    class CIniFile* m_pSbieIni;
    HANDLE m_hLockFile;
    ULONG m_session_id;

};


#endif /* _MY_SBIEINISERVER_H */
