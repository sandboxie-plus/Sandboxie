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


#ifndef _MY_BOX_PROC_H
#define _MY_BOX_PROC_H


class CBoxProc
{
    static const CString _unknown;

    const CString &m_name;

    ULONG m_pids[512];
    CString *m_images;
    CString *m_titles;
    HICON   *m_icons;
    int m_num, m_max;
    int m_old_num;

    static CStringList m_RecentExes;

    //
    //
    //

public:

    CBoxProc(const CString &name);
    ~CBoxProc();

    void RefreshProcesses();

    int GetOldProcessCount() const;
    int GetProcessCount() const;

    int GetIndexForProcessId(ULONG pid) const;

    int GetProcessId(int index) const;
    const CString &GetProcessImageName(int index) const;
    const CString &GetProcessWindowTitle(int index) const;
    HICON GetProcessWindowIcon(int index) const;

    static void AddToRecentExes(const CString &exe1);
    static void AddRecentExesToList(CStringList &list1);
};


#endif // _MY_BOX_PROC_H
