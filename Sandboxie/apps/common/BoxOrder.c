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
// Box Order Utility
//---------------------------------------------------------------------------


#include <windows.h>
#include "BoxOrder.h"
#include "core/dll/sbiedll.h"
#include "common/defines.h"
#include "core/svc/SbieIniWire.h"


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------


static WCHAR *BoxOrder_ReadSetting(const WCHAR *section_name);

static BOX_ORDER_ENTRY *BoxOrder_ReadGroup2(WCHAR *name, WCHAR **ptext);

static int BoxOrder_ReadGroup(BOX_ORDER_ENTRY *parent, WCHAR *text);

static int BoxOrder_ReadFindBox(BOX_ORDER_ENTRY *entry, const WCHAR *name);

static void BoxOrder_ReadMissing(BOX_ORDER_ENTRY *parent);

static BOX_ORDER_ENTRY *BoxOrder_ReadEx(int ReadUser, int ReadDefault);

static int BoxOrder_ToString2(BOX_ORDER_ENTRY *entry, WCHAR *text);

static WCHAR *BoxOrder_ToString(BOX_ORDER_ENTRY *root);

static int BoxOrder_WriteSetting(WCHAR *text);


//---------------------------------------------------------------------------
// Variables
//---------------------------------------------------------------------------


static const WCHAR *BoxOrder_SettingName = L"BoxDisplayOrder";


//---------------------------------------------------------------------------
// BoxOrder_Alloc
//---------------------------------------------------------------------------


_FX BOX_ORDER_ENTRY *BoxOrder_Alloc(const WCHAR *name)
{
    ULONG name_len = (wcslen(name) + 1) * sizeof(WCHAR);
    ULONG entry_len = sizeof(BOX_ORDER_ENTRY) + name_len;
    BOX_ORDER_ENTRY *entry = HeapAlloc(GetProcessHeap(), 0, entry_len);
    if (entry) {
        memzero(entry, entry_len);
        memcpy(entry->name, name, name_len);
        if (name_len > 128 * sizeof(WCHAR))
            entry->name[128] = L'\0';
        while (1) {
            WCHAR *ptr = wcschr(entry->name, L',');
            if (! ptr)
                ptr = wcschr(entry->name, L'(');
            if (! ptr)
                ptr = wcschr(entry->name, L')');
            if (! ptr)
                break;
            *ptr = L'_';
        }
    }
    return entry;
}


//---------------------------------------------------------------------------
// BoxOrder_Free
//---------------------------------------------------------------------------


_FX void BoxOrder_Free(BOX_ORDER_ENTRY *entry)
{
    while (entry) {
        BOX_ORDER_ENTRY *next = entry->next;
        if (entry->children)
            BoxOrder_Free(entry->children);
        HeapFree(GetProcessHeap(), 0, entry);
        entry = next;
    }
}


//---------------------------------------------------------------------------
// BoxOrder_Append
//---------------------------------------------------------------------------


_FX void BoxOrder_Append(BOX_ORDER_ENTRY *parent, BOX_ORDER_ENTRY *child)
{
    if (! parent->children)
        parent->children = child;
    else {
        BOX_ORDER_ENTRY *last = parent->children;
        while (last->next)
            last = last->next;
        last->next = child;
    }
}


//---------------------------------------------------------------------------
// BoxOrder_ReadSetting
//---------------------------------------------------------------------------


_FX WCHAR *BoxOrder_ReadSetting(const WCHAR *section_name)
{
    WCHAR *result = NULL;

    if (! section_name) {
        static SBIE_INI_GET_USER_RPL *rpl = NULL;
        if (! rpl) {
            SBIE_INI_GET_USER_REQ req;
            req.h.msgid = MSGID_SBIE_INI_GET_USER;
            req.h.length = sizeof(SBIE_INI_GET_USER_REQ);
            rpl = (SBIE_INI_GET_USER_RPL *)SbieDll_CallServer(&req.h);
            if (rpl && rpl->h.status != 0) {
                SbieDll_FreeMem(rpl);
                rpl = NULL;
            }
        }
        if (rpl && rpl->h.status == 0)
            section_name = rpl->section;
    }

    if (section_name) {

        ULONG result_len;
        ULONG setting_idx = 0;
        ULONG status;

        result_len = 8192;
        result = HeapAlloc(GetProcessHeap(), 0, result_len * sizeof(WCHAR));
        if (result) {

            WCHAR *result_ptr = result;
            while (1) {

                status = SbieApi_QueryConfAsIs(
                    section_name, BoxOrder_SettingName,
                    setting_idx | CONF_GET_NO_GLOBAL | CONF_GET_NO_TEMPLS,
                    result_ptr, (result_len - 8) * sizeof(WCHAR));

                if (status != 0)
                    break;

                ++setting_idx;
                result_len -= wcslen(result_ptr);
                result_ptr += wcslen(result_ptr);
                *result_ptr = L',';
                ++result_ptr;

                if (result_len <= 8)
                    break;
            }

            if (result_ptr != result)
                memzero(result_ptr, 4 * sizeof(WCHAR));
            else {
                HeapFree(GetProcessHeap(), 0, result);
                result = NULL;
            }
        }
    }

    return result;
}


//---------------------------------------------------------------------------
// BoxOrder_ReadGroup2
//---------------------------------------------------------------------------


_FX BOX_ORDER_ENTRY *BoxOrder_ReadGroup2(WCHAR *name, WCHAR **ptext)
{
    BOX_ORDER_ENTRY *group;
    WCHAR *ptr, *text;
    int count;

    text = *ptext;
    *text = L'\0';
    if (! *name)
        name = L"?";
    group = BoxOrder_Alloc(name);
    *text = L'(';

    ptr = text + 1;
    count = 1;
    while (1) {
        if (*ptr == L'(')
            ++count;
        else if (*ptr == L')') {
            --count;
            if (! count) {
                *ptr = L'\0';
                break;
            }
        } else if (! *ptr)
            break;
        ++ptr;
    }
    *ptext = ptr;

    count = BoxOrder_ReadGroup(group, text + 1);
    if (! count) {
        BoxOrder_Free(group);
        group = NULL;
    }

    return group;
}


//---------------------------------------------------------------------------
// BoxOrder_ReadGroup
//---------------------------------------------------------------------------


_FX int BoxOrder_ReadGroup(BOX_ORDER_ENTRY *parent, WCHAR *text)
{
    WCHAR *sep, *sep2;
    BOX_ORDER_ENTRY *new_entry;
    int count = 0;

    while (1) {

        sep = wcschr(text, L',');
        if (sep)
            *sep = L'\0';

        if (*text) {

            new_entry = NULL;

            sep2 = wcschr(text, L'(');
            if (sep2) {
                if (sep)
                    *sep = L',';
                sep = sep2;
                new_entry = BoxOrder_ReadGroup2(text, &sep);

            } else if (SbieApi_IsBoxEnabled(text) == 0)
                new_entry = BoxOrder_Alloc(text);

            else
                new_entry = NULL;

            if (new_entry) {
                BoxOrder_Append(parent, new_entry);
                ++count;
            }

            if ((! sep2) && sep)
                *sep = L',';
        }

        if (! sep)
            break;
        text = sep + 1;
        while (*sep == L',')
            ++sep;
    }

    return count;
}


//---------------------------------------------------------------------------
// BoxOrder_ReadFindBox
//---------------------------------------------------------------------------


_FX int BoxOrder_ReadFindBox(BOX_ORDER_ENTRY *entry, const WCHAR *name)
{
    while (entry) {
        if (entry->children) {
            if (BoxOrder_ReadFindBox(entry->children, name))
                return 1;
        } else if (_wcsicmp(entry->name, name) == 0)
            return 1;
        entry = entry->next;
    }
    return 0;
}


//---------------------------------------------------------------------------
// BoxOrder_ReadMissing
//---------------------------------------------------------------------------


_FX void BoxOrder_ReadMissing(BOX_ORDER_ENTRY *parent)
{
    WCHAR work_name[BOXNAME_COUNT], *all_names, *name;
    int box_count, box_index, name_index, small_index, alloc_len;

    box_count = 0;
    box_index = -1;
    while (1) {
        box_index = SbieApi_EnumBoxesEx(
                        box_index, work_name, TRUE);
        if (box_index == -1)
            break;
        if (SbieApi_IsBoxEnabled(work_name) == 0)
            ++box_count;
    }
    if (! box_count)
        return;

    alloc_len = box_count * (BOXNAME_COUNT * sizeof(WCHAR));
    all_names = HeapAlloc(GetProcessHeap(), 0, alloc_len);

    name_index = 0;
    box_index = -1;
    while (name_index < box_count) {
        name = &all_names[name_index * BOXNAME_COUNT];
        box_index = SbieApi_EnumBoxesEx(
                        box_index, name, TRUE);
        if (box_index == -1)
            break;
        if (SbieApi_IsBoxEnabled(name) == 0)
            ++name_index;
    }

    while (1) {
        small_index = -1;
        for (name_index = 0; name_index < box_count; ++name_index) {
            name = &all_names[name_index * BOXNAME_COUNT];
            if (! *name)
                continue;
            if (small_index == -1)
                small_index = name_index;
            else {
                const WCHAR *small_name = &all_names[small_index * BOXNAME_COUNT];
                if (_wcsicmp(small_name, name) > 0)
                    small_index = name_index;
            }
        }

        if (small_index == -1)
            break;
        name = &all_names[small_index * BOXNAME_COUNT];
        if (! BoxOrder_ReadFindBox(parent, name)) {

            BOX_ORDER_ENTRY *new_entry = BoxOrder_Alloc(name);
            if (new_entry)
                BoxOrder_Append(parent, new_entry);
        }
        name[0] = L'\0';
    }

    HeapFree(GetProcessHeap(), 0, all_names);
}


//---------------------------------------------------------------------------
// BoxOrder_ReadEx
//---------------------------------------------------------------------------


_FX BOX_ORDER_ENTRY *BoxOrder_ReadEx(int ReadUser, int ReadDefault)
{
    WCHAR *text;
    BOX_ORDER_ENTRY *root;

    if (ReadUser)
        text = BoxOrder_ReadSetting(NULL);
    else
        text = NULL;
    if ((! text) && ReadDefault)
        text = BoxOrder_ReadSetting(L"UserSettings_Default");

    root = BoxOrder_Alloc(L"");
    if (root) {

        if (text)
            BoxOrder_ReadGroup(root, text);

        BoxOrder_ReadMissing(root);
    }

    if (text)
        HeapFree(GetProcessHeap(), 0, text);

    return root;
}


//---------------------------------------------------------------------------
// BoxOrder_Read
//---------------------------------------------------------------------------


_FX BOX_ORDER_ENTRY *BoxOrder_Read(void)
{
    return BoxOrder_ReadEx(TRUE, TRUE);
}


//---------------------------------------------------------------------------
// BoxOrder_ReadDefault
//---------------------------------------------------------------------------


_FX BOX_ORDER_ENTRY *BoxOrder_ReadDefault(void)
{
    return BoxOrder_ReadEx(FALSE, TRUE);
}


//---------------------------------------------------------------------------
// BoxOrder_ToString2
//---------------------------------------------------------------------------


_FX int BoxOrder_ToString2(BOX_ORDER_ENTRY *entry, WCHAR *text)
{
    int count = wcslen(entry->name);
    if (text) {
        wcscpy(text, entry->name);
        text += count;
    }

    if (entry->children) {

        BOX_ORDER_ENTRY *child = entry->children;

        if (*entry->name) {
            ++count;
            if (text) {
                *text = L'(';
                ++text;
            }
        }

        while (child) {

            int count2 = BoxOrder_ToString2(child, text);
            count += count2;
            if (text)
                text += count2;

            if (child->next) {
                ++count;
                if (text) {
                    *text = L',';
                    ++text;
                }
            }

            child = child->next;
        }

        if (*entry->name) {
            ++count;
            if (text) {
                *text = L')';
                ++text;
            }
        }
    }

    return count;
}


//---------------------------------------------------------------------------
// BoxOrder_ToString
//---------------------------------------------------------------------------


_FX WCHAR *BoxOrder_ToString(BOX_ORDER_ENTRY *root)
{
    WCHAR *text = NULL;
    int count = BoxOrder_ToString2(root, NULL);
    if (count) {
        int count2 = (count + 1) * sizeof(WCHAR);
        text = HeapAlloc(GetProcessHeap(), 0, count2);
        if (text) {
            count2 = BoxOrder_ToString2(root, text);
            if (count2 > count)
                count2 = count;
            text[count2] = L'\0';
        }
    }
    return text;
}


//---------------------------------------------------------------------------
// BoxOrder_WriteSetting
//---------------------------------------------------------------------------


_FX int BoxOrder_WriteSetting(WCHAR *text)
{
    ULONG status;
    WCHAR *ptr;
    WCHAR opcode = L's';

    for (ptr = text; ptr; text = ptr) {

        if (wcslen(text) > 1000) {

            ptr = text + 1000;
            while (*ptr && *ptr != L',')
                ++ptr;
            if (*ptr == L',') {
                *ptr = L'\0';
                ++ptr;
            } else
                ptr = NULL;
        } else
            ptr = NULL;

        status = SbieDll_UpdateConf(
            opcode, NULL, L"UserSettings_?", BoxOrder_SettingName, text);
        if (status != 0)
            return 0;
        opcode = L'a';
    }

    return 1;
}


//---------------------------------------------------------------------------
// BoxOrder_Write
//---------------------------------------------------------------------------


int BoxOrder_Write(BOX_ORDER_ENTRY *root)
{
    WCHAR *text_old, *text_new;
    int return_code = -1;

    BOX_ORDER_ENTRY *root_old = BoxOrder_ReadDefault();
    if (root_old) {
        text_old = BoxOrder_ToString(root_old);
        BoxOrder_Free(root_old);
    } else
        text_old = NULL;

    text_new = BoxOrder_ToString(root);

    if (text_old && text_new) {

        int success_code = 1;

        if (wcscmp(text_old, text_new) == 0) {
            success_code = 0;
            *text_new = L'\0';
        }

        if (BoxOrder_WriteSetting(text_new))
            return_code = success_code;
    }

    if (text_new)
        HeapFree(GetProcessHeap(), 0, text_new);
    if (text_old)
        HeapFree(GetProcessHeap(), 0, text_old);

    return return_code;
}
