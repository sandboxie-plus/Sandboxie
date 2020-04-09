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

#include <ntstatus.h>
#define WIN32_NO_STATUS
typedef long NTSTATUS;

#include "global.h"
#include "core/dll/sbiedll.h"
#include "common/win32_ntddk.h"


//---------------------------------------------------------------------------
// DoListSections
//---------------------------------------------------------------------------


void DoListSections(void)
{
    ULONG index = -1;

    BOOLEAN boxes = FALSE;
    if (CmdOpt(L"boxes"))
        boxes = TRUE;

    if (! boxes)
        printf("GlobalSettings\n");

    while (1) {

        WCHAR section_name[34];
        index = SbieApi_EnumBoxesEx(
                    index | CONF_GET_NO_TEMPLS, section_name, TRUE);
        if (index == -1)
            break;

        if (boxes) {
            ULONG rc = SbieApi_IsBoxEnabled(section_name);
            if (rc != 0)
                continue;
        }

        printf("%S\n", section_name);
    }
}


//---------------------------------------------------------------------------
// DoListSettings
//---------------------------------------------------------------------------


void DoListSettings(void)
{
    ULONG setting_idx = 0;
    WCHAR setting_name[66];

    while (1) {

        ULONG status = SbieApi_QueryConfAsIs(
            CmdVerb(1), NULL, setting_idx | CONF_GET_NO_TEMPLS,
            setting_name, sizeof(WCHAR) * 66);

        if (! NT_SUCCESS(status))
            break;

        ++setting_idx;

        printf("%S\n", setting_name);
    }
}


//---------------------------------------------------------------------------
// DoQuerySetting
//---------------------------------------------------------------------------


void DoQuerySetting(void)
{
    WCHAR value[512];

    ULONG index = CONF_GET_NO_TEMPLS;
    BOOL expand = CmdIs(L"queryex");
    if ((! expand) && CmdOpt(L"expand"))
        expand = TRUE;
    if (! expand)
        index |= CONF_GET_NO_EXPAND;

    while (1) {

        ULONG status = SbieApi_QueryConf(
            CmdVerb(1), CmdVerb(2), index, value, sizeof(WCHAR) * 510);

        if (status != 0)
            break;

        if (expand)
            SbieDll_TranslateNtToDosPath(value);

        printf("%S\n", value);

        ++index;
    }
}


//---------------------------------------------------------------------------
// DoQuery
//---------------------------------------------------------------------------


int DoQuery(void)
{
    if ((! CmdVerb(1)) || (CmdVerb(2) && CmdVerb(3))) {

        const WCHAR *_usage =
            L"query[ex] [/expand] [/boxes] <section> [setting]\n"
            L"- specify * for section to get a list of sections"
                L" in the configuration\n"
            L"- specify /boxes to get a list of sections which"
                L" correspond to sandboxes\n"
            L"  that are enabled and active for this user account\n"
            L"- specify * for setting to get a list of settings"
                L" in a specific section\n"
            L"- specify setting to get the value of a setting in"
                L" a section\n"
            L"- specify /expand to expand variables in the value"
                L" of the setting\n"
            L"- queryex command is same as query /expand\n";

        UsageError(_usage);
    }

    if (wcscmp(CmdVerb(1), L"*") == 0) {

        DoListSections();

    } else if ((! CmdVerb(2)) || (wcscmp(CmdVerb(2), L"*") == 0)) {

        DoListSettings();

    } else if (CmdVerb(2)) {

        DoQuerySetting();
    }

    return 0;
}
