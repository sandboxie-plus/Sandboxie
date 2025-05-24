/*
 *
 * Copyright 2020-2025 David Xanatos, xanasoft.com
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "stdafx.h"
#include "SbieIni.h"
#include "../SbieAPI.h"
#include "../SbieDefs.h"

#include <ntstatus.h>
#define WIN32_NO_STATUS
typedef long NTSTATUS;

#include "..\..\Sandboxie\core\drv\api_flags.h"

CSbieIni::CSbieIni(const QString& Section, class CSbieAPI* pAPI, QObject* parent) : QObject(parent)
{
	Q_ASSERT(!Section.isEmpty());
	m_Name = Section;
	m_pAPI = pAPI;
	m_RefreshOnChange = true;

	m_IsVirtual = GetBool("IsVirtual", false, false, false, true);
}

CSbieIni::~CSbieIni()
{
}

SB_STATUS CSbieIni::SetText(const QString& Setting, const QString& Value)
{
	if (GetText(Setting) == Value)
		return SB_OK;
	return SbieIniSet(m_Name, Setting, Value, eIniUpdate, m_RefreshOnChange);
}

SB_STATUS CSbieIni::SetNum(const QString& Setting, int Value)
{
	return SetText(Setting, QString::number(Value));
}

SB_STATUS CSbieIni::SetNum64(const QString& Setting, __int64 Value)
{
	return SetText(Setting, QString::number(Value));
}

SB_STATUS CSbieIni::SetBool(const QString& Setting, bool Value)
{
	return SetText(Setting, Value ? "y" : "n");
}

SB_STATUS CSbieIni::SetBoolSafe(const QString& Setting, bool Value)
{
	QString StrValue = Value ? "y" : "n";
	bool bAdd = true;
	QStringList Values = GetTextList(Setting, false);
	foreach(const QString & CurValue, Values) {
		if (CurValue.contains(","))
			continue;
		if (CurValue == StrValue)
			bAdd = false;
		else
			DelValue(Setting, CurValue);
	}
	if(bAdd)
		return AppendText(Setting, StrValue);
	return SB_OK;
}

QString CSbieIni::GetText(const QString& Setting, const QString& Default, bool bWithGlobal, bool bNoExpand, bool withTemplates, bool getProperty) const
{
	int flags = (bWithGlobal ? 0 : CONF_GET_NO_GLOBAL);
	if (!withTemplates)
		flags |= CONF_GET_NO_TEMPLS;
	if (bNoExpand)
		flags |= CONF_GET_NO_EXPAND;
	if (getProperty)
		flags |= CONF_GET_PROPERTY;

	QString Value = SbieIniGet(m_Name, Setting, flags);
	if (Value.isNull()) Value = Default;
	return Value;
}

int CSbieIni::GetNum(const QString& Setting, int Default, bool bWithGlobal, bool withTemplates, bool getProperty) const
{
	QString StrValue = GetText(Setting, QString(), bWithGlobal, true, withTemplates, getProperty);
	bool ok;
	int Value = StrValue.toInt(&ok);
	if (!ok) return Default;
	return Value;
}

__int64 CSbieIni::GetNum64(const QString& Setting, __int64 Default, bool bWithGlobal, bool withTemplates, bool getProperty) const
{
	QString StrValue = GetText(Setting, QString(), bWithGlobal, true, withTemplates, getProperty);
	bool ok;
	__int64 Value = StrValue.toLongLong(&ok);
	if (!ok) return Default;
	return Value;
}

bool CSbieIni::GetBool(const QString& Setting, bool Default, bool bWithGlobal, bool withTemplates, bool getProperty) const
{
	QStringList Values;
	if (getProperty)
		Values << GetText(Setting, QString(), false, true, false, true); // when querying properties bWithGlobal and bWithTempaltes are ignored
	else
		Values = GetTextList(Setting, withTemplates, false, bWithGlobal);
	foreach(const QString &StrValue, Values) {
		if (StrValue.contains(","))
			continue;
		if (StrValue.compare("y", Qt::CaseInsensitive) == 0)
			return true;
		if (StrValue.compare("n", Qt::CaseInsensitive) == 0)
			return false;
	}
	return Default;
}

QStringList CSbieIni::GetTextList(const QString &Setting, bool withTemplates, bool bExpand, bool bWithGlobal) const
{
	QStringList TextList;

	int flags = (bWithGlobal ? 0 : CONF_GET_NO_GLOBAL);
	if (!withTemplates)
		flags |= CONF_GET_NO_TEMPLS;
	if (!bExpand)
		flags |= CONF_GET_NO_EXPAND;

	for (int index = 0; ; index++)
	{
		QString Value = SbieIniGet(m_Name, Setting, index | flags);
		if (Value.isNull())
			break;
		TextList.append(Value);
	}

	return TextList;
}

SB_STATUS CSbieIni::UpdateTextList(const QString &Setting, const QStringList& List, bool withTemplates)
{
	QStringList OldSettings = GetTextList(Setting, withTemplates);
	QStringList NewSettings;
	foreach(const QString& Value, List) {
		if (!OldSettings.removeOne(Value))
			NewSettings.append(Value);
	}
	// delete removed or changed settings
	foreach(const QString& Value, OldSettings)
		DelValue(Setting, Value);
	// add new or changed settings
	foreach(const QString& Value, NewSettings)
		AppendText(Setting, Value);
	return SB_OK;
}

QStringList CSbieIni::GetTemplates() const
{
	QStringList Templates;
	Templates.append("GlobalSettings");

	for (int tmpl_index = 0; ; tmpl_index++)
	{
		QString TmplName = SbieIniGet(m_Name, "Template", tmpl_index | CONF_GET_NO_TEMPLS);
		if (TmplName.isNull())
			break;
		Templates.append(TmplName);
	}

	return Templates;
}

QStringList CSbieIni::GetTextListTmpl(const QString &Setting, const QString& Template) const
{
	QStringList TextList;

	for (int index = 0; ; index++)
	{
		QString Value = SbieIniGet((Template != "GlobalSettings") ? "Template_" + Template : Template, Setting, index | CONF_GET_NO_GLOBAL | CONF_GET_NO_EXPAND);
		if (Value.isNull())
			break;
		TextList.append(Value);
	}

	return TextList;
}

SB_STATUS CSbieIni::InsertText(const QString& Setting, const QString& Value)
{
	return SbieIniSet(m_Name, Setting, Value, eIniInsert, m_RefreshOnChange);
}

SB_STATUS CSbieIni::AppendText(const QString& Setting, const QString& Value)
{
	return SbieIniSet(m_Name, Setting, Value, eIniAppend, m_RefreshOnChange);
}

SB_STATUS CSbieIni::DelValue(const QString& Setting, const QString& Value)
{
	return SbieIniSet(m_Name, Setting, Value, eIniDelete, m_RefreshOnChange);
}

void CSbieIni::SetTextMap(const QString& Setting, const QMap<QString, QStringList> Map)
{
	QStringList Mapping;
	foreach(const QString& Group, Map.keys())
	{
		QString CurrentLine;
		foreach(const QString & Name, Map[Group]) {
			if (Setting.length() + 1 + Group.length() + 1 + CurrentLine.length() + 1 + Name.length() >= CONF_LINE_LEN) { // limit line length
				Mapping.append(Group + ":" + CurrentLine);
				CurrentLine.clear();
			}
			if (!CurrentLine.isEmpty()) CurrentLine.append(",");
			CurrentLine.append(Name);
		}
		if(!CurrentLine.isEmpty())
			Mapping.append(Group + ":" + CurrentLine);
	}
	DelValue(Setting);
	foreach(const QString & Value, Mapping)
		AppendText(Setting, Value);
}

QMap<QString, QStringList> CSbieIni::GetTextMap(const QString& Setting)
{
	QMap<QString, QStringList> Map;
	foreach(const QString &CurrentLine, GetTextList(Setting, false)) {
		int pos = CurrentLine.lastIndexOf(":");
		Map[pos == -1 ? "" : CurrentLine.left(pos)].append(CurrentLine.mid(pos+1).split(","));
	}
	return Map;
}

QList<CSbieIni::SbieIniValue> CSbieIni::GetIniSection(qint32* pStatus, bool withTemplates, bool withGlobals, bool noExpand) const
{
	qint32 status = STATUS_SUCCESS;

	int uFlags = 0;
	if (noExpand)
		uFlags |= CONF_GET_NO_EXPAND;
	if (!withGlobals)
		uFlags |= CONF_GET_NO_GLOBAL;
	if (!withTemplates)
		uFlags |= CONF_GET_NO_TEMPLS;

	QSet<QString> Names;

	if (withGlobals)
	{
		for (int Index = 0; ; Index++)
		{
			QString Name = SbieIniGet("GlobalSettings", "", Index | uFlags, &status);
			if (status == STATUS_RESOURCE_NAME_NOT_FOUND) {
				status = STATUS_SUCCESS;
				break;
			}
			if (status != STATUS_SUCCESS)
				break;

			Names.insert(Name);
		}
	}

	for (int Index = 0; ; Index++)
	{
		QString Name = SbieIniGet(m_Name, "", Index | uFlags, &status);
		if (status == STATUS_RESOURCE_NAME_NOT_FOUND) {
			status = STATUS_SUCCESS;
			break;
		}
		if (status != STATUS_SUCCESS)
			break;

		Names.insert(Name);
	}

	QList<CSbieIni::SbieIniValue> Settings;
	foreach(const QString& Name, Names)
	{
		for (int Index = 0; ; Index++)
		{
			quint32 Type = 0;
			QString setting_value = SbieIniGet(m_Name, Name, Index | uFlags, &status, &Type);
			if (status == STATUS_RESOURCE_NAME_NOT_FOUND) {
				status = STATUS_SUCCESS;
				break;
			}
			if (status != STATUS_SUCCESS)
				break;

			Settings.append(SbieIniValue{ Name, Type, setting_value });
		}

		if (status != STATUS_SUCCESS)
			break;
	}

	if (pStatus) *pStatus = status;
	return Settings;
}

SB_STATUS CSbieIni::RenameSection(const QString& NewName, bool deleteOld) // Note: deleteOld is used when duplicating a box
{
	qint32 status = STATUS_SUCCESS;

	if (m_Name.isEmpty() || NewName.isEmpty())
		return SB_ERR();
	bool SameName = (bool)(NewName.compare(m_Name, Qt::CaseInsensitive) == 0);

	// Get all Settings
	QList<SbieIniValue> Settings = GetIniSection(&status);
	if (status != STATUS_SUCCESS)
		return SB_ERR(SB_FailedCopyConf, QVariantList() << m_Name << (quint32)status, status);

	// check if such a box already exists
	if (!SameName)
	{
		SbieIniGet(NewName, "", CONF_GET_NO_EXPAND, &status);
		if (status != STATUS_RESOURCE_NAME_NOT_FOUND)
			return SB_ERR(SB_AlreadyExists, QVariantList() << NewName);
	}

	// if the name is the same we first delete than write, 
	// else we first write and than delete, for safety reasons
	if (deleteOld && SameName)
		goto do_delete;

do_write:
	// Apply all Settings
	for (QList<SbieIniValue>::iterator I = Settings.begin(); I != Settings.end(); ++I)
	{
		SB_STATUS Status = SbieIniSet(NewName, I->Name, I->Value, eIniInsert, true);
		if (Status.IsError())
			return Status;
	}

do_delete:
	// Delete ini section
	if (deleteOld)
	{
		SB_STATUS Status = SbieIniSet(m_Name, "*", "", eIniUpdate, true);
		if (Status.IsError())
			return SB_ERR(SB_DeleteFailed, QVariantList() << m_Name << (quint32)Status.GetStatus(), Status.GetStatus());
		deleteOld = false;

		if (SameName)
			goto do_write;
	}

	if (m_RefreshOnChange)
		CommitIniChanges();

	m_Name = NewName;

	return SB_OK;
}

SB_STATUS CSbieIni::RemoveSection()
{
	return SbieIniSet(m_Name, "*", "", eIniUpdate, m_RefreshOnChange);
}

void CSbieIni::CommitIniChanges()
{
	m_pAPI->CommitIniChanges();
}

SB_STATUS CSbieIni::SbieIniSet(const QString& Section, const QString& Setting, const QString& Value, ESetMode Mode, bool bRefresh)
{
	if(m_IsVirtual)
		return m_pAPI->SbieIniSetDrv(Section, Setting, Value, Mode);
	return m_pAPI->SbieIniSet(Section, Setting, Value, Mode, bRefresh);
}

QString CSbieIni::SbieIniGet(const QString& Section, const QString& Setting, quint32 Index, qint32* ErrCode, quint32* pType) const
{
	return m_pAPI->SbieIniGet(Section, Setting, Index, ErrCode, pType);
}

QString CSbieIni::SbieIniGetEx(const QString& Section, const QString& Setting) const
{
	return m_pAPI->SbieIniGetEx(Section, Setting);
}