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
// Box Settings Property Page
//---------------------------------------------------------------------------


#include "MyApp.h"
#include "BoxPage.h"

#include "AppPage.h"
#include "SbieIni.h"
#include "TreePropSheet.h"
#include "UserSettings.h"
#include "TemplateListBox.h"
#include "ProgramSelector.h"
#include "BorderGuard.h"
#include "common/win32_ntddk.h"
#include "apps/common/MyGdi.h"
#include "apps/common/CommonUtils.h"
#include <objsel.h>


#define PATTERN XPATTERN
#define _MY_POOL_H          // prevent inclusion of pool.h by pattern.h
typedef void *POOL;
#include "common/pattern.h"


//---------------------------------------------------------------------------
// Defines
//---------------------------------------------------------------------------


enum {
    BoxPageUnknown = 0,
    BoxPageAppearance,
    BoxPageQuickRecover,
    BoxPageAutoRecover,
    BoxPageAutoDelete,
    BoxPageDeleteCommand,
    BoxPageProgramGroups,
    BoxPageForceFol,
    BoxPageForcePgm,
    BoxPageLingerPgm,
    BoxPageLeaderPgm,
    BoxPageFileMigrate,
    BoxPageInternetPgm,
    BoxPageStartRunPgm,
    BoxPageDropRights,
    BoxPagePrintSpooler,
    BoxPageNetworkFiles,
    BoxPageOpenFile,
    BoxPageOpenPipe,
    BoxPageReadFile,
    BoxPageWriteFile,
    BoxPageClosedFile,
    BoxPageOpenKey,
    BoxPageReadKey,
    BoxPageWriteKey,
    BoxPageClosedKey,
    BoxPageOpenIpc,
    BoxPageClosedIpc,
    BoxPageWinClass,
    BoxPageComClass,
    BoxPageUserAccounts
};


//---------------------------------------------------------------------------
// Message Map
//---------------------------------------------------------------------------


BEGIN_MESSAGE_MAP(CBoxPage, CPropertyPage)

    ON_MESSAGE(PSM_QUERYSIBLINGS,               OnQuerySiblings)

    ON_COMMAND(ID_HIDE_INDICATOR,               Appearance_OnCheckBox1)
    ON_COMMAND(ID_BOXNAMETITLE,                 Appearance_OnCheckBox2)
    ON_COMMAND(ID_SHOW_BORDER,                  Appearance_OnShowBorder)
    ON_COMMAND(ID_BORDER_COLOR,                 Appearance_OnBorderColor)
    ON_COMMAND(ID_BORDER_TITLE,                 Appearance_OnBorderTitle)
    ON_CONTROL(EN_CHANGE, ID_BORDER_WIDTH,      OnModified)

    ON_COMMAND(ID_DELETE_AUTO,                  AutoDelete_OnAuto)
    ON_COMMAND(ID_DELETE_NEVER,                 AutoDelete_OnNever)

    ON_COMMAND(ID_DELETE_RMDIR,                 DeleteCommand_RmDir)
    ON_COMMAND(ID_DELETE_SDELETE,               DeleteCommand_SDelete)
    ON_COMMAND(ID_DELETE_ERASERL,               DeleteCommand_EraserL)
    ON_COMMAND(ID_DELETE_ERASER6,               DeleteCommand_Eraser6)

    ON_CONTROL(EN_CHANGE, ID_DELETE_COMMAND,    OnModified)

    ON_COMMAND(ID_RECOVER_AUTO,                 OnModified)

    ON_COMMAND(ID_RECOVER_ADD,                  QuickRecover_OnCmdAdd)
    ON_COMMAND(ID_RECOVER_REMOVE,               QuickRecover_OnCmdRemove)

    ON_COMMAND(ID_RECOVER_ADD_FOLDER,           AutoRecover_OnCmdAddFolder)
    ON_COMMAND(ID_RECOVER_ADD_TYPE,             AutoRecover_OnCmdAddType)

    ON_COMMAND(ID_PROGRAM_ADD,                  ForceLinger_OnCmdAddPgm)
    ON_COMMAND(ID_PROGRAM_ADD_FOLDER,           ForceLinger_OnCmdAddFolder)
    ON_COMMAND(ID_PROGRAM_REMOVE,               ForceLinger_OnCmdRemove)
    ON_COMMAND(ID_PROGRAM_ADD_STAR,             RestrictPgm_AddStar)
    ON_COMMAND(ID_PROGRAM_NOTIFY,               OnModified)

    ON_CONTROL(EN_CHANGE, ID_MIGRATE_KB,        FileMigrate_OnChangeKb)
    ON_COMMAND(ID_MIGRATE_SILENT,               OnModified)

    ON_COMMAND(ID_DROPRIGHTS,                   OnModified)

    ON_COMMAND(ID_BLOCKNETWORKFILES, OnModified)

    ON_COMMAND(ID_ALLOWSPOOLERPRINTTOFILE, OnModified)

    ON_CONTROL(CBN_SELENDOK, ID_FILE_PROGRAM,   FileAccess_OnSelectPgm)
    ON_CONTROL(LBN_SELCHANGE, ID_FILE_LIST,     FileAccess_OnSelectRes)
    ON_CONTROL(LBN_SELCANCEL, ID_FILE_LIST,     FileAccess_OnSelectRes)
    ON_COMMAND(ID_FILE_ADD_PROGRAM,             FileAccess_OnAddPgm)
    ON_COMMAND(ID_FILE_NEGATE,                  FileAccess_OnNegate)
    ON_COMMAND(ID_FILE_ADD,                     FileAccess_OnAddEntry)
    ON_COMMAND(ID_FILE_EDIT,                    FileAccess_OnEditEntry)
    ON_COMMAND(ID_FILE_REMOVE,                  FileAccess_OnRemoveEntry)

    ON_CONTROL(CBN_SELENDOK, ID_GROUP_COMBO,    ProgramGroups_OnSelectGrp)
    ON_COMMAND(ID_GROUP_ADD,                    ProgramGroups_OnCmdAddGrp)
    ON_COMMAND(ID_GROUP_ADD_PROGRAM,            ProgramGroups_OnCmdAddPgm)
    ON_COMMAND(ID_GROUP_REMOVE,                 ProgramGroups_OnCmdRemove)

    ON_COMMAND(ID_USER_ADD,                     UserAccounts_OnCmdAdd)
    ON_COMMAND(ID_USER_REMOVE,                  UserAccounts_OnCmdRemove)

END_MESSAGE_MAP()


//---------------------------------------------------------------------------
// Variables
//---------------------------------------------------------------------------


static CString BoxPage_ProgramName;


static const CString _AutoApplySettings = L"AutoApplySettings";


extern const WCHAR *_SettingChangeNotify;


static const CString Template_PlugPlayService = L"PlugPlayService";


//---------------------------------------------------------------------------
// Constructor
//---------------------------------------------------------------------------


CBoxPage::CBoxPage(ULONG type, const CString &BoxName,
                   ULONG GroupMsgId, ULONG TitleMsgId)
    : m_BoxName(BoxName)
{
    m_template_copy = NULL;

    //
    // construct template for specified type
    //

    m_type = type;

    const WCHAR *template_base = NULL;
    if (m_type == BoxPageAppearance)
        template_base = L"BOXTITLE_PAGE";
    if (m_type == BoxPageAutoDelete)
        template_base = L"AUTODELETE_PAGE";
    if (m_type == BoxPageDeleteCommand)
        template_base = L"DELETECOMMAND_PAGE";
    if (m_type == BoxPageProgramGroups)
        template_base = L"PGMGROUP_PAGE";
    if (m_type == BoxPageQuickRecover)
        template_base = L"QUICKRECOVER_PAGE";
    if (m_type == BoxPageAutoRecover)
        template_base = L"AUTORECOVER_PAGE";
    if (m_type == BoxPageForceFol)
        template_base = L"FORCEFOL_PAGE";
    if (m_type == BoxPageForcePgm)
        template_base = L"FORCEPGM_PAGE";
    if (m_type == BoxPageLingerPgm || m_type == BoxPageLeaderPgm)
        template_base = L"LINGERLEADER_PAGE";
    if (m_type == BoxPageFileMigrate)
        template_base = L"FILEMIGRATE_PAGE";
    if (m_type == BoxPageInternetPgm || m_type == BoxPageStartRunPgm)
        template_base = L"RESTRICTPGM_PAGE";
    if (m_type == BoxPageDropRights)
        template_base = L"DROPRIGHTS_PAGE";
    if (m_type == BoxPagePrintSpooler)
        template_base = L"PRINTSPOOLER_PAGE";
    if (m_type == BoxPageNetworkFiles)
        template_base = L"NETWORK_FILES_PAGE";
    if (m_type == BoxPageOpenFile ||
        m_type == BoxPageOpenPipe ||
        m_type == BoxPageReadFile ||
        m_type == BoxPageWriteFile ||
        m_type == BoxPageClosedFile ||
        m_type == BoxPageOpenKey ||
        m_type == BoxPageReadKey ||
        m_type == BoxPageWriteKey ||
        m_type == BoxPageClosedKey ||
        m_type == BoxPageOpenIpc ||
        m_type == BoxPageClosedIpc ||
        m_type == BoxPageWinClass ||
        m_type == BoxPageComClass)
        template_base = L"RESOURCE_ACCESS_PAGE";
    if (m_type == BoxPageUserAccounts) {
        template_base = L"USERACCOUNTS_PAGE";
        UserAccounts_restricted = false;
    }

    if (template_base) {

        SetPageTemplate(template_base);
    }

    if (TitleMsgId) {

        if (GroupMsgId)
            m_titleForTree = CMyMsg(GroupMsgId) + L"::";

        m_titleForPage = CMyMsg(TitleMsgId);
        m_titleForTree += m_titleForPage;

        SetTitleForTree();
    }

    m_font = NULL;

    //
    // init global and page variables
    //

    m_modified = FALSE;

    FileAccess_index = -1;
    ProgramGroups_index = -1;

    Appearance_Bitmap = NULL;
}


//---------------------------------------------------------------------------
// Destructor
//---------------------------------------------------------------------------


CBoxPage::~CBoxPage()
{
    if (Appearance_Bitmap)
        DeleteObject(Appearance_Bitmap);

    if (m_font)
        delete m_font;

    if (m_template_copy) {
        HeapFree(GetProcessHeap(), 0, m_template_copy);
        m_template_copy = NULL;
    }
}


//---------------------------------------------------------------------------
// SetPageTemplate
//---------------------------------------------------------------------------


void CBoxPage::SetPageTemplate(const WCHAR *TemplateName)
{
    m_template = TemplateName;
    CommonConstruct(m_template, 0);

    if (CMyApp::m_LayoutRTL) {

        m_template_copy =
            Common_DlgTmplRtl(AfxGetInstanceHandle(), m_psp.pszTemplate);

        if (m_template_copy) {
            m_psp.pResource = (LPCDLGTEMPLATE)m_template_copy;
            m_psp.dwFlags |= PSP_DLGINDIRECT;
        }
    }
}


//---------------------------------------------------------------------------
// AddBeforeBrackets
//---------------------------------------------------------------------------


void CBoxPage::AddBeforeBrackets(CListBox *pListBox, const CString &str)
{
    BOOL added = FALSE;
    int index_added;

    int size = pListBox->GetCount();
    for (int index = 0; index < size; ++index) {
        CString text;
        pListBox->GetText(index, text);
        if (text.GetLength() > 2 && text.GetAt(0) == L'[') {
            index_added = AddStringAndSetWidth(pListBox, str, index);
            added = TRUE;
            break;
        }
    }

    if (! added)
        index_added = AddStringAndSetWidth(pListBox, str);

    if (index_added >= 0)
        pListBox->SetCurSel(index_added);
}


//---------------------------------------------------------------------------
// CheckIfBrackets
//---------------------------------------------------------------------------


BOOL CBoxPage::CheckIfBrackets(CListBox *pListBox, int index,
                               const CString &SettingName,
                               const CString &Prefix)
{
    CString ListBoxValue;
    pListBox->GetText(index, ListBoxValue);
    if (ListBoxValue.GetLength() < 2 || ListBoxValue.GetAt(0) != L'[')
        return FALSE;
    ListBoxValue = ListBoxValue.Mid(1, ListBoxValue.GetLength() - 2);
    if (! Prefix.IsEmpty())
        ListBoxValue = Prefix + "," + ListBoxValue;

    CString msg;

    if (SettingName.CompareNoCase(CBox::_ProcessGroup) == 0) {

        msg = L"?";

    } else {

        CSbieIni &ini = CSbieIni::GetInstance();
        CBox &GlobalSettings = CBoxes::GetInstance().GetBox(0);
        CBox &box = CBoxes::GetInstance().GetBox(m_BoxName);

        CStringList allTemplates;
        ini.GetTemplateNames(L"*", allTemplates);

        POSITION pos = allTemplates.GetHeadPosition();
        while (pos) {
            CString nameTemplate = allTemplates.GetNext(pos);
            BOOL local  = FALSE;
            BOOL global = GlobalSettings.IsTemplateEnabled(nameTemplate);
            if (! global)
                 local = box.IsTemplateEnabled(nameTemplate);
            if (local || global) {

                BOOL found = FALSE;

                CString SectionName = ini.m_Template_ + nameTemplate;
                CStringList values;
                ini.GetTextList(SectionName, SettingName, values);
                while (! values.IsEmpty()) {

                    CString value = values.RemoveHead();
                    if (value.CompareNoCase(ListBoxValue) == 0) {
                        found = TRUE;
                        break;
                    }
                }

                if (found) {
                    if (! msg.IsEmpty())
                        msg += L"\r\n";
                    msg += L"\t";
                    msg += CTemplateListBox::GetTemplateTitle(nameTemplate);

                    CString ClassName =
                        CTemplateListBox::GetTemplateClass(nameTemplate);
                    if (ClassName.CompareNoCase(L"Local") == 0) {
                        msg += L" - ";
                        msg += CMyMsg(MSG_4218);
                    }

                    if (global) {
                        msg += L" - ";
                        msg += CMyMsg(MSG_4251);
                    }
                }
            }
        }
    }

    msg = CMyMsg(MSG_4222, msg);
    CMyApp::MsgBox(GetParent(), msg, MB_OK);
    return TRUE;
}


//---------------------------------------------------------------------------
// SetTitleForTree
//---------------------------------------------------------------------------


void CBoxPage::SetTitleForTree()
{
    m_titleForTree.Remove(L'\r');
    m_titleForTree.Remove(L'\n');
    m_psp.pszTitle = m_titleForTree;
    m_psp.dwFlags |= PSP_USETITLE;
}


//---------------------------------------------------------------------------
// AdjustFont
//---------------------------------------------------------------------------


void CBoxPage::AdjustFont()
{
    if (m_font)
        return;

    if (! m_font) {

        LOGFONT data;
        CFont *font1 = GetFont();
        if (font1->GetLogFont(&data)) {

            CDC *pCdc = GetDC();
            LONG NewHeight =
                -MulDiv(10, GetDeviceCaps(pCdc->m_hDC, LOGPIXELSY), 72);
            ReleaseDC(pCdc);
            if (abs(NewHeight) > abs(data.lfHeight)) {
                data.lfHeight = NewHeight;
                data.lfWidth = 0;
            }
            CFont *font2 = new CFont();
            if (font2->CreateFontIndirect(&data))
                m_font = font2;
            else
                delete font2;
        }
    }

    if (! m_font)
        return;

    CWnd *child = GetWindow(GW_CHILD);
    CWnd *next = child;
    while (1) {
        next->SetFont(m_font);
        next = next->GetWindow(GW_HWNDNEXT);
        if ((! next) || next == child)
            break;
    }
}


//---------------------------------------------------------------------------
// CenterControl
//---------------------------------------------------------------------------


void CBoxPage::CenterControl(int idCtrl)
{
    CWnd *ctrl = GetDlgItem(idCtrl);

    CRect dlgRc, ctrlRc;
    GetClientRect(&dlgRc);
    ctrl->GetWindowRect(&ctrlRc);
    ScreenToClient(&ctrlRc);

    CDC *dc = GetDC();

    CFont *oldFont = NULL;
    if (m_font)
        oldFont = dc->SelectObject(m_font);

    CString text;
    ctrl->GetWindowText(text);
    CSize sz = dc->GetTextExtent(text);

    ctrlRc.left = (dlgRc.right - sz.cx) / 2 - 16;
    ctrl->MoveWindow(ctrlRc);

    if (oldFont)
        dc->SelectObject(oldFont);

    ReleaseDC(dc);
}


//---------------------------------------------------------------------------
// AddStringAndSetWidth
//---------------------------------------------------------------------------


int CBoxPage::AddStringAndSetWidth(
    CListBox *pListBox, const CString &str, int index)
{
    if (! str.IsEmpty()) {

        int width = pListBox->GetHorizontalExtent();
        if (width == 0) {
            CRect rc;
            pListBox->GetClientRect(&rc);
            width = rc.right - GetSystemMetrics(SM_CXVSCROLL);
            pListBox->SetHorizontalExtent(width);
        }

        CDC *dc = pListBox->GetDC();
        CFont *newFont = pListBox->GetFont();
        CFont *oldFont = dc->SelectObject(newFont);
        CSize sz = dc->GetTextExtent(str);
        dc->SelectObject(oldFont);
        pListBox->ReleaseDC(dc);

        if (sz.cx > width)
            pListBox->SetHorizontalExtent(sz.cx + GetSystemMetrics(SM_CXVSCROLL));
    }

    if (index == -1)
        index = pListBox->AddString(str);
    else
        index = pListBox->InsertString(index, str);
    return index;
}


//---------------------------------------------------------------------------
// MakeLTR
//---------------------------------------------------------------------------


void CBoxPage::MakeLTR(UINT id)
{
    if (CMyApp::m_LayoutRTL) {
        const ULONG _flag = SWP_NOACTIVATE;
        CWnd *wnd = GetDlgItem(id);
        wnd->ModifyStyleEx(
            WS_EX_RIGHT | WS_EX_RTLREADING | WS_EX_LAYOUTRTL, 0, _flag);
        wnd->ModifyStyle(ES_RIGHT, 0, _flag);
    }
}


//---------------------------------------------------------------------------
// SelectUsers
//---------------------------------------------------------------------------


bool CBoxPage::SelectUsers(CStringList &users)
{
    //
    // open the built-in security object picker dialog box
    //

    HRESULT hr;
    IDsObjectPicker *pObjectPicker = NULL;

    hr = CoCreateInstance(CLSID_DsObjectPicker, NULL, CLSCTX_INPROC_SERVER,
                          IID_IDsObjectPicker, (void **)&pObjectPicker);
    if (FAILED(hr))
        return false;

    bool ok = false;

    DSOP_SCOPE_INIT_INFO ScopeInit;
    memzero(&ScopeInit, sizeof(DSOP_SCOPE_INIT_INFO));
    ScopeInit.cbSize = sizeof(DSOP_SCOPE_INIT_INFO);
    ScopeInit.flType = DSOP_SCOPE_TYPE_TARGET_COMPUTER
                     | DSOP_SCOPE_TYPE_UPLEVEL_JOINED_DOMAIN
                     | DSOP_SCOPE_TYPE_DOWNLEVEL_JOINED_DOMAIN;
    ScopeInit.flScope = DSOP_SCOPE_FLAG_STARTING_SCOPE
                      | DSOP_SCOPE_FLAG_DEFAULT_FILTER_USERS
                      | DSOP_SCOPE_FLAG_DEFAULT_FILTER_GROUPS;
    ScopeInit.FilterFlags.Uplevel.flBothModes = DSOP_FILTER_USERS
        | DSOP_FILTER_WELL_KNOWN_PRINCIPALS
        | DSOP_FILTER_BUILTIN_GROUPS | DSOP_FILTER_UNIVERSAL_GROUPS_SE
        | DSOP_FILTER_GLOBAL_GROUPS_SE | DSOP_FILTER_DOMAIN_LOCAL_GROUPS_SE;
    ScopeInit.FilterFlags.flDownlevel = DSOP_DOWNLEVEL_FILTER_USERS
        | DSOP_DOWNLEVEL_FILTER_LOCAL_GROUPS
        | DSOP_DOWNLEVEL_FILTER_GLOBAL_GROUPS;

    DSOP_INIT_INFO InitInfo;
    memzero(&InitInfo, sizeof(InitInfo));
    InitInfo.cbSize = sizeof(InitInfo);
    InitInfo.pwzTargetComputer = NULL;
    InitInfo.cDsScopeInfos = 1;
    InitInfo.aDsScopeInfos = &ScopeInit;
    InitInfo.flOptions = DSOP_FLAG_MULTISELECT;

    hr = pObjectPicker->Initialize(&InitInfo);

    if (SUCCEEDED(hr)) {

        IDataObject *pDataObject = NULL;

        hr = pObjectPicker->InvokeDialog(GetParent()->m_hWnd, &pDataObject);

        if (SUCCEEDED(hr) && pDataObject) {

            //
            // extract the results from the dialog box
            //

            FORMATETC formatEtc;
            formatEtc.cfFormat = (CLIPFORMAT)
                RegisterClipboardFormat(CFSTR_DSOP_DS_SELECTION_LIST);
            formatEtc.ptd = NULL;
            formatEtc.dwAspect = DVASPECT_CONTENT;
            formatEtc.lindex = -1;
            formatEtc.tymed = TYMED_HGLOBAL;

            STGMEDIUM stgMedium;
            hr = pDataObject->GetData(&formatEtc, &stgMedium);
            if (SUCCEEDED(hr)) {

                ULONG i;
                PDS_SELECTION_LIST pResults =
                    (PDS_SELECTION_LIST)GlobalLock(stgMedium.hGlobal);
                if (pResults) {

                    //
                    // collect the results from the dialog box
                    //

                    for (i = 0; i < pResults->cItems; i++) {

                        WCHAR *UserName = pResults->aDsSelection[i].pwzName;
                        users.AddTail(UserName);
                        ok = true;
                    }

                    GlobalUnlock(stgMedium.hGlobal);
                }
            }

            pDataObject->Release();
        }
    }

    pObjectPicker->Release();
    return ok;
}


//---------------------------------------------------------------------------
// OnInitDialog
//---------------------------------------------------------------------------


BOOL CBoxPage::OnInitDialog()
{
    AdjustFont();

    //
    // invoke the appropriate OnInitDialog function
    //

    CBox &box = CBoxes::GetInstance().GetBox(m_BoxName);

    if (m_type == BoxPageAppearance)
        Appearance_OnInitDialog(box);
    if (m_type == BoxPageQuickRecover)
        QuickRecover_OnInitDialog(box);
    if (m_type == BoxPageAutoRecover)
        AutoRecover_OnInitDialog(box);
    if (m_type == BoxPageAutoDelete)
        AutoDelete_OnInitDialog(box);
    if (m_type == BoxPageDeleteCommand)
        DeleteCommand_OnInitDialog(box);
    if (m_type == BoxPageProgramGroups)
        ProgramGroups_OnInitDialog(box);
    if (m_type == BoxPageFileMigrate)
        FileMigrate_OnInitDialog(box);
    if (m_type == BoxPageDropRights)
        DropRights_OnInitDialog(box);
    if (m_type == BoxPagePrintSpooler)
        PrintSpooler_OnInitDialog(box);
    if (m_type == BoxPageNetworkFiles)
        NetworkFiles_OnInitDialog(box);

    if (m_type == BoxPageForceFol ||
        m_type == BoxPageForcePgm ||
        m_type == BoxPageLingerPgm ||
        m_type == BoxPageLeaderPgm)
        ForceLinger_OnInitDialog(box);
    if (m_type == BoxPageInternetPgm || m_type == BoxPageStartRunPgm)
        RestrictPgm_OnInitDialog(box);

    if (m_type == BoxPageOpenFile ||
        m_type == BoxPageOpenPipe ||
        m_type == BoxPageReadFile ||
        m_type == BoxPageWriteFile ||
        m_type == BoxPageClosedFile ||
        m_type == BoxPageOpenKey ||
        m_type == BoxPageReadKey ||
        m_type == BoxPageWriteKey ||
        m_type == BoxPageClosedKey ||
        m_type == BoxPageOpenIpc ||
        m_type == BoxPageClosedIpc ||
        m_type == BoxPageWinClass ||
        m_type == BoxPageComClass)
        FileAccess_OnInitDialog(box);

    if (m_type == BoxPageUserAccounts)
        UserAccounts_OnInitDialog(box);

    if (m_type == BoxPageUnknown)
        Plugin_OnInitDialog(box);

    //
    // visual enhancements: add a group box rectangle around the edges
    // of the property page, and change the font on all child controls
    //

    if (! m_titleForPage.IsEmpty()) {

        if (! m_groupbox.m_hWnd) {

            CRect rc;
            GetClientRect(&rc);
            rc.left += 5;
            rc.top += 5;
            rc.right -= 5 * 2;
            rc.bottom -= 5 * 2;

            m_groupbox.Create(L"", WS_CHILD | WS_VISIBLE | BS_GROUPBOX, rc,
                             this, ID_PAGE_TITLE);

            if (m_font)
                m_groupbox.SetFont(m_font);
        }

        m_groupbox.SetWindowText(m_titleForPage);
    }

    //
    // finish
    //

    m_modified = FALSE;
    return TRUE;
}


//---------------------------------------------------------------------------
// OnKillActive
//---------------------------------------------------------------------------


BOOL CBoxPage::OnKillActive()
{
    if (m_modified) {
        TreePropSheet::CTreePropSheet *parent =
            (TreePropSheet::CTreePropSheet *)GetParent();
        if ((! parent->IsAutoApplyChecked()) && parent->IsSelChanging()) {
            CMyApp::MsgBox(GetParent(), MSG_3803, MB_OK);
            return FALSE;
        }
    }

    if (m_type == BoxPageDeleteCommand) {
        CBox &box = CBoxes::GetInstance().GetBox(m_BoxName);
        return DeleteCommand_OnKillActive(box);
    }

    return TRUE;
}


//---------------------------------------------------------------------------
// OnOK
//---------------------------------------------------------------------------


void CBoxPage::OnOK()
{
    if (! m_modified)
        return;

    CBox &box = CBoxes::GetInstance().GetBox(m_BoxName);

    if (m_type == BoxPageAppearance)
        Appearance_OnOK(box);
    if (m_type == BoxPageAutoDelete)
        AutoDelete_OnOK(box);
    if (m_type == BoxPageDeleteCommand)
        DeleteCommand_OnOK(box);
    if (m_type == BoxPageQuickRecover)
        QuickRecover_OnOK(box);
    if (m_type == BoxPageAutoRecover)
        AutoRecover_OnOK(box);
    if (m_type == BoxPageProgramGroups)
        ProgramGroups_OnOK(box);
    if (m_type == BoxPageFileMigrate)
        FileMigrate_OnOK(box);
    if (m_type == BoxPageDropRights)
        DropRights_OnOK(box);
    if (m_type == BoxPageNetworkFiles)
        NetworkFiles_OnOK(box);
    if (m_type == BoxPagePrintSpooler)
        PrintSpooler_OnOK(box);

    if (m_type == BoxPageForceFol ||
        m_type == BoxPageForcePgm ||
        m_type == BoxPageLingerPgm ||
        m_type == BoxPageLeaderPgm)
        ForceLinger_OnOK(box);
    if (m_type == BoxPageInternetPgm || m_type == BoxPageStartRunPgm)
        RestrictPgm_OnOK(box);

    if (m_type == BoxPageOpenFile ||
        m_type == BoxPageOpenPipe ||
        m_type == BoxPageReadFile ||
        m_type == BoxPageWriteFile ||
        m_type == BoxPageClosedFile ||
        m_type == BoxPageOpenKey ||
        m_type == BoxPageReadKey ||
        m_type == BoxPageWriteKey ||
        m_type == BoxPageClosedKey ||
        m_type == BoxPageOpenIpc ||
        m_type == BoxPageClosedIpc ||
        m_type == BoxPageWinClass ||
        m_type == BoxPageComClass)
        FileAccess_OnOK(box);

    if (m_type == BoxPageUserAccounts)
        UserAccounts_OnOK(box);

    if (m_type == BoxPageUnknown)
        Plugin_OnOK(box);

    m_modified = FALSE;
    SetModified(FALSE);
    QuerySiblings(0, 0);
}


//---------------------------------------------------------------------------
// OnModified
//---------------------------------------------------------------------------


void CBoxPage::OnModified()
{
    m_modified = TRUE;
    SetModified();
}


//---------------------------------------------------------------------------
// OnQuerySiblings
//---------------------------------------------------------------------------


LRESULT CBoxPage::OnQuerySiblings(WPARAM wParam, LPARAM lParam)
{
    OnInitDialog();
    return 0;
}


//---------------------------------------------------------------------------
// Appearance_OnInitDialog
//---------------------------------------------------------------------------


void CBoxPage::Appearance_OnInitDialog(CBox &box)
{
    GetDlgItem(ID_PAGE_LABEL_1)->SetWindowText(CMyMsg(MSG_3812));
    GetDlgItem(ID_HIDE_INDICATOR)->SetWindowText(CMyMsg(MSG_3815));
    GetDlgItem(ID_PAGE_LABEL_2)->SetWindowText(CMyMsg(MSG_3813));
    GetDlgItem(ID_BOXNAMETITLE)->SetWindowText(CMyMsg(MSG_3814));
    GetDlgItem(ID_PAGE_LABEL_3)->SetWindowText(CMyMsg(MSG_3816));
    GetDlgItem(ID_SHOW_BORDER)->SetWindowText(CMyMsg(MSG_3817));
    GetDlgItem(ID_BORDER_TITLE)->SetWindowText(CMyMsg(MSG_3819));

    CenterControl(ID_HIDE_INDICATOR);
    CenterControl(ID_BOXNAMETITLE);
    CenterControl(ID_SHOW_BORDER);
    CenterControl(ID_BORDER_TITLE);

    CButton *pCheckBox1 = (CButton *)GetDlgItem(ID_HIDE_INDICATOR);
    CButton *pCheckBox2 = (CButton *)GetDlgItem(ID_BOXNAMETITLE);

    UCHAR BoxNameTitle = box.GetBoxNameTitle();
    if (BoxNameTitle == 'y') {
        pCheckBox1->SetCheck(BST_UNCHECKED);
        pCheckBox2->SetCheck(BST_CHECKED);
    } else if (BoxNameTitle == '-') {
        pCheckBox1->SetCheck(BST_CHECKED);
        pCheckBox2->SetCheck(BST_UNCHECKED);
    } else {
        pCheckBox1->SetCheck(BST_UNCHECKED);
        pCheckBox2->SetCheck(BST_UNCHECKED);
    }

    BOOL title;
    int width;
    BOOL enabled = box.GetBorder(&Appearance_BorderColor, &title, &width);
    if (! enabled)
        GetDlgItem(ID_BORDER_COLOR)->ShowWindow(SW_HIDE);

    CEdit* edit = (CEdit*)GetDlgItem(ID_BORDER_WIDTH);
    edit->SetLimitText(3);
    CString str;
    str.Format(L"%d", width);
    edit->SetWindowText(str);

    Appearance_SetBorderColor();

    CButton *pCheckBox3 = (CButton *)GetDlgItem(ID_SHOW_BORDER);
    CButton *pCheckBox4 = (CButton *)GetDlgItem(ID_BORDER_TITLE);
    pCheckBox3->SetCheck(enabled ? BST_CHECKED : BST_UNCHECKED);
    pCheckBox4->SetCheck(title ? BST_CHECKED : BST_UNCHECKED);
}


//---------------------------------------------------------------------------
// Appearance_OnOK
//---------------------------------------------------------------------------


void CBoxPage::Appearance_OnOK(CBox &box)
{
    CButton *pCheckBox1 = (CButton *)GetDlgItem(ID_HIDE_INDICATOR);
    CButton *pCheckBox2 = (CButton *)GetDlgItem(ID_BOXNAMETITLE);

    UCHAR BoxNameTitle;
    if (pCheckBox1->GetCheck() == BST_CHECKED)
        BoxNameTitle = '-';
    else if (pCheckBox2->GetCheck() == BST_CHECKED)
        BoxNameTitle = 'y';
    else
        BoxNameTitle = 'n';
    BOOL ok = box.SetBoxNameTitle(BoxNameTitle);

    if (ok) {
        CButton *pCheckBox3 = (CButton *)GetDlgItem(ID_SHOW_BORDER);
        CButton *pCheckBox4 = (CButton *)GetDlgItem(ID_BORDER_TITLE);
        BOOL enable = (pCheckBox3->GetCheck() == BST_CHECKED ? TRUE : FALSE);
        BOOL title  = (pCheckBox4->GetCheck() == BST_CHECKED ? TRUE : FALSE);
        CString str;
        GetDlgItem(ID_BORDER_WIDTH)->GetWindowText(str);
        int width = _wtoi(str);
        ok = box.SetBorder(enable, Appearance_BorderColor, title, width);
    }

    if (ok)
        CBorderGuard::RefreshConf();
}


//---------------------------------------------------------------------------
// Appearance_OnCheckBox1
//---------------------------------------------------------------------------


void CBoxPage::Appearance_OnCheckBox1()
{
    ((CButton *)GetDlgItem(ID_BOXNAMETITLE))->SetCheck(BST_UNCHECKED);
    OnModified();
}


//---------------------------------------------------------------------------
// Appearance_OnCheckBox2
//---------------------------------------------------------------------------


void CBoxPage::Appearance_OnCheckBox2()
{
    ((CButton *)GetDlgItem(ID_HIDE_INDICATOR))->SetCheck(BST_UNCHECKED);
    OnModified();
}


//---------------------------------------------------------------------------
// Appearance_OnShowBorder
//---------------------------------------------------------------------------


void CBoxPage::Appearance_OnShowBorder()
{
    CWnd *pColor = GetDlgItem(ID_BORDER_COLOR);

    if (((CButton *)GetDlgItem(ID_SHOW_BORDER))->GetCheck() == BST_CHECKED) {
        pColor->ShowWindow(SW_SHOW);
        Appearance_OnBorderColor();
    } else {
        pColor->ShowWindow(SW_HIDE);
        ((CButton *)GetDlgItem(ID_BORDER_TITLE))->SetCheck(BST_UNCHECKED);
    }

    OnModified();
}


//---------------------------------------------------------------------------
// Appearance_OnBorderColor
//---------------------------------------------------------------------------


void CBoxPage::Appearance_OnBorderColor()
{
    CHOOSECOLOR cc;
    COLORREF colors[20];
    memzero(&colors, sizeof(colors));

    cc.lStructSize = sizeof(CHOOSECOLOR);
    cc.hwndOwner = GetParent()->m_hWnd;
    cc.hInstance = NULL;
    cc.rgbResult = Appearance_BorderColor;
    cc.lpCustColors = colors;
    cc.Flags = CC_RGBINIT | CC_PREVENTFULLOPEN | CC_SOLIDCOLOR;
    cc.lCustData = NULL;
    cc.lpfnHook = NULL;
    cc.lpTemplateName = NULL;
    BOOL ok = ChooseColor(&cc);

    if (ok) {

        Appearance_BorderColor = cc.rgbResult;
        Appearance_SetBorderColor();

        OnModified();
    }
}


//---------------------------------------------------------------------------
// Appearance_SetBorderColor
//---------------------------------------------------------------------------


void CBoxPage::Appearance_SetBorderColor()
{
    CDC *dcWindow = GetParent()->GetDC();
    HDC hdc = CreateCompatibleDC(dcWindow->m_hDC);

    HBITMAP hbitmap = CreateCompatibleBitmap(dcWindow->m_hDC, 64, 64);
    SelectObject(hdc, hbitmap);

    for (int x = 0; x < 63; ++x)
        for (int y = 0; y < 63; ++y)
            SetPixel(hdc, x, y, Appearance_BorderColor);

    DeleteDC(hdc);
    ReleaseDC(dcWindow);

    CButton *button = (CButton *)GetDlgItem(ID_BORDER_COLOR);
    button->SetBitmap(hbitmap);

    if (! Appearance_ColorButton.m_hWnd)
        Appearance_ColorButton.Init(this, ID_BORDER_COLOR, CMyMsg(MSG_3818));

    if (Appearance_Bitmap)
        DeleteObject(Appearance_Bitmap);
    Appearance_Bitmap = hbitmap;
}


//---------------------------------------------------------------------------
// Appearance_OnBorderTitle
//---------------------------------------------------------------------------


void CBoxPage::Appearance_OnBorderTitle()
{
    CButton *pButton = (CButton *)GetDlgItem(ID_BORDER_TITLE);
    if (pButton->GetCheck() == BST_CHECKED) {
        pButton = (CButton *)GetDlgItem(ID_SHOW_BORDER);
        if (pButton->GetCheck() == BST_UNCHECKED) {
            pButton->SetCheck(BST_CHECKED);
            Appearance_OnShowBorder();
        }
    }
    OnModified();
}


//---------------------------------------------------------------------------
// QuickRecover_OnInitDialog
//---------------------------------------------------------------------------


void CBoxPage::QuickRecover_OnInitDialog(CBox &box)
{
    GetDlgItem(ID_PAGE_LABEL_1)->SetWindowText(CMyMsg(MSG_3823));
    GetDlgItem(ID_RECOVER_ADD)->SetWindowText(CMyMsg(MSG_3358));
    GetDlgItem(ID_RECOVER_REMOVE)->SetWindowText(CMyMsg(MSG_3351));

    CStringList folders;
    box.GetQuickRecoveryFolders(folders, TRUE);

    MakeLTR(ID_RECOVER_FOLDERS);
    CListBox *pListBox = (CListBox *)GetDlgItem(ID_RECOVER_FOLDERS);
    pListBox->ResetContent();
    pListBox->SetFont(&CMyApp::m_fontFixed);

    CSbieIni &ini = CSbieIni::GetInstance();

    while (! folders.IsEmpty()) {
        CString folder = folders.RemoveHead();
        BOOL withBrackets = CSbieIni::StripBrackets(folder);
        folder = ini.MakeSpecificPath(folder);
        if (withBrackets)
            folder = CSbieIni::AddBrackets(folder);
        AddStringAndSetWidth(pListBox, folder);
    }
}


//---------------------------------------------------------------------------
// QuickRecover_OnOK
//---------------------------------------------------------------------------


void CBoxPage::QuickRecover_OnOK(CBox &box)
{
    POSITION pos;
    CStringList folders;
    box.GetQuickRecoveryFolders(folders);

    int index;
    CListBox *pListBox = (CListBox *)GetDlgItem(ID_RECOVER_FOLDERS);
    int size = pListBox->GetCount();

    CSbieIni &ini = CSbieIni::GetInstance();

    //
    // add any listbox items that are not in RecoverFolder
    //

    for (index = 0; index < size; ++index) {

        CString newFolder;
        pListBox->GetText(index, newFolder);
        newFolder = ini.MakeGenericPath(newFolder);

        BOOL found = FALSE;
        pos = folders.GetHeadPosition();
        while (pos) {
            CString &oldFolder = folders.GetNext(pos);
            if (oldFolder.CompareNoCase(newFolder) == 0) {
                found = TRUE;
                break;
            }
        }

        if (! found) {
            BOOL ok = box.AddOrRemoveQuickRecoveryFolder(newFolder, TRUE);
            if (! ok)
                return;
        }
    }

    //
    // remove any RecoverFolder items that are not in listbox
    //

    pos = folders.GetHeadPosition();
    while (pos) {

        CString oldFolder = folders.GetNext(pos);
        oldFolder = ini.MakeGenericPath(oldFolder);

        for (index = 0; index < size; ++index) {
            CString newFolder;
            pListBox->GetText(index, newFolder);
            newFolder = ini.MakeGenericPath(newFolder);
            if (oldFolder.CompareNoCase(newFolder) == 0)
                break;
        }

        if (index == size) {
            BOOL ok = box.AddOrRemoveQuickRecoveryFolder(oldFolder, FALSE);
            if (! ok)
                return;
        }
    }
}


//---------------------------------------------------------------------------
// QuickRecover_OnCmdAdd
//---------------------------------------------------------------------------


void CBoxPage::QuickRecover_OnCmdAdd()
{
    WCHAR path[MAX_PATH + 32];

    CString title;
    if (m_type == BoxPageQuickRecover)
        title = CMyMsg(MSG_3726);
    else if (m_type == BoxPageAutoRecover)
        title = CMyMsg(MSG_3836);
    else
        title = "?";

    BROWSEINFO bi;
    memzero(&bi, sizeof(BROWSEINFO));
    bi.hwndOwner = m_hWnd;
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

            CListBox *pListBox = (CListBox *)GetDlgItem(ID_RECOVER_FOLDERS);
            AddStringAndSetWidth(pListBox, path);
            OnModified();
        }
    }
}


//---------------------------------------------------------------------------
// QuickRecover_OnCmdRemove
//---------------------------------------------------------------------------


void CBoxPage::QuickRecover_OnCmdRemove()
{
    CListBox *pListBox = (CListBox *)GetDlgItem(ID_RECOVER_FOLDERS);
    int index = pListBox->GetCurSel();
    if (index != LB_ERR) {

        const CString *SettingName = NULL;
        if (m_type == BoxPageQuickRecover)
            SettingName = &CBox::_RecoverFolder;
        if (m_type == BoxPageAutoRecover)
            SettingName = &CBox::_AutoRecoverIgnore;

        if (CheckIfBrackets(pListBox, index, *SettingName))
            return;

        pListBox->DeleteString(index);
        OnModified();
    }
}


//---------------------------------------------------------------------------
// AutoRecover_OnInitDialog
//---------------------------------------------------------------------------


void CBoxPage::AutoRecover_OnInitDialog(CBox &box)
{
    GetDlgItem(ID_PAGE_LABEL_1)->SetWindowText(CMyMsg(MSG_3832));
    GetDlgItem(ID_RECOVER_AUTO)->SetWindowText(CMyMsg(MSG_3833));
    GetDlgItem(ID_PAGE_LABEL_2)->SetWindowText(CMyMsg(MSG_3834));
    GetDlgItem(ID_RECOVER_ADD_FOLDER)->SetWindowText(CMyMsg(MSG_3358));
    GetDlgItem(ID_RECOVER_ADD_TYPE)->SetWindowText(CMyMsg(MSG_3835));
    GetDlgItem(ID_RECOVER_REMOVE)->SetWindowText(CMyMsg(MSG_3351));

    CenterControl(ID_RECOVER_AUTO);
    MakeLTR(ID_RECOVER_FOLDERS);

    BOOL enabled = box.GetImmediateRecoveryState();
    CButton *button = (CButton *)GetDlgItem(ID_RECOVER_AUTO);
    button->SetCheck(enabled ? BST_CHECKED : BST_UNCHECKED);

    CStringList items;
    box.GetAutoRecoveryIgnores(items, TRUE);

    CListBox *pListBox = (CListBox *)GetDlgItem(ID_RECOVER_FOLDERS);
    pListBox->ResetContent();
    pListBox->SetFont(&CMyApp::m_fontFixed);

    while (! items.IsEmpty()) {
        CString str = items.RemoveHead();
        AddStringAndSetWidth(pListBox, str);
    }
}


//---------------------------------------------------------------------------
// AutoRecover_OnOK
//---------------------------------------------------------------------------


void CBoxPage::AutoRecover_OnOK(CBox &box)
{
    POSITION pos;
    CStringList folders;
    box.GetAutoRecoveryIgnores(folders);

    BOOL ok = TRUE;

    int index;
    CListBox *pListBox = (CListBox *)GetDlgItem(ID_RECOVER_FOLDERS);
    int size = pListBox->GetCount();

    //
    // add any listbox items that are not in RecoverFolder
    //


    for (index = 0; (index < size) && ok; ++index) {

        CString newFolder;
        pListBox->GetText(index, newFolder);

        BOOL found = FALSE;
        pos = folders.GetHeadPosition();
        while (pos) {
            CString &oldFolder = folders.GetNext(pos);
            if (oldFolder.CompareNoCase(newFolder) == 0) {
                found = TRUE;
                break;
            }
        }

        if (! found)
            ok = box.AddOrRemoveAutoRecoveryIgnores(newFolder, TRUE);
    }

    //
    // remove any RecoverFolder items that are not in listbox
    //

    pos = folders.GetHeadPosition();
    while (pos && ok) {

        CString &oldFolder = folders.GetNext(pos);

        for (index = 0; index < size; ++index) {
            CString newFolder;
            pListBox->GetText(index, newFolder);
            if (oldFolder.CompareNoCase(newFolder) == 0)
                break;
        }

        if (index == size)
            ok = box.AddOrRemoveAutoRecoveryIgnores(oldFolder, FALSE);
    }

    //
    // enable/disable immediate recovery
    //

    if (ok) {

        BOOL enabled = FALSE;
        CButton *button = (CButton *)GetDlgItem(ID_RECOVER_AUTO);
        if (button->GetCheck() == BST_CHECKED)
            enabled = TRUE;
        box.SetImmediateRecoveryState(enabled, FALSE);
    }
}


//---------------------------------------------------------------------------
// AutoRecover_OnCmdAddFolder
//---------------------------------------------------------------------------


void CBoxPage::AutoRecover_OnCmdAddFolder()
{
    QuickRecover_OnCmdAdd();
}


//---------------------------------------------------------------------------
// AutoRecover_OnCmdAddType
//---------------------------------------------------------------------------


void CBoxPage::AutoRecover_OnCmdAddType()
{
    CString ext = CMyApp::InputBox(GetParent(), MSG_3837);
    ext.Remove(L'[');
    ext.Remove(L']');
    ext.Remove(L' ');
    if (! ext.IsEmpty()) {
        if (ext.Find(L'.') == -1)
            ext = L'.' + ext;
        CListBox *pListBox = (CListBox *)GetDlgItem(ID_RECOVER_FOLDERS);
        AddStringAndSetWidth(pListBox, ext);
        OnModified();
    }
}


//---------------------------------------------------------------------------
// AutoDelete_OnInitDialog
//---------------------------------------------------------------------------


void CBoxPage::AutoDelete_OnInitDialog(CBox &box)
{
    GetDlgItem(ID_PAGE_LABEL_1)->SetWindowText(CMyMsg(MSG_3843));
    GetDlgItem(ID_DELETE_AUTO)->SetWindowText(CMyMsg(MSG_3844));
    GetDlgItem(ID_PAGE_LABEL_2)->SetWindowText(CMyMsg(MSG_3845));
    GetDlgItem(ID_PAGE_LABEL_3)->SetWindowText(CMyMsg(MSG_3846));
    GetDlgItem(ID_DELETE_NEVER)->SetWindowText(CMyMsg(MSG_3847));

    CenterControl(ID_DELETE_AUTO);
    CenterControl(ID_DELETE_NEVER);

    CButton *pCheckBox = (CButton *)GetDlgItem(ID_DELETE_AUTO);
    pCheckBox->SetCheck(box.GetAutoDelete() ? BST_CHECKED : BST_UNCHECKED);
    pCheckBox = (CButton *)GetDlgItem(ID_DELETE_NEVER);
    pCheckBox->SetCheck(box.GetNeverDelete() ? BST_CHECKED : BST_UNCHECKED);
}


//---------------------------------------------------------------------------
// AutoDelete_OnOK
//---------------------------------------------------------------------------


void CBoxPage::AutoDelete_OnOK(CBox &box)
{
    BOOL autoDelete = FALSE;
    BOOL neverDelete = FALSE;

    CButton *pCheckBox = (CButton *)GetDlgItem(ID_DELETE_NEVER);
    if (pCheckBox->GetCheck() == BST_CHECKED)
        neverDelete = TRUE;
    else {
        pCheckBox = (CButton *)GetDlgItem(ID_DELETE_AUTO);
        if (pCheckBox->GetCheck() == BST_CHECKED)
            autoDelete = TRUE;
    }

    box.SetAutoDelete(autoDelete);
    box.SetNeverDelete(neverDelete);
}


//---------------------------------------------------------------------------
// AutoDelete_OnAuto
//---------------------------------------------------------------------------


void CBoxPage::AutoDelete_OnAuto()
{
    ((CButton *)GetDlgItem(ID_DELETE_NEVER))->SetCheck(BST_UNCHECKED);
    OnModified();
}


//---------------------------------------------------------------------------
// AutoDelete_OnNever
//---------------------------------------------------------------------------


void CBoxPage::AutoDelete_OnNever()
{
    ((CButton *)GetDlgItem(ID_DELETE_AUTO))->SetCheck(BST_UNCHECKED);
    OnModified();
}

//---------------------------------------------------------------------------
// DeleteCommand_OnInitDialog
//---------------------------------------------------------------------------


void CBoxPage::DeleteCommand_OnInitDialog(CBox &box)
{
    GetDlgItem(ID_PAGE_LABEL_1)->SetWindowText(CMyMsg(MSG_3853));
    GetDlgItem(ID_PAGE_LABEL_2)->SetWindowText(CMyMsg(MSG_3854));
    GetDlgItem(ID_PAGE_LABEL_3)->SetWindowText(CMyMsg(MSG_3855));

    MakeLTR(ID_DELETE_COMMAND);

    CString command = box.GetDeleteCommand();
    if (! command.IsEmpty())
        GetDlgItem(ID_DELETE_COMMAND)->SetWindowText(command);

    GetDlgItem(ID_DELETE_COMMAND)->SetFont(&CMyApp::m_fontFixed);
}


//---------------------------------------------------------------------------
// DeleteCommand_OnKillActive
//---------------------------------------------------------------------------


BOOL CBoxPage::DeleteCommand_OnKillActive(CBox &box)
{
    CString command;
    GetDlgItem(ID_DELETE_COMMAND)->GetWindowText(command);

    if (! command.IsEmpty()) {
        CString cmd = command;
        cmd.MakeLower();
        int index1 = cmd.Find(L"\"%sandbox%\"");
        int index2 = cmd.Find(L"\"%sandbox\\\\%\"");
        if (index1 == -1 && index2 == -1) {
            int rc = CMyApp::MsgBox(this, MSG_3856, MB_YESNO);
            if (rc != IDYES)
                return FALSE;
        }
    }

    return TRUE;
}


//---------------------------------------------------------------------------
// DeleteCommand_OnOK
//---------------------------------------------------------------------------


void CBoxPage::DeleteCommand_OnOK(CBox &box)
{
    CString command;
    GetDlgItem(ID_DELETE_COMMAND)->GetWindowText(command);
    box.SetDeleteCommand(command);
}


//---------------------------------------------------------------------------
// DeleteCommand_RmDir
//---------------------------------------------------------------------------


void CBoxPage::DeleteCommand_RmDir()
{
    const WCHAR *cmd =
        L"%SystemRoot%\\System32\\cmd.exe /c RMDIR /s /q \"%SANDBOX%\"";
    GetDlgItem(ID_DELETE_COMMAND)->SetWindowText(cmd);
}


//---------------------------------------------------------------------------
// DeleteCommand_SDelete
//---------------------------------------------------------------------------


void CBoxPage::DeleteCommand_SDelete()
{
    const WCHAR *PgmName = L"sdelete.exe";
    const WCHAR *PgmArgs =
        L"-p 3 -s -q \"%SANDBOX%\"";
    DeleteCommand_SetDeleteProgram(PgmName, PgmArgs);
}


//---------------------------------------------------------------------------
// DeleteCommand_EraserL
//---------------------------------------------------------------------------


void CBoxPage::DeleteCommand_EraserL()
{
    const WCHAR *PgmName = L"eraserl.exe";
    const WCHAR *PgmArgs =
        L"-folder \"%SANDBOX%\" -subfolders "
        L"-method DoD_E -resultsonerror -queue";
    DeleteCommand_SetDeleteProgram(PgmName, PgmArgs);
}


//---------------------------------------------------------------------------
// DeleteCommand_Eraser6
//---------------------------------------------------------------------------


void CBoxPage::DeleteCommand_Eraser6()
{
    const WCHAR *PgmName = L"eraser.exe";
    const WCHAR *PgmArgs =
        L"addtask --quiet --method=ecbf4998-0b4f-445c-9a06-23627659e419 "
        L"--schedule=now --dir=\"%SANDBOX\\\\%\"";
    DeleteCommand_SetDeleteProgram(PgmName, PgmArgs);
}


//---------------------------------------------------------------------------
// DeleteCommand_SetDeleteProgram
//---------------------------------------------------------------------------


void CBoxPage::DeleteCommand_SetDeleteProgram(
    const WCHAR *PgmName, const WCHAR *PgmArgs)
{
    CString pgm1;

    while (1) {

        pgm1 = CString();

        WCHAR buf[MAX_PATH + 8];
        wcscpy(buf, PgmName);

        WCHAR filter[64];
        wcscpy(filter, CMyMsg(MSG_3308));
        while (1) {
            WCHAR *ptr = wcsrchr(filter, L'#');
            if (! ptr)
                break;
            *ptr = L'\0';
        }

        CString title = CMyMsg(MSG_3857, PgmName);

        OPENFILENAME ofn;
        memzero(&ofn, sizeof(OPENFILENAME));
        ofn.lStructSize  = sizeof(OPENFILENAME);
        ofn.hwndOwner    = GetParent()->m_hWnd;
        ofn.nFilterIndex = 1;
        ofn.lpstrFilter  = filter;
        ofn.lpstrFile    = buf;
        ofn.nMaxFile     = MAX_PATH;
        ofn.lpstrTitle   = title;
        ofn.Flags        = OFN_DONTADDTORECENT
                         | OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
        if (! GetOpenFileName(&ofn))
            return;

        pgm1 = buf;
        int index = pgm1.ReverseFind(L'\\');
        if (index != -1)
            pgm1 = pgm1.Mid(index + 1);
        pgm1.MakeLower();

        if (pgm1.CompareNoCase(PgmName) == 0) {
            pgm1 = buf;
            break;
        }
    }

    CString cmd = L"\"" + pgm1 + L"\" " + PgmArgs;
    GetDlgItem(ID_DELETE_COMMAND)->SetWindowText(cmd);
}


//---------------------------------------------------------------------------
// ProgramGroups_OnInitDialog
//---------------------------------------------------------------------------


void CBoxPage::ProgramGroups_OnInitDialog(CBox &box)
{
    MakeLTR(ID_GROUP_LIST);

    GetDlgItem(ID_PAGE_LABEL_1)->SetWindowText(CMyMsg(MSG_3972));

    GetDlgItem(ID_GROUP_COMBO_TEXT)->SetWindowText(CMyMsg(MSG_3806));
    GetDlgItem(ID_GROUP_COMBO_TEXT)->SetFont(GetFont());

    GetDlgItem(ID_GROUP_ADD)->SetWindowText(CMyMsg(MSG_3974));
    GetDlgItem(ID_GROUP_ADD_PROGRAM)->SetWindowText(CMyMsg(MSG_3355));
    GetDlgItem(ID_GROUP_REMOVE)->SetWindowText(CMyMsg(MSG_3351));

    CComboBox *pCombo = (CComboBox *)GetDlgItem(ID_GROUP_COMBO);
    pCombo->SetFont(&CMyApp::m_fontFixed);
    GetDlgItem(ID_GROUP_LIST)->SetFont(&CMyApp::m_fontFixed);

    //
    // populate combobox1 with group names
    //

    CStringList groups;
    box.GetProcessGroupsInBox(groups);

    if (groups.IsEmpty()) {
        GetDlgItem(ID_GROUP_ADD_PROGRAM)->EnableWindow(FALSE);
        GetDlgItem(ID_GROUP_REMOVE)->EnableWindow(FALSE);
    }

    CString oldgrp;
    if (ProgramGroups_index != -1)
        pCombo->GetLBText(ProgramGroups_index, oldgrp);

    pCombo->ResetContent();

    while (! groups.IsEmpty()) {
        CString grp = groups.RemoveHead();
        pCombo->AddString(grp);
    }

    //
    // make sure we re-select the right entry, when Apply was clicked
    //

    if (ProgramGroups_index == -1) {

        pCombo->SetCurSel(0);

    } else {

        int size = pCombo->GetCount();
        int index = 0;
        for (index = 0; index < size; ++index) {
            CString grp;
            pCombo->GetLBText(index, grp);
            if (oldgrp.CompareNoCase(grp) == 0)
                break;
        }

        if (index == size)
            index = 0;

        pCombo->SetCurSel(index);
    }

    ((CListBox *)GetDlgItem(ID_GROUP_LIST))->ResetContent();

    ProgramGroups_OnSelectGrp();
}


//---------------------------------------------------------------------------
// ProgramGroups_OnOK
//---------------------------------------------------------------------------


void CBoxPage::ProgramGroups_OnOK(CBox &box)
{
    CString grp;
    CComboBox *pCombo = (CComboBox *)GetDlgItem(ID_GROUP_COMBO);
    int index = pCombo->GetCurSel();
    if (index != CB_ERR)
        pCombo->GetLBText(index, grp);
    if (grp.IsEmpty())
        grp = L"<Err>";

    CString pgms;
    CListBox *pList = (CListBox *)GetDlgItem(ID_GROUP_LIST);
    int size = pList->GetCount();
    for (int index = 0; index < size; ++index) {
        CString str;
        pList->GetText(index, str);
        if (str.IsEmpty() || str.GetAt(0) == L'[')
            continue;
        if (! pgms.IsEmpty())
            pgms += L",";
        pgms += str;
    }

    CString xInternetAccessGroup = L"<" + CBox::_InternetAccessGroup + L">";
    CString xStartRunAccessGroup = L"<" + CBox::_StartRunAccessGroup + L">";
    if (grp.CompareNoCase(xInternetAccessGroup) == 0)
        box.SetInternetProcess(pgms);
    else if (grp.CompareNoCase(xStartRunAccessGroup) == 0)
        box.SetStartRunProcess(pgms);
    else
        box.SetProcessGroup(grp, pgms);
}


//---------------------------------------------------------------------------
// ProgramGroups_OnSelectGrp
//---------------------------------------------------------------------------


void CBoxPage::ProgramGroups_OnSelectGrp()
{
    CComboBox *pCombo = (CComboBox *)GetDlgItem(ID_GROUP_COMBO);

    if (m_modified) {
        pCombo->SetCurSel(ProgramGroups_index);
        CMyApp::MsgBox(GetParent(), MSG_3803, MB_OK);
        return;
    }

    CString grp;
    int index = pCombo->GetCurSel();
    if (index == CB_ERR)
        return;
    pCombo->GetLBText(index, grp);
    ProgramGroups_index = index;

    ProgramGroups_SwitchGrp(grp);
}


//---------------------------------------------------------------------------
// ProgramGroups_OnCmdAddGrp
//---------------------------------------------------------------------------


void CBoxPage::ProgramGroups_OnCmdAddGrp()
{
    CComboBox *pCombo = (CComboBox *)GetDlgItem(ID_GROUP_COMBO);

    if (m_modified) {
        pCombo->SetCurSel(ProgramGroups_index);
        CMyApp::MsgBox(GetParent(), MSG_3803, MB_OK);
        return;
    }

    CString grp = CMyApp::InputBox(GetParent(), MSG_3975);
    grp.TrimLeft(L"[]<> \t\r\n");
    grp.TrimRight(L"[]<> \t\r\n");
    if (grp.IsEmpty())
        return;
    grp = L"<" + grp + L">";

    bool found = false;

    int size = pCombo->GetCount();
    for (int index = 0; index < size; ++index) {
        CString grp1;
        pCombo->GetLBText(index, grp1);
        if (grp1.CompareNoCase(grp) == 0) {
            pCombo->SetCurSel(index);
            found = true;
            break;
        }
    }

    if (! found) {
        ProgramGroups_index = pCombo->AddString(grp);
        pCombo->SetCurSel(ProgramGroups_index);
    }

    ProgramGroups_SwitchGrp(grp);

    GetDlgItem(ID_GROUP_ADD_PROGRAM)->EnableWindow(TRUE);
    GetDlgItem(ID_GROUP_REMOVE)->EnableWindow(TRUE);
}


//---------------------------------------------------------------------------
// ProgramGroups_SwitchGrp
//---------------------------------------------------------------------------


void CBoxPage::ProgramGroups_SwitchGrp(const CString &grp)
{
    CListBox *pListBox = (CListBox *)GetDlgItem(ID_GROUP_LIST);
    pListBox->ResetContent();

    CBox &box = CBoxes::GetInstance().GetBox(m_BoxName);
    CString pgms = box.GetProcessGroup(grp);

    if (! pgms.IsEmpty()) {

        int i = 0;
        while (i < pgms.GetLength()) {
            int j = pgms.Find(L',', i);
            if (j == -1)
                j = pgms.GetLength();
            CString token = pgms.Mid(i, j - i);
            token.TrimLeft();
            token.TrimRight();
            AddStringAndSetWidth(pListBox, token);
            i = j + 1;
        }
    }
}


//---------------------------------------------------------------------------
// ProgramGroups_OnCmdAddPgm
//---------------------------------------------------------------------------


void CBoxPage::ProgramGroups_OnCmdAddPgm()
{
    CComboBox *pCombo = (CComboBox *)GetDlgItem(ID_GROUP_COMBO);
    CString groupName;
    int index = pCombo->GetCurSel();
    if (index == CB_ERR)
        return;
    pCombo->GetLBText(index, groupName);

    CBox &box = CBoxes::GetInstance().GetBox(m_BoxName);
    CProgramSelector pgmsel(this, &box, true, groupName);

    CString &pgm1 = pgmsel.m_pgm1;
    CString &pgm2 = pgmsel.m_pgm2;

    if (! pgm1.IsEmpty()) {

        CListBox *pListBox = (CListBox *)GetDlgItem(ID_GROUP_LIST);
        pListBox->SetCurSel(-1);

        AddBeforeBrackets(pListBox, pgm1);
        if (! pgm2.IsEmpty())
            AddBeforeBrackets(pListBox, pgm2);

        OnModified();
    }
}


//---------------------------------------------------------------------------
// ProgramGroups_OnCmdRemove
//---------------------------------------------------------------------------


void CBoxPage::ProgramGroups_OnCmdRemove()
{
    CListBox *pListBox = (CListBox *)GetDlgItem(ID_GROUP_LIST);
    int index = pListBox->GetCurSel();
    if (index == LB_ERR)
        return;

    if (CheckIfBrackets(pListBox, index, CBox::_ProcessGroup))
        return;

    pListBox->SetCurSel(-1);
    pListBox->DeleteString(index);
    OnModified();
}


//---------------------------------------------------------------------------
// ForceLinger_OnInitDialog
//---------------------------------------------------------------------------


void CBoxPage::ForceLinger_OnInitDialog(CBox &box)
{
    CWnd *pWnd;

    MakeLTR(ID_PROGRAM_LIST);

    if (m_type == BoxPageForceFol) {

        GetDlgItem(ID_PAGE_LABEL_1)->SetWindowText(CMyMsg(MSG_3863));
        GetDlgItem(ID_PAGE_LABEL_2)->SetWindowText(CMyMsg(MSG_3864));
    }

    if (m_type == BoxPageForcePgm) {

        GetDlgItem(ID_PAGE_LABEL_1)->SetWindowText(CMyMsg(MSG_3873));
    }

    if (m_type == BoxPageLingerPgm) {

        GetDlgItem(ID_PAGE_LABEL_1)->SetWindowText(CMyMsg(MSG_3883));
    }

    if (m_type == BoxPageLeaderPgm) {

        GetDlgItem(ID_PAGE_LABEL_1)->SetWindowText(CMyMsg(MSG_3893));
    }

    pWnd = GetDlgItem(ID_PROGRAM_ADD_FOLDER);
    if (pWnd)
        pWnd->SetWindowText(CMyMsg(MSG_3358));

    pWnd = GetDlgItem(ID_PROGRAM_ADD);
    if (pWnd)
        pWnd->SetWindowText(CMyMsg(MSG_3355));

    pWnd = GetDlgItem(ID_PROGRAM_REMOVE);
    if (pWnd)
        pWnd->SetWindowText(CMyMsg(MSG_3351));

    GetDlgItem(ID_PROGRAM_LIST)->SetFont(&CMyApp::m_fontFixed);

    WCHAR type = L'?';
    if (m_type == BoxPageForceFol)
        type = L'O';
    if (m_type == BoxPageForcePgm)
        type = L'F';
    if (m_type == BoxPageLingerPgm)
        type = L'L';
    if (m_type == BoxPageLeaderPgm)
        type = L'R';

    CStringList programs;
    box.GetProcessList(type, programs, TRUE);

    CListBox *pListBox = (CListBox *)GetDlgItem(ID_PROGRAM_LIST);
    pListBox->ResetContent();

    while (! programs.IsEmpty()) {
        CString str = programs.RemoveHead();
        AddStringAndSetWidth(pListBox, str);
    }
}


//---------------------------------------------------------------------------
// ForceLinger_OnOK
//---------------------------------------------------------------------------


void CBoxPage::ForceLinger_OnOK(CBox &box)
{
    WCHAR type = L'?';
    if (m_type == BoxPageForceFol)
        type = L'O';
    if (m_type == BoxPageForcePgm)
        type = L'F';
    if (m_type == BoxPageLingerPgm)
        type = L'L';
    if (m_type == BoxPageLeaderPgm)
        type = L'R';

    BOOL ok = TRUE;

    POSITION pos;
    CStringList programs;
    box.GetProcessList(type, programs);

    int index;
    CListBox *pListBox = (CListBox *)GetDlgItem(ID_PROGRAM_LIST);
    int size = pListBox->GetCount();

    //
    // add any listbox items that are not in RecoverFolder
    //

    for (index = 0; (index < size) && ok; ++index) {

        CString newProgram;
        pListBox->GetText(index, newProgram);

        BOOL found = FALSE;
        pos = programs.GetHeadPosition();
        while (pos) {
            CString &oldProgram = programs.GetNext(pos);
            if (oldProgram.CompareNoCase(newProgram) == 0) {
                found = TRUE;
                break;
            }
        }

        if (! found)
            ok = box.SetProcessSetting(type, newProgram, TRUE);
    }

    //
    // remove any RecoverFolder items that are not in listbox
    //

    pos = programs.GetHeadPosition();
    while (pos && ok) {

        CString &oldProgram = programs.GetNext(pos);

        BOOL found = FALSE;
        for (index = 0; index < size; ++index) {
            CString newProgram;
            pListBox->GetText(index, newProgram);
            if (oldProgram.CompareNoCase(newProgram) == 0) {
                found = TRUE;
                break;
            }
        }

        if (! found)
            ok = box.SetProcessSetting(type, oldProgram, FALSE);
    }
}


//---------------------------------------------------------------------------
// ForceLinger_OnCmdAddPgm
//---------------------------------------------------------------------------


void CBoxPage::ForceLinger_OnCmdAddPgm()
{
    CString groupName;
    if (m_type == BoxPageInternetPgm)
        groupName = L"<" + CBox::_InternetAccessGroup + L">";
    else if (m_type == BoxPageStartRunPgm)
        groupName = L"<" + CBox::_StartRunAccessGroup + L">";

    CBox &box = CBoxes::GetInstance().GetBox(m_BoxName);
    CProgramSelector pgmsel(this, &box, true, groupName);

    CString &pgm1 = pgmsel.m_pgm1;
    CString &pgm2 = pgmsel.m_pgm2;
    if (! pgm1.IsEmpty())
        ForceLinger_OnCmdAddPgm2(pgm1);
    if (! pgm2.IsEmpty())
        ForceLinger_OnCmdAddPgm2(pgm2);
}


//---------------------------------------------------------------------------
// ForceLinger_OnCmdAddPgm2
//---------------------------------------------------------------------------


void CBoxPage::ForceLinger_OnCmdAddPgm2(const CString &pgm)
{
    if (m_type == BoxPageForcePgm &&
            ForceLinger_MatchPattern(pgm, _ExplorerExe)) {

        CString msg = CMyMsg(MSG_3775) + L"\n" + CMyMsg(MSG_3772, pgm);
        int rv = CMyApp::MsgBox(this, msg, MB_YESNO);
        if (rv != IDYES)
            return;
    }

    CListBox *pListBox = (CListBox *)GetDlgItem(ID_PROGRAM_LIST);
    AddStringAndSetWidth(pListBox, pgm);

    if (m_type == BoxPageInternetPgm || m_type == BoxPageStartRunPgm) {

        CString str;
        pListBox->GetText(0, str);

        if (str == m_accAll || str == m_accNone)
            pListBox->DeleteString(0);
    }

    OnModified();
}


//---------------------------------------------------------------------------
// ForceLinger_OnCmdAddFolder
//---------------------------------------------------------------------------


void CBoxPage::ForceLinger_OnCmdAddFolder()
{
    BOOL ok = FALSE;

    //
    // invoke Browse For Folder dialog box
    //

    WCHAR path[MAX_PATH + 32];
    ULONG len_path;

    CMyMsg title(MSG_3866);

    BROWSEINFO bi;
    memzero(&bi, sizeof(BROWSEINFO));
    bi.hwndOwner = GetParent()->m_hWnd;
    bi.pszDisplayName = path;
    bi.lpszTitle = title;
    bi.ulFlags = BIF_UAHINT | BIF_VALIDATE | BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE | BIF_EDITBOX;
    bi.lpfn = ForceLinger_OnCmdAddFolder_Callback;
    bi.lParam = (LPARAM)this;
    LPITEMIDLIST pidl = SHBrowseForFolder(&bi);

    if (pidl) {

        ok = SHGetPathFromIDList(pidl, path);
        CoTaskMemFree(pidl);
    }

    if (! ok) {

        wcsncpy(path, ForceLinger_editbox, MAX_PATH);
        path[MAX_PATH] = L'\0';
        len_path = wcslen(path);
        while (len_path && path[len_path - 1] == L'\\') {
            path[len_path - 1] = L'\0';
            --len_path;
        }

        if (len_path) {
            if (len_path == 2 && path[1] == L':') {
                path[2] = L'\\';
                path[3] = L'\0';
            }
            ok = TRUE;
        }
    }

    if (ok && m_type == BoxPageForceFol) {

        WCHAR windir[MAX_PATH + 8];
        ULONG len_windir = GetWindowsDirectory(windir, MAX_PATH);
        len_path = wcslen(path);

        if (len_path >= len_windir && (path[len_windir] == L'\\' ||
                                       path[len_windir] == L'\0')
            && _wcsnicmp(path, windir, len_windir) == 0) {

                // if path is (or begins with) the Windows directory
                ok = FALSE;

        } else if (ForceLinger_MatchPattern(path, windir)) {

            // if the path pattern matches the Windows directory

            ok = FALSE;
        }

        if (len_path == 3 && len_windir >= 3
            && _wcsnicmp(path, windir, len_path) == 0) {

                // if path is the drive letter for the Windows drives
                ok = FALSE;
        }

        if (! ok ) {

            CString msg = CMyMsg(MSG_3775) + L"\n" + CMyMsg(MSG_3773, path);
            int rv = CMyApp::MsgBox(this, msg, MB_YESNO);
            if (rv == IDYES)
                ok = TRUE;
        }
    }

    if (ok) {

        CString pathToAdd(path);
        pathToAdd.TrimLeft();
        pathToAdd.TrimRight();
        if (pathToAdd.IsEmpty())
            return;
        if (pathToAdd.GetAt(0) == L'[')
            return;

        CListBox *pListBox = (CListBox *)GetDlgItem(ID_PROGRAM_LIST);
        AddStringAndSetWidth(pListBox, pathToAdd);

        if (m_type == BoxPageInternetPgm || m_type == BoxPageStartRunPgm) {

            CString str;
            pListBox->GetText(0, str);

            if (str == m_accAll || str == m_accNone)
                pListBox->DeleteString(0);
        }

        OnModified();
    }
}


//---------------------------------------------------------------------------
// ForceLinger_OnCmdAddFolder_Callback
//---------------------------------------------------------------------------


int CALLBACK CBoxPage::ForceLinger_OnCmdAddFolder_Callback(
    HWND hwnd, UINT uMsg,LPARAM lParam, LPARAM lpData)
{
    CBoxPage *_this = (CBoxPage *)lpData;
    if (uMsg == BFFM_VALIDATEFAILED)
        _this->ForceLinger_editbox = (WCHAR *)lParam;
    else if (uMsg == BFFM_SELCHANGED || uMsg == BFFM_INITIALIZED)
        _this->ForceLinger_editbox = CString();
    return 0;
}


//---------------------------------------------------------------------------
// ForceLinger_OnCmdRemove
//---------------------------------------------------------------------------


void CBoxPage::ForceLinger_OnCmdRemove()
{
    CListBox *pListBox = (CListBox *)GetDlgItem(ID_PROGRAM_LIST);
    int index = pListBox->GetCurSel();
    if (index != LB_ERR) {

        const CString *SettingName = NULL;
        if (m_type == BoxPageForceFol)
            SettingName = &CBox::_ForceFolder;
        if (m_type == BoxPageForcePgm)
            SettingName = &CBox::_ForceProcess;
        if (m_type == BoxPageLingerPgm)
            SettingName = &CBox::_LingerProcess;
        if (m_type == BoxPageLeaderPgm)
            SettingName = &CBox::_LeaderProcess;
        if (m_type == BoxPageInternetPgm || m_type == BoxPageStartRunPgm)
            SettingName = &CBox::_ProcessGroup;

        if (CheckIfBrackets(pListBox, index, *SettingName))
            return;

        pListBox->DeleteString(index);

        if ((m_type == BoxPageInternetPgm || m_type == BoxPageStartRunPgm)
                && (pListBox->GetCount() == 0)) {

            AddStringAndSetWidth(pListBox, m_accAll);

            if (m_type == BoxPageInternetPgm) {
                GetDlgItem(ID_PROGRAM_ADD_STAR)->
                    SetWindowText(CMyMsg(MSG_3916));
            }
        }

        OnModified();
    }
}


//---------------------------------------------------------------------------
// ForceLinger_MatchPattern
//---------------------------------------------------------------------------


bool CBoxPage::ForceLinger_MatchPattern(
    const CString &pat, const CString &test)
{
    PATTERN *pattern = Pattern_Create(NULL, pat, TRUE);
    CString testlwr = test;
    testlwr.MakeLower();
    BOOLEAN match = Pattern_Match(pattern, testlwr, testlwr.GetLength());
    Pattern_Free(pattern);
    return (match ? true : false);
}


//---------------------------------------------------------------------------
// FileMigrate_OnInitDialog
//---------------------------------------------------------------------------


void CBoxPage::FileMigrate_OnInitDialog(CBox &box)
{
    GetDlgItem(ID_PAGE_LABEL_1)->SetWindowText(CMyMsg(MSG_3903));
    GetDlgItem(ID_PAGE_LABEL_2)->SetWindowText(CMyMsg(MSG_3904));
    GetDlgItem(ID_PAGE_LABEL_3)->SetWindowText(CMyMsg(MSG_3905));
    GetDlgItem(ID_PAGE_LABEL_4)->SetWindowText(CMyMsg(MSG_3906));
    GetDlgItem(ID_PAGE_LABEL_5)->SetWindowText(CMyMsg(MSG_3907));
    GetDlgItem(ID_MIGRATE_SILENT)->SetWindowText(CMyMsg(MSG_3908));
    CenterControl(ID_MIGRATE_SILENT);

    CEdit *edit = (CEdit *)GetDlgItem(ID_MIGRATE_KB);
    edit->SetLimitText(8);
    int size = box.GetSetCopyLimit(FALSE);
    CString str;
    str.Format(L"%d", size);
    edit->SetWindowText(str);

    BOOL silent = box.GetSetCopySilent(FALSE);
    CButton *button = (CButton *)GetDlgItem(ID_MIGRATE_SILENT);
    button->SetCheck(silent ? BST_CHECKED : BST_UNCHECKED);
}


//---------------------------------------------------------------------------
// FileMigrate_OnOK
//---------------------------------------------------------------------------


void CBoxPage::FileMigrate_OnOK(CBox &box)
{
    CString str;
    GetDlgItem(ID_MIGRATE_KB)->GetWindowText(str);
    ULONG64 size64 = _wtoi64(str);

    int size;
    if (size64 < 1)
        size = -1;
    else if (size64 > 999999999)
        size = 999999999;
    else
        size = (int)size64;

    box.GetSetCopyLimit(TRUE, size);

    BOOL silent = FALSE;
    CButton *button = (CButton *)GetDlgItem(ID_MIGRATE_SILENT);
    if (button->GetCheck() == BST_CHECKED)
        silent = TRUE;

    box.GetSetCopySilent(TRUE, silent);
}


//---------------------------------------------------------------------------
// FileMigrate_OnChangeKb
//---------------------------------------------------------------------------


void CBoxPage::FileMigrate_OnChangeKb(void)
{
    CString str;
    GetDlgItem(ID_MIGRATE_KB)->GetWindowText(str);
    ULONG64 size = _wtoi64(str);

    const WCHAR *units = L"KB";
    ULONG64 bytes64 = size;
    if (bytes64 >= 1024) {
        bytes64 /= 1024;
        units = L"MB";
        if (bytes64 >= 1024) {
            bytes64 /= 1024;
            units = L"GB";
        }
    }

    CString strBytes;
    if (CMyApp::m_LayoutRTL) {
        const ULONG U_LRO = 0x202D;   // Start of left-to-right override
        const ULONG U_PDF = 0x202C;   // Pop directional formatting
        strBytes.Format(L"%c(%d %s)%c", U_LRO, (ULONG)bytes64, units, U_PDF);
    } else
        strBytes.Format(L"(%d %s)", (ULONG)bytes64, units);


    GetDlgItem(ID_MIGRATE_MB)->SetWindowText(strBytes);

    OnModified();
}


//---------------------------------------------------------------------------
// RestrictPgm_OnInitDialog
//---------------------------------------------------------------------------


void CBoxPage::RestrictPgm_OnInitDialog(CBox &box)
{
    CString pgms;
    BOOL notify;

    MakeLTR(ID_PROGRAM_LIST);

    CButton *pCheck = (CButton *)GetDlgItem(ID_PROGRAM_NOTIFY);

    if (m_type == BoxPageInternetPgm) {

        GetDlgItem(ID_PAGE_LABEL_1)->SetWindowText(CMyMsg(MSG_3913));
        GetDlgItem(ID_PAGE_LABEL_2)->SetWindowText(CMyMsg(MSG_3914));
        GetDlgItem(ID_PROGRAM_ADD_STAR)->SetWindowText(CMyMsg(MSG_3916));
        pCheck->SetWindowText(CMyMsg(MSG_3915, L"1307"));

        pgms = box.GetInternetProcess(&notify);

        m_accAll  = CMyMsg(MSG_3918);
        m_accNone = CMyMsg(MSG_3919);
    }

    if (m_type == BoxPageStartRunPgm) {

        GetDlgItem(ID_PAGE_LABEL_1)->SetWindowText(CMyMsg(MSG_3923));
        GetDlgItem(ID_PAGE_LABEL_2)->SetWindowText(CMyMsg(MSG_3924));
        GetDlgItem(ID_PROGRAM_ADD_STAR)->SetWindowText(CMyMsg(MSG_3917));
        pCheck->SetWindowText(CMyMsg(MSG_3915, L"1308"));

        pgms = box.GetStartRunProcess(&notify);

        m_accAll  = CMyMsg(MSG_3928);
        m_accNone = CMyMsg(MSG_3769);
    }

    pCheck->SetCheck(notify ? BST_CHECKED : BST_UNCHECKED);

    GetDlgItem(ID_PROGRAM_ADD)->SetWindowText(CMyMsg(MSG_3355));
    GetDlgItem(ID_PROGRAM_REMOVE)->SetWindowText(CMyMsg(MSG_3351));

    GetDlgItem(ID_PROGRAM_LIST)->SetFont(&CMyApp::m_fontFixed);
    GetDlgItem(ID_PAGE_LABEL_2)->SetFont(GetFont());

    //
    // populate list box
    //

    CListBox *pListBox = (CListBox *)GetDlgItem(ID_PROGRAM_LIST);
    pListBox->ResetContent();

    if (pgms.IsEmpty()) {

        AddStringAndSetWidth(pListBox, m_accAll);

    } else if (pgms == L"*") {

        AddStringAndSetWidth(pListBox, m_accNone);
        GetDlgItem(ID_PROGRAM_ADD_STAR)->SetWindowText(CMyMsg(MSG_3917));

    } else {

        int i = 0;
        while (i < pgms.GetLength()) {
            int j = pgms.Find(L',', i);
            if (j == -1)
                j = pgms.GetLength();
            CString token = pgms.Mid(i, j - i);
            token.TrimLeft();
            token.TrimRight();
            AddStringAndSetWidth(pListBox, token);
            i = j + 1;
        }
    }
}


//---------------------------------------------------------------------------
// RestrictPgm_OnOK
//---------------------------------------------------------------------------


void CBoxPage::RestrictPgm_OnOK(CBox &box)
{
    CString pgms;

    CListBox *pList = (CListBox *)GetDlgItem(ID_PROGRAM_LIST);
    int size = pList->GetCount();
    for (int index = 0; index < size; ++index) {
        CString str;
        pList->GetText(index, str);

        if (str == m_accAll) {
            pgms = CString();
            break;
        }

        if (str == m_accNone) {
            pgms = CString(L"*");
            break;
        }

        if (str.GetLength() > 2 && str.GetAt(0) == L'[')
            continue;

        BOOLEAN dup = FALSE;
        for (int index2 = 0; index2 < index; ++index2) {
            CString str2;
            pList->GetText(index2, str2);
            if (str.CompareNoCase(str2) == 0) {
                dup = TRUE;
                break;
            }
        }
        if (dup)
            continue;

        if (! pgms.IsEmpty())
            pgms += L",";
        pgms += str;
    }

    CButton *pCheck = (CButton *)GetDlgItem(ID_PROGRAM_NOTIFY);
    BOOL notify = FALSE;
    if (pCheck->GetCheck() == BST_CHECKED)
        notify = TRUE;

    if (m_type == BoxPageInternetPgm)
        box.SetInternetProcess(pgms, &notify);

    if (m_type == BoxPageStartRunPgm)
        box.SetStartRunProcess(pgms, &notify);
}


//---------------------------------------------------------------------------
// RestrictPgm_AddStar
//---------------------------------------------------------------------------


void CBoxPage::RestrictPgm_AddStar()
{
    CWnd *pWnd = GetDlgItem(ID_PROGRAM_ADD_STAR);
    CString text;
    pWnd->GetWindowText(text);

    CListBox *pListBox = (CListBox *)GetDlgItem(ID_PROGRAM_LIST);
    pListBox->ResetContent();

    if (text == CMyMsg(MSG_3916)) {
        AddStringAndSetWidth(pListBox, m_accNone);
        text = CMyMsg(MSG_3917);
    } else {
        AddStringAndSetWidth(pListBox, m_accAll);
        if (m_type == BoxPageInternetPgm)
            text = CMyMsg(MSG_3916);
    }

    pWnd->SetWindowText(text);

    OnModified();
}


//---------------------------------------------------------------------------
// DropRights_OnInitDialog
//---------------------------------------------------------------------------


void CBoxPage::DropRights_OnInitDialog(CBox &box)
{
    GetDlgItem(ID_PAGE_LABEL_1)->SetWindowText(CMyMsg(MSG_3943));
    GetDlgItem(ID_PAGE_LABEL_2)->SetWindowText(CMyMsg(MSG_3944));
    GetDlgItem(ID_DROPRIGHTS)->SetWindowText(CMyMsg(MSG_3945));

    CenterControl(ID_DROPRIGHTS);

    BOOL enabled = box.GetSetDropAdminRights(FALSE);
    CButton *pCheckBox = (CButton *)GetDlgItem(ID_DROPRIGHTS);
    pCheckBox->SetCheck(enabled ? BST_CHECKED : BST_UNCHECKED);
}


//---------------------------------------------------------------------------
// DropRights_OnOK
//---------------------------------------------------------------------------


void CBoxPage::DropRights_OnOK(CBox &box)
{
    CButton *pCheckBox = (CButton *)GetDlgItem(ID_DROPRIGHTS);
    BOOL enabled = pCheckBox->GetCheck() == BST_CHECKED ? TRUE : FALSE;
    box.GetSetDropAdminRights(TRUE, enabled);
}

void CBoxPage::NetworkFiles_OnInitDialog(CBox &box)
{
    GetDlgItem(ID_PAGE_LABEL_1)->SetWindowText(CMyMsg(MSG_3956));
    GetDlgItem(ID_PAGE_LABEL_2)->SetWindowText(CMyMsg(MSG_3957));
    GetDlgItem(ID_BLOCKNETWORKFILES)->SetWindowText(CMyMsg(MSG_3958));

    CenterControl(ID_BLOCKNETWORKFILES);

    BOOL enabled = box.GetSetBlockNetworkFiles(FALSE);
    CButton *pCheckBox = (CButton *)GetDlgItem(ID_BLOCKNETWORKFILES);
    pCheckBox->SetCheck(enabled ? BST_CHECKED : BST_UNCHECKED);
}


void CBoxPage::NetworkFiles_OnOK(CBox &box)
{
    CButton *pCheckBox = (CButton *)GetDlgItem(ID_BLOCKNETWORKFILES);
    BOOL enabled = pCheckBox->GetCheck() == BST_CHECKED ? TRUE : FALSE;
    box.GetSetBlockNetworkFiles(TRUE, enabled);
}

void CBoxPage::PrintSpooler_OnInitDialog(CBox &box)
{
    GetDlgItem(ID_PAGE_LABEL_1)->SetWindowText(CMyMsg(MSG_3951));
    GetDlgItem(ID_PAGE_LABEL_2)->SetWindowText(CMyMsg(MSG_3952));
    GetDlgItem(ID_ALLOWSPOOLERPRINTTOFILE)->SetWindowText(CMyMsg(MSG_3953));

    BOOL enabled = box.GetSetAllowSpoolerPrintToFile(FALSE);
    CButton *pCheckBox = (CButton *)GetDlgItem(ID_ALLOWSPOOLERPRINTTOFILE);
    pCheckBox->SetCheck(enabled ? BST_CHECKED : BST_UNCHECKED);
}


void CBoxPage::PrintSpooler_OnOK(CBox &box)
{
    CButton *pCheckBox = (CButton *)GetDlgItem(ID_ALLOWSPOOLERPRINTTOFILE);
    BOOL enabled = pCheckBox->GetCheck() == BST_CHECKED ? TRUE : FALSE;
    box.GetSetAllowSpoolerPrintToFile(TRUE, enabled);
}



//---------------------------------------------------------------------------
// FileAccess_OnInitDialog
//---------------------------------------------------------------------------


void CBoxPage::FileAccess_OnInitDialog(CBox &box)
{
    MakeLTR(ID_FILE_LIST);

    //
    // set up dialog text
    //

    ULONG msgTitle, msgLabel1, msgLabel2 = 0;

    if (m_type == BoxPageOpenFile) {
        msgTitle = MSG_4002;
        msgLabel1 = MSG_4003;
        msgLabel2 = MSG_4004;
    }

    if (m_type == BoxPageOpenPipe) {
        msgTitle = MSG_4012;
        msgLabel1 = MSG_4013;
        msgLabel2 = MSG_4014;
    }

    if (m_type == BoxPageReadFile) {
        msgTitle = MSG_4032;
        msgLabel1 = MSG_4033;
    }

    if (m_type == BoxPageWriteFile) {
        msgTitle = MSG_4112;
        msgLabel1 = MSG_4113;
    }

    if (m_type == BoxPageClosedFile) {
        msgTitle = MSG_4022;
        msgLabel1 = MSG_4023;
        msgLabel2 = MSG_4024;
    }

    if (m_type == BoxPageOpenKey) {
        msgTitle = MSG_4042;
        msgLabel1 = MSG_4043;
        msgLabel2 = MSG_4044;
    }

    if (m_type == BoxPageReadKey) {
        msgTitle = MSG_4062;
        msgLabel1 = MSG_4063;
    }

    if (m_type == BoxPageWriteKey) {
        msgTitle = MSG_4122;
        msgLabel1 = MSG_4123;
    }

    if (m_type == BoxPageClosedKey) {
        msgTitle = MSG_4052;
        msgLabel1 = MSG_4053;
        msgLabel2 = MSG_4054;
    }

    if (m_type == BoxPageOpenIpc) {
        msgTitle = MSG_4072;
        msgLabel1 = MSG_4073;
        msgLabel2 = MSG_4074;
    }

    if (m_type == BoxPageClosedIpc) {
        msgTitle = MSG_4082;
        msgLabel1 = MSG_4083;
        msgLabel2 = MSG_4084;
    }

    if (m_type == BoxPageWinClass) {
        msgTitle = MSG_4092;
        msgLabel1 = MSG_4093;
    }

    if (m_type == BoxPageComClass) {
        msgTitle = MSG_4102;
        msgLabel1 = MSG_4103;
    }

    if (m_type == BoxPageWinClass || m_type == BoxPageComClass) {

        RECT rc1, rc2;
        GetDlgItem(ID_PAGE_LABEL_1)->GetWindowRect(&rc1);
        GetDlgItem(ID_PAGE_LABEL_2)->GetWindowRect(&rc2);
        GetDlgItem(ID_PAGE_LABEL_1)->SetWindowPos(
            NULL, 0, 0, rc1.right - rc1.left, rc2.bottom - rc1.top,
            SWP_NOMOVE | SWP_NOREPOSITION);
        GetDlgItem(ID_PAGE_LABEL_2)->ShowWindow(SW_HIDE);
    }

    m_titleForPage = CMyMsg(msgTitle);

    GetDlgItem(ID_PAGE_LABEL_1)->SetWindowText(CMyMsg(msgLabel1));
    if (msgLabel2) {
        GetDlgItem(ID_PAGE_LABEL_2)->SetWindowText(CMyMsg(msgLabel2));
        GetDlgItem(ID_PAGE_LABEL_2)->SetFont(GetFont());
    }

    GetDlgItem(ID_FILE_ADD)->SetWindowText(CMyMsg(MSG_3352));
    GetDlgItem(ID_FILE_EDIT)->SetWindowText(CMyMsg(MSG_3353));
    GetDlgItem(ID_FILE_REMOVE)->SetWindowText(CMyMsg(MSG_3351));
    GetDlgItem(ID_FILE_PROGRAM_TEXT)->SetWindowText(CMyMsg(MSG_3806));
    GetDlgItem(ID_FILE_ADD_PROGRAM)->SetWindowText(CMyMsg(MSG_3355));

    GetDlgItem(ID_FILE_PROGRAM_TEXT)->SetFont(GetFont());
    GetDlgItem(ID_FILE_PROGRAM_EXPLAIN)->SetFont(GetFont());
    GetDlgItem(ID_FILE_PROGRAM)->SetFont(&CMyApp::m_fontFixed);

    CListBox *pListBox = (CListBox *)GetDlgItem(ID_FILE_LIST);
    pListBox->SetFont(&CMyApp::m_fontFixed);

    //
    // for OpenClsid or for resource-only dialog, hide button to add pgm
    //

    if (m_type == BoxPageComClass || (! BoxPage_ProgramName.IsEmpty())) {

        GetDlgItem(ID_FILE_ADD_PROGRAM)->ShowWindow(FALSE);
    }

    //
    // handle resource-only dialog
    //

    if (! BoxPage_ProgramName.IsEmpty()) {

        GetDlgItem(ID_FILE_NEGATE)->ShowWindow(SW_HIDE);

        CComboBox *pCombo = (CComboBox *)GetDlgItem(ID_FILE_PROGRAM);
        pCombo->ResetContent();
        pCombo->AddString(BoxPage_ProgramName);

        pCombo->SetCurSel(0);
        FileAccess_OnSelectPgm();

        return;
    }

    //
    // handle negate button
    //

    if (m_type != BoxPageComClass) {

        static HBITMAP exclamation_bitmap = NULL;
        if (! exclamation_bitmap) {
            exclamation_bitmap =
                MyGdi_CreateFromResource(L"BLUE_EXCLAMATION");
        }

        CButton *pButton = (CButton *)GetDlgItem(ID_FILE_NEGATE);
        pButton->SetBitmap(exclamation_bitmap);

        FileAccess_NegateButton.Init(this, ID_FILE_NEGATE, CMyMsg(MSG_3552));
    }

    //
    // otherwise standard settings dialog:
    // build list of programs named in XxxPath setting
    //

    CStringList pgms;

    if (m_type != BoxPageComClass) {

        CStringList paths;
        FileAccess_GetPathList(paths, NULL);

        POSITION pos1 = paths.GetHeadPosition();
        while (pos1) {

            CString pgm1 = paths.GetNext(pos1);
            CSbieIni::StripBrackets(pgm1);

            int index = pgm1.Find(L',');
            if (index < 1)
                continue;
            pgm1 = pgm1.Left(index);
            if (pgm1 == L'*')
                continue;

            BOOL add = TRUE;

            POSITION pos2 = pgms.GetHeadPosition();
            while (pos2) {
                CString &pgm2 = pgms.GetNext(pos2);
                if (pgm2.CompareNoCase(pgm1) == 0) {
                    add = FALSE;
                    break;
                }
            }

            if (add) {
                pgm1.MakeLower();
                pgms.AddTail(pgm1);
            }
        }
    }

    //
    // populate combobox with process names
    //

    CComboBox *pCombo = (CComboBox *)GetDlgItem(ID_FILE_PROGRAM);

    CString oldpgm;
    if (FileAccess_index != -1)
        pCombo->GetLBText(FileAccess_index, oldpgm);

    pCombo->ResetContent();

    while (! pgms.IsEmpty()) {
        CString pgm = pgms.RemoveHead();
        pCombo->AddString(pgm);
    }

    pCombo->InsertString(0, CMyMsg(MSG_3805));

    //
    // make sure we re-select the right entry, when Apply was clicked
    //

    if (FileAccess_index == -1) {

        pCombo->SetCurSel(0);

    } else {

        int size = pCombo->GetCount();
        int index = 0;
        for (index = 0; index < size; ++index) {
            CString pgm;
            pCombo->GetLBText(index, pgm);
            if (oldpgm.CompareNoCase(pgm) == 0)
                break;
        }

        if (index == size)
            index = 0;

        pCombo->SetCurSel(index);
    }

    ((CListBox *)GetDlgItem(ID_FILE_LIST))->ResetContent();

    FileAccess_OnSelectPgm();
}


//---------------------------------------------------------------------------
// FileAccess_OnOK
//---------------------------------------------------------------------------


void CBoxPage::FileAccess_OnOK(CBox &box)
{
    CString pgm = BoxPage_ProgramName;
    if (pgm.IsEmpty()) {
        CComboBox *pCombo = (CComboBox *)GetDlgItem(ID_FILE_PROGRAM);
        int index = pCombo->GetCurSel();
        if (index != CB_ERR) {
            pCombo->GetLBText(FileAccess_index, pgm);
            if (pgm == CMyMsg(MSG_3805))
                pgm = CString();
        }
    }

    CStringList paths;
    CListBox *pList = (CListBox *)GetDlgItem(ID_FILE_LIST);
    int size = pList->GetCount();
    for (int index = 0; index < size; ++index) {
        CString str;
        pList->GetText(index, str);
        paths.AddTail(str);
    }

    ULONG type = 0;
    if (m_type == BoxPageOpenFile)
        type = 'fo';
    if (m_type == BoxPageOpenPipe)
        type = 'po';
    if (m_type == BoxPageReadFile)
        type = 'fr';
    if (m_type == BoxPageWriteFile)
        type = 'fw';
    if (m_type == BoxPageClosedFile)
        type = 'fc';
    if (m_type == BoxPageOpenKey)
        type = 'ko';
    if (m_type == BoxPageReadKey)
        type = 'kr';
    if (m_type == BoxPageWriteKey)
        type = 'kw';
    if (m_type == BoxPageClosedKey)
        type = 'kc';
    if (m_type == BoxPageOpenIpc)
        type = 'io';
    if (m_type == BoxPageClosedIpc)
        type = 'ic';
    if (m_type == BoxPageWinClass)
        type = 'wo';
    if (m_type == BoxPageComClass)
        type = 'co';

    box.SetXxxPathListForPgm(type, paths, pgm);
}


//---------------------------------------------------------------------------
// FileAccess_OnSelectPgm
//---------------------------------------------------------------------------


void CBoxPage::FileAccess_OnSelectPgm()
{
    CComboBox *pCombo = (CComboBox *)GetDlgItem(ID_FILE_PROGRAM);

    if (m_modified) {
        pCombo->SetCurSel(FileAccess_index);
        CMyApp::MsgBox(GetParent(), MSG_3803, MB_OK);
        return;
    }

    CString pgm = BoxPage_ProgramName;
    if (pgm.IsEmpty()) {

        int index = pCombo->GetCurSel();
        if (index == CB_ERR)
            return;
        if (index != 0)
            pCombo->GetLBText(index, pgm);
        FileAccess_index = index;
    }

    FileAccess_SwitchPgm(pgm);
}


//---------------------------------------------------------------------------
// FileAccess_OnAddPgm
//---------------------------------------------------------------------------


void CBoxPage::FileAccess_OnAddPgm()
{
    if (m_modified) {
        CMyApp::MsgBox(GetParent(), MSG_3803, MB_OK);
        return;
    }

    CBox &box = CBoxes::GetInstance().GetBox(m_BoxName);

    CProgramSelector pgmsel(this, &box, true, CString());
    if (pgmsel.m_pgm1.IsEmpty())
        return;
    const CString &pgm = pgmsel.m_pgm1;

    FileAccess_AddAndSwitchPgm(pgm);
}


//---------------------------------------------------------------------------
// FileAccess_OnNegate
//---------------------------------------------------------------------------


void CBoxPage::FileAccess_OnNegate()
{
    if (m_modified) {
        CMyApp::MsgBox(GetParent(), MSG_3803, MB_OK);
        return;
    }

    CString pgm;
    CComboBox *pCombo = (CComboBox *)GetDlgItem(ID_FILE_PROGRAM);
    int index = pCombo->GetCurSel();
    if (index != CB_ERR)
        pCombo->GetLBText(index, pgm);

    if (pgm.IsEmpty())
        return;
    if (pgm.GetLength() > 1 && pgm.GetAt(0) == L'!')
        pgm.Delete(0, 1);
    else
        pgm.Insert(0, L'!');

    FileAccess_AddAndSwitchPgm(pgm);
}


//---------------------------------------------------------------------------
// FileAccess_AddAndSwitchPgm
//---------------------------------------------------------------------------


void CBoxPage::FileAccess_AddAndSwitchPgm(const CString &pgm)
{
    CComboBox *pCombo = (CComboBox *)GetDlgItem(ID_FILE_PROGRAM);

    bool found = false;

    int size = pCombo->GetCount();
    for (int index = 0; index < size; ++index) {
        CString pgm1;
        pCombo->GetLBText(index, pgm1);
        if (pgm1.CompareNoCase(pgm) == 0) {
            FileAccess_index = index;
            found = true;
            break;
        }
    }

    if (! found)
        FileAccess_index = pCombo->AddString(pgm);

    pCombo->SetCurSel(FileAccess_index);

    FileAccess_SwitchPgm(pgm);
}


//---------------------------------------------------------------------------
// FileAccess_SwitchPgm
//---------------------------------------------------------------------------


void CBoxPage::FileAccess_SwitchPgm(const CString &pgm)
{
    CStringList paths;
    FileAccess_GetPathList(paths, &pgm);

    CListBox *pListBox = (CListBox *)GetDlgItem(ID_FILE_LIST);
    pListBox->ResetContent();

    GetDlgItem(ID_FILE_NEGATE)->ShowWindow(
        (FileAccess_index == 0) ? SW_HIDE : SW_SHOW);

    CWnd *explain = GetDlgItem(ID_FILE_PROGRAM_EXPLAIN);
    CString explainText;
    if (pgm.GetLength() > 1 && pgm.GetAt(0) == L'!')
        explainText = CMyMsg(MSG_3551, pgm.Mid(1));
    explain->SetWindowText(explainText);

    while (! paths.IsEmpty()) {
        CString val = paths.RemoveHead();
        AddStringAndSetWidth(pListBox, val);
    }
}


//---------------------------------------------------------------------------
// FileAccess_GetPathList
//---------------------------------------------------------------------------


void CBoxPage::FileAccess_GetPathList(CStringList &list, const CString *pgm)
{
    ULONG type = 0;
    if (m_type == BoxPageOpenFile)
        type = 'fo';
    if (m_type == BoxPageOpenPipe)
        type = 'po';
    if (m_type == BoxPageReadFile)
        type = 'fr';
    if (m_type == BoxPageWriteFile)
        type = 'fw';
    if (m_type == BoxPageClosedFile)
        type = 'fc';
    if (m_type == BoxPageOpenKey)
        type = 'ko';
    if (m_type == BoxPageReadKey)
        type = 'kr';
    if (m_type == BoxPageWriteKey)
        type = 'kw';
    if (m_type == BoxPageClosedKey)
        type = 'kc';
    if (m_type == BoxPageOpenIpc)
        type = 'io';
    if (m_type == BoxPageClosedIpc)
        type = 'ic';
    if (m_type == BoxPageWinClass)
        type = 'wo';
    if (m_type == BoxPageComClass)
        type = 'co';

    CBox &box = CBoxes::GetInstance().GetBox(m_BoxName);
    if (pgm)
        box.GetXxxPathListForPgm(type, list, *pgm, TRUE);
    else
        box.GetXxxPathList(type, list, TRUE);
}


//---------------------------------------------------------------------------
// FileAccess_OnAddEntry
//---------------------------------------------------------------------------


void CBoxPage::FileAccess_OnAddEntry()
{
    CListBox *pListBox = (CListBox *)GetDlgItem(ID_FILE_LIST);
    pListBox->SetCurSel(-1);
    GetDlgItem(ID_FILE_EDIT)->SetWindowText(CMyMsg(MSG_3353));

    WCHAR path[MAX_PATH + 32];

    //
    // input box for add
    //

    if (m_type == BoxPageOpenKey        ||
        m_type == BoxPageClosedKey      ||
        m_type == BoxPageReadKey        ||
        m_type == BoxPageWriteKey       ||
        m_type == BoxPageOpenIpc        ||
        m_type == BoxPageClosedIpc      ||
        m_type == BoxPageWinClass       ||
        m_type == BoxPageComClass) {

        CString str = CMyApp::InputBox(GetParent(), MSG_3808);
        str.TrimLeft();
        str.TrimRight();
        if (str.IsEmpty() || str.GetAt(0) == L'[')
            return;

        AddBeforeBrackets(pListBox, str);
        OnModified();

        return;
    }

    //
    // get folder or file to add
    //

    CMyMsg title(MSG_3807);

    BROWSEINFO bi;
    memzero(&bi, sizeof(BROWSEINFO));
    bi.hwndOwner = m_hWnd;
    bi.pszDisplayName = path;
    bi.lpszTitle = title;
    bi.ulFlags = BIF_UAHINT | BIF_RETURNONLYFSDIRS | BIF_BROWSEINCLUDEFILES | BIF_NEWDIALOGSTYLE | BIF_EDITBOX;
    LPITEMIDLIST pidl = SHBrowseForFolder(&bi);
    if (! pidl)
        return;

    BOOL ok = SHGetPathFromIDList(pidl, path);
    CoTaskMemFree(pidl);
    if (! ok)
        return;

    //
    // add file or folder
    //

    while (1) {
        int len = wcslen(path);
        if (len > 1 && path[len - 1] == L'\\')
            path[len - 1] = L'\0';
        else
            break;
    }

    ULONG FileAttrs = GetFileAttributes(path);
    if (FileAttrs == INVALID_FILE_ATTRIBUTES)
        FileAttrs = 0;

    if (FileAttrs & FILE_ATTRIBUTE_DIRECTORY) {

        int len = wcslen(path);
        path[len]     = L'\\';
        path[len + 1] = L'\0';
    }

    CString str = CSbieIni::GetInstance().MakeGenericPath(path);
    AddBeforeBrackets(pListBox, str);
    OnModified();
}


//---------------------------------------------------------------------------
// FileAccess_OnEditEntry
//---------------------------------------------------------------------------


void CBoxPage::FileAccess_OnEditEntry()
{
    CListBox *pListBox = (CListBox *)GetDlgItem(ID_FILE_LIST);
    int index = pListBox->GetCurSel();
    if (index != LB_ERR) {

        if (FileAccess_CheckIfBrackets(pListBox, index))
            return;

        pListBox->SetCurSel(-1);
        GetDlgItem(ID_FILE_EDIT)->SetWindowText(CMyMsg(MSG_3353));

        CString str1, str2;
        pListBox->GetText(index, str1);
        str2 = str1;

        CMyApp::InputBox(GetParent(), MSG_3809, str2);
        str2.TrimLeft();
        str2.TrimRight();
        if (str2.IsEmpty())
            return;
        if (str2.GetAt(0) == L'[')
            return;
        if (str1.CompareNoCase(str2) == 0)
            return;

        pListBox->DeleteString(index);

        str2 = CSbieIni::GetInstance().MakeGenericPath(str2);
        AddStringAndSetWidth(pListBox, str2, index);

        OnModified();

    } else {

        ULONG save_type = m_type;

        if (m_type == BoxPageOpenFile ||
            m_type == BoxPageOpenPipe ||
            m_type == BoxPageClosedFile ||
            m_type == BoxPageReadFile ||
            m_type == BoxPageWriteFile) {

            // cause "Add Resource" edit box to appear
            m_type = BoxPageOpenIpc;
        }

        FileAccess_OnAddEntry();

        m_type = save_type;
    }
}


//---------------------------------------------------------------------------
// FileAccess_OnRemoveEntry
//---------------------------------------------------------------------------


void CBoxPage::FileAccess_OnRemoveEntry()
{
    CListBox *pListBox = (CListBox *)GetDlgItem(ID_FILE_LIST);
    int index = pListBox->GetCurSel();
    if (index == LB_ERR)
        return;
    if (FileAccess_CheckIfBrackets(pListBox, index))
        return;

    pListBox->SetCurSel(-1);
    GetDlgItem(ID_FILE_EDIT)->SetWindowText(CMyMsg(MSG_3353));

    pListBox->DeleteString(index);
    OnModified();
}


//---------------------------------------------------------------------------
// FileAccess_OnSelectRes
//---------------------------------------------------------------------------


void CBoxPage::FileAccess_OnSelectRes()
{
    CListBox *pListBox = (CListBox *)GetDlgItem(ID_FILE_LIST);
    int index = pListBox->GetCurSel();
    if (index != LB_ERR)
        GetDlgItem(ID_FILE_EDIT)->SetWindowText(CMyMsg(MSG_3354));
}


//---------------------------------------------------------------------------
// FileAccess_CheckIfBrackets
//---------------------------------------------------------------------------


BOOL CBoxPage::FileAccess_CheckIfBrackets(CListBox *pListBox, int index)
{
    CString pgm = BoxPage_ProgramName;
    if (pgm.IsEmpty()) {

        CComboBox *pCombo = (CComboBox *)GetDlgItem(ID_FILE_PROGRAM);
        int index = pCombo->GetCurSel();
        if (index != CB_ERR && index != 0)
            pCombo->GetLBText(index, pgm);
    }

    const CString *SettingName = NULL;
    if (m_type == BoxPageOpenFile)
        SettingName = &CBox::_OpenFilePath;
    if (m_type == BoxPageOpenPipe)
        SettingName = &CBox::_OpenPipePath;
    if (m_type == BoxPageReadFile)
        SettingName = &CBox::_ReadFilePath;
    if (m_type == BoxPageWriteFile)
        SettingName = &CBox::_WriteFilePath;
    if (m_type == BoxPageClosedFile)
        SettingName = &CBox::_ClosedFilePath;
    if (m_type == BoxPageOpenKey)
        SettingName = &CBox::_OpenKeyPath;
    if (m_type == BoxPageReadKey)
        SettingName = &CBox::_ReadKeyPath;
    if (m_type == BoxPageWriteKey)
        SettingName = &CBox::_WriteKeyPath;
    if (m_type == BoxPageClosedKey)
        SettingName = &CBox::_ClosedKeyPath;
    if (m_type == BoxPageOpenIpc)
        SettingName = &CBox::_OpenIpcPath;
    if (m_type == BoxPageClosedIpc)
        SettingName = &CBox::_ClosedIpcPath;
    if (m_type == BoxPageWinClass)
        SettingName = &CBox::_OpenWinClass;
    if (m_type == BoxPageComClass)
        SettingName = &CBox::_OpenClsid;

    return CheckIfBrackets(pListBox, index, *SettingName, pgm);
}


//---------------------------------------------------------------------------
// UserAccounts_OnInitDialog
//---------------------------------------------------------------------------


void CBoxPage::UserAccounts_OnInitDialog(CBox &box)
{
    GetDlgItem(ID_PAGE_LABEL_1)->SetWindowText(CMyMsg(MSG_5102));
    GetDlgItem(ID_PAGE_LABEL_2)->SetWindowText(CMyMsg(MSG_5103));
    GetDlgItem(ID_USER_ADD)->SetWindowText(CMyMsg(MSG_3359));
    GetDlgItem(ID_USER_REMOVE)->SetWindowText(CMyMsg(MSG_3351));

    CStringList users;
    box.GetSetUserAccounts(FALSE, users);

    MakeLTR(ID_USER_LIST);
    CListBox *pListBox = (CListBox *)GetDlgItem(ID_USER_LIST);
    pListBox->ResetContent();
    pListBox->SetFont(&CMyApp::m_fontFixed);

    while (! users.IsEmpty()) {
        CString user = users.RemoveHead();
        AddStringAndSetWidth(pListBox, user);
    }
}


//---------------------------------------------------------------------------
// UserAccounts_OnOK
//---------------------------------------------------------------------------


void CBoxPage::UserAccounts_OnOK(CBox &box)
{
    CStringList users;
    CListBox *pListBox = (CListBox *)GetDlgItem(ID_USER_LIST);
    int size = pListBox->GetCount();
    for (int index = 0; index < size; ++index) {
        CString str;
        pListBox->GetText(index, str);
        users.AddTail(str);
    }
    box.GetSetUserAccounts(TRUE, users);

    if ((! UserAccounts_restricted) && (STATUS_ACCOUNT_RESTRICTION ==
                            SbieApi_IsBoxEnabled(box.GetName()))) {

        UserAccounts_restricted = true;
        CMyApp::MsgBox(GetParent(), MSG_5107, MB_OK);
    }
}


//---------------------------------------------------------------------------
// UserAccounts_OnCmdAdd
//---------------------------------------------------------------------------


void CBoxPage::UserAccounts_OnCmdAdd()
{
    CStringList users;
    SelectUsers(users);
    if (! users.IsEmpty()) {
        CListBox *pListBox = (CListBox *)GetDlgItem(ID_USER_LIST);
        while (! users.IsEmpty()) {
            CString user = users.RemoveHead();
            AddStringAndSetWidth(pListBox, user);
        }
    }
    OnModified();
}


//---------------------------------------------------------------------------
// UserAccounts_OnCmdRemove
//---------------------------------------------------------------------------


void CBoxPage::UserAccounts_OnCmdRemove()
{
    CListBox *pListBox = (CListBox *)GetDlgItem(ID_USER_LIST);
    int index = pListBox->GetCurSel();
    if (index == LB_ERR)
        return;

    pListBox->SetCurSel(-1);
    pListBox->DeleteString(index);
    OnModified();
}


//---------------------------------------------------------------------------
// Plugin_OnInitDialog
//---------------------------------------------------------------------------


void CBoxPage::Plugin_OnInitDialog(CBox &box)
{
    ASSERT(0);
}


//---------------------------------------------------------------------------
// Plugin_OnOK
//---------------------------------------------------------------------------


void CBoxPage::Plugin_OnOK(CBox &box)
{
    ASSERT(0);
}


//---------------------------------------------------------------------------
// DoPropertySheet
//---------------------------------------------------------------------------


void CBoxPage::DoPropertySheet(
    CWnd *pParentWnd, const CString &BoxName,
    bool *ReloadBoxes, const CString &ProgramName)
{
    CUserSettings &ini = CUserSettings::GetInstance();

    BOOL autoapply;
    ini.GetBool(_AutoApplySettings, autoapply, FALSE);

    CMyMsg caption(MSG_3801, BoxName);

    BoxPage_ProgramName = ProgramName;

    TreePropSheet::CTreePropSheet sheet(caption, pParentWnd);
    if (CMyApp::m_LayoutRTL)
        sheet.SetLayoutRTL();
    sheet.SetTreeWidth(210);
    sheet.SetEmptyPageText(CMyMsg(MSG_3802));
    sheet.SetAutoApply(CMyMsg(MSG_3804), autoapply);
    sheet.AddMinimizeButton();

    CBoxPage pageAppearance(
        BoxPageAppearance,      BoxName,    0,          MSG_3811);

    CBoxPage pageQuickRecover(
        BoxPageQuickRecover,    BoxName,    MSG_3821,   MSG_3822);
    CBoxPage pageAutoRecover(
        BoxPageAutoRecover,     BoxName,    MSG_3821,   MSG_3715);

    CBoxPage pageAutoDelete(
        BoxPageAutoDelete,      BoxName,    MSG_3841,   MSG_3842);
    CBoxPage pageDeleteCommand(
        BoxPageDeleteCommand,   BoxName,    MSG_3841,   MSG_3852);

    CBoxPage pageProgramGroups(
        BoxPageProgramGroups,   BoxName,    0,          MSG_3971);

    CBoxPage pageForceFol(
        BoxPageForceFol,        BoxName,    MSG_3861,   MSG_3862);
    CBoxPage pageForcePgm(
        BoxPageForcePgm,        BoxName,    MSG_3861,   MSG_3872);

    CBoxPage pageLingerPgm(
        BoxPageLingerPgm,       BoxName,    MSG_3881,   MSG_3882);
    CBoxPage pageLeaderPgm(
        BoxPageLeaderPgm,       BoxName,    MSG_3881,   MSG_3892);

    CBoxPage pageFileMigrate(
        BoxPageFileMigrate,     BoxName,    0,          MSG_3901);

    CBoxPage pageInternetPgm(
        BoxPageInternetPgm,     BoxName,    MSG_3911,   MSG_3912);
    CBoxPage pageStartRunPgm(
        BoxPageStartRunPgm,     BoxName,    MSG_3911,   MSG_3922);
    CBoxPage pageDropRights(
        BoxPageDropRights,      BoxName,    MSG_3911,   MSG_3942);
    CBoxPage pageBlockNetworkFiles(
        BoxPageNetworkFiles, BoxName, MSG_3911, MSG_3955);

    CBoxPage pageOpenFile(
        BoxPageOpenFile,        BoxName,    0,          MSG_4001);
    CBoxPage pageOpenPipe(
        BoxPageOpenPipe,        BoxName,    0,          MSG_4011);
    CBoxPage pageClosedFile(
        BoxPageClosedFile,      BoxName,    0,          MSG_4021);
    CBoxPage pageReadFile(
        BoxPageReadFile,        BoxName,    0,          MSG_4031);
    CBoxPage pageWriteFile(
        BoxPageWriteFile,       BoxName,    0,          MSG_4111);
    CBoxPage pageOpenKey(
        BoxPageOpenKey,         BoxName,    0,          MSG_4041);
    CBoxPage pageClosedKey(
        BoxPageClosedKey,       BoxName,    0,          MSG_4051);
    CBoxPage pageReadKey(
        BoxPageReadKey,         BoxName,    0,          MSG_4061);
    CBoxPage pageWriteKey(
        BoxPageWriteKey,        BoxName,    0,          MSG_4121);
    CBoxPage pageOpenIpc(
        BoxPageOpenIpc,         BoxName,    0,          MSG_4071);
    CBoxPage pageClosedIpc(
        BoxPageClosedIpc,       BoxName,    0,          MSG_4081);
    CBoxPage pageWinClass(
        BoxPageWinClass,        BoxName,    0,          MSG_4091);
    CBoxPage pageComClass(
        BoxPageComClass,        BoxName,    0,          MSG_4101);

    CBoxPage pageUserAccounts(
        BoxPageUserAccounts,    BoxName,    0,          MSG_5101);

    if (ProgramName.IsEmpty()) {

        sheet.AddPage(&pageAppearance);

        sheet.AddPage(&pageQuickRecover);
        sheet.AddPage(&pageAutoRecover);

        sheet.AddPage(&pageAutoDelete);
        sheet.AddPage(&pageDeleteCommand);

        sheet.AddPage(&pageProgramGroups);

        sheet.AddPage(&pageForceFol);
        sheet.AddPage(&pageForcePgm);
        sheet.AddPage(&pageLingerPgm);
        sheet.AddPage(&pageLeaderPgm);

        sheet.AddPage(&pageFileMigrate);

        sheet.AddPage(&pageInternetPgm);
        sheet.AddPage(&pageStartRunPgm);
        sheet.AddPage(&pageDropRights);
        sheet.AddPage(&pageBlockNetworkFiles);
    }

    sheet.AddPage(&pageOpenFile);
    sheet.AddPage(&pageOpenPipe);
    sheet.AddPage(&pageReadFile);
    sheet.AddPage(&pageWriteFile);
    sheet.AddPage(&pageClosedFile);
    sheet.AddPage(&pageOpenKey);
    sheet.AddPage(&pageReadKey);
    sheet.AddPage(&pageWriteKey);
    sheet.AddPage(&pageClosedKey);
    sheet.AddPage(&pageOpenIpc);
    sheet.AddPage(&pageClosedIpc);
    sheet.AddPage(&pageWinClass);

    if (ProgramName.IsEmpty()) {

        sheet.AddPage(&pageComClass);

        CAppPage::AddPages(sheet, BoxName);

        sheet.AddPage(&pageUserAccounts);
    }

    INT_PTR retval = sheet.DoModal();

    if (retval == IDOK) {

        if (ReloadBoxes && pageUserAccounts.UserAccounts_restricted)
            *ReloadBoxes = true;

        autoapply = sheet.IsAutoApplyChecked();
        ini.SetBool(_AutoApplySettings, autoapply);

        BOOL tip;
        ini.GetBool(_SettingChangeNotify, tip, TRUE);
        if (tip) {

            int rv = CMyApp::MsgCheckBox(pParentWnd, MSG_3513, 0, MB_OK);

            if (rv < 0) {
                rv = -rv;
                ini.SetBool(_SettingChangeNotify, FALSE);
            }
        }
    }

    if (ProgramName.IsEmpty())
        CAppPage::DeletePages();
}
