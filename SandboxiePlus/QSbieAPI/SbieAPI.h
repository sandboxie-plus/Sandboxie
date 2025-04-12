/*
 * 
 * Copyright (c) 2020-2023, David Xanatos
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
	static  bool			IsWow64();

	virtual QString			GetVersion();

	virtual SB_STATUS		TakeOver();
	virtual SB_STATUS		WatchIni(bool bEnable = true, bool bReLoad = true);

	virtual QString			GetSbiePath() const { return m_SbiePath; }
	virtual QString			GetIniPath() const { return m_IniPath; }

	virtual QString			ResolveAbsolutePath(const QString& Path);

	virtual void			UpdateDriveLetters();
	virtual QString			Nt2DosPath(QString NtPath, bool* pOk = NULL) const;
	static quint32			GetVolumeSN(const wchar_t* path, std::wstring* pLabel = NULL);

	virtual SB_STATUS		ReloadBoxes(bool bForceUpdate = false);
	static  SB_STATUS		ValidateName(const QString& BoxName);
	virtual QString			MkNewName(QString Name);
	virtual SB_STATUS		CreateBox(const QString& BoxName, bool bReLoad = true);

	virtual SB_STATUS		UpdateProcesses(int iKeep, bool bAllSessions);

	virtual QMap<QString, CSandBoxPtr> GetAllBoxes() { return m_SandBoxes; }
	virtual QMap<quint32, CBoxedProcessPtr> GetAllProcesses() { return m_BoxedProxesses; }

	virtual CSandBoxPtr		GetBoxByProcessId(quint32 ProcessId) const;
	virtual CSandBoxPtr		GetBoxByName(const QString &BoxName) const { return m_SandBoxes.value(BoxName.toLower()); }
	virtual CBoxedProcessPtr GetProcessById(quint32 ProcessId) const;

	virtual SB_STATUS		TerminateAll(bool bNoExceptions = false);

	virtual SB_STATUS		SetProcessExemption(quint32 process_id, quint32 action_id, bool NewState);
	virtual bool			GetProcessExemption(quint32 process_id, quint32 action_id);

	virtual QString			GetBoxedPath(const QString& BoxName, const QString& Path);
	virtual QString			GetBoxedPath(CSandBox* pBox, const QString& Path, const QString& Snapshot = QString());
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
	virtual QString			SbieIniGet2(const QString& Section, const QString& Setting, quint32 Index = 0, bool bWithGlobal = false, bool bNoExpand = true, bool withTemplates = false);
	virtual QString			SbieIniGetEx(const QString& Section, const QString& Setting);
	virtual SB_STATUS		SbieIniSet(const QString& Section, const QString& Setting, const QString& Value, ESetMode Mode = eIniUpdate, bool bRefresh = true);
	virtual bool			IsBox(const QString& BoxName, bool& bIsEnabled);
	virtual QSharedPointer<CSbieIni> GetGlobalSettings() const { return m_pGlobalSection; }
	virtual QSharedPointer<CSbieIni> GetUserSettings() const { return m_pUserSection; }
	virtual QString			GetCurrentUserName() const { return m_UserName; }
	virtual QString			GetCurrentUserSid() const { return m_UserSid; }
	virtual bool			IsConfigLocked();
	virtual SB_STATUS		UnlockConfig(const QString& Password);
	virtual SB_STATUS		LockConfig(const QString& NewPassword);
	virtual void			ClearPassword();

	virtual SB_RESULT(QByteArray) RC4Crypt(const QByteArray& Data);

	virtual bool			GetDriverInfo(quint32 InfoClass, void* pBuffer, size_t Size);

	enum EFeatureFlags
	{
		eSbieFeatureWFP			= 0x00000001,
		eSbieFeatureObCB		= 0x00000002,
		eSbieFeaturePMod		= 0x00000004,
		eSbieFeatureAppC		= 0x00000008,
		eSbieFeatureSbiL		= 0x00000010,

		eSbieFeatureARM64		= 0x40000000,
		eSbieFeatureCert		= 0x80000000
	};

	virtual quint32			GetFeatureFlags();
	virtual QString			GetFeatureStr();
	virtual int				IsDyndataActive();

	// Forced Processes
	virtual SB_STATUS		DisableForceProcess(bool Set, int Seconds = 0);
	virtual bool			AreForceProcessDisabled();

	// Mount Manager
	virtual SB_STATUS		ImBoxCreate(CSandBox* pBox, quint64 uSizeKb, const QString& Password = QString());
	virtual SB_STATUS		ImBoxMount(CSandBox* pBox, const QString& Password = QString(), bool bProtect = false, bool bAutoUnmount = false);
	virtual SB_STATUS		ImBoxUnmount(CSandBox* pBox);
	virtual SB_RESULT(QStringList) ImBoxEnum();
	virtual SB_RESULT(QVariantMap) ImBoxQuery(const QString& Root = QString());
	//virtual SB_STATUS		ImBoxUpdate(  // todo

	// Monitor
	virtual SB_STATUS		EnableMonitor(bool Enable);
	virtual bool			IsMonitoring();

	virtual const QVector<CTraceEntryPtr>& GetTrace();
	virtual int				GetTraceCount() const { return m_TraceList.count(); }
	virtual void			ClearTrace() { m_TraceList.clear(); QMutexLocker Lock(&m_TraceMutex); m_TraceCache.clear(); }

	// Other
	virtual quint64			QueryProcessInfo(quint32 ProcessId, quint32 InfoClass = 0);

	virtual QString			GetSbieMsgStr(quint32 code, quint32 Lang = 1033);

	enum EStartFlags
	{
		eStartDefault = 0,
		eStartElevated = 1,
		eStartFCP = 2
	};

	virtual SB_RESULT(quint32) RunStart(const QString& BoxName, const QString& Command, EStartFlags Flags = eStartDefault, const QString& WorkingDir = QString(), QProcess* pProcess = NULL);
	virtual QString			GetStartPath() const;

	virtual quint32			GetSessionID() const;

	virtual SB_STATUS		SetSecureParam(const QString& Name, const void* data, size_t size);
	virtual SB_STATUS		GetSecureParam(const QString& Name, void* data, size_t size, quint32* size_out = NULL, bool bVerify = false);

	virtual bool			TestSignature(const QByteArray& Data, const QByteArray& Signature);

	virtual SB_STATUS		SetDatFile(const QString& FileName, const QByteArray& Data);
	//virtual SB_RESULT(QByteArray) GetDatFile(const QString& FileName);


	enum ESbieQueuedRequests
	{
		ePrintSpooler = -1,
		eInvalidQueuedRequests = 0,
		eFileMigration = 1,
		eInetBlockade = 2,
	};

	void					LoadEventLog();

	virtual SB_RESULT(int)	RunUpdateUtility(const QStringList& Params, quint32 Elevate = 0, bool Wait = false);

public slots:
	virtual void			SendQueueRpl(quint32 RequestId, const QVariantMap& Result);

signals:
	void					StatusChanged();
	void					ConfigReloaded();
	//void					LogMessage(const QString& Message, bool bNotify = true);
	void					LogSbieMessage(quint32 MsgCode, const QStringList& MsgData, quint32 ProcessId);
	void					ProcessBoxed(quint32 ProcessId, const QString& Path, const QString& Box, quint32 ParentId, const QString& CmdLine);
	void					FileToRecover(const QString& BoxName, const QString& FilePath, const QString& BoxPath, quint32 ProcessId);

	void					BoxAdded(const CSandBoxPtr& pBox);
	void					BoxOpened(const CSandBoxPtr& pBox);
	void					BoxClosed(const CSandBoxPtr& pBox);
	void					BoxRemoved(const CSandBoxPtr& pBox);

	void					NotAuthorized(bool bLoginRequired, bool &bRetry);
	void					QueuedRequest(quint32 ClientPid, quint32 ClientTid, quint32 RequestId, const QVariantMap& Data);

protected slots:
	virtual void			OnIniChanged(const QString &path);
	virtual void			OnReloadConfig();
	virtual CBoxedProcessPtr OnProcessBoxed(quint32 ProcessId, const QString& Path, const QString& Box, quint32 ParentId, const QString& CmdLine);

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

	virtual bool			GetQueueReq();
	virtual bool			GetLog();
	virtual bool			GetMonitor();

	virtual SB_STATUS		TerminateAll(const QString& BoxName);
	virtual SB_STATUS		Terminate(quint32 ProcessId);

	virtual SB_STATUS		SetSuspendedAll(const QString& BoxName, bool bSuspended);
	virtual SB_STATUS		SetSuspended(quint32 ProcessId, bool bSuspended);

	virtual SB_STATUS		RunSandboxed(const QString& BoxName, const QString& Command, QString WrkDir = QString(), quint32 Flags = 0);

	virtual SB_STATUS		UpdateBoxPaths(CSandBox* pSandBox);
	virtual SB_STATUS		UpdateProcessInfo(const CBoxedProcessPtr& pProcess);

	virtual SB_STATUS		GetProcessInfo(quint32 ProcessId, quint32* pParentId = NULL, quint32* pInfo = NULL, bool* pSuspended = NULL, QString* pImagePath = NULL, QString* pCommandLine = NULL, QString* pWorkingDir = NULL);

	virtual void			GetUserPaths();

	virtual QString			GetDeviceMap();
	virtual QByteArray		MakeEnvironment(bool AddDeviceMap);

	virtual void			run();

	QMap<QString, CSandBoxPtr> m_SandBoxes;
	QMap<quint32, CBoxedProcessPtr> m_BoxedProxesses;

	mutable QMutex			m_TraceMutex;
	QVector<CTraceEntryPtr>	m_TraceCache;
	QVector<CTraceEntryPtr>	m_TraceList;

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

	QSharedPointer<CSbieIni>m_pGlobalSection;
	QSharedPointer<CSbieIni>m_pUserSection;
	QString					m_UserName;
	QString					m_UserSid;

	QString					m_ProgramDataDir;
	QString					m_PublicDir;
	QString					m_UserDir;

public:
	
	struct SScopedVoid {
		~SScopedVoid()					{ if (ptr) free(ptr); }

		inline void Assign(void* p, size_t s) { Q_ASSERT(!ptr); ptr = p; size = s; }

		inline size_t Size()			{return size;}

	protected:
		SScopedVoid(void* p) : ptr(p), size(0) {}
		void* ptr;
		size_t size;
	};

	template <typename T>
	struct SScoped : public SScopedVoid {
		SScoped(void* p = NULL) : SScopedVoid(p) {}

		inline T* Detache()				{T* p = ptr; ptr = NULL; return p;}
		inline T* Value()				{return (T*)ptr;}
		inline T* operator->() const	{return (T*)ptr;}
		inline T& operator*() const     {return *((T*)ptr);}
		inline operator T*() const		{return (T*)ptr;}

	private:
		SScoped(const SScoped& other)	{} // copying is explicitly forbidden
		SScoped<T>& operator=(T* p)	{return *this;}
		SScoped<T>& operator=(const SScoped<T>& other)	{return *this;}
	};

private:
	mutable QMutex			m_ThreadMutex;
	mutable QWaitCondition	m_ThreadWait;

	SB_STATUS CallServer(void* req, SScopedVoid* prpl) const;
	SB_STATUS SbieIniSet(void *RequestBuf, void *pPasswordWithinRequestBuf, const QString& SectionName, const QString& SettingName);
	struct SSbieAPI* m;
};
