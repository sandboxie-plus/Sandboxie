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
// Resource Monitor Dialog Box
//---------------------------------------------------------------------------


#ifndef _MY_MONITORDIALOG_H
#define _MY_MONITORDIALOG_H


class CMonitorDialog : public CBaseDialog
{
    // DECLARE_MESSAGE_MAP()

    WCHAR m_username[256];
    ULONG m_username_len;
	//ULONG m_last_entry_seq_num;

    virtual BOOL OnInitDialog();

    virtual void OnOK();

public:

    CMonitorDialog(CWnd *pParentWnd);
    ~CMonitorDialog();

    void OnIdle();

};


#endif // _MY_MONITORDIALOG_H
