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

#include "SbieError.h"

#include "./Sandboxie/SandBox.h"
#include "./Sandboxie/BoxedProcess.h"

class CResLogEntry : public QSharedData
{
public:
	CResLogEntry(quint64 ProcessId, quint32 Type, const QString& Value);

	quint64				GetProcessId() const { return m_ProcessId; }
	QDateTime			GetTimeStamp() const { return m_TimeStamp; }
	QString				GetType() const { return m_Type; }
	QString				GetValue() const { return m_Name; }

	quint64				GetUID() const { return m_uid; }

protected:
	QString m_Name;
	QString m_Type;
	quint64 m_ProcessId;
	QDateTime m_TimeStamp;
	bool m_Open;
	bool m_Deny;
	//bool m_Verbose;
	//bool m_User;

	quint64 m_uid;
};

typedef QSharedDataPointer<CResLogEntry> CResLogEntryPtr;


class QSBIEAPI_EXPORT CSbieAPI : public QThread
{
	Q_OBJECT
public:
	CSbieAPI(QObject* parent = 0);
	virtual ~CSbieAPI();

	virtual bool			IsValid() const;

	virtual QString			GetVersion();

	virtual SB_STATUS		TakeOver();

	virtual QString			GetSbiePath() const { return m_SbiePath; }
	virtual QString			GetIniPath() const { return m_IniPath; }

	virtual void			UpdateDriveLetters();
	virtual QString			Nt2DosPath(QString NtPath) const;

	virtual SB_STATUS		ReloadBoxes();
	virtual SB_STATUS		CreateBox(const QString& BoxName);

	virtual SB_STATUS		UpdateProcesses(bool bKeep);
	virtual SB_STATUS		UpdateProcesses(bool bKeep, const CSandBoxPtr& pBox);

	virtual QMap<QString, CSandBoxPtr> GetAllBoxes() { return m_SandBoxes; }

	virtual SB_STATUS		TerminateAll();

	enum ESetMode
	{
		eIniUpdate = 0,
		eIniAppend,
		eIniInsert,
		eIniDelete
	};

	// Config
	virtual SB_STATUS		ReloadConfig(quint32 SessionId = -1);
	virtual QString			SbieIniGet(const QString& Section, const QString& Setting, quint32 Index = 0, qint32* ErrCode = NULL);
	virtual SB_STATUS		SbieIniSet(const QString& Section, const QString& Setting, const QString& Value, ESetMode Mode = eIniUpdate);
	virtual bool			IsBoxEnabled(const QString& BoxName);

	// Monitor
	virtual SB_STATUS		EnableMonitor(bool Enable);
	virtual bool			IsMonitoring();

	virtual QList<CResLogEntryPtr> GetResLog() const { QReadLocker Lock(&m_ResLogMutex); return m_ResLogList; }

signals:
	void					LogMessage(const QString& Message);

private slots:
	//virtual void			OnMonitorEntry(quint64 ProcessId, quint32 Type, const QString& Value);

protected:
	friend class CSandBox;
	friend class CBoxedProcess;

	virtual QString			GetSbieHome() const;

	virtual SB_STATUS		RunStart(const QString& BoxName, const QString& Command);

	virtual SB_STATUS		CleanBox(const QString& BoxName);
	virtual SB_STATUS		RenameBox(const QString& OldName, const QString& NewName, bool deleteOld = true);
	virtual SB_STATUS		RemoveBox(const QString& BoxName);

	virtual bool			GetLog();
	virtual bool			GetMonitor();

	virtual SB_STATUS		TerminateAll(const QString& BoxName);
	virtual SB_STATUS		Terminate(quint64 ProcessId);

	virtual SB_STATUS		RunSandboxed(const QString& BoxName, const QString& Command, QString WrkDir = QString(), quint32 Flags = 0);

	virtual SB_STATUS		SetBoxPaths(const CSandBoxPtr& pSandBox);
	virtual SB_STATUS		SetProcessInfo(const CBoxedProcessPtr& pProcess);

	virtual QString			GetDeviceMap();
	virtual QByteArray		MakeEnvironment(bool AddDeviceMap);

	virtual void			run();

	QMap<QString, CSandBoxPtr> m_SandBoxes;
	QMap<quint64, CBoxedProcessPtr> m_BoxedProxesses;

	mutable QReadWriteLock	m_ResLogMutex;
	QList<CResLogEntryPtr>	m_ResLogList;

	QMap<QString, QString>	m_DriveLetters;

	QString					m_SbiePath;
	QString					m_IniPath;

	bool					m_bTerminate;
private:
	struct SSbieAPI* m;
};
