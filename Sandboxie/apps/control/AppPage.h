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
// Application Settings Property Page
//---------------------------------------------------------------------------


#ifndef _MY_APPPAGE_H
#define _MY_APPPAGE_H


#include "BoxPage.h"


class CAppPage : public CBoxPage
{
    DECLARE_MESSAGE_MAP()

    ULONG m_app;
    static CObList m_app_pages;

    virtual void Plugin_OnInitDialog(CBox &box);

    virtual void Plugin_OnOK(CBox &box);

    //
    // Access
    //

    void Access_OnInitDialog(CBox &box);
    void Access_OnOK(CBox &box);

    //
    // Template
    //

    typedef struct _TMPL_INFO {
        CString ClassName;
        int ClassModifier;
        ULONG TitleId;
        ULONG LabelId;
        BOOL WithLink;
        BOOL WithCreate;
    } TMPL_INFO;

    TMPL_INFO m_tmpl_info;

    CStringList m_tmpl_sections;

    BOOL m_tmpl_init_list_box;

    void Template_OnInitDialog(CBox &box);
    void Template_OnOK(CBox &box);

    afx_msg void Template_OnAdd();
    afx_msg void Template_OnRemove();
    afx_msg void Template_OnToggle();

    void Template_OnView();
    void Template_OnLink();
    void Template_OnCreate();

    void Template_Filter();
    void Template_Sort();

    afx_msg void OnMeasureItem(int idCtrl, LPMEASUREITEMSTRUCT lpmis);
    afx_msg void OnDrawItem(int idCtrl, LPDRAWITEMSTRUCT lpdis);

    //
    // Folders
    //

    CStringList m_folders_vars;

    void Folders_OnInitDialog(CBox &box);
    void Folders_OnOK(CBox &box);

    afx_msg void Folders_OnSelect();
    afx_msg void Folders_OnAdd();
    afx_msg void Folders_OnRemove();

    BOOL Folders_CheckExpectFile(const WCHAR *var, const WCHAR *path);

    //
    // More templates
    //

    static void UpdateWebTemplates(CBox &box);
    static void UpdateEmailTemplates(CBox &box);
    static void UpdateTemplates2(CBox &box,
                                const CString &image_name,
                                const CString &tmpl_name,
                                const CString &path1,
                                const CString &path2);
    static void UpdateTemplates3(CBox &box,
                                const CString &image_name,
                                const CString &tmpl_name,
                                CStringList &files,
                                CStringList &keys);

public:

    CAppPage(ULONG type, const CString &BoxName, ULONG TitleMsgId);
    CAppPage(TMPL_INFO *info, const CString &BoxName);
    ~CAppPage();

    static void AddPages(CPropertySheet &sheet, const CString &BoxName);
    static void DeletePages();

    static void UpdateTemplates(CBox &box);

    static void SetDefaultTemplates6(CBox &box);
    static void SetDefaultTemplates7(CBox &box);
    static void SetDefaultTemplates8(CBox& box);
    static void SetDefaultTemplates9(CBox& box);
};


#endif // _MY_APPPAGE_H
