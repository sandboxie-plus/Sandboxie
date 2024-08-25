/*
 * Copyright 2023 David Xanatos, xanasoft.com
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
// Box Manager
//---------------------------------------------------------------------------


#ifndef _MY_BOXMANAGER_H
#define _MY_BOXMANAGER_H


#include <windows.h>
#include "common/win32_ntddk.h"
#include "common/list.h"
#include "common/map.h"
#include "common/pool.h"

#include <string>
#include <map>
#include <list>
#include <vector>
#include <memory>

struct BOX_INSTANCE;
struct BOXED_PROCESS;

class BoxManager
{

public:

    static BoxManager *GetInstance();

    void LoadProcesses();

    bool ProcessCreated(ULONG ProcessId, const WCHAR* boxname, const WCHAR* reg_root, ULONG session_id);

    void BoxOpened(const WCHAR* reg_root, ULONG session_id);

    void BoxClosed(const WCHAR* reg_root, ULONG session_id);

protected:
    friend VOID CALLBACK WaitOrTimerCallback(PVOID lpParameter, BOOLEAN TimerOrWaitFired);

    BoxManager();

#if 0
    void InitLinger(BOX_INSTANCE* pBox);
    void CheckLinger(BOX_INSTANCE* pBox, ULONG SessionId);
#endif

    CRITICAL_SECTION m_CritSec;

    std::map<std::wstring, BOX_INSTANCE*> m_BoxMap;
};


#endif /* _MY_BOXMANAGER_H */
