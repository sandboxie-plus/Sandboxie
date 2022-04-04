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
// Processes in Box
//---------------------------------------------------------------------------


#include "MyApp.h"
#include "BoxProc.h"
#include "WindowTitleMap.h"


//---------------------------------------------------------------------------
// Variables
//---------------------------------------------------------------------------


const CString CBoxProc::_unknown(L"???");

CStringList CBoxProc::m_RecentExes;


//---------------------------------------------------------------------------
// Constructor
//---------------------------------------------------------------------------


CBoxProc::CBoxProc(const CString &name)
    : m_name(name)
{
    memzero(m_pids, sizeof(m_pids));
    m_images = NULL;
    m_titles = NULL;
    m_icons  = NULL;
    m_num = 0;
    m_max = 0;
    m_old_num = 0;
}


//---------------------------------------------------------------------------
// Destructor
//---------------------------------------------------------------------------


CBoxProc::~CBoxProc()
{
    if (m_images)
        delete[] m_images;
    if (m_titles)
        delete[] m_titles;
    if (m_icons)
        delete[] m_icons;
}


//---------------------------------------------------------------------------
// RefreshProcesses
//---------------------------------------------------------------------------


void CBoxProc::RefreshProcesses(void)
{
    if (m_name.IsEmpty())
        return;

    m_old_num = m_num;

    WCHAR name[256];
    wcscpy(name, m_name);
    SbieApi_EnumProcess(name, m_pids);

    m_num = m_pids[0];

    //
    // shortcut case:  no processes found
    //

    if (m_num == 0)
        return;

    //
    // allocate buffers for process names, if needed
    //

    if (m_num >= m_max) {

        if (m_images)
            delete[] m_images;
        if (m_titles)
            delete[] m_titles;
        if (m_icons)
            delete[] m_icons;

        m_max    = m_num + 8;
        m_images = new CString[m_max];
        m_titles = new CString[m_max];
        m_icons  = new HICON[m_max];
    }

    //
    // get the image name and window name for each process
    //

    CWindowTitleMap &theWindowTitleMap = CWindowTitleMap::GetInstance();

    int i = 1;
    for (i = 1; i <= m_num; ++i) {

        SbieApi_QueryProcessEx(
                (HANDLE)(ULONG_PTR)m_pids[i], 255, NULL, name, NULL, NULL);

        if (name[0])
            m_images[i] = name;
        else
            m_images[i] = _unknown;

        HICON icon;
        const WCHAR *title = theWindowTitleMap.Get(m_pids[i], icon);
        if (title)
            m_titles[i] = title;
        else
            m_titles[i] = CString();

        m_icons[i] = icon;
    }

    //
    // collect images into the list of recent EXEs
    //

    for (i = 1; i <= m_num; ++i) {

        ULONG64 ProcessFlags =
            SbieApi_QueryProcessInfo((HANDLE)(ULONG_PTR)m_pids[i], 0);
        if (! (ProcessFlags & SBIE_FLAG_IMAGE_FROM_SBIE_DIR)) {

            CString &exe1 = m_images[i];
            AddToRecentExes(exe1);
        }
    }
}


//---------------------------------------------------------------------------
// GetProcessCount
//---------------------------------------------------------------------------


int CBoxProc::GetOldProcessCount() const
{
    return m_old_num;
}


//---------------------------------------------------------------------------
// GetProcessCount
//---------------------------------------------------------------------------


int CBoxProc::GetProcessCount() const
{
    return m_num;
}


//---------------------------------------------------------------------------
// GetProcessId
//---------------------------------------------------------------------------


int CBoxProc::GetProcessId(int index) const
{
    if (index < 0 || index >= m_num)
        return 0;
    return m_pids[index + 1];
}


//---------------------------------------------------------------------------
// GetProcessImageName
//---------------------------------------------------------------------------


const CString &CBoxProc::GetProcessImageName(int index) const
{
    if (index < 0 || index >= m_num)
        return _unknown;
    return m_images[index + 1];
}


//---------------------------------------------------------------------------
// GetProcessWindowTitle
//---------------------------------------------------------------------------


const CString &CBoxProc::GetProcessWindowTitle(int index) const
{
    if (index < 0 || index >= m_num)
        return _unknown;
    return m_titles[index + 1];
}


//---------------------------------------------------------------------------
// GetProcessWindowIcon
//---------------------------------------------------------------------------


HICON CBoxProc::GetProcessWindowIcon(int index) const
{
    if (index < 0 || index >= m_num)
        return NULL;
    return m_icons[index + 1];
}


//---------------------------------------------------------------------------
// GetIndexForProcessId
//---------------------------------------------------------------------------


int CBoxProc::GetIndexForProcessId(ULONG pid) const
{
    for (int index = 0; index < m_num; ++index)
        if (m_pids[index + 1] == pid)
            return index;
    return -1;
}


//---------------------------------------------------------------------------
// AddToRecentExes
//---------------------------------------------------------------------------


void CBoxProc::AddToRecentExes(const CString &exe1)
{
    if (exe1.CompareNoCase(L"unknown executable image") == 0)
        return;
    if (exe1.CompareNoCase(_unknown) == 0)
        return;

    bool found = false;
    POSITION pos = m_RecentExes.GetHeadPosition();
    while (pos) {
        CString &exe2 = m_RecentExes.GetNext(pos);
        if (exe2.CompareNoCase(exe1) == 0) {
            found = true;
            break;
        }
    }

    if (! found) {
        CString exe2(exe1);
        exe2.MakeLower();
        m_RecentExes.AddTail(exe2);
    }
}


//---------------------------------------------------------------------------
// AddRecentExesToList
//---------------------------------------------------------------------------


void CBoxProc::AddRecentExesToList(CStringList &list1)
{
    list1.AddTail(&m_RecentExes);
}
