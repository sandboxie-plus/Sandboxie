/*
 * Copyright 2021-2023 David Xanatos, xanasoft.com
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


#ifndef _MY_HANDLE_H
#define _MY_HANDLE_H


//---------------------------------------------------------------------------
// Structures and Types
//---------------------------------------------------------------------------


typedef void(*P_HandlerFunc)(HANDLE handle, void* param);


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------


VOID Handle_SetDeleteOnClose(HANDLE FileHandle, BOOLEAN DeleteOnClose);

BOOLEAN Handle_RegisterHandler(HANDLE FileHandle, P_HandlerFunc CloseHandler, void* Params, BOOL bPropagate);

VOID Handle_UnRegisterHandler(HANDLE FileHandle, P_HandlerFunc CloseHandler, void** pParams);

VOID Handle_SetRelocationPath(HANDLE FileHandle, WCHAR* RelocationPath);

WCHAR* Handle_GetRelocationPath(HANDLE FileHandle, ULONG ExtraLength);

VOID Handle_ExecuteCloseHandler(HANDLE FileHandle, BOOLEAN* DeleteOnClose);


//---------------------------------------------------------------------------


#endif /* _MY_HANDLE_H */
