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
// Message Box Utility
//---------------------------------------------------------------------------


#include <windows.h>


#ifdef __cplusplus
extern "C" {
#endif


//
// MessageCheckBox
//
// The check parameter specifies the text for the additional checkbox.
//

int MessageCheckBox(
    HWND hwnd, const WCHAR *text, const WCHAR *caption, UINT flags,
    const WCHAR *check);

//
// InputBox
//
// For a password input box, pass INPUTBOX_PASSWORD in the flags parameter.
//

WCHAR *InputBox(HWND hwnd, const WCHAR *caption, UINT flags,
                const WCHAR *def);

#define INPUTBOX_PASSWORD MB_SERVICE_NOTIFICATION
#define INPUTBOX_READONLY MB_DEFAULT_DESKTOP_ONLY


#ifdef __cplusplus
}
#endif
