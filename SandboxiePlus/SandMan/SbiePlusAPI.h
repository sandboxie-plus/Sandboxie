#pragma once

#include "../QSbieAPI/SbieAPI.h"
#include "../QSbieAPI/Sandboxie/SbieTemplates.h"
#include "BoxJob.h"

enum ESbieExMsgCodes
{
	SBX_FirstError = SB_LastError,
	SBX_7zNotReady,
	SBX_7zCreateFailed,
	SBX_7zOpenFailed,
	SBX_7zExtractFailed,
	SBX_FailedCopyDir,
	SBX_NotBoxArchive
};

class CSbiePlusAPI : public CSbieAPI
{
	Q_OBJECT
public:
	CSbiePlusAPI(QObject* parent);
	virtual ~CSbiePlusAPI();

	virtual void			UpdateWindowMap();

	virtual QString			GetProcessTitle(quint32 pid) { return m_WindowMap.value(pid); }

	virtual bool			IsRunningAsAdmin();

	virtual bool			IsBusy() const { return m_JobCount > 0; }

	virtual void			StopMonitor();

	virtual SB_RESULT(quint32) RunStart(const QString& BoxName, const QString& Command, EStartFlags Flags = eStartDefault, const QString& WorkingDir = QString(), QProcess* pProcess = NULL);
	virtual bool			IsStarting(qint64 pid) const { return m_PendingStarts.contains(pid); }

private slots:
	virtual void			OnStartFinished();
	virtual void			SbieIniSetSection(const QString& Section, const QString& Value) { SbieIniSet(Section, "", Value); }

signals:
	void					BoxCleaned(CSandBoxPlus* pBoxEx);

protected:
	friend class CSandBoxPlus;
	friend class CNewBoxWizard;

	virtual CSandBox*		NewSandBox(const QString& BoxName, class CSbieAPI* pAPI);
	virtual CBoxedProcess*	NewBoxedProcess(quint32 ProcessId, class CSandBox* pBox);

	virtual SB_STATUS		UpdateBoxPaths(CSandBox* pSandBox) { return CSbieAPI::UpdateBoxPaths(pSandBox); }

	virtual CBoxedProcessPtr OnProcessBoxed(quint32 ProcessId, const QString& Path, const QString& Box, quint32 ParentId, const QString& CmdLine);

	int						m_JobCount;
	QMultiMap<quint32, QString> m_WindowMap;

	friend class CSandBoxPlus;
	class CBoxMonitor*		m_BoxMonitor;

	QSet<qint64>			m_PendingStarts;
};

///////////////////////////////////////////////////////////////////////////////
// CSandBoxPlus
//

class CSandBoxPlus : public CSandBox
{
	Q_OBJECT
public:
	CSandBoxPlus(const QString& BoxName, class CSbieAPI* pAPI);
	virtual ~CSandBoxPlus();

	virtual QString			GetDisplayName() const;

	SB_PROGRESS				ExportBox(const QString& FileName, const QString& Password = "", int Level = 5, bool Solid = false);
	SB_PROGRESS				ImportBox(const QString& FileName, const QString& Password);

	SB_PROGRESS				CopyBox(const QString& DestDir);

	virtual void			UpdateDetails();

	virtual void			ScanStartMenu();

	virtual void			SetBoxPaths(const QString& FilePath, const QString& RegPath, const QString& IpcPath);

	virtual void			OpenBox();
	virtual void			CloseBox();

	virtual SB_PROGRESS		CleanBox()							{ BeginModifyingBox(); SB_PROGRESS Status = CSandBox::CleanBox(); ConnectEndSlot(Status); return Status; }
	virtual SB_STATUS		RenameBox(const QString& NewName);
	virtual SB_STATUS		RemoveBox();
	virtual SB_PROGRESS		TakeSnapshot(const QString& Name)	{ BeginModifyingBox(); SB_PROGRESS Status = CSandBox::TakeSnapshot(Name); ConnectEndSlot(Status); return Status; }
	virtual SB_PROGRESS		RemoveSnapshot(const QString& ID)	{ BeginModifyingBox(); SB_PROGRESS Status = CSandBox::RemoveSnapshot(ID); ConnectEndSlot(Status); return Status; }
	virtual SB_PROGRESS		SelectSnapshot(const QString& ID)	{ BeginModifyingBox(); SB_PROGRESS Status = CSandBox::SelectSnapshot(ID); ConnectEndSlot(Status); return Status; }

	virtual SB_STATUS		ImBoxMount(const QString& Password = QString(), bool bProtect = false, bool bAutoUnmount = false) { BeginModifyingBox(); SB_STATUS Status = CSandBox::ImBoxMount(Password, bProtect, bAutoUnmount); ConnectEndSlot(Status); return Status; }
	virtual SB_STATUS		ImBoxUnmount()						{ BeginModifyingBox(); SB_STATUS Status = CSandBox::ImBoxUnmount(); if(!Status.IsError()) m_Mount.clear(); ConnectEndSlot(Status); return Status; }

	virtual bool			IsEmpty() const;

	virtual QString			GetStatusStr() const;

	virtual QString			GetBoxImagePath() const				{ return m_FilePath + ".box"; }

	virtual void			SetINetBlock(bool bEnable);
	virtual bool			IsINetBlocked() const				{ return m_bINetBlocked; }

	virtual void			SetAllowShares(bool bEnable);
	virtual bool			HasSharesAccess() const				{ return m_bSharesAllowed; }

	virtual void			SetDropRights(bool bEnable);
	virtual bool			IsDropRights() const				{ return m_bDropRights; }

	virtual bool			UseRamDisk() const					{ return m_bRamDisk; }
	virtual bool			UseImageFile() const				{ return m_bImageFile; }

	virtual bool			IsUnsecureDebugging() const			{ return m_iUnsecureDebugging != 0; }

	virtual void			BlockProgram(const QString& ProgName);
	virtual void			SetInternetAccess(const QString& ProgName, bool bSet);
	virtual bool			HasInternetAccess(const QString& ProgName);
	virtual void			SetForcedProgram(const QString& ProgName, bool bSet);
	virtual bool			IsForcedProgram(const QString& ProgName);
	virtual void			SetLingeringProgram(const QString& ProgName, bool bSet);
	virtual int				IsLingeringProgram(const QString& ProgName);
	virtual void			SetLeaderProgram(const QString& ProgName, bool bSet);
	virtual int				IsLeaderProgram(const QString& ProgName);

	virtual bool			IsEmptyCached() const { return m_IsEmpty; }

	virtual void			UpdateSize(bool bReset = true);
	virtual quint64			GetSize() const						{ if(m_TotalSize == -1) return 0; return m_TotalSize; }
	virtual void			SetSize(quint64 Size);				//{ m_TotalSize = Size; }
	virtual bool			IsSizePending() const;

	virtual bool			IsBoxexPath(const QString& Path);

	virtual bool			IsRecoverySuspended() const			{ return m_SuspendRecovery; }
	virtual void			SetSuspendRecovery(bool bSet = true) { m_SuspendRecovery = bSet; }

	virtual QString			MakeBoxCommand(const QString& FileName);
	virtual QString			GetCommandFile(const QString& Command, QString* Arguments = NULL);
	virtual QString			GetFullCommand(const QString& Command);

	const QSet<QString>&	GetRecentPrograms()					{ return m_RecentPrograms; }

	enum EBoxTypes
	{
		eHardenedPlus,
		eHardened,
		eDefaultPlus,
		eDefault,
		eAppBoxPlus,
		eAppBox,

		eInsecure,
		eOpen,

		ePrivatePlus,
		ePrivate,

		eUnknown
	};

	EBoxTypes				GetType() const { return m_BoxType; }
	bool					IsAutoDelete() const { return m_BoxDel; }
	bool					IsForceDisabled() const { return m_NoForce; }
	QRgb					GetColor() const { return m_BoxColor; }
	
	class COptionsWindow*	m_pOptionsWnd;
	class CRecoveryWindow*	m_pRecoveryWnd;

	bool					IsBoxBusy() const { return IsSizePending() || !m_JobQueue.isEmpty(); }
	SB_STATUS				DeleteContentAsync(bool DeleteSnapshots = true, bool bOnAutoDelete = false);

	struct SLink {
		SLink() :Url(false), IconIndex(0) {}
		QString Folder;
		QString Name;
		bool Url;
		QString Target;
		QString Arguments;
		QString Icon;
		int IconIndex;
		QString WorkDir;
	};

	QList<SLink>			GetStartMenu() const { return m_StartMenu.values(); }

signals:
	void					AboutToBeModified();
	void					StartMenuChanged();

public slots:
	void					OnAsyncFinished();
	void					OnAsyncMessage(const QString& Text);
	void					OnAsyncProgress(int Progress);
	void					OnCancelAsync();

protected slots:
	virtual	void			BeginModifyingBox();
	virtual	void			EndModifyingBox();

protected:
	friend class CSbiePlusAPI;

	struct SFoundLink {
		QString Snapshot;
		QString LinkPath;
		QString RealPath;
		QString SubPath;
	};

	virtual void			ConnectEndSlot(const SB_PROGRESS& Status);

	virtual bool			CheckUnsecureConfig() const;
	EBoxTypes				GetTypeImpl() const;

	virtual bool			TestProgramGroup(const QString& Group, const QString& ProgName);
	virtual void			EditProgramGroup(const QString& Group, const QString& ProgName, bool bSet);

	void					AddJobToQueue(CBoxJob* pJob);
	void					StartNextJob();

	static void				ExportBoxAsync(const CSbieProgressPtr& pProgress, const QString& ExportPath, const QString& RootPath, const QString& Section, const QVariantMap& Params);
	static void				ImportBoxAsync(const CSbieProgressPtr& pProgress, const QString& ImportPath, const QString& RootPath, const QString& BoxName, const QString& Password);

	static void				CopyBoxAsync(const CSbieProgressPtr& pProgress, const QString& SrcDir, const QString& DestDir);

	bool					IsFileDeleted(const QString& RealPath, const QString& Snapshot, const QStringList& SnapshotList, const QMap<QString, QList<QString>>& DeletedPaths);

	QList<QSharedPointer<CBoxJob>> m_JobQueue;

	bool					m_bINetBlocked;
	bool					m_bINetExceptions;
	bool					m_bSharesAllowed;
	bool					m_bDropRights;

	bool					m_bRamDisk;
	bool					m_bImageFile;

	bool					m_bSecurityEnhanced;
	bool					m_bPrivacyEnhanced;
	bool					m_bApplicationCompartment;
	int						m_iUnsecureDebugging;
	bool					m_bEncryptedAndConfidential;
	bool					m_bRootAccessOpen;

	quint64					m_TotalSize;

	bool					m_SuspendRecovery;
	bool					m_IsEmpty;
	QString					m_StatusStr;

	QSet<QString>			m_RecentPrograms;

	QMap<QString, SLink>	m_StartMenu;

	EBoxTypes				m_BoxType;
	bool					m_BoxDel;
	bool					m_NoForce;
	QRgb					m_BoxColor;
	QString					m_BoxAlias;
};

///////////////////////////////////////////////////////////////////////////////
// CSbieTemplatesEx
//

class CSbieTemplatesEx : public CSbieTemplates
{
	Q_OBJECT
public:
	CSbieTemplatesEx(class CSbieAPI* pAPI, QObject* paretn = 0) 
		:CSbieTemplates(pAPI, paretn) {}

	virtual void Reset();

	QList<QVariantMap> GetUpdates();

	bool CheckUpdates(const QString& Value);

protected:
	void    CollectUpdates();

	QList<QVariantMap> m_Updates;
};
