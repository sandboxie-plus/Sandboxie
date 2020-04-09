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
// Boxes
//---------------------------------------------------------------------------


#include "stdafx.h"
#include "Boxes.h"

#include "SbieIni.h"
#include "UserSettings.h"
#include "WindowTitleMap.h"
#include "common/win32_ntddk.h"


//---------------------------------------------------------------------------
// Variables
//---------------------------------------------------------------------------


CBoxes *CBoxes::m_instance = NULL;

CString CBoxes::m_DefaultBox(L"DefaultBox");

static const CString _BoxExpandedView(L"BoxExpandedView");


//---------------------------------------------------------------------------
// Constructor
//---------------------------------------------------------------------------


CBoxes::CBoxes()
{
    // _nullBox = new CBox(L"");
    ReloadBoxes();
}


//---------------------------------------------------------------------------
// Destructor
//---------------------------------------------------------------------------


CBoxes::~CBoxes()
{
    Clear();
    // delete _nullBox;
}


//---------------------------------------------------------------------------
// GetInstance
//---------------------------------------------------------------------------


CBoxes &CBoxes::GetInstance()
{
    if (! m_instance)
        m_instance = new CBoxes();
    return *m_instance;
}


//---------------------------------------------------------------------------
// ReloadBoxes
//---------------------------------------------------------------------------


void CBoxes::ReloadBoxes()
{
    Clear();

    WCHAR name[64];
    int index = -1;
    while (1) {

        if (GetSize() == 0)
            name[0] = L'\0';
        else {
            index = SbieApi_EnumBoxesEx(
                            index | CONF_GET_NO_TEMPLS, name, TRUE);
            if (index == -1)
                break;
            LONG rc = SbieApi_IsBoxEnabled(name);
            if (rc == STATUS_ACCOUNT_RESTRICTION)
                m_AnyHiddenBoxes = true;
            if (rc != STATUS_SUCCESS)
                continue;
        }

        CBox *box = new CBox(name);

        int index2;
        for (index2 = 1; index2 < GetSize(); ++index2) {
            const CBox &oldBox = GetBox(index2);
            if (oldBox.GetName().CompareNoCase(name) > 0)
                break;
        }

        if (index2 > GetSize())
            Add((CObject *)box);
        else
            InsertAt(index2, (CObject *)box);
    }

    //
    // if DefaultBox is missing, add it
    //

    bool doWriteExpandedView = false;

    if (GetSize() == 1 &&
            SbieApi_IsBoxEnabled(m_DefaultBox) ==
                STATUS_OBJECT_NAME_NOT_FOUND) {

        CBox *box = new CBox(m_DefaultBox);
        Add((CObject *)box);
        CBox::SetEnabled(box->GetName());
        box->SetExpandedView(TRUE);
        doWriteExpandedView = true;
    }

    ReadExpandedView();
    if (doWriteExpandedView)
        WriteExpandedView();
}


//---------------------------------------------------------------------------
// RefreshProcesses
//---------------------------------------------------------------------------


void CBoxes::RefreshProcesses()
{
    CWindowTitleMap::GetInstance().Refresh();
    for (int i = 0; i < GetSize(); ++i)
        GetBox(i).GetBoxProc().RefreshProcesses();
}


//---------------------------------------------------------------------------
// Clear
//---------------------------------------------------------------------------


void CBoxes::Clear()
{
    while (GetSize()) {
         CBox *box = (CBox *)GetAt(0);
         RemoveAt(0);
         delete box;
    }
    m_AnyHiddenBoxes = false;
}


//---------------------------------------------------------------------------
// GetBox
//---------------------------------------------------------------------------


CBox &CBoxes::GetBox(int index) const
{
    if (index < 0 || index > GetSize())
        index = 0;
    return *(CBox *)GetAt(index);
}


//---------------------------------------------------------------------------
// GetBox
//---------------------------------------------------------------------------


CBox &CBoxes::GetBox(const CString &name) const
{
    for (int i = 0; i < GetSize(); ++i) {
        CBox &box = GetBox(i);
        if (box.GetName().CompareNoCase(name) == 0)
            return box;
    }
    return GetBox(0);
}


//---------------------------------------------------------------------------
// GetBoxByProcessId
//---------------------------------------------------------------------------


CBox &CBoxes::GetBoxByProcessId(ULONG pid) const
{
    for (int i = 0; i < GetSize(); ++i) {
        CBox &box = GetBox(i);
        if (box.GetBoxProc().GetIndexForProcessId(pid) != -1)
            return box;
    }
    return GetBox(0);
}


//---------------------------------------------------------------------------
// GetBoxIndex
//---------------------------------------------------------------------------


int CBoxes::GetBoxIndex(const CString &name) const
{
    for (int i = 0; i < GetSize(); ++i) {
        CBox &box = GetBox(i);
        if (box.GetName().CompareNoCase(name) == 0)
            return i;
    }
    return 0;
}


//---------------------------------------------------------------------------
// AnyActivity
//---------------------------------------------------------------------------


BOOL CBoxes::AnyActivity() const
{
    for (int i = 1; i < GetSize(); ++i) {
        if (GetBox(i).GetBoxProc().GetProcessCount())
            return TRUE;
    }
    return FALSE;
}


//---------------------------------------------------------------------------
// KillAll
//---------------------------------------------------------------------------


void CBoxes::KillAll() const
{
    for (int i = 1; i < GetSize(); ++i)
        SbieDll_KillAll(-1, GetBox(i).GetName());
}


//---------------------------------------------------------------------------
// AnyHiddenBoxes
//---------------------------------------------------------------------------


bool CBoxes::AnyHiddenBoxes() const
{
    return m_AnyHiddenBoxes;
}


//---------------------------------------------------------------------------
// GetHiddenBoxes
//---------------------------------------------------------------------------


void CBoxes::GetHiddenBoxes(CStringList &list) const
{
    while (! list.IsEmpty())
        list.RemoveHead();

    WCHAR name[64];
    int index = -1;
    while (1) {
        index = SbieApi_EnumBoxesEx(index, name, TRUE);
        if (index == -1)
            break;
        LONG rc = SbieApi_IsBoxEnabled(name);
        if (rc == STATUS_ACCOUNT_RESTRICTION)
            list.AddTail(name);
    }
}


//---------------------------------------------------------------------------
// ReadExpandedView
//---------------------------------------------------------------------------


void CBoxes::ReadExpandedView()
{
    CUserSettings &ini = CUserSettings::GetInstance();

    //
    // read old BoxExpandedView_boxname settings
    //

    for (int i = 0; i < GetSize(); ++i) {
        CBox &box = GetBox(i);
        CString boxname = box.GetName();
        if (! boxname.IsEmpty()) {
            CString setting(_BoxExpandedView);
            setting += L"_";
            setting += boxname;
            BOOL value;
            ini.GetBool(setting, value, FALSE);
            if (value)
                box.SetExpandedView(TRUE);
        }
    }

    //
    // read new setting BoxExpandedView=boxname1,boxname2
    //

    CStringList list;
    ini.GetTextCsv(_BoxExpandedView, list);
    while (! list.IsEmpty()) {
        CString boxname = list.RemoveHead();
        if (! boxname.IsEmpty()) {
            CBox &box = GetBox(boxname);
            if (! box.GetName().IsEmpty())
                box.SetExpandedView(TRUE);
        }
    }
}


//---------------------------------------------------------------------------
// WriteExpandedView
//---------------------------------------------------------------------------


BOOL CBoxes::WriteExpandedView()
{
    CUserSettings &ini = CUserSettings::GetInstance();

    //
    // write new setting BoxExpandedView=boxname1,boxname2
    //

    CStringList list;
    for (int i = 0; i < GetSize(); ++i) {
        CBox &box = GetBox(i);
        CString boxname = box.GetName();
        if (! boxname.IsEmpty()) {
            if (box.IsExpandedView())
                list.AddTail(boxname);
        }
    }

    BOOL ok = ini.SetTextCsv(_BoxExpandedView, list);

    //
    // delete old BoxExpandedView_boxname settings
    //

    if (ok) {

        CString setting(_BoxExpandedView);
        setting += L"_";
        int len = setting.GetLength();

        CStringList names;
        ini.GetSettingsNames(names);
        while (! names.IsEmpty()) {
            CString name = names.RemoveHead();
            if (name.GetLength() >= len &&
                        name.Left(len).CompareNoCase(setting) == 0) {
                ini.SetText(name, CString());
            }
        }
    }

    return ok;
}
