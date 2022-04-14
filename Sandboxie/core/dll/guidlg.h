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
// CreateDialogTemplate1
//---------------------------------------------------------------------------

#if GUI_DLG_VER == 1

#pragma pack(push,1)

typedef struct {
    DWORD style;
    DWORD dwExtendedStyle;
    WORD cDlgItems;
    short x;
    short y;
    short cx;
    short cy;
    WORD menu[1];
    WORD windowClass[1];
    WCHAR title[1];
    // following only if DS_SETFONT (n.b. DS_SHELLFONT includes DS_SETFONT)
    WORD pointsize;
    WCHAR typeface[1];
} DLGTMPL1;

typedef struct {
    DWORD style;
    DWORD exStyle;
    short x;
    short y;
    short cx;
    short cy;
    WORD id;
    WORD windowClass[1];
    WORD title[1];
    WORD extraCount;
} DLGTMPLITEM1;

#pragma pack(pop)

#define DLGTMPL                     DLGTMPL1
#define DLGTMPLITEM                 DLGTMPLITEM1
#define GUI_CreateDialogTemplate    GUI_CreateDialogTemplate1

#endif  // GUI_DLG_VER == 1

//---------------------------------------------------------------------------
// CreateDialogTemplate2
//---------------------------------------------------------------------------

#if GUI_DLG_VER == 2

#pragma pack(push,1)

typedef struct {
    WORD dlgVer;
    WORD signature;
    DWORD helpID;
    DWORD exStyle;
    DWORD style;
    WORD cDlgItems;
    short x;
    short y;
    short cx;
    short cy;
    WORD menu[1];
    WORD windowClass[1];
    WCHAR title[1];
    // following only if DS_SETFONT (n.b. DS_SHELLFONT includes DS_SETFONT)
    WORD pointsize;
    WORD weight;
    BYTE italic;
    BYTE charset;
    WCHAR typeface[1];
} DLGTMPL2;

typedef struct {
    DWORD helpID;
    DWORD exStyle;
    DWORD style;
    short x;
    short y;
    short cx;
    short cy;
    DWORD id;               // even though MSDN says this is WORD
    WORD windowClass[1];
    WORD title[1];
    WORD extraCount;
} DLGTMPLITEM2;

#pragma pack(pop)

#define DLGTMPL                     DLGTMPL2
#define DLGTMPLITEM                 DLGTMPLITEM2
#define GUI_CreateDialogTemplate    GUI_CreateDialogTemplate2

#endif  // GUI_DLG_VER == 2

//---------------------------------------------------------------------------
// CreateDialogTemplateN
//---------------------------------------------------------------------------

#ifdef GUI_CreateDialogTemplate

#pragma check_stack(off) 

void *GUI_CreateDialogTemplate(
    DLGTMPL *tmpl)
{
    DLGTMPL1 *newTmpl;
    UCHAR *ptr, *out;
    WCHAR *old_clsnm[256];
    WCHAR *new_clsnm[256];
    WCHAR *old_winnm, *new_winnm;
    int i;
    ULONG alloc_size, size = 0;

    // skip past the fixed-length header
    ptr = (UCHAR *)&tmpl->menu;
    if (*(WORD *)ptr == 0xFFFF)                             // menu
        ptr += sizeof(WORD) * 2;
    else
        ptr += (wcslen((WCHAR *)ptr) + 1) * sizeof(WCHAR);
    if (*(WORD *)ptr == 0xFFFF) {                           // windowClass
        ptr += sizeof(WORD) * 2;
        old_clsnm[0] = new_clsnm[0] = NULL;
    } else if (*(WORD *)ptr == 0x0000) {
        ptr += sizeof(WORD);
        old_clsnm[0] = new_clsnm[0] = NULL;
    } else {
        old_clsnm[0] = (WCHAR *)ptr;
        new_clsnm[0] = Gui_CreateClassNameW(old_clsnm[0]);
        ptr += (wcslen((WCHAR *)ptr) + 1) * sizeof(WCHAR);
        size += wcslen(new_clsnm[0]) * sizeof(WCHAR);
    }

    old_winnm = (WCHAR *)ptr;
    new_winnm = Gui_CreateTitleW(old_winnm);

    ptr += (wcslen((WCHAR *)ptr) + 1) * sizeof(WCHAR);      // title
    if ((tmpl->style & DS_SETFONT) != 0) {
        int ofs =
            (ULONG)((UCHAR *)&tmpl->typeface - (UCHAR *)&tmpl->pointsize);
        ptr += ofs;
        ptr += (wcslen((WCHAR *)ptr) + 1) * sizeof(WCHAR);  // typeface
    }

    for (i = 0; i < tmpl->cDlgItems; ++i) {
        // every dialog item begins on a DWORD boundtry
        ptr = (UCHAR *)((((SIZE_T)ptr) + 3) & (~3));
        // skip past the variable-length item
        ptr = (UCHAR *)&(((DLGTMPLITEM *)ptr)->windowClass);
        if (*(WORD *)ptr == 0xFFFF) {       // windowClass
            ptr += sizeof(WORD) * 2;
            old_clsnm[i + 1] = new_clsnm[i + 1] = NULL;
        } else if (*(WORD *)ptr == 0x0000) {
            ptr += sizeof(WORD);
            old_clsnm[i + 1] = new_clsnm[i + 1] = NULL;
        } else {
            old_clsnm[i + 1] = (WCHAR *)ptr;
            new_clsnm[i + 1] = Gui_CreateClassNameW(old_clsnm[i + 1]);
            ptr += (wcslen((WCHAR *)ptr) + 1) * sizeof(WCHAR);
            size += wcslen(new_clsnm[i + 1]) * sizeof(WCHAR);
        }
        if (*(WORD *)ptr == 0xFFFF)         // title
            ptr += sizeof(WORD) * 2;
        else
            ptr += (wcslen((WCHAR *)ptr) + 1) * sizeof(WCHAR);
        if (*(WORD *)ptr != 0)              // extraCount
            ptr += *(WORD *)ptr;
        ptr += sizeof(WORD);                // skip extraCount itself
    }

    // allocate and initialize the new DLGTEMPLATEEX, making a little
    // more room for the Sandbox-prefixed window class.  note that if
    // any extra space was needed for fixing class names, then those
    // sizes were already added to the size variable by now
    size += ptr - (UCHAR *)tmpl
         +  wcslen(new_winnm) * sizeof(WCHAR)
         +  sizeof(DWORD) * 4 * tmpl->cDlgItems;
    alloc_size = size + 8;
    newTmpl = Dll_Alloc(alloc_size);
    if (! newTmpl)
        return NULL;
    memzero(newTmpl, alloc_size);
    *(ULONG *)((UCHAR *)newTmpl + alloc_size - sizeof(ULONG)) = tzuk;

    // now go through the template again, copying over to the new template
    out = (UCHAR *)newTmpl;
    ptr = (UCHAR *)tmpl;
    size = (ULONG)((UCHAR *)&tmpl->menu - ptr);
    memcpy(out, ptr, size);
    out += size;
    ptr += size;

    if (*(WORD *)ptr == 0xFFFF) {                           // menu
        *(WORD *)out = *(WORD *)ptr;
        out += sizeof(WORD);
        ptr += sizeof(WORD);
        *(WORD *)out = *(WORD *)ptr;
        out += sizeof(WORD);
        ptr += sizeof(WORD);
    } else {
        wcscpy((WCHAR *)out, (WCHAR *)ptr);
        out += (wcslen((WCHAR *)out) + 1) * sizeof(WCHAR);
        ptr += (wcslen((WCHAR *)ptr) + 1) * sizeof(WCHAR);
    }

    if (new_clsnm[0]) {                                     // windowClass
        wcscpy((WCHAR *)out, new_clsnm[0]);
        out += (wcslen((WCHAR *)out) + 1) * sizeof(WCHAR);
        ptr += (wcslen((WCHAR *)ptr) + 1) * sizeof(WCHAR);
    } else {
        *(WORD *)out = *(WORD *)ptr;
        out += sizeof(WORD);
        if (*(WORD *)ptr == 0xFFFF) {
            ptr += sizeof(WORD);
            *(WORD *)out = *(WORD *)ptr;
            out += sizeof(WORD);
        }
        ptr += sizeof(WORD);
    }

    wcscpy((WCHAR *)out, new_winnm);                        // title
    out += (wcslen((WCHAR *)out) + 1) * sizeof(WCHAR);
    ptr += (wcslen((WCHAR *)ptr) + 1) * sizeof(WCHAR);
    if (new_winnm != old_winnm)
        Gui_Free(new_winnm);

    if ((tmpl->style & DS_SETFONT) != 0) {
        int ofs = (ULONG)
            ((UCHAR *)&tmpl->typeface - (UCHAR *)&tmpl->pointsize);
        memcpy(out, ptr, ofs);
        out += ofs;
        ptr += ofs;
        wcscpy((WCHAR *)out, (WCHAR *)ptr);                 // typeface
        out += (wcslen((WCHAR *)out) + 1) * sizeof(WCHAR);
        ptr += (wcslen((WCHAR *)ptr) + 1) * sizeof(WCHAR);
    }

    for (i = 0; i < tmpl->cDlgItems; ++i) {
        // every dialog item begins on a DWORD boundary
        memzero(out, sizeof(ULONG));
        out = (UCHAR *)((((SIZE_T)out) + 3) & (~3));
        ptr = (UCHAR *)((((SIZE_T)ptr) + 3) & (~3));

        // skip past the variable-length item
        size = (ULONG)((UCHAR *)&((DLGTMPLITEM *)ptr)->windowClass - ptr);
        memcpy(out, ptr, size);
        out += size;
        ptr += size;

        if (new_clsnm[i + 1]) {             // windowClass
            wcscpy((WCHAR *)out, new_clsnm[i + 1]);
            out += (wcslen((WCHAR *)out) + 1) * sizeof(WCHAR);
            ptr += (wcslen((WCHAR *)ptr) + 1) * sizeof(WCHAR);
        } else {
            *(WORD *)out = *(WORD *)ptr;
            out += sizeof(WORD);
            if (*(WORD *)ptr == 0xFFFF) {
                ptr += sizeof(WORD);
                *(WORD *)out = *(WORD *)ptr;
                out += sizeof(WORD);
            }
            ptr += sizeof(WORD);
        }

        if (*(WORD *)ptr == 0xFFFF) {       // title
            *(WORD *)out = *(WORD *)ptr;
            out += sizeof(WORD);
            ptr += sizeof(WORD);
            *(WORD *)out = *(WORD *)ptr;
            out += sizeof(WORD);
            ptr += sizeof(WORD);
        } else {
            wcscpy((WCHAR *)out, (WCHAR *)ptr);
            out += (wcslen((WCHAR *)out) + 1) * sizeof(WCHAR);
            ptr += (wcslen((WCHAR *)ptr) + 1) * sizeof(WCHAR);
        }

        if (*(WORD *)ptr != 0) {            // extraCount
            size = sizeof(WORD) + *(WORD *)ptr;
            memcpy(out, ptr, size);
            out += size;
            ptr += size;
        } else {
            *(WORD *)out = *(WORD *)ptr;
            out += sizeof(WORD);
            ptr += sizeof(WORD);
        }

        if (new_clsnm[i] && old_clsnm[i] != new_clsnm[i])
            Gui_Free(new_clsnm[i]);
    }

    if (*(ULONG *)((UCHAR *)newTmpl + alloc_size - sizeof(ULONG)) != tzuk)
        SbieApi_Log(2316, NULL);

    return newTmpl;
}

#endif // GUI_CreateDialogTemplate

//---------------------------------------------------------------------------

#undef GUI_DLG_VER
#undef DLGTMPL
#undef DLGTMPLITEM
#undef GUI_CreateDialogTemplate
