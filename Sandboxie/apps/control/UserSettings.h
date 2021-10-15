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
// User Settings
//---------------------------------------------------------------------------


#ifndef _MY_USERSETTINGS_H
#define _MY_USERSETTINGS_H


class CUserSettings
{
    static CUserSettings *m_instance;

    CString m_SettingPrefix;
    CString m_Section;
    CString m_UserName;
    BOOL    m_IsAdmin;
    BOOL    m_CanEdit;
    BOOL    m_CanDisableForce;

    CUserSettings();

    CString WithPrefix(const CString &Setting) const;

    CString GetUserSectionName(const CString &Setting) const;

public:

    ~CUserSettings();

    static CUserSettings &GetInstance();

    CString GetUserName() const;

    BOOL IsAdmin() const;

    BOOL CanEdit() const;

    BOOL CanDisableForce() const;

    //CString Get(const CString &Setting);

    BOOL SetText(const CString &Setting, const CString &Value);
    BOOL SetNum(const CString &Setting, int Value);
    BOOL SetNum64(const CString &Setting, __int64 Value);
    BOOL SetBool(const CString &Setting, BOOL Value);

    void GetText(const CString &Setting, CString &Value,
                 const CString &Default = CString()) const;
    void GetNum(
        const CString &Setting, int &Value, int Default = 0) const;
    void GetNum64(
        const CString &Setting, __int64 &Value, __int64 Default = 0) const;
    void GetBool(
        const CString &Setting, BOOL &Value, BOOL Default = 0) const;

    void GetTextList(const CString &Setting, CStringList &Value) const;

    BOOL SetTextCsv(const CString &Setting, const CStringList &ValueList);
    void GetTextCsv(const CString &Setting, CStringList &ValueList) const;

    BOOL InsertText(const CString &Setting, const CString &Value);
    BOOL AppendText(const CString &Setting, const CString &Value);

    BOOL DelText(const CString &Setting, const CString &Value);

    void GetSettingsNames(CStringList &Names);
};


#endif // _MY_USERSETTINGS_H
