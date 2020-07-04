#pragma once

#include "../QSbieAPI/SbieAPI.h"


class CSbiePlusAPI : public CSbieAPI
{
	Q_OBJECT
public:
	CSbiePlusAPI(QObject* parent);
	virtual ~CSbiePlusAPI();


protected:
	virtual CSandBox*		NewSandBox(const QString& BoxName, class CSbieAPI* pAPI);
	virtual CBoxedProcess*	NewBoxedProcess(quint64 ProcessId, class CSandBox* pBox);

};

///////////////////////////////////////////////////////////////////////////////
// CSandBox
//

class CSandBoxPlus : public CSandBox
{
	Q_OBJECT
public:
	CSandBoxPlus(const QString& BoxName, class CSbieAPI* pAPI);
	virtual ~CSandBoxPlus();

	virtual void			UpdateDetails();

	virtual QString			GetStatusStr() const;

	virtual void			SetLogApi(bool bEnable);
	virtual bool			HasLogApi() const { return m_bLogApiFound; }

	virtual void			SetINetBlock(bool bEnable);
	virtual bool			IsINetBlocked() const { return m_bINetBlocked; }

	virtual void			SetAllowShares(bool bEnable);
	virtual bool			HasSharesAccess() const { return m_bSharesAllowed; }

	virtual void			SetDropRights(bool bEnable);
	virtual bool			IsDropRights() const { return m_bDropRights; }

	virtual bool			IsSecurityRestricted() const { return m_bSecurityRestricted; }
	virtual bool			IsUnsecureDebugging() const { return m_iUnsecureDebugging != 0; }

protected:
	virtual bool			CheckOpenToken() const;

	bool					m_bLogApiFound;
	bool					m_bINetBlocked;
	bool					m_bSharesAllowed;
	bool					m_bDropRights;

	bool					m_bSecurityRestricted;
	int						m_iUnsecureDebugging;
};