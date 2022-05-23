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
#include <QFileSystemWatcher>

#include "qsbieapi_global.h"

#include "SbieStatus.h"

#include "SbieTrace.h"

#include "./Sandboxie/SandBox.h"
#include "./Sandboxie/BoxedProcess.h"


class QSBIEAPI_EXPORT CSbieAPI : public QThread
{
	Q_OBJECT
public:
	CSbieAPI(QObject* parent = 0);
	virtual ~CSbieAPI();

	static bool				IsSbieCtrlRunning();
	static bool				TerminateSbieCtrl();

	virtual SB_STATUS		Connect(bool takeOver, bool withQueue);
	virtual SB_STATUS		Disconnect();
	virtual bool			IsConnected() const;

	virtual QString			GetVersion();

	virtual SB_STATUS		TakeOver();
	virtual SB_STATUS		WatchIni(bool bEnable = true, bool bReLoad = true);

	virtual QString			GetSbiePath() const { return m_SbiePath; }
	virtual QString			GetIniPath() const { return m_IniPath; }

	virtual QString			ResolveAbsolutePath(const QString& Path);

	virtual void			UpdateDriveLetters();
	virtual QString			Nt2DosPath(QString NtPath, bool* pOk = NULL) const;

	virtual SB_STATUS		ReloadBoxes(bool bForceUpdate = false);
	static  SB_STATUS		ValidateName(const QString& BoxName);
	virtual SB_STATUS		CreateBox(const QString& BoxName, bool bReLoad = true);

	virtual SB_STATUS		UpdateProcesses(bool bKeep, bool bAllSessions);
	//virtual SB_STATUS		UpdateProcesses(bool bKeep, const CSandBoxPtr& pBox);

	virtual QMap<QString, CSandBoxPtr> GetAllBoxes() { return m_SandBoxes; }
	virtual QMap<quint32, CBoxedProcessPtr> GetAllProcesses() { return m_BoxedProxesses; }

	virtual CSandBoxPtr		GetBoxByProcessId(quint32 ProcessId) const;
	virtual CSandBoxPtr		GetBoxByName(const QString &BoxName) const { return m_SandBoxes.value(BoxName.toLower()); }
	virtual CBoxedProcessPtr GetProcessById(quint32 ProcessId) const;

	virtual SB_STATUS		TerminateAll();

	virtual SB_STATUS		SetProcessExemption(quint32 process_id, quint32 action_id, bool NewState);
	virtual bool			GetProcessExemption(quint32 process_id, quint32 action_id);

	virtual QString			GetBoxedPath(const QString& BoxName, const QString& Path);
	virtual QString			GetBoxedPath(CSandBox* pBox, const QString& Path);
	virtual QString			GetRealPath(CSandBox* pBox, const QString& Path);

	enum ESetMode
	{
		eIniUpdate = 0,
		eIniAppend,
		eIniInsert,
		eIniDelete
	};

	// Config
	virtual SB_STATUS		ReloadConfig(bool ReconfigureDrv = false);
	virtual SB_STATUS		ReloadCert();
	virtual void			CommitIniChanges();
	virtual QString			SbieIniGet(const QString& Section, const QString& Setting, quint32 Index = 0, qint32* ErrCode = NULL);
	virtual QString			SbieIniGetEx(const QString& Section, const QString& Setting);
	virtual SB_STATUS		SbieIniSet(const QString& Section, const QString& Setting, const QString& Value, ESetMode Mode = eIniUpdate, bool bRefresh = true);
	virtual bool			IsBox(const QString& BoxName, bool& bIsEnabled);
	virtual CSbieIni*		GetGlobalSettings() const { return m_pGlobalSection; }
	virtual CSbieIni*		GetUserSettings() const { return m_pUserSection; }
	virtual QString			GetCurrentUserName() const { return m_UserName; }
	virtual QString			GetCurrentUserSid() const { return m_UserSid; }
	virtual bool			IsConfigLocked();
	virtual SB_STATUS		UnlockConfig(const QString& Password);
	virtual SB_STATUS		LockConfig(const QString& NewPassword);
	virtual void			ClearPassword();

	enum EFeatureFlags
	{
		eSbieFeatureWFP			= 0x00000001,
		eSbieFeatureObCB		= 0x00000002,
		eSbieFeaturePMod		= 0x00000004,
		eSbieFeatureAppC		= 0x00000008,
		eSbieFeatureSbiL		= 0x00000010,
		eSbieFeatureCert		= 0x80000000
	};

	virtual quint32			GetFeatureFlags();
	virtual QString			GetFeatureStr();
	virtual quint64			GetCertState();

	// Forced Processes
	virtual SB_STATUS		DisableForceProcess(bool Set, int Seconds = 0);
	virtual bool			AreForceProcessDisabled();

	// Monitor
	virtual SB_STATUS		EnableMonitor(bool Enable);
	virtual bool			IsMonitoring();

	virtual void			AddTraceEntry(const CTraceEntryPtr& LogEntry, bool bCanMerge = false);
	virtual QVector<CTraceEntryPtr> GetTrace() const;
	virtual void			ClearTrace() { QWriteLocker Lock(&m_TraceMutex); m_TraceList.clear(); m_LastTraceEntry = 0; }

	// Other
	virtual QString			GetSbieMsgStr(quint32 code, quint32 Lang = 1033);

	virtual SB_STATUS		RunStart(const QString& BoxName, const QString& Command, bool Elevated = false, const QString& WorkingDir = QString(), QProcess* pProcess = NULL);
	virtual QString			GetStartPath() const;

	virtual quint32			GetSessionID() const;


	enum ESbieQueuedRequests
	{
		ePrintSpooler = -1,
		eInvalidQueuedRequests = 0,
		eFileMigration = 1,
		eInetBlockade= 2,
	};

public slots:
	virtual void			SendReplyData(quint32 RequestId, const QVariantMap& Result);

signals:
	void					StatusChanged();
	void					ConfigReloaded();
	//void					LogMessage(const QString& Message, bool bNotify = true);
	void					LogSbieMessage(quint32 MsgCode, const QStringList& MsgData, quint32 ProcessId);
	void					ProcessBoxed(quint32 ProcessId, const QString& Path, const QString& Box, quint32 ParentId);
	void					FileToRecover(const QString& BoxName, const QString& FilePath, const QString& BoxPath, quint32 ProcessId);
	void					BoxOpened(const QString& BoxName);
	void					BoxClosed(const QString& BoxName);
	void					NotAuthorized(bool bLoginRequired, bool &bRetry);
	void					QueuedRequest(quint32 ClientPid, quint32 ClientTid, quint32 RequestId, const QVariantMap& Data);

protected slots:
	//virtual void			OnMonitorEntry(quint32 ProcessId, quint32 Type, const QString& Value);
	virtual void			OnIniChanged(const QString &path);
	virtual void			OnReloadConfig();
	virtual CBoxedProcessPtr OnProcessBoxed(quint32 ProcessId, const QString& Path, const QString& Box, quint32 ParentId);

protected:
	friend class CSandBox;
	friend class CBoxedProcess;

	virtual SB_STATUS		ReloadConf(quint32 flags, quint32 SessionId = -1);

	virtual CSandBox*		NewSandBox(const QString& BoxName, class CSbieAPI* pAPI);
	virtual CBoxedProcess*	NewBoxedProcess(quint32 ProcessId, class CSandBox* pBox);

	virtual QString			GetSbieHome() const;
	virtual QString			GetIniPath(bool* IsHome) const;
	virtual QString			GetUserSection(QString* pUserName = NULL, bool* pIsAdmin = NULL) const;

	virtual bool			HasProcesses(const QString& BoxName);

	virtual bool			GetQueue();
	virtual bool			GetLog();
	virtual bool			GetMonitor();

	virtual quint32			QueryProcessInfo(quint32 ProcessId, quint32 InfoClass = 0);

	virtual SB_STATUS		TerminateAll(const QString& BoxName);
	virtual SB_STATUS		Terminate(quint32 ProcessId);

	virtual SB_STATUS		RunSandboxed(const QString& BoxName, const QString& Command, QString WrkDir = QString(), quint32 Flags = 0);

	virtual SB_STATUS		UpdateBoxPaths(const CSandBoxPtr& pSandBox);
	virtual SB_STATUS		UpdateProcessInfo(const CBoxedProcessPtr& pProcess);

	virtual void			GetUserPaths();

	virtual QString			GetDeviceMap();
	virtual QByteArray		MakeEnvironment(bool AddDeviceMap);

	virtual void			run();

	QMap<QString, CSandBoxPtr> m_SandBoxes;
	QMap<quint32, CBoxedProcessPtr> m_BoxedProxesses;

	mutable QReadWriteLock	m_TraceMutex;
	QVector<CTraceEntryPtr>	m_TraceList;
	int						m_LastTraceEntry;

	mutable QReadWriteLock	m_DriveLettersMutex;
	struct SDrive
	{
		QString Letter;
		QString NtPath;
		enum EType
		{
			EVolume = 0,
			EShare
		} Type;
		QString Aux;
	};
	QMap<QString, SDrive>	m_DriveLetters;

	QString					m_SbiePath;
	QString					m_IniPath;
	QFileSystemWatcher		m_IniWatcher;
	bool					m_IniReLoad;
	bool					m_bReloadPending;
	bool					m_bBoxesDirty;

	bool					m_bWithQueue;
	bool					m_bTerminate;

	CSbieIni*				m_pGlobalSection;
	CSbieIni*				m_pUserSection;
	QString					m_UserName;
	QString					m_UserSid;

	QString					m_ProgramDataDir;
	QString					m_PublicDir;
	QString					m_UserDir;

private:
	mutable QMutex			m_ThreadMutex;
	mutable QWaitCondition	m_ThreadWait;

	SB_STATUS CallServer(void* req, void* rpl) const;
	SB_STATUS SbieIniSet(void *RequestBuf, void *pPasswordWithinRequestBuf, const QString& SectionName, const QString& SettingName);
	struct SSbieAPI* m;
};
