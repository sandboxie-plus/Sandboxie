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
// SbieIni scripting utility
//---------------------------------------------------------------------------


#include "global.h"
#include "common/my_version.h"


//---------------------------------------------------------------------------
// NewLine
//---------------------------------------------------------------------------


/*BOOL NewLine(void)
{
    static BOOL _checked = FALSE;
    static BOOL _result;

    if (! _checked) {

        if (GetFileType(GetStdHandle(STD_OUTPUT_HANDLE)) == FILE_TYPE_CHAR)
            _result = TRUE;
        else
            _result = FALSE;
        if (CmdOpt(L"newline")) {
            WCHAR ch = towlower(*CmdOpt(L"newline"));
            if (ch == L'y')
                _result = TRUE;
            else if (ch == L'n')
                _result= FALSE;
        }
    }

    return _result;
}*/


//---------------------------------------------------------------------------
// UsageError
//---------------------------------------------------------------------------


void UsageError(const WCHAR *text)
{
    printf("Usage:  %S %S\n", SBIEINI, text);
    ExitProcess(ERRLVL_CMDLINE);
}


//---------------------------------------------------------------------------
// main
//---------------------------------------------------------------------------


int __cdecl wmain(void)
{
    CmdParse();
    if (CmdIs(L"query") || CmdIs(L"queryex"))
        return DoQuery();
    if (CmdIs(L"set") || CmdIs(L"append")
            || CmdIs(L"insert") || CmdIs(L"delete"))
        return DoUpdate();
    else {
        UsageError(L"<query|queryex|set|append|insert|delete>");
        return ERRLVL_CMDLINE;  // not reached
    }
}
