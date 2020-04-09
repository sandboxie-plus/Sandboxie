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
#include "core/dll/sbiedll.h"


//---------------------------------------------------------------------------
// DoUpdate
//---------------------------------------------------------------------------


int DoUpdate(void)
{
    ULONG status;
    WCHAR op;

    BOOL usage_error = FALSE;
    if ((! CmdVerb(1)) || (! CmdVerb(2)))
        usage_error = TRUE;
    else if ((! CmdVerb(3)) && (! CmdIs(L"set")))
        usage_error = TRUE;
    else if (CmdVerb(3) && CmdVerb(4))
        usage_error = TRUE;

    if (usage_error) {

        const WCHAR *_usage =
            L"set|append|insert|delete <section> <setting> <value>\n"
            L"- set:     replaces a setting with a new value"
                L"  if value is omitted,\n"
            L"           the setting will be removed entirely\n"
            L"- append:  adds a new value line for the setting"
                L" at the end of the\n"
            L"           existing list of lines\n"
            L"- insert:  adds a new value line for the setting"
                L" at the top of the\n"
            L"           existing list of lines\n"
            L"- delete:  removes a value line which matches"
                L" the specified value\n";

        UsageError(_usage);
    }

    if (CmdIs(L"set"))
        op = L's';
    else if (CmdIs(L"append"))
        op = L'a';
    else if (CmdIs(L"insert"))
        op = L'i';
    else if (CmdIs(L"delete"))
        op = L'd';
    else
        return ERRLVL_CMDLINE;

    status = SbieDll_UpdateConf(
                    op, NULL, CmdVerb(1), CmdVerb(2), CmdVerb(3));

    return 0;
}
