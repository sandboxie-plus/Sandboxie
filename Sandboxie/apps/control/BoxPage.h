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


#ifndef _MY_BOXPAGE_H
#define _MY_BOXPAGE_H


#include "Boxes.h"
#include "ToolTipButton.h"


class CBoxPage : public CPropertyPage
{
    DECLARE_MESSAGE_MAP()

    ULONG m_type;
    CString m_template;
    void *m_template_copy;
    const CString &m_BoxName;
    CString m_titleForTree, m_titleForPage;
    BOOL m_modified;

    CButton m_groupbox;

    CFont *m_font;

    void SetTitleForTree();

    void AdjustFont();

    void CenterControl(int idCtrl);

    int AddStringAndSetWidth(
            CListBox *pListBox, const CString &str, int index = -1);

    void MakeLTR(UINT id);

    bool SelectUsers(CStringList &users);

    virtual BOOL OnInitDialog();

    virtual BOOL OnKillActive();

    virtual void OnOK();

    afx_msg void OnModified();

    afx_msg LRESULT OnQuerySiblings(WPARAM wParam, LPARAM lParam);

    //
    // Utilities
    //

    void AddBeforeBrackets(CListBox *pListBox, const CString &str);

    BOOL CheckIfBrackets(CListBox *pListBox, int index,
                         const CString &SettingName,
                         const CString &Prefix = CString());

    //
    // Appearance
    //

    COLORREF Appearance_BorderColor;
    HBITMAP Appearance_Bitmap;

    CToolTipButton Appearance_ColorButton;

    void Appearance_SetBorderColor();

    void Appearance_OnInitDialog(CBox &box);
    void Appearance_OnOK(CBox &box);

    afx_msg void Appearance_OnCheckBox1();
    afx_msg void Appearance_OnCheckBox2();
    afx_msg void Appearance_OnShowBorder();
    afx_msg void Appearance_OnBorderColor();
    afx_msg void Appearance_OnBorderTitle();

    //
    // QuickRecover
    //

    void QuickRecover_OnInitDialog(CBox &box);
    void QuickRecover_OnOK(CBox &box);

    afx_msg void QuickRecover_OnCmdAdd();
    afx_msg void QuickRecover_OnCmdRemove();

    //
    // AutoRecover
    //

    void AutoRecover_OnInitDialog(CBox &box);
    void AutoRecover_OnOK(CBox &box);

    afx_msg void AutoRecover_OnCmdAddFolder();
    afx_msg void AutoRecover_OnCmdAddType();

    //
    // AutoDelete
    //

    void AutoDelete_OnInitDialog(CBox &box);
    void AutoDelete_OnOK(CBox &box);

    afx_msg void AutoDelete_OnAuto();
    afx_msg void AutoDelete_OnNever();

    //
    // DeleteCommand
    //

    void DeleteCommand_OnInitDialog(CBox &box);
    BOOL DeleteCommand_OnKillActive(CBox &box);
    void DeleteCommand_OnOK(CBox &box);

    afx_msg void DeleteCommand_RmDir();
    afx_msg void DeleteCommand_SDelete();
    afx_msg void DeleteCommand_EraserL();
    afx_msg void DeleteCommand_Eraser6();
    void DeleteCommand_SetDeleteProgram(
        const WCHAR *PgmName, const WCHAR *PgmArgs);

    //
    // Program Groups
    //

    int ProgramGroups_index;

    void ProgramGroups_OnInitDialog(CBox &box);
    void ProgramGroups_OnOK(CBox &box);

    afx_msg void ProgramGroups_OnSelectGrp();
    afx_msg void ProgramGroups_OnCmdAddGrp();
    afx_msg void ProgramGroups_OnCmdAddPgm();
    afx_msg void ProgramGroups_OnCmdRemove();

    void ProgramGroups_SwitchGrp(const CString &grp);

    //
    // Force / Linger / Leader
    //

    CString ForceLinger_editbox;

    void ForceLinger_OnInitDialog(CBox &box);
    void ForceLinger_OnOK(CBox &box);

    afx_msg void ForceLinger_OnCmdAddPgm();
    afx_msg void ForceLinger_OnCmdAddFolder();
    afx_msg void ForceLinger_OnCmdRemove();

    void ForceLinger_OnCmdAddPgm2(const CString &pgm);

    static int CALLBACK ForceLinger_OnCmdAddFolder_Callback(
        HWND hwnd, UINT uMsg,LPARAM lParam, LPARAM lpData);

    bool ForceLinger_MatchPattern(const CString &pat, const CString &test);

    //
    // File Migrate
    //

    void FileMigrate_OnInitDialog(CBox &box);
    void FileMigrate_OnOK(CBox &box);

    afx_msg void FileMigrate_OnChangeKb();

    //
    // Internet/Start Restrictions
    //

    CString m_accAll, m_accNone;

    void RestrictPgm_OnInitDialog(CBox &box);
    void RestrictPgm_OnOK(CBox &box);

    afx_msg void RestrictPgm_AddStar();

    //
    // Not Implemented
    //

    void NotImplemented_OnInitDialog(CBox &box);

    afx_msg void OnLinkLabel(NMHDR *pNMHDR, LRESULT *pResult);

    //
    // File Access
    //

    int FileAccess_index;

    CToolTipButton FileAccess_NegateButton;

    void FileAccess_OnInitDialog(CBox &box);
    void FileAccess_OnOK(CBox &box);

    afx_msg void FileAccess_OnSelectPgm();
    afx_msg void FileAccess_OnAddPgm();
    afx_msg void FileAccess_OnNegate();

    afx_msg void FileAccess_OnAddEntry();
    afx_msg void FileAccess_OnEditEntry();
    afx_msg void FileAccess_OnRemoveEntry();

    afx_msg void FileAccess_OnSelectRes();

    void FileAccess_AddAndSwitchPgm(const CString &pgm);
    void FileAccess_SwitchPgm(const CString &pgm);
    void FileAccess_GetPathList(CStringList &list, const CString *pgm);

    BOOL FileAccess_CheckIfBrackets(CListBox *pListBox, int index);
    //
    // Drop Rights Restrictions
    //

    void DropRights_OnInitDialog(CBox &box);
    void DropRights_OnOK(CBox &box);

    //
    // Network Files Restrictions
    //
    void NetworkFiles_OnInitDialog(CBox &box);
    void NetworkFiles_OnOK(CBox &box);

    //
    // Print Spooler Restrictions
    //
    void PrintSpooler_OnInitDialog(CBox &box);
    void PrintSpooler_OnOK(CBox &box);

    //
    // Low-Level Restrictions
    //

    void LowLevel_OnInitDialog(CBox &box);
    void LowLevel_OnOK(CBox &box);

    void Hardware_OnInitDialog(CBox &box);
    void Hardware_OnOK(CBox &box);

    //
    // User Accounts
    //

    bool UserAccounts_restricted;

    void UserAccounts_OnInitDialog(CBox &box);
    void UserAccounts_OnOK(CBox &box);

    void UserAccounts_OnCmdAdd();
    void UserAccounts_OnCmdRemove();

    //
    // Pluggable derived class
    //

    virtual void Plugin_OnInitDialog(CBox &box);
    virtual void Plugin_OnOK(CBox &box);


protected:

    void SetPageTemplate(const WCHAR *TemplateName);


public:

    CBoxPage(ULONG type, const CString &BoxName,
             ULONG GroupMsgId, ULONG TitleMsgId);

    ~CBoxPage();

    static void DoPropertySheet(
        CWnd *pParentWnd, const CString &BoxName,
        bool *ReloadBoxes, const CString &ProgramName);
};


#endif // _MY_BOXPAGE_H
