/*
 * Copyright 2004-2020 Sandboxie Holdings, LLC 
 * Copyright 2020-2021 David Xanatos, xanasoft.com
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
// Application Settings Property Page
//---------------------------------------------------------------------------

#include "stdafx.h"
#include "MyApp.h"
#include "AppPage.h"

#include "SbieIni.h"
#include "UserSettings.h"
#include "TemplateListBox.h"
#include "ViewTemplateDialog.h"
#include "apps/common/RunBrowser.h"


//---------------------------------------------------------------------------
// Defines
//---------------------------------------------------------------------------


enum {
    AppPageUnknown,
    AppPageAccess,
    AppPageTemplate,
    AppPageFolders
};


#define MODIF_IEXPLORE  0x0001
#define MODIF_FIREFOX   0x0002
#define MODIF_CHROME    0x0004
#define MODIF_EDGE      0x0008
#define MODIF_OTHERWB   0x0080

#define MODIF_NONLOCAL  0x2000
#define MODIF_ADDONS    0x4000

#define MODIF_BROWSER   \
    (MODIF_IEXPLORE | MODIF_EDGE | MODIF_FIREFOX | MODIF_CHROME | MODIF_OTHERWB)


//---------------------------------------------------------------------------
// Message Map
//---------------------------------------------------------------------------


BEGIN_MESSAGE_MAP(CAppPage, CBoxPage)

    ON_COMMAND(ID_APP_SCREEN_READER,            OnModified)

    ON_COMMAND(ID_APP_TEMPLATE_ADD,             Template_OnAdd)
    ON_COMMAND(ID_APP_TEMPLATE_REMOVE,          Template_OnRemove)
    ON_CONTROL(LBN_DBLCLK, ID_APP_TEMPLATE_LIST, Template_OnToggle)
    ON_COMMAND(ID_APP_TEMPLATE_VIEW,            Template_OnView)
    ON_COMMAND(ID_APP_TEMPLATE_LINK,            Template_OnLink)
    ON_COMMAND(ID_APP_TEMPLATE_CREATE,          Template_OnCreate)

    ON_CONTROL(CBN_SELENDOK, ID_APP_FOLDERS_COMBO,  Folders_OnSelect)
    ON_COMMAND(ID_APP_FOLDERS_ADD,              Folders_OnAdd)
    ON_COMMAND(ID_APP_FOLDERS_REMOVE,           Folders_OnRemove)

    ON_WM_MEASUREITEM()
    ON_WM_DRAWITEM()

END_MESSAGE_MAP()


//---------------------------------------------------------------------------
// Variables
//---------------------------------------------------------------------------


static CString Template_ScreenReader(L"ScreenReader");


CObList CAppPage::m_app_pages;


//---------------------------------------------------------------------------
// Constructor
//---------------------------------------------------------------------------


CAppPage::CAppPage(ULONG type, const CString &BoxName, ULONG TitleMsgId)
    : CBoxPage(0, BoxName, MSG_4201, TitleMsgId)
{
    m_app = type;

    //
    // construct template for specified type
    //

    const WCHAR *template_base = NULL;
    if (m_app == AppPageAccess)
        template_base = L"APP_ACCESS_PAGE";
    if (m_app == AppPageFolders)
        template_base = L"APP_FOLDERS_PAGE";

    SetPageTemplate(template_base);

    SetTitleForTree();
}


//---------------------------------------------------------------------------
// Constructor
//---------------------------------------------------------------------------


CAppPage::CAppPage(TMPL_INFO *info, const CString &BoxName)
    : CBoxPage(0, BoxName, 0, 0)
{
    m_app = AppPageTemplate;

    m_tmpl_info.ClassName = info->ClassName;
    m_tmpl_info.ClassModifier = info->ClassModifier;
    m_tmpl_info.TitleId = info->TitleId;
    m_tmpl_info.LabelId = info->LabelId;
    m_tmpl_info.WithLink = info->WithLink;
    m_tmpl_info.WithCreate = info->WithCreate;

    m_tmpl_init_list_box = TRUE;

    SetPageTemplate(L"APP_TEMPLATE_PAGE");

    m_titleForPage = CMyMsg(m_tmpl_info.TitleId);
    m_titleForTree = CMyMsg(MSG_4201) + L"::" + m_titleForPage;
    if (m_tmpl_info.ClassModifier & MODIF_BROWSER) {

        if (m_tmpl_info.ClassModifier & MODIF_IEXPLORE)
            m_titleForPage = L"Internet Explorer";
        if (m_tmpl_info.ClassModifier & MODIF_EDGE)
            m_titleForPage = L"Microsoft Edge";
        if (m_tmpl_info.ClassModifier & MODIF_FIREFOX)
            m_titleForPage = L"Mozilla Firefox";
        if (m_tmpl_info.ClassModifier & MODIF_CHROME)
            m_titleForPage = L"Google Chrome";
        if (m_tmpl_info.ClassModifier & MODIF_OTHERWB)
            m_titleForPage = CMyMsg(MSG_4357);

        m_titleForTree += L"::" + m_titleForPage;
        if (m_tmpl_info.ClassModifier & MODIF_ADDONS) {
            m_titleForPage += L" " + CMyMsg(MSG_4358);
            m_titleForTree += L"::" + CMyMsg(MSG_4358);
        }
    }

    SetTitleForTree();
}


//---------------------------------------------------------------------------
// Destructor
//---------------------------------------------------------------------------


CAppPage::~CAppPage()
{
}


//---------------------------------------------------------------------------
// Plugin_OnInitDialog
//---------------------------------------------------------------------------


void CAppPage::Plugin_OnInitDialog(CBox &box)
{
    if (m_app == AppPageAccess)
        Access_OnInitDialog(box);
    if (m_app == AppPageTemplate)
        Template_OnInitDialog(box);
    if (m_app == AppPageFolders)
        Folders_OnInitDialog(box);
}


//---------------------------------------------------------------------------
// Plugin_OnOK
//---------------------------------------------------------------------------


void CAppPage::Plugin_OnOK(CBox &box)
{
    if (m_app == AppPageAccess)
        Access_OnOK(box);
    if (m_app == AppPageTemplate)
        Template_OnOK(box);
    if (m_app == AppPageFolders)
        Folders_OnOK(box);
}


//---------------------------------------------------------------------------
// Access_OnInitDialog
//---------------------------------------------------------------------------


void CAppPage::Access_OnInitDialog(CBox &box)
{
    //
    // set dialog texts
    //

    GetDlgItem(ID_PAGE_LABEL_1)->SetWindowText(CMyMsg(MSG_4303));
    GetDlgItem(ID_PAGE_LABEL_2)->SetWindowText(CMyMsg(MSG_4304));

    GetDlgItem(ID_APP_SCREEN_READER)->SetWindowText(
        CTemplateListBox::GetTemplateTitle(L"ScreenReader"));

    //
    // set up checkbox
    //

    BOOL enabled = box.IsTemplateEnabled(Template_ScreenReader);
    if (! enabled) {
        CBox &GlobalSettings = CBoxes::GetInstance().GetBox(0);
        enabled = GlobalSettings.IsTemplateEnabled(Template_ScreenReader);
    }

    CButton *check = (CButton *)GetDlgItem(ID_APP_SCREEN_READER);
    check->SetCheck(enabled ? BST_CHECKED : BST_UNCHECKED);
}


//---------------------------------------------------------------------------
// Access_OnOK
//---------------------------------------------------------------------------


void CAppPage::Access_OnOK(CBox &box)
{
    BOOL global = CBoxes::GetInstance().GetBox(0).IsTemplateEnabled(Template_ScreenReader);

    BOOL wasEnabled = box.IsTemplateEnabled(Template_ScreenReader);
    if (! wasEnabled)
        wasEnabled = global;

    CButton *check = (CButton *)GetDlgItem(ID_APP_SCREEN_READER);

    if (check->GetCheck() == BST_CHECKED) {

        if (! global)
            box.EnableTemplate(Template_ScreenReader, TRUE);

    } else if (wasEnabled) {

        if (global)
            CMyApp::MsgBox(GetParent(), MSG_4256, MB_OK);
        else
            box.EnableTemplate(Template_ScreenReader, FALSE);
    }
}


//---------------------------------------------------------------------------
// Template_OnInitDialog
//---------------------------------------------------------------------------


void CAppPage::Template_OnInitDialog(CBox &box)
{
    //
    // set dialog texts
    //

    GetDlgItem(ID_PAGE_LABEL_1)->SetWindowText(CMyMsg(m_tmpl_info.LabelId));
    GetDlgItem(ID_PAGE_LABEL_2)->SetWindowText(CMyMsg(MSG_4221));
    GetDlgItem(ID_APP_TEMPLATE_ADD)->SetWindowText(CMyMsg(MSG_3352));
    GetDlgItem(ID_APP_TEMPLATE_REMOVE)->SetWindowText(CMyMsg(MSG_3351));
    GetDlgItem(ID_APP_TEMPLATE_VIEW)->SetWindowText(CMyMsg(MSG_4202));

    if (m_tmpl_info.WithLink) {
        GetDlgItem(ID_APP_TEMPLATE_LINK)->SetWindowText(CMyMsg(MSG_4203));
        GetDlgItem(ID_APP_TEMPLATE_LINK)->ShowWindow(SW_SHOW);
    } else if (m_tmpl_info.WithCreate) {
        GetDlgItem(ID_APP_TEMPLATE_CREATE)->SetWindowText(CMyMsg(MSG_4204));
        GetDlgItem(ID_APP_TEMPLATE_CREATE)->ShowWindow(SW_SHOW);
    }

    GetDlgItem(ID_PAGE_LABEL_2)->SetFont(GetFont());

    //
    // enumerate templates
    //

    CSbieIni &ini = CSbieIni::GetInstance();
    CBox &GlobalSettings = CBoxes::GetInstance().GetBox(0);

    while (! m_tmpl_sections.IsEmpty())
        m_tmpl_sections.RemoveHead();
    ini.GetTemplateNames(m_tmpl_info.ClassName, m_tmpl_sections);

    Template_Filter();

    //
    // populate list box
    //

    CListBox *pListBox = (CListBox *)GetDlgItem(ID_APP_TEMPLATE_LIST);

    if (m_tmpl_init_list_box) {
        pListBox->SetItemHeight(0, pListBox->GetItemHeight(0) * 3 / 2);
        m_tmpl_init_list_box = FALSE;
    }

    pListBox->ResetContent();

    POSITION pos = m_tmpl_sections.GetHeadPosition();
    while (pos) {

        CString section = m_tmpl_sections.GetNext(pos);
        CString descr = CTemplateListBox::GetTemplateTitle(section);

        if (descr.Compare(L"-") == 0) {
            if (m_tmpl_info.ClassName.CompareNoCase(L"Local") != 0) {
                pListBox->AddString(CString());
                continue;
            }
            descr = section;
        }

        BOOL global = GlobalSettings.IsTemplateEnabled(section);
        BOOL local = box.IsTemplateEnabled(section);
        if (local || (! global))
            CTemplateListBox::Decorate(descr, local, TRUE);
        else if (global)
            CTemplateListBox::DecorateAster(descr);

        AddStringAndSetWidth(pListBox, descr);
    }
}


//---------------------------------------------------------------------------
// Template_OnOK
//---------------------------------------------------------------------------


void CAppPage::Template_OnOK(CBox &box)
{
    CSbieIni &ini = CSbieIni::GetInstance();

    CListBox *pListBox = (CListBox *)GetDlgItem(ID_APP_TEMPLATE_LIST);
    int size = pListBox->GetCount();
    for (int index = 0; index < size; ++index) {

        CString text;
        pListBox->GetText(index, text);
        if (text.IsEmpty())
            continue;

        POSITION pos = m_tmpl_sections.FindIndex(index);
        CString section = m_tmpl_sections.GetAt(pos);

        if (CTemplateListBox::IsCheck(text)) {
            BOOL ok = box.EnableTemplate(section, TRUE);
            if (! ok)
                break;
        }

        if (CTemplateListBox::IsClear(text)) {
            BOOL ok = box.EnableTemplate(section, FALSE);
            if (! ok)
                break;

            if (m_tmpl_info.ClassName.CompareNoCase(L"Local") != 0)
                continue;

            BOOL any_found = FALSE;
            CBoxes &boxes = CBoxes::GetInstance();
            for (int i = (int)boxes.GetSize() - 1; i >= 1; --i) {
                if (boxes.GetBox(i).IsTemplateEnabled(section)) {
                    any_found = TRUE;
                    break;
                }
            }
            if (any_found)
                continue;

            CString descr = CTemplateListBox::GetTemplateTitle(section);
            if (descr.Compare(L"-") == 0)
                descr = section;

            CMyMsg msg(MSG_4223, descr);
            int rv = CMyApp::MsgBox(GetParent(), msg, MB_YESNO);
            if (rv != IDYES)
                continue;

            ok = ini.RemoveSection(ini.m_Template_ + section);
            if (! ok)
                break;
        }
    }
}


//---------------------------------------------------------------------------
// Template_OnAdd
//---------------------------------------------------------------------------


void CAppPage::Template_OnAdd()
{
    CListBox *pListBox = (CListBox *)GetDlgItem(ID_APP_TEMPLATE_LIST);
    pListBox->SetFocus();

    if (CTemplateListBox::OnAddRemove(this, TRUE))
        OnModified();
}


//---------------------------------------------------------------------------
// Template_OnRemove
//---------------------------------------------------------------------------


void CAppPage::Template_OnRemove()
{
    CListBox *pListBox = (CListBox *)GetDlgItem(ID_APP_TEMPLATE_LIST);
    pListBox->SetFocus();

    if (CTemplateListBox::OnAddRemove(this, FALSE))
        OnModified();
}


//---------------------------------------------------------------------------
// Template_OnToggle
//---------------------------------------------------------------------------


void CAppPage::Template_OnToggle()
{
    if (CTemplateListBox::OnAddRemove(this, FALSE, TRUE))
        OnModified();
}


//---------------------------------------------------------------------------
// Template_OnView
//---------------------------------------------------------------------------


void CAppPage::Template_OnView()
{
    CListBox *pListBox = (CListBox *)GetDlgItem(ID_APP_TEMPLATE_LIST);
    pListBox->SetFocus();

    int index = pListBox->GetCurSel();
    if (index != LB_ERR) {

        POSITION pos = m_tmpl_sections.FindIndex(index);
        if (pos) {

            const CString &section = m_tmpl_sections.GetAt(pos);
            CViewTemplateDialog dlg(GetParent(), section);
            return;
        }
    }
}


//---------------------------------------------------------------------------
// Template_OnLink
//---------------------------------------------------------------------------


void CAppPage::Template_OnLink()
{
    CListBox *pListBox = (CListBox *)GetDlgItem(ID_APP_TEMPLATE_LIST);
    pListBox->SetFocus();

    int index = pListBox->GetCurSel();
    if (index != LB_ERR) {

        POSITION pos = m_tmpl_sections.FindIndex(index);
        if (pos) {

            const CString &section = m_tmpl_sections.GetAt(pos);
            CString url =
                CSbieIni::GetInstance().GetTemplateMetaData(section, L"Url");
            if (! url.IsEmpty()) {

                CRunBrowser x(GetParent(), url);
                return;
            }
        }
    }

    CMyApp::MsgBox(GetParent(), MSG_4205, MB_OK);
}


//---------------------------------------------------------------------------
// Template_OnCreate
//---------------------------------------------------------------------------


void CAppPage::Template_OnCreate()
{
    CViewTemplateDialog dlg(GetParent(), CString());
    CBox &box = CBoxes::GetInstance().GetBox(m_BoxName);
    Template_OnInitDialog(box);
}


//---------------------------------------------------------------------------
// Template_Filter
//---------------------------------------------------------------------------


void CAppPage::Template_Filter()
{
    int modif = m_tmpl_info.ClassModifier;
    if (! modif)
        return;

    CWaitCursor wait;

    POSITION pos = m_tmpl_sections.GetHeadPosition();
    while (pos) {
        POSITION old_pos = pos;
        CString name = m_tmpl_sections.GetNext(pos);
        BOOL ie = (name.Find(L"IExplore_") != -1);
        BOOL edge = (name.Find(L"Edge_") != -1);
        BOOL ff = (name.Find(L"Firefox_") != -1)
               || (name.Find(L"Waterfox_") != -1)
               || (name.Find(L"PaleMoon_") != -1)
               || (name.Find(L"SeaMonkey_") != -1);
        BOOL ch = (name.Find(L"Chrome_") != -1);
        BOOL other = (name.Find(L"Dragon_") != -1)
                  || (name.Find(L"Iron_") != -1)
                  || (name.Find(L"Ungoogled_") != -1)
                  || (name.Find(L"Vivaldi_") != -1)
                  || (name.Find(L"Brave_") != -1)
                  || (name.Find(L"Maxthon_6_") != -1)
                  || (name.Find(L"Opera_") != -1)
                  || (name.Find(L"Yandex_") != -1);

        BOOL keep = FALSE;

        if ((modif & MODIF_IEXPLORE) && ie)
            keep = TRUE;
        else if ((modif & MODIF_EDGE) && edge)
            keep = TRUE;
        else if ((modif & MODIF_FIREFOX) && ff)
            keep = TRUE;
        else if ((modif & MODIF_CHROME) && ch)
            keep = TRUE;
        else if ((modif & MODIF_OTHERWB) && other)
            keep = TRUE;

        if (modif & MODIF_NONLOCAL) {

            CString ClassName = CTemplateListBox::GetTemplateClass(name);
            if (ClassName.CompareNoCase(L"Local") != 0) {
                CString descr = CTemplateListBox::GetTemplateTitle(name);
                if (descr.GetLength() > 1)
                    keep = TRUE;
            }
        }

        if (keep) {
            if (name.Find(L"_Addon_") != -1) {
                if (! (modif & MODIF_ADDONS))
                    keep = FALSE;
            } else {
                if (modif & MODIF_ADDONS)
                    keep = FALSE;
            }
        }

        if (! keep)
            m_tmpl_sections.RemoveAt(old_pos);
    }

    if (modif & MODIF_NONLOCAL)
        Template_Sort();
}


//---------------------------------------------------------------------------
// Template_Sort
//---------------------------------------------------------------------------


void CAppPage::Template_Sort()
{
    POSITION endpos = m_tmpl_sections.GetTailPosition();
    for (int i = 1; i < m_tmpl_sections.GetCount(); ++i) {
        bool changed = false;
        POSITION pos2;
        POSITION pos1 = m_tmpl_sections.GetHeadPosition();
        while (pos1 != endpos) {
            pos2 = pos1;
            CString section1 = m_tmpl_sections.GetNext(pos1);
            CString section2 = m_tmpl_sections.GetAt(pos1);
            CString descr1 = CTemplateListBox::GetTemplateTitle(section1);
            CString descr2 = CTemplateListBox::GetTemplateTitle(section2);
            if (descr1.CompareNoCase(descr2) > 0) {
                m_tmpl_sections.SetAt(pos2, section2);
                m_tmpl_sections.SetAt(pos1, section1);
                changed = true;
            }
        }
        if (! changed)
            return;
        m_tmpl_sections.GetPrev(endpos);
    }
}


//---------------------------------------------------------------------------
// OnMeasureItem
//---------------------------------------------------------------------------


void CAppPage::OnMeasureItem(int idCtrl, LPMEASUREITEMSTRUCT lpmis)
{
    if (lpmis->CtlType == ODT_LISTBOX) {

        CDC *dc = GetDC();
        CFont *oldFont = dc->SelectObject(m_font);

        int height;
        TEXTMETRIC tm;
        if (dc->GetTextMetrics(&tm))
            height = tm.tmAscent;
        else
            height = 13;

        dc->SelectObject(oldFont);
        ReleaseDC(dc);

        lpmis->itemHeight = height;

    } else
        Default();
}


//---------------------------------------------------------------------------
// OnDrawItem
//---------------------------------------------------------------------------


void CAppPage::OnDrawItem(int idCtrl, LPDRAWITEMSTRUCT lpdis)
{
    if (lpdis->CtlType == ODT_LISTBOX && lpdis->itemID != -1) {

        CDC dc;
        dc.Attach(lpdis->hDC);
        COLORREF oldTextColor = dc.GetTextColor();
        COLORREF oldBkColor = dc.GetBkColor();

        COLORREF clr = oldBkColor;
        if (lpdis->itemState & ODS_SELECTED) {
            dc.SetTextColor(::GetSysColor(COLOR_HIGHLIGHTTEXT));
            dc.SetBkColor(::GetSysColor(COLOR_HIGHLIGHT));
            clr = ::GetSysColor(COLOR_HIGHLIGHT);
        }
        dc.FillSolidRect(&lpdis->rcItem, clr);

        CListBox *pListBox = (CListBox *)GetDlgItem(lpdis->CtlID);
        CString text1, text2;
        pListBox->GetText(lpdis->itemID, text1);

        if (! text1.IsEmpty()) {

            int index = text1.Find(L']');
            if (index != -1) {
                text2 = text1.Mid(index + 3);
                text1 = text1.Mid(1, index - 1);
                text1.TrimLeft();
            } else {
                text2 = text1;
                text1 = CString();
            }

            CString text3(L"[    ]");
            CRect rc1 = CRect(lpdis->rcItem);
            dc.DrawText(text3, rc1, DT_SINGLELINE | DT_VCENTER);
            CSize sz1 = dc.GetTextExtent(text3);

            if (! text1.IsEmpty()) {

                CSize sz2 = dc.GetTextExtent(text1);
                rc1.left = rc1.left + (sz1.cx - sz2.cx) / 2;
                int oldBkMode = dc.SetBkMode(TRANSPARENT);
                dc.DrawText(text1, rc1, DT_SINGLELINE | DT_VCENTER);
                dc.SetBkMode(oldBkMode);
            }

            rc1 = CRect(lpdis->rcItem);
            rc1.left += sz1.cx + 8;
            dc.DrawText(text2, rc1, DT_SINGLELINE | DT_VCENTER);
        }

        dc.SetTextColor(oldTextColor);
        dc.SetBkColor(oldBkColor);
        dc.Detach();

    } else
        Default();
}


//---------------------------------------------------------------------------
// Folders_OnInitDialog
//---------------------------------------------------------------------------


void CAppPage::Folders_OnInitDialog(CBox &box)
{
    //
    // set dialog texts
    //

    GetDlgItem(ID_PAGE_LABEL_1)->SetWindowText(CMyMsg(MSG_4210));
    GetDlgItem(ID_APP_FOLDERS_LABEL_1)->SetWindowText(CMyMsg(MSG_4211));
    GetDlgItem(ID_APP_FOLDERS_LABEL_2)->SetWindowText(CMyMsg(MSG_4212));
    GetDlgItem(ID_APP_FOLDERS_LABEL_3)->SetWindowText(CMyMsg(MSG_4213));

    GetDlgItem(ID_APP_FOLDERS_ADD)->SetWindowText(CMyMsg(MSG_3352));
    GetDlgItem(ID_APP_FOLDERS_REMOVE)->SetWindowText(CMyMsg(MSG_3351));
    GetDlgItem(ID_APP_FOLDERS_ADD)->ShowWindow(SW_HIDE);
    GetDlgItem(ID_APP_FOLDERS_REMOVE)->ShowWindow(SW_HIDE);

    GetDlgItem(ID_APP_FOLDERS_DEFAULT)->SetWindowText(CString());
    GetDlgItem(ID_APP_FOLDERS_OVERRIDE)->SetWindowText(CString());

    GetDlgItem(ID_APP_FOLDERS_DEFAULT)->SetFont(&CMyApp::m_fontFixed);
    GetDlgItem(ID_APP_FOLDERS_OVERRIDE)->SetFont(&CMyApp::m_fontFixed);

    //
    // reset everything
    //

    while (! m_folders_vars.IsEmpty())
        m_folders_vars.RemoveHead();

    //
    // enumerate templates in this sandbox
    //

    CSbieIni &ini = CSbieIni::GetInstance();

    CStringList tmpl_names;
    ini.GetTextList(box.GetName(), ini.m_Template, tmpl_names);
    ini.GetTextList(_GlobalSettings, ini.m_Template, tmpl_names);

    while (! tmpl_names.IsEmpty()) {
        CString tmpl_name = tmpl_names.RemoveHead();
        CString varname = ini.GetTemplateVariable(tmpl_name); // fix-me: there may be more than one folder per template
        if (varname.IsEmpty())
            continue;

        BOOL found = FALSE;
        POSITION pos = m_folders_vars.GetHeadPosition();
        while (pos) {
            CString varname2 = m_folders_vars.GetNext(pos);
            if (varname2.CompareNoCase(varname) == 0) {
                found = TRUE;
                break;
            }
        }

        if (! found)
            m_folders_vars.AddTail(varname);
    }

    //
    // populate combobox
    //

    CComboBox *pCombo = (CComboBox *)GetDlgItem(ID_APP_FOLDERS_COMBO);

    pCombo->ResetContent();

    POSITION pos = m_folders_vars.GetHeadPosition();
    while (pos) {
        CString varname = m_folders_vars.GetNext(pos);
        if (varname.Left(5).CompareNoCase(ini.m_Tmpl_) == 0)
            varname = varname.Mid(5);
        pCombo->AddString(varname);
    }

    if (FileAccess_index == -1)
        pCombo->SetCurSel(0);
    else {
        pCombo->SetCurSel(FileAccess_index);
        FileAccess_index = -1;
    }
    Folders_OnSelect();
}


//---------------------------------------------------------------------------
// Folders_OnOK
//---------------------------------------------------------------------------


void CAppPage::Folders_OnOK(CBox &box)
{
    CComboBox *pCombo = (CComboBox *)GetDlgItem(ID_APP_FOLDERS_COMBO);
    int index = pCombo->GetCurSel();
    if (index == CB_ERR)
        return;
    POSITION pos = m_folders_vars.FindIndex(index);
    if (! pos)
        return;
    CString varname = m_folders_vars.GetAt(pos);

    CString value;
    GetDlgItem(ID_APP_FOLDERS_OVERRIDE)->GetWindowText(value);

    CSbieIni &ini = CSbieIni::GetInstance();
    ini.SetTemplateVariable(varname, value, TRUE);
}


//---------------------------------------------------------------------------
// Folders_OnSelect
//---------------------------------------------------------------------------


void CAppPage::Folders_OnSelect()
{
    CComboBox *pCombo = (CComboBox *)GetDlgItem(ID_APP_FOLDERS_COMBO);

    if (m_modified) {
        pCombo->SetCurSel(FileAccess_index);
        CMyApp::MsgBox(GetParent(), MSG_3803, MB_OK);
        return;
    }

    int index = pCombo->GetCurSel();
    if (index == CB_ERR)
        return;
    POSITION pos = m_folders_vars.FindIndex(index);
    if (! pos)
        return;

    CString value;
    CSbieIni &ini = CSbieIni::GetInstance();

    CString varname = m_folders_vars.GetAt(pos);
    ini.GetText(ini.m_TemplateSettings, varname, value);
    GetDlgItem(ID_APP_FOLDERS_DEFAULT)->SetWindowText(value);

    varname = varname + L'.' + CUserSettings::GetInstance().GetUserName();
    ini.GetText(ini.m_TemplateSettings, varname, value);
    GetDlgItem(ID_APP_FOLDERS_OVERRIDE)->SetWindowText(value);

    GetDlgItem(ID_APP_FOLDERS_ADD)->EnableWindow(value.IsEmpty());
    GetDlgItem(ID_APP_FOLDERS_REMOVE)->EnableWindow(! value.IsEmpty());

    GetDlgItem(ID_APP_FOLDERS_ADD)->ShowWindow(SW_SHOW);
    GetDlgItem(ID_APP_FOLDERS_REMOVE)->ShowWindow(SW_SHOW);
}


//---------------------------------------------------------------------------
// Folders_OnAdd
//---------------------------------------------------------------------------


void CAppPage::Folders_OnAdd()
{
    CComboBox *pCombo = (CComboBox *)GetDlgItem(ID_APP_FOLDERS_COMBO);
    int index = pCombo->GetCurSel();
    if (index == CB_ERR)
        return;
    CString varname;
    pCombo->GetLBText(index, varname);
    CString title = CMyMsg(MSG_4215, varname);

    WCHAR path[MAX_PATH + 32];
    BROWSEINFO bi;
    memzero(&bi, sizeof(BROWSEINFO));
    bi.hwndOwner = GetParent()->m_hWnd;
    bi.pszDisplayName = path;
    bi.lpszTitle = title;
    bi.ulFlags = BIF_UAHINT | BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE | BIF_EDITBOX;
    LPITEMIDLIST pidl = SHBrowseForFolder(&bi);
    if (pidl) {

        BOOL ok = SHGetPathFromIDList(pidl, path);
        CoTaskMemFree(pidl);
        if (ok) {

            while (1) {
                int len = wcslen(path);
                if (len > 1 && path[len - 1] == L'\\')
                    path[len - 1] = L'\0';
                else
                    break;
            }

            if (Folders_CheckExpectFile(varname, path)) {

                GetDlgItem(ID_APP_FOLDERS_OVERRIDE)->SetWindowText(path);
                GetDlgItem(ID_APP_FOLDERS_ADD)->EnableWindow(FALSE);
                GetDlgItem(ID_APP_FOLDERS_REMOVE)->EnableWindow(TRUE);

                FileAccess_index = index;
                OnModified();
            }
        }
    }

}


//---------------------------------------------------------------------------
// Folders_OnRemove
//---------------------------------------------------------------------------


void CAppPage::Folders_OnRemove()
{
    CComboBox *pCombo = (CComboBox *)GetDlgItem(ID_APP_FOLDERS_COMBO);
    int index = pCombo->GetCurSel();
    if (index == CB_ERR)
        return;

    GetDlgItem(ID_APP_FOLDERS_OVERRIDE)->SetWindowText(CString());
    GetDlgItem(ID_APP_FOLDERS_ADD)->EnableWindow(TRUE);
    GetDlgItem(ID_APP_FOLDERS_REMOVE)->EnableWindow(FALSE);

    FileAccess_index = index;
    OnModified();
}


//---------------------------------------------------------------------------
// Folders_OnRemove
//---------------------------------------------------------------------------


BOOL CAppPage::Folders_CheckExpectFile(const WCHAR *var, const WCHAR *path)
{
    CSbieIni &ini = CSbieIni::GetInstance();

    CString varname = ini.m_Tmpl_ + var + L":ExpectFile";
    CString varvalue;
    ini.GetText(ini.m_TemplateSettings, varname, varvalue);

    if (! varvalue.IsEmpty()) {

        CString path2 = path;
        path2 += L"\\";
        path2 += varvalue;

        ULONG attrs = GetFileAttributes(path2);
        if (attrs == INVALID_FILE_ATTRIBUTES) {

            CMyMsg msgtext(MSG_4214, var, varvalue);
            CMyApp::MsgBox(GetParent(), msgtext, MB_OK);
            return FALSE;
        }
    }

    return TRUE;
}


//---------------------------------------------------------------------------
// AddPages
//---------------------------------------------------------------------------


void CAppPage::AddPages(CPropertySheet &sheet, const CString &BoxName)
{
    TMPL_INFO info;

    info.ClassName = L"WebBrowser";
    info.TitleId = MSG_4321;
    info.LabelId = MSG_4322;
    info.WithLink = FALSE;
    info.WithCreate = FALSE;
    info.ClassModifier = MODIF_IEXPLORE;                // IExplore
    m_app_pages.AddTail(new CAppPage(&info, BoxName));
    info.ClassModifier = MODIF_EDGE;                    // Microsoft Edge
    m_app_pages.AddTail(new CAppPage(&info, BoxName));
    info.ClassModifier = MODIF_FIREFOX;                 // Mozilla Firefox
    m_app_pages.AddTail(new CAppPage(&info, BoxName));
    info.WithLink = TRUE;
    info.ClassModifier |= MODIF_ADDONS;                 // Firefox Add-ons
    m_app_pages.AddTail(new CAppPage(&info, BoxName));
    info.WithLink = FALSE;
    info.ClassModifier = MODIF_CHROME;                  // Google Chrome
    m_app_pages.AddTail(new CAppPage(&info, BoxName));
    info.ClassModifier = MODIF_OTHERWB;                 // Other
    m_app_pages.AddTail(new CAppPage(&info, BoxName));
    info.ClassModifier = 0;

    info.ClassName = L"EmailReader";
    info.TitleId = MSG_4391;
    info.LabelId = MSG_4392;
    info.WithLink = FALSE;
    info.WithCreate = FALSE;
    m_app_pages.AddTail(new CAppPage(&info, BoxName));

    info.ClassName = L"Print";
    info.TitleId = MSG_4224;
    info.LabelId = MSG_4207;
    info.WithLink = TRUE;
    info.WithCreate = FALSE;
    m_app_pages.AddTail(new CAppPage(&info, BoxName));

    info.ClassName = L"Security";
    info.TitleId = MSG_4226;
    info.LabelId = MSG_4207;
    info.WithLink = TRUE;
    info.WithCreate = FALSE;
    m_app_pages.AddTail(new CAppPage(&info, BoxName));

    info.ClassName = L"Desktop";
    info.TitleId = MSG_4228;
    info.LabelId = MSG_4207;
    info.WithLink = TRUE;
    info.WithCreate = FALSE;
    m_app_pages.AddTail(new CAppPage(&info, BoxName));
    
    info.ClassName = L"MediaPlayer";
    info.TitleId = MSG_4393;
    info.LabelId = MSG_4394;
    info.WithLink = FALSE;
    info.WithCreate = FALSE;
    m_app_pages.AddTail(new CAppPage(&info, BoxName));
    
    info.ClassName = L"TorrentClient";
    info.TitleId = MSG_4396;
    info.LabelId = MSG_4397;
    info.WithLink = FALSE;
    info.WithCreate = FALSE;
    m_app_pages.AddTail(new CAppPage(&info, BoxName));

    info.ClassName = L"Download";
    info.TitleId = MSG_4230;
    info.LabelId = MSG_4207;
    info.WithLink = TRUE;
    info.WithCreate = FALSE;
    m_app_pages.AddTail(new CAppPage(&info, BoxName));

    info.ClassName = L"Misc";
    info.TitleId = MSG_4206;
    info.LabelId = MSG_4207;
    info.WithLink = TRUE;
    info.WithCreate = FALSE;
    m_app_pages.AddTail(new CAppPage(&info, BoxName));

    info.ClassName = L"*";
    info.TitleId = MSG_4288;
    info.LabelId = MSG_4289;
    info.WithLink = TRUE;
    info.WithCreate = FALSE;
    info.ClassModifier = MODIF_NONLOCAL;
    m_app_pages.AddTail(new CAppPage(&info, BoxName));
    info.ClassModifier = 0;

    info.ClassName = L"Local";
    info.TitleId = MSG_4218;
    info.LabelId = MSG_4219;
    info.WithLink = FALSE;
    info.WithCreate = TRUE;
    m_app_pages.AddTail(new CAppPage(&info, BoxName));

    m_app_pages.AddTail(
        new CAppPage(AppPageFolders, BoxName, MSG_4209));

    m_app_pages.AddTail(
        new CAppPage(AppPageAccess,  BoxName, MSG_4302));

    POSITION pos = m_app_pages.GetHeadPosition();
    while (pos) {
        CAppPage *page = (CAppPage *)m_app_pages.GetNext(pos);
        sheet.AddPage(page);
    }
}


//---------------------------------------------------------------------------
// DeletePages
//---------------------------------------------------------------------------


void CAppPage::DeletePages()
{
    while (! m_app_pages.IsEmpty()) {
        CAppPage *page = (CAppPage *)m_app_pages.RemoveHead();
        delete page;
    }
}


//---------------------------------------------------------------------------
// UpdateTemplates
//---------------------------------------------------------------------------


void CAppPage::UpdateTemplates(CBox &box)
{
    CStringList list;

    //
    // find and remove:
    //
    //      AutoRecoverIgnore=.jc!
    //      AutoRecoverIgnore=.part
    //
    // replace with:    Template=AutoRecoverIgnore
    //

    box.GetAutoRecoveryIgnores(list, TRUE);
    while (! list.IsEmpty()) {
        CString item = list.RemoveHead();
        int cmp1 = item.CompareNoCase(L".jc!");
        int cmp2 = item.CompareNoCase(L".part");
        if (cmp1 == 0 || cmp2 == 0)
            box.AddOrRemoveAutoRecoveryIgnores(item, FALSE);
    }

    //
    // find and replace other templates
    //

    UpdateWebTemplates(box);
    UpdateEmailTemplates(box);

    //
    // finally, enable default templates
    //

    SetDefaultTemplates6(box);
}


//---------------------------------------------------------------------------
// UpdateWebTemplates
//---------------------------------------------------------------------------


void CAppPage::UpdateWebTemplates(CBox &box)
{
    //
    // find and remove:
    //      OpenFilePath=firefox.exe,
    //          %AppData%\Mozilla\Firefox\Profiles\*\bookmark*
    //      OpenFilePath=firefox.exe,
    //          %AppData%\Mozilla\Firefox\Profiles\*\places*
    //
    // replace with:    Template=Firefox_Bookmarks_DirectAccess
    //

    UpdateTemplates2(
        box, L"firefox.exe", L"Firefox_Bookmarks_DirectAccess",
        L"%AppData%\\Mozilla\\Firefox\\Profiles\\*\\bookmark*",
        L"%AppData%\\Mozilla\\Firefox\\Profiles\\*\\places*"
    );

    //
    // find and remove:
    //      OpenFilePath=seamonkey.exe,%AppData%\Mozilla\Profiles\*\bookmark*
    //      OpenFilePath=seamonkey.exe,%AppData%\Mozilla\Profiles\*\places*
    //
    // replace with:    Template=SeaMonkey_Bookmarks_DirectAccess
    //

    const CString &SeaMonkeyTemplate(L"SeaMonkey_Bookmarks_DirectAccess");
    const CString &SeaMonkeyPath1(
        L"%AppData%\\Mozilla\\SeaMonkey\\Profiles\\*\\bookmark*");
    const CString &SeaMonkeyPath2(
        L"%AppData%\\Mozilla\\SeaMonkey\\Profiles\\*\\places*");

    UpdateTemplates2(
        box, L"seamonkey.exe", SeaMonkeyTemplate,
        SeaMonkeyPath1, SeaMonkeyPath2);

    //
    // find and remove:
    //      OpenFilePath=opera.exe,%AppData%\Opera Software\Opera Stable\Bookmarks
    //      OpenFilePath=opera.exe,%AppData%\Opera Software\Opera Stable\Bookmarks.bak
    //      OpenFilePath=opera.exe,%AppData%\Opera Software\Opera Stable\BookmarksExtras
    //
    // replace with:    Template=Opera_Bookmarks_DirectAccess
    //
    
    const CString &OperaBookmarksTmpl(L"Opera_Bookmarks_DirectAccess");
    const CString &OperaBookmarks1(L"%AppData%\\Opera Software\\Opera Stable\\Bookmarks");
    const CString &OperaBookmarks2(L"%AppData%\\Opera Software\\Opera Stable\\Bookmarks.bak");
    const CString &OperaBookmarks3(L"%AppData%\\Opera Software\\Opera Stable\\BookmarksExtras");

    UpdateTemplates2(
        box, L"opera.exe", OperaBookmarksTmpl, OperaBookmarks1, OperaBookmarks2);

    UpdateTemplates2(
        box, L"opera.exe", OperaBookmarksTmpl, OperaBookmarks3, OperaBookmarks3);

    //
    // find and remove invalid OpenFilePath reference:
    //      OpenFilePath=bookmarks.exe,bookmarks*
    //
    
    const CString &InvalidEntry1(L"bookmarks*");

    UpdateTemplates2(
        box, L"bookmarks.exe", L'\0', InvalidEntry1, InvalidEntry1);
}


//---------------------------------------------------------------------------
// UpdateEmailTemplates
//---------------------------------------------------------------------------


void CAppPage::UpdateEmailTemplates(CBox &box)
{
    //
    // find and remove:
    //      OpenFilePath=outlook.exe,%AppData%\Microsoft\Outlook
    //      OpenFilePath=outlook.exe,%Local AppData%\Microsoft\Outlook
    //      OpenFilePath=outlook.exe,*.eml
    //
    // replace with:    Template=Office_Outlook
    //
    // discard:
    //      OpenKeyPath=outlook.exe,
    //          HKEY_CURRENT_USER\Software\Microsoft\Office
    //      OpenKeyPath=outlook.exe,
    //          HKEY_CURRENT_USER\Software\Microsoft\Windows NT\
    //          CurrentVersion\Windows Messaging Subsystem\Profiles\Outlook
    //

    CStringList _OfficeOutlookFiles;
    CStringList _OfficeOutlookKeys;

    _OfficeOutlookFiles.AddTail(L"%AppData%\\Microsoft\\Outlook");
    _OfficeOutlookFiles.AddTail(L"%Local AppData%\\Microsoft\\Outlook");
    _OfficeOutlookFiles.AddTail(L"*.eml");

    _OfficeOutlookKeys.AddTail(
        L"HKEY_CURRENT_USER\\Software\\Microsoft\\Office");
    _OfficeOutlookKeys.AddTail(
        L"HKEY_CURRENT_USER\\Software\\Microsoft\\Windows NT\\"
        L"CurrentVersion\\Windows Messaging Subsystem\\Profiles\\Outlook");

    UpdateTemplates3(
        box, L"outlook.exe", L"Office_Outlook",
        _OfficeOutlookFiles, _OfficeOutlookKeys);

    //
    // find and remove:
    //      OpenFilePath=winmail.exe,%AppData%\Microsoft\Windows Mail
    //      OpenFilePath=winmail.exe,%Local AppData%\Microsoft\Windows Mail
    //      OpenFilePath=winmail.exe,*.eml
    //
    // replace with:    Template=Windows_Vista_Mail
    //
    // discard:
    //      OpenKeyPath=winmail.exe,
    //          HKEY_CURRENT_USER\Software\Microsoft\Windows Mail
    //

    CStringList _WindowsMailFiles;
    CStringList _WindowsMailKeys;

    _WindowsMailFiles.AddTail(L"%AppData%\\Microsoft\\Windows Mail");
    _WindowsMailFiles.AddTail(L"%Local AppData%\\Microsoft\\Windows Mail");
    _WindowsMailFiles.AddTail(L"*.eml");

    _WindowsMailKeys.AddTail(
        L"HKEY_CURRENT_USER\\Software\\Microsoft\\Windows Mail");

    UpdateTemplates3(
        box, L"winmail.exe", L"Windows_Vista_Mail",
        _WindowsMailFiles, _WindowsMailKeys);

    //
    // find and remove:
    //      OpenFilePath=wlmail.exe,%AppData%\Microsoft\Windows Live Mail
    //      OpenFilePath=wlmail.exe,
    //          %Local AppData%\Microsoft\Windows Live Mail
    //      OpenFilePath=wlmail.exe,*.eml
    //
    // replace with:    Template=Windows_Live_Mail
    //
    // discard:
    //      OpenKeyPath=wlmail.exe,
    //          HKEY_CURRENT_USER\Software\Microsoft\Windows Live
    //      OpenKeyPath=wlmail.exe,
    //          HKEY_CURRENT_USER\Software\Microsoft\Windows Live Mail
    //

    CStringList _LiveMailFiles;
    CStringList _LiveMailKeys;

    _LiveMailFiles.AddTail(L"%AppData%\\Microsoft\\Windows Live Mail");
    _LiveMailFiles.AddTail(L"%Local AppData%\\Microsoft\\Windows Live Mail");
    _LiveMailFiles.AddTail(L"*.eml");

    _LiveMailKeys.AddTail(
        L"HKEY_CURRENT_USER\\Software\\Microsoft\\Windows Live");
    _LiveMailKeys.AddTail(
        L"HKEY_CURRENT_USER\\Software\\Microsoft\\Windows Live Mail");

    UpdateTemplates3(
        box, L"wlmail.exe", L"Windows_Live_Mail",
        _LiveMailFiles, _LiveMailKeys);

    //
    // find and remove:
    //      OpenFilePath=thunderbird.exe,%AppData%\Thunderbird
    //      OpenFilePath=thunderbird.exe,%Local AppData%\Thunderbird
    //
    // replace with:    Template=Thunderbird
    //
    // discard:
    //      OpenKeyPath=thunderbird.exe,
    //          HKEY_CURRENT_USER\Software\Mozilla Thunderbird
    //      OpenKeyPath=thunderbird.exe,
    //          HKEY_LOCAL_MACHINE\Software\Mozilla Thunderbird
    //      OpenKeyPath=thunderbird.exe,
    //          HKEY_LOCAL_MACHINE\Software\Wow6432Node\Mozilla Thunderbird
    //

    CStringList _ThunderbirdFiles;
    CStringList _ThunderbirdKeys;

    _ThunderbirdFiles.AddTail(L"%AppData%\\Thunderbird");
    _ThunderbirdFiles.AddTail(L"%Local AppData%\\Thunderbird");

    _ThunderbirdKeys.AddTail(
        L"HKEY_CURRENT_USER\\Software\\Mozilla Thunderbird");
    _ThunderbirdKeys.AddTail(
        L"HKEY_LOCAL_MACHINE\\Software\\Mozilla Thunderbird");
    _ThunderbirdKeys.AddTail(
        L"HKEY_LOCAL_MACHINE\\Software\\Wow6432Node\\Mozilla Thunderbird");

    CString _Thunderbird(L"Thunderbird");

    UpdateTemplates3(
        box, L"thunderbird.exe", _Thunderbird,
        _ThunderbirdFiles, _ThunderbirdKeys);

    //
    // find and remove:
    //      OpenFilePath=seamonkey.exe,%AppData%\Mozilla\Profiles\*\Mail*
    //      OpenFilePath=seamonkey.exe,%Local AppData%\Mozilla\Profiles\*\Mail*
    //
    // replace with:    Template=SeaMonkey
    //
    // discard:
    //      OpenKeyPath=seamonkey.exe,
    //          HKEY_CURRENT_USER\Software\Mozilla*\SeaMonkey*
    //      OpenKeyPath=seamonkey.exe,
    //          HKEY_LOCAL_MACHINE\Software\Mozilla\SeaMonkey*
    //      OpenKeyPath=seamonkey.exe,
    //          HKEY_LOCAL_MACHINE\Software\Wow6432Node\Mozilla\SeaMonkey*
    //

    CStringList _SeaMonkeyFiles;
    CStringList _SeaMonkeyKeys;

    _SeaMonkeyFiles.AddTail(L"%AppData%\\Mozilla\\Profiles\\*\\Mail*");
    _SeaMonkeyFiles.AddTail(L"%Local AppData%\\Mozilla\\Profiles\\*\\Mail*");

    _SeaMonkeyKeys.AddTail(
        L"HKEY_CURRENT_USER\\Software\\Mozilla*\\SeaMonkey*");
    _SeaMonkeyKeys.AddTail(
        L"HKEY_LOCAL_MACHINE\\Software\\Mozilla\\SeaMonkey*");
    _SeaMonkeyKeys.AddTail(
        L"HKEY_LOCAL_MACHINE\\Software\\Wow6432Node\\Mozilla\\SeaMonkey*");

    CString _SeaMonkey(L"SeaMonkey");

    UpdateTemplates3(
        box, L"seamonkey.exe", _SeaMonkey,
        _SeaMonkeyFiles, _SeaMonkeyKeys);

    //
    // find and remove:
    //      OpenFilePath=opera.exe,%AppData%\Opera\Opera\mail
    //
    // replace with:    Template=Opera_Mail
    //

    CString _OperaMailPath1(L"%AppData%\\Opera\\Opera\\mail");

    UpdateTemplates2(
        box, L"opera.exe", L"Opera_Mail", _OperaMailPath1, _OperaMailPath1);
}


//---------------------------------------------------------------------------
// UpdateTemplates2
//---------------------------------------------------------------------------


void CAppPage::UpdateTemplates2(CBox &box,
                                const CString &image_name,
                                const CString &tmpl_name,
                                const CString &path1,
                                const CString &path2)
{
    BOOL found = FALSE;

    CStringList list;
    box.GetXxxPathListForPgm('fo', list, image_name, TRUE);

    POSITION pos = list.GetHeadPosition();
    while (pos) {
        POSITION pos0 = pos;
        CString &item = list.GetNext(pos);
        int cmp1 = item.CompareNoCase(path1);
        int cmp2 = item.CompareNoCase(path2);
        BOOL cmp3 = ((! item.IsEmpty()) && (item.GetAt(0) == L'['));
        if (cmp1 == 0 || cmp2 == 0 || cmp3) {
            if (! cmp3)
                found = TRUE;
            list.RemoveAt(pos0);
            pos = list.GetHeadPosition();
        }
    }

    if (found) {
        box.SetXxxPathListForPgm('fo', list, image_name);
        box.EnableTemplate(tmpl_name, TRUE);
    }
}


//---------------------------------------------------------------------------
// UpdateTemplates3
//---------------------------------------------------------------------------


void CAppPage::UpdateTemplates3(CBox &box,
                                const CString &image_name,
                                const CString &tmpl_name,
                                CStringList &files,
                                CStringList &keys)
{
    CStringList list;
    box.GetXxxPathListForPgm('fo', list, image_name, TRUE);

    while (! files.IsEmpty()) {
        CString curItem = files.RemoveHead();
        BOOL found_curItem = FALSE;

        POSITION pos = list.GetHeadPosition();
        while (pos) {

            POSITION pos0 = pos;
            CString &item = list.GetNext(pos);
            if (item.IsEmpty() || item.GetAt(0) == L'[') {
                list.RemoveAt(pos0);
                pos = list.GetHeadPosition();
                continue;
            }

            if (item.CompareNoCase(curItem) == 0) {
                list.RemoveAt(pos0);
                found_curItem = TRUE;
                break;
            }
        }

        if (! found_curItem)
            return;
    }

    box.SetXxxPathListForPgm('fo', list, image_name);

    while (! list.IsEmpty())
        list.RemoveHead();
    box.GetXxxPathListForPgm('ko', list, image_name, TRUE);

    while (! keys.IsEmpty()) {
        CString curItem = keys.RemoveHead();

        POSITION pos = list.GetHeadPosition();
        while (pos) {

            POSITION pos0 = pos;
            CString &item = list.GetNext(pos);
            if (item.IsEmpty() || item.GetAt(0) == L'[' ||
                    item.CompareNoCase(curItem) == 0) {
                list.RemoveAt(pos0);
                pos = list.GetHeadPosition();
                continue;
            }
        }
    }

    box.SetXxxPathListForPgm('ko', list, image_name);
    box.EnableTemplate(tmpl_name, TRUE);
}


//---------------------------------------------------------------------------
// SetDefaultTemplates6
//---------------------------------------------------------------------------


void CAppPage::SetDefaultTemplates6(CBox &box)
{
    box.EnableTemplate(L"AutoRecoverIgnore", TRUE);
    box.EnableTemplate(L"Firefox_Phishing_DirectAccess", TRUE);
    box.EnableTemplate(L"Chrome_Phishing_DirectAccess", TRUE);
    box.EnableTemplate(L"LingerPrograms", TRUE);
    SetDefaultTemplates7(box);
}


//---------------------------------------------------------------------------
// SetDefaultTemplates7
//---------------------------------------------------------------------------


void CAppPage::SetDefaultTemplates7(CBox &box)
{
    box.EnableTemplate(L"BlockPorts", TRUE);
    //box.EnableTemplate(L"WindowsFontCache", TRUE); // since 5.46.3 open by driver
    box.EnableTemplate(L"qWave", TRUE);
    SetDefaultTemplates8(box);
}

//---------------------------------------------------------------------------
// SetDefaultTemplates8
//---------------------------------------------------------------------------


void CAppPage::SetDefaultTemplates8(CBox& box)
{
    box.EnableTemplate(L"FileCopy", TRUE);
    box.EnableTemplate(L"SkipHook", TRUE);
    SetDefaultTemplates9(box);
}

//---------------------------------------------------------------------------
// SetDefaultTemplates9
//---------------------------------------------------------------------------


void CAppPage::SetDefaultTemplates9(CBox& box)
{
    CSbieIni &ini = CSbieIni::GetInstance();

    // fix the unfortunate typo
    if (box.IsTemplateEnabled(L"FileCppy")) {
        box.EnableTemplate(L"FileCopy", TRUE);
        box.EnableTemplate(L"FileCppy", FALSE);
    }

    box.EnableTemplate(L"WindowsFontCache", FALSE);

    BOOL bHardened = FALSE;
    ini.GetBool(box.GetName(), L"DropAdminRights", bHardened, FALSE);
    if (!bHardened) {
        // enable those templates only for non hardened boxes
        box.EnableTemplate(L"OpenBluetooth", TRUE);
        box.EnableTemplate(L"OpenSmartCard", TRUE);
    }
}
