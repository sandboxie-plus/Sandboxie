/*
 * Copyright 2021-2022 David Xanatos, xanasoft.com
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
// Defines
//---------------------------------------------------------------------------

#define MAX_CLOSE_HANDLERS 4


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------


typedef void(*P_CloseHandler)(HANDLE handle);

VOID Handle_SetDeleteOnClose(HANDLE FileHandle, BOOLEAN DeleteOnClose);

BOOLEAN Handle_RegisterCloseHandler(HANDLE FileHandle, P_CloseHandler CloseHandler);

BOOLEAN Handle_UnRegisterCloseHandler(HANDLE FileHandle, P_CloseHandler CloseHandler);

VOID Handle_SetRelocationPath(HANDLE FileHandle, WCHAR* RelocationPath);

WCHAR* Handle_GetRelocationPath(HANDLE FileHandle, ULONG ExtraLength);

BOOLEAN Handle_FreeCloseHandler(HANDLE FileHandle, P_CloseHandler* CloseHandlers, BOOLEAN* DeleteOnClose);


//---------------------------------------------------------------------------


#endif /* _MY_HANDLE_H */
