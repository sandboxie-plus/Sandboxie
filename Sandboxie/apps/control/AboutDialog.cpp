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
// About Sandboxie Dialog Box
//---------------------------------------------------------------------------

#include "stdafx.h"
#include "MyApp.h"
#include "AboutDialog.h"

#include "SbieIni.h"
#include "common/my_version.h"
#include "apps/common/MyGdi.h"
#include "apps/common/RunBrowser.h"


//---------------------------------------------------------------------------
// Message Map
//---------------------------------------------------------------------------


BEGIN_MESSAGE_MAP(CAboutDialog, CBaseDialog)

    ON_COMMAND(ID_ABOUT_TUTORIAL,       OnTutorial)

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

    GetDlgItem(IDOK)->SetWindowText(CMyMsg(MSG_3001));

    return TRUE;
}


//---------------------------------------------------------------------------
// OnRegister
//---------------------------------------------------------------------------


void CAboutDialog::OnRegister()
{
}


//---------------------------------------------------------------------------
// OnTutorial
//---------------------------------------------------------------------------


void CAboutDialog::OnTutorial()
{
    GetParent()->PostMessage(WM_COMMAND, ID_HELP_TUTORIAL, 0);
    EndDialog(0);
}


//---------------------------------------------------------------------------
// OnOK
//---------------------------------------------------------------------------


void CAboutDialog::OnOK()
{
    EndDialog(0);
}
