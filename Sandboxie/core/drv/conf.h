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
// Configuration
//---------------------------------------------------------------------------


#ifndef _MY_CONF_H
#define _MY_CONF_H


#include "driver.h"


//---------------------------------------------------------------------------
// Defines
//---------------------------------------------------------------------------


#define CONF_GET_NO_GLOBAL          0x40000000L
#define CONF_GET_NO_EXPAND          0x20000000L
#define CONF_GET_NO_TEMPLS          0x10000000L


//---------------------------------------------------------------------------
// Structures and Types
//---------------------------------------------------------------------------


struct _CONF_EXPAND_ARGS {

    POOL *pool;
    WCHAR *sandbox;
    WCHAR *sid;
    ULONG *session;

};


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------


BOOLEAN Conf_Init(void);

void Conf_Unload(void);


// Conf_AdjustUseCount:  use before and after a sequence of calls
// to Conf_Get, to make sure the strings returned do not evaporate
// if Conf_Api_Reload also happens to be called at the same time

void Conf_AdjustUseCount(BOOLEAN increase);


// Conf_Get:  returns a pointer to string configuration data.  use
// with Conf_AdjustUseCount to make sure the returned pointer is valid

const WCHAR *Conf_Get(
    const WCHAR *section, const WCHAR *setting, ULONG index);


// Conf_Get_Boolean:  parses a y/n setting.  this function does not
// have to be protected with Conf_AdjustUseCount

BOOLEAN Conf_Get_Boolean(
    const WCHAR *section, const WCHAR *setting, ULONG index, BOOLEAN def);


// Conf_Get_Boolean:  parses a numeric setting.  this function does
// not have to be protected with Conf_AdjustUseCount

ULONG Conf_Get_Number(
    const WCHAR *section, const WCHAR *setting, ULONG index, ULONG def);


// Conf_IsValidBox:  returns STATUS_SUCCESS for a valid and defined
// box section.  returns STATUS_OBJECT_TYPE_MISMATCH if the section
// does not define a box.  returns STATUS_OBJECT_NAME_NOT_FOUD if
// the section does not exist.

NTSTATUS Conf_IsValidBox(const WCHAR *section_name);


// Conf_Expand:  expands %-variables in a string which was retrieved
// using Conf_Get or by any other means

WCHAR *Conf_Expand(
    CONF_EXPAND_ARGS *args, const WCHAR *model_value,
    const WCHAR *setting_name);


//---------------------------------------------------------------------------
// Private Functions
//---------------------------------------------------------------------------


BOOLEAN Conf_Init_User(void);

void Conf_Unload_User(void);

BOOLEAN Conf_Expand_UserName(CONF_EXPAND_ARGS *args, WCHAR *varvalue);

BOOLEAN Conf_IsBoxEnabled(
    const WCHAR *BoxName, const WCHAR *SidString, ULONG SessionId);


//---------------------------------------------------------------------------


NTSTATUS Conf_Api_Reload(PROCESS *proc, ULONG64 *parms);

NTSTATUS Conf_Api_Query(PROCESS *proc, ULONG64 *parms);


//---------------------------------------------------------------------------


#endif // _MY_CONF_H
