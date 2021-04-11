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
// Box
//---------------------------------------------------------------------------


#include "stdafx.h"
#include "MyApp.h"
#include "Box.h"

#include "SbieIni.h"
#include "AppPage.h"


//---------------------------------------------------------------------------
// Variables
//---------------------------------------------------------------------------


static const CString _ConfigLevel(L"ConfigLevel");

static const CString _AlertProcess(L"AlertProcess");

static const CString _AutoRecover(L"AutoRecover");

static const CString _AutoDelete(L"AutoDelete");
static const CString _NeverDelete(L"NeverDelete");
static const CString _DeleteCommand(L"DeleteCommand");

static const CString _CopyLimitKb(L"CopyLimitKb");
static const CString _CopyLimitSilent(L"CopyLimitSilent");
static const CString _BoxNameTitle(L"BoxNameTitle");
static const CString _BorderColor(L"BorderColor");
static const CString _BlockDrivers(L"BlockDrivers");
static const CString _BlockFakeInput(L"BlockFakeInput");
static const CString _BlockSysParam(L"BlockSysParam");
static const CString _BlockPassword(L"BlockPassword");
static const CString _BlockNetParam(L"BlockNetParam");
static const CString _DropAdminRights(L"DropAdminRights");
static const CString _BlockeNetworkFiles(L"BlockNetworkFiles");
static const CString _AllowSpoolerPrintToFile(L"AllowSpoolerPrintToFile");

static const CString _InternetAccessDevices(L"InternetAccessDevices");
static const CString _AfdX(L"\\Device\\Afd*");
static const CString _TcpX(L"\\Device\\Tcp*");
static const CString _IpX(L"\\Device\\Ip*");
static const CString _Ip(L"\\Device\\Ip");
static const CString _Tcp(L"\\Device\\Tcp");
static const CString _Udp(L"\\Device\\Udp");
static const CString _RawIp(L"\\Device\\RawIp");
static const CString _Ip6(L"\\Device\\Ip6");
static const CString _Tcp6(L"\\Device\\Tcp6");
static const CString _Udp6(L"\\Device\\Udp6");
static const CString _RawIp6(L"\\Device\\RawIp6");
static const CString _Nsi(L"\\Device\\Nsi");
static const CString _Http(L"\\Device\\Http\\*");
static const CString _Star(L"*");

static const CString _NotifyInternetAccessDenied(
                                            L"NotifyInternetAccessDenied");
static const CString _NotifyStartRunAccessDenied(
                                            L"NotifyStartRunAccessDenied");

//static const WCHAR *BorderColor_off = L",off";
//static const WCHAR *BorderColor_ttl = L",ttl";


//---------------------------------------------------------------------------
// Public Variables
//---------------------------------------------------------------------------


const CString CBox::_Enabled            (L"Enabled");

const CString CBox::_ProcessGroup       (L"ProcessGroup");
const CString CBox::_InternetAccessGroup(L"InternetAccess");
const CString CBox::_StartRunAccessGroup(L"StartRunAccess");

const CString CBox::_AutoRecoverIgnore  (L"AutoRecoverIgnore");
const CString CBox::_ForceFolder        (L"ForceFolder");
const CString CBox::_ForceProcess       (L"ForceProcess");
const CString CBox::_LeaderProcess      (L"LeaderProcess");
const CString CBox::_LingerProcess      (L"LingerProcess");
const CString CBox::_RecoverFolder      (L"RecoverFolder");

const CString CBox::_ClosedFilePath     (L"ClosedFilePath");
const CString CBox::_ClosedIpcPath      (L"ClosedIpcPath");
const CString CBox::_ClosedKeyPath      (L"ClosedKeyPath");
const CString CBox::_OpenClsid          (L"OpenClsid");
const CString CBox::_OpenFilePath       (L"OpenFilePath");
const CString CBox::_OpenIpcPath        (L"OpenIpcPath");
const CString CBox::_OpenKeyPath        (L"OpenKeyPath");
const CString CBox::_OpenPipePath       (L"OpenPipePath");
const CString CBox::_OpenWinClass       (L"OpenWinClass");
const CString CBox::_ReadFilePath       (L"ReadFilePath");
const CString CBox::_ReadKeyPath        (L"ReadKeyPath");
const CString CBox::_WriteFilePath      (L"WriteFilePath");
const CString CBox::_WriteKeyPath       (L"WriteKeyPath");


//---------------------------------------------------------------------------
// Constructor
//---------------------------------------------------------------------------


CBox::CBox(const CString &name)
{
    m_name = name;
    m_BoxProc = NULL;
    m_BoxFile = NULL;

    if (m_name.IsEmpty()) {

        m_expandedView = TRUE;

    } else {

        m_expandedView = FALSE;

        SetDefaultSettings();
    }

    m_TemporarilyDisableQuickRecover = FALSE;
}


//---------------------------------------------------------------------------
// Destructor
//---------------------------------------------------------------------------


CBox::~CBox()
{
    if (m_BoxProc)
        delete m_BoxProc;

    if (m_BoxFile)
        delete m_BoxFile;
}


//---------------------------------------------------------------------------
// GetBoxProc
//---------------------------------------------------------------------------


CBoxProc &CBox::GetBoxProc()
{
    if (! m_BoxProc)
        m_BoxProc = new CBoxProc(m_name);
    return *m_BoxProc;
}


//---------------------------------------------------------------------------
// GetBoxFile
//---------------------------------------------------------------------------


CBoxFile &CBox::GetBoxFile()
{
    if (! m_BoxFile)
        m_BoxFile = new CBoxFile(m_name);
    return *m_BoxFile;
}


//---------------------------------------------------------------------------
// GetName
//---------------------------------------------------------------------------


const CString &CBox::GetName() const
{
    return m_name;
}


//---------------------------------------------------------------------------
// IsExpandedView
//---------------------------------------------------------------------------


BOOL CBox::IsExpandedView() const
{
    return m_expandedView;
}


//---------------------------------------------------------------------------
// SetExpandedView
//---------------------------------------------------------------------------


void CBox::SetExpandedView(BOOL view)
{
    m_expandedView = view;
}


//---------------------------------------------------------------------------
// SetDefaultSettings
//---------------------------------------------------------------------------


void CBox::SetDefaultSettings()
{
    CSbieIni &ini = CSbieIni::GetInstance();

    int cfglvl;
    ini.GetNum(m_name, _ConfigLevel, cfglvl);

    if (cfglvl >= 8)
        return;

    BOOL ok = TRUE;

    if (cfglvl >= 1) {

        ok = ini.SetNum(m_name, _ConfigLevel, 8);

        if (ok) {

            if (cfglvl == 7) {

                CAppPage::SetDefaultTemplates8(*this);

            }
            else if (cfglvl == 6) {

                CAppPage::SetDefaultTemplates7(*this);

            } else {

                CAppPage::UpdateTemplates(*this);
            }
        }

        goto done;
    }

    ok = ini.SetNum(m_name, _ConfigLevel, 8);

    if (ok)
    {
        //ok = ini.SetBool(m_name, _AutoRecover, TRUE);
        ok = ini.SetBool(m_name, L"BlockNetworkFiles", TRUE);
    }

    if (ok)
        CAppPage::SetDefaultTemplates6(*this);

    if (ok)
        ok = AddOrRemoveQuickRecoveryFolder(L"%Desktop%",    TRUE);
    //if (ok)
    //    ok = AddOrRemoveQuickRecoveryFolder(L"%Favorites%",  TRUE);
    if (ok)
        ok = AddOrRemoveQuickRecoveryFolder(L"%Personal%",   TRUE);
    if (ok && CMyApp::m_WindowsVista) {
        ok = AddOrRemoveQuickRecoveryFolder(
                L"%{374DE290-123F-4565-9164-39C4925E467B}%", TRUE);
    }

    if (ok)
        ok = SetBorder(TRUE, RGB(255,255,0), TRUE, 6);

done:

    if (! ok) {

        CMyMsg msg(MSG_3314, m_name);
        CMyApp::MsgBox(NULL, msg, MB_OK);
    }
}


//---------------------------------------------------------------------------
// SetEnabled
//---------------------------------------------------------------------------


void CBox::SetEnabled(const CString &BoxName)
{
    CSbieIni::GetInstance().SetBool(BoxName, _Enabled, TRUE);
}


//---------------------------------------------------------------------------
// GetSetProcessSetting
//---------------------------------------------------------------------------


BOOL CBox::GetProcessSetting(WCHAR type, const CString &image)
{
    if (image.IsEmpty())
        return FALSE;
    if (image.GetAt(0) == L'[')
        return FALSE;

    const CString *pName = &m_name;
    const CString *pSetting;
    if (type == L'A') {
        pName = &_GlobalSettings;
        pSetting = &_AlertProcess;
    } else if (type == L'O')
        pSetting = &_ForceFolder;
    else if (type == L'F')
        pSetting = &_ForceProcess;
    else if (type == L'L')
        pSetting = &_LingerProcess;
    else if (type == L'R')
        pSetting = &_LeaderProcess;
    else
        pSetting = NULL;

    CSbieIni &ini = CSbieIni::GetInstance();

    BOOL oldval = FALSE;
    CStringList list;
    ini.GetTextList(*pName, *pSetting, list);
    POSITION pos = list.GetHeadPosition();
    while (pos) {
        CString &text = list.GetNext(pos);
        if (text.CompareNoCase(image) == 0)
            oldval = TRUE;
    }

    return oldval;
}


//---------------------------------------------------------------------------
// SetProcessSetting
//---------------------------------------------------------------------------


BOOL CBox::SetProcessSetting(WCHAR type, const CString &image, BOOL newval)
{
    if (image.IsEmpty())
        return TRUE;
    if (image.GetAt(0) == L'[')
        return TRUE;

    const CString *pName = &m_name;
    const CString *pSetting;
    if (type == L'A') {
        pName = &_GlobalSettings;
        pSetting = &_AlertProcess;
    } else if (type == L'O')
        pSetting = &_ForceFolder;
    else if (type == L'F')
        pSetting = &_ForceProcess;
    else if (type == L'L')
        pSetting = &_LingerProcess;
    else if (type == L'R')
        pSetting = &_LeaderProcess;
    else
        pSetting = NULL;

    CSbieIni &ini = CSbieIni::GetInstance();

    BOOL ok = FALSE;

    if (newval == TRUE)
        ok = ini.InsertText(*pName, *pSetting, image);
    if (newval == FALSE)
        ok = ini.DelText(*pName, *pSetting, image);

    return ok;
}


//---------------------------------------------------------------------------
// GetProcessList
//---------------------------------------------------------------------------


void CBox::GetProcessList(WCHAR type, CStringList &list, BOOL withBrackets)
{
    const CString *pName = &m_name;
    const CString *pSetting;
    if (type == L'A') {
        pName = &_GlobalSettings;
        pSetting = &_AlertProcess;
    } else if (type == L'O')
        pSetting = &_ForceFolder;
    else if (type == L'F')
        pSetting = &_ForceProcess;
    else if (type == L'L')
        pSetting = &_LingerProcess;
    else if (type == L'R')
        pSetting = &_LeaderProcess;
    else
        pSetting = NULL;

    CSbieIni &ini = CSbieIni::GetInstance();
    ini.GetTextList(*pName, *pSetting, list, withBrackets);
}


//---------------------------------------------------------------------------
// GetProcessGroupsInBox
//---------------------------------------------------------------------------


void CBox::GetProcessGroupsInBox(CStringList &names)
{
    CSbieIni &ini = CSbieIni::GetInstance();

    CStringList list1;
    ini.GetTextList(m_name, _ProcessGroup, list1, TRUE);

    POSITION pos1 = list1.GetHeadPosition();
    while (pos1) {
        CString &text = list1.GetNext(pos1);
        int idxOpen = text.Find(L'<');
        int idxClose = text.Find(L'>');
        if (idxOpen != -1 && idxClose != -1 && idxClose > idxOpen) {
            CString name = text.Mid(idxOpen, idxClose + 1 - idxOpen);

            bool found = false;
            POSITION pos2 = names.GetHeadPosition();
            while (pos2) {
                CString &name2 = names.GetNext(pos2);
                if (name2.CompareNoCase(name) == 0) {
                    found = true;
                    break;
                }
            }

            if (! found)
                names.AddTail(name);
        }
    }
}


//---------------------------------------------------------------------------
// GetProcessGroupName
//---------------------------------------------------------------------------


void CBox::GetProcessGroupName(
    const CString &GroupName,
    CString &LocalGroupName, CString &GlobalGroupName)
{
    LocalGroupName = L"<" + GroupName;
    GlobalGroupName = LocalGroupName + L"_" + m_name + L">";
    LocalGroupName = LocalGroupName + L">";
}


//---------------------------------------------------------------------------
// GetProcessGroup
//---------------------------------------------------------------------------


CString CBox::GetProcessGroup(const CString &GroupName)
{
    CString value;

    //
    // collect all ProcessGroup settings from both the GlobalSettings
    // section and the sandbox section
    //

    CSbieIni &ini = CSbieIni::GetInstance();

    CStringList list;
    ini.GetTextList(_GlobalSettings, _ProcessGroup, list, TRUE);
    for (int i = 0; i < list.GetCount(); ++i) {
        CString str = list.RemoveHead();
        str = L"[" + str + L"]";
        list.AddTail(str);
    }

    CStringList list2;
    ini.GetTextList(m_name, _ProcessGroup, list2, TRUE);
    list.AddHead(&list2);

    CString LocalGroupName, GlobalGroupName;
    if (GroupName.GetAt(0) == L'<') {
        LocalGroupName = GroupName;
        GlobalGroupName = GroupName;
    } else
        GetProcessGroupName(GroupName, LocalGroupName, GlobalGroupName);

    //
    // scan each ProcessGroup setting, check if group name matches
    //

    POSITION pos = list.GetHeadPosition();
    while (pos) {
        CString &text = list.GetNext(pos);
        bool withBrackets = false;
        if (text.GetLength() > 2 && text.GetAt(0) == L'[') {
            withBrackets = true;
            text = text.Mid(1, text.GetLength() - 2);
        }
        int comma = text.Find(L',');
        if (comma != -1 && (
                text.Mid(0, comma) == LocalGroupName ||
                text.Mid(0, comma) == GlobalGroupName)) {

            //
            // collect the comma-separated list of processes
            //

            if (withBrackets && text.Mid(0, comma) == GlobalGroupName)
                withBrackets = false;

            int len = text.GetLength();
            while (1) {
                while (comma < len && text.GetAt(comma) == L',')
                    ++comma;
                if (comma == len)
                    break;
                int comma2 = text.Find(L',', comma);
                if (value.GetLength())
                    value +=  L",";
                if (withBrackets)
                    value += L"[";
                if (comma2 == -1)
                    value += text.Mid(comma);
                else {
                    value += text.Mid(comma, comma2 - comma);
                    comma = comma2 + 1;
                }
                if (withBrackets)
                    value += L"]";
                if (comma2 == -1)
                    break;
            }
        }
    }

    return value;
}


//---------------------------------------------------------------------------
// SetProcessGroup
//---------------------------------------------------------------------------


void CBox::SetProcessGroup(const CString &GroupName, const CString &value)
{
    //
    // collect all ProcessGroup settings from both the GlobalSettings
    // section and the sandbox section
    //

    CSbieIni &ini = CSbieIni::GetInstance();

    CStringList list;
    ini.GetTextList(_GlobalSettings, _ProcessGroup, list);
    CStringList list2;
    ini.GetTextList(m_name, _ProcessGroup, list2);
    list.AddHead(&list2);

    CString LocalGroupName, GlobalGroupName;
    if (GroupName.GetAt(0) == L'<') {
        LocalGroupName = GroupName;
        GlobalGroupName = CString();    // do not delete from global
    } else
        GetProcessGroupName(GroupName, LocalGroupName, GlobalGroupName);

    //
    // scan each ProcessGroup setting, check if group name matches
    //

    POSITION pos = list.GetHeadPosition();
    while (pos) {
        CString &text = list.GetNext(pos);
        int comma = text.Find(L',');
        if (comma != -1 && (
                text.Mid(0, comma) == LocalGroupName ||
                text.Mid(0, comma) == GlobalGroupName)) {

            //
            // delete matching setting
            //

            ini.DelText(m_name, _ProcessGroup, text);
            if (text.Mid(0, comma) == GlobalGroupName)
                ini.DelText(_GlobalSettings, _ProcessGroup, text);
        }
    }

    //
    // Add new setting
    //

    if (value.GetLength()) {

        CString text = LocalGroupName + L"," + value;

        CSbieIni::GetInstance().InsertText(m_name, _ProcessGroup, text);
    }
}


//---------------------------------------------------------------------------
// GetInternetProcess
//---------------------------------------------------------------------------


CString CBox::GetInternetProcess(BOOL *pNotify)
{
    CSbieIni &ini = CSbieIni::GetInstance();

    if (pNotify)
        ini.GetBool(m_name, _NotifyInternetAccessDenied, *pNotify, TRUE);

    //
    // check if the Internet resource is blocked unconditionally,
    // without a qualifying program name or process group names
    //

    CStringList list;
    ini.GetTextList(m_name, _ClosedFilePath, list);
    POSITION pos = list.GetHeadPosition();
    while (pos) {
        CString &text = list.GetNext(pos);
        int comma = text.Find(L',');
        if (comma == -1 &&
                text == _AfdX || text == _InternetAccessDevices)
            return _Star;
    }

    //
    // otherwise check for program name or process group name
    //

    CString LocalGroupName, GlobalGroupName;
    GetProcessGroupName(
        _InternetAccessGroup, LocalGroupName, GlobalGroupName);

    pos = list.GetHeadPosition();
    while (pos) {
        CString &text = list.GetNext(pos);
        int comma = text.Find(L',');
        if (comma != -1 && (
                text.Mid(comma + 1) == _AfdX ||
                text.Mid(comma + 1) == _InternetAccessDevices)) {

            CString retval = text.Mid(0, comma);
            if ((! retval.IsEmpty()) && retval.Mid(0, 1) == L'!')
                retval = retval.Mid(1);

            if (retval == LocalGroupName || retval == GlobalGroupName)
                retval = GetProcessGroup(_InternetAccessGroup);

            return retval;
        }
    }

    return CString();
}


//---------------------------------------------------------------------------
// SetInternetProcess
//---------------------------------------------------------------------------


BOOL CBox::SetInternetProcess(const CString &image, BOOL *pNotify)
{
    CSbieIni &ini = CSbieIni::GetInstance();

    if (pNotify)
        ini.SetBool(m_name, _NotifyInternetAccessDenied, *pNotify);

    //
    // remove old settings
    //

    CString LocalGroupName, GlobalGroupName;
    GetProcessGroupName(
        _InternetAccessGroup, LocalGroupName, GlobalGroupName);

    SetProcessGroup(_InternetAccessGroup, L"");

    BOOL ok = ini.DelText2(m_name, _ClosedFilePath,
                           _InternetAccessDevices, LocalGroupName);

    //
    // remove old settings coded in the old style
    //

    if (ok)
        ok = ini.DelText2(
                m_name, _ClosedFilePath, _AfdX, GlobalGroupName);
    if (ok)
        ok = ini.DelText2(
                m_name, _ClosedFilePath, _TcpX, GlobalGroupName);
    if (ok)
        ok = ini.DelText2(
                m_name, _ClosedFilePath, _IpX, GlobalGroupName);
    if (ok)
        ok = ini.DelText2(
                m_name, _ClosedFilePath, _Ip, GlobalGroupName);
    if (ok)
        ok = ini.DelText2(
                m_name, _ClosedFilePath, _Tcp, GlobalGroupName);
    if (ok)
        ok = ini.DelText2(
                m_name, _ClosedFilePath, _Udp, GlobalGroupName);
    if (ok)
        ok = ini.DelText2(
                m_name, _ClosedFilePath, _RawIp, GlobalGroupName);
    if (ok)
        ok = ini.DelText2(
                m_name, _ClosedFilePath, _Ip6, GlobalGroupName);
    if (ok)
        ok = ini.DelText2(
                m_name, _ClosedFilePath, _Tcp6, GlobalGroupName);
    if (ok)
        ok = ini.DelText2(
                m_name, _ClosedFilePath, _Udp6, GlobalGroupName);
    if (ok)
        ok = ini.DelText2(
                m_name, _ClosedFilePath, _RawIp6, GlobalGroupName);
    if (ok)
        ok = ini.DelText2(
                m_name, _ClosedFilePath, _Nsi, GlobalGroupName);
    if (ok)
        ok = ini.DelText2(
                m_name, _ClosedFilePath, _Http, GlobalGroupName);

    //
    // add new settings
    //

    if (! image.IsEmpty()) {

        CString pfx;
        if (image.Compare(_Star) != 0) {
            SetProcessGroup(_InternetAccessGroup, image);
            pfx = L"!" + LocalGroupName + L",";
        }

        if (ok)
            ok = ini.InsertText(m_name, _ClosedFilePath,
                                pfx + _InternetAccessDevices);
    }

    return ok;
}


//---------------------------------------------------------------------------
// GetStartRunProcess
//---------------------------------------------------------------------------


CString CBox::GetStartRunProcess(BOOL *pNotify)
{
    CSbieIni &ini = CSbieIni::GetInstance();

    if (pNotify)
        ini.GetBool(m_name, _NotifyStartRunAccessDenied, *pNotify, TRUE);

    //
    // check if the Start/Run resource is blocked unconditionally,
    // without a qualifying program name or process group names
    //

    CStringList list;
    ini.GetTextList(m_name, _ClosedIpcPath, list);
    POSITION pos = list.GetHeadPosition();
    while (pos) {
        CString &text = list.GetNext(pos);
        int comma = text.Find(L',');
        if (comma == -1 && text == _Star)
            return _Star;
    }

    //
    // otherwise check for program name or process group name
    //

    CString LocalGroupName, GlobalGroupName;
    GetProcessGroupName(
        _StartRunAccessGroup, LocalGroupName, GlobalGroupName);

    pos = list.GetHeadPosition();
    while (pos) {
        CString &text = list.GetNext(pos);
        int comma = text.Find(L',');
        if (comma != -1 && text.Mid(comma + 1) == _Star) {

            CString retval = text.Mid(0, comma);
            if ((! retval.IsEmpty()) && retval.Mid(0, 1) == L'!')
                retval = retval.Mid(1);

            if (retval == LocalGroupName || retval == GlobalGroupName)
                retval = GetProcessGroup(_StartRunAccessGroup);

            return retval;
        }
    }

    return CString();
}


//---------------------------------------------------------------------------
// SetStartRunProcess
//---------------------------------------------------------------------------


BOOL CBox::SetStartRunProcess(const CString &image, BOOL *pNotify)
{
    CSbieIni &ini = CSbieIni::GetInstance();

    if (pNotify)
        ini.SetBool(m_name, _NotifyStartRunAccessDenied, *pNotify);

    //
    // remove old settings
    //

    CString LocalGroupName, GlobalGroupName;
    GetProcessGroupName(
        _StartRunAccessGroup, LocalGroupName, GlobalGroupName);

    SetProcessGroup(_StartRunAccessGroup, L"");

    BOOL ok = ini.DelText2(m_name, _ClosedIpcPath, _Star, LocalGroupName);

    //
    // remove old settings coded in the old style
    //

    if (ok)
        ok = ini.DelText2(m_name, _ClosedIpcPath, _Star, GlobalGroupName);

    //
    // add new settings
    //

    if (! image.IsEmpty()) {

        CString pfx;
        if (image.Compare(_Star) != 0) {
            SetProcessGroup(_StartRunAccessGroup, image);
            pfx = L"!" + LocalGroupName + L",";
        }

        if (ok)
            ok = ini.InsertText(m_name, _ClosedIpcPath, pfx + _Star);
    }

    return ok;
}


//---------------------------------------------------------------------------
// AddOrRemoveQuickRecoveryFolder
//---------------------------------------------------------------------------


BOOL CBox::AddOrRemoveQuickRecoveryFolder(const CString &folder, BOOL add)
{
    if (folder.IsEmpty())
        return TRUE;
    if (folder.GetAt(0) == L'[')
        return TRUE;

    CSbieIni &ini = CSbieIni::GetInstance();

    CString str = folder;
    while (1) {
        int len = str.GetLength();
        if (len > 1 && str.GetAt(len - 1) == L'\\')
            str = str.Left(len - 1);
        else
            break;
    }
    CString folder2 = ini.MakeGenericPath(str);

    BOOL ok = ini.DelText(m_name, _RecoverFolder, folder);
    if (ok)
        ok = ini.DelText(m_name, _RecoverFolder, folder + L"\\");

    if (ok) {
        if (add)
            ok = ini.InsertText(m_name, _RecoverFolder, folder2);
        else {
            ini.DelText(m_name, _RecoverFolder, folder2);
            ini.DelText(m_name, _RecoverFolder, folder2 + L"\\");
        }
    }

    if (m_BoxFile)
        m_BoxFile->RebuildQuickRecoveryFolders();

    return ok;
}


//---------------------------------------------------------------------------
// GetQuickRecoveryFolders
//---------------------------------------------------------------------------


void CBox::GetQuickRecoveryFolders(CStringList &list, BOOL withBrackets)
{
    CSbieIni &ini = CSbieIni::GetInstance();
    ini.GetTextList(m_name, _RecoverFolder, list, withBrackets);

    POSITION pos = list.GetHeadPosition();
    while (pos) {
        POSITION oldpos = pos;

        CString str = list.GetNext(pos);
        withBrackets = CSbieIni::StripBrackets(str);

        while (1) {
            int len = str.GetLength();
            if (len > 1 && str.GetAt(len - 1) == L'\\')
                str = str.Left(len - 1);
            else
                break;
        }

        if (withBrackets)
            str = CSbieIni::AddBrackets(str);

        list.SetAt(oldpos, str);
    }
}


//---------------------------------------------------------------------------
// AddOrRemoveAutoRecoveryIgnores
//---------------------------------------------------------------------------


BOOL CBox::AddOrRemoveAutoRecoveryIgnores(const CString &item, BOOL add)
{
    if (item.IsEmpty())
        return TRUE;
    if (item.GetAt(0) == L'[')
        return TRUE;

    CSbieIni &ini = CSbieIni::GetInstance();

    CString str = item;
    while (1) {
        int len = str.GetLength();
        if (len > 1 && str.GetAt(len - 1) == L'\\')
            str = str.Left(len - 1);
        else
            break;
    }
    CString item2 = ini.MakeGenericPath(str);

    BOOL ok = ini.DelText(m_name, _AutoRecoverIgnore, item);
    if (ok)
        ok = ini.DelText(m_name, _AutoRecoverIgnore, item + L"\\");

    if (ok) {
        if (add)
            ini.InsertText(m_name, _AutoRecoverIgnore, item2);
        else {
            ini.DelText(m_name, _AutoRecoverIgnore, item2);
            ini.DelText(m_name, _AutoRecoverIgnore, item2 + L"\\");
        }
    }

    return ok;
}


//---------------------------------------------------------------------------
// GetAutoRecoveryIgnores
//---------------------------------------------------------------------------


void CBox::GetAutoRecoveryIgnores(CStringList &list, BOOL withBrackets)
{
    CSbieIni &ini = CSbieIni::GetInstance();
    ini.GetTextList(m_name, _AutoRecoverIgnore, list, withBrackets);

    POSITION pos = list.GetHeadPosition();
    while (pos) {
        POSITION oldpos = pos;

        CString str = list.GetNext(pos);
        withBrackets = CSbieIni::StripBrackets(str);

        while (1) {
            int len = str.GetLength();
            if (len > 1 && str.GetAt(len - 1) == L'\\')
                str = str.Left(len - 1);
            else
                break;
        }

        if (withBrackets)
            str = CSbieIni::AddBrackets(str);

        list.SetAt(oldpos, str);
    }
}


//---------------------------------------------------------------------------
// GetImmediateRecoveryState
//---------------------------------------------------------------------------


BOOL CBox::GetImmediateRecoveryState() const
{
    BOOL value;
    if (m_TemporarilyDisableQuickRecover)
        value = FALSE;
    else {
        CSbieIni &ini = CSbieIni::GetInstance();
        ini.GetBool(m_name, _AutoRecover, value, FALSE);
    }
    return value;
}


//---------------------------------------------------------------------------
// SetImmediateRecoveryState
//---------------------------------------------------------------------------


void CBox::SetImmediateRecoveryState(BOOL enabled, BOOL temporarily)
{
    if (temporarily)
        m_TemporarilyDisableQuickRecover = ! enabled;
    else {
        m_TemporarilyDisableQuickRecover = FALSE;
        CSbieIni &ini = CSbieIni::GetInstance();
        if (enabled)
            ini.SetBool(m_name, _AutoRecover, enabled);
        else
            ini.SetText(m_name, _AutoRecover, L"");
    }
}


//---------------------------------------------------------------------------
// GetAutoDelete
//---------------------------------------------------------------------------


BOOL CBox::GetAutoDelete() const
{
    BOOL value;
    CSbieIni &ini = CSbieIni::GetInstance();
    ini.GetBool(m_name, _AutoDelete, value, FALSE);
    return value;
}


//---------------------------------------------------------------------------
// SetAutoDelete
//---------------------------------------------------------------------------


void CBox::SetAutoDelete(BOOL enabled)
{
    CSbieIni &ini = CSbieIni::GetInstance();
    if (enabled)
        ini.SetBool(m_name, _AutoDelete, enabled);
    else
        ini.SetText(m_name, _AutoDelete, L"");
}


//---------------------------------------------------------------------------
// GetNeverDelete
//---------------------------------------------------------------------------


BOOL CBox::GetNeverDelete() const
{
    BOOL value;
    CSbieIni &ini = CSbieIni::GetInstance();
    ini.GetBool(m_name, _NeverDelete, value, FALSE);
    return value;
}


//---------------------------------------------------------------------------
// SetNeverDelete
//---------------------------------------------------------------------------


void CBox::SetNeverDelete(BOOL enabled)
{
    CSbieIni &ini = CSbieIni::GetInstance();
    ini.SetBool(m_name, _NeverDelete, enabled);
}


//---------------------------------------------------------------------------
// GetDeleteCommand
//---------------------------------------------------------------------------


CString CBox::GetDeleteCommand() const
{
    CString cmd;
    CSbieIni &ini = CSbieIni::GetInstance();
    ini.GetText(m_name, _DeleteCommand, cmd);
    return cmd;
}


//---------------------------------------------------------------------------
// SetDeleteCommand
//---------------------------------------------------------------------------


void CBox::SetDeleteCommand(const CString &cmd) const
{
    CSbieIni &ini = CSbieIni::GetInstance();
    ini.SetText(m_name, _DeleteCommand, cmd);
}


//---------------------------------------------------------------------------
// GetSetCopyLimit
//---------------------------------------------------------------------------


ULONG CBox::GetSetCopyLimit(BOOL set, ULONG newval)
{
    CSbieIni &ini = CSbieIni::GetInstance();
    int value = 0;
    if (set)
        value = ini.SetNum(m_name, _CopyLimitKb, newval);
    else
        ini.GetNum(m_name, _CopyLimitKb, value, (48 * 1024));
    return value;
}


//---------------------------------------------------------------------------
// GetSetCopySilent
//---------------------------------------------------------------------------


BOOL CBox::GetSetCopySilent(BOOL set, BOOL newval)
{
    CSbieIni &ini = CSbieIni::GetInstance();
    BOOL value = FALSE;
    if (set) {
        if (newval)
            ini.SetBool(m_name, _CopyLimitSilent, newval);
        else
            ini.SetText(m_name, _CopyLimitSilent, L"");
    } else
        ini.GetBool(m_name, _CopyLimitSilent, value);
    return value;
}


//---------------------------------------------------------------------------
// GetBoxNameTitle
//---------------------------------------------------------------------------


UCHAR CBox::GetBoxNameTitle() const
{
    CString value;
    CSbieIni &ini = CSbieIni::GetInstance();
    ini.GetText(m_name, _BoxNameTitle, value);
    if (! value.IsEmpty()) {
        WCHAR ch = value.GetAt(0);
        if (ch == L'y' || ch == L'Y')
            return 'y';
        if (ch == L'-')
            return '-';
    }
    return 'n';
}


//---------------------------------------------------------------------------
// SetBoxNameTitle
//---------------------------------------------------------------------------


BOOL CBox::SetBoxNameTitle(UCHAR enabled)
{
    CSbieIni &ini = CSbieIni::GetInstance();
    CString value;
    if (enabled == '-')
        value = L'-';
    else if (enabled != L'y')
        value = L'n';
    else
        value = L'y';
    CString strEnabled;
    strEnabled = (char)enabled;
    return ini.SetText(m_name, _BoxNameTitle, strEnabled);
}


//---------------------------------------------------------------------------
// GetBorder
//---------------------------------------------------------------------------


BOOL CBox::GetBorder(COLORREF *color, BOOL *title, int* width)
{
    /*color = RGB(255,255,0);
    *title = FALSE;

    CString text;
    CSbieIni &ini = CSbieIni::GetInstance();
    ini.GetText(m_name, _BorderColor, text);

    if (text.GetLength() < 7 || text.GetAt(0) != L'#')
        return FALSE;
    WCHAR *endptr;
    CString text_1_6 = text.Mid(1, 6);
    *color = wcstol(text_1_6, &endptr, 16);
    if (*endptr != L'\0') {
        *color = RGB(255,255,0);
        return FALSE;
    }

    if (text.Mid(7, 4).CompareNoCase(BorderColor_ttl) == 0) {
        *title = TRUE;
        if (text.Mid(11).CompareNoCase(BorderColor_off) == 0)
            return FALSE;

    } else if (text.Mid(7).CompareNoCase(BorderColor_off) == 0)
        return FALSE;

    return TRUE;*/

    return SbieDll_GetBorderColor(m_name, color, title, width);
}


//---------------------------------------------------------------------------
// SetBorder
//---------------------------------------------------------------------------


BOOL CBox::SetBorder(BOOL enabled, COLORREF color, BOOL title, int width)
{
    WCHAR text[32];
    swprintf(text, L"#%06X,%s,%d", color, !enabled ? L"off" : (title ? L"ttl" : L"on"), width);

    /*swprintf(text, L"#%06X", color);
    if (title)
        wcscat(text, BorderColor_ttl);
    if (! enabled)
        wcscat(text, BorderColor_off);*/
    
    CSbieIni &ini = CSbieIni::GetInstance();
    return ini.SetText(m_name, _BorderColor, text);
}


//---------------------------------------------------------------------------
// GetSetBlock
//---------------------------------------------------------------------------


BOOL CBox::GetSetBlock(WCHAR type, BOOL set, BOOL newval)
{
    CSbieIni &ini = CSbieIni::GetInstance();

    const CString *pSetting;
    if (type == L'D')
        pSetting = &_BlockDrivers;
    else if (type == L'K')
        pSetting = &_BlockFakeInput;
    else if (type == L'P')
        pSetting = &_BlockSysParam;
    else if (type == L'W')
        pSetting = &_BlockPassword;
    else if (type == L'N')
        pSetting = &_BlockNetParam;
    else
        pSetting = NULL;

    BOOL value = FALSE;
    if (set)
        ini.SetBool(m_name, *pSetting, newval);
    else
        ini.GetBool(m_name, *pSetting, value, TRUE);
    return value;
}


//---------------------------------------------------------------------------
// GetXxxPathList
//---------------------------------------------------------------------------


void CBox::GetXxxPathList(ULONG type, CStringList &list, BOOL withBrackets)
{
    const CString *pSetting;
    if (type == 'fo')
        pSetting = &_OpenFilePath;
    else if (type == 'po')
        pSetting = &_OpenPipePath;
    else if (type == 'fc')
        pSetting = &_ClosedFilePath;
    else if (type == 'fr')
        pSetting = &_ReadFilePath;
    else if (type == 'fw')
        pSetting = &_WriteFilePath;
    else if (type == 'ko')
        pSetting = &_OpenKeyPath;
    else if (type == 'kc')
        pSetting = &_ClosedKeyPath;
    else if (type == 'kr')
        pSetting = &_ReadKeyPath;
    else if (type == 'kw')
        pSetting = &_WriteKeyPath;
    else if (type == 'io')
        pSetting = &_OpenIpcPath;
    else if (type == 'ic')
        pSetting = &_ClosedIpcPath;
    else if (type == 'wo')
        pSetting = &_OpenWinClass;
    else if (type == 'co')
        pSetting = &_OpenClsid;
    else
        pSetting = NULL;

    CSbieIni &ini = CSbieIni::GetInstance();
    ini.GetTextList(m_name, *pSetting, list, withBrackets);
}


//---------------------------------------------------------------------------
// GetXxxPathListForPgm
//---------------------------------------------------------------------------


void CBox::GetXxxPathListForPgm(
    ULONG type, CStringList &list, const CString &pgm, BOOL withBrackets)
{
    CStringList list1;
    GetXxxPathList(type, list1, withBrackets);

    CString xpgm = pgm;
    if (xpgm == L'*')
        xpgm = CString();

    while (! list1.IsEmpty()) {

        CString val = list1.RemoveHead();
        withBrackets = CSbieIni::StripBrackets(val);

        CString valpgm;
        int index = val.Find(L',');
        if (index >= 1) {
            valpgm = val.Left(index);
            if (index + 1 < val.GetLength())
                val = val.Mid(index + 1);
            else
                val = CString();
            if (valpgm == L'*')
                valpgm = CString();
        }

        if (valpgm.CompareNoCase(xpgm) == 0) {
            if (withBrackets)
                val = CSbieIni::AddBrackets(val);
            list.AddTail(val);
        }
    }
}


//---------------------------------------------------------------------------
// SetXxxPathListForPgm
//---------------------------------------------------------------------------


void CBox::SetXxxPathListForPgm(
    ULONG type, const CStringList &list, const CString &pgm)
{
    CSbieIni &ini = CSbieIni::GetInstance();

    const CString *pSetting;
    if (type == 'fo')
        pSetting = &_OpenFilePath;
    else if (type == 'po')
        pSetting = &_OpenPipePath;
    else if (type == 'fc')
        pSetting = &_ClosedFilePath;
    else if (type == 'fr')
        pSetting = &_ReadFilePath;
    else if (type == 'fw')
        pSetting = &_WriteFilePath;
    else if (type == 'ko')
        pSetting = &_OpenKeyPath;
    else if (type == 'kc')
        pSetting = &_ClosedKeyPath;
    else if (type == 'kr')
        pSetting = &_ReadKeyPath;
    else if (type == 'kw')
        pSetting = &_WriteKeyPath;
    else if (type == 'io')
        pSetting = &_OpenIpcPath;
    else if (type == 'ic')
        pSetting = &_ClosedIpcPath;
    else if (type == 'wo')
        pSetting = &_OpenWinClass;
    else if (type == 'co')
        pSetting = &_OpenClsid;
    else
        pSetting = NULL;

    CStringList listOld;
    GetXxxPathList(type, listOld);

    CString xpgm = pgm;
    if (xpgm == L'*')
        xpgm = CString();

    while (! listOld.IsEmpty()) {
        CString val = listOld.RemoveHead();
        CString valpgm;
        int index = val.Find(L',');
        if (index >= 1) {
            valpgm = val.Left(index);
            if (valpgm == L'*')
                valpgm = CString();
        }
        if (valpgm.CompareNoCase(xpgm) == 0)
            ini.DelText(m_name, *pSetting, val);
    }

    POSITION pos = list.GetTailPosition();
    while (pos) {
        CString val = list.GetPrev(pos);
        if (val.IsEmpty())
            continue;
        if (val.GetAt(0) == L'[')
            continue;
        if (! xpgm.IsEmpty())
            val = xpgm + L"," + val;
        ini.InsertText(m_name, *pSetting, val);
    }
}


//---------------------------------------------------------------------------
// GetSetDropAdminRights
//---------------------------------------------------------------------------


BOOL CBox::GetSetDropAdminRights(BOOL set, BOOL newval)
{
    CSbieIni &ini = CSbieIni::GetInstance();
    BOOL value = FALSE;
    if (set) {
        if (newval)
            value = ini.SetBool(m_name, _DropAdminRights, newval);
        else
            value = ini.SetText(m_name, _DropAdminRights, L"");
    } else
        ini.GetBool(m_name, _DropAdminRights, value, FALSE);
    return value;
}


//---------------------------------------------------------------------------
// GetBlockNetworkFiles
//---------------------------------------------------------------------------


BOOL CBox::GetSetBlockNetworkFiles(BOOL set, BOOL newval)
{
    CSbieIni &ini = CSbieIni::GetInstance();
    BOOL value = FALSE;
    if (set) {
        if (newval)
            value = ini.SetBool(m_name, _BlockeNetworkFiles, newval);
        else
            value = ini.SetText(m_name, _BlockeNetworkFiles, L"");
    }
    else
        ini.GetBool(m_name, _BlockeNetworkFiles, value, FALSE);
    return value;
}


//---------------------------------------------------------------------------
// GetSetAllowSpoolerPrintToFile
//---------------------------------------------------------------------------


BOOL CBox::GetSetAllowSpoolerPrintToFile(BOOL set, BOOL newval)
{
    CSbieIni &ini = CSbieIni::GetInstance();
    BOOL value = FALSE;
    if (set) {
        if (newval)
            value = ini.SetBool(m_name, _AllowSpoolerPrintToFile, newval);
        else
            value = ini.SetText(m_name, _AllowSpoolerPrintToFile, L"");
    }
    else
        ini.GetBool(m_name, _AllowSpoolerPrintToFile, value, FALSE);
    return value;
}



//---------------------------------------------------------------------------
// GetSetUserAccounts
//---------------------------------------------------------------------------


void CBox::GetSetUserAccounts(
    const CString &BoxName, BOOL set, CStringList &list)
{
    CSbieIni &ini = CSbieIni::GetInstance();
    if (set) {

        CString text = L"y";
        while (! list.IsEmpty()) {
            CString user = list.RemoveHead();
            user.TrimLeft();
            user.TrimRight();
            text += L"," + user;
        }
        ini.SetText(BoxName, _Enabled, text);

    } else {

        while (! list.IsEmpty())
            list.RemoveHead();
        CString text;
        ini.GetText(BoxName, _Enabled, text);
        if (text.GetLength() &&
                (text.GetAt(0) == L'y' || text.GetAt(0) == L'Y')) {

            int index = text.Find(L',');
            while (index != -1 && (index + 1 < text.GetLength())) {
                ++index;
                int index2 = text.Find(L',', index);
                if (index2 == -1)
                    index2 = text.GetLength();
                CString user = text.Mid(index, index2 - index);
                user.TrimLeft();
                user.TrimRight();
                if (user.GetLength())
                    list.AddTail(user);
                index = index2;
            }
        }
    }
}


//---------------------------------------------------------------------------
// GetSetUserAccounts
//---------------------------------------------------------------------------


void CBox::GetSetUserAccounts(BOOL set, CStringList &list)
{
    GetSetUserAccounts(m_name, set, list);
}


//---------------------------------------------------------------------------
// IsTemplateEnabled
//---------------------------------------------------------------------------


BOOL CBox::IsTemplateEnabled(const WCHAR *TemplateName)
{
    CSbieIni &ini = CSbieIni::GetInstance();

    //
    // check easy case:  if box (or global settings, for the null box)
    // includes the setting Template=TemplateName
    //

    CString SectionName = m_name.IsEmpty() ? _GlobalSettings : m_name;

    CStringList values;
    ini.GetTextList(SectionName, ini.m_Template, values);

    POSITION pos1 = values.GetHeadPosition();
    while (pos1) {
        CString value = values.GetNext(pos1);
        if (value.CompareNoCase(TemplateName) == 0)
            return TRUE;
    }

    //
    // don't check for harder case if this is the null box / global settings
    //

    if (m_name.IsEmpty())
        return FALSE;

    //
    // harder case:  box includes all the settings for the template
    //

    CString TemplateSectionName = ini.m_Template_ + TemplateName;

    CStringList tmpl_names;
    ini.GetSettingsNames(TemplateSectionName, tmpl_names);

    BOOL at_least_one = FALSE;

    pos1 = tmpl_names.GetHeadPosition();
    while (pos1) {
        CString name = tmpl_names.GetNext(pos1);
        if (name.Left(5).CompareNoCase(ini.m_Tmpl_) == 0)
            continue;

        while (! values.IsEmpty())
            values.RemoveHead();
        ini.GetTextList(m_name, name, values, TRUE);

        CStringList tmpl_values;
        ini.GetTextList(TemplateSectionName, name, tmpl_values);

        POSITION pos2 = tmpl_values.GetHeadPosition();
        while (pos2) {
            CString tmpl_value = tmpl_values.GetNext(pos2);

            BOOL tmpl_value_found_in_box = FALSE;

            POSITION pos3 = values.GetHeadPosition();
            while (pos3) {
                CString value = values.GetNext(pos3);
                if (value.CompareNoCase(tmpl_value) == 0) {
                    tmpl_value_found_in_box = TRUE;
                    break;
                }
            }

            if (! tmpl_value_found_in_box)
                return FALSE;
            at_least_one = TRUE;
        }
    }

    return at_least_one;
}


//---------------------------------------------------------------------------
// EnableTemplate
//---------------------------------------------------------------------------


BOOL CBox::EnableTemplate(const WCHAR *TemplateName, BOOL enable)
{
    CSbieIni &ini = CSbieIni::GetInstance();
    BOOL ok = TRUE;

    //
    // first, remove any settings from the template that appear for the box
    //

    if (! m_name.IsEmpty()) {

        CString TemplateSectionName = ini.m_Template_ + TemplateName;

        CStringList tmpl_names;
        ini.GetSettingsNames(TemplateSectionName, tmpl_names);

        POSITION pos1 = tmpl_names.GetHeadPosition();
        while (pos1 && ok) {
            CString name = tmpl_names.GetNext(pos1);
            if (name.Left(5).CompareNoCase(ini.m_Tmpl_) == 0)
                continue;

            CStringList tmpl_values;
            ini.GetTextList(TemplateSectionName, name, tmpl_values);

            POSITION pos2 = tmpl_values.GetHeadPosition();
            while (pos2 && ok) {
                CString tmpl_value = tmpl_values.GetNext(pos2);
                ok = ini.DelText(m_name, name, tmpl_value);
            }
        }
    }

    //
    // second, either set or remove the setting Template=TemplateName
    //

    if (ok) {

        CString SectionName = m_name.IsEmpty() ? _GlobalSettings : m_name;

        if (enable)
            ok = ini.InsertText(SectionName, ini.m_Template, TemplateName);
        else
            ok = ini.DelText(SectionName, ini.m_Template, TemplateName);
    }

    //
    // third, update quick recovery folders, in case the template
    // contains RecoverFolder settings
    //

    if (m_BoxFile)
        m_BoxFile->RebuildQuickRecoveryFolders();

    return ok;
}


//---------------------------------------------------------------------------
// IsTemplateRejected
//---------------------------------------------------------------------------


BOOL CBox::IsTemplateRejected(const WCHAR *TemplateName)
{
    CSbieIni &ini = CSbieIni::GetInstance();

    CString SectionName = m_name.IsEmpty() ? _GlobalSettings : m_name;
    CString SettingName = ini.m_Template + L"Reject";

    CStringList values;
    ini.GetTextList(SectionName, SettingName, values);

    POSITION pos1 = values.GetHeadPosition();
    while (pos1) {
        CString value = values.GetNext(pos1);
        if (value.CompareNoCase(TemplateName) == 0)
            return TRUE;
    }

    return FALSE;
}


//---------------------------------------------------------------------------
// RejectTemplate
//---------------------------------------------------------------------------


BOOL CBox::RejectTemplate(const WCHAR *TemplateName, BOOL reject)
{
    CSbieIni &ini = CSbieIni::GetInstance();

    CString SectionName = m_name.IsEmpty() ? _GlobalSettings : m_name;
    CString SettingName = ini.m_Template + L"Reject";

    BOOL ok;
    if (reject)
        ok = ini.InsertText(SectionName, SettingName, TemplateName);
    else
        ok = ini.DelText(SectionName, SettingName, TemplateName);

    return ok;
}
