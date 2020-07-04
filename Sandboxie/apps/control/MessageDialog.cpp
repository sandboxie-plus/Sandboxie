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
// Popup Messages Dialog Box
//---------------------------------------------------------------------------

#include "stdafx.h"
#include "MyApp.h"
#include "MessageDialog.h"

#include "UserSettings.h"
#include "Boxes.h"
#include "apps/common/RunBrowser.h"
#include "common/win32_ntddk.h"
#include "common/my_version.h"
#include "core/drv/api_defs.h"


//---------------------------------------------------------------------------
// Structures and Types
//---------------------------------------------------------------------------


struct MsgEntry {

    ULONG code;
    WCHAR *str2;
    WCHAR str1[1];

};


struct HideEntry {

    ULONG code;
    CString detail;

};


//---------------------------------------------------------------------------
// Message Map
//---------------------------------------------------------------------------


BEGIN_MESSAGE_MAP(CMessageDialog, CBaseDialog)

    ON_COMMAND(ID_MESSAGE_HELP,     OnHelp)
    ON_COMMAND(ID_MESSAGE_HIDE,     OnHide)
    ON_COMMAND(ID_MESSAGE_COPY,     OnCopy)
    ON_CONTROL(LBN_DBLCLK, ID_MESSAGE_LIST, OnListDoubleClick)

END_MESSAGE_MAP()


//---------------------------------------------------------------------------
// Variables
//---------------------------------------------------------------------------


CWnd *CMessageDialog::m_GettingStartedWindow = NULL;


//---------------------------------------------------------------------------
// Constructor
//---------------------------------------------------------------------------


CMessageDialog::CMessageDialog(CWnd *pParentWnd, int mode)
    : CBaseDialog(pParentWnd, L"MESSAGE_DIALOG")
{
    //m_pParentWnd = pParentWnd;

    m_firsttime = TRUE;
    m_visible = FALSE;
    m_qr = NULL;
    m_can_hide = (mode == MSGDLG_NORMAL) ? TRUE : FALSE;

    m_buf_len = (8 * 1024);
    m_buf = malloc_WCHAR(m_buf_len);

	m_last_message_number = 0;

    if (mode == MSGDLG_NORMAL)
        ReloadConf();

    //
    // init dialog
    //

    if (mode == MSGDLG_EVENT_VIEWER)
        ReadEventLog();
}


//---------------------------------------------------------------------------
// Destructor
//---------------------------------------------------------------------------


CMessageDialog::~CMessageDialog()
{
    if (m_visible)
        EndDialog(0);

    if (m_qr)
        delete m_qr;

    if (m_buf)
        free(m_buf);
}


//---------------------------------------------------------------------------
// ReloadConf
//---------------------------------------------------------------------------


void CMessageDialog::ReloadConf()
{
    HideEntry *entry;

    //
    // free existing entries
    //

    while (m_hidden.GetSize()) {
        entry = (HideEntry *)m_hidden.GetAt(0);
        delete entry;
        m_hidden.RemoveAt(0);
    }

    //
    // rebuild list
    //

    CStringList list;
    CUserSettings::GetInstance().GetTextList(_HideMessage, list);
    while (! list.IsEmpty()) {

        CString head = list.RemoveHead();

        entry = new HideEntry();

        if (head == L"*") {

            entry->code = -1;

        } else if (head.Mid(0, 4).CompareNoCase(SBIE_IN_MSGS) == 0 ||
                   head.Mid(0, 4).CompareNoCase(L"SBIE") == 0 ||
                   head.Mid(0, 4).CompareNoCase(L"SBOX") == 0) {

            entry->code = _wtoi(head.Mid(4));

        } else {

            entry->code = _wtoi(head);
        }

        int pos = head.Find(L',');
        if (pos != -1)
            entry->detail = head.Mid(pos + 1);

        if (entry->code)
            m_hidden.Add(entry);
        else
            delete entry;
    }
}


//---------------------------------------------------------------------------
// IsHiddenMessage
//---------------------------------------------------------------------------


BOOL CMessageDialog::IsHiddenMessage(
    ULONG code, const WCHAR *detail_1, const WCHAR *detail_2)
{
    code = code & 0xFFFF;

    //
    // hack, always hide the combination,
    // SBIE1406 (Bad) expansion for {374DE290-123F-4565-9164-39C4925E467B}
    // SBIE1412 In text: [SandboxName] RecoverFolder=...
    //

    if (code == 1406 && 0 == wcscmp(detail_2,
            L"{374DE290-123F-4565-9164-39C4925E467B}"))
        return TRUE;

    if (code == 1412 && wcsstr(detail_1,
            L"RecoverFolder=%{374DE290-123F-4565-9164-39C4925E467B}%"))
        return TRUE;

    //
    // check the HideMessage settings to see if the message should be hidden
    //

    int i = (int)m_hidden.GetSize();
    while (i > 0) {
        HideEntry *entry = (HideEntry *)m_hidden.GetAt(i - 1);
        if (entry->code == -1)
            return TRUE;
        if (entry->code == code) {
            if (entry->code == MSG_1319)    // hide MSG_1319 for all detail
                return TRUE;
            BOOL match = (! entry->detail.GetLength()) ||
                         (entry->detail.CompareNoCase(detail_1) == 0);
            if (match)
                return TRUE;
        }
        --i;
    }

    //
    // otherwise, don't hide message
    //

    return FALSE;
}


//---------------------------------------------------------------------------
// OnTimer
//---------------------------------------------------------------------------


void CMessageDialog::OnTimer()
{
    //
    // extract messages from SbieDrv and record them
    //

    bool seen_2218 = false;

    while (1) {

        ULONG len = m_buf_len;
		ULONG message_number = m_last_message_number;
		ULONG code = -1;
		ULONG pid = 0;
        LONG status = SbieApi_GetMessage(&message_number, CMyApp::m_session_id, &code, &pid, m_buf, len);
		if (status != 0)
			break; // error or no more entries

		//if (message_number != m_last_message_number + 1)
		//	we missed something
		m_last_message_number = message_number;

		if (code == 0)
			continue; // empty dummy

        if (/*code == MSG_2199 &&*/ m_firsttime)
            continue;

        WCHAR *str1 = m_buf;
        ULONG str1_len = wcslen(str1);
        WCHAR *str2 = str1 + str1_len + 1;
        ULONG str2_len = wcslen(str2);

        if (IsHiddenMessage(code, str1, str2))
            continue;

        len = sizeof(MsgEntry)
            + (str1_len + 4 + str2_len + 4) * sizeof(WCHAR);
        MsgEntry *msg = (MsgEntry *)new UCHAR[len];

        if (msg) {

            msg->code = code;
            wmemcpy(msg->str1, str1, str1_len + 1);
            msg->str2 = msg->str1 + str1_len + 1;
            wmemcpy(msg->str2, str2, str2_len + 1);

            m_queue.AddTail(msg);

            //
            // if we're displaying the Blocked spooler print to file message 1319
            // then also display the follow up message 1320
            //

            if (msg->code == MSG_1319) {

                MsgEntry *msg = (MsgEntry *)new UCHAR[len];
                msg->code = MSG_1320;
                wmemcpy(msg->str1, str1, str1_len + 1);
                msg->str2 = msg->str1 + str1_len + 1;
                wmemcpy(msg->str2, str2, str2_len + 1);

                m_queue.AddTail(msg);
            }


            //
            // if we're displaying the Drop Rights message 2219
            // then also display the follow up message 2220
            //

            if (msg->code == MSG_2218)
                seen_2218 = true;

            if (msg->code == MSG_2219 && (! seen_2218)) {

                MsgEntry *msg = (MsgEntry *)new UCHAR[len];
                msg->code = MSG_2220;
                wmemcpy(msg->str1, str1, str1_len + 1);
                msg->str2 = msg->str1 + str1_len + 1;
                wmemcpy(msg->str2, str2, str2_len + 1);

                m_queue.AddTail(msg);
            }

            //
            // if we're displaying SBIE1307 or SBIE1308 then also
            // display message SBIE2221 or SBIE2222 respectively
            //

            if (msg->code == MSG_1307 || msg->code == MSG_1308) {

                BOOLEAN image_copy = FALSE;
                WCHAR *asterisk = wcsrchr(msg->str1, L'*');
                if (asterisk && asterisk[1] == L'\0')
                    image_copy = TRUE;
                if (! image_copy) {

                    ULONG new_code =
                        (msg->code == MSG_1307) ? MSG_2221 : MSG_2222;
                    MsgEntry *msg = (MsgEntry *)new UCHAR[len];
                    msg->code = new_code;
                    wmemcpy(msg->str1, str1, str1_len + 1);
                    msg->str2 = msg->str1 + str1_len + 1;
                    wmemcpy(msg->str2, str2, str2_len + 1);

                    m_queue.AddTail(msg);

                    CString exe(str1);
                    int index = exe.Find(L'[');
                    if (index >= 2)
                        CBoxProc::AddToRecentExes(exe.Left(index - 1));
                }
            }

            //
            // if we're displaying SBIE2102 then also display SBIE2223
            //

            if (msg->code == MSG_2102) {

                MsgEntry *msg = (MsgEntry *)new UCHAR[len];
                msg->code = MSG_2223;
                wmemcpy(msg->str1, str1, str1_len + 1);
                msg->str2 = msg->str1 + str1_len + 1;
                wmemcpy(msg->str2, str2, str2_len + 1);

                m_queue.AddTail(msg);
            }
        }
    }

    m_firsttime = FALSE;

    if (! m_queue.IsEmpty() && (! m_qr))
        DoPopups();

    if (! m_queue.IsEmpty() && (! m_visible))
        DoRecovery();
}


//---------------------------------------------------------------------------
// DoPopups
//---------------------------------------------------------------------------


void CMessageDialog::DoPopups()
{
    MsgEntry *msg = (MsgEntry *)m_queue.GetHead();

    //
    // if the message dialog is not visible, then the first entry
    // in the queue must be a message, otherwise we don't do anything
    //

    if (! m_visible) {

        if (msg->code != MSG_2199) {
            m_visible = TRUE;
            MyDoModal(this);
            m_visible = FALSE;
        }

        return;
    }

    //
    // if the message dialog is visible, add all waiting messages
    //

    POSITION pos = m_queue.GetHeadPosition();
    m_queue.GetNext(pos);
    while (msg) {

        if (msg->code != MSG_2199) {

            CMyMsg text(msg->code, msg->str1, msg->str2);
            int index = m_listbox->AddString(text);
            m_listbox->SetItemDataPtr(index, new CString(msg->str1));

            m_queue.RemoveHead();
            delete msg;
            pos = m_queue.GetHeadPosition();
        }

        if (pos)
            msg = (MsgEntry *)m_queue.GetNext(pos);
        else
            msg = NULL;
    }
}


//---------------------------------------------------------------------------
// MyDoModal
//---------------------------------------------------------------------------


void CMessageDialog::MyDoModal(CDialog *pDialog)
{
    CWnd *pMainWnd = AfxGetMainWnd();

    BOOL sendback = TRUE;
    if (GetForegroundWindow() == pMainWnd)
        sendback = FALSE;

    m_pParentWnd = GetInputWindow();
    pDialog->DoModal();

    if (sendback && pMainWnd) {
        pMainWnd->SetWindowPos(
            &wndBottom, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
    }
}


//---------------------------------------------------------------------------
// OnInitDialog
//---------------------------------------------------------------------------


BOOL CMessageDialog::OnInitDialog()
{
    if (! m_qr)
        m_HelpButton.Init(this, ID_MESSAGE_HELP);

    SetWindowText(CMyMsg(MSG_3645));

    GetDlgItem(ID_MESSAGE_HELP)->SetWindowText(CMyMsg(MSG_3451));

    if (m_can_hide) {
        GetDlgItem(ID_MESSAGE_HIDE)->SetWindowText(CMyMsg(MSG_3646));
        GetDlgItem(ID_MESSAGE_HIDE)->ShowWindow(SW_SHOW);
    }

    GetDlgItem(IDOK)->SetWindowText(CMyMsg(MSG_3004));

    GetDlgItem(ID_MESSAGE_COPY)->SetWindowText(CMyMsg(MSG_3657));

    SetWindowPos(&wndTopMost, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

    m_listbox = (CListBox *)GetDlgItem(ID_MESSAGE_LIST);
    m_listbox->SetItemHeight(0, m_listbox->GetItemHeight(0) * 2);

    do {

        MsgEntry *msg = (MsgEntry *)m_queue.RemoveHead();

        CMyMsg text(msg->code, msg->str1, msg->str2);
        int index = m_listbox->AddString(text);
        m_listbox->SetItemDataPtr(index, new CString(msg->str1));
        m_listbox->SetCurSel(index);

        delete msg;

    } while (! m_queue.IsEmpty());

    // Find the longest string in the list box.
    CString    str;
    CSize      sz;
    int        dx = 0;
    TEXTMETRIC tm;
    CDC*       pDC = m_listbox->GetDC();
    CFont*     pFont = m_listbox->GetFont();

    // Select the listbox font, save the old font
    CFont* pOldFont = pDC->SelectObject(pFont);
    // Get the text metrics for avg char width
    pDC->GetTextMetrics(&tm);

    for (int i = 0; i < m_listbox->GetCount(); i++)
    {
        m_listbox->GetText(i, str);
        sz = pDC->GetTextExtent(str);

        // Add the avg width to prevent clipping
        sz.cx += tm.tmAveCharWidth;

        if (sz.cx > dx)
            dx = sz.cx;
    }
    // Select the old font back into the DC
    pDC->SelectObject(pOldFont);
    m_listbox->ReleaseDC(pDC);

    // Set the horizontal extent so every character of all strings  
    // can be scrolled to.
    m_listbox->SetHorizontalExtent(dx);

    AddMinimizeButton();

    return TRUE;
}


//---------------------------------------------------------------------------
// GetSBIExxxx
//---------------------------------------------------------------------------


CString CMessageDialog::GetSBIExxxx(ULONG *pCode, CString *pDetail)
{
    CString sbie;

    int index = m_listbox->GetCurSel();
    if (index != LB_ERR) {

        CString text;
        m_listbox->GetText(index, text);
        int pos = text.Find(SBIE_IN_MSGS);
        if (pos != -1) {

            ULONG code = _wtoi(text.Mid(pos + 4, 4));
            if (pCode)
                *pCode = code;
            if (code)
                sbie.Format(SBIE_IN_MSGS L"%04d", code);

            if (pDetail) {
                CString *p_detail =
                    (CString *)m_listbox->GetItemDataPtr(index);
                *pDetail = *p_detail;
            }
        }
    }

    return sbie;
}


//---------------------------------------------------------------------------
// DiscardMessages
//---------------------------------------------------------------------------


void CMessageDialog::DiscardMessages(
    const CString &sbie, const CString &detail)
{
    while (1) {

        int index;
        int size = m_listbox->GetCount();

        for (index = 0; index < size; ++index) {

            CString text;
            m_listbox->GetText(index, text);
            if (text.Find(sbie) != -1) {

                CString *p_detail =
                    (CString *)m_listbox->GetItemDataPtr(index);

                BOOL match = (detail.IsEmpty() || p_detail->IsEmpty()) ||
                             (p_detail->CompareNoCase(detail) == 0);
                if (match) {
                    m_listbox->DeleteString(index);
                    break;
                }
            }
        }

        if (index == size)
            break;
    }
}


//---------------------------------------------------------------------------
// OnHelp
//---------------------------------------------------------------------------


void CMessageDialog::OnHelp()
{
    CString sbie = GetSBIExxxx(NULL, NULL);
    if (! sbie.IsEmpty())
        CRunBrowser::OpenHelp(this, sbie);
}


//---------------------------------------------------------------------------
// OnHide
//---------------------------------------------------------------------------


void CMessageDialog::OnHide()
{
    ULONG code;
    CString detail;
    CString sbie = GetSBIExxxx(&code, &detail);
    if (sbie.IsEmpty())
        return;

    //
    // confirm hiding
    //

    CMyMsg msg(MSG_3647, sbie);
    int rc = CMyApp::MsgBox(this, msg, MB_YESNO);
    if (rc != IDYES)
        return;

    CString text;
    text.Format(L"%04d%c%s", code,
                             (detail.IsEmpty() ? L'\0' : L','), detail);
    CUserSettings::GetInstance().InsertText(_HideMessage, text);

    ReloadConf();

    DiscardMessages(sbie, detail);
}


//---------------------------------------------------------------------------
// OnCopy
//---------------------------------------------------------------------------


void CMessageDialog::OnCopy()
{
    CMyApp::CopyListBoxToClipboard(this, ID_MESSAGE_LIST);
    EndDialog(0);
}


//---------------------------------------------------------------------------
// OnListDoubleClick
//---------------------------------------------------------------------------


void CMessageDialog::OnListDoubleClick()
{
    ULONG code;
    CString detail;
    CString sbie = GetSBIExxxx(&code, &detail);
    if (sbie.IsEmpty())
        return;

    //
    // handle SBIE1320
    //

    if (code == 1320) {

        int pos1 = detail.Find(L'[');
        if (pos1 != -1) {
            ++pos1;
            int pos2 = detail.Find(L']', pos1);
            if (pos2 != -1) {
                CString sPid = detail.Mid(pos1, pos2 - pos1);
                DWORD process_id = _wtoi(sPid.GetString());

                // tell driver to allow spooler print to file for this process
                LONG rc = SbieApi_CallOne(API_ALLOW_SPOOLER_PRINT_TO_FILE, (ULONG_PTR)(process_id));
                sbie.Format(SBIE_IN_MSGS L"%04d", 1319);
                DiscardMessages(sbie, detail);
                sbie.Format(SBIE_IN_MSGS L"%04d", 1320);
                DiscardMessages(sbie, detail);
            }
        }
    }

    //
    // handle SBIE2220
    //

    if (code == 2220) {

        int pos1 = detail.Find(L'[');
        if (pos1 != -1) {
            ++pos1;
            int pos2 = detail.Find(L']', pos1);
            if (pos2 != -1) {
                CString boxname = detail.Mid(pos1, pos2 - pos1);

                CMyMsg msg(MSG_3949, boxname);
                int rc = CMyApp::MsgBox(this, msg, MB_OKCANCEL);
                if (rc != IDOK)
                    return;

                CBox &box = CBoxes::GetInstance().GetBox(boxname);
                if (! box.GetName().IsEmpty()) {

                    ShowWindow(SW_HIDE);
                    BOOL ok = box.GetSetDropAdminRights(TRUE, FALSE);
                    ShowWindow(SW_SHOW);
                    if (ok) {

                        detail = CString();
                        sbie.Format(SBIE_IN_MSGS L"%04d", 2214);
                        DiscardMessages(sbie, detail);
                        sbie.Format(SBIE_IN_MSGS L"%04d", 2217);
                        DiscardMessages(sbie, detail);
                        sbie.Format(SBIE_IN_MSGS L"%04d", 2219);
                        DiscardMessages(sbie, detail);
                        sbie.Format(SBIE_IN_MSGS L"%04d", 2220);
                        DiscardMessages(sbie, detail);
                    }
                }
            }
        }
    }

    //
    // handle SBIE2221 and SBIE2222
    //

    if (code == 2221 || code == 2222) {

        int pos1 = detail.Find(L'[');
        if (pos1 != -1) {
            ++pos1;
            int pos2 = detail.Find(L']', pos1);
            if (pos2 != -1) {
                CString boxname = detail.Mid(pos1, pos2 - pos1);
                CString pgmname = detail.Mid(0, pos1 - 1);
                pgmname.TrimLeft();
                pgmname.TrimRight();

                CMyMsg msg((code == 2221 ? MSG_3920 : MSG_3930),
                           boxname, pgmname);
                int rc = CMyApp::MsgBox(this, msg, MB_OKCANCEL);
                if (rc != IDOK)
                    return;

                CBox &box = CBoxes::GetInstance().GetBox(boxname);
                if (! box.GetName().IsEmpty()) {

                    BOOL ok;
                    ShowWindow(SW_HIDE);
                    if (code == 2221) {
                        CString pgms = box.GetInternetProcess(NULL);
                        pgms += L"," + pgmname;
                        ok = box.SetInternetProcess(pgms, NULL);
                    } else {
                        CString pgms = box.GetStartRunProcess(NULL);
                        pgms += L"," + pgmname;
                        ok = box.SetStartRunProcess(pgms, NULL);
                    }
                    ShowWindow(SW_SHOW);
                    if (ok) {

                        ULONG code2 = (code == 2221) ? 1307 : 1308;
                        detail = CString();
                        sbie.Format(SBIE_IN_MSGS L"%04d", code2);
                        DiscardMessages(sbie, detail);
                        sbie.Format(SBIE_IN_MSGS L"%04d", code);
                        DiscardMessages(sbie, detail);
                    }
                }
            }
        }
    }

    //
    // handle SBIE2223
    //

    if (code == 2223) {

        int pos1 = detail.Find(L'[');
        if (pos1 != -1) {
            ++pos1;
            int pos2 = detail.Find(L'/', pos1);
            if (pos2 != -1) {
                int pos3 = detail.Find(L']', pos2);
                if (pos3 != -1) {
                    CString boxname = detail.Mid(pos1, pos2 - pos1);
                    CString filesize = detail.Mid(pos2 + 1, pos3 - pos2 - 1);
                    boxname.TrimLeft();
                    boxname.TrimRight();
                    filesize.TrimLeft();
                    filesize.TrimRight();

                    CBox &box = CBoxes::GetInstance().GetBox(boxname);
                    if (! box.GetName().IsEmpty()) {

                        ULONG limitOld = box.GetSetCopyLimit(FALSE);
                        if (! limitOld)
                            limitOld = 1;
                        ULONG limitNew = _wtoi(filesize);
                        limitNew = (limitNew / 1024) + 2;
                        if (limitNew <= limitOld)
                            limitNew = limitOld + 2;
                        CString limitOldTxt;
                        limitOldTxt.Format(L"%d", limitOld);
                        CString limitNewTxt;
                        limitNewTxt.Format(L"%d", limitNew);

                        CMyMsg msg(
                            MSG_3909, boxname, limitOldTxt, limitNewTxt);
                        int rc = CMyApp::MsgBox(this, msg, MB_OKCANCEL);
                        if (rc != IDOK)
                            return;

                        ShowWindow(SW_HIDE);
                        BOOL ok = box.GetSetCopyLimit(TRUE, limitNew);
                        ShowWindow(SW_SHOW);
                        if (ok) {

                            detail = CString();
                            sbie.Format(SBIE_IN_MSGS L"%04d", 2102);
                            DiscardMessages(sbie, detail);
                            sbie.Format(SBIE_IN_MSGS L"%04d", 2223);
                            DiscardMessages(sbie, detail);
                        }
                    }
                }
            }
        }
    }
}


//---------------------------------------------------------------------------
// DoRecovery
//---------------------------------------------------------------------------


void CMessageDialog::DoRecovery()
{
    MsgEntry *msg = (MsgEntry *)m_queue.GetHead();

    //
    // if the quick recovery dialog is not visible, then the first entry
    // in the queue must be for recovery, otherwise we don't do anything
    //

    if (! m_qr) {

        if (msg->code != MSG_2199)
            return;

        msg = (MsgEntry *)m_queue.RemoveHead();

        WCHAR *space = wcschr(msg->str1, L' ');
        if (! space) {
            delete msg;
            return;
        }
        *space = L'\0';
        ++space;

        CBox &box = CBoxes::GetInstance().GetBox(msg->str1);

        if (box.GetName().IsEmpty()) {
            delete msg;
            return;
        }

        if (wcsncmp(space, L"*AUTOPLAY*", 10) == 0) {
            DoAutoPlay(box, space[10]);
            delete msg;
            return;
        }

        if (! box.GetImmediateRecoveryState()) {
            delete msg;
            return;
        }

        m_qr_box = box.GetName();
        m_qr = new CQuickRecover(m_pParentWnd, m_qr_box, space, QR_AUTO);
        MyDoModal(m_qr);

        delete m_qr;
        m_qr = NULL;
        m_qr_box = CString();
        delete msg;

        if (m_GettingStartedWindow) {
            m_GettingStartedWindow->PostMessage(
                WM_COMMAND, ID_SANDBOX_MENU | ID_SANDBOX_RECOVER);
        }

        return;
    }

    //
    // if the recovery dialog is visible, add all waiting recoveries
    // that belong to the same box
    //

    POSITION pos = m_queue.GetHeadPosition();
    POSITION lastpos = pos;
    m_queue.GetNext(pos);
    while (msg) {

        while (msg->code == MSG_2199) {

            WCHAR *space = wcschr(msg->str1, L' ');
            if (! space) {
                m_queue.RemoveHead();
                delete msg;
                break;
            }
            *space = L'\0';

            CBox &box = CBoxes::GetInstance().GetBox(msg->str1);

            if (box.GetName() != m_qr_box) {
                *space = L' ';
                break;
            }

            if (box.GetImmediateRecoveryState())
                m_qr->AddAutoRecoverItem(space + 1);

            m_queue.RemoveAt(lastpos);
            delete msg;
            pos = m_queue.GetHeadPosition();

            break;
        }

        if (pos) {
            lastpos = pos;
            msg = (MsgEntry *)m_queue.GetNext(pos);
        } else
            msg = NULL;
    }
}


//---------------------------------------------------------------------------
// DoAutoPlay
//---------------------------------------------------------------------------


void CMessageDialog::DoAutoPlay(const CBox &box, const WCHAR drive)
{
    WCHAR cmd[4];
    cmd[0] = drive;
    cmd[1] = L':';
    cmd[2] = L'\\';
    cmd[3] = L'\0';
    CMyApp::RunStartExe(cmd, box.GetName(), FALSE);
}


//---------------------------------------------------------------------------
// ReadStartError
//---------------------------------------------------------------------------


void CMessageDialog::ReadStartError()
{
    const WCHAR *StartError = SbieDll_GetStartError();
    if (! StartError)
        return;

    ULONG len = sizeof(MsgEntry) + (wcslen(StartError) + 1) * sizeof(WCHAR);
    MsgEntry *msg = (MsgEntry *)new UCHAR[len];

    if (msg) {

        msg->code = MSG_2331;
        wcscpy(msg->str1, StartError);
        msg->str2 = msg->str1 + wcslen(msg->str1);
        msg->str2[0] = L'\0';

        m_queue.AddTail(msg);
    }
}


//---------------------------------------------------------------------------
// ReadEventLog
//---------------------------------------------------------------------------


void CMessageDialog::ReadEventLog()
{
    HANDLE hEventLog = OpenEventLog(NULL, L"System");

    while (hEventLog) {

        time_t recently = time(NULL) - 15 * 60;

        ULONG bytesRead, bytesNeeded;
        BOOL ok = ::ReadEventLog(
            hEventLog, EVENTLOG_SEQUENTIAL_READ | EVENTLOG_FORWARDS_READ,
            0, m_buf, m_buf_len, &bytesRead, &bytesNeeded);

        if (! ok)
            break;

        EVENTLOGRECORD *rec = (EVENTLOGRECORD *)m_buf;
        while (bytesRead > 0) {

            WCHAR *source = (WCHAR *)
                ((UCHAR *)rec + sizeof(EVENTLOGRECORD));
            if ((time_t)rec->TimeGenerated >= recently &&
                    (wcscmp(source, SBIEDRV) == 0 ||
                     wcscmp(source, SBIESVC) == 0)) {

                ULONG len = sizeof(MsgEntry)
                          + rec->Length;
                MsgEntry *msg = (MsgEntry *)new UCHAR[len];
                if (msg) {

                    msg->code = rec->EventID;
                    msg->str1[0] = L'\0';
                    msg->str2 = msg->str1;

                    if (rec->NumStrings >= 2) {

                        WCHAR *str0 =
                            (WCHAR *)((UCHAR *)rec + rec->StringOffset);
                        WCHAR *str1 = str0 + wcslen(str0) + 1;
                        wcscpy(msg->str1, str1);
                        msg->str2 = msg->str1 + wcslen(msg->str1);

                        if (rec->NumStrings >= 3) {

                            WCHAR *str2 = str1 + wcslen(str1) + 1;
                            ++msg->str2;
                            wcscpy(msg->str2, str2);
                        }
                    }

                    m_queue.AddTail(msg);
                }
            }

            bytesRead -= rec->Length;
            rec = (EVENTLOGRECORD *)((UCHAR *)rec + rec->Length);
        }
    }

    if (hEventLog)
        CloseEventLog(hEventLog);

    ReadStartError();

    if (! m_queue.IsEmpty())
        DoPopups();
}
