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
	QString Value = m_pAPI->SbieIniGet(m_Name, Setting, CONF_GET_NO_GLOBAL | CONF_GET_NO_EXPAND);
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