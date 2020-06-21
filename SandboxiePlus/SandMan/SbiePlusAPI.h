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

	virtual bool			HasLogApi() const { return m_bLogApiFound; }
	virtual bool			NoAnonymousLogon() const { return m_bNoAnonymousLogon; }
	virtual bool			HasOpenToken() const { return m_bHasOpenToken; }

protected:
	bool					m_bLogApiFound;
	bool					m_bNoAnonymousLogon;
	bool					m_bHasOpenToken;

};