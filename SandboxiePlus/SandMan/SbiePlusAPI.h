#pragma once

#include "../QSbieAPI/SbieAPI.h"


class CSbiePlusAPI : public CSbieAPI
{
	Q_OBJECT
public:
	CSbiePlusAPI(QObject* parent);
	virtual ~CSbiePlusAPI();

	virtual void			UpdateWindowMap();

	virtual QString			GetProcessTitle(quint32 pid) { return m_WindowMap.value(pid); }

	virtual bool			IsRunningAsAdmin();

protected:
	virtual CSandBox*		NewSandBox(const QString& BoxName, class CSbieAPI* pAPI);
	virtual CBoxedProcess*	NewBoxedProcess(quint32 ProcessId, class CSandBox* pBox);

	virtual CBoxedProcessPtr OnProcessBoxed(quint32 ProcessId, const QString& Path, const QString& Box, quint32 ParentId);

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

	virtual bool			IsSecurityRestricted() const		{ return m_bSecurityRestricted; }
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
		eDefault = 0,
		eHardened,
		//eHasLogApi,
		eInsecure,

		eUnknown = -1
	};

	EBoxTypes				GetType() const;
	
	class COptionsWindow*	m_pOptionsWnd;
	class CRecoveryWindow*	m_pRecoveryWnd;

protected:
	friend class CSbiePlusAPI;
	virtual bool			CheckUnsecureConfig() const;

	virtual bool			TestProgramGroup(const QString& Group, const QString& ProgName);
	virtual void			EditProgramGroup(const QString& Group, const QString& ProgName, bool bSet);

	bool					m_bLogApiFound;
	bool					m_bINetBlocked;
	bool					m_bSharesAllowed;
	bool					m_bDropRights;

	bool					m_bSecurityRestricted;
	int						m_iUnsecureDebugging;

	bool					m_SuspendRecovery;

	QSet<QString>			m_RecentPrograms;
};

///////////////////////////////////////////////////////////////////////////////
// CSbieProcess
//

class CSbieProcess : public CBoxedProcess
{
	Q_OBJECT
public:
	CSbieProcess(quint32 ProcessId, class CSandBox* pBox) : CBoxedProcess(ProcessId, pBox) {}

	virtual QString	GetStatusStr() const;

	virtual void BlockProgram()									{ GetBox()->BlockProgram(m_ImageName); }
	virtual void SetInternetAccess(bool bSet)					{ GetBox()->SetInternetAccess(m_ImageName, bSet); }
	virtual bool HasInternetAccess()							{ return GetBox()->HasInternetAccess(m_ImageName); }
	virtual void SetForcedProgram(bool bSet)					{ GetBox()->SetForcedProgram(m_ImageName, bSet); }
	virtual bool IsForcedProgram()								{ return GetBox()->IsForcedProgram(m_ImageName); }
	virtual void SetLingeringProgram(bool bSet)					{ GetBox()->SetLingeringProgram(m_ImageName, bSet); }
	virtual int	 IsLingeringProgram()							{ return GetBox()->IsLingeringProgram(m_ImageName); }
	virtual void SetLeaderProgram(bool bSet)					{ GetBox()->SetLeaderProgram(m_ImageName, bSet); }
	virtual int	 IsLeaderProgram()								{ return GetBox()->IsLeaderProgram(m_ImageName); }

	virtual CSandBoxPlus* GetBox()								{ return qobject_cast<CSandBoxPlus*>(m_pBox); }

	virtual int GetRememberedAction(int Action)					{ return m_RememberedActions.value(Action, -1); }
	virtual void SetRememberedAction(int Action, int retval)	{ m_RememberedActions.insert(Action, retval); }

	static QString ImageTypeToStr(quint32 type);

protected:
	QMap<int, int>			m_RememberedActions;
};