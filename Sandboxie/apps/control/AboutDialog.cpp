/*
 * Copyright 2004-2020 Sandboxie Holdings, LLC 
 * Copyright 2020-2023 David Xanatos, xanasoft.com
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
// About Sandboxie Dialog Box
//---------------------------------------------------------------------------

#include "stdafx.h"
#include "MyApp.h"
#include "AboutDialog.h"

#include "SbieIni.h"
#include "common/my_version.h"
#include "apps/common/MyGdi.h"
#include "apps/common/RunBrowser.h"
#include "common/win32_ntddk.h"
#include "core/drv/api_defs.h"


//---------------------------------------------------------------------------
// Message Map
//---------------------------------------------------------------------------


BEGIN_MESSAGE_MAP(CAboutDialog, CBaseDialog)

END_MESSAGE_MAP()


//---------------------------------------------------------------------------
// Variables
//---------------------------------------------------------------------------


HBITMAP CAboutDialog::m_bitmap = NULL;


//---------------------------------------------------------------------------
// Constructor
//---------------------------------------------------------------------------


CAboutDialog::CAboutDialog(CWnd *pParentWnd)
    : CBaseDialog(pParentWnd, NULL)
{
    //
    // prepare image
    //

    if (! m_bitmap)
        m_bitmap = MyGdi_CreateFromResource(L"MASTHEADLOGO");

    //
    // display dialog
    //

    CString DialogTemplateName = L"ABOUT_DIALOG";
    SetDialogTemplate(DialogTemplateName);

    DoModal();
}


//---------------------------------------------------------------------------
// Destructor
//---------------------------------------------------------------------------


CAboutDialog::~CAboutDialog()
{
}


//---------------------------------------------------------------------------
// OnInitDialog
//---------------------------------------------------------------------------


BOOL CAboutDialog::OnInitDialog()
{
    CStatic *pic = (CStatic *)GetDlgItem(ID_ABOUT_LOGO);
    pic->SetBitmap(m_bitmap);

    CRect rc;
    GetClientRect(&rc);
    ULONG wDlg = rc.right - rc.left;

    pic->GetClientRect(&rc);
    ULONG wBtn = rc.right - rc.left;
    ULONG hBtn = rc.bottom - rc.top;

    pic->GetWindowRect(&rc);
    ScreenToClient((POINT *)&rc);
    rc.left = (wDlg - wBtn) / 2;
    pic->MoveWindow(rc.left, rc.top, wBtn, hBtn, TRUE);

    //
    //
    //

    pic->GetWindowRect(rc);
    ScreenToClient(rc);
    rc.left -= 5;
    rc.top -= 10;
    rc.right += 3 * 2;
    rc.bottom += 3 * 2;
    GetDlgItem(ID_ABOUT_FRAME)->MoveWindow(rc);

    CString text = CMyMsg(MSG_3601);
    SetWindowText(text);

    //
    //
    //

    const ULONG _bitness =
#ifdef _WIN64
                            64;
#else
                            32;
#endif _WIN64

    ULONG U_LRO, U_PDF;
    if (CMyApp::m_LayoutRTL) {
        U_LRO = 0x202D;   // Start of left-to-right override
        U_PDF = 0x202C;   // Pop directional formatting
    } else {
        U_LRO = L' ';
        U_PDF = L' ';
    }
    text.Format(L"%S %c(%d-bit)%c",
        MY_VERSION_STRING, U_LRO, _bitness, U_PDF);

    CString ver = CMyMsg(MSG_3302, text);
    GetDlgItem(ID_ABOUT_VERSION)->SetWindowText(ver);

    //
    //
    //

    text.Format(L"%S\r\n%S", MY_COPYRIGHT_STRING, MY_COPYRIGHT_STRING_OLD);
    GetDlgItem(ID_ABOUT_COPYRIGHT)->SetWindowText(text);

    ULONG64 CertInfo = 0;
    SbieApi_Call(API_QUERY_DRIVER_INFO, 3, -1, (ULONG_PTR)&CertInfo, sizeof(CertInfo));
    if (CertInfo & 1) // valid
        GetDlgItem(ID_ABOUT_INFO)->SetWindowText(CMyMsg(MSG_7988));
    else if (CertInfo & 2) // expired
        GetDlgItem(ID_ABOUT_INFO)->SetWindowText(CMyMsg(MSG_7989));


    GetDlgItem(IDOK)->SetWindowText(CMyMsg(MSG_3001));

    return TRUE;
}


//---------------------------------------------------------------------------
// OnOK
//---------------------------------------------------------------------------


void CAboutDialog::OnOK()
{
    EndDialog(0);
}


//---------------------------------------------------------------------------
// ApplyCertificate
//---------------------------------------------------------------------------


void ApplyCertificate()
{
    if (CMyApp::MsgBox(NULL, MSG_7990, MB_OKCANCEL) != IDOK)
        return;
    
    WCHAR CertPath[MAX_PATH];
    GetTempPath(MAX_PATH, CertPath);
    wcscat(CertPath, L"Sbie+Certificate.dat");

    ULONG lenWritten = 0;
    if (OpenClipboard(nullptr)) {
        HANDLE hData = GetClipboardData(CF_UNICODETEXT);
        if (hData != nullptr) {
            WCHAR* pszText = static_cast<WCHAR*>(GlobalLock(hData));
            if (pszText != nullptr) {
                HANDLE hFile = CreateFile(CertPath, FILE_GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
                if (hFile != INVALID_HANDLE_VALUE) {
                    ULONG utf8_len = WideCharToMultiByte(CP_UTF8, 0, pszText, wcslen(pszText), NULL, 0, NULL, NULL);
                    char* text_utf8 = (char*)HeapAlloc(GetProcessHeap(), 0, utf8_len);
                    if (text_utf8) {
                        ULONG lenToWrite = WideCharToMultiByte(CP_UTF8, 0, pszText, wcslen(pszText), text_utf8, utf8_len, NULL, NULL);
                        if (! WriteFile(hFile, (void*)text_utf8, lenToWrite, &lenWritten, NULL))
                            lenWritten = 0;
                        HeapFree(GetProcessHeap(), 0, text_utf8);
                    }
                    CloseHandle(hFile);
                }
                GlobalUnlock(hData);
            }
        }
        CloseClipboard();
    }

    if (lenWritten == 0) {
        CMyApp::MsgBox(NULL, MSG_7991, MB_OK | MB_ICONERROR);
        return;
    }

    WCHAR HomePath[MAX_PATH];
    SbieApi_GetHomePath(NULL, 0, HomePath, MAX_PATH);
    wcscat(HomePath, L"\\Certificate.dat");

    SHFILEOPSTRUCT SHFileOp;
    memset(&SHFileOp, 0, sizeof(SHFILEOPSTRUCT));
    SHFileOp.hwnd = NULL;
    SHFileOp.wFunc = FO_MOVE; // FO_DELETE;
	SHFileOp.pFrom = CertPath;
    SHFileOp.pTo = HomePath;
    SHFileOp.fFlags = NULL;    
    SHFileOperation(&SHFileOp);

    NTSTATUS status = SbieApi_Call(API_RELOAD_CONF, 2, -1, SBIE_CONF_FLAG_RELOAD_CERT);
    if (!NT_SUCCESS(status)) {
        CMyApp::MsgBox(NULL, MSG_7992, MB_OK | MB_ICONWARNING);
        return;
    }

    CMyApp::MsgBox(NULL, MSG_7993, MB_OK | MB_ICONINFORMATION);
}

