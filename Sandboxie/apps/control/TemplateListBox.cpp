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
// Template List Box
//---------------------------------------------------------------------------


#include "stdafx.h"
#include "TemplateListBox.h"

#include "SbieIni.h"


//---------------------------------------------------------------------------
// Variables
//---------------------------------------------------------------------------


CString CTemplateListBox::Prefix2(L"[--]  ");
CString CTemplateListBox::Prefix1(L"[+]  ");
CString CTemplateListBox::Prefix0(L"[  ]  ");


//---------------------------------------------------------------------------
// GetTemplateClass
//---------------------------------------------------------------------------


CString CTemplateListBox::GetTemplateClass(const CString &TemplateName)
{
    return CSbieIni::GetInstance().GetTemplateMetaData(
                                                    TemplateName, L"Class");
}


//---------------------------------------------------------------------------
// GetTemplateTitle
//---------------------------------------------------------------------------


CString CTemplateListBox::GetTemplateTitle(const CString &TemplateName)
{
    CString descr =
        CSbieIni::GetInstance().GetTemplateMetaData(TemplateName, L"Title");

    if (descr.GetAt(0) == L'#') {
        CString detail;
        int index = descr.Find(L',');
        if (index != -1) {
            detail = descr.Mid(index + 1);
            descr = descr.Left(index);
        }
        descr = CMyMsg(_wtoi(descr.Mid(1)), detail);
    }

    if (descr.IsEmpty())
        descr = L"-";

    return descr;
}


//---------------------------------------------------------------------------
// Decorate
//---------------------------------------------------------------------------


void CTemplateListBox::Decorate(CString &text, BOOL enable, BOOL force)
{
    if (! force) {
        int index = text.Find(L']');
        text = text.Mid(index + 3);
    }
    if (enable)
        text = Prefix1 + text;
    else
        text = Prefix0 + text;
}


//---------------------------------------------------------------------------
// DecorateAster
//---------------------------------------------------------------------------


void CTemplateListBox::DecorateAster(CString &text)
{
    text = Prefix2 + text;
}


//---------------------------------------------------------------------------
// IsAster
//---------------------------------------------------------------------------


BOOL CTemplateListBox::IsAster(const CString &text)
{
    return (text.Left(Prefix2.GetLength()) == Prefix2);
}


//---------------------------------------------------------------------------
// IsCheck
//---------------------------------------------------------------------------


BOOL CTemplateListBox::IsCheck(const CString &text)
{
    return (text.Left(Prefix1.GetLength()) == Prefix1);
}


//---------------------------------------------------------------------------
// IsClear
//---------------------------------------------------------------------------


BOOL CTemplateListBox::IsClear(const CString &text)
{
    return (text.Left(Prefix0.GetLength()) == Prefix0);
}


//---------------------------------------------------------------------------
// OnAddRemove
//---------------------------------------------------------------------------


BOOL CTemplateListBox::OnAddRemove(CWnd *wnd, BOOL enable, BOOL toggle)
{
    CListBox *pListBox = (CListBox *)wnd->GetDlgItem(ID_APP_TEMPLATE_LIST);
    int index = pListBox->GetCurSel();
    if (index != LB_ERR) {
        CString text;
        pListBox->GetText(index, text);
        if (! text.IsEmpty()) {

            if (IsAster(text)) {
                CMyApp::MsgBox(wnd->GetParent(), MSG_4256, MB_OK);
                return FALSE;
            }

            if (toggle) {
                if (IsCheck(text))
                    enable = FALSE;
                else if (IsClear(text))
                    enable = TRUE;
                else
                    return FALSE;
            }

            Decorate(text, enable, FALSE);
            void *data = pListBox->GetItemDataPtr(index);
            pListBox->DeleteString(index);
            pListBox->InsertString(index, text);
            if (data)
                pListBox->SetItemDataPtr(index, data);
            pListBox->SetCurSel(index);

            return TRUE;
        }
    }

    return FALSE;
}
