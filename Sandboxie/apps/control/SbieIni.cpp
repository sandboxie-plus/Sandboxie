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


#include "stdafx.h"
#include "SbieIni.h"

#include "core/svc/SbieIniWire.h"
#include "apps/common/MyMsgBox.h"
#include "common/win32_ntddk.h"
#include "common/my_version.h"
#include <wincred.h>


//---------------------------------------------------------------------------
// Defines
//---------------------------------------------------------------------------


#define REQUEST_LEN                 4096


//---------------------------------------------------------------------------
// Variables
//---------------------------------------------------------------------------


CSbieIni *CSbieIni::m_instance = NULL;

CString CSbieIni::m_Tmpl_(L"Tmpl.");
CString CSbieIni::m_Template(L"Template");
CString CSbieIni::m_Template_(L"Template_");
CString CSbieIni::m_TemplateSettings(L"TemplateSettings");
CString CSbieIni::m_UserSettings_(L"UserSettings_");

volatile bool CSbieIni::m_ServiceStopped = false;


//---------------------------------------------------------------------------
// Constructor
//---------------------------------------------------------------------------


CSbieIni::CSbieIni()
{
    //
    // allocate and initialize the SbieSvc request buffer
    //

    m_ExpandsInit = FALSE;

    m_Password[0] = L'\0';
}


//---------------------------------------------------------------------------
// Destructor
//---------------------------------------------------------------------------


CSbieIni::~CSbieIni()
{
}


//---------------------------------------------------------------------------
// GetInstance
//---------------------------------------------------------------------------


CSbieIni &CSbieIni::GetInstance()
{
    if (! m_instance)
        m_instance = new CSbieIni();
    return *m_instance;
}


//---------------------------------------------------------------------------
// GetPath
//---------------------------------------------------------------------------


CString CSbieIni::GetPath(BOOL *IsHomePath)
{
    CString path;

    if (IsHomePath)
        *IsHomePath = FALSE;

    SBIE_INI_GET_PATH_REQ *req =
        (SBIE_INI_GET_PATH_REQ *)malloc(REQUEST_LEN);
    if (req) {

        req->h.msgid = MSGID_SBIE_INI_GET_PATH;
        req->h.length = sizeof(SBIE_INI_GET_PATH_REQ);

        SBIE_INI_GET_PATH_RPL *rpl =
            (SBIE_INI_GET_PATH_RPL *)SbieDll_CallServer(&req->h);
        if (rpl) {

            if (rpl->h.status == 0) {

                path = rpl->path;

                if (IsHomePath && rpl->is_home_path)
                    *IsHomePath = TRUE;
            }

            SbieDll_FreeMem(rpl);
        }

        free(req);
    }

    return path;
}


//---------------------------------------------------------------------------
// GetUser
//---------------------------------------------------------------------------


BOOL CSbieIni::GetUser(CString &Section, CString &Name, BOOL &IsAdmin)
{
    Section = CString();
    Name = CString();
    IsAdmin = FALSE;

    SBIE_INI_GET_USER_REQ *req =
        (SBIE_INI_GET_USER_REQ *)malloc(REQUEST_LEN);
    if (req) {

        req->h.msgid = MSGID_SBIE_INI_GET_USER;
        req->h.length = sizeof(SBIE_INI_GET_USER_REQ);

        SBIE_INI_GET_USER_RPL *rpl =
            (SBIE_INI_GET_USER_RPL *)SbieDll_CallServer(&req->h);
        if (rpl) {
            if (rpl->h.status == 0) {
                if (rpl->admin)
                    IsAdmin = TRUE;
                Section = CString(rpl->section);
                Name    = CString(rpl->name);
            }
            SbieDll_FreeMem(rpl);
        }

        free(req);
    }

    if (Section.IsEmpty()) {

        CMyApp::MsgBox(NULL, CMyMsg(MSG_3313), MB_OK);
        Section = L"Unknown";
        Name = L"Unknown";
    }

    return (! (Section.IsEmpty() || Name.IsEmpty()));
}


//---------------------------------------------------------------------------
// GetSettingsNames
//---------------------------------------------------------------------------


void CSbieIni::GetSettingsNames(const CString &Section, CStringList &Names)
{
    ULONG buf_len = sizeof(WCHAR) * CONF_LINE_LEN;
    WCHAR *buf = malloc_WCHAR(buf_len);

    ULONG index = 0;
    while (1) {
        LONG rc = SbieApi_QueryConfAsIs(
                        Section, NULL, index, buf, buf_len - 4);
        if (rc != 0)
            break;
        Names.AddTail(buf);
        ++index;
    }

    free(buf);
}


//---------------------------------------------------------------------------
// SetText
//---------------------------------------------------------------------------


BOOL CSbieIni::SetText(
    const CString &Section, const CString &Setting, const CString &Value)
{
    CString oldValue;
    GetText(Section, Setting, oldValue);
    if (oldValue == Value)
        return TRUE;

    return CallSbieSvc(MSGID_SBIE_INI_SET_SETTING, Section, Setting, Value);
}


//---------------------------------------------------------------------------
// SetNum
//---------------------------------------------------------------------------


BOOL CSbieIni::SetNum(
    const CString &Section, const CString &Setting, int Value)
{
    CString ValueStr;
    ValueStr.Format(L"%d", Value);
    return SetText(Section, Setting, ValueStr);
}


//---------------------------------------------------------------------------
// SetNum64
//---------------------------------------------------------------------------


BOOL CSbieIni::SetNum64(
    const CString &Section, const CString &Setting, __int64 Value)
{
    CString ValueStr;
    ValueStr.Format(L"%I64d", Value);
    return SetText(Section, Setting, ValueStr);
}


//---------------------------------------------------------------------------
// SetBool
//---------------------------------------------------------------------------


BOOL CSbieIni::SetBool(
    const CString &Section, const CString &Setting, BOOL Value)
{
    return SetText(Section, Setting, (Value ? L"y" : L"n"));
}


//---------------------------------------------------------------------------
// GetText
//---------------------------------------------------------------------------


void CSbieIni::GetText(
    const CString &Section, const CString &Setting,
    CString &Value, const CString &Default) const
{
    if (Section.IsEmpty()) {
        Value = Default;
        return;
    }

    ULONG buf_len = sizeof(WCHAR) * CONF_LINE_LEN;
    WCHAR *buf = malloc_WCHAR(buf_len);
    int flags = Section.IsEmpty() ? 0 : CONF_GET_NO_GLOBAL;
    SbieApi_QueryConfAsIs(
        Section, Setting, flags, buf, buf_len - 4);
    if (buf[0])
        Value = buf;
    else
        Value = Default;
    free(buf);
}


//---------------------------------------------------------------------------
// GetNum
//---------------------------------------------------------------------------


void CSbieIni::GetNum(
    const CString &Section, const CString &Setting,
    int &Value, int Default) const
{
    CString ValueStr;
    GetText(Section, Setting, ValueStr);
    Value = _wtoi(ValueStr);
    if (! Value)
        Value = Default;
}


//---------------------------------------------------------------------------
// GetNum64
//---------------------------------------------------------------------------


void CSbieIni::GetNum64(
    const CString &Section, const CString &Setting,
    __int64 &Value, __int64 Default) const
{
    CString ValueStr;
    GetText(Section, Setting, ValueStr);
    Value = _wtoi64(ValueStr);
    if (! Value)
        Value = Default;
}


//---------------------------------------------------------------------------
// GetBool
//---------------------------------------------------------------------------


void CSbieIni::GetBool(
    const CString &Section, const CString &Setting,
    BOOL &Value, BOOL Default) const
{
    CString ValueStr;
    GetText(Section, Setting, ValueStr);
    WCHAR ch = 0;
    if (! ValueStr.IsEmpty())
        ch = ValueStr.GetAt(0);
    if (ch == L'y' || ch == L'Y')
        Value = TRUE;
    else if (ch == L'n' || ch == L'N')
        Value = FALSE;
    else
        Value = Default;
}


//---------------------------------------------------------------------------
// GetTextList
//---------------------------------------------------------------------------


void CSbieIni::GetTextList(
    const CString &Section, const CString &Setting, CStringList &Value,
    BOOL withBrackets) const
{
    if (Section.IsEmpty())
        return;

    ULONG buf_len = sizeof(WCHAR) * CONF_LINE_LEN;
    WCHAR *buf = malloc_WCHAR(buf_len);

    int index = 0;
    int flags = Section.IsEmpty() ? 0 : CONF_GET_NO_GLOBAL;
    if (withBrackets)
        flags |= CONF_GET_NO_TEMPLS;
    while (1) {
        SbieApi_QueryConfAsIs(
            Section, Setting, index | flags, buf, buf_len - 4);
        if (! buf[0])
            break;
        ++index;
        Value.AddTail(buf);
    }

    if (withBrackets)
        GetTextListWithBrackets(Section, Setting, Value, buf, buf_len);

    free(buf);
}


//---------------------------------------------------------------------------
// InsertText
//---------------------------------------------------------------------------


BOOL CSbieIni::InsertText(
    const CString &Section, const CString &Setting, const CString &Value)
{
    return CallSbieSvc(MSGID_SBIE_INI_INS_SETTING, Section, Setting, Value);
}


//---------------------------------------------------------------------------
// AppendText
//---------------------------------------------------------------------------


BOOL CSbieIni::AppendText(
    const CString &Section, const CString &Setting, const CString &Value)
{
    return CallSbieSvc(MSGID_SBIE_INI_ADD_SETTING, Section, Setting, Value);
}


//---------------------------------------------------------------------------
// DelText
//---------------------------------------------------------------------------


BOOL CSbieIni::DelText(
    const CString &Section, const CString &Setting, const CString &Value)
{
    return CallSbieSvc(MSGID_SBIE_INI_DEL_SETTING, Section, Setting, Value);
}


//---------------------------------------------------------------------------
// DelText2
//---------------------------------------------------------------------------


BOOL CSbieIni::DelText2(
    const CString &Section, const CString &Setting, const CString &Value,
    const CString &ProcessGroup)
{
    BOOL ok = CallSbieSvc(
        MSGID_SBIE_INI_DEL_SETTING, Section, Setting, Value);

    if (ok) {
        CString value1 = L"!" + ProcessGroup + L"," + Value;
        ok = CallSbieSvc(
            MSGID_SBIE_INI_DEL_SETTING, Section, Setting, value1);
    }

    return ok;
}


//---------------------------------------------------------------------------
// CallSbieSvc
//---------------------------------------------------------------------------


BOOL CSbieIni::CallSbieSvc(
    ULONG msgid,
    const CString &Section, const CString &Setting,
    const CString &Value)
{
    if (Section.IsEmpty())
        return TRUE;

    BOOL ok = FALSE;

    SBIE_INI_SETTING_REQ *req =
        (SBIE_INI_SETTING_REQ *)malloc(REQUEST_LEN);
    if (req) {

        req->refresh = TRUE;

        wcscpy(req->section, Section);
        wcscpy(req->setting, Setting);

        wcscpy(req->value, Value);
        req->value_len = Value.GetLength();

        req->h.msgid = msgid;
        req->h.length = sizeof(SBIE_INI_SETTING_REQ)
                      + req->value_len * sizeof(WCHAR);

        ok = CallServerWithPassword(
                req, req->password, Section, Setting);

        free(req);
    }

    return ok;
}


//---------------------------------------------------------------------------
// InitGenericPaths
//---------------------------------------------------------------------------


void CSbieIni::InitGenericPaths(BOOL WithUser)
{
    CString keyPath(
                L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\");
    if (WithUser)
        keyPath += L"User ";
    keyPath += L"Shell Folders";

    HKEY hkey;
    LONG rc = RegOpenKey(HKEY_CURRENT_USER, keyPath, &hkey);

    ULONG index = 0;
    while (rc == 0) {

        WCHAR name[64];
        WCHAR value[MAX_PATH + 8];
        ULONG name_len;
        ULONG value_len;
        ULONG type;

        name_len = 60;
        value_len = MAX_PATH + 4;
        rc = RegEnumValue(hkey, index,
                name, &name_len, NULL, &type, (BYTE *)value, &value_len);
        if (rc == 0 && (type == REG_SZ || type == REG_EXPAND_SZ)) {

            WCHAR expand[MAX_PATH + 8];
            ULONG len =
                ExpandEnvironmentStrings(value, expand, MAX_PATH + 4);
            if (len > 0 && len <= MAX_PATH) {
                CString value(expand);
                value.TrimLeft();
                value.TrimRight();
                if (! value.IsEmpty())
                    m_Expands.SetAt(name, expand);
            }
        }

        ++index;
    }

    RegCloseKey(hkey);
}


//---------------------------------------------------------------------------
// MakeGenericPath
//---------------------------------------------------------------------------


CString CSbieIni::MakeGenericPath(const CString &path)
{
    if (! m_ExpandsInit) {
        InitGenericPaths(FALSE);
        InitGenericPaths(TRUE);
        m_ExpandsInit = TRUE;
    }

    POSITION pos = m_Expands.GetStartPosition();
    while (pos) {
        CString key;
        CString val;
        m_Expands.GetNextAssoc(pos, key, val);

        if (path.GetLength() >= val.GetLength() &&
            path.Mid(0, val.GetLength()).CompareNoCase(val) == 0) {

            CString retval = L"%" + key + L"%";
            if (path.GetLength() > val.GetLength())
                retval += path.Mid(val.GetLength());
            return retval;
        }
    }

    return path;
}


//---------------------------------------------------------------------------
// MakeSpecificPath
//---------------------------------------------------------------------------


CString CSbieIni::MakeSpecificPath(const CString &path)
{
    if (! m_ExpandsInit) {
        InitGenericPaths(FALSE);
        InitGenericPaths(TRUE);
        m_ExpandsInit = TRUE;
    }

    POSITION pos = m_Expands.GetStartPosition();
    while (pos) {
        CString key;
        CString val;
        m_Expands.GetNextAssoc(pos, key, val);
        key = L"%" + key + L"%";

        if (path.GetLength() >= key.GetLength() &&
            path.Mid(0, key.GetLength()).CompareNoCase(key) == 0) {

            CString retval = val;
            if (path.GetLength() > key.GetLength())
                retval += path.Mid(key.GetLength());
            return retval;
        }
    }

    return path;
}


//---------------------------------------------------------------------------
// RenameOrCopySection
//---------------------------------------------------------------------------


BOOL CSbieIni::RenameOrCopySection(
    const CString &OldName, const CString &NewName, bool deleteOldName)
{
    if (OldName.IsEmpty() || NewName.IsEmpty())
        return FALSE;
    bool SameName = (bool)(NewName.CompareNoCase(OldName) == 0);

    CStringList settings;

    //
    // get settings from old section
    //

    ULONG buf_len = sizeof(WCHAR) * CONF_LINE_LEN;
    WCHAR *buf = malloc_WCHAR(buf_len);
    int setting_index = 0;
    LONG status = STATUS_SUCCESS;

    while (1) {

        status = SbieApi_QueryConfAsIs(
                    OldName, NULL, setting_index | CONF_GET_NO_TEMPLS,
                    buf, buf_len - 4);
        if (status == STATUS_RESOURCE_NAME_NOT_FOUND) {
            status = STATUS_SUCCESS;
            break;
        }
        if (status != STATUS_SUCCESS)
            break;

        ++setting_index;
        CString setting_name = buf;

        int value_index = 0;
        while (1) {

            status = SbieApi_QueryConfAsIs(
                    OldName, setting_name,
                    value_index | CONF_GET_NO_GLOBAL | CONF_GET_NO_TEMPLS,
                    buf, buf_len - 4);
            if (status == STATUS_RESOURCE_NAME_NOT_FOUND) {
                status = STATUS_SUCCESS;
                break;
            }
            if (status != STATUS_SUCCESS)
                break;
            ++value_index;
            settings.AddTail(setting_name + L"=" + buf);
        }

        if (status != STATUS_SUCCESS)
            break;
    }

    //
    // check for any settings in new section, to prevent overwriting
    //

    if (status == STATUS_SUCCESS && NewName.CompareNoCase(OldName) != 0) {

        status = SbieApi_QueryConfAsIs(NewName, NULL, 0, buf, buf_len - 4);
        if (status == STATUS_RESOURCE_NAME_NOT_FOUND)
            status = STATUS_SUCCESS;
        else
            status = STATUS_OBJECT_NAME_COLLISION;
    }

    //
    // delete old section before creating the new section,
    // if NewName and OldName are same
    //

    if (deleteOldName && SameName && status == STATUS_SUCCESS) {

        if (! RemoveSection(OldName))
            status = STATUS_UNSUCCESSFUL;
    }

    //
    // apply new settings
    //

    if (status == STATUS_SUCCESS) {

        while (! settings.IsEmpty()) {
            CString setting = settings.RemoveHead();
            int index = setting.Find(L'=');
            CString value = setting.Mid(index + 1);
            setting = setting.Left(index);
            if (! InsertText(NewName, setting, value)) {
                status = STATUS_UNSUCCESSFUL;
                break;
            }
        }
    }

    //
    // delete old section only after creating the new section,
    // if NewName and OldName are different
    //

    if (deleteOldName && (! SameName) && status == STATUS_SUCCESS) {

        if (! RemoveSection(OldName))
            status = STATUS_UNSUCCESSFUL;
    }

    //
    // finish
    //

    free(buf);

    if (status != STATUS_SUCCESS) {

        CString err;

        if (status == STATUS_LOGON_NOT_GRANTED)
            err = CMyMsg(MSG_3312, NewName);
        else {
            err.Format(L"%08X", status);
            err = CMyMsg(MSG_3311, NewName, L"*", err);
        }

        CMyApp::MsgBox(NULL, err, MB_OK);
        return FALSE;
    }

    return TRUE;
}


//---------------------------------------------------------------------------
// RemoveSection
//---------------------------------------------------------------------------


BOOL CSbieIni::RemoveSection(const CString &Section)
{
    return CallSbieSvc(MSGID_SBIE_INI_SET_SETTING, Section, L"*", L"");
}


//---------------------------------------------------------------------------
// SetTemplateVariable
//---------------------------------------------------------------------------


BOOL CSbieIni::SetTemplateVariable(const CString &Name, const CString &Value,
                                   BOOL UserSpecific)
{
    BOOL ok = FALSE;

    SBIE_INI_TEMPLATE_REQ *req =
        (SBIE_INI_TEMPLATE_REQ *)malloc(REQUEST_LEN);
    if (req) {

        wcsncpy(req->varname, Name, 64);
        req->varname[65] = L'\0';

        if (UserSpecific)
            req->user = TRUE;
        else
            req->user = FALSE;

        wcscpy(req->value, Value);
        req->value_len = Value.GetLength();

        req->h.msgid = MSGID_SBIE_INI_TEMPLATE;
        req->h.length = sizeof(SBIE_INI_TEMPLATE_REQ)
                      + req->value_len * sizeof(WCHAR);

        ok = CallServerWithPassword(
                req, req->password, m_TemplateSettings, Name);

        free(req);
    }

    return ok;
}


//---------------------------------------------------------------------------
// GetTemplateClasses
//---------------------------------------------------------------------------


void CSbieIni::GetTemplateClasses(CStringList &list)
{
    ULONG buf_len = sizeof(WCHAR) * CONF_LINE_LEN;
    WCHAR *buf = malloc_WCHAR(buf_len);

    int index = 0;
    while (1) {
        SbieApi_QueryConfAsIs(NULL, NULL, index, buf, buf_len - 4);
        if (! buf[0])
            break;
        ++index;

        CString section(buf);
        SbieApi_QueryConfAsIs(section, L"Tmpl.Class", CONF_GET_NO_GLOBAL,
                              buf, buf_len - 4);
        if (! buf[0])
            break;

        BOOL found = FALSE;
        POSITION pos = list.GetHeadPosition();
        while (pos) {
            CString cls = list.GetNext(pos);
            if (cls.CompareNoCase(buf) == 0) {
                found = TRUE;
                break;
            }
        }

        if (! found)
            list.AddTail(buf);
    }

    free(buf);
}


//---------------------------------------------------------------------------
// GetTemplateNames
//---------------------------------------------------------------------------


void CSbieIni::GetTemplateNames(const CString &forClass, CStringList &list)
{
    ULONG buf_len = sizeof(WCHAR) * CONF_LINE_LEN;
    WCHAR *buf = malloc_WCHAR(buf_len);

    BOOL all_classes = (forClass.Compare(L"*") == 0);

    int index = 0;
    while (1) {
        SbieApi_QueryConfAsIs(NULL, NULL, index, buf, buf_len - 4);
        if (! buf[0])
            break;
        ++index;

        CString section(buf);
        if (section.Left(9).CompareNoCase(m_Template_) != 0)
            continue;

        SbieApi_QueryConfAsIs(section, L"Tmpl.Class", CONF_GET_NO_GLOBAL,
                              buf, buf_len - 4);
        if (buf[0] && (all_classes || forClass.CompareNoCase(buf) == 0))
            list.AddTail(section.Mid(9));
    }

    free(buf);
}


//---------------------------------------------------------------------------
// GetTemplateVariable
//---------------------------------------------------------------------------


CString CSbieIni::GetTemplateVariable(const CString &Template)
{
    CString TemplateSectionName = m_Template_ + Template;

    CStringList settings;
    GetSettingsNames(TemplateSectionName, settings);

    while (! settings.IsEmpty()) {
        CString setting = settings.RemoveHead();
        CStringList values;
        GetTextList(TemplateSectionName, setting, values);

        while (! values.IsEmpty()) {
            CString value = values.RemoveHead();
            int index = value.Find(L'%' + m_Tmpl_);
            if (index != -1) {
                CString varname = value.Mid(index + 1);
                index = varname.Find(L'%');
                if (index != -1)
                    return varname.Left(index);
            }
        }
    }

    return CString();
}


//---------------------------------------------------------------------------
// GetTemplateMetaData
//---------------------------------------------------------------------------


CString CSbieIni::GetTemplateMetaData(
    const CString &Template, const CString &Setting)
{
    CString TemplateSectionName = m_Template_ + Template;
    CString TemplateSettingName = m_Tmpl_     + Setting;
    CString value;
    GetText(TemplateSectionName, TemplateSettingName, value);
    return value;
}


//---------------------------------------------------------------------------
// GetTextListWithBrackets
//---------------------------------------------------------------------------


NOINLINE void CSbieIni::GetTextListWithBrackets(
    const CString &Section, const CString &Setting, CStringList &Value,
    WCHAR *buf, ULONG buf_len) const
{
    int index = 0;
    while (1) {
        SbieApi_QueryConfAsIs(
            Section, m_Template, index | CONF_GET_NO_TEMPLS,
            buf, buf_len - 4);
        if (! buf[0])
            break;
        ++index;

        CString tmpl_name(buf);
        tmpl_name = m_Template_ + tmpl_name;

        int tmpl_index = 0;
        while (1) {

            SbieApi_QueryConfAsIs(
                tmpl_name, Setting, tmpl_index | CONF_GET_NO_GLOBAL,
                buf, buf_len - 4);
            if (! buf[0])
                break;
            ++tmpl_index;

            if (_wcsnicmp(buf, L"Tmpl.", 5) == 0)
                continue;

            Value.AddTail(AddBrackets(buf));
        }
    }
}


//---------------------------------------------------------------------------
// AddBrackets
//---------------------------------------------------------------------------


NOINLINE CString CSbieIni::AddBrackets(const CString &value)
{
    return L'[' + value + L']';
}


//---------------------------------------------------------------------------
// StripBrackets
//---------------------------------------------------------------------------


NOINLINE BOOL CSbieIni::StripBrackets(CString &value)
{
    int len = value.GetLength();
    if (len > 2 && value.GetAt(0) == L'[' && value.GetAt(len - 1) == L']') {
        value = value.Mid(1, len - 2);
        return TRUE;
    } else
        return FALSE;
}


//---------------------------------------------------------------------------
// CallServerWithPassword
//---------------------------------------------------------------------------


BOOL CSbieIni::CallServerWithPassword(
    void *RequestBuf, WCHAR *pPasswordWithinRequestBuf,
    const CString &SectionName, const CString &SettingName)
{
    if (m_ServiceStopped)
        return FALSE;

    BOOL ok = FALSE;
    ULONG status = STATUS_INSUFFICIENT_RESOURCES;
    BOOL FirstRetry = TRUE;

    while (1) {

        wcscpy(pPasswordWithinRequestBuf, m_Password);

        MSG_HEADER *rpl = SbieDll_CallServer((MSG_HEADER *)RequestBuf);
        if (rpl) {
            status = rpl->status;
            SbieDll_FreeMem(rpl);
        } else
            status = STATUS_SERVER_DISABLED;

        SecureZeroMemory(pPasswordWithinRequestBuf, sizeof(WCHAR) * 64);

        if (status != STATUS_WRONG_PASSWORD) {
            if (status == 0)
                ok = TRUE;
            break;
        }

        SecureZeroMemory(m_Password, sizeof(m_Password));

        if (FirstRetry)
            FirstRetry = FALSE;
        else {
            int rc = CMyApp::MsgBox(NULL, MSG_4274, MB_YESNO);
            if (rc != IDYES)
                break;
        }

        if (! InputPassword(MSG_4271, m_Password))
            break;
    }

    if (! ok) {

        CString err;

        if (status == STATUS_LOGON_NOT_GRANTED ||
            status == STATUS_WRONG_PASSWORD)
        {
            err = CMyMsg(MSG_3312, SectionName);
        } else {
            err.Format(L"%08X", status);
            err = CMyMsg(MSG_3311, SectionName, SettingName, err);
        }

        CMyApp::MsgBox(NULL, err, MB_OK);
    }

    return ok;
}


//---------------------------------------------------------------------------
// SetOrTestPassword
//---------------------------------------------------------------------------


BOOL CSbieIni::SetOrTestPassword(WCHAR *NewPassword66)
{
    BOOL ok = FALSE;

    SBIE_INI_PASSWORD_REQ *req =
        (SBIE_INI_PASSWORD_REQ *)malloc(REQUEST_LEN);

    if (req) {

        req->h.msgid = MSGID_SBIE_INI_TEST_PASSWORD;
        req->h.length = sizeof(SBIE_INI_PASSWORD_REQ);

        ok = CallServerWithPassword(
                req, req->old_password, _GlobalSettings, L"*");
    }

    if (ok && NewPassword66) {

        wmemcpy(req->new_password, NewPassword66, 64);
        req->new_password[65] = L'\0';

        req->h.msgid = MSGID_SBIE_INI_SET_PASSWORD;
        req->h.length = sizeof(SBIE_INI_PASSWORD_REQ);

        ok = CallServerWithPassword(
                req, req->old_password, _GlobalSettings, L"*");
    }

    if (req)
        free(req);

    return ok;
}


//---------------------------------------------------------------------------
// GetRestrictions
//---------------------------------------------------------------------------


void CSbieIni::GetRestrictions(
    BOOL &EditAdminOnly, BOOL &DisableForceAdminOnly,
    BOOL &NeedPassword, BOOL &HavePassword, BOOL &ForgetPassword)
{
    GetBool(_GlobalSettings, L"EditAdminOnly",
            EditAdminOnly, FALSE);

    GetBool(_GlobalSettings, L"ForceDisableAdminOnly",
            DisableForceAdminOnly, FALSE);

    CString hash;
    GetText(_GlobalSettings, L"EditPassword",  hash);
    NeedPassword = hash.IsEmpty() ? FALSE : TRUE;

    HavePassword = m_Password[0] ? TRUE : FALSE;

    GetBool(_GlobalSettings, L"ForgetPassword",
            ForgetPassword, FALSE);
}


//---------------------------------------------------------------------------
// SetRestrictions
//---------------------------------------------------------------------------


void CSbieIni::SetRestrictions(
        BOOL EditAdminOnly, BOOL DisableForceAdminOnly,
        BOOL ForgetPassword)
{
    SetBool(_GlobalSettings, L"EditAdminOnly",
            EditAdminOnly);

    SetBool(_GlobalSettings, L"ForceDisableAdminOnly",
            DisableForceAdminOnly);

    SetBool(_GlobalSettings, L"ForgetPassword",
            ForgetPassword);
}


//---------------------------------------------------------------------------
// ForgetPasswordNow
//---------------------------------------------------------------------------


void CSbieIni::ForgetPasswordNow()
{
    BOOL ForgetPassword;
    GetBool(_GlobalSettings, L"ForgetPassword", ForgetPassword, FALSE);
    if (ForgetPassword)
        SecureZeroMemory(m_Password, sizeof(m_Password));
}


//---------------------------------------------------------------------------
// InputPassword
//---------------------------------------------------------------------------


BOOL CSbieIni::InputPassword(ULONG msgid, WCHAR *Password66)
{
    typedef ULONG (*P_CredUIPromptForCredentials)(
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

    static P_CredUIPromptForCredentials pCredUIPromptForCredentials = NULL;

    if (! pCredUIPromptForCredentials) {
        HMODULE credui = LoadLibrary(L"credui.dll");
        if (credui) {
            pCredUIPromptForCredentials = (P_CredUIPromptForCredentials)
                GetProcAddress(credui, "CredUIPromptForCredentialsW");
        }
    }
    if (! pCredUIPromptForCredentials)
        pCredUIPromptForCredentials = MyPromptForCredentials;

    //
    // invoke credentials prompt
    //

    CMyMsg msgtext(msgid);

    CREDUI_INFOW info;
    info.cbSize = sizeof(CREDUI_INFOW);
    info.pszMessageText = msgtext;
    info.pszCaptionText = SANDBOXIE;
    info.hbmBanner = NULL;
    CWnd *pParentWnd = CBaseDialog::GetInputWindow();
    info.hwndParent = pParentWnd ? pParentWnd->m_hWnd : NULL;

    Password66[0] = L'\0';

    WCHAR UserName[66];
    wcscpy(UserName, CMyMsg(MSG_3769));

    BOOL save = FALSE;

    ULONG rc = pCredUIPromptForCredentials(
        &info, SANDBOXIE,
        NULL, 0,
        UserName, 64 + 1,
        Password66, 64 + 1,
        &save,
        CREDUI_FLAGS_DO_NOT_PERSIST | CREDUI_FLAGS_GENERIC_CREDENTIALS);

    Password66[65] = L'\0';

    return (rc == 0 && Password66[0] != L'\0');
}


//---------------------------------------------------------------------------
// MyPromptForCredentials
//---------------------------------------------------------------------------


ULONG CSbieIni::MyPromptForCredentials(
    void *pUiInfo,
    const WCHAR *pszTargetName,
    void *pContext,
    ULONG dwAuthError,
    WCHAR *pszUserName,
    ULONG ulUserNameBufferSize,
    WCHAR *pszPassword,
    ULONG ulPasswordBufferSize,
    BOOL *save,
    ULONG dwFlags)
{
    CREDUI_INFOW *info = (CREDUI_INFOW *)pUiInfo;

    ULONG flags = MB_OKCANCEL | INPUTBOX_PASSWORD;
    if (CMyApp::m_LayoutRTL)
        flags |= MB_RTLREADING;
    WCHAR *edit =
            ::InputBox(info->hwndParent, info->pszMessageText, flags, NULL);
    if (! edit)
        return ERROR_CANCELLED;

    wcsncpy(pszPassword, edit, ulPasswordBufferSize - 1);
    pszPassword[ulPasswordBufferSize - 1] = L'\0';

    LocalFree(edit);
    return 0;
}


//---------------------------------------------------------------------------
// MonitorService
//---------------------------------------------------------------------------


void CSbieIni::MonitorService(CWnd *pWnd, ULONG idExitCmd)
{
    SBIE_INI_GET_WAIT_HANDLE_REQ req;
    req.h.length = sizeof(SBIE_INI_GET_WAIT_HANDLE_REQ);
    req.h.msgid = MSGID_SBIE_INI_GET_WAIT_HANDLE;

    SBIE_INI_GET_WAIT_HANDLE_RPL *rpl =
        (SBIE_INI_GET_WAIT_HANDLE_RPL *)SbieDll_CallServer(&req.h);
    if (rpl) {
        if (rpl->h.status == 0) {
            ULONG ThreadId;
            ULONG_PTR *ThreadArgs = new ULONG_PTR[3];
            ThreadArgs[0] = (ULONG_PTR)pWnd->m_hWnd;
            ThreadArgs[1] = (ULONG_PTR)idExitCmd;
            ThreadArgs[2] = (ULONG_PTR)rpl->hProcess;
            CreateThread(
                NULL, 0, MonitorServiceThread, ThreadArgs, 0, &ThreadId);
        }
        SbieDll_FreeMem(rpl);
    }
}


//---------------------------------------------------------------------------
// MonitorServiceThread
//---------------------------------------------------------------------------


ULONG CSbieIni::MonitorServiceThread(void *lpParameter)
{
    ULONG_PTR *ThreadArgs = (ULONG_PTR *)lpParameter;
    WaitForSingleObject((HANDLE *)ThreadArgs[2], INFINITE);
    m_ServiceStopped = true;
    ::PostMessage((HWND)ThreadArgs[0], WM_COMMAND, ThreadArgs[1], 0);
    return 0;
}
