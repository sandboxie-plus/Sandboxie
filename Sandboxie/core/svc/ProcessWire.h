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
// Process Server -- using PipeServer
//---------------------------------------------------------------------------


#ifndef _MY_PROCESSWIRE_H
#define _MY_PROCESSWIRE_H


#include "msgids.h"


//---------------------------------------------------------------------------
// Kill One Process
//---------------------------------------------------------------------------


struct tagPROCESS_KILL_ONE_REQ
{
    MSG_HEADER h;
    ULONG pid;
};

typedef struct tagPROCESS_KILL_ONE_REQ PROCESS_KILL_ONE_REQ;


//---------------------------------------------------------------------------
// Kill All Process
//---------------------------------------------------------------------------


struct tagPROCESS_KILL_ALL_REQ
{
    MSG_HEADER h;
    ULONG session_id;
    WCHAR boxname[34];
};

typedef struct tagPROCESS_KILL_ALL_REQ PROCESS_KILL_ALL_REQ;


//---------------------------------------------------------------------------
// Set Device Map
//---------------------------------------------------------------------------


struct tagPROCESS_SET_DEVICE_MAP_REQ
{
    MSG_HEADER h;
    ULONG64 DirectoryHandle;
};

typedef struct tagPROCESS_SET_DEVICE_MAP_REQ PROCESS_SET_DEVICE_MAP_REQ;


//---------------------------------------------------------------------------
// Open Device Map
//---------------------------------------------------------------------------


struct tagPROCESS_OPEN_DEVICE_MAP_REQ
{
    MSG_HEADER h;
    ULONG64 DirectoryHandlePtr;
    WCHAR DirectoryName[96];
};

typedef struct tagPROCESS_OPEN_DEVICE_MAP_REQ PROCESS_OPEN_DEVICE_MAP_REQ;


//---------------------------------------------------------------------------
// Run Sandboxed Process
//---------------------------------------------------------------------------


struct tagPROCESS_RUN_SANDBOXED_REQ
{
    MSG_HEADER h;
    WCHAR boxname[34];
    ULONG cmd_ofs;
    ULONG cmd_len;
    ULONG dir_ofs;
    ULONG dir_len;
    ULONG env_ofs;
    ULONG env_len;
    ULONG si_flags;
    ULONG si_show_window;
    ULONG creation_flags;
};

struct tagPROCESS_RUN_SANDBOXED_RPL
{
    MSG_HEADER h;                       // status is win32 error
    ULONG64 hProcess;
    ULONG64 hThread;
    ULONG dwProcessId;
    ULONG dwThreadId;
};

typedef struct tagPROCESS_RUN_SANDBOXED_REQ PROCESS_RUN_SANDBOXED_REQ;
typedef struct tagPROCESS_RUN_SANDBOXED_RPL PROCESS_RUN_SANDBOXED_RPL;


//---------------------------------------------------------------------------


#endif /* _MY_PROCESSWIRE_H */
