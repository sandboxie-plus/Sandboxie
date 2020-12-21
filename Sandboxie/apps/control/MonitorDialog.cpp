/*
 * Copyright 2004-2020 Sandboxie Holdings, LLC 
 * Copyright 2020 David Xanatos, xanasoft.com
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


#include "stdafx.h"
#include "MyApp.h"
#include "MonitorDialog.h"

#include "common/win32_ntddk.h"
#include "core/drv/api_defs.h"


//---------------------------------------------------------------------------
// Constructor
//---------------------------------------------------------------------------


CMonitorDialog::CMonitorDialog(CWnd *pParentWnd)
    : CBaseDialog(pParentWnd, L"MONITOR_DIALOG")
{
    m_username_len = 256;
    if (! GetUserName(m_username, &m_username_len))
        m_username[0] = L'\0';
    m_username[255] = L'\0';
    m_username_len = wcslen(m_username);

	m_last_entry_seq_num = 0;
}


//---------------------------------------------------------------------------
// Destructor
//---------------------------------------------------------------------------


CMonitorDialog::~CMonitorDialog()
{
    ULONG NewState = FALSE;
    SbieApi_MonitorControl(&NewState, NULL);
}


//---------------------------------------------------------------------------
// OnInitDialog
//---------------------------------------------------------------------------


BOOL CMonitorDialog::OnInitDialog()
{
    SetWindowText(CMyMsg(MSG_3655));

    GetDlgItem(ID_MESSAGE_HELP)->SetWindowText(CMyMsg(MSG_3656));

    GetDlgItem(IDOK)->SetWindowText(CMyMsg(MSG_3657));

    MakeLTR(ID_MESSAGE_LIST);

    CFont font;
    font.CreateStockObject(ANSI_FIXED_FONT);
    CListBox *listbox = (CListBox *)GetDlgItem(ID_MESSAGE_LIST);
    listbox->SetFont(&font);
    listbox->SetHorizontalExtent(3000);

    AddMinimizeButton();

    ULONG NewState = TRUE;
    SbieApi_MonitorControl(&NewState, NULL);

    return TRUE;
}


//---------------------------------------------------------------------------
// OnIdle
//---------------------------------------------------------------------------


void CMonitorDialog::OnIdle()
{
    static const WCHAR *_Unknown    = L"(Unk)    ";
    static const WCHAR *_Pipe       = L"Pipe     ";
    static const WCHAR *_Ipc        = L"Ipc      ";
    static const WCHAR *_WinClass   = L"WinCls   ";
    static const WCHAR *_Drive      = L"(Drive)  ";
    static const WCHAR *_Clsid      = L"Clsid    ";
    static const WCHAR *_Image      = L"Image    ";
    static const WCHAR *_FileOrKey  = L"File/Key ";
	static const WCHAR *_Other      = L"Other    ";
    static const WCHAR *_Separator  = L"   -------------------------------";

    CListBox *listbox = (CListBox *)GetDlgItem(ID_MESSAGE_LIST);
    WCHAR name[300];

    while (1) {

		ULONG seq_num = m_last_entry_seq_num;
        USHORT type;
		ULONG64 pid;
        ULONG status = SbieApi_MonitorGetEx(&seq_num, &type, &pid, &name[12]);
		if (status != 0)
			break; // error or no more entries

		if(seq_num != m_last_entry_seq_num + 1)
			SbieApi_Log(MSG_1242, NULL); // MSG_MONITOR_OVERFLOW
		m_last_entry_seq_num = seq_num;

		if ((!type) || (!name[12]))
			break;

		// privacy protection, hide username
        while (m_username_len) {
            WCHAR *username_ptr = wcsstr(&name[12], m_username);
            if (! username_ptr)
                break;
            for (ULONG i = 0; i < m_username_len; ++i)
                username_ptr[i] = L'*';
        }

        name[11] = L' ';
        name[10] = L' ';
        name[9] = L' ';
        if (type & MONITOR_OPEN) {
            name[9] = L'O';
        } else if (type & MONITOR_DENY) {
            name[9] = L'X';
        }
		type &= 0x0FFF;

        const WCHAR *PrefixPtr = _Unknown;
        if (type == MONITOR_PIPE)
            PrefixPtr = _Pipe;
        else if (type == MONITOR_IPC)
            PrefixPtr = _Ipc;
        else if (type == MONITOR_WINCLASS)
            PrefixPtr = _WinClass;
        else if (type == MONITOR_DRIVE)
            PrefixPtr = _Drive;
        else if (type == MONITOR_COMCLASS)
            PrefixPtr = _Clsid;
        else if (type == MONITOR_IMAGE)
            PrefixPtr = _Image;
        else if (type == MONITOR_FILE_OR_KEY)
            PrefixPtr = _FileOrKey;
        else if (type == MONITOR_OTHER)
            PrefixPtr = _Other;
        wcsncpy(name, PrefixPtr, 9);

		wsprintf(&name[wcslen(name)], L"; PID: %I64u", pid);

        int index = listbox->AddString(name);

        WCHAR oldname[280];
        BOOL same = FALSE;
        if (index) {
            listbox->GetText(index - 1, oldname);
            if (wcscmp(name, oldname) == 0)
                same = TRUE;
        }
        if (! same) {
            listbox->GetText(index + 1, oldname);
            if (wcscmp(name, oldname) == 0)
                same = TRUE;
        }
        if (same)
            listbox->DeleteString(index);

        if (listbox->GetCount() == 1) {

            wcscpy(name, _Clsid);
            wcscat(name, _Separator);
            listbox->AddString(name);

            wcscpy(name, _FileOrKey);
            wcscat(name, _Separator);
            listbox->AddString(name);

            wcscpy(name, _Image);
            wcscat(name, _Separator);
            listbox->AddString(name);

            wcscpy(name, _Ipc);
            wcscat(name, _Separator);
            listbox->AddString(name);

            wcscpy(name, _Pipe);
            wcscat(name, _Separator);
            listbox->AddString(name);

            wcscpy(name, _WinClass);
            wcscat(name, _Separator);
            listbox->AddString(name);
        }
    }
}


//---------------------------------------------------------------------------
// OnOK
//---------------------------------------------------------------------------


void CMonitorDialog::OnOK()
{
    CMyApp::CopyListBoxToClipboard(this, ID_MESSAGE_LIST);
    EndDialog(0);
}
