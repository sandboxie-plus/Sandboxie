#pragma once

#include "../QSbieAPI/SbieStatus.h"
#include "../QSbieAPI/SbieAPI.h"

class CSandBoxPlus;

class CBoxJob : public QObject
{
	Q_OBJECT
public:
	CBoxJob(QObject* parent = NULL) : QObject(parent) { }

	virtual SB_PROGRESS	Start() = 0;
	virtual void		Finished() = 0;

	CSbieProgressPtr	GetProgress()	{ return m_pProgress; }
	QString				GetDescription(){ return m_Description; }

	CSandBoxPlus*		GetBox()		{ return (CSandBoxPlus*)parent(); }

protected:
	CSbieProgressPtr	m_pProgress;
	QString				m_Description;
};

///////////////////////////////////////////////////////////////////////////////
// CCleanUpBoxJob
//

class CCleanUpJob : public CBoxJob
{
protected:
	friend CSandBoxPlus;
	CCleanUpJob(CSandBoxPlus* pBox, bool DeleteSnapshots = true, bool bOnAutoDelete = false) : CBoxJob((QObject*)pBox) { 
		m_Description = tr("Deleting Content");
		m_DeleteSnapshots = DeleteSnapshots; 
		m_bOnAutoDelete = bOnAutoDelete;
	}

	virtual SB_PROGRESS	Start();
	virtual void		Finished();

protected:
	bool m_DeleteSnapshots;
	bool m_bOnAutoDelete;
};

///////////////////////////////////////////////////////////////////////////////
// COnDeleteJob
//

class COnDeleteJob : public CBoxJob
{
protected:
	friend CSandBoxPlus;
	COnDeleteJob(CSandBoxPlus* pBox, const QString& Command) : CBoxJob((QObject*)pBox) { 
		m_Description = tr("OnDelete: %1").arg(Command);
		m_Command = Command; 
	}

	virtual SB_PROGRESS	Start();
	virtual void		Finished() {}

protected:
	QString m_Command;
};