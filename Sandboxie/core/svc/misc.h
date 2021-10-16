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

#ifndef MISC_H_WITHOUT_WIN32_NTDDK_H
#include "common/win32_ntddk.h"
#endif
#include "msgs/msgs.h"

void LogEvent(ULONG msgid, ULONG level, ULONG detail);
void AbortServer(void);
bool RestrictToken(void);
bool CheckDropRights(const WCHAR *BoxName);

bool IsProcessWoW64(HANDLE pid);

extern HMODULE _Ntdll;
extern HMODULE _Kernel32;

extern SYSTEM_INFO _SystemInfo;
#define NUMBER_OF_PROCESSORS (_SystemInfo.dwNumberOfProcessors)
#ifdef _WIN64
#define MINIMUM_NUMBER_OF_THREADS 8
#else
#define MINIMUM_NUMBER_OF_THREADS 4
#endif
#define NUMBER_OF_THREADS   \
            ((NUMBER_OF_PROCESSORS * 2) <= MINIMUM_NUMBER_OF_THREADS \
                    ? MINIMUM_NUMBER_OF_THREADS : (NUMBER_OF_PROCESSORS * 2))
