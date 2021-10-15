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


class QSBIEAPI_EXPORT CTraceEntry : public QSharedData
{
public:
	CTraceEntry(quint32 ProcessId, quint32 ThreadId, quint32 Type, const QString& Message);

	virtual QString		GetMessage() const { return m_Message; }
	virtual quint32		GetProcessId() const { return m_ProcessId; }
	virtual quint32		GetThreadId() const { return m_ThreadId; }
	virtual QDateTime	GetTimeStamp() const { return m_TimeStamp; }

	virtual quint16		GetType() const { return m_Type.Flags; }
	static QString		GetTypeStr(quint32 Type);
	virtual QString		GetTypeStr() const;
	virtual QString		GetStautsStr() const;

	virtual void		SetProcessName(const QString& name) { m_ProcessName = name; }
	virtual QString		GetProcessName() const { return m_ProcessName; }

	virtual void		SetBoxPtr(void* ptr) { m_BoxPtr = ptr; }
	virtual void*		GetBoxPtr() const { return m_BoxPtr; }

	virtual int			GetCount() const { return m_Counter; }

	virtual bool		Equals(const QSharedDataPointer<CTraceEntry>& pOther) const {
			return pOther->m_ProcessId == this->m_ProcessId && pOther->m_ThreadId == this->m_ThreadId
			&& pOther->m_Message == this->m_Message;
	}
	virtual void		Merge(const QSharedDataPointer<CTraceEntry>& pOther) {
		m_Counter++; this->m_Type.Flags |= pOther->m_Type.Flags;
	}

	virtual bool		IsOpen() const;
	virtual bool		IsClosed() const;
	virtual bool		IsTrace() const;

	quint64				GetUID() const { return m_uid; }

protected:
	QString m_Message;
	quint32 m_ProcessId;
	quint32 m_ThreadId;
	QDateTime m_TimeStamp;
	QString m_ProcessName;
	void* m_BoxPtr;

	union
	{
		quint32 Flags;
		struct
		{
			quint32
				Type : 8,

				SubType : 8,

				Disposition : 4,
				Allowed : 1,
				Denided : 1,
				Success : 1,
				Failed : 1,

				Reserved : 6,
				Trace : 1,
				User : 1;
		};
	} m_Type;

	quint64 m_uid;

	int m_Counter;
};

typedef QSharedDataPointer<CTraceEntry> CTraceEntryPtr;
