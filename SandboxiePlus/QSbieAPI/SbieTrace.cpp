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
#include "..\..\Sandboxie\common\defines.h"

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
	LPWSTR s;
	if (::FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | flags, handle, err, 0, (LPTSTR)&s, 0, NULL) > 0)
	{
		LPWSTR p = wcschr(s, L'\r');
		if (p != NULL) *p = L'\0';
		Error = QString::fromWCharArray(s);
		::LocalFree(s);
	}
	return Error;
}

CTraceEntry::CTraceEntry(quint64 Timestamp, quint32 ProcessId, quint32 ThreadId, quint32 Type, const QStringList& LogData, const QVector<quint64>& Stack)
{
	m_ProcessId = ProcessId;
	m_ThreadId = ThreadId;
	m_Name = LogData.length() > 0 ? LogData.at(0) : QString("(empty)");
	m_Message = LogData.length() > 1 ? LogData.at(1) : QString();
	m_SubType = LogData.length() > 2 ? LogData.at(2) : QString();
	m_Type.Flags = Type;
	m_Stack = Stack;

	if (m_Type.Type == MONITOR_SYSCALL && !m_SubType.isEmpty()) {
		m_Message += ", name=" + m_SubType;
		m_SubType.clear();
	}
	/*if (m_Type.Type == MONITOR_HOOK)
	{
		int colon_pos = m_Message.lastIndexOf(':');
		if (colon_pos != -1) {
			m_Name = m_Message.mid(colon_pos + 1).trimmed();
			m_Message = m_Message.left(colon_pos);
		}
	}*/

	m_TimeStamp = Timestamp ? Timestamp : QDateTime::currentDateTime().toMSecsSinceEpoch();

	m_BoxPtr = 0;

	static std::atomic<quint64> uid = 0;
	m_uid = uid.fetch_add(1);
	
#ifdef USE_MERGE_TRACE
	m_Counter = 1;
#endif

	m_Message = m_Message.replace("\r", "").replace("\n", " ");

	// if this is a set error, then get the actual error string
	if (m_Type.Type == MONITOR_OTHER && m_Message.indexOf("SetError:") == 0)
	{
		auto tmp = m_Message.split(":");
		if (tmp.length() >= 2)
		{
			QString temp = tmp[1].trimmed();
			int endPos = temp.indexOf(QRegularExpression("[ \r\n]"));
			if (endPos != -1)
				temp.truncate(endPos);

			qint32 errCode = temp.toInt();
			QString Error = ErrorString(errCode);
			if (!Error.isEmpty())
				m_Message += " (" + Error + ")";
		}
	}
}

QList<quint32> CTraceEntry::AllTypes()
{
	return QList<quint32>() << MONITOR_APICALL << MONITOR_SYSCALL 
		<< MONITOR_KEY << MONITOR_FILE << MONITOR_PIPE 
		<< MONITOR_IPC << MONITOR_RPC << MONITOR_COMCLASS << MONITOR_RTCLASS
		<< MONITOR_WINCLASS << MONITOR_DRIVE  << MONITOR_IGNORE << MONITOR_IMAGE 
		<< MONITOR_NETFW << MONITOR_DNS << MONITOR_SCM << MONITOR_HOOK << MONITOR_OTHER;
}

QString CTraceEntry::GetTypeStr(quint32 Type)
{
	switch (Type)
	{
	case MONITOR_APICALL:		return "ApiCall"; break;
	case MONITOR_SYSCALL:		return "SysCall"; break;
	case MONITOR_PIPE:			return "Pipe"; break;
	case MONITOR_IPC:			return "Ipc"; break;
	case MONITOR_RPC:			return "Rpc"; break;
	case MONITOR_WINCLASS:		return "WinClass"; break;
	case MONITOR_DRIVE:			return "Drive"; break;
	case MONITOR_COMCLASS:		return "ComClass"; break;
	case MONITOR_RTCLASS:		return "RtClass"; break;
	case MONITOR_IGNORE:		return "Ignore"; break;
	case MONITOR_IMAGE:			return "Image"; break;
	case MONITOR_FILE:			return "File"; break;
	case MONITOR_KEY:			return "Key"; break;
	case MONITOR_NETFW:			return "Socket"; break;
	case MONITOR_DNS:			return "Dns"; break;
	case MONITOR_SCM:			return "SCM"; break; // Service Control Manager
	case MONITOR_HOOK:			return "Hook"; break;
	case MONITOR_OTHER:			return "Debug"; break;
	default:					return QString();
	}
}

QString CTraceEntry::GetTypeStr() const
{
	QString Type = GetTypeStr(m_Type.Type);
	if(Type.isEmpty())
		Type = "Unknown: " + QString::number(m_Type.Type);

	if(!m_SubType.isEmpty())
		Type.append(" / " + m_SubType);

	if (m_Type.User)
		Type.append(" (U)"); // user mode (sbiedll.dll)
	//else if (m_Type.Agent)
	//	Type.append(" (S)"); // system mode (sbiesvc.exe)
	else
		Type.append(" (K)"); // kernel mode (sbiedrv.sys)

	return Type;
}

bool CTraceEntry::IsOpen() const 
{
	return (m_Type.Flags & MONITOR_DISPOSITION_MASK) == MONITOR_OPEN;
}

bool CTraceEntry::IsClosed() const
{
	return (m_Type.Flags & MONITOR_DISPOSITION_MASK) == MONITOR_DENY;
}

bool CTraceEntry::IsTrace() const
{
	return m_Type.Trace;
}

QString CTraceEntry::GetStautsStr() const
{
	QString Status;
	if (IsOpen())
		Status.append("Open ");
	if (IsClosed())
		Status.append("Closed ");

	if (IsTrace())
		Status.append("Trace ");

#ifdef USE_MERGE_TRACE
	if (m_Counter > 1)
		Status.append(QString("(%1) ").arg(m_Counter));
#endif

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