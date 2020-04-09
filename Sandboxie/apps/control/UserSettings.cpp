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


#include "stdafx.h"
#include "UserSettings.h"

#include "SbieIni.h"
#include "common/my_version.h"


//---------------------------------------------------------------------------
// Variables
//---------------------------------------------------------------------------


CUserSettings *CUserSettings::m_instance = NULL;


//---------------------------------------------------------------------------
// Constructor
//---------------------------------------------------------------------------


CUserSettings::CUserSettings()
{
    m_SettingPrefix = CString(SBIECTRL_);

    CSbieIni::GetInstance().GetUser(m_Section, m_UserName, m_IsAdmin);

    m_CanEdit = TRUE;
    m_CanDisableForce = TRUE;

    if (! m_IsAdmin) {

        BOOL isEditAdminOnly, isForceDisableAdminOnly;
        BOOL isEditPassword, isHavePassword, isForgetPassword;
        CSbieIni::GetInstance().GetRestrictions(
            isEditAdminOnly, isForceDisableAdminOnly,
            isEditPassword, isHavePassword, isForgetPassword);

        if (isEditAdminOnly)
            m_CanEdit = FALSE;

        if (isForceDisableAdminOnly)
            m_CanDisableForce = FALSE;
    }

    //
    // make sure the UserName setting is there
    //

    static const WCHAR *_UserName     = L"UserName";
    static const WCHAR *_QuestionMark = L"?";
    CString TempUserName;
    GetText(_UserName, TempUserName, _QuestionMark);
    if (TempUserName == _QuestionMark)
        SetText(_UserName, m_UserName);
}


//---------------------------------------------------------------------------
// Destructor
//---------------------------------------------------------------------------


CUserSettings::~CUserSettings()
{
}


//---------------------------------------------------------------------------
// GetInstance
//---------------------------------------------------------------------------


CUserSettings &CUserSettings::GetInstance()
{
    if (! m_instance)
        m_instance = new CUserSettings();
    return *m_instance;
}


//---------------------------------------------------------------------------
// IsAdmin
//---------------------------------------------------------------------------


BOOL CUserSettings::IsAdmin() const
{
    return m_IsAdmin;
}


//---------------------------------------------------------------------------
// CanEdit
//---------------------------------------------------------------------------


BOOL CUserSettings::CanEdit() const
{
    return m_CanEdit;
}


//---------------------------------------------------------------------------
// CanDisableForce
//---------------------------------------------------------------------------


BOOL CUserSettings::CanDisableForce() const
{
    return m_CanDisableForce;
}


//---------------------------------------------------------------------------
// GetUserName
//---------------------------------------------------------------------------


CString CUserSettings::GetUserName() const
{
    return m_UserName;
}


//---------------------------------------------------------------------------
// WithPrefix
//---------------------------------------------------------------------------


CString CUserSettings::WithPrefix(const CString &Setting) const
{
    return m_SettingPrefix + Setting;
}


//---------------------------------------------------------------------------
// GetUserSectionName
//---------------------------------------------------------------------------


CString CUserSettings::GetUserSectionName(const CString &Setting) const
{
    static const CString _invalid(L"|!@#$%^&*(),.?|");
    static const CString _default(L"UserSettings_Default");

    CString section = m_Section;
    CString value;
    CSbieIni::GetInstance().
        GetText(section, WithPrefix(Setting), value, _invalid);
    if (value == _invalid)
        section = _default;
    return section;
}


//---------------------------------------------------------------------------
// SetText
//---------------------------------------------------------------------------


BOOL CUserSettings::SetText(const CString &Setting, const CString &Value)
{
    return CSbieIni::GetInstance().
        SetText(m_Section, WithPrefix(Setting), Value);
}


//---------------------------------------------------------------------------
// SetNum
//---------------------------------------------------------------------------


BOOL CUserSettings::SetNum(const CString &Setting, int Value)
{
    return CSbieIni::GetInstance().
        SetNum(m_Section, WithPrefix(Setting), Value);
}


//---------------------------------------------------------------------------
// SetNum64
//---------------------------------------------------------------------------


BOOL CUserSettings::SetNum64(const CString &Setting, __int64 Value)
{
    return CSbieIni::GetInstance().
        SetNum64(m_Section, WithPrefix(Setting), Value);
}


//---------------------------------------------------------------------------
// SetBool
//---------------------------------------------------------------------------


BOOL CUserSettings::SetBool(const CString &Setting, BOOL Value)
{
    return CSbieIni::GetInstance().
        SetBool(m_Section, WithPrefix(Setting), Value);
}


//---------------------------------------------------------------------------
// GetText
//---------------------------------------------------------------------------


void CUserSettings::GetText(
    const CString &Setting, CString &Value, const CString &Default) const
{
    CString section = GetUserSectionName(Setting);
    CSbieIni::GetInstance().GetText(
        section, WithPrefix(Setting), Value, Default);
}


//---------------------------------------------------------------------------
// GetNum
//---------------------------------------------------------------------------


void CUserSettings::GetNum(
    const CString &Setting, int &Value, int Default) const
{
    CString section = GetUserSectionName(Setting);
    CSbieIni::GetInstance().GetNum(
        section, WithPrefix(Setting), Value, Default);
}


//---------------------------------------------------------------------------
// GetNum64
//---------------------------------------------------------------------------


void CUserSettings::GetNum64(
    const CString &Setting, __int64 &Value, __int64 Default) const
{
    CString section = GetUserSectionName(Setting);
    CSbieIni::GetInstance().GetNum64(
        section, WithPrefix(Setting), Value, Default);
}


//---------------------------------------------------------------------------
// GetBool
//---------------------------------------------------------------------------


void CUserSettings::GetBool(
    const CString &Setting, BOOL &Value, BOOL Default) const
{
    CString section = GetUserSectionName(Setting);
    CSbieIni::GetInstance().GetBool(
        section, WithPrefix(Setting), Value, Default);
}


//---------------------------------------------------------------------------
// GetTextList
//---------------------------------------------------------------------------


void CUserSettings::GetTextList(
    const CString &Setting, CStringList &Value) const
{
    CString section = GetUserSectionName(Setting);
    CSbieIni::GetInstance().GetTextList(
        section, WithPrefix(Setting), Value);
}


//---------------------------------------------------------------------------
// InsertText
//---------------------------------------------------------------------------


BOOL CUserSettings::InsertText(const CString &Setting, const CString &Value)
{
    return CSbieIni::GetInstance().
        InsertText(m_Section, WithPrefix(Setting), Value);
}


//---------------------------------------------------------------------------
// AppendText
//---------------------------------------------------------------------------


BOOL CUserSettings::AppendText(const CString &Setting, const CString &Value)
{
    return CSbieIni::GetInstance().
        AppendText(m_Section, WithPrefix(Setting), Value);
}


//---------------------------------------------------------------------------
// DelText
//---------------------------------------------------------------------------


BOOL CUserSettings::DelText(const CString &Setting, const CString &Value)
{
    return CSbieIni::GetInstance().
        DelText(m_Section, WithPrefix(Setting), Value);
}


//---------------------------------------------------------------------------
// SetTextCsv
//---------------------------------------------------------------------------


BOOL CUserSettings::SetTextCsv(
    const CString &Setting, const CStringList &ValueList)
{
    CString NextLine;
    if (! SetText(Setting, NextLine))
        return FALSE;
    bool AnythingWritten = false;

    POSITION pos = ValueList.GetHeadPosition();
    while (pos) {

        CString NextValue = ValueList.GetNext(pos);
        NextValue.Remove(L',');
        NextValue.TrimLeft();
        NextValue.TrimRight();
        if (! NextValue.IsEmpty()) {

            if (! NextLine.IsEmpty())
                NextLine += L",";
            NextLine += NextValue;
            if (NextLine.GetLength() > 1000) {

                if (! AppendText(Setting, NextLine))
                    return FALSE;
                NextLine = CString();
                AnythingWritten = true;
            }
        }
    }

    if (NextLine.IsEmpty() && (! AnythingWritten))
        NextLine = L",";

    if (! NextLine.IsEmpty()) {
        if (! AppendText(Setting, NextLine))
            return FALSE;
    }

    return TRUE;
}


//---------------------------------------------------------------------------
// GetTextCsv
//---------------------------------------------------------------------------


void CUserSettings::GetTextCsv(
    const CString &Setting, CStringList &ValueList) const
{
    while (! ValueList.IsEmpty())
        ValueList.RemoveHead();

    CStringList ValueListRead;
    GetTextList(Setting, ValueListRead);

    while (! ValueListRead.IsEmpty()){
        CString text = ValueListRead.RemoveHead();
        while (! text.IsEmpty()) {

            CString NextValue;
            int comma = text.Find(L',');
            if (comma == -1) {
                NextValue = text;
                text = CString();
            } else {
                NextValue = text.Left(comma);
                text = text.Mid(comma + 1);
            }
            NextValue.TrimLeft();
            NextValue.TrimRight();
            if (! NextValue.IsEmpty())
                ValueList.AddTail(NextValue);
        }
    }
}


//---------------------------------------------------------------------------
// GetSettingsNames
//---------------------------------------------------------------------------


void CUserSettings::GetSettingsNames(CStringList &Names)
{
    ULONG PrefixLen = m_SettingPrefix.GetLength();
    CStringList list;
    CSbieIni::GetInstance().GetSettingsNames(m_Section, list);
    while (! list.IsEmpty()) {
        CString name = list.RemoveHead();
        if (name.Left(PrefixLen) == m_SettingPrefix)
            Names.AddTail(name.Mid(PrefixLen));
    }
}
