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
// Sandboxie Ini Server -- using PipeServer
//---------------------------------------------------------------------------


#ifndef _MY_SBIEINIWIRE_H
#define _MY_SBIEINIWIRE_H


#include "msgids.h"

#define CONF_LINE_LEN               2000    // keep in sync with drv/conf.c
#define CONF_MAX_LINES              100000   // keep in sync with drv/conf.c

//---------------------------------------------------------------------------
// Get Version
//---------------------------------------------------------------------------


struct tagSBIE_INI_GET_VERSION_REQ
{
    MSG_HEADER h;
};

struct tagSBIE_INI_GET_VERSION_RPL
{
    MSG_HEADER h;       // status is STATUS_SUCCESS or STATUS_UNSUCCESSFUL
    ULONG abi_ver;
    WCHAR version[1];
};

typedef struct tagSBIE_INI_GET_VERSION_REQ SBIE_INI_GET_VERSION_REQ;
typedef struct tagSBIE_INI_GET_VERSION_RPL SBIE_INI_GET_VERSION_RPL;


//---------------------------------------------------------------------------
// Get Wait Handle
//---------------------------------------------------------------------------


struct tagSBIE_INI_GET_WAIT_HANDLE_REQ
{
    MSG_HEADER h;
};

struct tagSBIE_INI_GET_WAIT_HANDLE_RPL
{
    MSG_HEADER h;                       // status is win32 error code
    HANDLE hProcess;
};

typedef struct tagSBIE_INI_GET_WAIT_HANDLE_REQ SBIE_INI_GET_WAIT_HANDLE_REQ;
typedef struct tagSBIE_INI_GET_WAIT_HANDLE_RPL SBIE_INI_GET_WAIT_HANDLE_RPL;


//---------------------------------------------------------------------------
// Get Path
//---------------------------------------------------------------------------


struct tagSBIE_INI_GET_PATH_REQ
{
    MSG_HEADER h;
};

struct tagSBIE_INI_GET_PATH_RPL
{
    MSG_HEADER h;       // status is STATUS_SUCCESS or STATUS_UNSUCCESSFUL
    BOOLEAN is_home_path;
    WCHAR path[1];
};

typedef struct tagSBIE_INI_GET_PATH_REQ SBIE_INI_GET_PATH_REQ;
typedef struct tagSBIE_INI_GET_PATH_RPL SBIE_INI_GET_PATH_RPL;


//---------------------------------------------------------------------------
// Get User
//---------------------------------------------------------------------------


struct tagSBIE_INI_GET_USER_REQ
{
    MSG_HEADER h;
};

struct tagSBIE_INI_GET_USER_RPL
{
    MSG_HEADER h;       // status is STATUS_SUCCESS or STATUS_UNSUCCESSFUL
    BOOLEAN admin;
    WCHAR section[BOXNAME_COUNT];
    ULONG name_len;
    WCHAR name[1];
};

typedef struct tagSBIE_INI_GET_USER_REQ SBIE_INI_GET_USER_REQ;
typedef struct tagSBIE_INI_GET_USER_RPL SBIE_INI_GET_USER_RPL;


//---------------------------------------------------------------------------
// Set/Add/Delete Setting
//---------------------------------------------------------------------------


struct tagSBIE_INI_SETTING_REQ
{
    MSG_HEADER h;
    WCHAR password[66];
    BOOLEAN refresh;
    WCHAR section[66];
    WCHAR setting[66];
    ULONG value_len;
    WCHAR value[1];
};

struct tagSBIE_INI_SETTING_RPL
{
    MSG_HEADER h;       // status is STATUS_SUCCESS or STATUS_UNSUCCESSFUL
    ULONG value_len;
    WCHAR value[1];
};

typedef struct tagSBIE_INI_SETTING_REQ SBIE_INI_SETTING_REQ;
typedef struct tagSBIE_INI_SETTING_RPL SBIE_INI_SETTING_RPL;


//---------------------------------------------------------------------------
// Set Template Setting
//---------------------------------------------------------------------------


struct tagSBIE_INI_TEMPLATE_REQ
{
    MSG_HEADER h;
    WCHAR password[66];
    WCHAR varname[66];
    BOOLEAN user;
    ULONG value_len;
    WCHAR value[1];
};

typedef struct tagSBIE_INI_TEMPLATE_REQ SBIE_INI_TEMPLATE_REQ;


//---------------------------------------------------------------------------
// Set or Test Password
//---------------------------------------------------------------------------


struct tagSBIE_INI_PASSWORD_REQ
{
    MSG_HEADER h;
    WCHAR old_password[66];
    WCHAR new_password[66];
};

typedef struct tagSBIE_INI_PASSWORD_REQ SBIE_INI_PASSWORD_REQ;


//---------------------------------------------------------------------------


#endif /* _MY_SBIEINIWIRE_H */
