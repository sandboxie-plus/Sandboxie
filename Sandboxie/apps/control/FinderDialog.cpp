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
// Finder Tool Dialog Box
//---------------------------------------------------------------------------


#include "stdafx.h"
#include "MyApp.h"
#include "FinderDialog.h"

#include "FindTool.h"
#include "Boxes.h"


//---------------------------------------------------------------------------
// Variables
//---------------------------------------------------------------------------


CFinderDialog *CFinderDialog::m_instance = NULL;


//---------------------------------------------------------------------------
// Constructor
//---------------------------------------------------------------------------


CFinderDialog::CFinderDialog(CWnd *pParentWnd, int id)
    : CBaseDialog(pParentWnd, L"FINDER_TOOL")
{
    m_instance = this;
    m_pid = 0;

    m_wndClose = pParentWnd;
    m_idClose = id;

    if (CBaseDialog::m_template_copy)
        CDialog::CreateIndirect(m_template_copy, pParentWnd);
    else
        CDialog::Create(m_lpszTemplateName, pParentWnd);

    MakeFinderTool(GetDlgItem(ID_FINDER_TARGET)->m_hWnd, FindProc);

    CRect rc;
    GetWindowRect(&rc);
    m_InitialSize = CSize(rc.right - rc.left, rc.bottom - rc.top);

    SetWindowPos(
        &wndTopMost, 0, 0, 0, 0, SWP_SHOWWINDOW | SWP_NOSIZE | SWP_NOMOVE);
}


//---------------------------------------------------------------------------
// Destructor
//---------------------------------------------------------------------------


CFinderDialog::~CFinderDialog()
{
    DestroyWindow();
}


//---------------------------------------------------------------------------
// OnInitDialog
//---------------------------------------------------------------------------


BOOL CFinderDialog::OnInitDialog()
{
    SetWindowText(CMyMsg(MSG_3661));

    GetDlgItem(ID_FINDER_EXPLAIN)->SetWindowText(CMyMsg(MSG_3662));

    GetDlgItem(IDCANCEL)->SetWindowText(CMyMsg(MSG_3004));

    return TRUE;
}


//---------------------------------------------------------------------------
// FindProc
//---------------------------------------------------------------------------


UINT CALLBACK CFinderDialog::FindProc(HWND hwndTool, UINT uCode, HWND hwnd)
{
    if (uCode == WFN_CANCELLED)
        m_instance->OnCancel();
    else {

        ULONG pid;
        if (uCode == WFN_END)
            GetWindowThreadProcessId(hwnd, &pid);
        else
            pid = 0;
        if (pid)
            m_instance->ShowInfo(pid);
        else
            m_instance->HideInfo();
    }

    return 0;
}


//---------------------------------------------------------------------------
// ShowInfo
//---------------------------------------------------------------------------


void CFinderDialog::ShowInfo(ULONG pid)
{
    m_pid = pid;
    if (pid == GetCurrentProcessId()) {
        HideInfo();
        return;
    }

    SetWindowPos(
        &wndTopMost, 0, 0, m_InitialSize.cx, m_InitialSize.cy + 160,
        SWP_SHOWWINDOW | SWP_NOMOVE);

    CBoxes &boxes = CBoxes::GetInstance();
    CBox &box = boxes.GetBoxByProcessId(pid);
    const CString &boxName = box.GetName();
    if (! boxName.IsEmpty()) {

        CBoxProc &boxproc = box.GetBoxProc();
        int index = boxproc.GetIndexForProcessId(pid);
        const CString &imageName = boxproc.GetProcessImageName(index);
        GetDlgItem(ID_FINDER_RESULT)->SetWindowText(
            CMyMsg(MSG_3663, imageName, boxName));

        ::ShowWindow(GetDlgItem(ID_FINDER_YES_BOXED)->m_hWnd, SW_SHOW);

    } else {

        GetDlgItem(ID_FINDER_RESULT)->SetWindowText(CMyMsg(MSG_3664));

        ::ShowWindow(GetDlgItem(ID_FINDER_NOT_BOXED)->m_hWnd, SW_SHOW);
    }

    ::ShowWindow(GetDlgItem(ID_FINDER_RESULT)->m_hWnd, SW_SHOW);
    ::ShowWindow(GetDlgItem(IDCANCEL)->m_hWnd, SW_SHOW);
}


//---------------------------------------------------------------------------
// HideInfo
//---------------------------------------------------------------------------


void CFinderDialog::HideInfo()
{
    if (! m_pid)
        return;

    m_pid = 0;

    SetWindowPos(
        &wndTopMost, 0, 0, m_InitialSize.cx, m_InitialSize.cy,
        SWP_SHOWWINDOW | SWP_NOMOVE);

    ::ShowWindow(GetDlgItem(ID_FINDER_YES_BOXED)->m_hWnd, SW_HIDE);
    ::ShowWindow(GetDlgItem(ID_FINDER_NOT_BOXED)->m_hWnd, SW_HIDE);
    ::ShowWindow(GetDlgItem(ID_FINDER_RESULT)->m_hWnd, SW_HIDE);
    ::ShowWindow(GetDlgItem(IDCANCEL)->m_hWnd, SW_HIDE);
}


//---------------------------------------------------------------------------
// OnCancel
//---------------------------------------------------------------------------


void CFinderDialog::OnCancel()
{
    ::SendMessage(m_wndClose->m_hWnd, WM_COMMAND, m_idClose, 0);
}


//---------------------------------------------------------------------------
// GetProcessId
//---------------------------------------------------------------------------


ULONG CFinderDialog::GetProcessId()
{
    return m_pid;
}
