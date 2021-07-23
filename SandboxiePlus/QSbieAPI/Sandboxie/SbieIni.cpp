/*
 *
 * Copyright (c) 2020, David Xanatos
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

#include <ntstatus.h>
#define WIN32_NO_STATUS
typedef long NTSTATUS;

#include "..\..\Sandboxie\core\drv\api_flags.h"

CSbieIni::CSbieIni(const QString& Section, class CSbieAPI* pAPI, QObject* parent) : QObject(parent)
{
	m_Name = Section;
	m_pAPI = pAPI;
}

CSbieIni::~CSbieIni()
{
}

SB_STATUS CSbieIni::SetText(const QString& Setting, const QString& Value)
{
	if (GetText(Setting) == Value)
		return SB_OK;
	return m_pAPI->SbieIniSet(m_Name, Setting, Value);
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

QString CSbieIni::GetText(const QString& Setting, const QString& Default) const
{
	int flags = (m_Name.isEmpty() ? 0 : CONF_GET_NO_GLOBAL) | CONF_GET_NO_EXPAND;
	QString Value = m_pAPI->SbieIniGet(m_Name, Setting, flags);
	if (Value.isNull()) Value = Default;
	return Value;
}

int CSbieIni::GetNum(const QString& Setting, int Default) const
{
	QString StrValue = GetText(Setting);
	bool ok;
	int Value = StrValue.toInt(&ok);
	if (!ok) return Default;
	return Value;
}

__int64 CSbieIni::GetNum64(const QString& Setting, __int64 Default) const
{
	QString StrValue = GetText(Setting);
	bool ok;
	__int64 Value = StrValue.toULongLong(&ok);
	if (!ok) return Default;
	return Value;
}

bool CSbieIni::GetBool(const QString& Setting, bool Default) const
{
	QString StrValue = GetText(Setting);
	if (StrValue.compare("y", Qt::CaseInsensitive) == 0)
		return true;
	if (StrValue.compare("n", Qt::CaseInsensitive) == 0)
		return false;
	return Default;
}

QStringList CSbieIni::GetTextList(const QString &Setting, bool withTemplates, bool expand) const
{
	QStringList TextList;

	int flags = (m_Name.isEmpty() ? 0 : CONF_GET_NO_GLOBAL);
	if (!withTemplates)
		flags |= CONF_GET_NO_TEMPLS;
	if (!expand)
		flags |= CONF_GET_NO_EXPAND;

	for (int index = 0; ; index++)
	{
		QString Value = m_pAPI->SbieIniGet(m_Name, Setting, index | flags);
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
		InsertText(Setting, Value);
	return SB_OK;
}

QStringList CSbieIni::GetTemplates() const
{
	QStringList Templates;

	for (int tmpl_index = 0; ; tmpl_index++)
	{
		QString TmplName = m_pAPI->SbieIniGet(m_Name, "Template", tmpl_index | CONF_GET_NO_TEMPLS);
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
		QString Value = m_pAPI->SbieIniGet("Template_" + Template, Setting, index | CONF_GET_NO_GLOBAL | CONF_GET_NO_EXPAND);
		if (Value.isNull())
			break;
		TextList.append(Value);
	}

	return TextList;
}

SB_STATUS CSbieIni::InsertText(const QString& Setting, const QString& Value)
{
	return m_pAPI->SbieIniSet(m_Name, Setting, Value, CSbieAPI::eIniInsert);
}

SB_STATUS CSbieIni::AppendText(const QString& Setting, const QString& Value)
{
	return m_pAPI->SbieIniSet(m_Name, Setting, Value, CSbieAPI::eIniAppend);
}

SB_STATUS CSbieIni::DelValue(const QString& Setting, const QString& Value)
{
	return m_pAPI->SbieIniSet(m_Name, Setting, Value, CSbieAPI::eIniDelete);
}

QList<QPair<QString, QString>> CSbieIni::GetIniSection(qint32* pStatus, bool withTemplates) const
{
	qint32 status = STATUS_SUCCESS;

	int flags = CONF_GET_NO_EXPAND;
	if (!withTemplates)
		flags |= CONF_GET_NO_TEMPLS;

	QList<QPair<QString, QString>> Settings;
	for (int setting_index = 0; ; setting_index++)
	{
		QString setting_name = m_pAPI->SbieIniGet(m_Name, "", setting_index | flags, &status);
		if (status == STATUS_RESOURCE_NAME_NOT_FOUND) {
			status = STATUS_SUCCESS;
			break;
		}
		if (status != STATUS_SUCCESS)
			break;

		for (int value_index = 0; ; value_index++)
		{
			QString setting_value = m_pAPI->SbieIniGet(m_Name, setting_name, value_index | CONF_GET_NO_GLOBAL | flags, &status);
			if (status == STATUS_RESOURCE_NAME_NOT_FOUND) {
				status = STATUS_SUCCESS;
				break;
			}
			if (status != STATUS_SUCCESS)
				break;

			Settings.append(qMakePair(setting_name, setting_value));
		}

		if (status != STATUS_SUCCESS)
			break;
	}

	if (pStatus) *pStatus = status;
	return Settings;
}

SB_STATUS CSbieIni::RenameSection( const QString& NewName, bool deleteOld) // Note: deleteOld is used when duplicating a box
{
	qint32 status = STATUS_SUCCESS;

	if (m_Name.isEmpty() || NewName.isEmpty())
		return SB_ERR();
	bool SameName = (bool)(NewName.compare(m_Name, Qt::CaseInsensitive) == 0);

	// Get all Settigns
	QList<QPair<QString, QString>> Settings = GetIniSection(&status);
	if (status != STATUS_SUCCESS)
		return SB_ERR(SB_FailedCopyConf, QVariantList() << m_Name << (quint32)status, status);

	// check if such a box already exists
	if (!SameName)
	{
		m_pAPI->SbieIniGet(NewName, "", CONF_GET_NO_EXPAND, &status);
		if (status != STATUS_RESOURCE_NAME_NOT_FOUND)
			return SB_ERR(SB_AlreadyExists, QVariantList() << NewName);
	}

	// if the name is the same we first delete than write, 
	// else we first write and than delete, fro safety reasons
	if (deleteOld && SameName)
		goto do_delete;

do_write:
	// Apply all Settigns
	for (QList<QPair<QString, QString>>::iterator I = Settings.begin(); I != Settings.end(); ++I)
	{
		SB_STATUS Status = m_pAPI->SbieIniSet(NewName, I->first, I->second, CSbieAPI::eIniInsert);
		if (Status.IsError())
			return Status;
	}

do_delete:
	// Selete ini section
	if (deleteOld)
	{
		SB_STATUS Status = m_pAPI->SbieIniSet(m_Name, "*", "");
		if (Status.IsError())
			return SB_ERR(SB_DeleteFailed, QVariantList() << m_Name << (quint32)Status.GetStatus(), Status.GetStatus());
		deleteOld = false;

		if (SameName)
			goto do_write;
	}

	return SB_OK;
}

SB_STATUS CSbieIni::RemoveSection()
{
	return m_pAPI->SbieIniSet(m_Name, "*", "");
}