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


#ifndef _MY_FINDERDIALOG_H
#define _MY_FINDERDIALOG_H


class CFinderDialog : public CBaseDialog
{
    static CFinderDialog *m_instance;

    CSize m_InitialSize;

    CWnd *m_wndClose;
    int m_idClose;

    ULONG m_pid;

    virtual BOOL OnInitDialog();

    static UINT CALLBACK FindProc(HWND hwndTool, UINT uCode, HWND hwnd);
    void ShowInfo(ULONG pid);
    void HideInfo();

    virtual void OnCancel();

public:

    CFinderDialog(CWnd *pParentWnd, int id);
    ~CFinderDialog();

    ULONG GetProcessId();
};


#endif // _MY_FINDERDIALOG_H
