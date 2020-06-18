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
// Sandboxie Debug
//---------------------------------------------------------------------------


#ifndef _MY_DEBUG_H
#define _MY_DEBUG_H


//---------------------------------------------------------------------------


#undef  WITH_DEBUG
//#define WITH_DEBUG


//---------------------------------------------------------------------------


#ifdef  WITH_DEBUG

//#define  BREAK_IMAGE_1      L"java.exe" // L"jp2launcher.exe"
#define  BREAK_IMAGE_1      L"TestTarget.exe" 

int Debug_Init(void);

#endif  WITH_DEBUG


//---------------------------------------------------------------------------


#endif /* _MY_DEBUG_H */
