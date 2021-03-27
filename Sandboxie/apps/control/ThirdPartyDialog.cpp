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
// Third Party Software Dialog Box
//---------------------------------------------------------------------------


#include "stdafx.h"
#include "ThirdPartyDialog.h"

#include "TemplateListBox.h"
#include "SbieIni.h"
#include "UserSettings.h"
#include "Boxes.h"
#include "apps/common/RunBrowser.h"
#include "common/win32_ntddk.h"
#include <psapi.h>


#define PATTERN XPATTERN
#define _MY_POOL_H          // prevent inclusion of pool.h by pattern.h
typedef void *POOL;
#include "common/pattern.h"


//---------------------------------------------------------------------------
// Message Map
//---------------------------------------------------------------------------


BEGIN_MESSAGE_MAP(CThirdPartyDialog, CBaseDialog)

    ON_COMMAND(ID_APP_TEMPLATE_ADD,                 OnAdd)
    ON_COMMAND(ID_APP_TEMPLATE_REMOVE,              OnRemove)
    ON_CONTROL(LBN_DBLCLK, ID_APP_TEMPLATE_LIST,    OnToggle)
    ON_COMMAND(ID_APP_TEMPLATE_AUTORUN,             OnCheckBox)
    ON_COMMAND(ID_APP_TEMPLATE_REMOVE_OLD,          OnRemoveOld)
    ON_COMMAND(ID_APP_TEMPLATE_CONFLICTS,           OnKnownConflicts)
    ON_WM_SHOWWINDOW()

END_MESSAGE_MAP()


//---------------------------------------------------------------------------
// Defines
//---------------------------------------------------------------------------


#define KC_MUST_SEE     1
#define KC_AUTO_OPEN    2
#define KC_POST_OK      4
#define KC_POST_CANCEL  8


//---------------------------------------------------------------------------
// Variables
//---------------------------------------------------------------------------


HANDLE CThirdPartyDialog::hWatchThread = NULL;

static const WCHAR *_KnownConflicts = L"KnownConflicts";


//---------------------------------------------------------------------------
// Constructor
//---------------------------------------------------------------------------


CThirdPartyDialog::CThirdPartyDialog(
    CWnd *pParentWnd, BOOL AutoRun, WPARAM wParamAlert)
    : CBaseDialog(pParentWnd)
{
    //
    // launch watch thread
    //

    if (hWatchThread == NULL) {

        ULONG ThreadId;
        ULONG_PTR *ThreadArgs = new ULONG_PTR[2];
        ThreadArgs[0] = (ULONG_PTR)pParentWnd->m_hWnd;
        ThreadArgs[1] = (ULONG_PTR)wParamAlert;
        hWatchThread = CreateThread(
            NULL, 0, WatchThread, (void *)ThreadArgs, 0, &ThreadId);
        if (! hWatchThread)
            hWatchThread = INVALID_HANDLE_VALUE;
    }

    //
    // don't bother if user can't make any changes
    //

    if (AutoRun && (! CUserSettings::GetInstance().CanEdit()))
        return;

    //
    // display dialog
    //

    m_doKnownConflicts = 0;

    m_autorun  = AutoRun;
    m_checkbox = AutoRunSoftCompat();
    BOOL oldcheck = m_checkbox;

    SetDialogTemplate(L"THIRD_PARTY_DIALOG");

    DoModal();

    BOOL newcheck = m_checkbox;
    if (newcheck != oldcheck)
        AutoRunSoftCompat(&newcheck);
}


//---------------------------------------------------------------------------
// GetAutoRunSoftCompat
//---------------------------------------------------------------------------


BOOL CThirdPartyDialog::AutoRunSoftCompat(BOOL *NewValue)
{
    static const WCHAR *_AutoRunSoftCompat = L"AutoRunSoftCompat";
    BOOL value;
    CUserSettings &settings = CUserSettings::GetInstance();
    if (NewValue)
        value = settings.SetBool(_AutoRunSoftCompat, *NewValue);
    else
        settings.GetBool(_AutoRunSoftCompat, value, TRUE);
    return value;
}


//---------------------------------------------------------------------------
// Destructor
//---------------------------------------------------------------------------


CThirdPartyDialog::~CThirdPartyDialog()
{
}


//---------------------------------------------------------------------------
// OnInitDialog
//---------------------------------------------------------------------------


BOOL CThirdPartyDialog::OnInitDialog()
{
    CWaitCursor cursor;

    SetWindowPos(&wndTopMost, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

    //
    // prepare dialog text
    //

    SetWindowText(CMyMsg(MSG_4251));

    GetDlgItem(ID_PAGE_LABEL_1)->SetWindowText(CMyMsg(MSG_4252));
    GetDlgItem(ID_PAGE_LABEL_2)->SetWindowText(CMyMsg(MSG_4253));
    GetDlgItem(ID_PAGE_LABEL_3)->SetWindowText(CMyMsg(MSG_4254));

    GetDlgItem(ID_APP_TEMPLATE_ADD)->SetWindowText(CMyMsg(MSG_3352));
    GetDlgItem(ID_APP_TEMPLATE_REMOVE)->SetWindowText(CMyMsg(MSG_3351));

    GetDlgItem(IDOK)->SetWindowText(CMyMsg(MSG_3001));
    GetDlgItem(IDCANCEL)->SetWindowText(CMyMsg(MSG_3002));

    GetDlgItem(ID_APP_TEMPLATE_AUTORUN)->SetWindowText(CMyMsg(MSG_4255));

    CButton *pCheckBox = (CButton *)GetDlgItem(ID_APP_TEMPLATE_AUTORUN);
    pCheckBox->SetCheck(m_checkbox ? BST_UNCHECKED : BST_CHECKED);

    MakeLTR(ID_APP_TEMPLATE_LIST);

    //
    // collect data
    //

    CollectObjects();
    CollectClasses();
    CollectServices();
    CollectProducts();
    CollectTemplates();

    //
    // check for any known conflicts
    //

    CStringList conflicts;
    FindConflicts(conflicts, 1);
    if (! conflicts.IsEmpty()) {

        GetDlgItem(ID_APP_TEMPLATE_CONFLICTS)->SetWindowText(
                                                        CMyMsg(MSG_4452));
        GetDlgItem(ID_APP_TEMPLATE_CONFLICTS)->ShowWindow(SW_SHOW);

        while (! conflicts.IsEmpty())
            conflicts.RemoveHead();
        FindConflicts(conflicts, 0);
        if (! conflicts.IsEmpty()) {

            m_ConflictsButton.Init(this, ID_APP_TEMPLATE_CONFLICTS);
            m_doKnownConflicts |= KC_MUST_SEE;
        }
    }

    //
    // populate list box
    //

    BOOL quit = m_autorun;

    CBox &GlobalSettings = CBoxes::GetInstance().GetBox(0);

    CListBox *pListBox = (CListBox *)GetDlgItem(ID_APP_TEMPLATE_LIST);

    pListBox->SetItemHeight(0, pListBox->GetItemHeight(0) * 3 / 2);

    POSITION pos = m_templates.GetStartPosition();
    while (pos) {
        void *enabled;
        CString tmpl_name;
        m_templates.GetNextAssoc(pos, tmpl_name, enabled);
        if (enabled) {

            CString descr = CTemplateListBox::GetTemplateTitle(tmpl_name);

            BOOL checked = TRUE;
            if (GlobalSettings.IsTemplateRejected(tmpl_name))
                checked = FALSE;
            else if (! GlobalSettings.IsTemplateEnabled(tmpl_name))
                quit = FALSE;

            CTemplateListBox::Decorate(descr, checked, TRUE);
            int index = pListBox->AddString(descr);

            const WCHAR *srckey;
            m_templates.LookupKey(tmpl_name, srckey);
            pListBox->SetItemDataPtr(index, (void *)srckey);
        }
    }

    //
    // if this is AutoRun, and no new template was detetected, then quit
    // if this is AutoRun, and a new template was detected, disable close
    //

    if (m_autorun) {

        if (quit) {

            if (m_doKnownConflicts & KC_MUST_SEE)
                m_doKnownConflicts |= KC_AUTO_OPEN;
            else
                EndDialog(0);
        }

        CRect rc;
        GetDlgItem(IDCANCEL)->GetWindowRect(&rc);
        ScreenToClient(&rc);
        GetDlgItem(IDOK)->MoveWindow(rc);

        GetDlgItem(IDCANCEL)->ShowWindow(SW_HIDE);

    } else if (! m_stale_templates.IsEmpty()) {

        GetDlgItem(ID_APP_TEMPLATE_REMOVE_OLD)->SetWindowText(
                                                        CMyMsg(MSG_4257));
        GetDlgItem(ID_APP_TEMPLATE_REMOVE_OLD)->ShowWindow(SW_SHOW);
    }

    //
    //
    //

    AddMinimizeButton();

    return TRUE;
}


//---------------------------------------------------------------------------
// ApplyTemplates
//---------------------------------------------------------------------------


void CThirdPartyDialog::ApplyTemplates()
{
    CSbieIni &ini = CSbieIni::GetInstance();
    CBox &GlobalSettings = CBoxes::GetInstance().GetBox(0);

    CListBox *pListBox = (CListBox *)GetDlgItem(ID_APP_TEMPLATE_LIST);

    int size = pListBox->GetCount();
    for (int index = 0; index < size; ++index) {

        CString text;
        pListBox->GetText(index, text);
        if (text.IsEmpty())
            continue;

        const WCHAR *tmpl_name =
            (const WCHAR *)pListBox->GetItemDataPtr(index);

        if (CTemplateListBox::IsCheck(text)) {
            GlobalSettings.RejectTemplate(tmpl_name, FALSE);
            GlobalSettings.EnableTemplate(tmpl_name, TRUE);
        }

        if (CTemplateListBox::IsClear(text)) {
            GlobalSettings.EnableTemplate(tmpl_name, FALSE);
            GlobalSettings.RejectTemplate(tmpl_name, TRUE);
        }
    }
}


//---------------------------------------------------------------------------
// OnAdd
//---------------------------------------------------------------------------


void CThirdPartyDialog::OnAdd()
{
    CTemplateListBox::OnAddRemove(this, TRUE);
}


//---------------------------------------------------------------------------
// OnRemove
//---------------------------------------------------------------------------


void CThirdPartyDialog::OnRemove()
{
    CTemplateListBox::OnAddRemove(this, FALSE);
}


//---------------------------------------------------------------------------
// OnToggle
//---------------------------------------------------------------------------


void CThirdPartyDialog::OnToggle()
{
    CTemplateListBox::OnAddRemove(this, FALSE, TRUE);
}


//---------------------------------------------------------------------------
// OnCheckBox
//---------------------------------------------------------------------------


void CThirdPartyDialog::OnCheckBox()
{
    m_checkbox = ! m_checkbox;
    CButton *pCheckBox = (CButton *)GetDlgItem(ID_APP_TEMPLATE_AUTORUN);
    pCheckBox->SetCheck(m_checkbox ? BST_UNCHECKED : BST_CHECKED);
}


//---------------------------------------------------------------------------
// OnOK
//---------------------------------------------------------------------------


void CThirdPartyDialog::OnOK()
{
    ApplyTemplates();

    if (m_doKnownConflicts & KC_MUST_SEE) {
        m_doKnownConflicts |= KC_POST_OK;
        PostMessage(WM_COMMAND, ID_APP_TEMPLATE_CONFLICTS, 0);

    } else
        EndDialog(0);
}


//---------------------------------------------------------------------------
// OnCancel
//---------------------------------------------------------------------------


void CThirdPartyDialog::OnCancel()
{
    if (m_autorun)
        OnOK();

    else if (m_doKnownConflicts & KC_MUST_SEE) {
        m_doKnownConflicts |= KC_POST_CANCEL;
        PostMessage(WM_COMMAND, ID_APP_TEMPLATE_CONFLICTS, 0);

    } else
        CDialog::OnCancel();
}


//---------------------------------------------------------------------------
// OnRemoveOld
//---------------------------------------------------------------------------


void CThirdPartyDialog::OnRemoveOld()
{
    CString descrs;

    POSITION pos = m_stale_templates.GetHeadPosition();
    while (pos) {
        const CString &tmpl_name = m_stale_templates.GetNext(pos);
        CString descr = CTemplateListBox::GetTemplateTitle(tmpl_name);
        descrs += descr;
        descrs += L"\r\n";
    }

    int rv = CMyApp::MsgBox(this, CMyMsg(MSG_4258, descrs), MB_OKCANCEL);
    if (rv == IDOK) {

        CBox &GlobalSettings = CBoxes::GetInstance().GetBox(0);

        pos = m_stale_templates.GetHeadPosition();
        while (pos) {
            const CString &tmpl_name = m_stale_templates.GetNext(pos);
            GlobalSettings.RejectTemplate(tmpl_name, FALSE);
        }

        GetDlgItem(ID_APP_TEMPLATE_REMOVE_OLD)->ShowWindow(SW_HIDE);
    }
}


//---------------------------------------------------------------------------
// OnKnownConflicts
//---------------------------------------------------------------------------


void CThirdPartyDialog::OnKnownConflicts()
{
    CString text;

    m_doKnownConflicts &= ~KC_MUST_SEE;

    CStringList conflicts;
    FindConflicts(conflicts, 2);
    if (conflicts.IsEmpty())
        goto finish;

    m_ConflictsButton.EnableFlashing(false);

    while (! conflicts.IsEmpty())
        text += conflicts.RemoveHead() + L"\n";
    text = CMyMsg(MSG_4451, text);

    int rv = CMyApp::MsgBox(this, text, MB_YESNO);
    if (rv == IDYES) {
        ShellExecute(m_pParentWnd->m_hWnd, NULL,
                     CRunBrowser::GetTopicUrl(_KnownConflicts),
                     NULL, NULL, SW_SHOWNORMAL);
    }

    if (! CUserSettings::GetInstance().CanEdit())
        goto finish;

    while (! conflicts.IsEmpty())
        conflicts.RemoveHead();
    FindConflicts(conflicts, 1);

    text = "";
    while (! conflicts.IsEmpty()) {
        if (! text.IsEmpty())
            text += L",";
        text += conflicts.RemoveHead();
    }
    CSbieIni &ini = CSbieIni::GetInstance();
    ini.SetText(_GlobalSettings, _KnownConflicts, text);

    //
    //
    //

finish:

    if (m_doKnownConflicts & KC_POST_OK)
        PostMessage(WM_COMMAND, IDOK, 0);
    else if (m_doKnownConflicts & KC_POST_CANCEL)
        PostMessage(WM_COMMAND, IDCANCEL, 0);
}


//---------------------------------------------------------------------------
// OnShowWindow
//---------------------------------------------------------------------------


void CThirdPartyDialog::OnShowWindow(BOOL bShow, UINT nStatus)
{
    if (bShow && (m_doKnownConflicts & KC_AUTO_OPEN)) {
        m_doKnownConflicts &= ~KC_AUTO_OPEN;
        ApplyTemplates();
        PostMessage(WM_COMMAND, ID_APP_TEMPLATE_CONFLICTS, 0);
    }
}


//---------------------------------------------------------------------------
// CollectObjects
//---------------------------------------------------------------------------


void CThirdPartyDialog::CollectObjects()
{
    while (! m_objects.IsEmpty())
        m_objects.RemoveHead();

    CStringList objdirs;
    objdirs.AddTail(L"\\BaseNamedObjects");
    objdirs.AddTail(L"\\Sessions");
    objdirs.AddTail(L"\\RPC Control");
    objdirs.AddTail(L"\\Device");

    while (! objdirs.IsEmpty())
        CollectObjects2(objdirs);
}


//---------------------------------------------------------------------------
// CollectObjects2
//---------------------------------------------------------------------------


void CThirdPartyDialog::CollectObjects2(CStringList &objdirs)
{
    static const WCHAR *_WantedTypes[] = {
        L"Directory",
        L"Event", L"Mutant", L"Section", L"Semaphore",
        L"Port", L"ALPC Port",
        L"Device",
        NULL
    };

    NTSTATUS status;
    OBJECT_ATTRIBUTES objattrs;
    UNICODE_STRING objname;
    HANDLE handle;
    OBJECT_DIRECTORY_INFORMATION *info, *info_ptr;
    ULONG info_len, len, i, context;

    //
    // enumerate contents of the object directory
    //

    CString objdir = objdirs.RemoveHead();
    if (objdir.Left(10) == L"\\Sessions\\" && objdir.GetLength() <= 13)
        objdir += L"\\BaseNamedObjects";

    InitializeObjectAttributes(
        &objattrs, &objname, OBJ_CASE_INSENSITIVE, NULL, NULL);
    RtlInitUnicodeString(&objname, objdir);

    status = NtOpenDirectoryObject(&handle, DIRECTORY_QUERY, &objattrs);
    if (! NT_SUCCESS(status))
        return;

    info = NULL;
    info_len = 32768;

    for (i = 0; i < 10; ++i) {

        info = (OBJECT_DIRECTORY_INFORMATION *)
            HeapAlloc(GetProcessHeap(), 0, info_len);
        if (! info) {
            status = STATUS_INSUFFICIENT_RESOURCES;
            break;
        }

        status = NtQueryDirectoryObject(
            handle, info, info_len, FALSE, TRUE, &context, &len);

        if (status == STATUS_MORE_ENTRIES)
            status = STATUS_BUFFER_OVERFLOW;

        if (NT_SUCCESS(status))
            break;

        HeapFree(GetProcessHeap(), 0, info);
        info_len += 32768;

        if (status == STATUS_BUFFER_OVERFLOW ||
            status == STATUS_INFO_LENGTH_MISMATCH ||
            status == STATUS_BUFFER_TOO_SMALL) {

            continue;
        }

        break;
    }

    NtClose(handle);
    if (! NT_SUCCESS(status))
        return;

    //
    // collect IPC objects
    //

    for (info_ptr = info; info_ptr->Name.Buffer; ++info_ptr) {

        for (i = 0; _WantedTypes[i]; ++i) {
            if (_wcsicmp(info_ptr->TypeName.Buffer, _WantedTypes[i]) == 0)
                break;
        }
        if (! _WantedTypes[i])
            continue;

        CString ThisObjectName;
        ThisObjectName.Format(L"%s\\%s", objdir, info_ptr->Name.Buffer);

        if (i == 0)
            objdirs.AddTail(ThisObjectName);
        else {
            ThisObjectName.MakeLower();
            m_objects.AddTail(ThisObjectName);
        }
    }

    HeapFree(GetProcessHeap(), 0, info);
}


//---------------------------------------------------------------------------
// CollectClasses
//---------------------------------------------------------------------------


void CThirdPartyDialog::CollectClasses()
{
    while (! m_classes.IsEmpty())
        m_classes.RemoveHead();

    EnumWindows(CollectClasses2, (LPARAM)this);
}


//---------------------------------------------------------------------------
// CollectClassesEnum
//---------------------------------------------------------------------------


BOOL CThirdPartyDialog::CollectClasses2(HWND hwnd, LPARAM lparam)
{
    WCHAR clsnm[256];

    GetClassName(hwnd, clsnm, 250);
    clsnm[250] = L'\0';
    _wcslwr(clsnm);

    if (clsnm[0] && wcsncmp(clsnm, L"Sandbox:", 8) != 0) {
        CThirdPartyDialog *_this = (CThirdPartyDialog *)lparam;
        _this->m_classes.AddTail(clsnm);
    }

    return TRUE;
}


//---------------------------------------------------------------------------
// CollectServices
//---------------------------------------------------------------------------


void CThirdPartyDialog::CollectServices()
{
    SC_HANDLE hManager;
    ENUM_SERVICE_STATUS *info;
    ULONG info_len;
    ULONG ResumeHandle;

    while (! m_services.IsEmpty())
        m_services.RemoveHead();

    hManager = OpenSCManager(NULL, NULL, SC_MANAGER_ENUMERATE_SERVICE);
    if (! hManager)
        return;

    info_len = 10240;
    info = (ENUM_SERVICE_STATUS *)HeapAlloc(GetProcessHeap(), 0, info_len);
    if (! info) {
        CloseServiceHandle(hManager);
        return;
    }

    ResumeHandle = 0;
    while (1) {

        ULONG i, num, len;
        BOOL b = EnumServicesStatus(
                    hManager,
                    SERVICE_TYPE_ALL,
                    SERVICE_STATE_ALL,
                    info,
                    info_len,
                    &len,
                    &num,
                    &ResumeHandle);

        if (! b) {
            if (GetLastError() != ERROR_MORE_DATA)
                break;
        }

        for (i = 0; i < num; ++i) {
            _wcslwr(info[i].lpServiceName);
            m_services.AddTail(info[i].lpServiceName);
        }

        if (b)
            break;
    }

    HeapFree(GetProcessHeap(), 0, info);
    CloseServiceHandle(hManager);
}


//---------------------------------------------------------------------------
// CollectProducts
//---------------------------------------------------------------------------


void CThirdPartyDialog::CollectProducts()
{
    HKEY hkey;
    ULONG DesiredAccess;

    while (! m_products.IsEmpty())
        m_products.RemoveHead();

    DesiredAccess = KEY_READ;

    while (1) {

        const WCHAR *_Uninstall =
            L"Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall";
        LONG rc = RegOpenKeyEx(
            HKEY_LOCAL_MACHINE, _Uninstall, 0, DesiredAccess, &hkey);
        if (rc == 0) {

            ULONG index = 0;
            WCHAR name[128];
            while (rc != ERROR_NO_MORE_ITEMS) {
                ULONG name_len = 120;
                rc = RegEnumKeyEx(hkey, index, name, &name_len,
                                  NULL, NULL, NULL, NULL);
                if (rc == 0) {
                    _wcslwr(name);
                    m_products.AddTail(name);
                }
                ++index;
            }

            RegCloseKey(hkey);
        }

#ifdef _WIN64

        if (DesiredAccess & KEY_WOW64_32KEY)
            break;
        DesiredAccess |= KEY_WOW64_32KEY;
        continue;

#else // ! _WIN64

        break;

#endif _WIN64

    }
}


//---------------------------------------------------------------------------
// CollectTemplates
//---------------------------------------------------------------------------


void CThirdPartyDialog::CollectTemplates()
{
    CSbieIni &ini = CSbieIni::GetInstance();
    CBox &GlobalSettings = CBoxes::GetInstance().GetBox(0);

    CStringList names;
    ini.GetTemplateNames(L"EmailReader", names);
    ini.GetTemplateNames(L"Print", names);
    ini.GetTemplateNames(L"Security", names);
    ini.GetTemplateNames(L"Desktop", names);
    ini.GetTemplateNames(L"Download", names);
    ini.GetTemplateNames(L"Misc", names);
    ini.GetTemplateNames(L"WebBrowser", names);
    ini.GetTemplateNames(L"MediaPlayer", names);
    ini.GetTemplateNames(L"TorrentClient", names);
    
    m_templates.RemoveAll();
    while (! names.IsEmpty()) {
        CString tmpl_name = names.RemoveHead();
        m_templates.SetAt(tmpl_name, NULL);
    }

    void *dummy;
    CString tmpl_name;
    BOOL enabled;

    POSITION pos = m_templates.GetStartPosition();
    while (pos) {
        m_templates.GetNextAssoc(pos, tmpl_name, dummy);

        if (GlobalSettings.IsTemplateEnabled(tmpl_name))
            enabled = TRUE;
        else {
            enabled = CheckTemplate(tmpl_name);

            if ((! enabled) && GlobalSettings.IsTemplateRejected(tmpl_name))
                m_stale_templates.AddTail(tmpl_name);
        }

        if (enabled)
            m_templates.SetAt(tmpl_name, (void *)1);
    }
}


//---------------------------------------------------------------------------
// CheckTemplate
//---------------------------------------------------------------------------


BOOL CThirdPartyDialog::CheckTemplate(const CString &tmpl_name)
{
    CSbieIni &ini = CSbieIni::GetInstance();
    CString SectionName = ini.m_Template_ + tmpl_name;

    CStringList settings;
    ini.GetSettingsNames(SectionName, settings);

    CString scan;
    ini.GetText(SectionName, L"Tmpl.Scan", scan);
    BOOL scanIpc  = (scan.Find(L'i') != -1);
    BOOL scanWin  = (scan.Find(L'w') != -1);
    BOOL scanSvc  = (scan.Find(L's') != -1);

    if (! (scanIpc || scanWin || scanSvc))
        return FALSE;

    CStringList keys, files;

    while (! settings.IsEmpty()) {
        CString setting = settings.RemoveHead();

        CStringList *list = NULL;
        if (scanIpc && setting.CompareNoCase(L"OpenIpcPath") == 0)
            list = &m_objects;
        else if (scanWin && setting.CompareNoCase(L"OpenWinClass") == 0)
            list = &m_classes;
        else if (scanSvc && setting.CompareNoCase(L"Tmpl.ScanService") == 0)
            list = &m_services;
        else if (scanSvc && setting.CompareNoCase(L"Tmpl.ScanWinClass") == 0)
            list = &m_classes;
        else if (scanSvc && setting.CompareNoCase(L"Tmpl.ScanProduct") == 0)
            list = &m_products;
        else if (scanSvc && setting.CompareNoCase(L"Tmpl.ScanIpc") == 0)
            list = &m_objects;
        else if (scanSvc && setting.CompareNoCase(L"Tmpl.ScanKey") == 0)
            list = &keys;
        else if (scanSvc && setting.CompareNoCase(L"Tmpl.ScanFile") == 0)
            list = &files;
        else
            continue;

        CStringList values;
        ini.GetTextList(SectionName, setting, values);

        while (! values.IsEmpty()) {
            CString value = values.RemoveHead();

            if (list == &m_classes && value.Left(2).Compare(L"*:") == 0)
                continue;
            if (list == &m_objects) {
                if (value.Compare(L"\\RPC Control\\epmapper") == 0)
                    continue;
                if (value.Compare(L"\\RPC Control\\OLE*") == 0)
                    continue;
                if (value.Compare(L"\\RPC Control\\LRPC*") == 0)
                    continue;
                if (value.Compare(L"*\\BaseNamedObjects*\\"
                                       L"NamedBuffer*mAH*Process*API*") == 0)
                    continue;
            }

            if (list == &keys) {
                if (CheckRegistryKey(value))
                    return TRUE;
                continue;
            } else if (list == &files) {
                if (CheckFile(value))
                    return TRUE;
                continue;
            }

            PATTERN *pat = Pattern_Create(NULL, value, TRUE);
            BOOLEAN match = FALSE;

            POSITION pos = list->GetHeadPosition();
            while (pos && (! match)) {
                const CString &name = list->GetNext(pos);
                match = Pattern_Match(pat, name, name.GetLength());
            }

            Pattern_Free(pat);
            if (match)
                return TRUE;
        }
    }

    return FALSE;
}


//---------------------------------------------------------------------------
// CheckRegistryKey
//---------------------------------------------------------------------------


BOOL CThirdPartyDialog::CheckRegistryKey(const WCHAR *keypath)
{
    NTSTATUS status;
    OBJECT_ATTRIBUTES objattrs;
    UNICODE_STRING objname;
    HANDLE handle;

    RtlInitUnicodeString(&objname, keypath);
    InitializeObjectAttributes(
        &objattrs, &objname, OBJ_CASE_INSENSITIVE, NULL, NULL);

    status = NtOpenKey(&handle, KEY_QUERY_VALUE, &objattrs);
    if (NT_SUCCESS(status)) {
        NtClose(handle);
        return TRUE;
    }

    return FALSE;
}


//---------------------------------------------------------------------------
// CheckFile
//---------------------------------------------------------------------------


BOOL CThirdPartyDialog::CheckFile(const WCHAR *filepath)
{
    CString path = CSbieIni::GetInstance().MakeSpecificPath(filepath);
    if (GetFileAttributes(path) != INVALID_FILE_ATTRIBUTES)
        return TRUE;
    return FALSE;
}


//---------------------------------------------------------------------------
// FindConflicts
//---------------------------------------------------------------------------


void CThirdPartyDialog::FindConflicts(CStringList &list, int what)
{
    //
    // what == 0:  get new product codes
    // what == 1:  get all product codes
    // what == 2:  get all product names
    //

    CSbieIni &ini = CSbieIni::GetInstance();
    CString TemplateName = ini.m_Template_ + _KnownConflicts;
    CStringList entries;
    ini.GetTextList(TemplateName, L"Tmpl.Entry", entries);

    CStringList KnownConflicts;
    if (what == 0) {
        CString text;
        ini.GetText(_GlobalSettings, _KnownConflicts, text);
        while (! text.IsEmpty()) {

            int index = text.Find(',');
            if (index < 1)
                index = text.GetLength();
            CString left = text.Left(index);
            left.TrimLeft();
            left.TrimRight();
            if (! left.IsEmpty())
                KnownConflicts.AddTail(left);

            text = text.Mid(index + 1);
        }
    }

    while (! entries.IsEmpty()) {

        CString entry = entries.RemoveHead();
        int index = entry.Find(L'|');
        if (index < 1)
            continue;
        CString left  = entry.Left(index);
        CString right = entry.Mid(index + 1);
        left.TrimLeft();
        left.TrimRight();
        right.TrimLeft();
        right.TrimRight();
        if (right.IsEmpty())
            right = left;

        bool add = false;

        POSITION pos = m_products.GetHeadPosition();
        while (pos) {
            const CString &product = m_products.GetNext(pos);
            if (product.CompareNoCase(right) == 0) {
                add = true;
                break;
            }
        }

        if (add && (what == 0)) {
            pos = KnownConflicts.GetHeadPosition();
            while (pos) {
                const CString &product = KnownConflicts.GetNext(pos);
                if (product.CompareNoCase(right) == 0) {
                    add = false;
                    break;
                }
            }
        }

        if (add) {
            CString msg;
            msg.Format(L"Left=<%s> Right=<%s>\n", left, right);
            list.AddTail((what == 2) ? left : right);
        }
    }
}


//---------------------------------------------------------------------------
// WatchThread
//---------------------------------------------------------------------------


ULONG CThirdPartyDialog::WatchThread(void *lpParameter)
{
    ULONG_PTR *ThreadArgs = (ULONG_PTR *)lpParameter;
    HWND   hWnd   = (HWND)ThreadArgs[0];
    WPARAM wParam = (WPARAM)ThreadArgs[1];
    delete ThreadArgs;

    static const WCHAR *_KeyPath =
        L"Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall";

    HKEY hKey1 = NULL;
    HKEY hKey2 = NULL;
    HANDLE hEvent1 = NULL;
    HANDLE hEvent2 = NULL;

restart:

    if (hKey1) {
        CloseHandle(hKey1);
        hKey1 = NULL;
    }
    if (hEvent1) {
        CloseHandle(hEvent1);
        hEvent1 = NULL;
    }

    if (hKey2) {
        CloseHandle(hKey2);
        hKey2 = NULL;
    }
    if (hEvent2) {
        CloseHandle(hEvent1);
        hEvent2 = NULL;
    }

    RegOpenKeyEx(HKEY_LOCAL_MACHINE, _KeyPath, 0, KEY_NOTIFY, &hKey1);
    if (hKey1)
        hEvent1 = CreateEvent(NULL, TRUE, FALSE, NULL);

#ifdef _WIN64
    RegOpenKeyEx(HKEY_LOCAL_MACHINE, _KeyPath, 0,
                 KEY_NOTIFY | KEY_WOW64_32KEY, &hKey2);
    if (hKey2)
        hEvent2 = CreateEvent(NULL, TRUE, FALSE, NULL);
#endif _WIN64

    if (hKey1) {
        RegNotifyChangeKeyValue(
            hKey1, TRUE, REG_NOTIFY_CHANGE_NAME, hEvent1, TRUE);
    }
    if (hKey2) {
        RegNotifyChangeKeyValue(
            hKey2, TRUE, REG_NOTIFY_CHANGE_NAME, hEvent2, TRUE);
    }

    HANDLE Handles[2];
    ULONG HandleCount = 0;
    if (hKey1) {
        Handles[HandleCount] = hEvent1;
        ++HandleCount;
    }
    if (hKey2) {
        Handles[HandleCount] = hEvent2;
        ++HandleCount;
    }

    if (! HandleCount)
        return 0;
    LONG rc =
        WaitForMultipleObjects(HandleCount, Handles, FALSE, INFINITE);
    if (rc != STATUS_WAIT_0 && rc != STATUS_WAIT_1)
        return 0;

    Sleep(10 * 1000);
    ::PostMessage(hWnd, WM_COMMAND, wParam, 0);
    goto restart;
}


//---------------------------------------------------------------------------
// Import PATTERN module
//---------------------------------------------------------------------------


#pragma warning( disable: 4200 )

#define Pool_Alloc(pool,size) (PATTERN *)HeapAlloc(GetProcessHeap(),0,size)
#define Pool_Free(ptr,size)   HeapFree(GetProcessHeap(),0,ptr)

#include "common/pattern.c"
