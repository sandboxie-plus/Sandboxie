#pragma once

#include "../../mischelpers_global.h"

#include <QMutex>
#include <QWaitCondition>

/**********************************************************************************************
* CThreadLock
* This class allows to wait for events form a differet thread.
* When Lock returns successfuly it is guaranted that the other thread have issued a release.
* This works undependant of the real order Lock and Release ware calles
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
