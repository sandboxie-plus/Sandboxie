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


#ifndef _MY_THIRDPARTYDIALOG_H
#define _MY_THIRDPARTYDIALOG_H


#include "FlashingButton.h"


class CThirdPartyDialog : public CBaseDialog
{
    DECLARE_MESSAGE_MAP()

    virtual BOOL OnInitDialog();

    void ApplyTemplates();

    afx_msg void OnAdd();
    afx_msg void OnRemove();
    afx_msg void OnToggle();

    afx_msg void OnCheckBox();

    virtual void OnOK();
    virtual void OnCancel();

    afx_msg void OnRemoveOld();
    afx_msg void OnKnownConflicts();
    afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);

    void CollectObjects();
    void CollectObjects2(CStringList &objdirs);

    void CollectClasses();
    static BOOL CollectClasses2(HWND hwnd, LPARAM lparam);

    void CollectServices();

    void CollectProducts();

    void CollectTemplates();
    BOOL CheckTemplate(const CString &tmpl_name);
    BOOL CheckRegistryKey(const WCHAR *keypath);
    BOOL CheckFile(const WCHAR *filepath);

    void FindConflicts(CStringList &list, int what);

    static ULONG WatchThread(void *lpParameter);

private:

    CFlashingButton m_ConflictsButton;

    int m_doKnownConflicts;

    CStringList m_objects;
    CStringList m_classes;
    CStringList m_services;
    CStringList m_products;

    CMapStringToPtr m_templates;

    CStringList m_stale_templates;

    BOOL m_checkbox;
    BOOL m_autorun;

    static HANDLE hWatchThread;

public:

    CThirdPartyDialog(CWnd *pParentWnd, BOOL AutoRun, WPARAM wParamAlert);
    ~CThirdPartyDialog();

    static BOOL AutoRunSoftCompat(BOOL *NewValue = NULL);
};


#endif // _MY_THIRDPARTYDIALOG_H
