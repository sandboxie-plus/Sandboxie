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
#include <qobject.h>

#include "../qsbieapi_global.h"

#include "../SbieStatus.h"

class QSBIEAPI_EXPORT CBoxedProcess : public QObject
{
	Q_OBJECT
public:
	CBoxedProcess(quint64 ProcessId, class CSandBox* pBox);
	virtual ~CBoxedProcess();

	virtual void			InitProcessInfo();

	virtual quint64			GetProcessId() const { return m_ProcessId; }
	virtual quint64			GetParendPID() const  { return m_ParendPID; }
	virtual QString			GetProcessName() const  { return m_ImageName; }
	virtual QString			GetFileName() const { return m_ImagePath; }
	virtual QDateTime		GetTimeStamp() const { return m_StartTime; }
	virtual QString			GetStatusStr() const;

	virtual SB_STATUS		Terminate();
	virtual bool			IsTerminated() const  { return m_bTerminated; }
	virtual void			SetTerminated() { m_bTerminated = true; }

	virtual SB_STATUS		SetSuspend(bool bSet);
	virtual bool			IsSuspended() const;

	virtual QString			GetBoxName() const;

protected:
	friend class CSbieAPI;

	quint64			m_ProcessId;
	quint64			m_ParendPID;
	QString			m_ImageName;
	QString			m_ImagePath;
	quint32			m_SessionId;
	QDateTime		m_StartTime;
	bool			m_bTerminated;
	bool			m_bSuspended;

	class CSandBox*	m_pBox;

//private:
//	struct SBoxedProcess* m;
};

typedef QSharedPointer<CBoxedProcess> CBoxedProcessPtr;
typedef QWeakPointer<CBoxedProcess> CBoxedProcessRef;