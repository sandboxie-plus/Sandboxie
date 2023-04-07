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

	virtual bool	HasElevatedToken()							{ return m_ProcessInfo.IsElevated; }
	virtual bool	HasSystemToken()							{ return m_ProcessInfo.IsSystem; }
	virtual bool	HasRestrictedToken()						{ return m_ProcessInfo.IsRestricted; }
	virtual bool	HasAppContainerToken()						{ return m_ProcessInfo.IsAppContainer; }

protected:

	virtual void			InitProcessInfoImpl(void* ProcessHandle);

	QMap<int, int>			m_RememberedActions;

	// Flags
	union
	{
		quint32 Flags;
		struct
		{
			quint32
				IsElevated : 1,
				IsSystem : 1,
				IsRestricted : 1,
				IsAppContainer : 1,
				Spare : 28;
		};
	}						m_ProcessInfo;
};