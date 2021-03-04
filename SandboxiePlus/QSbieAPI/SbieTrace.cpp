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
#include <QDebug>
#include <QStandardPaths>
#include "SbieTrace.h"

#include <ntstatus.h>
#define WIN32_NO_STATUS
typedef long NTSTATUS;

#include <windows.h>
#include "SbieDefs.h"

#include "..\..\Sandboxie\common\win32_ntddk.h"

#include "..\..\Sandboxie\core\drv\api_defs.h"

#include "..\..\Sandboxie\core\svc\msgids.h"
#include "..\..\Sandboxie\core\svc\ProcessWire.h"
#include "..\..\Sandboxie\core\svc\sbieiniwire.h"
#include "..\..\Sandboxie\core\svc\QueueWire.h"
#include "..\..\Sandboxie\core\svc\InteractiveWire.h"



///////////////////////////////////////////////////////////////////////////////
// 
//

QString ErrorString(qint32 err)
{
	QString Error;
	HMODULE handle = NULL; //err < 0 ? GetModuleHandle(L"NTDLL.DLL") : NULL;
	DWORD flags = 0; //err < 0 ? FORMAT_MESSAGE_FROM_HMODULE : 0;
	LPTSTR s;
	if (::FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | flags, handle, err, 0, (LPTSTR)&s, 0, NULL) > 0)
	{
		LPTSTR p = wcschr(s, L'\r');
		if (p != NULL) *p = L'\0';
		Error = QString::fromWCharArray(s);
		::LocalFree(s);
	}
	return Error;
}

CTraceEntry::CTraceEntry(quint32 ProcessId, quint32 ThreadId, quint32 Type, const QString& Message)
{
	m_ProcessId = ProcessId;
	m_ThreadId = ThreadId;
	m_Message = Message;
	m_Type.Flags = Type;

	m_TimeStamp = QDateTime::currentDateTime(); // ms resolution

	static atomic<quint64> uid = 0;
	m_uid = uid.fetch_add(1);
	
	m_Counter = 0;

	m_Message = m_Message.replace("\r", "").replace("\n", " ");

	// if this is a set error, then get the actual error string
	if (m_Type.Type == MONITOR_OTHER && Message.indexOf("SetError:") == 0)
	{
		auto tmp = Message.split(":");
		if (tmp.length() >= 2)
		{
			QString temp = tmp[1].trimmed();
			int endPos = temp.indexOf(QRegExp("[ \r\n]"));
			if (endPos != -1)
				temp.truncate(endPos);

			qint32 errCode = temp.toInt();
			QString Error = ErrorString(errCode);
			if (!Error.isEmpty())
				m_Message += " (" + Error + ")";
		}
	}
}

QString CTraceEntry::GetTypeStr() const
{
	QString Type;
	switch (m_Type.Type)
	{
	case MONITOR_APICALL:		Type = "ApiCall"; break;
	case MONITOR_SYSCALL:		Type = "SysCall"; break;
	case MONITOR_PIPE:			Type = "Pipe"; break;
	case MONITOR_IPC:			Type = "Ipc"; break;
	case MONITOR_WINCLASS:		Type = "WinClass"; break;
	case MONITOR_DRIVE:			Type = "Drive"; break;
	case MONITOR_COMCLASS:		Type = "ComClass"; break;
	case MONITOR_IGNORE:		Type = "Ignore"; break;
	case MONITOR_IMAGE:			Type = "Image"; break;
	case MONITOR_FILE:			Type = "File"; break;
	case MONITOR_KEY:			Type = "Key"; break;
	case MONITOR_OTHER:			Type = "Debug"; break;
	default:					Type = "Unknown: " + QString::number(m_Type.Type);
	}

	//if (!m_Type.User)
	//	Type.append(" (drv)");

	return Type;
}

QString CTraceEntry::GetStautsStr() const
{
	QString Status;
	if ((m_Type.Flags & MONITOR_DISPOSITION_MASK) == MONITOR_OPEN)
		Status.append("Open ");
	if ((m_Type.Flags & MONITOR_DISPOSITION_MASK) == MONITOR_DENY)
		Status.append("Closed ");

	if (m_Type.Trace)
		Status.append("Trace ");

	if (m_Counter > 1)
		Status.append(QString("(%1) ").arg(m_Counter));

	if (m_Type.User)
		Status = Status.toLower();

	return Status;
}

///////////////////////////////////////////////////////////////////////////////
// 
//

QString GetLastErrorAsString()
{
	DWORD errorMessageID = ::GetLastError();
	if (errorMessageID == 0)
		return QString();

	char* messageBuffer = NULL;
	FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

	QString message(messageBuffer);
	LocalFree(messageBuffer);
	return message;
}