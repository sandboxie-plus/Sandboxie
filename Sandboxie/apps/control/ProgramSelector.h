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
// Program Selector Dialog Box
//---------------------------------------------------------------------------


#ifndef _MY_PROGRAMSELECTOR_H
#define _MY_PROGRAMSELECTOR_H


#include "Box.h"


class CProgramSelector : public CBaseDialog
{
    DECLARE_MESSAGE_MAP()

    CStringList m_list1;
    CStringList m_list2;
    CStringList m_list3;

    void GetProgramNamesFromSettings(CBox &box);
    void GetProgramNamesFromPathLists(CBox &box, CStringList &pgms);
    void GetProgramNamesFromGroups(
            CBox &box, const CStringList &groups, CStringList &out_pgms);

    void PrepareListBox(int idCtrl, CStringList &list);

    virtual BOOL OnInitDialog();

    afx_msg void OnListSelected();

    afx_msg void OnEditChanged();

    afx_msg void OnOpenFile();

    virtual void OnOK();

    virtual void OnCancel();

public:

    CProgramSelector(CWnd *pParentWnd, CBox *box,
                     bool withProgramGroups,
                     const CString &groupName = CString());
    ~CProgramSelector();

    CString m_pgm1;
    CString m_pgm2;
};


#endif // _MY_PROGRAMSELECTOR_H
