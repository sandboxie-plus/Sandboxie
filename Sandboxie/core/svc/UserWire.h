/*
 * Copyright 2022-2023 David Xanatos, xanasoft.com
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
// USER Proxy Server
//---------------------------------------------------------------------------


#ifndef _MY_USERWIRE_H
#define _MY_USERWIRE_H


//---------------------------------------------------------------------------
// Defines
//---------------------------------------------------------------------------


enum {

    USER_OPEN_FILE = 1,
    USER_SHELL_EXEC,
    USER_MAX_REQUEST_CODE
};


//---------------------------------------------------------------------------
// Open File
//---------------------------------------------------------------------------


struct tagUSER_OPEN_FILE_REQ
{
    ULONG msgid;
    
    ACCESS_MASK DesiredAccess;
    ULONG FileNameOffset;
    //ULONG FileNameSize;
    ULONG64 AllocationSize;
    ULONG FileAttributes;
    ULONG ShareAccess;
    ULONG CreateDisposition;
    ULONG CreateOptions;
    ULONG EaBufferOffset;
    ULONG EaLength;
};

struct tagUSER_OPEN_FILE_RPL
{
    ULONG status;
    ULONG error;
    ULONG64 FileHandle;
    NTSTATUS Status;
    ULONG64 Information;
};

typedef struct tagUSER_OPEN_FILE_REQ USER_OPEN_FILE_REQ;
typedef struct tagUSER_OPEN_FILE_RPL USER_OPEN_FILE_RPL;


//---------------------------------------------------------------------------
// Shell Execute
//---------------------------------------------------------------------------


struct tagUSER_SHELL_EXEC_REQ
{
    ULONG msgid;
    
    ULONG FileNameOffset;
};


//struct tagUSER_SHELL_EXEC_RPL
//{
//    ULONG status;
//    ULONG error;
//};

typedef struct tagUSER_SHELL_EXEC_REQ USER_SHELL_EXEC_REQ;
//typedef struct tagUSER_SHELL_EXEC_RPL USER_SHELL_EXEC_RPL;

//---------------------------------------------------------------------------


#endif /* _MY_USERWIRE_H */
