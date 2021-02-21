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
#pragma once

#include <QThread>

#include "qsbieapi_global.h"

#include "SbieStatus.h"

#define MONITOR_APICALL				0x000A

/*

 // Log Event
#define TRACE_LOG_SYSCALL			0x00000001
#define TRACE_LOG_PIPE              0x00000002
#define TRACE_LOG_IPC               0x00000003
#define TRACE_LOG_WINCLASS          0x00000004
#define TRACE_LOG_DRIVE             0x00000005
#define TRACE_LOG_COMCLASS          0x00000006
#define TRACE_LOG_IGNORE            0x00000007
#define TRACE_LOG_IMAGE             0x00000008
#define TRACE_LOG_FILE              0x00000009
#define TRACE_LOG_KEY				0x0000000A
#define TRACE_LOG_OTHER1			0x0000000B
#define TRACE_LOG_OTHER2			0x0000000C
#define TRACE_LOG_OTHER3			0x0000000D
#define TRACE_LOG_OTHER4			0x0000000E
#define TRACE_LOG_APICALL			0x0000000F // needs the logapi.dll
#define TRACE_LOG_EVENT_MASK		0x000000FF

// Event States
#define TRACE_LOG_ALLOWED           0x00000100
#define TRACE_LOG_DENIED            0x00000200
#define TRACE_LOG_STATE_MASK		0x00000F00

// Event Results
#define TRACE_LOG_SUCCESS           0x00001000
#define TRACE_LOG_FAILED            0x00002000
#define TRACE_LOG_RESULT_MASK		0x0000F000

// Reserved
#define TRACE_LOG_RESERVED_MASK		0x00FFFF00

// Event Presets
#define TRACE_LOG_OPEN              0x01000000
#define TRACE_LOG_CLOSED            0x02000000
#define TRACE_LOG_READONLY          0x03000000
#define TRACE_LOG_HIDDEN            0x04000000
#define TRACE_LOG_REDIRECTED        0x05000000
#define TRACE_LOG_TYPE_MASK			0x0F000000

// Event Sources
#define TRACE_LOG_DLL				0x10000000
#define TRACE_LOG_DRV				0x20000000
#define TRACE_LOG_SVC				0x30000000
#define TRACE_LOG_TRACE             0x40000000
#define TRACE_LOG_SOURCE_MASK		0xF0000000

*/


class QSBIEAPI_EXPORT CTraceEntry : public QSharedData
{
public:
	CTraceEntry(quint32 ProcessId, quint32 ThreadId, quint32 Type, const QString& Message);

	virtual QString		GetMessage() const { return m_Message; }
	virtual quint32		GetProcessId() const { return m_ProcessId; }
	virtual quint32		GetThreadId() const { return m_ThreadId; }
	virtual QDateTime	GetTimeStamp() const { return m_TimeStamp; }

	virtual quint16		GetType() const { return m_Type.Flags; }
	virtual QString		GetTypeStr() const;
	virtual QString		GetStautsStr() const;

	virtual int			GetCount() const { return m_Counter; }

	virtual bool		Equals(const QSharedDataPointer<CTraceEntry>& pOther) const {
			return pOther->m_ProcessId == this->m_ProcessId && pOther->m_ThreadId == this->m_ThreadId
			&& pOther->m_Message == this->m_Message;
	}
	virtual void		Merge(const QSharedDataPointer<CTraceEntry>& pOther) {
		m_Counter++; this->m_Type.Flags |= pOther->m_Type.Flags;
	}

	quint64				GetUID() const { return m_uid; }

protected:
	QString m_Message;
	quint32 m_ProcessId;
	quint32 m_ThreadId;
	QDateTime m_TimeStamp;

	union
	{
		quint16 Flags;
		struct
		{
			quint16
				Type : 12,
				Open : 1,
				Deny : 1,
				User : 1,
				Trace : 1;
		};
	} m_Type;

	quint64 m_uid;

	int m_Counter;
};

typedef QSharedDataPointer<CTraceEntry> CTraceEntryPtr;
