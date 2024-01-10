#pragma once

#include "SbiePlusAPI.h"

class CSbieProcess : public CBoxedProcess
{
	Q_OBJECT
public:
	CSbieProcess(quint32 ProcessId, class CSandBox* pBox);

	virtual QString	GetStatusStr() const;

	virtual void	BlockProgram()								{ GetBox()->BlockProgram(m_ImageName); }
	virtual void	SetInternetAccess(bool bSet)				{ GetBox()->SetInternetAccess(m_ImageName, bSet); }
	virtual bool	HasInternetAccess()							{ return GetBox()->HasInternetAccess(m_ImageName); }
	virtual void	SetForcedProgram(bool bSet)					{ GetBox()->SetForcedProgram(m_ImageName, bSet); }
	virtual bool	IsForcedProgram()							{ return GetBox()->IsForcedProgram(m_ImageName); }
	virtual void	SetLingeringProgram(bool bSet)				{ GetBox()->SetLingeringProgram(m_ImageName, bSet); }
	virtual int		IsLingeringProgram()						{ return GetBox()->IsLingeringProgram(m_ImageName); }
	virtual void	SetLeaderProgram(bool bSet)					{ GetBox()->SetLeaderProgram(m_ImageName, bSet); }
	virtual int		IsLeaderProgram()							{ return GetBox()->IsLeaderProgram(m_ImageName); }

	virtual CSandBoxPlus* GetBox()								{ return qobject_cast<CSandBoxPlus*>(m_pBox); }

	virtual int		GetRememberedAction(int Action)				{ return m_RememberedActions.value(Action, -1); }
	virtual void	SetRememberedAction(int Action, int retval)	{ m_RememberedActions.insert(Action, retval); }

	static QString	ImageTypeToStr(quint32 type);

protected:

	//virtual void			InitProcessInfoImpl(void* ProcessHandle);

	QMap<int, int>			m_RememberedActions;
};