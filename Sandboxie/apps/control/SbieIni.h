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
// Sandboxie Ini Interface
//---------------------------------------------------------------------------


#ifndef _MY_SBIEINI_H
#define _MY_SBIEINI_H


class CSbieIni
{
    static CSbieIni *m_instance;

    CMapStringToString m_Expands;
    BOOL m_ExpandsInit;

    WCHAR m_Password[66];

    CSbieIni();

    BOOL CallSbieSvc(
        ULONG msgid,
        const CString &Section, const CString &Setting,
        const CString &Value);

    void InitGenericPaths(BOOL WithUser);

public:

    static CString m_Tmpl_;
    static CString m_Template;
    static CString m_Template_;
    static CString m_TemplateSettings;
    static CString m_UserSettings_;
    static volatile bool m_ServiceStopped;

    ~CSbieIni();

    static CSbieIni &GetInstance();

    CString GetPath(BOOL *IsHomePath = NULL);

    BOOL GetUser(CString &Section, CString &Name, BOOL &IsAdmin);

    void GetSettingsNames(const CString &Section, CStringList &Names);

    //
    // Basic editing
    //

    BOOL SetText(
        const CString &Section, const CString &Setting,
        const CString &Value);
    BOOL SetNum(
        const CString &Section, const CString &Setting, int Value);
    BOOL SetNum64(
        const CString &Section, const CString &Setting, __int64 Value);
    BOOL SetBool(
        const CString &Section, const CString &Setting, BOOL Value);

    void GetText(
        const CString &Section, const CString &Setting,
        CString &Value, const CString &Default = CString()) const;
    void GetNum(
        const CString &Section, const CString &Setting,
        int &Value, int Default = 0) const;
    void GetNum64(
        const CString &Section, const CString &Setting,
        __int64 &Value, __int64 Default = 0) const;
    void GetBool(
        const CString &Section, const CString &Setting,
        BOOL &Value, BOOL Default = 0) const;

    void GetTextList(
        const CString &Section, const CString &Setting,
        CStringList &Value, BOOL withBrackets = FALSE) const;

    BOOL InsertText(
        const CString &Section, const CString &Setting,
        const CString &Value);

    BOOL AppendText(
        const CString &Section, const CString &Setting,
        const CString &Value);

    BOOL DelText(
        const CString &Section, const CString &Setting,
        const CString &Value);

    BOOL DelText2(
        const CString &Section, const CString &Setting,
        const CString &Value, const CString &ProcessGroup);

    CString MakeGenericPath(const CString &path);

    CString MakeSpecificPath(const CString &path);

    BOOL RenameOrCopySection(const CString &OldName, const CString &NewName,
                             bool deleteOldName);

    BOOL RemoveSection(const CString &Section);

    //
    // Template-related
    //

    BOOL SetTemplateVariable(const CString &Name, const CString &Value,
                             BOOL UserSpecific);

    CString GetTemplateVariable(const CString &Template);

    CString GetTemplateMetaData(
        const CString &Template, const CString &Setting);

    void GetTemplateClasses(CStringList &list);
    void GetTemplateNames(const CString &forClass, CStringList &list);

    void GetTextListWithBrackets(
        const CString &Section, const CString &Setting,
        CStringList &Value, WCHAR *buf, ULONG buf_len) const;

    static CString AddBrackets(const CString &value);

    static BOOL StripBrackets(CString &value);

    //
    // Password-related
    //

    BOOL CallServerWithPassword(
        void *RequestBuf, WCHAR *pPasswordWithinRequestBuf,
        const CString &SectionName, const CString &SettingName);

    BOOL SetOrTestPassword(WCHAR *NewPassword66);

    void GetRestrictions(
        BOOL &EditAdminOnly, BOOL &DisableForceAdminOnly,
        BOOL &NeedPassword, BOOL &HavePassword, BOOL &ForgetPassword);

    void SetRestrictions(
        BOOL EditAdminOnly, BOOL DisableForceAdminOnly,
        BOOL ForgetPassword);

    void ForgetPasswordNow();

    static BOOL InputPassword(ULONG msgid, WCHAR *Password66);

    static ULONG MyPromptForCredentials(
        void *pUiInfo,
        const WCHAR *pszTargetName,
        void *pContext,
        ULONG dwAuthError,
        WCHAR *pszUserName,
        ULONG ulUserNameBufferSize,
        WCHAR *pszPassword,
        ULONG ulPasswordBufferSize,
        BOOL *save,
        ULONG dwFlags);

    static void MonitorService(CWnd *pWnd, ULONG idExitCmd);
    static ULONG MonitorServiceThread(void *lpParameter);

};


#endif // _MY_SBIEINI_H
