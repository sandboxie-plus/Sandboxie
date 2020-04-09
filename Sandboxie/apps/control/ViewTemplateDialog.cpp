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
// Create Sandbox Dialog Box
//---------------------------------------------------------------------------


#include "stdafx.h"
#include "MyApp.h"
#include "ViewTemplateDialog.h"

#include "SbieIni.h"


//---------------------------------------------------------------------------
// Message Map
//---------------------------------------------------------------------------


BEGIN_MESSAGE_MAP(CViewTemplateDialog, CBaseDialog)
END_MESSAGE_MAP()


//---------------------------------------------------------------------------
// Constructor
//---------------------------------------------------------------------------


CViewTemplateDialog::CViewTemplateDialog(
    CWnd *pParentWnd, const CString &Template)
    : CBaseDialog(pParentWnd, L"VIEW_TEMPLATE_DIALOG")
{
    m_TemplateName = Template;

    DoModal();
}


//---------------------------------------------------------------------------
// Destructor
//---------------------------------------------------------------------------


CViewTemplateDialog::~CViewTemplateDialog()
{
}


//---------------------------------------------------------------------------
// OnInitDialog
//---------------------------------------------------------------------------


BOOL CViewTemplateDialog::OnInitDialog()
{
    CString text;
    if (m_TemplateName.IsEmpty())
        text = CMyMsg(MSG_4204);
    else
        text = CMyMsg(MSG_4202);
    text.Remove(L'&');
    SetWindowText(text);

    if (m_TemplateName.IsEmpty())
        text = CMyMsg(MSG_4217);
    else
        text = CMyMsg(MSG_4216, m_TemplateName);
    GetDlgItem(ID_PAGE_LABEL_1)->SetWindowText(text);

    CEdit *pEdit = (CEdit *)GetDlgItem(IDTEXT);

    CFont font;
    if (font.CreateStockObject(ANSI_FIXED_FONT))
        pEdit->SetFont(&font);

    if (m_TemplateName.IsEmpty())
        pEdit->Clear();
    else {

        CSbieIni &ini = CSbieIni::GetInstance();

        CString SectionName = ini.m_Template_ + m_TemplateName;
        text = L"[" + SectionName + L"]\r\n";

        CStringList settings;
        ini.GetSettingsNames(SectionName, settings);

        while (! settings.IsEmpty()) {
            CString setting = settings.RemoveHead();

            CStringList values;
            ini.GetTextList(SectionName, setting, values);

            while (! values.IsEmpty())
                text += setting + L"=" + values.RemoveHead() + L"\r\n";
        }

        pEdit->SetWindowText(text);
        pEdit->SetReadOnly(TRUE);
    }

    GetDlgItem(IDOK)->SetWindowText(CMyMsg(MSG_3001));
    GetDlgItem(IDCANCEL)->SetWindowText(CMyMsg(MSG_3002));

    return TRUE;
}


//---------------------------------------------------------------------------
// OnOK
//---------------------------------------------------------------------------


void CViewTemplateDialog::OnOK()
{
    if (m_TemplateName.IsEmpty()) {

        if (! VerifyAndAddTemplate()) {
            CMyApp::MsgBox(this, MSG_4220, MB_OK);
            return;
        }
    }

    EndDialog(0);
}


//---------------------------------------------------------------------------
// VerifyAndAddTemplate
//---------------------------------------------------------------------------


BOOL CViewTemplateDialog::VerifyAndAddTemplate()
{
    CEdit *pEdit = (CEdit *)GetDlgItem(IDTEXT);
    CString text;
    pEdit->GetWindowText(text);

    //
    // break text into separate lines
    //

    int index;
    CString line;

    CStringList lines;
    while (! text.IsEmpty()) {

        index = text.Find(L'\r');
        if (index == -1)
            index = text.Find(L'\n');
        if (index == -1)
            index = text.GetLength();

        line = text.Left(index);
        line.TrimLeft();
        line.TrimRight();
        if (! line.IsEmpty())
            lines.AddTail(line);

        ++index;
        if (index > text.GetLength())
            text = CString();
        else
            text = text.Mid(index);
    }

    if (lines.IsEmpty())
        return FALSE;

    //
    // first line must be [Template_Local_Xxx]
    //

    CSbieIni &ini = CSbieIni::GetInstance();

    CString section = lines.RemoveHead();
    if (section.GetAt(0) != L'[')
        return FALSE;
    section = section.Mid(1);
    if (section.Left(9).CompareNoCase(ini.m_Template_) != 0)
        return FALSE;
    section = section.Mid(9);
    index = section.Find(L']');
    if (index == -1)
        return FALSE;
    section = section.Left(index);
    const CString _Local_(L"Local_");
    if (section.Left(6).CompareNoCase(_Local_) != 0)
        section = _Local_ + section;
    section = ini.m_Template_ + section;

    //
    // every other line must be Setting=Value
    //

    POSITION pos = lines.GetHeadPosition();
    while (pos) {
        line = lines.GetNext(pos);
        index = line.Find(L'=');
        if (index < 1 || index + 1 >= line.GetLength())
            return FALSE;
    }

    //
    // if we're still here, remove the old template
    //

    while (1) {

        CStringList names;
        ini.GetSettingsNames(section, names);

        if (names.IsEmpty())
            break;

        while (! names.IsEmpty()) {
            CString &name = names.RemoveHead();
            ini.SetText(section, name, CString());
        }
    }

    //
    // if we're still here, add the new template
    //

    const CString TmplTitle(L"Tmpl.Title");
    const CString TmplClass(L"Tmpl.Class");
    BOOL TitleFound = FALSE;

    pos = lines.GetHeadPosition();
    while (pos) {
        line = lines.GetNext(pos);
        index = line.Find(L'=');
        if (index < 1 || index + 1 >= line.GetLength())
            return FALSE;

        CString setting = line.Left(index);
        setting.TrimLeft();
        setting.TrimRight();

        CString value = line.Mid(index + 1);
        value.TrimLeft();
        value.TrimRight();

        if (setting.CompareNoCase(TmplClass) == 0)
            continue;

        if (setting.CompareNoCase(TmplTitle) == 0)
            TitleFound = TRUE;

        ini.InsertText(section, setting, value);
    }

    if (! TitleFound)
        ini.SetText(section, TmplTitle, section);
    ini.SetText(section, TmplClass, L"Local");

    return TRUE;
}
