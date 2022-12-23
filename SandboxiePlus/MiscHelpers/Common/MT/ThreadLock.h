#pragma once

#include "../../mischelpers_global.h"

#include <QMutex>
#include <QWaitCondition>

/**********************************************************************************************
* CThreadLock
* This class allows to wait for events from a different thread.
* When Lock returns successfully, it is guaranteed that the other thread has issued a release.
* This works regardless of the real order in which Lock and Release were called.
*/

class MISCHELPERS_EXPORT CThreadLock
{
public:
	CThreadLock();

	void		Reset();
	bool		Lock(unsigned long time = ULONG_MAX);
	void		Release();

protected:
	QMutex				m_Mutex;
	QWaitCondition		m_Wait;
	bool				m_Released;
};
