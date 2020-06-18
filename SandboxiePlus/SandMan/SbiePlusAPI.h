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
	virtual bool			IsOpenBox() const { return m_bIsOpenBox; }

protected:
	bool					m_bLogApiFound;
	bool					m_bIsOpenBox;

};