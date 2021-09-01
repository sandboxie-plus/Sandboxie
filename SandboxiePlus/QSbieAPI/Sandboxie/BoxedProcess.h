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
	CBoxedProcess(quint32 ProcessId, class CSandBox* pBox);
	virtual ~CBoxedProcess();

	virtual bool			InitProcessInfo();
	virtual bool			InitProcessInfoEx();

	virtual quint32			GetProcessId() const { return m_ProcessId; }
	virtual quint32			GetParendPID() const  { return m_ParendPID; }
	virtual QString			GetProcessName() const  { return m_ImageName; }
	virtual QString			GetCommandLine() const  { return m_CommandLine; }
	virtual QString			GetFileName() const { return m_ImagePath; }
	virtual QDateTime		GetTimeStamp() const { return m_StartTime; }
	virtual quint32			GetImageType() const { return m_ImageType; }

	virtual SB_STATUS		Terminate();
	virtual bool			IsTerminated(quint64 forMs = 0) const;
	virtual void			SetTerminated();

	//virtual SB_STATUS		SetSuspend(bool bSet);
	//virtual bool			IsSuspended() const;

	virtual bool			IsWoW64() const { return m_bIsWoW64; }

	virtual QString			GetBoxName() const { return m_BoxName; }
	virtual class CSandBox* GetBoxPtr() const { return m_pBox; }

protected:
	friend class CSbieAPI;

	quint32			m_ProcessId;
	QString			m_BoxName;
	quint32			m_ParendPID;
	QString			m_ImageName;
	QString			m_ImagePath;
	quint32			m_ImageType;
	QString			m_CommandLine;
	quint32			m_SessionId;
	QDateTime		m_StartTime;
	quint64			m_uTerminated;
	//bool			m_bSuspended;
	bool			m_bIsWoW64;

	class CSandBox*	m_pBox;

//private:
//	struct SBoxedProcess* m;
};

typedef QSharedPointer<CBoxedProcess> CBoxedProcessPtr;
typedef QWeakPointer<CBoxedProcess> CBoxedProcessRef;