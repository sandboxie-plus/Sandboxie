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
#include "IniSection.h"
#include "../SbieAPI.h"

#include <ntstatus.h>
#define WIN32_NO_STATUS
typedef long NTSTATUS;

#include "..\..\Sandboxie\core\drv\api_flags.h"

CIniSection::CIniSection(const QString& Section, class CSbieAPI* pAPI, QObject* parent) : QObject(parent)
{
	m_Name = Section;
	m_pAPI = pAPI;
}

CIniSection::~CIniSection()
{
}

SB_STATUS CIniSection::SetText(const QString& Setting, const QString& Value)
{
	if (GetText(Setting) == Value)
		return SB_OK;
	return m_pAPI->SbieIniSet(m_Name, Setting, Value);
}

SB_STATUS CIniSection::SetNum(const QString& Setting, int Value)
{
	return SetText(Setting, QString::number(Value));
}

SB_STATUS CIniSection::SetNum64(const QString& Setting, __int64 Value)
{
	return SetText(Setting, QString::number(Value));
}

SB_STATUS CIniSection::SetBool(const QString& Setting, bool Value)
{
	return SetText(Setting, Value ? "y" : "n");
}

QString CIniSection::GetText(const QString& Setting, const QString& Default) const
{
	int flags = (m_Name.isEmpty() ? 0 : CONF_GET_NO_GLOBAL) | CONF_GET_NO_EXPAND;
	QString Value = m_pAPI->SbieIniGet(m_Name, Setting, flags);
	if (Value.isNull()) Value = Default;
	return Value;
}

int CIniSection::GetNum(const QString& Setting, int Default) const
{
	QString StrValue = GetText(Setting);
	bool ok;
	int Value = StrValue.toInt(&ok);
	if (!ok) return Default;
	return Value;
}

__int64 CIniSection::GetNum64(const QString& Setting, __int64 Default) const
{
	QString StrValue = GetText(Setting);
	bool ok;
	__int64 Value = StrValue.toULongLong(&ok);
	if (!ok) return Default;
	return Value;
}

bool CIniSection::GetBool(const QString& Setting, bool Default) const
{
	QString StrValue = GetText(Setting);
	if (StrValue.compare("y", Qt::CaseInsensitive) == 0)
		return true;
	if (StrValue.compare("n", Qt::CaseInsensitive) == 0)
		return false;
	return Default;
}

QStringList CIniSection::GetTextList(const QString &Setting, bool withBrackets)
{
	QStringList TextList;

	int flags = (m_Name.isEmpty() ? 0 : CONF_GET_NO_GLOBAL) | CONF_GET_NO_EXPAND;
	if (withBrackets)
		flags |= CONF_GET_NO_TEMPLS;

	for(int index = 0; ; index++)
	{
		QString Value = m_pAPI->SbieIniGet(m_Name, Setting, index | flags);
		if (Value.isNull())
			break;
		TextList.append(Value);
	}

	return TextList;
}

SB_STATUS CIniSection::InsertText(const QString& Setting, const QString& Value)
{
	return m_pAPI->SbieIniSet(m_Name, Setting, Value, CSbieAPI::eIniInsert);
}

SB_STATUS CIniSection::AppendText(const QString& Setting, const QString& Value)
{
	return m_pAPI->SbieIniSet(m_Name, Setting, Value, CSbieAPI::eIniAppend);
}

SB_STATUS CIniSection::DelValue(const QString& Setting, const QString& Value)
{
	return m_pAPI->SbieIniSet(m_Name, Setting, Value, CSbieAPI::eIniDelete);
}


SB_STATUS CIniSection::RenameSection( const QString& NewName, bool deleteOld) // Note: deleteOld is used when duplicating a box
{
	if (m_Name.isEmpty() || NewName.isEmpty())
		return SB_ERR();
	bool SameName = (bool)(NewName.compare(m_Name, Qt::CaseInsensitive) == 0);

	qint32 status = STATUS_SUCCESS;

	// Get all Settigns
	QList<QPair<QString, QString>> Settings;
	for (int setting_index = 0; ; setting_index++)
	{
		QString setting_name = m_pAPI->SbieIniGet(m_Name, NULL, setting_index | CONF_GET_NO_TEMPLS | CONF_GET_NO_EXPAND, &status);
		if (status == STATUS_RESOURCE_NAME_NOT_FOUND) {
			status = STATUS_SUCCESS;
			break;
		}
		if (status != STATUS_SUCCESS)
			break;

		for (int value_index = 0; ; value_index++)
		{
			QString setting_value = m_pAPI->SbieIniGet(m_Name, setting_name, value_index | CONF_GET_NO_GLOBAL | CONF_GET_NO_TEMPLS | CONF_GET_NO_EXPAND, &status);
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

	if (status != STATUS_SUCCESS)
		return SB_ERR(CSbieAPI::tr("Failed to copy configuration from sandbox %1: %2").arg(m_Name).arg(status, 8, 16), status);

	// check if such a box already exists
	if (!SameName)
	{
		m_pAPI->SbieIniGet(NewName, NULL, CONF_GET_NO_EXPAND, &status);
		if (status != STATUS_RESOURCE_NAME_NOT_FOUND)
			return SB_ERR(CSbieAPI::tr("A sandbox of the name %1 already exists").arg(NewName));
	}

	// if the name is the same we first delete than write, 
	// else we first write and than delete, fro safety reasons
	if (deleteOld && SameName)
		goto do_delete;

do_write:
	// Apply all Settigns
	for (QList<QPair<QString, QString>>::iterator I = Settings.begin(); I != Settings.end(); ++I)
	{
		SB_STATUS Status = m_pAPI->SbieIniSet(NewName, I->first, I->second);
		if (Status.IsError())
			return Status;
	}

do_delete:
	// Selete ini section
	if (deleteOld)
	{
		SB_STATUS Status = m_pAPI->SbieIniSet(m_Name, "*", "");
		if (Status.IsError())
			return SB_ERR(CSbieAPI::tr("Failed to delete sandbox %1: %2").arg(m_Name).arg(Status.GetStatus(), 8, 16), Status.GetStatus());
		deleteOld = false;

		if (SameName)
			goto do_write;
	}

	return SB_OK;
}

SB_STATUS CIniSection::RemoveSection()
{
	return m_pAPI->SbieIniSet(m_Name, "*", "");
}