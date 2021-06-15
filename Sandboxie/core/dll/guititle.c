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
// GUI Services
//---------------------------------------------------------------------------

#include "dll.h"

#include "gui_p.h"
#include "common/my_version.h"


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------


static int Gui_GetWindowTextW(
    HWND hWnd, WCHAR *lpWindowTitle, int nMaxCount);

static int Gui_GetWindowTextA(
    HWND hWnd, UCHAR *lpWindowTitle, int nMaxCount);


//---------------------------------------------------------------------------
// Variables
//---------------------------------------------------------------------------


BOOLEAN Gui_DisableTitle = FALSE;

const WCHAR *Gui_TitleSuffixW = TITLE_SUFFIX_W;
static ULONG Gui_TitleSuffixW_len = 0;

const UCHAR *Gui_TitleSuffixA = TITLE_SUFFIX_A;
static ULONG Gui_TitleSuffixA_len = 0;

ULONG Gui_BoxNameTitleLen = 0;
WCHAR *Gui_BoxNameTitleW = NULL;
static ANSI_STRING Gui_BoxNameTitleA;


//---------------------------------------------------------------------------
// Gui_InitTitle
//---------------------------------------------------------------------------


_FX BOOLEAN Gui_InitTitle(void)
{
    WCHAR buf[10];

    //
    // initialize title variables
    //

    SbieApi_QueryConfAsIs(NULL, L"BoxNameTitle", 0, buf, sizeof(buf));
    if (*buf == L'y' || *buf == L'Y') { // indicator + box name

        UNICODE_STRING uni;

        Gui_BoxNameTitleLen = wcslen(Dll_BoxName) + 3;
        Gui_BoxNameTitleW =
            Dll_Alloc((Gui_BoxNameTitleLen + 3) * sizeof(WCHAR));
        Gui_BoxNameTitleW[0] = Gui_TitleSuffixW[1];         // L'['
        wcscpy(&Gui_BoxNameTitleW[1], Dll_BoxName);
        wcscat(Gui_BoxNameTitleW, &Gui_TitleSuffixW[3]);    // L"]"
        wcscat(Gui_BoxNameTitleW, L" ");

        RtlInitUnicodeString(&uni, Gui_BoxNameTitleW);
        RtlUnicodeStringToAnsiString(&Gui_BoxNameTitleA, &uni, TRUE);

    } else if (*buf == L'-') // don't alter boxed window titles at all
        Gui_DisableTitle = TRUE;
    //  else if(*buf == L'n' || *buf == L'N') means show indicator but not box name

    Gui_TitleSuffixW_len = wcslen(Gui_TitleSuffixW);
    Gui_TitleSuffixA_len = strlen(Gui_TitleSuffixA);

    //
    // hook functions
    //

    if (! Gui_DisableTitle) {

        SBIEDLL_HOOK_GUI(GetWindowTextW);
        SBIEDLL_HOOK_GUI(GetWindowTextA);
    }

    return TRUE;
}


//---------------------------------------------------------------------------
// Gui_ShouldCreateTitle
//---------------------------------------------------------------------------


_FX BOOLEAN Gui_ShouldCreateTitle(HWND hWnd)
{
    if (Gui_DisableTitle)
        return FALSE;

    if (hWnd == (HWND)(ULONG_PTR)tzuk)      // console window
        return TRUE;

    if (__sys_GetParent(hWnd)) {

        //
        // don't do title for child windows (i.e. windows that
        // have a parent), except if this is a popup dialog window
        //

        ULONG_PTR dlgproc;
        if (__sys_IsWindowUnicode(hWnd))
            dlgproc = __sys_GetWindowLongPtrW(hWnd, DWLP_DLGPROC);
        else
            dlgproc = __sys_GetWindowLongPtrA(hWnd, DWLP_DLGPROC);
        if (dlgproc) {

            ULONG style = (ULONG)__sys_GetWindowLongW(hWnd, GWL_STYLE);
            if (style & WS_POPUP)
                return TRUE;
        }

    } else {

        //
        // do title if the window has a caption, unless it is a
        // window of type Edit control
        //

        ULONG style = (ULONG)__sys_GetWindowLongW(hWnd, GWL_STYLE);
        if ((style & WS_CAPTION) == WS_CAPTION) {

            WCHAR clsnm[256];
            UINT nChars = __sys_RealGetWindowClassW(hWnd, clsnm, sizeof(clsnm) - 1);

            // MS stupidly added a WS_CAPTION attribute to some hidden window that comes up with the Office splash screens -- but they don't actually
            // have any captions. When we replace the caption, Office doesn't like it and goes into an infinite loop calling SetWindowPos.
            // The only way I can see to detect these splash screens reliably is to check for each individual dialog class name.
            if (wcsstr(clsnm, L":XLMAIN"))          // Excel
                return FALSE;
            if (wcsstr(clsnm, L":OpusApp"))         // Word
                return FALSE;
            if (wcsstr(clsnm, L":PPTFrameClass"))   // PowerPoint
                return FALSE;
            if (wcsstr(clsnm, L":MSWinPub"))        // Publisher
                return FALSE;
            if (wcsstr(clsnm, L":rctrl_renwnd32"))  // Outlook
                return FALSE;
            if (wcsstr(clsnm, L":Framework::CFrame"))   // Onenote
                return FALSE;

            if (_wcsicmp(clsnm, L"Edit") != 0)
                return TRUE;
            
        }
    }

    return FALSE;
}


//---------------------------------------------------------------------------
// Gui_CreateTitleW
//---------------------------------------------------------------------------


_FX WCHAR *Gui_CreateTitleW(const WCHAR *oldTitle)
{
    WCHAR *newTitle, *ptr;
    ULONG len_new, len_old;

    if ((! oldTitle) || Gui_DisableTitle)
        return (WCHAR *)oldTitle;

    __try {

        len_old = wcslen(oldTitle);

    } __except (EXCEPTION_EXECUTE_HANDLER) {

        return (WCHAR *)oldTitle;
    }

    if (len_old > Gui_TitleSuffixW_len) {
        ptr = (WCHAR *)oldTitle + len_old - Gui_TitleSuffixW_len;
        if (wmemcmp(ptr, Gui_TitleSuffixW, Gui_TitleSuffixW_len) == 0)
            return (WCHAR *)oldTitle;
    }

    len_new = (len_old + Gui_TitleSuffixW_len * 2 + 1) * sizeof(WCHAR);
    if (Gui_BoxNameTitleLen)
        len_new += 40 * sizeof(WCHAR);
    newTitle = Dll_Alloc(len_new);

    wmemcpy(newTitle, Gui_TitleSuffixW + 1, Gui_TitleSuffixW_len - 1);
    ptr = newTitle + Gui_TitleSuffixW_len - 1;
    *ptr = L' ';
    ++ptr;

    if (Gui_BoxNameTitleLen) {
        wmemcpy(ptr, Gui_BoxNameTitleW, Gui_BoxNameTitleLen);
        ptr += Gui_BoxNameTitleLen;
    }

    wmemcpy(ptr, oldTitle, len_old);
    ptr += len_old;
    wmemcpy(ptr, Gui_TitleSuffixW, Gui_TitleSuffixW_len);
    ptr += Gui_TitleSuffixW_len;
    *ptr = L'\0';

    return newTitle;
}


//---------------------------------------------------------------------------
// Gui_CreateTitleA
//---------------------------------------------------------------------------


_FX UCHAR *Gui_CreateTitleA(const UCHAR *oldTitle)
{
    UCHAR *newTitle, *ptr;
    ULONG len_new, len_old;

    if ((! oldTitle) || Gui_DisableTitle)
        return (char *)oldTitle;

    __try {

        len_old = strlen(oldTitle);

    } __except (EXCEPTION_EXECUTE_HANDLER) {

        return (char *)oldTitle;
    }

    if (len_old > Gui_TitleSuffixA_len) {
        ptr = (UCHAR *)oldTitle + len_old - Gui_TitleSuffixA_len;
        if (memcmp(ptr, Gui_TitleSuffixA, Gui_TitleSuffixA_len) == 0)
            return (UCHAR *)oldTitle;
    }

    len_new = (len_old + Gui_TitleSuffixA_len * 2 + 1) * sizeof(UCHAR);
    if (Gui_BoxNameTitleLen)
        len_new += 40 * sizeof(UCHAR);
    newTitle = Dll_Alloc(len_new);

    memcpy(newTitle, Gui_TitleSuffixA + 1, Gui_TitleSuffixA_len - 1);
    ptr = newTitle + Gui_TitleSuffixA_len - 1;
    *ptr = ' ';
    ++ptr;

    if (Gui_BoxNameTitleLen) {
        memcpy(ptr, Gui_BoxNameTitleA.Buffer, Gui_BoxNameTitleLen);
        ptr += Gui_BoxNameTitleLen;
    }

    memcpy(ptr, oldTitle, len_old);
    ptr += len_old;
    memcpy(ptr, Gui_TitleSuffixA, Gui_TitleSuffixA_len);
    ptr += Gui_TitleSuffixA_len;
    *ptr = '\0';

    return newTitle;
}


//---------------------------------------------------------------------------
// Gui_FixTitleW
//---------------------------------------------------------------------------


_FX int Gui_FixTitleW(HWND hWnd, WCHAR *lpWindowTitle, int len)
{
    if (len >= (int)Gui_TitleSuffixW_len * 2 &&
                                            Gui_ShouldCreateTitle(hWnd)) {

        if (wmemcmp(lpWindowTitle, &Gui_TitleSuffixW[1], 3) == 0) {
            len -= 4;
            wmemmove(lpWindowTitle, lpWindowTitle + 4, len);
            lpWindowTitle[len] = L'\0';
        }
        if (wmemcmp(lpWindowTitle + len - 4, Gui_TitleSuffixW, 4) == 0) {
            len -= 4;
            lpWindowTitle[len] = L'\0';
        }
        if (Gui_BoxNameTitleLen) {
            const int lenTitle    = Gui_BoxNameTitleLen;
            const WCHAR *ptrTitle = Gui_BoxNameTitleW;
            if (len >= lenTitle
                    && wmemcmp(lpWindowTitle, ptrTitle, lenTitle) == 0) {
                len -= lenTitle;
                wmemmove(lpWindowTitle, lpWindowTitle + lenTitle, len);
                lpWindowTitle[len] = L'\0';
            }
        }
    }

    return len;
}


//---------------------------------------------------------------------------
// Gui_FixTitleA
//---------------------------------------------------------------------------


_FX int Gui_FixTitleA(HWND hWnd, UCHAR *lpWindowTitle, int len)
{
    if (len >= (int)Gui_TitleSuffixA_len * 2 &&
                                            Gui_ShouldCreateTitle(hWnd)) {

        if (memcmp(lpWindowTitle, &Gui_TitleSuffixA[1], 3) == 0) {
            len -= 4;
            memmove(lpWindowTitle, lpWindowTitle + 4, len);
            lpWindowTitle[len] = '\0';
        }
        if (memcmp(lpWindowTitle + len - 4, Gui_TitleSuffixA, 4) == 0) {
            len -= 4;
            lpWindowTitle[len] = '\0';
        }
        if (Gui_BoxNameTitleLen) {
            const int lenTitle    = Gui_BoxNameTitleLen;
            const UCHAR *ptrTitle = Gui_BoxNameTitleA.Buffer;
            if (len >= lenTitle
                    && memcmp(lpWindowTitle, ptrTitle, lenTitle) == 0) {
                len -= lenTitle;
                memmove(lpWindowTitle, lpWindowTitle + lenTitle, len);
                lpWindowTitle[len] = '\0';
            }
        }
    }

    return len;
}


//---------------------------------------------------------------------------
// Gui_GetWindowTextW
//---------------------------------------------------------------------------


_FX int Gui_GetWindowTextW(
    HWND hWnd, WCHAR *lpWindowTitle, int nMaxCount)
{
    int rc = __sys_GetWindowTextW(hWnd, lpWindowTitle, nMaxCount);
    return Gui_FixTitleW(hWnd, lpWindowTitle, rc);
}


//---------------------------------------------------------------------------
// Gui_GetWindowTextA
//---------------------------------------------------------------------------


_FX int Gui_GetWindowTextA(
    HWND hWnd, UCHAR *lpWindowTitle, int nMaxCount)
{
    int rc = __sys_GetWindowTextA(hWnd, lpWindowTitle, nMaxCount);
    return Gui_FixTitleA(hWnd, lpWindowTitle, rc);
}
