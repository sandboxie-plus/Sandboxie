#pragma once

#include "../QSbieAPI/SbieAPI.h"
#include "BoxJob.h"


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

protected:
	friend class CSandBoxPlus;

	virtual CSandBox*		NewSandBox(const QString& BoxName, class CSbieAPI* pAPI);
	virtual CBoxedProcess*	NewBoxedProcess(quint32 ProcessId, class CSandBox* pBox);

	virtual CBoxedProcessPtr OnProcessBoxed(quint32 ProcessId, const QString& Path, const QString& Box, quint32 ParentId);

	int						m_JobCount;
	QMultiMap<quint32, QString> m_WindowMap;
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

	virtual void			UpdateDetails();

	virtual void			CloseBox();

	virtual QString			GetStatusStr() const;

	virtual void			SetLogApi(bool bEnable);
	virtual bool			HasLogApi() const					{ return m_bLogApiFound; }

	virtual void			SetINetBlock(bool bEnable);
	virtual bool			IsINetBlocked() const				{ return m_bINetBlocked; }

	virtual void			SetAllowShares(bool bEnable);
	virtual bool			HasSharesAccess() const				{ return m_bSharesAllowed; }

	virtual void			SetDropRights(bool bEnable);
	virtual bool			IsDropRights() const				{ return m_bDropRights; }

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

	virtual bool			IsRecoverySuspended() const			{ return m_SuspendRecovery; }
	virtual void			SetSuspendRecovery(bool bSet = true) { m_SuspendRecovery = bSet; }

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

		eUnknown
	};

	EBoxTypes				GetType() const;
	
	class COptionsWindow*	m_pOptionsWnd;
	class CRecoveryWindow*	m_pRecoveryWnd;

	bool					IsBusy() const { return !m_JobQueue.isEmpty(); }
	SB_STATUS				DeleteContentAsync(bool DeleteShapshots = true, bool bOnAutoDelete = false);

public slots:
	void					OnAsyncFinished();
	void					OnAsyncMessage(const QString& Text);
	void					OnAsyncProgress(int Progress);
	void					OnCancelAsync();

protected:
	friend class CSbiePlusAPI;
	virtual bool			CheckUnsecureConfig() const;

	virtual bool			TestProgramGroup(const QString& Group, const QString& ProgName);
	virtual void			EditProgramGroup(const QString& Group, const QString& ProgName, bool bSet);

	void					AddJobToQueue(CBoxJob* pJob);
	void					StartNextJob();

	QList<QSharedPointer<CBoxJob>> m_JobQueue;

	bool					m_bLogApiFound;
	bool					m_bINetBlocked;
	bool					m_bSharesAllowed;
	bool					m_bDropRights;

	bool					m_bSecurityEnhanced;
	bool					m_bPrivacyEnhanced;
	bool					m_bApplicationCompartment;
	int						m_iUnsecureDebugging;

	bool					m_SuspendRecovery;
	QString					m_StatusStr;

	QSet<QString>			m_RecentPrograms;

};
