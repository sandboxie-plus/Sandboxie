/*
 * Copyright 2022 David Xanatos, xanasoft.com
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
// Run Sandboxed Dialog
//---------------------------------------------------------------------------

#include "stdafx.h"

#include "resource.h"
#include "msgs/msgs.h"
#define COBJMACROS
#include "core/dll/sbiedll.h"
#include "core/dll/pstore.h"
#include "common/defines.h"
#include "apps/common/CommonUtils.h"
#include "common/my_version.h"
#define INITGUID
#include <guiddef.h>
#include <commdlg.h>
#include "apps/common/MyGdi.h"
#include "common/win32_ntddk.h"
#include "core/drv/api_defs.h"
#include <time.h>
#include "core/svc/InteractiveWire.h"



//---------------------------------------------------------------------------
// Types
//---------------------------------------------------------------------------


#ifndef _T
#define __T(x)      L ## x
#define _T(x)       __T(x)
#endif


struct SScopedStr
{
    SScopedStr(WCHAR* str) { c_str = str; }
    ~SScopedStr() { LocalFree(c_str); }

    WCHAR* c_str;
};


//---------------------------------------------------------------------------
// Variables
//---------------------------------------------------------------------------


static HICON hProgramIcon;

static HBITMAP hLogoBitmap;

static BOOL initialized = FALSE;

static BOOL g_bReminder = FALSE;

static int btnActions[3] = { 0, 0, 0 };

static UINT_PTR timer_id = 0;
static int btnCountDown = 5;

static int Days = 0;

extern BOOLEAN layout_rtl;

//---------------------------------------------------------------------------
// RunDialogProc
//---------------------------------------------------------------------------


INT_PTR AboutDialogProc(
    HWND hwnd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam)
{
    switch (uMsg) {

        //
        // initialize dialog
        //

        case WM_INITDIALOG:
        {
            RECT rc;

            //
            // position window in the middle of the screen
            //

            GetClientRect(GetDesktopWindow(), &rc);
            int x = (rc.left + rc.right) / 3;
            int y = (rc.top + rc.bottom) / 3;
            GetClientRect(hwnd, &rc);
            x -= rc.right / 2;
            y -= rc.bottom / 2;
            SetWindowPos(hwnd, HWND_TOP, x, y, 0, 0, SWP_NOSIZE);

            //
            // setup logo
            //

            HWND hwndPic = GetDlgItem(hwnd, ID_ABOUT_LOGO);
            ::SendMessage(hwndPic, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hLogoBitmap);
            
            GetClientRect(hwnd, &rc);
            ULONG wDlg = rc.right - rc.left;

            GetClientRect(hwndPic, &rc);
            ULONG wBtn = rc.right - rc.left;
            ULONG hBtn = rc.bottom - rc.top;

            GetWindowRect(hwndPic, &rc);
            ScreenToClient(hwnd, (POINT *)&rc);
            rc.left = (wDlg - wBtn) / 2;
            MoveWindow(hwndPic, rc.left, rc.top, wBtn, hBtn, TRUE);

            //
            // adjust frame
            //

            GetWindowRect(hwndPic, &rc);
	        ScreenToClient(hwnd, (LPPOINT)&rc);
	        ScreenToClient(hwnd, ((LPPOINT)&rc)+1);
            rc.left -= 5;
            rc.top -= 10;
            rc.right += 3 * 2;
            rc.bottom += 3 * 2;
            wBtn = rc.right - rc.left;
            hBtn = rc.bottom - rc.top;
            MoveWindow(GetDlgItem(hwnd,ID_ABOUT_FRAME), rc.left, rc.top, wBtn, hBtn, TRUE);

            //
            // set window title
            //

            SetWindowText(hwnd, SScopedStr(SbieDll_FormatMessage0(MSG_3601)).c_str);

            //
            // set info text
            //

            WCHAR* info = (WCHAR *)HeapAlloc(GetProcessHeap(), 0, 2048);
            if (g_bReminder) 
            {
                WCHAR sDays[10];
                swprintf(sDays, L"%d", Days);
                wcscpy(info, SScopedStr(SbieDll_FormatMessage1(MSG_6010, sDays)).c_str);

                for (int s = rand() % 3, i = 0; i < 3; i++)
                    btnActions[i] = (s + i) % 3;

                SetDlgItemText(hwnd, ID_BUTTON_2, SScopedStr(SbieDll_FormatMessage0(MSG_6011)).c_str);

                timer_id = SetTimer(hwnd, 'wait', 1000, NULL);
            }
            else
            {
                wcscpy(info, SScopedStr(SbieDll_FormatMessage1(MSG_3302, _T(MY_VERSION_STRING))).c_str);
                wcscat(info, L"\r\n\r\n");
                wcscat(info, _T(MY_COPYRIGHT_STRING));
                wcscat(info, L"\r\n");
                wcscat(info, _T(MY_COPYRIGHT_STRING_OLD));

                ShowWindow(GetDlgItem(hwnd, ID_BUTTON_1), SW_HIDE);
                SetDlgItemText(hwnd, ID_BUTTON_2, SScopedStr(SbieDll_FormatMessage0(MSG_3001)).c_str);
                ShowWindow(GetDlgItem(hwnd, ID_BUTTON_3), SW_HIDE);
            }

            SetDlgItemText(hwnd, ID_ABOUT_INFO, info);
            HeapFree(GetProcessHeap(), 0, info);

            //
            // end dialog initialization
            //

            SetForegroundWindow(hwnd);  // explicitly go to foreground

            return FALSE;   // don't set focus, we already did
        }

        //
        // handle timer
        //

        case WM_TIMER:
        {
            if (wParam != 'wait')
                break;

            if (btnCountDown >= 0) 
            {
                if (btnCountDown-- > 0) 
                {
                    WCHAR wait[10];
                    swprintf(wait, L"%d", btnCountDown + 1);
                    SetDlgItemText(hwnd, ID_BUTTON_2, wait);
                }
                else 
                {
                    int ButtonIDs[] = { ID_BUTTON_1, ID_BUTTON_2, ID_BUTTON_3 };
                    for (int i = 0; i < 3; i++) {
                        switch (btnActions[i]) {
                        case 0: SetDlgItemText(hwnd, ButtonIDs[i], SScopedStr(SbieDll_FormatMessage0(MSG_6013)).c_str); break;
                        case 1: SetDlgItemText(hwnd, ButtonIDs[i], SScopedStr(SbieDll_FormatMessage0(MSG_6012)).c_str); break;
                        case 2: SetDlgItemText(hwnd, ButtonIDs[i], SScopedStr(SbieDll_FormatMessage0(MSG_6014)).c_str); break;
                        }
                    }

                    if(timer_id)
                        KillTimer(NULL, timer_id);
                }
            }
            break;
        }

        //
        // handle buttons
        //

        case WM_COMMAND:

            int Actions = 0;
            if (LOWORD(wParam) == ID_BUTTON_1) {
                Actions = btnActions[0];
            } else if (LOWORD(wParam) == ID_BUTTON_2) {
                Actions = btnActions[1];
            } else if (LOWORD(wParam) == ID_BUTTON_3) {
                Actions = btnActions[2];
            } /*else if (LOWORD(wParam) == IDCANCEL) {

                EndDialog(hwnd, IDOK);

            } */

            if (Actions == 1)
                EndDialog(hwnd, IDOK);
            else if (Actions == 2)
                ShellExecute(NULL , NULL, L"https://sandboxie-plus.com/go.php?to=sbie-get-cert", NULL, NULL, SW_SHOWNORMAL);
            else
                EndDialog(hwnd, IDCANCEL);

            break;
    }

    return FALSE;
}


//---------------------------------------------------------------------------
// DoRunDialog
//---------------------------------------------------------------------------


bool DoAboutDialog(bool bReminder)
{
    HINSTANCE hInstance = GetModuleHandle(NULL);
    INITCOMMONCONTROLSEX icc;
    INT_PTR r;

    srand(GetTickCount());

    g_bReminder = bReminder;

    if (g_bReminder) {

        ULONG64 CertInfo = 0;
        SbieApi_Call(API_QUERY_DRIVER_INFO, 3, -1, (ULONG_PTR)&CertInfo, sizeof(CertInfo));

        if (CertInfo & 1) // valid
            return true;

        time_t InstallDate = 0;
        SbieApi_Call(API_GET_SECURE_PARAM, 3, L"InstallationDate", (ULONG_PTR)&InstallDate, sizeof(InstallDate));

        time_t CurrentTime;
        time(&CurrentTime);

        if (InstallDate == 0 || InstallDate > CurrentTime) {
            InstallDate = CurrentTime;
            SbieApi_Call(API_SET_SECURE_PARAM, 3, L"InstallationDate", (ULONG_PTR)&InstallDate, sizeof(InstallDate));
        }

        Days = (int)((CurrentTime - InstallDate) / (24 * 3600));

        if (!SbieApi_QueryConfBool(L"GlobalSettings", L"AlwaysShowReminder", FALSE)) {

            if (Days < 40)
                return true;

		    int Interval;
            if (Days > 730) Interval = 5 * 24;
            else if (Days > 365) Interval = 10 * 24;
            else if (Days > 180) Interval = 20 * 24;
            else Interval = 30 * 24;

		    USHORT ReminderShedule[2*11];
		    if (NT_SUCCESS(SbieApi_Call(API_GET_SECURE_PARAM, 3, L"ReminderShedule", (ULONG_PTR)&ReminderShedule, sizeof(ReminderShedule)))) {
			    for (USHORT* Cur = ReminderShedule; (ULONG_PTR)Cur < (ULONG_PTR)ReminderShedule + sizeof(ReminderShedule) && *Cur != 0; Cur += 2) {
				    if (Days > Cur[0]) {
					    if (Interval > Cur[1]) 
						    Interval = Cur[1];
					    break;
				    }
			    }
		    }

			time_t LastReminder = 0;
            SbieApi_Call(API_GET_SECURE_PARAM, 3, L"LastReminder", (ULONG_PTR)&LastReminder, sizeof(LastReminder));
			if (LastReminder > 0 && LastReminder < CurrentTime) {
				if (CurrentTime - LastReminder < (time_t(Interval) * 3600))
					return true;
			}

            if ((rand() % 5) != 0)
                return true;
        }

        ULONG req = -1;
        ULONG* rpl;
	    rpl = (ULONG*)SbieDll_CallServerQueue(INTERACTIVE_QUEUE_NAME, &req, sizeof(req), sizeof(*rpl));
	    if (rpl)
	    {
            ULONG status = rpl[0];
            ULONG retval = rpl[1];
		    SbieDll_FreeMem(rpl);

            return !!retval;
	    }

	    SbieApi_Call(API_SET_SECURE_PARAM, 3, L"LastReminder", (ULONG_PTR)&CurrentTime, sizeof(CurrentTime));
    }

    if (! initialized) {

        icc.dwSize = sizeof(INITCOMMONCONTROLSEX);
        icc.dwICC = ICC_USEREX_CLASSES | ICC_TAB_CLASSES;
        InitCommonControlsEx(&icc);

        hProgramIcon = (HICON)LoadImage(
            hInstance, MAKEINTRESOURCE(IDICON), IMAGE_ICON,
            0, 0, LR_DEFAULTSIZE);

        MyGdi_Init();

        hLogoBitmap = MyGdi_CreateFromResource(L"MASTHEADLOGO");

        initialized = TRUE;
    }

    if (layout_rtl) {

        LPCDLGTEMPLATE tmpl = (LPCDLGTEMPLATE)Common_DlgTmplRtl(
                                    hInstance, MAKEINTRESOURCE(ABOUT_DIALOG));
        r = DialogBoxIndirectParam(hInstance, tmpl,
                           NULL, AboutDialogProc, (LPARAM)hInstance);

    } else {

        r = DialogBoxParam(hInstance, MAKEINTRESOURCE(ABOUT_DIALOG),
                           NULL, AboutDialogProc, (LPARAM)hInstance);
    }

    return (r == IDOK);
}
