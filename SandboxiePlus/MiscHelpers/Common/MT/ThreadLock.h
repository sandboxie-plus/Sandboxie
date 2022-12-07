#pragma once

#include "../../mischelpers_global.h"

#include <QMutex>
#include <QWaitCondition>

/**********************************************************************************************
* CThreadLock
* This class allows to wait for events form a differet thread.
* When Lock returns successfully it is guaranteed that the other thread have issued a release.
* This works undependant of the real order Lock and Release ware called
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
